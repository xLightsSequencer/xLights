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

#include <wx/event.h>
#include <wx/colour.h>
#include <wx/bitmap.h>
#include <string>
#include <vector>

#include "../Color.h"

class wxBitmap;
class EffectManager;
class wxWindow;
class xlTouchBar;
class MainSequencer;

class xlTouchBarSupport {
public:
    xlTouchBarSupport();
    ~xlTouchBarSupport();


    void Init(wxWindow *w);

    bool HasTouchBar() { return controller != nullptr; }

    wxWindow *GetWindow() { return window; }
    wxWindow *GetControlParent() { return parent; }

    void SetActive(xlTouchBar *tb);
    bool IsActive(xlTouchBar *tb) {
        return currentBar == tb;
    }
    xlTouchBar *GetCurrentBar() const {return currentBar;};

private:
    wxWindow *window;
    wxWindow *parent;
    void *controller;
    xlTouchBar *currentBar;
};


class TouchBarItem {
public:
    TouchBarItem(const std::string &n) : name(n) {}
    virtual ~TouchBarItem() {}

    const std::string &GetName() const {return name;}
protected:
    std::string name;
};

class wxControlTouchBarItem : public TouchBarItem {
public:
    wxControlTouchBarItem(wxWindow *c);
    virtual ~wxControlTouchBarItem() {}

    wxWindow *GetControl() const { return control; }
private:
    wxWindow *control;
};

typedef std::function<void(void)> ButtonTouchBarItemClicked;
class ButtonTouchBarItem : public TouchBarItem {
public:
    ButtonTouchBarItem(ButtonTouchBarItemClicked cb, const std::string &n, const std::string &l)
        : TouchBarItem(n), callback(cb), label(l), backgroundColor(0, 0, 0, 0) {};
    ButtonTouchBarItem(ButtonTouchBarItemClicked cb, const std::string &n, const wxBitmap &l)
        : TouchBarItem(n), callback(cb), bmp(l), backgroundColor(0, 0, 0, 0)  {};
    virtual ~ButtonTouchBarItem() {}

    virtual void Clicked() {
        callback();
    }

    void SetBackgroundColor(const xlColor &c) {backgroundColor = c;}


    const std::string &GetLabel() const { return label; };
    const wxBitmap &GetBitmap() const { return bmp; };
    const xlColor GetBackgroundColor() const {return backgroundColor;};
protected:
    ButtonTouchBarItemClicked callback;
    std::string label;
    wxBitmap bmp;
    xlColor backgroundColor;
};

class GroupTouchBarItem : public TouchBarItem {
public:
    GroupTouchBarItem(const std::string &n, const std::vector<ButtonTouchBarItem*> i) : TouchBarItem(n), items(i) {}
    virtual ~GroupTouchBarItem();
    
    virtual const std::vector<ButtonTouchBarItem*> &GetItems() { return items; }
protected:
    std::vector<ButtonTouchBarItem*> items;
};

typedef std::function<void(xlColor)> ColorPickerItemChangedFunction;
class ColorPickerItem : public TouchBarItem {
public:
    ColorPickerItem(ColorPickerItemChangedFunction f,const std::string &n) : TouchBarItem(n), callback(f) {}
    virtual ~ColorPickerItem() {};

    void SetColor(const wxBitmap &b, wxColor &c);

    wxBitmap &GetBitmap() { return bmp;}
    wxColor &GetColor() {return color;}
    ColorPickerItemChangedFunction &GetCallback() {return callback;};
private:
    ColorPickerItemChangedFunction callback;
    wxBitmap bmp;
    wxColor color;
};


typedef std::function<void(int)> SliderItemChangedFunction;
class SliderItem : public TouchBarItem {
public:
    SliderItem(SliderItemChangedFunction f,
               const std::string &n,
               int mn, int mx) : TouchBarItem(n), callback(f), value(mn), min(mn), max(mx) {}
    virtual ~SliderItem() {};

    void SetValue(int i);
    int GetValue() const { return value; }
    int GetMin() const { return min;}
    int GetMax() const { return max;}

    SliderItemChangedFunction &GetCallback() {return callback;}
private:
    SliderItemChangedFunction callback;
    int value;
    int min;
    int max;
};


class xlTouchBar : public wxEvtHandler {
public:
    xlTouchBar(xlTouchBarSupport &support);
    virtual ~xlTouchBar();

    virtual void SetActive() { support.SetActive(this); }

    virtual bool IsCustomizable() { return false; }
    virtual const std::vector<TouchBarItem*> &GetItems() { return items; }
    virtual const std::vector<TouchBarItem*> &GetDefaultItems() { return GetItems(); }

    virtual std::string GetName() = 0;
    virtual bool ShowEsc() { return true; }
protected:
    xlTouchBarSupport &support;
    std::vector<TouchBarItem*> items;
};

typedef std::function<void(int, xlColor)> ColorChangedFunction;
class ColorPanelTouchBar : public xlTouchBar {
public:
    ColorPanelTouchBar(ColorChangedFunction f,
                       SliderItemChangedFunction spark,
                       xlTouchBarSupport &support);
    virtual ~ColorPanelTouchBar();

    virtual std::string GetName() override { return "ColorBar";}

    void SetColor(int idx, const wxBitmap &bmp, wxColor &c);
    void SetSparkles(int v);

    virtual void SetActive() override;

private:
    ColorChangedFunction colorCallback;
    SliderItemChangedFunction sparkCallback;
    xlTouchBar *lastBar;
    bool inCallback;
};


class EffectGridTouchBar : public xlTouchBar {
public:
    EffectGridTouchBar(xlTouchBarSupport &support, const EffectManager &m, MainSequencer *seq, ColorPanelTouchBar *colorBar);
    virtual ~EffectGridTouchBar();

    virtual std::string GetName() override { return "EffectGrid";}
    virtual bool IsCustomizable() override { return true; }

private:
    ColorPanelTouchBar *colorBar;
    const EffectManager &manager;
};
