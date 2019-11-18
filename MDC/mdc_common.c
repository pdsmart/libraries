/******************************************************************************
 * Product:       #     # ######   #####          #         ###   ######
 *                ##   ## #     # #     #         #          #    #     #
 *                # # # # #     # #               #          #    #     #
 *                #  #  # #     # #               #          #    ######
 *                #     # #     # #               #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                #     # ######   #####  ####### #######   ###   ######
 *
 * File:          mdc_common.c
 * Description:   Meta Data Communications common functions shared between
 *                the client and server API interfaces.
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
#if defined(SOLARIS)
#include    <thread.h>
#endif
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <stdarg.h>
#include    <string.h>
#include    <ux.h>

/* Indicate that we are a C module for any header specifics.
*/
#define     MDC_COMMON_C

/* Bring in local specific header files.
*/
#include    "mdc.h"
#include    "mdc_common.h"

/******************************************************************************
 * Function:    _MDC_Init
 * Description: Function to perform MDC initialisation. The library performs
 *              checking to ensure that initialisation only occurs once
 *              prior to an _MDC_Terminate.
 * 
 * Returns:     MDC_FAIL- Couldnt initialise library.
 *              MDC_OK    - Library initialised.
 ******************************************************************************/
int    _MDC_Init( void  )
{
    /* Local variables.
    */
    static int    nInitialised = FALSE;
    int           nReturn;
    UCHAR         *szFunc = "_MDC_Init";

    /* If this is the first invocation, setup the global state flag to
     * a known value.
    */
    if(nInitialised == FALSE)
    {
        nInitialised = TRUE;
        MDC.nInitialised = FALSE;

        MDC.nMDCCommsMode = FALSE;
        MDC.spChanDetHead = NULL;
        MDC.spChanDetTail = NULL;
    }

    /* Have we been initialised already..? If we have, just get out.
    */
    if( MDC.nInitialised == FALSE )
    {
        /* Initialise all so-called globals.
        */
        MDC.spIFifoHead = NULL;
        MDC.spIFifoTail = NULL;
        MDC.spOFifoHead = NULL;
        MDC.spOFifoTail = NULL;
        MDC.nClientChanId = 0;
        MDC.nCloseDown = FALSE;
        MDC.nInitialised = TRUE;
        MDC.nNewSrvTimeout = DEF_NEW_SERVICE_TIMEOUT;
        MDC.nSrvReqTimeout = DEF_SRV_REQ_TIMEOUT;
        MDC.nSndReqTimeout = DEF_SEND_REQ_TIMEOUT;
#if defined(SOLARIS)
        /* MDC.thMDCLock = DEFAULTMUTEX; */
#endif

        /* Initialise UX comms
        */
        if ((nReturn = SL_Init(MDC_SRV_KEEPALIVE, (UCHAR *) NULL)) != R_OK)
        {
            Lgr(LOG_DEBUG, szFunc, "SL_Init failed");
            return(MDC_FAIL);
        }
    }

    /* Return result to caller.
    */
    return(MDC_OK);
}

/******************************************************************************
 * Function:    _MDC_Terminate
 * Description: Function to shutdown the MDC library. After a successful
 *              shutdown, _MDC_Init may be called to re-initialise the
 *              library. If MDC_Terminate fails, program exit is advised.
 * 
 * Returns:     MDC_FAIL- Couldnt perform a clean shutdown.
 *              MDC_OK    - Library successfully shutdown.
 ******************************************************************************/
int    _MDC_Terminate( void )
{
    /* Local variables.
    */

    /* Free up all resources that weve used to return us to a virgin state.
    */

    /* Give the UX library a slice of CPU so that it can tidy up.
    */
    SL_Poll(MAX_TERMINATE_TIME);

    /* Reset the initialisation flag so that user code can re-initialise us.
    */
    MDC.nInitialised = FALSE;

    /* Return result to caller.
    */
    return(MDC_OK);
}
