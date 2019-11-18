/******************************************************************************
 * Product:    ####### #######  #####  #######         #     # ####### #     #
 *                #    #       #     #    #            ##   ## #     # ##    #
 *                #    #       #          #            # # # # #     # # #   #
 *                #    #####    #####     #            #  #  # #     # #  #  #
 *                #    #             #    #            #     # #     # #   # #
 *                #    #       #     #    #            #     # #     # #    ##
 *                #    #######  #####     #    ####### #     # ####### #     #
 *
 * File:          test_mon.h
 * Description:   Header file for declaration of structures, datatypes etc for
 *                the ux library monitor testing program.
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D.Smart, 1996-2019.
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
#ifndef    TEST_MON_H
#define    TEST_MON_H

/* Definitions for maxims etc.
*/
#define    MAX_ERRMSG_LEN        256
#define    MAX_LOGFILELEN        256

/* Definitions for defaults.
*/
#define    DEF_HTML_PORT         9000
#define    DEF_NL_PORT           9001
#define    DEF_POLLTIME          1000
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX) || defined(ZPU)
#define    DEF_LOGFILE           "/tmp/test_mon.log"
#endif
#if defined(_WIN32)
#define    DEF_LOGFILE           "\\TEST_MON.LOG"
#endif

/* Define constants etc.
*/
#define    TMON_SRV_KEEPALIVE    1000    /* TCP/IP keep alive */

/* Define command line flags.
*/
#define    FLG_LOGFILE           "-l"
#define    FLG_LOGMODE           "-m"
#define    FLG_HTMLPORT          "-html_port"
#define    FLG_NLPORT            "-nl_port"

/* Monitor commands for Natural Language interface.
*/
#define    MC_NLHELP             "HELP"

/* Maxims.
*/
#define    MAX_FILENAMELEN       80

/* Globals (yuggghhh!).
*/
typedef struct {
    int            nHtmlPort;
    int            nNLPort;
    UINT           nLogMode;
    UCHAR          szLogFile[MAX_LOGFILELEN];
} TMON_GLOBALS;

/* Declare any globals required by the daemon, or any specifics to the
 * C module.
*/
#if defined(TEST_MON_C)
    static    TMON_GLOBALS    TMON={9000, 9001, LOG_DEBUG, ""};
#endif

/* Prototypes for functions.
*/
int        GetConfig( int, UCHAR **, char **, UCHAR * );
int        TMONInit( UCHAR * );
int        TMONClose( UCHAR * );
int        main( int, char **, char ** );

#endif    /* TEST_MON_H */
