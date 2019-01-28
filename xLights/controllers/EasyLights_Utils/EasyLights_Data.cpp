#include <wx/msgdlg.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include <wx/progdlg.h>

#include <wx/intl.h>
#include <wx/string.h>

#include "../../models/Model.h"
#include "../../outputs/OutputManager.h"
#include "../../outputs/Output.h"
#include "../../models/ModelManager.h"


#include <log4cpp/Category.hh>
#include "UtilFunctions.h"
#include <wx/filename.h>

#include <wx/dir.h>
#include <wx/filesys.h>

#include "../../models/Model.h"
#include "../../outputs/OutputManager.h"
#include "../../outputs/Output.h"
#include "../../models/ModelManager.h"

#include "../../FSEQFile.h"

#include "../EasyLights.h"
#include "EasyLights_Data.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/avutil.h>

}




void Get_EasyLights_Specifications(std::string &ShowDirectory)
{
	EL_Controller_Info *I;
	wxDir EL_Dir;

	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	std::string SD = ShowDirectory;

	wxFileName fnDir(SD + "/EasyLightsData");
	std::string _filename = fnDir.GetFullPath();

	if(!EL_Dir.Exists(_filename))	// is our directory alive
	{
		EL_Dir.Make(_filename);
	}

	wxFileName fn(SD + "/EasyLightsData/EasyLights_Controller_Specification.xml");
	_filename = fn.GetFullPath();

	if(!fn.FileExists())
	{
		wxMessageBox(wxT("Please Download File EasyLights_Controller_Specification.xml from EasyLights.com and Put the file in the Folder EasyLightsData within your Show Directory.\nI will Create a Temporary one for Now"),
			wxT("EasyLights_Controller_Specification.xml Missing"), wxOK, NULL);

		Create_Temp_EasyLights_spec_File();
	}

	if(!fn.FileExists())
	{
		wxMessageBox(wxT("Failed to Create Temp File. Please Download File EasyLights_Controller_Specification.xml from EasyLights.com and Put the file in the Folder EasyLightsData within your Show Directory."),
			wxT("EasyLights_Controller_Specification.xml Missing"), wxOK, NULL);

		return;
	}


	wxXmlDocument doc;
	doc.Load(fn.GetFullPath());

	wxString VendorWho;

	if(doc.IsOk())
	{
		for(auto e = doc.GetRoot()->GetChildren(); e != nullptr; e = e->GetNext())
		{
			if(e->GetName() == "Vendor")
			{
				VendorWho = e->GetAttribute("name");
				if(VendorWho == "EasyLights")
				{
					for(auto ee = doc.GetRoot()->GetChildren()->GetChildren(); ee != nullptr; ee = ee->GetNext())
					{
						if(ee->GetName() == "controller")
						{
							I = new EL_Controller_Info();

							//         <controller Type="Pixel" Model="EasyLightsPix16_V1"  MaxChannels="33280" MaxUniverses="65" Slave_Control="Yes" Max_SPI_Ports="16"  Max_Diff_Ports="0"  Master_Capable="Yes"/>

							I->Model = ee->GetAttribute("Model", "");

							I->MaxChannels = wxAtoi(ee->GetAttribute("MaxChannels", "0"));
							I->MaxUniverses = wxAtoi(ee->GetAttribute("MaxUniverses", "0"));
							I->Slave_Control = wxAtoi(ee->GetAttribute("Slave_Control", "0"));
							I->Max_SPI_Ports = wxAtoi(ee->GetAttribute("Max_SPI_Ports", "0"));
							I->Max_Diff_Ports = wxAtoi(ee->GetAttribute("Max_Diff_Ports", "0"));
							I->Master_Capable = wxAtoi(ee->GetAttribute("Master_Capable", "0"));

							EL_Ptr->EL_Controller.push_back(I);
						}
					}

					break;
				}

			}
		}
	}
}

void Build_EL_EndPoint_Array(std::string &ShowDirectory)
{
	EasyLights_EndPoint *I;
	wxDir EL_Dir;
	wxXmlDocument doc;

	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	std::string SD = ShowDirectory;

	wxFileName fnDir(SD + "/EasyLightsData");
	std::string _filename = fnDir.GetFullPath();

	if(!EL_Dir.Exists(_filename))	// is our directory alive
	{
		EL_Dir.Make(_filename);
	}

	wxFileName fn(SD + "/EasyLightsData/EasyLights_EndPoints.xml");
	_filename = fn.GetFullPath();

	if(fn.FileExists())
	{
		doc.Load(fn.GetFullPath());

		std::string J;

		wxString VendorWho;

		if(doc.IsOk())
		{
			for(auto ee = doc.GetRoot()->GetChildren(); ee != nullptr; ee = ee->GetNext())
			{
				J = ee->GetName();

				if(ee->GetName() == "endpoint")
				{
					I = new EasyLights_EndPoint();

//	<endpoint EL_Controller="1" Type="1"  Model ="Pixel" MaxChannels="1" MaxUniverses ="1"  Slave_Control="1"   Max_SPI_Ports="1"  Max_Diff_Ports="1"  Master_Capable="1"  IP="10.10.10.10" S1_IP = "10.10.10.10"   S2_IP = "10.10.10.10" Name="Tree"/ >

					I->Model = ee->GetAttribute("Model", "");
					I->MaxChannels = wxAtoi(ee->GetAttribute("MaxChannels", "0"));
					I->MaxUniverses = wxAtoi(ee->GetAttribute("MaxUniverses", "0"));
					I->Slave_Control = wxAtoi(ee->GetAttribute("Slave_Control", "0"));
					I->Max_SPI_Ports = wxAtoi(ee->GetAttribute("Max_SPI_Ports", "0"));
					I->Max_Diff_Ports = wxAtoi(ee->GetAttribute("Max_Diff_Ports", "0"));
					I->Master_Capable = wxAtoi(ee->GetAttribute("Master_Capable", "0"));
					I->Master = wxAtoi(ee->GetAttribute("Master", "0"));
					I->Audio = wxAtoi(ee->GetAttribute("Audio", "0"));
					I->IP = ee->GetAttribute("IP", "");
					I->S1_IP = ee->GetAttribute("S1_IP", "");
					I->S2_IP = ee->GetAttribute("S2_IP", "");
					I->Name = ee->GetAttribute("Name", "");
					I->Channels_Used = wxAtoi(ee->GetAttribute("Channels_Used", "0"));
					I->Slave_Attached = wxAtoi(ee->GetAttribute("Slave_Attached", "0"));
					I->Start_Channel = wxAtoi(ee->GetAttribute("Start_Channel", "0"));
					I->End_Channel = wxAtoi(ee->GetAttribute("End_Channel", "0"));
					

					EL_Ptr->EL_EndPoints.push_back(I);

				}
			}
		}
	}
}

