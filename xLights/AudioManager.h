#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <string>
#include <list>

#include "sequencer/mpg123.h"
#include "vamp-hostsdk/PluginLoader.h"

class AudioManager;

class xLightsVamp
{
	Vamp::HostExt::PluginLoader *_loader;
	std::map<std::string, Vamp::Plugin *> plugins;
	std::vector<Vamp::Plugin *> _loadedPlugins;

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
	Vamp::Plugin* GetPlugin(std::string name);
};

class AudioManager
{
	std::string _audio_file;
	xLightsVamp _vamp;
	mpg123_handle *_phm;
	long _rate;
	int _channels;
	int _trackSize;
	int _encoding;
	int _bits;
	int _extra;
	std::string _resultMessage;
	int _state;
	float *_data[2]; // audio data
	std::string _title;
	std::string _artist;
	std::string _album;
	int _lengthMS;

	int CalcTrackSize(int bits, int channels);
	int CalcLengthMS();
	void SplitTrackDataAndNormalize(signed short* trackData, int trackSize, float* leftData, float* rightData);
	void NormalizeMonoTrackData(signed short* trackData, int trackSize, float* leftData);
	void LoadTrackData(char* data, int maxSize);
	int OpenMediaFile();
	void ExtractMP3Tags();

public:
	xLightsVamp* GetVamp() { return &_vamp; };
	AudioManager(std::string audio_file, int step, int block);
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
};

#endif
