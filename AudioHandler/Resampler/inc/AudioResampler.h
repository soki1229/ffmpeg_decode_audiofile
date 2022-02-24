#ifndef AUDIORESAMPLER_H
#define AUDIORESAMPLER_H

extern "C" {
    #include "libswresample/swresample.h"
    #include "libavutil/opt.h"
    #include "libavutil/samplefmt.h"
}

#include "Audio_Definition.h"

class AudioResampler
{
public :
    static AudioResampler* getInstance() {
        if(!m_pInstance) {
            m_pInstance = new AudioResampler;
        }
        return m_pInstance;
    }

    static void destory() {
        if (m_pInstance) {
            delete m_pInstance;
            m_pInstance = NULL;
        }
    }

    int resample(CODEC_INFO* codec_info, short* pWavBuf, int wavLen, short* pWav16k, int wavLen16k);

private:
    AudioResampler();
    ~AudioResampler();

    static AudioResampler* m_pInstance;
};




#endif // AUDIORESAMPLER_H
