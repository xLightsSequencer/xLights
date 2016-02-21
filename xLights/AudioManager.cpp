#include "AudioManager.h"
#include <sstream>
#include <algorithm>
#include <wx/wx.h>
#include <wx/string.h>
#include <wx/ffile.h>
#include "xLightsXmlFile.h"
#include <wx/log.h>
#include <math.h>
#include <stdlib.h>
#include "kiss_fft/tools/kiss_fftr.h"
#ifdef __WXMSW__
//#include "wx/msw/debughlp.h"
//wxString s;
//s.Printf("%f -> %f", val, db);
//wxDbgHelpDLL::LogError(s);
#endif

using namespace Vamp;

// Audio Manager Functions

AudioManager::AudioManager(std::string audio_file, xLightsXmlFile* xml_file, int step = 1024, int block = 1024)
{
	// save parameters and initialise defaults
	_job = NULL;
	_xml_file = xml_file;
	_audio_file = audio_file;
	_state = -1; // state uninitialised. 0 is error. 1 is loaded ok
	_resultMessage = "";
	_data[0] = NULL; // Left channel data
	_data[1] = NULL; // right channel data
	_intervalMS = -1; // no length
	_frameDataPrepared = false; // frame data is used by effects to react to the sone

	// extra is the extra bytes added to the data we read. This allows analysis functions to exceed the file length without causing memory exceptions
	_extra = std::max(step, block) + 1;

	// Open the media file
	OpenMediaFile();

	// Check if it is Constant Bit Rate
	_isCBR = CheckCBR();

	// If we opened it successfully kick off the frame data extraction ... this will run on another thread
	if (_intervalMS > 0)
	{
		PrepareFrameData(true);
	}

	// if we got here without setting state to zero then all must be good so set state to 1 success
	if (_state == -1)
	{
		_state = 1;
	}
}

#ifdef KISS_FFT
std::list<float> AudioManager::CalculateSpectrumAnalysis(const float* in, int n, float& max, int id)
{
	std::list<float> res;
	int outcount = n / 2 + 1;
	//float scaling = 2.0 / (float)n;
	kiss_fftr_cfg cfg;
	kiss_fft_cpx* out = (kiss_fft_cpx*)malloc(sizeof(kiss_fft_cpx) * (outcount));
	if (out != NULL)
	{
		if ((cfg = kiss_fftr_alloc(n, 0/*is_inverse_fft*/, NULL, NULL)) != NULL)
		{
			kiss_fftr(cfg, in, out);
			free(cfg);
		}

		int start = 65.0 * (1.0 / (float)_rate) * (float)n;
		int end = 1046.0 * (1.0 / (float)_rate) * (float)n;

		for (int i = start; i < end; i++)
		{
			kiss_fft_cpx* cur = out + i;
			float val = sqrtf(cur->r * cur->r + cur->i * cur->i);
			//float valscaled = val * scaling;
			float db = log10(val);
			if (db < 0.0)
			{
				db = 0.0;
			}

			res.push_back(db);
			if (db > max)
			{
				max = db;
			}
		}

		free(out);
	}

	return res;
}
#endif

