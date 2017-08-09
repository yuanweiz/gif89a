#include <vector>
#include <string.h>
#include <assert.h>
#include "Decoder.h"
#include "Exception.h"
#include "StreamReader.h"
#include "Logging.h"
#include "Types.h"

using namespace std;

const uint8_t EXT_INTRO = 0x21;
const uint8_t DESC_INTRO = 0x2c;
const uint8_t TERM_INTRO = 0x3b; //aka TRAILER
//const uint8_t TERM = 0x00;
//
const uint8_t CMT_EXT = 0xfe;
const uint8_t APP_EXT = 0xff;
const uint8_t TXT_EXT = 0x01;
const uint8_t GRA_CTL_EXT = 0xf9;

struct Img{
    Img()=default;
    Img(const Img&) = delete;
    Img(Img&&) noexcept = default;
    unique_ptr<GraphicCtrlBlk> ctrlBlk; //optional
    ImgDesc desc;
    vector<Color> localColorTbl;
    vector<uint32_t> lzw;
};

class Decoder::Impl{
    public:
    explicit Impl(const char* fname)
        :reader_(fname)
    {
        staticCheck();
    }
    void decode(){
        parseHeader();
        //parse logical screen desc
        reader_.retrieveString(lgcScrDesc_.head(),lgcScrDesc_.size());
        if (lgcScrDesc_.g_color_tbl) 
        {
            parseGlobalColorTbl();
        }

        //parse body
        while(true){
            if (reader_.requireBytes(1)<1){
                throw ParseError("require introducer byte");
            }
            union {
                uint16_t u16;
                uint8_t u8[2];
            };
            u8[0] = reader_.peekInt8();
            if(u8[0]== EXT_INTRO ){
                if (reader_.requireBytes(2) != 2) break;
                u16 = reader_.peekInt16();
            }
            //uint8_t intro = reader_.readInt8();
            switch (u8[0]){
                case EXT_INTRO:
                    //special case
                    if (u8[1]==GRA_CTL_EXT ){
                        parseGraphicBlock();
                    }
                    else 
                        parseExtension();
                    break;
                case DESC_INTRO:
                    parseGraphicBlock();
                    break;
                case TERM_INTRO:
                    parseTerm();
                    return;
            }

        }
    }
    private:
    void staticCheck(){
        static_assert(sizeof(Color) == 3,"wrong pixel size");
        static_assert(sizeof(ImgDesc) == 12,"wrong image desc size");
        static_assert(sizeof(LgcScrDesc) == 8,"LgcScrDesc size");
        static_assert(sizeof(GraphicCtrlBlk) == 8,"GraphicCtrlBlk");
    }
    void parseHeader();
    void parseGlobalColorTbl();
    void parseExtension();
    void parseTerm();
    void parseDesc();
    void parseImageDesc();
    void parseGraphicBlock();
    void eatSubblock();
    void parseLzw(Img&);

    StreamReader reader_;
    LgcScrDesc lgcScrDesc_;
    vector<Color> glbColorTbl_;
    vector<Img> imgs_;
};


void Decoder::Impl::eatSubblock(){
    do{
        if (reader_.requireBytes(1)<1)break;
        uint8_t sz = reader_.readInt8();
        if (reader_.requireBytes(sz)<sz) break;
        reader_.retrieve(sz);
    }while(false);
    throw ParseError("eatSubblock()");
}
void Decoder::Impl::parseTerm(){
    //Trailer
    if (reader_.peekInt8()!= TERM_INTRO)
        throw ParseError("expect 0x3b");
}
void Decoder::Impl::parseExtension(){
    //skip extension
    uint8_t intro = reader_.readInt8();
    assert (intro == EXT_INTRO); (void) intro;
    uint8_t buf[2];
    uint8_t extType;
    uint8_t sz;
    do {
        if ( reader_.requireBytes(2)<2)break;
        reader_.retrieveString(buf,2);
        extType = buf[0];
        sz = buf[1];
        if ( reader_.requireBytes(sz) < sz)break; 
        //if (reader_.requireBytes(1) < 1) break; 
        switch (extType){
            case APP_EXT:
            case TXT_EXT:
                reader_.retrieve(buf[1]);
                //more subblocks
                eatSubblock();
                break;
            case GRA_CTL_EXT:
                assert(false);
                break;
            case CMT_EXT:
            default:
                reader_.retrieve(buf[1]);
                //no more sub-blocks
        }
        if (reader_.readInt8()!=0x00)break; //NULL terminator
        return;
    }while (false);
    throw ParseError("broken extension block");
}

