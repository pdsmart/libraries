/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_comms.h
 * Description:   Generic communications routines.
 *
 * Version:       %I%    
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1994-2019
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

/* Ensure file is only included once - avoid compile loops.
*/
#ifndef    UX_COMMS_H
#define    UX_COMMS_H

#define    SL_SINGLE_THREAD_ONLY
#define    SL_THREAD_ONLY



/* Windows comms result values are different to unix, so define them
 * here.
*/
#if defined(_WIN32)
#define    EWOULDBLOCK           WSAEWOULDBLOCK
#define    EINVAL                WSAEINVAL
#define    EINPROGRESS           WSAEINPROGRESS
#define    EALREADY              WSAEALREADY
#define    ENOTSOCK              WSAENOTSOCK
#define    EDESTADDRREQ          WSAEDESTADDRREQ
#define    EMSGSIZE              WSAEMSGSIZE
#define    EPROTOTYPE            WSAEPROTOTYPE
#define    ENOPROTOOPT           WSAENOPROTOOPT
#define    EPROTONOSUPPORT       WSAEPROTONOSUPPORT
#define    ESOCKTNOSUPPORT       WSAESOCKTNOSUPPORT
#define    EOPNOTSUPP            WSAEOPNOTSUPP
#define    EPFNOSUPPORT          WSAEPFNOSUPPORT
#define    EAFNOSUPPORT          WSAEAFNOSUPPORT
#define    EADDRINUSE            WSAEADDRINUSE
#define    EADDRNOTAVAIL         WSAEADDRNOTAVAIL
#define    ENETDOWN              WSAENETDOWN
#define    ENETUNREACH           WSAENETUNREACH
#define    ENETRESET             WSAENETRESET
#define    ECONNABORTED          WSAECONNABORTED
#define    ECONNRESET            WSAECONNRESET
#define    ENOBUFS               WSAENOBUFS
#define    EISCONN               WSAEISCONN
#define    ENOTCONN              WSAENOTCONN
#define    ESHUTDOWN             WSAESHUTDOWN
#define    ETOOMANYREFS          WSAETOOMANYREFS
#define    ETIMEDOUT             WSAETIMEDOUT
#define    ECONNREFUSED          WSAECONNREFUSED
#define    ELOOP                 WSAELOOP
#define    EHOSTDOWN             WSAEHOSTDOWN
#define    EHOSTUNREACH          WSAEHOSTUNREACH
#define    EPROCLIM              WSAEPROCLIM
#define    EUSERS                WSAEUSERS
#define    EDQUOT                WSAEDQUOT
#define    ESTALE                WSAESTALE
#define    EREMOTE               WSAEREMOTE

/* Getdtablesize is not implemented in windows, so we set a fixed value
 * for the RLIMIT_NOFILE equivalent.
*/
#define    MAX_WIN_RLIMIT        256
#endif

/* Defaults
*/
#define    DEF_CHANID            1000    /* Starting internal comms chan Id */
#define    DEF_BUFINCSIZE        65536   /* Default comms buffer increment */
#define    DEF_INITRECVBUF       524288  /* Default size of comms receive buffer */
#define    DEF_MAXBLOCKPERIOD    10000   /* Default max select sleep period in mS */
#define    DEF_CONWAITPER        50      /* Default wait period for reconnect */
#define    DEF_CONFAILPER        30000   /* Default wait period for a fail */

/* Communications framing characters.
*/
#define    A_STX                 0x02    /* Start of Text */
#define    A_ETX                 0x03    /* End of Text */
#define    A_SYN                 0x22    /* Synchronise */

/* Timer callback option flags. 
*/
#define    TCB_OFF               0       /* Disable callback */
#define    TCB_FLIPFLOP          1       /* Callback toggles, not uniform in time */
#define    TCB_ASTABLE           2       /* Callback toggles, uniform in time */
#define    TCB_ONESHOT           3       /* Callback occurs only once */

/* Timer callback status flags.
*/
#define    TCB_UP                128     /* Callback is active */
#define    TCB_DOWN              129     /* Callback is in-active */

/* Socket/Line status flags.
*/
#define    SSL_UP                128     /* Socket/Line is up */
#define    SSL_DOWN              129     /* Socket/Line is down */
#define    SSL_LISTENING         130     /* Socket is listening for connections */
#define    SSL_FAIL              131     /* Socket/Line failure */

/* Connection type flags.
*/
#define    STP_SERVER            'S'     /* Connection is a server */
#define    STP_CLIENT            'C'     /* Connection is a client */

/* Communication callback control types. These are typically used when a
 * socket, acting as a server port, receives a connection, or a client 
 * eventually connects to a server.
*/
#define    SLC_NEWSERVICE         1      /* New service connection event */
#define    SLC_CONNECT            2      /* New client connect event */
#define    SLC_LINKDOWN           3      /* A Client link has gone down */
#define    SLC_LINKFAIL           4      /* A link has failed and been removed */

/* Operating system specifics.
*/
#define    SocketClose    close

