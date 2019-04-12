/*
* ZCPP.h
*
* Project: Official ZCPP Protocol Header File
* Copyright (c) 2019 Keith Westley
*
*  This program is provided free for you to use in any way that you wish,
*  subject to the laws and regulations where you are using it.  Due diligence
*  is strongly suggested before using this code.  Please give credit where due.
*
*  The Author makes no warranty of any kind, express or implied, with regard
*  to this program or the documentation contained in this document.  The
*  Author shall not be liable in any event for incidental or consequential
*  damages in connection with, or arising out of, the furnishing, performance
*  or use of these programs.
*
*/

#ifndef ZCPP_H
#define ZCPP_H

#define ZCPP_MULTICAST_ADDRESS "224.0.30.5"
#define ZCPP_MULTICAST_DATA_ADDRESS "224.0.31."

// Defaults
#define ZCPP_PORT 30005

// Packet Type Codes
#define ZCPP_TYPE_DISCOVERY 0x00
#define ZCPP_TYPE_DISCOVERY_RESPONSE 0x01
#define ZCPP_TYPE_CONFIG 0x0A
#define ZCPP_TYPE_EXTRA_DATA 0x0B
#define ZCPP_TYPE_QUERY_CONFIG 0x0C
#define ZCPP_TYPE_QUERY_CONFIG_RESPONSE 0x0D
#define ZCPP_TYPE_DATA 0x14
#define ZCPP_TYPE_SYNC 0x15

#define ZCPP_CURRENT_PROTOCOL_VERSION 0x00

#define ZCPP_RS_PORT_MASK 0x80

#define ZCPP_PRIORITY_RESET_SECONDS 2

#define ZCPP_VENDOR_FALCON 0x0000
#define ZCPP_VENDOR_FPP 0x0001
#define ZCPP_VENDOR_ESPIXELSTICK 0x0002
#define ZCPP_VENDOR_UNKNOWN 0xFFFF

#define ZCPP_DISCOVERY_PROTOCOL_WS2811  0x00000001
#define ZCPP_DISCOVERY_PROTOCOL_GECE    0x00000002
#define ZCPP_DISCOVERY_PROTOCOL_DMX     0x00000004
#define ZCPP_DISCOVERY_PROTOCOL_LX1203  0x00000008
#define ZCPP_DISCOVERY_PROTOCOL_TLS3001 0x00000010
#define ZCPP_DISCOVERY_PROTOCOL_LPD6803 0x00000020
#define ZCPP_DISCOVERY_PROTOCOL_WS2801  0x00000040
#define ZCPP_DISCOVERY_PROTOCOL_SM16716 0x00000080
#define ZCPP_DISCOVERY_PROTOCOL_MB16020 0x00000100
#define ZCPP_DISCOVERY_PROTOCOL_MY9231  0x00000200
#define ZCPP_DISCOVERY_PROTOCOL_APA102  0x00000400
#define ZCPP_DISCOVERY_PROTOCOL_MY9221  0x00000800
#define ZCPP_DISCOVERY_PROTOCOL_SK6812  0x00001000
#define ZCPP_DISCOVERY_PROTOCOL_UCS1903 0x00002000
#define ZCPP_DISCOVERY_PROTOCOL_TM18XX  0x00004000
#define ZCPP_DISCOVERY_PROTOCOL_RENARD  0x00008000
#define ZCPP_DISCOVERY_PROTOCOL_LPD8806 0x00010000
#define ZCPP_DISCOVERY_PROTOCOL_DM412   0x00020000
#define ZCPP_DISCOVERY_PROTOCOL_P9813   0x00040000

#define ZCPP_PROTOCOL_WS2811 0x00
#define ZCPP_PROTOCOL_GECE 0x01
#define ZCPP_PROTOCOL_DMX 0x02
#define ZCPP_PROTOCOL_LX1203 0x03
#define ZCPP_PROTOCOL_TLS3001 0x04
#define ZCPP_PROTOCOL_LPD6803 0x05
#define ZCPP_PROTOCOL_WS2801 0x06
#define ZCPP_PROTOCOL_SM16716 0x07
#define ZCPP_PROTOCOL_MB16020 0x08
#define ZCPP_PROTOCOL_MY9231 0x09
#define ZCPP_PROTOCOL_APA102 0x0A
#define ZCPP_PROTOCOL_MY9221 0x0B
#define ZCPP_PROTOCOL_SK6812 0x0C
#define ZCPP_PROTOCOL_UCS1903 0x0D
#define ZCPP_PROTOCOL_TM18XX 0x0E
#define ZCPP_PROTOCOL_RENARD 0x0F
#define ZCPP_PROTOCOL_LPD8806 0x10
#define ZCPP_PROTOCOL_DM412   0x11
#define ZCPP_PROTOCOL_P9813   0x12

