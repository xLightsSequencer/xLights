#include "EasyLights.h"
#include <wx/msgdlg.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include <wx/progdlg.h>

#include "../models/Model.h"
#include "../outputs/OutputManager.h"
#include "../outputs/Output.h"
#include "../models/ModelManager.h"
#include "ControllerUploadData.h"
#include "UtilFunctions.h"
#include "ControllerRegistry.h"

#include <log4cpp/Category.hh>

EasyLights_Network_Communication::EasyLights_Network_Communication()
{
	Xlights_IP_DW = 0;
	udp = nullptr;
	Reset_Complete = 0;
    _firmwareVersion = 0;
    _Board_type = 0;
    Start_Of_Recv_Message = nullptr;
    Reply = nullptr;
}

EasyLights_Network_Communication::~EasyLights_Network_Communication()
{
	if(udp)
	{
		udp->Destroy();
		udp = nullptr;
	}
}

bool EasyLights_Network_Communication::Send_UDP_Packet_Acquire_Reply(wxByte *B, int SZ)
{
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	int Actuall_Sent = 0;

    while (true)
	{
		udp->SendTo(controller, (const void*)&B[Actuall_Sent], (SZ - Actuall_Sent));

		int r = udp->LastError();
		if(r)
		{
			logger_base.error("Error Transmit/Connecting to EasyLights  UDP Error %d : %s. %s", udp->LastError(), (const char *)DecodeIPError(udp->LastError()).c_str(), (const char *)controller_ip.c_str());
			return 1;
		}

		Actuall_Sent += udp->LastWriteCount();

		if(Actuall_Sent >= SZ)
			break;
	}

	return Get_Controller_Reply();
}


// all return true if failure detected - false = complete nor errors
bool EasyLights_Network_Communication::Initial_Connection(const std::string& ip, wxByte *Board_type, wxUint16 *firmwareVersion)
{
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	logger_base.debug("Connecting to EasyLights on %s.", (const char *)controller_ip.c_str());
	logger_base.debug("Xlights Local IP %s.", (const char *)Xlights_IP.c_str());

	controller_ip = ip;

	wxIPV4address addr;
	addr.Hostname(wxGetFullHostName());
	Xlights_IP = addr.IPAddress();

	Xlights_IP_DW = Convert_IP_String((const char *)Xlights_IP.c_str());

	controller.Hostname(controller_ip);
	controller.Service(Controller_Recv_Port);

	udp = new wxDatagramSocket(addr, wxSOCKET_NONE);
	if(udp == nullptr)
	{
		logger_base.error("Error initializing EasyLights datagram. %s", (const char *)Xlights_IP.c_str());
		return true;
	}
	else if(!udp->IsOk())
	{
		logger_base.error("Error initializing EasyLights datagram ... is network connected? OK: FALSE %s", (const char *)Xlights_IP.c_str());
		udp->Destroy();
		udp = nullptr;
		return true;
	}
	else if(udp->Error() != wxSOCKET_NOERROR)
	{
		logger_base.error("Error creating EasyLights datagram => %d : %s. %s", udp->LastError(), (const char *)DecodeIPError(udp->LastError()).c_str(), (const char *)Xlights_IP.c_str());
		udp->Destroy();
		udp = nullptr;
		return true;
	}

	udp->SetTimeout(3);

	wxByte B[6 + Size_struct_Tag_UDP_Packet_Request]; // 6 for 3 byte header and trailer
	int AbsSize = sizeof(B);

    wxByte *M = Build_Output_UDP_Header((wxByte*)B, AbsSize);

	//M->MSG_Size = 0;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_MSG_Size, 0);

	//M->AbsoluteMsgSize = AbsSize;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_AbsoluteMsgSize, AbsSize);

	//M->Xlights_IP = Xlights_IP_DW;
	Set_Network_Pack_dword(M, Tag_UDP_Packet_Request_Offset_Xlights_IP, Xlights_IP_DW);

	//M->CMD = UDP_CMD_Xlights_Connect;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_CMD, UDP_CMD_Xlights_Connect);

	//M->Index = 0;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Index, 0);

	int r = Send_UDP_Packet_Acquire_Reply(B, AbsSize);

	if(r == 0)
	{
		*Board_type = _Board_type;
		*firmwareVersion = _firmwareVersion;
	}
	else
	{
		udp->Destroy();
		udp = nullptr;
	}

	return r;
}

bool EasyLights_Network_Communication::Command_Reset_Controller(const std::string& ip)
{
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	logger_base.debug("Resetting EasyLights on %s.", (const char *)controller_ip.c_str());
	logger_base.debug("Xlights Local IP %s.", (const char *)Xlights_IP.c_str());

	controller_ip = ip;

	wxIPV4address addr;
	addr.Hostname(wxGetFullHostName());
	Xlights_IP = addr.IPAddress();

	Xlights_IP_DW = Convert_IP_String((const char *)Xlights_IP.c_str());

	controller.Hostname(controller_ip);
	controller.Service(Controller_Recv_Port);

	udp = new wxDatagramSocket(addr, wxSOCKET_NONE);
	if(udp == nullptr)
	{
		logger_base.error("Error initializing EasyLights datagram. %s", (const char *)Xlights_IP.c_str());
		return true;
	}
	else if(!udp->IsOk())
	{
		logger_base.error("Error initializing EasyLights datagram ... is network connected? OK: FALSE %s", (const char *)Xlights_IP.c_str());
		udp->Destroy();
		udp = nullptr;
		return true;
	}
	else if(udp->Error() != wxSOCKET_NOERROR)
	{
		logger_base.error("Error creating EasyLights datagram => %d : %s. %s", udp->LastError(), (const char *)DecodeIPError(udp->LastError()).c_str(), (const char *)Xlights_IP.c_str());
		udp->Destroy();
		udp = nullptr;
		return true;
	}

	udp->SetTimeout(3);

	wxByte B[6 + Size_struct_Tag_UDP_Packet_Request]; // 6 for 3 byte header and trailer
	int AbsSize = sizeof(B);

    wxByte *M = Build_Output_UDP_Header((wxByte*)B, AbsSize);

	//M->MSG_Size = 0;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_MSG_Size, 0);

	//M->AbsoluteMsgSize = AbsSize;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_AbsoluteMsgSize, AbsSize);

	//M->Xlights_IP = Xlights_IP_DW;
	Set_Network_Pack_dword(M, Tag_UDP_Packet_Request_Offset_Xlights_IP, Xlights_IP_DW);

	//M->CMD = UDP_CMD_Xlights_Reset;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_CMD, UDP_CMD_Xlights_Reset);

	//M->Index = 0;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Index, 0);

	int r = Send_UDP_Packet_Acquire_Reply(B, AbsSize);

	udp->Destroy();
	udp = nullptr;

	if(r == 0)
		Reset_Complete = 1;

	return r;
}

