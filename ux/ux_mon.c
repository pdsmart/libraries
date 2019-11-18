/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_mon.c
 * Description:   Interactive Monitor functionality. Provides a suite of 
 *                interactive commands (HTML or Natural Language) that a user
 *                can issue to an executing application that incorporates
 *                these facilities.
 *
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1994-2019.
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

#if defined(_WIN32)
#include    <winsock.h>
#include    <time.h>
#include    <errno.h>
#include    <io.h>
#include    <fstream.h>
#endif

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
#include    <errno.h>
#include    <sys/socket.h>
#include    <netdb.h>
#include    <sys/time.h>
#include    <netinet/in.h>
#include    <sys/wait.h>
#include    <string.h>
#include    <unistd.h>
#endif

#if    defined(SOLARIS)
#include    <sys/file.h>
#endif

#include    <sys/timeb.h>
#include    <sys/stat.h>

/* Indicate that we are a C module for any header specifics.
*/
#define        UX_MONITOR_C

/* Bring in specific header files.
*/
#include    "ux.h"

/* Local module variables.
*/
static ML_GLOBALS    Ml;

/******************************************************************************
 * Internal Library functions.
 ******************************************************************************/

/******************************************************************************
 * Function:    _ML_HTMLDefaultCB
 * Description: A default handler for the HTML interpreter which basically
 *              sends a not-coded message to the client.
 *                 
 * Returns:     Non.
 ******************************************************************************/
int     _ML_HTMLDefaultCB( UINT        nChanId,        /* I: Id - xmit to client*/
                           UCHAR       *szData,        /* I: Data Buffer */
                           UINT        nDataLen )      /* I: Length of data buf */
{
    /* Local variables.
    */
    UCHAR           szTmpBuf[MAX_TMPBUFLEN+1];
    char            *szFunc = "_ML_HTMLDefaultCB";

    /* Create a basic message to inform client that the command has not
     * yet been implemented.
    */
    sprintf(szTmpBuf, "<TITLE>Not Implemented</TITLE>"
            "<H1>Not Implemented</H1>\n"
            "No Handler has been implemented for the issued command. "
            "Please contact the developer of this software and decide upon"
            " appropriate action.\n\n\n"
            "<H1>Have a Nice Day!</H1>\n\n");

    /* Fire it off to the client, logging any errors as required.
    */
    if(ML_Send(nChanId, szTmpBuf, strlen(szTmpBuf)) == R_FAIL)
    {
        /* Log error and setup exit code.
        */
        Lgr(LOG_DEBUG, szFunc,
            "Unable to xmit negative response to client on channel (%d)",
            nChanId);
    }

    /* All done, get out!
    */
    return( R_OK );
}

/******************************************************************************
 * Function:    _ML_InterpretHTMLRequest
 * Description: Buffer from an external client (ie. Web Browser) contains a
 *              an HTML request. Interpret it into a set of actions through
 *              comparisons and deductions.
 * Returns:     Non.
 ******************************************************************************/
