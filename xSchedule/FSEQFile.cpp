#include "FSEQFile.h"
#include <wx/file.h>
#include <log4cpp/Category.hh>
#include <wx/filename.h>

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
    _currentFrame = 0;
    _ok = false;
}

FSEQFile::FSEQFile(const std::string& filename)
{
    _audiofilename = "";
    _channelsPerFrame = 0;
    _filename = filename;
    _frameMS = 0;
    _frames = 0;
    _fh = nullptr;
    _frameBuffer = nullptr;
    _minorVersion = 0;
    _majorVersion = 0;
    _colourEncoding = 0;
    _gamma = 255;
    _frame0Offset = 0;
    _currentFrame = 0;
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

std::list<std::string> FSEQFile::GetBlendModes()
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

    _filename = filename;
    _fh = new wxFile(filename);

    if (_fh->IsOpened())
    {
        char tag[5];
        memset(tag, 0x00, sizeof(tag));
        _fh->Read(tag, sizeof(tag)-1);
        if (std::string(tag) == "PSEQ")
        {
            _frame0Offset = ReadInt16(_fh);
            _fh->Read(&_minorVersion, sizeof(_minorVersion));
            _fh->Read(&_majorVersion, sizeof(_majorVersion));
            ReadInt16(_fh); // fixed header length
            _channelsPerFrame = ReadInt32(_fh);
            _frames = ReadInt32(_fh);
            _frameMS = ReadInt16(_fh);
            ReadInt16(_fh); // universes
            ReadInt16(_fh); // universe size
            _gamma = _fh->Read(&_gamma, sizeof(_gamma));
            _fh->Read(&_colourEncoding, sizeof(_colourEncoding));
            ReadInt16(_fh); // fill
            int mediafilenamelength = ReadInt16(_fh);
            if (mediafilenamelength > 0)
            {
                char* buf = (char*)malloc(mediafilenamelength + 1);
                memset(buf, 0x00, mediafilenamelength + 1);
                ReadInt16(_fh); // mf
                _fh->Read(buf, mediafilenamelength);
                _audiofilename = std::string(buf);
                _audiofilename = FSEQFile::FixFile("", _audiofilename);
                free(buf);
            }
            _currentFrame = 0;
            _frameBuffer = (wxByte*)malloc(_channelsPerFrame);

            logger_base.info("FSEQ file %s opened.", (const char *)filename.c_str());
            _ok = true;
        }
        else
        {
            logger_base.error("FSEQ file %s format does not look valid.", (const char *)filename.c_str());
            Close();
        }
    }
    else
    {
        logger_base.error("FSEQ file %s could not be opened.", (const char *)filename.c_str());
        Close();
    }
}

void FSEQFile::ReadData(wxByte* buffer, size_t buffersize, size_t frame, APPLYMETHOD applyMethod, size_t offset, size_t channels)
{
    if (frame >= _frames) return; // cant read past end of file

    if (frame != _currentFrame)
    {
        // we need to seek to our frame
        _fh->Seek(_frame0Offset + _channelsPerFrame * frame);
    }

    // read in the frame from disk
    _fh->Read(_frameBuffer, _channelsPerFrame);
    _currentFrame = frame + 1;

    size_t bytesToUse = std::min(buffersize, _channelsPerFrame);

    if (channels > 0)
    {
        bytesToUse = std::min(bytesToUse, channels);
    }

    switch(applyMethod)
    {
    case APPLYMETHOD::METHOD_OVERWRITE:
        memcpy(buffer+offset, _frameBuffer+offset, bytesToUse);
        break;
    case APPLYMETHOD::METHOD_AVERAGE:
        for (size_t i = 0; i < bytesToUse; i++)
        {
            *(buffer + i + offset) = ((int)*(buffer + i + offset) + (int)*(_frameBuffer + i + offset)) / 2;
        }
        break;
    case APPLYMETHOD::METHOD_MASK:
        for (size_t i = 0; i < bytesToUse; i++)
        {
            if (*(_frameBuffer + i + offset) > 0)
            {
                *(buffer + i + offset) = 0x00;
            }
        }
        break;
    case APPLYMETHOD::METHOD_UNMASK:
        for (size_t i = 0; i < bytesToUse; i++)
        {
            if (*(_frameBuffer + i + offset) == 0)
            {
                *(buffer + i + offset) = 0x00;
            }
        }
        break;
    case APPLYMETHOD::METHOD_MAX:
        for (size_t i = 0; i < bytesToUse; i++)
        {
            *(buffer + i + offset) = std::max(*(buffer + i + offset), *(_frameBuffer + i + offset));
        }
        break;
    case APPLYMETHOD::METHOD_OVERWRITEIFBLACK:
        for (size_t i = 0; i < bytesToUse; i++)
        {
            if (*(buffer + i + offset) == 0)
            {
                *(buffer + i + offset) = *(_frameBuffer + i + offset);
            }
        }
        break;
    }
}

std::string FSEQFile::FixFile(const std::string& ShowDir, const std::string& file, bool recurse)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    // This is cheating ... saves me from having every call know the showdir as long as an early one passes it in
    static wxString RememberShowDir;
    wxString sd;

        if (ShowDir == "")
        {
            sd = RememberShowDir;
        }
        else
        {
            if (!recurse)
            {
                if (wxString(ShowDir).Lower() != wxString(RememberShowDir).Lower())
                {
                    RememberShowDir = ShowDir;
                }
            }
            sd = ShowDir;
        }

    if (file == "")
    {
        return file;
    }

    if (wxFileExists(file))
    {
        return file;
    }

    logger_base.debug("    Looking for file %s ... not found.", (const char *)file.c_str());

#ifndef __WXMSW__
    wxFileName fnUnix(file, wxPATH_UNIX);
    wxFileName fn3(sd, fnUnix.GetFullName());
    if (fn3.Exists()) {
        return fn3.GetFullPath().ToStdString();
    }
#endif
    wxFileName fnWin(file, wxPATH_WIN);
    wxFileName fn4(sd, fnWin.GetFullName());
    if (fn4.Exists()) {
        return fn4.GetFullPath().ToStdString();
    }

    wxString sdlc = sd;
    sdlc.LowerCase();
    wxString flc = file;
    flc.LowerCase();

    wxString path;
    wxString fname;
    wxString ext;
    wxFileName::SplitPath(sd, &path, &fname, &ext);
    //wxArrayString parts = wxSplit(path, '\\', 0);
    if (fname == "")
    {
        // no subdirectory
        return file;
    }

    wxString showfolder = fname;
    wxString sflc = showfolder;
    sflc.LowerCase();

    if (flc.Contains(sflc))
    {
        wxString f(file);
        int offset = flc.Find(sflc) + showfolder.Length();
        std::string relative = (sd + f.SubString(offset, f.Length())).ToStdString();

        if (wxFileExists(relative))
        {
            logger_base.debug("File location fixed: %s -> %s.", (const char *)f.c_str(), (const char *)relative.c_str());
            return relative;
        }
    }
#ifndef __WXMSW__
    if (ShowDir == "" && fnUnix.GetDirCount() > 0) {
        return FixFile((sd + "/" + fnUnix.GetDirs().Last()).ToStdString() , file, true);
    }
#endif
    if (ShowDir == "" && fnWin.GetDirCount() > 0) {
        return FixFile((sd + "\\" + fnWin.GetDirs().Last()).ToStdString(), file, true);
    }
    return file;
}

