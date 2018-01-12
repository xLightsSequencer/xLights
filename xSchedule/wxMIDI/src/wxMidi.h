//=====================================================================================
// wxMidi: A MIDI interface based on PortMidi, the Portable Real-Time MIDI Library
// --------------------------------------------------------------------------------
//
// Author:      Cecilio Salmeron <s.cecilio@gmail.com>
// Copyright:   (c) 2005-2015 Cecilio Salmeron
// Licence:     wxWidgets license, version 3.1 or later at your choice.
//=====================================================================================
/// \file wxMidi.h
/// \brief The only header file for wxMidi

#ifdef __GNUG__
#pragma interface "wxMidi.cpp"
#endif

#ifndef __WXMIDI_H__		//to avoid nested includes
#define __WXMIDI_H__

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

/*
	To do, when a new version of portmidi:
	1. Review wxMidiError enumeration (copied and adapted from portmidi.h)
	2. Review filter codes taken from portmidi.h
*/

// MIDI support throgh Portmidi lib
#include "portmidi.h"
#include "porttime.h"

//other
#include <time.h>


/// Constants related to wxMidi version
#define wxMIDI_VERSION		_T("2.0")
#define wxMIDI_MAJOR		2
#define wxMIDI_MINOR		0

/// Rename some data types inherited from portmidi
#define wxMidiDeviceID		PmDeviceID
#define wxMidiTimestamp		PmTimestamp
#define wxMidiPmMessage		PmMessage
#define wxMidiPmEvent		PmEvent


/// Error codes from portmidi and additional errors from wxMidi
/// \anchor error_codes
typedef enum
{
    //AWARE: When adding more error codes, remember to update method
    //       wxMidiSystem::GetErrorText()

	wxMIDI_NO_ERROR = 0,			/** No error. The requested operation succedeed.*/

	//error codes from portmidi. Name changed from pmXxxx to wxMIDI_ERROR_Xxxx
    wxMIDI_ERROR_HostError = -10000,	/** Portmidi reported a host error. You have to
										 * call \ref wxMidiSystem::GetHostErrorText
                                         * to get more details about the error.
                                         */

    wxMIDI_ERROR_InvalidDeviceId,  /** Invalid device ID. The ID is out of range (less
								    * than zero or greater or equal than the value returned
                                    * by \ref wxMidiSystem::CountDevices. Or it is an input
							        * device and it is being used to construct a
								    * wxMidiOutDevice, or viceversa. Or the
                                    * device is already opened
                                    */

    wxMIDI_ERROR_InsufficientMemory,	/** Insufficient memory.*/
    wxMIDI_ERROR_BufferTooSmall,		/** PortMidi reports a 'Buffer too small' error.
										 * You should close the device and reopen it
										 * specifying a greater buffersize value.
										 */

    wxMIDI_ERROR_BufferOverflow,  	   /** PortMidi reported a 'Buffer overflow'.
										 * You should close the device and reopen it
										 * specifying a greater buffersize value.
										 */

    wxMIDI_ERROR_BadPtr,         /** PortMidi reports a `Bad pointer error'. This should
							      * not happen as wxMidi deals with PmDevices
							      * inside objects wxMidiDevice, and so, user has no need
							      * to deal with pointers to PmDevices.
							      */
    wxMIDI_ERROR_BadData,        /** PortMidi reported an 'Invalid MIDI message Data' such
								  * as illegal MIDI data or a missing EOX. */
    wxMIDI_ERROR_InternalError,	 /** Portmidi reported an 'Internal Error'. Sorry,
								  * no more information. */
    wxMIDI_ERROR_BufferMaxSize,  /** Portmidi informs that 'buffer is already as large
                                  * as it can be'. */

	//Additional error codes not in portmidi
	wxMIDI_ERROR_AlreadyListening = -10100,	/** Informative error. A second call to method
											 * \ref wxMidiInDevice::StartListening without
											 * previously calling method
										     * \ref wxMidiInDevice::StopListening.
											 * The lisening thread is left active.
										     */
	wxMIDI_ERROR_CreateThread,				/** An error ocurred during a call to
											 * \ref wxMidiInDevice::StartListening.
											 * The error ocurred while trying to create
									         * the wxThread object.
											 */
	wxMIDI_ERROR_StartThread,				/** An error ocurred during a call to
											 * \ref wxMidiInDevice::StartListening.
											 * The error ocurred after successful creation
											 * of the wxThread object, while starting the
											 * thread execution.
											 */
	wxMIDI_ERROR_BadSysExMsg_Start,			/** The wxByte string received as parameter
											 * to create a wxMidiSysExMessage object does
											 * not start with 0xF0 (start-of-sysex status
											 * byte).
											 */
	//wxMIDI_ERROR_BadSysExMsg_Length,      //2.0. No longer used. Removed unnecessary limit.
	wxMIDI_ERROR_NoDataAvailable,			/** You called method \ref wxMidiInDevice::Read
											 * but there is no message waiting to be
											 * delivered. You have to call Read method
											 * only after an wxEVT_MIDI_INPUT event takes
											 * place or after the method
											 * \ref wxMidiInDevice::Poll reports that there
											 * is a message waiting.
											 */
	wxMIDI_ERROR_TimeOut,
} wxMidiError;





//Macros
//----------------------------------------------------------------------------------

//macro Pm_Channel renamed as wxMIDI_CHANNEL
/*
    Pm_SetChannelMask() filters incoming messages based on channel.
    The mask is a 16-bit bitfield corresponding to appropriate channels
    The Pm_Channel macro can assist in calling this function.
    i.e. to set receive only input on channel 1, call with
    Pm_SetChannelMask(Pm_Channel(1));
    Multiple channels should be OR'd together, like
    Pm_SetChannelMask(Pm_Channel(10) | Pm_Channel(11))

    All channels are allowed by default
*/
/// \anchor macro_midi_channel
/// Macro to be used in method wxMidiInDevice::SetChannelMask(). This method requires
/// a mask value that is a 16-bit bitfield with bits set to signal the appropriate
/// channels. Macro  wxMIDI_CHANNEL can assist in preparing this mask. For example,
/// to set for receiving input only from channel 12, prepare the mask with
/// \code
/// wxMIDI_CHANNEL(12).
/// \endcode
/// Multiple channels should be OR'd together, like
/// \code
/// wxMIDI_CHANNEL(10) | wxMIDI_CHANNEL(11).
/// \endcode
#define wxMIDI_CHANNEL(channel) (1<<(channel))

