//=====================================================================================
// wxMidi: A MIDI interface based on PortMidi, the Portable Real-Time MIDI Library
// --------------------------------------------------------------------------------
//
// Author:      Cecilio Salmeron <s.cecilio@gmail.com>
// Copyright:   (c) 2005-2015 Cecilio Salmeron
// Licence:     wxWidgets license, version 3.1 or later at your choice.
//=====================================================================================
#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "wxMidi.h"
#endif

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "wxMidi.h"

/*
	To do, when a new version of portmidi:
	1. Review error messages (method GetErrorText(), copied and adapted from pormidi.c)


*/

// Private global functions
//---------------------------------------------------------------------------------
wxMidiTimestamp wxMidiGetTime() { return Pt_Time(); }

//Define the new command event to inform that MIDI input data is available
DEFINE_EVENT_TYPE(wxEVT_MIDI_INPUT)

//================================================================================
// Implementation of classes wxMidiPmEvent, wxMidiShortEvent and wxMidiSysExEvent
//================================================================================

#define SYSEX_CHUNK_SIZE		4								// PmEvent size
#define SYSEX_BUFFER_SIZE		(SYSEX_CHUNK_SIZE * 1024)		// alloc in 4K chunks

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Default constructor, intended for wxMidi internal use.
wxMidiSysExMessage::wxMidiSysExMessage()
    : wxMidiMessage()
	, m_pMessage(NULL)
	, m_timestamp(0)
	, m_nError(wxMIDI_NO_ERROR)
	, m_nSize(0L)
{
	m_type = wxMIDI_SYSEX_MSG;
}

//-----------------------------------------------------------------------------
/** Constructor, creating a wxMidiSysExMessage object from a string
    of wxBytes.

    In case of error during construction, subsequent
    calls to Error() will return a error code with more
    information about the error.
    \param msg  The raw MIDI message, as string of wxBytes. It must
           include the start-of-sysex status byte (0xF0) at the
           beginning and the end-of-sysex status byte (0xF7) at the
           end.
    \param timestamp  The timestamp value. It is milliseconds elapsed
           since the wxMidi package initialization. Current time can be
           obtained by calling wxMidi::GetTime(). Only meaningful if
           the wxOutDevice is open with latency time different from
           zero. In any case, if no timestamp is specified or if a
           value of zero is specified, the message will be delivered
           immediately. See wxMidiOutDevice::Write().
*/
wxMidiSysExMessage::wxMidiSysExMessage(wxByte* msg, wxMidiTimestamp timestamp)
    : wxMidiMessage()
	, m_pMessage(NULL)
	, m_timestamp(timestamp)
	, m_nError(wxMIDI_NO_ERROR)
	, m_nSize(0L)
{
	m_type = wxMIDI_SYSEX_MSG;

	//verify start of message
	if (*msg != 0xF0) {
		m_nError = wxMIDI_ERROR_BadSysExMsg_Start;
		return;
	}

	//determine size of message
	wxByte* pData = msg;
	for (; *pData != 0xF7; pData++) {
		m_nSize++;
	}

	// allocate the buffer
	m_nSize++;		//include the end of sysex byte 0xF7
	m_pMessage = new wxByte[m_nSize];

	// copy the data
	memcpy(m_pMessage, msg, m_nSize);
}

//-----------------------------------------------------------------------------
/// Destructor
wxMidiSysExMessage::~wxMidiSysExMessage()
{
	if (m_pMessage) {
		delete [] m_pMessage;
		m_pMessage = (wxByte*) NULL;
	}
}

//================================================================================
// Implementation of class wxMidiDevice
//================================================================================


wxMidiDevice::wxMidiDevice(wxMidiDeviceID nDevice)
    : m_nDevice(nDevice)
{
	int devices = Pm_CountDevices();
	if (m_nDevice > devices-1) m_nDevice = 0;
	m_pInfo = Pm_GetDeviceInfo(m_nDevice);
	m_stream = (PortMidiStream*)NULL;
}

wxMidiDevice::~wxMidiDevice()
{
    //AWARE: Pm_GetDeviceInfo() returns a pointer to a PmDeviceInfo structure
    //referring to the device specified by id. The returned structure is owned by
    //the PortMidi implementation and must not be manipulated or freed.

	//delete m_pInfo;
}

