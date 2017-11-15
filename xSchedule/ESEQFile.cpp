#include "ESEQFile.h"
#include <wx/file.h>
#include <wx/filename.h>
#include <log4cpp/Category.hh>
#include "../xLights/UtilFunctions.h"

ESEQFile::ESEQFile()
{
    _offset = 0;
    _channelsPerFrame = 0;
    _filename = "";
    _frames = 0;
    _fh = nullptr;
    _frameBuffer = nullptr;
    _frame0Offset = 0;
    _ok = false;
}

ESEQFile::ESEQFile(const std::string& filename)
{
    _offset = 0;
    _channelsPerFrame = 0;
    _filename = FixFile("", filename);
    _frames = 0;
    _fh = nullptr;
    _frameBuffer = nullptr;
    _frame0Offset = 0;
    _ok = true;
    Load(_filename);
}

ESEQFile::~ESEQFile()
{
    Close();
}


void ESEQFile::Close()
{
    if (_fh != nullptr)
    {
        _fh->Close();
        delete _fh;
        _fh = nullptr;
    }

    if (_frameBuffer != nullptr)
    {
        free(_frameBuffer);
        _frameBuffer = nullptr;
    }

    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.info("ESEQ file %s closed.", (const char *)_filename.c_str());

    _ok = false;
}

int ESEQFile::ReadInt16(wxFile* fh)
{
    wxByte lsb;
    fh->Read(&lsb, sizeof(lsb));
    wxByte msb;
    fh->Read(&msb, sizeof(msb));
    return (((int)msb) << 8) + lsb;
}

int ESEQFile::ReadInt32(wxFile* fh)
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

std::list<std::string> ESEQFile::GetBlendModes()
{
    std::list<std::string> res;

    res.push_back("Overwrite");
    res.push_back("Overwrite if black");
    res.push_back("Mask out if not black");
    res.push_back("Mask out if black");
    res.push_back("Average");
    res.push_back("Maximum");

    return res;
}

void ESEQFile::Load(const std::string& filename)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    Close();

    _filename = FixFile("", filename);
    _fh = new wxFile(_filename);

    if (_fh->IsOpened())
    {
        char tag[5];
        memset(tag, 0x00, sizeof(tag));
        _fh->Read(tag, sizeof(tag)-1);
        if (std::string(tag) == "ESEQ")
        {
            _frame0Offset = 20;
            ReadInt32(_fh); // models count ... should be 1
            _channelsPerFrame = ReadInt32(_fh);
            _offset = ReadInt32(_fh);
            ReadInt32(_fh); // model size
            //_frames = ReadInt32(_fh);
            _frameBuffer = (wxByte*)malloc(_channelsPerFrame);

            wxFileName fn(_filename);
            _frames = (size_t)(fn.GetSize().ToULong() - _frame0Offset) / _channelsPerFrame;

            logger_base.info("ESEQ file %s opened.", (const char *)_filename.c_str());
        }
        else
        {
            logger_base.error("ESEQ file %s format does not look valid.", (const char *)_filename.c_str());
            Close();
        }
    }
    else
    {
        logger_base.error("ESEQ file %s could not be opened.", (const char *)_filename.c_str());
        Close();
    }
}

void ESEQFile::ReadData(wxByte* buffer, size_t buffersize, size_t frame, APPLYMETHOD applyMethod)
{
    if (frame >= _frames) return; // cant read past end of file

    if (_fh->Tell() != _frame0Offset + _channelsPerFrame * frame)
    {
        // we need to seek to our frame
        _fh->Seek(_frame0Offset + _channelsPerFrame * frame, wxFromStart);
    }

    // read in the frame from disk
    _fh->Read(_frameBuffer, _channelsPerFrame);

    Blend(buffer, buffersize, _frameBuffer, _channelsPerFrame, applyMethod, _offset);
}