// Frame Data Extraction Functions
// process audio data and build data for each frame
void AudioManager::DoPrepareFrameData()
{
	// lock the mutex
    std::unique_lock<std::shared_timed_mutex> locker(_mutex);

	// if we have already done it ... bail
	if (_frameDataPrepared)
	{
		return;
	}

	// samples per frame
	int samplesperframe = _rate * _intervalMS / 1000;
	int frames = _lengthMS / _intervalMS;
	while (frames * _intervalMS < _lengthMS)
	{
		frames++;
	}
	int totalsamples = frames * samplesperframe;

	// these are used to normalise output
	_bigmax = -1;
	_bigspread = -1;
	_bigmin = 1;
	_bigspectogrammax = -1;

#ifdef CONSTANT_Q
	// make sure plugins are loaded
	_vamp.GetAllAvailablePlugins(this);
	Vamp::Plugin *p = _vamp.GetPlugin("Constant-Q Spectrogram");
	size_t step = 0;
	size_t block = 0;
#endif

#ifdef KISS_FFT
	size_t step = 2048;
#endif

	float *pdata[2];

#ifdef CONSTANT_Q
	if (p != NULL)
	{
		// Constant-Q Sepectogram configuration
		p->setParameter("minpitch", 36);
		p->setParameter("maxpitch", 84);
		p->setParameter("tuning", 440);
		p->setParameter("bpo", 12);
		p->setParameter("normalized", 0);

		// setup the config
		int channels = GetChannels();
		if (channels > p->getMaxChannelCount()) {
			channels = 1;
		}
		step = p->getPreferredStepSize();
		block = p->getPreferredBlockSize();
		if (block == 0) {
			if (step != 0) {
				block = step;
			}
			else {
				block = 1024;
			}
		}
		if (step == 0) {
			step = block;
		}
		p->initialise(channels, step, block);

		// make sure our data is ok for this ... basically ensures there is enough extra space for the window at the end of the song
		SetStepBlock(step, block);
	}
#endif

	int pos = 0;
	std::list<float> spectrogram;

	// process each frome of the song
	for (int i = 0; i < frames; i++)
	{
		std::vector<std::list<float>> aFrameData;

		// accumulators
		float max = -100.0;
		float min = 100.0;
		float spread = -100;

#ifdef CONSTANT_Q
		if (p != NULL)
#endif
		{
			int count = 0;

			// clear the data if we are about to get new data ... dont clear it if we wont
			// this happens because the spectrogram function has a fixed window based on the parameters we set and it
			// does not match our time slices exactly so we have to select which one to use
			if (pos < i * samplesperframe + samplesperframe  && pos + step < totalsamples)
			{
				spectrogram.clear();
			}

			// only get the data if we are not ahead of the music
			while (pos < i * samplesperframe + samplesperframe && pos + step < totalsamples)
			{
				std::list<float> subspectrogram;
				pdata[0] = GetLeftDataPtr(pos);
				pdata[1] = GetRightDataPtr(pos);
				float max = 0;

#ifdef CONSTANT_Q
				Vamp::RealTime timestamp = Vamp::RealTime::frame2RealTime(pos, GetRate());

				// This function processes the data
				Vamp::Plugin::FeatureSet features = p->process(pdata, timestamp);
				// Now we interpret the results
				subspectrogram = ProcessFeatures(features[0], max);
#endif

#ifdef KISS_FFT
				if (pdata[0] == NULL)
				{
					subspectrogram.clear();
				}
				else
				{
					subspectrogram = CalculateSpectrumAnalysis(pdata[0], step, max, i);
				}
#endif

				// and keep track of the larges value so we can normalise it
				if (max > _bigspectogrammax)
				{
					_bigspectogrammax = max;
				}
				pos += step;

				// either take the newly calculated values or if we are merging two results take the maximum of each value
				if (spectrogram.size() == 0)
				{
					spectrogram = subspectrogram;
				}
				else
				{
					if (subspectrogram.size() > 0)
					{
						std::list<float>::iterator sub = subspectrogram.begin();
						for (std::list<float>::iterator fr = spectrogram.begin(); fr != spectrogram.end(); ++fr)
						{
							if (*sub > *fr)
							{
								*fr = *sub;
							}
							++sub;
						}
					}
				}
			}
		}

		// now do the raw data analysis for the frame
		for (int j = 0; j < samplesperframe; j++)
		{
			float data = GetLeftData(i * samplesperframe + j);

			// Max data
			if (data > max)
			{
				max = data;
			}

			// Min data
			if (data < min)
			{
				min = data;
			}

			// Spread data
			if (max - min > spread)
			{
				spread = max - min;
			}
		}

		if (max > _bigmax)
		{
			_bigmax = max;
		}
		if (min < _bigmin)
		{
			_bigmin = min;
		}
		if (spread > _bigspread)
		{
			_bigspread = spread;
		}

		// Now save the results for the frame
		std::list<float> maxlist;
		maxlist.push_back(max);
		std::list<float> minlist;
		minlist.push_back(min);
		std::list<float> spreadlist;
		spreadlist.push_back(spread);
		aFrameData.push_back(maxlist);
		aFrameData.push_back(minlist);
		aFrameData.push_back(spreadlist);
		aFrameData.push_back(spectrogram);

		_frameData.push_back(aFrameData);
	}

	// normalise data ... basically scale the data so the highest value is the scale value.
	float scale = 1.0; // 0-1 ... where 0.x means that the max value displayed would be x0% of model size
	float bigmaxscale = 1 / (_bigmax * scale);
	float bigminscale = 1 / (_bigmin * scale);
	float bigspreadscale = 1 / (_bigspread * scale);
	float bigspectrogramscale = 1 / (_bigspectogrammax * scale);
	for (std::vector<std::vector<std::list<float>>>::iterator itframe = _frameData.begin(); itframe != _frameData.end(); ++itframe)
	{
		std::list<float>* fl = &(*itframe)[0];
		std::list<float>::iterator f = fl->begin();
		*f = (*f * bigmaxscale);

		fl = &(*itframe)[1];
		f = fl->begin();
		*f = (*f * bigminscale);

		fl = &(*itframe)[2];
		f = fl->begin();
		*f = (*f * bigspreadscale);

		fl = &(*itframe)[3];
		for (std::list<float>::iterator ff = fl->begin(); ff != fl->end(); ++ff)
		{
			*ff = *ff * bigspectrogramscale;
		}
	}

	// flag the fact that the data is all ready
	_frameDataPrepared = true;
}

#ifdef CONSTANT_Q
// Extract the Vamp data and reduce it to one array of values
std::list<float> AudioManager::ProcessFeatures(Vamp::Plugin::FeatureList &feature, float& max) 
{
	max = 0;
	std::list<float> res;

	for (int i = 0; i < feature.size(); i++)
	{
		std::list<float>::iterator rp = res.begin();
		for (std::vector<float>::iterator j = feature[i].values.begin(); j != feature[i].values.end(); ++j) 
		{
			if (i == 0)
			{
				res.push_back((*j));
			}
			else
			{
				// this is a second set of data ... ie > 1 dimension array so we take the maximum of corresponding elements
				if (*j > *rp)
				{
					*rp = *j;
				}
				++rp;
			}
		}
	}

	// work out the maximum for normalisation
	for (std::list<float>::iterator j = res.begin(); j != res.end(); ++j)
	{
		*j = log10(*j);
		if (*j > max)
		{
			max = *j;
		}
	}

	return res;
}
#endif

// Called to trigger frame data creation
void AudioManager::PrepareFrameData(bool separateThread)
{
	if (separateThread)
	{
		// if we have not prepared the frame data and no job has been created
		if (!_frameDataPrepared && _job == NULL)
		{
			_job = (Job*)new AudioScanJob(this);
			_jobPool.PushJob(_job);
		}
	}
	else
	{
		DoPrepareFrameData();
	}
}