EasyLights_EndPoint::EasyLights_EndPoint()
{
	MaxChannels = 0;
	MaxUniverses = 0;
	Slave_Control = 0;
	Max_SPI_Ports = 0;
	Max_Diff_Ports = 0;
	Master_Capable = 0;
	Master = 0;
	Audio = 0;
	Channels_Used = 0;
	Slave_Attached = 0;
	DDP_Network = 0;
	Start_Channel = 0;
	End_Channel = 0;


}


int Add_IPs_From_Output_To_EndPoint_Array()
{
	int Found = 0;
	int dirty = 0;
	std::string J;
	EasyLights_EndPoint *I;
	int SC, CH;

	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	std::list<Output*> OutPuts = EL_Ptr->OutPutMgr->GetAllOutputs();

// clear out channels used

	for(auto ep = EL_Ptr->EL_EndPoints.begin(); ep != EL_Ptr->EL_EndPoints.end(); ++ep)
	{
		(*ep)->Channels_Used = 0;
		(*ep)->Start_Channel = 0;
		(*ep)->End_Channel = 0;

		Set_EndPoint_EL_Max_Controller_Values(*ep);

	}


	for(auto it = OutPuts.begin(); it != OutPuts.end(); ++it)
	{
		Found = 0;

		J = (*it)->GetIP();
		CH = (*it)->GetChannels();
		SC = (*it)->GetStartChannel();


		if((*it)->GetType() == OUTPUT_E131)
		{
			for(auto ep = EL_Ptr->EL_EndPoints.begin(); ep != EL_Ptr->EL_EndPoints.end(); ++ep)
			{
				if(J == (*ep)->IP)
				{
					(*ep)->Channels_Used += CH;
					if((*ep)->Start_Channel == 0)
						(*ep)->Start_Channel = SC;

					(*ep)->End_Channel = (*ep)->Start_Channel + (*ep)->Channels_Used - 1;

					Found++;
					break;
				}
			}

			if(Found == 0)	// we have an IP not in the list
			{
				I = new EasyLights_EndPoint();
				I->IP = J;
				I->DDP_Network = 0;
				I->Channels_Used = CH;
				I->Start_Channel = SC;
				I->End_Channel = SC + CH - 1;
				EL_Ptr->EL_EndPoints.push_back(I);

				dirty = 1;
			}
		}

		if((*it)->GetType() == OUTPUT_DDP)
		{
			for(auto ep = EL_Ptr->EL_EndPoints.begin(); ep != EL_Ptr->EL_EndPoints.end(); ++ep)
			{
				if(J == (*ep)->IP)
				{
					(*ep)->Channels_Used += CH;
					if((*ep)->Start_Channel == 0)
						(*ep)->Start_Channel = SC;

					(*ep)->End_Channel = (*ep)->Start_Channel + (*ep)->Channels_Used - 1;

					Found++;
					break;
				}
			}

			if(Found == 0)	// we have an IP not in the list
			{
				I = new EasyLights_EndPoint();
				I->IP = J;
				I->DDP_Network = 1;
				I->Channels_Used = CH;
				I->End_Channel = SC + CH - 1;
				EL_Ptr->EL_EndPoints.push_back(I);

				dirty = 1;
			}
		}



	}

	return dirty;
}

void Save_EndPoint_Array_to_XML(std::string &ShowDirectory)
{
	wxXmlDocument doc;
	std::string _filename;
	wxXmlNode *node;

	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	std::string SD = ShowDirectory;
	wxFileName fn(SD + "/EasyLightsData/EasyLights_EndPoints.xml");
	_filename = fn.GetFullPath();

	wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, "EndPoints");

	doc.SetRoot(root);


	for(auto it = EL_Ptr->EL_EndPoints.begin(); it != EL_Ptr->EL_EndPoints.end(); ++it)
	{

		node = new wxXmlNode(wxXML_ELEMENT_NODE, "endpoint");


		node->AddAttribute("Model", (*it)->Model);

		node->AddAttribute("MaxChannels", wxString::Format("%d", (*it)->MaxChannels));
		node->AddAttribute("MaxUniverses", wxString::Format("%d", (*it)->MaxUniverses));
		node->AddAttribute("Slave_Control", wxString::Format("%d", (*it)->Slave_Control));
		node->AddAttribute("Max_SPI_Ports", wxString::Format("%d", (*it)->Max_SPI_Ports));
		node->AddAttribute("Max_Diff_Ports", wxString::Format("%d", (*it)->Max_Diff_Ports));
		node->AddAttribute("Master_Capable", wxString::Format("%d", (*it)->Master_Capable));
		node->AddAttribute("Master", wxString::Format("%d", (*it)->Master));
		node->AddAttribute("Audio", wxString::Format("%d", (*it)->Audio));
		node->AddAttribute("Channels_Used", wxString::Format("%d", (*it)->Channels_Used));
		node->AddAttribute("Slave_Attached", wxString::Format("%d", (*it)->Slave_Attached));
		node->AddAttribute("Start_Channel", wxString::Format("%d", (*it)->Start_Channel));
		node->AddAttribute("End_Channel", wxString::Format("%d", (*it)->End_Channel));
		node->AddAttribute("IP", (*it)->IP);
		node->AddAttribute("S1_IP", (*it)->S1_IP);
		node->AddAttribute("S2_IP", (*it)->S2_IP);
		node->AddAttribute("Name", (*it)->Name);

		root->AddChild(node);

	}

	doc.Save(_filename);
	doc.DetachRoot();

}


