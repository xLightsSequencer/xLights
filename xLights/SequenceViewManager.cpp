#include "SequenceViewManager.h"
#include <wx/wx.h>
#include "models/ModelManager.h"
#include "models/Model.h"
#include <log4cpp/Category.hh>

#define MASTER_VIEW "Master View"

#pragma region SequenceView

SequenceView::SequenceView(const std::string& name, ModelManager* modelManager)
{
	_modelManager = modelManager;
	_name = name;
}

SequenceView::SequenceView(wxXmlNode* node, ModelManager* modelManager)
{
	_modelManager = modelManager;
	Load(node);
}

void SequenceView::Load(wxXmlNode* node)
{
	_name = node->GetAttribute("name").ToStdString();
	SetModels(node->GetAttribute("models").ToStdString());
}

void SequenceView::SetModels(const std::string& models)
{
	_modelNames.clear();

	if (models == "") return;

	auto ms = wxSplit(models, ',');
	for (auto m = ms.begin(); m != ms.end(); ++m)
	{
		_modelNames.push_back(m->ToStdString());
	}
}

void SequenceView::AddModel(const std::string& model, int pos)
{
	if (std::find(_modelNames.begin(), _modelNames.end(), model) == _modelNames.end())
	{
        if (pos == -1)
        {
            _modelNames.push_back(model);
        }
        else
        {
            auto it = _modelNames.begin();
            for (int i = 0; i < pos && it != _modelNames.end(); ++i)
            {
                ++it;
            }
            _modelNames.insert(it, model);
        }
	}
}

void SequenceView::AddModel(const Model* model, int pos)
{
	AddModel(model->GetName(), pos);
}

void SequenceView::MoveModelAfter(const std::string& model, const std::string& after)
{
	RemoveModel(model);
	InsertModelAfter(model, after);
}

void SequenceView::MoveModelBefore(const std::string& model, const std::string& before)
{
    RemoveModel(model);
    InsertModelBefore(model, before);
}

void SequenceView::InsertModelAfter(const std::string& model, const std::string& after)
{
	if (ContainsModel(model))
	{
		RemoveModel(model);
	}

	if (after == "")
	{
		_modelNames.push_front(model);
	}
	else
	{
		for (auto it = _modelNames.begin(); it != _modelNames.end(); ++it)
		{
			if (*it == after)
			{
				++it;
				_modelNames.insert(it, model);
				return;
			}
		}

		// didnt find after so just add it to the end
		AddModel(model);
	}
}

void SequenceView::InsertModelBefore(const std::string& model, const std::string& before)
{
    if (ContainsModel(model))
    {
        RemoveModel(model);
    }

    if (before == "")
    {
        _modelNames.push_back(model);
    }
    else
    {
        for (auto it = _modelNames.begin(); it != _modelNames.end(); ++it)
        {
            if (*it == before)
            {
                _modelNames.insert(it, model);
                return;
            }
        }

        // didnt find after so just add it to the end
        AddModel(model);
    }
}

void SequenceView::RemoveModel(const std::string& model)
{
	_modelNames.remove(model);
}

void SequenceView::RemoveModel(const Model* model)
{
	RemoveModel(model->GetName());
}

void SequenceView::Save(wxXmlNode* node) const
{
	wxXmlNode* newnode = Save();
	node->AddChild(newnode);
}

void SequenceView::RenameModel(const std::string& oldname, const std::string& newname)
{
	if (ContainsModel(oldname))
	{
		RemoveModel(oldname);
		AddModel(newname);
	}
}

bool SequenceView::ContainsModel(const std::string& model)
{
	return (std::find(_modelNames.begin(), _modelNames.end(), model) != _modelNames.end());
}

std::string SequenceView::GetModelsString() const
{
	std::string models = "";
	for (auto it = _modelNames.begin(); it != _modelNames.end(); ++it)
	{
		if (models != "") models += ",";
		models += *it;
	}

	return models;
}

wxXmlNode* SequenceView::Save() const
{
	wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "view");
	node->AddAttribute("name", _name);

	node->AddAttribute("models", GetModelsString());

	return node;
}

#pragma  endregion 

