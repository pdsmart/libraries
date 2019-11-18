/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_ftpx.h
 * Description:   Server Data-source Driver library driver header file for
 *                the system command execution driver.
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
#ifndef    SDD_FTPX_H
#define    SDD_FTPX_H

/* Define FTP response codes require by this module.
*/
#define    FTP_RESPONSE_NONE        0
#define    FTP_RESPONSE_DATASTART   150
#define    FTP_RESPONSE_CWDOK       200
#define    FTP_RESPONSE_CONNECTED   220
#define    FTP_RESPONSE_GOODBYE     221
#define    FTP_RESPONSE_DATAEND     226
#define    FTP_RESPONSE_PASSIVE     227
#define    FTP_RESPONSE_PWDOK       230
#define    FTP_RESPONSE_CMDOK       250
#define    FTP_RESPONSE_USEROK      331
#define    FTP_RESPONSE_RENFROK     350
#define    FTP_RESPONSE_BADSEQ      503
#define    FTP_RESPONSE_PWDFAIL     530
#define    FTP_RESPONSE_CMDFAIL     550
#define    FTP_REQUIRE_NOTHING      0
#define    FTP_REQUIRE_RESPONSE     1

/* Definitions for maxims etc.
*/
#define    MAX_RETURN_BUF           2048+1
#define    MAX_FTP_FILENAME         256
#define    MAX_FTP_BUF_SIZE         65536
#define    MAX_TMP_BUF_SIZE         1024

/* Definitions for constants, configurable options etc.
*/
#define    FTP_DATA                 "DATA="
#define    FTP_END                  "END="
#define    FTP_SRCFILE              "SRCFILE="
#define    FTP_DSTFILE              "DSTFILE="
#define    FTP_MODE                 "MODE="
#define    FTP_SRCPATH              "SRCPATH="
#define    FTP_DSTPATH              "DSTPATH="
#define    FTP_ASCII_MODE           0
#define    FTP_BINARY_MODE          1
#define    FTP_ASCII                "ASCII"
#define    FTP_BINARY               "BINARY"
#define    FTP_CONNECT_TIME         10000
#define    DEF_FTP_SERVICE_NAME     "ftp"
#define    DEF_FTP_BUF_SIZE         65536
#define    DEF_FTP_BUF_INCSIZE      65536
#define    DEF_FTP_XMIT_SIZE        1024
#define    DEF_SRV_START_PORT       10000
#define    DEF_SRV_END_PORT         15000

/* Structure for variables internal to this driver module.
*/
typedef struct {
    UINT        nPIChanId;            /* Comms identifier for PI channel */
    UINT        nPIDataBufLen;        /* Max len of snzPIDataBuf */
    UINT        nPIDataBufPos;        /* Position locator in snzPIDataBuf */
    UINT        nPIResponseCode;      /* Response code on the PI channel */
    UCHAR       *snzPIDataBuf;        /* Buffer for incoming confirmations */
    UINT        nDTPChanId;           /* Comms identifier for DTP channel */
    UINT        nDTPConnected;        /* DTP channel connected, TRUE=connected */
    ULNG        lDTPIPaddr;           /* IP address of the DTP server */
    UINT        nDTPPortNo;           /* Port of the DTP server */
    FILE        *fDataFile;           /* Data I/O stream for FTP file */
} FTPX_DRIVER;

/* Allocate any global variables needed within this module.
*/
static    FTPX_DRIVER        FTPX;

/* Prototypes of internal functions, not seen by any outside module.
*/
int        _FTPX_GetStrArg( UCHAR *, int, UCHAR *, UCHAR ** );
int        _FTPX_GetMode( UCHAR *, int );
int        _FTPX_GetWriteData( UCHAR *, int, FILE *, int *, UCHAR * );
int        _FTPX_PutReadData( FILE *, int (*)(UCHAR *, UINT), UCHAR * );
void       _FTPX_PIDataCB( UINT, UCHAR *, UINT );
void       _FTPX_PICtrlCB( int, ... );
void       _FTPX_DTPDataCB( UINT, UCHAR *, UINT );
void       _FTPX_DTPCtrlCB( int, ... );
int        _FTPX_PISendCmd( UCHAR *, UINT *, UCHAR * );
int        _FTPX_PIGetDTPResponse( void );
int        _FTPX_PISendDTPCmd( UCHAR *, UINT *, UCHAR * );
int        _FTPX_SetMode( UINT, UCHAR * );
int        _FTPX_SetCwd( UCHAR *, UCHAR * );
int        _FTPX_FTPInit( UCHAR *, UCHAR *, UCHAR *, UCHAR * );
int        _FTPX_FTPClose( UCHAR * );
int        _FTPX_FTPRenFile( UCHAR *, UCHAR *, UCHAR *, UCHAR * );
int        _FTPX_FTPRcvFile( UCHAR *, UCHAR *, UCHAR *, UINT, UCHAR * );
int        _FTPX_FTPXmitFile( UCHAR *, UCHAR *, UCHAR *, UINT, UCHAR * );

#endif    /* SDD_FTPX_H */