// Get the pre-prepared data for this frame
std::list<float>* AudioManager::GetFrameData(int frame, FRAMEDATATYPE fdt, std::string timing)
{
    // Grab the lock so we can safely access the frame data
    std::shared_lock<std::shared_timed_mutex> lock(_mutex);
    

	// if the frame data has not been prepared
	if (!_frameDataPrepared)
	{
		// prepare it
		lock.unlock();
		PrepareFrameData(false);

        lock.lock();
		// wait until the new thread grabs the lock
		while (!_frameDataPrepared)
		{
			lock.unlock();
			wxMilliSleep(5);
            lock.lock();
		}
	}

	// now we can grab the data we need
	std::list<float>* rc = NULL;
	try
	{
		if (_frameData.size() > 0)
		{
			std::vector<std::list<float>>* framedata = &_frameData[frame];

			switch (fdt)
			{
			case FRAMEDATA_HIGH:
				rc = &framedata->at(0);
				break;
			case FRAMEDATA_LOW:
				rc = &framedata->at(1);
				break;
			case FRAMEDATA_SPREAD:
				rc = &framedata->at(2);
				break;
			case FRAMEDATA_VU:
				rc = &framedata->at(3);
				break;
			case FRAMEDATA_ISTIMINGMARK:
				// we dont need to do anything here
				break;
			}
		}
	}
	catch (...)
	{
		rc = NULL;
	}

	return rc;
}

// Constant Bitrate Detection Functions

// Decode bitrate
int AudioManager::decodebitrateindex(int bitrateindex, int version, int layertype)
{
	switch (version)
	{
	case 0: // v2.5
	case 2: // v2
		switch (layertype)
		{
		case 0:
			// invalid
			return 0;
		case 1: // L3
		case 2: // L2
			if (bitrateindex == 0 || bitrateindex == 0x0F)
			{
				return 0;
			}
			else if (bitrateindex < 8)
			{
				return 8 * bitrateindex;
			}
			else
			{
				return 64 + (bitrateindex - 8) * 16;
			}
		case 3: // L1
			if (bitrateindex == 0 || bitrateindex == 0x0F)
			{
				return 0;
			}
			else
			{
				return 16 + bitrateindex * 16;
			}
		}
		break;
	case 3: // v1
		switch (layertype)
		{
		case 0:
			// invalid
			return 0;
		case 1: // L3
			if (bitrateindex == 0 || bitrateindex == 0x0F)
			{
				return 0;
			}
			else if (bitrateindex < 6)
			{
				return 32 + (bitrateindex - 1) * 8;
			}
			else if (bitrateindex < 9)
			{
				return 64 + (bitrateindex - 6) * 16;
			}
			else if (bitrateindex < 14)
			{
				return 128 + (bitrateindex - 9) * 32;
			}
			else
			{
				return 320;
			}
		case 2: // L2
			if (bitrateindex == 0 || bitrateindex == 0x0F)
			{
				return 0;
			}
			else if (bitrateindex < 3)
			{
				return 32 + (bitrateindex - 1) * 16;
			}
			else if (bitrateindex < 5)
			{
				return 56 + (bitrateindex - 3) * 8;
			}
			else if (bitrateindex < 9)
			{
				return 80 + (bitrateindex - 5) * 16;
			}
			else if (bitrateindex < 13)
			{
				return 160 + (bitrateindex - 9) * 32;
			}
			else 
			{
				return 320 + (bitrateindex - 13) * 64;
			}
		case 3: // L1
			if (bitrateindex == 0 || bitrateindex == 0x0F)
			{
				return 0;
			}
			else
			{
				return bitrateindex * 32;
			}
		}
		break;
	case 1:
		// invalid
		return 0;
	}
    return 0;
}

// Decode samplerate
int AudioManager::decodesamplerateindex(int samplerateindex, int version)
{
	switch (version)
	{
	case 0: // v2.5
		switch (samplerateindex)
		{
		case 0:
			return 11025;
		case 1:
			return 12000;
		case 2:
			return 8000;
		case 3:
			return 0;
		}
		break;
	case 2: // v2
		switch (samplerateindex)
		{
		case 0:
			return 22050;
		case 1:
			return 24000;
		case 2:
			return 16000;
		case 3:
			return 0;
		}
		break;
	case 3: // v1
		switch (samplerateindex)
		{
		case 0:
			return 44100;
		case 1:
			return 48000;
		case 2:
			return 32000;
		case 3:
			return 0;
		}
		break;
	case 1:
		// invalid
		return 0;
	}

	// this should never happen
	return 0;
}

// Decode side info
int AudioManager::decodesideinfosize(int version, int mono)
{
	if (version == 3) // v1
	{
		if (mono == 3) // mono
		{
			return 17;
		}
		else
		{
			return 32;
		}
	}
	else
	{
		if (mono == 3) // mono
		{
			return 9;
		}
		else
		{
			return 17;
		}
	}
}

// Set the frame interval we will be using
void AudioManager::SetFrameInterval(int intervalMS)
{
	// If this is different from what it was previously
	if (_intervalMS != intervalMS)
	{
		// save it and regenerate the frame data for effects that rely upon it ... but do it on a background thread
		_intervalMS = intervalMS;
		PrepareFrameData(true);
	}
}

// Set the set and block that vamp analysis will be using
// This controls how much extra space at the end of the file we need so VAMP functions dont try to read past the end of the allocated memory
void AudioManager::SetStepBlock(int step, int block)
{
	int extra = std::max(step, block) + 1;

	// we only need to reopen if the extra bytes are greater
	if (extra > _extra)
	{
		_extra = extra;
		_state = -1;
		OpenMediaFile();
	}
}

// Clean up our data buffers
AudioManager::~AudioManager()
{
	if (_data[1] != _data[0] && _data[1] != NULL)
	{
		free(_data[1]);
		_data[1] = NULL;
	}
	if (_data[0] != NULL)
	{
		free(_data[0]);
		_data[0] = NULL;
	}

	// I am not deleting _job as I think JobPool takes care of this
}

// Split the MP# data into left and right and normalise the values
void AudioManager::SplitTrackDataAndNormalize(signed short* trackData, int trackSize, float* leftData, float* rightData)
{
    signed short lSample, rSample;

    for(int i=0;i<trackSize;i++)
    {
        lSample = trackData[i*2];
        leftData[i] = (float)lSample/(float)32768;
        rSample = trackData[(i*2)+1];
        rightData[i] = (float)rSample/(float)32768;
    }
}

