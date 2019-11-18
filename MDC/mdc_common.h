/******************************************************************************
 * Product:       #     # ######   #####          #         ###   ######
 *                ##   ## #     # #     #         #          #    #     #
 *                # # # # #     # #               #          #    #     #
 *                #  #  # #     # #               #          #    ######
 *                #     # #     # #               #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                #     # ######   #####  ####### #######   ###   ######
 *
 * File:          mdc_common.h
 * Description:   General purpose library routines for MDC.
 *
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1996-2019.
 *
 * History:       1.0 - Initial Release.
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
#ifndef    MDC_COMMON_H
#define    MDC_COMMON_H

/* Define constants, definitions etc.
*/
#define    DEF_POLLTIME            1000    /* Default comms poll wait time */
#define    DEF_SERVICENAME         "vdwd"  /* Name of service in /etc/services */
#define    MDC_SRV_KEEPALIVE       1000    /* TCP/IP keep alive for MDC Server */
#define    MAX_TERMINATE_TIME      2000    /* Time for termination of MDC layer */

/* Timeout definitions.
*/
#define    MDC_CL_KEEPALIVE        1000    /* TCP/IP keep alive for MDC client */
#define    CS_SLEEP_TIME           1       /* Create service loop sleep time (mS)*/
#define    SR_SLEEP_TIME           10      /* Srv req reply loop sleep time (mS) */
#define    SNDREQ_SLEEP_TIME       10      /* Wait on send req sleep time (mS) */
#define    DEF_NEW_SERVICE_TIMEOUT 30000   /* Number of mS before give up trying */
                                           /* to make the connection           */
#define    DEF_SRV_REQ_TIMEOUT     10000   /* Number of mS to wait for Service  */
                                           /* Request reply               */
#define    DEF_SEND_REQ_TIMEOUT    5400000 /* Number of mS before timing out */
#define    NEW_SERVICE_TIMEOUT     "NEW_SERVICE"
#define    SRV_REQ_TIMEOUT         "SERVICE_REQUEST"
#define    SEND_REQ_TIMEOUT        "SEND_REQUEST"

/* Globals (yugghhh!) for the MDC library. They are contained within a 
 * structure so they are more manageable and readers can see immediately 
 * the variables scope.
*/
typedef struct {
    /* Shared Globals
    */
    UCHAR        szErrMsg[MAX_ERRMSGLEN];    /* Storage for error messages */

    /* Server Globals
    */
    LINKLIST    *spIFifoHead;      /* Pointer to head of Incoming FIFO list */
    LINKLIST    *spIFifoTail;      /* Pointer to tail of Incoming FIFO list */
    LINKLIST    *spOFifoHead;      /* Pointer to head of Outgoing FIFO list */
    LINKLIST    *spOFifoTail;      /* Pointer to tail of Outgoing FIfO list */
    UINT        nClientChanId;     /* Server to client comms channel Id */
    UINT        nCloseDown;        /* Shutdown flag */
    UINT        nInitialised;      /* Flag to indicate if library initialised */
    void        (*fCntrlCB)( UCHAR );    /* User Control callback */

    /* Client Globals
    */
    UINT        nMDCCommsMode;     /* Indicates whether the Client software */
                                   /* is in MDC Comms Mode, i.e. between an */
                                   /* MDC_start and MDC_End                 */
    LINKLIST    *spChanDetHead;    /* Pointer to head of Channel Details list */
    LINKLIST    *spChanDetTail;    /* Pointer to tail of Channel Details list */
    UINT        nNewSrvTimeout;    /* New service timeout */
    UINT        nSrvReqTimeout;    /* Service Request timeout */
    UINT        nSndReqTimeout;    /* Send Request timeout */
    UINT        nLinkUp;           /* Indicates if the link is up and running */
    UINT        nPendCon;          /* Indicates whether a connection is     */
                                   /* pending                               */
    UINT        nPendConChanId;    /* Indicates chanid for the pending      */
                                   /* connection, control call back sets    */
                                   /* this to zero when connection received */
    UINT        nPendSRChanId;     /* Indicates chanid for a pending        */
                                   /* service request reply                 */
                                   /* 0: no pending service request reply   */
    char        cReplyType;        /* Reply to service request ACK or NAK   */
#if defined(MDC_CLIENT_C) && defined(SOLARIS)
    mutex_t        thMDCLock;      /* Single thread lock for MT environment */
#endif
} MDC_GLOBALS;