bool EasyLights_Network_Communication::Send_Universe_Data(std::vector<EasyLights_E131*>& e131)
{
	memset(Xmit_Buffer, 0, Controller_MTU_Size);

	int MessageSize = ((e131.size() * 2) + 1) * sizeof(wxUint16);	// we send array of 16 bit words

	wxUint16 *Wptr = (wxUint16*)(3 + Size_struct_Tag_UDP_Packet_Request + Xmit_Buffer);

	// first 2 locations hold protocal and size, rest alternamt univ then chan

	*Wptr++ = e131.size();

    for(int i = 0; i < e131.size(); i++)
	{
		*Wptr++ = e131[i]->Univ;
		*Wptr++ = e131[i]->Channels;
	}

	int AbsSize = Size_struct_Tag_UDP_Packet_Request + 6 + MessageSize;

    wxByte *M = Build_Output_UDP_Header((wxByte*)Xmit_Buffer, AbsSize);

	//M->MSG_Size = MessageSize;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_MSG_Size, MessageSize);

	//M->AbsoluteMsgSize = AbsSize;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_AbsoluteMsgSize, AbsSize);

	//M->Xlights_IP = Xlights_IP_DW;
	Set_Network_Pack_dword(M, Tag_UDP_Packet_Request_Offset_Xlights_IP, Xlights_IP_DW);

	//M->CMD = UDP_CMD_Xlights_Universes_IN;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_CMD, UDP_CMD_Xlights_Universes_IN);

	//M->Index = 0;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Index, 0);

	int r = Send_UDP_Packet_Acquire_Reply(Xmit_Buffer, AbsSize);

	return r;
}

bool EasyLights_Network_Communication::Get_Port_Config_Rows(std::vector<std::string>&  Row_Data)
{
	wxByte B[6 + Size_struct_Tag_UDP_Packet_Request];

	int AbsSize = Size_struct_Tag_UDP_Packet_Request + 6;

    int Row_Index = 0;

    wxByte *M = Build_Output_UDP_Header((wxByte*)B, AbsSize);


	//M->MSG_Size = 0;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_MSG_Size, 0);

	//M->AbsoluteMsgSize = AbsSize;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_AbsoluteMsgSize, AbsSize);

	//M->Xlights_IP = Xlights_IP_DW;
	Set_Network_Pack_dword(M, Tag_UDP_Packet_Request_Offset_Xlights_IP, Xlights_IP_DW);

	//M->CMD = UDP_CMD_Xlights_Get_Port_Config_Row;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_CMD, UDP_CMD_Xlights_Get_Port_Config_Row);

	while (true)
	{
		//M->Index = Row_Index;
		Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Index, Row_Index);

		int r = Send_UDP_Packet_Acquire_Reply(B, AbsSize);
		if(r)
			return 1;

		// see if we have data or at end of available rows;
		Reply = &Rcv_Buffer[3];

		if(Get_Network_Pack_word(Reply, Tag_UDP_Packet_Reply_Offset_Reply))	// we have data
		{
			Row_Data.push_back((char*)Start_Of_Recv_Message);
		}
		else
			return false;

		Row_Index++;
	}

	return true;
}

bool EasyLights_Network_Communication::Send_Port_Config_Rows(std::vector<EasyLightsString*> MystringData)
{
    memset(Xmit_Buffer, 0, Controller_MTU_Size);
    char *SOM = (char*)(3 + Size_struct_Tag_UDP_Packet_Request + Xmit_Buffer);

	for(int i = 0; i < MystringData.size(); i++)
	{
		sprintf(SOM, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
			MystringData[i]->Port_Segment,
			MystringData[i]->StringType,
			MystringData[i]->Segmented,
			MystringData[i]->StartChan,
			MystringData[i]->PixelCount,
			MystringData[i]->GroupCount,
			MystringData[i]->EndChan,
			MystringData[i]->Direction,
			MystringData[i]->RGB_Order,
			MystringData[i]->NullPixels,
			MystringData[i]->ZigZig,
			MystringData[i]->ZigZagCount,
			MystringData[i]->Brightness,
			MystringData[i]->Gamma,
			MystringData[i]->Slots);

		int MessageSize = strlen(SOM) + 1;	// include term zero

		int AbsSize = Size_struct_Tag_UDP_Packet_Request + 6 + MessageSize;

		wxByte *M = Build_Output_UDP_Header((wxByte*)Xmit_Buffer, AbsSize);

		//M->MSG_Size = MessageSize;
		Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_MSG_Size, MessageSize);

		//M->AbsoluteMsgSize = AbsSize;
		Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_AbsoluteMsgSize, AbsSize);

		//M->Xlights_IP = Xlights_IP_DW;
		Set_Network_Pack_dword(M, Tag_UDP_Packet_Request_Offset_Xlights_IP, Xlights_IP_DW);

		//M->CMD = UDP_CMD_Xlights_Send_Port_Config_Row;
		Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_CMD, UDP_CMD_Xlights_Send_Port_Config_Row);

		//M->Index = i;
		Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Index, i);

		//M->Max_Port_Config_Row = MystringData.size();
		Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Max_Port_Config_Row, MystringData.size());

		int r = Send_UDP_Packet_Acquire_Reply(Xmit_Buffer, AbsSize);

		if(r)
			return r;
	}

	return false;
}