// NOrmalise mono track data
void AudioManager::NormalizeMonoTrackData(signed short* trackData, int trackSize, float* leftData)
{
    signed short lSample;
    for(int i=0;i<trackSize;i++)
    {
        lSample = trackData[i];
        leftData[i] = (float)lSample/(float)32768;
    }
}

// Calculate the song lenth in MS
int AudioManager::CalcLengthMS()
{
	float seconds = (float)_trackSize * ((float)1 / (float)_rate);
	return (int)(seconds * (float)1000);
}

#ifdef USE_FFMPEG
// Open and read the media file into memory
int AudioManager::OpenMediaFile()
{
	int err = 0;

	// TODO
	// Initialize FFmpeg
	av_register_all();

	AVFormatContext* formatContext = NULL;
	int res = avformat_open_input(&formatContext, _audio_file.c_str(), NULL, NULL);
	if (res != 0)
	{
		std::cout << "Error opening the file" << std::endl;
		return 1;
	}

	if (avformat_find_stream_info(formatContext, NULL) < 0)
	{
		avformat_close_input(&formatContext);
		std::cout << "Error finding the stream info" << std::endl;
		return 1;
	}

	// Find the audio stream
	AVCodec* cdc = nullptr;
	int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
	if (streamIndex < 0)
	{
		avformat_close_input(&formatContext);
		std::cout << "Could not find any audio stream in the file" << std::endl;
		return 1;
	}

	AVStream* audioStream = formatContext->streams[streamIndex];
	AVCodecContext* codecContext = audioStream->codec;
	codecContext->codec = cdc;

	if (avcodec_open2(codecContext, codecContext->codec, NULL) != 0)
	{
		avformat_close_input(&formatContext);
		std::cout << "Couldn't open the context with the decoder" << std::endl;
		return 1;
	}

	_channels = codecContext->channels;
	_rate = codecContext->sample_rate;
	_bits = av_get_bytes_per_sample(codecContext->sample_fmt);
	int bitrate = codecContext->bit_rate;

	/* Get Track Size */
	GetTrackMetrics(formatContext, codecContext, audioStream);

	// Check if we have read this before ... if so dump the old data
	if (_data[1] != NULL && _data[1] != _data[0])
	{
		free(_data[1]);
		_data[1] = NULL;
	}
	if (_data[0] != NULL)
	{
		free(_data[0]);
		_data[0] = NULL;
	}

	_data[0] = (float*)calloc(sizeof(float)*(_trackSize + _extra), 1);
	for (int i = 0; i < _trackSize + _extra; i++)
	{
		(*(_data[0] + i)) = 0.0;
	}
	if (_channels == 2)
	{
		_data[1] = (float*)calloc(sizeof(float)*(_trackSize + _extra), 1);
		for (int i = 0; i < _trackSize + _extra; i++)
		{
			(*(_data[1] + i)) = 0.0;
		}
	}
	else
	{
		_data[1] = _data[0];
	}

	LoadTrackData(formatContext, codecContext, audioStream);

	ExtractMP3Tags(formatContext);

	avcodec_close(codecContext);
	avformat_close_input(&formatContext);
	return err;
}

void AudioManager::LoadTrackData(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream)
{
	AVFrame* frame = av_frame_alloc();
	int read = 0;
	if (!frame)
	{
		_resultMessage = "Error allocating the frame";
		_state = 0;
		return;
	}

	AVPacket readingPacket;
	av_init_packet(&readingPacket);

	// start at the beginning
	av_seek_frame(formatContext, -1, 0, AVSEEK_FLAG_ANY);

	// Read the packets in a loop
	while (av_read_frame(formatContext, &readingPacket) == 0)
	{
		if (readingPacket.stream_index == audioStream->index)
		{
			AVPacket decodingPacket = readingPacket;

			// Audio packets can have multiple audio frames in a single packet
			while (decodingPacket.size > 0)
			{
				// Try to decode the packet into a frame
				// Some frames rely on multiple packets, so we have to make sure the frame is finished before
				// we can use it
				int gotFrame = 0;
				int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, &decodingPacket);

				if (result >= 0 && gotFrame)
				{
					decodingPacket.size -= result;

					for (int i = 0; i < frame->nb_samples; i++)
					{
						int16_t s;
						float j;
						//int32_t j;
						//int64_t l;
						if (_bits == 2)
						{
							s = *(int16_t*)(frame->data[0] + i * sizeof(int16_t));
							_data[0][read + i] = ((float)s) / (float)0x8000;
							if (_channels > 1)
							{
								s = *(int16_t*)(frame->data[1] + i * sizeof(int16_t));
								_data[1][read + i] = ((float)s) / (float)0x8000;
							}
						}
						else if (_bits == 4)
						{
							j = *(float*)(frame->data[0] + i * sizeof(float));
							_data[0][read + i] = j;
							if (_channels > 1)
							{
								j = *(float*)(frame->data[1] + i * sizeof(float));
								_data[1][read + i] = j;
							}
						}
						//else if (_bits == 4)
						//{
						//	j = *(int32_t*)(frame->data[0] + i * sizeof(int32_t));
						//	_data[0][read + i] = ((float)j) / (float)0x80000000;
						//	if (_channels > 1)
						//	{
						//		j = *(int32_t*)(frame->data[1] + i * sizeof(int32_t));
						//		_data[1][read + i] = ((float)j) / (float)0x80000000;
						//	}
						//}
						//else if (_bits == 8)
						//{
						//	l = *(int64_t*)(frame->data[0] + i * sizeof(int64_t));
						//	_data[0][read + i] = ((float)l) / (float)0x8000000000000000;
						//	if (_channels > 1)
						//	{
						//		l = *(int64_t*)(frame->data[1] + i * sizeof(int64_t));
						//		_data[1][read + i] = ((float)l) / (float)0x8000000000000000;
						//	}
						//}
					}
					read += frame->nb_samples;
				}
				else
				{
					decodingPacket.size = 0;
				}
			}
		}

		// You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
		av_free_packet(&readingPacket);
	}

	// Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
	// is set, there can be buffered up frames that need to be flushed, so we'll do that
	if (codecContext->codec->capabilities & CODEC_CAP_DELAY)
	{
		av_init_packet(&readingPacket);
		// Decode all the remaining frames in the buffer, until the end is reached
		int gotFrame = 1;
		while (gotFrame)
		{
			int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, &readingPacket);
			if (result >= 0 && gotFrame)
			{
				for (int i = 0; i < frame->nb_samples; i++)
				{
					int16_t s;
					float j;
					//int32_t j;
					//int64_t l;
					if (_bits == 2)
					{
						s = *(int16_t*)(frame->data[0] + i * sizeof(int16_t));
						_data[0][read + i] = ((float)s) / (float)0x8000;
						if (_channels > 1)
						{
							s = *(int16_t*)(frame->data[1] + i * sizeof(int16_t));
							_data[1][read + i] = ((float)s) / (float)0x8000;
						}
					}
					else if (_bits == 4)
					{
						j = *(float*)(frame->data[0] + i * sizeof(float));
						_data[0][read + i] = j;
						if (_channels > 1)
						{
							j = *(float*)(frame->data[1] + i * sizeof(float));
							_data[1][read + i] = j;
						}
					}
					//else if (_bits == 4)
					//{
					//	j = *(int32_t*)(frame->data[0] + i * sizeof(int32_t));
					//	_data[0][read + i] = ((float)j) / (float)0x80000000;
					//	if (_channels > 1)
					//	{
					//		j = *(int32_t*)(frame->data[1] + i * sizeof(int32_t));
					//		_data[1][read + i] = ((float)j) / (float)0x80000000;
					//	}
					//}
					//else if (_bits == 8)
					//{
					//	l = *(int64_t*)(frame->data[0] + i * sizeof(int64_t));
					//	_data[0][read + i] = ((float)l) / (float)0x8000000000000000;
					//	if (_channels > 1)
					//	{
					//		l = *(int64_t*)(frame->data[1] + i * sizeof(int64_t));
					//		_data[1][read + i] = ((float)l) / (float)0x8000000000000000;
					//	}
					//}
				}
				read += frame->nb_samples;
			}
		}
	}

	// Clean up!
	av_free(frame);
}