/* Structure to hold 16 bit CRC lookup values. For speed, the CRC algorithm
 * has been constructed using lookup tables.
*/
typedef struct {
    UCHAR    cHiCRC;                     /* Hi Byte of CRC */
    UCHAR    cLoCRC;                     /* Lo Byte of CRC */
} SL_CRCTAB;

/* A structure to define and hold a timed callback event. A timed callback
 * event is the invocation of a function after a certain period of time. The
 * invocation can be single, multiple etc.
*/
typedef struct {
    void    (*nCallback)();              /* Function to call when timer expired */
    UINT    nOptions;                    /* Options controlling callback */
    UINT    nStatus;                     /* Status. Active or Inactive */
    ULNG    lTimeExpire;                 /* Time when callback is triggered */
    ULNG    lTimePeriod;                 /* Period inbetween triggers */
    ULNG    lCBData;                     /* Callback specific data */
} SL_CALLIST;

/* A structure to define and maintain a connection, either server of client
 * with its opposite on another process.
*/
typedef struct {
    UINT    nChanId;                     /* Internal channel Id associated with link */
    UINT    nClose;                      /* Flag for sync socket closure */
    UINT    nForkForAccept;              /* Fork a child prior to every accept on srv port */
    UINT    nOurPortNo;                  /* Port number where using */
    UINT    nRawMode;                    /* No prepackaging and post packaging of data */
    UINT    nRecvLen;                    /* Current number of bytes in receive buffer */
    UINT    nRecvBufLen;                 /* Current size of receive buffer */
    UINT    nServerPortNo;               /* Port number of server service */
    UINT    nStatus;                     /* Status of link */
    UINT    nXmitLen;                    /* Total length of data in xmit buffer */
    UINT    nXmitPos;                    /* Pos in buffer for xmission */
    int     nSd;                         /* Socket descriptor */
    ULNG    lDownTimer;                  /* Amount of time a downed connection remains idle*/
    ULNG    lServerIPaddr;               /* IP address of server */
    UCHAR   cCorS;                       /* (C) or (S)erver */
    UCHAR   *spRecvBuf;                  /* Flat, dynamic expand/shrink receive buffer */
    UCHAR   *spXmitBuf;                  /* Singular xmit buffer, to contain 1 packet */
    UCHAR   szServerName[MAX_SERVERNAME+1];/* Name of server */
    void    (*nDataCallback)();          /* Function to call with data */
    void    (*nCntrlCallback)(int, ...); /* Function to call with out-of-band info */
} SL_NETCONS;

/* Global variables for the Comms module.
*/
typedef struct {
    LINKLIST    *spHead;                 /* Head of LinkedList containing connections */
    LINKLIST    *spTail;                 /* Tail ... */
    LINKLIST    *spCBHead;               /* Head of LinkedList containing timer callbacks */
    LINKLIST    *spCBTail;               /* Tail ... */
    UINT        nCloseDown;              /* Shutdown in progress flag */
    UINT        nSockKeepAlive;          /* Time to keep socket alive */
} SL_GLOBALS;

/* Prototypes for functions internal to SocketLib module.
*/
UINT    _SL_CalcCRC( UCHAR *, UINT );
UINT    _SL_CheckCRC( UCHAR *, UINT );
int     _SL_FdBlocking( int, int );
UINT    _SL_GetPortNo( SL_NETCONS    * );
int     _SL_AcceptClient( UINT, SL_NETCONS *, SL_NETCONS ** );
int     _SL_Close( SL_NETCONS *, UINT );
int     _SL_ConnectToServer( SL_NETCONS * );
int     _SL_ReceiveFromSocket( SL_NETCONS * );
int     _SL_ProcessRecvBuf( SL_NETCONS * );
int     _SL_ProcessWaitingPorts( ULNG );
ULNG    _SL_ProcessCallbacks( void );

/* Prototypes to externally visible and usable functions.
*/
UCHAR   *SL_HostIPtoString( ULNG    );
int     SL_GetIPaddr( UCHAR *, ULNG * );
int     SL_GetService( UCHAR *, UINT * );
int     SL_Init( UINT, UCHAR * );
int     SL_Exit( UCHAR * );
void    SL_PostTerminate( void );
UINT    SL_GetChanId( ULNG );
int     SL_RawMode( UINT, UINT );
int     SL_AddServer( UINT, UINT, void (*)(), void (*)(int, ...) );
int     SL_AddClient( UINT, ULNG, UCHAR *, void (*)(), void (*)(int, ...) );
int     SL_AddTimerCB( ULNG, UINT, ULNG, void (*)() );
int     SL_DelServer( UINT    );
int     SL_DelClient( UINT );
int     SL_Close( UINT );
int     SL_SendData( UINT, UCHAR *, UINT );
int     SL_BlockSendData( UINT, UCHAR *, UINT );
int     SL_Poll( ULNG );
int     SL_Kernel( void );

#endif    /* UX_COMMS_H */
