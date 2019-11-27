Please consult my [GitHub](https://pdsmart.github.io) website for more upto date information.
<br>


This repository is a collection of software libraries I've developed over time, some which I aim to use with the ZPU Evo.

Currently there is an issue with the ZPU GCC Toolchain in that it doesnt support network programming or the newlib library. This means UX wont fully build for the ZPU Evo but it is something I will endeavour to resolve as I want network functionality and threads in zOS.

<br>

# UX Library

In 1994 most C program creation involved buying expensive toolkits (such as Rogue Wave) and cobbling the often faulty library API's together or writing your own which was no trivial task. Today, most modern languages have very rich eco systems, ie. Java, Perl, Python etc. Libraries can be found for C/C++ such as this one which are open source to speed development but you may have to look hard.
I wrote this library to make it easier to write C programs for clients and often the whole toolkit was eventually given to the client and enhanced considerably for them whilst designing their applications (unfortunately due to licensing I couldnt back port the enhancements).
It makes it much easier to write C programs on Linux, Solaris and Windows and for embedded development will be very useful. The ZPU Evo is one such target for this toolkit, hence bringing it back to life. 

The methods in the UX Library are described below ordered by the module to which they belong. If a method begins with '_' then it is internal and normally not called directly, albeit being C there is no Private definition to methods or their data so you can call them if it helps.

### ux_cli

Unix or Windows Command Line Processing functions.

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**GetCLIParam**|
 |Description:    |Get a Command Line Interface Parameter from the command line of the shell which started this program... Confusing!|
 |Returns:        |R_OK   - Parameter obtained.<br>R_FAIL - Parameter does not exist.|
 |Prototype:      |`int GetCLIParam( int nArgc /* I: Argc or equiv */, UCHAR **Argv /* IO: Argv or equiv */, UCHAR *szParm /* I: Param flag to look for */, UINT nParmType /* I: Type of param (ie int) */, UCHAR *pVar /* O: Pointer to variable for parm */, UINT nVarLen /* I: Length pointed to by pVar */, UINT nZapParam ) /* I: Delete argument after proc */`|

### ux_cmprs

A set of methods to compress/decompress data. The basic code stems from a LINUX public domain lzw compression/decompression algorithm, basically tidied up a little and enhanced to allow embedding within programs. Eventually, a more hi-tech algorithm will be implemented, but for now, this lzw appears to have very high compression ratio's on text.

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**Compress**|
 |Description:    |A generic function to compress a buffer of text. |
 |Returns:        |NULL - Memory problems.<br>Memory buffer containing compressed copy of input.|
 |Prototype:      |`UCHAR *Compress( UCHAR *spInBuf /* I: Buffer to be compressed. */, UINT *nLen ) /* IO: Length of dec/compressed buffer. */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**Decompress**|
 |Description:    |A generic function to de-compress a buffer to text. |
 |Returns:        |NULL - Memory problems.<br>Memory buffer containing decompressed copy of input.|
 |Prototype:      |`UCHAR *Decompress( UCHAR *spInBuf /* I: Buffer to be decompressed. */, UINT *nCmpLen ) /* IO: Length of comp/dec buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**WLZW**|
 |Description:    |Write or compress data in LZW format.|
 |Returns:        |0  = Worthless CPU waste (No compression)<br>-1 = General error<br>-2 = Logical error<br>-3 = Expand error<br>>0 = OK/total length|
 |Prototype:      |`int WLZW( byte *si /* I: Data for compression */, code *so /* O: Compressed data */, int len /* I: Length of data for compression */, int maxlen ) /* I: Maximum length of compressed data */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**RLZW**|
 |Description:    |Read or de-compress data from LZW format.|
 |Returns:        |0  = Worthless CPU waste (No compression)<br>-1 = General error<br>-2 = Logical error<br>-3 = Expand error<br>>0 = OK/total length|
 |Prototype:      |`int RLZW( code *si /* I: Data to be decompressed */, byte *so /* O: Decompressed data */, int silen /* I: Compressed length */, int len ) /* I: Expected length of decompressed data. */`|

### ux_comms

Generic network communications routines. These form the basis of daemon functionality, receiving connections and scheduling processes and callbacks.

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_CalcCRC**|
 |Description:    |Calculate the CRC on a buffer.|
 |Thread Safe:    | Yes|
 |Returns:        |16bit CRC|
 |Prototype:      |`UINT _SL_CalcCRC( UCHAR *szBuf /* I: Data buffer to perform CRC on */, UINT nBufLen ) /* I: Length of data buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_CheckCRC**|
 |Description:    |Validate the CRC on a buffer with the one given.|
 |Thread Safe:    | Yes|
 |Returns:        |R_OK     - CRC match.<br>R_FAIL   - CRC failure.|
 |<Errno>         |  |
 |Prototype:      |`UINT _SL_CheckCRC( UCHAR *szBuf /* I: Data buffer to calc CRC on */, UINT nBufLen ) /* I: Length of data in buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_FdBlocking**|
 |Description:    |Set a file descriptors blocking mode.|
 |Thread Safe:    | Yes|
 |Returns:        |R_OK   - Mode set.<br>R_FAIL - Couldnt set mode.|
 |Prototype:      |`int _SL_FdBlocking( int nFd /* File descr to perform action on*/, int nBlock ) /* Block (1) or non-blocking (0) */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_AcceptClient**|
 |Description:    |Accept an incoming request from a client. Builds a duplicate table entry for the client (if one doesnt already exist) and allocates a unique Channel Id to it.|
 |Thread Safe:    | No, ensures only SL library thread may enter.|
 |Returns:        |R_OK     - Comms functionality initialised.<br>R_FAIL   - Initialisation failed, see Errno.<br>|
 |<Errno>         |E_NOMEM  - Memory exhaustion.|
 |Prototype:      |`int _SL_AcceptClient( UINT nServerSd /* I: Server Socket Id */, SL_NETCONS *spServer /* I: Server descr record */, SL_NETCONS **spNewClnt ) /* O: New client */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_GetPortNo**|
 |Description:    |Get valid port number from structures.|
 |Thread Safe:    | Yes|
 |Returns:        |Port Number.|
 |Prototype:      |`UINT _SL_GetPortNo( SL_NETCONS *spNetCon ) /* I: Connection description */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_Close**|
 |Description:    |Close a client or server connection.|
 |Thread Safe:    | No, forces SL thread entry only.|
 |Returns:        |R_OK     - Connection closed successfully.<br>R_FAIL   - Failed to close connection, see Errno.|
 |<Errno>         |  |
 |Prototype:      |`int _SL_Close( SL_NETCONS *spNetCon /* I: Connection to sever */, UINT nDoCallback ) /* I: Perform closure callback? */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_ConnectToServer**|
 |Description:    |Attempt to make a connection with a remote server.|
 |Thread Safe:    | No, forces SL thread entry only.|
 |Returns:        |R_OK     - <br>R_FAIL   - <br>|
 |<Errno>         |E_NOMEM    - Memory exhaustion.<br>E_NOSOCKET - Couldnt allocate a socket for connection.|
 |Prototype:      |`int _SL_ConnectToServer( SL_NETCONS *spNetCon )|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_ReceiveFromSocket**|
 |Description:    |Receive data from a given socket, growing the receive buffer if needed.|
 |Thread Safe:    | No, forces SL thread entry only.|
 |Returns:        |R_OK     - <br>R_FAIL   - <br>|
 |<Errno>         |E_NOMEM   - Memory exhaustion.<br>E_BADPARM - Bad parameters passed to function.<br>E_NOSERVICE - No service on socket.|
 |Prototype:      |`int    _SL_ReceiveFromSocket( SL_NETCONS    *spNetCon )    /* IO: Active connection */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_ProcessRecvBuf**|
 |Description:    |Process the data held in a network connection's receive buffer. If a complete packet has been assembled and passed a CRC check, pass the data to the subscribing application via its callback.|
 |Thread Safe:    | No, forces SL thread entry only.|
 |Returns:        |R_OK     - <br>R_FAIL   - <br>|
 |<Errno>         |E_NOMEM    - Memory exhaustion.<br>E_NOSOCKET - Couldnt allocate a socket for connection.|
 |Prototype:      |`int _SL_ProcessRecvBuf( SL_NETCONS *spNetCon )`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_ProcessWaitingPorts**|
 |Description:    |
 |Thread Safe:    | No, forces SL Thread only.|
 |Returns:        |R_OK    - Select succeeded.<br>R_FAIL  - Catastrophe, see Errno.|
 |<Errno>         |E_BADSELECT  - Internal failure causing select to fail.<br>E_NONWAITING - No sockets waiting processing.|
 |Prototype:      |`int _SL_ProcessWaitingPorts( ULNG nHibernationPeriod )    /* I: Select sleep*/`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SL_ProcessCallbacks**|
 |Description:    |Activate any callbacks whose timers are active and have expired.|
 |Thread Safe:    | No, only allows SL Thread.|
 |Returns:        |Time in mS till next callback.|
 |<Errno>         |  |
 |Prototype:      |`ULNG _SL_ProcessCallbacks( void )`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_HostIPtoString**|
 |Description:    |Convert a given IP address into a string dot notation.|
 |Thread Safe:    | No, API only allows one thread at a time.|
 |Returns:        |16bit CRC|
 |Prototype:      |`UCHAR *SL_HostIPtoString( ULNG lIPaddr ) /* I: IP address to convert */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_GetIPaddr**|
 |Description:    |Get the Internet address of the local machine or a named machine.|
 |Thread Safe:    | No, API only allows one thread at a time.|
 |Returns:        |R_OK   - IP address obtained.<br>R_FAIL - IP address not obtained.|
 |Prototype:      |`int SL_GetIPaddr( UCHAR *szHost /* I: Hostname string */, ULNG *lIPaddr ) /* O: Storage for the Internet Addr */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_GetService**|
 |Description:    |Get the TCP/UDP service port number from the Services File.|
 |Thread Safe:    | No, API Function only allows one thread at a time.|
 |Returns:        |R_OK   - Service port obtained.<br>R_FAIL - Service port not obtained.|
 |Prototype:      |`int SL_GetService( UCHAR *szService /* I: Service Name string */, UINT *nPortNo ) /* O: Storage for the Port Number */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_Init**|
 |Description:    |Initialise communication variables and connect or setup listening for required socket connections. |
 |Thread Safe:    | No, API function only allows one thread at a time.|
 |Returns:        |R_OK     - Comms functionality initialised.<br>R_FAIL   - Initialisation failed, see Errno.|
 |<Errno>         |E_NOMEM  - Memory exhaustion.|
 |Prototype:      |`int SL_Init( UINT nSockKeepAlive /* I: Socket keep alive time period */, UCHAR *szErrMsg ) /* O: Error message buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_Exit**|
 |Description:    |Decommission the Comms module ready for program termination or re-initialisation.|
 |Thread Safe:    | No, API function, only allows one thread at a time.|
 |Returns:        |R_OK     - Exit succeeded.<br>R_FAIL   - Couldnt perform exit processing, see errno.|
 |<Errno>         |
 |Prototype:      |`int SL_Exit( UCHAR *szErrMsg ) /* O: Error message buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_PostTerminate**|
 |Description:    |Post a request for the program to terminate as soon as possible.|
 |Thread Safe:    | Yes|
 |Returns:        |Non.|
 |Prototype:      |`void    SL_PostTerminate( void )`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_GetChanId**|
 |Description:    |Get a channel Id from a given IP addr. If the IP addr doesnt exist or is still pending a connection, return 0 as an error.|
 |Thread Safe:    | No, API function only allows one thread at a time.|
 |Returns:        |> 0  - Channel Id associated with IP address.<br>0    - Couldnt find an associated channel.|
 |Prototype:      |`UINT SL_GetChanId( ULNG lIPaddr )    /* I: Address to xlate */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_RawMode**|
 |Description:    |Function to switch a channel into/out of raw mode processing. Raw mode processing foregoes all forms of checking and is typically used for connections with a non SL lib server/client.|
 |Thread Safe:    | No, API Function, only allows one thread at a time.|
 |Returns:        |R_FAIL  - Illegal Channel Id given.<br>R_OK    - Mode set|
 |<Errno>         |  |
 |Prototype:      |`int SL_RawMode( UINT nChanId /* I: Channel to apply change to */, UINT nMode ) /* I: Mode to set channel to */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_AddServer**|
 |Description:    |Add an entry into the Network Connections table as a Server. An entry is built up and a socket created and set listening.|
 |Thread Safe:    | No, API Function, only allows one thread at a time.|
 |Returns:        |R_OK     - Successfully added.<br>R_FAIL   - Error, see Errno.|
 |<Errno>         |E_NOMEM    - Memory exhaustion.<br>E_BADPARM  - Bad parameters passed.<br>E_EXISTS   - Entry already exists.<br>E_NOSOCKET - Couldnt grab a socket.<br>E_NOLISTEN - Couldnt listen on given port.|
 |Prototype:      |`int SL_AddServer( UINT nPortNo /* I: Port to listen on */, UINT nForkForAccept /* I: Fork prior to accept */, void    (*nDataCallback)() /* I: Data ready callback */, void (*nCntrlCallback)(int, ...) ) /* I: Control callback */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_AddClient**|
 |Description:    |Add an entry into the Network Connections table as a client. Socket creation and connect are left to the kernels discretion.|
 |Thread Safe:    | No, API function, only allows one thread at a time.|
 |Returns:        |>= 0     - Channel Id.<br>-1       - Error, see Errno.|
 |<Errno>         |E_NOMEM  - Memory exhaustion.<br>E_EXISTS - A client of same detail exists.|
 |Prototype:      |`int SL_AddClient( UINT nServerPortNo /* I: Server port to talk on */, ULNG lServerIPaddr /* I: Server IP address */, UCHAR *szServerName /* I: Name of Server */, void (*nDataCallback)() /* I: Data ready callback */, void (*nCntrlCallback)(int, ...) ) /* I: Control callback */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_AddTimerCB**|
 |Description:    |Add a timed callback. Basically, a timed callback is a function which gets invoked after a period of time. This function can be invoked once (TCB_ONESHOT), every Xms (TCB_ASTABLE) or a fixed period of time Xms from last execution (TCB_FLIPFLOP). Each callback can pass a predefined variable/pointer, so multiple instances of the same callback can exist, each referring to the same function, but passing different values to it. The callbacks are maintained in a dynamic link list.|
 |Thread Safe:    | No, API Function, only allows single thread at a time.|
 |Returns:        |R_OK     - Callback added successfully.<br>R_FAIL   - Failure, see Errno.|
 |<Errno>         |E_NOMEM  - Memory exhaustion.|
 |Prototype:      |`int SL_AddTimerCB( ULNG lTimePeriod, /* I: Time between callbacks */, UINT nOptions /* I: Option flags on callback */, ULNG lCBData /* I: Data to be passed to cb */, void (*nCallback)() ) /* I: Function to call */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_DelServer**|
 |Description:    |Delete an entry from the Network Connections table and disable the actual communications associated with it.|
 |Thread Safe:    | No, API function allows one thread at a time.|
 |Returns:        |R_OK     - Successfully deleted.<br>R_FAIL   - Error, see Errno.|
 |<Errno>         |E_BADPARM  - Bad parameters passed.|
 |Prototype:      |`int SL_DelServer( UINT nPortNo )    /* I: Port number that server on */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_DelClient**|
 |Description:    |Delete a client entry from the Network Connections table and free up all resources that it used.|
 |Thread Safe:    | No, API function allows on thread at a time.|
 |Returns:        |R_OK     - Client connection successfully closed.<br>R_FAIL   - Error, see Errno.|
 |<Errno>         |  |
 |Prototype:      |`int SL_DelClient( UINT nChanId ) /* I: Channel Id of client to del*/`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_Close**|
 |Description:    |Close a socket connection (Client or Server) based on the given channel ID. Due to the nature of the socket library, this cannot be performed immediately, as nearly always, the application requesting the close is within a socket library callback, and modifying internal structures in this state is fraught with danger.|
 |Thread Safe:    | No, API function, only allows one thread at a time.|
 |Returns:        |Non.|
 |Prototype:      |`int SL_Close( UINT nChanId ) /* I: Channel Id to close */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_SendData**|
 |Description:    |Transmit a packet of data to a given destination identified by it channel Id.|
 |Thread Safe:    | No, API function, only allows one thread at a time.|
 |Returns:        |R_OK     - Data sent successfully.<br>R_FAIL   - Couldnt send data, see Errno.|
 |<Errno>         |E_INVCHANID - Invalid channel Id.<br>E_BUSY      - Channel is busy, retry later.<br>E_BADSOCKET - Internal failure on socket, terminal.<br>E_NOSERVICE - No remote connection established yet.|
 |Prototype:      |`int SL_SendData( UINT nChanId /* I: Channel Id to send data on */, UCHAR *szData /* I: Data to be sent */, UINT nDataLen )    /* I: Length of data */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_BlockSendData**|
 |Description:    |Transmit a packet of data to a given destination but ensure it is sent prior to exit. If an error occurs, then return it to the caller. |
 |Thread Safe:    | No, API function, only allows one thread at a time.|
 |Returns:        |R_OK     - Data sent successfully.<br>R_FAIL   - Couldnt send data, see Errno.|
 |<Errno>         |E_INVCHANID - Invalid channel Id.<br>E_BADSOCKET - Internal failure on socket, terminal.<br>E_NOSERVICE - No remote connection established yet.|
 |Prototype:      |`int SL_BlockSendData( UINT nChanId /* I: Channel Id to send data on */, UCHAR   *szData /* I: Data to be sent */, UINT nDataLen )  /* I: Length of data */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_Poll**|
 |Description:    |Function for programs which cant afford UX taking control of the CPU. This function offers these type of applications the ability to allow comms processing by frequently calling this Poll function.|
 |Thread Safe:    | No, API function, only allows one thread at a time.|
 |Returns:        |R_OK    - System closing down.R_FAIL  - Catastrophe, see Errno.|
 |<Errno>         |  |
 |Prototype:      |`int SL_Poll( ULNG lSleepTime )`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SL_Kernel**|
 |Description:    |Application process control is passed over to this function and it allocates and manages time/events. The application registers callbacks with this library, and they are invoked as events occur or as time elapses. Control passes out of this function on application completion.|
 |Thread Safe:    | No, Assumes main thread or one control thread.|
 |Returns:        |R_OK    - System closing down.<br>R_FAIL  - Catastrophe, see Errno.|
 |<Errno>         |  |
 |Prototype:      |`int SL_Kernel( void )`|

