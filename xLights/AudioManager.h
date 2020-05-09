#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <memory>
#include <string>
#include <list>
#include <shared_mutex>
#include <vector>
#include <future>

extern "C"
{
#include <libavformat/avformat.h> // for AVSampleFormat

    struct AVFormatContext;
    struct AVCodecContext;
    struct AVStream;
    struct AVFrame;
    struct AVPacket;
    struct SwrContext;
}

extern "C"
{
#define SDL_MAIN_HANDLED
#include "sdl/include/SDL.h"
}

#include "vamp-hostsdk/PluginLoader.h"
#include <wx/progdlg.h>

class AudioManager;

enum class AUDIOSAMPLETYPE
{
    RAW,
    BASS,
    TREBLE,
    CUSTOM,
    ALTO,
    NONVOCALS
};

class xLightsVamp
{
	Vamp::HostExt::PluginLoader *_loader = nullptr;
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
    static void ProcessFeatures(Vamp::Plugin::FeatureList &feature, std::vector<int> &starts, std::vector<int> &ends, std::vector<std::string> &labels);
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
        long  _audio_len;
        Uint8* _audio_pos;
        Uint8* _original_pos;
        int _volume = SDL_MIX_MAXVOLUME;
        int _rate;
        long _original_len;
        long _savedpos;
        long _trackSize;
        long _lengthMS;
        bool _paused;
        AudioData();
        ~AudioData() {}
        long Tell() const;
        void Seek(long ms);
        void SavePos();
        void RestorePos();
        void SeekAndLimitPlayLength(long pos, long len);
        void Pause(bool pause) { _paused = pause; }
};

class SDL
{
    SDL_AudioDeviceID _dev;
    SDL_AudioDeviceID _inputdev;
    SDLSTATE _state;
    std::list<AudioData*> _audioData;
    std::mutex _audio_Lock;
    float _playbackrate = 1.0f;
    SDL_AudioSpec _wanted_spec;
    SDL_AudioSpec _wanted_inputspec;
    int _initialisedRate = 44100;
    std::string _device;
    std::string _inputDevice;
    int _listeners = 0;

    void Reopen();
    AudioData* GetData(int id);

public:
    SDL(const std::string& device = "", const std::string& inputDevice = "");
    virtual ~SDL();
    std::list<AudioData*> GetAudio() const { return _audioData; }
    long Tell(int id);
    void Seek(int id, long ms);
    void SetRate(float rate);
    int AddAudio(long len, Uint8* buffer, int volume, int rate, long tracksize, long lengthMS);
    void RemoveAudio(int id);
    void Play();
    void Pause();
    void Unpause();
    void TogglePause();
    void Stop();
    void SetVolume(int id, int volume);
    int GetVolume(int id);
    static void SetGlobalVolume(int volume);
    static int GetGlobalVolume();
    void SeekAndLimitPlayLength(int id, long pos, long len);
    void Pause(int id, bool pause);
    bool HasAudio(int id);
    static std::list<std::string> GetAudioDevices();
    static std::list<std::string> GetInputAudioDevices();
    bool AudioDeviceChanged();
    bool OpenAudioDevice(const std::string& device);
    bool OpenInputAudioDevice(const std::string& device);
    void SetAudioDevice(const std::string& device);
    void SetInputAudioDevice(const std::string& inputDevice);
    bool CloseAudioDevice();
    bool CloseInputAudioDevice();
    int GetInputMax(int ms);
	std::vector<float> GetOutputSpectrum(int ms);
    int GetInputAudio(uint8_t* buffer, int bufsize);
    void PurgeAllButInputAudio(int ms);
    std::vector<float> GetInputSpectrum(int ms);
    void PurgeInput();
    void DumpState(std::string device, int devid, SDL_AudioSpec* wanted, SDL_AudioSpec* actual);
    void StartListening(const std::string& inputDevice = "");
    void StopListening();
    bool IsListening();
};

struct FilteredAudioData
{
    AUDIOSAMPLETYPE type;
    int lowNote = 0;
    int highNote = 127;
    float* data = nullptr;
    int16_t *pcmdata = nullptr;
};

