/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_sybc.h
 * Description:   Server Data-source Driver library driver header file for
 *                sybc driver.
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
#ifndef    SDD_SYBC_H
#define    SDD_SYBC_H

/* Definitions for maxims, defaults etc.
*/
#define    DEF_APPNAME            "sdd_sybc"
#define    DEF_LINELEN            1024
#define    MAX_SYBC_COLNAME       256
#define    MAX_SYBC_DBNAME        256
#define    MAX_SYBC_TABLENAME     256
#define    MAX_SYBC_TYPENAME      256

/* Definitions for constants, configurable options etc.
*/
#define    SYBC_COLFILTER         "COLUMN="
#define    SYBC_DBNAME            "DBNAME="
#define    SYBC_TABLENAME         "TABLENAME="

/* Structure for variables internal to this driver module.
*/
typedef struct {
    UINT        nAbortPending;                  /* An abort is pending */
    UCHAR        szUserName[MAX_USERNAMELEN];   /* Name of user that this */
                                                /* module will use when */
                                                /* connecting to data source. */
    UCHAR        szPassword[MAX_PASSWORDLEN];   /* Password that this module */
                                                /* will use when connecting */
                                                /* to data source. */
    UCHAR        szServer[MAX_SERVERNAMELEN];   /* Name of Data Source server */
    UCHAR        szDatabase[MAX_DBNAMELEN];     /* Name of Data Source data */
                                                /* base. */
    DBPROCESS    *dbProc;                       /* Process handle into sybase */
    LOGINREC    *sLoginRec;                     /* Login record for making */
                                                /* and maintaining connection */
                                                /* with sybase */
} SYB_DRIVER;

/* Allocate any global variables needed within this module.
*/
static    SYB_DRIVER        SYB;

/* Prototypes of internal functions, not seen by any outside module.
*/
int    _SYBC_GetArg( UCHAR *, UCHAR *, int, UCHAR ** );
int    _SYBC_RunSql( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
int    _SYBC_ListDB( int (*)(UCHAR *, UINT), UCHAR * );
int    _SYBC_ListTables( UCHAR    *, int, int (*)(UCHAR *, UINT), UCHAR * );
int    _SYBC_ListCols( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );

#endif    /* SDD_SYBC_H */