/// Filter codes, renamed from portmidi
/// \anchor filter_codes
enum wxMidiFilter
{
    /// No filter. All MIDI messages will be delivered to user application.
    wxMIDI_NO_FILTER                    = 0x00,
    /// Filter active sensing messages (0xFE).
    wxMIDI_FILT_ACTIVE					= PM_FILT_ACTIVE,
    /// Filter system exclusive messages (0xF0).
    wxMIDI_FILT_SYSEX					= PM_FILT_SYSEX,
    /// Filter real-time clock messages (0xF8 only, does not filter clock start, etc.).
    wxMIDI_FILT_CLOCK					= PM_FILT_CLOCK,
    /// Filter play messages (start 0xFA, stop 0xFC, continue 0xFB).
    wxMIDI_FILT_PLAY					= PM_FILT_PLAY,
    /// Filter undefined 0xF9 messages (some equipment uses this as a 10ms 'tick').
    wxMIDI_FILT_TICK					= PM_FILT_TICK,
    /// Filter undefined 0xFD messages
    wxMIDI_FILT_FD						= PM_FILT_FD,
    /// Filter undefined real-time messages =   (wxMIDI_FILT_F9 | wxMIDI_FILT_FD).
    wxMIDI_FILT_UNDEFINED				= PM_FILT_UNDEFINED,
    /// Filter reset messages (0xFF).
    wxMIDI_FILT_RESET					= PM_FILT_RESET,
    /// Filter all real-time messages. Equivalent to
    /// (wxMIDI_FILT_ACTIVE | wxMIDI_FILT_SYSEX | wxMIDI_FILT_CLOCK | wxMIDI_FILT_PLAY
    ///  | wxMIDI_FILT_UNDEFINED | wxMIDI_FILT_RESET).
    wxMIDI_FILT_REALTIME				= PM_FILT_REALTIME,
    /// Filter note-on and note-off messages (0x90-0x9F and 0x80-0x8F).
    wxMIDI_FILT_NOTE					= PM_FILT_NOTE,
    /// Filter channel aftertouch (most MIDI controllers use this) (0xD0-0xDF).
    wxMIDI_FILT_CHANNEL_AFTERTOUCH		= PM_FILT_CHANNEL_AFTERTOUCH,
    /// Filter per-note aftertouch messages (Ensoniq holds a patent on generating
    /// these messages on keyboards until June 2006) (0xA0-0xAF).
    wxMIDI_FILT_POLY_AFTERTOUCH			= PM_FILT_POLY_AFTERTOUCH,
    /// Filter both channel and poly aftertouch. Equivalent to
    /// (wxMIDI_FILT_CHANNEL_AFTERTOUCH | wxMIDI_FILT_POLY_AFTERTOUCH)
    wxMIDI_FILT_AFTERTOUCH				= PM_FILT_AFTERTOUCH,
    /// Filter Program change messages (0xC0-0xCF).
    wxMIDI_FILT_PROGRAM					= PM_FILT_PROGRAM,
    /// Filter Control Change messages (0xB0-0xBF).
    wxMIDI_FILT_CONTROL					= PM_FILT_CONTROL,
    /// Filetr Pitch Bender messages (0xE0-0xEF).
    wxMIDI_FILT_PITCHBEND				= PM_FILT_PITCHBEND,
    /// Filter MIDI Time Code messages (0xF1).
    wxMIDI_FILT_MTC						= PM_FILT_MTC,
    /// Filter Song Position messages (0xF2).
    wxMIDI_FILT_SONG_POSITION			= PM_FILT_SONG_POSITION,
    /// Filter Song Select messages (0xF3).
    wxMIDI_FILT_SONG_SELECT				= PM_FILT_SONG_SELECT,
    /// Filter Tuning request messages (0xF6).
    wxMIDI_FILT_TUNE					= PM_FILT_TUNE,
    /// Filter all System Common messages (MTC, song position, song select, tune request).
    /// Equivalent to (wxMIDI_FILT_MTC | wxMIDI_FILT_SONG_POSITION | wxMIDI_FILT_SONG_SELECT
    /// | wxMIDI_FILT_TUNE).
    wxMIDI_FILT_SYSTEMCOMMON			= PM_FILT_SYSTEMCOMMON,
};


//Declare a new command event to inform that MIDI input data is available
/// \page midi_event The wxEVT_MIDI_INPUT event
///
/// Event wxEVT_MIDI_INPUT is a command event triggered when a new MIDI message is available
/// to be read. This event is only triggered if the user application has called method
/// wxMidiInDevice::StartListening().
DECLARE_EVENT_TYPE(wxEVT_MIDI_INPUT, -1)

/// Identifies the MIDI message type
enum wxMidiMsgType
{
	wxMIDI_UNDEFINED_MSG = 0,   ///< Undefined (invalid wxMidiMessage object)
	wxMIDI_SHORT_MSG,           ///< Short message (wxMidiShortMessage)
	wxMIDI_SYSEX_MSG            ///< SysEx message (wxMidiSysExMessage)
};


//forward declarations
class wxMidiThread;
class wxControlWithItems;

//-----------------------------------------------------------------------------
/// \class wxMidiMessage
/// \brief Abstract class representing a MIDI message
///
/// In wxMidi, messages are represented by a wxMidiMessage object. It is an abstract
/// class from which the specific Midi messages derive. It contains information about
/// the type and status byte of the MIDI message. In addition, it
/// provides a timestamp value that is used by devices dealing with MIDI timing,
/// such as sequencers.
/// \sa wxMidiShortMessage, wxMidiSysExMessage
class wxMidiMessage
{
public:
    /// Constructor
	wxMidiMessage() { m_type = wxMIDI_UNDEFINED_MSG; }
	/// Destructor
	virtual ~wxMidiMessage() {}

    /// Set the message timestamp value.  On output to a wxMidiOutDevice opened
    /// with non-zero latency, the timestamp will determine the time to begin
    /// sending the message. If the wxMidiOutDevice was opened with a latency
    /// value of zero, timestamps will be ignored and messages will be
    /// delivered inmediatelly.
    /// \param timestamp  The timestamp value. It is milliseconds elapsed
    ///        since the wxMidi package initialization. Current time can be
    ///        obtained by calling wxMidi::GetTime().
	virtual void SetTimestamp(wxMidiTimestamp timestamp) = 0;

	virtual wxMidiTimestamp GetTimestamp() = 0;

    /// Returns either wxMIDI_SHORT_MSG, or
    /// wxMIDI_SYSEX_MSG, identifying the type of the message object.
	wxMidiMsgType GetType() { return m_type; }

