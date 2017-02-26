#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <string>
#include <list>

class wxXmlNode;

class Projector
{
	std::string _name;
  std::string _ip;
  std::string _password;
	int _changeCount;
	int _lastSavedChangeCount;

    void Load(wxXmlNode* node);

    public:

		Projector(wxXmlNode* node);
		Projector();
		virtual ~Projector() {}
		wxXmlNode* Save();
    bool IsDirty() const { return _changeCount != _lastSavedChangeCount; }
    void ClearDirty() { _lastSavedChangeCount = _changeCount; }
		void SetName(const std::string& name) { if (_name != name) { _name = name; _changeCount++; } }
    void SetIP(const std::string& ip) { if (_ip != ip) { _ip = ip; _changeCount++; } }
    void SetPassword(const std::string& password) { if (_password != password) { _password = password; _changeCount++; } }
		std::string GetName() const { return _name; }
    std::string GetIP() const { return _ip; }
    std::string GetPassword() const { return _password; }
};

#endif 
