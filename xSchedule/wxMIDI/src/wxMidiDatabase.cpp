//=====================================================================================
// wxMidi: A MIDI interface based on PortMidi, the Portable Real-Time MIDI Library
// --------------------------------------------------------------------------------
//
// Author:      Cecilio Salmeron <s.cecilio@gmail.com>
// Copyright:   (c) 2005-2015 Cecilio Salmeron
// Licence:     wxWidgets license, version 3.1 or later at your choice.
//=====================================================================================
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "wxMidiDatabase.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wxMidi.h"

/*
wxMidiDatabaseGM
Methods to load and manage midi instruments names and codes

The General MIDI (GM) Specification, published by the International MIDI Association,
defines a set of general capabilities for General MIDI Instruments. The General MIDI
Specification includes the definition of a General MIDI Sound Set (a patch map), a General
MIDI Percussion map (mapping of percussion sounds to note numbers), and a set of General
MIDI Performance capabilities (number of voices, types of MIDI messages recognized, etc.).
A MIDI sequence which has been generated for use on a General MIDI Instrument should play
correctly on any General MIDI synthesizer or sound module.

The General MIDI system utilizes MIDI channels 1-9 and 11-16 for chromatic instrument sounds,
while channel number 10 is utilized for "key-based" percussion sounds.
The General MIDI Sound set for channels 1-9 and 11-16 is given in table 1.
These instrument sounds are grouped into "sets" of related sounds. For example, program
numbers 1-8 are piano sounds, 6-16 are chromatic percussion sounds, 17-24 are organ sounds,
25-32 are guitar sounds, etc.

For the instrument sounds on channels 1-9 and 11-16, the note number in a Note On message
is used to select the pitch of the sound which will be played. For example if the Vibraphone
instrument (program number 12) has been selected on channel 3, then playing note number 60
on channel 3 would play the middle C note (this would be the default note to pitch assignment
on most instruments), and note number 59 on channel 3 would play B below middle C.
Both notes would be played using the Vibraphone sound.

The General MIDI percussion map used for channel 10 is given in table 2. For these "key-based"
sounds, the note number data in a Note On message is used differently. Note numbers on
channel 10 are used to select which drum sound will be played. For example, a Note On
message on channel 10 with note number 60 will play a Hi Bongo drum sound. Note number 59
on channel 10 will play the Ride Cymbal 2 sound.

It should be noted that the General MIDI system specifies sounds using program numbers 1
through 128. The MIDI Program Change message used to select these sounds uses an 8-bit byte,
which corresponds to decimal numbering from 0 through 127, to specify the desired program
number. Thus, to select GM sound number 10, the Glockenspiel, the Program Change message
will have a data byte with the decimal value 9.

The General MIDI system specifies which instrument or sound corresponds with each
program/patch number, but General MIDI does not specify how these sounds are produced.
Thus, program number 1 should select the Acoustic Grand Piano sound on any General MIDI
instrument. However, the Acoustic Grand Piano sound on two General MIDI synthesizers
which use different synthesis techniques may sound quite different.

*/

//instruments table
static bool m_fStringsInitialized = false;
static wxString m_sInstrName[128];

//initialize the only instance
wxMidiDatabaseGM* wxMidiDatabaseGM::m_pInstance = (wxMidiDatabaseGM*)NULL;

// ----------------------------------------------------------------------------
// wxMidiDatabaseGM ctor/dtor
// ----------------------------------------------------------------------------