bool EasyLights_Network_Communication::Send_DMX_Universe(int DMX_Univ, wxUint32 Start_Chan, wxUint32 Num_Chans)
{

	wxByte B[6 + Size_struct_Tag_UDP_Packet_Request]; // 6 for 3 byte header and trailer
	int AbsSize = sizeof(B);

    wxByte *M = Build_Output_UDP_Header((wxByte*)B, AbsSize);

	//M->MSG_Size = 0;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_MSG_Size, 0);

	//M->AbsoluteMsgSize = AbsSize;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_AbsoluteMsgSize, AbsSize);

	//M->Xlights_IP = Xlights_IP_DW;
	Set_Network_Pack_dword(M, Tag_UDP_Packet_Request_Offset_Xlights_IP, Xlights_IP_DW);

	//M->CMD = UDP_CMD_Xlights_Send_DMX_Univ;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_CMD, UDP_CMD_Xlights_Send_DMX_Univ);

	//M->Index = DMX_Univ;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Index, DMX_Univ);

	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Max_Port_Config_Row, Start_Chan);

	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_DMX_Chans, Num_Chans);

	return Send_UDP_Packet_Acquire_Reply(B, AbsSize);
}

bool EasyLights_Network_Communication::Send_Network_Type(int DMX_Univ)
{

	wxByte B[6 + Size_struct_Tag_UDP_Packet_Request]; // 6 for 3 byte header and trailer
	int AbsSize = sizeof(B);

    wxByte *M = Build_Output_UDP_Header((wxByte*)B, AbsSize);

	//M->MSG_Size = 0;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_MSG_Size, 0);

	//M->AbsoluteMsgSize = AbsSize;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_AbsoluteMsgSize, AbsSize);

	//M->Xlights_IP = Xlights_IP_DW;
	Set_Network_Pack_dword(M, Tag_UDP_Packet_Request_Offset_Xlights_IP, Xlights_IP_DW);

	//M->CMD = UDP_CMD_Xlights_Send_DMX_Univ;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_CMD, UDP_CMD_Xlights_Send_NetworkType);

	//M->Index = DMX_Univ;
	Set_Network_Pack_word(M, Tag_UDP_Packet_Request_Offset_Index, DMX_Univ);

	return Send_UDP_Packet_Acquire_Reply(B, AbsSize);
}

wxByte* EasyLights_Network_Communication::Build_Output_UDP_Header(wxByte *B, int ABSZ) const
{
	B[0] = 'J';
	B[1] = 'H';
	B[2] = 'S';

	B[ABSZ - 3] = 'J';
	B[ABSZ - 2] = 'H';
	B[ABSZ - 1] = 'E';

	return &B[3];
}

int EasyLights_Network_Communication::Get_Controller_Reply()
{
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int Acquired = 0;

    wxIPV4address Controller_Address_From_UDP;

	memset(Rcv_Buffer, 0, Controller_MTU_Size);
	memset((uint8_t*)&Controller_Address_From_UDP, 0, sizeof(Controller_Address_From_UDP));

	Reply = &Rcv_Buffer[3];	// offset header

	Start_Of_Recv_Message = Rcv_Buffer + 3 + Size_struct_Tag_UDP_Packet_Reply;

	while (true)
	{
		udp->RecvFrom(Controller_Address_From_UDP, &Rcv_Buffer[Acquired], (Controller_MTU_Size - Acquired));

		int n = udp->LastError();	// wxSOCKET_TIMEDOUT
		if(n)
			return -n;

		Acquired += udp->LastReadCount();

		if(Acquired >= Size_struct_Tag_UDP_Packet_Reply)
		{
			if(Rcv_Buffer[0] != 'J' || Rcv_Buffer[1] != 'H' || Rcv_Buffer[2] != 'S')
				return -100;

			Reply = &Rcv_Buffer[3];

			wxUint16 Reply_AbsoluteMsgSize = Get_Network_Pack_word(Reply, Tag_UDP_Packet_Reply_Offset_AbsoluteMsgSize);

			if(Acquired < Reply_AbsoluteMsgSize)
				continue;

			if(Rcv_Buffer[Reply_AbsoluteMsgSize - 1] != 'E' || Rcv_Buffer[Reply_AbsoluteMsgSize - 2] != 'H' || Rcv_Buffer[Reply_AbsoluteMsgSize - 3] != 'J')
				return -101;

			if(Get_Network_Pack_word(Reply, Tag_UDP_Packet_Reply_Offset_Reply) == 0)
			{
				wxUint16 Reply_type = Get_Network_Pack_word(Reply, Tag_UDP_Packet_Reply_Offset_type);
				wxUint16 Reply_FW_Version = Get_Network_Pack_word(Reply, Tag_UDP_Packet_Reply_Offset_FW_Version);

				if(_Board_type != Reply_type || _firmwareVersion != Reply_FW_Version)	// only debug once per major call
					logger_base.debug("Connected to Easylights Controller at IP %s - Type %d using FirmWare Version %d", (const char *)controller_ip.c_str(), Reply_type, Reply_FW_Version);

				_Board_type = Reply_type;
				_firmwareVersion = Reply_FW_Version;

			}


			return 0;

		}

	}

	return 0;
}