### ux_lgr

General purpose standalone (programmable) logging utilities.

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**Lgr**|
 |Description:    |A function to log a message to a flatfile, database or both.|
 |Returns:        |Non.|
 |Prototype:      |`void Lgr( int nLevel /* I: Level of error message/or command */, ... ) /* I: Varargs */`|

### ux_linkl

A library of linked list functions for creating, deleting, searching (etc..) linked lists.

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**AddItem**|
 |Description:    |A simplistic mechanism to compose a linked list. The link is only singly linked, and items are only added to the tail of the list.|
 |Returns:        |R_OK      - Item added successfully.<br>R_FAIL    - Failure in addition, see Errno.|
 |<Errno>         |E_NOMEM   - Memory exhaustion.<br>E_BADHEAD - Head pointer is bad.<br>E_BADTAIL - Tail pointer is bad.<br>E_NOKEY   - No search key provided.|
 |Prototype:      |`int AddItem( LINKLIST **spHead /* IO: Pointer to head of list */, LINKLIST **spTail /* IO: Pointer to tail of list */, int nMode /* I: Mode of addition to link */, UINT *nKey /* I: Integer based search key */, ULNG *lKey /* I: Long based search key */, UCHAR *szKey /* I: String based search key */, void *spData ) /* I: Address of carried data */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**DelItem**|
 |Description:    |Delete an element from a given linked list. The underlying carried data is not freed, it is assumed that the caller will free that, as it was the caller that allocated it.|
 |Returns:        |R_OK      - Item deleted successfully.<br>R_FAIL    - Failure in deletion, see Errno.|
 |<Errno>         |E_BADHEAD - Head pointer is bad.<br>E_BADTAIL - Tail pointer is bad.<br>E_MEMFREE - Couldnt free memory to sys pool.<br>E_NOKEY   - No search key provided.|
 |Prototype:      |`int DelItem( LINKLIST **spHead /* IO: Pointer to head of list */, LINKLIST **spTail /* IO: Pointer to tail of list */, void *spKey /* I: Addr of item, direct update */, UINT *nKey /* I: Integer based search key */, ULNG *lKey /* I: Long based search key */, UCHAR *szKey ) /* I: String based search key */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**FindItem**|
 |Description:    |Find an element in a given linked list.|
 |Returns:        |NOTNULL    - Item found, address returned.<br>NULL       - Item not found, see Errno.|
 |<Errno>         |E_BADHEAD - Head pointer is bad.<br>E_BADTAIL - Tail pointer is bad.<br>E_NOKEY   - No search key provided.|
 |Prototype:      |`void *FindItem( LINKLIST *spHead /* I: Pointer to head of list */, UINT *nKey /* I: Integer based search key */, ULNG *lKey /* I: Long based search key */, UCHAR *szKey ) /* I: String based search key */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**StartItem**|
 |Description:    |Setup pointers for a complete list scan. Return the top most list 'data item' to caller.|
 |Returns:        |NOTNULL    - Item found, address returned.<br>NULL       - Item not found, see Errno.|
 |<Errno>         |E_BADHEAD - Head pointer is bad.|
 |Prototype:      |`void *StartItem( LINKLIST *spHead /* I: Pointer to head of list */, LINKLIST **spNext ) /* O: Pointer to next item in list */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**NextItem**|
 |Description:    |Move to next item in a given list. Return the current 'data item' to caller.|
 |Returns:        |NOTNULL    - Item found, address returned.<br>NULL       - Item not found, see Errno.|
 |<Errno>         |E_BADPARM - Bad parameter passed to function.|
 |Prototype:      |`void *NextItem( LINKLIST **spNext ) /* O: Pointer to next item in list */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MergeLists**|
 |Description:    |Merge two list together. The Source list is merged into the target list. Lists are re-sorted if required.|
 |Returns:        |R_OK      - Item added successfully.<br>R_FAIL    - Failure in addition, see Errno.|
 |<Errno>         |E_NOMEM   - Memory exhaustion.<br>E_BADHEAD - Head pointer is bad.<br>E_BADTAIL - Tail pointer is bad.<br>E_NOKEY   - No search key provided.|
 |Prototype:      |`int MergeLists( LINKLIST **spDstHead,   /* IO: Pointer to head of dest list */, LINKLIST **spDstTail /* IO: Pointer to tail of dest list */, LINKLIST *spSrcHead /* I: Pointer to head of src list */, LINKLIST *spSrcTail /* I: Pointer to tail of src list */, int nMode ) /* I: Mode of list merging  */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**DelList**|
 |Description:    |Delete an entire list and free memory used by the list and the underlying carried data.|
 |Returns:        |R_OK      - List deleted successfully.<br>R_FAIL    - Failed to delete list, see Errno.|
 |<Errno>         |E_BADHEAD - Head pointer is bad.<br>E_BADTAIL - Tail pointer is bad.|
 |Prototype:      |`int DelList( LINKLIST **spHead /* IO: Pointer to head of list */, LINKLIST **spTail ) /* IO: Pointer to tail of list */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SizeList**|
 |Description:    |Find the total number of elements in a given list by scanning it.|
 |Returns:        |R_OK      - List size calculated.<br>R_FAIL    - Failed to calculate list size, see Errno.|
 |<Errno>         |E_BADHEAD - Head pointer is bad.|
 |Prototype:      |`int SizeList( LINKLIST *spHead /* I: Pointer to head of list */, UINT *nCnt ) /* O: Count of elements in list */`|

### ux_mon

Interactive Monitor functionality. Provides a suite of interactive commands (HTML or Natural Language) that a user can issue to an executing application that incorporates these facilities.

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ML_HTMLDefaultCB**|
 |Description:    |A default handler for the HTML interpreter which basically sends a not-coded message to the client.|
 |Returns:        |Non.|
 |Prototype:      |`int _ML_HTMLDefaultCB( UINT nChanId /* I: Id - xmit to client*/, UCHAR *szData /* I: Data Buffer */, UINT nDataLen ) /* I: Length of data buf */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ML_InterpretHTMLRequest**|
 |Description:    |Buffer from an external client (ie. Web Browser) contains a an HTML request. Interpret it into a set of actions through comparisons and deductions.|
 |Returns:        |Non.|
 |Prototype:      |`UINT _ML_InterpretHTMLRequest( ML_MONLIST *spMon /* I: Monitor Desc */, ML_CONLIST *spCon /* I: Connection Desc */, UCHAR *szData )   /* I: Command Buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ML_InterpretNLRequest**|
 |Description:    |Buffer from an external client contains a natural language command request. Interpret it into a set of actions through comparisons.|
 |Returns:        |Non.|
 |Prototype:      |`UINT _ML_InterpretNLRequest( ML_MONLIST *spMon /* I: Monitor Desc */, ML_CONLIST *spCon /* I: Connection Desc */, UCHAR *szData )   /* I: Command Buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ML_MonitorCB**|
 |Description:    |When an external client is issuing commands to us, the command data is delivered to this function for processing.|
 |Returns:        |Non.|
 |Prototype:      |`void _ML_MonitorCB( UINT nChanId /* I: Channel data received on */, UCHAR *szData /* I: Buffer containing data */, UINT nDLen ) /* I: Length of data in buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ML_MonitorCntrl**|
 |Description:    |Interactive monitor control function. WHen a connection is mad or broken with an external client, this function is requested to handle it.|
 |Returns:        |Non.|
 |Prototype:      |`void _ML_MonitorCntrl( int nType /* I: Type of callback */, ... ) /* I: Argument list according to type */`|
    
 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ML_MonTerminate**|
 |Description:    |An interactive monitor command to terminate us (the program).|
 |Returns:        |Non.|
 |Prototype:      |`int _ML_MonTerminate( UINT nChanId /* I: Channel to Im session */, UCHAR *szBuf /* I: Remainder of input line */, UINT nBufLen ) /* I: Length of input line */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ML_Init**|
 |Description:    |Initialise all functionality to allow a remote user to connect with this executing program and issue commands to it.|
 |Returns:        |Non.|
 |Prototype:      |`int ML_Init( UINT nMonPort /* I: Port that monitor service is on */, UINT nServiceType /* I: Type of monitor service. ie HTML */, UCHAR *szServerName /* I: HTTP Server Name Response */, int (*nConnectCB)() /* I: CB on client connection */, int (*nDisconCB)() /* I: CB on client disconnection */, int (*nInterpretOvr)()) /* I: Builtin Interpret override func */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ML_Exit**|
 |Description:    |Decommission the Monitor module ready for program termination or re-initialisation.|
 |Returns:        |R_OK     - Exit succeeded.<br>R_FAIL   - Couldnt perform exit processing, see errno.|
 |<Errno>         |
 |Prototype:      |`int ML_Exit( UCHAR *szErrMsg ) /* O: Error message buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ML_Send**|
 |Description:    |Transmit data to a given remote session.|
 |Returns:        |Non.|
 |Prototype:      |`int    ML_Send( UINT nChanId /* I: Channel to Im session */, UCHAR *szBuf /* I: Xmit Data */, UINT nBufLen ) /* I: Length of Xmit Data */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ML_AddMonCommand**|
 |Description:    |Add a command to the interactive monitors database of recognised reserved words. When a command comes in from an external user, if checks it against its reserved word list for verification and identification.|
 |Returns:        |R_OK     - Command added.<br>R_FAIL   - Couldnt add command, see errno.|
 |<Errno>         |
 |Prototype:      |`int    ML_AddMonCommand( UINT nMonPort /* I: Service Mon Port */, UCHAR *szCommand /* I: Command in text */, int (*nCallback)()) /* I: Command callback */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ML_DelMonCommand**|
 |Description:    |Delete a command currently active in a monitor channels database of recognised words.|
 |Returns:        |R_OK     - Command deleted.<br>R_FAIL   - Couldnt delete command, see errno.|
 |<Errno>         |
 |Prototype:      |`int    ML_DelMonCommand( UINT nMonPort, /* I: Service Mon Port */, UCHAR *szCommand ) /* I: Command to delete */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ML_Broadcast**|
 |Description:    |Broadcast a message to all listening monitor processes.|
 |Returns:        |R_OK     - Data sent to some/all successfully.<br>R_FAIL   - Couldnt send to any, see Errno.|
 |<Errno>         |  |
 |Prototype:      |`int ML_Broadcast( UCHAR *szData /* I: Data to be sent */, UINT nDataLen ) /* I: Length of data */`|