    /// Returns the status byte of the message.
    ///
    /// For a wxSysExMessage status byte is always 0xF0 but the end of sysex
    /// message (0xF7) is also included in the message data. According MIDI
    /// standard  the most significant bit of the status byte is always 1 so
    /// the range of status values goes from 0x80 to 0xFF.
    ///
    /// Values lower than 0xF0 identify channel messages, with the four lower
    /// bits specifying the channel (0-15); for example, status byte 0x93
    /// is a NoteOn for channel 3:
    ///
    /// |Message                |Status byte|
    /// |:----------------------|:---------:|
    /// |Note-Off Event         |   0x8n    |
    /// |Note-On  Event         |   0x9n    |
    /// |Polyphonic Key Pressure|   0xAn    |
    /// |Control Change         |   0xBn    |
    /// |Program Change         |   0xCn    |
    /// |Channel Pressure       |   0xDn    |
    /// |Pitch Bend             |   0xEn    |
    ///
    /// Values 0xF0 to 0xFF are for system messages and are no intended for
    /// a specific channel:
    ///
    /// |Message                |Status byte|
    /// |:----------------------|:---------:|
    /// |Begin System Exclusive |   0xF0    |
    /// |MIDI  Time Code        |   0xF1    |
    /// |Song Position Pointer  |   0xF2    |
    /// |Song Select            |   0xF3    |
    /// |Tune Request           |   0xF6    |
    /// |End System Exclusive   |   0xF7    |
    /// |Real-time Clock        |   0xF8    |
    /// |Undefined              |   0xF9    |
    /// |Start                  |   0xFA    |
    /// |Continue               |   0xFB    |
    /// |Stop                   |   0xFC    |
    /// |Undefined              |   0xFD    |
    /// |Active Sensing         |   0xFE    |
    /// |System Reset           |   0xFF    |
    ///
	virtual wxByte GetStatus() = 0;


protected:
	wxMidiMsgType	m_type;

};

//-----------------------------------------------------------------------------
/// wxMidiShortMessage represents a MIDI short message.
/// They are the most common and have a status byte and at most two data bytes.
/// \sa wxMidiMessage, wxMidiSysExMessage
class wxMidiShortMessage : public wxMidiMessage
{
public:
    /// Constructor. To build a message from its components:
    /// a status byte and at most two data bytes.
	wxMidiShortMessage(wxByte status, wxByte data1, wxByte data2)
		: wxMidiMessage()
		{
			m_type = wxMIDI_SHORT_MSG;
			m_buffer.message = (((data2) << 16) & 0xFF0000) |
						(((data1) << 8) & 0xFF00) |
						((status) & 0xFF);
			m_buffer.timestamp = 0;
		}
	~wxMidiShortMessage() {}

	//timestamp
	void SetTimestamp(wxMidiTimestamp timestamp) { m_buffer.timestamp = timestamp; }
	wxMidiTimestamp GetTimestamp() { return m_buffer.timestamp; }

	// message data
	wxByte GetStatus() { return ((m_buffer.message) & 0xFF); }

	/// Returns the first data byte of the message or 0x00 if the type of
	/// message only has the status byte.
	wxByte GetData1() { return (((m_buffer.message) >> 8) & 0xFF); }

	/// Returns the second data byte of the message or 0x00 if the type of
	/// message only has one data byte.
	wxByte GetData2() { return (((m_buffer.message) >> 16) & 0xFF); }

	//internal usage
	PmEvent* GetBuffer() { return &m_buffer; }

private:
	PmEvent m_buffer;
};

//-----------------------------------------------------------------------------
/** Represents a system-exclusive MIDI message.

    As the MIDI specification allows that sysex messages be interrupted by
    real-time messages, the wxMidi package takes care of this and ensures
    that any real-time message embedded into a sysex message will be delivered
    first. Also it ensures that when a sysex message is delivered it is
    complete and does not contain real-time messages embedded into it.

    The sysex message encapsulated in a wxMidiSysExMessage will be normally
    ended by an EOX status byte (0xF7), but this can not be guaranteed. If
    the last byte of a received wxMidiSysExMessage is not an EOX it means
    the sysex message was somehow truncated. This is not considered an error,
    as a missing EOX can result from the user disconnecting a MIDI cable
    during sysex transmission.

    The timestamp of a wxMidiSysExMessage is the time at which the status
    byte (the first byte of the message) arrived.

    \sa wxMidiMessage, wxMidiShortMessage
*/
class wxMidiSysExMessage : public wxMidiMessage
{
public:
	wxMidiSysExMessage(wxByte* msg, wxMidiTimestamp timestamp=0);
	wxMidiSysExMessage();
	~wxMidiSysExMessage();

	//timestamp
	void SetTimestamp(wxMidiTimestamp timestamp) { m_timestamp = timestamp; }
	wxMidiTimestamp GetTimestamp() { return m_timestamp; }

	// message data
	wxByte GetStatus() { return *m_pMessage; }

	/** Returns a wxByte string containing the raw MIDI message.
        It includes the start-of-sysex status byte (0xF0) and the end-of-sysex
        status byte (0xF7) - unless it is a truncated sysex message - .
        See wxMidiSysExMessage.

        The returned string is owned by the wxMidiSysExMessage and must not
        be deleted.
    */
	wxByte* GetMessage() { return m_pMessage; }

	// information
	/** Returns the error code for the wxMidiSysExMessage constructor.
        When building the wxMidiSysExMessage in one step, copying data from
        a wxByte string, some checking is done, for example, to verify that
        the message starts with a start-of-sysex status byte (0xF0) and that
        the buffer ends with an end-of-sysex status byte (0xF7). If any error
        is detected, the error is recorded and is returned when this method
        is called.

        If no error, zero (wxMIDI_NO_ERROR) is returned.

        \sa \ref error_codes, wxMidiSysExMessage()
    */
	wxMidiError Error() { return m_nError; }

	/** Returns the length (number of bytes) of the message returned by
        GetMessage(). It is, the length, in bytes, of the raw MIDI message,
        not the length of the buffer containing it.
    */
	long Length() { return m_nSize; }

	//two steps construction
	/** This method is mainly intended for internal use of wxMidi. It stores
        the pointer received as parameter, so that internal buffer will point
        to the received string.

        The wxMidiSysExMessage object will become the owner of the string and
        will be deleted in the destructor. User must not delete the passed
        buffer.

        The length of the passed string must be set by calling
        SetLength()
    */
	void SetBuffer(wxByte* pBuffer) { m_pMessage = pBuffer; }

	/** This method is mainly intended for internal use of wxMidi. It stores
        the length of the wxByte string received in a previous call to
        SetBuffer() (or to be received in a subsequent call to that method).

        The value passed as parameter must be the real size of the raw MIDI
        message not the size of the buffer containing it, that could be greater.
    */
	void SetLength(long lenght) { m_nSize = lenght; }


private:
	wxByte*			m_pMessage;
	wxMidiTimestamp	m_timestamp;
	wxMidiError		m_nError;
	long   		    m_nSize;
};

