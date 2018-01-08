
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

#include <wx/filefn.h>
#include <wx/progdlg.h>

#include "VideoExporter.h"


double VideoExporter::s_t = 0.;
double VideoExporter::s_freq = 750.;
double VideoExporter::s_deltaTime = 1. / 44100;

VideoExporter::VideoExporter(int width, int height, float scaleFactor, unsigned int frameDuration, unsigned int frameCount, int audioChannelCount, int audioSampleRate, log4cpp::Category &logger_base)
	: m_width(width), m_height(height), m_scaleFactor(scaleFactor), m_frameDuration(frameDuration), m_frameCount(frameCount)
	, m_audioChannelCount(audioChannelCount), m_audioSampleRate(audioSampleRate), m_audioFrameSize(audioSampleRate / (1000 / frameDuration)), m_logger_base(logger_base)
	, m_GetVideo(dummyGetVideoFrame), m_GetAudio(dummyGetAudioFrame)
{
	s_t = 0.;
}

bool VideoExporter::Export(const char *path)
{
	int width = m_width;
	int height = m_height;

	// width and height must be even
	if (width % 2)
		++width;
	if (height % 2)
		++height;

	avcodec_register_all();
	av_register_all();

	AVOutputFormat* fmt = av_guess_format(nullptr, path, nullptr);
	AVCodec *videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	AVCodec *audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (videoCodec == nullptr || audioCodec == nullptr)
	{
		m_logger_base.error("  error finding codecs.");
		return false;
	}

	AVFormatContext* formatContext;
	avformat_alloc_output_context2(&formatContext, fmt, nullptr, path);
	if (formatContext == nullptr)
	{
		m_logger_base.error("  error opening output-context");
		return false;
	}

	AVStream* video_st = avformat_new_stream(formatContext, videoCodec);
	video_st->id = formatContext->nb_streams - 1;
	video_st->time_base.num = 1;
	video_st->time_base.den = 1000 / m_frameDuration;
	AVCodecContext* videoCodecContext = video_st->codec;
	avcodec_get_context_defaults3(videoCodecContext, videoCodec);
	videoCodecContext->codec_id = fmt->video_codec;
	videoCodecContext->bit_rate = 400000;
	videoCodecContext->width = width;
	videoCodecContext->height = height;
	videoCodecContext->time_base.num = 1;
	videoCodecContext->time_base.den = 1000 / m_frameDuration;
	videoCodecContext->gop_size = 20;
	videoCodecContext->max_b_frames = 1;
	videoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	av_opt_set(videoCodecContext->priv_data, "preset", "medium", 0);
	av_opt_set(videoCodecContext->priv_data, "crf", "18", AV_OPT_SEARCH_CHILDREN);

	if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
		videoCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;

	int status = avcodec_open2(videoCodecContext, videoCodec, nullptr);
	if (status != 0)
	{
		m_logger_base.error("  error opening video codec.");
		return false;
	}

	AVStream *audio_st = nullptr;
	bool hasAudio = m_audioChannelCount != 0 && m_audioSampleRate != 0;
	if (hasAudio)
	{
		audio_st = avformat_new_stream(formatContext, audioCodec);
		audio_st->id = formatContext->nb_streams - 1;
		AVCodecContext *audioCodecContext = audio_st->codec;
		avcodec_get_context_defaults3(audioCodecContext, audioCodec);
		audioCodecContext->codec_id = fmt->audio_codec;
		audioCodecContext->sample_fmt = AV_SAMPLE_FMT_FLTP;
		audioCodecContext->bit_rate = 128000;
		audioCodecContext->sample_rate = m_audioSampleRate;
		audioCodecContext->channels = 1;
		audioCodecContext->channel_layout = AV_CH_LAYOUT_MONO;

		if (formatContext->oformat->flags & AVFMT_GLOBALHEADER)
			audioCodecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
		status = avcodec_open2(audioCodecContext, audioCodec, nullptr);
		if (status != 0)
		{
			m_logger_base.error("  error opening audio codec.");
			return false;
		}
	}

	AudioSampleQueue audioSampleQueue;

	AVFrame *frame = av_frame_alloc();
	frame->format = videoCodecContext->pix_fmt;
	frame->width = videoCodecContext->width;
	frame->height = videoCodecContext->height;

	int sws_flags = SWS_FAST_BILINEAR; // doesn't matter too much since we're only doing a colorspace conversion currently
	AVPixelFormat informat = AV_PIX_FMT_RGB24;
	SwsContext *sws_ctx = sws_getContext(width, height, informat,
		videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
		sws_flags, nullptr, nullptr, nullptr);

	AVFrame src_picture;
	int ret = av_image_alloc(src_picture.data, src_picture.linesize, width, height, informat, 1);
	if (ret < 0)
	{
		m_logger_base.error("  error allocating for src-picture buffer");
		return false;
	}

	ret = av_image_alloc(frame->data, frame->linesize, videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt, 1);
	if (ret < 0)
	{
		m_logger_base.error("  error allocatinf for encoded-picture buffer");
		return false;
	}

	if (!(fmt->flags & AVFMT_NOFILE))
	{
		ret = avio_open(&formatContext->pb, path, AVIO_FLAG_WRITE);
		if (ret < 0)
		{
			m_logger_base.error("  error opening output file");
			return false;
		}
	}

	if (avformat_write_header(formatContext, nullptr) < 0)
	{
		m_logger_base.error("  error writing file header");
		return false;
	}

	// buffer for RGB input
	unsigned char *buf = new unsigned char[width * 3 * height];

	wxProgressDialog progressDialog(_("Export progress"), _T("Exporting video..."), 100, nullptr, wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT);
	bool wasCanceled = false, wasErrored = false;
	int progressValue = 0;
	progressDialog.ShowModal();

	// Loop through each frame
	frame->pts = 0;

	for (unsigned int i = 0; i < m_frameCount; ++i)
	{
		if (progressDialog.WasCancelled())
		{
			wasCanceled = true;
			break;
		}
		double percentage = double(i) / m_frameCount;
		int progressAsInt = int(100 * percentage);
		if (progressAsInt != progressValue)
		{
			progressDialog.Update(progressAsInt);
			progressValue = progressAsInt;
		}

		if (!write_video_frame(formatContext, video_st->index, videoCodecContext, &src_picture, frame, sws_ctx, buf, width, height, i, m_logger_base))
		{
			wasErrored = true;
			break;
		}

		if (hasAudio)
		{
			if (!write_audio_frame(formatContext, audio_st, audioSampleQueue, m_logger_base))
			{
				wasErrored = true;
				break;
			}
		}

		frame->pts += av_rescale_q(1, video_st->codec->time_base, video_st->time_base);
	}

	progressDialog.Update(100);

	if (!wasErrored && !wasCanceled)
	{
		// delayed video frames
		for (int got_video_output = 1; got_video_output;)
		{
			AVPacket pkt;
			av_init_packet(&pkt);
			pkt.data = nullptr;
			pkt.size = 0;
			pkt.stream_index = video_st->index;

			status = avcodec_encode_video2(videoCodecContext, &pkt, nullptr, &got_video_output);
			if (status != 0)
			{
				m_logger_base.error("  error encoding delayed video frame");
				wasErrored = true;
				break;
			}
			if (got_video_output)
			{
				ret = av_interleaved_write_frame(formatContext, &pkt);
				if (ret != 0)
				{
					m_logger_base.error("  error writing delayed video frame");
					wasErrored = true;
				}

				av_packet_unref(&pkt);
			}
		}

		// Take care of any remaining samples in the audio queue
		size_t numLeftInQueue = audioSampleQueue.size();
		if (audioSampleQueue.size())
			write_audio_frame(formatContext, audio_st, audioSampleQueue, m_logger_base, true);

		// delayed_audio_frames
		for (int got_audio_output = 1; hasAudio && got_audio_output != 0;)
		{
			AVPacket pkt;
			av_init_packet(&pkt);
			pkt.data = nullptr;
			pkt.size = 0;
			pkt.stream_index = audio_st->index;

			int ret = avcodec_encode_audio2(audio_st->codec, &pkt, nullptr, &got_audio_output);
			if (ret < 0)
			{
				m_logger_base.error("  error encoding delayed audio frame");
				return false;
			}

			if (got_audio_output)
			{
				pkt.stream_index = audio_st->index;
				if (av_interleaved_write_frame(formatContext, &pkt) != 0)
				{
					m_logger_base.error("  error writing delayed audio frame");
					return false;
				}

				av_packet_unref(&pkt);
			}
		}
	}

	delete[] buf;

	if (!wasErrored && !wasCanceled)
	{
		ret = av_write_trailer(formatContext);
		if (ret)
		{
			m_logger_base.error("  error writing file trailer");
			wasErrored = true;
		}
	}

	progressDialog.Hide();

	// Clean-up and close the output file
	sws_freeContext(sws_ctx);
	avcodec_close(video_st->codec);
	if (audio_st)
		avcodec_close(audio_st->codec);
	av_frame_free(&frame);
	for (unsigned i = 0; i < formatContext->nb_streams; ++i)
	{
		av_freep(&formatContext->streams[i]->codec);
		av_freep(&formatContext->streams[i]);
	}
	if (!(fmt->flags & AVFMT_NOFILE))
	{
		avio_close(formatContext->pb);
	}
	av_free(formatContext);

	if (wasErrored || wasCanceled)
	{
		wxRemoveFile(path);
	}

	return !wasErrored;
}

