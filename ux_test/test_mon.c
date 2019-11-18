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