// when ZCPP_DISCOVERY_FLAG_SEND_DATA_AS_MULTICAST is set the controllers IP is slightly ignored
// if controller IP is 10.10.10.10 then the data will be multicast to 224.0.31.10 ... ie the last octet is added
// to ZCPP_MULTICAST_DATA_ADDRESS
#define ZCPP_DISCOVERY_FLAG_SEND_DATA_AS_MULTICAST 0x0002
#define ZCPP_DISCOVERY_FLAG_CONFIGURATION_LOCKED 0x0004
#define ZCPP_DISCOVERY_FLAG_SUPPORTS_VIRTUAL_STRINGS 0x0008
#define ZCPP_DISCOVERY_FLAG_SUPPORTS_SMART_REMOTES 0x0010

#define ZCPP_SMART_REMOTE_MASK 0xC0
#define ZCPP_STRING_NUMBER_MASK 0x3F

#define ZCPP_REVERSE_MASK 0x80
#define ZCPP_COLOUR_ORDER_MASK 0x7

#define ZCPP_COLOUR_ORDER_RGB 0x00
#define ZCPP_COLOUR_ORDER_RBG 0x01
#define ZCPP_COLOUR_ORDER_GRB 0x02
#define ZCPP_COLOUR_ORDER_GBR 0x03
#define ZCPP_COLOUR_ORDER_BRG 0x04
#define ZCPP_COLOUR_ORDER_BGR 0x05

#define ZCPP_CONFIG_FLAG_QUERY_CONFIGURATION_RESPONSE_REQUIRED 0x10
#define ZCPP_CONFIG_FLAG_FIRST 0x40
#define ZCPP_CONFIG_FLAG_LAST 0x80

#define ZCPP_CONFIG_MAX_PORT_PER_PACKET 90

#define ZCPP_CONFIGURATION_QUERY_ERRORS 0x01

#define ZCPP_DATA_FLAG_SYNC_WILL_BE_SENT 0x01
#define ZCPP_DATA_FLAG_FIRST 0x40
#define ZCPP_DATA_FLAG_LAST 0x80

inline uint8_t ZCPP_GetSmartRemote(uint8_t string) {
	return (string & ZCPP_SMART_REMOTE_MASK) >> 6;
}

inline uint8_t ZCPP_GetStringNumber(uint8_t string) {
	return (string & ZCPP_STRING_NUMBER_MASK);
}

inline bool ZCPP_IsReversed(uint8_t directionColourOrder) {
	return (directionColourOrder & ZCPP_REVERSE_MASK) != 0;
}

inline uint8_t ZCPP_GetColourOrder(uint8_t directionColourOrder) {
	return directionColourOrder & ZCPP_COLOUR_ORDER_MASK;
}

inline float ZCPP_GetGamma(uint8_t gamma) {
	return ((float)gamma) / 10.0;
}

inline uint8_t ZCPP_ConvertDiscoveryProtocolToProtocol(uint32_t discoveryProtocol) {
	uint8_t res = 0x00;
	discoveryProtocol = discoveryProtocol > 1;
	while (discoveryProtocol != 0) 	{
		res++;
		discoveryProtocol = discoveryProtocol > 1;
	}
	return res;
}

inline uint32_t ZCPP_ConvertProtocolToDiscoveryProtocol(uint8_t protocol)
{
	uint32_t res = 0x00000001;
	while (protocol != 0) 	{
		protocol--;
		res = res << 1;
	}
	return res;
}

const uint8_t ZCPP_token[4] = {'Z', 'C', 'P', 'P'};

#ifdef _MSC_VER
#pragma pack(push)
#pragma pack(1)
#endif

// NOTE: All 16 bit values must be aligned on even bytes
//       All 32 bit values must be aligned on 4 byte boundaries

// Common header across all packet types - 6 bytes
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	uint8_t token[4]; 			// Always ZCPP
	uint8_t type;				// packet type
	uint8_t protocolVersion;	// version of the protocol
} ZCPP_Header;

// Discovery Request - 8 bytes
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	ZCPP_Header Header;
	uint8_t minProtocolVersion; // The minimum version of the protocol the requester supports
	uint8_t maxProtocolVersion; // The maximum version of the protocol the requester supports
} ZCPP_Discovery;