wxMidiDatabaseGM::wxMidiDatabaseGM()
{
    //language dependent strings. Can not be statically initiallized because
    //then they do not get translated
    if (!m_fStringsInitialized) {
	    //instrument names
	    m_sInstrName[0] = _("Acoustic Grand Piano");
	    m_sInstrName[1] = _("Bright Acoustic Piano");
	    m_sInstrName[2] = _("Electric Grand Piano");
	    m_sInstrName[3] = _("Honky-tonk Piano");
	    m_sInstrName[4] = _("Electric Piano 1");
	    m_sInstrName[5] = _("Electric Piano 2");
	    m_sInstrName[6] = _("Harpsichord");
	    m_sInstrName[7] = _("Clavinet");
	    m_sInstrName[8] = _("Celesta");
	    m_sInstrName[9] = _("Glockenspiel");
	    m_sInstrName[10] = _("Music Box");
	    m_sInstrName[11] = _("Vibraphone");
	    m_sInstrName[12] = _("Marimba");
	    m_sInstrName[13] = _("Xylophone");
	    m_sInstrName[14] = _("Tubular Bells");
	    m_sInstrName[15] = _("Dulcimer");
	    m_sInstrName[16] = _("Drawbar Organ");
	    m_sInstrName[17] = _("Percussive Organ");
	    m_sInstrName[18] = _("Rock Organ");
	    m_sInstrName[19] = _("Church Organ");
	    m_sInstrName[20] = _("Reed Organ");
	    m_sInstrName[21] = _("Accordion");
	    m_sInstrName[22] = _("Harmonica");
	    m_sInstrName[23] = _("Tango Accordion");
	    m_sInstrName[24] = _("Acoustic Guitar (nylon)");
	    m_sInstrName[25] = _("Acoustic Guitar (steel)");
	    m_sInstrName[26] = _("Electric Guitar (jazz)");
	    m_sInstrName[27] = _("Electric Guitar (clean)");
	    m_sInstrName[28] = _("Electric Guitar (muted)");
	    m_sInstrName[29] = _("Overdriven Guitar");
	    m_sInstrName[30] = _("Distortion Guitar");
	    m_sInstrName[31] = _("Guitar harmonics");
	    m_sInstrName[32] = _("Acoustic Bass");
	    m_sInstrName[33] = _("Electric Bass (finger)");
	    m_sInstrName[34] = _("Electric Bass (pick)");
	    m_sInstrName[35] = _("Fretless Bass");
	    m_sInstrName[36] = _("Slap Bass 1");
	    m_sInstrName[37] = _("Slap Bass 2");
	    m_sInstrName[38] = _("Synth Bass 1");
	    m_sInstrName[39] = _("Synth Bass 2");
	    m_sInstrName[40] = _("Violin");
	    m_sInstrName[41] = _("Viola");
	    m_sInstrName[42] = _("Cello");
	    m_sInstrName[43] = _("Contrabass");
	    m_sInstrName[44] = _("Tremolo Strings");
	    m_sInstrName[45] = _("Pizzicato Strings");
	    m_sInstrName[46] = _("Orchestral Harp");
	    m_sInstrName[47] = _("Timpani");
	    m_sInstrName[48] = _("String Ensemble 1");
	    m_sInstrName[49] = _("String Ensemble 2");
	    m_sInstrName[50] = _("SynthStrings 1");
	    m_sInstrName[51] = _("SynthStrings 2");
	    m_sInstrName[52] = _("Choir Aahs");
	    m_sInstrName[53] = _("Voice Oohs");
	    m_sInstrName[54] = _("Synth Voice");
	    m_sInstrName[55] = _("Orchestra Hit");
	    m_sInstrName[56] = _("Trumpet");
	    m_sInstrName[57] = _("Trombone");
	    m_sInstrName[58] = _("Tuba");
	    m_sInstrName[59] = _("Muted Trumpet");
	    m_sInstrName[60] = _("French Horn");
	    m_sInstrName[61] = _("Brass Section");
	    m_sInstrName[62] = _("SynthBrass 1");
	    m_sInstrName[63] = _("SynthBrass 2");
	    m_sInstrName[64] = _("Soprano Sax");
	    m_sInstrName[65] = _("Alto Sax");
	    m_sInstrName[66] = _("Tenor Sax");
	    m_sInstrName[67] = _("Baritone Sax");
	    m_sInstrName[68] = _("Oboe");
	    m_sInstrName[69] = _("English Horn");
	    m_sInstrName[70] = _("Bassoon");
	    m_sInstrName[71] = _("Clarinet");
	    m_sInstrName[72] = _("Piccolo");
	    m_sInstrName[73] = _("Flute");
	    m_sInstrName[74] = _("Recorder");
	    m_sInstrName[75] = _("Pan Flute");
	    m_sInstrName[76] = _("Blown Bottle");
	    m_sInstrName[77] = _("Shakuhachi");
	    m_sInstrName[78] = _("Whistle");
	    m_sInstrName[79] = _("Ocarina");
	    m_sInstrName[80] = _("Lead 1 (square)");
	    m_sInstrName[81] = _("Lead 2 (sawtooth)");
	    m_sInstrName[82] = _("Lead 3 (calliope)");
	    m_sInstrName[83] = _("Lead 4 (chiff)");
	    m_sInstrName[84] = _("Lead 5 (charang)");
	    m_sInstrName[85] = _("Lead 6 (voice)");
	    m_sInstrName[86] = _("Lead 7 (fifths)");
	    m_sInstrName[87] = _("Lead 8 (bass + lead)");
	    m_sInstrName[88] = _("Pad 1 (new age)");
	    m_sInstrName[89] = _("Pad 2 (warm)");
	    m_sInstrName[90] = _("Pad 3 (polysynth)");
	    m_sInstrName[91] = _("Pad 4 (choir)");
	    m_sInstrName[92] = _("Pad 5 (bowed)");
	    m_sInstrName[93] = _("Pad 6 (metallic)");
	    m_sInstrName[94] = _("Pad 7 (halo)");
	    m_sInstrName[95] = _("Pad 8 (sweep)");
	    m_sInstrName[96] = _("FX 1 (rain)");
	    m_sInstrName[97] = _("FX 2 (soundtrack)");
	    m_sInstrName[98] = _("FX 3 (crystal)");
	    m_sInstrName[99] = _("FX 4 (atmosphere)");
	    m_sInstrName[100] = _("FX 5 (brightness)");
	    m_sInstrName[101] = _("FX 6 (goblins)");
	    m_sInstrName[102] = _("FX 7 (echoes)");
	    m_sInstrName[103] = _("FX 8 (sci-fi)");
	    m_sInstrName[104] = _("Sitar");
	    m_sInstrName[105] = _("Banjo");
	    m_sInstrName[106] = _("Shamisen");
	    m_sInstrName[107] = _("Koto");
	    m_sInstrName[108] = _("Kalimba");
	    m_sInstrName[109] = _("Bag pipe");
	    m_sInstrName[110] = _("Fiddle");
	    m_sInstrName[111] = _("Shanai");
	    m_sInstrName[112] = _("Tinkle Bell");
	    m_sInstrName[113] = _("Agogo");
	    m_sInstrName[114] = _("Steel Drums");
	    m_sInstrName[115] = _("Woodblock");
	    m_sInstrName[116] = _("Taiko Drum");
	    m_sInstrName[117] = _("Melodic Tom");
	    m_sInstrName[118] = _("Synth Drum");
	    m_sInstrName[119] = _("Reverse Cymbal");
	    m_sInstrName[120] = _("Guitar Fret Noise");
	    m_sInstrName[121] = _("Breath Noise");
	    m_sInstrName[122] = _("Seashore");
	    m_sInstrName[123] = _("Bird Tweet");
	    m_sInstrName[124] = _("Telephone Ring");
	    m_sInstrName[125] = _("Helicopter");
	    m_sInstrName[126] = _("Applause");
	    m_sInstrName[127] = _("Gunshot");

        m_fStringsInitialized = true;
    }

}