const wxString wxMidiDevice::DeviceName()
{
	if (m_pInfo) {
#if defined(_UNICODE) || defined(UNICODE)
		return wxString(m_pInfo->name, wxConvUTF8);
#else
		return m_pInfo->name;
#endif
	}
	else {
		wxString sMsg = _("Error: Device not properly created");
		return sMsg;
	}
}

const wxString wxMidiDevice::InterfaceUsed()
{
	if (m_pInfo) {
#if defined(_UNICODE) || defined(UNICODE)
		return wxString(m_pInfo->interf, wxConvUTF8);
#else
		return m_pInfo->interf;
#endif
	}
	else {
		wxString sMsg = _("Error: Device not properly created");
		return sMsg;
	}
}

bool wxMidiDevice::IsInputPort()
{
	if (m_pInfo)
		return (m_pInfo->input != 0);
	else
		return false;
}

bool wxMidiDevice::IsOutputPort()
{
	if (m_pInfo)
		return (m_pInfo->output != 0);
	else
		return false;
}


//================================================================================
// Class wxMidiOutDevice implementation
//================================================================================

wxMidiError wxMidiOutDevice::Open(long latency, void* pDriverInfo)
{
	return (wxMidiError)Pm_OpenOutput(&m_stream, m_nDevice, pDriverInfo,
									  0, NULL, NULL, latency );
}

wxMidiError wxMidiOutDevice::Write(wxMidiShortMessage* pMsg)
{
	return (wxMidiError)Pm_Write(m_stream, pMsg->GetBuffer(), 1);
}

wxMidiError wxMidiOutDevice::Write(wxMidiSysExMessage* pMsg)
{
	wxMidiError nError = pMsg->Error();
	if (nError != wxMIDI_NO_ERROR)
		return nError;
	else
		return (wxMidiError)Pm_WriteSysEx(m_stream,
									  pMsg->GetTimestamp(),
									  (unsigned char *)pMsg->GetMessage() );
}

wxMidiError wxMidiOutDevice::Write(wxByte* msg, wxMidiTimestamp when)
{
	return (wxMidiError)Pm_WriteSysEx(m_stream,
									  when,
									  (unsigned char *)msg );
}

wxMidiError	wxMidiOutDevice::NoteOff(int channel, int note, int velocity)
{
	//0x80-0x8F		Note off
	wxMidiShortMessage msg(0x80+channel, note, velocity);
	return Write(&msg);
}

wxMidiError	wxMidiOutDevice::NoteOn(int channel, int note, int velocity)
{
    //0x90-0x9F		Note on
	wxMidiShortMessage msg(0x90+channel, note, velocity);
	return Write(&msg);
}

wxMidiError	wxMidiOutDevice::ProgramChange(int channel, int instrument)
{
    //0xC0-0xCF		Program change
	wxMidiShortMessage msg(0xC0+channel, instrument, 0);
	return Write(&msg);
}

wxMidiError wxMidiOutDevice::AllSoundsOff()
{
    //  http://www.midi.org/techspecs/midimessages.php

    for (int channel=0; channel < 16; ++channel)
    {
        wxMidiShortMessage msg(0xB0+channel, 0x78, 0);      //all sound off
        Write(&msg);
    }
    return wxMIDI_NO_ERROR;

}


//================================================================================
// Class wxMidiInDevice implementation
//================================================================================

wxMidiInDevice::wxMidiInDevice(wxMidiDeviceID nDevice, double timeoutSeconds)
	: wxMidiDevice(nDevice)
	, m_pThread(NULL)
	, m_fUseTimeAlgorithm(true)
    , m_timeCounter(time_t(-1))
	, m_timeoutSeconds(timeoutSeconds)
	, m_numNullReads(0)
	, m_SysexBuffer(NULL)
	, m_fReadingSysex(false)
	, m_fEventPending(false)
{
}

wxMidiInDevice::~wxMidiInDevice()
{
	if (m_pThread) {
		StopListening();
	}
	if (m_SysexBuffer) delete [] m_SysexBuffer;
}

