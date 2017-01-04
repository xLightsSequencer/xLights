#ifndef FSEQFILE_H
#define FSEQFILE_H

#include <wx/wx.h>
#include <string>
#include <list>

#ifndef APPLYMETHOD_DEF
#define APPLYMETHOD_DEF
typedef enum {
	METHOD_OVERWRITE,
	METHOD_OVERWRITEIFBLACK,
	METHOD_MASK,
	METHOD_UNMASK,
	METHOD_AVERAGE,
	METHOD_MAX
} APPLYMETHOD;
#endif

class wxFile;

class FSEQFile
{
	std::string _filename;
	std::string _audiofilename;
	size_t _frames;
	size_t _channelsPerFrame;
	size_t _frameMS;
    wxByte _minorVersion;
    wxByte _majorVersion;
	bool _ok;
    wxFile* _fh;
    wxByte _gamma;
    wxByte _colourEncoding;
    size_t _frame0Offset;
    size_t _currentFrame;
    wxByte* _frameBuffer;
	
    public:

        static int ReadInt16(wxFile* fh);
        static int ReadInt32(wxFile* fh);
        static std::list<std::string> GetBlendModes();

		FSEQFile();
		FSEQFile(const std::string& filename);
		void Load(const std::string& filename);
		virtual ~FSEQFile();
		int GetFrameMS() const { return _frameMS; }
		int GetLengthMS() const { return _frames * _frameMS; }
		std::string GetAudioFileName() const { return _audiofilename; }
		int GetLengthFrames() const { return _frames; }
		void ReadData(wxByte* buffer, size_t buffersize, size_t frame, APPLYMETHOD applyMethod);
		bool IsOk() const { return _ok; }
		size_t GetChannels() const { return _channelsPerFrame; }
        void Close();
};

#endif 