wxUint32 EasyLights_Network_Communication::Convert_IP_String(const char *IP) const
{
	char B[20];
    wxByte IPB[4];
    wxUint32 DW = 0;

	memset(B, 0, 20);
	strncpy(B, IP, 20);

	char *p = B;

	while(*p && *p == ' ') p++;
	char *V1 = p;
	while(*p && *p != '.') p++;
	if(*p)
	{
		*p = 0;
		p++;
	}

	while(*p && *p == ' ') p++;
	char *V2 = p;
	while(*p && *p != '.') p++;
	if(*p)
	{
		*p = 0;
		p++;
	}

	while(*p && *p == ' ') p++;
	char *V3 = p;
	while(*p && *p != '.') p++;
	if(*p)
	{
		*p = 0;
		p++;
	}

	while(*p && *p == ' ') p++;
	char *V4 = p;
	while(*p && *p != ' ') p++;
	if(*p)
	{
		*p = 0;
		p++;
	}

	IPB[0] = atoi(V1);
	IPB[1] = atoi(V2);
	IPB[2] = atoi(V3);
	IPB[3] = atoi(V4);

	DW = (IPB[3] << 24) | (IPB[2] << 16) | (IPB[1] << 8) | IPB[0];

	return DW;
}

void EasyLightsString::Dump() const
{
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	logger_base.debug("    Index %02d  Port_Segment %02d  StringType %d  Segmented %d  StartChan %d  PixelCount %d  GroupCount %d  EndChan %d  Direction %d  RGB_Order %d  NullPixels %d  ZigZig %d  ZigZagCount %d  Brightness %d  Gamma %d  Slots %d",

		index,
		Port_Segment,
		StringType,
		Segmented,
		StartChan,
		PixelCount,
		GroupCount,
		EndChan,
		Direction,
		RGB_Order,
		NullPixels,
		ZigZig,
		ZigZagCount,
		Brightness,
		Gamma,
		Slots );
}

static const std::string EASYLIGHTS_24 = "EasyLights 24"; //this Controller  propably doesn't work........
static const std::string EASYLIGHTS_PIX16 = "EasyLights PIX16";

class EasyLightsControllerRules: public ControllerRules
{
	int _type;
	//int _version;    //unused varible
	int _expansions;

public:

    EasyLightsControllerRules(int type, int version): ControllerRules()
	{
		_expansions = 0;
		_type = type;
		//_version = version;  //unused varible
	}

    virtual ~EasyLightsControllerRules() {}
    virtual bool SupportsLEDPanelMatrix() const override {
        return false;
    }

    virtual const std::string GetControllerId() const override {
        if (_type == 1) {
            return EASYLIGHTS_PIX16;
        }
        return EASYLIGHTS_24;
    }
    virtual const std::string GetControllerManufacturer() const override {
        return "EasyLights";
    }

    virtual bool SupportsVirtualStrings() const override { return true; }

	virtual int GetMaxPixelPortChannels() const override
	{
		if(_type == 1)
		{
			return 680 * 3;
		}
		else
		{
			return 0;
		}
	}

    virtual int GetMaxPixelPort() const override
	{
		if(_type == 1)
		{
			return 16;
		}
		else 
		{
			return 0;
		}

		return 48; //unreachable code....
	}

	virtual int GetMaxSerialPortChannels() const override
	{
		return 512;
	}

	virtual int GetMaxSerialPort() const override
	{
		if(_type == 1)//unneeded code....
		{
			return 1;
		}
		else
		{
			return 1;
		}
	}

	virtual bool IsValidPixelProtocol(const std::string protocol) const override
	{
		wxString p(protocol);
		p = p.Lower();
		if(p == "ws2811") return true;

		return false;
	
	}

    virtual bool IsValidSerialProtocol(const std::string protocol) const override
	{
		wxString p(protocol);
		p = p.Lower();
		if(p == "dmx") return true;

		return false;
	}

    virtual bool SupportsMultipleProtocols() const override
	{
		return false;
	}

    virtual bool SupportsSmartRemotes() const override { return false; }

    
    // Joe ... not sure this is right
    virtual bool SupportsMultipleInputProtocols() const override { return true; }

    virtual bool AllUniversesSameSize() const override
	{
		return false;
	}

    virtual std::set<std::string> GetSupportedInputProtocols() const override
	{
        std::set<std::string> res = {"E131", "ARTNET", "DDP"};
		return res;
	}

    virtual bool UniversesMustBeSequential() const override
	{
		return false;
	}

	virtual bool SingleUpload() const override { return true; }
};

static std::vector<EasyLightsControllerRules> CONTROLLER_TYPE_MAP = {
	EasyLightsControllerRules(1, 1)
};

void EasyLights::RegisterControllers() {
	for (auto &a : CONTROLLER_TYPE_MAP) {
		ControllerRegistry::AddController(&a);
	}
}

int EasyLights::GetMaxPixels() const
{
	if(Board_type == 1)
		return 680;
	else
		return 0;
}

EasyLights::EasyLights(const std::string& ip, int Reset)
{
    //static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	controller_ip = ip;
    firmwareVersion = 0;
	_version = 0;
	_model = 0;
	Port_Link_List_Size = 0;
    Port_LL_Head = nullptr;
    Board_type = 0;
    Network_Type = 0;
    Last_Channel_Supported = 0;

	if(Reset)
	{
		EL_Comm.Command_Reset_Controller(ip);
	}
}

EasyLights::~EasyLights()
{
	//EL_Comm.~EasyLights_Network_Communication();
}

bool EasyLights::Test_Reset_Complete() const
{
	return EL_Comm.Reset_Complete;
}

void EasyLights::Set_Model_Version_Based_on_Type()
{
	//wxByte Board_type;		// 100 = pixel 17 ports, 101 = AC 25 ports
	_version = 0;
	_model = 0;

	if(Board_type == 1)
	{
		_version = 1;
		_model = 1;
	}
	else if(Board_type == 2)
	{
		_version = 2;
		_model = 1;
	}

}

