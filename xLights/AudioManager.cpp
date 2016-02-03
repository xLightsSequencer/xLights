#include "AudioManager.h"
#include <wx/string.h>
#include <sstream>
#include <algorithm>

using namespace Vamp;

xLightsVamp::xLightsVamp()
{
	_loader = Vamp::HostExt::PluginLoader::getInstance();
}

xLightsVamp::~xLightsVamp()
{
	for (int i = 0; i < _loadedPlugins.size(); i++)
	{
		delete _loadedPlugins[i];
	}
	_loadedPlugins.empty();
}

AudioManager::AudioManager(std::string audio_file, int step = 1024, int block = 1024)
{
	_audio_file = audio_file;
	_phm = NULL;
	_state = -1; // state uninitialised. 0 is error. 1 is loaded ok
	_resultMessage = "";
	_data[0] = NULL;
	_data[1] = NULL;

	_extra = std::max(step, block) + 1;
	OpenMediaFile();

	// if we got here without setting state to zero then all must be good so set state to 1 success
	if (_state == -1)
	{
		_state = 1;
	}
}

void AudioManager::SetStepBlock(int step, int block)
{
	int extra = std::max(step, block) + 1;

	if (extra != _extra)
	{
		_extra = extra;
		_state = -1;
		OpenMediaFile();
	}
}

AudioManager::~AudioManager()
{
	if (_phm != NULL)
	{
		mpg123_close(_phm);
		mpg123_delete(_phm);
		mpg123_exit();
		_phm = NULL;
	}

	if (_data[0] != NULL)
	{
		delete _data[0];
		_data[0] = NULL;
	}
	if (_data[1] != NULL)
	{
		delete _data[1];
		_data[1] = NULL;
	}
}

int AudioManager::CalcTrackSize(int bits, int channels)
{
    size_t buffer_size;
    unsigned char *buffer;
    size_t done;
    int trackSize = 0;
    int fileSize = 0;

    if(mpg123_length(_phm) > 0)
    {
        return mpg123_length(_phm);
    }

    buffer_size = mpg123_outblock(_phm);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));

    mpg123_seek(_phm,0,SEEK_SET);
    for (fileSize = 0 ; mpg123_read(_phm, buffer, buffer_size, &done) == MPG123_OK ; )
    {
        fileSize += done;
    }

    free(buffer);
    trackSize = fileSize/(bits*channels);
    return trackSize;
}

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

void AudioManager::NormalizeMonoTrackData(signed short* trackData, int trackSize, float* leftData)
{
    signed short lSample;
    for(int i=0;i<trackSize;i++)
    {
        lSample = trackData[i];
        leftData[i] = (float)lSample/(float)32768;
    }
}

