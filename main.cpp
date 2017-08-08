#include "StreamReader.h"
#include "Exception.h"
#include "Logging.h"
#include <string.h>
#include <vector>
#include <assert.h>

uint16_t lgcScrWidth;
uint16_t lgcScrHeight;
struct {
	union {
		struct {
			//WARNING: Allocation of bit-fields within a class object is
			//implementation-defined. Alignment of 
			//bit-fields is implementation-defined.
			uint8_t g_color_tbl_sz : 3;
			uint8_t sort : 1;
			uint8_t color_res : 3;
			uint8_t g_color_tbl : 1;
		}u;
		uint8_t c;
	};
}lgcScrField;
uint8_t bgColorIdx;
uint8_t bgPixelRatio;
int colorTblSz = -1;

struct Color {
	uint8_t r, g, b;
};

struct ImgDesc {
	void * head() { return &imgSep; }
	static size_t size() { return 10; }
	uint8_t _pad1;
	uint8_t imgSep;
	uint16_t left;
	uint16_t top;
	uint16_t width;
	uint16_t height;
	uint8_t localTblSz : 3;
	uint8_t reserved : 2;
	uint8_t sort : 1;
	uint8_t interlace : 1;
	uint8_t localTbl : 1;
	uint8_t _pad2 ;
};

void parseHeader( StreamReader & reader) {
	do {
		char buf[3];
		if (reader.readable()<6) {
			break;
		}
		reader.retrieveString(buf, 3);
		if (strncmp(buf, "GIF", 3)) {
			break;
		}
		reader.retrieveString(buf, 3);
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

int main() {
    try {
        static_assert(sizeof(Color) == 3,"wrong pixel size");
        static_assert(sizeof(ImgDesc) == 12,"wrong image desc size");
        FILE * fp = ::fopen("/home/ywz/Pictures/a.gif", "rw");
        //fseek(fp, 0, SEEK_END);
        //auto sz = ftell(fp);
        //LOG_DEBUG << sz;
        //fseek(fp, 0, SEEK_SET);
        StreamReader reader(fp);
        reader.readFile(65536);
        parseHeader(reader);
        //parse global conf
        lgcScrHeight = reader.readInt16();
        lgcScrWidth = reader.readInt16();
        lgcScrField.c = reader.readInt8();
        bgColorIdx = reader.readInt8();
        bgPixelRatio = reader.readInt8();

        std::vector<Color> colorTbl;
        if (lgcScrField.u.g_color_tbl) {
            //parse global table here
            LOG_DEBUG << "parse global color table";
            colorTblSz = 2 << (lgcScrField.u.g_color_tbl_sz);
            colorTbl.resize(colorTblSz);
            if (reader.readable() < colorTblSz * sizeof(Color)) {
                throw Exception("broken global color table");
            }
            reader.retrieveString(colorTbl.data(), 3 * colorTblSz);
        }


        //monkey patch
        {
            char buf[30];
            reader.retrieveString(buf, 8);
        }
        //parse Image desc
        ImgDesc desc;
        memset(&desc, 0, sizeof(ImgDesc));
        //char buf[10];
        reader.retrieveString(&desc.imgSep, 10);
        //reader.retrieveString(buf, sizeof(buf));
        if (desc.imgSep != 0x2c) {
            throw Exception("bad image desc: seperator should be 0x2c");
        }
        std::vector<Color> localColorTbl(desc.localTblSz);
        if (desc.localTbl) {
            size_t nBytes = desc.localTblSz*3;
            auto avail = reader.readable();
            if (avail < nBytes) {
                LOG_DEBUG << "expect " << nBytes << " bytes but only got " << avail;
                throw Exception("Broken local color table");
            }
            reader.retrieveString(localColorTbl.data(), nBytes);
        }
        //StreamReader reader_(stdin);
        //parse table-based image data
        uint8_t lzwCodeSz = reader.readInt8();
        uint8_t blkSz = reader.readInt8();
        uint32_t nBits=lzwCodeSz+1;
        for (int maxBits = blkSz * 8; maxBits>=0 ;){
            maxBits-=nBits;
            if (maxBits <0)
                break;
            uint32_t tmp = reader.readBits(nBits);
            LOG_DEBUG << tmp;
        }
        nBits = reader.remainingBits();
        if (nBits < 8)
            reader.readBits(nBits);
        assert(reader.remainingBits()==8);
        auto term = reader.readInt8();
        if (term!=0x00){
            throw Exception("expect a zero terminator");
        }
    }
	catch (Exception& ex) {
		LOG_DEBUG << ex.what();
	}
	//getchar();
	return 0;
}