void Decoder::Impl::parseGraphicBlock(){
    union {
        uint16_t u16;
        uint8_t u8[2];
    };
    u16 = reader_.peekInt16();
    Img img;
    do {
        if (u8[0]==EXT_INTRO ){
            //optional block
            img.ctrlBlk.reset(new GraphicCtrlBlk);
            GraphicCtrlBlk & graphicCtrlBlk=*img.ctrlBlk;
            size_t sz = sizeof (GraphicCtrlBlk);
            if (reader_.requireBytes(sz) < sz) break;
            reader_.retrieveString(&graphicCtrlBlk, sz);
            if ( graphicCtrlBlk.intro!= EXT_INTRO 
                    || graphicCtrlBlk.label != GRA_CTL_EXT
                    || graphicCtrlBlk.sz != 4)
                break;
        }
        //read image Descriptor
        ImgDesc & desc=img.desc;
        if (reader_.requireBytes(desc.size())< desc.size()) break;
        reader_.retrieveString(desc.head(),desc.size());
        if (desc.imgSep != DESC_INTRO) break;
        auto & localColorTbl = img.localColorTbl;
        if (desc.localTbl) {
            //optional local color table
            size_t nBytes = desc.localTblSz*3;
            auto avail = reader_.requireBytes(nBytes);
            if ( avail < nBytes) {
                break;
            }
            reader_.retrieveString(localColorTbl.data(), nBytes);
        }
        parseLzw(img);
        auto term = reader_.readInt8();
        if (term!=0x00) break;
        imgs_.push_back(std::move(img));
        return;
    }while(false);
    throw ParseError("GraphicCtrlBlk");
}
void Decoder::Impl::parseLzw(Img & img){
    uint8_t u8 = reader_.readInt8();
    uint32_t lzwCodeSz = u8; //boost to int32
    uint8_t blkSz = reader_.readInt8();
    uint32_t nBits=lzwCodeSz+1;

    const uint32_t CLEAR=(1<<lzwCodeSz);
    const uint32_t STOP = CLEAR+1;
    std::vector<std::string> dict(STOP);
    auto & res = img.lzw;
    uint32_t prevCode=0;
    bool first = true;

    int maxBits=blkSz*8;
    while (true){
        if ( static_cast<size_t>(1<<nBits)< dict.size())
            nBits++;
        maxBits-=nBits;
        if ( maxBits < 0) break;
        auto currIdx= reader_.readBits(nBits);
        if (currIdx==STOP){
            break;
        }
        else if (currIdx == CLEAR){
            dict.clear();
            dict.resize(STOP);
            continue;
        }
        else if (currIdx < CLEAR){
            res.push_back(currIdx);
            continue;
        } 
        auto & curr = dict[currIdx];
        if (first) {
            //FIXME: very inefficient
            std::copy(curr.begin(),curr.end(), std::back_inserter(res));
            continue;
        }
        auto & prev = dict[prevCode]; //DANGEROUS for malformed input
        std::string newEntry;
        assert (currIdx <=dict.size());
        if (currIdx == dict.size()){
            newEntry = prev+prev[0];
        }
        else {
            newEntry = prev+curr[0];
        }
        dict.push_back(newEntry);
        //LOG_DEBUG << "New entry: " << dict.size()-1 <<"=" << newEntry;
        std::copy(curr.begin(),curr.end(), std::back_inserter(res));
    }

    /*
       for (int maxBits = blkSz * 8; maxBits>=0 ;){
       maxBits-=nBits;
       if (maxBits <0)
       break;
       uint32_t tmp = reader_.readBits(nBits);
       img.lzw.push_back(tmp);
       LOG_DEBUG << tmp;
       }
       */
    nBits = reader_.remainingBits();
    if (nBits < 8)
        reader_.readBits(nBits);
    assert(reader_.remainingBits()==8);
}
void Decoder::Impl::parseGlobalColorTbl()
{
    //parse global table here
    LOG_DEBUG << "parse global color table";
    int colorTblSz = 2 << (lgcScrDesc_.g_color_tbl_sz);
    glbColorTbl_.resize(colorTblSz);
    size_t nBytes = colorTblSz * 3;
    if (reader_.requireBytes(nBytes) < nBytes) {
        throw ParseError("broken global color table");
    }
    reader_.retrieveString(glbColorTbl_.data(), 3 * colorTblSz);
}
void Decoder::Impl::parseHeader(){
    do {
        char buf[3];
        if (reader_.requireBytes(6)<6) {
            break;
        }
        reader_.retrieveString(buf, 3);
        if (strncmp(buf, "GIF", 3)) {
            break;
        }
        reader_.retrieveString(buf, 3);
        if (0==strncmp(buf, "89a", 3)) {
            //TODO global conf
        }
        else if (0 == strncmp(buf, "87a", 3)) {
            //TODO global conf
        }
        else break;
        return;
    } while (false);
    throw ParseError("Bad header");
}

//forwarding
Decoder::Decoder(const char * fname)
    :impl_(new Impl(fname))
{
}
Decoder::~Decoder()
{
}

void Decoder::decode(){
    impl_->decode();
}