void AudioManager::GetTrackMetrics(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream)
{
	_trackSize = 0;
	int duration = 0;
	int sampleduration = -1;
	AVFrame* frame = av_frame_alloc();
	if (!frame)
	{
		_resultMessage = "Error allocating the frame";
		_state = 0;
		return;
	}

	AVPacket readingPacket;
	av_init_packet(&readingPacket);

	// Read the packets in a loop
	while (av_read_frame(formatContext, &readingPacket) == 0)
	{
		if (readingPacket.stream_index == audioStream->index)
		{
			AVPacket decodingPacket = readingPacket;

			// Audio packets can have multiple audio frames in a single packet
			while (decodingPacket.size > 0)
			{
				// Try to decode the packet into a frame
				// Some frames rely on multiple packets, so we have to make sure the frame is finished before
				// we can use it
				int gotFrame = 0;
				int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, &decodingPacket);

				if (result >= 0 && gotFrame)
				{
					if (_isCBR)
					{
						if (sampleduration == -1)
						{
							sampleduration = frame->pkt_duration / frame->nb_samples;
						}
						else
						{
							if (sampleduration != frame->pkt_duration / frame->nb_samples)
							{
								_isCBR = false;
							}
						}
					}

					decodingPacket.size -= result;
					_trackSize += frame->nb_samples;
					duration += frame->pkt_duration;
				}
				else
				{
					decodingPacket.size = 0;
				}
			}
		}

		// You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
		av_free_packet(&readingPacket);
	}

	// Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
	// is set, there can be buffered up frames that need to be flushed, so we'll do that
	if (codecContext->codec->capabilities & CODEC_CAP_DELAY)
	{
		av_init_packet(&readingPacket);
		// Decode all the remaining frames in the buffer, until the end is reached
		int gotFrame = 1;
		while (gotFrame)
		{
			int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, &readingPacket);
			if (result >= 0 && gotFrame)
			{
				if (_isCBR)
				{
					if (sampleduration == -1)
					{
						sampleduration = frame->pkt_duration / frame->nb_samples;
					}
					else
					{
						if (sampleduration != frame->pkt_duration / frame->nb_samples)
						{
							_isCBR = false;
						}
					}
				}

				_trackSize += frame->nb_samples;
				duration += frame->pkt_duration;
			}
		}
	}

	// Clean up!
	av_free(frame);

	_lengthMS = duration / (audioStream->time_base.den / 1000);
}

void AudioManager::ExtractMP3Tags(AVFormatContext* formatContext)
{
	AVDictionaryEntry* tag = av_dict_get(formatContext->metadata, "title", NULL, 0);
	if (tag != NULL)
	{
		_title = tag->value;
	}
	tag = av_dict_get(formatContext->metadata, "album", NULL, 0);
	if (tag != NULL)
	{
		_album = tag->value;
	}
	tag = av_dict_get(formatContext->metadata, "artist", NULL, 0);
	if (tag != NULL)
	{
		_artist = tag->value;
	}
}

bool AudioManager::CheckCBR()
{
	// already calculated
	return _isCBR;
}
#endif

