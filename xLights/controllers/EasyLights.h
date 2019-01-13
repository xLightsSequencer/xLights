#ifndef EASYLIGHTS_H
#define EASYLIGHTS_H

#include <wx/socket.h>
#include <wx/timer.h>

#include <wx/xml/xml.h>
#include <list>


wxUint16 Get_Network_Pack_word(wxByte *Start_Of_Struct, int Offset);
void Set_Network_Pack_dword(wxByte *Start_Of_Struct, int Offset, wxUint32 V);
void Set_Network_Pack_word(wxByte *Start_Of_Struct, int Offset, wxUint16 V);

class Output;
class OutputManager;
class ModelManager;
class EasyLights_Network_Communication;

// wxByte
// wxUint16
// wxUint32

enum
{
	UDP_CMD_Xlights_Connect = 200,
	UDP_CMD_Xlights_Reset,
	UDP_CMD_Xlights_Universes_IN,
	UDP_CMD_Xlights_Get_Port_Config_Row,
	UDP_CMD_Xlights_Send_Port_Config_Row,
	UDP_CMD_Xlights_Send_DMX_Univ,
	UDP_CMD_Xlights_Send_NetworkType,
};

#define Controller_Recv_Port	49147
#define Controller_MTU_Size 1500


// size of udp header is 46 bytes
// controller mtu is 700 - 46 stack header - 10 for my struct = 644 max udp message data length = 161 words

class EasyLightsString
{
public:
	int index;
	int Port_Segment;
	int StringType;
	int Segmented;
	int StartChan;
	int PixelCount;
	int GroupCount;
	int EndChan;
	int Direction;
	int RGB_Order;
	int NullPixels;
	int ZigZig;
	int ZigZagCount;
	int Brightness;
	int Gamma;
	int Slots;
	void Dump() const;
};


struct Tag_Port_Config_Row
{
	int Port_Segment;
	int StringType;
	int Segmented;
	int StartChan;
	int PixelCount;
	int GroupCount;
	int EndChan;
	int Direction;
	int RGB_Order;
	int NullPixels;
	int ZigZig;
	int ZigZagCount;
	int Brightness;
	int Gamma;
	int Slots;

	struct Tag_Port_Config_Row *next;
};


struct Tag_Port_Config_Link_List
{
	char PD[60];
	Tag_Port_Config_Link_List *next;
};

struct Tag_E131_Universe_Channel
{
	wxUint16 Univ;
	wxUint16 Num_Chan;
	wxUint16 Start_Chan;
	wxUint16 End_Chan;
};


class EasyLights_E131
{
public:
	wxUint16 Univ;
	wxUint16 Channels;
};

class EasyLights_Network_Communication
{
public:
	wxByte Reset_Complete;

	wxUint16 _firmwareVersion;
	wxByte _Board_type;		// 1 = pixel 17 ports, 2 = AC 25 ports

	wxDatagramSocket *udp;
	wxString Xlights_IP;
	wxUint32 Xlights_IP_DW;

	wxByte Xmit_Buffer[Controller_MTU_Size];
	wxByte Rcv_Buffer[Controller_MTU_Size];
	wxByte *Start_Of_Recv_Message;


	wxIPV4address controller;
	std::string controller_ip;

