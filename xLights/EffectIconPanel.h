#ifndef EFFECTICONPANEL_H
#define EFFECTICONPANEL_H

//(*Headers(EffectIconPanel)
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bmpbuttn.h>
//*)

#include "DragEffectBitmapButton.h"

class EffectIconPanel: public wxPanel
{
	public:

		EffectIconPanel(wxWindow* parent,wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~EffectIconPanel();

		//(*Declarations(EffectIconPanel)
		DragEffectBitmapButton* BitmapButton29;
		DragEffectBitmapButton* BitmapButton33;
		DragEffectBitmapButton* BitmapButton18;
		DragEffectBitmapButton* BitmapButton1;
		DragEffectBitmapButton* BitmapButton13;
		DragEffectBitmapButton* BitmapButton24;
		DragEffectBitmapButton* BitmapButton22;
		DragEffectBitmapButton* BitmapButton5;
		DragEffectBitmapButton* BitmapButton32;
		DragEffectBitmapButton* BitmapButton2;
		DragEffectBitmapButton* BitmapButton23;
		DragEffectBitmapButton* BitmapButton15;
		DragEffectBitmapButton* BitmapButton11;
		DragEffectBitmapButton* BitmapButton27;
		DragEffectBitmapButton* BitmapButton4;
		DragEffectBitmapButton* BitmapButton25;
		DragEffectBitmapButton* BitmapButton26;
		DragEffectBitmapButton* BitmapButton28;
		DragEffectBitmapButton* BitmapButton16;
		DragEffectBitmapButton* BitmapButton10;
		DragEffectBitmapButton* BitmapButton7;
		DragEffectBitmapButton* BitmapButton9;
		DragEffectBitmapButton* BitmapButton17;
		DragEffectBitmapButton* BitmapButton31;
		DragEffectBitmapButton* BitmapButton12;
		DragEffectBitmapButton* BitmapButton19;
		DragEffectBitmapButton* BitmapButton6;
		DragEffectBitmapButton* BitmapButton21;
		DragEffectBitmapButton* BitmapButton20;
		DragEffectBitmapButton* BitmapButton8;
		DragEffectBitmapButton* BitmapButton30;
		DragEffectBitmapButton* BitmapButton14;
		DragEffectBitmapButton* BitmapButton3;
		//*)

	protected:

		//(*Identifiers(EffectIconPanel)
		static const long ID_BITMAPBUTTON1;
		static const long ID_BITMAPBUTTON2;
		static const long ID_BITMAPBUTTON3;
		static const long ID_BITMAPBUTTON4;
		static const long ID_BITMAPBUTTON5;
		static const long ID_BITMAPBUTTON6;
		static const long ID_BITMAPBUTTON7;
		static const long ID_BITMAPBUTTON8;
		static const long ID_BITMAPBUTTON9;
		static const long ID_BITMAPBUTTON10;
		static const long ID_BITMAPBUTTON11;
		static const long ID_BITMAPBUTTON12;
		static const long ID_BITMAPBUTTON13;
		static const long ID_BITMAPBUTTON14;
		static const long ID_BITMAPBUTTON15;
		static const long ID_BITMAPBUTTON16;
		static const long ID_BITMAPBUTTON17;
		static const long ID_BITMAPBUTTON18;
		static const long ID_BITMAPBUTTON19;
		static const long ID_BITMAPBUTTON20;
		static const long ID_BITMAPBUTTON21;
		static const long ID_BITMAPBUTTON22;
		static const long ID_BITMAPBUTTON23;
		static const long ID_BITMAPBUTTON24;
		static const long ID_BITMAPBUTTON25;
		static const long ID_BITMAPBUTTON26;
		static const long ID_BITMAPBUTTON27;
		static const long ID_BITMAPBUTTON28;
		static const long ID_BITMAPBUTTON29;
		static const long ID_BITMAPBUTTON30;
		static const long ID_BITMAPBUTTON31;
		static const long ID_BITMAPBUTTON32;
		static const long ID_BITMAPBUTTON36;
		//*)

	private:

		//(*Handlers(EffectIconPanel)
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
