/******************************************************************************
 * Product:       #     # ######  #     #
 *                #     # #     # #  #  #  #####
 *                #     # #     # #  #  #  #    #
 *                #     # #     # #  #  #  #    #
 *                 #   #  #     # #  #  #  #    #
 *                  # #   #     # #  #  #  #    #
 *                   #    ######   ## ##   #####
 *
 * File:          vdwd.h
 * Description:   Header file for declaration of structures, datatypes etc for
 *                the Virtual Data Warehouse Daemon.
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D.Smart, 1996-2019
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
#ifndef    VDWD_H
#define    VDWD_H

/* Return type definitions.
*/
#define    VDWD_OK              0
#define    VDWD_FAIL            1
#define    VDWD_EXIT            2

/* Definitions for maxims etc.
*/
#define    MAX_ERRMSG_LEN       256
#define    MAX_LOGFILELEN       256

/* Definitions for defaults.
*/
#define    DEF_SERVICENAME      "vdwd"
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
#define    DEF_LOGFILE          "/tmp/vdwd.log"
#endif
#if defined(_WIN32)
#define    DEF_LOGFILE          "\\VDWD.LOG"
#endif

/* Define command line flags.
*/
#define    FLG_LOGFILE          "-l"
#define    FLG_LOGMODE          "-m"

/* Define error return codes which are embedded into returned error messages
 * for the user to decipher.
*/
#define    VDWD_EMSG_BADSERVICE "V0001"
#define    VDWD_EMSG_BADACK     "V0002"
#define    VDWD_EMSG_BADNAK     "V0003"
#define    VDWD_EMSG_BADDATA    "V0004"

/* Structure to provide the interface between the VDW Daemon and its
 * linked in driver modules. Provides entry points into the driver for the
 * daemon.
*/
typedef struct {
    int        nType;
    int        (*InitService)( SERVICEDETAILS *, UCHAR * );
    int        (*CloseService)( UCHAR * );
    int        (*ProcessRequest)( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
    void       (*ProcessOOB)( UCHAR );
} VDWD_DRIVERS;

/* Globals (yuggghhh!) for the VDW Daemon. They are contained within a
 * structure so they are more manageable and readers can see immediately
 * the variables scope.
*/
typedef struct {
    int        nActiveService;
    UINT       nLogMode;
    UINT       nServiceInitialised;
    UCHAR      szLogFile[MAX_LOGFILELEN];
} VDWD_GLOBALS;

/* Declare any globals required by the daemon, or any specifics to the
 * C module.
*/
#if defined(VDWD_C)
    static     VDWD_GLOBALS    VDWD={0, LOG_DEBUG, FALSE, ""};
    extern     VDWD_DRIVERS    Driver[];
#endif

/* Prototypes for functions within VDWd
*/
int        GetConfig( int, UCHAR **, char **, UCHAR * );
int        VDWDInit( UCHAR * );
int        VDWDClose( UCHAR * );
int        VDWDInitService( int, SERVICEDETAILS *, UCHAR * );
int        VDWDCloseService( int, UCHAR * );
int        VDWDProcessRequest( int, UCHAR *, UINT, UCHAR * );
int        VDWDSendToClient( UCHAR *, UINT );
int        VDWDDataCallback( UCHAR *, int, UCHAR * );
void       VDWDOOBCallback( UCHAR );
int        main( int, char **, char ** );

#endif    /* VDWD_H */
