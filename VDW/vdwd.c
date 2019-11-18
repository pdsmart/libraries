/******************************************************************************
 * Product:       #     # ######  #     #
 *                #     # #     # #  #  #  #####
 *                #     # #     # #  #  #  #    #
 *                #     # #     # #  #  #  #    #
 *                 #   #  #     # #  #  #  #    #
 *                  # #   #     # #  #  #  #    #
 *                   #    ######   ## ##   #####
 *
 * File:          vdwd.c
 * Description:   The Virtual Data Warehouse server Daemon process. A daemon
 *                built upon the API libraries of MDC and the Server Data-source
 *                Driver libraries to provide a generic mechanism for 
 *                retrieving data from any data source local to the daemon.
 *                The daemon is designed to interact with VDW client 
 *                applications and serve their data needs.
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
#include    <mdc.h>
#include    <sdd.h>

/* Specials for Solaris.
*/
#if defined(SOLARIS) || defined(LINUX)
#include    <sys/types.h>
#include    <unistd.h>
#include    <sys/stat.h>
#endif

/* Indicate that we are a C module for any header specifics.
*/
#define     VDWD_C

/* Bring in local specific header files.
*/
#include    "vdwd.h"

/******************************************************************************
 * Function:    GetConfig
 * Description: Get configuration information from the OS or command line
 *              flags.
 * 
 * Returns:     VDWD_OK        - Configuration obtained.
 *              VDWD_FAIL    - Failure, see error message.
 ******************************************************************************/
int    GetConfig( int     argc,          /* I: CLI argument count */
                  UCHAR   **argv,        /* I: CLI argument contents */
                  char    **envp,        /* I: Environment variables */
                  UCHAR   *szErrMsg )    /* O: Any generated error message */
{
    /* Local variables.
    */
    int      nReturn = VDWD_OK;
    FILE     *fp;
    UCHAR    *szFunc = "GetConfig";

    /* See if the user wishes to use a logfile?
    */
    if( GetCLIParam(argc, argv, FLG_LOGFILE, T_STR, VDWD.szLogFile,
                    MAX_LOGFILELEN, FALSE) == R_OK )
    {
        /* Check to see if the filename is valid.
        */
        if((fp=fopen(VDWD.szLogFile, "a")) == NULL)
        {
            sprintf(szErrMsg, "Cannot write to logfile (%s)", VDWD.szLogFile);
            return(VDWD_FAIL);
        }

        /* Close the file as test complete.
        */
        fclose(fp);
    } else
     {
        /* Set logfile to a default, dependant on OS.
        */
        strcpy(VDWD.szLogFile, DEF_LOGFILE);
    }

    /* Get log mode from command line.
    */
    if(GetCLIParam(argc, argv, FLG_LOGMODE, T_INT, (UCHAR *)&VDWD.nLogMode,
                   0, 0) == R_OK)
    {
        /* Check the validity of the mode.
        */
        if((VDWD.nLogMode < LOG_OFF || VDWD.nLogMode > LOG_FATAL) &&
            VDWD.nLogMode != LOG_CONFIG)
        {
            sprintf(szErrMsg, "Illegal Logger mode (%d)", VDWD.nLogMode);
            return(VDWD_FAIL);
        }
    } else
     {
        /* Setup default log mode.
        */
        VDWD.nLogMode = LOG_MESSAGE;
    }

    /* Finished, get out!
    */
    return( nReturn );
}

/******************************************************************************
 * Function:    VDWDInit
 * Description: Initialisation of variables, functionality, communications
 *              and turning the process into a daemon.
 * 
 * Returns:     VDWD_OK        - Initialised successfully.
 *              VDWD_FAIL    - Failure, see error message.
 ******************************************************************************/
