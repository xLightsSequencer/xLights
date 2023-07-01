#ifndef MODELREMAP_H
#define MODELREMAP_H

//(*Headers(ModelRemap)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/filepicker.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <wx/xml/xml.h>

#include <map>

class RemapModelProperties
{
    std::string _filename;
    std::string _message;
    bool _ok = false;
    wxXmlDocument _xmodel;
    std::vector<uint32_t> _data;
    uint32_t _faces = 0;
    uint32_t _states = 0;
    uint32_t _submodels = 0;
    uint32_t _w = 0;
    uint32_t _h = 0;
    uint32_t _d = 0;

    void RemapNodes(wxXmlNode* n, const std::string& attr, const std::map<uint32_t, uint32_t>& mapping);
	void ParseData(const std::string& data);
    uint32_t GetWidth() const
    {
        return _w;
    }
    uint32_t GetHeight() const
    {
        return _h;
    }
    uint32_t GetDepth() const
    {
        return _d;
    }
    const std::vector<uint32_t>& GetData() const
    {
        return _data;
    }
    wxXmlNode* GetXml()
    {
        return _xmodel.GetRoot();
    }

public:
    RemapModelProperties()
    {}
    virtual ~RemapModelProperties()
    {}
    void Load(const std::string& filename);

    std::string Message() const
    {
        return _message;
    }
    bool IsOk() const
    {
        return _ok;
    }
    void Compare(RemapModelProperties& original);
    void Remap(RemapModelProperties& original);
    void Save(wxWindow* parent);
    bool ContainsFSSM()
    {
        // if we are asking and it contains none then it really isnt ok.
        if (_faces + _states + _submodels == 0) {
            _ok = false;
            return false;
        }

        return true;
    }
};

class ModelRemap: public wxDialog
{
    RemapModelProperties originalProperties;
    RemapModelProperties newWiringProperties;

	public:

		ModelRemap(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ModelRemap();
        void ValidateWindow();

		//(*Declarations(ModelRemap)
		wxButton* Button_Generate;
		wxFilePickerCtrl* FilePickerCtrl_NewWiring;
		wxFilePickerCtrl* FilePickerCtrl_Original;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_Messages;
		//*)

	protected:

		//(*Identifiers(ModelRemap)
		static const long ID_STATICTEXT1;
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL1;
		static const long ID_STATICTEXT3;
		static const long ID_FILEPICKERCTRL2;
		static const long ID_TEXTCTRL1;
		static const long ID_BUTTON1;
		//*)

	private:

		//(*Handlers(ModelRemap)
		void OnButton_GenerateClick(wxCommandEvent& event);
		void OnFilePickerCtrl_OriginalFileChanged(wxFileDirPickerEvent& event);
		void OnFilePickerCtrl_NewWiringFileChanged(wxFileDirPickerEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
