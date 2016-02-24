#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <string>
#include <list>
#include <shared_mutex>

//#define USE_MPG123
#define USE_FFMPEG

//#define USE_WXMEDIAPLAYER
#define USE_SDLPLAYER

#ifdef USE_MPG123
#include "sequencer/mpg123.h"
#endif
#ifdef USE_FFMPEG
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#endif
#ifdef USE_SDLPLAYER
extern "C"
{
#include "sdl/include/SDL.h"
}
#endif
#include "vamp-hostsdk/PluginLoader.h"
#include "JobPool.h"

class AudioManager;
class xLightsXmlFile;

class AudioScanJob : Job
{
private:
	AudioManager* _audio;
	std::string _status;

public:
	AudioScanJob(AudioManager* audio);
	virtual ~AudioScanJob() {};
	virtual void Process();
	virtual std::string GetStatus() { return _status; }
};

class xLightsVamp
{
	Vamp::HostExt::PluginLoader *_loader;
	std::map<std::string, Vamp::Plugin *> _plugins;
	std::vector<Vamp::Plugin *> _loadedPlugins;
	std::map<std::string, Vamp::Plugin *> _allplugins;
	std::vector<Vamp::Plugin *> _allloadedPlugins;
	void LoadPlugins(AudioManager* paudio);

public:

	struct PluginDetails
	{
	public:
		std::string Name;
		std::string Description;
	};

	xLightsVamp();
	~xLightsVamp();
	std::string ProcessPlugin(AudioManager* paudio, std::string& plugin);
	void ProcessFeatures(Vamp::Plugin::FeatureList &feature, std::vector<int> &starts, std::vector<int> &ends, std::vector<std::string> &labels);
	std::list<std::string> GetAvailablePlugins(AudioManager* paudio);
	std::list<std::string> GetAllAvailablePlugins(AudioManager* paudio);
	Vamp::Plugin* GetPlugin(std::string name);
};

typedef enum FRAMEDATATYPE {
	FRAMEDATA_HIGH,
	FRAMEDATA_LOW,
	FRAMEDATA_SPREAD,
	FRAMEDATA_VU,
	FRAMEDATA_ISTIMINGMARK
} FRAMEDATATYPE;

typedef enum MEDIAPLAYINGSTATE {
	PLAYING,
	PAUSED,
	STOPPED
} MEDIAPLAYINGSTATE;

class AudioManager
{
	JobPool _jobPool;
	Job* _job;
    std::shared_timed_mutex _mutex;
	std::vector<std::vector<std::list<float>>> _frameData;
	std::string _audio_file;
	xLightsVamp _vamp;
	long _rate;
	int _channels;
	int _trackSize;
	int _bits;
	int _extra;
	std::string _resultMessage;
	int _state;
	float *_data[2]; // audio data
#ifdef USE_SDLPLAYER
	Uint8* _pcmdata;
	Uint64 _pcmdatasize;
	SDL_AudioSpec wanted_spec;
#endif
	std::string _title;
	std::string _artist;
	std::string _album;
	int _intervalMS;
	int _lengthMS;
	bool _frameDataPrepared;
	xLightsXmlFile* _xml_file;
	float _bigmax;
	float _bigspread;
	float _bigmin;
	float _bigspectogrammax;

#ifdef USE_MPG123
	int _encoding;
	int CalcTrackSize(mpg123_handle *phm, int bits, int channels);
	void LoadTrackData(mpg123_handle *phm, char* data, int maxSize);
	void ExtractMP3Tags(mpg123_handle *phm);
	bool CheckCBR();
#endif
#ifdef USE_FFMPEG
	void GetTrackMetrics(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream);
	void LoadTrackData(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream);
	void ExtractMP3Tags(AVFormatContext* formatContext);
#endif
	int CalcLengthMS();
	void SplitTrackDataAndNormalize(signed short* trackData, int trackSize, float* leftData, float* rightData);
	void NormalizeMonoTrackData(signed short* trackData, int trackSize, float* leftData);
	int OpenMediaFile();
	void PrepareFrameData(bool separateThread);
	int decodebitrateindex(int bitrateindex, int version, int layertype);
	int decodesamplerateindex(int samplerateindex, int version);
	int decodesideinfosize(int version, int mono);
	std::list<float> CalculateSpectrumAnalysis(const float* in, int n, float& max, int id);

public:
#ifndef USE_WXMEDIAPLAYER
	void Seek(int pos);
	void Pause();
	void Play();
	void Stop();
	static void SetGlobalPlaybackRate(float rate);
	void SetPlaybackRate(float rate);
	MEDIAPLAYINGSTATE GetPlayingState();
	int Tell();
#endif
	xLightsVamp* GetVamp() { return &_vamp; };
	AudioManager(std::string audio_file, xLightsXmlFile* xml_file, int step, int block);
	~AudioManager();
	int GetTrackSize() { return _trackSize; };
	long GetRate() { return _rate; };
	int GetChannels() { return _channels; };
	int GetState() { return _state; };
	std::string GetResultMessage() { return _resultMessage; };
	std::string GetVampTiming(std::string plugin);
	std::string Title() { return _title; };
	std::string Artist() { return _artist; };
	std::string Album() { return _album; };
	std::string FileName() { return _audio_file; };
	int LengthMS() { return _lengthMS; };
	float GetRightData(int offset);
	float GetLeftData(int offset);
	float* GetRightDataPtr(int offset);
	float* GetLeftDataPtr(int offset);
	void SetStepBlock(int step, int block);
	void SetFrameInterval(int intervalMS);
	int GetFrameInterval() { return _intervalMS; }
	std::list<float>* GetFrameData(int frame, FRAMEDATATYPE fdt, std::string timing);
	void DoPrepareFrameData();
};

#endif
