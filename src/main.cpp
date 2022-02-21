#include <iostream>
#include <fstream>

#include "AudioDecoder.h"

int main(int, char**) {
    AudioDecoder* pAudioDecoder = AudioDecoder::getInstance();
    
    string              input, output;
    string              wavPath;
	clock_t             startTime, endTime, elapsed;
    struct CODEC_INFO   codec_info;
    
    cout << "audio(mp3) file path : ";
    cin >> input;
    
    ifstream fIn;
    fIn.open(input);
    if(!fIn){
        cout << "ERROR: File [" << input << "] cannot be found." << endl;
        return -1;
    }
    cout << "Start converting [" << input << "]" << endl;

    startTime   = clock();

    pAudioDecoder->decode(&codec_info, input, output);

    input = output;
    pAudioDecoder->convertPcmToWav(&codec_info, input, wavPath);
    cout << "mp3 file converted to " << wavPath <<endl;

    endTime     = clock();

	elapsed     = endTime - startTime;
    cout << "audio conversion took : " << double(elapsed) << "ms" <<endl;

    return 0;
}
