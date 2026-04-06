/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "VendorMusicHelpers.h"
#include "CachedFileDownloader.h"
#include "UtilFunctions.h"
#include "ui/shared/utils/wxUtilities.h"

bool MSLSequenceLyric::InCategory(std::string category)
{
    for (auto it = _categoryIds.begin(); it != _categoryIds.end(); ++it)
    {
        if (*it == category) return true;
    }

    return false;
}

std::string MSLSequenceLyric::GetExt() const
{
	switch (_type) {
    case MSLTYPE::MSL_LYRIC:
        return "xtiming";
    case MSLTYPE::MSL_SEQUENCE:
        return "zip";//download sequence are normlly zip files
    default:
        break;
	}
    return wxFileName(_downloadFile.GetFullName()).GetExt();
}

MSLSequenceLyric::MSLSequenceLyric(pugi::xml_node n, MSLVendor* vendor)
{
	_vendor = vendor;

	for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling())
	{
		std::string nn = Lower(l.name());
		if (nn == "hash")
		{
			_hashes.push_back(l.text().get());
		}
        else if (nn == "categoryid")
        {
            _categoryIds.push_back(l.text().get());
        }
        else if (nn == "creator")
		{
			_creator = l.text().get();
		}
		else if (nn == "title")
		{
			_title = l.text().get();
		}
		else if (nn == "artist")
		{
			_artist = l.text().get();
		}
		else if (nn == "notes")
		{
			_notes = l.text().get();
		}
		else if (nn == "sequence")
		{
			_type = MSLTYPE::MSL_SEQUENCE;
		}
		else if (nn == "lyric")
		{
			_type = MSLTYPE::MSL_LYRIC;
		}
		else if (nn == "weblink")
		{
			_webpage = wxURI(l.text().get());
		}
		else if (nn == "video")
		{
			_video = wxURI(l.text().get());
		}
		else if (nn == "music")
		{
			_music = wxURI(l.text().get());
		}
		else if (nn == "download")
		{
			_download = wxURI(l.text().get());
		}
		else
		{
			wxASSERT(false);
		}
	}
}

std::string MSLSequenceLyric::GetType() const
{
	if (_type == MSLTYPE::MSL_SEQUENCE)
	{
		return "Sequence";
	}
	else
	{
		return "Lyrics";
	}
}

std::string MSLSequenceLyric::PadTitle(std::string t) const
{
	std::string res = t;
	while (res.size() < 18) res += " ";
	return res;
}

std::string MSLSequenceLyric::GetDescription() const
{
	std::string desc;

	if (_title != "")
	{
		desc += PadTitle("Song:") + _title + "\n\n";
	}
	if (_artist != "")
	{
		desc += PadTitle("Artist:") + _artist + "\n\n";
	}
	desc += PadTitle("Type:") + GetType() + "\n";
	if (_creator != "")
	{
		desc += PadTitle("Creator:") + _creator + "\n";
	}
	if (_notes != "")
	{
		desc += "\n" + _notes + "\n";
	}

    std::string find = "\\n";
    std::string replace = "\n";
    for (std::string::size_type i = 0; (i = desc.find(find, i)) != std::string::npos;)
    {
        desc.replace(i, find.length(), replace);
        i += replace.length();
    }

	return desc;
}

std::string MSLSequenceLyric::Download(std::string folder)
{
	_downloadFile.SetPath(folder);
	wxString fn = _download.GetPath();
	if (fn.Contains('/'))
	{
		fn = fn.AfterLast('/');
	}
	_downloadFile.SetName(fn);

	if (!_downloadFile.Exists())
	{
		FileCacheItem::DownloadURL(_download.BuildURI().ToStdString(), _downloadFile.GetFullPath().ToStdString());
	}

	return _downloadFile.GetFullPath().ToStdString();
}

std::list<MSLSequenceLyric*> MSLVendor::GetSequenceLyricsForCategory(std::string categoryId)
{
    std::list<MSLSequenceLyric*> res;

    for (auto it = _sequencesLyrics.begin(); it != _sequencesLyrics.end(); ++it)
    {
        if ((*it)->InCategory(categoryId))
        {
            res.push_back(*it);
        }
    }

    return res;
}

std::list<MSLSequenceLyric*> MSLVendor::GetSequenceLyrics(std::string hash)
{
	std::list<MSLSequenceLyric*> res;

	for (auto it = _sequencesLyrics.begin(); it != _sequencesLyrics.end(); ++it)
	{
		if (hash == "" || std::find((*it)->_hashes.begin(), (*it)->_hashes.end(), hash) != (*it)->_hashes.end())
		{
			res.push_back(*it);
		}
	}

	return res;
}

std::string MSLVendor::PadTitle(std::string t) const
{
	std::string res = t;
	while (res.size() < 9) res += " ";
	return res;
}

