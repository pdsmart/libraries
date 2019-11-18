/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_cli.c
 * Description:   Unix or Windows Command Line Processing functions.
 *
 * Version:       %I%
 * Dated:         %D%
 * Author:        P.D. Smart
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
 ******************************************************************************

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
#define        UX_CLI_C

/* Bring in specific header files.
*/
#include    "ux.h"

/******************************************************************************
 * Function:    GetCLIParam
 * Description: Get a Command Line Interface Parameter from the command line
 *              of the shell which started this program... Confusing!
 * Returns:     R_OK   - Parameter obtained.
 *              R_FAIL - Parameter does not exist.
 ******************************************************************************/
int    GetCLIParam( int     nArgc,        /* I: Argc or equiv */
                    UCHAR   **Argv,       /* IO: Argv or equiv */
                    UCHAR   *szParm,      /* I: Param flag to look for */
                    UINT    nParmType,    /* I: Type of param (ie int) */
                    UCHAR   *pVar,        /* O: Pointer to variable for parm */
                    UINT    nVarLen,      /* I: Length pointed to by pVar */
                    UINT    nZapParam )   /* I: Delete argument after proc */

{
    /* Local variables.
    */
    int            nNdx;
    int            nReturn = R_FAIL;
    UCHAR          *pArg = NULL;

    /* Scan through the command line, seeing if the required flag exists.
    */
    for(nNdx=1; nNdx < nArgc; nNdx++)
    {
        if(strcasecmp(Argv[nNdx], szParm) == 0)
        {
            /* Has the user provided a parameter?
            */
            if( strlen(Argv[nNdx]) < strlen(szParm) )
            {
                pArg = Argv[nNdx]+strlen(Argv[nNdx]);
            } else
            if( nNdx+1 <= nArgc )
            {
                pArg = Argv[nNdx+1];
            } else pArg = NULL;
            break;
        }
    }

    /* Found the required flag?
    */
    if( nNdx != nArgc && pArg != NULL )
    {
        /* Convert the parameter into the required destination format.
        */
        switch( nParmType )
        {
            case T_INT:
                if(sscanf(pArg, "%d", (UINT *)pVar) == 1)
                    nReturn = R_OK;
                break;

            case T_STR:
                if(sscanf(pArg, "%s", (UCHAR *)pVar) == 1)
                    nReturn = R_OK;
                break;

            case T_CHAR:
                if(sscanf(pArg, "%c", (UCHAR *)pVar) == 1)
                    nReturn = R_OK;
                break;

            case T_LONG:
                if(sscanf(pArg, "%ld", (ULNG *)pVar) == 1)
                    nReturn = R_OK;
                break;

            default:
                break;
        }

        /* If the caller has requested that the command line argument
         * be deleted for security purposes, delete!
        */
        if(nZapParam == TRUE)
        {
            memset(pArg, '*', strlen(pArg));
        }
    } else
    /* If a valid flag appeared, but it did not have an argument, then
     * assume its a state flag, so set the var parameter to NULL but return
     * success.
    */
    if( nNdx != nArgc && pArg == NULL )
    {
        pVar = NULL;
        nReturn = R_OK;
    }

    /* Finished, get out!!
    */
    return( nReturn );
}