void Set_EndPoint_EL_Max_Controller_Values(EasyLights_EndPoint *EP)
{
	if(EP->Model.size() < 3 || EP->Model == "Size")
	{
		EP->MaxChannels = 0;
		EP->MaxUniverses = 0;
		EP->Slave_Control = 0;
		EP->Max_SPI_Ports = 0;
		EP->Max_Diff_Ports = 0;
		EP->Master_Capable = 0;
		EP->Slave_Attached = 0;
	}
	else
	{
		for(auto ec = EL_Ptr->EL_Controller.begin(); ec != EL_Ptr->EL_Controller.end(); ++ec)
		{
			if((*ec)->Model == EP->Model)
			{
				EP->MaxChannels = (*ec)->MaxChannels;
				EP->MaxUniverses = (*ec)->MaxUniverses;
				EP->Slave_Control = (*ec)->Slave_Control;
				EP->Max_SPI_Ports = (*ec)->Max_SPI_Ports;
				EP->Max_Diff_Ports = (*ec)->Max_Diff_Ports;
				EP->Master_Capable = (*ec)->Master_Capable;
			}
		}

	}


}


void printAudioFrameInfo(const AVCodecContext* codecContext, const AVFrame* frame)
{
	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));


	// See the following to know what data type (unsigned char, short, float, etc) to use to access the audio data:
	// http://ffmpeg.org/doxygen/trunk/samplefmt_8h.html#af9a51ca15301871723577c730b5865c5

	std::ostringstream stringStream;

	stringStream << "Audio frame info:\n"
		<< "  Sample count: " << frame->nb_samples << '\n'
		<< "  Channel count: " << codecContext->channels << '\n'
		<< "  Format: " << av_get_sample_fmt_name(codecContext->sample_fmt) << '\n'
		<< "  Bytes per sample: " << av_get_bytes_per_sample(codecContext->sample_fmt) << '\n'
		<< "  Is planar? " << av_sample_fmt_is_planar(codecContext->sample_fmt) << '\n';


	stringStream << "frame->linesize[0] tells you the size (in bytes) of each plane\n";

	if(codecContext->channels > AV_NUM_DATA_POINTERS && av_sample_fmt_is_planar(codecContext->sample_fmt))
	{
		stringStream << "The audio stream (and its frames) have too many channels to fit in\n"
			<< "frame->data. Therefore, to access the audio data, you need to use\n"
			<< "frame->extended_data to access the audio data. It's planar, so\n"
			<< "each channel is in a different element. That is:\n"
			<< "  frame->extended_data[0] has the data for channel 1\n"
			<< "  frame->extended_data[1] has the data for channel 2\n"
			<< "  etc.\n";
	}
	else
	{
		stringStream << "Either the audio data is not planar, or there is enough room in\n"
			<< "frame->data to store all the channels, so you can either use\n"
			<< "frame->data or frame->extended_data to access the audio data (they\n"
			<< "should just point to the same data).\n";
	}

	stringStream << "If the frame is planar, each channel is in a different element.\n"
		<< "That is:\n"
		<< "  frame->data[0]/frame->extended_data[0] has the data for channel 1\n"
		<< "  frame->data[1]/frame->extended_data[1] has the data for channel 2\n"
		<< "  etc.\n";

	stringStream << "If the frame is packed (not planar), then all the data is in\n"
		<< "frame->data[0]/frame->extended_data[0] (kind of like how some\n"
		<< "image formats have RGB pixels packed together, rather than storing\n"
		<< " the red, green, and blue channels separately in different arrays.\n";

	std::string copyOfStr = stringStream.str();
	logger_base.debug(copyOfStr);

}

