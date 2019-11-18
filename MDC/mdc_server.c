/******************************************************************************
 * Product:       #     # ######   #####          #         ###   ######
 *                ##   ## #     # #     #         #          #    #     #
 *                # # # # #     # #               #          #    #     #
 *                #  #  # #     # #               #          #    ######
 *                #     # #     # #               #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                #     # ######   #####  ####### #######   ###   ######
 *
 * File:          mdc_server.c
 * Description:   Meta Data Communications API providing all functionality
 *                for server processes within the Virtual Data Warehouse
 *                project design.
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
#include    <sys/types.h>
#include    <unistd.h>
#undef      R_OK


/* Indicate that we are a C module for any header specifics.
*/
#define     MDC_SERVER_C

/* Bring in local specific header files.
*/
#include    "mdc.h"
#include    "mdc_common.h"

/******************************************************************************
 * Function:    _MDC_SendACK
 * Description: Function to send an acknowledge to the client in response to
 *              a data block received correctly or a request processed
 *              successfully.
 * 
 * Returns:     MDC_FAIL- Couldnt transmit an ACK message to the client.
 *              MDC_OK    - ACK sent successfully.
 ******************************************************************************/
int    _MDC_SendACK( void )
{
    /* Local variables.
    */
    int          nReturn = MDC_OK;
    UCHAR        szAckBuf[2];
    UCHAR        *szFunc = "_MDC_SendACK";

    /* Make sure that we have a valid channel connection in case of rogue
     * program code.
    */
    if( MDC.nClientChanId != 0 )
    {
        /* Build up the message to transmit.
        */
        sprintf(szAckBuf, "%c", MDC_ACK);

        /* Try and transmit it.
        */
        if(SL_BlockSendData(MDC.nClientChanId, szAckBuf, 1) == R_FAIL)
        {
            /* Log a message as this condition shouldnt occur.
            */
            Lgr(LOG_ALERT, szFunc, "Couldnt transmit an ACK packet");

            /* Set exit code to indicate failure.
            */
            nReturn = MDC_FAIL;
        }
    } else
     {
        /* Log a message as this condition shouldnt not occur!
        */
        Lgr(LOG_DEBUG, szFunc,
            "Trying to transmit an ACK to an unknown client!");

        /* Set exit code to indicate failure.
        */
        nReturn = MDC_FAIL;
    }

    /* All finished, exit with result code.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _MDC_SendNAK
 * Description: Function to send a negative acknowledge to the client in
 *              response to a data block which arrived incorrectly or a
 *              request which couldnt be processed successfully.
 * 
 * Returns:     MDC_FAIL- Couldnt transmit a NAK message to the client.
 *              MDC_OK    - NAK sent successfully.
 ******************************************************************************/
int    _MDC_SendNAK( UCHAR    *szErrMsg )    /* I: Error msg to send with NAK */
{
    /* Local variables.
    */
    UINT            nErrLen;
    int             nReturn = MDC_OK;
    UINT            nXmitLen;
    UCHAR           *psnzCmpBuf;
    UCHAR           *psnzTmpBuf;
    UCHAR           *szFunc = "_MDC_SendNAK";

    /* Make sure that we have a valid channel connection in case of rogue
     * program code.
    */
    if( MDC.nClientChanId != 0 )
    {
        /* Calculate length of error message buffer for use in later
         * memory manipulations.
        */
        nErrLen = strlen(szErrMsg);

        /* Allocate enough memory to hold a message id and the data prior
         * to transmission.
        */
        if((psnzTmpBuf=(UCHAR *)malloc(nErrLen+2)) == NULL)
        {
            /* Log a message as this condition shouldnt occur.
            */
            Lgr(LOG_DEBUG, szFunc, "Couldnt allocate (%d) bytes memory",
                nErrLen+2);

            /* Set exit code to indicate failure.
            */
            nReturn = MDC_FAIL;
        } else
         {
            /* Log the message if we are in debug mode.
            */
            Lgr(LOG_DEBUG, szFunc, "%s", szErrMsg);

            /* Build up the message to transmit.
            */
            *psnzTmpBuf = (UCHAR)MDC_NAK;
            memcpy(psnzTmpBuf+1, szErrMsg, nErrLen);

            /* Compress it to save on transmission overheads.
            */
            nXmitLen=nErrLen+1;
            if((psnzCmpBuf=Compress(psnzTmpBuf, &nXmitLen)) != NULL)
            {
                /* Free up memory we used to store the original message.
                */
                if(psnzCmpBuf != psnzTmpBuf)
                {
                    free(psnzTmpBuf);

                    /* Make our temporary pointer point to the new compressed
                     * memory block.
                    */
                    psnzTmpBuf = psnzCmpBuf;
                }
            }

            /* Try and transmit it.
            */
            if(SL_BlockSendData(MDC.nClientChanId,psnzTmpBuf,nXmitLen) ==R_FAIL)
            {
                /* Log a message as this condition shouldnt occur.
                */
                Lgr(LOG_ALERT, szFunc, "Couldnt transmit NAK message");

                /* Set exit code to indicate failure.
                */
                nReturn = MDC_FAIL;
            }

            /* Free up used memory, no longer needed.
            */
            free(psnzTmpBuf);
        }
    } else
     {
        /* Log a message as this condition shouldnt not occur!
        */
        Lgr(LOG_DEBUG, szFunc,
            "Trying to transmit a NAK to an unknown client!");

        /* Set exit code to indicate failure.
        */
        nReturn = MDC_FAIL;
    }

    /* All finished, exit with success.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _MDC_ServerCntlCB
 * Description: A function to handle any communications control callbacks
 *              that are generated as a result of MDC_Server being executed.
 * 
 * Returns:     No Returns.
 ******************************************************************************/
void    _MDC_ServerCntlCB( int     nType,    /* I: Type of callback */
                            ... )            /* I: Arg list according to type */
{
    /* Local variables.
    */
    UINT        nChanId;
    UINT        nPortNo;
    ULNG        lIPaddr;
    va_list     pArgs;
    UCHAR       *szFunc = "_MDC_ServerCntlCB";

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
            /* Extract var args which indicate who the client is.
            */
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log a message to indicate who the client is.
            */
            Lgr(LOG_DEBUG, szFunc,
                "New service: PPID=%d, PID=%d, Chan=%d, Port=%d, IPaddr=%s",
                getppid(), getpid(), nChanId, nPortNo, SL_HostIPtoString(lIPaddr));

            /* Store the Channel Id of the client so that it can be used
             * for async communications.
            */
            MDC.nClientChanId = nChanId;
            break;

        /* Given connection has become temporarily unavailable.
        */
        case SLC_LINKDOWN:
            /* Extract var args which indicate who the client is.
            */
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log a message to indicate that the link has gone down.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Link temporarily down, Chan=%d, Port=%d, IPaddr=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));
            break;

        /* Given connection has died.
        */
        case SLC_LINKFAIL:
            /* Extract var args which indicate who the client is.
            */
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Post a signal to indicate that we are about to shut down.
            */
            if(MDC.nCloseDown == FALSE)
            {
                /* Call out of band processing mechanism.
                */
                MDC.fCntrlCB(MDC_EXIT);

                /* Setup exit flag so that we can terminate, as failure of
                 * our main connection port means we should exit.
                */
                MDC.nCloseDown = TRUE;
            }

            /* Log a message to indicate that the link has gone down.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Link closed or failed, Chan=%d, Port=%d, IPaddr=%s",
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

    /* Return to caller.
    */
    return;
}