//-----------------------------------------------------------------------------
/// Abstract class representing a MIDI device.
///
/// The base interface for devices is wxMidiDevice. It is an abstract class
/// with two derived classes: wxMidiInDevice and wxMidiOutDevice.
///
/// A wxMidiDevice object represents a MIDI stream on which MIDI data traffic
/// can be read or sent. All MIDI communication is achieved through a
/// wxMidiDevice by using MIDI messages. Devices can be opened and closed, and
/// they provide descriptions of themselves.
///
/// MIDI messages are represented by the wxMidiMessage abstract class and
/// are read and sent from/to the MIDI stream by using methods Read() and
/// Write(), respectively.
///
/// For the most common output operations, such as note on or note off, there
/// are specific methods implemented so that wxMidi user has not to build
/// and send the appropriate messages. See:
/// + wxMidiOutDeviceNoteOn::NoteOn()
/// + wxMidiOutDeviceNoteOff::NoteOff()
/// + wxMidiOutDeviceProgramChange::ProgramChange()
/// + wxMidiOutDeviceAllSoundsOff::AllSoundsOff()
///
/// \sa wxMidiInDevice, wxMidiOutDevice, wxMidiMessage, wxMidiSystem,
///     \ref main_quick_start
class wxMidiDevice
{
public:

    /// Constructor.
    /// \param nDevice  The number of the device.  Device numbers (also referred
    ///                 as device IDs) range from 0 to
    ///                 wxMidiSystem::CountDevices() - 1
    /// \sa wxMidiSystem::CountDevices()
	wxMidiDevice(wxMidiDeviceID nDevice);

	///Destructor
	virtual ~wxMidiDevice();

    /// Closes a MIDI stream, flushing any pending messages.
    /// Any wxMidiDevice that is successfully opened should eventually be
    /// closed calling Close().
    /// \sa \ref error_codes
	wxMidiError Close() { return (wxMidiError)Pm_Close(m_stream); }


	// Device information

	/// Returns a string containing the name of the MIDI device. For
    /// example "MPU-401 Compatible card".
	const wxString DeviceName();

	/// Returns a string containing the name of the API used internally by portmidi
	/// to implement MIDI communication.
	/// Examples are "MMSystem", "DirectX", "ALSA", "OSS", etc.
	const wxString InterfaceUsed();

	/// Returns true if the device has input capabilities and then can be
	/// used as wxMidiInDevice.
	bool IsInputPort();

	/// Returns true if the device has output capabilities and then can be
	/// used as wxMidiOutDevice.
	bool IsOutputPort();

	// Errors

    /// Check if the wxMidiDevice has a pending host error to be reported.
    /// Normally, the application finds out about wxMidi errors through
    /// returned error codes, but some errors can occur asynchronously where the
    /// application does not explicitly call a function, and therefore cannot receive
    /// an error code.
    ///
    /// The client can test for a pending error using method HasHostError(). If true,
    /// the error can be accessed and cleared by calling wxMidiSystem::GetErrorText().
    /// It is nor necessary to call HasHostError() as any pending error will be
    /// reported the next time the application  performs an explicit function call
    /// on the wxMidiDevice, i.e. an input or output operation.
	int HasHostError() { return Pm_HasHostError(m_stream); }



protected:

	wxMidiDeviceID			m_nDevice;
	const PmDeviceInfo*		m_pInfo;
	PortMidiStream*			m_stream;

};

//-----------------------------------------------------------------------------
/// Represents a MIDI stream on which MIDI data traffic can be sent.
class wxMidiOutDevice : public wxMidiDevice
{
public:

    /// Constructor.
    /// \param nDevice  The number of the device.  Device numbers (also referred
    ///        as device IDs) range from 0 to wxMidiSystem::CountDevices() - 1
    /// \sa wxMidiSystem::CountDevices()
	wxMidiOutDevice(wxMidiDeviceID nDevice) : wxMidiDevice(nDevice) {}

    /// Destructor
	~wxMidiOutDevice() {}

    // Open the device

	/// This methods opens a wxMidiOutDevice stream so that it can be used.
	///
	///  \param latency  It is the delay in milliseconds applied to timestamps to
	/// determine when the output should actually occur (If latency is less than zero,
	/// 0 is assumed). If latency is zero, timestamps are ignored and all output is
	/// delivered immediately. If latency is greater than zero, output is delayed until
	/// the message timestamp plus the latency. Timestamps are absolute, not relative
	/// delays or offsets. In some cases, by using the latency parameter you can obtain
	/// better timing than your application by passing timestamps along to the device
	/// driver or hardware. Latency may also help you to synchronize midi data to audio
	/// data by matching midi latency to the audio buffer latency.
	///
	/// \param pDriverInfo  It is a pointer to an optional driver specific data structure
	/// containing additional information for device setup or handle processing.
	/// pDriverInfo is never required for correct operation. If not used
	/// pDriverInfo should be NULL.
	///
	/// \return  Upon success Open() returns zero (wxMIDI_NO_ERROR). If a call to
	/// Open() fails a nonzero error code is returned.
	/// \sa \ref error_codes
	wxMidiError Open(long latency, void* pDriverInfo=NULL);


	// Write operations

	/// Sends a MIDI message to the MIDI device represented by this wxMidiOutDevice object.
	/// \sa \ref error_codes
	wxMidiError Write(wxMidiShortMessage* pMsg);

	/// Sends a MIDI message to the MIDI device represented by this wxMidiOutDevice object.
	/// \sa \ref error_codes
	wxMidiError Write(wxMidiSysExMessage* pMsg);

	/// Sends a MIDI message to the MIDI device represented by this wxMidiOutDevice object.
	///
	/// \param msg  String of bytes with the message to send.
	///
	/// \param when  Timestamp with the absolute time at which message must be sent.
	/// This parameter is only meaningful if the device was opened with latency parameter
	/// different from zero. Otherwise messages are sent immediately and parameter
	/// Timestamp will be ignored.
    /// \sa \ref error_codes
    wxMidiError Write(wxByte* msg, wxMidiTimestamp when=0);

	/// This method is just a wrapper for the portmidi native Write function. This method
	/// is provided in case it is needed but, probably, it is not needed.
	/// \sa \ref error_codes
	wxMidiError Write(PmEvent *buffer, long length) {
		return (wxMidiError)Pm_Write(m_stream, buffer, length);
	}


	// Very common channel voice commands

	/// Prepares and sends a NoteOn MIDI message.
	///
	/// \param channel   The number (0 to 15) of the logical channel that will be used
	/// for sending the message. See \ref macro_midi_channel
	///
	/// \param note  A number in the range 0 to 127 that specifies the note's pitch.
	/// Middle C note corresponds to MIDI note 60, middle C # is 61, middle D is 62,
	/// and so on.
	///
	/// \param velocity  Velocity refers to how hard the key, on a MIDI keyboard, was
	/// pressed. This parameter controls the volume of the sound to be produced. So you
	/// can consider velocity as synonymous to volume. Parameter \b velocity is
	/// a value from 0 to 127. A logarithmic scale is recommended, for example:
	///
	/// |Dynamics|velocity|
	/// |--------|--------|
	/// |OFF     | 0      |
	/// |ppp     | 1      |
	/// |pp      | 2      |
	/// |p       | 4      |
	/// |mp      | 8      |
	/// |mf      | 16     |
	/// |f       | 32     |
	/// |ff      | 64     |
	/// |fff     | 127    |
	///
	/// \b Remarks:
	/// You must be aware that in MIDI systems, the activation of a particular note and
	/// the release of the same note are considered as two separate events. So, after
	/// calling method NoteOn() it is necessary to call later to method NoteOff() to stop
	/// the sound generator, even if the sound produced by the NoteOn() command is no
	/// longer audible due to natural sound extinction because of the chosen
	/// instrument (i.e.piano or percussion instruments).
	/// \sa \ref error_codes, \ref macro_midi_channel
    wxMidiError	NoteOn(int channel, int note, int velocity);