wxMidiDatabaseGM::~wxMidiDatabaseGM()
{
}

wxMidiDatabaseGM* wxMidiDatabaseGM::GetInstance()
{
	if (!m_pInstance) {
		m_pInstance = new wxMidiDatabaseGM();
		m_pInstance->Initialize();
	}
	return m_pInstance;
}

int wxMidiDatabaseGM::GetNumSections()
{
	return NUM_SECTIONS;
}

wxString wxMidiDatabaseGM::GetInstrumentName(int nInstr)
{
	wxASSERT(nInstr >=0 && nInstr < 128);
	return m_sInstrName[nInstr];
}

wxString wxMidiDatabaseGM::GetSectionName(int nSect)
{
	wxASSERT(nSect >=0 && nSect < NUM_SECTIONS);
	return m_sSectName[nSect];
}

int wxMidiDatabaseGM::GetNumInstrumentsInSection(int nSect)
{
	wxASSERT(nSect >=0 && nSect < NUM_SECTIONS);
	return m_nNumInstrInSection[nSect];
}

int wxMidiDatabaseGM::GetInstrFromSection(int nSect, int i)
{
	wxASSERT(nSect >=0 && nSect < NUM_SECTIONS);
	wxASSERT(i >=0 && i < m_nNumInstrInSection[nSect]);
	return m_nSectInstr[nSect][i];
}

void wxMidiDatabaseGM::PopulateWithInstruments(wxControlWithItems* pCtrol, int nSection,
                                               int nInstr, bool fAddNumber)
{
	//Populate a wxControlWithItems (wxListBox, wxCheckListBox, wxChoice, wxComboBox, ...)
	//with the list of intruments in a section.
	//Leave selected instrument number nInstr (default: the first one of the section)
    //If fAddNumber is true, added name is preceeded by instrument number

	pCtrol->Clear();
	int iSel=0;
    if (nSection < 0 || nSection > NUM_SECTIONS) {
		//error in section number. Leave control cleared
        return;
    } else {
		//populate control
		int nCurInstr;
		for (int i=0; i < m_nNumInstrInSection[nSection]; i++ )
        {
			nCurInstr = m_nSectInstr[nSection][i];
            wxString sName = (fAddNumber ? wxString::Format(_T("%d - "), nCurInstr) : _T(""));
            sName += m_sInstrName[nCurInstr];
            pCtrol->Append(sName);
			if (nCurInstr == nInstr) iSel = i;
        }
    }
    pCtrol->SetStringSelection( pCtrol->GetString(iSel) );

}