class AudioManager
{
    std::shared_timed_mutex _mutex;
    std::shared_timed_mutex _mutexAudioLoad;
    long _loadedData = 0;
    std::vector<std::vector<std::list<float>>> _frameData;
	std::string _audio_file;
	xLightsVamp _vamp;
	long _rate = 44100;
	int _channels = 0;
    long _trackSize = 0;
	int _bits = 0;
	int _extra = 0;
	std::string _resultMessage;
	int _state = 0;
	float *_data[2]; // audio data
	Uint8* _pcmdata = nullptr;
	long _pcmdatasize = 0;
	std::string _title;
	std::string _artist;
	std::string _album;
	int _intervalMS = 50;
	long _lengthMS = 0;
	bool _frameDataPrepared = false;
	float _bigmax = 0;
	float _bigspread = 0;
	float _bigmin = 0;
	float _bigspectogrammax = 0;
	MEDIAPLAYINGSTATE _media_state;
	bool _polyphonicTranscriptionDone = false;
    std::vector<FilteredAudioData*> _filtered;
    int _sdlid = 0;
    bool _ok = false;
    std::string _hash;
    std::future<void> _prepFrameData;
    std::future<void> _loadingAudio;

	void GetTrackMetrics(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream);
	void LoadTrackData(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream);
	void ExtractMP3Tags(AVFormatContext* formatContext);
	long CalcLengthMS() const;
	void SplitTrackDataAndNormalize(signed short* trackData, long trackSize, float* leftData, float* rightData) const;
    static void NormalizeMonoTrackData(signed short* trackData, long trackSize, float* leftData);
	int OpenMediaFile();
	void PrepareFrameData(bool separateThread);
    static int decodebitrateindex(int bitrateindex, int version, int layertype);
	int decodesamplerateindex(int samplerateindex, int version) const;
    static int decodesideinfosize(int version, int mono);
	std::list<float> CalculateSpectrumAnalysis(const float* in, int n, float& max, int id) const;

    void LoadAudioFromFrame( AVFormatContext* formatContext, AVCodecContext* codecContext, AVPacket* decodingPacket, AVFrame* frame, SwrContext* au_convert_ctx,
                             bool receivedEOF, int out_channels, uint8_t* out_buffer, long& read, int& lastpct );
    void LoadDecodedAudioFromFrame( AVFrame* frame, AVFormatContext* formatContext, SwrContext* au_convert_ctx,
                                    int out_channels, uint8_t* out_buffer, long& read, int& lastpct );
    void LoadResampledAudio( int sampleCount, int out_channels, uint8_t* out_buffer, long& read, int& lastpct );
    void SetLoadedData(long pos);

    static bool WriteAudioFrame( AVFormatContext *oc, AVCodecContext* codecContext, AVStream *st, float *sampleBuff, int sampleCount, bool clearQueue = false );

public:
    static double MidiToFrequency(int midi);
    static std::string MidiToNote(int midi);
    bool IsOk() const { return _ok; }
    static size_t GetAudioFileLength(std::string filename);
	void Seek(long pos) const;
	void Pause();
	void Play();
    bool IsPlaying() const;
    void Play(long posms, long lenms);
    void Stop();
    void AbsoluteStop();
    long GetLoadedData();
    bool IsDataLoaded(long pos = -1);
    static void SetPlaybackRate(float rate);
	MEDIAPLAYINGSTATE GetPlayingState() const;
	long Tell() const;
	xLightsVamp* GetVamp() { return &_vamp; };
	AudioManager(const std::string& audio_file, int step = 4096, int block = 32768);
	~AudioManager();
	void SetVolume(int volume) const;
    int GetVolume() const;
    static void SetGlobalVolume(int volume);
    static int GetGlobalVolume();
    static void SetAudioDevice(const std::string& device);
    static std::list<std::string> GetAudioDevices();
    static void SetInputAudioDevice(const std::string& device);
    static std::list<std::string> GetInputAudioDevices();
    long GetTrackSize() const { return _trackSize; };
	long GetRate() const { return _rate; };
	int GetChannels() const { return _channels; };
	int GetState() const { return _state; };
	std::string GetResultMessage() const { return _resultMessage; };
	std::string Title() const { return _title; };
	std::string Artist() const { return _artist; };
	std::string Album() const { return _album; };
	std::string FileName() const { return _audio_file; };
    std::string Hash();
	long LengthMS() const { return _lengthMS; };
	float GetRightData(long offset);
	float GetLeftData(long offset);
    void SwitchTo(AUDIOSAMPLETYPE type, int lowNote = 0, int highNote = 127);
    void GetLeftDataMinMax(long start, long end, float& minimum, float& maximum, AUDIOSAMPLETYPE type, int lowNote = 0, int highNote = 127);
	float* GetRightDataPtr(long offset);
	float* GetLeftDataPtr(long offset);
	void SetStepBlock(int step, int block);
	void SetFrameInterval(int intervalMS);
	int GetFrameInterval() const { return _intervalMS; }
	std::list<float>* GetFrameData(int frame, FRAMEDATATYPE fdt, std::string timing);
	std::list<float>* GetFrameData(FRAMEDATATYPE fdt, std::string timing, long ms);
	void DoPrepareFrameData();
	void DoPolyphonicTranscription(wxProgressDialog* dlg, AudioManagerProgressCallback progresscallback);
	bool IsPolyphonicTranscriptionDone() const { return _polyphonicTranscriptionDone; };

