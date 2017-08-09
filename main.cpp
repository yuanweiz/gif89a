#include "Decoder.h"
#include "Exception.h"
#include "Logging.h"
#include <string.h>
#include <assert.h>

#include <string>
#include <map>
#include <vector>

struct LogHelper{
    LogHelper(int x){
        if (x < 5){
            sz=snprintf(buf,sizeof (buf),"%c",'A'+x);
        }
        else sz = snprintf(buf,sizeof(buf),"%d",x);
        buf[sz]='\0';
    }
    char buf[32];
    size_t sz;
};
LogStream& operator<< (LogStream& s, const LogHelper& h){
    s<< h.buf;
    return s;
}
int main() {
    try {
        Decoder decoder ("/home/ywz/Pictures/a.gif");
        decoder.decode();
    }

	catch (Exception& ex) {
		LOG_DEBUG << ex.what();
	}
    std::string str= "ABABABABBBABABAACDACDADCABAAABAB";
    std::vector<int> out;
    int sz=6;
    std::map< std::pair<int,int>, int> mp_;
    for (auto c: str){
        int suffix = c-'A';
        if (out.empty()) {
            out.push_back(suffix);
            //continue;
        }
        else {
            auto p = std::make_pair(out.back(),suffix);
            auto it = mp_.find(p);
            if (it == mp_.end()){
                out.push_back(suffix);
                mp_.insert( {p,sz});
                LOG_DEBUG << sz << ": " << LogHelper(p.first) << " " << LogHelper(p.second) ;
                ++sz; 
                //continue;
            }
            else {
                //actually a push stack+reduce operation
                out.back()=it->second;
            }
        }

        //print stack
        for (auto x : out){
            printf("%s ",LogHelper(x).buf);
        }
        putchar('\n');
    }

	//getchar();
	return 0;
}