wxMidiError wxMidiInDevice::Open(void* pDriverInfo, int buffersize)
{
	return (wxMidiError)Pm_OpenInput(&m_stream, m_nDevice, pDriverInfo,
									 buffersize, NULL, NULL);
}

wxMidiError wxMidiInDevice::Read(wxMidiPmEvent* buffer, long* length )
{
	/*
	If no error, the real number of buffers read is returned in "length" and
	value PmNoError is returned.
	In case of error, the error type is returned.
	*/

	int nErr = (int) Pm_Read(m_stream, buffer, *length);
	/* Pm_Read() retrieves midi data into a buffer, and returns the number
    of events read. Result is a non-negative number unless an error occurs,
    in which case a wxMidiError value will be returned. */
	if (nErr < 0)
		//error
		return wxMidiError(nErr);
	else {
		*length = long(nErr);
		return wxMIDI_NO_ERROR;
	}

}

wxMidiMessage* wxMidiInDevice::Read(wxMidiError* pError)
{
	/*
	According to pormidi documentation, portmidi is allowed to pass real-time MIDI
	messages embedded within the chunks of a sysex message, and it is up to the client
	to detect, process, and	remove these messages as they arrive.

	To deal with this, xMidiInDevice will maintain buffers with data not yet delivered.
	Flag m_fReadingSysex will signal that a sysex message was interrupted by a real-time one
	Flag m_fEventPending will signal that there is a PmEvent pending to be processed and
	delivered, as consequence of a previous truncated sysex message.

	*/

    reset_timeout_counters();

	// get data from midi stream
	PmEvent buffer;
	wxMidiError nError;

	if (m_fEventPending) {
		buffer = m_event;
		m_fEventPending = false;
	}
	else {
        nError = (wxMidiError) Pm_Read( m_stream, &buffer, 1 );

		// if read was not successful return the error
		if (nError < wxMIDI_NO_ERROR) {
			*pError = nError;
			return (wxMidiMessage*)NULL;
		}

		// check if something read
		if (nError == 0) {
			*pError = wxMIDI_ERROR_NoDataAvailable;
			return (wxMidiMessage*)NULL;
		}
	}

	// check what type of message we are receiving
	if (m_fReadingSysex || (Pm_MessageStatus( buffer.message ) == 0xF0))
	{
		// Start or continuation of a Sysex message. Move data to buffer and
		// continue reading until end of message

		//create the message object
		wxMidiSysExMessage* pSysExMsg = new wxMidiSysExMessage();

		if (!m_fReadingSysex)
		{
			//store timestamp
			m_timestamp = buffer.timestamp;

			// alloc a new buffer
			m_SizeOfSysexBuffer = SYSEX_BUFFER_SIZE;
			m_SysexBuffer = new wxByte[ m_SizeOfSysexBuffer ];
			m_CurSysexDataPtr = m_SysexBuffer;

			m_fReadingSysex = true;
		}

		//move data to buffer and continue reading until end of sysex message
		bool fEndSysex = MoveDataToSysExBuffer(buffer.message);

		while(!fEndSysex)
        {
            nError = (wxMidiError) Pm_Read( m_stream, &buffer, 1 );

			if (nError < wxMIDI_NO_ERROR) {
				*pError = nError;
				delete pSysExMsg;
				delete [] m_SysexBuffer;
				m_SysexBuffer = (wxByte*)NULL;
				m_fReadingSysex = false;
				return (wxMidiMessage*)NULL;
			}

            //Read operations can be faster than arrival of bytes (3.8 KB/sec at max.).
            //Therefore, some read operations will return 0 bytes
            if (nError == 0)
            {
                //nothing read. Connection failure?
                if (should_report_timeout())
                {
                    *pError = wxMIDI_ERROR_TimeOut;
                    delete pSysExMsg;
                    return (wxMidiMessage*)NULL;
                }
            }
            else
            {
                //some bytes received
                reset_timeout_counters();

                //check if it is a real-time message inserted into the sysex chunks stream
                if (Pm_MessageStatus( buffer.message ) == 0xF8)
                {
                    //it is a real time message. Deliver it immediately and save sysex buffer
                    *pError = wxMIDI_NO_ERROR;
                    wxMidiShortMessage* pShortMsg = new wxMidiShortMessage(
                                                            Pm_MessageStatus( buffer.message ),
                                                            0, 0 );
                    pShortMsg->SetTimestamp( buffer.timestamp );
                    delete pSysExMsg;
                    return pShortMsg;
                }

                /*
                When receiving sysex messages, if you get a non-real-time status byte
                but there was no EOX byte, it means the sysex message was somehow truncated.
                This is not	considered an error; e.g., a missing EOX can result from the user
                disconnecting a MIDI cable during sysex transmission.
                */

                // lets check if there is a status byte different from end-of-sysex
                if (Pm_MessageStatus( buffer.message ) & 0x80  &&
                    Pm_MessageStatus( buffer.message ) != 0xF7)
                {
                    //The systex message is somehow truncated. Return the sysex message
                    // and store the new message received

                    // save the PmEvent
                    m_fEventPending = true;
                    m_event.message = buffer.message;
                    m_event.timestamp = buffer.timestamp;

                    //terminate the current sysex message
                    fEndSysex = true;
                }
                else
                {
                    // it is a chunck of the sysex message. Move data to the sysex buffer
                    fEndSysex = MoveDataToSysExBuffer(buffer.message);
                }
            }
		}

		//prepare the sysex message to return it
		pSysExMsg->SetTimestamp( m_timestamp );
		pSysExMsg->SetBuffer( m_SysexBuffer );
		pSysExMsg->SetLength( m_CurSysexDataPtr - m_SysexBuffer );
		*pError = wxMIDI_NO_ERROR;

		//reset the wxMidiInDevice buffer
		m_SysexBuffer = (wxByte*)NULL;
		m_CurSysexDataPtr = (wxByte*)NULL;
		m_fReadingSysex = false;

		return pSysExMsg;
	}
	else
	{
		//it is not a sysex message
		*pError = wxMIDI_NO_ERROR;
		wxMidiShortMessage* pShortMsg = new wxMidiShortMessage(
												Pm_MessageStatus( buffer.message ),
												Pm_MessageData1( buffer.message ),
												Pm_MessageData2( buffer.message ) );
		pShortMsg->SetTimestamp( buffer.timestamp );
		return pShortMsg;
	}

}