/* Declare any common module prototypes which can only be seen by the MDC
 * API library.
*/
#if defined(MDC_COMMON_C) || defined(MDC_CLIENT_C) || defined(MDC_SERVER_C)
    int        _MDC_Init(    void  );
    int        _MDC_Terminate(    void );
#endif

/* Declare any required data types or variables which are specific to the
 * common module.
*/
#ifdef    MDC_COMMON_C
    MDC_GLOBALS        MDC;
#endif

/* Declare any required data types or variables which are specific to the
 * client API module.
*/
#ifdef    MDC_CLIENT_C
    extern    MDC_GLOBALS        MDC;

    typedef enum ChState
    {
        MAKING_CONN,            /* Making connection to daemon */
        IN_SERVICE_REQUEST,     /* Waiting for reply to service request */
        IDLE,                   /* Waiting for a send request, Change    */
                                /* service or Close service          */
        IN_CHANGE_SERVICE,      /* In process of changing service */
        IN_SEND_REQUEST,        /* Processing a request */
        SEND_REQUEST_COMPLETE,  /* Processing for send request is complete. */
                                /* Will go to IDLE after user has done a    */
                                /* GetStatus for the Channel                */
    } CHSTATE;
        
    typedef struct ChanStatus
    {
        UINT nChanId;           /* Channel ID for connection */
        CHSTATE State;          /* State for channel */
        void (*UserDataCB) (UINT, UCHAR *, UINT);     
                                /* Call back function for send request */

        /* The following are only valid if the state is
           SEND_REQUEST_COMPLETE    
        */
        UINT bSendReqResult;    /* Result of send request               */
        UCHAR pszErrStr[MAX_ERRMSGLEN];        
                                /* If NAK returned then contains       */
                                /* the NAK Error String                      */
    } CHANSTATUS;
    
    int _MDC_SendPacket(UINT, char, UCHAR *, UINT);
    int _MDC_PrintErrMsg(UCHAR *, UINT);
    int _MDC_GetSrvRqtReply(UINT, char *);
    int _MDC_CreateChStatus(UINT);
    int _MDC_DelChStatus(UINT);
    int _MDC_SetChState(UINT, CHSTATE);
    int _MDC_GetChState(UINT, CHSTATE *);
    int _MDC_SetSRResult(UINT, UINT);
    int _MDC_GetSRResult(UINT, UINT *);
    int _MDC_GetNAKErrStr(UINT, UCHAR **);
    int _MDC_SetUserCB(UINT, void (*) (UINT, UCHAR *, UINT));
    int _MDC_GetUserCB(UINT, void (**) (UINT, UCHAR *, UINT));
    int    _MDC_WaitOnSndReq( UINT );
#endif

/* Declare any required data types or variables which are specific to the
 * server API module.
*/
#ifdef    MDC_SERVER_C
    /* Structure to hold incoming and outgoing data packets within a FIFO
     * linklist mechanism.
    */
    typedef struct {
        UCHAR        *pszData;    /* Pointer to a data block */
        UINT         nDataLen;    /* Length of data contained in block */
    } FIFO;

    /* External global variable.
    */
    extern    MDC_GLOBALS        MDC;

    /* Prototypes for functions internal to MDC Server module.
    */
    int        _MDC_SendACK( void );
    int        _MDC_SendNAK( UCHAR    * );
    void    _MDC_ServerCntlCB(    int, ... );
    void    _MDC_ServerDataCB(    UINT, UCHAR *, UINT );
#endif

/* A set of macros for implementing a simple thread locking strategy
 * for the MDC library under solaris. The strategy is that only one
 * thread may have access to the MDC libraries at any one time. Eventually
 * the library will become completely thread-safe.
*/
#if defined(SOLARIS)
#define    SINGLE_THREAD_LOCK        mutex_lock(&MDC.thMDCLock);
#define    THREAD_UNLOCK_RETURN(p)   {                            \
                                         mutex_unlock(&MDC.thMDCLock);    \
                                         return(p);                        \
                                     }
#else
#define    SINGLE_THREAD_LOCK        
#define    THREAD_UNLOCK_RETURN(p)   return(p)
#endif

#endif    /* MDC_COMMON_H */
