
#include "xLightsMain.h"

// Process Test Panel Events

void xLightsFrame::SetTestCheckboxes(bool NewValue)
{
    int ChCount = CheckListBoxTestChannels->GetCount();
    for (int i=0; i < ChCount; i++)
    {
        CheckListBoxTestChannels->Check(i,NewValue);
    }
    CheckChannelList = true;
}

void xLightsFrame::OnButtonTestSelectAllClick(wxCommandEvent& event)
{
    SetTestCheckboxes(true);
}

void xLightsFrame::OnButtonTestClearClick(wxCommandEvent& event)
{
    SetTestCheckboxes(false);
}

void xLightsFrame::OnCheckListBoxTestChannelsToggled(wxCommandEvent& event)
{
    int ChCount = CheckListBoxTestChannels->GetCount();
    int row=event.GetInt();
    bool newval=CheckListBoxTestChannels->IsChecked(row);

    if (CheckListBoxTestChannels->IsSelected(row))
    {
        for (int i=0; i < ChCount; i++)
        {
            if (CheckListBoxTestChannels->IsSelected(i)) CheckListBoxTestChannels->Check(i,newval);
        }
    }

    CheckChannelList = true;
}

void xLightsFrame::GetCheckedItems(wxArrayInt& chArray)
{
    chArray.Clear();
    int maxch = CheckListBoxTestChannels->GetCount();
    for (int ch=0; ch < maxch; ch++)
    {
        if (CheckListBoxTestChannels->IsChecked(ch))
        {
            chArray.Add(ch);
        }
    }
}

void xLightsFrame::TestButtonsOff()
{
    RadioButtonOff->SetValue(true);
    RadioButtonChase->SetValue(false);
    RadioButtonChase3->SetValue(false);
    RadioButtonChase4->SetValue(false);
    RadioButtonChase5->SetValue(false);
    RadioButtonAlt->SetValue(false);
    RadioButtonTwinkle05->SetValue(false);
    RadioButtonTwinkle10->SetValue(false);
    RadioButtonTwinkle25->SetValue(false);
    RadioButtonTwinkle50->SetValue(false);
    RadioButtonShimmer->SetValue(false);
    RadioButtonDim->SetValue(false);

    RadioButtonRgbChaseOff->SetValue(true);
    RadioButtonRgbChase->SetValue(false);
    RadioButtonRgbChase3->SetValue(false);
    RadioButtonRgbChase4->SetValue(false);
    RadioButtonRgbChase5->SetValue(false);
    RadioButtonRgbAlt->SetValue(false);
    RadioButtonRgbTwinkle05->SetValue(false);
    RadioButtonRgbTwinkle10->SetValue(false);
    RadioButtonRgbTwinkle25->SetValue(false);
    RadioButtonRgbTwinkle50->SetValue(false);
    RadioButtonRgbShimmer->SetValue(false);
    RadioButtonRgbDim->SetValue(false);

    RadioButtonRgbCycleOff->SetValue(true);
    RadioButtonRgbCycle3->SetValue(false);
    RadioButtonRgbCycle4->SetValue(false);
    RadioButtonRgbCycle5->SetValue(false);
    RadioButtonRgbCycleMixed->SetValue(false);

    TestFunc=OFF;
}

// get list of test config names
void xLightsFrame::GetTestPresetNames(wxArrayString& PresetNames)
{
    wxString name;
    wxXmlNode* root=NetworkXML.GetRoot();
    if (!root) return;
    for( wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "testpreset")
        {
            name=e->GetAttribute("name", "");
            if (!name.IsEmpty()) PresetNames.Add(name);
        }
    }
}

