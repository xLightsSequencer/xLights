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

#include <log4cpp/Category.hh>
#include "UtilFunctions.h"



EasyLights_Network_Communication::EasyLights_Network_Communication()
{
	Xlights_IP_DW = 0;
	udp = nullptr;
	Reset_Complete = 0;
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
	int r;

	while(1)
	{
		udp->SendTo(controller, (const void*)&B[Actuall_Sent], (SZ - Actuall_Sent));

		r = udp->LastError();
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
		return 1;
	}
	else if(!udp->IsOk())
	{
		logger_base.error("Error initializing EasyLights datagram ... is network connected? OK: FALSE %s", (const char *)Xlights_IP.c_str());
		udp->Destroy();
		udp = nullptr;
		return 1;
	}
	else if(udp->Error() != wxSOCKET_NOERROR)
	{
		logger_base.error("Error creating EasyLights datagram => %d : %s. %s", udp->LastError(), (const char *)DecodeIPError(udp->LastError()).c_str(), (const char *)Xlights_IP.c_str());
		udp->Destroy();
		udp = nullptr;
		return 1;
	}


	udp->SetTimeout(3);


	wxByte B[6 + sizeof(struct Tag_UDP_Packet_Request)]; // 6 for 3 byte header and trailer
	int AbsSize = sizeof(B);
	struct Tag_UDP_Packet_Request *M;
	int r;

	M = (struct Tag_UDP_Packet_Request*)Build_Output_UDP_Header((wxByte*)B, AbsSize);

	M->MSG_Size = 0;
	M->AbsoluteMsgSize = AbsSize;
	M->Xlights_IP = Xlights_IP_DW;
	M->CMD = UDP_CMD_Xlights_Connect;
	M->Index = 0;

	r = Send_UDP_Packet_Acquire_Reply(B, AbsSize);

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
		return 1;
	}
	else if(!udp->IsOk())
	{
		logger_base.error("Error initializing EasyLights datagram ... is network connected? OK: FALSE %s", (const char *)Xlights_IP.c_str());
		udp->Destroy();
		udp = nullptr;
		return 1;
	}
	else if(udp->Error() != wxSOCKET_NOERROR)
	{
		logger_base.error("Error creating EasyLights datagram => %d : %s. %s", udp->LastError(), (const char *)DecodeIPError(udp->LastError()).c_str(), (const char *)Xlights_IP.c_str());
		udp->Destroy();
		udp = nullptr;
		return 1;
	}


	udp->SetTimeout(3);


	wxByte B[6 + sizeof(struct Tag_UDP_Packet_Request)]; // 6 for 3 byte header and trailer
	int AbsSize = sizeof(B);
	struct Tag_UDP_Packet_Request *M;
	int r;

	M = (struct Tag_UDP_Packet_Request*)Build_Output_UDP_Header((wxByte*)B, AbsSize);

	M->MSG_Size = 0;
	M->AbsoluteMsgSize = AbsSize;
	M->Xlights_IP = Xlights_IP_DW;
	M->CMD = UDP_CMD_Xlights_Reset;
	M->Index = 0;

	r = Send_UDP_Packet_Acquire_Reply(B, AbsSize);

	udp->Destroy();
	udp = nullptr;

	if(r == 0)
		Reset_Complete = 1;

	return r;

}

bool EasyLights_Network_Communication::Send_Universe_Data(std::vector<EasyLights_E131*>& e131, int E131_Artnet)
{

	memset(Xmit_Buffer, 0, Controller_MTU_Size);

	int MessageSize = ((e131.size() * 2) + 2) * sizeof(wxUint16);	// we send array of 16 bit words

	wxUint16 *Wptr = (wxUint16*)(3 + sizeof(struct Tag_UDP_Packet_Request) + Xmit_Buffer);

	// first 2 locations hold protocal and size, rest alternamt univ then chan

	*Wptr++ = E131_Artnet;
	*Wptr++ = e131.size();

	int i;
	int r;

	for(i = 0; i < e131.size(); i++)
	{
		*Wptr++ = e131[i]->Univ;
		*Wptr++ = e131[i]->Channels;
	}

	int AbsSize = sizeof(struct Tag_UDP_Packet_Request) + 6 + MessageSize;
	struct Tag_UDP_Packet_Request *M;


	M = (struct Tag_UDP_Packet_Request*)Build_Output_UDP_Header((wxByte*)Xmit_Buffer, AbsSize);

	M->MSG_Size = MessageSize;
	M->AbsoluteMsgSize = AbsSize;
	M->Xlights_IP = Xlights_IP_DW;
	M->CMD = UDP_CMD_Xlights_Universes_IN;
	M->Index = 0;

	r = Send_UDP_Packet_Acquire_Reply(Xmit_Buffer, AbsSize);

	return r;


}

