/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_scmd.h
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
#ifndef    SDD_SCMD_H
#define    SDD_SCMD_H

/* Definitions for constants, configurable options etc.
*/
#define    SCMD_ASCII              "ASCII"
#define    SCMD_ARGS               "ARGS="
#define    SCMD_BINARY             "BINARY"
#define    SCMD_CMD                "COMMAND="
#define    SCMD_DATA               "DATA="
#define    SCMD_END                "END="
#define    SCMD_MODE               "MODE="
#define    SCMD_BUFSIZE            "BUFSIZE="
#define    SCMD_TIME               "TIME="
#define    SCMD_SRCPATH            "SRCPATH="
#define    SCMD_DSTPATH            "DSTPATH="
#define    SCMD_SRCFILE            "SRCFILE="
#define    SCMD_DSTFILE            "DSTFILE="
#define    SCMD_ASCIIMODE          0
#define    SCMD_BINMODE            1

/* Definitions for maxims and defaults etc.
*/
#define    DEF_RETURN_BUF_SIZE     2048
#define    DEF_RETURN_MODE         SCMD_BINMODE
#define    MAX_MODE_SIZE           6
#define    MAX_FILENAME_LEN        128
#define    MIN_RETURN_BUF_SIZE     1
#define    MAX_RETURN_BUF_SIZE     65536

/* Structure for variables needed within this module.
*/
typedef struct {
    UINT        nRetBufSize;       /* Return buffer transmit size */
    UINT        nRetMode;          /* Return Transmit mode 1=B, 0=A */
} SCMD_DRIVER;

/* Allocate global variables by instantiating the above structure.
*/
static    SCMD_DRIVER        SCMD;

/* Prototypes of internal functions, not seen by any outside module.
*/
int    _SCMD_GetStrArg( UCHAR *, int, UCHAR *, UCHAR ** );
int    _SCMD_ValidatePath( UCHAR * );
int    _SCMD_ValidateFile( UCHAR *, UCHAR *, UINT );
int    _SCMD_ValidateTime( UCHAR *, ULNG * );
#if defined(SOLARIS) || defined(SUNOS)
int    _SCMD_Exec(int, UCHAR *, UCHAR *, UCHAR *, ULNG, int(*)(UCHAR *, UINT), UCHAR * );
#endif

#endif    /* SDD_SCMD_H */