int Create_EasyLights_PCM_File(std::string &Xfseq_FN, std::string &FN_pcm, EasyLights_FSEQ_Dialog *Display, uint32_t *Size_PCM_Bytes, uint32_t *Sample_Rate_Used)
{
	wxString T;
	std::string TT;
	int mp3_info = 0;
	int outSamples;
	char *p;
	int Little_Endian = 0;
	uint16_t *W;
	int i;
	uint32_t Number_PCM_Stereo_Samples = 0;

	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	std::string FN_Audio = FSEQFile::getMediaFilename(Xfseq_FN);

	if(FN_Audio.size() < 3)
	{
		p = "EL Audio File Name could NOT be extracted from FSEQ file";
		Display->Write_Msg_to_Display(p);
		logger_base.debug(p);
		return 1;
	}

	wxFile fo;

	fo.Open(FN_pcm, wxFile::write);
	if(!fo.IsOpened())
	{
		T = wxString::Format("Write Open Failed %s", FN_pcm);
		Display->Write_Msg_to_Display(T);
		logger_base.debug(T);
		return 1;

	}

	// Audio samples are signed 16bit ints.  EasyLights wants audio sample in little-endian
	
	outSamples = 1;
	p = (char*)&outSamples;
	Little_Endian = *p;	

	if(Little_Endian)
		logger_base.debug("Easylights Audio Decoding is on a Little-Endian machine");
	else
		logger_base.debug("Easylights Audio Decoding is on a Big-Endian machine");




	// Initialize FFmpeg
	av_register_all();	// make all codecs available

	AVFrame *frame = av_frame_alloc();	// does not create working buffers
	if(!frame)
	{
		TT = "Error allocating the frame";
		logger_base.debug(TT);
		Display->Write_Msg_to_Display(TT);

		return 1;
	}

	AVFormatContext* formatContext = NULL;
	if(avformat_open_input(&formatContext, FN_Audio.c_str(), NULL, NULL) != 0)
	{
		av_frame_free(&frame);
		TT = "Error opening the Audio file";
		logger_base.debug(TT);
		Display->Write_Msg_to_Display(TT);

		return 1;
	}

	if(avformat_find_stream_info(formatContext, NULL) < 0)
	{
		av_frame_free(&frame);
		avformat_close_input(&formatContext);
		TT = "Error finding the Audio stream info";
		logger_base.debug(TT);
		Display->Write_Msg_to_Display(TT);

		return 1;
	}

	// Find the audio stream
	AVCodec* cdc = nullptr;
	int streamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_AUDIO, -1, -1, &cdc, 0);
	if(streamIndex < 0)
	{
		av_frame_free(&frame);
		avformat_close_input(&formatContext);
		TT = "Could not find any audio stream in the file";
		logger_base.debug(TT);
		Display->Write_Msg_to_Display(TT);

		return 1;
	}

	AVStream* audioStream = formatContext->streams[streamIndex];
	AVCodecContext* codecContext = audioStream->codec;
	codecContext->codec = cdc;

	if(avcodec_open2(codecContext, codecContext->codec, NULL) != 0)	// open the codec
	{
		av_frame_free(&frame);
		avformat_close_input(&formatContext);
		TT =  "Couldn't open the context with the decoder";
		logger_base.debug(TT);
		Display->Write_Msg_to_Display(TT);

		return 1;
	}

	T = wxString::Format("This Audio stream has %d channels and a sample rate of %d Hz", codecContext->channels, codecContext->sample_rate);
	logger_base.debug(T);
	Display->Write_Msg_to_Display(T);

	T = wxString::Format("The Audio data is in the format %s", av_get_sample_fmt_name(codecContext->sample_fmt));
	logger_base.debug(T);
	Display->Write_Msg_to_Display(T);

	// setup our conversion format ... we need to conver the input to a standard format before we can process anything
	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
	AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	int out_sample_rate = 22050;

	*Sample_Rate_Used = out_sample_rate;

#define CONVERSION_BUFFER_SIZE 192000
	uint8_t* out_buffer = (uint8_t *)av_malloc(CONVERSION_BUFFER_SIZE * out_channels * 2); // 1 second of audio

	int64_t in_channel_layout = av_get_default_channel_layout(codecContext->channels);

	struct SwrContext *au_convert_ctx = swr_alloc_set_opts(nullptr, out_channel_layout, out_sample_fmt, out_sample_rate,
		in_channel_layout, codecContext->sample_fmt, codecContext->sample_rate, 0, nullptr);

	if(au_convert_ctx == nullptr)
	{
		logger_base.error("DoLoadAudioData: swe_alloc_set_opts was null");
		// let it go as it may be the cause of a crash
		wxASSERT(false);
	}

	swr_init(au_convert_ctx);


	AVPacket readingPacket;
	av_init_packet(&readingPacket);

	// Read the packets in a loop
	while(av_read_frame(formatContext, &readingPacket) == 0)
	{
		if(readingPacket.stream_index == audioStream->index)
		{
			AVPacket decodingPacket = readingPacket;

			// Audio packets can have multiple audio frames in a single packet
			while(decodingPacket.size > 0)
			{
				// Try to decode the packet into a frame
				// Some frames rely on multiple packets, so we have to make sure the frame is finished before
				// we can use it
				int gotFrame = 0;
				int result = avcodec_decode_audio4(codecContext, frame, &gotFrame, &decodingPacket);

				if(result >= 0 && gotFrame)
				{
					decodingPacket.size -= result;
					decodingPacket.data += result;

					// We now have a fully decoded audio frame
					outSamples = swr_convert(au_convert_ctx, &out_buffer, CONVERSION_BUFFER_SIZE, (const uint8_t **)frame->data, frame->nb_samples);

					// save pcm data

					Number_PCM_Stereo_Samples += outSamples;

					if(Little_Endian)
						fo.Write(out_buffer, (outSamples * out_channels * 2));
					else // big endian - need to swapp
					{
						W = (uint16_t*)out_buffer;

						for(i = 0; i < (outSamples * 2), i++;)	// stereo - two 16 bits words per sample
						{
							*W = wxUINT16_SWAP_ALWAYS(*W);
							W++;
						}
					}



					if(mp3_info == 0)
					{
						mp3_info = 1;
						printAudioFrameInfo(codecContext, frame);
					}
				}
				else
				{
					decodingPacket.size = 0;
					decodingPacket.data = nullptr;
				}
			}
		}

		// You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
		av_packet_unref(&readingPacket);
	}

	// Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
	// is set, there can be buffered up frames that need to be flushed, so we'll do that
	if(codecContext->codec->capabilities & CODEC_CAP_DELAY)
	{
		av_init_packet(&readingPacket);
		// Decode all the remaining frames in the buffer, until the end is reached
		int gotFrame = 0;
		while(avcodec_decode_audio4(codecContext, frame, &gotFrame, &readingPacket) >= 0 && gotFrame)
		{
			// We now have a fully decoded audio frame
			outSamples = swr_convert(au_convert_ctx, &out_buffer, CONVERSION_BUFFER_SIZE, (const uint8_t **)frame->data, frame->nb_samples);

			// save pcm data

			Number_PCM_Stereo_Samples += outSamples;

			if(Little_Endian)
				fo.Write(out_buffer, (outSamples * out_channels * 2));
			else // big endian - need to swapp
			{
				W = (uint16_t*)out_buffer;

				for(i = 0; i < (outSamples * 2), i++;)	// stereo - two 16 bits words per sample
				{
					*W = wxUINT16_SWAP_ALWAYS(*W);
					W++;
				}
			}
		}
	}

	fo.Close();

	*Size_PCM_Bytes = Number_PCM_Stereo_Samples * 4;	// stereo - two channels 16 bits each channel

	T = wxString::Format("Audio PCM - %d seconds in duration", (Number_PCM_Stereo_Samples / out_sample_rate));
	logger_base.debug(T);
	Display->Write_Msg_to_Display(T);

	// Clean up!
	swr_free(&au_convert_ctx);
	av_free(out_buffer);
	av_frame_free(&frame);
	avcodec_close(codecContext);
	avformat_close_input(&formatContext);

	return 0;
}

