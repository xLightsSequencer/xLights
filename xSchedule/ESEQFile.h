#ifndef ESEQFILE_H
#define ESEQFILE_H

#include <string>
#include <wx/file.h>
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

class ESEQFile
{
	std::string _filename;
	size_t _frames;
	size_t _channelsPerFrame;
	size_t _offset;
    wxFile* _fh;
    wxByte* _frameBuffer;
    size_t _frame0Offset;
    size_t _currentFrame;
    bool _ok;

    public:

        static int ReadInt16(wxFile* fh);
        static int ReadInt32(wxFile* fh);
        static std::list<std::string> GetBlendModes();

        ESEQFile();
        ESEQFile(const std::string& filename);
		void Load(const std::string& filename);
		virtual ~ESEQFile();
		int GetLengthFrames() const { return _frames; }
		void ReadData(wxByte* buffer, size_t buffersize, size_t frame, APPLYMETHOD applyMethod);
		bool IsOk() const { return _ok; }
		size_t GetChannels() const { return _channelsPerFrame; }
		size_t GetOffset() const { return _offset; }
        void Close();
};

#endif 