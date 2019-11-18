/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_lgr.c
 * Description:   General purpose standalone (programmable) logging
 *                utilities.
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
#include    <errno.h>
#include    <sys/timeb.h>
#include    <sys/stat.h>
#include    <fcntl.h>

#if defined(SUNOS) || defined(SOLARIS) || defined(LINUX)
#include    <sys/socket.h>
#include    <sys/time.h>
#include    <string.h>
#include    <time.h>
#endif

#if defined(SOLARIS)
#include    <sys/file.h>
#endif

#if defined(LINUX)
#include    <term.h>
#endif

#if defined(_WIN32)
#include    <winsock.h>
#include    <time.h>
#endif

#if defined(SUNOS) || defined(SOLARIS)
#include    <netinet/in.h>
#include    <sys/wait.h>
#endif

/* Indicate that we are a C module for any header specifics.
*/
#define        UX_LOGGER_C

/* Bring in specific header files.
*/
#include    "ux.h"

/******************************************************************************
 * Function:    Lgr
 * Description: A function to log a message to a flatfile, database or both.
 * Returns:     Non.
 ******************************************************************************/
void Lgr( int        nLevel,        /* I: Level of error message/or command */
          ... )                     /* I: Varargs */
{
    /* Static's.
    */
#ifdef    SL_MONITOR
    static UCHAR    nAlert = FALSE;
#endif
    static UINT        nLogMode = LGM_STDOUT;
    static int        nErrLevel = LOG_MESSAGE;
    static UCHAR    *szLogFile = NULL;

    /* Local variables.
    */
    va_list        pArgs;
    struct tm    sTime;
    time_t        nTime;
    UCHAR        szBuf[MAX_VARARGBUF];
    UCHAR        *szFormat;
    UCHAR        *szFuncName;
#ifdef    SL_MONITOR
    UCHAR        szMonBuf[MAX_ERRMSG];
#endif
    UCHAR        szTime[50];
    FILE        *fp;

    /* Start variable argument passing.
    */
    va_start(pArgs, nLevel);

    /* Is this a configuration call? If so, set up for future calls.
    */
    if(nLevel == LOG_CONFIG)
    {
        /* Extract varargs off stack. Caller should have called with the
         * format: (nLevel, nLogMode, nErrLevel, szProgName)
        */
        nLogMode    = va_arg(pArgs, UINT);
        nErrLevel   = va_arg(pArgs, UINT);
        szLogFile   = va_arg(pArgs, UCHAR *);

        /* Tidy up for exit and return to caller.
        */
        va_end(pArgs);
        return;
    }

    /* If the logger is switched off, just exit.
    */
    if((nErrLevel == LOG_OFF || nLogMode == LGM_OFF) && nLevel != LOG_DIRECT)
    {
        /* Tidy up for exit and return to caller.
        */
        va_end(pArgs);
        return;
    }

    /* Is the level of this error worth logging.. ie. When configured, the
     * caller sets a level of which future calls have to equal or better.
    */
#if defined(UX_DEBUG)
    if(TRUE)
#else
    if(nLevel >= nErrLevel)
#endif
    {
        /* Extract varargs off stack. Caller should have called with the
         * format: (nLevel, szFuncName, szFormat, ...)
        */
        szFuncName  = va_arg(pArgs, UCHAR *);
        szFormat    = va_arg(pArgs, UCHAR *);

        /* Build up full message for logging.
        */
        vsprintf(szBuf, szFormat, pArgs);

        /* Get current time to stamp message with. This is only used for non-
         * database log messages.
        */
        time(&nTime);
#if defined(SOLARIS)
        localtime_r(&nTime, &sTime);
#else
        memcpy(&sTime, localtime(&nTime), sizeof(struct tm));
#endif
        sprintf(szTime, "%02d/%02d/%02d %02d:%02d:%02d", sTime.tm_mday,
                sTime.tm_mon+1, sTime.tm_year, sTime.tm_hour, sTime.tm_min,
                sTime.tm_sec);

        /* Log according to programmed mode.
         *   LGM_OFF     - No logging.
         *     LGM_STDOUT     - Log to stdout.
         *   LGM_FLATFILE- Log to a flatfile.
         *   LGM_DB      - Log to Database.
         *   LGM_ALL     - Log to all output destinations.
        */
        if(nLogMode == LGM_STDOUT || nLevel == LOG_DIRECT)
        {
            /* Print to stdout as requested.
            */
            printf("[%d %s %s] %s\n", nLevel, szTime, szFuncName,szBuf);
            fflush(stdout);
        }
        if((nLogMode == LGM_ALL || nLogMode == LGM_FLATFILE) &&
            nLevel != LOG_DIRECT)
        {
            /* Only log to flatfile if a program name exists.
            */
            if(szLogFile != NULL)
            {
                /* Build up the name of file to open.
                */
                if((fp=fopen(szLogFile, "a")) != NULL)
                {
                    /* Place string in buffer.
                    */
                    fprintf(fp, "[%d %s %s] %s\n", nLevel, szTime,
                            szFuncName, szBuf);

                    /* Close file when finished, in case a crash occurs,
                     * sys-op wants to see last message.
                    */
                    fclose(fp);
                }
            }
        }
        if((nLogMode == LGM_ALL || nLogMode == LGM_DB) && nLevel != LOG_DIRECT)
        {
            /* Call the database to log the message.
            */
        }
    }

/* If monitor processing is enabled, then dispatch all log messages to 
 * monitor processes.
*/
#ifdef    SL_MONITOR
    /* If we've had a previous alert and we've got an new message,
     * then send cancel alert.
    */
    if(nAlert == TRUE && nLevel >= LOG_MESSAGE)
    {
        sprintf(szMonBuf, "%c", MON_MSG_CANALERT);
        SL_MonitorBroadcast(szMonBuf, strlen(szMonBuf));
        nAlert = FALSE;
    }

    /* Look at error level, Alerts and Fatal messages require dispatch
     * of an ALERT message to all monitor sites.
    */
    if(nLevel >= LOG_ALERT)
    {
        sprintf(szMonBuf, "%c", MON_MSG_ALERT);
        SL_MonitorBroadcast(szMonBuf, strlen(szMonBuf));
        nAlert = TRUE;
    }

    /* Build up error message and transmit seperately.
    */
    sprintf(szMonBuf, "%c%s %s", MON_MSG_ERRMSG, szTime, szBuf);
    SL_MonitorBroadcast(szMonBuf, strlen(szMonBuf));
#endif

    /* Stop vararg processing... ie tidy up stack.
    */
    va_end(pArgs);

    /* Finished, get out!!
    */
    return;
}