int Make_WAV_From_PCM(std::string &EL_PCM, std::string &EL_WAV, uint32_t Size_PCM_Bytes, uint32_t Sample_Rate_Used)
{
	wavHeader WH;
	uint32_t r;
	uint32_t T;


	WH.RIFF[0] = 'R';
	WH.RIFF[1] = 'I';
	WH.RIFF[2] = 'F';
	WH.RIFF[3] = 'F';

	//WH.ChunkSize = Size_PCM_Bytes + 36;
	T = Size_PCM_Bytes + 36;
	write4ByteUInt((uint8_t*)&WH.ChunkSize, T);

	WH.WAVE[0] = 'W';
	WH.WAVE[1] = 'A';
	WH.WAVE[2] = 'V';
	WH.WAVE[3] = 'E';

	WH.fmt[0] = 'f';
	WH.fmt[1] = 'm';
	WH.fmt[2] = 't';
	WH.fmt[3] = ' ';

	//WH.Subchunk1Size = 16;
	T = 16;
	write4ByteUInt((uint8_t*)&WH.Subchunk1Size, T);

	//WH.AudioFormat = 1;
	T = 1;
	write2ByteUInt((uint8_t*)&WH.AudioFormat, T);

	//WH.NumOfChannels = 2;
	T = 2;
	write2ByteUInt((uint8_t*)&WH.NumOfChannels, T);

	//WH.SamplesPerSec = 44100;
	T = Sample_Rate_Used;
	write4ByteUInt((uint8_t*)&WH.SamplesPerSec, T);

	//WH.bytesPerSec = 176400;
	T = Sample_Rate_Used * 4;
	write4ByteUInt((uint8_t*)&WH.bytesPerSec, T);

	//WH.blockAlign = 4;
	T = 4;
	write2ByteUInt((uint8_t*)&WH.blockAlign, T);

	//WH.bitsPerSample = 16;
	T = 16;
	write2ByteUInt((uint8_t*)&WH.bitsPerSample, T);


	WH.Subchunk2ID[0] = 'd';
	WH.Subchunk2ID[1] = 'a';
	WH.Subchunk2ID[2] = 't';
	WH.Subchunk2ID[3] = 'a';

	//WH.Subchunk2Size = Size_PCM_Bytes;
	T = Size_PCM_Bytes;
	write4ByteUInt((uint8_t*)&WH.Subchunk2Size, T);


	wxFile fo;
	wxFile fi;

	fo.Open(EL_WAV, wxFile::write);
	if(!fo.IsOpened())
	{
		return 1;
	}

	fo.Write(&WH, sizeof(WH));

	fi.Open(EL_PCM, wxFile::read);
	if(!fi.IsOpened())
	{
		fo.Close();
		return 1;
	}

	uint8_t *Buff = (uint8_t*)malloc(Size_PCM_Bytes);
	if(Buff == 0)
	{
		fo.Close();
		fi.Close();
		return 1;
	}

	r = fi.Read(Buff, Size_PCM_Bytes);
	if(r != Size_PCM_Bytes)
	{
		fo.Close();
		fi.Close();
		return 1;
	}


	fo.Write(Buff, Size_PCM_Bytes);

	fo.Close();
	fi.Close();

	return 0;


}


EasyLights_EndPoint *Find_EndPoint_By_IP(std::string &IP)
{
	for(auto it = EL_Ptr->EL_EndPoints.begin(); it != EL_Ptr->EL_EndPoints.end(); ++it)
	{
		if((*it)->IP == IP)
			return *it;
	}

	return 0;
}


struct Tag_Controller_Channels
{
	uint32_t Start_Channel_Zero_Base;
	uint32_t Num_Channels;
};

