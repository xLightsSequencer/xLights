#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <string>
#include <list>
#include <shared_mutex>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}

extern "C"
{
#define SDL_MAIN_HANDLED
#include "sdl/include/SDL.h"
}

#include "vamp-hostsdk/PluginLoader.h"
#include "JobPool.h"
#include <wx/progdlg.h>

class AudioManager;

class AudioScanJob : Job
{
private:
	AudioManager* _audio;
	std::string _status;

public:
	AudioScanJob(AudioManager* audio);
	virtual ~AudioScanJob() {};
	virtual void Process() override;
	virtual std::string GetStatus() override { return _status; }
    virtual bool DeleteWhenComplete() override { return true; }
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
	FRAMEDATA_ISTIMINGMARK,
	FRAMEDATA_NOTES
} FRAMEDATATYPE;

typedef enum MEDIAPLAYINGSTATE {
	PLAYING,
	PAUSED,
	STOPPED
} MEDIAPLAYINGSTATE;

typedef void (__cdecl * AudioManagerProgressCallback) (wxProgressDialog* dlg, int pct);

typedef enum SDLSTATE {
    SDLUNINITIALISED,
    SDLINITIALISED,
    SDLOPENED,
    SDLPLAYING,
    SDLNOTPLAYING
} SDLSTATE;

class AudioData
{
    public:
        static int __nextId;
        int _id;
        Uint64  _audio_len;
        Uint8  *_audio_pos;
        Uint8* _original_pos;
        int _volume = SDL_MIX_MAXVOLUME;
        int _rate;
        Uint64 _original_len;
        int _savedpos;
        int _trackSize;
        int _lengthMS;
        bool _paused;
        AudioData();
        ~AudioData() {}
        int Tell();
        void Seek(int ms);
        void SavePos();
        void RestorePos();
        void SeekAndLimitPlayLength(int pos, int len);
        void Pause(bool pause) { _paused = pause; }
};

class SDL
{
    SDLSTATE _state;
    std::list<AudioData*> _audioData;
    std::mutex _audio_Lock;
    float _playbackrate;
    SDL_AudioSpec _wanted_spec;
    int _initialisedRate;

    void Reopen();
    AudioData* GetData(int id);

public:
    SDL();
    virtual ~SDL();
    std::list<AudioData*> GetAudio() const { return _audioData; }
    int Tell(int id);
    void Seek(int id, int ms);
    void SetRate(float rate);
    int AddAudio(Uint64 len, Uint8* buffer, int volume, int rate, int tracksize, int lengthMS);
    void RemoveAudio(int id);
    void Play();
    void Pause();
    void Unpause();
    void TogglePause();
    void Stop();
    void SetVolume(int id, int volume);
    int GetVolume(int id);
    void SetGlobalVolume(int volume);
    int GetGlobalVolume() const;
    void SeekAndLimitPlayLength(int id, int pos, int len);
    void Pause(int id, bool pause);
};

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
	int64_t _trackSize;
	int _bits;
	int _extra;
	std::string _resultMessage;
	int _state;
	float *_data[2]; // audio data
	Uint8* _pcmdata;
	Uint64 _pcmdatasize;
	std::string _title;
	std::string _artist;
	std::string _album;
	int _intervalMS;
	int _lengthMS;
	bool _frameDataPrepared;
	float _bigmax;
	float _bigspread;
	float _bigmin;
	float _bigspectogrammax;
	MEDIAPLAYINGSTATE _media_state;
	bool _polyphonicTranscriptionDone;
    int _sdlid;
    bool _ok;

	void GetTrackMetrics(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream);
	void LoadTrackData(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream);
	void ExtractMP3Tags(AVFormatContext* formatContext);
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
    bool IsOk() const { return _ok; }
    static size_t GetAudioFileLength(std::string filename);
	void Seek(int pos);
	void Pause();
	void Play();
    void Play(int posms, int lenms);
    void Stop();
	static void SetPlaybackRate(float rate);
	MEDIAPLAYINGSTATE GetPlayingState();
	int Tell();
	xLightsVamp* GetVamp() { return &_vamp; };
	AudioManager(const std::string& audio_file, int step = 4096, int block = 32768);
	~AudioManager();
	void SetVolume(int volume);
    int GetVolume();
    static void SetGlobalVolume(int volume);
    static int GetGlobalVolume();
	int GetTrackSize() { return _trackSize; };
	long GetRate() { return _rate; };
	int GetChannels() { return _channels; };
	int GetState() { return _state; };
	std::string GetResultMessage() { return _resultMessage; };
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
	void DoPolyphonicTranscription(wxProgressDialog* dlg, AudioManagerProgressCallback progresscallback);
	bool IsPolyphonicTranscriptionDone() { return _polyphonicTranscriptionDone; };
};

#endif