/******************************************************************************
 * Function:    _MDC_ServerDataCB
 * Description:    A function to handle any data callbacks that are generated as
 *                a result of data arriving during an MDC_Server execution.
 * 
 * Returns:        No Returns.
 ******************************************************************************/
void    _MDC_ServerDataCB(  UINT        nChanId,    /* I: Channel data rcv on*/
                            UCHAR       *szData,    /* I: Rcvd data */
                            UINT        nDataLen )  /* I: Rcvd data length */
{
    /* Local variables.
    */
    UINT        nDLen = nDataLen;
    FIFO        *psFifo;
    UCHAR       *szFunc = "_MDC_ServerDataCB";

    /* Special case processing for out of bands message. Normally messages
     * are queued in a FIFO awaiting processing, but certain messages must be
     * acted upon as soon as they arrive.
    */
    if(nDataLen == 1)
    {
        /* Process according to type of message.
        */
        switch(szData[0])
        {
            /* Special message which only has meaning when the daemon is
             * returning data to the caller. Its actions are to basically 
             * force the driver to stop sending data and cancel all further
             * data that may be awaiting transmission.
            */
            case MDC_ABORT:
                /* Initiate abort mechanism.
                */
                MDC.fCntrlCB(MDC_ABORT);
                return;
    
            case MDC_EXIT:
                /* Initiate closedown mechanism.
                */
                MDC.fCntrlCB(MDC_EXIT);
                MDC.nCloseDown = TRUE;
                return;

            default:
                break;
        }
    }

    /* Allocate memory to store a FIFO carrier. This FIFO carrier is then
     * populated with the FIFO data below.
    */
    if( (psFifo=(FIFO *)malloc(sizeof(FIFO))) == NULL )
    {
        /* Log a message if needed.
        */
        Lgr(LOG_DEBUG, szFunc, "Memory exhausted, couldnt create FIFO carrier");

        /* Send a NAK to client to indicate that we've run out of memory.
        */
        if( _MDC_SendNAK("Memory exhausted on server, packet rejected (1)")
                                                                == MDC_FAIL )
        {
            /* Log a message if needed.
            */
            Lgr(LOG_ALERT, szFunc,
                "Couldnt send a NAK message, Houston we have problems!!");
        }

        /* Get out, nothing more can be done.
        */
        return;
    }

    /* Data block arrives in a compressed format, so uncompress prior to
     * placing it into the incoming FIFO list.
    */
    if( (psFifo->pszData=Decompress(szData, &nDLen)) == NULL )
    {
        /* Log a message if needed.
        */
        Lgr(LOG_DEBUG, szFunc, "Couldnt decompress buffer, Chan (%d), Len (%d)",
            nChanId, nDLen);

        /* Send a NAK to client to indicate that the buffer sent cant be
         * processed.
        */
        if( _MDC_SendNAK("Couldnt decompress buffer, memory problems!")
                                                                == MDC_FAIL )
        {
            /* Log a message if needed.
            */
            Lgr(LOG_ALERT, szFunc,
                "Couldnt send a NAK message, Houston we have problems!!");
        }

        /* Free up used resources and get out as nothing more can be done.
        */
        free(psFifo);
        return;
    }

    /* Complete the FIFO carrier information set.
    */
    psFifo->nDataLen = nDLen;

    /* Log message as to what has been received, may help track bugs.
    */
    Lgr(LOG_DEBUG, szFunc,
        "Message received: Data=%s, Len=%d", psFifo->pszData, psFifo->nDataLen);

    /* Add buffer to the end of the incoming FIFO list.
    */
    if( AddItem(&MDC.spIFifoHead, &MDC.spIFifoTail, SORT_NONE, NULL, NULL, NULL,
                psFifo) == R_FAIL )
    {
        /* Log a message if needed.
        */
        Lgr(LOG_DEBUG, szFunc,
            "Memory exhausted, couldnt add packet onto the FIFO link list");

        /* Send a NAK to client to indicate that we've run out of memory.
        */
        if(_MDC_SendNAK("Memory exhausted on server, packet rejected (2)")
                                                                == MDC_FAIL )
        {
            /* Log a message if needed.
            */
            Lgr(LOG_ALERT, szFunc,
                "Couldnt send a NAK message, Houston we have problems!!");
        }

        /* Free up used resources and get out as nothing more can be done.
        */
        free(psFifo->pszData);
        free(psFifo);
        return;
    }

    /* Return to caller.
    */
    return;
}