	/// Prepares and sends a NoteOff MIDI message. It is normally used to
	/// stop a sound started by a previous call to NoteOn().
	///
	/// \param channel  The number (0 to 15) of the logical channel that will be used
	/// for sending the message.
	/// \param note  A number in the range 0 to 127 that specifies the note's pitch to stop.
	/// \param velocity  A number in the range 0 to 127 that specifies the velocity with
	/// which the key was released. This information is  usually ignored by MIDI synthesizers.
	/// \sa \ref error_codes, \ref macro_midi_channel
	wxMidiError	NoteOff(int channel, int note, int velocity);

	/// Prepares and sends a ProgramChange MIDI message. This message is used to change
	/// the sound (instrument) assigned to a channel.
	///
	/// \param channel  The number (0 to 15) of the logical channel whose sound is
	/// to be re-programmed
	///
	///  \param instrument  A number in the range 0 to 127 that specifies the new
	/// instrument sound to assign to the channel.
	///
	/// \sa wxMidiDatabaseGM, \ref macro_midi_channel
	wxMidiError	ProgramChange(int channel, int instrument);

	/// From time to time, specially during user application debugging, you may encounter
	/// a situation in which a MIDI synthesizer sounds as if its keys are "stuck," and
	/// it plays continuously as though someone's still pressing them. Method
	/// AllSoundsOff() sends the MIDI device a signal that tells it to stop immediately
	/// all sounds.
	/// \sa \ref error_codes
	wxMidiError AllSoundsOff();


	// miscellaneous

	/// Terminates any outgoing message immediately; this call may result in a partial
	/// transmission of a MIDI message. The caller should immediately close the output port.
	/// \sa \ref error_codes
	wxMidiError Abort() { return (wxMidiError)Pm_Abort(m_stream); }


private:

};


//-----------------------------------------------------------------------------
/// Represents a MIDI stream on which MIDI data traffic can be read.
class wxMidiInDevice : public wxMidiDevice
{
public:

    /// Constructor.
    /// \param nDevice  The number of the device.  Device numbers (also referred
    ///        as device IDs) range from 0 to wxMidiSystem::CountDevices() - 1
    /// \param timeoutSeconds  Specifies the maximum wait time without
    /// receiving more data for aborting an incomplete SysEx message
    /// reception. This will happen if a SysEx message reception is
    /// interrupted (i.e. the MIDI cable is accidentally unplugged).
    /// \sa wxMidiSystem::CountDevices()
	wxMidiInDevice(wxMidiDeviceID nDevice, double timeoutSeconds=5.0);

    /// Destructor
	~wxMidiInDevice();

	/// Open the device, so that the device can be used. Upon success Open() returns
	/// zero (wxMIDI_NO_ERROR). If the open process fails a non-zero error code
	/// is returned.
	///
	/// Any successfully opened device should eventually be closed calling Close().
	///
	/// \param pDriverInfo  It is a pointer to an optional driver specific data
	/// structure containing additional information for device setup or handle processing.
	/// pDriverInfo is never required for correct operation. If not used
	/// pDriverInfo should be NULL.
	///
	/// \param buffersize The portmidi library doesn't allow for dynamic
	/// buffer allocation. Therefore, you should allocate enough space for the
	/// largest SysEx message you expect to receive. As each slot of a portmidi
	/// buffer holds 4 SysEx bytes, \p buffersize parameter must be something like
	/// the maximum SysEx message length in bytes divided by 4.
	/// The default value allows to receive at least 16KB long SysEx
	/// messages.
	///
	/// portmidi uses an 'int' for specifying buffer size. This introduces
	/// an apparent limitation to approx. 128KB SysEx messages (32KB x 4).
	/// But as MIDI data transfer rate is 3.8 KB/sec, in principle wxMidi
	/// will read arriving bytes much faster. So, in practice, the default
	/// value for the portmidi buffersize should not cause any limitation
	/// for receiving SysEx messages of any length.
	/// \sa \ref error_codes
	wxMidiError Open(void *pDriverInfo = NULL, int buffersize=4096);

    // Read operations

	/// Retrieves a received MIDI message and modifies value of variable pointed by
	/// pError to leave an error code. On success, a pointer to a MIDI message
	/// (either wxMIDIShortMessage or wxMidiSysExMessage) is returned and value
	/// zero (wxMIDI_NO_ERROR) is left in *pError.
	/// If any error occurs, a NULL pointer is returned and the error code is left in *pError.
	/// \param pError  Pointer to an wxMidiError variable in which to leave the error
	/// code for the Read operation.
	wxMidiMessage* Read(wxMidiError* pError);

	/// This second Read operation is just a wrapper for the portmidi native Read
	/// function. This method is provided in case it is needed but it should not be
	/// required.
	/// \sa \ref error_codes
	wxMidiError Read(wxMidiPmEvent *buffer, long* length);

	/// Sets filters on an open wxMidiInDevice to drop selected input types.
	/// By default, active sensing messages are filtered. To allow all messages you
	/// have to call SetFilter(wxMIDI_NO_FILTER).
	///
	/// Filtering is useful when midi routing or midi thru functionality is being
	/// provided by the user application. For example, you may want to exclude timing
	/// messages (clock, MTC, start/stop/continue), while allowing note-related messages
	/// to pass. Or you may be using a sequencer or drum-machine for MIDI clock
	/// information but want to exclude any notes it may play.
	///
	/// \param filters  Filter codes combined with the OR operator (|).
	/// See \ref filter_codes
	///
	/// Example:
	/// \code
	/// // filter out active sensing and sysex messages
	/// pMidiIn->SetFilter(wxMIDI_FILT_ACTIVE | wxMIDI_FILT_SYSEX);
	/// \endcode
	wxMidiError SetFilter(long filters )
					{ return (wxMidiError)Pm_SetFilter(m_stream, filters); }

	/// SetChannelMask() filters incoming messages based on channel. The mask is
	/// a 16-bit bitfield corresponding to appropriate channels. By default, messages for
	/// all channels are allowed. Macro wxMIDI_CHANNEL can assist in preparing this mask.
	/// \sa \ref error_codes \ref macro_midi_channel
	wxMidiError SetChannelMask(long mask )
					{ return (wxMidiError)Pm_SetChannelMask(m_stream, mask); }