#ifdef USE_MPG123
// Open and read the media file into memory
int AudioManager::OpenMediaFile()
{
    int err;
    size_t buffer_size;
	mpg123_handle *phm;

    err = mpg123_init();
    if(err != MPG123_OK || (phm = mpg123_new(NULL, &err)) == NULL)
    {
		std::ostringstream stringStream;
		stringStream << "Basic setup goes wrong: " << mpg123_plain_strerror(err);
		_resultMessage = stringStream.str();
		_state = 0;
		if (phm != NULL)
		{
			mpg123_delete(phm);
			mpg123_exit();
		}
		return -1;
    }

    /* open the file and get the decoding format */
    if( mpg123_open(phm, _audio_file.c_str()) != MPG123_OK ||
       mpg123_getformat(phm, &_rate, &_channels, &_encoding) != MPG123_OK )
    {
		std::ostringstream stringStream;
		stringStream << "Trouble with mpg123: " << mpg123_strerror(phm);
		_resultMessage = stringStream.str();
		_state = 0;
		if (phm != NULL)
		{
			mpg123_close(phm);
			mpg123_delete(phm);
			mpg123_exit();
		}
		return -1;
    }

    if(_encoding != MPG123_ENC_SIGNED_16 )
    {
        _resultMessage = "Encoding unsupported.  Must be signed 16 bit.";
		_state = 0;
    }

    /* set the output format and open the output device */
    _bits = mpg123_encsize(_encoding);

    /* Get Track Size */
    _trackSize = CalcTrackSize(phm, _bits, _channels);
	_lengthMS = CalcLengthMS();
    buffer_size = mpg123_outblock(phm);
    int size = (_trackSize+buffer_size)*_bits*_channels;

	// Check if we have read this before ... if so dump the old data
	if (_data[1] != NULL && _data[1] != _data[0])
	{
		free(_data[1]);
		_data[1] = NULL;
	}
	if (_data[0] != NULL)
	{
		free(_data[0]);
		_data[0] = NULL;
	}

	char * trackData = (char*)malloc(size);
    LoadTrackData(phm, trackData, size);

	// Split data into left and right and normalize -1 to 1
	_data[0] = (float*)calloc(sizeof(float)*(_trackSize + _extra), 1);
	for (int i = 0; i < _trackSize + _extra; i++)
	{
		(*(_data[0] + i)) = 0.0;
	}
	if( _channels == 2 )
    {
        _data[1] = (float*)calloc(sizeof(float)*(_trackSize + _extra), 1);
		for (int i = 0; i < _trackSize + _extra; i++)
		{
			(*(_data[1] + i)) = 0.0;
		}
        SplitTrackDataAndNormalize((signed short*)trackData, _trackSize, _data[0], _data[1]);
    }
    else if( _channels == 1 )
    {
        NormalizeMonoTrackData((signed short*)trackData,_trackSize,_data[0]);
        _data[1] = _data[0];
    }
    else
    {
        _resultMessage = "More than 2 audio channels is not supported yet.";
		_state = 0;
    }
	free(trackData);

	ExtractMP3Tags(phm);

    mpg123_close(phm);
    mpg123_delete(phm);
    mpg123_exit();
	phm = NULL;

	return _trackSize;
}

// Load the track data
void AudioManager::LoadTrackData(mpg123_handle *phm, char* data, int maxSize)
{
	size_t buffer_size;
	unsigned char *buffer;
	size_t done;
	int bytesRead = 0;
	buffer_size = mpg123_outblock(phm);
	buffer = (unsigned char*)malloc(buffer_size * sizeof(unsigned char));
	mpg123_seek(phm, 0, SEEK_SET);
	for (bytesRead = 0; mpg123_read(phm, buffer, buffer_size, &done) == MPG123_OK; )
	{
		if ((bytesRead + done) >= maxSize)
		{
			_resultMessage = "Error reading data from mp3, too much data read.";
			_state = 0;
			free(buffer);
			return;
		}
		memcpy(data + bytesRead, buffer, done);
		bytesRead += done;
	}
	free(buffer);
}

// WOrk out the number of float values in the song data
int AudioManager::CalcTrackSize(mpg123_handle *phm, int bits, int channels)
{
	size_t buffer_size;
	unsigned char *buffer;
	size_t done;
	int trackSize = 0;
	int fileSize = 0;

	if (mpg123_length(phm) > 0)
	{
		return mpg123_length(phm);
	}

	buffer_size = mpg123_outblock(phm);
	buffer = (unsigned char*)malloc(buffer_size * sizeof(unsigned char));

	mpg123_seek(phm, 0, SEEK_SET);
	for (fileSize = 0; mpg123_read(phm, buffer, buffer_size, &done) == MPG123_OK; )
	{
		fileSize += done;
	}

	free(buffer);
	trackSize = fileSize / (bits*channels);
	return trackSize;
}