bool VideoExporter::write_video_frame(AVFormatContext *oc, int streamIndex, AVCodecContext *cc, AVFrame *srcFrame, AVFrame *dstFrame, SwsContext *sws_ctx, unsigned char *buf, int width, int height, int frameIndex, log4cpp::Category &logger_base)
{
	if (m_GetVideo == nullptr)
	{
		m_logger_base.error("  GetVideo un-set");
		return false;
	}

	bool getStatus = m_GetVideo(buf, width * 3 * height, m_width, m_height, m_scaleFactor, frameIndex);
	if (!getStatus)
	{
		m_logger_base.error("  GetVideo fails");
		return false;
	}

	int ret = av_image_fill_arrays(srcFrame->data, srcFrame->linesize, buf, AV_PIX_FMT_RGB24, width, height, 1);
	if (ret < 0)
	{
		m_logger_base.error("  error retrieving src-image data");
		return false;
	}

	uint8_t * data = srcFrame->data[0];
	uint8_t* tmp[4] = { data, nullptr, nullptr, nullptr };
	int stride[4] = { srcFrame->linesize[0], 0, 0, 0 };
	ret = sws_scale(sws_ctx, tmp, stride, 0, height, dstFrame->data, dstFrame->linesize);
	if (ret != cc->height)
	{
		m_logger_base.error("  error scaling src-image");
		return false;
	}

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = nullptr;
	pkt.size = 0;

	int got_video_output = 0;
	ret = avcodec_encode_video2(cc, &pkt, dstFrame, &got_video_output);
	if (ret < 0)
	{
		logger_base.error("  error encoding video frame");
		return false;
	}

	if (got_video_output)
	{
		pkt.stream_index = streamIndex;

		ret = av_interleaved_write_frame(oc, &pkt);

		av_packet_unref(&pkt);
	}

	return true;
}