    /// Check if there is a received MIDI message waiting to be Read().  It returns
	/// TRUE if there are messages waiting, FALSE if not, or an error value if an
	/// error occurred.
	/// \sa \ref error_codes
	wxMidiError Poll() { return (wxMidiError)Pm_Poll(m_stream); }

	/// Discard any MIDI messages waiting to be read.
	void Flush();

    // Polling

	/// This method creates and starts a thread to call Poll() method every \b nPollingRate
	/// milliseconds, and trigger a wxEVT_MIDI_INPUT command event if a MIDI message
	/// has been received and is waiting to be Read().
	///
	/// \param pWindow  Pointer to window that will receive the wxEVT_MIDI_INPUT command events.
	///
	/// \param nPollingRate  Polling interval, in milliseconds.
	///
	/// \b Remarks:
	/// This method is useful for dealing with input messages by means of an event
	/// handler. See \ref main_receive for an example.
	/// \sa \ref error_codes
	wxMidiError StartListening(wxWindow* pWindow, unsigned long nPollingRate=50);

	/// Stops a listener thread created in a previous call to
	/// wxMidiInDevice::StartListening().
	/// \sa \ref error_codes
	wxMidiError StopListening();

private:
	bool MoveDataToSysExBuffer(PmMessage message);
    bool too_much_time_without_receiving_bytes();
    bool too_much_reads_without_receiving_bytes();
    bool should_report_timeout();
    //helpers
    inline void reset_timeout_counters() { m_timeCounter=time_t(0); m_numNullReads=0; }
    inline bool use_time_algorithm() { return m_fUseTimeAlgorithm; }
    inline void switch_to_alternate_algorithm() { m_fUseTimeAlgorithm = false; }


	wxMidiThread*	m_pThread;		//thread for polling

	//for reporting timeout errors
	bool    m_fUseTimeAlgorithm;
	time_t  m_timeCounter;      //time when first read failure observed
	double  m_timeoutSeconds;   //max time without receiving data before reporting timeout
	int     m_numNullReads;     //number of reads without receiving data

	//buffer for sysex messages
	long	m_SizeOfSysexBuffer;
	wxByte*	m_SysexBuffer;
	wxByte*	m_CurSysexDataPtr;
	bool	m_fReadingSysex;		// sysex message interrupted by real-time message
	wxMidiTimestamp	m_timestamp;	// timestamp of the interrupted sysex message
	bool	m_fEventPending;		// sysex message ended without EOX. There is a PmEvent
									// .. read but not delivered
	PmEvent	m_event;				// event pending to be processed
};


//-----------------------------------------------------------------------------
/// A database for Midi GM (General MIDI Standard) instruments.
/// See \ref midi_database for details
class wxMidiDatabaseGM
{
public:
	~wxMidiDatabaseGM();

	/// Returns a pointer to the only wxMidiDatabaseGM instance. Note that
	/// wxMidiDatabaseGM is a singleton and, therefore, the constructor is not public.
	/// Access to the only instance must be through this GetInstance() method.
	static wxMidiDatabaseGM* GetInstance();

    // Populate controls with database items

	/// Load control \c pCtrol  with the list of all instruments that belong to section
	/// \c nSection . If \c nInstr  is specified, the name of that instrument is left
	/// selected in the control. Otherwise, the first instrument of the section is
	/// left selected.
	///
	/// \param pCtrol  Pointer to wxControlWithItems object to be initialized and populated.
	/// \param nSection  The number of the section whose instruments' names are going
	/// 	   to be loaded. Must be a number in range 0 to
	/// 	   wxMidiDatabaseGM::GetNumSections() - 1.
	/// \param nInstr  Number (0 to GetNumInstrumentsInSection() - 1 ) of the
	/// 	   instrument to select.
	/// \param fAddNumber When TRUE, instrument number is included in the strings loaded in
	///        in the control, i.e. "37 - Side Stick". Otherwise, if FALSE, only the
	///        instrument name is generated, i.e. "Side Stick".
	///
	/// \b Remarks:
	/// This method is useful when in your application, the instruments are presented
	/// organized into sections instead of in a single long list with all GM instruments.
	/// For example, you have to load a combo box with the list of all sections and
	/// other combo box with the list of instruments in currently selected section,
	/// as in following example:
	///
	/// \code
	/// MyFrame::MyFrame (...)
	/// {
	///    ...
	///
	///    wxMidiDatabaseGM* pMidiGM = wxMidiDatabaseGM::GetInstance();
	///    m_pSectCombo =
	///       new wxComboBox(this, ID_COMBO_SECTIONS,
	/// 			_T("This"),
	/// 			wxPoint(20,25), wxSize(270, wxDefaultCoord),
	/// 			0, NULL,
	/// 			wxCB_DROPDOWN | wxCB_READONLY | wxPROCESS_ENTER);
	///
	///    pMidiGM->PopulateWithSections(m_pSectCombo);
	///
	///    m_pInstrCombo =
	///       new wxComboBox(this, ID_COMBO_INSTRUMENTS,
	/// 			_T("This"),
	/// 			wxPoint(20,25), wxSize(270, wxDefaultCoord),
	/// 			0, NULL,
	/// 			wxCB_DROPDOWN | wxCB_READONLY | wxPROCESS_ENTER);
	///
	///    pMidiGM->PopulateWithInstruments(m_pInstrCombo, 0, 0);
	///
	///    ...
	/// }
	///
	/// void MyFrame::OnComboSections(wxCommandEvent &event)
	/// {
	///    // A new section selected. Reload Instruments combo
	///    // with the instruments in the selected section
	///
	///    wxMidiDatabaseGM* pMidiGM = wxMidiDatabaseGM::GetInstance();
	///    int nSect = m_pSectCombo->GetSelection();
	///    pMidiGM->PopulateWithInstruments(m_pInstrCombo, nSect);
	///    DoProgramChange();
	///
	/// }
	///
	/// void MyFrame::OnComboInstruments(wxCommandEvent &event)
	/// {
	/// 	// A new instrument selected. Change Midi program
	/// 	DoProgramChange();
	/// }
	///
	/// void MyFrame::DoProgramChange()
	/// {
	///     // A new instrument has been selected. Change Midi program
	///
	///     int nInstr = m_pInstrCombo->GetSelection();
	///     int nSect = m_pSectCombo->GetSelection();
	///     wxMidiDatabaseGM* pMidiGM = wxMidiDatabaseGM::GetInstance();
	///     int nProgram = pMidiGM->GetInstrFromSection(nSect, nInstr);
	///     int nChannel = m_channel;
	///     wxMidiError nErr = m_pOutDev->ProgramChange(nChannel, nProgram);
	///     if (nErr) {
	///         ...
	///     }
	///     else {
	///         ...
	///     }
	///
	/// }
	/// \endcode
	///
	/// \sa PopulateWithAllInstruments, PopulateWithPercusionInstr,
	///     PopulateWithSections
	void PopulateWithInstruments(wxControlWithItems* pCtrol, int nSection, int nInstr=0,
                                     bool fAddNumber=false);

