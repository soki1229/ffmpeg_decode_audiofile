#ifndef AUDIO_DEFINITION_H
#define AUDIO_DEFINITION_H

#include <ctime>
#include <string>

extern "C" {
    #include "libswscale/swscale.h"
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libavutil/opt.h"
    #include "libswresample/swresample.h"
    #include "libavutil/channel_layout.h"
}

using namespace std;

#define FILE_EXTENSION_FMT_MP3  ".mp3"
#define FILE_EXTENSION_FMT_PCM  ".pcm"
#define FILE_EXTENSION_FMT_WAV  ".wav"

#define WAV_FMT_CONTEXT_RIFF    "RIFF"
#define WAV_FMT_CONTEXT_WAVE    "WAVE"
#define WAV_FMT_CONTEXT_FMT     "fmt "
#define WAV_FMT_CONTEXT_DATA    "data"

/************************************************************************************/
/**                                                                                **/
/**                              STRUCTURE DEFINITION                              **/
/**                                                                                **/
/************************************************************************************/
typedef struct WAVE_HEADER{
    char            fccID[4];           //The content is "riff"
    unsigned int    dwSize;             //Finally fill in, the size of the WAVE format audio
    char            fccType[4];         //The content is "Wave"
}WAVE_HEADER;

typedef struct WAVE_FMT{
    char            fccID[4];           //The content is "FMT"
    unsigned int    dwSize;             //The content of the content is WAVE_FMT, 16
    short int       wFormatTag;         //PCM=1, MP3=3
    short int       wChannels;          //Number of channels, single channel = 1, dual channel = 2
    unsigned int    dwSamplesPerSec;    //Sampling frequency
    unsigned int    dwAvgBytesPerSec;   /* ==dwSamplesPerSec*wChannels*uiBitsPerSample/8 */
    short int       wBlockAlign;        //==wChannels*uiBitsPerSample/8
    short int       uiBitsPerSample;    //BIT number of each sampling point, 8bits = 8, 16bits = 16
}WAVE_FMT;

typedef struct WAVE_DATA{
    char            fccID[4];           //Content is "DATA"
    unsigned int    dwSize;             //==NumSamples*wChannels*uiBitsPerSample/8
}WAVE_DATA;


typedef struct CODEC_INFO{
    AVCodecID       codecID;
    int             channel_layout;
    int             channels;
    int             sample_rate;
    AVSampleFormat  sample_fmt;
}CODEC_INFO;


#endif // AUDIO_DEFINITION_H