int Create_EasyLights_FSEQ_File(EL_FSEQ_Info *ELC, EasyLights_FSEQ_Dialog *Display)
{
	FSEQFile *xf;
	FSEQFile *ef;
	wxString T;
	uint32_t xNumber_of_Frames;
	uint32_t numChannels;
	int xFrame_Rate;
	EasyLights_EndPoint *Slave;
	uint8_t *VHeader = 0;
	int Size_VHeader = 0;

	struct Tag_Controller_Channels Controllers[3];

	memset(Controllers, 0, sizeof(Controllers));
	int Cindex = 0;

	static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

	T = wxString::Format("EL FSEQ Creation from %s", ELC->IX ->FN_short);
	logger_base.debug(T);
	Display->Write_Msg_to_Display(T);


	xf = FSEQFile::openFSEQFile(ELC->IX->FN_long);
	if(!xf) 
	{
		T = wxString::Format("EasyLights Failed opening FSEQ %s", ELC->IX->FN_short);
		logger_base.debug(T);
		Display->Write_Msg_to_Display(T);
		return 1;
	}

	xNumber_of_Frames = xf->getNumFrames();
	numChannels = xf->getChannelCount();
	xFrame_Rate = xf->getStepTime();


	std::vector<std::pair<uint32_t, uint32_t>> rng;
	rng.push_back(std::pair<uint32_t, uint32_t>(0, numChannels));
	xf->prepareRead(rng);

// read file ready -- do write file

	ef = FSEQFile::createFSEQFile(ELC->ELC_FN_long, 1, FSEQFile::none, 0);
	if(!ef) 
	{
		T = wxString::Format("EasyLights Failed Write opening FSEQ %s", ELC->ELC_FN_short);
		logger_base.debug(T);
		Display->Write_Msg_to_Display(T);
		delete xf;
		return 1;
	}

	// acquire channel data on all controllers

	Controllers[0].Start_Channel_Zero_Base = ELC->EP->Start_Channel - 1;
	Controllers[0].Num_Channels = ELC->EP->Channels_Used;
	Cindex++;

	if(ELC->EP->S1_IP.size() > 3)
	{
		Slave = Find_EndPoint_By_IP(ELC->EP->S1_IP);
		if(Slave)
		{
			Controllers[Cindex].Start_Channel_Zero_Base = Slave->Start_Channel - 1;
			Controllers[Cindex].Num_Channels = Slave->Channels_Used;
			Cindex++;
		}
	}

	if(ELC->EP->S2_IP.size() > 3)
	{
		Slave = Find_EndPoint_By_IP(ELC->EP->S2_IP);
		if(Slave)
		{
			Controllers[Cindex].Start_Channel_Zero_Base = Slave->Start_Channel - 1;
			Controllers[Cindex].Num_Channels = Slave->Channels_Used;
			Cindex++;
		}
	}


	int ef_Num_Channel_To_Write = Controllers[0].Num_Channels + Controllers[1].Num_Channels + Controllers[2].Num_Channels;

	uint8_t *src, *dest;

	ef->setChannelCount(ef_Num_Channel_To_Write);
	ef->setStepTime(xFrame_Rate);
	ef->setNumFrames(xNumber_of_Frames);

	if(Cindex > 1)	// we have slaves = make variable header for fseq
	{
		VHeader = Get_VHeader_for_ELC_FSEQ(ELC, &Size_VHeader);

		FSEQFile::VariableHeader header;
		header.code[0] = 'e';
		header.code[1] = 'l';
		header.data.resize(Size_VHeader);
		memmove(&header.data[0], VHeader, Size_VHeader);
		ef->addVariableHeader(header);

		free(VHeader);
		VHeader = 0;
	}

	ef->writeHeader();	// ready for frame data

	uint8_t *WriteBuf = new uint8_t[ef_Num_Channel_To_Write];

	// read buff
	uint8_t *tmpBuf = new uint8_t[numChannels];

	uint32_t frame = 0;

	while(frame < xNumber_of_Frames)
	{
		FSEQFile::FrameData *data = xf->getFrame(frame);

		data->readFrame(tmpBuf);	// we have a read frame

		// move wanted write channels into write frame buffer

		src = tmpBuf + Controllers[0].Start_Channel_Zero_Base;		 // start of my data with global channel array
		dest = WriteBuf;
		memmove(dest, src, Controllers[0].Num_Channels);
		dest += Controllers[0].Num_Channels;

		if(Controllers[1].Num_Channels)
		{
			src = tmpBuf + Controllers[1].Start_Channel_Zero_Base;		 // start of my data with global channel array
			memmove(dest, src, Controllers[1].Num_Channels);
			dest += Controllers[1].Num_Channels;
		}

		if(Controllers[2].Num_Channels)
		{
			src = tmpBuf + Controllers[2].Start_Channel_Zero_Base;		 // start of my data with global channel array
			memmove(dest, src, Controllers[2].Num_Channels);
			dest += Controllers[2].Num_Channels;
		}

		ef->addFrame(frame, WriteBuf);

		delete data;
		frame++;
	}

	ef->finalize();

	delete[]tmpBuf;
	delete[]WriteBuf;

	delete xf;
	delete ef;

	T = wxString::Format("EasyLights Completed FSEQ %s", ELC->ELC_FN_short);
	logger_base.debug(T);
	Display->Write_Msg_to_Display(T);


	return 0;
}


int Return_FSEQ_Version(std::string FSEQ_FN)
{
	int V;
	FSEQFile *f;

	f = FSEQFile::openFSEQFile(FSEQ_FN);
	if(f == 0)
	{
		return 0;
	}

	V = f->getVersionMajor();

	delete f;

	return V;
}

#define VHeader_Size 5000