	/// In the MIDI standard channel 9 is reserved for "key-based" percussion sounds.
	/// For these "key-based" sounds, the note number data in a NoteOn message is used
	/// to select which drum sound will be played. For example, a NoteOn message on
	/// channel 9 with note number 60 will play a Hi Bongo drum sound. Note number 59
	/// on channel 9 will play the Ride Cymbal 2 sound. Method PopulateWithPercusionInstr()
	/// is used to load a control (combo box, list box, or other derived from
	/// wxControlWithItems) whit the list of all key-based percussion sounds in the GM standard.
	///
	/// \param pCtrol  Pointer to wxControlWithItems object to be initialized and populated.
	/// \param iSel  Number (35 to 81) of the sound to leave selected in the control.
	/// Default value (zero) is for leaving selected the first available sound
	/// (35-Acoustic Bass Drum) as well as any out of range value.
	///
	/// The list of percussion sounds in the GM standard is included here for your conveniece:
	///
    /// |iSel| Instrument          |iSel| Instrument     |
    /// |:--:|:--------------------|:--:|:---------------|
	/// |35  |Acoustic Bass Drum   |59  |Ride Cymbal 2   |
	/// |36  |Bass Drum 1          |60  |High Bongo      |
	/// |37  |Side Stick           |61  |Low Bongo       |
	/// |38  |Acoustic Snare       |62  |Mute High Conga |
	/// |39  |Hand clap            |63  |Open High Conga |
	/// |40  |Electric Snare       |64  |Low Conga       |
	/// |41  |Low Floor Tom        |65  |High Timbale    |
	/// |42  |Closed High-Hat      |66  |Low Timbale     |
	/// |43  |High Floor Tom       |67  |High Agogo      |
	/// |44  |Pedal High-Hat       |68  |Low Agogo       |
	/// |45  |Low Tom              |69  |Casaba          |
	/// |46  |Open High-Hat        |70  |Maracas         |
	/// |47  |Low-Mid Tom          |71  |Short Whistle   |
	/// |48  |High-Mid Tom         |72  |Long Whistle    |
	/// |49  |Crash Cymbal 1       |73  |Short Guiro     |
	/// |50  |High Tom             |74  |Long Guiro      |
	/// |51  |Ride Cymbal 1        |75  |Claves          |
	/// |52  |Chinese Cymbal       |76  |High Wood Block |
	/// |53  |Ride Bell            |77  |Low Wood Block  |
	/// |54  |Tambourine           |78  |Mute Cuica      |
	/// |55  |Splash Cymbal        |79  |Open Cuica      |
	/// |56  |Cowbell              |80  |Mute Triangle   |
	/// |57  |Crash Cymbal 2       |81  |Open Triangle   |
	/// |58  |Vibraslap            |    |                |
	///
	void PopulateWithPercusionInstr(wxControlWithItems* pCtrol, int iSel=0);
	/// Fix spelling error without changing the API, for backwards compatibility
	#define	 PopulateWithPercussionInstr  PopulateWithPercusionInstr

	/// Load control \c pCtrol  with the list of all sections in which the instruments
	/// in the GM standard have been organized in the wxMidiDatabaseGM. If \c nSelInstr
	/// is specified, the name of that section is left selected in the control.
	/// Otherwise, the first section is left selected.
	///
	/// \param pCtrol  Pointer to wxControlWithItems object to be initialized and populated.
	/// \param nSelInstr  Number (0 to GetNumSections() - 1 ) of the section to select.
	///
	/// \b Remarks:
	/// This method is useful when in your application, the instruments are presented
	/// organized into sections instead of in a single long list with all GM instruments.
	/// See an example in method PopulateWithInstruments().
	///
	/// \sa PopulateWithAllInstruments, PopulateWithPercusionInstr,
	///      PopulateWithInstruments
	int PopulateWithSections(wxControlWithItems* pCtrol, int nSelInstr=-1);

	/// Load control \c pCtrol  with the list of all instrumens in the GM standard.
	/// If \c nInstr  is specified, the name of that instrument is left selected in the
	/// control. Otherwise, the first instrument is left selected.
	///
	/// \param pCtrol  Pointer to wxControlWithItems object to be initialized and populated.
	/// \param nInstr  Number (0 to 127) of the instrument to select.
	///
	/// Example:
	/// \code
	///
	/// pInstrCombo =
	///     new wxComboBox(panel, ID_COMBO_INSTRUMENTS,
	/// 			_T("This"),
	/// 			wxPoint(20,25), wxSize(270, wxDefaultCoord),
	/// 			0, NULL,
	/// 			wxCB_DROPDOWN | wxCB_READONLY | wxPROCESS_ENTER);
	///
	/// wxMidiDatabaseGM* pMidiGM = wxMidiDatabaseGM::GetInstance();
	/// pMidiGM->PopulateWithAllInstruments(pInstrCombo);
	///
	/// \endcode
	/// \sa PopulateWithSections, PopulateWithInstruments, PopulateWithPercusionInstr
	void PopulateWithAllInstruments(wxControlWithItems* pCtrol, int nInstr=0);

    // get information about sections and instruments

	/// Returns the number of sections in which the instruments in the GM standard has
	/// been organized. In the wxMidiDatabaseGM, the number sections is always 16.
	/// This method was included to allow for future extensions and to deal
	/// with other MIDI standards.
	/// \sa GetNumInstrumentsInSection, GetSectionName, PopulateWithSections
	int GetNumSections();

	/// Returns the number of instruments that are included in section number \c nSect.
	/// In the wxMidiDatabaseGM, the number of instruments in a section is always eight.
	/// This method was included to allow for future extensions and to deal
	/// with other MIDI standards.
	///
	/// \param nSect  The number of the section for which the number of instruments
	/// in it is desired. Must be a number in the range 0 to
	/// wxMidiDatabaseGM::GetNumSections() - 1.
	///
	/// \sa GetInstrFromSection, GetNumSections, GetSectionName,
	/// 	PopulateWithInstruments
	int GetNumInstrumentsInSection(int nSect);

