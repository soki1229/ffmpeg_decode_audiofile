#include <iostream>
#include <fstream>

#include "AudioDecoder.h"
#include "AudioResampler.h"

int main(int, char**) {
    AudioDecoder*   pAudioDecoder   = AudioDecoder::getInstance();

    string              input, output;
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

    short* data;
    int     size;

    pAudioDecoder->decode_audio_file(input.c_str(), 22050, &data, &size);

    double sum = 0.0;
    for (int i=0; i<size; ++i) {
        sum += data[i];
    }

    // display result and exit cleanly
    printf("sum is %f\n", sum);
    printf("size is %d\n", size);

    FILE*       outfile     = NULL;
    outfile = fopen("/home/wskim/shared/after_resample.pcm", "wb");
    if(!outfile){
        printf("decode - outfilie fopen failed!\n");
        return 0;
    }

    for(int channel = 0; channel < 1; channel++){
        fwrite(&data[channel], 1, size*2, outfile);
    }
    fclose(outfile);

    free(data);


//    pAudioDecoder->decode(&codec_info, input, output);

//    input = output;
//    pAudioDecoder->convertPcmToWav(&codec_info, input, output);

//    cout << input << " converted to " << output <<endl;

    endTime     = clock();

    elapsed     = endTime - startTime;
    cout << "audio conversion took : " << (long double)elapsed/CLOCKS_PER_SEC << "s" <<endl;


    cout << "+++Start converting [" << input << "]" << endl;

    return 0;
}