uint8_t *Get_VHeader_for_ELC_FSEQ(EL_FSEQ_Info *ELC, int *VHSize)
{
	uint8_t *H = (uint8_t*)malloc(VHeader_Size);	// plenty big
	struct Tag_FSEQ_Variable_Header_Main *HM = (struct Tag_FSEQ_Variable_Header_Main*)H;
	struct Tag_FSEQ_Variable_Header_H1 *H1 = (struct Tag_FSEQ_Variable_Header_H1*)(H + sizeof(struct Tag_FSEQ_Variable_Header_Main));
	uint8_t *D = H + sizeof(struct Tag_FSEQ_Variable_Header_Main) + sizeof(struct Tag_FSEQ_Variable_Header_H1);
	EasyLights_EndPoint *Slave;
	uint16_t VHsize = 0;
	uint32_t Offset;
	std::vector< Slave_Univ_Chan> *SUC1 = 0;
	std::vector< Slave_Univ_Chan> *SUC2 = 0;

	// fill in H1
	memset(H, 0, VHeader_Size);

	write4ByteUInt((uint8_t*)&H1->Main_Num_Channels, ELC->EP->Channels_Used);

	if(ELC->EP->S1_IP.size() > 3)
	{
		Slave = Find_EndPoint_By_IP(ELC->EP->S1_IP);
		if(Slave)
		{
			HM->Number_Slaves++;

			H1->Slave1_Active = 1;
			write2ByteUInt((uint8_t*)&H1->Slave1_Num_Channels, Slave->Channels_Used);

			SUC1 = Get_Slave_Universe_Channel_Config(ELC->EP->S1_IP);

			H1->Slave1_Num_Univ = SUC1->size();

			Offset = D - H;
			write2ByteUInt((uint8_t*)&H1->Slave1_Univ_Offset, Offset);
			D = Add_Slave_Univ_Chan_Data_to_VHeader(D, SUC1);

			SUC1->clear();
			SUC1->shrink_to_fit();
			delete SUC1;


		}
	}

	if(ELC->EP->S2_IP.size() > 3)
	{
		Slave = Find_EndPoint_By_IP(ELC->EP->S2_IP);
		if(Slave)
		{
			HM->Number_Slaves++;

			H1->Slave2_Active = 1;
			write2ByteUInt((uint8_t*)&H1->Slave2_Num_Channels, Slave->Channels_Used);

			SUC2 = Get_Slave_Universe_Channel_Config(ELC->EP->S2_IP);

			H1->Slave2_Num_Univ = SUC2->size();

			Offset = D - H;
			write2ByteUInt((uint8_t*)&H1->Slave2_Univ_Offset, Offset);
			D = Add_Slave_Univ_Chan_Data_to_VHeader(D, SUC2);

			SUC2->clear();
			SUC2->shrink_to_fit();
			delete SUC2;

		}
	}

	VHsize = D - H;
	*VHSize = VHsize;
	write2ByteUInt((uint8_t*)&HM->Total_Size_VHeader, VHsize);

	return H;
	
}


std::vector< Slave_Univ_Chan> *Get_Slave_Universe_Channel_Config(std::string IP)
{
	Slave_Univ_Chan T;

	std::vector< Slave_Univ_Chan> *SUC = new std::vector< Slave_Univ_Chan>;

	// Get universes based on IP
	std::list<Output*> outputs = EL_Ptr->OutPutMgr->GetAllOutputs(IP);


	for(auto it = outputs.begin(); it != outputs.end(); ++it)
	{


		T.Univ = (*it)->GetUniverse();
		T.Num_Chan = (*it)->GetChannels();
		T.Start_Chan = (*it)->GetStartChannel();

		SUC->push_back(T);

	}

	return SUC;
}

uint8_t *Add_Slave_Univ_Chan_Data_to_VHeader(uint8_t *D, std::vector< Slave_Univ_Chan> *SUC)
{
	for(auto it = SUC->begin(); it != SUC->end(); ++it)
	{
		write2ByteUInt(D, it->Univ);
		D+= 2;
		write2ByteUInt(D, it->Num_Chan);
		D += 2;
		write2ByteUInt(D, it->Start_Chan);
		D += 2;
	}

	return D;

}


int ELC_Status_To_Support_Slaves(EasyLights_EndPoint *ELC)
{
	EasyLights_EndPoint *Slave;

	if(ELC->Channels_Used > ELC_NO_Slaves_Channel_Count)
		return ELC_Support_NO_Slaves;

	if(ELC->Channels_Used > ELC_ONE_Slaves_Channel_Count)	// only support one small slave
	{
		if(ELC->S1_IP.size() < 3 && ELC->S2_IP.size() < 3)	// no slaves at this time
			return ELC_Support_One_Slave;	// can only support 1 slave
		else
			return ELC_Support_NO_Slaves;
	}

	// elc can support 1 big slave or 2 little ones

	if(ELC->S1_IP.size() < 3 && ELC->S2_IP.size() < 3)	// no slaves at this time
		return ELC_Support_TWO_Slaves;

	if(ELC->S1_IP.size() > 3 && ELC->S2_IP.size() > 3)	// 2 slaves at this time
		return ELC_Support_NO_Slaves;

	if(ELC->S1_IP.size() > 3)
	{
		Slave = Find_EndPoint_By_IP(ELC->S1_IP);
		if(Slave)
		{
			if(Slave->Channels_Used < Slave_Max_Channel_Count_For_Two)
				return ELC_Support_One_Slave;

			return ELC_Support_NO_Slaves;
		}
	}

	if(ELC->S2_IP.size() > 3)
	{
		Slave = Find_EndPoint_By_IP(ELC->S2_IP);
		if(Slave)
		{
			if(Slave->Channels_Used < Slave_Max_Channel_Count_For_Two)
				return ELC_Support_One_Slave;

			return ELC_Support_NO_Slaves;
		}
	}

	return ELC_Support_NO_Slaves;
}

int Slave_Consumes_This_Number_Of_Controllers(std::string IP)
{
	EasyLights_EndPoint *Slave;

	Slave = Find_EndPoint_By_IP(IP);
	if(Slave)
	{
		if(Slave->Channels_Used > Slave_Max_Channel_Count_for_One)
			return 10;	// too big, can't use

		if(Slave->Channels_Used > Slave_Max_Channel_Count_For_Two)
			return 2;

		return 1;
	}

	return 0;
}

int Slave_Consumes_This_Number_Of_Controllers(EasyLights_EndPoint *Slave)
{
	if(Slave->Channels_Used > Slave_Max_Channel_Count_for_One)
		return 10;	// too big, can use

	if(Slave->Channels_Used > Slave_Max_Channel_Count_For_Two)
		return 2;

	return 1;


}