UINT _ML_InterpretHTMLRequest( ML_MONLIST    *spMon,        /* I: Monitor Desc */
                               ML_CONLIST    *spCon,        /* I: Connection Desc */
                               UCHAR         *szData    )   /* I: Command Buffer */
{
    /* Local variables.
    */
    UINT            nPos = 0;
    UINT            nTokType;
    UCHAR           *spTmpTok = NULL;
    UCHAR           szTmpBuf[MAX_TMPBUFLEN+1];
    LINKLIST        *spNext = NULL;
    ML_COMMANDLIST  *spMC = NULL;
    char            *szFunc = "_ML_InterpretHTMLRequest";

    /* Allocate temporary working buffers.
    */
    if( (spTmpTok=(UCHAR *)malloc(strlen(szData)+1)) == NULL )
    {
        if(spTmpTok != NULL) free(spTmpTok);

        /* Get out with a failure, memory exhausted.
        */
        return(R_FAIL);
    }

    /* Get first item in buffer.
    */
    nTokType = ParseForToken(szData, &nPos, spTmpTok);

    /* Get the first token from the HTML buffer and check to see if its
     * within the recognised command set.
    */
    for(spMC=(ML_COMMANDLIST *)StartItem(spMon->spMCHead, &spNext);
        spMC != NULL; spMC=(ML_COMMANDLIST *)NextItem(&spNext))
    {
        /* Match?
        */
        if(StrCaseCmp(spMC->szCommand, spTmpTok) == 0)
        {
            /* Send positive header to browser client.
            */
            sprintf(szTmpBuf, "HTTP/1.0 200 OK\nServer: %s\n",
                    spMon->szServerName);
            if(ML_Send(spCon->nChanId, szTmpBuf, strlen(szTmpBuf)) == R_FAIL)
            {
                /* Log error and setup exit code.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "Unable to xmit positive response to client on chan (%d)",
                    spCon->nChanId);
            } else
             {
                /* Client setup, call callback to actually do all the
                 * processing.
                */
                if(spMC->nCallback != NULL)
                    spMC->nCallback(spCon->nChanId,&szData[nPos], spMon->nMonPort);
            }
            break;
        }
    }

    /* If we got to the end of the list without a match then we have an 
     * unrecognised command, log it and send fail message.
    */
    if(spMC == NULL)
    {
        /* Send negative header to browser client.
        */
        sprintf(szTmpBuf, "HTTP/1.0 400 OK\nServer: %s\n\n"
                "<TITLE>Not Found</TITLE><H1>%s - Not Found</H1>\n"
                "The requested object does not exist on this server.\n\n",
                spMon->szServerName, spMon->szServerName);
        if(ML_Send(spCon->nChanId, szTmpBuf, strlen(szTmpBuf)) == R_FAIL)
        {
            /* Log error and setup exit code.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Unable to xmit negative response to client on channel (%d)",
                spCon->nChanId);
        }
    }

    /* Success, exit.
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    _ML_InterpretNLRequest
 * Description: Buffer from an external client contains a natural language
 *              command request. Interpret it into a set of actions through
 *              comparisons.
 * Returns:     Non.
 ******************************************************************************/
UINT _ML_InterpretNLRequest( ML_MONLIST    *spMon,        /* I: Monitor Desc */
                             ML_CONLIST    *spCon,        /* I: Connection Desc */
                             UCHAR         *szData    )   /* I: Command Buffer */
{
    /* Local variables.
    */
    UINT            nFound;
    UINT            nPos = 0;
    UINT            nResult;
    UINT            nTokType;
    UCHAR           *spTmpTok = NULL;
    LINKLIST        *spNext = NULL;
    ML_COMMANDLIST  *spMC = NULL;

    /* Allocate temporary working buffers.
    */
    if( (spTmpTok=(UCHAR *)malloc(strlen(szData)+1)) == NULL )
    {
        if(spTmpTok != NULL) free(spTmpTok);

        /* Get out with a failure, memory exhausted.
        */
        return(R_FAIL);
    }

    /* Get first item in buffer.
    */
    nTokType = ParseForToken(szData, &nPos, spTmpTok);

    /* If the first parameter is not a alpha or alphanum, then invalid,
     * get out.
    */
    if(nTokType != TOK_ALPHA)
    {
        /* Bad input, warn remote and get out.
        */
        SL_BlockSendData(spCon->nChanId, "*** Input Invalid ***\n", 22);
        return(R_FAIL);
    }

    /* Check amount of data entered, cant be greater than internal 
     * buffers, cos a) it would overflow, b) its meaningless.
    */
    if(strlen(&szData[nPos]) >= MAX_TMPBUFLEN)
    {
        /* Line too long, warn remote and get out.
        */
        SL_BlockSendData(spCon->nChanId, "*** Line too Long ***\n", 22);
        return(R_FAIL);
    }

    /* Try and locate command in link list.
    */
    for(nFound=FALSE, spMC=StartItem(spMon->spMCHead, &spNext);
        spMC != NULL; spMC=NextItem(&spNext))
    {
        /* Have we located the required command.
        */
        if(strncasecmp(spMC->szCommand, spTmpTok, 3) == 0 &&
           spMC->nCallback != NULL)
        {
            nFound = TRUE;
            spMC->nCallback(spCon->nChanId,&szData[nPos],strlen(&szData[nPos]));
        }
    }

    /* If the command was not located... Send a default error message.
    */
    if(nFound == FALSE)
    {
        SL_BlockSendData(spCon->nChanId, "*** Illegal Command ***\n", 24);
        return(R_FAIL);
    }

    /* Exit with success.
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    _ML_MonitorCB
 * Description: When an external client is issuing commands to us, the command
 *              data is delivered to this function for processing.
 * Returns:     Non.
 ******************************************************************************/
void _ML_MonitorCB( UINT    nChanId,    /* I: Channel data received on */
                    UCHAR   *szData,    /* I: Buffer containing data */
                    UINT    nDLen )     /* I: Length of data in buffer */
{
    /* Local variables.
    */
    UINT        nResult;
    UCHAR       *spTmpData = NULL;
    UCHAR       *spTmpTok = NULL;
    UCHAR       *szFunc="_ML_MonitorCB";
    LINKLIST    *spNext = NULL;
    ML_CONLIST  *spCon;
    ML_MONLIST  *spMon;

    /* Locate the Monitor Descriptor record corresponding to this 
     * request.
    */
    for(spMon=(ML_MONLIST *)StartItem(Ml.spMonHead, &spNext);
        spMon != NULL && (spCon=(ML_CONLIST *)FindItem(spMon->spConHead, &nChanId, NULL, NULL)) == NULL;
        spMon=(ML_MONLIST *)NextItem(&spNext));

    /* Error condition? Without the record, carrying on is pointless.
    */
    if(spMon == NULL)
    {
        /* Log error and setup exit code.
        */
        Lgr(LOG_DEBUG, szFunc,
            "Unable to locate Monitor Descriptor record for ChanId (%d)",
            nChanId);
        return;
    }

    /* Allocate temporary working buffers.
    */
    if( (spTmpData=(UCHAR *)malloc(nDLen+1)) == NULL )
    {
        if(spTmpData != NULL) free(spTmpData);

        /* Get out, we must waste this request as we are in a situation where
         * the machine is dying or weve reached our memory quota, either case
         * it requires external rectification.
        */
        return;
    }

    /* Copy data and add a terminator.
    */
    strncpy(spTmpData, szData, nDLen);
    spTmpData[nDLen] = '\0';

    /* Has the application provided its own interpreter for received data
     * buffers? If it has, use it, else use in-built interpreter.
    */
    if(spMon->nInterpretOvr != NULL)
    {
        /* Pass all received data over to application handler.
        */
        nResult=spMon->nInterpretOvr(spCon->nChanId, spMon->nMonPort,
                                     spTmpData);
    } else
     {
        /* Process data according to the service type in operation.
        */
        switch(spMon->nServiceType)
        {
            case MON_SERVICE_HTML:
                nResult=_ML_InterpretHTMLRequest(spMon, spCon, spTmpData);

                /* For an HTML connection, once the request has been processed
                 * its use comes to an end, so close it and clean up 
                 * resources used.
                 * Post a close message to the Socket Library to close and
                 * remove the connection.
                */
                if(SL_Close(nChanId) == R_FAIL)
                {
                    /* A problem has occurred, weve either remembered the
                     * wrong channel Id, or corruption is occuring.
                    */
                    Lgr(LOG_DEBUG, szFunc,
                        "Cannot close socket with channel Id = %d", nChanId);
                }
                break;

            case MON_SERVICE_NL:
                nResult=_ML_InterpretNLRequest(spMon, spCon, spTmpData);
                break;

            default:
                break;
        }
    }

    /* Free up buffers.
    */
    if(spTmpData != NULL) free(spTmpData);
    if(spTmpTok != NULL) free(spTmpTok);

    /* Finished, get out!
    */
    return;
}

/******************************************************************************
 * Function:    _ML_MonitorCntrl
 * Description: Interactive monitor control function. WHen a connection is 
 *              mad or broken with an external client, this function is
 *              requested to handle it.
 * Returns:     Non.
 ******************************************************************************/
void _ML_MonitorCntrl( int    nType,    /* I: Type of callback */
                       ... )            /* I: Argument list according to type */
{
    /* Local variables.
    */
    UINT                nChanId;
    UINT                nOurPortNo;
    UINT                nPortNo;
    ULNG                lIPaddr;
    UCHAR                szTmpBuf[MAX_TMPBUFLEN];
    UCHAR                *szFunc="_ML_MonitorCntrl";
    va_list                pArgs;
    ML_CONLIST            *spCon;
    ML_MONLIST            *spMon;

    /* Start var-arg list by making pArgs point to first arg in list.
    */
    va_start(pArgs, nType);

    /* What type of callback is it...?
    */
    switch(nType)
    {
        case SLC_NEWSERVICE:
            /* Extract the relevant details of the new connection.
            */
            nChanId    = va_arg(pArgs, UINT);
            nPortNo    = va_arg(pArgs, UINT);
            lIPaddr    = va_arg(pArgs, ULNG);
            nOurPortNo = va_arg(pArgs, UINT);

            /* Locate the Monitor Descriptor record corresponding to this
             * Port Number. This record contains all control information and
             * a list of current connections which we browse, validate and
             * add to.
            */
            if((spMon=(ML_MONLIST*)FindItem(Ml.spMonHead,&nOurPortNo,NULL,NULL))
                                                                        == NULL)
            {
                /* Log error and setup exit code.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "Unable to locate Monitor Descriptor record for port (%d)",
                    nOurPortNo);
                break;
            }

            /* Lookup this channel ID in the list of active connections. If it
             * exists, we have a ghost, log it and exit.
            */
            if((ML_CONLIST *)FindItem(spMon->spConHead,&nChanId,NULL,NULL)
                                                                    != NULL)
            {
                /* Log error and setup exit code.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "Duplicate channel ID (%d). Channel ID already active",
                    nChanId);
                break;
            }

            /* Create a new connection list record for this connection.
            */
            if((spCon=(ML_CONLIST *)malloc(sizeof(ML_CONLIST))) == NULL)
            {
                /* Weve run out of memory, log error,setup exit code and
                 * exit, wasting connection.
                */
                Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                    sizeof(ML_CONLIST));
                break;
            }

            /* Populate the new record with the required data.
            */
            spCon->nChanId       = nChanId;
            spCon->nClientPortNo = nPortNo;
            spCon->lClientIPaddr = lIPaddr;

            /* Tag onto end of existing list.
            */
            if(AddItem(&spMon->spConHead,&spMon->spConTail,SORT_NONE,&nChanId,
                       NULL, NULL, spCon) == R_FAIL)
            {
                /* Dont modify Errno as AddItem has already set it for the
                 * correct error condition.
                */
                free(spCon);
                break;
            }

            /* Call the application Connect CB Handler for its own internal
             * processing if one has been specified.
            */
            if(spMon->nConnectCB != NULL)
                spMon->nConnectCB(spCon->nChanId, spMon->nMonPort);

            /* Configure the channel so that it uses Raw Mode Technology.
            */
            SL_RawMode(spCon->nChanId, TRUE);
            break;

        case SLC_LINKDOWN:
        case SLC_LINKFAIL:
            /* Extract the relevant details of the new connection.
            */
            nChanId    = va_arg(pArgs, UINT);
            nPortNo    = va_arg(pArgs, UINT);
            lIPaddr    = va_arg(pArgs, ULNG);
            nOurPortNo = va_arg(pArgs, UINT);

            /* Locate the Monitor Descriptor record corresponding to this
             * Port Number. 
            */
            if((spMon=(ML_MONLIST *)FindItem(Ml.spMonHead, &nOurPortNo,
                                             NULL,NULL)) == NULL)
            {
                /* Log error and setup exit code.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "Unable to locate Monitor Descriptor record for port (%d)",
                    nOurPortNo);
                break;
            }

            /* Delete the record off the linked list, no longer needed.
            */
            if(DelItem(&spMon->spConHead, &spMon->spConTail, NULL, &nChanId,
                       NULL, NULL) == R_FAIL)
            {
                /* Log error and setup exit code.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "Connection closure on unknown channel Id (%d)", nChanId);
                break;
            }

            /* Call the application Disconnect CB Handler for its own internal
             * processing if one has been specified.
            */
            if(spMon->nDisconCB != NULL)
                spMon->nDisconCB(nChanId, spMon->nMonPort);
            break;

        default:
            break;
    }

    /* Terminate var-arg list.
    */
    va_end(pArgs);

    /* Finished, get out!
    */
    return;
}

/******************************************************************************
 * Function:    _ML_MonTerminate
 * Description: An interactive monitor command to terminate us (the program).
 * Author:      P.D. Smart
 * Returns:     Non.
 ******************************************************************************/
int    _ML_MonTerminate( UINT        nChanId,     /* I: Channel to Im session */
                         UCHAR       *szBuf,      /* I: Remainder of input line */
                         UINT        nBufLen )    /* I: Length of input line */
{
    /* Local variables.
    */
    UCHAR        szTmpBuf[MAX_TMPBUFLEN];

    /* Send signoff message.
    */
    sprintf(szTmpBuf, "%c", MON_MSG_TERMINATE);
    SL_BlockSendData(nChanId, szTmpBuf, strlen(szTmpBuf));

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
    /* Sleep for a while to let the comms filter through.
    */
    sleep(2);
#endif

    /* Request that the process terminates.
    */
    SL_PostTerminate();

    /* Finished, get out!!
    */
    return( R_OK );
}



/******************************************************************************
 * User API functions.
 ******************************************************************************/

/******************************************************************************
 * Function:    ML_Init
 * Description: Initialise all functionality to allow a remote user to 
 *              connect with this executing program and issue commands to it.
 * Returns:     Non.
 ******************************************************************************/
int ML_Init( UINT     nMonPort,              /* I: Port that monitor service is on */
             UINT     nServiceType,          /* I: Type of monitor service. ie HTML */
             UCHAR    *szServerName,         /* I: HTTP Server Name Response */
             int      (*nConnectCB)(),       /* I: CB on client connection */
             int      (*nDisconCB)(),        /* I: CB on client disconnection */
             int      (*nInterpretOvr)())    /* I: Builtin Interpret override func */
{
    /* Local variables.
    */
    UCHAR        szTmpBuf[MAX_TMPBUFLEN];
    char         *szFunc = "ML_Init";
    LINKLIST     *spNext;
    ML_MONLIST   *spMon;

    /* First, some checks:
     * 1: Has this port already been registered.
    */
    for(spMon=(ML_MONLIST *)StartItem(Ml.spMonHead, &spNext); spMon != NULL;
                spMon=(ML_MONLIST *)NextItem(&spNext))
    {
        /* Port already stored in linked list?
        */
        if(spMon->nMonPort == nMonPort)
        {
            /* Log error.
            */
            Lgr(LOG_DEBUG, szFunc, "Port (%d) already registered", nMonPort);

            /* This port has already been registered, exit.
            */
            Errno = E_EXISTS;
            return(R_FAIL);
        }
    }

    /* 2: Valid service type. NB. Could use enumerated types for compile
     *    time checking, but believe future ops may be limited by this.
    */
    if(nServiceType != MON_SERVICE_HTML && nServiceType != MON_SERVICE_NL &&
       nServiceType != MON_SERVICE_EXT )
    {
        /* Log error.
        */
        Lgr(LOG_DEBUG, szFunc, "Unknown Service Type (%d)", nServiceType);

        /* Setup error and exit.
        */
        Errno = E_BADPARM;
        return(R_FAIL);
    }

    /* 3: If the service is for an external interpreter, ensure the
     * application has provided an interpreter function callback.
    */
    if(nServiceType == MON_SERVICE_EXT && spMon->nInterpretOvr == NULL)
    {
        /* Log error.
        */
        Lgr(LOG_DEBUG, szFunc, "Application requested external interpretation but failed to provide a interpretation handler.");

        /* Setup error and exit.
        */
        Errno = E_BADPARM;
        return(R_FAIL);
    }

    /* Create a new record for storage of all details relating to this
     * service.
    */
    if((spMon=(ML_MONLIST *)malloc(sizeof(ML_MONLIST))) == NULL)
    {
        /* Log error and get out.
        */
        Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
            sizeof(ML_MONLIST));
        Errno = E_NOMEM;
        return(R_FAIL);
    }

    /* Did the caller provide a server name for HTTP response headers?
    */    
    if(szServerName != NULL && strlen(szServerName) > 0)
    {
        /* Allocate memory for the server name storage.
        */
        if((spMon->szServerName=(UCHAR *)malloc(strlen(szServerName)+1))==NULL)
        {
            /* Log error and get out.
            */
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                strlen(szServerName)+1);
            Errno = E_NOMEM;
            return(R_FAIL);
        }

        /* Copy the provided server name into allocated storage.
        */
        strcpy(spMon->szServerName, szServerName);
    }

    /* Load up record with values we currenlty know about.
    */
    spMon->nMonPort = nMonPort;
    spMon->nServiceType = nServiceType;
    spMon->nConnectCB = nConnectCB;
    spMon->nDisconCB = nDisconCB;
    spMon->nInterpretOvr = nInterpretOvr;
    spMon->spConHead = NULL;
    spMon->spConTail = NULL;
    spMon->spMCHead = NULL;
    spMon->spMCTail = NULL;

    /* Add a comms service on the given port number. If failure occurs,
     * Errno set by SL library.
    */
    if(SL_AddServer(nMonPort, FALSE, _ML_MonitorCB, _ML_MonitorCntrl) == R_FAIL)
    {
        free(spMon);
        return(R_FAIL);
    }

    /* Success so far... so tag record onto end of our monitor list.
    */
    if(AddItem(&Ml.spMonHead, &Ml.spMonTail, SORT_NONE, &spMon->nMonPort, NULL,
               NULL, spMon) == R_FAIL)
    {
        /* Get rid of comms server.
        */
        SL_DelServer(nMonPort);

        /* Free memory.
        */
        free(spMon);

        /* Set Errno to reflect condition and get out.
        */
        Errno = E_NOMEM;
        return(R_FAIL);
    }

    /* Add any default commands for this service...
    */
    switch(nServiceType)
    {
        case MON_SERVICE_NL:
            ML_AddMonCommand(nMonPort, MC_TERMINATE, _ML_MonTerminate);
            break;

        case MON_SERVICE_HTML:
            ML_AddMonCommand(nMonPort, MC_HTMLGET, _ML_HTMLDefaultCB);
            ML_AddMonCommand(nMonPort, MC_HTMLHEAD, _ML_HTMLDefaultCB);
            ML_AddMonCommand(nMonPort, MC_HTMLPOST, _ML_HTMLDefaultCB);
            ML_AddMonCommand(nMonPort, MC_HTMLPUT, _ML_HTMLDefaultCB);
            ML_AddMonCommand(nMonPort, MC_HTMLDELETE, _ML_HTMLDefaultCB);
            ML_AddMonCommand(nMonPort, MC_HTMLCONNECT, _ML_HTMLDefaultCB);
            ML_AddMonCommand(nMonPort, MC_HTMLOPTIONS, _ML_HTMLDefaultCB);
            ML_AddMonCommand(nMonPort, MC_HTMLTRACE, _ML_HTMLDefaultCB);

        default:
            break;
    } 

    /* Finished, get out!
    */
    return( R_OK );
}

/******************************************************************************
 * Function:    ML_Exit
 * Description: Decommission the Monitor module ready for program termination
 *              or re-initialisation.
 * Returns:     R_OK     - Exit succeeded.
 *              R_FAIL   - Couldnt perform exit processing, see errno.
 * <Errno>    
 ******************************************************************************/
int    ML_Exit( UCHAR        *szErrMsg )        /* O: Error message buffer */
{
    /* Local variables.
    */
    int             nReturn = R_OK;
    LINKLIST        *spNext;
    ML_COMMANDLIST  *spMC;
    ML_MONLIST      *spMon;

    /* Loop, freeing up all memory used by monitor lists.
    */
    for(spMon=(ML_MONLIST *)StartItem(Ml.spMonHead, &spNext); spMon != NULL;
        spMon=(ML_MONLIST *)NextItem(&spNext))
    {
        /* Free up interactive monitoring command memory.
        */
        for(spMC=(ML_COMMANDLIST *)StartItem(spMon->spMCHead, &spNext);
            spMC != NULL; spMC=(ML_COMMANDLIST *)NextItem(&spNext))
        {
            if(spMC->szCommand != NULL)
                free(spMC->szCommand);
        }

        /* Delete all sublists.
        */
        if(spMon->spConHead != NULL)
            DelList(&spMon->spConHead, &spMon->spConTail);
        if(spMon->spMCHead != NULL)
            DelList(&spMon->spMCHead, &spMon->spMCTail);
    }

    /* Free up the main global linked list.
    */
    if(Ml.spMonHead != NULL)
        DelList(&Ml.spMonHead, &Ml.spMonTail);

    /* Free up any character buffers...
    */

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    ML_Send
 * Description: Transmit data to a given remote session.
 * Returns:     Non.
 ******************************************************************************/
int    ML_Send( UINT        nChanId,     /* I: Channel to Im session */
                UCHAR       *szBuf,      /* I: Xmit Data */
                UINT        nBufLen )    /* I: Length of Xmit Data */
{
    /* Local variables.
    */
    UINT        nBufferLen = nBufLen;
    UINT        nLen;
    UINT        nPos = 0;
    int            nReturn;
    UINT        nRetry = 20;

    /* If nBufLen is 0, then the data is in a null terminated string,
     * so work out its length.
    */
    if(nBufferLen == 0)
        nBufferLen = strlen(szBuf);

    /* Split the message up into packets, for faster viewing response time
     * at the users end.
    */
    do {
        /* Work out length of packet to be xmitted.
        */
        nLen = ((nBufLen-nPos) > 2000 ? 2000 : (nBufferLen-nPos));

        /* Loop until we manage to send the data or our counter expires.
        */
        while((nReturn=SL_SendData(nChanId, &szBuf[nPos], nLen)) == R_FAIL)
        {
            /* If the failure is not due to a device being busy, get out, we'll
             * never be able to transmit the data.
            */
            if(Errno != E_BUSY && Errno != EINTR && Errno != ENOBUFS &&
               Errno != EWOULDBLOCK)
            {
                nRetry = 0;
                break;
            }

            /* If our retry counter expires, then something is very busy, so
             * scrap the data.
            */
            if(nRetry-- == 0)
                break;

            /* Sleep, hopefully the device will become free.
            */
            sleep(1);
        }

        /* Update position pointer.
        */
        nPos += nLen;
    } while(nPos < nBufferLen && nRetry != 0);

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    ML_AddMonCommand
 * Description: Add a command to the interactive monitors database of 
 *              recognised reserved words. When a command comes in from an
 *              external user, if checks it against its reserved word list
 *              for verification and identification.
 * Returns:     R_OK     - Command added.
 *              R_FAIL   - Couldnt add command, see errno.
 * <Errno>    
 ******************************************************************************/
int    ML_AddMonCommand( UINT        nMonPort,          /* I: Service Mon Port */
                         UCHAR       *szCommand,        /* I: Command in text */
                         int         (*nCallback)())    /* I: Command callback */
{
    /* Local variables.
    */
    int                nReturn = R_OK;
    char               *szFunc = "ML_AddMonCommand";
    ML_COMMANDLIST     *spMC;
    ML_MONLIST         *spMon;

    /* Locate the correct record for this Monitor Port service.
    */
    if((spMon=(ML_MONLIST *)FindItem(Ml.spMonHead, &nMonPort, NULL, NULL))
                                                                        == NULL)
    {
        /* Log error and setup exit code.
        */
        Lgr(LOG_DEBUG, szFunc, "Service for port (%d) not registered",
            nMonPort);
        Errno = E_BADPARM;
        return(R_FAIL);
    }

    /* Does this command already exist? If it does, then caller application
     * just wishes to override the callback function.
    */
    if((spMC=(ML_COMMANDLIST *)FindItem(spMon->spMCHead, NULL, NULL,
                                        szCommand)) != NULL)
    {
        /* Update the callback, all that needs doing.
        */
        spMC->nCallback = nCallback;
    } else
     {
        /* Create record to store command details in.
        */
        if((spMC = (ML_COMMANDLIST *)malloc(sizeof(ML_COMMANDLIST))) == NULL)
        {
            /* Log error, setup exit code and getout.
            */
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                sizeof(ML_MONLIST));
            Errno = E_NOMEM;
            return(R_FAIL);
        } else
         {
            /* Wash the new memory, just in case.
            */
            memset((UCHAR *)spMC, '\0', sizeof(ML_COMMANDLIST));

            /* Now allocate memory to store the command.
            */
            if((spMC->szCommand = (UCHAR *)malloc(strlen(szCommand)+1)) == NULL)
            {
                Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                    strlen(szCommand)+1);
                free(spMC);
                Errno = E_NOMEM;
                return(R_FAIL);
            }

            /* Store command and callback in new memory.
            */
            strcpy(spMC->szCommand, szCommand);
            spMC->nCallback = nCallback;
        }

        /* Add to Monitor Command list.
        */
        if(AddItem(&spMon->spMCHead, &spMon->spMCTail, SORT_NONE, NULL, NULL,
                   spMC->szCommand, spMC) == R_FAIL)
        {
            /* Dont modify Errno as AddItem has already set it for the
             * correct error condition.
            */
            free(spMC->szCommand);
            free(spMC);
            return(R_FAIL);
        }
    }

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    ML_DelMonCommand
 * Description: Delete a command currently active in a monitor channels
 *              database of recognised words.
 * Returns:     R_OK     - Command deleted.
 *              R_FAIL   - Couldnt delete command, see errno.
 * <Errno>    
 ******************************************************************************/
int    ML_DelMonCommand( UINT        nMonPort,       /* I: Service Mon Port */
                         UCHAR       *szCommand )    /* I: Command to delete */
{
    /* Local variables.
    */
    int             nReturn = R_OK;
    char            *szFunc = "ML_DelMonCommand";
    ML_MONLIST      *spMon;

    /* Locate the correct record for this Monitor Port service.
    */
    if((spMon=(ML_MONLIST *)FindItem(Ml.spMonHead, &nMonPort, NULL, NULL)) == NULL)
    {
        /* Log error and setup exit code.
        */
        Lgr(LOG_DEBUG, szFunc, "Service for port (%d) not registered",
            nMonPort);
        Errno = E_BADPARM;
        return(R_FAIL);
    }

    /* Attempt to delete the command. If its not found, then report an error,
     * command may have been previously deleted, misspelled or not added.
    */
    if(DelItem(&spMon->spMCHead, &spMon->spMCTail, NULL, NULL, NULL,
                                        szCommand) == R_FAIL)
    {
        /* Log error and setup exit code.
        */
        Lgr(LOG_DEBUG, szFunc, "Command (%s) couldnt be deleted, not present",
            szCommand);
        Errno = E_BADPARM;
        nReturn = R_FAIL;
    }

    /* Finished, get out!!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    ML_Broadcast
 * Description: Broadcast a message to all listening monitor processes.
 * Returns:     R_OK     - Data sent to some/all successfully.
 *              R_FAIL   - Couldnt send to any, see Errno.
 * <Errno>        
 ******************************************************************************/
int ML_Broadcast( UCHAR   *szData,     /* I: Data to be sent */
                  UINT    nDataLen )   /* I: Length of data */
{
    /* Local variables.
    */
    int            nReturn = R_FAIL;
    LINKLIST    *spNext;

    /* Scan list to find monitor clients.
    for(spNetCon=(ML_NETCONS *)StartItem(Ml.spHead, &spNext); spNetCon != NULL;
        spNetCon=(ML_NETCONS *)NextItem(&spNext))
    {
    */
        /* A monitor port is identified by the original server port being
         * equal to that stored within the control record.
        if(spNetCon->cCorS == STP_SERVER && spNetCon->nServerPortNo != 0 &&
           spNetCon->nOurPortNo == Ml.nMonPort)
        {
            if(SL_BlockSendData(spNetCon->nChanId, szData, nDataLen) == R_OK)
                nReturn = R_OK;
        }
    }
        */

    /* Finished, get out.
    */
    return(nReturn);
}


