#include "EffectDropTarget.h"
#include <wx/dnd.h>
#include "wx/wx.h"
#include "EffectsGrid.h"

#include "effects/PicturesEffect.h"
#include "effects/VideoEffect.h"
#include "effects/GlediatorEffect.h"

EffectDropTarget::EffectDropTarget(EffectsGrid* parent)
{
    //ctor
    mParent = parent;

    wxDataObjectComposite* dataobj = new wxDataObjectComposite();
    dataobj->Add(new wxTextDataObject(), true);
    dataobj->Add(new wxFileDataObject());
    SetDataObject(dataobj);
}

EffectDropTarget::~EffectDropTarget()
{
    //dtor
}

bool EffectDropTarget::OnDrop(wxCoord x, wxCoord y)
{
    if (GetData())
    {
        wxDataObjectComposite *
            dataobjComp = static_cast<wxDataObjectComposite *>(GetDataObject());
        wxDataFormat format = dataobjComp->GetReceivedFormat();
        wxDataObject *dataobj = dataobjComp->GetObject(format);
        if (format.GetType() == wxDF_FILENAME)
        {
            wxFileDataObject *
                dataobjFile = static_cast<wxFileDataObject *>(dataobj);
            wxArrayString filenames = dataobjFile->GetFilenames();
            mParent->OnDropFiles(x, y, filenames);
            return true;
        }
    }

    mParent->OnDrop(x,y);
    return true;
}

bool EffectDropTarget::OnDropText(wxCoord x, wxCoord y,const wxString &data )
{
    return true;
}

wxDragResult EffectDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)
{
    // check if we can drag here
    if(mParent->DragOver(x,y)) {
        // check what is being dragged is ok
        return OnData(x ,y, wxDragNone);
        //return wxDragCopy;
    }

    return wxDragNone;
}

wxDragResult EffectDropTarget::OnData(wxCoord x, wxCoord y, wxDragResult def)
{
    if (!GetData())
        return wxDragNone;
    wxDataObjectComposite *
        dataobjComp = static_cast<wxDataObjectComposite *>(GetDataObject());
    wxDataFormat format = dataobjComp->GetReceivedFormat();
    wxDataObject *dataobj = dataobjComp->GetObject(format);
    switch (format.GetType())
    {
    case wxDF_FILENAME:
    {
        wxFileDataObject *
            dataobjFile = static_cast<wxFileDataObject *>(dataobj);
        auto filenames = dataobjFile->GetFilenames();

        for (auto fn = filenames.begin(); fn != filenames.end(); ++fn)
        {
            if (PicturesEffect::IsPictureFile(fn->ToStdString()) ||
                VideoEffect::IsVideoFile(fn->ToStdString()) ||
                GlediatorEffect::IsGlediatorFile(fn->ToStdString()))
            {
                return wxDragCopy;
            }
        }
    }
    break;
    case wxDF_UNICODETEXT:
    case wxDF_TEXT:
    {
        return wxDragCopy;
    }
    default:
        wxFAIL_MSG("unexpected data object format");
    }
    return def;
}
