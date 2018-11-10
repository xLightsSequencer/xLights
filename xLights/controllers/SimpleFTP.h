#ifndef SIMPLEFTP_H
#define SIMPLEFTP_H

#include <wx/socket.h>
#include <wx/protocol/ftp.h>
#include <wx/progdlg.h>

class SimpleFTP
{
    wxFTP _ftp;
    std::string _ip;

public:
    SimpleFTP();
    SimpleFTP(std::string ip, std::string user, std::string password);
    ~SimpleFTP();
    std::string Pwd();
    bool Connect(std::string ip, std::string user, std::string password);
    bool IsConnected() const;
    bool UploadFile(std::string file, std::string folder, std::string newfilename, bool backup, bool binary, wxWindow* parent);
    bool GetFile(std::string targetfile, std::string folder, std::string file, bool binary, wxWindow* parent);

};

#endif
