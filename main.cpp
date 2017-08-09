#include "StreamReader.h"
#include "Exception.h"
#include "Logging.h"
#include <string.h>
#include <vector>
#include <assert.h>



int main() {
    try {
    }
	catch (Exception& ex) {
		LOG_DEBUG << ex.what();
	}
	//getchar();
	return 0;
}
