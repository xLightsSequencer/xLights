#include "UtilFunctions.h"
#include <log4cpp/Category.hh>
#include <wx/filename.h>
#include <wx/config.h>

bool IsFileInShowDir(const wxString& showDir, const std::string filename)
{
    wxString fixedFile = FixFile(showDir, filename, true);

    return fixedFile.StartsWith(showDir);
}

wxString FixFile(const wxString& ShowDir, const wxString& file, bool recurse)
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
			if (ShowDir != RememberShowDir)
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

#ifndef __WXMSW__
    wxFileName fnUnix(file, wxPATH_UNIX);
    wxFileName fn3(sd, fnUnix.GetFullName());
    if (fn3.Exists()) {
        return fn3.GetFullPath();
    }
#endif
    wxFileName fnWin(file, wxPATH_WIN);
    wxFileName fn4(sd, fnWin.GetFullName());
    if (fn4.Exists()) {
        return fn4.GetFullPath();
    }


    wxString sdlc = sd;
    sdlc.LowerCase();
    wxString flc = file;
    flc.LowerCase();

    wxString path;
    wxString fname;
    wxString ext;
    wxFileName::SplitPath(sd, &path, &fname, &ext);
    wxArrayString parts = wxSplit(path, '\\', 0);
    if (fname == "")
    {
        // no subdirectory
        return file;
    }

    wxString showfolder = fname;
    wxString sflc = showfolder;
    sflc.LowerCase();
    wxString newLoc = sd;

    bool appending = false;
    for (size_t x = 0; x < fnWin.GetDirs().size(); x++) {
        if (fnWin.GetDirs()[x].Lower() == sflc) {
            appending = true;
        } else if (appending) {
            newLoc += wxFileName::GetPathSeparator();
            newLoc += fnWin.GetDirs()[x];
        }
    }
    if (appending) {
        newLoc += wxFileName::GetPathSeparator();
        newLoc += fnWin.GetFullName();
        if (wxFileExists(newLoc)) {
            return newLoc;
        }
    }
#ifndef __WXMSW__
    newLoc = sd;
    appending = false;
    for (int x = 0; x < fnUnix.GetDirs().size(); x++) {
        if (fnUnix.GetDirs()[x].Lower() == sflc) {
            appending = true;
        } else if (appending) {
            newLoc += wxFileName::GetPathSeparator();
            newLoc += fnWin.GetDirs()[x];
        }
    }
    if (appending) {
        newLoc += wxFileName::GetPathSeparator();
        newLoc += fnUnix.GetFullName();
        if (wxFileExists(newLoc)) {
            return newLoc;
        }
    }
#endif


    if (flc.Contains(sflc))
    {
        int offset = flc.Find(sflc) + showfolder.Length();
        wxString relative = file.SubString(offset, file.Length());

        if (fnWin.GetDirs().size() > 0) {

        }
        wxFileName sdFn =  wxFileName::DirName(sd);

        if (wxFileExists(relative))
        {
            logger_base.debug("File location fixed: " + file + " -> " + relative);
            return relative;
        }
    }
#ifndef __WXMSW__
    if (ShowDir == "" && fnUnix.GetDirCount() > 0) {
        return FixFile(sd + "/" + fnUnix.GetDirs().Last(), file, true);
    }
#endif
    if (ShowDir == "" && fnWin.GetDirCount() > 0) {
        return FixFile(sd + "\\" + fnWin.GetDirs().Last(), file, true);
    }
   	return file;
}

wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir)
{
	int startparamname = parametervalue.Find(paramname);
	int endparamname = parametervalue.find("=", startparamname) - 1;
	int startvalue = endparamname + 2;
	int endvalue = parametervalue.find(",", startvalue) - 1;
	wxString file = parametervalue.SubString(startvalue, endvalue);
	wxString newfile = FixFile(ShowDir, file);
	wxString rc = parametervalue.Left(startvalue) + newfile + parametervalue.Right(parametervalue.Length() - endvalue - 1);
	return rc;
}

std::string UnXmlSafe(const std::string& s)
{
    wxString res = s;

    for (int i = 0; i< 32; ++i)
    {
        wxString ss = wxString::Format("&#%d;", i);
        res.Replace(ss, wxString::Format("%c", i));
    }
    res.Replace("&lt;", "<");
    res.Replace("&gt;", ">");
    res.Replace("&apos;", "'");
    res.Replace("&quot;", "\"");
    res.Replace("&amp;", "&");
    return res.ToStdString();
}

std::string XmlSafe(const std::string& s)
{
    std::string res = "";
    for (auto c = s.begin(); c != s.end(); ++c)
    {
        if ((int)(*c) < 32)
        {
            //res += wxString::Format("&#x%x;", (int)(*c));
            res += wxString::Format("&#%d;", (int)(*c)).ToStdString();
        }
        else if (*c == '&')
        {
            res += "&amp;";
        }
        else if (*c == '<')
        {
            res += "&lt;";
        }
        else if (*c == '>')
        {
            res += "&gt;";
        }
        else if (*c == '\'')
        {
            res += "&apos;";
        }
        else if (*c == '\"')
        {
            res += "&quot;";
        }
        else
        {
            res += (*c);
        }
    }

    return res;
}

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