bool EasyLights::SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent)
{
	int r;
	wxUint32 DMX_Start = 0;
	wxUint32 DMX_Size = 0;
	int DMX_Universe = 0;

	Network_Type = 0;

	r = EL_Comm.Initial_Connection(controller_ip, &Board_type, &firmwareVersion);
	if(r)
	{
		DisplayError(wxString::Format("Can Not contact EasyLights Controller.").ToStdString());
		return false;
	}

	// Get universes based on IP
	std::list<Output*> outputs = outputManager->GetAllOutputs(controller_ip, selected);

	Network_Type = Check_And_Return_Controllers_Network_Type(outputs);	// 1 = e131, 2 = artnet, 3 = ddp
	if(Network_Type == 0)
		return false;


	EL_Comm.Send_Network_Type(Network_Type);

	// set model/version variables based on Easylights type

	Set_Model_Version_Based_on_Type();

	wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
	progress.Show();

	Last_Channel_Supported = 0;

	if(Network_Type == Network_E131 || Network_Type == Network_ArtNet)
	{
		// first validate E131 data
		r = Build_E131_Channel_Map(outputs);
		if(r)
			return false;

		if(Last_Channel_Supported == 0)	// no channels for e131 or artnet
			return false;
	}


	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
	logger_base.debug("EasyLights Outputs Upload: Uploading to %s", (const char *)controller_ip.c_str());

	progress.Update(0, "Scanning models");
	logger_base.info("Scanning models.");

	std::string check;
	UDController cud(controller_ip, controller_ip, allmodels, outputManager, &selected, check);

	EasyLightsControllerRules rules(_model, _version);
	bool success = cud.Check(&rules, check);

	logger_base.debug(check);

	logger_base.debug("CUD Dump");
	cud.Dump();	// this dump model data 

	progress.Update(10, "Retrieving string configuration from EasyLights.");
	logger_base.info("Retrieving string configuration from EasyLights.");


	std::vector<EasyLightsString*> stringData;

	progress.Update(40, "Processing current configuration data.");
	logger_base.info("Processing current configuration data.");

	std::vector<std::string> Config_Rows;

	r = EL_Comm.EasyLights_Network_Communication::Get_Port_Config_Rows(Config_Rows);

	if(r)
		return false;	// error

	
	ReadStringData(stringData, Config_Rows);	// convert control EasyLights port config data to string array

	logger_base.debug("Downloaded string data.");
	DumpStringData(stringData);


	progress.Update(50, "Configuring string ports.");
	logger_base.info("Configuring string ports.");

	// break it up into virtual strings
	std::vector<EasyLightsString*> newStringData;
	std::vector<EasyLightsString*> toDelete;
	int index = 0;
	for(int pp = 1; pp <= 16; pp++)
	{
		if(cud.HasPixelPort(pp))
		{
			UDControllerPort* port = cud.GetControllerPixelPort(pp);
			logger_base.info("Pixel Port %d Protocol %s.", pp, (const char *)port->GetProtocol().c_str());

			port->CreateVirtualStrings(true);

			EasyLightsString* firstString = nullptr;
			for(auto sd : stringData)
			{
				if(sd->Port_Segment == pp)	// was pp - 1)
				{
					if(firstString == nullptr)
					{
						firstString = sd;
					}
					else
					{
						toDelete.push_back(sd);
					}
				}
			}
			wxASSERT(firstString != nullptr);

			// need to add virtual strings
			bool first = true;
			for(auto vs : port->GetVirtualStrings())
			{
				// perform sanity check to make sure pixel/chan is within the channels supported on the controller

				if(Last_Channel_Supported && ((wxUint32)vs->_startChannel > Last_Channel_Supported || (wxUint32)vs->_endChannel > Last_Channel_Supported))	// not executed for ddp
				{
					DisplayError(wxString::Format("EasyLights uses Sequential Channels.  Your Model %s using Universe %d has Start Chan %d or End Chan %d which is Greater than the supported %d number of channels",
						vs->_description, vs->_universe, vs->_startChannel, vs->_endChannel, Last_Channel_Supported).ToStdString());

					return false;
				}
				
				EasyLightsString* fs;
				if(first)
				{
					fs = firstString;
					first = false;
				}
				else
				{
					fs = new EasyLightsString();
				}

				// ignore index ... we will fix them up when done
				fs->Port_Segment = firstString->Port_Segment;
				fs->index = index++;
				fs->StringType = DecodeStringPortProtocol(vs->_protocol);
				//fs->universe = vs->_universe;
				fs->StartChan = vs->_startChannel;
				fs->EndChan = vs->_endChannel;
				fs->PixelCount = vs->Channels() / 3;
				
				//fs->description = SafeDescription(vs->_description);
				if(vs->_brightnessSet)
				{
					fs->Brightness = vs->_brightness;
				}
				else
				{
					fs->Brightness = firstString->Brightness;
				}
				if(vs->_nullPixelsSet)
				{
					fs->NullPixels = vs->_nullPixels;
				}
				else
				{
					fs->NullPixels = firstString->NullPixels;
				}
				if(vs->_gammaSet)
				{
					fs->Gamma = vs->_gamma;
				}
				else
				{
					fs->Gamma = firstString->Gamma;
				}
				if(vs->_colourOrderSet)
				{
					fs->RGB_Order = EncodeColourOrder(vs->_colourOrder);
				}
				else
				{
					fs->RGB_Order = firstString->RGB_Order;
				}
				if(vs->_reverseSet)
				{
					fs->Direction = EncodeDirection(vs->_reverse);
				}
				else
				{
					fs->Direction = firstString->Direction;
				}
				if(vs->_groupCountSet)
				{
					fs->GroupCount = vs->_groupCount;
				}
				else
				{
					fs->GroupCount = firstString->GroupCount;
				}

				newStringData.push_back(fs);
			}
		}
		else
		{
			for(auto sd : stringData)
			{
				if(sd->Port_Segment == pp)	//pp - 1)
				{
					sd->index = index++;
					newStringData.push_back(sd);
				}
			}
		}
	}

	stringData = newStringData;

	// delete any read strings we didnt keep
	for(auto d : toDelete)
	{
		delete d;
	}

	logger_base.debug("Virtual strings created.");
	DumpStringData(stringData);

	if(success && cud.GetMaxPixelPort() > 0)
	{
		progress.Update(60, "Uploading string ports.");

		if(check != "")
		{
			DisplayWarning("Upload warnings:\n" + check);
			check = ""; // to suppress double display
		}

		logger_base.info("Uploading string ports.");

		r = UploadStringPorts(stringData);
		if(r)
		{

			DisplayError("Not uploaded due to errors.\n" + check);
			check = "";
		}
	}
	else
	{
		DisplayError("Not uploaded due to errors.\n" + check);
		check = "";

	}

	// delete all our string data
	while(stringData.size() > 0)
	{
		delete stringData.back();
		stringData.pop_back();
	}

	if(success && cud.GetMaxSerialPort() > 0)
	{
		progress.Update(90, "Uploading serial ports.");

		if(check != "")
		{
			DisplayWarning("Upload warnings:\n" + check);
		}

		for(int sp = 1; sp <= cud.GetMaxSerialPort(); sp++)
		{
			if(sp > 1)
			{
				DisplayError(wxString::Format("EasyLight Controllers only have 1 DMX port").ToStdString());

				DisplayError("Not uploaded due to errors.\n" + check);

				return false;
			}

			if(cud.HasSerialPort(sp))
			{
				UDControllerPort* port = cud.GetControllerSerialPort(sp);
				logger_base.info("Serial Port %d Protocol %s.", sp, (const char *)port->GetProtocol().c_str());

				int dmxOffset = 1;
				UDControllerPortModel* m = port->GetFirstModel();
				if(m != nullptr)
				{
					dmxOffset = m->GetDMXChannelOffset();
					if(dmxOffset < 1) dmxOffset = 1; // a value less than 1 makes no sense
				}

				int sc = port->GetStartChannel() - dmxOffset + 1;
				logger_base.debug("    sc:%d - offset:%d -> %d", port->GetStartChannel(), dmxOffset, sc);

				DMX_Start = sc;	//port->GetStartChannel();
				DMX_Size = port->GetEndChannel() - DMX_Start + 1;

				// validate that it is within out E131 channel count and acquire Universe associated with DMX;

				if(Network_Type == Network_E131 || Network_Type == Network_ArtNet)
				{
					if(DMX_Start > Last_Channel_Supported)
					{
						DisplayError(wxString::Format("EasyLights uses Sequential Channels.  Your Model uses DMX Starting at Channel %d which is Greater than the supported %d number of channels",
							DMX_Start, Last_Channel_Supported).ToStdString());

						DisplayError("Not uploaded due to errors.\n" + check);

						return false;
					}
				}

			    if(Network_Type == Network_E131 || Network_Type == Network_ArtNet)
				{
					for (int i = 0; i < 65; i++)
					{
						if(E131_Array[i].Univ == 0)
						{
							DisplayError(wxString::Format("Error - EasyLights was unable to identify the Universe associated with DMX channel %d ", DMX_Start).ToStdString());

							DisplayError("Not uploaded due to errors.\n" + check);

							break;
						}

						if(E131_Array[i].Start_Chan == DMX_Start)
						{
							DMX_Universe = E131_Array[i].Univ;
							break;
						}
					}
				}
			}
		}
	}
	else
	{
		if(GetMaxSerialOutputs() > 0 && check != "")
		{
			DisplayError("Not uploaded due to errors.\n" + check);
		}
	}

	EL_Comm.Send_DMX_Universe(DMX_Universe, DMX_Start, DMX_Size);

	r = Send_E131_Data_to_Controller(outputs);

	progress.Update(100, "Done.");

	if(r == 0)
	{

		EL_Comm.Command_Reset_Controller(controller_ip);

		logger_base.info("EasyLights upload Completed and Reset Applied.");


		return true;
	}
	else
	{
		logger_base.info("EasyLights upload Failed.");

		return false;
	}
	
}

