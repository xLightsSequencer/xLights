#ifndef SIMPLEFTP_H
#define SIMPLEFTP_H

#include <wx/socket.h>
#include <wx/protocol/ftp.h>
#include <wx/progdlg.h>

class SimpleFTP
{
    wxFTP ftp;

public:
    SimpleFTP();
    SimpleFTP(std::string ip, std::string user, std::string password);
    bool Connect(std::string ip, std::string user, std::string password);
    bool IsConnected();
    bool UploadFile(std::string file, std::string folder, std::string newfilename, bool backup, bool binary, wxWindow* parent);
    ~SimpleFTP();
};

#endif