### ux_str

General purpose string processing funtions. Additions to those which exist within the C libraries.

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**PutCharFromLong**|
 |Description:    |Place a long type variable into a character buffer in a known byte order. IE. A long is 32 bit, and is placed into the char buffer as MSB (3), 2, 1, LSB (0). Where MSB fits into the first byte of the buffer.|
 |Returns:        |R_OK   - Cannot fail ... will I be eating my words...?|
 |Prototype:      |`int PutCharFromLong( UCHAR *pDestBuf /* O: Destination buffer */, ULNG lVar ) /* I: Variable of type long */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**PutCharFromInt**|
 |Description:    |Place an int type variable into a character buffer in a known byte order. IE. An int is 16 bit, and is placed into the char buffer as MSB (1), LSB (0). Where MSB fits into the first byte of the buffer.|
 |Returns:        |R_OK   - Cannot fail ... see comment above.|
 |Prototype:      |`int PutCharFromInt( UCHAR *pDestBuf /* O: Destination buffer */, UINT lVar ) /* I: Variable of type int */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**GetLongFromChar**|
 |Description:    |Get a long type variable from a character buffer. The byte ordering in the buffer is assumed to be 32bit, MSB(3), 2, 1, LSB (0), where the MSB fits into the first byte of the buffer.|
 |Returns:        |R_OK   - Cannot fail ... will I be eating my words...?|
 |Prototype:      |`ULNG GetLongFromChar( UCHAR *pDestBuf ) /* I: Source buffer to convert */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**GetIntFromChar**|
 |Description:    |Get a long type variable from a character buffer. The byte ordering in the buffer is assumed to be 16bit, MSB(1), LSB(0) where the MSB fits into the first byte of the buffer.|
 |Returns:        |R_OK   - Cannot fail ... will I be eating my words...?|
 |Prototype:      |`UINT GetIntFromChar( UCHAR *pDestBuf ) /* I: Source buffer to convert */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**StrPut**|
 |Description:    |Put a string INTO another string. Same as strcpy BUT it doesnt terminate the destination string.|
 |Returns:        |R_OK   - Cannot fail ... will I be eating my words...?|
 |Prototype:      |`UINT StrPut( UCHAR *spDestBuf /* I: Destination buffer to copy into */, UCHAR *spSrcBuf  /* I: Source buffer to copy from */, UINT nBytes ) /* I: Number of bytes to copy */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**FFwdOverWhiteSpace**|
 |Description:    |Forward a pointer past whitespace in the input buffer.|
 |Returns:        |Non.|
 |Prototype:      |`void FFwdOverWhiteSpace( UCHAR *szInBuf /* I: Input data buffer */, UINT *nPos ) /* IO: Start/End position in buf */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ParseForToken**|
 |Description:    |Parse the input buffer for the next token. A token can be a Alpha/Alphanum word, a numeric, a single character or a string.|
 |Returns:        |Type of Token located.|
 |Prototype:      |`UINT ParseForToken( UCHAR *szInBuf /* I: Input buffer */, UINT *nPos /* IO: Current pos in buffer */, UCHAR *szTokBuf ) /* O: Token output buffer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ParseForString**|
 |Description:    |Get next valid string from input buffer.|
 |Returns:        |Non.|
 |Prototype:      |`int ParseForString( UCHAR *szInBuf /* I: Input buffer */, UINT *nPos /* I: Position in input buffer */, UCHAR *szOutBuf ) /* O: Target buffer for string */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ParseForInteger**|
 |Description:    |Get next valid integer from input buffer.|
 |Returns:        |Non.|
 |Prototype:      |`int ParseForInteger( UCHAR *szInBuf /* I: Input buffer */, UINT *nPos /* I: Position in input buffer */, UINT *nMin /* I: Minimum allowable value */, UINT *nMax /* I: Maximum allowable value */, int *pOutInt ) /* O: Target buffer for integer */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ParseForLong**|
 |Description:    |Get next valid Long from input buffer.|
 |Returns:        |Non.|
 |Prototype:      |`int ParseForLong( UCHAR *szInBuf /* I: Input buffer */, UINT *nPos /* I: Position in input buffer */, long *lMin /* I: Minimum allowable value */, long *lMax /* I: Maximum allowable value */, long *pOutLong )   /* O: Target buffer for Long */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**StrRTrim**|
 |Description:    |A function to trim off all trailing spaces for a given string. The function works by starting at the end of a null terminated string and looking for the first non-space character. It then places a null terminator at the new location.|
 |Returns:        |Pointer to new string.|
 |Prototype:      |`char *StrRTrim( char *szSrc ) /* IO: Base string to trim */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**StrCaseCmp**|
 |Description:    |A function to perform string compares regardless of character case. Provided mainly for operating systems that dont possess such functionality.|
 |Returns:        |0 - Strings compare.<br>> 0 <br>< 0|
 |Prototype:      |`int StrCaseCmp( const char *szSrc /* I: Base string to compare against */, const char *szCmp ) /* I: Comparator string */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**StrnCaseCmp**|
 |Description:    |A function to perform string compares regardless of character case for a specified number of characters within both strings. Provided mainly for operating systems that dont possess such functionality.|
 |Returns:        |0 - Strings compare.<br>> 0 <br>< 0|
 |Prototype:      |`int StrnCaseCmp( const char *szSrc /* I: Base string to compare against */, const char *szCmp /* I: Comparator string */, size_t nCount ) /* I: Number of bytes to compare */`|

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**SplitFQFN**|
 |Description:    |A function to split a fully qualified filename into a directory and filename components.|
 |Returns:        |R_OK   - Filename split.<br>R_FAIL - Couldnt split due to errors, ie. memory.|
 |Prototype:      |`int SplitFQFN( char *szFQFN /* I: Fully Qualified File Name */, char **szDir /* O: Directory component */, char **szFN ) /* O: Filename component */`|

### Example UX test program

This example can be found in the repository in the ux_test folder.

````c
/******************************************************************************
 * Product:    ####### #######  #####  #######         #     # ####### #     #
 *                #    #       #     #    #            ##   ## #     # ##    #
 *                #    #       #          #            # # # # #     # # #   #
 *                #    #####    #####     #            #  #  # #     # #  #  #
 *                #    #             #    #            #     # #     # #   # #
 *                #    #       #     #    #            #     # #     # #    ##
 *                #    #######  #####     #    ####### #     # ####### #     #
 *
 * File:          test_mon.c
 * Description:   A Test Harness program specifically for testing out ux
 *                monitor functionality. Ie the ability to add an interactive
 *                monitor to any application.
 *
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1996-2019.
 *
 * History:       1.0  - Initial Release.
 *
 ******************************************************************************
 * This source file is free software: you can redistribute it and#or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This source file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

/* Bring in system header files.
*/
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <stdarg.h>
#include    <string.h>

/* Bring in UX header files.
*/
#include    <ux.h>

/* Specials for Solaris.
*/
#if defined(SOLARIS) || defined(LINUX) || defined(ZPU)
#include    <sys/types.h>
#endif

/* Indicate that we are a C module for any header specifics.
*/
#define     TEST_MON_C

/* Bring in local specific header files.
*/
#include    "test_mon.h"

/******************************************************************************
 * Function:    _HTML_GetHandler
 * Description: Call back to override default HTML GET handler. This function
 *              works out what the client browser requires and tries to 
 *              satisfy it.
 * 
 * Returns:     R_OK    - Configuration obtained.
 *              R_FAIL  - Failure, see error message.
 ******************************************************************************/