char *ParseToken(char *p)
{
	while(*p && *p != ',') p++;
	if(*p)
	{
		*p = 0;
		p++;
	}

	return p;
}

void EasyLights::ReadStringData(std::vector<EasyLightsString*>& stringData, std::vector<std::string>&  Row_Data) const
{
	//static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	char PD[60];

	int oldCount = stringData.size();
	stringData.resize(Port_Link_List_Size);
	for(int i = oldCount; i < Port_Link_List_Size; ++i)
	{
		stringData[i] = nullptr;
	}

    for (int i = 0; i < Row_Data.size(); i++)
	{
		EasyLightsString* string = new EasyLightsString();
		stringData.push_back(string);

		strcpy(PD, Row_Data[i].c_str());	// don't work with string in linke list - use temp string

		char *p = PD;

		while(*p && *p == ' ') p++;

		string->index = i;

		char *p1 = p;
		p = ParseToken(p);
		string->Port_Segment = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->StringType = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->Segmented = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->StartChan = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->PixelCount = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->GroupCount = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->EndChan = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->Direction = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->RGB_Order = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->NullPixels = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->ZigZig = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->ZigZagCount = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->Brightness = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->Gamma = atoi(p1);

		p1 = p;
		p = ParseToken(p);
		string->Slots = atoi(p1);
	}
}

void EasyLights::DumpStringData(std::vector<EasyLightsString*> stringData) const
{
	for(auto sd : stringData)
	{
		sd->Dump();
	}
}

int EasyLights::DecodeBrightness(int brightnessCode) const
{
	switch(brightnessCode)
	{
	case 0: return 100;
	case 1: return 90;
	case 2: return 80;
	case 3: return 70;
	case 4: return 60;
	case 5: return 50;
	case 6: return 40;
	case 7: return 30;
	case 8: return 20;
	case 9: return 15;
	case 10: return 10;
	default: break;
	}

	return 100;
}

int EasyLights::EncodeBrightness(int brightness) const
{
	if(brightness < 11) return 10;
	if(brightness < 16) return 9;
	if(brightness < 21) return 8;
	if(brightness < 31) return 7;
	if(brightness < 41) return 6;
	if(brightness < 51) return 5;
	if(brightness < 61) return 4;
	if(brightness < 71) return 3;
	if(brightness < 81) return 2;
	if(brightness < 91) return 1;
	return 0;
}

