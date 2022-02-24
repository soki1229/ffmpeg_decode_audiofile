#include "AudioDecoder.h"
#include "AudioResampler.h"

AudioDecoder* AudioDecoder::m_pInstance = 0;

AudioDecoder::AudioDecoder(){}
AudioDecoder::~AudioDecoder(){}

int AudioDecoder::decode(CODEC_INFO* codec_info, string input, string &output)
{
    output = input;
    if (!output.empty()){
        output.resize(output.length()-string(FILE_EXTENSION_FMT_MP3).length());
        output.append(FILE_EXTENSION_FMT_PCM);
    }
    printf("decode - path: %s, pcmPath: %s\n", input.c_str(), output.c_str());

    #define AUDIO_INBUF_SIZE        10240
    #define AUDIO_REFILL_THRESH     4096

    const char* filename    = input.c_str();
    const char* outfilename = output.c_str();

    const AVCodec *codec;
    AVCodecContext *codec_ctx= NULL;
    AVCodecParserContext *parser = NULL;
    int         len         = 0;
    int         ret         = 0;
    FILE*       infile      = NULL;
    FILE*       outfile     = NULL;
    uint8_t     inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t*    data        = NULL;
    size_t      data_size   = 0;
    AVPacket*   pkt         = NULL;
    AVFrame*    de_frame    = NULL;

    pkt = av_packet_alloc();

    enum AVCodecID audio_codec_id = AV_CODEC_ID_MP3;
    codec = avcodec_find_decoder(audio_codec_id);
    if(!codec){
        printf("decode - Codec not find!\n");
        return 0;
    }

    parser = av_parser_init(codec->id);
    if(!parser){
        printf("decode - Parser not find!\n");
        return 0;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if(!codec_ctx){
        printf("decode - avcodec_alloc_context3 failed!\n");
        return 0;
    }

    if(avcodec_open2(codec_ctx, codec, NULL) < 0){
        printf("decode - avcodec_open2 failed!\n");
        return 0;
    }

    infile = fopen(filename, "rb");
    if(!infile){
        printf("decode - infile fopen failed!\n");
        return 0;
    }

    outfile = fopen(outfilename, "wb");
    if(!outfile){
        printf("decode - outfilie fopen failed!\n");
        return 0;
    }

    int file_end = 0;
    data = inbuf;
    data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, infile);
    de_frame = av_frame_alloc();

    while (data_size > 0)
    {
        ret = av_parser_parse2(parser, codec_ctx, &pkt->data, &pkt->size, data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if(ret < 0){
            printf("decode_main - av_parser_parser2 Error\n");
            return 0;
        }
        data        += ret;
        data_size   -= ret;

        if(pkt->size)
            write_audio(codec_ctx, pkt, de_frame, outfile);

        if((data_size < AUDIO_REFILL_THRESH) && !file_end ){
            memmove(inbuf, data, data_size);
            data = inbuf;
            len = fread(data + data_size, 1, AUDIO_INBUF_SIZE - data_size, infile);
            if(len > 0){
                data_size += len;
            }else if(len == 0){
                file_end = 1;
                printf("decode_main - file end\n");
            }
        }
    }


    printf("decode_main - codec_id  [%d]\n",    codec->id);
    printf("decode_main - chnl_c    [%d]\n",    codec_ctx->channels);
    printf("decode_main - layout    [%zu]\n",    codec_ctx->channel_layout);
    printf("decode_main - S_rate    [%d]\n",    codec_ctx->sample_rate);
    printf("decode_main - format    [%d]\n",    codec_ctx->sample_fmt);

    codec_info->codecID         = codec->id;
    codec_info->channels        = codec_ctx->channels;
    codec_info->channel_layout  = codec_ctx->channel_layout;
    codec_info->sample_rate     = codec_ctx->sample_rate;
    codec_info->sample_fmt      = codec_ctx->sample_fmt;

//    AudioResampler* pAudioResampler = AudioResampler::getInstance();

//    short* inputBuff = (short*)pkt->data;
//    int wavLen = 1152;
//    short* outputBuff;
//    int wavLen16k;

//    ret = pAudioResampler->resample(codec_info, inputBuff, wavLen, outputBuff, wavLen16k);
//    if (ret < 0){
//        printf("ERROR\n");
//        return 0;
//    }

    pkt->data = NULL;
    pkt->size = 0;
    write_audio(codec_ctx, pkt, de_frame, outfile);

    fclose(infile);
    fclose(outfile);

    avcodec_free_context(&codec_ctx);
    av_parser_close(parser);
    av_frame_free(&de_frame);
    av_packet_free(&pkt);

    return 0;
}

void AudioDecoder::write_audio(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, FILE *outfile)
{
    int i, ch;
    int ret, sample_size;
    int send_ret = 1;
    do {
        ret = avcodec_send_packet(dec_ctx, pkt);
        if(ret == AVERROR(EAGAIN)){
            send_ret = 0;
            printf("write_audio - avcodec_send_packet = AVERROR(EAGAIN)\n");
        }else if(ret < 0){
            char err[128] = {0};
            av_strerror(ret, err, 128);
            printf("write_audio - avcodec_send_packet = ret < 0 : %s\n", err);
            return;
        }

        while (ret >= 0){
            ret = avcodec_receive_frame(dec_ctx, frame);
            if(ret == AVERROR(EAGAIN)){
                return;
            }else if(ret == AVERROR_EOF){
                printf("write_audio - avcodec_receive_frame = EOF\n");
                return;
            }else if(ret < 0){
                printf("write_audio - avcodec_receive_frame = ret < 0\n");
                exit(1);
            }

            sample_size = av_get_bytes_per_sample(dec_ctx->sample_fmt);

            if(sample_size < 0){
                printf("write_audio - av_get_bytes_per_sample = sample_size < 0\n");
                exit(1);
            }
            for(i = 0; i < frame->nb_samples; i++){
                for(int channel = 0; channel < dec_ctx->channels; channel++){
                    fwrite(frame->data[channel] + sample_size * i, 1, sample_size, outfile);
                }
            }
        }
    }
    while (!send_ret);
}

int AudioDecoder::convertPcmToWav(CODEC_INFO* codec_info, string pcmPath, string &wavPath)
{
    AVCodecID       codecID         = codec_info->codecID;
    int             channels        = codec_info->channels;
    int             channel_layout  = codec_info->channel_layout;
    unsigned int    sample_rate     = codec_info->sample_rate;
    int             sample_fmt      = codec_info->sample_fmt;
    
    int             wav_fmt         = 16;

    printf("convertPcmToWav - av_get_bytes_per_sample: %d byte\n", av_get_bytes_per_sample(AVSampleFormat(sample_fmt)));

    switch (AVSampleFormat(sample_fmt)) {
    case AV_SAMPLE_FMT_U8   :
    case AV_SAMPLE_FMT_U8P  :
        wav_fmt = 8;
        break;
    case AV_SAMPLE_FMT_S16  :
    case AV_SAMPLE_FMT_S16P :
        wav_fmt = 16;
        break;
    case AV_SAMPLE_FMT_S32  :
    case AV_SAMPLE_FMT_S32P :
    case AV_SAMPLE_FMT_FLT  :
    case AV_SAMPLE_FMT_FLTP :
    case AV_SAMPLE_FMT_DBL  :
    case AV_SAMPLE_FMT_DBLP :
        wav_fmt = 32;
        break;
    case AV_SAMPLE_FMT_S64  :
    case AV_SAMPLE_FMT_S64P :
        wav_fmt = 64;
        break;
    case AV_SAMPLE_FMT_NONE :
    case AV_SAMPLE_FMT_NB   :
    default:
        break;
    };

    wavPath = pcmPath;
    if (!wavPath.empty()){
        wavPath.resize(wavPath.length()-string(FILE_EXTENSION_FMT_PCM).length());
        wavPath.append(FILE_EXTENSION_FMT_WAV);
    }
    printf("convertPcmToWav - pcmPath: %s, wavPath: %s\n", pcmPath.c_str(), wavPath.c_str());

    if(channels==2 || sample_rate==0){
        channels    = 2;
        sample_rate = 44100;
    }

    WAVE_HEADER pcmHEADER;
    WAVE_FMT    pcmFMT;
    WAVE_DATA   pcmDATA;

    short int m_pcmData;
    FILE *fp, *fpout;

    fp = fopen(pcmPath.c_str(), "rb+");
    if(fp==NULL){
        printf("convertPcmToWav - Open pcm file error\n");
        return -1;
    }
    fpout = fopen(wavPath.c_str(), "wb+");
    if(fpout==NULL){
        printf("convertPcmToWav - Create wav file error\n");
        return -1;
    }

    /* WAVE_HEADER */
    memcpy(pcmHEADER.fccID,     WAV_FMT_CONTEXT_RIFF,   string(WAV_FMT_CONTEXT_RIFF).length());
    memcpy(pcmHEADER.fccType,   WAV_FMT_CONTEXT_WAVE,   string(WAV_FMT_CONTEXT_WAVE).length());
    fseek(fpout, sizeof(WAVE_HEADER), 1);
    
    /* WAVE_FMT */
    memcpy(pcmFMT.fccID,        WAV_FMT_CONTEXT_FMT,    string(WAV_FMT_CONTEXT_FMT).length());
    pcmFMT.dwSize           = 16;
    pcmFMT.wFormatTag       = codecID==AV_CODEC_ID_MP3 ? 0x0003 : 0x0001;
    pcmFMT.wChannels        = channels;
    pcmFMT.dwSamplesPerSec  = sample_rate;
    pcmFMT.uiBitsPerSample  = wav_fmt;
    pcmFMT.dwAvgBytesPerSec = pcmFMT.dwSamplesPerSec*pcmFMT.wChannels*pcmFMT.uiBitsPerSample/8;
    pcmFMT.wBlockAlign      = pcmFMT.wChannels*pcmFMT.uiBitsPerSample/8;

    printf("convertPcmToWav - convertPcmToWav - formatTag: %d\n",   pcmFMT.wFormatTag);
    printf("convertPcmToWav - convertPcmToWav - channels: %d\n",    pcmFMT.wChannels);
    printf("convertPcmToWav - convertPcmToWav - sampleRate: %d\n",  pcmFMT.dwSamplesPerSec);
    printf("convertPcmToWav - convertPcmToWav - Samplebits: %d\n",  pcmFMT.uiBitsPerSample);

    fwrite(&pcmFMT, sizeof(WAVE_FMT), 1, fpout);

    /* WAVE_DATA */
    memcpy(pcmDATA.fccID,       WAV_FMT_CONTEXT_DATA,   string(WAV_FMT_CONTEXT_DATA).length());
    pcmDATA.dwSize = 0;
    fseek(fpout, sizeof(WAVE_DATA), 1);

    size_t ret;
    ret = fread(&m_pcmData, sizeof(short int), 1, fp);
    if(ret<0){
        return 0;
    }
    while(!feof(fp)){
        pcmDATA.dwSize += sizeof(short int);
        fwrite(&m_pcmData, sizeof(short int), 1, fpout);
        ret = fread(&m_pcmData, sizeof(short int), 1, fp);
        if(ret<0){
            return 0;
        }
    }
    pcmHEADER.dwSize = 36 + pcmDATA.dwSize;

    rewind(fpout);
    fwrite(&pcmHEADER, sizeof(WAVE_HEADER), 1, fpout);
    fseek(fpout, sizeof(WAVE_FMT), SEEK_CUR);
    fwrite(&pcmDATA, sizeof(WAVE_DATA), 1, fpout);

    fclose(fp);
    fclose(fpout);

    //remove(pcmPath.c_str());
    
    return 0;
}

int AudioDecoder::decode_audio_file(const char* path, const int sample_rate, short** data, int* size) {
    // initialize all muxers, demuxers and protocols for libavformat
    // (does nothing if called twice during the course of one program execution)
    av_register_all();

    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return -1;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return -1;
    }

    // Find the index of the first audio stream
    int stream_index =- 1;
    for (int i=0; i<format->nb_streams; i++) {
        if (format->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            stream_index = i;
            break;
        }
    }
    if (stream_index == -1) {
        fprintf(stderr, "Could not retrieve audio stream from file '%s'\n", path);
        return -1;
    }
    AVStream* stream = format->streams[stream_index];

    // find & open codec
    AVCodecContext* codec = stream->codec;
    if (avcodec_open2(codec, avcodec_find_decoder(codec->codec_id), NULL) < 0) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n", stream_index, path);
        return -1;
    }

    // prepare resampler
    struct SwrContext* swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count",  codec->channels, 0);
    av_opt_set_int(swr, "in_channel_layout",  codec->channel_layout, 0);
    av_opt_set_int(swr, "in_sample_rate", codec->sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt",  codec->sample_fmt, 0);

    printf("decode_audio_file - chnl_c [%d]\n", codec->channels);
    printf("decode_audio_file - layout [%d]\n", codec->channel_layout);
    printf("decode_audio_file - S_rate [%d]\n", codec->sample_rate);
    printf("decode_audio_file - format [%d]\n", codec->sample_fmt);


    av_opt_set_int(swr, "out_channel_count", 1, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(swr, "out_sample_rate", sample_rate, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_S16,  0);
    swr_init(swr);
    if (!swr_is_initialized(swr)) {
        fprintf(stderr, "Resampler has not been properly initialized\n");
        return -1;
    }

    printf("resampled dest:: - chnl_c [1]\n");
    printf("resampled dest:: - layout [AV_CH_LAYOUT_MONO]\n");
    printf("resampled dest:: - S_rate [%d]\n", sample_rate);
    printf("resampled dest:: - format [AV_SAMPLE_FMT_S16]\n");

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Error allocating the frame\n");
        return -1;
    }

    //~~~~~~~~~~
    FILE*       outfile1;
    outfile1 = fopen("/home/wskim/shared/before_resample.pcm", "wb");
    if(!outfile1){
        printf("decode - outfilie1 fopen failed!\n");
        return 0;
    }


    // iterate through frames
    *data = NULL;
    *size = 0;
    while (av_read_frame(format, &packet) >= 0) {
        // decode one frame
        int gotFrame;
        if (avcodec_decode_audio4(codec, frame, &gotFrame, &packet) < 0) {
            break;
        }
        if (!gotFrame) {
            continue;
        }

        //Extracting ORIGIN PCM to FILE
        int sample_size = av_get_bytes_per_sample(codec->sample_fmt);
        for(int i = 0; i < frame->nb_samples; i++){
            for(int channel = 0; channel < codec->channels; channel++){
                fwrite(frame->data[channel] + sample_size * i, 1, sample_size, outfile1);
            }
        }

        // resample frames
        double* buffer;
        av_samples_alloc((uint8_t**) &buffer, NULL, 1, frame->nb_samples, AV_SAMPLE_FMT_S16, 0);

        int frame_count = swr_convert(swr, (uint8_t**) &buffer, frame->nb_samples, (const uint8_t**) frame->data, frame->nb_samples);
        // append resampled frames to data
        *data = (short*) realloc(*data, (*size + frame->nb_samples) * sizeof(short));
        memcpy(*data + *size, buffer, frame_count * sizeof(short));
        *size += frame_count;

    }

    fclose(outfile1);
    //~~~~~~~~~~

    // clean up
    av_frame_free(&frame);
    swr_free(&swr);
    avcodec_close(codec);
    avformat_free_context(format);

    // success
    return 0;

}