	/*** reference only -- use network packing functions --- controller expects these structures

	wxUint16 Get_Network_Pack_word(wxByte *Start_Of_Struct, int Offset);
	void Set_Network_Pack_dword(wxByte *Start_Of_Struct, int Offset, wxUint32 V);
	void Set_Network_Pack_word(wxByte *Start_Of_Struct, int Offset, wxUint16 V);

	These are PACK'd 2 bytes little endian

	struct Tag_UDP_Packet_Request
	{
		word AbsoluteMsgSize;
		word CMD;
		word MSG_Size;
		dword Xlights_IP;
		word Index;			// variable associated with CMD
		word Max_Port_Config_Row;
		word DMX_Chans;
	};

	struct Tag_UDP_Packet_Reply
	{
		word AbsoluteMsgSize;
		word CMD;
		word type;
		word FW_Version;
		word Reply;	// 0 = no error, else error #
	};
	****/

#define Size_struct_Tag_UDP_Packet_Request 16
#define Tag_UDP_Packet_Request_Offset_AbsoluteMsgSize 0
#define Tag_UDP_Packet_Request_Offset_CMD 2
#define Tag_UDP_Packet_Request_Offset_MSG_Size 4
#define Tag_UDP_Packet_Request_Offset_Xlights_IP 6
#define Tag_UDP_Packet_Request_Offset_Index 10
#define Tag_UDP_Packet_Request_Offset_Max_Port_Config_Row 12
#define Tag_UDP_Packet_Request_Offset_DMX_Chans 14


#define Size_struct_Tag_UDP_Packet_Reply 10
#define Tag_UDP_Packet_Reply_Offset_AbsoluteMsgSize 0
#define Tag_UDP_Packet_Reply__Offset_CMD 2
#define Tag_UDP_Packet_Reply_Offset_type 4
#define Tag_UDP_Packet_Reply_Offset_FW_Version 6
#define Tag_UDP_Packet_Reply_Offset_Reply 8



	//struct Tag_UDP_Packet_Reply *Reply;
	wxByte *Reply;

	EasyLights_Network_Communication();
	virtual ~EasyLights_Network_Communication();

	wxUint32 Convert_IP_String(const char *IP);
	int Get_Controller_Reply();
	wxByte* Build_Output_UDP_Header(wxByte *Buff, int Absolute_Size);

	bool Send_UDP_Packet_Acquire_Reply(wxByte *B, int SZ);

	// all return true if failure detected - false = complete nor errors
	bool Initial_Connection(const std::string& ip, wxByte *Board_type, wxUint16 *firmwareVersion);
	bool Command_Reset_Controller(const std::string& ip);
	bool Send_Universe_Data(std::vector<EasyLights_E131*>& e131);
	bool Get_Port_Config_Rows(std::vector<std::string>&  Row_Data);
	bool Send_Port_Config_Rows(std::vector<EasyLightsString*> MystringData);
	bool Send_DMX_Universe(int DMX_Univ, wxUint32 Start_Chan, wxUint32 Num_Chans);
	bool Send_Network_Type(int DMX_Univ);

};

class EasyLights 
{
	EasyLights_Network_Communication EL_Comm;

	struct Tag_Port_Config_Link_List *Port_LL_Head;
	int Port_Link_List_Size;


	wxUint16 firmwareVersion;
	wxByte Board_type;		// 1 = pixel 17 ports, 2 = AC 25 ports
	int _version;
	int _model;
	int Network_Type;

	std::string controller_ip;

	struct Tag_E131_Universe_Channel E131_Array[65];
	wxUint32 Last_Channel_Supported;


	enum
	{
		Network_E131 = 1,
		Network_ArtNet,
		Network_DDP,

	};


	


	int GetMaxPixels() const;
	void Set_Model_Version_Based_on_Type();
	void ReadStringData(std::vector<EasyLightsString*>& stringData, std::vector<std::string>&  Row_Data) const;
	void DumpStringData(std::vector<EasyLightsString*> stringData) const;
	int DecodeStringPortProtocol(std::string protocol) const;
	int EncodeDirection(const std::string& direction) const;
	int DecodeDirection(int directionCode) const;
	int EncodeColourOrder(const std::string& colourOrder) const;
	int DecodeColourOrder(int colourOrderCode) const;
	int EncodeGamma(float gamma) const;
	float DecodeGamma(int gammaCode) const;
	int EncodeBrightness(int brightness) const;
	int DecodeBrightness(int brightnessCode) const;
	int DecodeSerialOutputProtocol(std::string protocol) const;
	void UploadSerialOutput(int output, OutputManager* outputManager, int protocol, int portstart, wxWindow* parent);
	int GetMaxSerialOutputs() const;
	int Build_E131_Channel_Map(std::list<Output*>& outputs);
	int UploadStringPorts(const std::vector<EasyLightsString*>& stringData);
	int Check_And_Return_Controllers_Network_Type(std::list<Output*>& outputs);

	int Send_E131_Data_to_Controller(std::list<Output*>& outputs);

public:
    EasyLights(const std::string& ip, int Reset);
    virtual ~EasyLights();
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
	bool Test_Reset_Complete();
};


#endif