float EasyLights::DecodeGamma(int gammaCode) const
{
	switch(gammaCode)
	{
	case 0: return 1.0f;
	case 1: return 2.0f;
	case 2: return 2.3f;
	case 3: return 2.5f;
	case 4: return 2.8f;

	default: break;
	}
	return 1.0f;
}

int EasyLights::EncodeGamma(float gamma) const
{
	if(gamma < 1.5) return 0;
	if(gamma < 2.15) return 1;
	if(gamma < 2.4) return 2;
	if(gamma < 2.65) return 3;

	return 4;
}

int EasyLights::DecodeColourOrder(int colourOrderCode) const
{

	return colourOrderCode;
}

int EasyLights::EncodeColourOrder(const std::string& colourOrder) const
{
	if(colourOrder == "RGB") return 0;
	if(colourOrder == "RBG") return 1;
	if(colourOrder == "GRB") return 2;
	if(colourOrder == "GBR") return 3;
	if(colourOrder == "BRG") return 4;
	if(colourOrder == "BGR") return 5;
	return 0;
}

int EasyLights::DecodeDirection(int directionCode) const
{
	switch(directionCode)
	{
	case 0: return 0;	//"Forward";
	case 1: return 1;	//"Reverse";
	default: break;
	}
	return 0;	//"Forward";
}

int EasyLights::EncodeDirection(const std::string& direction) const
{
	if(direction == "Forward") return 0;
	if(direction == "Reverse") return 1;
	return 0;
}

int EasyLights::DecodeStringPortProtocol(std::string protocol) const
{
	wxString p(protocol);
	p = p.Lower();
	if(p == "ws2811") return 1;

	return 0;
}

int EasyLights::DecodeSerialOutputProtocol(std::string protocol) const
{
	wxString p(protocol);
	p = p.Lower();

	if(p == "dmx") return 0;

	return -1;
}

