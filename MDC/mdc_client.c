/******************************************************************************
 * Product:       #     # ######   #####          #         ###   ######
 *                ##   ## #     # #     #         #          #    #     #
 *                # # # # #     # #               #          #    #     #
 *                #  #  # #     # #               #          #    ######
 *                #     # #     # #               #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                #     # ######   #####  ####### #######   ###   ######
 *
 * File:          mdc_client.c
 * Description:   MDC Generic communications routines.
 *
 * Version:       %I%    
 * Dated:         %D%
 * Copyright:     P. Smart, 1996-2019
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
#if defined(SOLARIS)
#include    <thread.h>
#endif
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <stdarg.h>
#include    <string.h>

/* Bring in Unix Library headers for TCP/IP communications.
*/
#include    <ux.h>

/* Indicate that we are a C module for any header specifics.
*/
#define     MDC_CLIENT_C

/* Bring in mdc header files.
*/
#include    "mdc.h"
#include    "mdc_common.h"

/******************************************************************************
 * Function:    _MDC_DataCB
 * Description: This function is called back when data is received on any of
 *              the service connections
 * Returns:     void
 ******************************************************************************/
void _MDC_DataCB(UINT nChanId, UCHAR *szData, UINT nDataLen)
{
    UCHAR       *szFunc = "_MDC_DataCB";
    UCHAR       *szDeComData;         /* pointer to decompressed data buffer */
    CHSTATE     eChanSt;
    void        (*pUserCB) (UINT, UCHAR *, UINT);
    UCHAR       *pszNAKErrStr;        /* pointer to Error string in      */
                                      /* channel status structure           */

    /* Decompress 
    */
    szDeComData = Decompress(szData, &nDataLen);
    if (szDeComData == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot malloc");
        return;
    }

    /* Check if the data is a service request reply
    */
    if (MDC.nPendSRChanId > 0 && nChanId == MDC.nPendSRChanId)
    {
        /* Reply received for service request
        */
        MDC.cReplyType = * ((char *) szDeComData);
        if (MDC.cReplyType == MDC_NAK)
        {
            Lgr(LOG_DEBUG, szFunc, "NAK received for service request");
            _MDC_PrintErrMsg((szDeComData + 1), (nDataLen - 1));
        }

        MDC.nPendSRChanId = 0;

        if (szDeComData != szData)
            free(szDeComData);

        return;
    }

    /* This is User Data, an ACK or a NAK
       Get channel state
    */
    if (_MDC_GetChState(nChanId, &eChanSt) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "No status information for channel ID %d", nChanId);
        if (szDeComData != szData)
            free(szDeComData);
        return;
    }
    
    switch(* ((char *) szDeComData))
    {
        case MDC_DATA:
            if (eChanSt != IN_SEND_REQUEST)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "Received data when in wrong state on channel %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }
            
            /* Get call back function address
            */    
            if (_MDC_GetUserCB(nChanId, &pUserCB) != MDC_OK)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "_MDC_GetUserCB failed for channel ID %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }

            if (pUserCB == NULL)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "User call back is NULL for channel ID %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }

            /* Call User call back.
               Remove packet type character
            */    
            (*pUserCB) (nChanId, (szDeComData + 1), (nDataLen - 1));
            break;

        case MDC_ACK:
            if (eChanSt != IN_SEND_REQUEST)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "Received ACK when in wrong state on channel %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }

            if (_MDC_SetSRResult(nChanId, TRUE) != MDC_OK)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "_MDC_SetSRResult failed for channel %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }

            if (_MDC_SetChState(nChanId, SEND_REQUEST_COMPLETE) != MDC_OK)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "_MDC_SetChState failed for channel %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }

            break;

        case MDC_NAK:
            if (eChanSt != IN_SEND_REQUEST)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "Received ACK when in wrong state on channel %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }
 
            /* Extract and save Error String 
            */
            if (_MDC_GetNAKErrStr(nChanId, &pszNAKErrStr) != MDC_OK)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "_MDC_GetNAKErrStr failed for channel %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }
            
            strcpy((char *) pszNAKErrStr, (char *) (szDeComData + 1));

            if (_MDC_SetSRResult(nChanId, FALSE) != MDC_OK)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "_MDC_SetSRResult failed for channel %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }
 
            if (_MDC_SetChState(nChanId, SEND_REQUEST_COMPLETE) != MDC_OK)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "_MDC_SetChState failed for channel %d", nChanId);
                if (szDeComData != szData)
                    free(szDeComData);
                return;
            }

            break;

        default:
            /* Unrecognised message type */
            Lgr(LOG_DEBUG, szFunc,
                "Unrecognised message type %c for channel ID %d",
                eChanSt, nChanId);
            break;
    }

    if (szDeComData != szData)
        free(szDeComData);
}


