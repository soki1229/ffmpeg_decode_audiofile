#include "AudioResampler.h"

AudioResampler* AudioResampler::m_pInstance = 0;

AudioResampler::AudioResampler(){}
AudioResampler::~AudioResampler(){}

int AudioResampler::resample(CODEC_INFO* codec_info, short* inputBuff, int wavLen, short* outputBuff, int wavLen16k)
{
    //pWavBuf For input raw Format voice data , Sampling rate 8K, Sampling depth 16bit;
    //wavLen For the length of the input data , The unit is sample number , The number of bytes that are not the length of the data
    //pWav16k Cache for output
    //wavLen16k Is the length of the output data , The unit is still sample number

    int ret;

    //setting src and dst format
    uint8_t**       src_data        = NULL;
    int             src_rate        = codec_info->sample_rate;
    int             src_nb_channels = codec_info->channels;
    int64_t         src_ch_layout   = codec_info->channel_layout;
    AVSampleFormat  src_sample_fmt  = codec_info->sample_fmt;
    int             src_nb_samples  = wavLen;
    int             src_linesize;

    uint8_t**       dst_data        = NULL;   // Data caching
    int             dst_rate        = 22050;
    int             dst_nb_channels = 1;
    int64_t         dst_ch_layout   = AV_CH_LAYOUT_MONO;
    AVSampleFormat  dst_sample_fmt  = AV_SAMPLE_FMT_S16; // Coding format , Here is 16bit Signed number
    int             dst_nb_samples;
    int             dst_linesize;

    int max_dst_nb_samples;            // Number of sampling points （samples * channels）

    //context Define context
    struct SwrContext *swr_ctx;

    //create resampler context
    swr_ctx = swr_alloc_set_opts(NULL, dst_ch_layout,  dst_sample_fmt, dst_rate, src_ch_layout, src_sample_fmt, src_rate, 0, NULL);      // set an option
    if (!swr_ctx) {
        printf("Cannot allocate resampler context!\n");
        return false;
    }

    //initialize resampling context  Initialization context , After each setting, the context must be reinitialized to take effect
    ret = swr_init(swr_ctx);
    if (ret < 0) {
        printf("Initialize the resampling context failed!\n");
        return false;
    }

    //allocate source sample buffer
    ret = av_samples_alloc_array_and_samples(&src_data, &src_linesize, src_nb_channels, src_nb_samples, src_sample_fmt, 0);     // Allocate space
    if (ret < 0) {
        printf("allocate source samples failed!\n");
        return false;
    }

    //compute the number of converted samples and allocate buffer
    dst_nb_samples = av_rescale_rnd(src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);  // Calculate the number of output samples , Rounding up
    max_dst_nb_samples = dst_nb_samples;

    ret = av_samples_alloc_array_and_samples(&dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 0);
    if (ret < 0) {
        printf("allocate destination samples failed!\n");
        return false;
    }

    //copy wav data to src_data
    memcpy(src_data[0], (char*)inputBuff, wavLen * sizeof(short));

    //compute number of dst samples
    dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, src_rate) + src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);    //delay I may not use it here , The statement found on the Internet is to ensure that it can be processed in real time , Allocate space for newly generated data in the processing time of the function
    if (dst_nb_samples > max_dst_nb_samples) {
        av_freep(&dst_data[0]);
        ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels, dst_nb_samples, dst_sample_fmt, 1);
        if (ret < 0) {
            printf("allocate destination samples failed!\n");
            return false;
        }
    }

    //resample
    ret = swr_convert(swr_ctx, dst_data, dst_nb_samples, (const uint8_t **)src_data, src_nb_samples);
    if (ret < 0) {
        printf("Error while converting.\n");
        return false;
    }
    int dst_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels, ret, dst_sample_fmt, 1);   //buffer size of dst The space occupied by the output samples of the actual transformation
    if (dst_bufsize < 0) {
        printf("Cannot get sample buffer size!\n");
        return false;
    }
    memcpy((char*)outputBuff, dst_data[0], dst_bufsize);   // Save the transformation output
    wavLen16k = ret;        // Number of samples converted

    //flush swr Looking forward to further input , Some of the converted data will remain in the buffer , Need to be informed by swr There is no input to flush out the data in the buffer . The method is to set the input data to NULL, Length set to 0.
    dst_nb_samples -= ret;      // Residual length
    ret = swr_convert(swr_ctx, dst_data, dst_nb_samples, NULL, 0);
    int rest_bufsize = av_samples_get_buffer_size(&dst_linesize, dst_nb_channels, ret, dst_sample_fmt, 1);
    if (rest_bufsize < 0) {
        printf("Can not get sample buffer size!\n");
        return false;
    }
    memcpy((char*)outputBuff + dst_bufsize, dst_data[0], rest_bufsize);          // Save the rest of the data
    wavLen16k += ret;   // The remaining number of samples

    //free
    if (src_data) {
        av_freep(&src_data[0]);
    }
    av_freep(&src_data);
    if (dst_data) {
        av_freep(&dst_data[0]);
    }
    av_freep(dst_data);
    swr_free(&swr_ctx);

    return true;

    return 0;
}
