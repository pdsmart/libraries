/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_comms.c
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

/* Bring in system header files.
*/
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <stdarg.h>
#include    <sys/types.h>
#include    <fcntl.h>
#include    <string.h>

#if defined(_WIN32)
#include    <winsock.h>
#include    <time.h>
#include    <errno.h>
#include    <io.h>
#include    <fstream.h>
#endif

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
#include    <errno.h>
#include    <sys/socket.h>
#include    <netdb.h>
#include    <sys/time.h>
#include    <netinet/in.h>
#include    <sys/wait.h>
#include    <unistd.h>
#endif

#if    defined(SOLARIS)
#include    <sys/file.h>
#endif

#include    <sys/timeb.h>
#include    <sys/stat.h>

/* Indicate that we are a C module for any header specifics.
*/
#define        UX_COMMS_C

/* Bring in specific header files.
*/
#include    "ux.h"

/* Local module variables.
*/
static SL_GLOBALS    Sl;

/* 16 bit CRC lookup table.
*/
SL_CRCTAB tCRCTable[] = {
    {   0,   0 }, { 193, 192 }, { 129, 193 }, {  64,   1 },
    {   1, 195 }, { 192,   3 }, { 128,   2 }, {  65, 194 },
    {   1, 198 }, { 192,   6 }, { 128,   7 }, {  65, 199 },
    {   0,   5 }, { 193, 197 }, { 129, 196 }, {  64,   4 },
    {   1, 204 }, { 192,  12 }, { 128,  13 }, {  65, 205 },
    {   0,  15 }, { 193, 207 }, { 129, 206 }, {  64,  14 },
    {   0,  10 }, { 193, 202 }, { 129, 203 }, {  64,  11 },
    {   1, 201 }, { 192,   9 }, { 128,   8 }, {  65, 200 },
    {   1, 216 }, { 192,  24 }, { 128,  25 }, {  65, 217 },
    {   0,  27 }, { 193, 219 }, { 129, 218 }, {  64,  26 },
    {   0,  30 }, { 193, 222 }, { 129, 223 }, {  64,  31 },
    {   1, 221 }, { 192,  29 }, { 128,  28 }, {  65, 220 },
    {   0,  20 }, { 193, 212 }, { 129, 213 }, {  64,  21 },
    {   1, 215 }, { 192,  23 }, { 128,  22 }, {  65, 214 },
    {   1, 210 }, { 192,  18 }, { 128,  19 }, {  65, 211 },
    {   0,  17 }, { 193, 209 }, { 129, 208 }, {  64,  16 },
    {   1, 240 }, { 192,  48 }, { 128,  49 }, {  65, 241 },
    {   0,  51 }, { 193, 243 }, { 129, 242 }, {  64,  50 },
    {   0,  54 }, { 193, 246 }, { 129, 247 }, {  64,  55 },
    {   1, 245 }, { 192,  53 }, { 128,  52 }, {  65, 244 },
    {   0,  60 }, { 193, 252 }, { 129, 253 }, {  64,  61 },
    {   1, 255 }, { 192,  63 }, { 128,  62 }, {  65, 254 },
    {   1, 250 }, { 192,  58 }, { 128,  59 }, {  65, 251 },
    {   0,  57 }, { 193, 249 }, { 129, 248 }, {  64,  56 },
    {   0,  40 }, { 193, 232 }, { 129, 233 }, {  64,  41 },
    {   1, 235 }, { 192,  43 }, { 128,  42 }, {  65, 234 },
    {   1, 238 }, { 192,  46 }, { 128,  47 }, {  65, 239 },
    {   0,  45 }, { 193, 237 }, { 129, 236 }, {  64,  44 },
    {   1, 228 }, { 192,  36 }, { 128,  37 }, {  65, 229 },
    {   0,  39 }, { 193, 231 }, { 129, 230 }, {  64,  38 },
    {   0,  34 }, { 193, 226 }, { 129, 227 }, {  64,  35 },
    {   1, 225 }, { 192,  33 }, { 128,  32 }, {  65, 224 },
    {   1, 160 }, { 192,  96 }, { 128,  97 }, {  65, 161 },
    {   0,  99 }, { 193, 163 }, { 129, 162 }, {  64,  98 },
    {   0, 102 }, { 193, 166 }, { 129, 167 }, {  64, 103 },
    {   1, 165 }, { 192, 101 }, { 128, 100 }, {  65, 164 },
    {   0, 108 }, { 193, 172 }, { 129, 173 }, {  64, 109 },
    {   1, 175 }, { 192, 111 }, { 128, 110 }, {  65, 174 },
    {   1, 170 }, { 192, 106 }, { 128, 107 }, {  65, 171 },
    {   0, 105 }, { 193, 169 }, { 129, 168 }, {  64, 104 },
    {   0, 120 }, { 193, 184 }, { 129, 185 }, {  64, 121 },
    {   1, 187 }, { 192, 123 }, { 128, 122 }, {  65, 186 },
    {   1, 190 }, { 192, 126 }, { 128, 127 }, {  65, 191 },
    {   0, 125 }, { 193, 189 }, { 129, 188 }, {  64, 124 },
    {   1, 180 }, { 192, 116 }, { 128, 117 }, {  65, 181 },
    {   0, 119 }, { 193, 183 }, { 129, 182 }, {  64, 118 },
    {   0, 114 }, { 193, 178 }, { 129, 179 }, {  64, 115 },
    {   1, 177 }, { 192, 113 }, { 128, 112 }, {  65, 176 },
    {   0,  80 }, { 193, 144 }, { 129, 145 }, {  64,  81 },
    {   1, 147 }, { 192,  83 }, { 128,  82 }, {  65, 146 },
    {   1, 150 }, { 192,  86 }, { 128,  87 }, {  65, 151 },
    {   0,  85 }, { 193, 149 }, { 129, 148 }, {  64,  84 },
    {   1, 156 }, { 192,  92 }, { 128,  93 }, {  65, 157 },
    {   0,  95 }, { 193, 159 }, { 129, 158 }, {  64,  94 },
    {   0,  90 }, { 193, 154 }, { 129, 155 }, {  64,  91 },
    {   1, 153 }, { 192,  89 }, { 128,  88 }, {  65, 152 },
    {   1, 136 }, { 192,  72 }, { 128,  73 }, {  65, 137 },
    {   0,  75 }, { 193, 139 }, { 129, 138 }, {  64,  74 },
    {   0,  78 }, { 193, 142 }, { 129, 143 }, {  64,  79 },
    {   1, 141 }, { 192,  77 }, { 128,  76 }, {  65, 140 },
    {   0,  68 }, { 193, 132 }, { 129, 133 }, {  64,  69 },
    {   1, 135 }, { 192,  71 }, { 128,  70 }, {  65, 134 },
    {   1, 130 }, { 192,  66 }, { 128,  67 }, {  65, 131 },
    {   0,  65 }, { 193, 129 }, { 129, 128 }, {  64,  64 }};

/******************************************************************************
 * Function:    _SL_CalcCRC
 * Description: Calculate the CRC on a buffer.
 * Thread Safe: Yes
 * Returns:     16bit CRC
 ******************************************************************************/
UINT _SL_CalcCRC( UCHAR   *szBuf,        /* I: Data buffer to perform CRC on */
                  UINT    nBufLen )    /* I: Length of data buffer */
{
    /* Local variables.
    */
    UINT        nCRC = 0;
    UINT        nNdx;
    UINT        nTabNdx;
    UCHAR        cHiCRC = 0;
    UCHAR        cLoCRC = 0;

    /* Loop through each byte, using the CRC table to update the CRC value.
    */
    for(nNdx=0; nNdx < nBufLen; nNdx++)
    {
        nTabNdx = cHiCRC ^ (UCHAR)szBuf[nNdx];
        cHiCRC = cLoCRC ^ tCRCTable[nTabNdx].cHiCRC;
        cLoCRC = tCRCTable[nTabNdx].cLoCRC;
    }

    /* Place CRC lower and upper bytes into one integer for caller.
    */
    nCRC = cLoCRC | (cHiCRC << 8);

    /* Return calculated CRC to caller.
    */
    return(nCRC);
}

/******************************************************************************
 * Function:    _SL_CheckCRC
 * Description: Validate the CRC on a buffer with the one given.
 * Thread Safe: Yes
 * Returns:     R_OK     - CRC match.
 *              R_FAIL   - CRC failure.
 * <Errno>        
 ******************************************************************************/
UINT _SL_CheckCRC( UCHAR    *szBuf,        /* I: Data buffer to calc CRC on */
                   UINT     nBufLen   )    /* I: Length of data in buffer */
{
    /* Local variables.
    */
    UINT        nBufferCRC;
    UINT        nPacketCRC;

    /* Get packet CRC from buffer.
    */
    nPacketCRC = szBuf[nBufLen] + (szBuf[nBufLen-1] << 8);

    /* Calculate comparison CRC based on data in buffer.
    */
    nBufferCRC = _SL_CalcCRC( szBuf, nBufLen-2) ;

/* Debugging code.
*/
#if defined(UX_DEBUG)
    if(nPacketCRC != nBufferCRC)
    {
        printf("CRC failed (%u, %u)\n", nPacketCRC, nBufferCRC);
        fflush(stdout);
    }
#endif

    /* Passed or failed?
    */
    return(nPacketCRC == nBufferCRC ? R_OK : R_FAIL);
}

/******************************************************************************
 * Function:    _SL_FdBlocking
 * Description: Set a file descriptors blocking mode.
 * Thread Safe: Yes
 * Returns:     R_OK   - Mode set.
 *              R_FAIL - Couldnt set mode.
 ******************************************************************************/