// Check if the MP3 file is constant bitrate
bool AudioManager::CheckCBR()
{
	// MP3 files are nasty
	// The spec is not public

	unsigned int checkvalidmask = 0xFFFF0DCF; // this mask removes things that can vary between frames
	unsigned int master = 0; // this is the master header we use to verify that the frame header is a header
	int masterbitrate = -1; // this is the bitrate we first believe the file to be ... it is used to find VBR files which dont contain the Xing frame
	bool isCBR = true; // we start assuming the file is CBR ... and look for evidence it isnt
#ifdef _DEBUG
	int px = 0; // used in debugging to see frame start point
#endif

	wxFFile mp3file(_audio_file, "rb");

	// we are going to scan the whole file looking for evidence it isnt CBR
	while (!mp3file.Eof())
	{
		// scan until I find the next header frame ... this has a 0xFF byte followed by a byte with the first 3 bits high ... even then you cant be sure it is a header
		bool atstart = false;
		while (!atstart && !mp3file.Eof())
		{
			char start = ' ';
			while (start != (char)0xFF && start != 'I' && !mp3file.Eof())
			{
				mp3file.Read(&start, 1);
			}
#ifdef _DEBUG
			px = mp3file.Tell() - 1;
#endif
			if (start == (char)0xFF)
			{
				if (!mp3file.Eof())
				{
					mp3file.Read(&start, 1);
					if ((char)(start & 0xE0) == (char)0xE0)
					{
						atstart = true;
						mp3file.Seek(-2, wxFromCurrent);
					}
					else
					{
						mp3file.Seek(-1, wxFromCurrent);
					}
				}
			}
			else if (start == 'I')
			{
				mp3file.Read(&start, 1);
				if (start == 'D')
				{
					mp3file.Read(&start, 1);
					if (start == '3')
					{
						// ID3 tag
						char v[2];
						mp3file.Read(&v, 2);
						char f;
						mp3file.Read(&f, 1);
						bool xh = f & 0x40;
						bool ft = f & 0x10;
						char sizess[4];
						mp3file.Read(&sizess, 4);
						int32_t size;
						size = ((((int32_t)sizess[0]) & 0x7F) << 21) +
							((((int32_t)sizess[1]) & 0x7F) << 14) +
							((((int32_t)sizess[2]) & 0x7F) << 7) +
							(((int32_t)sizess[3]) & 0x7F);
						mp3file.Seek(size, wxFromCurrent);
					}
					else
					{
						// Jump back to the D ... I cant go back to the I or I will be back here again
						mp3file.Seek(-2, wxFromCurrent);
					}
				}
				else
				{
					// Jump back to the D ... I cant go back to the I or I will be back here again
					mp3file.Seek(-1, wxFromCurrent);
				}
			}
		}

		// At this point we have found a possible header ... and reset the file pointer to the start of the header
		if (!mp3file.Eof())
		{
			// read the header 4 bytes
			char fh[4];
			mp3file.Read(&fh[0], sizeof(fh));

			// this must be true
			if (fh[0] == (char)0xFF && (char)(fh[1] & 0xE0) == (char)0xE0)
			{
				// if we have not found a header before work out what the non variant parts of the header are and save them
				if (master == 0)
				{
					master = ((((unsigned int)fh[0]) << 24) + (((unsigned int)fh[1]) << 16) + (((unsigned int)fh[2]) << 8) + ((unsigned int)fh[3])) & checkvalidmask;
				}

				// extract the non variant parts of the header for the current record
				unsigned int compare = ((((unsigned int)fh[0]) << 24) + (((unsigned int)fh[1]) << 16) + (((unsigned int)fh[2]) << 8) + ((unsigned int)fh[3])) & checkvalidmask;

				// check all the fields that should match across frames do
				// this is the best way to be reasonably sure this really is a new frame
				if (compare == master)
				{
					// now extract the frame attributes
					int version = (fh[1] & 0x18) >> 3;
					int layertype = (fh[1] & 0x06) >> 1;
					int bitrateindex = (fh[2] & 0xF0) >> 4;
					int bitrate = decodebitrateindex(bitrateindex, version, layertype) * 1000;
					int samplerateindex = (fh[2] & 0x0C) >> 2;
					int samplerate = decodesamplerateindex(samplerateindex, version);
					int padding = (fh[2] & 0x02) >> 1;
					//int mono = fh[3] & 0xC0 >> 6;

					int framesize;

					// reject anything that looks invalid
					if (samplerate == 0 || layertype == 0 || version == 1 || bitrate == 0 || samplerate == 3)
					{
						// frame header is not valid
						framesize = 0;
						// this was a false first header so reset our master frame header
						if (masterbitrate == -1)
						{
							master = 0;
						}
					}
					else
					{
						// calculate the frame size ... this is the full size including the header
						if (layertype == 3)
						{
							framesize = (12 * bitrate / samplerate + padding) * 4;
						}
						else
						{
							framesize = 144 * bitrate / samplerate + padding;
						}
					}

					// if framesize is zero then this clearly isnt a proper frame
					if (framesize != 0)
					{
						// if we have not saved the bitrate we think it is from the first frame do so
						if (masterbitrate == -1)
						{
							masterbitrate = bitrate;
						}

						// if this frame has a different bitrate to the first frame ... then this is a VBR file
						if (masterbitrate != bitrate)
						{
							isCBR = false;

							// jump to the end of the file
							mp3file.Seek(0, wxFromEnd);
							break;
						}

						// seek to Xing tag offset
						mp3file.Seek(32, wxFromCurrent);
						mp3file.Read(&fh[0], sizeof(fh));

						if (fh[0] == 'X' && fh[1] == 'i' && fh[2] == 'n' && fh[3] == 'g')
						{
							isCBR = false;

							// jump to the end of the file
							mp3file.Seek(0, wxFromEnd);
							break;
						}
						else
						{
							// jump over the rest of the frame ... this helps avoiding find false headers in the music data
							mp3file.Seek(framesize - 32 - 2 * sizeof(fh), wxFromCurrent);
						}
					}
				}
			}
		}
	}

	mp3file.Close();

	return isCBR;
}

// Extract mp3 tags from the file
void AudioManager::ExtractMP3Tags(mpg123_handle *phm)
{
	_title = "";
	_album = "";
	_artist = "";

	// get meta tags
	mpg123_id3v1* v1;
	mpg123_id3v2* v2;

	mpg123_scan(phm);
	int meta = mpg123_meta_check(phm);

	if (meta == MPG123_ID3 && mpg123_id3(phm, &v1, &v2) == MPG123_OK)
	{
		if (v2 != NULL) // "ID3V2 tag found"
		{
			_title = v2->title == NULL ? "" : v2->title->p;
			_artist = v2->artist == NULL ? "" : v2->artist->p;
			_album = v2->album == NULL ? "" : v2->album->p;
		}
		else if (v1 != NULL) // "ID3V1 tag found"
		{
			_title = v1->title[0];
			_artist = v1->artist[0];
			_album = v1->album[0];
		}
	}
}
#endif

// Access a single piece of track data
float AudioManager::GetLeftData(int offset)
{
	if (offset > _trackSize)
	{
		return 0;
	}
	return _data[0][offset];
}

