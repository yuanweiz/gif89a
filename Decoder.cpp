#include <vector>
#include <string.h>
#include <assert.h>
#include "Decoder.h"
#include "Exception.h"
#include "StreamReader.h"
#include "Logging.h"
#include "Types.h"
class Decoder::Impl{
    public:
    explicit Impl(const char* fname)
        :reader_(fname)
    {
        staticCheck();
    }
    void decode(){
        reader_.readFile(65536);
        parseHeader();
        //parse logical screen desc
        reader_.retrieveString(lgcScrDesc_.head(),lgcScrDesc_.size());
        if (lgcScrDesc_.g_color_tbl) 
        {
            parseGlobalColorTbl();
        }
        while(true){
            //monkey patch
            {
                char buf[30];
                reader_.retrieveString(buf, 8);
            }
            //parse Image desc
            ImgDesc desc;
            memset(&desc, 0, sizeof(ImgDesc));
            //char buf[10];
            reader_.retrieveString(&desc.imgSep, 10);
            //reader_.retrieveString(buf, sizeof(buf));
            if (desc.imgSep != 0x2c) {
                throw Exception("bad image desc: seperator should be 0x2c");
            }
            std::vector<Color> localColorTbl(desc.localTblSz);
            if (desc.localTbl) {
                size_t nBytes = desc.localTblSz*3;
                auto avail = reader_.readable();
                if (avail < nBytes) {
                    LOG_DEBUG << "expect " << nBytes << " bytes but only got " << avail;
                    throw Exception("Broken local color table");
                }
                reader_.retrieveString(localColorTbl.data(), nBytes);
            }
            uint8_t lzwCodeSz = reader_.readInt8();
            uint8_t blkSz = reader_.readInt8();
            uint32_t nBits=lzwCodeSz+1;
            for (int maxBits = blkSz * 8; maxBits>=0 ;){
                maxBits-=nBits;
                if (maxBits <0)
                    break;
                uint32_t tmp = reader_.readBits(nBits);
                LOG_DEBUG << tmp;
            }
            nBits = reader_.remainingBits();
            if (nBits < 8)
                reader_.readBits(nBits);
            assert(reader_.remainingBits()==8);
            auto term = reader_.readInt8();
            if (term!=0x00){
                throw Exception("expect a zero terminator");
            }
        }
    }
    private:
    void staticCheck(){
        static_assert(sizeof(Color) == 3,"wrong pixel size");
        static_assert(sizeof(ImgDesc) == 12,"wrong image desc size");
        static_assert(sizeof(LgcScrDesc) == 8,"wrong logical desc size");
    }
    void parseHeader() {
        do {
            char buf[3];
            if (reader_.readable()<6) {
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
        throw Exception("Bad header");
    }
    void parseGlobalColorTbl(){
        //parse global table here
        LOG_DEBUG << "parse global color table";
        int colorTblSz = 2 << (lgcScrDesc_.g_color_tbl_sz);
        glbColorTbl_.resize(colorTblSz);
        if (reader_.readable() < colorTblSz * sizeof(Color)) {
            throw Exception("broken global color table");
        }
        reader_.retrieveString(glbColorTbl_.data(), 3 * colorTblSz);
    }

    StreamReader reader_;
    LgcScrDesc lgcScrDesc_;
    std::vector<Color> glbColorTbl_;
};


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