bool EasyLights_Network_Communication::Get_Port_Config_Row(std::vector<std::string>&  Row_Data)
{
	wxByte B[6 + sizeof(struct Tag_UDP_Packet_Request)];

	int AbsSize = sizeof(struct Tag_UDP_Packet_Request) + 6;
	struct Tag_UDP_Packet_Request *M;

	int Row_Index = 0;
	int r;


	M = (struct Tag_UDP_Packet_Request*)Build_Output_UDP_Header((wxByte*)B, AbsSize);

	M->MSG_Size = 0;
	M->AbsoluteMsgSize = AbsSize;
	M->Xlights_IP = Xlights_IP_DW;
	M->CMD = UDP_CMD_Xlights_Get_Port_Config_Row;

	while(1)
	{
		M->Index = Row_Index;

		r = Send_UDP_Packet_Acquire_Reply(B, AbsSize);
		if(r)
			return 1;

		// see if we have data or at end of available rows;

		if(Reply->Reply)	// we have data
		{
			Row_Data.push_back((char*)Start_Of_Recv_Message);
		}
		else
			return 0;

		Row_Index++;
	}

	return 1;
}

bool EasyLights_Network_Communication::Send_Port_Config_Row(std::vector<EasyLightsString*> MystringData)
{
	int i, r;

	memset(Xmit_Buffer, 0, Controller_MTU_Size);
	int MessageSize;
	int AbsSize;
	struct Tag_UDP_Packet_Request *M;
	char *SOM = (char*)(3 + sizeof(struct Tag_UDP_Packet_Request) + Xmit_Buffer);


	for(i = 0; i < MystringData.size(); i++)
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

		MessageSize = strlen(SOM) + 1;	// include term zero

		AbsSize = sizeof(struct Tag_UDP_Packet_Request) + 6 + MessageSize;

		M = (struct Tag_UDP_Packet_Request*)Build_Output_UDP_Header((wxByte*)Xmit_Buffer, AbsSize);

		M->MSG_Size = MessageSize;
		M->AbsoluteMsgSize = AbsSize;
		M->Xlights_IP = Xlights_IP_DW;
		M->CMD = UDP_CMD_Xlights_Send_Port_Config_Row;
		M->Index = i;
		M->Max_Port_Config_Row = MystringData.size();;


		r = Send_UDP_Packet_Acquire_Reply(Xmit_Buffer, AbsSize);

		if(r)
			return r;
	}

	return 0;

}

bool EasyLights_Network_Communication::Send_DMX_Universe(int DMX_Univ)
{

	wxByte B[6 + sizeof(struct Tag_UDP_Packet_Request)]; // 6 for 3 byte header and trailer
	int AbsSize = sizeof(B);
	struct Tag_UDP_Packet_Request *M;

	M = (struct Tag_UDP_Packet_Request*)Build_Output_UDP_Header((wxByte*)B, AbsSize);

	M->MSG_Size = 0;
	M->AbsoluteMsgSize = AbsSize;
	M->Xlights_IP = Xlights_IP_DW;
	M->CMD = UDP_CMD_Xlights_Send_DMX_Univ;
	M->Index = DMX_Univ;

	return Send_UDP_Packet_Acquire_Reply(B, AbsSize);

}


