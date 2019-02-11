#ifndef CONTROL_H
#define CONTROL_H

#include <cstdint>

#define FPP_CTRL_PORT 32320
#define FPP_CTRL_CSV_PORT 32321

#define CTRL_PKT_CMD   0
#define CTRL_PKT_SYNC  1
#define CTRL_PKT_EVENT 2
#define CTRL_PKT_BLANK 3

// Force the structures to not be aligned on boundaries
#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#elif defined(__WXWINDOWS__)
#pragma pack(push)
#pragma pack(1)
#define PACK( __Declaration__ ) __Declaration__
#else
#define PACK( __Declaration__ ) __attribute__((packed)) __Declaration__
#endif

PACK( typedef struct { char fppd[4]; uint8_t pktType; uint16_t extraDataLen; } ControlPkt);

#define CTRL_PKT_SYNC   1
#define CTRL_PKT_EVENT  2

#define SYNC_PKT_START 0
#define SYNC_PKT_STOP  1
#define SYNC_PKT_SYNC  2

#define SYNC_FILE_SEQ   0
#define SYNC_FILE_MEDIA 1

PACK( typedef struct { uint8_t  pktType; uint8_t  fileType; uint32_t frameNumber; float secondsElapsed; char filename[1]; } SyncPkt);

#ifdef _MSC_VER
#elif defined(__WXWINDOWS__)
#pragma pack(pop)
#endif
#endif /* _CONTROL_H */
