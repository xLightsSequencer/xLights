#ifndef EVENTPANEL_H
#define EVENTPANEL_H

#include <wx/panel.h>

class EventBase;

class EventPanel : public wxPanel
{
	public:
		EventPanel() : 
			wxPanel(){}
		virtual ~EventPanel() {}
        virtual void Save(EventBase* event) = 0;
        virtual void Load(EventBase* event) = 0;
		virtual bool ValidateWindow() = 0;
};

#endif
