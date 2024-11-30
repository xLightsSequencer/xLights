#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Need to do these manually due to issues with wxSmith and wxTreeListCtrl
#include <wx/treelist.h>
#include <wx/treectrl.h>
#include <wx/filename.h>

//(*Headers(PixelTestDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
#include <wx/timer.h>
//*)

#include <glm/glm.hpp>

#include <list>
#include <string>

#include "xLightsTimer.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"
#include "SequenceData.h"

class ModelGroup;
typedef SequenceData SeqDataType;
class xLightsFrame;

class ChannelTracker
{
    long _changeCount;
    std::list<wxLongLong> _ranges;
    long _lastReturnedChannel;
    #define NORANGE (wxLongLong)0xFFFFFFFFFFFFFFF
    wxLongLong _lastReturnedRange;

    wxLongLong SetStart(wxLongLong value, long start) const { return (value & 0xFFFFFFFF) + (((wxLongLong)start) << 32); }
    wxLongLong SetEnd(wxLongLong value, long end) const { return (value & 0xFFFFFFFF00000000) + end; }
    wxLongLong SetBoth(long start, long end) const { return (((wxLongLong)start) << 32) + end; }

    void ClearLast()
    {
        _lastReturnedChannel = -1;
        _lastReturnedRange = NORANGE;
    }


public:

    ChannelTracker()
    {
        _changeCount = 0;
        ClearLast();
    }

    virtual ~ChannelTracker() {}

    static long GetStart(wxLongLong value) { return (value >> 32).ToLong(); }
    static long GetEnd(wxLongLong value) { return (value & 0xFFFFFFFF).ToLong(); }
    void FixOverlaps();
    void Dump();
    void AddRange(long start, long end);
    void RemoveRange(long start, long end);
    long GetChangeCount() const { return _changeCount; }
    void Clear()
    {
        _changeCount++;
        _ranges.clear();
    }
    bool IsChannelOn(long ch) const
    {
        for (const auto& it : _ranges)
        {
            if (ch >= GetStart(it) && ch <= GetEnd(it)) return true;
        }

        return false;
    }

    bool AreAnyIncluded(long start, long end)
    {
        auto it = _ranges.begin();

        while (it != _ranges.end())
        {
            long s = GetStart(*it);
            long e = GetEnd(*it);

            if ((s >= start && s <= end) || (e >= start && e <= end)) return true;

            ++it;
        }

        return false;
    }

    long GetFirst()
    {
        if (_ranges.size() == 0) return -1;

        _lastReturnedChannel = GetStart(_ranges.front());

        return _lastReturnedChannel;
    }

    long GetNext()
    {
        // Assumes ranges are sorted
        if (_lastReturnedChannel == -1) return -1;

        for (const auto& it : _ranges)
        {
            long s = GetStart(it);
            long e = GetEnd(it);

            _lastReturnedChannel++;

            if (s <= _lastReturnedChannel && e >= _lastReturnedChannel) return _lastReturnedChannel;

            if (s > _lastReturnedChannel)
            {
                _lastReturnedChannel = s;
                return _lastReturnedChannel;
            }
            _lastReturnedChannel--;
        }

        return -1;
    }

    long GetChannelAfter(long ch)
    {
        for (const auto& it : _ranges)
        {
            long s = GetStart(it);
            long e = GetEnd(it);

            ch++;
            if (s <= ch && e >= ch) return ch;
            if (s > ch)
            {
                return s;
            }
        }

        return -1;
    }

    void GetFirstRange(long& start, long& end)
    {
        if (_ranges.size() == 0)
        {
            start = -1;
            end = -1;
            return;
        }

        _lastReturnedRange = _ranges.front();
        start = GetStart(_lastReturnedRange);
        end = GetEnd(_lastReturnedRange);
    }

    void GetNextRange(long& start, long& end)
    {
        if (_lastReturnedRange == NORANGE) 
        {
            start = -1;
            end = -1;
            return;
        }

        for (auto it = _ranges.begin(); it != _ranges.end(); ++it)
        {
            if (*it == _lastReturnedRange)
            {
                ++it;
                if (it == _ranges.end())
                {
                    _lastReturnedRange = NORANGE;
                    start = -1;
                    end = -1;
                    return;
                }
                else
                {
                    _lastReturnedRange = *it;
                    start = GetStart(*it);
                    end = GetEnd(*it);
                    return;
                }
            }
        }

        _lastReturnedRange = NORANGE;
        start = -1;
        end = -1;
    }
};

class ModelTestItem;
class ModelGroupTestItem;
class ModelPreview;

class PixelTestDialog: public wxDialog
{
	enum class TestFunctions
	{
		OFF,
		CHASE,
		CHASE2,
		CHASE3,
		CHASE4,
		DIM,
		TWINKLE,
		SHIMMER,
		RGBW,
        PortCycle,
        ColorBlocks
	};