wxByte* EasyLights_Network_Communication::Build_Output_UDP_Header(wxByte *B, int ABSZ)
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

	int n;
	int Acquired = 0;

	wxIPV4address Controller_Address_From_UDP;

	memset(Rcv_Buffer, 0, Controller_MTU_Size);
	memset((byte*)&Controller_Address_From_UDP, 0, sizeof(Controller_Address_From_UDP));

	Reply = (struct Tag_UDP_Packet_Reply*)&Rcv_Buffer[3];	// offset header

	Start_Of_Recv_Message = Rcv_Buffer + 3 + sizeof(struct Tag_UDP_Packet_Reply);


	while(1)
	{
		udp->RecvFrom(Controller_Address_From_UDP, &Rcv_Buffer[Acquired], (Controller_MTU_Size - Acquired));

		n = udp->LastError();	// wxSOCKET_TIMEDOUT
		if(n)
			return -n;

		Acquired += udp->LastReadCount();

		if(Acquired >= sizeof(struct Tag_UDP_Packet_Reply))
		{
			if(Rcv_Buffer[0] != 'J' || Rcv_Buffer[1] != 'H' || Rcv_Buffer[2] != 'S')
				return -100;

			Reply = (struct Tag_UDP_Packet_Reply*)&Rcv_Buffer[3];

			if(Acquired < Reply->AbsoluteMsgSize)
				continue;


			if(Rcv_Buffer[Reply->AbsoluteMsgSize - 1] != 'E' || Rcv_Buffer[Reply->AbsoluteMsgSize - 2] != 'H' || Rcv_Buffer[Reply->AbsoluteMsgSize - 3] != 'J')
				return -101;

			if(Reply->Reply == 0)
			{
				if(_Board_type != Reply->type || _firmwareVersion != Reply->FW_Version)	// only debug once per major call
					logger_base.debug("Connected to Easylights Controller at IP %s - Type %d using FirmWare Version %d", (const char *)controller_ip.c_str(), Reply->type, Reply->FW_Version);

				_Board_type = Reply->type;
				_firmwareVersion = Reply->FW_Version;

			}


			return 0;

		}

	}

	return 0;

}




wxUint32 EasyLights_Network_Communication::Convert_IP_String(const char *IP)
{
	char B[20];
	char *p;
	wxByte IPB[4];
	char *V1, *V2, *V3, *V4;
	wxUint32 DW = 0;

	memset(B, 0, 20);
	strncpy(B, IP, 20);

	p = B;

	while(*p && *p == ' ') p++;
	V1 = p;
	while(*p && *p != '.') p++;
	if(*p)
	{
		*p = 0;
		p++;
	}

	while(*p && *p == ' ') p++;
	V2 = p;
	while(*p && *p != '.') p++;
	if(*p)
	{
		*p = 0;
		p++;
	}

	while(*p && *p == ' ') p++;
	V3 = p;
	while(*p && *p != '.') p++;
	if(*p)
	{
		*p = 0;
		p++;
	}

	while(*p && *p == ' ') p++;
	V4 = p;
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


class EasyLightsControllerRules: public ControllerRules
{
	int _type;
	int _version;
	int _expansions;

public:
	EasyLightsControllerRules(int type, int version): ControllerRules()
	{
		_expansions = 0;
		_type = type;
		_version = version;
	}
	virtual ~EasyLightsControllerRules() {}

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

		return 48;
	}
	virtual int GetMaxSerialPortChannels() const override
	{
		return 512;
	}
	virtual int GetMaxSerialPort() const override
	{
		if(_type == 1)
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
	virtual bool AllUniversesSameSize() const override
	{
		return false;
	}
	virtual std::list<std::string> GetSupportedInputProtocols() const override
	{
		std::list<std::string> res;
		res.push_back("E131");
		res.push_back("ARTNET");
		return res;
	}
	virtual bool UniversesMustBeSequential() const override
	{
		return false;
	}
};



int EasyLights::GetMaxPixels() const
{
	if(Board_type == 1)
		return 680;
	else
		return 0;
}



EasyLights::EasyLights(const std::string& ip, int Reset)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	controller_ip = ip;

    firmwareVersion = 0;
	_version = 0;
	_model = 0;
	Port_Link_List_Size = 0;

	if(Reset)
	{
		EL_Comm.Command_Reset_Controller(ip);
	}


}


EasyLights::~EasyLights()
{

	//EL_Comm.~EasyLights_Network_Communication();

}