int    VDWDInit( UCHAR        *szErrMsg    )    /* O: Generated error message */
{
    /* Local variables.
    */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
    pid_t        pid;
#endif

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
    /* To start daemonisation, we must fork a child to detach from the
     * parent process.
    */
    if( (pid=fork()) < 0 )
    {
        /* Couldnt fork a child, so build up an error message and then exit
         * with failure code.
        */
        sprintf(szErrMsg, "Couldnt fork a child process for daemonisation");
        return(VDWD_FAIL);
    } else
    if( pid != 0 )
    {
        /* As the parent, we exit here, cela-vie.
        */
        exit(0);
    }

    /* OK, we are a child and are maturing to be a parent, so lets shed
     * our childish skin....
     *
     * Firstly, become session leader..
    */
    setsid();

    /* Then ensure we are not hogging an NFS directory.
    */
    chdir("/");

    /* Setup the UMASK for known file creation state.
    */
    umask(0);
#endif

    /* Setup logger mode.
    */
    Lgr(LOG_CONFIG, LGM_FLATFILE, VDWD.nLogMode, VDWD.szLogFile);

    /* All done, lets get out.
    */
    return(VDWD_OK);
}

/******************************************************************************
 * Function:    VDWDClose
 * Description: Function to perform closure of all used resources within the
 *              module.
 * 
 * Returns:     VDWD_OK        - Closed successfully.
 *              VDWD_FAIL    - Failure, see error message.
 ******************************************************************************/
int    VDWDClose(    UCHAR        *szErrMsg    )    /* O: Generated error message */
{
    /* Local variables.
    */

    /* If the service hasnt been previously initialised, then there is no
     * reason to close it down!!!
    */
    if(VDWD.nServiceInitialised == TRUE)
    {
        /* Close the currently active service
        */
        if(VDWDCloseService(VDWD.nActiveService, szErrMsg) == VDWD_FAIL)
            return(VDWD_FAIL);

        /* Tidy up variables.
        */
        VDWD.nServiceInitialised = FALSE;
        VDWD.nActiveService = 0;
    }

    /* Exit with success.
    */
    return(VDWD_OK);
}

/******************************************************************************
 * Function:    VDWDSentToClient
 * Description: Function to send data from this daemon back to the relevant
 *              client.
 * 
 * Returns:     SDD_OK        - Data sent successfully.
 *              SDD_FAIL    - Failure in sending data.
 ******************************************************************************/