/******************************************************************************
 * Function:    MDC_Server
 * Description: Entry point into the Meta Data Communications for a server
 *              process. This function initialises all communications etc
 *              and then runs the given user callback to perform any required
 *              actions.
 * 
 * Returns:     MDC_FAIL- Function terminated due to a critical error, see
 *              Errno for exact reason code.
 *              MDC_OK    - Function completed successfully without error.
 ******************************************************************************/
int    MDC_Server( UINT        *nPortNo,             /* I: TCP/IP port number */
                   UCHAR       *szService,           /* I: Name of TCP/IP Service */
                   int         (*fLinkDataCB)        /* I: User function callback*/
                   (UCHAR *, int, UCHAR *),
                    void       (*fControlCB)(UCHAR)  /* I: User control callback */
                 )
{
    /* Local variables.
    */
    FIFO        *psFifo;
    LINKLIST    *psNext;
    UINT        nServicePort;
    static int  nInitialised = FALSE;
    int         nReturn;
    UCHAR       *szFunc = "MDC_Server";

    /* Check to see that we are not being called twice... some people may
     * be idiotic.
    */
    if( nInitialised == TRUE )
    {
        /* Log a message if needed.
        */
        Lgr(LOG_DEBUG, szFunc, "Attempt to initialise function more than once");
        
        /* Exit as nothing more can be done.
        */
        return(MDC_FAIL);
    }

    /* Initialise the library, exiting if we fail.
    */
    if( (nReturn=_MDC_Init()) == MDC_FAIL )
    {
        /* Log a message if needed.
        */
        Lgr(LOG_DEBUG, szFunc, "Failed to initialise MDC Library");
        
        /* Exit as nothing more can be done.
        */
        return(nReturn);
    }

    /* Do we need to work out the TCP port number by looking in /etc/services?
    */
    if( nPortNo == NULL )
    {
        /* Work out the port number by a lookup on the given service name.
        */
        if( szService == NULL ||
            SL_GetService(szService, &nServicePort) == R_FAIL )
        {
            /* Log a message if needed.
            */
            Lgr(LOG_ALERT, szFunc,
                "Service (%s) does not exist in /etc/services", szService);

            /* Exit as nothing more can be achieved.
            */
            return(MDC_FAIL);
        }
    } else
     {
        /* Store the port number provided into a common variable for the
         * add server command.
        */
        nServicePort = *nPortNo;
    }

    /* Add a service port so that we can accept incoming TCP connections.
    */
    if( SL_AddServer(nServicePort, TRUE, _MDC_ServerDataCB, _MDC_ServerCntlCB)
                                                                == R_FAIL )
    {
        /* Log a message if needed.
        */
        Lgr(LOG_ALERT, szFunc, "Couldnt add service on port %d", nServicePort);

        /* Exit, nothing more that can be done.
        */
        return(MDC_FAIL);
    }

    /* Save the control callback within MDC structure so out of band
     * control messages can call the function directly.
    */
    MDC.fCntrlCB = fControlCB;

    /* Where initialised, so set the flag to prevent further re-entry.
    */
    nInitialised = TRUE;

ML_Init(8080, MON_SERVICE_HTML, "MDC Server/2.2.aaa", NULL, NULL, NULL);

    /* Now enter a tight loop, performing CPU scheduling in a non-preemptive
     * fashion and handling events as and when they occur.
    */
    do {
        /* Call the UX scheduler to ensure comms events occur.
        */
        SL_Poll(DEF_POLLTIME);

        /* Any packets on the incoming FIFO? If there are, then grab the
         * first one.
        */
        if( (psFifo=StartItem(MDC.spIFifoHead, &psNext)) != NULL )
        {
            /* Delete the entry off the FIFO before working with it to
             * guarantee that we never call the user twice with the same
             * data.
            */
            if(DelItem(&MDC.spIFifoHead, &MDC.spIFifoTail, psFifo, NULL, NULL,
                        NULL) == R_FAIL)
            {
                /* Log a message if needed.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "Couldnt delete first FIFO entry, will retry later..");
            } else
             {
                /* OK, weve got the data and we know its no longer on the FIFO,
                 * so lets call the users callback with this data.
                */
                MDC.szErrMsg[0] = '\0';
                if(fLinkDataCB(psFifo->pszData, psFifo->nDataLen, MDC.szErrMsg) 
                                                                == MDC_FAIL)
                {
                    /* The callback failed, need to send a NAK to the client
                     * plus the provided error message.
                    */
                    if( _MDC_SendNAK(MDC.szErrMsg) == MDC_FAIL )
                    {
                        /* Log a message if needed.
                        */
                        Lgr(LOG_ALERT, szFunc,
                            "Couldnt send a NAK message, we have problems!!");
                    }
                } else
                 {
                    /* The callback succeeded, so send out an ACK to the client
                     * to let him continue on his merry way.
                    */
                    if( _MDC_SendACK() == MDC_FAIL )
                    {
                        /* Log a message to indicate problem.
                        */
                        Lgr(LOG_ALERT, szFunc,
                            "Couldnt send an ACK to the client...");
                    }
                }
            }
        }
    } while( MDC.nCloseDown == FALSE );

    /* Where exitting cleanly, so toggle flag so that a new entry can
     * succeed.
    */
    nInitialised = FALSE;

    /* Return result to caller.
    */
    return(MDC_OK);
}

