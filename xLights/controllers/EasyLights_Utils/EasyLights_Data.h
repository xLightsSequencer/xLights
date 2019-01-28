#ifndef EASYLIGHTS_Data
#define EASYLIGHTS_Data


#include <wx/xml/xml.h>
#include <list>

#include "EasyLights_Schedule_Dialog.h"
#include "EasyLights_PlayList_Dialog.h"
#include "EasyLights_FSEQ_Dialog.h"
#include "EasyLights_Add_Slaves_Dialog.h"
#include "EasyLights_Identify_Dialog.h"
#include "EasyLights_StandAlone_Dialog.h"


/*****

Criteria for driving slaves

If ELC has less than 16 univ then it can drive 32 external universes on one controller or 2 controllers at 16 each

If ELS has more than 16 univ but less than 32 univ - than it can drive 1 controller with less than 16 univ


***/

#define ELC_NO_Slaves_Channel_Count (32 * 512)
#define ELC_ONE_Slaves_Channel_Count (16 * 512)

#define Slave_Max_Channel_Count_for_One (30 * 512)
#define Slave_Max_Channel_Count_For_Two (16 * 512)

#define ELC_Support_TWO_Slaves 0
#define ELC_Support_One_Slave 1
#define ELC_Support_NO_Slaves 2

#define Slave_Allowed_Two 0
#define Slave_Allowed_One 1
#define	Slave_Allowed_None 2

extern EasyLights_StandAlone_Dialog *EL_Ptr;


class Slave_Univ_Chan
{
public:
	uint16_t Univ;
	uint16_t Num_Chan;
	uint16_t Start_Chan;
};

struct Tag_FSEQ_Variable_Header_Main
{
	uint8_t Number_Slaves;
	uint8_t ELC_Port_Config_ReMap;
	uint16_t Total_Size_VHeader;
};

struct Tag_FSEQ_Variable_Header_H1
{
	uint32_t Main_Num_Channels;
	uint16_t Num_Remap_Ports;
	uint16_t Remap_Ports_Offset;
	uint8_t Slave1_Active;
	uint8_t Slave2_Active;
	uint8_t Slave1_Num_Univ;
	uint8_t Slave2_Num_Univ;
	uint16_t Slave1_Num_Channels;
	uint16_t Slave2_Num_Channels;
	uint16_t Slave1_Univ_Offset;
	uint16_t Slave2_Univ_Offset;
};

inline void write4ByteUInt(uint8_t* data, uint32_t v) 
{
	data[0] = (uint8_t)(v & 0xFF);
	data[1] = (uint8_t)((v >> 8) & 0xFF);
	data[2] = (uint8_t)((v >> 16) & 0xFF);
	data[3] = (uint8_t)((v >> 24) & 0xFF);
};

inline void write2ByteUInt(uint8_t* data, uint32_t v) 
{
	data[0] = (uint8_t)(v & 0xFF);
	data[1] = (uint8_t)((v >> 8) & 0xFF);
};



typedef struct  wavHeaderInfo
{
	char                RIFF[4];        /* RIFF Header      */ //Magic header
	unsigned long       ChunkSize;      /* RIFF Chunk Size  */
	char                WAVE[4];        /* WAVE Header      */
	char                fmt[4];         /* FMT header       */
	unsigned long       Subchunk1Size;  /* Size of the fmt chunk                                */
	unsigned short      AudioFormat;    /* Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM */
	unsigned short      NumOfChannels;      /* Number of channels 1=Mono 2=Sterio                   */
	unsigned long       SamplesPerSec;  /* Sampling Frequency in Hz                             */
	unsigned long       bytesPerSec;    /* bytes per second */
	unsigned short      blockAlign;     /* 2=16-bit mono, 4=16-bit stereo */
	unsigned short      bitsPerSample;  /* Number of bits per sample      */
	char                Subchunk2ID[4]; /* "data"  string   */
	unsigned long       Subchunk2Size;  /* Sampled data length    */
} wavHeader;


struct Tag_RIFF_Header
{
	char                RIFF[4];        /* RIFF Header      */ //Magic header
	unsigned long       ChunkSize;      /* RIFF Chunk Size  */
	char                WAVE[4];        /* WAVE Header      */
};

struct Tag_ChunkHeader
{
	char                Subchunk2ID[4]; /* "fmt or data"  string   */
	unsigned long       Subchunk2Size;  /* Sampled data length    */
};


class EL_Controller_Info
{
public:
	std::string Model;
	int MaxChannels;
	int MaxUniverses;
	bool Slave_Control;
	int Max_SPI_Ports;
	int Max_Diff_Ports;
	bool Master_Capable;
};

class EasyLights_EndPoint
{
public:
	std::string Model;
	int MaxChannels;
	int MaxUniverses;
	bool Slave_Control;
	int Max_SPI_Ports;
	int Max_Diff_Ports;
	bool Master_Capable;
	bool Master;
	bool Audio;
	bool DDP_Network;
	bool Slave_Attached;
	int Channels_Used;
	int Start_Channel;
	int End_Channel;
	std::string IP;
	std::string S1_IP;
	std::string S2_IP;
	std::string Name;


	EasyLights_EndPoint();

};

void Get_EasyLights_Specifications(std::string &ShowDirectory);
void Create_Temp_EasyLights_spec_File();

void Build_EL_EndPoint_Array(std::string &ShowDirectory);

int Add_IPs_From_Output_To_EndPoint_Array();

void Save_EndPoint_Array_to_XML(std::string &ShowDirectory);

void Set_EndPoint_EL_Max_Controller_Values(EasyLights_EndPoint *EP);

int Create_EasyLights_PCM_File(std::string &Xfseq_FN, std::string &FN_pcm, EasyLights_FSEQ_Dialog *Display, uint32_t *Size_PCM_Bytes, uint32_t *Sample_Rate_Used);
int Make_WAV_From_PCM(std::string &EL_PCM, std::string &EL_WAV, uint32_t Size_PCM_Bytes, uint32_t Sample_Rate_Used);
int Create_EasyLights_FSEQ_File(EL_FSEQ_Info *ELC, EasyLights_FSEQ_Dialog *Display);

EasyLights_EndPoint *Find_EndPoint_By_IP(std::string &IP);

int Return_FSEQ_Version(std::string FSEQ_FN);

uint8_t *Get_VHeader_for_ELC_FSEQ(EL_FSEQ_Info *ELC, int *VHSize);

std::vector< Slave_Univ_Chan> *Get_Slave_Universe_Channel_Config(std::string IP);

uint8_t *Add_Slave_Univ_Chan_Data_to_VHeader(uint8_t *D, std::vector< Slave_Univ_Chan> *SUC);

int ELC_Status_To_Support_Slaves(EasyLights_EndPoint *ELC);
int Slave_Consumes_This_Number_Of_Controllers(std::string IP);
int Slave_Consumes_This_Number_Of_Controllers(EasyLights_EndPoint *Slave);

char *Return_EasuLights_FSEQ_FN_Without_IP(char *S);

char *Get_Next_JArg(char *p, char **JF, char **JV, int *Last_Key, int Ignore_Bracket);

#endif