void EasyLights::UploadSerialOutput(int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent)
{
	//static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
	if(output > GetMaxSerialOutputs())
	{
		DisplayError("EasyLights only supports " + wxString::Format("%d", GetMaxSerialOutputs()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", parent);
		return;
	}

	int32_t sc;
	auto o = outputManager->GetOutput(portstart, sc);

	if(o != nullptr)
	{
		wxString request = wxString::Format("btnSave=Save&t%d=%d&u%d=%d&s%d=%d",
			output - 1, protocol,
			output - 1, o->GetUniverse(),
			output - 1, sc);
		//PutURL("/SerialOutputs.htm", request.ToStdString());
	}
	else
	{
		DisplayError("Error uploading serial output to EasyLights. " + wxString::Format("%i", portstart) + " does not map to a universe.");
	}
}

int EasyLights::GetMaxSerialOutputs() const
{
	return 1;
}

int EasyLights::Check_And_Return_Controllers_Network_Type(std::list<Output*>& outputs)
{
	int Type = 0;

    for (const auto& it : outputs)
	{
		int tt = 0;

		if(it->GetType() == "E131")
			tt = Network_E131;
		else if(it->GetType() == "ArtNet")
			tt = Network_ArtNet;
		else if(it->GetType() == "DDP")
			tt = Network_DDP;

		if(Type == 0)
			Type = tt;

		if(Type != tt)
		{
			DisplayError(wxString::Format("Your EasyLight Controller can one support E131, ArtNet, or DDP and only one at a time").ToStdString());
			return 0;
		}
	}

	return Type;
}


int EasyLights::Build_E131_Channel_Map(std::list<Output*>& outputs)
{
    wxUint32 Total_Channel_Count = 0;

	memset(E131_Array, 0, sizeof(E131_Array));
	Last_Channel_Supported = 0;
	int E131_Array_Index = 0;

	int Number_Of_Universes = outputs.size();
	if(Number_Of_Universes > 65)
	{
		DisplayError(wxString::Format("EasyLights controller only suports 65 Universes and you have %d.", outputs.size()).ToStdString());
		return 1;
	}

	for (const auto& it : outputs)
	{
		int U = it->GetUniverse();
		int CH = it->GetChannels();
		int SC = it->GetStartChannel();

		if(SC != (Total_Channel_Count + 1))
		{
			DisplayError(wxString::Format("EasyLights Channels need to be Sequencial.  Univ %d has of Start Chan of %d but was Expecting %d", U, SC, (Total_Channel_Count + 1)).ToStdString());
			return 1;

		}

		E131_Array[E131_Array_Index].Univ = U;
		E131_Array[E131_Array_Index].Num_Chan = CH;
		E131_Array[E131_Array_Index].Start_Chan = Total_Channel_Count + 1;
		E131_Array[E131_Array_Index].End_Chan = Total_Channel_Count + CH;

		Total_Channel_Count += CH;
		E131_Array_Index++;
	}

	Last_Channel_Supported = Total_Channel_Count;

	return 0;
}

// here is where we do custome EasyLights port config process based on generic virtual strings

int EasyLights::UploadStringPorts(const std::vector<EasyLightsString*>& stringData)
{
    int Last_Port_Processed = 0;
	int Segment_Row_Number = 0;

	// If we have virtual/segmented rows, Easylights uses a master row followed by the segments that make up that row so Easylights has 1 additional row for each
	// port that utilizes a virtual/segmented string.  That Master string must also be update to hold 

/***

This shows how EasyLight Port Config data looks with Port 1 being configured as 3 segments (4 rows with top row master)
Easylights Port Config Row 0 data => 1,1,0,1,150,1,0,0,0,0,0,0,100,1,450
Easylights Port Config Row 1 data => 1,0,1,1,50,1,150,0,0,0,0,0,100,1,150
Easylights Port Config Row 2 data => 1,0,1,151,50,1,300,0,3,0,0,0,50,1,150
Easylights Port Config Row 3 data => 1,0,1,301,50,1,450,1,0,0,0,0,100,1,150
Easylights Port Config Row 4 data => 2,1,0,451,100,1,750,0,0,0,0,0,100,1,300

Segmented rows do not have data for string type since the port can't mix types.  The segmented field identifies a segment row.
Segmented row has slots for its segment only

The master row does not have ending channel data as that is held in the segment rows.
Master has slots for complete port

Slots are physical memory locations required to build/hold the port data.  Normally slots will equal channel count unless null pixels are used.
Null pixels take of physical memory space and is used in memory allocation so the differenec in max channels and max slots will be the number
of null pixel chaanels allocated

We must also make sure we have rows for all 16 ports -- none can be missing.

We will use a link list to build the actual row data to be sent to the controller based on the incoming virtual string data

***/

	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	std::vector<EasyLightsString*> MystringData;

    EasyLightsString *Master = nullptr;

	for (int i = 0; i < stringData.size(); ++i)
	{
		EasyLightsString *R = new EasyLightsString();

		memmove(R, stringData[i], sizeof(EasyLightsString));	// create a copy of xlights data

		// is this a master row or a segment

		if(stringData[i]->Port_Segment != Last_Port_Processed)	// master row ... OR First xlights segment
		{
			Master = R;	// used when updating from segments

			Last_Port_Processed = R->Port_Segment;
			Segment_Row_Number = 0;

			R->Slots = R->EndChan - R->StartChan + 1;

			MystringData.push_back(R);

		}
		else // else we have a segments row
		{
			Segment_Row_Number++;

			if(Segment_Row_Number == 1)	// We need duplicate the last xlights row -- because it is really the first segment and this is segment 2
			{
                EasyLightsString  *TMP = new EasyLightsString();
				memmove(TMP, Master, sizeof(EasyLightsString));	// create a copy of last master record as it will be segment 1

				// Port_Master_Row is now master row
				// TMP is first segment
				// R is second segment

				// fix up master first

				Master->EndChan = 0;
				Master->Slots = 0;		// sum all segments slots here
				Master->PixelCount = 0;	// sum all segments slots here

				// fix up first segment

				TMP->StringType = 0;
				TMP->Segmented = 1;

				TMP->Slots = (TMP->EndChan - TMP->StartChan + 1 + TMP->NullPixels);

				Master->Slots += (TMP->EndChan - TMP->StartChan + 1 + TMP->NullPixels);
				Master->PixelCount += TMP->PixelCount;

				// process this segment which is seg 2

				R->StringType = 0;
				R->Segmented = 1;

				R->Slots = (R->EndChan - R->StartChan + 1 + R->NullPixels);

				Master->Slots += (R->EndChan - R->StartChan + 1 + R->NullPixels);
				Master->PixelCount += R->PixelCount;

				// link them into the list

				MystringData.push_back(TMP);
				MystringData.push_back(R);

			}
			else // we have 3 or more segments
			{
				R->StringType = 0;
				R->Segmented = 1;

				R->Slots = (R->EndChan - R->StartChan + 1 + R->NullPixels);

				Master->Slots += (R->EndChan - R->StartChan + 1 + R->NullPixels);
				Master->PixelCount += R->PixelCount;

				MystringData.push_back(R);
			}
		}
	}

	// we are done with xlights data -- make sure all ports have data

	while(Last_Port_Processed < 16)
	{
        EasyLightsString  *TMP = new EasyLightsString();
		memmove(TMP, Master, sizeof(EasyLightsString));	// create a copy of last master record
		Last_Port_Processed++;
		TMP->Port_Segment = Last_Port_Processed;
		MystringData.push_back(TMP);
	}

	// dump the data for debug
	logger_base.debug("EasyLights Rows Created.");
	DumpStringData(MystringData);
		// send rows to controller

	return EL_Comm.Send_Port_Config_Rows(MystringData);
}

int EasyLights::Send_E131_Data_to_Controller(std::list<Output*>& outputs)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxUint32 Total_Channel_Count = 0;
	wxString request;

	if(outputs.size() > 65) {
		DisplayError(wxString::Format("Attempt to upload %d universes to EasyLights controller but only 65 are supported.", outputs.size()).ToStdString());
		return 1;
	}

	std::vector<EasyLights_E131*> U_CH_Array;

    for (const auto& it : outputs) {
		int U = it->GetUniverse();
		int CH = it->GetChannels();
		int SC = it->GetStartChannel();

		if(SC != (Total_Channel_Count + 1)) {
			DisplayError(wxString::Format("EasyLights Channels need to be Sequencial.  Univ %d has of Start Chan of %d but was Expecting %d", U, SC, (Total_Channel_Count + 1)).ToStdString());
			return 1;
		}

		EasyLights_E131 *U_CH = new EasyLights_E131;
		U_CH->Univ = U;
		U_CH->Channels = CH;
		U_CH_Array.push_back(U_CH);

		Total_Channel_Count += CH;
	}

	int r = EL_Comm.Send_Universe_Data(U_CH_Array);
	if(r)
		return 1;

	logger_base.debug("Easylights - Controller at IP %s Received Universe Data", (const char *)controller_ip.c_str());

	return 0;
}

// struct is always packed as little endian
void Set_Network_Pack_word(wxByte *Start_Of_Struct, int Offset, wxUint16 V)
{
	wxByte  *P = Start_Of_Struct + Offset;

	*P++ = V & 0xff;
	*P = (V >> 8) & 0xff;
}

// struct is always packed as little endian
void Set_Network_Pack_dword(wxByte *Start_Of_Struct, int Offset, wxUint32 V)
{
	wxByte  *P = Start_Of_Struct + Offset;

	*P++ = V & 0xff;
	*P++ = (V >> 8) & 0xff;
	*P++ = (V >> 16) & 0xff;
	*P++ = (V >> 24) & 0xff;
}

// struct is always packed as little endian
wxUint16 Get_Network_Pack_word(wxByte *Start_Of_Struct, int Offset)
{
	wxByte  *P = Start_Of_Struct + Offset;
	
    wxUint16 V = *P++;
	V |= (*P << 8);

	return V;
}