int    VDWDSendToClient( UCHAR   *snzData,     /* I: Data to send */
                         UINT    nDataLen )    /* I: Length of data */
{
    /* Local variables.
    */
    int        nReturn = SDD_OK;

    /* Call the MDC library to transmit the data. If it fails, then just
     * let the driver know by the return value.
    */
    if(MDC_ReturnData(snzData, nDataLen) == MDC_FAIL)
    {
        nReturn = SDD_FAIL;
    }

    /* Return code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    VDWDInitService
 * Description: Function to call a given drivers initialisation function.
 * 
 * Returns:     VDWD_OK        - Service was initialised successfully.
 *              VDWD_FAIL    - Failure, see error message.
 ******************************************************************************/
int    VDWDInitService( int                nServiceType,     /* I: Type of service*/
                        SERVICEDETAILS     *sServiceDet,     /* I: Service Data */
                        UCHAR              *szErrMsg    )    /* O: Error message */
{
    /* Local variables.
    */
    UINT        nNdx;
    int         nReturn = VDWD_OK;
    UCHAR       *szFunc = "VDWDInitService";

    /* Go through list of drivers and locate one that is of the correct type.
    */
    for(nNdx=0; Driver[nNdx].nType != 0 && Driver[nNdx].nType != nServiceType;
        nNdx++);

    /* If we located the correct driver then we can perform initialisation.
    */
    if(Driver[nNdx].nType == nServiceType)
    { 
        /* If there is a registered Initialisation function for this driver
         * then invoke it, else just return as though everything completed
         * successfully.
        */
        if(Driver[nNdx].InitService != NULL)
        {
            /* Perform the initialisation and set return code accordingly.
            */
            if(Driver[nNdx].InitService(sServiceDet, szErrMsg) == SDD_FAIL)
            {
                nReturn = VDWD_FAIL;
            }
        } else
         {
            /* Log the fact that there is no driver.
            */ 
            Lgr(LOG_DEBUG, szFunc,
                "No registered InitService for Driver type (%d)", nServiceType);
        }
    } else
     {
        /* Build up an error message and exit.
        */
        sprintf(szErrMsg,
                "%s: Illegal service type (%d)",
                VDWD_EMSG_BADSERVICE, nServiceType);
        nReturn = VDWD_FAIL;
    }

    /* Finished, return result to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    VDWDCloseService
 * Description: Function to call a given drivers closedown function.
 * 
 * Returns:     VDWD_OK        - Service was closed successfully.
 *              VDWD_FAIL    - Failure, see error message.
 ******************************************************************************/
int    VDWDCloseService( int       nServiceType,     /* I: Type of service*/
                         UCHAR     *szErrMsg    )    /* O: Error message */
{
    /* Local variables.
    */
    UINT        nNdx;
    int         nReturn = VDWD_OK;
    UCHAR       *szFunc = "VDWDCloseService";

    /* Go through list of drivers and locate one that is of the correct type.
    */
    for(nNdx=0; Driver[nNdx].nType != 0 && Driver[nNdx].nType != nServiceType;
        nNdx++);

    /* If we located the correct driver then we can perform a closedown.
    */
    if(Driver[nNdx].nType == nServiceType)
    { 
        /* If there is a registered Closedown function for this driver
         * then invoke it, else just return as though everything completed
         * successfully.
        */
        if(Driver[nNdx].CloseService != NULL)
        {
            /* Perform the closedown and set return code accordingly.
            */
            if(Driver[nNdx].CloseService(szErrMsg) == SDD_FAIL)
            {
                nReturn = VDWD_FAIL;
            }
        } else
         {
            /* Log the fact that there is no driver.
            */ 
            Lgr(LOG_DEBUG, szFunc,
                "No registered CloseService for Driver type (%d)",nServiceType);
        }
    } else
     {
        /* Build up an error message and exit.
        */
        sprintf(szErrMsg,
                "%s: Illegal service type (%d)",
                VDWD_EMSG_BADSERVICE, nServiceType);
        nReturn = VDWD_FAIL;
    }

    /* Finished, return result to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    VDWDProcessRequest
 * Description: Function to call a given drivers function to process a
 *              service request.
 * 
 * Returns:     VDWD_OK        - Request was processed successfully.
 *              VDWD_FAIL    - Failure, see error message.
 ******************************************************************************/
int    VDWDProcessRequest( int         nServiceType,    /* I: Type of service */
                           UCHAR       *snzData,        /* I: Data Buffer */
                           UINT        nDataLen,        /* I: Len of Data */
                           UCHAR       *szErrMsg    )   /* O: Error message */
{
    /* Local variables.
    */
    UINT        nNdx;
    int         nReturn = VDWD_OK;
    UCHAR       *szFunc = "VDWDProcessRequest";

    /* Go through list of drivers and locate one that is of the correct type.
    */
    for(nNdx=0; Driver[nNdx].nType != 0 && Driver[nNdx].nType != nServiceType;
        nNdx++);

    /* If we located the correct driver then we can perform the request.
    */
    if(Driver[nNdx].nType == nServiceType)
    { 
        /* If there is a registered Process Request function for this driver
         * then invoke it, else just return as though everything completed
         * successfully.
        */
        if(Driver[nNdx].ProcessRequest != NULL)
        {
            /* Call the driver function to process the request.
            */
            if(Driver[nNdx].ProcessRequest(snzData, nDataLen, VDWDSendToClient,
                                           szErrMsg) == SDD_FAIL)
            {
                nReturn=VDWD_FAIL;
            }
        } else
         {
            /* Log the fact that there is no driver.
            */ 
            Lgr(LOG_DEBUG, szFunc,
                "No registered ProcessRequest for Driver type (%d)",
                nServiceType);
        }
    } else
     {
        /* Build up an error message and exit.
        */
        sprintf(szErrMsg,
                "%s: Illegal service type (%d)",
                VDWD_EMSG_BADSERVICE, nServiceType);
        nReturn = VDWD_FAIL;
    }

    /* Finished, return result to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    VDWDDataCallback
 * Description: Function which is registered as a callback and is called 
 *              every time data arrives from a new client.
 * 
 * Returns:     MDC_OK        - Closed successfully.
 *              MDC_FAIL    - Failure, see error message.
 ******************************************************************************/
int    VDWDDataCallback( UCHAR    *snzData,    /* I: Buffer containing data */
                         int      nDataLen,    /* I: Length of data in buffer */
                         UCHAR    *szErrMsg )  /* O: Error messages generated */
{
    /* Local variables.
    */
    SERVICEDETAILS   *sServiceDet;
    UCHAR            *szFunc = "VDWDDataCallback";

    /* Data has arrived from a connected client. We know for sure that we
     * will only ever deal with one client, so its safe to make a lot of
     * assumptions about the data path. In this function, we only need to
     * decipher the data packet that we've been passed and dispatch it to
     * the correct handler.
    */

    /* First, check the length... it must be at least 1 byte!!
    */
    if(nDataLen <= 0)
    {
        Lgr(LOG_DEBUG, szFunc, "Being called with %dByte packets", nDataLen);
        return(MDC_FAIL);
    }

    /* Work out the type of packet that we've been passed by analysing the
     * first byte.
    */
    switch(snzData[0])
    {
        case MDC_ACK:
            sprintf(szErrMsg,
                    "%s: Server being sent an ACK, illegal!!",
                    VDWD_EMSG_BADACK);
            return(MDC_FAIL);

        case MDC_NAK:
            sprintf(szErrMsg,
                    "%s: Server being sent a NAK, illegal!!",
                    VDWD_EMSG_BADNAK);
            return(MDC_FAIL);

        case MDC_PREQ:
            /* Has the service been initialised yet? If it hasnt, then we
             * cant possibly pass a data block to the unknown.
            */
            if(VDWD.nServiceInitialised == TRUE)
            {
                /* Try and process the given request.
                */
                if(VDWDProcessRequest(VDWD.nActiveService, &snzData[1],
                                      nDataLen-1, szErrMsg) == VDWD_FAIL)
                {
                    return(MDC_FAIL);
                }
            } else
             {
                /* Just return an error message/code to indicate the problem.
                */
                sprintf(szErrMsg,
                        "%s: No service initialised, data packet illegal!!!",
                        VDWD_EMSG_BADDATA);
                return(MDC_FAIL);
            }
            break;

        case MDC_CHANGE:
        case MDC_INIT:
            /* Is this the first initialisation call...? If it isnt then close
             * the original service prior to initialising the new service.
            */
            if(VDWD.nServiceInitialised == TRUE)
            {
                /* Close the service, exit if we cant, using provided error
                 * message.
                */
                if(VDWDCloseService(VDWD.nActiveService, szErrMsg) == VDWD_FAIL)
                    return(MDC_FAIL);

                /* Toggle the flag in case of failure.
                */
                VDWD.nServiceInitialised = FALSE;
            }

            /* Extract the type of service by casting the provided data to
             * a service block and then using the structure types.
            */
            snzData++;
            sServiceDet = (SERVICEDETAILS *)snzData;
            VDWD.nActiveService=(UINT)sServiceDet->cServiceType;

            /* Initialise the requested sevice.
            */
            if(VDWDInitService(VDWD.nActiveService, sServiceDet, szErrMsg)
                                                                == VDWD_FAIL)
            {
                return(MDC_FAIL);
            }

            /* Set flag to indicate that service has been initialised.
            */
            VDWD.nServiceInitialised = TRUE;
            break;

        default:
            sprintf(szErrMsg,
                    "%s: Unknown Service Type (%x)",
                    VDWD_EMSG_BADSERVICE, snzData[0]);
            return(MDC_FAIL);
    }

    /* Return any result codes to MDC library.
    */
    return(MDC_OK);
}

/******************************************************************************
 * Function:    VDWDOOBCallback
 * Description: Function to take action on out of band commands from the
 *              MDC layer. Out of band messages are generally commands which
 *              need to be actioned upon immediately, so they are passed up
 *              into the Drivers out of band processing function. 
 * 
 * Returns:     No returns.
 ******************************************************************************/
void    VDWDOOBCallback( UCHAR    cCmd    )    /* I: Command to action upon */
{
    /* Local variables.
    */
    UINT        nNdx;
    UCHAR       *szFunc = "VDWDOOBCallback";

    /* Has a service been initialised yet? If one hasnt, then there is no
     * point in going further as OOB processing is only relevant to an
     * intialised module.
    */
    if(VDWD.nServiceInitialised == FALSE)
    {
        /* Log an error message and exit.
        */
        Lgr(LOG_DEBUG, szFunc,
            "Services not initialised, cannot process OOB (%x)",cCmd);
        return;
    }

    /* Go through list of drivers and locate one that is of the correct type.
    */
    for(nNdx=0; Driver[nNdx].nType != 0 &&
                Driver[nNdx].nType != VDWD.nActiveService;
        nNdx++);

    /* If we located the correct driver then we can perform the request.
    */
    if(Driver[nNdx].nType == VDWD.nActiveService)
    { 
        /* Process out of band message according to type.
        */
        switch(cCmd)
        {
            case MDC_ABORT:
                /* Log message for debug purposes.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "ABORT Out Of Band message received, processing");
                
                if(Driver[nNdx].ProcessOOB != NULL)
                {
                    Driver[nNdx].ProcessOOB(SDD_ABORT);
                } else
                 {
                    Lgr(LOG_DEBUG, szFunc,
                        "No registered ABORT OOB handler in driver");
                }
                break;

            case MDC_EXIT:
                /* Log message for debug purposes.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "EXIT Out Of Band message received, exitting");

                if(Driver[nNdx].ProcessOOB != NULL)
                {
                    Driver[nNdx].ProcessOOB(SDD_EXIT);
                } else
                 {
                    Lgr(LOG_DEBUG, szFunc,
                        "No registered EXIT OOB handler in driver");
                }
                break;

            default:
                /* Build up an error message and exit.
                */
                Lgr(LOG_DEBUG, szFunc, "Illegal command (%x)", cCmd);
                break;
        }
    } else
     {
        /* Build up an error message and exit.
        */
        Lgr(LOG_DEBUG, szFunc, "Called with no active service, cCmd=(%x)",cCmd);
    }

    /* Return to caller.
    */
    return;
}

/******************************************************************************
 * Function:    main
 * Description: Entry point into the Virtual Data Warehouse Daemon. Basic
 *              purpose is to invoke intialisation, enter the main program
 *              loop and finally tidy up and close down.
 * 
 * Returns:     0    - Program completed successfully without errors.
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
    if( GetConfig(argc, (UCHAR **)argv, envp, szErrMsg) == VDWD_FAIL )
    {
        printf( "%s\n"
                "Usage:                 %s <parameters>\n"
                "<parameters>:          -l<LogFile Name>\n"
                "                       -m<Logging Mode>\n",
                szErrMsg, argv[0]);
    }

    /* Initialise variables, communications and become a daemon.
    */
    if( VDWDInit(szErrMsg) == VDWD_FAIL )
    {
        /* Log an error message to indicate reason for failure.
        */
        Lgr(LOG_DIRECT, szFunc, "%s: %s", argv[0], szErrMsg);
        exit(-1);
    }

    /* Start the daemon running by passing control into the MDC library and
     * letting it generate callbacks as events occur.
    */
    if( MDC_Server(NULL, DEF_SERVICENAME, VDWDDataCallback,
                   VDWDOOBCallback ) == MDC_FAIL )
    {
        /* Problems within the MDC library, indicate error and get out.
        */
        Lgr(LOG_DIRECT, szFunc, "%s: MDC_Server error, aborting..", argv[0]);
        exit(-1);
    }

    /* Perform close down of used facilities ready for exit.
    */
    if( VDWDClose(szErrMsg) == VDWD_FAIL )
    {
        /* Log an error message to indicate reason for failure.
        */
        Lgr(LOG_DIRECT, szFunc, "%s: %s", argv[0], szErrMsg);
        exit(-1);
    }

    /* Log off message.
    */
    Lgr(LOG_DEBUG, szFunc, "Child daemon closing.");

    /* All done, go bye bye's.
    */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
    exit;
#endif
#if defined(_WIN32)
    return(0);
#endif
}