//returns number of chars at the end that couldn't be decoded
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data)
{
    size_t in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i <4; i++)
            {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
            {
                data.resize(data.size() + 1);
                data[data.size() - 1] = char_array_3[i];
            }
            i = 0;
        }
    }

    if (i && encoded_string[in_] == '=')
    {
        for (j = i; j <4; j++)
        {
            char_array_4[j] = 0;
        }

        for (j = 0; j <4; j++)
        {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++)
        {
            data.resize(data.size() + 1);
            data[data.size() - 1] = char_array_3[j];
        }
    }
    return i;
}

// encodes contents of SeqData in channel order
wxString base64_encode(SequenceData& SeqData)
{
    wxString ret;
    int i = 0;
    int j = 0;

    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (size_t channel = 0; channel < SeqData.NumChannels(); channel++) {
        for (size_t frame = 0; frame < SeqData.NumFrames(); frame++) {
            char_array_3[i++] = SeqData[frame][channel];
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i <4); i++)
                {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
    }

    if (i)
    {
        for (j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
        {
            ret += base64_chars[char_array_4[j]];
        }

        while ((i++ < 3))
        {
            ret += '=';
        }

    }
    return ret;
}

bool IsVersionOlder(const std::string compare, const std::string version)
{
    wxArrayString compare_parts = wxSplit(compare, '.');
    wxArrayString version_parts = wxSplit(version, '.');
    if (wxAtoi(version_parts[0]) < wxAtoi(compare_parts[0])) return true;
    if (wxAtoi(version_parts[0]) > wxAtoi(compare_parts[0])) return false;
    if (wxAtoi(version_parts[1]) < wxAtoi(compare_parts[1])) return true;
    if (wxAtoi(version_parts[1]) > wxAtoi(compare_parts[1])) return false;
    // From 2016 versions only have 2 parts
    if (version_parts.Count() == 2 || compare_parts.Count() == 2)
    {
        if (version_parts.Count() > 2)
        {
            return true;
        }
        return false;
    }
    else
    {
        if (wxAtoi(version_parts[2]) < wxAtoi(compare_parts[2])) return true;
    }
    return false;
}

void SaveWindowPosition(const std::string tag, wxWindow* window)
{
    wxConfigBase* config = wxConfigBase::Get();
    if (window != nullptr)
    {
        wxPoint position = window->GetPosition();
        wxSize size = window->GetSize();
        config->Write(tag, wxString::Format("%d|%d|%d|%d", position.x, position.y, size.GetWidth(), size.GetHeight()));
    }
    else
    {
        config->DeleteEntry(tag);
    }
}

void LoadWindowPosition(const std::string tag, wxSize& size, wxPoint& position)
{
    wxConfigBase* config = wxConfigBase::Get();

    wxString pos = config->Read(tag, "");

    position.x = -1;
    position.y = -1;
    size.SetWidth(-1);
    size.SetHeight(-1);

    wxArrayString ss = wxSplit(pos, '|');

    if (ss.size() == 4)
    {
        position.x = wxAtoi(ss[0]);
        position.y = wxAtoi(ss[1]);
        size.SetWidth(wxAtoi(ss[2]));
        size.SetHeight(wxAtoi(ss[3]));
    }
}

std::string AfterFirstInt(std::string s)
{
    std::string res = "";
    int i = 0;
    while (i < s.size() && (s[i] < '0' || s[i] > '9'))
    {
        i++;
    }

    while (i < s.size() && s[i] >= '0' && s[i] <= '9')
    {
        i++;
    }

    while (i < s.size())
    {
        res += s[i++];
    }

    return res;
}

std::string GetFirstInt(std::string s)
{
    std::string res = "";
    int i = 0;
    while (i < s.size() && (s[i] < '0' || s[i] > '9'))
    {
        i++;
    }

    while (i < s.size() && s[i] >= '0' && s[i] <= '9')
    {
        res += s[i++];
    }

    return res;
}

int NumberAwareStringCompare(std::string a, std::string b)
{
    // first replace all the numbers with zeros and compare
    wxString A = wxString(a);
    A.Replace("0", "");
    A.Replace("1", "");
    A.Replace("2", "");
    A.Replace("3", "");
    A.Replace("4", "");
    A.Replace("5", "");
    A.Replace("6", "");
    A.Replace("7", "");
    A.Replace("8", "");
    A.Replace("9", "");

    wxString B = wxString(b);
    B.Replace("0", "");
    B.Replace("1", "");
    B.Replace("2", "");
    B.Replace("3", "");
    B.Replace("4", "");
    B.Replace("5", "");
    B.Replace("6", "");
    B.Replace("7", "");
    B.Replace("8", "");
    B.Replace("9", "");

    if (A == B)
    {
        while (true)
        {
            std::string ai = GetFirstInt(a);
            int ia = wxAtoi(ai);
            int ib = wxAtoi(GetFirstInt(b));

            if (ia == ib)
            {
                if (ai == "")
                {
                    return 0;
                }
                else
                {
                    a = AfterFirstInt(a);
                    b = AfterFirstInt(b);
                }
            }
            else
            {
                if (ia < ib)
                {
                    return -1;
                }
                return 1;
            }
        }
    }
    else
    {
        return A.compare(B);
    }
}
