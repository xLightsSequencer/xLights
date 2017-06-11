#include "SimpleFTP.h"
#include <log4cpp/Category.hh>
#include <wx/progdlg.h>
#include <wx/msgdlg.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/log.h>
#include <wx/sckstrm.h>

class MySocketOutputStream : public wxSocketOutputStream {
public:
    MySocketOutputStream(wxSocketBase &tmp, MySocketOutputStream *s) : wxSocketOutputStream(tmp) {
        s->m_o_socket->SetFlags(wxSOCKET_NOWAIT_WRITE);
    }
};

SimpleFTP::SimpleFTP()
{
    ftp.SetFlags(wxSOCKET_NOWAIT_WRITE);
}

bool SimpleFTP::Connect(std::string ip, std::string user, std::string password)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    ftp.SetUser(user);
    ftp.SetPassword(password);
    ftp.SetTimeout(5);
    if (!ftp.Connect(ip))
    {
        logger_base.warn("Could not connect using address '%s'.", (const char *)ip.c_str());
        wxString wxip = wxString(ip.c_str());
        wxMessageBox("Could not connect using address '" + wxip + "'.");
        return false;
    }

    return true;
}

SimpleFTP::SimpleFTP(std::string ip, std::string user, std::string password)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    ftp.SetFlags(wxSOCKET_NOWAIT_WRITE);
    ftp.SetUser(user);
    ftp.SetPassword(password);
    if (!ftp.Connect(ip))
    {
        logger_base.warn("Could not connect using address '%s'.", (const char *)ip.c_str());
        wxString wxip = wxString(ip.c_str());
        wxMessageBox("Could not connect using address '" + wxip + "'.");
    }
}

bool SimpleFTP::IsConnected()
{
    return (ftp.IsConnected() && ftp.IsOk() && !ftp.IsClosed());
}

bool SimpleFTP::UploadFile(std::string file, std::string folder, std::string newfilename, bool backup, bool binary, wxWindow* parent)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (!IsConnected()) return false;

    if (newfilename == "") newfilename = file;

    ftp.ChDir(folder);

    if (binary)
    {
        ftp.SetBinary();
    }
    else
    {
        ftp.SetAscii();
    }

    wxProgressDialog progress("FTP Upload", wxString(file.c_str()), 100, parent, wxPD_CAN_ABORT | wxPD_APP_MODAL | wxPD_AUTO_HIDE);
    progress.Show();

    bool cancelled = false;

    progress.Update(0, wxEmptyString, &cancelled);

    wxFileName fn(wxString(file.c_str()));
    wxString ext = fn.GetExt();
    if (ext != "") ext = "." + ext;
    wxString basefile = fn.GetName() + ext;
    wxFileName fnnew(wxString(newfilename.c_str()));
    wxString extnew = fnnew.GetExt();
    if (extnew != "") extnew = "." + extnew;
    wxString basefilenew = fnnew.GetName() + extnew;

    wxLogNull logNo; //kludge: avoid "error 0" message from wxWidgets

    //logger_base.info("FTP current directory %s.", (const char *)ftp.Pwd().c_str());

    if (backup)
    {
        int size = ftp.GetFileSize((folder + "/" + basefile).c_str());
        if (size == -1)
        {
            // file not there so no need to backup
        }
        else
        {
            wxDateTime dt = wxDateTime::Now();
            wxString tgtfile = wxString((folder + "/" + basefilenew).c_str()) + "_" + dt.Format("%Y%m%d_%H%M%S");
            logger_base.info("FTP Backing up file %s to %s.", (const char *)(folder + "/" + basefilenew).c_str(), (const char *)tgtfile.c_str());
            ftp.Rename((folder + "/" + basefilenew).c_str(), tgtfile);
            if (!cancelled)
            {
                cancelled = progress.WasCancelled();
            }
        }
    }

    wxFile in;
    in.Open(wxString(file.c_str()));
    if (in.IsOpened())
    {
        logger_base.info("FTP Uploading file %s to %s.", (const char *)file.c_str(), (const char *)(folder + "/" + basefilenew).c_str());
        wxFileOffset length = in.Length();
        wxFileOffset done = 0;
        wxSocketOutputStream *out = dynamic_cast<wxSocketOutputStream*>(ftp.GetOutputStream((folder + "/" + basefilenew).c_str()));
        wxSocketBase sock;
        MySocketOutputStream sout(sock, (MySocketOutputStream*)out);
        if (out != nullptr && length > 0)
        {
            uint8_t buffer[8192]; // 8KB at a time
            int lastDone = 0;
            while (!in.Eof() && !cancelled)
            {
                ssize_t read = in.Read(&buffer[0], sizeof(buffer));
                wxASSERT(read <= sizeof(buffer));
                done += read;
                //logger_base.debug("   FTP Upload read %lu so far %lu.", (unsigned long)read, (unsigned long)done);

                int bufPos = 0;
                while (read) {
                    out->Write(&buffer[bufPos], read);
                    size_t written = out->LastWrite();
                    bufPos += written;
                    read -= written;
                    //logger_base.debug("   FTP Upload wrote %lu left %lu.", (unsigned long)written, (unsigned long)read);
                }

                int donePct = (int)(done * 100 / length);
                if (donePct != lastDone) {
                    wxASSERT(donePct > lastDone);
                    lastDone = donePct;
                    cancelled = !progress.Update(donePct, wxEmptyString, &cancelled);
                    wxYield();
                }
            }

            if (length == 0)
            {
                progress.Update(100, wxEmptyString, &cancelled);
                logger_base.error("   FTP Upload of file %s failed due to zero length.", (const char *)file.c_str());
            }
            else if (in.Eof())
            {
                progress.Update(100, wxEmptyString, &cancelled);
                logger_base.info("   FTP Upload of file %s done.", (const char *)file.c_str());
            }
            else
            {
                progress.Update(100, wxEmptyString, &cancelled);
                logger_base.warn("   FTP Upload of file %s cancelled.", (const char *)file.c_str());
            }

            in.Close();
            out->Close();
            delete out;
            out = nullptr;
            if (ftp.GetFileSize((folder + "/" + basefilenew).c_str()) != length)
            {
                logger_base.warn("   FTP Upload of file %s failed. Source size (%d) != Destination Size (%d)", (const char *)file.c_str(), length, ftp.GetFileSize((folder + "/" + basefile).c_str()));
            }
        }
        else
        {
            wxMessageBox("FTP Upload of file failed to create the target file.");
            progress.Update(100, wxEmptyString, &cancelled);
            logger_base.error("   FTP Upload of file %s failed as file %s could not be created on FPP.", (const char *)file.c_str(), (const char *)(folder + "/" + basefilenew).c_str());
        }
    }
    else
    {
        wxMessageBox("FTP Upload of file failed to open the file.");
        progress.Update(100, wxEmptyString, &cancelled);
        logger_base.error("   FTP Upload of file %s failed as file could not be opened.", (const char *)file.c_str());
    }

    if (!cancelled)
    {
        cancelled = progress.WasCancelled();
    }

    return cancelled;
}

SimpleFTP::~SimpleFTP()
{
    ftp.Close();
}