bool wxMidiInDevice::MoveDataToSysExBuffer(PmMessage message)
{
	/*
	Moves the chunk received to the sysex buffer, allocating a greater buffer
	if necessary.
	Returns true if with this chunk the sysex message is ended, false otherwise
	*/

	// if we are at the end of the buffer
	if( m_CurSysexDataPtr > (m_SysexBuffer + m_SizeOfSysexBuffer - SYSEX_CHUNK_SIZE) )
	{
		// keep previous size for usage
		long oldSize = m_SizeOfSysexBuffer;

		// increment size
		m_SizeOfSysexBuffer += SYSEX_BUFFER_SIZE;

		// allocate the new buffer
		wxByte* newSysexBuffer = new wxByte[ m_SizeOfSysexBuffer ];

//		wxLogDebug(wxString::Format(
//			_T("Increasing buffer size from %ld to %ld"), oldSize, m_SizeOfSysexBuffer ));

		// move the data from old buffer
		memcpy( newSysexBuffer, m_SysexBuffer, oldSize );

		// delete the old buffer
		delete [] m_SysexBuffer;

		// point to the new buffer
		m_SysexBuffer = newSysexBuffer;

		// set the write data pointer
		m_CurSysexDataPtr = m_SysexBuffer + oldSize;
	}

	// move data to the buffer. See portmidi/pm_test/sysex.c function receive_sysex()
	int shift = 0;
	int data = 0;
	for (shift = 0; shift < 32 && (data != 0xF7); shift += 8)
	{
		data = (message >> shift) & 0xFF;
		*m_CurSysexDataPtr = data;
		++m_CurSysexDataPtr;

			/* portmidi:
			When receiving sysex messages, the sysex message is terminated
			by either an EOX status byte (anywhere in the 4 byte messages) or
			by a non-real-time status byte in the low order byte of the message.
			If you get a non-real-time status byte but there was no EOX byte, it
			means the sysex message was somehow truncated. This is not
			considered an error; e.g., a missing EOX can result from the user
			disconnecting a MIDI cable during sysex transmission.
			*/

        // if this is a status byte that is not EOX, the sysex
        // message is incomplete and there is no more sysex data
		//
		// WARNING: Is this true? Shouldn't portmidi send the status byte
		// in a new PmEvent? See portmidi/pm_test/sysex.c function receive_sysex()
		//
		// TODO: if we break without doing anything else, the status byte and
		// remaining bytes will be lost !!!
        if (data & 0x80 && data != 0xF0 && data != 0xF7) break;
	}

	return (data & 0x80 && data != 0xF0);

}

