/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_odbc.h
 * Description:   Server Data-source Driver library driver header file for
 *                odbc driver.
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
#ifndef    SDD_ODBC_H
#define    SDD_ODBC_H

/* Definitions for maxims etc.
*/
#define    MAX_DSN_DESCR_LEN      255
#define    MAX_TABLE_DESCR_LEN    255

/* Definitions for constants, configurable options etc.
*/
#define    ODBC_COLFILTER         "COLUMN="
#define    ODBC_DBNAME            "DBNAME="
#define    ODBC_TABLENAME         "TABLENAME="

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
    HENV        hEnv;                           /* Environment buffer for ODBC*/
    HDBC        hDbc;                           /* ODBC connection handle */
} ODBC_DRIVER;

/* Allocate any global variables needed within this module.
*/
static    ODBC_DRIVER        ODBC;

/* Prototypes of internal functions, not seen by any outside module.
*/
void      _ODBC_LogODBCError(    HSTMT );
int       _ODBC_GetArg( UCHAR *, UCHAR *, int, UCHAR ** );
int       _ODBC_RunSql( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
int       _ODBC_ListDB( int (*)(UCHAR *, UINT), UCHAR * );
int       _ODBC_ListTables( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );
int       _ODBC_ListCols( UCHAR *, int, int (*)(UCHAR *, UINT), UCHAR * );

#endif    /* SDD_ODBC_H */