	public:

		PixelTestDialog(xLightsFrame* parent, OutputManager* outputManager, wxFileName networkFile, ModelManager* modelManager, wxWindowID id=wxID_ANY);
		virtual ~PixelTestDialog();
		wxTreeListCtrl* TreeListCtrl_Outputs = nullptr;
		wxTreeListCtrl* TreeListCtrl_ModelGroups = nullptr;
		wxTreeListCtrl* TreeListCtrl_Models = nullptr;
        wxTreeListCtrl* TreeListCtrl_Controllers = nullptr;
        wxFileName _networkFile;
		ModelManager* _modelManager = nullptr;
		bool _cascading = false;
        ModelTestItem* _lastModel = nullptr;
        std::list<ModelTestItem*> _models;
        ChannelTracker _channelTracker;
        ModelPreview* _modelPreview = nullptr;

        int _twinkleRatio = 0;
		int _chaseGrouping = 0;
		bool _checkChannelList = false;
		wxDateTime _starttime;
		SeqDataType _seqData;
        wxTreeListItem _rcItem;
        wxTreeListCtrl* _rcTree = nullptr;
        bool m_creating_bound_rect = false;
        int m_bound_start_x = 0;
        int m_bound_start_y = 0;
        int m_bound_end_x = 0;
        int m_bound_end_y = 0;
        int mPointSize = 1;

		//(*Declarations(PixelTestDialog)
		wxButton* Button_Load;
		wxButton* Button_Save;
		wxCheckBox* CheckBox_OutputToLights;
		wxCheckBox* CheckBox_SuppressUnusedOutputs;
		wxCheckBox* CheckBox_Tag50th;
		wxChoice* Choice_VisualModel;
		wxFlexGridSizer* FlexGridSizer_Controllers;
		wxFlexGridSizer* FlexGridSizer_ModelGroups;
		wxFlexGridSizer* FlexGridSizer_Models;
		wxFlexGridSizer* FlexGridSizer_Outputs;
		wxFlexGridSizer* FlexGridSizer_VisualModelSizer;
		wxNotebook* Notebook1;
		wxNotebook* Notebook2;
		wxPanel* Panel1;
		wxPanel* Panel2;
		wxPanel* PanelController;
		wxPanel* PanelRGB;
		wxPanel* PanelRGBCycle;
		wxPanel* PanelStandard;
		wxPanel* Panel_Controllers;
		wxPanel* Panel_Model;
		wxPanel* Panel_ModelGroups;
		wxPanel* Panel_Models;
		wxPanel* Panel_Outputs;
		wxPanel* Panel_VisualModel;
		wxRadioButton* RadioButton_Controller_CyclePorts;
		wxRadioButton* RadioButton_Controller_Off;
		wxRadioButton* RadioButton_Controller_PixelCount;
		wxRadioButton* RadioButton_RGBCycle_ABC;
		wxRadioButton* RadioButton_RGBCycle_ABCAll;
		wxRadioButton* RadioButton_RGBCycle_ABCAllNone;
		wxRadioButton* RadioButton_RGBCycle_MixedColors;
		wxRadioButton* RadioButton_RGBCycle_Off;
		wxRadioButton* RadioButton_RGBCycle_RGBW;
		wxRadioButton* RadioButton_RGB_Alternate;
		wxRadioButton* RadioButton_RGB_Background;
		wxRadioButton* RadioButton_RGB_Chase13;
		wxRadioButton* RadioButton_RGB_Chase14;
		wxRadioButton* RadioButton_RGB_Chase15;
		wxRadioButton* RadioButton_RGB_Chase;
		wxRadioButton* RadioButton_RGB_Off;
		wxRadioButton* RadioButton_RGB_Shimmer;
		wxRadioButton* RadioButton_RGB_Twinkle10;
		wxRadioButton* RadioButton_RGB_Twinkle25;
		wxRadioButton* RadioButton_RGB_Twinkle50;
		wxRadioButton* RadioButton_RGB_Twinkle5;
		wxRadioButton* RadioButton_Standard_Alternate;
		wxRadioButton* RadioButton_Standard_Background;
		wxRadioButton* RadioButton_Standard_Chase13;
		wxRadioButton* RadioButton_Standard_Chase14;
		wxRadioButton* RadioButton_Standard_Chase15;
		wxRadioButton* RadioButton_Standard_Chase;
		wxRadioButton* RadioButton_Standard_Off;
		wxRadioButton* RadioButton_Standard_Shimmer;
		wxRadioButton* RadioButton_Standard_Twinkle10;
		wxRadioButton* RadioButton_Standard_Twinkle25;
		wxRadioButton* RadioButton_Standard_Twinkle50;
		wxRadioButton* RadioButton_Standard_Twinkle5;
		wxSlider* Slider_RGB_BG_B;
		wxSlider* Slider_RGB_BG_G;
		wxSlider* Slider_RGB_BG_R;
		wxSlider* Slider_RGB_H_B;
		wxSlider* Slider_RGB_H_G;
		wxSlider* Slider_RGB_H_R;
		wxSlider* Slider_Speed;
		wxSlider* Slider_Standard_Background;
		wxSlider* Slider_Standard_Highlight;
		wxSplitterWindow* SplitterWindow1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText8;
		wxStaticText* StatusBar1;
		xLightsTimer Timer1;
		//*)

