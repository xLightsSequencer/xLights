#ifndef SEQUENCEVIEWMANAGER_H
#define SEQUENCEVIEWMANAGER_H

#include <wx/xml/xml.h>
#include <list>
#include <string>

class ModelManager;
class Model;

class SequenceView
{
	ModelManager* _modelManager;
	std::string _name;
	std::list<std::string> _modelNames;

public:
	SequenceView(const std::string& name, ModelManager* modelManager);
	SequenceView(wxXmlNode* node, ModelManager* modelManager);
	virtual ~SequenceView() {};
	void Load(wxXmlNode* node);
	void Save(wxXmlNode* node) const;
	wxXmlNode* Save() const;
	std::list<std::string> GetModels() const { return _modelNames; }
	std::string GetName() const { return _name; }
	void SetName(const std::string& name) { _name = name; }
	void AddModel(const std::string& model, int pos = -1);
	void AddModel(const Model* model, int pos = -1);
	void RemoveModel(const std::string& model);
	void RemoveModel(const Model* model);
	void RenameModel(const std::string& oldname, const std::string& newname);
	bool ContainsModel(const std::string& model);
	std::string GetModelsString() const;
    void MoveModelAfter(const std::string& model, const std::string& after);
    void MoveModelBefore(const std::string& model, const std::string& before);
    void InsertModelAfter(const std::string& model, const std::string& after);
    void InsertModelBefore(const std::string& model, const std::string& before);
	void SetModels(const std::string& models);
};

class SequenceViewManager
{
	ModelManager* _modelManager;
	std::list<SequenceView*> _views;
	int _selectedView;

	void AddMasterView();

public:
	SequenceViewManager() { _modelManager = nullptr; _selectedView = 0; };
	SequenceViewManager(ModelManager* modelManager);
	virtual ~SequenceViewManager();
	std::list<SequenceView*> GetViews() { AddMasterView(); return _views; }
	void SetModelManager(ModelManager* modelManager) { _modelManager = modelManager; }
	void Load(wxXmlNode* node, int selectedView);
	void Save(wxXmlDocument* doc);
	wxXmlNode* Save() const;
	int GetSelectedViewIndex() const { return _selectedView; }
	SequenceView* GetSelectedView() const { return GetView(_selectedView); }
	SequenceView* AddView(const std::string& name);
	void DeleteView(const std::string& name);
	void DeleteView(SequenceView* view);
	void RenameView(const std::string& oldname, const std::string& newname);
	SequenceView* GetView(const std::string& name) const;
	int GetViewIndex(const std::string& name) const;
	SequenceView* GetView(int index) const;
	void RenameModel(const std::string& oldname, const std::string& newname);
	void DeleteModel(const std::string& model);
	void Reset();
    int GetViewCount() const { return _views.size(); }

protected:
    friend class SequenceElements;
    // This is in protected section with SequenceElements as a friend class.
    // Don't call this directly to set the view call SetCurrentView in
    // SequenceElements and let it call this method.
    void SetSelectedView(int view);
};

#endif
