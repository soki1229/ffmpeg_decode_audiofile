extern "C" {
    #include "libswscale/swscale.h"
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
}

using namespace std;

class BaseModule
{
public :
    static BaseModule* getInstance() {
		if(!m_pInstance) {
            m_pInstance = new BaseModule;
		}
		return m_pInstance;
	}

	static void destory() {
		if (m_pInstance) {
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}

    int decode();
    void write_audio(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, FILE *outfile);

private:
    BaseModule();
    ~BaseModule();

    static BaseModule* m_pInstance;
};