void wxMidiDatabaseGM::PopulateWithAllInstruments(wxControlWithItems* pCtrol, int nInstr)
{
	/*
	Populate a wxControlWithItems (wxListBox, wxCheckListBox, wxChoice, wxComboBox, ...)
	with the list of all intruments in GM standard.
	Leave selected instrument number nInstr.
	*/

	pCtrol->Clear();
	int iSel = 0;
	if (nInstr >=0 && nInstr < 128) iSel = nInstr;

	//populate control
	for (int i=0; i < 128; i++ ) {
        pCtrol->Append( m_sInstrName[i] );
    }
    pCtrol->SetStringSelection( pCtrol->GetString(iSel) );

}

int wxMidiDatabaseGM::PopulateWithSections(wxControlWithItems* pCtrol, int nSelInstr)
{
	/*
	Populate a wxControlWithItems (wxListBox, wxCheckListBox, wxChoice, wxComboBox, ...)
	with the list of section names.

	If nSelInstr != -1 select the section which includes instrument nSelInstr. Otherwise,
	select first section

	Returns the number of the section selected
	*/

    //load section names
	for (int i=0; i < NUM_SECTIONS; i++) {
        pCtrol->Append( m_sSectName[i] );
    }

    //Find section for instrument nSelInstr
    ;
	int iSel = 0;
    if (nSelInstr != -1) {
		bool fFound = false;
		for (int iSec=0; !fFound && iSec < NUM_SECTIONS; iSec++) {
			for (int j=0; j < m_nNumInstrInSection[iSec]; j++) {
				if (m_nSectInstr[iSec][j] == nSelInstr) {
					iSel = iSec;
					fFound = true;
					break;
				}
			}
        }
    }
    pCtrol->SetStringSelection( pCtrol->GetString(iSel) );

    return iSel;

}

void wxMidiDatabaseGM::PopulateWithPercusionInstr(wxControlWithItems* pCtrol, int iSel)
{
	pCtrol->Clear();
	pCtrol->Append( _("35-Acoustic Bass Drum") );
	pCtrol->Append( _("36-Bass Drum 1") );
	pCtrol->Append( _("37-Side Stick") );
	pCtrol->Append( _("38-Acoustic Snare") );
	pCtrol->Append( _("39-Hand clap") );
	pCtrol->Append( _("40-Electric Snare") );
	pCtrol->Append( _("41_Low Floor Tom") );
	pCtrol->Append( _("42_Closed High-Hat") );
	pCtrol->Append( _("43-High Floor Tom") );
	pCtrol->Append( _("44-Pedal High-Hat") );
	pCtrol->Append( _("45-Low Tom") );
	pCtrol->Append( _("46-Open High-Hat") );
	pCtrol->Append( _("47-Low-Mid Tom") );
	pCtrol->Append( _("48-High-Mid Tom") );
	pCtrol->Append( _("49-Crash Cymbal 1") );
	pCtrol->Append( _("50-High Tom") );
	pCtrol->Append( _("51-Ride Cymbal 1") );
	pCtrol->Append( _("52-Chinese Cymbal") );
	pCtrol->Append( _("53-Ride Bell") );
	pCtrol->Append( _("54-Tambourine") );
	pCtrol->Append( _("55-Splash Cymbal") );
	pCtrol->Append( _("56-Cowbell") );
	pCtrol->Append( _("57-Crash Cymbal 2") );
	pCtrol->Append( _("58-Vibraslap") );
	pCtrol->Append( _("59-Ride Cymbal 2") );
	pCtrol->Append( _("60-High Bongo") );
	pCtrol->Append( _("61-Low Bongo") );
	pCtrol->Append( _("62-Mute High Conga") );
	pCtrol->Append( _("63-Open High Conga") );
	pCtrol->Append( _("64-Low Conga") );
	pCtrol->Append( _("65-High Timbale") );
	pCtrol->Append( _("66-Low Timbale") );
	pCtrol->Append( _("67-High Agogo") );
	pCtrol->Append( _("68-Low Agogo") );
	pCtrol->Append( _("69-Casaba") );
	pCtrol->Append( _("70-Maracas") );
	pCtrol->Append( _("71-Short Whistle") );
	pCtrol->Append( _("72-Long Whistle") );
	pCtrol->Append( _("73-Short Guiro") );
	pCtrol->Append( _("74-Long Guiro") );
	pCtrol->Append( _("75-Claves") );
	pCtrol->Append( _("76-High Wood Block") );
	pCtrol->Append( _("77-Low Wood Block") );
	pCtrol->Append( _("78-Mute Cuica") );
	pCtrol->Append( _("79-Open Cuica") );
	pCtrol->Append( _("80-Mute Triangle") );
	pCtrol->Append( _("81-Open Triangle") );

	//select item
    int i = iSel - 35;
    if (i < 0 || i > 46)  i=0;
    pCtrol->SetStringSelection( pCtrol->GetString(i) );

}