/******************************************************************************
 * Function:    MDC_ReturnData
 * Description: Function called by user code to return any required data
 *              to a connected client. This function can only be called
 *              in response to a callback 'from the MDC layer to the user
 *              code' which has provided data.
 * 
 * Returns:     MDC_FAIL- An error occurred in transmitting the given data
 *                        block to the client process, see Errno for exact
 *                        reason code.
 *              MDC_OK    - Data packet was transmitted successfully.
 ******************************************************************************/
int    MDC_ReturnData( UCHAR    *snzDataBuf,    /* I: Data to return */
                       int      nDataLen        /* I: Length of data */
                     )
{
    /* Local variables.
    */
    int      nReturn = MDC_OK;
    UINT     nXmitLen;
    UCHAR    *psnzCmpBuf;
    UCHAR    *psnzTmpBuf;
    UCHAR    *szFunc = "MDC_ReturnData";

    /* Make sure that we have a valid channel connection in case of rogue
     * program code.
    */
    if( MDC.nClientChanId != 0 )
    {
        /* Allocate enough memory to hold a message id and the data prior
         * to transmission.
        */
        if((psnzTmpBuf=(UCHAR *)malloc(nDataLen+1)) == NULL)
        {
            /* Log a message as this condition shouldnt occur.
            */
            Lgr(LOG_DEBUG, szFunc, "Couldnt allocate (%d) bytes memory",
                nDataLen+1);

            /* Set exit code to indicate failure.
            */
            nReturn = MDC_FAIL;
        } else
         {
            /* Build up the message to transmit.
            */
            *psnzTmpBuf = (UCHAR)MDC_DATA;
            memcpy(psnzTmpBuf+1, snzDataBuf, nDataLen);

            /* Compress it to save on transmission overheads.
            */
            nXmitLen=nDataLen+1;
            if((psnzCmpBuf=Compress(psnzTmpBuf, &nXmitLen)) != NULL)
            {
                /* Free up memory we used to store the original message.
                */
                if(psnzCmpBuf != psnzTmpBuf)
                {
                    free(psnzTmpBuf);

                    /* Make our temporary pointer point to the new compressed
                     * memory block.
                    */
                    psnzTmpBuf = psnzCmpBuf;
                }
            }

            /* Try and transmit the new buffer.
            */
            if(SL_BlockSendData(MDC.nClientChanId,psnzTmpBuf,nXmitLen) ==R_FAIL)
            {
                /* Log a message as this condition shouldnt occur.
                */
                Lgr(LOG_ALERT, szFunc, "Couldnt transmit data packet");

                /* Set exit code to indicate failure.
                */
                nReturn = MDC_FAIL;
            }

            /* Free up used memory, no longer needed.
            */
            free(psnzTmpBuf);
        }
    } else
     {
        /* Log a message as this condition shouldnt not occur!
        */
        Lgr(LOG_DEBUG, szFunc,
            "Trying to transmit a data packet to an unknown client!");

        /* Set exit code to indicate failure.
        */
        nReturn = MDC_FAIL;
    }

    /* Return result to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    MDC_TimerCB
 * Description: Function to allow user code to register a callback event
 *              which is activated upon a timer expiring. The user provides
 *              the frequency and a function to callback and the MDC 
 *              schedules it.
 * 
 * Returns:     No Return Values.
 ******************************************************************************/
int    MDC_TimerCB( ULNG    lTimePeriod,     /* I: CB Time in Millseconds */
                    UINT    nEnable,         /* I: Enable(TRUE)/Dis(FALSE) CB*/
                    UINT    nAstable,        /* I: Astable(TRUE) or Mono CB */
                    void    (*fTimerCB)(void)/* I: Function to call back */
                  )
{
    /* Local variables.
    */
    int        nReturn;
    int        nTimerMode;
    UCHAR      *szFunc = "MDC_TimerCB";

    /* Initialise the library, if needed.
    */
    if( (nReturn=_MDC_Init()) == MDC_FAIL )
    {
        /* Log a message if needed.
        */
        Lgr(LOG_DEBUG, szFunc, "Failed to initialise MDC Library");
        
        /* Exit as nothing more can be done.
        */
        return(nReturn);
    }

    /* Work out the UX timer mode with given parameters.    
    */
    if( nEnable == TRUE )
    {
        if( nAstable == TRUE )
        {
            nTimerMode = TCB_ASTABLE;
        } else
         {
            nTimerMode = TCB_FLIPFLOP;
        }
    } else
     {
        nTimerMode = TCB_OFF;
    }

    /* Call UX to add the timed event.
    */
    if( SL_AddTimerCB( lTimePeriod, nTimerMode, 0, fTimerCB ) == R_FAIL )
    {
        /* Log a message if needed.
        */
        Lgr(LOG_DEBUG, szFunc, "Failed to add timer CB, Time=%ld, Mode=%d",
            lTimePeriod, nTimerMode );
        
        /* Exit as nothing more can be done.
        */
        return(MDC_FAIL);
    }

    /* Return result to caller.
    */
    return(MDC_OK);
}