bool EasyLights::SetInputUniverses(OutputManager* outputManager, std::list<int>& selected)
{
	int tt, r;
	int U, CH, SC;;
	wxUint32 Total_Channel_Count = 0;



	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
	wxString request;
	int output = 0;

	int E131_Artnet = 0;	// e131 = 1, artnet = 2

	r = EL_Comm.Initial_Connection(controller_ip, &Board_type, &firmwareVersion);
	if(r)
	{
		DisplayError(wxString::Format("Can Not contact EasyLights Controller.").ToStdString());
		return false;
	}


	// Get universes based on IP
	std::list<Output*> outputs = outputManager->GetAllOutputs(controller_ip, selected);


	if(outputs.size() > 65)
	{
		DisplayError(wxString::Format("Attempt to upload %d universes to EasyLights controller but only 65 are supported.", outputs.size()).ToStdString());
		return false;
	}

	std::vector<EasyLights_E131*> U_CH_Array;
	EasyLights_E131 *U_CH;

	for(auto it = outputs.begin(); it != outputs.end(); ++it)
	{
		tt = 0;

		if((*it)->GetType() == "E131")
			tt = 1;
		else if((*it)->GetType() == "ArtNet")
			tt = 2;

		
		U = (*it)->GetUniverse();
		CH = (*it)->GetChannels();
		SC = (*it)->GetStartChannel();

		if(SC != (Total_Channel_Count + 1))
		{
			DisplayError(wxString::Format("EasyLights Channels need to be Sequencial.  Univ %d has of Start Chan of %d but was Expecting %d", U, SC, (Total_Channel_Count+1)).ToStdString());
			return false;

		}


		if(tt == 0)
		{
			DisplayError(wxString::Format("Attempt to upload universe %d to EasyLights controller but NOT identified as E131 or ArtNet", U).ToStdString());
			return false;
		}

		if(E131_Artnet == 0)
		{
			E131_Artnet = tt;
		}
		else
		{
			if(tt != E131_Artnet)
			{
				DisplayError(wxString::Format("Attempt to upload universe %d to EasyLights controller but NOT identified as previous universes ( E131 or ArtNet)", U).ToStdString());
				return false;
			}
		}

		U_CH = new EasyLights_E131;
		U_CH->Univ = U;
		U_CH->Channels = CH;
		U_CH_Array.push_back(U_CH);

		Total_Channel_Count += CH;

	}

	r = EL_Comm.Send_Universe_Data(U_CH_Array, E131_Artnet);
	if(r)
		return false;

	logger_base.debug("Easylights - Controller at IP %s Received Universe Data", (const char *)controller_ip.c_str());

	return true;
}

bool EasyLights::Test_Reset_Complete()
{
	return EL_Comm.Reset_Complete;
}

void EasyLights::Set_Model_Version_Based_on_Type()
{
	//wxByte Board_type;		// 1 = pixel 17 ports, 2 = AC 25 ports
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
	int r, DMX_Start;;

	r = EL_Comm.Initial_Connection(controller_ip, &Board_type, &firmwareVersion);
	if(r)
	{
		DisplayError(wxString::Format("Can Not contact EasyLights Controller.").ToStdString());
		return false;
	}

	// first validate E131 data

	r = Build_E131_Channel_Map(outputManager, selected);
	if(r)
		return false;

	// set model/version variables based on Easylights type

	Set_Model_Version_Based_on_Type();

	//ResetStringOutputs(); // this shouldnt be used normally
	wxProgressDialog progress("Uploading ...", "", 100, parent, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
	progress.Show();

	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
	logger_base.debug("EasyLights Outputs Upload: Uploading to %s", (const char *)controller_ip.c_str());

	progress.Update(0, "Scanning models");
	logger_base.info("Scanning models.");

	std::string check;
	UDController cud(controller_ip, allmodels, outputManager, &selected, check);

	EasyLightsControllerRules rules(_model, _version);
	bool success = cud.Check(&rules, check);

	logger_base.debug(check);

	cud.Dump();

	progress.Update(10, "Retrieving string configuration from EasyLights.");
	logger_base.info("Retrieving string configuration from EasyLights.");


	std::vector<EasyLightsString*> stringData;

	progress.Update(40, "Processing current configuration data.");
	logger_base.info("Processing current configuration data.");

	std::vector<std::string> Config_Rows;

	r = EL_Comm.EasyLights_Network_Communication::Get_Port_Config_Row(Config_Rows);

	if(r)
		return false;	// error

	
	int mainPixels = GetMaxPixels();

	ReadStringData(stringData, Config_Rows);	// convert control EasyLights port config data in link list to string array

	logger_base.debug("Downloaded string data.");
	DumpStringData(stringData);


	progress.Update(50, "Configuring string ports.");
	logger_base.info("Configuring string ports.");

	bool portdone[100];
	memset(&portdone, 0x00, sizeof(portdone)); // all false

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

			port->CreateVirtualStrings();

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

				if(vs->_startChannel > Last_Channel_Supported || vs->_endChannel > Last_Channel_Supported)
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

				DMX_Start = port->GetStartChannel();

				// validate that it is within out E131 channel count and acquire Universe associated with DMX;

				if(DMX_Start > Last_Channel_Supported)
				{
					DisplayError(wxString::Format("EasyLights uses Sequential Channels.  Your Model uses DMX Starting at Channel %d which is Greater than the supported %d number of channels",
						DMX_Start, Last_Channel_Supported).ToStdString());

					DisplayError("Not uploaded due to errors.\n" + check);
				}

				int i;

				for(i = 0; i < 65; i++)
				{
					if(E131_Array[i].Univ == 0)
					{
						DisplayError(wxString::Format("Error - EasyLights was unable to identify the Universe associated with DMX channel %d ", DMX_Start).ToStdString());

						DisplayError("Not uploaded due to errors.\n" + check);

						break;
					}

					if(E131_Array[i].Start_Chan == DMX_Start)
					{
						EL_Comm.Send_DMX_Universe(E131_Array[i].Univ);

						break;
					}
				}
			}
		}
	}
	else
	{
		EL_Comm.Send_DMX_Universe(0);

		if(GetMaxSerialOutputs() > 0 && check != "")
		{
			DisplayError("Not uploaded due to errors.\n" + check);
		}
	}



	progress.Update(100, "Done.");
	logger_base.info("EasyLights upload done.");

	return true;
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
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	char PD[60];

	int oldCount = stringData.size();
	stringData.resize(Port_Link_List_Size);
	for(int i = oldCount; i < Port_Link_List_Size; ++i)
	{
		stringData[i] = nullptr;
	}

	int i = 0;
	char *p, *p1;


	for(i = 0; i < Row_Data.size(); i++)
	{
		EasyLightsString* string = new EasyLightsString();
		stringData.push_back(string);

		strcpy(PD, Row_Data[i].c_str());	// don't work with string in linke list - use temp string

		p = PD;

		while(*p && *p == ' ') p++;

		string->index = i;

		p1 = p;
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
	case 0: return 1.0;
	case 1: return 2.0;
	case 2: return 2.3;
	case 3: return 2.5;
	case 4: return 2.8;

	default: break;
	}
	return 1.0;
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
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
	if(output > GetMaxSerialOutputs())
	{
		DisplayError("EasyLights only supports " + wxString::Format("%d", GetMaxSerialOutputs()) + " outputs. Attempt to upload to output " + wxString::Format("%d", output) + ".", parent);
		return;
	}

	long sc;
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