/******************************************************************************
 * Function:    _MDC_CtrlCB
 * Description: This function is called back when control information is received 
 *              on any of the service connections
 * Returns:     void
 ******************************************************************************/
void _MDC_CtrlCB(int nType, ...)
{
    /* Local variables.
    */
    UINT        nChanId;
    UINT        nPortNo;
    ULNG        lIPaddr;
    va_list     pArgs;
    UCHAR       *szFunc = "_MDC_CtrlCB";
 
    /* Start var-arg list by making pArgs point to first arg in list.
    */
    va_start(pArgs, nType);
 
    /* What type of callback is it...?
    */
    switch(nType)
    {
        /* A new connection has arrived.
        */
        case SLC_NEWSERVICE:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log link going down event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "New Service with client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));
            break;
 
        /* A connection has been made
        */
        case SLC_CONNECT:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            if (nChanId == MDC.nPendConChanId)
            {
                MDC.nPendConChanId = 0;
                MDC.nLinkUp = TRUE;
                Lgr(LOG_DEBUG, szFunc,
                    "Connected to client: nChanId=%d, nPortNo=%d, IP=%s",
                    nChanId, nPortNo, SL_HostIPtoString(lIPaddr));
            } else
             {
                Lgr(LOG_DEBUG, szFunc,
                    "Rcvd unexpected connection: nChanId=%d, nPortNo=%d, IP=%s",
                    nChanId, nPortNo, SL_HostIPtoString(lIPaddr));
            }
            break;
 
        /* Given connection has become temporarily unavailable.
        */
        case SLC_LINKDOWN:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log link going down event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Link down to client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));

            /* Reset flag to indicate that we dont have a connection.
            */
            MDC.nLinkUp = FALSE;
            break;
 
        /* Given connection has died.
        */
        case SLC_LINKFAIL:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Clean up for Channel ID    
            */
            if (SL_DelClient(nChanId) != R_OK)
            {
                Lgr(LOG_DEBUG, szFunc, 
                    "SL_DelClient failed for Channel ID %d", nChanId);
            }

            /* Log link failure event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Link closed to client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));
            break;
 
        default:
            /* Log a message as this condition shouldnt occur.
            */
            Lgr(LOG_DEBUG, szFunc, "Unrecognised message type (%d)", nType);
            break;
    }
 
    /* Terminate var-arg list.
    */
    va_end(pArgs);
}

/******************************************************************************
 * Function:    _MDC_SendPacket
 * Description: Send a packet to a daemon
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_SendPacket(UINT nChanId, char cPacketType, UCHAR *psnzBuf, UINT nBuflen)
{
    char  *psnzPktMsgBuf;      /* buffer for preparing message to be sent */
    char  *psnzComBuf;         /* pointer to buffer containing compressed message */
    UCHAR *szFunc = "_MDC_SendPacket";
    UINT  nlocalBufLen;        /* Local buffer length variable */

    nlocalBufLen = nBuflen + 1;

    psnzPktMsgBuf = malloc(nlocalBufLen);
    if (psnzPktMsgBuf == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot malloc");
        return(MDC_FAIL);
    }
 
    /* Prepend packet type to service request data */
    *psnzPktMsgBuf = cPacketType;
    memcpy((psnzPktMsgBuf + 1), psnzBuf, (nlocalBufLen - 1));

    /* Log message that we are sending to ease debugging.
    */
    Lgr(LOG_DEBUG, szFunc, "Sending packet (Before Compress): Data=%s, Len=%d",
        psnzPktMsgBuf, nlocalBufLen);

    /* Compress packet to be sent
    */
    psnzComBuf = Compress(psnzPktMsgBuf, &nlocalBufLen);
    if (psnzComBuf == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot malloc for Compress");
        free(psnzPktMsgBuf);
        return(MDC_FAIL);
    }

    /* Log message that we are sending to ease debugging.
    */
    Lgr(LOG_DEBUG, szFunc,
        "Sending packet (After Compress): nChanId=%d, Data=%s, Len=%d",
        nChanId, psnzPktMsgBuf, nlocalBufLen);
 
    /* Send packet to the daemon */
    if (SL_SendData(nChanId, (UCHAR *) psnzComBuf, nlocalBufLen) != R_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "SL_SendData failed");
        free(psnzPktMsgBuf);
        if (psnzPktMsgBuf != psnzComBuf)
            free(psnzComBuf);
        return(MDC_FAIL);
    }

    free(psnzPktMsgBuf);
    if (psnzPktMsgBuf != psnzComBuf)
        free(psnzComBuf);

    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_GetSrvRqtReply
 * Description: Get reply to a service request packet
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_GetSrvRqtReply(UINT nChanId, char *pcPacketType)
{
    /* Local variables.
    */
    int     nTotalTime;
    UCHAR   *szFunc = "_MDC_GetSrvRqtReply";

    MDC.nPendSRChanId = nChanId;
   
    /* Wait for the service request reply
    */
    nTotalTime = 0;
    while (MDC.nSrvReqTimeout > nTotalTime && MDC.nPendSRChanId != 0)
    {
        SL_Poll((ULNG) SR_SLEEP_TIME);
        nTotalTime += SR_SLEEP_TIME;
    }
 
    if (MDC.nPendSRChanId != 0)
    {
        MDC.nPendSRChanId = 0;
        Lgr(LOG_DEBUG, szFunc, "Timed out waiting for service request reply");
        return(MDC_FAIL);
    }

    *pcPacketType = MDC.cReplyType;

    return(MDC_OK);
}