    void LoadAudioData(bool separateThread, AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream, AVFrame* frame);
    void DoLoadAudioData(AVFormatContext* formatContext, AVCodecContext* codecContext, AVStream* audioStream, AVFrame* frame);

    static bool CreateAudioFile( const std::vector<float>& left, const std::vector<float>& right, const std::string& targetFile, long bitrate );
    bool WriteCurrentAudio( const std::string& path, long bitrate );

    bool AudioDeviceChanged();

    static SDL* GetSDL();
};

struct AudioParams
{
   int            channelCount;
   AVSampleFormat sampleFormat;
   int            sampleRate;
   int            bytesPerSample;

   AudioParams()
    : channelCount(0), sampleFormat( AV_SAMPLE_FMT_NONE ), sampleRate( 0 ), bytesPerSample( 0 ) {}
   AudioParams( int i_cc, AVSampleFormat i_sf, int i_sampleRate, int i_bytesPerSample )
    : channelCount( i_cc ), sampleFormat( i_sf ), sampleRate( i_sampleRate ), bytesPerSample( i_bytesPerSample ) {}
};

class AudioReaderDecoder;
enum class AudioReaderDecoderInitState
{
    Ok, NoInit,
    FormatContextAllocFails, OpenFails, NoAudioStream, FindStreamInfoFails, CodecContextAllocFails, CodecOpenFails, FrameAllocFails, PacketAllocFails
};

class AudioResampler;
enum class AudioResamplerInitState
{
   Ok, NoInit, InitFails, OutputInitFails
};

class AudioLoader
{
public:
    AudioLoader( const std::string& path, bool forceLittleEndian=false );
    virtual ~AudioLoader();

    enum State { Ok, NoInit, ReaderDecoderInitFails, ResamplerInitFails, LoadAudioFails };

    bool loadAudioData();

    State state() const { return _state; }
    bool readerDecoderInitState( AudioReaderDecoderInitState& state ) const;
    bool resamplerInitState( AudioResamplerInitState& state ) const;

    // 16-bit stereo interleaved audio samples
    const std::vector<int16_t> & processedAudio() const { return _processedAudio; }

protected:
    void processDecodedAudio( const AVFrame* );
    void copyResampledAudio( int sampleCount );
    void flushResampleBuffer();

    const std::string                   _path;
    const bool                          _forceLittleEndian;
    State                               _state;
    std::unique_ptr<AudioReaderDecoder> _readerDecoder;
    std::unique_ptr<AudioResampler>     _resampler;
    std::vector<int16_t>                _processedAudio;
    std::unique_ptr<uint8_t[]>          _resampleBuff;
    int                                 _numInResampleBuffer;
    int                                 _resampleBufferSampleCapacity;
    AudioParams                         _inputParams;
    AudioParams                         _resamplerInputParams;
    int                                 _primingAdjustment;
};
