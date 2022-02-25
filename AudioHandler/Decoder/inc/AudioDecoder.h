#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include "Audio_Definition.h"

/************************************************************************************/
/**                                                                                **/
/**                                CLASS DEFINITION                                **/
/**                                                                                **/
/************************************************************************************/

class AudioDecoder
{
public :
    static AudioDecoder* getInstance() {
		if(!m_pInstance) {
            m_pInstance = new AudioDecoder;
		}
		return m_pInstance;
	}

	static void destory() {
		if (m_pInstance) {
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

    int decode(CODEC_INFO* codec_info, string input, string &output);

    void write_audio(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, FILE *outfile);

    int convertPcmToWav(CODEC_INFO* codec_info, string pcmPath, string &wavPath);

    int decode_audio_file(const char* path, const int sample_rate, uint16_t** data, int* size);

private:
    AudioDecoder();
    ~AudioDecoder();

    static AudioDecoder* m_pInstance;
};

#endif // AUDIODECODER_H