void wxMidiInDevice::Flush()
{
    // empty the buffer
	wxMidiPmEvent buffer[1];
	long nNumEvents;
    while (Poll()) {
		nNumEvents = 1;			//initialize with buffer size before calling Read
        Read(buffer, &nNumEvents);
    }

}

wxMidiError wxMidiInDevice::StartListening(wxWindow* pWindow, unsigned long nPollingRate)
{
	if (m_pThread) return wxMIDI_ERROR_AlreadyListening;

    //Create a new thread. The thread object is created in the suspended state
	m_pThread = new wxMidiThread(this, pWindow, nPollingRate);
	if ( m_pThread->Create() != wxTHREAD_NO_ERROR )
    {
        return wxMIDI_ERROR_CreateThread;		//Can't create thread!
    }

    //wxCriticalSectionLocker enter(wxGetApp().m_critsect);

	//Start the thread execution
    if (m_pThread->Run() != wxTHREAD_NO_ERROR )
    {
        return wxMIDI_ERROR_StartThread;		//Can't start thread!
    }

	return wxMIDI_NO_ERROR;
}

wxMidiError wxMidiInDevice::StopListening()
{
	//stop the thread and wait for its termination
	m_pThread->Delete();
	delete m_pThread;
	m_pThread = (wxMidiThread*)NULL;
	return wxMIDI_NO_ERROR;
}

bool wxMidiInDevice::should_report_timeout()
{
    return use_time_algorithm() ?
           too_much_time_without_receiving_bytes() :
           too_much_reads_without_receiving_bytes();
}

bool wxMidiInDevice::too_much_time_without_receiving_bytes()
{
    //true when elapsed time since first void Read operation is greater
    //than the user specified timeout (in wxMidiInDevice constructor)

    time_t currentTime = time(NULL);    //current time (in seconds elapsed since the Epoch)

    if (currentTime == time_t(-1))
    {
        //failure to compute current time
        switch_to_alternate_algorithm();
        return false;
    }

    if (m_timeCounter == time_t(0))
    {
        m_timeCounter = currentTime;
        return false;
    }
    else
    {
        double iddleTime = difftime(currentTime, m_timeCounter);
        return iddleTime > m_timeoutSeconds;
    }
}

bool wxMidiInDevice::too_much_reads_without_receiving_bytes()
{
    //true when too much consecutive Read operations without receiving bytes

    int maxReads = int(m_timeoutSeconds / 0.01);    //assume 10ms per Read operation
    return ++m_numNullReads > maxReads;
}


//================================================================================
// Class wxMidiThread implementation
//================================================================================

// In MSWin32 all threads are joinable. So for platform independence I will
// always use joinable threads
wxMidiThread::wxMidiThread(wxMidiInDevice* pDev, wxWindow* pWindow, unsigned long milliseconds)
	: wxThread(wxTHREAD_JOINABLE)
{
	m_pDev = pDev;
	m_pWindow = pWindow;
	m_nMilliseconds = milliseconds;
}

wxMidiThread::~wxMidiThread()
{
}

void* wxMidiThread::Entry()
{
    try
    {
        while(true)
        {
            // check if the thread was asked to exit and do it
            if (TestDestroy()) break;

            // check if Midi data is available
            if ( m_pDev->Poll() ) {
                // Data available. Create a Midi event
                wxCommandEvent event( wxEVT_MIDI_INPUT );
                ::wxPostEvent( m_pWindow, event );
            }

            // pause the thread execution during polling rate interval
            wxThread::Sleep(m_nMilliseconds);
        }
    }
    catch(...)
    {
    }
    return NULL;
}