bool VideoExporter::write_audio_frame(AVFormatContext *oc, AVStream *st, AudioSampleQueue &queue, log4cpp::Category &logger_base, bool clearQueue/*= false*/)
{
	AVCodecContext *c = st->codec;

	if (m_GetAudio == nullptr)
	{
		m_logger_base.error("  GetAudio un-set");
		return false;
	}

	// We'll ask for audio samples in video-frame-time chunks but for AAC, we can
	// only encode in 1024-sample chunks, so we'll use a FIFO buffer in between.
	AudioSample *buf = nullptr;

	if (!clearQueue)
	{
		buf = new AudioSample[m_audioFrameSize];
		bool getStatus = m_GetAudio((float *)buf, m_audioFrameSize, c->channels);
		if (getStatus == false)
		{
			logger_base.error("  GetAudio fails");
			return false;
		}

		for (int i = 0; i < m_audioFrameSize; ++i)
			queue.push(buf[i]);
	}

	int encode_frame_size = clearQueue ? queue.size() : c->frame_size;

	// Re-use our AudoSample buffer for encoding if possible...
	if (buf == nullptr || encode_frame_size > m_audioFrameSize)
	{
		if (buf != nullptr)
			delete[] buf;
		buf = new AudioSample[encode_frame_size];
	}

	int got_packet = 0;
	while (queue.size() >= encode_frame_size)
	{
		for (int i = 0; i < encode_frame_size; ++i)
		{
			buf[i] = queue.front();
			queue.pop();
		}

		AVFrame *frame = av_frame_alloc();
		frame->format = AV_SAMPLE_FMT_FLTP;
		frame->channel_layout = c->channel_layout;
		frame->nb_samples = encode_frame_size;

		int buffer_size = av_samples_get_buffer_size(nullptr, c->channels, encode_frame_size, c->sample_fmt, 0);
		int audioSize = avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt, (uint8_t *)buf, buffer_size, 1);
		if (audioSize < 0)
		{
			logger_base.error("  error filling audio frame");
			return false;
		}

		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = nullptr;    // packet data will be allocated by the encoder
		pkt.size = 0;
		pkt.stream_index = st->index;

		int ret = avcodec_encode_audio2(c, &pkt, frame, &got_packet);
		if (ret < 0)
		{
			logger_base.error("  error encoding audio frame");
			return false;
		}

		if (got_packet)
		{
			pkt.stream_index = st->index;
			if (av_interleaved_write_frame(oc, &pkt) != 0)
			{
				logger_base.error("  error writing audio data");
				return false;
			}

			av_packet_unref(&pkt);
		}

		av_frame_free(&frame);
	}

	delete[] buf;

	return true;
}

bool VideoExporter::dummyGetVideoFrame(unsigned char *buf, int bufSize, int width, int height, float scaleFactor, unsigned frameIndex)
{
	unsigned char *ptr = buf;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			*ptr++ = 0xff;
			*ptr++ = 0x00;
			*ptr++ = 0x00;
		}
	}
	return true;
}

bool VideoExporter::dummyGetAudioFrame(float *samples, int frameSize, int numChannels)
{
	float *q = samples;
	for (int i = 0; i < frameSize; ++i)
	{
		double v = sin(2 * M_PI * s_freq * s_t);
		for (int j = 0; j < numChannels; ++j)
			*q++ = float(v);
		s_t += s_deltaTime;
	}

	return true;
}