	protected:

		OutputManager* _outputManager = nullptr;
		static const long ID_TREELISTCTRL_Outputs;
		static const long ID_TREELISTCTRL_ModelGroups;
		static const long ID_TREELISTCTRL_Models;
        static const long ID_TREELISTCTRL_Controllers;
        static const long ID_MNU_TEST_SELECTALL;
        static const long ID_MNU_TEST_DESELECTALL;
        static const long ID_MNU_SELECTHIGH;
		static const long ID_MNU_DESELECTHIGH;
        static const long ID_MNU_TEST_SELECTN;
        static const long ID_MNU_TEST_DESELECTN;
        static const long ID_MNU_TEST_NUMBER;

		//(*Identifiers(PixelTestDialog)
		static const long ID_BUTTON_Load;
		static const long ID_BUTTON_Save;
		static const long ID_PANEL3;
		static const long ID_PANEL6;
		static const long ID_PANEL7;
		static const long ID_STATICTEXT8;
		static const long ID_CHOICE1;
		static const long ID_PANEL11;
		static const long ID_PANEL5;
		static const long ID_PANEL4;
		static const long ID_NOTEBOOK1;
		static const long ID_PANEL1;
		static const long ID_CHECKBOX_OutputToLights;
		static const long ID_CHECKBOX1;
		static const long ID_STATICTEXT2;
		static const long ID_RADIOBUTTON_Standard_Off;
		static const long ID_RADIOBUTTON_Standard_Chase;
		static const long ID_RADIOBUTTON_Standard_Chase13;
		static const long ID_RADIOBUTTON_Standard_Chase14;
		static const long ID_RADIOBUTTON_Standard_Chase15;
		static const long ID_RADIOBUTTON_Standard_Alternate;
		static const long ID_RADIOBUTTON_Standard_Twinke5;
		static const long ID_RADIOBUTTON_Standard_Twinkle10;
		static const long ID_RADIOBUTTON_Standard_Twinkle25;
		static const long ID_RADIOBUTTON_Standard_Twinkle50;
		static const long ID_RADIOBUTTON_Standard_Shimmer;
		static const long ID_RADIOBUTTON_Standard_Background;
		static const long ID_STATICTEXT3;
		static const long ID_SLIDER_Standard_Background;
		static const long ID_STATICTEXT4;
		static const long ID_SLIDER_Standard_Highlight;
		static const long ID_PANEL8;
		static const long ID_STATICTEXT5;
		static const long ID_RADIOBUTTON_RGB_Off;
		static const long ID_RADIOBUTTON_RGB_Chase;
		static const long ID_RADIOBUTTON_RGB_Chase13;
		static const long ID_RADIOBUTTON_RGB_Chase14;
		static const long ID_RADIOBUTTON_RGB_Chase15;
		static const long ID_RADIOBUTTON_RGB_Alternate;
		static const long ID_RADIOBUTTON_RGB_Twinkle5;
		static const long ID_RADIOBUTTON_RGB_Twinkle10;
		static const long ID_RADIOBUTTON_RGB_Twinkle25;
		static const long ID_RADIOBUTTON_RGB_Twinkle50;
		static const long ID_RADIOBUTTON_RGB_Shimmer;
		static const long ID_RADIOBUTTON_RGB_Background;
		static const long ID_SLIDER1;
		static const long ID_SLIDER2;
		static const long ID_SLIDER3;
		static const long ID_SLIDER4;
		static const long ID_SLIDER5;
		static const long ID_SLIDER6;
		static const long ID_PANEL9;
		static const long ID_STATICTEXT6;
		static const long ID_RADIOBUTTON_RGBCycle_Off;
		static const long ID_RADIOBUTTON_RGBCycle_ABC;
		static const long ID_RADIOBUTTON_RGBCycle_ABCAll;
		static const long ID_RADIOBUTTON_RGBCycle_ABCAllNone;
		static const long ID_RADIOBUTTON_RGBCycle_MixedColors;
		static const long ID_RADIOBUTTON_RGBCycle_RGBW;
		static const long ID_CHECKBOX2;
		static const long ID_PANEL10;
		static const long ID_STATICTEXT9;
		static const long ID_RADIOBUTTON_CONTROLLER_OFF;
		static const long ID_RADIOBUTTON_CONTROLLER_CYCLEPORTS;
		static const long ID_RADIOBUTTON_CONTROLLER_PIXELCOUNT;
		static const long ID_PANEL12;
		static const long ID_NOTEBOOK2;
		static const long ID_STATICTEXT1;
		static const long ID_SLIDER_Speed;
		static const long ID_PANEL2;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_STATICTEXT7;
		static const long ID_TIMER1;
		//*)