void wxMidiDatabaseGM::Initialize()
{
	//organize instruments into sections
	//All code is prepared to deal with an arbitray number of sections, each
	//containing a variable number of instruments. But for now lets use the
	//MIDI groups with 8 instruments in each group

	// WARNING: If you change this, modify constants NUM_SECTIONS and NUM_INSTRS in
	// header file!!!

	//section "Keyboards"
	m_sSectName[0] = _("Keyboards");
    m_nSectInstr[0][0] = 0;				//0 - Acoustic Grand Piano
    m_nSectInstr[0][1] = 1;				//1 - Bright Acoustic Piano
    m_nSectInstr[0][2] = 2;				//2 - Electric Grand Piano
	m_nSectInstr[0][3] = 3;				//3 - Honky-tonk Piano
	m_nSectInstr[0][4] = 4;				//4 - Electric Piano 1
	m_nSectInstr[0][5] = 5;				//5 - Electric Piano 2
	m_nSectInstr[0][6] = 6;				//6 - Harpsichord
	m_nSectInstr[0][7] = 7;				//7 - Clavinet
	m_nNumInstrInSection[0] = 8;		//number of instruments in this section

	//section "Chromatic percussion"
	m_sSectName[1] = _("Chromatic percussion");
	m_nSectInstr[1][0] = 8; 			//8  - Celesta");			//"Celesta"
	m_nSectInstr[1][1] = 9; 			//9  - Glockenspiel");		//"Glockenspiel"
	m_nSectInstr[1][2] = 10;			//10 - Music Box");			//"Caja de música"
	m_nSectInstr[1][3] = 11;			//11 - Vibraphone");		//"Vibráfono"
	m_nSectInstr[1][4] = 12;			//12 - Marimba");			//"Marimba"
	m_nSectInstr[1][5] = 13;			//13 - Xylophone");			//"Xilófono"
	m_nSectInstr[1][6] = 14;			//14 - Tubular Bells");		//"Campanas"
	m_nSectInstr[1][7] = 15;			//15 - Dulcimer");			//"Dulcemer (salterio alemán)"
	m_nNumInstrInSection[1] = 8;		//number of instruments in this section

    //section "Organs and harmonicas"
	m_sSectName[2] = _("Organs and harmonicas");
    m_nSectInstr[2][0] = 16;			//16 - Drawbar Organ");		//"Organo Hammond"
    m_nSectInstr[2][1] = 17;			//17 - Percussive Organ");	//"Organo de percusión"
    m_nSectInstr[2][2] = 18;			//18 - Rock Organ");			//"Organo de rock"
    m_nSectInstr[2][3] = 19;			//19 - Church Organ");		//"Organo de iglesia"
    m_nSectInstr[2][4] = 20;			//20 - Reed Organ");			//"Armonio"
    m_nSectInstr[2][5] = 21;			//21 - Accordion");			//"Acordeón"
    m_nSectInstr[2][6] = 22;			//22 - Harmonica");			//"Armónica"
    m_nSectInstr[2][7] = 23;			//23 - Tango Accordion");    //"Bandoneón"
	m_nNumInstrInSection[2] = 8;		//number of instruments in this section

    //section "Guitars"
	m_sSectName[3] = _("Guitars");
    m_nSectInstr[3][0] = 24;			//24 - Acoustic Guitar (nylon)");    //"Guitarra cuerdas de nylon"
	m_nSectInstr[3][1] = 25;			//25 - Acoustic Guitar (steel)");    //"Guitarra cuerdas metálicas"
    m_nSectInstr[3][2] = 26;			//26 - Electric Guitar (jazz)");     //"Guitarra eléctrica de jazz"
    m_nSectInstr[3][3] = 27;			//27 - Electric Guitar (clean)");    //"Guitarra eléctrica"
    m_nSectInstr[3][4] = 28;			//28 - Electric Guitar (muted)");    //"Guitarra eléctrica (con sordina)"
    m_nSectInstr[3][5] = 29;			//29 - Overdriven Guitar");			//"Guitarra eléctrica (overdriven)"
    m_nSectInstr[3][6] = 30;			//30 - Distortion Guitar");			//"Guitarra eléctrica (con distorsión)"
    m_nSectInstr[3][7] = 31;			//31 - Guitar harmonics");			//"Guitarra de armónicos"
	m_nNumInstrInSection[3] = 8;		//number of instruments in this section

    //section "Bass"
	m_sSectName[4] = _("Basses");
    m_nSectInstr[4][0] = 32;			//32 - Acoustic Bass");				//"Bajo acústico"
    m_nSectInstr[4][1] = 33;			//33 - Electric Bass (finger)");     //"Bajo eléctrico (finger)"
    m_nSectInstr[4][2] = 34;			//34 - Electric Bass (pick)");	    //"Bajo eléctrico (pick)"
    m_nSectInstr[4][3] = 35;			//35 - Fretless Bass");				//"Bajo sin trastes"
    m_nSectInstr[4][4] = 36;			//36 - Slap Bass 1");				//"Slap Bass 1"
    m_nSectInstr[4][5] = 37;			//37 - Slap Bass 2");				//"Slap Bass 2"
    m_nSectInstr[4][6] = 38;			//38 - Synth Bass 1");				//"Synth Bass 1"
    m_nSectInstr[4][7] = 39;			//39 - Synth Bass 2");				//"Synth Bass 2"
	m_nNumInstrInSection[4] = 8;		//number of instruments in this section

    //section "Strings"
	m_sSectName[5] = _("Strings");
    m_nSectInstr[5][0] = 40;			//40 - Violin");				//"Violín"
    m_nSectInstr[5][1] = 41;			//41 - Viola");				//"Viola"
    m_nSectInstr[5][2] = 42;			//42 - Cello");				//"Violoncelo"
    m_nSectInstr[5][3] = 43;			//43 - Contrabass");			//"Contrabajo"
    m_nSectInstr[5][4] = 44;			//44 - Tremolo Strings");	//"Tremolo strings"
    m_nSectInstr[5][5] = 45;			//45 - Pizzicato Strings");	//"Pizzicato strings"
    m_nSectInstr[5][6] = 46;			//46 - Orchestral Harp");	//"Arpa"
    m_nSectInstr[5][7] = 47;			//47 - Timpani");			//"Timbales"
	m_nNumInstrInSection[5] = 8;		//number of instruments in this section

    //section "Ensembles"
	m_sSectName[6] = _("Ensembles");
    m_nSectInstr[6][0] = 48;			//48 - String Ensemble 1");     //"Cuerdas"
    m_nSectInstr[6][1] = 49;			//49 - String Ensemble 2");     //"Slow Str."
    m_nSectInstr[6][2] = 50;			//50 - SynthStrings 1");     //"Syn Str. 1"
    m_nSectInstr[6][3] = 51;			//51 - SynthStrings 2");     //"Syn Str. 2"
    m_nSectInstr[6][4] = 52;			//52 - Choir Aahs");     //"Coro de ¡aahs!"
    m_nSectInstr[6][5] = 53;			//53 - Voice Oohs");     //"Coro de Oohs"
    m_nSectInstr[6][6] = 54;			//54 - Synth Voice");     //"Voces sintetizadas"
    m_nSectInstr[6][7] = 55;			//55 - Orchestra Hit");     //"Orchestra Hit"
	m_nNumInstrInSection[6] = 8;		//number of instruments in this section

    //section "Brass"
	m_sSectName[7] = _("Brass");
    m_nSectInstr[7][0] = 56;			//56 - Trumpet");     //"Trompeta"
    m_nSectInstr[7][1] = 57;			//57 - Trombone");     //"Trombón"
    m_nSectInstr[7][2] = 58;			//58 - Tuba");     //"Tuba"
    m_nSectInstr[7][3] = 59;			//59 - Muted Trumpet");     //"Trompeta con sordina"
    m_nSectInstr[7][4] = 60;			//60 - French Horn");     //"Trompa"
    m_nSectInstr[7][5] = 61;			//61 - Brass Section");     //"Grupo de viento-metal"
    m_nSectInstr[7][6] = 62;			//62 - SynthBrass 1");     //"Synth Br.1"
    m_nSectInstr[7][7] = 63;			//63 - SynthBrass 2");     //"Synth Br.2"
	m_nNumInstrInSection[7] = 8;		//number of instruments in this section

    //section "Reed"
	m_sSectName[8] = _("Reed");
    m_nSectInstr[8][0] = 64;			//64 - Soprano Sax");     //"Saxofón soprano"
    m_nSectInstr[8][1] = 65;			//65 - Alto Sax");     //"Saxofón contralto"
    m_nSectInstr[8][2] = 66;			//66 - Tenor Sax");     //"Saxofón tenor"
    m_nSectInstr[8][3] = 67;			//67 - Baritone Sax");     //"Saxofón barítono"
    m_nSectInstr[8][4] = 68;			//68 - Oboe");     //"Oboe"
    m_nSectInstr[8][5] = 69;			//69 - English Horn");     //"Corno ingles"
    m_nSectInstr[8][6] = 70;			//70 - Bassoon");     //"Fagot"
    m_nSectInstr[8][7] = 71;			//71 - Clarinet");     //"Clarinete"
	m_nNumInstrInSection[8] = 8;		//number of instruments in this section

    //section "Pipe"
	m_sSectName[9] = _("Pipe");
    m_nSectInstr[9][0] = 72;			//72 - Piccolo");     //"Falutín"
    m_nSectInstr[9][1] = 73;			//73 - Flute");     //"Flauta travesera"
    m_nSectInstr[9][2] = 74;			//74 - Recorder");     //"Flauta dulce"
    m_nSectInstr[9][3] = 75;			//75 - Pan Flute");     //"Flauta de Pan"
    m_nSectInstr[9][4] = 76;			//76 - Blown Bottle");     //"Botella soplada"
    m_nSectInstr[9][5] = 77;			//77 - Shakuhachi");     //"Shakuhachi"
    m_nSectInstr[9][6] = 78;			//78 - Whistle");     //"Silbato"
    m_nSectInstr[9][7] = 79;			//79 - Ocarina");     //"Ocarina"
	m_nNumInstrInSection[9] = 8;		//number of instruments in this section

    //section "Synth. lead"
	m_sSectName[10] = _("Synth. lead");
    m_nSectInstr[10][0] = 80;			//80 - Lead 1 (square)");     //"Onda cuadrada"
    m_nSectInstr[10][1] = 81;			//81 - Lead 2 (sawtooth)");     //"Onda en diente de sierra"
    m_nSectInstr[10][2] = 82;			//82 - Lead 3 (calliope)");     //"Calliope"
    m_nSectInstr[10][3] = 83;			//83 - Lead 4 (chiff)");     //"Chiffer Lead"
    m_nSectInstr[10][4] = 84;			//84 - Lead 5 (charang)");     //"Charanga"
    m_nSectInstr[10][5] = 85;			//85 - Lead 6 (voice)");     //"Voz"
    m_nSectInstr[10][6] = 86;			//86 - Lead 7 (fifths)");     //"Quintas"
    m_nSectInstr[10][7] = 87;			//87 - Lead 8 (bass + lead)");     //"Bass& Lead"
	m_nNumInstrInSection[10] = 8;		//number of instruments in this section

    //section "Synth. pad"
	m_sSectName[11] = _("Synth. pad");
    m_nSectInstr[11][0] = 88;			//88 - Pad 1 (new age)");     //"Fantasia"
    m_nSectInstr[11][1] = 89;			//89 - Pad 2 (warm)");     //"Warm Pad"
    m_nSectInstr[11][2] = 90;			//90 - Pad 3 (polysynth)");     //"PolySynth"
    m_nSectInstr[11][3] = 91;			//91 - Pad 4 (choir)");     //"Space Voice"
    m_nSectInstr[11][4] = 92;			//92 - Pad 5 (bowed)");     //"Bowed Glass"
    m_nSectInstr[11][5] = 93;			//93 - Pad 6 (metallic)");     //"Metal Pad"
    m_nSectInstr[11][6] = 94;			//94 - Pad 7 (halo)");     //"Halo Pad"
    m_nSectInstr[11][7] = 95;			//95 - Pad 8 (sweep)");     //"Sweep Pad"
	m_nNumInstrInSection[11] = 8;		//number of instruments in this section

    //section "Synth. F/X"
	m_sSectName[12] = _("Synth. F/X");
    m_nSectInstr[12][0] = 96;			//96 - FX 1 (rain)");     //"Ice Rain"
    m_nSectInstr[12][1] = 97;			//97 - FX 2 (soundtrack)");     //"Sound Track"
    m_nSectInstr[12][2] = 98;			//98 - FX 3 (crystal)");     //"Crystal"
    m_nSectInstr[12][3] = 99;			//99 - FX 4 (atmosphere)");     //"Atmosphere"
    m_nSectInstr[12][4] = 100;			//100 - FX 5 (brightness)");    //"Brightness"
    m_nSectInstr[12][5] = 101;			//101 - FX 6 (goblins)");    //"Goblin"
    m_nSectInstr[12][6] = 102;			//102 - FX 7 (echoes)");    //"Echo Drops"
    m_nSectInstr[12][7] = 103;			//103 - FX 8 (sci-fi)");    //"Star Theme"
	m_nNumInstrInSection[12] = 8;		//number of instruments in this section

    //section "Ethnic"
	m_sSectName[13] = _("Ethnic");
    m_nSectInstr[13][0] = 104;			//104 - Sitar");    //"Sitar"
    m_nSectInstr[13][1] = 105;			//105 - Banjo");    //"Banjo"
    m_nSectInstr[13][2] = 106;			//106 - Shamisen");    //"Shamisen"
    m_nSectInstr[13][3] = 107;			//107 - Koto");    //"Koto"
    m_nSectInstr[13][4] = 108;			//108 - Kalimba");    //"Kalimba"
    m_nSectInstr[13][5] = 109;			//109 - Bag pipe");    //"Gaita"
    m_nSectInstr[13][6] = 110;			//110 - Fiddle");    //"Fiddle"
    m_nSectInstr[13][7] = 111;			//111 - Shanai");    //"Shanai"
	m_nNumInstrInSection[13] = 8;		//number of instruments in this section

    //section "Percussive"
	m_sSectName[14] = _("Percussive");
    m_nSectInstr[14][0] = 112;			//112 - Tinkle Bell");    //"Tinkle Bell"
    m_nSectInstr[14][1] = 113;			//113 - Agogo");    //"Agogo"
    m_nSectInstr[14][2] = 114;			//114 - Steel Drums");    //"Steel Drums"
    m_nSectInstr[14][3] = 115;			//115 - Woodblock");    //"Woodblock"
    m_nSectInstr[14][4] = 116;			//116 - Taiko Drum");    //"Taiko"
    m_nSectInstr[14][5] = 117;			//117 - Melodic Tom");    //"Melo Tom 1"
    m_nSectInstr[14][6] = 118;			//118 - Synth Drum");    //"Synth Drum"
    m_nSectInstr[14][7] = 119;			//119 - Reverse Cymbal");    //"Reverse Cym."
	m_nNumInstrInSection[14] = 8;		//number of instruments in this section

    //section "Sound F/X"
	m_sSectName[15] = _("Sound F/X");
    m_nSectInstr[15][0] = 120;			//120 - Guitar Fret Noise");    //"Gt.FretNoise"
    m_nSectInstr[15][1] = 121;			//121 - Breath Noise");    //"Breath Noise"
    m_nSectInstr[15][2] = 122;			//122 - Seashore");    //"Seashore"
    m_nSectInstr[15][3] = 123;			//123 - Bird Tweet");    //"Bird"
    m_nSectInstr[15][4] = 124;			//124 - Telephone Ring");    //"Telephone 1"
    m_nSectInstr[15][5] = 125;			//125 - Helicopter");    //"Helicopter"
    m_nSectInstr[15][6] = 126;			//126 - Applause");    //"Applause"
    m_nSectInstr[15][7] = 127;			//127 - Gunshot");    //"Gun Shot"
	m_nNumInstrInSection[15] = 8;		//number of instruments in this section

}

/*

The Roland General Synthesizer (GS) Standard
The Roland General Synthesizer (GS) functions are a superset of those specified for
General MIDI. The GS system includes all of the GM sounds (which are referred to as
"capital instrument" sounds), and adds new sounds which are organized as variations
of the capital instruments.

Variations are selected using the MIDI Control Change message in conjunction with the
Program Change message. The Control Change message is sent first, and it is used to set
controller number 0 to some specified nonzero value indicating the desired variation
(some capital sounds have several different variations). The Control Change message
is followed by a MIDI Program Change message which indicates the program number of the
related capital instrument. For example, Capital instrument number 25 is the Nylon String
Guitar. The Ukulele is a variation of this instrument. The Ukulele is selected by sending
a Control Change message which sets controller number 0 to a value of 8, followed by a
program change message on the same channel which selects program number 25. Sending
the Program change message alone would select the capital instrument, the Nylon String
Guitar. Note also that a Control Change of controller number 0 to a value of 0 followed
by a Program Change message would also select the capital instrument.

The GS system also includes adjustable reverberation and chorus effects. The effects
depth for both reverb and chorus may be adjusted on an individual MIDI channel basis
using Control Change messages. The type of reverb and chorus sounds employed may also
be selected using System Exclusive messages.

*/