int EasyLights::Build_E131_Channel_Map(OutputManager* outputManager, std::list<int>& selected)
{
	int tt, T;
	int U, CH, SC;
	wxUint32 Total_Channel_Count = 0;


	memset(E131_Array, 0, sizeof(E131_Array));
	Last_Channel_Supported = 0;
	int E131_Array_Index = 0;


	// Get universes based on IP
	std::list<Output*> outputs = outputManager->GetAllOutputs(controller_ip, selected);

	int Number_Of_Universes = outputs.size();
	if(Number_Of_Universes > 65)
	{
		DisplayError(wxString::Format("EasyLights controller only suports 65 Universes and you have %d.", outputs.size()).ToStdString());
		return 1;
	}

	T = 0;

	for(auto it = outputs.begin(); it != outputs.end(); ++it)
	{
		tt = 0;

		if((*it)->GetType() == "E131")
			tt = 1;
		else if((*it)->GetType() == "ArtNet")
			tt = 2;


		U = (*it)->GetUniverse();
		CH = (*it)->GetChannels();
		SC = (*it)->GetStartChannel();

		if(SC != (Total_Channel_Count + 1))
		{
			DisplayError(wxString::Format("EasyLights Channels need to be Sequencial.  Univ %d has of Start Chan of %d but was Expecting %d", U, SC, (Total_Channel_Count + 1)).ToStdString());
			return 1;

		}


		if(tt == 0)
		{
			DisplayError(wxString::Format("Universe %d to EasyLights controller but NOT identified as E131 or ArtNet", U).ToStdString());
			return 1;
		}

		if(T == 0)
		{
			T = tt;
		}
		else
		{
			if(tt != T)
			{
				DisplayError(wxString::Format("Universe %d EasyLights controller but NOT mix E131 and ArtNet", U).ToStdString());
				return 1;
			}
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
	int i;
	int Last_Port_Processed = 0;
	int Segment_Row_Number;

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

	EasyLightsString *R, *Master, *TMP;

	for(i = 0; i < stringData.size(); ++i)
	{
		R = new EasyLightsString();

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
				TMP = new EasyLightsString();
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
		TMP = new EasyLightsString();
		memmove(TMP, Master, sizeof(EasyLightsString));	// create a copy of last master record
		Last_Port_Processed++;
		TMP->Port_Segment = Last_Port_Processed;
		MystringData.push_back(TMP);
	}


	// dump the data for debug
	logger_base.debug("EasyLights Rows Created.");
	DumpStringData(MystringData);
		// send rows to controller

	return EL_Comm.Send_Port_Config_Row(MystringData);

}