	private:

		//(*Handlers(PixelTestDialog)
		void OnButton_LoadClick(wxCommandEvent& event);
		void OnButton_SaveClick(wxCommandEvent& event);
		void OnTimer1Trigger(wxTimerEvent& event);
		void OnCheckBox_OutputToLightsClick(wxCommandEvent& event);
		void OnClose(wxCloseEvent& event);
		void OnCheckBox_SuppressUnusedOutputsClick(wxCommandEvent& event);
		void OnNotebook1PageChanged(wxNotebookEvent& event);
		void OnCheckBox_Tag50thClick(wxCommandEvent& event);
		void OnChoice_VisualModelSelect(wxCommandEvent& event);
		//*)

		void OnTreeListCtrlCheckboxtoggled(wxTreeListEvent& event);
        void OnTreeListCtrlItemActivated(wxTreeListEvent& event);
        void OnContextMenu(wxTreeListEvent& event);
        void OnListPopup(wxCommandEvent& event);
        void OnTreeListCtrlItemSelected(wxTreeListEvent& event);
        void OnTreeListCtrlItemExpanding(wxTreeListEvent& event);

		void PopulateOutputTree(OutputManager* outputManager);
		void PopulateModelGroupTree(ModelManager* modelManager);
		void PopulateModelTree(ModelManager* modelManager);
        void PopulateVisualModelTree(ModelManager* modelManager);
        void PopulateControllerTree(OutputManager* outputManager, ModelManager* modelManager);
        void SelectVisualModel(const std::string& model);
        void AddChannel(wxTreeListCtrl* tree, wxTreeListItem parent, long absoluteChannel, long relativeChannel, char colour);
        void AddNode(wxTreeListCtrl* tree, wxTreeListItem parent, ModelTestItem* model, long node);
        char GetChannelColour(long ch);
        void AddModelGroup(wxTreeListItem parent, Model* m);
        void AddModelGroup(wxTreeListItem parent, ModelGroupTestItem* m);

		void OnPreviewLeftUp(wxMouseEvent& event);
        void OnPreviewMouseLeave(wxMouseEvent& event);
        void OnPreviewLeftDown(wxMouseEvent& event);
        void OnPreviewLeftDClick(wxMouseEvent& event);
        void OnPreviewMouseMove(wxMouseEvent& event);

		void RenderModel();
        void GetMouseLocation(int x, int y, glm::vec3& ray_origin, glm::vec3& ray_direction);
        void SelectAllInBoundingRect(bool shiftdwn);
        void UpdateVisualModelFromTracker();

        bool AreChannelsAvailable(Model* model);
        bool AreChannelsAvailable(ModelGroup* model);

		void CascadeSelected(wxTreeListCtrl* tree, const wxTreeListItem& item, wxCheckBoxState state);
        void DumpSelected();

		void DestroyTreeControllerData(wxTreeListCtrl* tree, wxTreeListItem& item);
		std::list<std::string> GetModelsOnChannels(int start, int end);
		void Clear(wxTreeListCtrl* tree, wxTreeListItem& item);
		void GetCheckedItems(wxArrayInt& chArray);
		void GetCheckedItems(wxArrayInt& chArray, char col);
		void OnTimer(long curtime);
		void TestButtonsOff();
		void RollUpAll(wxTreeListCtrl* tree, wxTreeListItem start);
		void DeactivateNotClickableModels(wxTreeListCtrl* tree);
        void SetTreeTooltip(wxTreeListCtrl* tree, wxTreeListItem& item);
		wxTreeListItem AddController(wxTreeListItem root, Controller* controller);
		void AddOutput(wxTreeListItem root, Output* output);
		std::string SerialiseSettings();
        void DeserialiseSettings(const std::string& settings);
        TestFunctions GetTestFunction(int notebookSelection);
        void SetCheckBoxItemFromTracker(wxTreeListCtrl* tree, wxTreeListItem item, wxCheckBoxState parentState);
        void SetSuspend(bool suspend);

		DECLARE_EVENT_TABLE()
};