int    _HTML_GetHandler( UINT    nChanId,    /* I: Channel Id of new con */
                         UCHAR   *szData,    /* I: Data received by WWW */
                         UINT    nMonPort )  /* I: Monitor Port Number */
{
    /* Local variables.
    */
    int         nCnt=0;
    int         nChar;
    UINT        nPos = 0;
    UINT        nTokType;
    UCHAR       *spEndStr;
    UCHAR       *spMsgBuf = NULL;
    UCHAR       szFileName[MAX_FILENAMELEN+1];
    char        *szFunc = "_HTML_GetHandler";
    FILE        *fp;

    /* Log a debug message to indicate details of this connection.
    */
    Lgr(LOG_DEBUG, szFunc,
        "GET Handler called: Data=%s, nChanId=%d, MonPort=%d\n",
        szData, nChanId, nMonPort);

    /* Setup HTML content type.
    */
    ML_Send(nChanId, "Content-type: text/html\n\n", 0);

    /* Scan the buffer for the recognised browser end of stream:
     * HTTP/version. If it doesnt exist, send an error message to client
     * and exit.
    */
    if( (spMsgBuf=(UCHAR *)malloc((strlen(szData)*3)+1)) == NULL )
    {
        /* Get out with a failure, memory exhausted.
        */
        ML_Send(nChanId, "<HTML><TITLE>Out of Memory</TITLE>"
                "Out of Memory, Re-Try Later</HTML>/n/n", 0);
        if(spMsgBuf != NULL) free(spMsgBuf);
        return(R_FAIL);
    }
    if((spEndStr=strstr(szData, "HTTP")) != NULL)
    {
        /* Command we are required to understand lies between the beginning
         * of the buffer and where HTTP starts, extract it.
        */
        FFwdOverWhiteSpace(szData, &nPos);
        strncpy(spMsgBuf, &szData[nPos], ((spEndStr - szData)-nPos));
        spMsgBuf[(spEndStr - szData)-nPos] = '\0';
    } else
     {
        /* Dispatch an error message to the client as their is not much
         * we can do.
        */
        ML_Send(nChanId, "<HTML><TITLE>Illegal HTML</TITLE>"
                "The HTML that your browser issued is illegal, or it is from"
                " a newer version not supported by this product</HTML>\n\n", 0);
        return(R_FAIL);
    }

    /* Trim off the fat and open file.
    */
    strcpy(szFileName, StrRTrim(spMsgBuf));
    if((fp=fopen(szFileName, "r")) == NULL)
    {
        sprintf(spMsgBuf, "<HTML><TITLE>Cannot access %s</TITLE>"
                "<H1>File Not Available</H1>\n"
                "The file requested (%s) cannot be accessed.</HTML>\n\n",
                szFileName, szFileName);
        ML_Send(nChanId, spMsgBuf, 0);
        return(R_FAIL);
    } else
     {
        /* Crude but effective, read 1 byte at a time and fire it off,
         * wrapped in a HTML structure.
        */
        ML_Send(nChanId, "<HTML>\n<BODY><PRE>\n", 0);
        spMsgBuf[1]='\0';
        while((nChar=fgetc(fp)) != EOF)
        {
        nCnt++;
            spMsgBuf[0]=nChar;
            ML_Send(nChanId, spMsgBuf, 1);
        }
        ML_Send(nChanId, "</PRE></BODY></HTML>", 0);
        fclose(fp);
    }

    /* All done, get out!
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    _HTML_ConnectCB
 * Description: Call back for when an incoming WWW browser makes a connection
 *              with us.
 * 
 * Returns:     R_OK    - Configuration obtained.
 *              R_FAIL  - Failure, see error message.
 ******************************************************************************/
int    _HTML_ConnectCB( UINT    nChanId,      /* I: Channel Id of new con */
                        UINT    nMonPort )    /* I: Monitor Port Number */
{
    /* Local variables.
    */
    char        *szFunc = "_HTML_ConnectCB";

    /* Log a debug message to indicate details of this connection.
    */
    Lgr(LOG_DEBUG, szFunc, "New Connection: ChanID=%d, MonPort=%d\n",
        nChanId, nMonPort);

    /* All done, get out!
    */
    return;
}

/******************************************************************************
 * Function:    _HTML_DisconnectCB
 * Description: Call back for when an existing WWW browser connection ceases
 *              to exist.
 * 
 * Returns:     R_OK    - Configuration obtained.
 *              R_FAIL  - Failure, see error message.
 ******************************************************************************/
int    _HTML_DisconnectCB( UINT    nChanId,      /* I: Channel Id of new con */
                           UINT    nMonPort )    /* I: Monitor Port Number */
{
    /* Local variables.
    */
    char        *szFunc = "_HTML_DisconnectCB";

    /* Log a debug message to indicate details of this connection.
    */
    Lgr(LOG_DEBUG, szFunc, "Connection Closed: ChanID=%d, MonPort=%d\n",
        nChanId, nMonPort);

    /* All done, get out!
    */
    return;
}

/******************************************************************************
 * Function:    _NL_HelpHandler
 * Description: Call back to implement a HELP feature in the natural
 *              language command interface. This command basically lists
 *              global or specific help according to the arguments of the
 *              command and fires it back to the client.
 * 
 * Returns:     R_OK    - Configuration obtained.
 *              R_FAIL  - Failure, see error message.
 ******************************************************************************/
int    _NL_HelpHandler( UINT    nChanId,    /* I: Channel Id of new con */
                        UCHAR   *szData,    /* I: Data received by WWW */
                        UINT    nMonPort )  /* I: Monitor Port Number */
{
    /* Local variables.
    */
    char        *szFunc = "_NL_HelpHandler";

    /* Log a debug message to indicate details of this connection.
    */
    Lgr(LOG_DEBUG, szFunc,
        "Help Handler called: Data=%s, nChanId=%d, MonPort=%d\n",
        szData, nChanId, nMonPort);

    /* All done, get out!
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    _NL_ConnectCB
 * Description: Call back for when an incoming WWW browser makes a connection
 *              with us.
 * 
 * Returns:     R_OK    - Configuration obtained.
 *              R_FAIL  - Failure, see error message.
 ******************************************************************************/
int    _NL_ConnectCB( UINT    nChanId,      /* I: Channel Id of new con */
                      UINT    nMonPort )    /* I: Monitor Port Number */
{
    /* Local variables.
    */
    char        *szFunc = "_NL_ConnectCB";

    /* Log a debug message to indicate details of this connection.
    */
    Lgr(LOG_DEBUG, szFunc, "New Connection: ChanID=%d, MonPort=%d\n",
        nChanId, nMonPort);

    /* All done, get out!
    */
    return;
}

/******************************************************************************
 * Function:    _NL_DisconnectCB
 * Description: Call back for when an existing WWW browser connection ceases
 *              to exist.
 * 
 * Returns:     R_OK      - Configuration obtained.
 *              R_FAIL    - Failure, see error message.
 ******************************************************************************/
int    _NL_DisconnectCB( UINT    nChanId,      /* I: Channel Id of new con */
                         UINT    nMonPort )    /* I: Monitor Port Number */
{
    /* Local variables.
    */
    char        *szFunc = "_NL_DisconnectCB";

    /* Log a debug message to indicate details of this connection.
    */
    Lgr(LOG_DEBUG, szFunc, "Connection Closed: ChanID=%d, MonPort=%d\n",
        nChanId, nMonPort);

    /* All done, get out!
    */
    return;
}

/******************************************************************************
 * Function:    GetConfig
 * Description: Get configuration information from the OS or command line
 *              flags.
 * 
 * Returns:     R_OK      - Configuration obtained.
 *              R_FAIL    - Failure, see error message.
 ******************************************************************************/
int    GetConfig( int      argc,          /* I: CLI argument count */
                  UCHAR    **argv,        /* I: CLI argument contents */
                  char     **envp,        /* I: Environment variables */
                  UCHAR    *szErrMsg )    /* O: Any generated error message */
{
    /* Local variables.
    */
    int      nReturn = R_OK;
    FILE     *fp;
    UCHAR    *szFunc = "GetConfig";

    /* See if the user wishes to use a logfile?
    */
    if( GetCLIParam(argc, argv, FLG_LOGFILE, T_STR, TMON.szLogFile,
                    MAX_LOGFILELEN, FALSE) == R_OK )
    {
        /* Check to see if the filename is valid.
        */
        if((fp=fopen(TMON.szLogFile, "a")) == NULL)
        {
            sprintf(szErrMsg, "Cannot write to logfile (%s)", TMON.szLogFile);
            return(R_FAIL);
        }

        /* Close the file as test complete.
        */
        fclose(fp);
    } else
     {
        /* Set logfile to a default, dependant on OS.
        */
        strcpy(TMON.szLogFile, DEF_LOGFILE);
    }

    /* Get log mode from command line.
    */
    if(GetCLIParam(argc, argv, FLG_LOGMODE, T_INT, (UCHAR *)&TMON.nLogMode,
                   0, 0) == R_OK)
    {
        /* Check the validity of the mode.
        */
        if((TMON.nLogMode < LOG_OFF || TMON.nLogMode > LOG_FATAL) &&
            TMON.nLogMode != LOG_CONFIG)
        {
            sprintf(szErrMsg, "Illegal Logger mode (%d)", TMON.nLogMode);
            return(R_FAIL);
        }
    } else
     {
        /* Setup default log mode.
        */
        TMON.nLogMode = LOG_DEBUG;
    }

    /* Get the port to be used for HTML monitoring.
    */
    if(GetCLIParam(argc, argv, FLG_HTMLPORT, T_INT, (UCHAR *)&TMON.nHtmlPort,
                   0, 0) == R_OK)
    {
        /* Check the validity of the port.
        */
        if((TMON.nHtmlPort < 2000 || TMON.nHtmlPort > 10000))
        {
            sprintf(szErrMsg, "Illegal HTML TCP Port (%d)", TMON.nHtmlPort);
            return(R_FAIL);
        }
    } else
     {
        /* Setup default port.
        */
        TMON.nHtmlPort = DEF_HTML_PORT;
    }

    /* Get the port to be used for HTML monitoring.
    */
    if(GetCLIParam(argc, argv, FLG_NLPORT, T_INT, (UCHAR *)&TMON.nNLPort,
                   0, 0) == R_OK)
    {
        /* Check the validity of the port.
        */
        if((TMON.nNLPort < 2000 || TMON.nNLPort > 10000))
        {
            sprintf(szErrMsg, "Illegal Natural Language TCP Port (%d)",
                    TMON.nNLPort);
            return(R_FAIL);
        }
    } else
     {
        /* Setup default port.
        */
        TMON.nNLPort = DEF_NL_PORT;
    }

    /* Finished, get out!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    TMONInit
 * Description: Initialisation of variables, functionality, communications etc.
 * 
 * Returns:     VDWD_OK      - Initialised successfully.
 *              VDWD_FAIL    - Failure, see error message.
 ******************************************************************************/
int    TMONInit( UCHAR        *szErrMsg )    /* O: Generated error message */
{
    /* Local variables.
    */
    char        *szFunc = "TMONInit";

    /* Setup logger mode.
    */
    Lgr(LOG_CONFIG, LGM_FLATFILE, TMON.nLogMode, TMON.szLogFile);

    /* Initialise Socket Library.
    */    
    if(SL_Init(TMON_SRV_KEEPALIVE, (UCHAR *)NULL) != R_OK)
    {
        sprintf(szErrMsg, "SL_Init failed");
        Lgr(LOG_DEBUG, szFunc, szErrMsg); 
        return(R_FAIL);
    }

    /* Initialise Monitor Library for HTML servicing.
    */
    if(ML_Init(TMON.nHtmlPort, MON_SERVICE_HTML, "Test Monitor Program (HTML)", 
               _HTML_ConnectCB, _HTML_DisconnectCB, NULL) == R_FAIL)
    {
        sprintf(szErrMsg, "ML_Init failed for HTML service");
        Lgr(LOG_DEBUG, szFunc, szErrMsg);
        return(R_FAIL);
    }

    /* Initialise Monitor Library for Natural Language servicing.
    */
    if(ML_Init(TMON.nNLPort, MON_SERVICE_NL, "Test Monitor Program (NL)", 
               _NL_ConnectCB, _NL_DisconnectCB, NULL) == R_FAIL)
    {
        sprintf(szErrMsg, "ML_Init failed for NL service");
        Lgr(LOG_DEBUG, szFunc, szErrMsg);
        return(R_FAIL);
    }

    /* Add test commands for HTML.
    */
    ML_AddMonCommand(TMON.nHtmlPort, MC_HTMLGET, _HTML_GetHandler);

    /* Add test commands for Natural Language.
    */
    ML_AddMonCommand(TMON.nNLPort, MC_NLHELP, _NL_HelpHandler);

    /* All done, lets get out.
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    TMONClose
 * Description: Function to perform closure of all used resources within the
 *              program.
 * 
 * Returns:     R_OK    - Closed successfully.
 *              R_FAIL  - Failure, see error message.
 ******************************************************************************/
int    TMONClose( UCHAR        *szErrMsg )    /* O: Generated error message */
{
    /* Local variables.
    */
    char        *szFunc = "TMONClose";

    /* Call monitor library to close and tidy up.
    */
    if(ML_Exit(NULL) == R_FAIL)
    {
        Lgr(LOG_DEBUG, szFunc, "Failed to close Monitor Library");
    }

    /* Exit with success.
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    main
 * Description: Entry point into the Monitor facility Test program. Basic
 *              purpose is to invoke intialisation, enter the main program
 *              loop and finally tidy up and close down.
 * 
 * Returns:     0     - Program completed successfully without errors.
 *              -1    - Program terminated with errors, see logged message.
 ******************************************************************************/
int    main( int     argc,       /* I: Count of available arguments */
             char    **argv,     /* I: Array of arguments */
             char    **envp )    /* I: Array of environment parameters */
{
    /* Local variables.
    */
    UCHAR        szErrMsg[MAX_ERRMSG_LEN];
    UCHAR        *szFunc = "main";

    /* Bring in any configuration parameters passed on the command line etc.
    */
    if( GetConfig(argc, (UCHAR **)argv, envp, szErrMsg) == R_FAIL )
    {
        printf( "%s\n"
                "Usage:                 %s <parameters>\n"
                "<parameters>:          -l<LogFile Name>\n"
                "                       -m<Logging Mode>\n"
                "                       -html_port<TCP Port No>\n"
                "                       -nl_port<TCP Port No>\n",
                szErrMsg, argv[0]);
    }

    /* Initialise variables, communications etc.
    */
    if( TMONInit(szErrMsg) == R_FAIL )
    {
        /* Log an error message to indicate reason for failure.
        */
        Lgr(LOG_DIRECT, szFunc, "%s: %s", argv[0], szErrMsg);
        exit(-1);
    }

    /* Do nothing basically, where testing monitor functionality, so just loop.
    */
    while(TRUE)
    {
        SL_Poll(DEF_POLLTIME);
    }

    /* Perform close down of used facilities ready for exit.
    */
    if( TMONClose(szErrMsg) == R_FAIL )
    {
        /* Log an error message to indicate reason for failure.
        */
        Lgr(LOG_DIRECT, szFunc, "%s: %s", argv[0], szErrMsg);
        exit(-1);
    }

    /* All done, go bye bye's.
    */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
    exit;
#endif
#if defined(_WIN32)
    return(0);
#endif
}
````

<br>


# SDD Library

The Server Data-source Driver library is a set of API's for communicating with a data source. Currently drivers have been written for:
  - Audio Playback - not technically a data source, more a data target but the SDD library is bi-directional in nature.
  - FTP - allows data to be transmitted and received from a remote source via the File Transfer Protocol.
  - Java - allows a java program to be run on a remote source to provide and receive data. Useful when a data source is not standard.
  - ODBC - allows connection to any database supported by the Open Database Connectivity drivers.
  - SCMD - System Commands, allows a System (Linux, Solaris, Windows) command to be run on a remote server to extract or receive data.
  - SYBC - Sybase database, allows native connection to a Sybase database for data storage and extraction.

Additonal drivers can be written using the templates in the templates/ directory.

The drivers are opened via the MDC layer or directly in an application wishing to use a given data source (ie. an admin application issuing commands to a remote server via the SCMD driver - an early version of Ansible!).

The methods in the SDD Library are described below ordered by the driver to which they belong. If a method begins with '_' then it is internal and normally not called directly, albeit being C there is no Private definition to methods or their data so you can call them if it helps.

### Audio driver sdd_aupl

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_AUPL_GetStrArg**|
 |Description:    |Function to scan an input buffer and extract a string based argument. |
 |Returns:        |SDD_FAIL- Couldnt obtain argument.<br>SDD_OK    - Argument obtained. |
 |Prototype:      |`int _AUPL_GetStrArg( UCHAR *snzDataBuf /* I: Input buffer */, int nDataLen /* I: Len of data */, UCHAR *szArg /* I: Arg to look for */, UCHAR **pszPath ) /* O: Pointer to argument */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_AUPL_ValidatePath**|
 |Description:    |Function to validate the existence of a path. |
 |Returns:        |SDD_FAIL- Couldnt validate PATH.<br>SDD_OK    - PATH validated. |
 |Prototype:      |`int _AUPL_ValidatePath( UCHAR *pszPath ) /* I: Path to validate */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_AUPL_ValidateFile**|
 |Description:    |Function to validate the existence of a file or to validate that a file can be created. |
 |Returns:        |SDD_FAIL- Couldnt obtain Filename or validate it.<br>SDD_OK    - Filename obtained and validated. |
 |Prototype:      |`int _AUPL_ValidateFile( UCHAR *pszPath /* I: Path to file */, UCHAR *pszFile /* I: File to validate */, UINT nWriteFlag ) /* I: Read = 0, Write = 1 */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_AUPL_PlayZ**|
 |Description:    |Function to play a compressed audio file. Method of attach is to launch a child which is the actual decompressor, this feeds data back via the stdout of the child to our stdin. The data is then buffered in a round robin fashion and fed to the audio DSP hardware. |
 |Returns:        |SDD_FAIL- Command failed during execution.<br>SDD_OK    - Command executed successfully. |
 |Prototype:      |`int _AUPL_PlayZ( UCHAR *pszAudioPath /* I: Path to Audio File */, UCHAR *pszAudioFile /* I: Audio Filename */, int (*fSendDataCB)(UCHAR *, UINT) /* I: Func for returning data */, UCHAR *szErrMsg ) /* O: Error message generated */`jjjjj |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**aupl_InitService**|
 |Description:    |Entry point which initialises the driver into a defined state. It is mandatory that this function is called before any other in order for the driver to function correctly. The caller provides it with two types of data, 1) A structure containing data for it to use in initialising itself, 2) a pointer to a buffer which the driver uses to place an error message should it not be able to complete initialisation. |
 |Returns:        |SDD_FAIL- An error occurred in initialising the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver initialised successfully. |
 |Prototype:      |`int aupl_InitService( SERVICEDETAILS *sServiceDet /* I: Init data */, UCHAR *szErrStr ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**aupl_CloseService**|
 |Description:    |Entry point which performs a drive closedown. The closedown procedure ensure that the driver returns to a virgin state (ie.like at power up) so that InitService can be called again. |
 |Returns:        |SDD_FAIL- An error occurred in closing the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver successfully closed. |
 |Prototype:      |`int aupl_CloseService( UCHAR *szErrMsg ) /* O: Error message if failed */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**aupl_ProcessRequest**|
 |Description:    |Entry point into driver to initiate the driver into processing a request. A data block is passed as a parameter to the driver which represents a request with relevant parameters. The data within the structure is only relevant to the original client and this driver code. |
 |Returns:        |SDD_FAIL- An error occurred within the driver whilst trying to process the request, see error text.<br>SDD_OK    - Request processed successfully. |
 |Prototype:      |`int aupl_ProcessRequest( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply*/, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**aupl_ProcessOOB**|
 |Description:    |Entry point into driver to process an out of band command that may or may not be relevant to current state of operation. The task of this function is to decipher the command and act on it immediately, ie. a cancel command would abort any ProcessRequest that is in process and clean up. |
 |Returns:        |No returns. |
 |Prototype:      |`void aupl_ProcessOOB( UCHAR nCommand ) /* I: OOB Command */` |

### FTP Protocol driver sdd_ftpx

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_GetStrArg**|
 |Description:    |Function to scan an input buffer and extract a string based argument. |
 |Returns:        |SDD_FAIL- Couldnt obtain argument.<br>SDD_OK    - Argument obtained. |
 |Prototype:      |`int _FTPX_GetStrArg( UCHAR *snzDataBuf /* I: Input buffer */, int nDataLen /* I: Len of data */, UCHAR *szArg /* I: Arg to look for */, UCHAR **pszPath ) /* O: Pointer to argument */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_GetMode**|
 |Description:    |Function to scan an input buffer and determine the mode of FTP operation that the caller requires (Binary or Ascii). If no mode is provided then default to binary. |
 |Returns:        |Mode Flag - 1 = Binary mode selected.<br>- 0 = Ascii mode selected. |
 |Prototype:      |`int _FTPX_GetMode( UCHAR *snzDataBuf /* I: Input buffer */, int nDataLen ) /* I: Len of data */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_GetWriteData**|
 |Description:    |Function to scan an input buffer, verify that it has data in it, extract the data and store in the opened file stream and set the start flag if the block is the final block. |
 |Returns:        |SDD_FAIL- Bad block of data or error writing to file.<br>SDD_OK    - Block obtained and stored. |
 |Prototype:      |`int _FTPX_GetWriteData( UCHAR *snzDataBuf /* I: Input buffer */, int nDataLen /* I: Len of data */, FILE *fpFile /* IO: Opened file stream */, int *nLast /* O: Last block flag */, UCHAR *szErrMsg ) /* O: Any resultant error msg */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_PutReadData**|
 |Description:    |Function to read an open stream and transmit the data contents to the caller via the callback mechanism.  |
 |Returns:        |SDD_FAIL- Couldnt obtain PATH.<br>SDD_OK    - PATH obtained. |
 |Prototype:      |`int _FTPX_PutReadData( FILE *fpFile /* I: Stream to read from */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send data to */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_PIDataCB**|
 |Description:    |Function to handle any control information passed back from the FTP server. |
 |Returns:        |No returns. |
 |Prototype:      |`void _FTPX_PIDataCB( UINT nChanId /* I: Channel data arrived on */, UCHAR *szData /* I: Actual data */, UINT nDataLen ) /* I: Length of data */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_PICtrlCB**|
 |Description:    |Function to handle any control callbacks during connectivity with the FTP server. |
 |Returns:        |No returns. |
 |Prototype:      |`void _FTPX_PICtrlCB( int nType /* I: Type of callback */, ... ) /* I: Var args */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_DTPDataCB**|
 |Description:    |Function to handle any data passed back from the FTP server on the data transfer connection.. |
 |Returns:        |No returns. |
 |Prototype:      |`void _FTPX_DTPDataCB( UINT nChanId /* I: Channel data arrived on */, UCHAR *szData /* I: Actual data */, UINT nDataLen )  /* I: Length of data */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_DTPCtrlCB**|
 |Description:    |Function to handle any control callbacks on the Data Transfer connection with the FTP server. |
 |Returns:        |No returns. |
 |Prototype:      |`void _FTPX_DTPCtrlCB( int nType /* I: Type of callback */, ... ) /* I: Var args */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_PIGetResponse**|
 |Description:    |Function to get a response code from the FTP server. |
 |Returns:        |Response Code. |
 |Prototype:      |`int _FTPX_PIGetResponse( void )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_PISendCmd**|
 |Description:    |Function to send a command to an FTP server. |
 |Returns:        |SDD_FAIL - FTP Server failed to respond, see szErrMsg.<br>SDD_OK   - Command sent successfully. |
 |Prototype:      |`int _FTPX_PISendCmd( UCHAR *szCmd /* I: Command to send */, UINT *panReqResponses /* I: Array of req resp */, UCHAR *szErrMsg ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_PIGetDTPResponse**|
 |Description:    |Function to get a response code during a DTP transfer. |
 |Returns:        |Response Code. |
 |Prototype:      |`int _FTPX_PIGetDTPResponse( void )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_PISendDTPCmd**|
 |Description:    |Function to send a command to an FTP server which will invoke a DTP channel for data transfer. |
 |Returns:        |SDD_FAIL - FTP Server failed to respond, see szErrMsg.<br>SDD_OK   - Command sent successfully. |
 |Prototype:      |`int _FTPX_PISendDTPCmd( UCHAR *szCmd /* I: Command to send */, UINT *panReqResponses /* I: Allowed responses */, UCHAR *szErrMsg ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_SetMode**|
 |Description:    |Function to setup the transfer mode between the FTP server and the driver. |
 |Returns:        |SDD_FAIL - Failed to set transfer mode, critical error.<br>SDD_OK     - Mode set. |
 |Prototype:      |`int _FTPX_SetMode( UINT nBinaryMode /* I: Select binary mode = TRUE */, UCHAR *szErrMsg ) /* O: Generated error messages */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_SetCwd**|
 |Description:    |Function to set the FTP servers current working directory. |
 |Returns:        |SDD_FAIL - Failed to set directory to that specified.<br>SDD_OK     - Current Working Directory set. |
 |Prototype:      |`int _FTPX_SetCwd( UCHAR *szPath /* I: Path to set CWD */, UCHAR *szErrMsg ) /* O: Generated error messages */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_FTPInit**|
 |Description:    |Function to initialise a connection with an FTP server. The caller provides the name/IP address of the server and the user name/password to complete the connection. |
 |Returns:        |SDD_FAIL - Couldnt make connection with given details.<br>SDD_OK     - FTP connection made. |
 |Prototype:      |`int _FTPX_FTPInit( UCHAR *szFTPServer /* I: Name of FTP server */, UCHAR *szUserName /* I: User name to login with */, UCHAR *szPassword /* I: Password for login */, UCHAR *szErrMsg ) /* O: Error message if failed */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_FTPClose**|
 |Description:    |Function to close a connected FTP connection and tidy up in preparation for next task. |
 |Returns:        |SDD_FAIL - Failed to close properly, library wont work again.<br>SDD_OK     - Closed. |
 |Prototype:      |`int _FTPX_FTPClose( UCHAR *szErrMsg ) /* O: Generated error messages */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_FTPRenFile**|
 |Description:    |Function to rename a file on a remote FTP server. |
 |Returns:        |SDD_FAIL - Failed to rename the required file.<br>SDD_OK     - File renamed successfully. |
 |Prototype:      |`int _FTPX_FTPRenFile( UCHAR *szPath /* I: Path to remote file */, UCHAR *szSrcFile /* I: Original remote file name */, UCHAR *szDstFile /* I: New remote file name */, UCHAR *szErrMsg ) /* O: Generated error messages */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_FTPRcvFile**|
 |Description:    |Function to initiate a file transfer from the FTP server to the current machine file system. |
 |Returns:        |SDD_FAIL - Failed to complete file transfer.<br>SDD_OK     - File received successfully. |
 |Prototype:      |`int _FTPX_FTPRcvFile( UCHAR *szRcvFile /* I: Name of file to store in */, UCHAR *szPath /* I: Path to remote file */, UCHAR *szFile /* I: Remote file */, UINT nBinaryMode /* I: Select binary transfer mode */, UCHAR *szErrMsg ) /* O: Generated error messages */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_FTPX_FTPXmitFile**|
 |Description:    |Function to initiate a file transfer from the current machine file system to the FTP server. |
 |Returns:        |SDD_FAIL - Failed to complete file transfer.<br>SDD_OK     - File transmitted successfully. |
 |Prototype:      |`int _FTPX_FTPXmitFile( UCHAR *szXmitFile /* I: Name of file to transmit */, UCHAR *szPath /* I: Path to remote destination */, UCHAR *szFile /* I: Remote file */, UINT nBinaryMode /* I: Select binary transfer Mode */, UCHAR *szErrMsg ) /* O: Generated error messages */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ftpx_InitService**|
 |Description:    |Entry point which initialises the driver into a defined state. It is mandatory that this function is called before any other in order for the driver to function correctly. The caller provides it with two types of data, 1) A structure containing data for it to use in initialising itself, 2) a pointer to a buffer which the driver uses to place an error message should it not be able to complete initialisation. |
 |Returns:        |SDD_FAIL- An error occurred in initialising the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver initialised successfully. |
 |Prototype:      |`int ftpx_InitService( SERVICEDETAILS *sServiceDet /* I: Init data */, UCHAR *szErrStr ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ftpx_CloseService**|
 |Description:    |Entry point which performs a drive closedown. The closedown procedure ensure that the driver returns to a virgin state (ie.like at power up) so that InitService can be called again. |
 |Returns:        |SDD_FAIL- An error occurred in closing the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver successfully closed. |
 |Prototype:      |`int ftpx_CloseService( UCHAR *szErrMsg ) /* O: Error message if failed */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ftpx_ProcessRequest**|
 |Description:    |Entry point into driver to initiate the driver into processing a request. A data block is passed as a parameter to the driver which represents a request with relevant parameters. The data within the structure is only relevant to the original client and this driver code. |
 |Returns:        |SDD_FAIL- An error occurred within the driver whilst trying to process the request, see error text.<br>SDD_OK    - Request processed successfully. |
 |Prototype:      |`int ftpx_ProcessRequest( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply*/, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**ftpx_ProcessOOB**|
 |Description:    |Entry point into driver to process an out of band command that may or may not be relevant to current state of operation. The task of this function is to decipher the command and act on it immediately, ie. a cancel command would abort any ProcessRequest that is in process and clean up. |
 |Returns:        |No returns. |
 |Prototype:      |`void ftpx_ProcessOOB( UCHAR nCommand ) /* I: OOB Command */` |

### Java driver sdd_java

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**java_InitService**|
 |Description:    |Entry point which initialises the driver into a defined state. It is mandatory that this function is called before any other in order for the driver to function correctly. The caller provides it with two types of data, 1) A structure containing data for it to use in initialising itself, 2) a pointer to a buffer which the driver uses to place an error message should it not be able to complete initialisation. |
 |Returns:        |SDD_FAIL- An error occurred in initialising the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver initialised successfully. |
 |Prototype:      |`int java_InitService( SERVICEDETAILS *sServiceDet /* I: Init data */, UCHAR *szErrStr ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**java_CloseService**|
 |Description:    |Entry point which performs a drive closedown. The closedown procedure ensure that the driver returns to a virgin state (ie.like at power up) so that InitService can be called again. |
 |Returns:        |SDD_FAIL- An error occurred in closing the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver successfully closed. |
 |Prototype:      |`int java_CloseService( UCHAR *szErrMsg ) /* O: Error message if failed */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**java_ProcessRequest**|
 |Description:    |Entry point into driver to initiate the driver into processing a request. A data block is passed as a parameter to the driver which represents a request with relevant parameters. The data within the structure is only relevant to the original client and this driver code. |
 |Returns:        |SDD_FAIL- An error occurred within the driver whilst trying to process the request, see error text.<br>SDD_OK    - Request processed successfully. |
 |Prototype:      |`int java_ProcessRequest( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply*/, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**java_ProcessOOB**|
 |Description:    |Entry point into driver to process an out of band command that may or may not be relevant to current state of operation. The task of this function is to decipher the command and act on it immediately, ie. a cancel command would abort any ProcessRequest that is in process and clean up. |
 |Returns:        |No returns. |
 |Prototype:      |`void java_ProcessOOB( UCHAR nCommand ) /* I: OOB Command */` |

### ODBC driver sdd_odbc

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ODBC_GetArg**|
 |Description:    |Function to scan an input buffer and extract a required argument from it. |
 |Returns:        |SDD_FAIL- Couldnt obtain argument.<br>SDD_OK    - Argument obtained and validated. |
 |Prototype:      |`int _ODBC_GetArg( UCHAR *szArgType /* I: Type of Arg to scan for */, UCHAR *snzDataBuf /* I: Input buffer */, int nDataLen /* I: Len of data */, UCHAR **pszArg ) /* O: Pointer to Arg */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ODBC_LogODBCError**|
 |Description:    |Function to dump an error message/code from the ODBC driver to the log device. Typically used for debugging. |
 |Returns:        |No returns. |
 |Prototype:      |`void _ODBC_LogODBCError( HSTMT hStmt )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ODBC_RunSql**|
 |Description:    |Function to execute a given buffer of SQL on the current database and return resultant data to the original caller. |
 |Returns:        |SDD_FAIL- SQL execution failed, see error message.<br>SDD_OK    - SQL execution succeeded. |
 |Prototype:      |`int _ODBC_RunSql( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ODBC_ListDB**|
 |Description:    |Function to list all the names of databases available on the currently open data source. |
 |Returns:        |SDD_FAIL- SQL execution failed, see error message.<br>SDD_OK    - SQL execution succeeded. |
 |Prototype:      |`int _ODBC_ListDB( int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ODBC_ListTables**|
 |Description:    |Function to list all names of tables in a given database (or current database if no database name given). |
 |Returns:        |SDD_FAIL- SQL execution failed, see error message.<br>SDD_OK    - SQL execution succeeded. |
 |Prototype:      |`int _ODBC_ListTables( UCHAR *snzDataBuf  /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_ODBC_ListCols**|
 |Description:    |Function to list all names and attributes of columns in a given table in a given database (or current database/table if no database name given). |
 |Returns:        |SDD_FAIL- SQL execution failed, see error message.<br>SDD_OK    - SQL execution succeeded. |
 |Prototype:      |`int _ODBC_ListCols( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**odbc_InitService**|
 |Description:    |Entry point which initialises the driver into a defined state. It is mandatory that this function is called before any other in order for the driver to function correctly. The caller provides it with two types of data, 1) A structure containing data for it to use in initialising itself, 2) a pointer to a buffer which the driver uses to place an error message should it not be able to complete initialisation. |
 |Returns:        |SDD_FAIL- An error occurred in initialising the driver and an error message is stored in szErrMsg.<br>SDD_OK    - Driver initialised successfully. |
 |Prototype:      |`int odbc_InitService( SERVICEDETAILS *sServiceDet /* I: Init data */, UCHAR *szErrMsg ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**odbc_CloseService**|
 |Description:    |Entry point which performs a drive closedown. The closedown procedure ensure that the driver returns to a virgin state (ie.like at power up) so that InitService can be called again. |
 |Returns:        |SDD_FAIL- An error occurred in closing the driver and an error message is stored in szErrMsg.<br>SDD_OK    - Driver successfully closed. |
 |Prototype:      |`int odbc_CloseService( UCHAR *szErrMsg ) /* O: Error message if failed */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**odbc_ProcessRequest**|
 |Description:    |Entry point into driver to initiate the driver into processing a request. A data block is passed as a parameter to the driver which represents a request with relevant parameters. The data within the structure is only relevant to the original client and this driver code. |
 |Returns:        |SDD_FAIL- An error occurred within the driver whilst trying to process the request, see error text.<br>SDD_OK    - Request processed successfully. |
 |Prototype:      |`int odbc_ProcessRequest( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply*/, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**odbc_ProcessOOB**|
 |Description:    |Entry point into driver to process an out of band command that may or may not be relevant to current state of operation. The task of this function is to decipher the command and act on it immediately, ie. a cancel command would abort any ProcessRequest that is in process and clean up. |
 |Returns:        |No returns. |
 |Prototype:      |`void odbc_ProcessOOB( UCHAR nCommand ) /* I: OOB Command */` |

### SCMD driver sdd_scmd

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_GetStrArg**|
 |Description:    |Function to scan an input buffer and extract a string based argument. |
 |Returns:        |SDD_FAIL- Couldnt obtain argument.<br>SDD_OK    - Argument obtained. |
 |Prototype:      |`int _SCMD_GetStrArg( UCHAR *snzDataBuf /* I: Input buffer */, int nDataLen /* I: Len of data */, UCHAR *szArg /* I: Arg to look for */, UCHAR **pszPath ) /* O: Pointer to argument */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_ValidatePath**|
 |Description:    |Function to validate the existence of a path. |
 |Returns:        |SDD_FAIL- Couldnt validate PATH.<br>SDD_OK    - PATH validated. |
 |Prototype:      |`int _SCMD_ValidatePath( UCHAR *pszPath ) /* I: Path to validate */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_ValidateFile**|
 |Description:    |Function to validate the existence of a file or to validate that a file can be created. |
 |Returns:        |SDD_FAIL- Couldnt obtain Filename or validate it.<br>SDD_OK    - Filename obtained and validated. |
 |Prototype:      |`int _SCMD_ValidateFile( UCHAR *pszPath /* I: Path to file */, UCHAR *pszFile /* I: File to validate */, UINT nWriteFlag ) /* I: Read = 0, Write = 1 */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_ValidateTime**|
 |Description:    |Function to validate a time value given as an ascii string. |
 |Returns:        |SDD_FAIL- Couldnt obtain a TIME or validate it.<br>SDD_OK    - TIME obtained and validated. |
 |Prototype:      |`int _SCMD_ValidateTime( UCHAR *pszTime /* I: Time to verify */, ULNG *lTime ) /* O: Time in seconds */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_Exec**|
 |Description:    |Function to execute a given command via a fork and exec, attaching the parent to the childs I/O so that any data output by the child can be captured by the parent and fed back to the caller. |
 |Returns:        |SDD_FAIL- Command failed during execution.<br>SDD_OK    - Command executed successfully. |
 |Prototype:      |`int _SCMD_Exec( int nTimedExec /* I: Is this a timed exec (T/F)? */, UCHAR *pszPath /* I: Path to command */, UCHAR *pszCmd /* I: Command name */, UCHAR *pszArgs /* I: Arguments to command */, ULNG lTimeToExec /* I: Time to execution */, int (*fSendDataCB)(UCHAR *, UINT) /* I: Func for returning data */, UCHAR *szErrMsg ) /* O: Error message generated */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_GetWriteData**|
 |Description:    |Function to scan an input buffer, verify that it has data in it, extract the data and store in the opened file stream and set the start flag if the block is the final block. |
 |Returns:        |SDD_FAIL- Bad block of data or error writing to file.<br>SDD_OK    - Block obtained and stored. |
 |Prototype:      |`int _SCMD_GetWriteData( UCHAR *snzDataBuf /* I: Input buffer */, int nDataLen /* I: Len of data */, FILE *fpFile /* IO: Opened file stream */, int *nLast /* O: Last block flag */, UCHAR *szErrMsg ) /* O: Any resultant error msg */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_PutReadData**|
 |Description:    |Function to read an open stream and transmit the data contents to the caller via the callback mechanism.  |
 |Returns:        |SDD_FAIL- Couldnt obtain PATH.<br>SDD_OK    - PATH obtained. |
 |Prototype:      |`int _SCMD_PutReadData( FILE *fpFile /* I: Stream to read from */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send data to */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_MoveFile**|
 |Description:    |Function to move a file from one location/name to another. This is performed as a copy and unlink operation because the underlying may not support moves across file systems. |
 |Returns:        |SDD_FAIL- An error whilst moving file, see szErrMsg.<br>SDD_OK    - File moved successfully. |
 |Prototype:      |`int _SCMD_MoveFile( UCHAR *pszSrcPath /* I: Path to source file */, UCHAR *pszSrcFile /* I: Source File */, UCHAR *pszDstPath /* I: Path to dest file */, UCHAR *pszDstFile /* I: Dest File */, UCHAR *szErrMsg ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SCMD_DeleteFile**|
 |Description:    |Function to delete a file from the given path. |
 |Returns:        |SDD_FAIL- An error whilst moving file, see szErrMsg.<br>SDD_OK    - File moved successfully. |
 |Prototype:      |`int _SCMD_DeleteFile( UCHAR *pszDelPath /* I: Path to file */, UCHAR *pszDelFile /* I: File to delete */, UCHAR *szErrMsg ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**scmd_InitService**|
 |Description:    |Entry point which initialises the driver into a defined state. It is mandatory that this function is called before any other in order for the driver to function correctly. The caller provides it with two types of data, 1) A structure containing data for it to use in initialising itself, 2) a pointer to a buffer which the driver uses to place an error message should it not be able to complete initialisation. |
 |Returns:        |SDD_FAIL- An error occurred in initialising the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver initialised successfully. |
 |Prototype:      |`int scmd_InitService( SERVICEDETAILS *sServiceDet /* I: Init data */, UCHAR *szErrStr ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**scmd_CloseService**|
 |Description:    |Entry point which performs a drive closedown. The closedown procedure ensure that the driver returns to a virgin state (ie.like at power up) so that InitService can be called again. |
 |Returns:        |SDD_FAIL- An error occurred in closing the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver successfully closed. |
 |Prototype:      |`int scmd_CloseService( UCHAR *szErrMsg ) /* O: Error message if failed */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**scmd_ProcessRequest**|
 |Description:    |Entry point into driver to initiate the driver into processing a request. A data block is passed as a parameter to the driver which represents a request with relevant parameters. The data within the structure is only relevant to the original client and this driver code. |
 |Returns:        |SDD_FAIL- An error occurred within the driver whilst trying to process the request, see error text.<br>SDD_OK    - Request processed successfully. |
 |Prototype:      |`int scmd_ProcessRequest( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply*/, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**scmd_ProcessOOB**|
 |Description:    |Entry point into driver to process an out of band command that may or may not be relevant to current state of operation. The task of this function is to decipher the command and act on it immediately, ie. a cancel command would abort any ProcessRequest that is in process and clean up. |
 |Returns:        |No returns. |
 |Prototype:      |`void scmd_ProcessOOB( UCHAR nCommand ) /* I: OOB Command */` |

### Sybase driver sdd_sybc

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SYBC_GetArg**|
 |Description:    |Function to scan an input buffer and extract a required argument from it. |
 |Returns:        |SDD_FAIL- Couldnt obtain argument.<br>SDD_OK    - Argument obtained and validated. |
 |Prototype:      |`int _SYBC_GetArg( UCHAR *szArgType /* I: Type of Arg to scan for */, UCHAR *snzDataBuf /* I: Input buffer */, int nDataLen /* I: Len of data */, UCHAR **pszArg ) /* O: Pointer to Arg */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SYBC_RunSql**|
 |Description:    |Function to execute a given buffer of SQL on the current database and return resultant data to the original caller. |
 |Returns:        |SDD_FAIL- SQL execution failed, see error message.<br>SDD_OK    - SQL execution succeeded. |
 |Prototype:      |`int _SYBC_RunSql( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**sybc_InitService**|
 |Description:    |Entry point which initialises the driver into a defined state. It is mandatory that this function is called before any other in order for the driver to function correctly. The caller provides it with two types of data, 1) A structure containing data for it to use in initialising itself, 2) a pointer to a buffer which the driver uses to place an error message should it not be able to complete initialisation. |
 |Returns:        |SDD_FAIL- An error occurred in initialising the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver initialised successfully. |
 |Prototype:      |`int sybc_InitService( SERVICEDETAILS *sServiceDet /* I: Init data */, UCHAR *szErrStr ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SYBC_ListDB**|
 |Description:    |Function to list all the names of databases available on the currently open data source. |
 |Returns:        |SDD_FAIL- SQL execution failed, see error message.<br>SDD_OK    - SQL execution succeeded. |
 |Prototype:      |`int _SYBC_ListDB( int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SYBC_ListTables**|
 |Description:    |Function to list all names of tables in a given database (or current database if no database name given). |
 |Returns:        |SDD_FAIL- SQL execution failed, see error message.<br>SDD_OK    - SQL execution succeeded. |
 |Prototype:      |`int _SYBC_ListTables( UCHAR *snzDataBuf  /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_SYBC_ListCols**|
 |Description:    |Function to list all names and attributes of columns in a given table in a given database (or current database/table if no database name given). |
 |Returns:        |SDD_FAIL- SQL execution failed, see error message.<br>SDD_OK    - SQL execution succeeded. |
 |Prototype:      |`int _SYBC_ListCols( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**sybc_CloseService**|
 |Description:    |Entry point which performs a drive closedown. The closedown procedure ensure that the driver returns to a virgin state (ie.like at power up) so that InitService can be called again. |
 |Returns:        |SDD_FAIL- An error occurred in closing the driver and an error message is stored in szErrStr.<br>SDD_OK    - Driver successfully closed. |
 |Prototype:      |`int    sybc_CloseService( UCHAR        *szErrMsg )    /* O: Error message if failed */ |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**sybc_ProcessRequest**|
 |Description:    |Entry point into driver to initiate the driver into processing a request. A data block is passed as a parameter to the driver which represents a request with relevant parameters. The data within the structure is only relevant to the original client and this driver code. |
 |Returns:        |SDD_FAIL- An error occurred within the driver whilst trying to process the request, see error text.<br>SDD_OK    - Request processed successfully. |
 |Prototype:      |`int sybc_ProcessRequest( UCHAR *snzDataBuf /* I: Input data */, int nDataLen /* I: Len of data */, int (*fSendDataCB)(UCHAR *, UINT) /* I: CB to send reply */, UCHAR *szErrMsg ) /* O: Error text */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**sybc_ProcessOOB**|
 |Description:    |Entry point into driver to process an out of band command that may or may not be relevant to current state of operation. The task of this function is to decipher the command and act on it immediately, ie. a cancel command would abort any ProcessRequest that is in process and clean up. |
 |Returns:        |No returns. |
 |Prototype:      |`void sybc_ProcessOOB( UCHAR nCommand ) /* I: OOB Command */` |

<br>

# VDW Library

The Virtual Data Warehouse Library was a set of API's to build a Virtual Data Warehouse. Typically these methods are used to build a set of cross platform (Linux, SunOS, Solaris, Windows) daemons each sited on a server with a data source. The daemon would have a set of SDD drivers built in and would open the required data sources and communicate with an application or the MDC layer and serve accordingly.
ie. A SQL Server on Network A, a Sybase Server on Network B, an FTP source on Network C, a VDW daemon would be placed on a server in all the 3 networks and open connections with the data source. Via routing, an MDC application sat on Network D would be able to query all the sources as needed.
At the moment I can see no use for the VDW library in the ZPU Evo but it may be useful if your trying to create a network of decentralized data sources.

The methods in the VDW Library are as follows. If a method begins with '_' then it is internal and normally not called directly, albeit being C there is no Private definition to methods or their data so you can call them if it helps.

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**GetConfig**|
 |Description:    |Get configuration information from the OS or command line flags. |
 |Returns:        |VDWD_OK        - Configuration obtained.<br>VDWD_FAIL    - Failure, see error message. |
 |Prototype:      |`int GetConfig( int argc /* I: CLI argument count */.  UCHAR **argv /* I: CLI argument contents */, char **envp /* I: Environment variables */, UCHAR *szErrMsg ) /* O: Any generated error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**VDWDInit**|
 |Description:    |Initialisation of variables, functionality, communications and turning the process into a daemon. |
 |Returns:        |VDWD_OK        - Initialised successfully.<br>VDWD_FAIL    - Failure, see error message. |
 |Prototype:      |`int VDWDInit( UCHAR *szErrMsg ) /* O: Generated error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**VDWDClose**|
 |Description:    |Function to perform closure of all used resources within the module. |
 |Returns:        |VDWD_OK        - Closed successfully.<br>VDWD_FAIL    - Failure, see error message. |
 |Prototype:      |`int VDWDClose( UCHAR *szErrMsg ) /* O: Generated error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**VDWDSentToClient**|
 |Description:    |Function to send data from this daemon back to the relevant client. |
 |Returns:        |SDD_OK        - Data sent successfully.<br>SDD_FAIL    - Failure in sending data. |
 |Prototype:      |`int VDWDSendToClient( UCHAR *snzData /* I: Data to send */, UINT nDataLen ) /* I: Length of data */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**VDWDInitService**|
 |Description:    |Function to call a given drivers initialisation function. |
 |Returns:        |VDWD_OK        - Service was initialised successfully.<br>VDWD_FAIL    - Failure, see error message. |
 |Prototype:      |`int VDWDInitService( int nServiceType /* I: Type of service*/, SERVICEDETAILS *sServiceDet /* I: Service Data */, UCHAR *szErrMsg ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**VDWDCloseService**|
 |Description:    |Function to call a given drivers closedown function. |
 |Returns:        |VDWD_OK        - Service was closed successfully.<br>VDWD_FAIL    - Failure, see error message. |
 |Prototype:      |`int VDWDCloseService( int nServiceType /* I: Type of service*/, UCHAR *szErrMsg )    /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**VDWDProcessRequest**|
 |Description:    |Function to call a given drivers function to process a service request. |
 |Returns:        |VDWD_OK        - Request was processed successfully.<br>VDWD_FAIL    - Failure, see error message. |
 |Prototype:      |`int VDWDProcessRequest( int nServiceType /* I: Type of service */, UCHAR *snzData /* I: Data Buffer */, UINT nDataLen /* I: Len of Data */, UCHAR *szErrMsg ) /* O: Error message */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**VDWDDataCallback**|
 |Description:    |Function which is registered as a callback and is called every time data arrives from a new client. |
 |Returns:        |MDC_OK        - Closed successfully.<br>MDC_FAIL    - Failure, see error message. |
 |Prototype:      |`int VDWDDataCallback( UCHAR *snzData /* I: Buffer containing data */, int nDataLen /* I: Length of data in buffer */, UCHAR *szErrMsg )  /* O: Error messages generated */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**VDWDOOBCallback**|
 |Description:    |Function to take action on out of band commands from the MDC layer. Out of band messages are generally commands which need to be actioned upon immediately, so they are passed up into the Drivers out of band processing function.  |
 |Returns:        |No returns. |
 |Prototype:      |`void VDWDOOBCallback( UCHAR cCmd ) /* I: Command to action upon */` |

<br>

# MDC Library

The Meta Data Communications API was part of my Virtual Data Warehouse design which sought to encapsulate disparate systems and data sources and through the MDC API's provide a central uniform way of querying them, ie. Sybase, SQL Server, FTP CSV, Flat File etc. The application would make a normalized request, irrespective of data source, to the MDC layer and the MDC would take care of targetting the correct source and any needed transalations.
At the moment I can see no use for it in the ZPU Evo but it may be useful if your trying to join disparate data sources using the VDW daemons.

The methods in the MDC Library are described below ordered by the functionality to which they belong. If a method begins with '_' then it is internal and normally not called directly, albeit being C there is no Private definition to methods or their data so you can call them if it helps.

### MDC Server API

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_SendACK** |
 |Description:    |Function to send an acknowledge to the client in response to a data block received correctly or a request processed successfully. |
 |Returns:        |MDC_FAIL- Couldnt transmit an ACK message to the client.<br>MDC_OK    - ACK sent successfully. |
 |Prototype:      |`int _MDC_SendACK( void )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_SendNAK** |
 |Description:    |Function to send a negative acknowledge to the client in to a data block which arrived incorrectly or a request which couldnt be processed successfully. |
 |Returns:        |MDC_FAIL- Couldnt transmit a NAK message to the client.<br>MDC_OK    - NAK sent successfully. |
 |Prototype:      |`int _MDC_SendNAK( UCHAR szErrMsg /* I: Error msg to send with NAK */ )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_ServerCntlCB** |
 |Description:    |A function to handle any communications control callbacks that are generated as a result of MDC_Server being executed.  |
 |Returns:        |No Returns. |
 |Prototype:      | `void _MDC_ServerCntlCB( int nType /* I: Type of callback */, ...  /* I: Arg list according to type */` ) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_ServerDataCB** |
 |Description:    |A function to handle any data callbacks that are generated as a result of data arriving during an MDC_Server execution. |
 |Returns:        |No Returns. |
 |Prototype:      |`void _MDC_ServerDataCB( UINT nChanId / I: Channel data rcv on */, UCHAR szData /* I: Rcvd data */, UINT nDataLen /* I: Rcvd data length */)` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_Server** |
 |Description:    |Entry point into the Meta Data Communications for a server process. This function initialises all communications etc and then runs the given user callback to perform any required actions. |
 |Returns:        |MDC_FAIL- Function terminated due to a critical error, see Errno for exact reason code.<br>MDC_OK    - Function completed successfully without error. |
 |Prototype:      |`int MDC_Server( UINT nPortNo /* I: TCP/IP port number */, UCHAR szService /* I: Name of TCP/IP Service */, int (fLinkDataCB) /* I: User function callback */, (UCHAR , int, UCHAR ), void (fControlCB)(UCHAR)  /* I: User control callback */ )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_ReturnData** |
 |Description:    |Function called by user code to return any required data to a connected client. This function can only be called in response to a callback 'from the MDC layer to the user code' which has provided data.  |
 |Returns:        |MDC_FAIL- An error occurred in transmitting the given data block to the client process, see Errno for exact reason code.<br> MDC_OK    - Data packet was transmitted successfully. |
 |Prototype:      |`int MDC_ReturnData( UCHAR snzDataBuf /* I: Data to return */, int nDataLen /* I: Length of data */ )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_TimerCB** |
 |Description:    |Function to allow user code to register a callback event which is activated upon a timer expiring. The user provides the frequency and a function to callback and the MDC schedules it. |
 |Returns:        |No Return Values. |
 |Prototype:      |`int MDC_TimerCB( ULNG lTimePeriod /* I: CB Time in Millseconds */, UINT nEnable /* I: Enable(TRUE)/Dis(FALSE) CB */, UINT nAstable /* I: Astable(TRUE) or Mono CB */, void (fTimerCB)(void) /* I: Function to call back */ )` |

### MDC Client API

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_DataCB** |
 |Description:    |This function is called back when data is received on any of the service connections |
 |Returns:        |    void |
 |Prototype:      |`void _MDC_DataCB(UINT nChanId, UCHAR *szData, UINT nDataLen) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_CtrlCB** |
 |Description:    |This function is called back when control information is received on any of the service connections |
 |Returns:        |    void |
 |Prototype:      |`void _MDC_CtrlCB(int nType, ...) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_SendPacket** |
 |Description:    |Send a packet to a daemon |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_SendPacket(UINT nChanId, char cPacketType, UCHAR *psnzBuf, UINT nBuflen) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_GetSrvRqtReply** |
 |Description:    |Get reply to a service request packet |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_GetSrvRqtReply(UINT nChanId, char *pcPacketType) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_CreateChStatus** |
 |Description:    |Make a new Channel status structure and add to linked list |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_CreateChStatus(UINT nChanId)         /* Channel ID */ |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_DelChStatus** |
 |Description:    |Delete an item from the Channel status linked list |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_DelChStatus(UINT nChanId)         /* Channel ID */ |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_SetChState** |
 |Description:    |Set Channel State to the given value |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_SetChState( UINT nChanId /* Channel ID */, CHSTATE eNewState  /* New state  */ ) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_SetSRResult** |
 |Description:    |Set Channel Send Request result |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_SetSRResult( UINT nChanId /* Channel ID */, UINT bResult /* Send Request result */) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_GetSRResult** |
 |Description:    |Get Channel Send Request result |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_GetSRResult( UINT nChanId /* Channel ID */, UINT *bResult /* Send Request result */) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_SetUserCB** |
 |Description:    |Set Send Request call back to the given value |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_SetUserCB( UINT nChanId /* Channel ID */, void (*UserCB) (UINT, UCHAR *, UINT) /* call back */) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_GetChState** |
 |Description:    |Get Channel State for a given channel |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_GetChState( UINT nChanId /* Channel ID */, CHSTATE *eState /* Channel state  */) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_GetNAKErrStr** |
 |Description:    |Get pointer to NAK error string |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_GetNAKErrStr( UINT nChanId /* Channel ID */, UCHAR **ppszErrStr /* pointer to pointer to error string */) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_GetUserCB** |
 |Description:    |Get User call back for the channel |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_GetUserCB( UINT nChanId /* Channel ID */, void (**UserCB) (UINT, UCHAR *, UINT /* User call back */) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_PrintErrMsg** |
 |Description:    |Print error message text |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_PrintErrMsg( UCHAR *psnzErrMsg /* Error message none terminated */, UINT nBufLen /* Buffer Length  */) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_WaitOnSndReq** |
 |Description:    |Block until send request completes |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int _MDC_WaitOnSndReq(UINT nChanId) /* Channel ID  */  |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_SendRequest** |
 |Description:    |Send a request to a driver |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int MDC_SendRequest( UINT nChanId  /* I: Channel to send message on */,  UCHAR *szData  /* I: Data to send */, UINT nDataLen /* I: Length of data */, void (*DataCB) (UINT, UCHAR *, UINT) /* I: call back function for data */ ) |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_GetResult** |
 |Description:    |Wait for all replies to a send request and then return result |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int MDC_GetResult( UINT nChanId /* I: Channel ID */, UCHAR **ppszErrorMsg  /* O: Associated error message */)` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_GetStatus** |
 |Description:    |Returns a boolean that indicates whether the Send Request for given channel has completed. |
 |Returns:        |    MDC_OK or MDC_FAIL |
 |Prototype:      |`int MDC_GetStatus( UINT nChanId /* I: Channel ID */, UINT *bSndReqCom  /* O: Indicates whether Send Request has completed */ )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_CreateService** |
 |Description:    |Create a connection to a daemon so that service requests can be issued |
 |Returns:        |    Channel ID, or negative error code |
 |Prototype:      |`int MDC_CreateService( UCHAR *szHostName /* I: Host for connect*/, UINT *nPortNo /* I: Port host on */, SERVICEDETAILS *serviceDet /* I: Service details */ )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_SetTimeout** |
 |Description:    |Function to program one of the system timeout values from the default to a user setting. |
 |Returns:        |    MDC_OK     -    Setting changed.<br>MDC_FAIL -    Setting couldnt be changed, see error message. |
 |Prototype:      |`int MDC_SetTimeout( UCHAR *pszWhichTimeout /* I: Timeout to set */, UINT nTimeoutValue /* I: New value */, UCHAR *pszErrMsg /* O: Error message */ )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_CloseService** |
 |Description:    |Close a service channel |
 |Returns:        |    MDC_FAIL or MDC_OK or MDC_BADCONTEXT |
 |Prototype:      |`int MDC_CloseService( UINT nChanId )    /* I: Channel to close */` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_Start** |
 |Description:    |This is called to initialise the MDC Comms. |
 |Returns:        |    MDC_FAIL or MDC_OK |
 |Prototype:      |`int MDC_Start( void )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_End** |
 |Description:    |This is called to shutdown the MDC Comms. |
 |Returns:        |    MDC_FAIL or MDC_OK |
 |Prototype:      |`int MDC_End( void )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**MDC_ChangeService** |
 |Description:    |Change Service for an existing daemon connection |
 |Returns:        |    MDC_OK, MDC_FAIL, MDC_NODAEMON, MDC_NOSERVICE, MDC_BADPARMS |
 |Prototype:      |`int MDC_ChangeService( UINT  nChanId /* I: Chan ID of srvc */, SERVICEDETAILS  *serviceDet /* I: Service details */ )` |

### MDC Common API

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_Init** |
 |Description:    |Function to perform MDC initialisation. The library performs checking to ensure that initialisation only occurs once prior to an _MDC_Terminate. |
 |Returns:        |MDC_FAIL- Couldnt initialise library.<br> MDC_OK    - Library initialised. |
 |Prototype:      |`int    _MDC_Init( void  )` |

 |                |                                                                               |
 | ----------     | ----------------------------------------------------------------------------- |
 |**Function**:   |**_MDC_Terminate** |
 |Description:    |Function to shutdown the MDC library. After a successful shutdown, _MDC_Init may be called to re-initialise the library. If MDC_Terminate fails, program exit is advised. |
 |Returns:        |MDC_FAIL- Couldnt perform a clean shutdown.<br>MDC_OK    - Library successfully shutdown. |
 |Prototype:      |`int    _MDC_Terminate( void )` |

 <br>


## Licenses

These software libraries are licensed under the GNU Public Licence v3.

### The Gnu Public License v3
 The source and binary files in this project marked as GPL v3 are free software: you can redistribute it and-or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

 The source files are distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program.  If not, see http://www.gnu.org/licenses/.
