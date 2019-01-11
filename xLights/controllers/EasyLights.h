#ifndef EASYLIGHTS_H
#define EASYLIGHTS_H

#include <wx/socket.h>
#include <wx/timer.h>

#include <wx/xml/xml.h>
#include <list>

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

#pragma pack(push)
#pragma pack(2)
	struct Tag_UDP_Packet_Request
	{
		wxUint16 AbsoluteMsgSize;
		wxUint16 CMD;
		wxUint16 MSG_Size;
		wxUint32 Xlights_IP;
		wxUint16 Index;			// variable associated with CMD
		wxUint16 Max_Port_Config_Row;
	};

	struct Tag_UDP_Packet_Reply
	{
		wxUint16 AbsoluteMsgSize;
		wxUint16 CMD;
		wxUint16 type;
		wxUint16 FW_Version;
		wxUint16 Reply;	// 0 = no error, else error #
	};
#pragma pack(pop)

	struct Tag_UDP_Packet_Reply *Reply;

	EasyLights_Network_Communication();
	virtual ~EasyLights_Network_Communication();

	wxUint32 Convert_IP_String(const char *IP);
	int Get_Controller_Reply();
	wxByte* Build_Output_UDP_Header(wxByte *Buff, int Absolute_Size);

	bool Send_UDP_Packet_Acquire_Reply(wxByte *B, int SZ);

	// all return true if failure detected - false = complete nor errors
	bool Initial_Connection(const std::string& ip, wxByte *Board_type, wxUint16 *firmwareVersion);
	bool Command_Reset_Controller(const std::string& ip);
	bool Send_Universe_Data(std::vector<EasyLights_E131*>& e131, int E131_Artnet);
	bool Get_Port_Config_Row(std::vector<std::string>&  Row_Data);
	bool Send_Port_Config_Row(std::vector<EasyLightsString*> MystringData);
	bool Send_DMX_Universe(int DMX_Univ);

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

	std::string controller_ip;

	struct Tag_E131_Universe_Channel E131_Array[65];
	wxUint32 Last_Channel_Supported;




	


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
	int Build_E131_Channel_Map(OutputManager* outputManager, std::list<int>& selected);
	int UploadStringPorts(const std::vector<EasyLightsString*>& stringData);


public:
    EasyLights(const std::string& ip, int Reset);
    virtual ~EasyLights();
    bool SetInputUniverses(OutputManager* outputManager, std::list<int>& selected);
    bool SetOutputs(ModelManager* allmodels, OutputManager* outputManager, std::list<int>& selected, wxWindow* parent);
	bool Test_Reset_Complete();
};


#endif