// Discovery Response - 88 bytes
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	ZCPP_Header Header;
	uint8_t minProtocolVersion; // The minimum version of the protocol the controller supports
	uint8_t maxProtocolVersion; // The maximum version of the protocol the controller supports
	uint16_t vendor;			// The vendor of the controller
	uint16_t model;				// A vendor specific model code
	char firmwareVersion[12];	// A string of up to 12 characters which is the firmware version as a string. It does not
								// need to be null terminated but should be null filled if all 12 characters are not used
	uint8_t macAddress[6];		// The controllers mac Address
    uint8_t filler1[2];
	uint32_t ipv4Address;		// The controllers IP V4 IP Address
	uint32_t ipv4Mask;			// The controllers IP V4 Subnet Mask
	char userControllerName[32];// Up to 32 characters of user controller name
    uint32_t maxTotalChannels;  // Maximum number of channels that the controller will accept. This may not just be the sum
                                // of all port channels as the controller may have some global limits
    uint8_t pixelPorts;			// Number of pixel ports supported by the controller
	uint8_t rsPorts;			// Number of RSxxx ports supported by the controller
	uint16_t channelsPerPixelPort; // Maximum number of channels each pixel port can accept
	uint16_t channelsPerRSPort; // Maximum number of channels each RSxxx port can accept
    uint16_t flags;				// Discovery Flags
    uint32_t protocolsSupported; // Bitmask of all supported protocols
} ZCPP_DiscoveryResponse;

// Describes the configuration of a port or virtual string - 16 bytes
// This structure must be a multiple of 4 bytes in size
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	uint8_t port;					// zero based port that is being configured
	uint8_t string;					// smart remote and virtual string number within port
    uint8_t protocol;				// port protocol
    uint8_t grouping;				// pixel grouping on this port. If 2 then channels 123456789 becomes 123123456456789789
    uint32_t startChannel;			// zero based start channel within the ZCPP data space
	uint32_t channels;				// number of channels to send out this port
	uint8_t directionColourOrder;   // should data be reversed and what is the pixel colour order
	uint8_t nullPixels;				// number of null pixels at the start of this string
	uint8_t brightness;				// 0-100 brightness
	uint8_t gamma;					// Gamma * 10
} ZCPP_PortConfig;

// Configuration - 44 - 1442 bytes
// This structure excluding Port config must be a multiple of 4 bytes in size
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	ZCPP_Header Header;
	uint16_t sequenceNumber;		// sequence number unique each time the configuration changes
	char userControllerName[32];	// Up to 32 characters of user controller name
	uint8_t flags;					// Configuration flags
    uint8_t priority;               // priority of this source
    uint8_t filler1;
    uint8_t ports;					// Number of ports being configured
    ZCPP_PortConfig PortConfig[1];		// Up to 100 of them
} ZCPP_Configuration;
#define ZCPP_CONFIGURATION_HEADER_SIZE (sizeof(ZCPP_Configuration) - sizeof(ZCPP_PortConfig))
#define ZCPP_CONFIGURATION_HEADER_MUTABLE_SIZE 8

// Query Configuration Response 44 - 1442 bytes
// This structure must be a multiple of 4 bytes in size
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	ZCPP_Header Header;
	uint16_t sequenceNumber;		// sequence number unique each time the configuration changes
	char userControllerName[32];	// Up to 32 characters of user controller name
    uint16_t filler;
    uint8_t flags;					// Configuration result flags
	uint8_t ports;					// Number of ports configured
    ZCPP_PortConfig PortConfig[1];		// Up to 100 of them
} ZCPP_QueryConfigurationResponse;
#define ZCPP_QUERYCONFIGURATIONRESPONSE_HEADER_SIZE (sizeof(ZCPP_QueryConfigurationResponse) - sizeof(ZCPP_PortConfig))

// QueryConfiguration - 6 bytes
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	ZCPP_Header Header;
} ZCPP_QueryConfiguration;

// Extra port data - 3 - 1458 bytes
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	uint8_t port;					// zero based port that is being configured
	uint8_t string;					// smart remote and virtual string number within port
	uint8_t descriptionLength;		// length of the description which must fit entirely within this ethernet frame
    char description[1];				// the port description
} ZCPP_PortExtraData;
#define ZCPP_PORTEXTRADATA_HEADER_SIZE (sizeof(ZCPP_PortExtraData) - 1)

// Extra port configuration data - 10 - 1458 bytes
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	ZCPP_Header Header;
	uint16_t sequenceNumber;		// sequence number unique each time the configuration changes
	uint8_t flags;					// Configuration flags
    uint8_t priority;               // priority of this source
    uint8_t filler1;
    uint8_t ports;					// Number of ports being configured
    ZCPP_PortExtraData PortExtraData[1];
} ZCPP_ExtraData;
#define ZCPP_EXTRADATA_HEADER_SIZE (sizeof(ZCPP_ExtraData) - sizeof(ZCPP_PortExtraData))
#define ZCPP_EXTRADATA_HEADER_MUTABLE_SIZE 9