//================================================================================
// Class wxMidiSystem implementation
//
//	According to documentation Pm_Initialize and Pm_Terminate return a error code, but
//	looking at the source code they always return pmNoError. So it is useless to
//	try to preserve the return code by forcing the user to explicitly call
//	these methods. It is easier to initialize and terminate in
//	the constructor and destructor, respectively
//================================================================================


wxMidiSystem* wxMidiSystem::m_pInstance = (wxMidiSystem*)NULL;

wxMidiSystem::~wxMidiSystem()
{
	Terminate();
	wxMidiDatabaseGM* pGM = wxMidiDatabaseGM::GetInstance();
	delete pGM;
}

wxMidiError wxMidiSystem::Initialize()
{
	Pt_Start(1, 0, 0);						// start the timer
	return (wxMidiError)Pm_Initialize();	// initialize portmidi package
}

wxMidiError wxMidiSystem::Terminate()
{
	Pt_Stop();				//stop the timer
	return (wxMidiError)Pm_Terminate();
}

const wxString wxMidiSystem::GetErrorText( wxMidiError errnum )
{
    wxString sError;
    switch(errnum)
    {
    case wxMIDI_NO_ERROR:
		sError = _T("wxMidi: 'No error'");
		return sError;
    case wxMIDI_ERROR_HostError:
        sError = _("PortMidi: `Host error'");
		return sError;
    case wxMIDI_ERROR_InvalidDeviceId:
		sError = _("PortMidi: `Invalid device ID'");
		return sError;
    case wxMIDI_ERROR_InsufficientMemory:
        sError = _("PortMidi: `Insufficient memory'");
		return sError;
    case wxMIDI_ERROR_BufferTooSmall:
        sError = _("PortMidi: 'Buffer too small'");
		return sError;
    case wxMIDI_ERROR_BadPtr:
        sError = _("PortMidi: `Bad pointer'");
		return sError;
    case wxMIDI_ERROR_InternalError:
        sError = _("PortMidi: 'Internal PortMidi Error'");
		return sError;
    case wxMIDI_ERROR_BufferOverflow:
        sError = _("PortMidi: 'Buffer overflow'");
		return sError;
    case wxMIDI_ERROR_BadData:
		sError = _("PortMidi: 'Invalid MIDI message Data'");
		return sError;
	case wxMIDI_ERROR_BufferMaxSize:
		sError = _("PortMidi: 'Buffer is already as large as it can be'");
		return sError;

	// Additional error messages for wxMidi
	case wxMIDI_ERROR_AlreadyListening:
        sError = _("wxMidi: 'Already lisening!'");
		return sError;
	case wxMIDI_ERROR_CreateThread:
        sError = _("wxMidi: 'Error while creating the thread object'");
		return sError;
	case wxMIDI_ERROR_StartThread:
        sError = _("wxMidi: 'Error while starting the thread execution'");
		return sError;
	case wxMIDI_ERROR_BadSysExMsg_Start:
		sError = _("wxMidi. 'Bad sysex msg: It does not start with 0xF0'");
		return sError;
//	case wxMIDI_ERROR_BadSysExMsg_Length:
//		sError = _("wxMidi. 'Bad sysex msg: Length greater than 10000 or no final byte 0xF7'");
//		return sError;
	case wxMIDI_ERROR_NoDataAvailable:
		sError = _("wxMidi. 'There are no MIDI messages pending to be read'");
		return sError;
	case wxMIDI_ERROR_TimeOut:
		sError = _("wxMidi: Timeout while receiving a SysEx message");
		return sError;

	default:
        sError = _("wxMidi: 'Illegal error number'");
 		return sError;
   }
}

//-----------------------------------------------------------------------------
wxString wxMidiSystem::GetHostErrorText()
{
	//TODO: review this
	char msg[1000];
	Pm_GetHostErrorText(&msg[0], 1000);
	return wxString( (const wxChar *)&msg[0] );
}

wxMidiSystem* wxMidiSystem::GetInstance()
{
	if (!m_pInstance) {
		m_pInstance = new wxMidiSystem();
		m_pInstance->Initialize();
	}
	return m_pInstance;
}

