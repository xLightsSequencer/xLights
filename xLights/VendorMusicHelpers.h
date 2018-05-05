#ifndef VENDORMUSICHELPERS_H
#define VENDORMUSICHELPERS_H

#include <wx/xml/xml.h>
#include <wx/uri.h>
#include <wx/filename.h>
#include <wx/treebase.h>

class CachedFileDownloader;
class MSLVendor;

class MSLSequenceLyric
{
public:

    enum MSLTYPE { MSL_SEQUENCE, MSL_LYRIC };

    std::list<std::string> _hashes;
    std::string _title;
    std::string _creator;
    std::string _artist;
    MSLTYPE _type;
    wxURI _webpage;
    wxURI _download;
    wxURI _video;
    wxURI _music;
    std::string _notes;
    MSLVendor* _vendor;
    wxFileName _downloadFile;

    MSLSequenceLyric(wxXmlNode* n, MSLVendor* vendor);
    virtual ~MSLSequenceLyric() { }

    std::string GetType() const;
    std::string PadTitle(std::string t) const;
    std::string GetDescription() const;
    std::string Download(std::string folder);
};

class MSLVendor
{
public:
    std::string _name = "";
    std::string _contact = "";
    std::string _email = "";
    std::string _phone = "";
    wxURI _website;
    wxURI _facebook;
    std::string _twitter = "";
    std::string _notes = "";
    wxFileName _logoFile;
    std::list<MSLSequenceLyric*> _sequencesLyrics;
    int _max;

    std::list<MSLSequenceLyric*> GetSequenceLyrics(std::string hash = "");
    std::string PadTitle(std::string t) const;
    std::string GetDescription() const;
    MSLVendor(wxXmlDocument* doc, int max, CachedFileDownloader* cache);
    virtual ~MSLVendor();
};

class MSLVendorBaseTreeItemData : public wxTreeItemData
{
public:
    MSLVendorBaseTreeItemData(std::string type) : _type(type) { }
    std::string GetType() const { return _type; }

private:
    std::string _type;
};

class MSLVendorTreeItemData : public MSLVendorBaseTreeItemData
{
public:
    MSLVendorTreeItemData(MSLVendor* vendor) : MSLVendorBaseTreeItemData("Vendor"), _vendor(vendor) { }

    MSLVendor* GetVendor() const { return _vendor; }

private:
    MSLVendor* _vendor;
};

class MSLSequenceLyricTreeItemData : public MSLVendorBaseTreeItemData
{
public:
    MSLSequenceLyricTreeItemData(MSLSequenceLyric* msl) : MSLVendorBaseTreeItemData("SequenceLyric"), _msl(msl) { }

    MSLSequenceLyric* GetSequenceLyric() const { return _msl; }

private:
    MSLSequenceLyric* _msl;
};

#endif