/******************************************************************************
 * Function:    _MDC_CreateChStatus
 * Description: Make a new Channel status structure and add to linked list
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_CreateChStatus(UINT nChanId)         /* Channel ID */
{
    UINT       nLocalChanId = nChanId;
    UCHAR      *szFunc = "_MDC_CreateChStatus";
    CHANSTATUS *psNewChanSt;
    
    /* Check that there is not already an entry for the Channel ID
    */
    if (FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL) != NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Channel ID %d already in use", nLocalChanId);
        return(MDC_FAIL);
    }

    psNewChanSt = (CHANSTATUS *) malloc(sizeof(CHANSTATUS));
    if (psNewChanSt == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot malloc");
        return(MDC_FAIL);
    }

    psNewChanSt->nChanId = nLocalChanId;
    psNewChanSt->State = MAKING_CONN;
    psNewChanSt->UserDataCB = NULL;

    if (AddItem(&MDC.spChanDetHead, &MDC.spChanDetTail, SORT_NONE, &nLocalChanId, 
                NULL, NULL, psNewChanSt) != R_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "AddItem failed for Channel ID %d",
            nLocalChanId);
        return(MDC_FAIL);
    }
 
    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_DelChStatus
 * Description: Delete an item from the Channel status linked list
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_DelChStatus(UINT nChanId)         /* Channel ID */
{
    UINT        nLocalChanId = nChanId;
    UCHAR       *szFunc = "_MDC_DelChStatus";
    CHANSTATUS  *ChanSt;

    if ((ChanSt = FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc,
            "FindItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }

    if (DelItem(&MDC.spChanDetHead, &MDC.spChanDetTail, NULL, &nLocalChanId, NULL, NULL) != R_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "DelItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }

    /* pds: 12/7/96. Ian forgot to tell the UX library that the channel had closed, so
     * the following statement sends a request to UX to close the channel.
    */
    SL_DelClient(nChanId);
    free(ChanSt);

    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_SetChState
 * Description: Set Channel State to the given value
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_SetChState( UINT nChanId,        /* Channel ID */
                        CHSTATE eNewState)   /* New state  */
{
    UINT        nLocalChanId = nChanId;
    UCHAR       *szFunc = "_MDC_SetChState";
    CHANSTATUS  *ChanSt;
 
    if ((ChanSt = FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc,
            "FindItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }
 
    ChanSt->State = eNewState;
 
    return(MDC_OK);
}
 

/******************************************************************************
 * Function:    _MDC_SetSRResult
 * Description: Set Channel Send Request result
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_SetSRResult( UINT nChanId,        /* Channel ID */
                         UINT bResult)        /* Send Request result */
{
    UINT        nLocalChanId = nChanId;
    UCHAR       *szFunc = "_MDC_SetSRResult";
    CHANSTATUS  *ChanSt;
 
    if ((ChanSt = FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc,
            "FindItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }
 
    ChanSt->bSendReqResult = bResult;
 
    return(MDC_OK);
}
 
 
/******************************************************************************
 * Function:    _MDC_GetSRResult
 * Description: Get Channel Send Request result
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_GetSRResult( UINT nChanId,         /* Channel ID */
                         UINT *bResult)        /* Send Request result */
{
    UINT        nLocalChanId = nChanId;
    UCHAR       *szFunc = "_MDC_GetSRResult";
    CHANSTATUS  *ChanSt;

    if ((ChanSt = FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc,
            "FindItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }

    *bResult = ChanSt->bSendReqResult;

    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_SetUserCB
 * Description: Set Send Request call back to the given value
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_SetUserCB( UINT nChanId,                           /* Channel ID */
                        void (*UserCB) (UINT, UCHAR *, UINT))  /* call back */
{
    UINT        nLocalChanId = nChanId;
    UCHAR       *szFunc = "_MDC_SetUserCB";
    CHANSTATUS  *ChanSt;
 
    if ((ChanSt = FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc,
            "FindItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }
 
    ChanSt->UserDataCB = UserCB;
 
    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_GetChState
 * Description: Get Channel State for a given channel
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_GetChState( UINT nChanId,      /* Channel ID */
                        CHSTATE *eState)   /* Channel state  */
{
    UINT        nLocalChanId = nChanId;
    UCHAR       *szFunc = "_MDC_GetChState";
    CHANSTATUS  *ChanSt;
 
    if ((ChanSt = FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc,
            "FindItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }
 
    *eState = ChanSt->State;
 
    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_GetNAKErrStr
 * Description: Get pointer to NAK error string
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_GetNAKErrStr( UINT nChanId,       /* Channel ID */
                          UCHAR **ppszErrStr) /* pointer to pointer to error string  */
{
    UINT        nLocalChanId = nChanId;
    UCHAR       *szFunc = "_MDC_GetNAKErrStr";
    CHANSTATUS  *ChanSt;
 
    if ((ChanSt = FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc,
            "FindItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }
 
    *ppszErrStr = ChanSt->pszErrStr;
 
    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_GetUserCB
 * Description: Get User call back for the channel
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_GetUserCB( UINT nChanId,                           /* Channel ID */
                       void (**UserCB) (UINT, UCHAR *, UINT))  /* User call back */
{
    UINT        nLocalChanId = nChanId;
    UCHAR       *szFunc = "_MDC_GetUserCB";
    CHANSTATUS  *ChanSt;
 
    if ((ChanSt = FindItem(MDC.spChanDetHead, &nLocalChanId, NULL, NULL)) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc,
            "FindItem failed: Channel ID %d not found", nLocalChanId);
        return(MDC_FAIL);
    }
 
    *UserCB = ChanSt->UserDataCB;
 
    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_PrintErrMsg
 * Description: Print error message text
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_PrintErrMsg( UCHAR *psnzErrMsg,       /* Error message none terminated */
                         UINT nBufLen)            /* Buffer Length  */
{
    char    *pszTmpBuf;
    UCHAR   *szFunc = "_MDC_PrintErrMsg";

    pszTmpBuf = (char *) malloc(nBufLen + 1);
    if (pszTmpBuf == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot malloc");
        return(MDC_FAIL);
    }

    memcpy(pszTmpBuf, psnzErrMsg, nBufLen);

    pszTmpBuf[nBufLen] = '\0';

    Lgr(LOG_DEBUG, "", "Error Message: %s", pszTmpBuf);
 
    free(pszTmpBuf);

    return(MDC_OK);
}


/******************************************************************************
 * Function:    _MDC_WaitOnSndReq
 * Description: Block until send request completes
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    _MDC_WaitOnSndReq(UINT nChanId)            /* Channel ID  */
{
    /* Local variables.
    */
    UCHAR          *szFunc = "_MDC_WaitOnSndReq";
    int            nTotalTime;
    int            bSendreqCom;    /* Indicates if the send request is complete */
    CHSTATE        eChanSt;
 
    /* Wait for send request to complete
    */
    nTotalTime = 0;
    bSendreqCom = FALSE;
    while (MDC.nSndReqTimeout > nTotalTime && bSendreqCom == FALSE)
    {
        SL_Poll((ULNG) SNDREQ_SLEEP_TIME);
        if (_MDC_GetChState(nChanId, &eChanSt) != MDC_OK)
        {
            Lgr(LOG_DEBUG, szFunc,
                "No status information for channel ID %d", nChanId);
            return(MDC_FAIL);
        }
        if(MDC.nLinkUp == FALSE)
        {
            Lgr(LOG_DEBUG, szFunc, "Link to daemon failed, aborting...");
            return(MDC_FAIL);
        }

        if (eChanSt == SEND_REQUEST_COMPLETE)
            bSendreqCom = TRUE;

        nTotalTime += SNDREQ_SLEEP_TIME;
    }
 
    return(MDC_OK);
}

/******************************************************************************
 * Function:    MDC_SendRequest
 * Description: Send a request to a driver
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    MDC_SendRequest( UINT    nChanId,     /* I: Channel to send message on */
                        UCHAR    *szData,    /* I: Data to send */
                        UINT    nDataLen,    /* I: Length of data */
                        void    (*DataCB) (UINT, UCHAR *, UINT))
                                             /* I: call back function for data */
{
    /* Local variables.
    */
    UCHAR       *szFunc = "MDC_SendRequest";
    CHSTATE     eCurrState;       /* Current State for Channel ID        */

    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK
    
    /* Check if in MDC Comms mode
    */
    if (MDC.nMDCCommsMode == FALSE)
    {
        Lgr(LOG_DEBUG, szFunc, "Not in MDC Comms Mode");
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }

    if (_MDC_GetChState(nChanId, &eCurrState) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot get status for channel %d", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    if (eCurrState != IDLE)
    {
        Lgr(LOG_DEBUG, szFunc, "Channel state not IDLE");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Store user call back function
    */
    if (_MDC_SetUserCB(nChanId, DataCB) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "_MDC_SetUserCB failed for Channel ID %d", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    if (_MDC_SetChState(nChanId, IN_SEND_REQUEST) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot set channel status to IDLE");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    if (_MDC_SendPacket(nChanId, MDC_PREQ, szData, nDataLen) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_SendPacket failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }
 
    /* Return success to caller as getting this far without hitch is success!
    */
    THREAD_UNLOCK_RETURN(MDC_OK);
}

/******************************************************************************
 * Function:    MDC_GetResult
 * Description: Wait for all replies to a send request and then return result
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    MDC_GetResult( UINT    nChanId,         /* I: Channel ID */
                      UCHAR   **ppszErrorMsg)  /* O: Associated error message */
{
    /* Local variables.
    */
    UCHAR        *szFunc = "MDC_GetResult";
    CHSTATE      eChanSt;
    UINT         bReqResult;

    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK

    /* Check if in MDC Comms mode
    */
    if (MDC.nMDCCommsMode == FALSE)
    {
        Lgr(LOG_DEBUG, szFunc, "Not in MDC Comms Mode");
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }

    if (_MDC_GetChState(nChanId, &eChanSt) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "No status information for channel ID %d", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    switch(eChanSt)
    {
        case SEND_REQUEST_COMPLETE:
            break;

        case IN_SEND_REQUEST:
            /* Block until to Send Request Completes
            */
            if(_MDC_WaitOnSndReq(nChanId) != MDC_OK)
            {
                Lgr(LOG_DEBUG, szFunc,
                    "_MDC_WaitOnSndReq failed for channel ID %d", nChanId);
                THREAD_UNLOCK_RETURN(MDC_FAIL);
            }
            break;

        default:
            Lgr(LOG_DEBUG, szFunc,
                "MDC_GetResult called when in state %c on channel %d", 
                eChanSt, nChanId);
            THREAD_UNLOCK_RETURN(MDC_FAIL);
            break;
    }

    if (_MDC_SetChState(nChanId, IDLE) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "Cannot set channel state to IDLE for channel ID %d", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }
   
    if (_MDC_GetSRResult(nChanId, &bReqResult) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "_MDC_GetSRResult failed for channel ID %d", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }
 
    if (_MDC_GetNAKErrStr(nChanId, ppszErrorMsg) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "_MDC_GetNAKErrStr failed for channel %d", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }
 
    if (bReqResult == FALSE)
        THREAD_UNLOCK_RETURN(MDC_SNDREQNAK);

    /* Return success to caller as getting this far without hitch is success!
    */
    THREAD_UNLOCK_RETURN(MDC_OK);
}

/******************************************************************************
 * Function:    MDC_GetStatus
 * Description: Returns a boolean that indicates whether the Send Request for
 *              given channel has completed.
 * Returns:     MDC_OK or MDC_FAIL
 ******************************************************************************/
int    MDC_GetStatus( UINT    nChanId,        /* I: Channel ID                 */
                      UINT    *bSndReqCom )   /* O: Indicates whether Send */
                                              /*    Request has completed */
{
    UCHAR       *szFunc = "MDC_GetStatus";
    CHSTATE     eChanSt;
 
    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK

    /* Check if in MDC Comms mode
    */
    if (MDC.nMDCCommsMode == FALSE)
    {
        Lgr(LOG_DEBUG, szFunc, "Not in MDC Comms Mode");
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }

    /* Call SL_Poll to process any outstanding messages
    */
    SL_Poll(0);

    if (_MDC_GetChState(nChanId, &eChanSt) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "No status information for channel ID %d", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    switch(eChanSt)
    {
        case SEND_REQUEST_COMPLETE:
            *bSndReqCom = TRUE;
            break;

        case IN_SEND_REQUEST:
            *bSndReqCom = FALSE;
            break;
 
        default:
            Lgr(LOG_DEBUG, szFunc,
                "MDC_GetStatus called when in state %c on channel %d",
                eChanSt, nChanId);
            THREAD_UNLOCK_RETURN(MDC_FAIL);
            break;
    }

    /* Return success to caller as getting this far without hitch is success!
    */
    THREAD_UNLOCK_RETURN(MDC_OK);
}

/******************************************************************************
 * Function:    MDC_CreateService
 * Description: Create a connection to a daemon so that service requests can be
 *              issued
 * Returns:     Channel ID, or negative error code
 ******************************************************************************/
int    MDC_CreateService( UCHAR             *szHostName,    /* I: Host for connect*/
                          UINT              *nPortNo,        /* I: Port host on */
                          SERVICEDETAILS    *serviceDet )    /* I: Service details */
{
    /* Statics.
    */
    static int    nProcessingFlag = FALSE;

    /* Local variables.
    */
    ULNG        lIPAddr;
    UINT        nServicesPortNo;
    UCHAR       *szFunc = "MDC_CreateService";
    int         nTotalTime;  /* total time waiting for connection to be made */
    int         ChanId;
    char        ReplyPktType;

    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK
    
    /* Check to see that we are not in the midst of a create service. If we are,
     * get out!
    */
    if( nProcessingFlag == TRUE )
    {
        Lgr(LOG_DEBUG, szFunc, "Not allowed to issue a %s from within a %s", 
            szFunc, szFunc);
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }

    /* Check if in MDC comms mode
    */
    if (MDC.nMDCCommsMode != TRUE)
    {
        Lgr(LOG_DEBUG, szFunc, "Not in MDC Comms Mode");
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }
 
    if (MDC.nPendCon == TRUE)
    {
        Lgr(LOG_DEBUG, szFunc, "Already an outstanding service request");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }
 
    if (SL_GetIPaddr(szHostName, &lIPAddr) != R_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "SL_GetIPaddr failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* OK, we are active, so set the processing flag to avoid being clobbered.
    */
    nProcessingFlag = TRUE;

    /* If the port number has not been given, then look one up.
    */
    if (nPortNo == NULL)
    {
        if (SL_GetService(DEF_SERVICENAME, &nServicesPortNo) != R_OK)
        {
            Lgr(LOG_DEBUG, szFunc, "SL_GetService failed");
            nProcessingFlag = FALSE;
            THREAD_UNLOCK_RETURN(MDC_FAIL);
        }
    }
    else
        nServicesPortNo = *nPortNo;

    if ((ChanId = SL_AddClient(nServicesPortNo, lIPAddr, szHostName,
                               _MDC_DataCB, _MDC_CtrlCB)) < 0)
    {
        Lgr(LOG_DEBUG, szFunc, "SL_AddClient failed");
        nProcessingFlag = FALSE;
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Create Status structure for the new Channel
    */
    if (_MDC_CreateChStatus(ChanId) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_CreateChStatus failed");
        nProcessingFlag = FALSE;
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    MDC.nPendConChanId = (UINT) ChanId;

    MDC.nPendCon = TRUE;
    
    /* Wait for an indication that the connection has been made 
    */
    nTotalTime = 0;
    while (MDC.nNewSrvTimeout > nTotalTime && MDC.nPendConChanId != 0)
    {
        SL_Poll((ULNG) CS_SLEEP_TIME);
        nTotalTime += CS_SLEEP_TIME;
    }

    MDC.nPendCon = FALSE;

    if (MDC.nPendConChanId != 0)
    {
        /* No connection made
        */
        Lgr(LOG_DEBUG, szFunc,
            "Timed out making connection to daemon at %s, port no %d", 
            szHostName, nServicesPortNo);
        MDC.nPendConChanId = 0;
        SL_DelClient((UINT) ChanId);
        _MDC_DelChStatus((UINT) ChanId);
        nProcessingFlag = FALSE;
        THREAD_UNLOCK_RETURN(MDC_NODAEMON);
    }

    if (_MDC_SetChState((UINT) ChanId, IN_SERVICE_REQUEST) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_SendData failed");
        SL_DelClient((UINT) ChanId);
        _MDC_DelChStatus((UINT) ChanId);
        nProcessingFlag = FALSE;
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Send service request structure to the daemon
    */
    if (_MDC_SendPacket((UINT) ChanId, MDC_INIT, (UCHAR *) serviceDet,
                        (UINT) sizeof(SERVICEDETAILS)) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_SendData failed");
        SL_DelClient((UINT) ChanId);
        _MDC_DelChStatus((UINT) ChanId);
        nProcessingFlag = FALSE;
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Get reply to service request.
    */
    if (_MDC_GetSrvRqtReply((UINT) ChanId, &ReplyPktType) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_GetSrvRqtReply failed");
        SL_DelClient((UINT) ChanId);
        _MDC_DelChStatus((UINT) ChanId);
        nProcessingFlag = FALSE;
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    if (ReplyPktType == MDC_NAK)
    {
        Lgr(LOG_DEBUG, szFunc, "NAK reply received for service request");
        SL_DelClient((UINT) ChanId);
        _MDC_DelChStatus((UINT) ChanId);
        nProcessingFlag = FALSE;
        THREAD_UNLOCK_RETURN(MDC_SERVICENAK);
    }

    if (_MDC_SetChState((UINT) ChanId, IDLE) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot set channel status to IDLE");
        SL_DelClient((UINT) ChanId);
        _MDC_DelChStatus((UINT) ChanId);
        nProcessingFlag = FALSE;
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    } 

    /* OK, finished processing successfully, so reset flag.
    */
    nProcessingFlag = FALSE;

    /* Get out, returning channel identifier of newly created service.
    */
    THREAD_UNLOCK_RETURN(ChanId);
}

/******************************************************************************
 * Function:    MDC_SetTimeout
 * Description: Function to program one of the system timeout values from the
 *              default to a user setting.
 * Returns:     MDC_OK     -    Setting changed.
 *              MDC_FAIL -    Setting couldnt be changed, see error message.
 ******************************************************************************/
int    MDC_SetTimeout( UCHAR       *pszWhichTimeout,   /* I: Timeout to set */
                       UINT        nTimeoutValue,      /* I: New value */
                       UCHAR       *pszErrMsg )        /* O: Error message */
{
    /* Local variables.
    */
    UINT        nReturn = MDC_OK;
    UCHAR       *szFunc = "MDC_SetTimeout";

    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK

    /* Simply compare the string given by the caller and match against known
     * strings. If we get a match, then set the timeout value to that given.
    */
    if(strcmp(pszWhichTimeout, NEW_SERVICE_TIMEOUT) == 0)
    {
        MDC.nNewSrvTimeout = nTimeoutValue;
    } else
    if(strcmp(pszWhichTimeout, SRV_REQ_TIMEOUT) == 0)
    {
        MDC.nSrvReqTimeout = nTimeoutValue;
    } else
    if(strcmp(pszWhichTimeout, SEND_REQ_TIMEOUT) == 0)
    {
        MDC.nSndReqTimeout = nTimeoutValue;
    } else
     {
        nReturn = MDC_FAIL;
    }

    /* Exit with result code.
    */
    THREAD_UNLOCK_RETURN(nReturn);
}

/******************************************************************************
 * Function:    MDC_CloseService
 * Description: Close a service channel
 * Returns:     MDC_FAIL or MDC_OK or MDC_BADCONTEXT
 ******************************************************************************/
int    MDC_CloseService(    UINT    nChanId    )    /* I: Channel to close */
{
    /* Local variables.
    */
    UCHAR        szTmpBuf[2];
    UCHAR       *szFunc = "MDC_CloseService";

    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK

    /* Prepare initial messages.
    */
    sprintf(szTmpBuf, "%c", MDC_EXIT);

    /* Check if in MDC Comms mode
    */
    if (MDC.nMDCCommsMode == FALSE)
    {
        Lgr(LOG_DEBUG, szFunc, "Not in MDC Comms Mode");
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }

    /* Send command to remote to initiate closedown procedures.
    */
    if (SL_BlockSendData(nChanId, (UCHAR *) szTmpBuf, strlen(szTmpBuf) != R_OK))
    {
        Lgr(LOG_DEBUG, szFunc,
            "Transmission of closedown message to client failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Give the UX library a slice of CPU so that it can tidy up.
    SL_Poll(MAX_TERMINATE_TIME);
    */

    /* Delete the channel.
    */
    if (_MDC_DelChStatus(nChanId) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_DelChStatus failed for Channel ID %d",
            nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* All done, return completion code to caller.
    */
    THREAD_UNLOCK_RETURN(MDC_OK);
}

/******************************************************************************
 * Function:    MDC_Start
 * Description: This is called to initialise the MDC Comms.
 * Returns:     MDC_FAIL or MDC_OK
 ******************************************************************************/
int    MDC_Start(    void    )
{
    /* Local variables.
    */
    int         ret;
    UCHAR       *szFunc = "MDC_Start";

    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK

    /* Setup logger mode.
    */
    Lgr(LOG_CONFIG, LGM_STDOUT, LOG_MESSAGE, "");

    if (_MDC_Init() != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_Init failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Check if MDC Comms is already initialised
    */
    if (MDC.nMDCCommsMode == TRUE)
    {
        Lgr(LOG_DEBUG, szFunc, "Already in MDC Comms Mode");
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }

    /* Initialise UX comms
    */
    if ((ret = SL_Init(MDC_CL_KEEPALIVE, (UCHAR *) NULL)) != R_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "SL_Init failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Now in MDC Comms Mode
    */
    MDC.nMDCCommsMode = TRUE;

    /* Return success to caller as getting this far without hitch is success!
    */
    THREAD_UNLOCK_RETURN(MDC_OK);
}

/******************************************************************************
 * Function:    MDC_End
 * Description: This is called to shutdown the MDC Comms.
 * Returns:     MDC_FAIL or MDC_OK
 ******************************************************************************/
int    MDC_End(    void    )
{
    /* Local variables.
    */
    int           ret;
    CHANSTATUS    *spChanDet;
    LINKLIST      *spNext;
    UCHAR         *szFunc = "MDC_End";
 
    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK

    /* Check if in MDC Comms mode
    */
    if (MDC.nMDCCommsMode == FALSE)
    {
        Lgr(LOG_DEBUG, szFunc, "Not in MDC Comms Mode");
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }

    /* Go through all current connections and close them if they are open.
    */
    for(spChanDet=(CHANSTATUS *)StartItem(MDC.spChanDetHead, &spNext);
        spChanDet != NULL;
        spChanDet=(CHANSTATUS *)NextItem(&spNext))
    {
        /* Force closure of connection.
        */
        MDC_CloseService(spChanDet->nChanId);
    }

    /* Final MDC termination tidy up.
    */
    if (_MDC_Terminate() != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_SL_Terminate failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Shutdown UX comms
    */
    if ((ret = SL_Exit((UCHAR *) NULL)) != R_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "SL_Exit failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Exit MDC Comms Mode
    */
    MDC.nMDCCommsMode = FALSE;

    /* Return success to caller as weve got to the end without hitch.
    */
    THREAD_UNLOCK_RETURN(MDC_OK);
}

/******************************************************************************
 * Function:    MDC_ChangeService
 * Description: Change Service for an existing daemon connection
 * Returns:     MDC_OK, MDC_FAIL, MDC_NODAEMON, MDC_NOSERVICE, MDC_BADPARMS
 ******************************************************************************/
int    MDC_ChangeService( UINT            nChanId,         /* I: Chan ID of srvc */
                          SERVICEDETAILS  *serviceDet )    /* I: Service details */
{
    /* Local variables.
    */
    UCHAR        *szFunc = "MDC_ChangeService";
    char        ReplyPktType;
    CHSTATE        eChanSt;

    /* If threading is enabled, then lock this function so that no other
     * thread can enter.
    */
    SINGLE_THREAD_LOCK
   
    /* Check if in MDC comms mode
    */
    if (MDC.nMDCCommsMode != TRUE)
    {
        Lgr(LOG_DEBUG, szFunc, "Not in MDC Comms Mode");
        THREAD_UNLOCK_RETURN(MDC_BADCONTEXT);
    }

    /* Check that in IDLE state on channel
    */
    if (_MDC_GetChState(nChanId, &eChanSt) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "No status information for channel ID %d", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    if (eChanSt != IDLE)
    {
        Lgr(LOG_DEBUG, szFunc, "Channel ID %d not in idle state", nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }
 
    if (_MDC_SetChState(nChanId, IN_CHANGE_SERVICE) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc,
            "Cannot set status to IN_CHANGE_SERVICE for channel ID %d",nChanId);
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Send service request structure to the daemon.
    */
    if (_MDC_SendPacket((UINT)nChanId, MDC_CHANGE, (UCHAR *) serviceDet,
                        (UINT) sizeof(SERVICEDETAILS)) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_SendPacket failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }
 
    /* Get reply to service request.
    */
    if (_MDC_GetSrvRqtReply((UINT)nChanId, &ReplyPktType) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "_MDC_GetSrvRqtReply failed");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    if (ReplyPktType == MDC_NAK)
    {
        Lgr(LOG_DEBUG, szFunc, "NAK reply received for service request");
        THREAD_UNLOCK_RETURN(MDC_SERVICENAK);
    }
 
    if (_MDC_SetChState((UINT)nChanId, IDLE) != MDC_OK)
    {
        Lgr(LOG_DEBUG, szFunc, "Cannot set channel status to IDLE");
        THREAD_UNLOCK_RETURN(MDC_FAIL);
    }

    /* Return new channel id, shouldnt really have changed.
    */
    THREAD_UNLOCK_RETURN(nChanId);
}
