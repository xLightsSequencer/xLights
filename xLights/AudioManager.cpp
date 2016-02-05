#include "AudioManager.h"
#include <wx/string.h>
#include <sstream>
#include <algorithm>
#include <wx/ffile.h>

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
	_intervalMS = -1;

	_extra = std::max(step, block) + 1;
	OpenMediaFile();

	_isCBR = CheckCBR();

	// if we got here without setting state to zero then all must be good so set state to 1 success
	if (_state == -1)
	{
		_state = 1;
	}
}

void AudioManager::PrepareFrameData()
{
	// process audio data and build data for each frame

	// Max data
	// Min data
	// Spread data
	// beat in frame
	// Note onset in frame
	// frequency breakdown
}

std::list<float> AudioManager::GetFrameData(int frame, FRAMEDATATYPE fdt)
{
	std::list<float> result;

	switch (fdt)
	{
	case FRAMEDATA_HIGH:
		break;
	case FRAMEDATA_LOW:
		break;
	case FRAMEDATA_SPREAD:
		break;
	case FRAMEDATA_ISBEAT:
		break;
	case FRAMEDATA_ISNOTESTART:
		break;
	case FRAMEDATA_VU:
		break;
	}

	return result;
}

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
}

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

bool AudioManager::CheckCBR()
{
	// MP3 files are nasty
	// The spec is not public

	unsigned int checkvalidmask = 0xFFFF0DFF; // this mask removes things that can vary between frames
	unsigned int master = 0; // this is the master header we use to verify that the frame header is a header
	int masterbitrate = -1; // this is the bitrate we first believe the file to be ... it is used to find VBR files which dont contain the Xing frame
	bool isCBR = true; // we start assuming the file is CBR ... and look for evidence it isnt

	wxFFile mp3file(_audio_file, "rb");

	// we are going to scan the whole file looking for evidence it isnt CBR
	while (!mp3file.Eof())
	{
		// scan until I find the next header frame ... this has a 0xFF byte followed by a byte with the first 3 bits high ... even then you cant be sure it is a header
		bool atstart = false;
		while (!atstart && !mp3file.Eof())
		{
			char start = ' ';
			while (start != (char)0xFF && !mp3file.Eof())
			{
				mp3file.Read(&start, 1);
			}
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
					int mono = fh[3] & 0xC0 >> 6;

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

void AudioManager::SetFrameInterval(int intervalMS)
{
	if (_intervalMS != intervalMS)
	{
		_intervalMS = intervalMS;
		PrepareFrameData();
	}
}

void AudioManager::SetStepBlock(int step, int block)
{
	int extra = std::max(step, block) + 1;

	// we only need to reopen if the extra bytes are greater
	// ... and to be honest I am not even sure this is necessary
	if (extra > _extra)
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
		return 0;
	}
	return _data[0][offset];
}

float AudioManager::GetRightData(int offset)
{
	if (offset > _trackSize)
	{
		return 0;
	}
	return _data[1][offset];
}

float* AudioManager::GetLeftDataPtr(int offset)
{
	if (offset > _trackSize)
	{
		return 0;
	}
	return &_data[0][offset];
}

float* AudioManager::GetRightDataPtr(int offset)
{
	if (offset > _trackSize)
	{
		return 0;
	}
	return &_data[1][offset];
}