char *Return_EasuLights_FSEQ_FN_Without_IP(char *S)
{
	char *B = (char*)malloc(200);
	char *p;

	strncpy(B, S, 199);

	p = B;
	while(*p) p++;
	while(p > B && *p != ' ') p--;
	if(*p == ' ')
	{
		*p = 0;
		p++;
	}
	else
	{
		free(B);
		return 0;
	}

	// this should be start of ip

	if(*p < '0' || *p > '9')
	{
		free(B);
		return 0;
	}

	p = strstr(p, ".fseq");
	if(p == 0)
	{
		free(B);
		return 0;
	}

	strcat(B, p);

	return B;
}

char *Get_Next_JArg(char *p, char **JF, char **JV, int *Last_Key, int Ignore_Bracket)
{
	int Quote = 0;
	char *k;

	if(Last_Key)
		*Last_Key = 0;

	while(*p && (*p == ' ' || *p == '\t' || *p == '{' || *p == '}' || *p == '[' || *p == ']' || *p == ',')) p++;	// find start - skip leading space

	if(*p == 0)
		return 0;

	// parse KEY

	if(*p == '\"')
	{
		Quote = 1;
		p++;
	}

	*JF = p;


	if(Quote)
	{
		while(*p && *p != '\"')	// cAP and find end
		{
			*p = toupper(*p);
			p++;
		}

		if(*p == 0)
			return 0;	// error

		*p = 0;	// terminate quote
		p++;	// step over

		if(*p != ':')	// next char must be colon
			return 0;	// error


		p++;	// over colon
	}
	else
	{
		while(*p && *p != ':')	// cAP and find end
		{
			*p = toupper(*p);
			p++;
		}

		if(*p == 0)
			return 0;	// error

		*p = 0;	// terminate colon

		p++;	// over colon
	}

	// parse VALUE

	Quote = 0;

	while(*p && (*p == ' ' || *p == '\t')) p++;	// skip leading space

	if(*p == '\"')
	{
		Quote = 1;
		p++;
	}

	*JV = p;

	if(Quote)
	{
		if(Ignore_Bracket)
		{
			// if data is an array - bracket follows quote
			if(*p == '[')	// data is an array
			{
				p++;	// step over

			}
		}
		else
		{
			// if data is an array - bracket follows quote
			if(*p == '[')	// data is an array
			{
				p++;	// step over

				while(*p && *p != ']') p++;	// find end

				p++;	// step over
			}
		}

		while(*p && *p != '\"') p++;	// find end

		if(*p == 0)
			return 0;	// error

		*p = 0;	// terminate quote
		p++;	// step over

		while(*p && *p != ',' && *p != '{' && *p != '}' && *p != '[' && *p != ']') p++;	// find end of value
	}
	else
	{
		if(Ignore_Bracket)
		{
			// if data is an array - bracket follows quote
			if(*p == '[')	// data is an array
			{
				p++;	// step over

			}
		}
		else
		{
			// if data is an array - bracket follows quote
			if(*p == '[')	// data is an array
			{
				p++;	// step over

				while(*p && *p != ']') p++;	// find end

				p++;	// step over
			}
		}



		while(*p && *p != ',' && *p != '{' && *p != '}' && *p != '[' && *p != ']') p++;	// find end
	}


	if(*p)
	{
		*p = 0;
		p++;
	}

	// now see if last key

	if(Last_Key)
	{
		k = p;
		while(*k && *k != ':') k++;
		if(*k == 0)
			*Last_Key = 1;
	}


	return p;

}

static const char *Temp_ELC_Spec = 
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	"<Controller_MFG>\n"
	"<Vendor name=\"EasyLights\">\n"
	"<controller Type=\"Pixel\" Model=\"HinksPix16\" MaxChannels=\"33280\" MaxUniverses=\"65\" Slave_Control=\"1\" Max_SPI_Ports=\"16\"  Max_Diff_Ports=\"0\" Master_Capable=\"0\"/>\n"
	"<controller Type=\"Pixel\" Model=\"EasyLightsPix16_V1\"  MaxChannels=\"33280\" MaxUniverses=\"65\" Slave_Control=\"1\" Max_SPI_Ports=\"16\"  Max_Diff_Ports=\"0\"  Master_Capable=\"1\"/>\n"
	"<controller Type=\"Pixel\" Model=\"EasyLightsPix16_V2\"  MaxChannels=\"33280\" MaxUniverses=\"65\" Slave_Control=\"1\" Max_SPI_Ports=\"16\"  Max_Diff_Ports=\"8\" Master_Capable=\"1\"/>\n"
	"<controller Type=\"AC\" Model=\"EasyLightsAC24_V1\"  MaxChannels=\"24\" MaxUniverses=\"1\" Slave_Control=\"1\" Max_SPI_Ports=\"0\"  Max_Diff_Ports=\"0\" Master_Capable=\"1\"/>\n"
	"<controller Type=\"AC\" Model=\"EasyLightsAC24_V2\"  MaxChannels=\"24\" MaxUniverses=\"17\" Slave_Control=\"1\" Max_SPI_Ports=\"0\"  Max_Diff_Ports=\"8\" Master_Capable=\"1\"/>\n"
	"</Vendor>\n"
	"</Controller_MFG>\n";


void Create_Temp_EasyLights_spec_File()
{

	wxString FN = wxString::Format("%s\\EasyLightsData\\%s", EL_Ptr->ShowDirectory, "EasyLights_Controller_Specification.xml");
	std::string fn = FN;

	FILE *fo = fopen((const char *)fn.c_str(), "w");

	if(fo == 0)
		return;

	fputs(Temp_ELC_Spec, fo);

	fclose(fo);

}