#include "DimmingCurve.h"

#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

class BaseDimmingCurve : public DimmingCurve {
public:
    BaseDimmingCurve(int ch = -1) : DimmingCurve(), channel(ch)  {
        for (int x = 0; x < 256; x++) {
            data[x] = x;
        }
    }
    virtual ~BaseDimmingCurve() {}

    
    virtual void apply(xlColor &c) {
        switch (channel) {
            case -1:
                c.green = data[c.green];
                c.blue = data[c.blue];
                c.red = data[c.red];
                break;
            case 0:
                c.red = data[c.red];
                break;
            case 1:
                c.green = data[c.green];
                break;
            case 2:
                c.blue = data[c.blue];
                break;
            case 4:
                if (c.blue == c.red && c.green == c.red) {
                    c.green = c.blue = c.red = data[c.red];
                }
                break;
        }
    }
    
    int channel;
    unsigned char data[256];
};


class BasicDimmingCurve : public BaseDimmingCurve {
public:
    BasicDimmingCurve(int ch = -1) : BaseDimmingCurve(ch)  {
    }
    BasicDimmingCurve(int brightness, float gamma, int ch = -1) : BaseDimmingCurve(ch) {
        init(brightness, gamma);
    }
    virtual ~BasicDimmingCurve() {}

    void init(int brightness, float gamma) {
        for (int x = 0; x < 256; x++) {
            float i = x;
            i = i * float(brightness + 100) / 100.0;
            i = 255 * powf(i / 255.0, gamma);
            if (i > 255) {
                i = 255;
            }
            if (i < 0) {
                i = 0;
            }
            data[x] = i;
        }
    }
};

class FileDimmingCurve : public BaseDimmingCurve {
public:
    FileDimmingCurve(const wxString &f, int ch = -1) : BaseDimmingCurve(ch) {
        wxFileInputStream fin(f);
        wxTextInputStream text(fin);
        
        int count = 0;
        while(fin.Eof() == false){
            wxString data = text.ReadLine();
            if (data != "") {
                data[count] = wxAtoi(data);
                count++;
                if (count == 256) {
                    return;
                }
            }
        }
    }
};

class CompositeDimmingCurve : public DimmingCurve {
public:
    CompositeDimmingCurve(DimmingCurve *r, DimmingCurve *g, DimmingCurve *b) : DimmingCurve(), red(r), green(g), blue(b) {
    }
    virtual ~CompositeDimmingCurve() {
        if (red != nullptr) {
            delete red;
        }
        if (green != nullptr) {
            delete green;
        }
        if (blue != nullptr) {
            delete blue;
        }
    }
    virtual void apply(xlColor &c) {
        if (red != nullptr) {
            red->apply(c);
        }
        if (green != nullptr) {
            green->apply(c);
        }
        if (blue != nullptr) {
            blue->apply(c);
        }
    }
    DimmingCurve *red;
    DimmingCurve *green;
    DimmingCurve *blue;
};

DimmingCurve::DimmingCurve()
{
}

DimmingCurve::~DimmingCurve()
{
}

DimmingCurve *createCurve(wxXmlNode *dc, int channel = -1) {
    if (dc->HasAttribute("filename")) {
        wxString fn = dc->GetAttribute("filename");
        if (wxFile::Exists(fn)) {
            return new FileDimmingCurve(fn);
        }
    } else {
        return new BasicDimmingCurve(wxAtoi(dc->GetAttribute("brightness", "0")),
                                     wxAtof(dc->GetAttribute("gamma", "1.0")));
    }
    return nullptr;
}

DimmingCurve *DimmingCurve::createFromXML(wxXmlNode *node) {
    DimmingCurve *red = nullptr;
    DimmingCurve *green = nullptr;
    DimmingCurve *blue = nullptr;
    
    wxXmlNode *dc = node->GetChildren();
    while (dc != nullptr) {
        wxString name = dc->GetName();
        if ("all" == name) {
            return createCurve(dc);
        } else if ("red" == dc->GetName()) {
            red = createCurve(dc, 0);
        } else if ("green" == dc->GetName()) {
            green = createCurve(dc, 1);
        } else if ("blue" == dc->GetName()) {
            blue = createCurve(dc, 2);
        }
        dc = dc->GetNext();
    }
    if (red != nullptr || blue != nullptr || green != nullptr) {
        return new CompositeDimmingCurve(red, green, blue);
    }
    return nullptr;
}

DimmingCurve *DimmingCurve::createBrightnessGamma(int brightness, float gamma) {
    BasicDimmingCurve *c = new BasicDimmingCurve(brightness, gamma);
    return c;
}
DimmingCurve *DimmingCurve::createFromFile(const wxString &fileName) {
    if (wxFile::Exists(fileName)) {
        return new FileDimmingCurve(fileName);
    }
    return new BasicDimmingCurve(100, 1.0);
}