// Allow user to select test configuration located in Network config file
void xLightsFrame::OnButtonTestLoadClick(wxCommandEvent& event)
{
    wxArrayString PresetNames;
    GetTestPresetNames(PresetNames);
    if (PresetNames.Count() == 0)
    {
        wxMessageBox(_("No test configurations found"), _("Error"));
        return;
    }

    // get user selection
    PresetNames.Sort();
    wxSingleChoiceDialog dialog(this, _("Select test configuration"), _("Load Test Settings"), PresetNames);
    if (dialog.ShowModal() != wxID_OK) return;

    // re-find testpreset node, then set channels
    SetTestCheckboxes(false);
    wxString name = dialog.GetStringSelection();
    wxString chidstr;
    long chid;
    long ChCount = CheckListBoxTestChannels->GetCount();
    wxXmlNode* root=NetworkXML.GetRoot();
    for( wxXmlNode* e=root->GetChildren(); e!=NULL; e=e->GetNext() )
    {
        if (e->GetName() == "testpreset" && e->GetAttribute("name", "") == name)
        {
            for( wxXmlNode* c=e->GetChildren(); c!=NULL; c=c->GetNext() )
            {
                if (c->GetName() == "channel" && c->GetAttribute("id", &chidstr) && chidstr.ToLong(&chid) && chid >= 0 && chid < ChCount)
                {
                    CheckListBoxTestChannels->Check(chid,true);
                }
            }
            break;
        }
    }
}

// Save test configuration to Network config file
void xLightsFrame::OnButtonTestSaveClick(wxCommandEvent& event)
{
    wxString name;
    wxXmlNode *channel, *PresetNode;
    wxArrayString PresetNames;
    GetTestPresetNames(PresetNames);
    wxXmlNode* root=NetworkXML.GetRoot();
    wxTextEntryDialog NameDialog(this, _("Enter a name for this test configuration"), _("Save Test Settings"));
    if (NameDialog.ShowModal() != wxID_OK) return;
    name = NameDialog.GetValue();
    name.Trim(true);
    name.Trim(false);
    if (name.IsEmpty())
    {
        wxMessageBox(_("Name cannot be empty"), _("Error"));
    }
    else if (name.Len() > 240)
    {
        wxMessageBox(_("Name is too long"), _("Error"));
    }
    else if (name.Find('"') != wxNOT_FOUND)
    {
        wxMessageBox(_("Name cannot contain quotes"), _("Error"));
    }
    else if (PresetNames.Index(name,false) != wxNOT_FOUND)
    {
        wxMessageBox(_("Name already exists, please enter a unique name"), _("Error"));
    }
    else
    {
        PresetNode = new wxXmlNode( wxXML_ELEMENT_NODE, "testpreset" );
        PresetNode->AddAttribute( "name", name);
        root->AddChild( PresetNode );
        int ChCount = CheckListBoxTestChannels->GetCount();
        for (int c=0; c < ChCount; c++)
        {
            if (CheckListBoxTestChannels->IsChecked(c))
            {
                channel = new wxXmlNode( wxXML_ELEMENT_NODE, "channel" );
                channel->AddAttribute( "id", wxString::Format("%d",c));
                PresetNode->AddChild( channel );
            }
        }

        if (NetworkXML.Save( networkFile.GetFullPath() ))
        {
            UnsavedChanges=false;
        }
        else
        {
            wxMessageBox(_("Unable to save network definition file"), _("Error"));
        }
    }
}

void xLightsFrame::OnRadioButtonOffSelect(wxCommandEvent& event)
{
    TestFunc=OFF;
}

void xLightsFrame::OnRadioButtonChaseSelect(wxCommandEvent& event)
{
    TestFunc=CHASE;
    ChaseGrouping=std::numeric_limits<int>::max();
}

void xLightsFrame::OnRadioButtonChase3Select(wxCommandEvent& event)
{
    TestFunc=CHASE;
    ChaseGrouping=3;
}

void xLightsFrame::OnRadioButtonChase4Select(wxCommandEvent& event)
{
    TestFunc=CHASE;
    ChaseGrouping=4;
}

void xLightsFrame::OnRadioButtonChase5Select(wxCommandEvent& event)
{
    TestFunc=CHASE;
    ChaseGrouping=5;
}

void xLightsFrame::OnRadioButtonAltSelect(wxCommandEvent& event)
{
    TestFunc=CHASE;
    ChaseGrouping=2;
}

void xLightsFrame::OnRadioButtonTwinkle05Select(wxCommandEvent& event)
{
    TestFunc=TWINKLE;
    TwinkleRatio=20;
}