void AudioManager::LoadTrackData(char* data, int maxSize)
{
    size_t buffer_size;
    unsigned char *buffer;
    size_t done;
    int bytesRead=0;
    buffer_size = mpg123_outblock(_phm);
    buffer = (unsigned char*) malloc(buffer_size * sizeof(unsigned char));
    mpg123_seek(_phm, 0, SEEK_SET);
    for (bytesRead = 0 ; mpg123_read(_phm, buffer, buffer_size, &done) == MPG123_OK ; )
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

int AudioManager::CalcLengthMS()
{
	float seconds = (float)_trackSize * ((float)1 / (float)_rate);
	return (int)(seconds * (float)1000);
}

int AudioManager::OpenMediaFile()
{
    int err;
    size_t buffer_size;

    err = mpg123_init();
    if(err != MPG123_OK || (_phm = mpg123_new(NULL, &err)) == NULL)
    {
		std::ostringstream stringStream;
		stringStream << "Basic setup goes wrong: " << mpg123_plain_strerror(err);
		_resultMessage = stringStream.str();
		_state = 0;
        return -1;
    }

    /* open the file and get the decoding format */
    if( mpg123_open(_phm, _audio_file.c_str()) != MPG123_OK ||
       mpg123_getformat(_phm, &_rate, &_channels, &_encoding) != MPG123_OK )
    {
		std::ostringstream stringStream;
		stringStream << "Trouble with mpg123: " << mpg123_strerror(_phm);
		_resultMessage = stringStream.str();
		_state = 0;
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
    _trackSize = CalcTrackSize(_bits, _channels);
	_lengthMS = CalcLengthMS();
    buffer_size = mpg123_outblock(_phm);
    int size = (_trackSize+buffer_size)*_bits*_channels;

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
    LoadTrackData(trackData, size);

	// Split data into left and right and normalize -1 to 1
	_data[0] = (float*)calloc(sizeof(float)*(_trackSize + _extra), 1);
    if( _channels == 2 )
    {
        _data[1] = (float*)calloc(sizeof(float)*(_trackSize + _extra), 1);
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

    mpg123_close(_phm);
    mpg123_delete(_phm);
    mpg123_exit();
	_phm = NULL;

	return _trackSize;
}

Vamp::Plugin* xLightsVamp::GetPlugin(std::string name)
{
	return plugins[name];
}

std::list<std::string> xLightsVamp::GetAvailablePlugins(AudioManager* paudio)
{
    std::list<std::string> ret;

    Vamp::HostExt::PluginLoader::PluginKeyList pluginList = _loader->listPlugins();
    for (int x = 0; x < pluginList.size(); x++) {
        Vamp::Plugin *p = _loader->loadPlugin(pluginList[x], paudio->GetRate());
        if (p == nullptr) {
            continue;
        }
        _loadedPlugins.push_back(p);
        Plugin::OutputList outputs = p->getOutputDescriptors();

        for (Plugin::OutputList::iterator j = outputs.begin(); j != outputs.end(); ++j) {
            if (j->sampleType == Plugin::OutputDescriptor::FixedSampleRate ||
                j->sampleType == Plugin::OutputDescriptor::OneSamplePerStep ||
                !j->hasFixedBinCount ||
                (j->hasFixedBinCount && j->binCount > 1)) {

                continue;
            }

            std::string name = std::string(wxString::FromUTF8(p->getName().c_str()).c_str());

            if (outputs.size() > 1) {
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

            plugins[name] = p;
        }
    }

    for (std::map<std::string, Vamp::Plugin *>::iterator it = plugins.begin(); it != plugins.end(); ++it) {
        ret.push_back(it->first);
    }

    return ret;
}

void xLightsVamp::ProcessFeatures( Vamp::Plugin::FeatureList &feature, std::vector<int> &starts, std::vector<int> &ends, std::vector<std::string> &labels) {
    bool hadDuration = true;
    for (int x = 0; x < feature.size(); x++) {
        int start = feature[x].timestamp.msec() + feature[x].timestamp.sec * 1000;
        starts.push_back(start);
        if (!hadDuration) {
            ends.push_back(start);
        }
        hadDuration = feature[x].hasDuration;
        if (hadDuration) {
            int end = start + feature[x].duration.msec() + feature[x].duration.sec * 1000;
            ends.push_back(end);
        }
        labels.push_back(feature[x].label);
    }
    if (!hadDuration) {
        ends.push_back(starts[starts.size() - 1]);
    }
}

void AudioManager::ExtractMP3Tags()
{
	_title = "";
	_album = "";
	_artist = "";

	// get meta tags
	mpg123_id3v1* v1;
	mpg123_id3v2* v2;

	mpg123_scan(_phm);
	int meta = mpg123_meta_check(_phm);

	if (meta == MPG123_ID3 && mpg123_id3(_phm, &v1, &v2) == MPG123_OK)
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

float AudioManager::GetLeftData(int offset)
{
	if (offset > _trackSize)
	{
		int a = 0;
	}
	return _data[0][offset];
}

float AudioManager::GetRightData(int offset)
{
	return _data[1][offset];
}

float* AudioManager::GetLeftDataPtr(int offset)
{
	if (offset > _trackSize)
	{
		int a = 0;
	}
	return &_data[0][offset];
}

float* AudioManager::GetRightDataPtr(int offset)
{
	return &_data[1][offset];
}
