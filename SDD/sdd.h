/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd.h
 * Description:   Server Data-source Driver library driver global header file
 *                for all library drivers.
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
#ifndef    SDD_H
#define    SDD_H

/* Bring in the MDC Library global definitions header file as we need to
 * know about SERVICEDETAILS.
*/
#include    <mdc.h>

/* Define any default values common to all drivers.
*/
#define    DEF_COLSEP            "|"
#define    DEF_PADCHR            " "

/* Define return types.
*/
#define    SDD_OK                0        /* Success reture code */
#define    SDD_FAIL             -1        /* Fail return code */

/* Define driver sub-commands.
*/
#define    SDD_ABORT            'A'        /* Abort command */
#define    SDD_EXIT             'E'        /* Exit command */

/* System command module commands.
*/
#define    SDD_CMD_DEL          'D'        /* Perform a delete on D host */
#define    SDD_CMD_EXEC         'X'        /* Execute a cmd on the local system */
#define    SDD_CMD_EXEC_TIMED   'T'        /* Execute a cmd on local system @ Time */
#define    SDD_CMD_MOVE         'M'        /* Perform a rename/move on D host */
#define    SDD_CMD_READ         'R'        /* Perform a read operation D -> Client */
#define    SDD_CMD_WRITE        'W'        /* Perform a write operation D <- Client */

/* FTP sub-commands.
*/
#define    SDD_FTPX_RCV         'R'        /* FTP Receive a File command */
#define    SDD_FTPX_XMIT        'X'        /* FTP Transmit a File command */
#define    SDD_FTPX_REN         'M'        /* FTP Rename a File command */

/* Database (Sybase, ODBC) sub-commands).
*/
#define    SDD_LIST_DB          'F'        /* List all database names on data source */
#define    SDD_LIST_COLS        'G'        /* List all column names on data source */
#define    SDD_LIST_TABLES      'H'        /* List all tables names on data source */
#define    SDD_RUNSQL           'R'        /* Execute an SQL statement */

/* Audio sub-commands.
*/
#define    SDD_PLAY_Z           'Z'        /* Play a compressed audio file */

/* Define driver data return sub-commands.
*/
#define    SDD_DATABLOCK        'D'        /* Data is one block out of many blocks */
#define    SDD_ENDBLOCK         'E'        /* Data is last block in series */

/* Define error return codes which are embedded into returned error messages
 * for the user to decipher.
*/
#define    SDD_EMSG_MEMORY      "D0001"
#define    SDD_EMSG_SQLLOAD     "D0002"
#define    SDD_EMSG_SQLSYNTAX   "D0003"
#define    SDD_EMSG_SQLRUNERR   "D0004"
#define    SDD_EMSG_FETCHHEAD   "D0005"
#define    SDD_EMSG_FETCHROW    "D0006"
#define    SDD_EMSG_SENDROW     "D0007"
#define    SDD_EMSG_ABORTRCVD   "D0008"
#define    SDD_EMSG_SRCINIT     "D0009"
#define    SDD_EMSG_BADLOGIN    "D0010"
#define    SDD_EMSG_BADDBASE    "D0011"
#define    SDD_EMSG_BADREQ      "D0012"
#define    SDD_EMSG_BADCMD      "D0013"
#define    SDD_EMSG_BADPATH     "D0014"
#define    SDD_EMSG_BADARGS     "D0015"
#define    SDD_EMSG_BADTIME     "D0016"
#define    SDD_EMSG_BADEXEC     "D0017"
#define    SDD_EMSG_BADEXIT     "D0018"
#define    SDD_EMSG_BADFREE     "D0019"
#define    SDD_EMSG_NOTYI       "D0020"
#define    SDD_EMSG_BADBLOCKSZ  "D0021"
#define    SDD_EMSG_FILEERROR   "D0022"
#define    SDD_EMSG_BADFILE     "D0023"

/* JAVA Driver.
*/
int        java_InitService( SERVICEDETAILS *, UCHAR * );
int        java_CloseService( UCHAR * );
int        java_ProcessRequest( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
void       java_ProcessOOB( UCHAR );

/* ODBC Driver.
*/
int        odbc_InitService( SERVICEDETAILS *, UCHAR * );
int        odbc_CloseService( UCHAR * );
int        odbc_ProcessRequest( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
void       odbc_ProcessOOB( UCHAR );

/* System Command Driver.
*/
int        scmd_InitService( SERVICEDETAILS *, UCHAR * );
int        scmd_CloseService( UCHAR * );
int        scmd_ProcessRequest( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
void       scmd_ProcessOOB( UCHAR );

/* FTP Driver.
*/
int        ftpx_InitService( SERVICEDETAILS *, UCHAR * );
int        ftpx_CloseService( UCHAR * );
int        ftpx_ProcessRequest( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
void       ftpx_ProcessOOB( UCHAR );

/* SYBASE Driver.
*/
int        sybc_InitService( SERVICEDETAILS *, UCHAR * );
int        sybc_CloseService( UCHAR * );
int        sybc_ProcessRequest( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
void       sybc_ProcessOOB( UCHAR );

/* Audio Player Driver.
*/
int        aupl_InitService( SERVICEDETAILS *, UCHAR * );
int        aupl_CloseService( UCHAR * );
int        aupl_ProcessRequest( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
void       aupl_ProcessOOB( UCHAR );

#endif    /* SDD_H */
