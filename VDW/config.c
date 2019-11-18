/******************************************************************************
 * Product:       #     # ######  #     #
 *                #     # #     # #  #  #  #####
 *                #     # #     # #  #  #  #    #
 *                #     # #     # #  #  #  #    #
 *                 #   #  #     # #  #  #  #    #
 *                  # #   #     # #  #  #  #    #
 *                   #    ######   ## ##   #####
 *
 * File:          config.c
 * Description:   Configuration file for the Virtual Data Warehouse server Daemon
 *                process. This configuration file indicates which drivers the
 *                final built daemon knows about and provides entry points for
 *                the main vdwd code to call the drivers.
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
#include    <mdc.h>
#include    <sdd.h>

/* Indicate that we are a C module for any header specifics.
*/
#define     VDWD_CONFIG_C

/* Bring in local specific header files.
*/
#include    "vdwd.h"

/* Driver entry point configuration table.
 * If you have a driver that you wish to embed within the daemon then enter
 * it into this table. Ordering is not relevant as you must provide a driver
 * type code which is known to both the client code and your driver.
*/
VDWD_DRIVERS    Driver[]={
/* #if defined(SOLARIS) || defined(_WIN32)
#    { SRV_ODBC,     odbc_InitService,       odbc_CloseService,    
#                    odbc_ProcessRequest,    odbc_ProcessOOB },
#endif */
#if defined(SOLARIS) || defined(SUNOS) || defined(_WIN32)
    { SRV_SYBASE,    sybc_InitService,       sybc_CloseService,
                     sybc_ProcessRequest,    sybc_ProcessOOB },
#endif
    { SRV_JAVA,      java_InitService,       java_CloseService,
                     java_ProcessRequest,    java_ProcessOOB },
    { SRV_SCMD,      scmd_InitService,       scmd_CloseService,
                     scmd_ProcessRequest,    scmd_ProcessOOB },
    { SRV_FTPX,      ftpx_InitService,       ftpx_CloseService,
                     ftpx_ProcessRequest,    ftpx_ProcessOOB },
/* #    { SRV_AUPL,  aupl_InitService,       aupl_CloseService,
#                    aupl_ProcessRequest,    aupl_ProcessOOB }, */
    { 0,             NULL,                   NULL,
                     NULL,                   NULL }
};
