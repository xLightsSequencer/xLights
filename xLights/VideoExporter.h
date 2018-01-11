#ifndef VIDEOEXPORTER_H
#define VIDEOEXPORTER_H

#include <functional>

struct AVCodecContext;
struct AVFormatContext;
struct AVFrame;
struct AVStream;
struct SwsContext;

#include <log4cpp/Category.hh>

class VideoExporter
{
public:
	VideoExporter(wxWindow *parent, int width, int height, float scaleFactor, unsigned int frameDuration, unsigned int frameCount, int audioChannelCount, int audioSampleRate, log4cpp::Category &logger_base);

	typedef std::function<bool(unsigned char * /*buf*/, int /*bufSize*/, int/*width*/, int /*height*/, float/*scaleFactor*/, unsigned /*frameIndex*/)> GetVideoFrameFn;
	typedef std::function<bool(float * /*samples*/, int/*frameSize*/, int/*numChannels*/)> GetAudioFrameFn;

	void SetGetVideoFrameCallback(GetVideoFrameFn gvfn) { m_GetVideo = gvfn; }
	void SetGetAudioFrameCallback(GetAudioFrameFn gafn) { m_GetAudio = gafn; }

	bool Export(const char *path);

protected:
	static bool dummyGetVideoFrame(unsigned char *buf, int bufSize, int width, int height, float scaleFactor, unsigned frameIndex);
	static bool dummyGetAudioFrame(float *samples, int framSize, int numChannels);

	bool write_video_frame(AVFormatContext *oc, int streamIndex, AVCodecContext *cc, AVFrame *srcFrame, AVFrame *dstFrame,
		SwsContext *sws_ctx, unsigned char *buf, int width, int height, int frameIndex, log4cpp::Category &logger_base);
	bool write_audio_frame(AVFormatContext *oc, AVStream *st, float *sampleBuff, int sampleCount, log4cpp::Category &logger_base, bool clearQueue = false);

   wxWindow * const m_parent;
	const int m_width;
	const int m_height;
	const float m_scaleFactor;
	const unsigned int m_frameDuration;
	const unsigned int m_frameCount;
	const int m_audioChannelCount;
	const int m_audioSampleRate;
	const int m_audioFrameSize;
	log4cpp::Category &m_logger_base;

	GetVideoFrameFn m_GetVideo;
	GetAudioFrameFn m_GetAudio;

	static double s_t;
	static double s_freq;
	static double s_deltaTime;
};

#endif // VIDEOEXPORTER_H
