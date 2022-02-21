#include <iostream>

#include "BaseModule.h"

int main(int, char**) {
    BaseModule* pBaseModule = BaseModule::getInstance();
    
    string              input, output;
    string              wavPath;
	clock_t             startTime, endTime, elapsed;
    struct CODEC_INFO   codec_info;
    
    startTime   = clock();

    input = "Symphony No.6 (1st movement).mp3";
    pBaseModule->decode(&codec_info, input, output);

    input = output;
    pBaseModule->convertPcmToWav(&codec_info, input, wavPath);
    std::cout << "mp3 file converted to " << wavPath <<endl;

    endTime     = clock();

	elapsed     = endTime - startTime;
    std::cout << "audio conversion took : " << double(elapsed) << "ms" <<endl;


}
