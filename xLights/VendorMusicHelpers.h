#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/uri.h>
#include <wx/filename.h>
#include <wx/treebase.h>

class CachedFileDownloader;
class MSLVendor;

class MSLVendorCategory
{
    void ParseCategories(wxXmlNode *n);

public:
    std::string _id;
    std::string _name;
    MSLVendorCategory* _parent;
    std::list<MSLVendorCategory*> _categories;
    MSLVendor* _vendor;

    std::string GetPath() const;
    MSLVendor* GetVendor() const { return _vendor; }
    MSLVendorCategory(wxXmlNode* n, MSLVendorCategory* parent, MSLVendor* vendor);
    virtual ~MSLVendorCategory();
};

class MSLSequenceLyric
{
public:

    enum MSLTYPE { MSL_SEQUENCE, MSL_LYRIC };

    std::list<std::string> _hashes;
    std::list<std::string> _categoryIds;
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
    bool InCategory(std::string category);
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
    std::list<MSLVendorCategory*> _categories;
    int _max;

    std::list<MSLSequenceLyric*> GetSequenceLyricsForCategory(std::string categoryId);
    std::list<MSLSequenceLyric*> GetSequenceLyrics(std::string hash = "");
    std::string PadTitle(std::string t) const;
    std::string GetDescription() const;
    MSLVendor(wxXmlDocument* doc, int max, CachedFileDownloader* cache);
    virtual ~MSLVendor();
    void ParseCategories(wxXmlNode *n);
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

class MSLCategoryTreeItemData : public MSLVendorBaseTreeItemData
{
public:
    MSLCategoryTreeItemData(MSLVendorCategory* category) : MSLVendorBaseTreeItemData("Category"), _category(category) { }

    MSLVendorCategory* GetCategory() const { return _category; }

private:
    MSLVendorCategory * _category;
};

class MSLSequenceLyricTreeItemData : public MSLVendorBaseTreeItemData
{
public:
    MSLSequenceLyricTreeItemData(MSLSequenceLyric* msl) : MSLVendorBaseTreeItemData("SequenceLyric"), _msl(msl) { }

    MSLSequenceLyric* GetSequenceLyric() const { return _msl; }

private:
    MSLSequenceLyric* _msl;
};