void xLightsFrame::OnRadioButtonTwinkle10Select(wxCommandEvent& event)
{
    TestFunc=TWINKLE;
    TwinkleRatio=10;
}

void xLightsFrame::OnRadioButtonTwinkle25Select(wxCommandEvent& event)
{
    TestFunc=TWINKLE;
    TwinkleRatio=4;
}

void xLightsFrame::OnRadioButtonTwinkle50Select(wxCommandEvent& event)
{
    TestFunc=TWINKLE;
    TwinkleRatio=2;
}

void xLightsFrame::OnRadioButtonShimmerSelect(wxCommandEvent& event)
{
    TestFunc=SHIMMER;
}

void xLightsFrame::OnRadioButtonDimSelect(wxCommandEvent& event)
{
    TestFunc=DIM;
}

// called on each Timer tick while Test dialog is open
void xLightsFrame::OnTimerTest(long curtime)
{
    static int LastNotebookSelection = -1;
    static int LastBgIntensity,LastFgIntensity,LastBgColor[3],LastFgColor[3],*ShimColor,ShimIntensity;
    static int LastSequenceSpeed;
    static int LastAutomatedTest;
    static long NextSequenceStart = -1;
    static TestFunctions LastFunc = OFF;
    static unsigned int interval, rgbCycle, TestSeqIdx;
    static wxArrayInt chArray,TwinkleState;
    static float frequency;
    int v,BgIntensity,FgIntensity,BgColor[3],FgColor[3];
    unsigned int i;
    bool ColorChange;

    if (!xout) return;
    xout->TimerStart(curtime);
    int NotebookSelection = NotebookTest->GetSelection();
    if (NotebookSelection != LastNotebookSelection)
    {
        LastNotebookSelection = NotebookSelection;
        CheckChannelList = true;
        TestSeqIdx=0;
        TestButtonsOff();
    }
    if (TestFunc != LastFunc)
    {
        LastFunc = TestFunc;
        rgbCycle=0;
        CheckChannelList = true;
        NextSequenceStart = -1;
    }

    if (CheckChannelList)
    {
        // get list of checked channels
        xout->alloff();
        GetCheckedItems(chArray);
        LastSequenceSpeed=-1;
        LastBgIntensity=-1;
        LastFgIntensity=-1;
        LastAutomatedTest=-1;
        for (i=0; i < 3; i++)
        {
            LastBgColor[i] = -1;
            LastFgColor[i] = -1;
        }
        if (!CheckBoxLightOutput->IsChecked())
        {
            StatusBar1->SetStatusText(_("Testing disabled - Output to Lights is not checked"));
        }
        else if (TestFunc == OFF)
        {
            StatusBar1->SetStatusText(_("Testing off"));
        }
        else
        {
            StatusBar1->SetStatusText(wxString::Format(_("Testing %ld channels"),static_cast<long>(chArray.Count())));
        }
        CheckChannelList = false;
    }

    if (TestFunc != OFF && chArray.Count() > 0) switch (NotebookSelection)
        {
        case 0:
            // standard tests
            v=SliderChaseSpeed->GetValue();  // 0-100
            BgIntensity = SliderBgIntensity->GetValue();
            FgIntensity = SliderFgIntensity->GetValue();
            ColorChange = BgIntensity != LastBgIntensity || FgIntensity != LastFgIntensity;
            LastBgIntensity = BgIntensity;
            LastFgIntensity = FgIntensity;
            interval = 1600 - v*15;

            switch (TestFunc)
            {
            case DIM:
                if (ColorChange)
                {
                    for (i=0; i < chArray.Count(); i++)
                    {
                        xout->SetIntensity(chArray[i], BgIntensity);
                    }
                }
                break;

            case TWINKLE:
                if (LastSequenceSpeed < 0)
                {
                    LastSequenceSpeed=0;
                    TwinkleState.Clear();
                    for (i=0; i < chArray.Count(); i++)
                    {
                        TestSeqIdx = static_cast<int>(rand01()*TwinkleRatio);
                        TwinkleState.Add(TestSeqIdx == 0 ? -1 : 1);
                    }
                }
                for (i=0; i < TwinkleState.Count(); i++)
                {
                    if (TwinkleState[i] < -1)
                    {
                        // background
                        TwinkleState[i]++;
                    }
                    else if (TwinkleState[i] > 1)
                    {
                        // highlight
                        TwinkleState[i]--;
                    }
                    else if (TwinkleState[i] == -1)
                    {
                        // was background, now highlight for random period
                        TwinkleState[i]=static_cast<int>(rand01()*interval+100) / XTIMER_INTERVAL;
                        xout->SetIntensity(chArray[i], FgIntensity);
                    }
                    else
                    {
                        // was on, now go to bg color for random period
                        TwinkleState[i]=-static_cast<int>(rand01()*interval+100) / XTIMER_INTERVAL * (TwinkleRatio - 1);
                        xout->SetIntensity(chArray[i], BgIntensity);
                    }
                }
                break;

            case SHIMMER:
                if (ColorChange || curtime >= NextSequenceStart)
                {
                    ShimIntensity = (ShimIntensity == FgIntensity) ? BgIntensity : FgIntensity;
                    for (i=0; i < chArray.Count(); i++)
                    {
                        xout->SetIntensity(chArray[i], ShimIntensity);
                    }
                }
                if (curtime >= NextSequenceStart)
                {
                    NextSequenceStart = curtime + interval/2;
                }
                break;

            case CHASE:
                //StatusBar1->SetStatusText(wxString::Format(_("chase curtime=%ld, NextSequenceStart=%ld"),curtime,NextSequenceStart));
                if (ColorChange || curtime >= NextSequenceStart)
                {
                    for (i=0; i < chArray.Count(); i++)
                    {
                        v = (i % ChaseGrouping) == TestSeqIdx ? FgIntensity : BgIntensity;
                        xout->SetIntensity(chArray[i], v);
                    }
                }
                if (curtime >= NextSequenceStart)
                {
                    NextSequenceStart = curtime + interval;
                    TestSeqIdx = (TestSeqIdx + 1) % ChaseGrouping;
                    if (TestSeqIdx >= chArray.Count()) TestSeqIdx=0;
                }
                StatusBar1->SetStatusText(wxString::Format(_("Testing %ld channels; chase now at ch# %d"),static_cast<long>(chArray.Count()), TestSeqIdx)); //show current ch# -DJ
                break;
            default:
                break;
            }
            break;

        case 1:
            // RGB tests
            v=SliderRgbChaseSpeed->GetValue();  // 0-100
            BgColor[0] = SliderBgColorA->GetValue();
            BgColor[1] = SliderBgColorB->GetValue();
            BgColor[2] = SliderBgColorC->GetValue();
            FgColor[0] = SliderFgColorA->GetValue();
            FgColor[1] = SliderFgColorB->GetValue();
            FgColor[2] = SliderFgColorC->GetValue();

            interval = 1600 - v*15;
            for (ColorChange=false,i=0; i < 3; i++)
            {
                ColorChange |= (BgColor[i] != LastBgColor[i]);
                ColorChange |= (FgColor[i] != LastFgColor[i]);
                LastBgColor[i] = BgColor[i];
                LastFgColor[i] = FgColor[i];
            }
            switch (TestFunc)
            {
            case DIM:
                if (ColorChange)
                {
                    for (i=0; i < chArray.Count(); i++)
                    {
                        xout->SetIntensity(chArray[i], BgColor[i % 3]);
                    }
                }
                break;

            case TWINKLE:
                if (LastSequenceSpeed < 0)
                {
                    LastSequenceSpeed=0;
                    TwinkleState.Clear();
                    for (i=0; i < chArray.Count()-2; i+=3)
                    {
                        TestSeqIdx = static_cast<int>(rand01()*TwinkleRatio);
                        TwinkleState.Add(TestSeqIdx == 0 ? -1 : 1);
                    }
                }
                for (i=0; i < TwinkleState.Count(); i++)
                {
                    if (TwinkleState[i] < -1)
                    {
                        // background
                        TwinkleState[i]++;
                    }
                    else if (TwinkleState[i] > 1)
                    {
                        // highlight
                        TwinkleState[i]--;
                    }
                    else if (TwinkleState[i] == -1)
                    {
                        // was background, now highlight for random period
                        TwinkleState[i]=static_cast<int>(rand01()*interval+100) / XTIMER_INTERVAL;
                        TestSeqIdx = i * 3;
                        xout->SetIntensity(chArray[TestSeqIdx], FgColor[0]);
                        xout->SetIntensity(chArray[TestSeqIdx+1], FgColor[1]);
                        xout->SetIntensity(chArray[TestSeqIdx+2], FgColor[2]);
                    }
                    else
                    {
                        // was on, now go to bg color for random period
                        TwinkleState[i]=-static_cast<int>(rand01()*interval+100) / XTIMER_INTERVAL * (TwinkleRatio - 1);
                        TestSeqIdx = i * 3;
                        xout->SetIntensity(chArray[TestSeqIdx], BgColor[0]);
                        xout->SetIntensity(chArray[TestSeqIdx+1], BgColor[1]);
                        xout->SetIntensity(chArray[TestSeqIdx+2], BgColor[2]);
                    }
                }
                break;
            case SHIMMER:
                if (ColorChange || curtime >= NextSequenceStart)
                {
                    ShimColor = (ShimColor == FgColor) ? BgColor : FgColor;
                    for (i=0; i < chArray.Count(); i++)
                    {
                        xout->SetIntensity(chArray[i], ShimColor[i % 3]);
                    }
                }
                if (curtime >= NextSequenceStart)
                {
                    NextSequenceStart = curtime + interval/2;
                }
                break;
            case CHASE:
                if (ColorChange || curtime >= NextSequenceStart)
                {
                    for (i=0; i < chArray.Count(); i++)
                    {
                        v = (i / 3 % ChaseGrouping) == TestSeqIdx ? FgColor[i % 3] : BgColor[i % 3];
                        xout->SetIntensity(chArray[i], v);
                    }
                }
                if (curtime >= NextSequenceStart)
                {
                    NextSequenceStart = curtime + interval;
                    TestSeqIdx = (TestSeqIdx + 1) % ChaseGrouping;
                    if (TestSeqIdx >= (chArray.Count()+2) / 3) TestSeqIdx=0;
                }
                StatusBar1->SetStatusText(wxString::Format(_("Testing %ld channels; chase now at ch# %d"),static_cast<long>(chArray.Count()), TestSeqIdx)); //show current ch# -DJ
                break;
            default:
                break;
            }
            break;

        case 2:
            // RGB Cycle
            v=SliderRgbCycleSpeed->GetValue();  // 0-100
            if (TestFunc == DIM)
            {
                // color mixing
                if (v != LastSequenceSpeed)
                {
                    frequency=v/1000.0 + 0.05;
                    LastSequenceSpeed = v;
                }
                BgColor[0] = sin(frequency*TestSeqIdx + 0.0) * 127 + 128;
                BgColor[1] = sin(frequency*TestSeqIdx + 2.0) * 127 + 128;
                BgColor[2] = sin(frequency*TestSeqIdx + 4.0) * 127 + 128;
                TestSeqIdx++;
                for (i=0; i < chArray.Count(); i++)
                {
                    xout->SetIntensity(chArray[i], BgColor[i % 3]);
                }
            }
            else
            {
                // RGB cycle
                if (v != LastSequenceSpeed)
                {
                    interval = (101-v)*50;
                    NextSequenceStart = curtime + interval;
                    LastSequenceSpeed = v;
                }
                if (curtime >= NextSequenceStart)
                {
                    for (i=0; i < chArray.Count(); i++)
                    {
                        switch (rgbCycle)
                        {
                        case 3:
                            v=255;
                            break;
                        default:
                            v = (i % 3) == rgbCycle ? 255 : 0;
                            break;
                        }
                        xout->SetIntensity(chArray[i], v);
                    }
                    rgbCycle=(rgbCycle + 1) % ChaseGrouping;
                    NextSequenceStart += interval;
                }
            }
            break;
        }
    xout->TimerEnd();
}