int    _SL_FdBlocking( int        nFd,        /* File descr to perform action on*/
                       int        nBlock )    /* Block (1) or non-blocking (0) */
{
    /* Local variables.
    */
    int            nReturn = R_FAIL;
#if defined(_WIN32)
    ULNG        lBlock;
#endif

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
    /* Set the mode accordingly.
    */
    if(fcntl(nFd, F_SETFL, nBlock == 1 ? 0 : FNDELAY) >= 0)
        nReturn = R_OK;
#endif

#if defined(_WIN32)
    /* Set up blocking flag.
    */
    lBlock = (nBlock == 1 ? 0L : 1L);

    /* Set the mode accordingly.
    */
    if(ioctlsocket(nFd, FIONBIO, &lBlock) == 0)
        nReturn = R_OK;
#endif

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    _SL_AcceptClient
 * Description: Accept an incoming request from a client. Builds a duplicate
 *              table entry for the client (if one doesnt already exist) and
 *              allocates a unique Channel Id to it.
 * Thread Safe: No, ensures only SL library thread may enter.
 * Returns:     R_OK     - Comms functionality initialised.
 *              R_FAIL   - Initialisation failed, see Errno.
 * <Errno>      E_NOMEM  - Memory exhaustion.
 ******************************************************************************/
int    _SL_AcceptClient( UINT          nServerSd,    /* I: Server Socket Id */
                         SL_NETCONS    *spServer,    /* I: Server descr record */
                         SL_NETCONS    **spNewClnt ) /* O: New client */
{
    /* Local variables.
    */
    struct linger        sLinger;
    struct sockaddr_in   sPeer;
    int                  nReturn = R_FAIL;
    UINT                 nChanId = DEF_CHANID;
    UINT                 nResult = sizeof(sPeer);
    int                  nTmpSd;
    char                 *szFunc = "_SL_AcceptClient";
    LINKLIST             *spNext;
    SL_NETCONS           *spNetCon;

    SL_THREAD_ONLY;

    /* Accept the connection to yield client information.
    */
    if( (nTmpSd=accept(nServerSd, (struct sockaddr *)&sPeer, &nResult)) < 0 )
    {
        Errno = E_BADACCEPT;
        return(nReturn);
    } 

    /* Need to find the next channel ID.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext);
        spNetCon != NULL; spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        /* Find the greatest Channel Id.
        */
        if(spNetCon->nChanId > nChanId)
            nChanId = spNetCon->nChanId;
    }

    /* New entry, need to duplicate masters record.
    */
    if((spNetCon=(SL_NETCONS *)malloc(sizeof(SL_NETCONS))) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
            sizeof(SL_NETCONS));
        Errno = E_NOMEM;
    } else
     {
        /* Duplicate!!! Watch for dynamic copyrights!
        */
        memcpy((UCHAR *)spNetCon, (UCHAR *)spServer, sizeof(SL_NETCONS));

        /* Try and allocate an initial receive buffer.
        */
        if((spNetCon->spRecvBuf=(UCHAR *)malloc(DEF_INITRECVBUF))==NULL)
        {
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                DEF_INITRECVBUF);
            Errno = E_NOMEM;
            free(spNetCon);
        } else
         {
            /* Add in new specific information.
            */
            spNetCon->nSd = nTmpSd;
            spNetCon->nServerPortNo = ntohs(sPeer.sin_port);
            spNetCon->lServerIPaddr = ntohl(sPeer.sin_addr.s_addr);
            spNetCon->nChanId = nChanId + 1;
            spNetCon->nStatus = SSL_UP;
            spNetCon->spXmitBuf = NULL;
            spNetCon->nXmitLen = 0;
            spNetCon->nXmitPos = 0;

            /* Set up KEEPALIVE, so the underlying keeps an eye on the net/
             * processes going up/down.
            */
            if( setsockopt(spNetCon->nSd, SOL_SOCKET, SO_KEEPALIVE,
                           (UCHAR *)&Sl.nSockKeepAlive,
                           sizeof(Sl.nSockKeepAlive)) < 0 )
            {
                Lgr(LOG_WARNING, szFunc, 
                    "Couldnt set KEEPALIVE on socket (%d)", spNetCon->nSd);
            }

            /* Set up LINGER to be disabled, if we die unexpectedly, the socket
             * /ports should be freed up. We lose data, but nothing can be done.
            */
            sLinger.l_onoff = 0;
            sLinger.l_linger = 0;
            if( setsockopt(spNetCon->nSd, SOL_SOCKET, SO_LINGER,
                           (UCHAR *)&sLinger, sizeof(struct linger)) < 0 )
            {
                Lgr(LOG_WARNING, szFunc,
                    "Couldnt disable LINGER on socket (%d)", spNetCon->nSd);
            }

            /* Set the socket to non-blocking mode, not prepared for anything
             * to bring us to a halt!
            */
            _SL_FdBlocking(spNetCon->nSd, 0);

            /* Place in NetCon list.
            */
            if(AddItem(&Sl.spHead, &Sl.spTail, SORT_NONE,
                       &spNetCon->nChanId,
                       &spNetCon->lServerIPaddr,
                       spNetCon->szServerName,
                       spNetCon) == R_OK)
            {
                /* Finally, call the users control callback to let him
                 * know about the new connection.
                */
                spNetCon->nCntrlCallback(SLC_NEWSERVICE, spNetCon->nChanId,
                                         _SL_GetPortNo(spNetCon),
                                         spNetCon->lServerIPaddr,
                                         spNetCon->nOurPortNo);
                nReturn = R_OK;
            } else
             {
                free(spNetCon->spRecvBuf);
                free(spNetCon);
            }

            /* If the caller requires a pointer to the new clients control
             * record then set it up for passback.
            */
            if(spNewClnt != NULL)
            {
                *spNewClnt = spNetCon;
            }
        }
    }

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    _SL_GetPortNo
 * Description: Get valid port number from structures.
 * Thread Safe: Yes
 * Returns:     Port Number.
 ******************************************************************************/
