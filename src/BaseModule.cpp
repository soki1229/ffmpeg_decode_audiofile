#include "BaseModule.h"

BaseModule* BaseModule::m_pInstance = 0;

BaseModule::BaseModule(){}
BaseModule::~BaseModule(){}

int BaseModule::decode()
{
    #define AUDIO_INBUF_SIZE        10240
    #define AUDIO_REFILL_THRESH     4096

    const char* filename    = "sample.mp3";
    const char* outfilename = "out.pcm";

    const AVCodec *codec;
    AVCodecContext *codec_ctx= NULL;
    AVCodecParserContext *parser = NULL;
    int len = 0;
    int ret = 0;
    FILE *infile = NULL;
    FILE *outfile = NULL;
    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t *data = NULL;
    size_t   data_size = 0;
    AVPacket *pkt = NULL;
    AVFrame *de_frame = NULL;

    pkt = av_packet_alloc();
    enum AVCodecID audio_codec_id = AV_CODEC_ID_MP3;
    codec = avcodec_find_decoder(audio_codec_id);
    if(!codec)
    {
        printf("decode - Codec not find!\n");
        return 0;
    }

    parser = av_parser_init(codec->id);
    if(!parser)
    {
        printf("decode - Parser not find!\n");
        return 0;
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if(!codec_ctx)
    {
        printf("decode - avcodec_alloc_context3 failed!\n");
        return 0;
    }

    if(avcodec_open2(codec_ctx, codec, NULL) < 0)
    {
        printf("decode - avcodec_open2 failed!\n");
        return 0;
    }

    infile = fopen(filename, "rb");
    if(!infile)
    {
        printf("decode - infile fopen failed!\n");
        return 0;
    }

    outfile = fopen(outfilename, "wb");
    if(!outfile)
    {
        printf("decode - outfilie fopen failed!\n");
        return 0;
    }


    int file_end = 0;
    data = inbuf;
    data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, infile);
    de_frame = av_frame_alloc();

    printf("decode_main - data_size [%d]\n", data_size);

    printf("decode_main - de_frame->data[0] = %s\n", de_frame->data[0]);

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

        if((data_size < AUDIO_REFILL_THRESH) && !file_end )
        {
            memmove(inbuf, data, data_size);
            data = inbuf;
            len = fread(data + data_size, 1, AUDIO_INBUF_SIZE - data_size, infile);
            if(len > 0)
                data_size += len;
            else if(len == 0)
            {
                file_end = 1;
                printf("decode_main - file end\n");
            }
        }
    }

    printf("decode_main - chnl_c [%d]\n", codec_ctx->channels);
    printf("decode_main - layout [%d]\n", codec_ctx->channel_layout);
    printf("decode_main - S_rate [%d]\n", codec_ctx->sample_rate);
    printf("decode_main - format [%d]\n", codec_ctx->sample_fmt);

    pkt->data = NULL;
    pkt->size = 0;
    write_audio(codec_ctx, pkt, de_frame, outfile);

    fclose(infile);
    fclose(outfile);

    avcodec_free_context(&codec_ctx);
    av_parser_close(parser);
    av_frame_free(&de_frame);
    av_packet_free(&pkt);

    printf("decode_main - end\n");
    return 0;
}

void BaseModule::write_audio(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, FILE *outfile)
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
            if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
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

    printf("write_audio - frame->data[0] = %s\n", frame->data[0]);
}