// Sync - 7 bytes
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	ZCPP_Header Header;
	uint8_t sequenceNumber;			// sequence number matching the data frame sequence number this sync packet is for
} ZCPP_Sync;

// Data - 15 - 1458 bytes
typedef struct 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	ZCPP_Header Header;
	uint8_t sequenceNumber;			// sequence number matching of the data frame. If multiple packets are needed all packets in the
									// same frame will have the same sequence number. Frame numbers start at zero and increment and
									// then go back to zero
	uint8_t flags;					// data packet flags
    uint32_t frameAddress;			// where in the zero based data address space the data in this packet belongs
    uint16_t packetDataLength;		// how many data bytes are in this packet
    uint8_t priority;               // priority of this source
    uint8_t data[1];
} ZCPP_Data;
#define ZCPP_DATA_HEADER_SIZE (sizeof(ZCPP_Data) - 1)

typedef union 
#ifndef _MSC_VER
__attribute__((packed))
#endif
{
	// sent discovery
	ZCPP_Discovery Discovery;
		// Must lead to a DiscoveryResponse

	// discovery response
	ZCPP_DiscoveryResponse DiscoveryResponse;

	// configuration
	ZCPP_Configuration Configuration;
		// May optionally lead to a QueryConfigurationResponse

	// extra data
	ZCPP_ExtraData ExtraData;
		// May optionally lead to a QueryConfigurationResponse

	// query configuration
	ZCPP_QueryConfiguration QueryConfiguration;
		// Must lead to a QueryConfigurationResponse

	// query configuration data response
	ZCPP_QueryConfigurationResponse QueryConfigurationResponse;

	// data
	ZCPP_Data Data;

	// sync
	ZCPP_Sync Sync;

    uint8_t raw[1458];
} ZCPP_packet_t;

#ifdef _MSC_VER
#pragma pack(pop)
#endif

inline uint32_t ZCPP_FromWire32(uint32_t value)
{
    uint8_t* p = (uint8_t*)&value;
    return (((uint32_t)(*p)) << 24) + (((uint32_t)*(p + 1)) << 16) + (((uint32_t)*(p + 2)) << 8) + ((uint32_t)*(p + 3));
}

inline uint16_t ZCPP_FromWire16(uint16_t value)
{
    uint8_t* p = (uint8_t*)&value;
    return (((uint16_t)(*p)) << 8) + (uint16_t)(*(p + 1));
}

inline uint32_t ZCPP_ToWire32(uint32_t value)
{
    uint8_t res[4];
    res[0] = value >> 24;
    res[1] = (value & 0xFF0000) >> 16;
    res[2] = (value & 0xFF00) >> 8;
    res[3] = value & 0xFF;
    return *(uint32_t*)res;
}

inline uint16_t ZCPP_ToWire16(uint32_t value)
{
    uint8_t res[2];
    res[0] = (value & 0xFF00) >> 8;
    res[1] = value & 0xFF;
    return *(uint16_t*)res;
}

inline uint16_t ZCPP_GetPacketActualSize(const ZCPP_packet_t& packet)
{
    switch (packet.Discovery.Header.type)
    {
    case ZCPP_TYPE_DISCOVERY:
        return sizeof(ZCPP_Discovery);
    case ZCPP_TYPE_DISCOVERY_RESPONSE:
        return sizeof(ZCPP_DiscoveryResponse);
    case ZCPP_TYPE_CONFIG:
        return ZCPP_CONFIGURATION_HEADER_SIZE + packet.Configuration.ports * sizeof(ZCPP_PortConfig);
    case ZCPP_TYPE_EXTRA_DATA:
    {
        int size = ZCPP_EXTRADATA_HEADER_SIZE;
        int i;
        for (i = 0; i < packet.ExtraData.ports; i++)
        {
            ZCPP_PortExtraData* p = (ZCPP_PortExtraData*)& packet.raw[size];
            size += ZCPP_PORTEXTRADATA_HEADER_SIZE + p->descriptionLength;
        }
        return size;
    }
    case ZCPP_TYPE_QUERY_CONFIG:
        return sizeof(ZCPP_QueryConfiguration);
    case ZCPP_TYPE_QUERY_CONFIG_RESPONSE:
        return ZCPP_QUERYCONFIGURATIONRESPONSE_HEADER_SIZE + packet.QueryConfigurationResponse.ports * sizeof(ZCPP_PortConfig);
    case ZCPP_TYPE_DATA:
        return ZCPP_DATA_HEADER_SIZE + ZCPP_FromWire16(packet.Data.packetDataLength);
    case ZCPP_TYPE_SYNC:
        return sizeof(ZCPP_Sync);
    }

    return 0;
}

#endif