UINT _SL_GetPortNo( SL_NETCONS    *spNetCon )    /* I: Connection description */
{
    /* Local variables.
    */
    UINT    nReturn;

    /* Work out correct callback port number.
    */
    nReturn = (spNetCon->nServerPortNo == 0 ? spNetCon->nOurPortNo :
                                                    spNetCon->nServerPortNo);

    /* Return valid port number.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SL_Close
 * Description: Close a client or server connection.
 * Thread Safe: No, forces SL thread entry only.
 * Returns:     R_OK     - Connection closed successfully.
 *              R_FAIL   - Failed to close connection, see Errno.
 * <Errno>        
 ******************************************************************************/
int    _SL_Close( SL_NETCONS    *spNetCon,        /* I: Connection to sever */
                  UINT           nDoCallback )    /* I: Perform closure callback? */
{
    /* Local variables.
    */
    int        nReturn = R_OK;
    UCHAR    *szFunc = "_SL_Close";

    SL_THREAD_ONLY;

    /* Close the port, no longer needed.
    */
    SocketClose(spNetCon->nSd);

    /* OK, send a close/fail callback to user code if required.
    */
    if(nDoCallback == TRUE)
    {
        spNetCon->nCntrlCallback(SLC_LINKFAIL, spNetCon->nChanId,
                             _SL_GetPortNo(spNetCon), spNetCon->lServerIPaddr,
                             spNetCon->nOurPortNo);
    }

    /* Free up receive buffer, not needed.
    */
    if(spNetCon->spRecvBuf != NULL)
    {
        free(spNetCon->spRecvBuf);
        spNetCon->spRecvBuf = NULL;
    }

    /* Free up transmit buffer, not needed.
    */
    if(spNetCon->spXmitBuf != NULL)
    {
        free(spNetCon->spXmitBuf);
        spNetCon->spXmitBuf = NULL;
    }

    /* Free up control record, no longer needed.
    */
    if(DelItem(&Sl.spHead,&Sl.spTail,spNetCon,NULL,NULL,NULL)
                                                    == R_FAIL)
    {
        Lgr(LOG_WARNING, szFunc,
            "Couldnt delete server entry (%d, %d, %s)",
            spNetCon->nChanId, spNetCon->nOurPortNo,
            SL_HostIPtoString(spNetCon->lServerIPaddr));

        /* Set return value to indicate failure.
        */
        nReturn=R_FAIL;
    } else
     {
        /* Free up the control memory.
        */
        free(spNetCon);
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SL_ConnectToServer
 * Description: Attempt to make a connection with a remote server.
 * Thread Safe: No, forces SL thread entry only.
 * Returns:     R_OK     - 
 *              R_FAIL   - 
 * <Errno>      E_NOMEM    - Memory exhaustion.
 *              E_NOSOCKET - Couldnt allocate a socket for connection.
 ******************************************************************************/
int    _SL_ConnectToServer( SL_NETCONS        *spNetCon )
{
    /* Local variables.
    */
#if defined(_WIN32)
    int                    nWinErr;
#endif
    char                *szFunc = "_SL_ConnectToServer";
    struct linger        sLinger;
    struct sockaddr_in    sServer;

    SL_THREAD_ONLY;

    /* Build up info of the Server we intend to connect to.
    */
    memset((UCHAR *)&sServer, '\0', sizeof(struct sockaddr));
    sServer.sin_family      = AF_INET;
    sServer.sin_addr.s_addr = htonl(spNetCon->lServerIPaddr);
    sServer.sin_port        = htons((USHRT)spNetCon->nServerPortNo);

    /* If required (nearly always), create an end point (socket) for our
     * side of the communications link.
    */
    if(spNetCon->nSd == -1)
    {
        if((spNetCon->nSd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            Errno = E_NOSOCKET;
            return(R_FAIL);
        } else
         {
            /* Set the socket to non-blocking mode, not prepared for anything
             * to bring us to a halt!
            */
            _SL_FdBlocking(spNetCon->nSd, 0);
        }
    }

    /* Try to connect to the other side.
    */
    if(connect(spNetCon->nSd, (struct sockaddr *)&sServer,
               sizeof(struct sockaddr)) == -1)
    {
#if defined(_WIN32)
        /* What was the error...? Under windows its bound to be bad news!!!
         * Call windows own function (well blow me down and call me 
         * gandolfo) to liberate the error code.
        */
        switch((nWinErr = WSAGetLastError()))
#endif
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
        /* What was the error..? Bad news...?
        */
        switch(errno)
#endif
        {
            /* A previous connect on this socket has now completed, so get
             * out and finish connection process.
            */
            case EISCONN:
                break;

            /* The connection process would block, but the underlying is
             * still trying to connect.
            */
            case EINPROGRESS:
            case EWOULDBLOCK:
                spNetCon->nStatus = SSL_DOWN;
                Errno = E_BADCONNECT;
                return(R_FAIL);

            /* Soft errors which may be resolved dynamically, so just back 
             * off for a while.
            */
            case EADDRINUSE:
            case EALREADY:
            case EBADF:
            case ECONNREFUSED:
            case EINTR:
            case ENOTSOCK:
            case ETIMEDOUT:
            case EINVAL:
            case EIO:
                /* Get rid of socket, no longer needed.
                */
                SocketClose(spNetCon->nSd);
                spNetCon->nSd = -1;
                spNetCon->nStatus = SSL_DOWN;
                Errno = E_NOCONNECT;
                return(R_FAIL);

            /* Hard errors which cannot be resolved, mark the connection
             * as down permanently.
            */
            case EADDRNOTAVAIL:
            case EAFNOSUPPORT:
            case EFAULT:
            case ENETUNREACH:
            default:
                /* Get rid of socket, no longer needed.
                */
                SocketClose(spNetCon->nSd);
                spNetCon->nSd = -1;
                Lgr(LOG_WARNING, szFunc, 
                    "Cannot connect (%s, %s, %d) Error (%d)",
                    spNetCon->szServerName, 
                    SL_HostIPtoString(spNetCon->lServerIPaddr),
                    spNetCon->nServerPortNo,
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
                    errno);
#endif
#if defined(_WIN32)
                    nWinErr);
#endif
                spNetCon->nStatus = SSL_FAIL;
                Errno = E_NOCONNECT;
                return(R_FAIL);
        }
    }

    /* Set up KEEPALIVE, so the underlying keeps an eye on the net/
     * processes going up/down.
    */
    if( setsockopt(spNetCon->nSd, SOL_SOCKET, SO_KEEPALIVE,
                   (UCHAR *)&Sl.nSockKeepAlive,
                   sizeof(Sl.nSockKeepAlive)) < 0 )
    {
        Lgr(LOG_WARNING, szFunc, 
            "Couldnt set KEEPALIVE on socket (%d)", spNetCon->nSd);
    }

    /* Set up LINGER to be disabled, if we die unexpectedly, the socket
     * /ports should be freed up. We lose data, but nothing can be done.
    */
    sLinger.l_onoff = 0;
    sLinger.l_linger = 0;
    if( setsockopt(spNetCon->nSd, SOL_SOCKET, SO_LINGER, (UCHAR *)&sLinger,
                   sizeof(struct linger)) < 0 )
    {
        Lgr(LOG_WARNING, szFunc,
            "Couldnt disable LINGER on socket (%d)", spNetCon->nSd);
    }

    /* Finally, call the users control callback to let him
     * know about the new connection.
    */
    spNetCon->nCntrlCallback(SLC_CONNECT, spNetCon->nChanId, 
                             _SL_GetPortNo(spNetCon), spNetCon->lServerIPaddr,
                             spNetCon->nOurPortNo);

    /* Mark connection as up.
    */
    spNetCon->nStatus = SSL_UP;

    /* Finished, success, get out!!
    */
    return( R_OK );
}

/******************************************************************************
 * Function:    _SL_ReceiveFromSocket
 * Description: Receive data from a given socket, growing the receive buffer
 *              if needed.
 * Thread Safe: No, forces SL thread entry only.
 * Returns:     R_OK     - 
 *              R_FAIL   - 
 * <Errno>      E_NOMEM   - Memory exhaustion.
 *              E_BADPARM - Bad parameters passed to function.
 *              E_NOSERVICE - No service on socket.
 ******************************************************************************/
int    _SL_ReceiveFromSocket( SL_NETCONS    *spNetCon )    /* IO: Active connection */
{
    /* Local variables.
    */
    UINT         nBufLen;
    UINT         nNdx;
    int          nRet;
    int          nReturn = R_FAIL;
#if defined(_WIN32)
    int          nWinErr;
#endif
    char         *szFunc = "_SL_ReceiveFromSocket";
    UCHAR        *spNewBuf;
    UCHAR        *spTmp;
    UCHAR        *spTmp2;

    SL_THREAD_ONLY;

    /* Check parameters.
    */
    if( spNetCon == NULL )
    {
        Errno = E_BADPARM;
        Lgr(LOG_DEBUG, szFunc, "spNetCon is null");
        return(nReturn);
    }

    do {
        /* Calculate number of bytes to read.
        */
        if((nBufLen=(spNetCon->nRecvBufLen-spNetCon->nRecvLen)) > MAX_RECVLEN)
        {
            nBufLen = MAX_RECVLEN;
        }

        /* Read as many bytes as possible from the active socket upto the
         * size of the receive buffer. If more bytes are available, then 
         * grow the receive buffer to accomodate.
        */
        nRet=recv(spNetCon->nSd, &(spNetCon->spRecvBuf[spNetCon->nRecvLen]),
                  nBufLen, 0);

        /* If data has been received, then check on the number of bytes read.
        */
        if( nRet > 0 )
        {
            /* Does the number of bytes read fill the buffer?
            */
            if((nRet + spNetCon->nRecvLen) >= spNetCon->nRecvBufLen)
            {
                /* For safety's sake, an upper limit on the size of the
                 * receive buffer has to be implemented. If this ceiling
                 * is hit, then assume something is going wrong, so keep
                 * the current buffer, but dump the contents.
                */
                if( spNetCon->nRecvBufLen >= MAX_RECVBUFSIZE )
                {
                    Lgr(LOG_WARNING, szFunc,
                        "Exceeded maximum size of recv buffer, dumping");
                    spNetCon->nRecvLen = 0;
                    break;    
                }

                /* OK, lets grow the buffer by a fixed size. If no memory
                 * is available, then keep the current buffer, just stop
                 * reading. Later functionality will prune the data.
                */
                if((spNewBuf=(UCHAR *)malloc(spNetCon->nRecvBufLen+
                                            DEF_BUFINCSIZE)) == NULL)
                {
                    Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                        DEF_BUFINCSIZE);
                    nRet = -1;
                    break;
                }

                /* Copy the old buffer onto the new, so we can release it
                 * back to the sys pool.
                */
                for(spTmp=spNetCon->spRecvBuf, spTmp2=spNewBuf, 
                    nNdx=spNetCon->nRecvBufLen;
                    nNdx != 0;
                    nNdx--, *spTmp2 = *spTmp, spTmp++, spTmp2++);

                /* Get rid of old buffer, and update structure pointers 
                 * to accomodate the new buffer and new size values.
                */
                free(spNetCon->spRecvBuf);
                spNetCon->spRecvBuf = spNewBuf;
                spNetCon->nRecvBufLen += DEF_BUFINCSIZE;

                /* Log a message indicating that the buffer has grown in size.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "Allocated new receive buffer of %d bytes",
                    spNetCon->nRecvBufLen);
            }

            /* Update the total number of bytes held in the receive buffer.
            */
            spNetCon->nRecvLen += nRet;
            nReturn = R_OK;
        }
    } while( nRet > 0 );

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
    /* Check for errors. Errno should never be anything but EWOULDBLOCK.
    */
    if(nRet < 1 && errno != EWOULDBLOCK)
#endif
#if defined(_WIN32)
    /* Get the error code from windows to decide on our course of action.
    */
    nWinErr = WSAGetLastError();

    /* Test for errors.
    */
    if(nRet < 1 && nWinErr != EWOULDBLOCK)
#endif
    {
        Errno = E_NOSERVICE;
        nReturn = R_FAIL;
    }

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    _SL_ProcessRecvBuf
 * Description: Process the data held in a network connection's receive buffer.
 *              If a complete packet has been assembled and passed a CRC check,
 *              pass the data to the subscribing application via its callback.
 * Thread Safe: No, forces SL thread entry only.
 * Returns:     R_OK     - 
 *              R_FAIL   - 
 * <Errno>      E_NOMEM    - Memory exhaustion.
 *              E_NOSOCKET - Couldnt allocate a socket for connection.
 ******************************************************************************/
int    _SL_ProcessRecvBuf( SL_NETCONS        *spNetCon )
{
    /* Local variables.
    */
    int            nReturn = R_OK;
    UINT        nTmpLen;
    char        *szFunc = "_SL_ProcessRecvBuf";
    UCHAR        *spTmp;

    SL_THREAD_ONLY;

    if(spNetCon->nRawMode == FALSE)
    {
        do {
            /* Go through the receive buffer and look for two SYNch characters
             * followed by a start of message (STX), this identifies the start 
             * of a data packet. Once found, extract the two preceeding bytesi
             * which indicate the total length of the data packet.
            */
            for(spTmp=spNetCon->spRecvBuf, nTmpLen=0;
                (spTmp+5) < (spNetCon->spRecvBuf+spNetCon->nRecvLen);
                spTmp++, nTmpLen=0)
            {
                /* Start of packet?
                */
                if( *(spTmp+0) == A_SYN &&
                    *(spTmp+1) == A_SYN &&
                    *(spTmp+2) == A_STX )
                {
                    /* Get length.
                    */
                    nTmpLen = GetIntFromChar(spTmp+3);

                    /* If there are not enough bytes in the buffer, then loop,
                     * because this probably isnt a valid packet or we havent
                     * yet received the packet in its entirety.
                    */
                    if((spTmp+nTmpLen+7) <=
                                        (spNetCon->nRecvLen+spNetCon->spRecvBuf)

                    /* Using the length, take a peek at the end of the data
                     * packet and establish that it is a valid packet by the
                     * presence of an end of message (ETX).
                    */
                       && *(spTmp+5+nTmpLen) == A_ETX 

                    /* Finally, check to see if the CRC on the data within the
                     * packet matches that stored within it.
                    */
                       && _SL_CheckCRC(spTmp+5, nTmpLen+2) == R_OK )
                    {
                        break;
                    }
                }
            }

            /* If we have isolated a packet, then call the data callback to
             * process it.
            */
            if(nTmpLen > 0)
            {
                /* Execute the callback function with the obtained data.
                */
                if(spNetCon->nDataCallback != NULL)
                {
                    spNetCon->nDataCallback(spNetCon->nChanId,spTmp+5,nTmpLen);
                } else
                 {
                    Lgr(LOG_DEBUG, szFunc,
                        "Data arriving on a channel (%d) with no handler",
                        spNetCon->nChanId);
                }

                /* Finally, shift data up in the buffer from the end of the last
                 * byte of the packet we've just processed.
                */
                spNetCon->nRecvLen -= ((spTmp+nTmpLen+8) - spNetCon->spRecvBuf);
                if(spNetCon->nRecvLen > 0)
                    memcpy(spNetCon->spRecvBuf, spTmp+nTmpLen+8,
                           spNetCon->nRecvLen);
            }
        } while(nTmpLen > 0);
    } else
     {
        /* Execute the callback function with all the data in the buffer.
        */
        if(spNetCon->nDataCallback != NULL)
        {
            spNetCon->nDataCallback(spNetCon->nChanId, spNetCon->spRecvBuf,
                                    spNetCon->nRecvLen);
            spNetCon->nRecvLen = 0;
        } else
         {
            Lgr(LOG_DEBUG, szFunc,
                "Data arriving on a channel (%d) with no handler",
                spNetCon->nChanId);
        }
    }

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    _SL_ProcessWaitingPorts
 * Description:    
 * Thread Safe: No, forces SL Thread only.
 * Returns:     R_OK    - Select succeeded.
 *              R_FAIL  - Catastrophe, see Errno.
 * <Errno>      E_BADSELECT  - Internal failure causing select to fail.
 *              E_NONWAITING - No sockets waiting processing.
 ******************************************************************************/
int _SL_ProcessWaitingPorts( ULNG    nHibernationPeriod )    /* I: Select sleep*/
{
    /* Local variables.
    */
    int             nReturn = R_FAIL;
    int             nStatus;
    ULNG            lCurrTimeMs;
    fd_set          ReadList;
    fd_set          WriteList;
    fd_set          ExceptList;
    LINKLIST        *spNext;
    SL_NETCONS      *spNetCon;
    struct timeb    sTp;
    struct timeval  sTimeDelay;
    char            *szFunc = "_SL_ProcessWaitingPorts";

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
    pid_t           nPid;
    SL_NETCONS      *spNewClnt;
#endif

    SL_THREAD_ONLY;

    /* Zap select lists, only interested in our own Sockets.
    */
    FD_ZERO(&ReadList);
    FD_ZERO(&WriteList);
    FD_ZERO(&ExceptList);

    /* Get current time to validate comms down timers.
    */
    ftime(&sTp);
    lCurrTimeMs = (sTp.time * 1000L) + (ULNG)sTp.millitm;

    /* Scan list and enable all read/write/except flags on active sockets.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext); spNetCon != NULL;
        spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        /* Connection still waiting to be connected to server?
        */
        if(spNetCon->nStatus == SSL_DOWN &&
           spNetCon->cCorS == STP_CLIENT &&
           spNetCon->lDownTimer < lCurrTimeMs)
        {
            /* If connection fails, set down timer, so we dont bother re-trying
             * to process for a while.
            */
            if(_SL_ConnectToServer(spNetCon) == R_FAIL)
            {
                switch(Errno)
                {
                    case E_BADCONNECT:
                        spNetCon->lDownTimer = lCurrTimeMs + DEF_CONWAITPER;
                        break;

                    default:
                    case E_NOCONNECT:
                        spNetCon->lDownTimer = lCurrTimeMs + DEF_CONFAILPER;
                        break;
                }
            }
        }

        /* Server port still listening? See if any inbounds are waiting.
        */
        if(spNetCon->nStatus == SSL_LISTENING)
        {
            FD_SET(spNetCon->nSd, &ReadList);
        }

        /* Active connections? Need to know if they are ready to accept
         * data or have data awaiting.
        */
        if(spNetCon->nStatus == SSL_UP)
        {
            FD_SET(spNetCon->nSd, &ReadList);
            FD_SET(spNetCon->nSd, &WriteList);
        }

        /* If there is data which is awaiting xmission, then try to send it.
        */
        if(spNetCon->nStatus == SSL_UP && spNetCon->spXmitBuf != NULL)
        {
            SL_SendData(spNetCon->nChanId, NULL, 0);
        }
    }

    /* Issue select on ports of interest, should return immediately or after
     * the programmed delay, thereby not blocking action for too long.
    */
    sTimeDelay.tv_sec = nHibernationPeriod/1000;
    nHibernationPeriod -= sTimeDelay.tv_sec * 1000;
    sTimeDelay.tv_usec = (nHibernationPeriod * 1000L);

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
    nStatus=select(getdtablesize(), &ReadList, NULL, NULL, &sTimeDelay);
#endif
#if defined(_WIN32)
    nStatus=select(MAX_WIN_RLIMIT, &ReadList, NULL, NULL, &sTimeDelay);
#endif

    if(nStatus > 0)
    {
        /* Go through lists and process any pending server connections, data
         * for reception or transmit buffer waiting sessions.
        */
        for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext);
            spNetCon != NULL;
            spNetCon=(SL_NETCONS *)NextItem(&spNext))
        {
            /* Dont process any inactive ports.
            */
            if(spNetCon->nStatus == SSL_FAIL || spNetCon->nStatus == SSL_DOWN)
                continue;

            /* If the read bit is set, receive all data from the socket and
             * store in internal buffer, ready for processing.
            */
            if(FD_ISSET(spNetCon->nSd, &ReadList))
            {
                /* Is this a listening device.... server port?
                */
                if(spNetCon->nStatus == SSL_LISTENING)
                {
#if defined(SOLARIS) || defined(LINUX) || defined(SUNOS) || defined(ZPU)
                    /* If the option to Fork on a new connection has been set,
                     * then fork a child and let it perform the accept of the
                     * incoming connections.
                    */
                    if(spNetCon->nForkForAccept == TRUE)
                    {
                        /* Accept the connection prior to child fork.
                        */
                        if(_SL_AcceptClient(spNetCon->nSd, spNetCon, &spNewClnt)
                                                                     == R_OK)
                        {
                            /* Fork child to handle new connection.
                            */
                            if((nPid=fork()) < 0)
                            {
                                Lgr(LOG_DEBUG, szFunc,
                                    "Couldnt fork a new process, will retry..");
                            } else
                            /* If we are the child then close the parent service
                             * port as we are not interested in it. 
                            */
                            if(nPid == 0)
                            {
                                _SL_Close(spNetCon, FALSE);
                            } else
                            /* If we are the parent then close the accepted
                             * child socket.
                            */
                             {
                                _SL_Close(spNewClnt, FALSE);
                                fflush(stdout);
                            }
                        }
                    } else
                     {
                        /* For standard comms, just accept the connection.
                         * No need to worry about forking etc.
                        */
                        _SL_AcceptClient(spNetCon->nSd, spNetCon, NULL);
                    }
#endif

#if defined(_WIN32)
                    /* For standard comms, just accept the connection.
                     * No need to worry about forking etc.
                    */
                    _SL_AcceptClient(spNetCon->nSd, spNetCon, NULL);
#endif
                } else
                 {
                    if(_SL_ReceiveFromSocket(spNetCon) == R_OK)
                    {
                        /* See if a full packet has been assembled.
                        */
                        _SL_ProcessRecvBuf(spNetCon);
                    } else
                     {
                        /* Process any remaining valid packets prior to
                         * exception handling.
                        */
                        _SL_ProcessRecvBuf(spNetCon);

                        /* Indicate that an exception has occurred on
                         * this socket.
                        */
                        if(Errno == E_NOSERVICE)
                            FD_SET(spNetCon->nSd, &ExceptList);
                    }
                }
            }

            /* Any exceptions occurred on a socket?
            */
            if(FD_ISSET(spNetCon->nSd, &ExceptList))
            {
                /* If this is a server then delete the connection.
                */
                if(spNetCon->cCorS == STP_SERVER)
                {
                    /* Close the connection as it is no longer required.
                    */
                    _SL_Close(spNetCon, TRUE);
                } else
                 {
                    /* A client failure just requires the link to be marked
                     * down and it will eventually be rebuilt.
                    */
                    spNetCon->nStatus = SSL_DOWN;
                    spNetCon->nCntrlCallback(SLC_LINKDOWN, spNetCon->nChanId,
                                             _SL_GetPortNo(spNetCon),
                                             spNetCon->lServerIPaddr);
                }
            }

            /* This Channel marked for closure? Close it only if all data
             * for transmission has been sent.
            */
            if(spNetCon->nClose == TRUE && spNetCon->spXmitBuf == NULL)
            {
                _SL_Close(spNetCon, TRUE);
            }
        }
        nReturn = R_OK;
    } else
     {
        if(nStatus == 0)
        {
            nReturn = R_OK;
        } else
        {
            printf("Bad Select (%d)\n", nStatus);
            fflush(stdout);
            Errno = E_BADSELECT;
        }
    }

#if defined(SUNOS) || defined(LINUX) || defined(ZPU)
    /* Addition: 26/5/1996. Soak up all child result codes for fork on accept
     * children.
    */
    wait4(0, &nStatus, WNOHANG, NULL);
#endif
#if defined(SOLARIS)
    /* Addition: 4/9/1996. Soak up all child result codes for fork on accept
     * children.
    */
    wait3(&nStatus, WNOHANG|WUNTRACED, NULL);
#endif


    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SL_ProcessCallbacks
 * Description: Activate any callbacks whose timers are active and have
 *              expired.
 * Thread Safe: No, only allows SL Thread.
 * Returns:     Time in mS till next callback.
 * <Errno>        
 ******************************************************************************/
ULNG _SL_ProcessCallbacks( void )
{
    /* Local variables.
    */
    ULNG            nReturn = DEF_MAXBLOCKPERIOD;
    ULNG            lCurrTimeMs;
    LINKLIST        *spNext;
    SL_CALLIST      *spCB;
    struct timeb    sTp;

    SL_THREAD_ONLY;

    /* Get current time to set any new expiration timers.
    */
    ftime(&sTp);
    lCurrTimeMs = (sTp.time * 1000L) + (ULNG)sTp.millitm;

    /* Go through callback list and invoke callback function if timer has
     * expired.
    */
    for(spCB=(SL_CALLIST *)StartItem(Sl.spCBHead, &spNext); spCB != NULL;
        spCB=(SL_CALLIST *)NextItem(&spNext))
    {
        /* If the callback is active, and its timer has expired, execute
         * the callback.
        */
        if( spCB->nStatus == TCB_UP && spCB->lTimeExpire <= lCurrTimeMs )
        {
            if( spCB->nCallback != NULL )
                spCB->nCallback(spCB->lCBData);

            /* Update counter according to options flag.
            */
            switch(spCB->nOptions)
            {
                case TCB_ASTABLE:
                    /* Astable means that the time is equidistant from
                     * the last.
                    */
                    spCB->lTimeExpire = lCurrTimeMs + spCB->lTimePeriod;
                    break;

                case TCB_FLIPFLOP:
                    /* Re-read time, as flip flop commences from whence the
                     * function completed.
                    */
                    ftime(&sTp);
                    lCurrTimeMs = (sTp.time * 1000L) + (ULNG)sTp.millitm;
                    spCB->lTimeExpire = lCurrTimeMs + spCB->lTimePeriod;
                    break;

                case TCB_ONESHOT:
                default:
                    spCB->nStatus = TCB_DOWN;
                    break;
            }
        }

        /* If this is an active callback, then see if its expiration time
         * is the lowest. The lowest time period is used for the select
         * hibernation time.
        */
        if( spCB->nStatus == TCB_UP &&
            nReturn > (spCB->lTimeExpire - lCurrTimeMs) )
        {
            nReturn = (spCB->lTimeExpire - lCurrTimeMs);
        }
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    SL_HostIPtoString
 * Description: Convert a given IP address into a string dot notation.
 * Thread Safe: No, API only allows one thread at a time.
 * Returns:     16bit CRC
 ******************************************************************************/
UCHAR *SL_HostIPtoString( ULNG    lIPaddr )    /* I: IP address to convert */
{
    /* Statics and local variables.
    */
    static UCHAR    szIPaddr[16];
    UCHAR           szTmpBuf[5];

    SL_SINGLE_THREAD_ONLY;
    
    /* Convert long into 4 bytes.
    */
    PutCharFromLong(szTmpBuf, lIPaddr);

    /* Copy into string.
    */
    sprintf(szIPaddr, "%d%c%d%c%d%c%d",
            (UINT)szTmpBuf[0], '.',
            (UINT)szTmpBuf[1], '.',
            (UINT)szTmpBuf[2], '.',
            (UINT)szTmpBuf[3]);

    /* Return address of string to caller.
    */
    SL_SINGLE_THREAD_EXIT(szIPaddr);
}

/******************************************************************************
 * Function:    SL_GetIPaddr
 * Description: Get the Internet address of the local machine or a named
 *              machine.
 * Thread Safe: No, API only allows one thread at a time.
 * Returns:     R_OK   - IP address obtained.
 *              R_FAIL - IP address not obtained.
 ******************************************************************************/
int    SL_GetIPaddr( UCHAR       *szHost,        /* I: Hostname string */
                     ULNG        *lIPaddr )      /* O: Storage for the Internet Addr */
{
    /* Local variables.
    */
    int             nReturn = R_FAIL;
    char            szHostName[MAX_MACHINENAME+1];
    struct hostent  *spHostEnt;

    SL_SINGLE_THREAD_ONLY;

    /* If no hostname provided, use local machine name.
    */
    if(szHost == NULL)
    {
        /* Get host name from /etc/hosts.
        */
        gethostname(szHostName, MAX_MACHINENAME);
    } else
     {
        strcpy(szHostName, szHost);
    }

    if( (spHostEnt=gethostbyname( szHostName )) != NULL )
    {
        if( spHostEnt->h_addrtype == AF_INET )
        {
            /* This is cheating a little, as Im assuming in_addr will
             * always be 32 bit.
            */
            memcpy((UCHAR *)lIPaddr, spHostEnt->h_addr, 4);
            *lIPaddr = ntohl(*lIPaddr);
            nReturn = R_OK;
        }
    }
    
    /* Finished, get out!!
    */
    SL_SINGLE_THREAD_EXIT( nReturn );
}

/******************************************************************************
 * Function:    SL_GetService
 * Description: Get the TCP/UDP service port number from the Services File.
 * Thread Safe: No, API Function only allows one thread at a time.
 * Returns:     R_OK   - Service port obtained.
 *              R_FAIL - Service port not obtained.
 ******************************************************************************/
int    SL_GetService( UCHAR    *szService,        /* I: Service Name string */
                      UINT     *nPortNo )         /* O: Storage for the Port Number */
{
    /* Local variables.
    */
    int                nReturn = R_FAIL;
    struct servent    *spServiceEnt;

    SL_SINGLE_THREAD_ONLY;

    /* Call unix functions to search the services file/database for the
     * requested service.
    */
    if( (spServiceEnt=getservbyname( szService, NULL )) != NULL )
    {
        /* Setup callers variable with the service port number.
        */
        *nPortNo = ntohs(spServiceEnt->s_port);
        nReturn = R_OK;
    }
    
    /* Finished, get out!!
    */
    SL_SINGLE_THREAD_EXIT( nReturn );
}

/******************************************************************************
 * Function:    SL_Init
 * Description: Initialise communication variables and connect or setup
 *              listening for required socket connections. 
 * Thread Safe: No, API function only allows one thread at a time.
 * Returns:     R_OK     - Comms functionality initialised.
 *              R_FAIL   - Initialisation failed, see Errno.
 * <Errno>      E_NOMEM  - Memory exhaustion.
 ******************************************************************************/
int    SL_Init( UINT        nSockKeepAlive,    /* I: Socket keep alive time period */
                UCHAR       *szErrMsg )        /* O: Error message buffer */
{
    /* Local variables.
    */
    int            nReturn = R_OK;

    SL_SINGLE_THREAD_ONLY;

#if defined(_WIN32)
    WSADATA        WSAData;

    if(WSAStartup(MAKEWORD(1,1), &WSAData) != 0)
    {
        sprintf(szErrMsg, "Failed to initialise Windows Socket API");
        SL_SINGLE_THREAD_EXIT(R_FAIL);
    }
#endif

    /* Save configuration information.
    */
    Sl.nSockKeepAlive = nSockKeepAlive;

    /* Initialise all variables as needed.
    */
    Sl.spHead = NULL;
    Sl.spTail = NULL;
    Sl.spCBHead = NULL;
    Sl.spCBTail = NULL;

    /* Finished, get out!!
    */
    SL_SINGLE_THREAD_EXIT( nReturn );
}

/******************************************************************************
 * Function:    SL_Exit
 * Description: Decommission the Comms module ready for program termination
 *              or re-initialisation.
 * Thread Safe: No, API function, only allows one thread at a time.
 * Returns:     R_OK     - Exit succeeded.
 *              R_FAIL   - Couldnt perform exit processing, see errno.
 * <Errno>    
 ******************************************************************************/
int    SL_Exit( UCHAR        *szErrMsg )        /* O: Error message buffer */
{
    /* Local variables.
    */
    int            nReturn = R_OK;
    LINKLIST    *spNext;
    SL_NETCONS    *spNetCon;

    SL_SINGLE_THREAD_ONLY;

    /* Free up network connection receive buffer memory.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext); spNetCon != NULL;
        spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        if(spNetCon->spRecvBuf != NULL)
            free(spNetCon->spRecvBuf);
        if(spNetCon->spXmitBuf != NULL)
            free(spNetCon->spXmitBuf);
    }

    /* Free up linked list memory.
    */
    if(Sl.spHead != NULL) DelList(&Sl.spHead, &Sl.spTail);
    if(Sl.spCBHead != NULL) DelList(&Sl.spCBHead, &Sl.spCBTail);

    /* Free up any character buffers...
    */

    /* Finished, get out!!
    */
    SL_SINGLE_THREAD_EXIT( nReturn );
}

/******************************************************************************
 * Function:    SL_PostTerminate
 * Description: Post a request for the program to terminate as soon as
 *              possible.
 * Thread Safe: Yes
 * Returns:     Non.
 ******************************************************************************/
void    SL_PostTerminate( void )
{
    /* Local variables.
    */

    /* To terminate, simply set the flag nCloseDown.
    */
    Sl.nCloseDown = TRUE;

    /* Get out and let the wheels of the Kernel close us down.
    */
    return;
}

/******************************************************************************
 * Function:    SL_GetChanId
 * Description: Get a channel Id from a given IP addr. If the IP addr doesnt
 *              exist or is still pending a connection, return 0 as an error.
 * Thread Safe: No, API function only allows one thread at a time.
 * Returns:     > 0  - Channel Id associated with IP address.
 *              0    - Couldnt find an associated channel.
 ******************************************************************************/
UINT SL_GetChanId( ULNG        lIPaddr )    /* I: Address to xlate */
{
    /* Local variables.
    */
    UINT        nChanId = 0;
    SL_NETCONS  *spNetCon;
    LINKLIST    *spNext;

    SL_SINGLE_THREAD_ONLY;

    /* Go through list of client/server connections and see if we can obtain
     * an IP address match. If a match occurs, then see if the Channel Id
     * is valid.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext);
        spNetCon != NULL;
        spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        if(spNetCon != NULL && spNetCon->lServerIPaddr == lIPaddr)
        {
            /* Consider the channel id to be valid if its active (UP) or
             * temporarily out-of-service (DOWN).
            */
            if( spNetCon->nStatus == SSL_DOWN || spNetCon->nStatus == SSL_UP )
            {
                nChanId = spNetCon->nChanId;
            }
            break;
        }
    }

    /* Finished, get out!!
    */
    SL_SINGLE_THREAD_EXIT( nChanId );
}

/******************************************************************************
 * Function:    SL_RawMode
 * Description: Function to switch a channel into/out of raw mode processing.
 *              Raw mode processing foregoes all forms of checking and is
 *              typically used for connections with a non SL lib server/client.
 * Thread Safe: No, API Function, only allows one thread at a time.
 * Returns:     R_FAIL  - Illegal Channel Id given.
 *              R_OK    - Mode set
 * <Errno>        
 ******************************************************************************/
int SL_RawMode( UINT    nChanId,    /* I: Channel to apply change to */
                UINT    nMode )     /* I: Mode to set channel to */
{
    /* Local variables.
    */
    int                 nReturn = R_FAIL;
    LINKLIST            *spNext;
    SL_NETCONS          *spNetCon;

    SL_SINGLE_THREAD_ONLY;

    /* Scan list to find corresponding channel information.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext); spNetCon != NULL;
        spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        /* Find the greatest Channel Id.
        */
        if(spNetCon != NULL && spNetCon->nChanId == nChanId)
            break;
    }

    /* Did we find it?
    */
    if(spNetCon != NULL)
    {
        spNetCon->nRawMode = (nChanId == 0 ? FALSE : TRUE);
        nReturn = R_OK;
    }

    /* Return result code to caller.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_AddServer
 * Description: Add an entry into the Network Connections table as a Server.
 *              An entry is built up and a socket created and set listening.
 * Thread Safe: No, API Function, only allows one thread at a time.
 * Returns:     R_OK     - Successfully added.
 *              R_FAIL   - Error, see Errno.
 * <Errno>      E_NOMEM    - Memory exhaustion.
 *              E_BADPARM  - Bad parameters passed.
 *              E_EXISTS   - Entry already exists.
 *              E_NOSOCKET - Couldnt grab a socket.
 *              E_NOLISTEN - Couldnt listen on given port.
 ******************************************************************************/
int SL_AddServer( UINT    nPortNo,                      /* I: Port to listen on */
                  UINT    nForkForAccept,               /* I: Fork prior to accept */
                  void    (*nDataCallback)(),           /* I: Data ready callback */
                  void    (*nCntrlCallback)(int, ...) ) /* I: Control callback */
{
    /* Local variables.
    */
    int                   nReturn = R_FAIL;
    char                  *szFunc = "SL_AddServer";
    LINKLIST              *spNext;
    SL_NETCONS            *spNetCon;
    struct sockaddr_in    sServer;

    SL_SINGLE_THREAD_ONLY;

    /* Scan list to see if an entry exists for requested server, if it does
     * then just exit.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext); spNetCon != NULL;
        spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        if(spNetCon != NULL && spNetCon->cCorS == STP_SERVER &&
           spNetCon->nOurPortNo == nPortNo)
        {
            Errno = E_EXISTS;
            SL_SINGLE_THREAD_EXIT(nReturn);
        }
    }

    /* Create a Network Connection record, populate, set in motion and add
     * to the support lists.
    */
    if((spNetCon=(SL_NETCONS *)malloc(sizeof(SL_NETCONS))) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
            sizeof(SL_NETCONS));
        Errno = E_NOMEM;
    } else
     {
        /* Wash the new memory, just in case.
        */
        memset((UCHAR *)spNetCon, '\0', sizeof(SL_NETCONS));

        /* Fill out the remaining structure conflab.
        */
        spNetCon->cCorS = STP_SERVER;
        spNetCon->nClose = FALSE;
        spNetCon->nOurPortNo = nPortNo;
        spNetCon->nDataCallback = nDataCallback;
        spNetCon->nCntrlCallback = nCntrlCallback;
        spNetCon->nRecvBufLen = DEF_INITRECVBUF;
        spNetCon->nStatus = SSL_LISTENING;
        spNetCon->nForkForAccept = nForkForAccept;
        spNetCon->spXmitBuf = NULL;
        spNetCon->nXmitLen = 0;
        spNetCon->nXmitPos = 0;

        /* Build up Server address info, so it can be publicised by bind to
         * the big wide world.
        */
        memset((UCHAR *)&sServer, '\0', sizeof(struct sockaddr));
        sServer.sin_family = AF_INET;
        sServer.sin_port = htons((USHRT)spNetCon->nOurPortNo);
        sServer.sin_addr.s_addr = htonl(INADDR_ANY);

        /* Fire up a socket and lets listen.
        */
        if((spNetCon->nSd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            Errno = E_NOSOCKET;
            free(spNetCon->spRecvBuf);
            free(spNetCon);
        } else
        if(bind(spNetCon->nSd, (struct sockaddr *)&sServer,
                sizeof(struct sockaddr)) == -1)
        {
            Errno = E_NOBIND;
            free(spNetCon->spRecvBuf);
            free(spNetCon);
        } else
        if(listen(spNetCon->nSd, MAX_SOCKETBACKLOG) == -1)
        {
            Errno = E_NOLISTEN;
            free(spNetCon->spRecvBuf);
            free(spNetCon);
        } else
         {
            /* OK, almost there, now will it stick onto the lists!!?
            */
            if(AddItem(&Sl.spHead, &Sl.spTail, SORT_NONE, &spNetCon->nChanId,
                       &spNetCon->lServerIPaddr, spNetCon->szServerName,
                       spNetCon) == R_OK)
            {
                nReturn = R_OK;
            } else
             {
                /* Free used memory, Errno already set by AddItem.
                */
                free(spNetCon);
            }
        }
    }

    /* Return Channel ID or error to caller.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_AddClient
 * Description: Add an entry into the Network Connections table as a client.
 *              Socket creation and connect are left to the kernels 
 *              discretion.
 * Thread Safe: No, API function, only allows one thread at a time.
 * Returns:     >= 0     - Channel Id.
 *              -1       - Error, see Errno.
 * <Errno>      E_NOMEM  - Memory exhaustion.
 *              E_EXISTS - A client of same detail exists.
 ******************************************************************************/
int SL_AddClient( UINT    nServerPortNo,                /* I: Server port to talk on */
                  ULNG    lServerIPaddr,                /* I: Server IP address */
                  UCHAR   *szServerName,                /* I: Name of Server */
                  void    (*nDataCallback)(),           /* I: Data ready callback */
                  void    (*nCntrlCallback)(int, ...) ) /* I: Control callback */
{
    /* Local variables.
    */
    UINT        nChanId = DEF_CHANID;
    int         nReturn = -1;
    char        *szFunc = "SL_AddClient";
    LINKLIST    *spNext;
    SL_NETCONS  *spNetCon;

    SL_SINGLE_THREAD_ONLY;

    /* Scan list to see if an entry exists for requested client, if it does
     * then just exit.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext); spNetCon != NULL;
        spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        /* Find the greatest Channel Id.
        */
        if(spNetCon != NULL && spNetCon->nChanId > nChanId)
            nChanId = spNetCon->nChanId;

        /* Duplicate?
        if(spNetCon != NULL && spNetCon->cCorS == STP_CLIENT &&
           spNetCon->nServerPortNo == nServerPortNo &&
           spNetCon->lServerIPaddr == lServerIPaddr )
        {
            Errno = E_EXISTS;
            SL_SINGLE_THREAD_EXIT(nReturn);
        }
        */
    }

    /* Create a Network Connection record, populate, see if a connection with
     * the remote server can be obtained, then add to the support lists.
    */
    if((spNetCon=(SL_NETCONS *)malloc(sizeof(SL_NETCONS))) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
            sizeof(SL_NETCONS));
        Errno = E_NOMEM;
    } else
     {
        /* Wash the new memory, just in case.
        */
        memset((UCHAR *)spNetCon, '\0', sizeof(SL_NETCONS));

        /* Try and allocate an initial receive buffer.
        */
        if((spNetCon->spRecvBuf=(UCHAR *)malloc(DEF_INITRECVBUF)) == NULL)
        {
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                DEF_INITRECVBUF);
            Errno = E_NOMEM;
            free(spNetCon);
        } else
         {
            /* Fill out the remaining structure conflab.
            */
            spNetCon->cCorS = STP_CLIENT;
            spNetCon->nSd = -1;
            spNetCon->nRawMode = FALSE;
            spNetCon->nServerPortNo = nServerPortNo;
            spNetCon->lServerIPaddr = lServerIPaddr;
            strcpy(spNetCon->szServerName, szServerName);
            spNetCon->nDataCallback = nDataCallback;
            spNetCon->nCntrlCallback = nCntrlCallback;
            spNetCon->nRecvBufLen = DEF_INITRECVBUF;
            spNetCon->nStatus = SSL_DOWN;
            spNetCon->nChanId = nChanId + 1;
            spNetCon->lDownTimer = 0L;
            spNetCon->spXmitBuf = NULL;
            spNetCon->nXmitLen = 0;
            spNetCon->nXmitPos = 0;

            /* OK, almost there, now will it stick onto the lists!!?
            */
            if(AddItem(&Sl.spHead, &Sl.spTail, SORT_NONE, &spNetCon->nChanId,
                       &spNetCon->lServerIPaddr, spNetCon->szServerName,
                       spNetCon) == R_OK)
            {
                nReturn = spNetCon->nChanId;
            } else
             {
                /* Free up used memory, Errno has been set by AddItem.
                */
                free(spNetCon->spRecvBuf);
                free(spNetCon);
            }
        }
    }

    /* Return Channel ID or error to caller.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_AddTimerCB
 * Description: Add a timed callback. Basically, a timed callback is a function
 *              which gets invoked after a period of time. This function
 *              can be invoked once (TCB_ONESHOT), every Xms (TCB_ASTABLE) or
 *              a fixed period of time Xms from last execution (TCB_FLIPFLOP).
 *              Each callback can pass a predefined variable/pointer, so
 *              multiple instances of the same callback can exist, each
 *              referring to the same function, but passing different values
 *              to it. The callbacks are maintained in a dynamic link list.
 * Thread Safe: No, API Function, only allows single thread at a time.
 * Returns:     R_OK     - Callback added successfully.
 *              R_FAIL   - Failure, see Errno.
 * <Errno>      E_NOMEM  - Memory exhaustion.
 ******************************************************************************/
int SL_AddTimerCB( ULNG    lTimePeriod,        /* I: Time between callbacks */
                   UINT    nOptions,           /* I: Option flags on callback */
                   ULNG    lCBData,            /* I: Data to be passed to cb */
                   void    (*nCallback)() )    /* I: Function to call */
{
    /* Local variables.
    */
    int             nReturn = R_FAIL;
    ULNG            lCurrTimeMs;
    char            *szFunc = "SL_AddTimerCB";
    SL_CALLIST      *spCB;
    LINKLIST        *spNext;
    struct timeb    sTp;

    SL_SINGLE_THREAD_ONLY;

    /* Go through callback list to locate an existing entry. Existing entries
     * may occur as the application is just updating the configuration of
     * the given callback.
    */
    for(spCB=(SL_CALLIST *)StartItem(Sl.spCBHead, &spNext); spCB != NULL;
        spCB=(SL_CALLIST *)NextItem(&spNext))
    {
        /* If the callback is the same... and the data back is the same, then
         * we are updating an existing record.
        */
        if( spCB->nCallback ==  nCallback && spCB->lCBData == lCBData )
            break;
    }

    /* Does a current entry exist? If not, allocate and add to linked list.
    */
    if( spCB == NULL )
    {
        /* Create record.
        */
        if((spCB = (SL_CALLIST *)malloc(sizeof(SL_CALLIST))) == NULL)
        {
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                sizeof(SL_CALLIST));
            Errno = E_NOMEM;
        } else
         {
            /* Wash the new memory, just in case.
            */
            memset((UCHAR *)spCB, '\0', sizeof(SL_CALLIST));
        }

        /* Add to callback list.
        */
        if(AddItem(&Sl.spCBHead, &Sl.spCBTail, SORT_NONE, NULL, NULL, NULL,
                   spCB) == R_FAIL)
        {
            /* Dont modify Errno as AddItem has already set it for the
             * correct error condition.
            */
            free(spCB);
            spCB = NULL;
        } else
         {
            /* Store the callback address, no need to accomplish in an update
             * scenario.
            */
            spCB->nCallback    = nCallback;
        }
    }

    /* Still ok...? ie Where in an update situation or we successfully
     * allocated, initialised and linked in a new record!
    */
    if( spCB != NULL )
    {
        /* Get current time to set expiration timer.
        */
        ftime(&sTp);
        lCurrTimeMs = (sTp.time * 1000L) + (ULNG)sTp.millitm;

        /* Copy in/update the required parameters.
        */
        if((spCB->nOptions = nOptions) == TCB_OFF)
        {
            spCB->nStatus = TCB_DOWN;
        } else
         {
            spCB->lTimePeriod  = lTimePeriod;
            spCB->lCBData      = lCBData;
            spCB->lTimeExpire  = lCurrTimeMs + lTimePeriod;
            spCB->nStatus      = TCB_UP;
        }
        nReturn = R_OK;
    }

    /* Return result code to caller.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_DelServer
 * Description: Delete an entry from the Network Connections table and 
 *              disable the actual communications associated with it.
 * Thread Safe: No, API function allows one thread at a time.
 * Returns:     R_OK     - Successfully deleted.
 *              R_FAIL   - Error, see Errno.
 * <Errno>      E_BADPARM  - Bad parameters passed.
 ******************************************************************************/
int    SL_DelServer(    UINT    nPortNo        )    /* I: Port number that server on */
{
    /* Local variables.
    */
    int                 nReturn = R_FAIL;
    char                *szFunc = "SL_DelServer";
    LINKLIST            *spNext;
    SL_NETCONS          *spNetCon;

    SL_SINGLE_THREAD_ONLY;

    /* Scan list to find the required entry for deletion.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext); spNetCon != NULL;
        spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        if(spNetCon != NULL && spNetCon->cCorS == STP_SERVER &&
           spNetCon->nOurPortNo == nPortNo)
        {
            /* Entry found, so close it down.
            */
            nReturn=_SL_Close(spNetCon, FALSE);

            /* Exit with result code.
            */
            SL_SINGLE_THREAD_EXIT(nReturn);
        }
    }

    /* Didnt find the entry so exit with fail.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_DelClient
 * Description: Delete a client entry from the Network Connections table and
 *              free up all resources that it used.
 * Thread Safe: No, API function allows on thread at a time.
 * Returns:     R_OK     - Client connection successfully closed.
 *              R_FAIL   - Error, see Errno.
 * <Errno>        
 ******************************************************************************/
int SL_DelClient(    UINT    nChanId )        /* I: Channel Id of client to del*/
{
    /* Local variables.
    */
    int         nReturn = R_FAIL;
    char        *szFunc = "SL_DelClient";
    LINKLIST    *spNext;
    SL_NETCONS  *spNetCon;

    SL_SINGLE_THREAD_ONLY;

    /* Scan list to see if an entry exists for requested client, if it does
     * then delete it.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext); spNetCon != NULL;
        spNetCon=(SL_NETCONS *)NextItem(&spNext))
    {
        /* If we have a match on channel Id then we have located the correct
         * entry.
        */
        if(spNetCon != NULL && /* spNetCon->cCorS == STP_CLIENT &&*/
           spNetCon->nChanId == nChanId )
        {
            /* Entry found, so close it down.
            */
            nReturn=_SL_Close(spNetCon, FALSE);

            /* Exit with result code.
            */
            SL_SINGLE_THREAD_EXIT(nReturn);
        }
    }

    /* Didnt find the required entry so exit with failure code.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_Close
 * Description: Close a socket connection (Client or Server) based on the
 *              given channel ID. Due to the nature of the socket library,
 *              this cannot be performed immediately, as nearly always, the
 *              application requesting the close is within a socket library
 *              callback, and modifying internal structures in this state is
 *              fraught with danger.
 * Thread Safe: No, API function, only allows one thread at a time.
 * Returns:     Non.
 ******************************************************************************/
int    SL_Close( UINT    nChanId )    /* I: Channel Id to close */
{
    /* Local variables.
    */
    char        *szFunc = "SL_Close";
    LINKLIST    *spNext;
    SL_NETCONS  *spNetCon;

    SL_SINGLE_THREAD_ONLY;

    /* Using the channel Id, seek out the corresponding Net Connection
     * record.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext);
        spNetCon != NULL && spNetCon->nChanId != nChanId;
        spNetCon=(SL_NETCONS *)NextItem(&spNext));

    /* If we cant locate a record corresponding to the given channel Id,
     * then the calling application has passed us a bad value or internal
     * workings are going wrong.
    */
    if(spNetCon != NULL && spNetCon->nChanId != nChanId)
    {
        Errno = E_INVCHANID;
        SL_SINGLE_THREAD_EXIT(R_FAIL);
    }

    /* OK, record found, mark the channel for closure and get out.
    */
    spNetCon->nClose = TRUE;

    /* Get out, big bang time!
    */
    SL_SINGLE_THREAD_EXIT(R_OK);
}

/******************************************************************************
 * Function:    SL_SendData
 * Description: Transmit a packet of data to a given destination identified
 *              by it channel Id.
 * Thread Safe: No, API function, only allows one thread at a time.
 * Returns:     R_OK     - Data sent successfully.
 *              R_FAIL   - Couldnt send data, see Errno.
 * <Errno>      E_INVCHANID - Invalid channel Id.
 *              E_BUSY      - Channel is busy, retry later.
 *              E_BADSOCKET - Internal failure on socket, terminal.
 *              E_NOSERVICE - No remote connection established yet.
 ******************************************************************************/
int SL_SendData( UINT    nChanId,      /* I: Channel Id to send data on */
                 UCHAR    *szData,     /* I: Data to be sent */
                 UINT    nDataLen )    /* I: Length of data */
{
    /* Local variables.
    */
    UINT        nDataCRC;
    int            nNewBuf = FALSE;
    int            nReturn = R_FAIL;
    int            nSend;
#if defined(_WIN32)
    int            nWinErr;
#endif
    char        *szFunc = "SL_SendData";
    LINKLIST    *spNext;
    SL_NETCONS    *spNetCon;

    SL_SINGLE_THREAD_ONLY;

    /* Scan list to see if an entry exists for requested channel.
    */
    for(spNetCon=(SL_NETCONS *)StartItem(Sl.spHead, &spNext);
        spNetCon != NULL && spNetCon->nChanId != nChanId;
        spNetCon=(SL_NETCONS *)NextItem(&spNext));

    /* If the channel is invalid, get out.
    */
    if(spNetCon == NULL)
    {
        Errno = E_INVCHANID;
        SL_SINGLE_THREAD_EXIT(nReturn);
    }

    /* If the caller has passed no data in then he is wanting to flush any
     * existing buffer out and get a result from it. If there is no data
     * pending for transmission then exit with OK.
    */
    if(szData == NULL && spNetCon->spXmitBuf == NULL)
    {
        SL_SINGLE_THREAD_EXIT(R_OK);
    }

    /* If the caller wants to send a buffer but a previous buffer is still
     * pending transmission, then exit with busy.
    */
    if(szData != NULL && spNetCon->spXmitBuf != NULL)
    {
        Errno = E_BUSY;
        SL_SINGLE_THREAD_EXIT(R_FAIL);
    }

    /* Is there data currently pending transmission...?
    */
    if(spNetCon->spXmitBuf == NULL && szData != NULL)
    {
        /* Pre-load sizing variables.
        */
        spNetCon->nXmitLen = nDataLen;
        if(spNetCon->nRawMode == FALSE) spNetCon->nXmitLen += 8;
        spNetCon->nXmitPos = 0;

        /* Set the new buffer flag to indicate that we have just undertaken
         * to transmit a new buffer. This flag affects the return code in
         * the event that we get a busy on the send port.
        */
        nNewBuf = TRUE;

        /* If not in Raw Mode, format the data in format:
         * <SYN><SYN><STX><LEN_LSB><LEN_MSB><..DATA..><ETX><CRC_LSB><CRC_MSB>
        */
        if((spNetCon->spXmitBuf=(UCHAR *)malloc(spNetCon->nXmitLen)) == NULL)
        {
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                spNetCon->nXmitLen);
            Errno = E_NOMEM;
            SL_SINGLE_THREAD_EXIT(nReturn);
        } else
         {
            /* Copy data as is into buffer.
            */
            if(spNetCon->nRawMode == FALSE)
            {
                memcpy((UCHAR *)spNetCon->spXmitBuf+5,(UCHAR *)szData,nDataLen);

                /* Add formatting data.
                */
                spNetCon->spXmitBuf[0] = A_SYN;
                spNetCon->spXmitBuf[1] = A_SYN;
                spNetCon->spXmitBuf[2] = A_STX;
                PutCharFromInt(&spNetCon->spXmitBuf[3], nDataLen);
                spNetCon->spXmitBuf[nDataLen+5] = A_ETX;

                /* Finally, get CRC on data.
                */
                nDataCRC = _SL_CalcCRC(szData, nDataLen);
                PutCharFromInt(&spNetCon->spXmitBuf[nDataLen+6], nDataCRC);
            } else
             {
                memcpy((UCHAR *)spNetCon->spXmitBuf, (UCHAR *)szData, nDataLen);
            }
        }
    }

    /* If we've got info to xmit, then transmit it.
    */
    if(spNetCon->spXmitBuf != NULL)
    {
        switch(spNetCon->nStatus)
        {
            case SSL_UP:
                /* Send the data, if an error occurs, group into relevant
                 * internal code.
                */
                nSend = send(spNetCon->nSd,
                             &spNetCon->spXmitBuf[spNetCon->nXmitPos],
                             spNetCon->nXmitLen - spNetCon->nXmitPos, 0);

                /* Any failure's.
                */
                if(nSend == -1)
                {
#if defined(_WIN32)
                    /* Windows again folks, old bill just doesnt like standards.
                    */
                    switch((nWinErr = WSAGetLastError()))
#endif
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
                    switch(errno)
#endif
                    {
                        case EINTR:
                        case ENOBUFS:
                        case EWOULDBLOCK:
                            Errno = E_BUSY;
                            break;
                        case EBADF:
                        case EFAULT:
                        case EINVAL:
                        case ENOTSOCK:
                        default:
                            Errno = E_BADSOCKET;
                            break;
                    }
                } else
                 {
                    /* Move position pointer on according to number of bytes
                     * transmitted.
                    */
                    spNetCon->nXmitPos += nSend;

                    /* If we've finished transmitting the entire buffer, then
                     * clean up.
                    */
                    if(spNetCon->nXmitPos == spNetCon->nXmitLen)
                    {
                        /* No further need for dynamic buffer.
                        */
                        free(spNetCon->spXmitBuf);
                        spNetCon->spXmitBuf = NULL;
                    }
                    nReturn = R_OK;
                }
                break;
            case SSL_LISTENING:
            case SSL_DOWN:
                Errno = E_NOSERVICE;
                break;
            case SSL_FAIL:
            default:
                Errno = E_BADSOCKET;
                break;
        }
    } else
     {
        nReturn = R_OK;
    }

    /* If a failure occurs due to the send-buffer becoming full, tell
     * the user that the packet has been sent ok, as we'll flush it out
     * in the background.
    */
    if(nNewBuf == TRUE && nReturn == R_FAIL && Errno == E_BUSY)
    {
        nReturn = R_OK;
    }

    /* Return result code to caller.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_BlockSendData
 * Description: Transmit a packet of data to a given destination but ensure it
 *              is sent prior to exit. If an error occurs, then return it
 *              to the caller. 
 * Thread Safe: No, API function, only allows one thread at a time.
 * Returns:     R_OK     - Data sent successfully.
 *              R_FAIL   - Couldnt send data, see Errno.
 * <Errno>      E_INVCHANID - Invalid channel Id.
 *              E_BADSOCKET - Internal failure on socket, terminal.
 *              E_NOSERVICE - No remote connection established yet.
 ******************************************************************************/
int SL_BlockSendData( UINT    nChanId,    /* I: Channel Id to send data on */
                      UCHAR   *szData,    /* I: Data to be sent */
                      UINT    nDataLen )  /* I: Length of data */
{
    /* Local variables.
    */
    int        nReturn;

    SL_SINGLE_THREAD_ONLY;

    /* Flush out any data that is already queued for transmission.
    */
    for(; (nReturn=SL_SendData(nChanId, NULL, 0))==R_FAIL && Errno == E_BUSY; );

    /* Check to ensure that no other failure occurred with flushing the
     * buffers. If there is, pass back values setup by SL_SendData.
    */
    if(nReturn != R_OK)
    {
        SL_SINGLE_THREAD_EXIT(nReturn);
    }

    /* Send the actual data. Return if an error occurs.
    */
    if((nReturn=SL_SendData(nChanId, szData, nDataLen)) == R_FAIL)
    {
        SL_SINGLE_THREAD_EXIT(nReturn);
    }

    /* Flush out the data which we have just queued.
    */
    for(; (nReturn=SL_SendData(nChanId, NULL, 0))==R_FAIL && Errno == E_BUSY; );

    /* Return the result code given when flushing the data out as it 
     * will reflect any errors exactly.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_Poll
 * Description: Function for programs which cant afford UX taking control of
 *              the CPU. This function offers these type of applications the
 *              ability to allow comms processing by frequently calling this
 *              Poll function.
 * Thread Safe: No, API function, only allows one thread at a time.
 * Returns:     R_OK    - System closing down.
 *              R_FAIL  - Catastrophe, see Errno.
 * <Errno>        
 ******************************************************************************/
int SL_Poll( ULNG    lSleepTime )
{
    /* Local variables.
    */
    int            nReturn = R_OK;

    SL_SINGLE_THREAD_ONLY;

    /* Process list of callback routines. A callback works in time, when
      * a certain amount of time has elapsed an application provided
     * function is called.
    */
    _SL_ProcessCallbacks();

    /* Any sockets awaiting attention?
    */
    _SL_ProcessWaitingPorts(lSleepTime);

    /* Return result code to caller.
    */
    SL_SINGLE_THREAD_EXIT(nReturn);
}

/******************************************************************************
 * Function:    SL_Kernel
 * Description: Application process control is passed over to this function
 *              and it allocates and manages time/events. The application
 *              registers callbacks with this library, and they are invoked
 *              as events occur or as time elapses. Control passes out of
 *              this function on application completion.
 * Thread Safe: No, Assumes main thread or one control thread.
 * Returns:     R_OK    - System closing down.
 *              R_FAIL  - Catastrophe, see Errno.
 * <Errno>        
 ******************************************************************************/
int SL_Kernel( void )
{
    /* Local variables.
    */
    int         nReturn = R_FAIL;
    ULNG        nHibernationPeriod;

    /* Stay in this function forever until a serious event occurs or an
     * external event modifies the CloseDown flag.
    */
    Sl.nCloseDown = 0;

    do {
        /* Process list of callback routines. A callback works in time, when
         * a certain amount of time has elapsed an application provided
         * function is called.
        */
        nHibernationPeriod=_SL_ProcessCallbacks();

        /* Any sockets awaiting attention?
        */
        _SL_ProcessWaitingPorts(1000);
    } while(!Sl.nCloseDown);

    /* Return result code to caller.
    */
    return(nReturn);
}