std::string MSLVendor::GetDescription() const
{
	std::string desc;

	if (_name != "")
	{
		desc += PadTitle("Name:") + _name + "\n\n";
	}
	if (_contact != "")
	{
		desc += PadTitle("Contact:") + _contact + "\n";
	}
	if (_phone != "")
	{
		desc += PadTitle("Phone:") + _phone + "\n";
	}
	if (_email != "")
	{
		desc += PadTitle("Email:") + _email + "\n";
	}
	if (_twitter != "")
	{
		desc += PadTitle("Twitter:") + _twitter + "\n";
	}
	if (_notes != "")
	{
		desc += "\n" + _notes + "\n";
	}

    std::string find = "\\n";
    std::string replace = "\n";
    for (std::string::size_type i = 0; (i = desc.find(find, i)) != std::string::npos;)
    {
        desc.replace(i, find.length(), replace);
        i += replace.length();
    }

	return desc;
}

bool compare_seq_lyrics(const MSLSequenceLyric* first, const MSLSequenceLyric* second)
{
	return Lower(first->_title) < Lower(second->_title);
}

MSLVendor::MSLVendor(pugi::xml_document& doc, int max, CachedFileDownloader* cache)
{
	_max = max;

	pugi::xml_node root = doc.document_element();
	if (root)
	{
		std::string nn = Lower(root.name());
		if (nn == "musicinventory")
		{
			for (pugi::xml_node e = root.first_child(); e; e = e.next_sibling())
			{
				nn = Lower(e.name());
				if (nn == "vendor")
				{
					for (pugi::xml_node v = e.first_child(); v; v = v.next_sibling())
					{
						nn = Lower(v.name());
						if (nn == "name")
						{
							_name = v.text().get();
						}
                        else if (nn == "contact")
						{
							_contact = v.text().get();
						}
						else if (nn == "email")
						{
							_email = v.text().get();
						}
						else if (nn == "phone")
						{
							_phone = v.text().get();
						}
						else if (nn == "website")
						{
							_website = wxURI(v.text().get());
						}
						else if (nn == "facebook")
						{
							_facebook = wxURI(v.text().get());
						}
						else if (nn == "twitter")
						{
							_twitter = v.text().get();
						}
						else if (nn == "notes")
						{
							_notes = v.text().get();
						}
						else if (nn == "logolink")
						{
							 std::string logoUrl = v.text().get();
							 _logoFile = wxFileName(cache->GetFile(logoUrl, CACHEFOR::CACHETIME_LONG));
						}
						else
						{
							wxASSERT(false);
						}
					}
				}
                else if (nn == "categories")
                {
                    ParseCategories(e);
                }
                else if (nn == "music")
				{
					int items = 0;
					for (pugi::xml_node m = e.first_child(); m; m = m.next_sibling())
					{
						nn = Lower(m.name());
						if (nn == "song")
						{
							items++;
							if (max < 1 || items < _max)
							{
								_sequencesLyrics.push_back(new MSLSequenceLyric(m, this));
							}
						}
					}
				}
				else
				{
					wxASSERT(false);
				}
			}
		}
	}

	_sequencesLyrics.sort(compare_seq_lyrics);
}

MSLVendor::~MSLVendor()
{
    for (auto it = _categories.begin(); it != _categories.end(); ++it)
    {
        delete *it;
    }

    for (auto it = _sequencesLyrics.begin(); it != _sequencesLyrics.end(); ++it)
	{
		delete *it;
	}
}

void MSLVendor::ParseCategories(pugi::xml_node n)
{
    for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling())
    {
        std::string nn = Lower(l.name());
        if (nn == "category")
        {
            _categories.push_back(new MSLVendorCategory(l, nullptr, this));
        }
    }
}

void MSLVendorCategory::ParseCategories(pugi::xml_node n)
{
    for (pugi::xml_node l = n.first_child(); l; l = l.next_sibling())
    {
        std::string nn = Lower(l.name());
        if (nn == "category")
        {
            _categories.push_back(new MSLVendorCategory(l, this, _vendor));
        }
    }
}

std::string MSLVendorCategory::GetPath() const
{
    if (_parent != nullptr)
    {
        return _parent->GetPath() + "/" + _name;
    }
    else
    {
        return _name;
    }
}

MSLVendorCategory::MSLVendorCategory(pugi::xml_node n, MSLVendorCategory* parent, MSLVendor* vendor)
{
    _vendor = vendor;
    _parent = parent;
    for (pugi::xml_node e = n.first_child(); e; e = e.next_sibling())
    {
        std::string nn = Lower(e.name());
        if (nn == "id")
        {
            _id = e.text().get();
        }
        else if (nn == "name")
        {
            _name = e.text().get();
        }
        else if (nn == "categories")
        {
            ParseCategories(e);
        }
        else
        {
            wxASSERT(false);
        }
    }
}
  
MSLVendorCategory::~MSLVendorCategory()
{
    for (auto it = _categories.begin(); it != _categories.end(); ++it)
    {
        delete *it;
    }
}
