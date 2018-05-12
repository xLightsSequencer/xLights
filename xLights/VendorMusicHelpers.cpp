#include "VendorMusicHelpers.h"
#include "CachedFileDownloader.h"

bool MSLSequenceLyric::InCategory(std::string category)
{
    for (auto it = _categoryIds.begin(); it != _categoryIds.end(); ++it)
    {
        if (*it == category) return true;
    }

    return false;
}

MSLSequenceLyric::MSLSequenceLyric(wxXmlNode* n, MSLVendor* vendor)
{
	_vendor = vendor;

	for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext())
	{
		wxString nn = l->GetName().Lower().ToStdString();
		if (nn == "hash")
		{
			_hashes.push_back(l->GetNodeContent().ToStdString());
		}
        else if (nn == "categoryid")
        {
            _categoryIds.push_back(l->GetNodeContent().ToStdString());
        }
        else if (nn == "creator")
		{
			_creator = l->GetNodeContent().ToStdString();
		}
		else if (nn == "title")
		{
			_title = l->GetNodeContent().ToStdString();
		}
		else if (nn == "artist")
		{
			_artist = l->GetNodeContent().ToStdString();
		}
		else if (nn == "notes")
		{
			_notes = l->GetNodeContent().ToStdString();
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
			_webpage = wxURI(l->GetNodeContent());
		}
		else if (nn == "video")
		{
			_video = wxURI(l->GetNodeContent());
		}
		else if (nn == "music")
		{
			_music = wxURI(l->GetNodeContent());
		}
		else if (nn == "download")
		{
			_download = wxURI(l->GetNodeContent());
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
		FileCacheItem::DownloadURL(_download, _downloadFile);
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

	return desc;
}

MSLVendor::MSLVendor(wxXmlDocument* doc, int max, CachedFileDownloader* cache)
{
	_max = max;

	if (doc->IsOk())
	{
		wxXmlNode* root = doc->GetRoot();
		wxString nn = root->GetName().Lower();
		if (nn == "musicinventory")
		{
			for (wxXmlNode* e = root->GetChildren(); e != nullptr; e = e->GetNext())
			{
				nn = e->GetName().Lower();
				if (nn == "vendor")
				{
					for (wxXmlNode* v = e->GetChildren(); v != nullptr; v = v->GetNext())
					{
						nn = v->GetName().Lower();
						if (nn == "name")
						{
							_name = v->GetNodeContent().ToStdString();
						}
                        else if (nn == "contact")
						{
							_contact = v->GetNodeContent().ToStdString();
						}
						else if (nn == "email")
						{
							_email = v->GetNodeContent().ToStdString();
						}
						else if (nn == "phone")
						{
							_phone = v->GetNodeContent().ToStdString();
						}
						else if (nn == "website")
						{
							_website = wxURI(v->GetNodeContent().ToStdString());
						}
						else if (nn == "facebook")
						{
							_facebook = wxURI(v->GetNodeContent().ToStdString());
						}
						else if (nn == "twitter")
						{
							_twitter = v->GetNodeContent().ToStdString();
						}
						else if (nn == "notes")
						{
							_notes = v->GetNodeContent().ToStdString();
						}
						else if (nn == "logolink")
						{
							 wxURI logo(v->GetNodeContent().ToStdString());
							 _logoFile = wxFileName(cache->GetFile(logo, CACHEFOR::CACHETIME_LONG));
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
					for (wxXmlNode* m = e->GetChildren(); m != nullptr; m = m->GetNext())
					{
						nn = m->GetName().Lower();
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

void MSLVendor::ParseCategories(wxXmlNode* n)
{
    for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext())
    {
        wxString nn = l->GetName().Lower().ToStdString();
        if (nn == "category")
        {
            _categories.push_back(new MSLVendorCategory(l, nullptr, this));
        }
    }
}

void MSLVendorCategory::ParseCategories(wxXmlNode *n)
{
    for (wxXmlNode* l = n->GetChildren(); l != nullptr; l = l->GetNext())
    {
        wxString nn = l->GetName().Lower().ToStdString();
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

MSLVendorCategory::MSLVendorCategory(wxXmlNode* n, MSLVendorCategory* parent, MSLVendor* vendor)
{
    _vendor = vendor;
    _parent = parent;
    for (wxXmlNode* e = n->GetChildren(); e != nullptr; e = e->GetNext())
    {
        wxString nn = e->GetName().Lower();
        if (nn == "id")
        {
            _id = e->GetNodeContent().ToStdString();
        }
        else if (nn == "name")
        {
            _name = e->GetNodeContent().ToStdString();
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
