#include "FSEQFile.h"
#include <wx/file.h>
#include <log4cpp/Category.hh>
#include <wx/filename.h>
#include "../xLights/UtilFunctions.h"

FSEQFile::FSEQFile()
{
    _audiofilename = "";
    _channelsPerFrame = 0;
    _filename = "";
    _frameMS = 0;
    _frames = 0;
    _fh = nullptr;
    _frameBuffer = nullptr;
    _minorVersion = 0;
    _majorVersion = 0;
    _colourEncoding = 0;
    _gamma = 255;
    _frame0Offset = 0;
    _ok = false;
}

FSEQFile::FSEQFile(const std::string& filename)
{
    _audiofilename = "";
    _channelsPerFrame = 0;
    _filename = FixFile("", filename);
    _frameMS = 0;
    _frames = 0;
    _fh = nullptr;
    _frameBuffer = nullptr;
    _minorVersion = 0;
    _majorVersion = 0;
    _colourEncoding = 0;
    _gamma = 255;
    _frame0Offset = 0;
    _ok = true;
    Load(filename);
}

std::string FSEQFile::GrabAudioFilename(const std::string& fseqFilename)
{
    FSEQFile fseq(fseqFilename);

    return fseq.GetAudioFileName();
}

FSEQFile::~FSEQFile()
{
    Close();
}

void FSEQFile::Close()
{
    if (_fh != nullptr)
    {
        _fh->Close();
        delete _fh;
        _fh = nullptr;

        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.info("FSEQ file %s closed.", (const char *)_filename.c_str());
    }

    if (_frameBuffer != nullptr)
    {
        free(_frameBuffer);
        _frameBuffer = nullptr;
    }

    _ok = false;
}

int FSEQFile::ReadInt16(wxFile* fh)
{
    wxByte lsb;
    fh->Read(&lsb, sizeof(lsb));
    wxByte msb;
    fh->Read(&msb, sizeof(msb));
    return (((int)msb) << 8) + lsb;
}

int FSEQFile::ReadInt32(wxFile* fh)
{
    wxByte lsb;
    fh->Read(&lsb, sizeof(lsb));
    wxByte lsb1;
    fh->Read(&lsb1, sizeof(lsb1));
    wxByte lsb2;
    fh->Read(&lsb2, sizeof(lsb2));
    wxByte msb;
    fh->Read(&msb, sizeof(msb));
    return (((int)msb) << 24) + (((int)lsb2) << 16) + (((int)lsb1) << 8) + lsb;
}

void FSEQFile::Load(const std::string& filename)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    Close();

    _filename = FixFile("", filename);
    _fh = new wxFile(FixFile("", _filename));

    if (_fh->IsOpened())
    {
        int offset = 0;
        char tag[5];
        memset(tag, 0x00, sizeof(tag));
        _fh->Read(tag, sizeof(tag)-1);
        if (std::string(tag) == "PSEQ")
        {
            _frame0Offset = ReadInt16(_fh);
            _fh->Read(&_minorVersion, sizeof(_minorVersion));
            _fh->Read(&_majorVersion, sizeof(_majorVersion));
            int fixedheader = ReadInt16(_fh); // fixed header length
            _channelsPerFrame = ReadInt32(_fh);
            _frames = ReadInt32(_fh);
            _frameMS = ReadInt16(_fh);
            int universes = ReadInt16(_fh); // universes
            int usize  = ReadInt16(_fh); // universe size
            _gamma = _fh->Read(&_gamma, sizeof(_gamma));
            _fh->Read(&_colourEncoding, sizeof(_colourEncoding));
            int fill = ReadInt16(_fh); // fill
            if (_frame0Offset > 28)
            {
                int mediafilenamelength = ReadInt16(_fh);
                if (mediafilenamelength > 0)
                {
                    char* buf = (char*)malloc(mediafilenamelength + 1);
                    memset(buf, 0x00, mediafilenamelength + 1);
                    ReadInt16(_fh); // mf
                    _fh->Read(buf, mediafilenamelength);
                    _audiofilename = std::string(buf);
                    _audiofilename = FixFile("", _audiofilename);
                    free(buf);
                }
            }
            _frameBuffer = (wxByte*)malloc(_channelsPerFrame);

            logger_base.info("FSEQ file %s opened.", (const char *)_filename.c_str());
            _ok = true;
        }
        else
        {
            logger_base.error("FSEQ file %s format does not look valid.", (const char *)_filename.c_str());
            Close();
        }
    }
    else
    {
        logger_base.error("FSEQ file %s could not be opened.", (const char *)_filename.c_str());
        Close();
    }
}

void FSEQFile::ReadData(wxByte* buffer, size_t buffersize, size_t frame, APPLYMETHOD applyMethod, size_t offset, size_t channels)
{
    if (frame >= _frames) return; // cant read past end of file

    if (_fh->Tell() != _frame0Offset + _channelsPerFrame * frame)
    {
        // we need to seek to our frame
        _fh->Seek(_frame0Offset + _channelsPerFrame * frame, wxFromStart);
    }
    
    // read in the frame from disk
    _fh->Read(_frameBuffer, _channelsPerFrame);

    if (channels > 0)
    {
        Blend(buffer, buffersize, _frameBuffer + offset, std::min(_channelsPerFrame, channels), applyMethod, offset);
    }
    else
    {
        Blend(buffer, buffersize, _frameBuffer, _channelsPerFrame, applyMethod, offset);
    }
}