	/// Returns the GM number of the \e n th instrument in section \c nSect.
	/// \param nSect  The number of the section in which it is included the instrument
	/// to retrieve. Must be a number in the range 0 to  wxMidiDatabaseGM::GetNumSections() - 1.
	/// \param i  The position (0 to GetNumInstrumentsInSection() - 1 ) in section of
	/// the instrument whose GM index is desired.
	/// \b Remarks:
	/// This method is useful when in your application, the instruments are presented
	/// organized into sections instead of in a single list with all GM instruments.
	/// In the first case, when the user selects an instrument, it is necessary to
	/// translate the pair (section number, instrument number - relative to section -)
	/// to the GM standard program number, as in following example:
	///
	/// \code
	/// // It is assumed that combo box <I>m_pSectCombo</I> is loaded
	/// // with section names and that combo box <I>m_pInstrCombo</I>
	/// // is loaded with the names of the instruments
	/// // in the selected section.
	///
	/// void MyFrame::OnComboInstruments(wxCommandEvent &event)
	/// {
	///     // A new instrument has been selected. Change Midi program
	///
	///     int nInstr = m_pInstrCombo->GetSelection();
	///     int nSect = m_pSectCombo->GetSelection();
	///     wxMidiDatabaseGM* pMidiGM = wxMidiDatabaseGM::GetInstance();
	///     int nProgram = pMidiGM->GetInstrFromSection(nSect, nInstr);
	///     int nChannel = m_channel;
	///     wxMidiError nErr = m_pOutDev->ProgramChange(nChannel, nProgram);
	///     if (nErr) {
	///         ...
	///     }
	///     else {
	///         ...
	///     }
	/// }
	///
	/// \endcode
	int GetInstrFromSection(int nSect, int i);

	/// Returns a string with the name of instrument \c nInstr.
	/// \param nInstr  The number (0 to 127, according the GM standard) whose name
	/// is to be retrieved.
	///
	/// \b Remarks:
	/// To populate a control (i.e. a combo box) with the names of all instruments you
	/// can do it in a single call by using method PopulateWithAllInstruments()
	/// instead of doing it inside a loop of calls to GetInstrumentName().
	///
	/// \sa PopulateWithAllInstruments, PopulateWithInstruments,
	///     PopulateWithPercusionInstr
	wxString GetInstrumentName(int nInstr);

	/// Returns a string containig the name of section number \c nSect.
	///
	/// \param nSect  The number of the section whose name is desired. Must be a number
	/// in range 0 to  wxMidiDatabaseGM::GetNumSections() - 1.
	///
	/// \b Remarks:
	/// To populate a control (i.e. a combo box) with the names of all sections you can
	/// do it in a single call by using method PopulateWithSections() instead of doing
	/// it inside a loop of calls to GetSectionName().
	///
	/// \sa GetInstrFromSection, GetNumInstrumentsInSection, GetNumSections,
	/// 	PopulateWithSections
	wxString GetSectionName(int nSect);


private:
	wxMidiDatabaseGM();
	void	Initialize();

    enum {
        NUM_SECTIONS = 16,  //number of sections
        NUM_INSTRS = 8,     //max. number of instruments per section
    };
	int			m_nSectInstr[NUM_SECTIONS][NUM_INSTRS];		//instruments in each section
	int			m_nNumInstrInSection[NUM_SECTIONS];			//points to last instrument in section
	wxString	m_sSectName[NUM_SECTIONS];					//section names

	static wxMidiDatabaseGM*	m_pInstance;	//the only instance of this class

};

//-----------------------------------------------------------------------------
/// Class wxMidiSystem is the entry point to the wxMidi library. It provides
/// information about, and access to, the set of installed MIDI devices.
///
/// wxMidiSystem is a singleton and, therefore, the constructor is not public.
/// Access to the only instance must be through method wxMidiSystem::GetInstance().
class wxMidiSystem
{
public:
	/// Destructor.
	~wxMidiSystem();

	/// Returns a pointer to the only wxMidiSystem instance. Note that wxMidiSystem is
	/// a singleton and, therefore, the constructor is not public. Access to the only
	/// instance must be through this GetInstance() method.
	static wxMidiSystem* GetInstance();

	/// Returns current value of the Midi timer. This timer is started when the Midi
	/// package is initialized and has a resolution of one millisecond.
	wxMidiTimestamp GetTime() { return Pt_Time(); }

	/// Translates a portmidi error number into a human readable message. For an
	/// explanation of error codes see wxMidi error codes.
	const wxString GetErrorText( wxMidiError errnum );

	/// Translate portmidi host error into human readable message. After this
	/// routine executes, the host error is cleared.
	wxString GetHostErrorText();

	/// Returns the number of MIDI devices present in the system.
	/// The IDs of the MIDI devices will range from 0 to CountDevices() - 1. So,
	/// for example,  if CountDevices() returns 3, it means that there are three
	/// MIDI devices in the system and that their IDs will be 0, 1 and 2. To
	/// determine if they are output devices or input devices, you will have
	/// to instantiate the corresponding wxMidiDevice object and use methods
	/// wxMidiDevice::IsInput() or wxMidiDevice::IsOutput(), as in the
	/// following example:
	///
	/// \code
	/// // populate two combo boxes with the available Midi devices,
	/// // one combo box with input devices and the other one
	/// // with output devices.
	///
	/// wxMidiSystem* pMidi = wxMidiSystem::GetInstance();
	/// int nNumDevices = pMidi->CountDevices();
	/// wxString sMsg;
	///
	/// // get available input and output devices
	/// int nItem, nInput=0, nOutput=0;
	/// for (int i = 0; i < nNumDevices; i++) {
	///     wxMidiOutDevice* pMidiDev = new wxMidiOutDevice(i);
	///     if (pMidiDev->IsOutputPort()) {
	///         nOutput++;
	///         sMsg.Printf(_T("%s [%s]"),
	///                 pMidiDev->DeviceName().c_str(),
	///                 pMidiDev->InterfaceUsed().c_str() );
	///         nItem = m_pOutCombo->Append(sMsg);
	///         m_pOutCombo->SetClientData(nItem, reinterpret_cast<void*>(i));
	///     }
	///     if (pMidiDev->IsInputPort()) {
	///         nInput++;
	///         sMsg.Printf(_T("%s [%s]"),
	///                 pMidiDev->DeviceName().c_str(),
	///                 pMidiDev->InterfaceUsed().c_str() );
	///         nItem = m_pInCombo->Append(sMsg);
	///         m_pInCombo->SetClientData(nItem, reinterpret_cast<void*>(i));
	///     }
	///     delete pMidiDev;
	/// }
	/// if (nOutput > 0) m_pOutCombo->SetSelection(0);
	/// if (nInput > 0) m_pInCombo->SetSelection(0);
	///
	/// \endcode
	inline int CountDevices() { return Pm_CountDevices(); }

protected:
	wxMidiSystem() {}

private:
	wxMidiError Initialize();
	wxMidiError Terminate();

	static wxMidiSystem*	m_pInstance;	//the only instance of this class
};


//-----------------------------------------------------------------------------
/// Helper class for internal use
class wxMidiThread : public wxThread
{
public:
	wxMidiThread(wxMidiInDevice* pDev, wxWindow* pWindow, unsigned long milliseconds);
	~wxMidiThread();

    // thread execution starts here
    void *Entry();

    // called when the thread exits
	void OnExit() {}

public:
	wxMidiInDevice*	m_pDev;				//owner Midi device
	wxWindow*		m_pWindow;			//the window that will receive the events
	unsigned long	m_nMilliseconds;	//Midi-In polling interval, in milliseconds

};


#endif  // __WXMIDI_H__