// Access a single piece of track data
float AudioManager::GetRightData(int offset)
{
	if (offset > _trackSize)
	{
		return 0;
	}
	return _data[1][offset];
}

// Access track data but get a pointer so you can then read a block directly
float* AudioManager::GetLeftDataPtr(int offset)
{
	if (offset > _trackSize)
	{
		return 0;
	}
	return &_data[0][offset];
}

// Access track data but get a pointer so you can then read a block directly
float* AudioManager::GetRightDataPtr(int offset)
{
	if (offset > _trackSize)
	{
		return 0;
	}
	return &_data[1][offset];
}

// AudioScanJob Functions
// This job runs the frame data extraction on a background thread
AudioScanJob::AudioScanJob(AudioManager* audio)
{
	_audio = audio;
	_status = "Idle.";
}

// Run the job
void AudioScanJob::Process()
{
	_status = "Processing.";
	_audio->DoPrepareFrameData();
	_status = "Done.";
}

// xLightsVamp Functions
xLightsVamp::xLightsVamp()
{
	_loader = Vamp::HostExt::PluginLoader::getInstance();
}

xLightsVamp::~xLightsVamp() {}

// extract the features data from a Vamp plugins output
void xLightsVamp::ProcessFeatures(Vamp::Plugin::FeatureList &feature, std::vector<int> &starts, std::vector<int> &ends, std::vector<std::string> &labels) 
{
	bool hadDuration = true;

	for (int x = 0; x < feature.size(); x++) 
	{
		int start = feature[x].timestamp.msec() + feature[x].timestamp.sec * 1000;
		starts.push_back(start);

		if (!hadDuration) 
		{
			ends.push_back(start);
		}
		hadDuration = feature[x].hasDuration;

		if (hadDuration) 
		{
			int end = start + feature[x].duration.msec() + feature[x].duration.sec * 1000;
			ends.push_back(end);
		}
		labels.push_back(feature[x].label);
	}

	if (!hadDuration) 
	{
		ends.push_back(starts[starts.size() - 1]);
	}
}

// Load plugins
void xLightsVamp::LoadPlugins(AudioManager* paudio)
{
	// dont need to load it twice
	if (_loadedPlugins.size() > 0)
	{
		return;
	}

	Vamp::HostExt::PluginLoader::PluginKeyList pluginList = _loader->listPlugins();

	for (int x = 0; x < pluginList.size(); x++)
	{
		Vamp::Plugin *p = _loader->loadPlugin(pluginList[x], paudio->GetRate());
		if (p == nullptr)
		{
			// skip any that dont load
			continue;
		}
		_loadedPlugins.push_back(p);
	}
}

// Get & load plugins that return timing marks
std::list<std::string> xLightsVamp::GetAvailablePlugins(AudioManager* paudio)
{
	std::list<std::string> ret;

	// Load the plugins in case they have not already been loaded
	LoadPlugins(paudio);

	for (std::vector<Vamp::Plugin *>::iterator it = _loadedPlugins.begin(); it != _loadedPlugins.end(); ++it)
	{
		Plugin::OutputList outputs = (*it)->getOutputDescriptors();

		for (Plugin::OutputList::iterator j = outputs.begin(); j != outputs.end(); ++j) 
		{
			if (j->sampleType == Plugin::OutputDescriptor::FixedSampleRate ||
				j->sampleType == Plugin::OutputDescriptor::OneSamplePerStep ||
				!j->hasFixedBinCount ||
				(j->hasFixedBinCount && j->binCount > 1)) 
			{
				// We are filering out this from our return array
				continue;
			}

			std::string name = std::string(wxString::FromUTF8((*it)->getName().c_str()).c_str());

			if (outputs.size() > 1)
			{
				// This is not the plugin's only output.
				// Use "plugin name: output name" as the effect name,
				// unless the output name is the same as the plugin name
				std::string outputName = std::string(wxString::FromUTF8(j->name.c_str()).c_str());
				if (outputName != name)
				{
					std::ostringstream stringStream;
					stringStream << name << ": " << outputName.c_str();
					name = stringStream.str();
				}
			}

			_plugins[name] = (*it);
		}
	}

	for (std::map<std::string, Vamp::Plugin *>::iterator it = _plugins.begin(); it != _plugins.end(); ++it) 
	{
		ret.push_back(it->first);
	}

	return ret;
}

// Get a list of all plugins
std::list<std::string> xLightsVamp::GetAllAvailablePlugins(AudioManager* paudio)
{
	std::list<std::string> ret;

	// load the plugins if they have not already been loaded
	LoadPlugins(paudio);

	for (std::vector<Vamp::Plugin *>::iterator it = _loadedPlugins.begin(); it != _loadedPlugins.end(); ++it) 
	{
		Plugin::OutputList outputs = (*it)->getOutputDescriptors();

		for (Plugin::OutputList::iterator j = outputs.begin(); j != outputs.end(); ++j)
		{
			std::string name = std::string(wxString::FromUTF8((*it)->getName().c_str()).c_str());

			if (outputs.size() > 1)
			{
				// This is not the plugin's only output.
				// Use "plugin name: output name" as the effect name,
				// unless the output name is the same as the plugin name
				std::string outputName = std::string(wxString::FromUTF8(j->name.c_str()).c_str());
				if (outputName != name)
				{
					std::ostringstream stringStream;
					stringStream << name << ": " << outputName.c_str();
					name = stringStream.str();
				}
			}

			_allplugins[name] = *it;
		}
	}

	for (std::map<std::string, Vamp::Plugin *>::iterator it = _allplugins.begin(); it != _allplugins.end(); ++it) 
	{
		ret.push_back(it->first);
	}

	return ret;
}

// Get a plugin
Vamp::Plugin* xLightsVamp::GetPlugin(std::string name)
{
	Plugin* p = _plugins[name];

	if (p == NULL)
	{
		p = _allplugins[name];
	}

	return p;
}