SequenceViewManager::SequenceViewManager(ModelManager* modelManager)
{
    _selectedView = 0;
	_modelManager = modelManager;
}

SequenceViewManager::~SequenceViewManager()
{
	Reset();
}

void SequenceViewManager::Load(wxXmlNode* node, int selectedView)
{
	wxASSERT(_modelManager != nullptr);

	_selectedView = selectedView;

	AddMasterView();

	for (wxXmlNode* view = node->GetChildren(); view != nullptr; view = view->GetNext())
	{
		_views.push_back(new SequenceView(view, _modelManager));
	}

	if (GetView(_selectedView) == nullptr)
	{
		_selectedView = 0;
	}
}

void SequenceViewManager::AddMasterView()
{
	if (GetView(MASTER_VIEW) == nullptr)
	{
		_views.push_back(new SequenceView(MASTER_VIEW, _modelManager));
	}
}

void SequenceViewManager::Save(wxXmlDocument* doc)
{
	wxXmlNode* palette = nullptr;
	wxXmlNode* view = nullptr;

	// find an existing view node in the document and delete it
	for (wxXmlNode* e = doc->GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
	{
		if (e->GetName() == "palettes") palette = e;
		if (e->GetName() == "views") view = e;
	}
	if (view != nullptr)
	{
		doc->GetRoot()->RemoveChild(view);
	}

	wxXmlNode* newnode = Save();

	// add it after the palette if one is there. If not at the end.
	if (palette != nullptr)
	{
		doc->GetRoot()->InsertChild(newnode, palette);
	}
	else
	{
		doc->GetRoot()->AddChild(newnode);
	}
}

wxXmlNode* SequenceViewManager::Save() const
{
	wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "views");

	for (auto it = _views.begin(); it != _views.end(); ++it)
	{
		if ((*it)->GetName() != MASTER_VIEW)
		{
			node->AddChild((*it)->Save());
		}
	}

	return node;
}

SequenceView* SequenceViewManager::AddView(const std::string& name)
{
	wxASSERT(_modelManager != nullptr);

	AddMasterView();

	SequenceView* view = GetView(name);

	if (view == nullptr)
	{
		view = new SequenceView(name, _modelManager);
		_views.push_back(view);
	}
	return view;
}

void SequenceViewManager::DeleteView(const std::string& name)
{
	SequenceView* view = GetView(name);
	if (view != nullptr)
	{
		DeleteView(view);
	}
}

void SequenceViewManager::DeleteView(SequenceView* view)
{
	_views.remove(view);
}

void SequenceViewManager::RenameView(const std::string& oldname, const std::string& newname)
{
	SequenceView* view = GetView(oldname);

	if (view != nullptr) view->SetName(newname);
}

void SequenceViewManager::RenameModel(const std::string& oldname, const std::string& newname)
{
	for (auto it = _views.begin(); it != _views.end(); ++it)
	{
		(*it)->RenameModel(oldname, newname);
	}
}

void SequenceViewManager::DeleteModel(const std::string& model)
{
	for (auto it = _views.begin(); it != _views.end(); ++it)
	{
		(*it)->RemoveModel(model);
	}
}

SequenceView* SequenceViewManager::GetView(const std::string& name) const
{
	for (auto it = _views.begin(); it != _views.end(); ++it)
	{
		if ((*it)->GetName() == name) return *it;
	}

	return nullptr;
}

int SequenceViewManager::GetViewIndex(const std::string& name) const
{
	int index = 0;
	for (auto it = _views.begin(); it != _views.end(); ++it)
	{
		if ((*it)->GetName() == name) return index;
		index++;
	}

	return -1;
}

SequenceView* SequenceViewManager::GetView(int index) const
{
	int count = 0;
	for (auto it = _views.begin(); it != _views.end(); ++it)
	{
		if (count == index) return *it;
		count++;
	}

	return nullptr;
}

void SequenceViewManager::Reset()
{
	while (_views.size() > 0)
	{
		auto todelete = _views.front();
		_views.pop_front();
		delete todelete;
	}
}

void SequenceViewManager::SetSelectedView(int view)
{
	if (view >= _views.size())
	{
		_selectedView = 0;
	}
	else
	{
		_selectedView = view;
	}
}
