/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_java.c
 * Description:   Server Data-source Driver library driver to handle java
 *                connectivity.
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
#include    <string.h>

/* Indicate that we are a C module for any header specifics.
*/
#define     SDD_JAVA_C

/* Bring in local specific header files.
*/
#include    "sdd.h"
#include    "sdd_java.h"

/******************************************************************************
 * Function:    java_InitService
 * Description: Entry point which initialises the driver into a defined state.
 *              It is mandatory that this function is called before any other
 *              in order for the driver to function correctly. The caller
 *              provides it with two types of data, 1) A structure containing
 *              data for it to use in initialising itself, 2) a pointer to a
 *              buffer which the driver uses to place an error message should
 *              it not be able to complete initialisation.
 * 
 * Returns:     SDD_FAIL- An error occurred in initialising the driver and an
 *                        error message is stored in szErrStr.
 *              SDD_OK    - Driver initialised successfully.
 ******************************************************************************/
int    java_InitService( SERVICEDETAILS   *sServiceDet,    /* I: Init data */
                         UCHAR            *szErrStr )      /* O: Error message */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "java_InitService";

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "Java Driver: Initialised:");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    java_CloseService
 * Description: Entry point which performs a drive closedown. The closedown
 *              procedure ensure that the driver returns to a virgin state
 *              (ie.like at power up) so that InitService can be called again.
 * 
 * Returns:     SDD_FAIL- An error occurred in closing the driver and an
 *                        error message is stored in szErrStr.
 *              SDD_OK    - Driver successfully closed.
 ******************************************************************************/
int    java_CloseService( UCHAR        *szErrMsg )    /* O: Error message if failed */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "java_CloseService";

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "Java Driver: Closed.");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    java_ProcessRequest
 * Description: Entry point into driver to initiate the driver into 
 *              processing a request. A data block is passed as a parameter
 *              to the driver which represents a request with relevant
 *              parameters. The data within the structure is only relevant
 *              to the original client and this driver code.
 * 
 * Returns:     SDD_FAIL- An error occurred within the driver whilst trying to
 *                        process the request, see error text.
 *              SDD_OK    - Request processed successfully.
 ******************************************************************************/
int    java_ProcessRequest( UCHAR    *snzDataBuf,           /* I: Input data */
                            int      nDataLen,              /* I: Len of data */
                            int      (*fSendDataCB)(UCHAR *, UINT),
                                                            /* I: CB to send reply*/
                            UCHAR    *szErrMsg )            /* O: Error text */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "java_ProcessRequest";

    /* If the request block doesnt contain any data, something went wrong
     * somewhere??
    */
    if(nDataLen <= 1)
    {
        sprintf(szErrMsg,
                "%s: Illegal request, has size of %dBytes",
                SDD_EMSG_BADREQ, nDataLen);
        return(SDD_FAIL);
    }

    /* First byte of the request data block indicates actions required, so
     * decipher it.    
    */
    switch(snzDataBuf[0])
    {
        default:
            sprintf(szErrMsg,
                    "%s: Illegal command in request buffer (%x)",
                    SDD_EMSG_BADCMD, snzDataBuf[0]);
            return(SDD_FAIL);
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    java_ProcessOOB
 * Description: Entry point into driver to process an out of band command
 *              that may or may not be relevant to current state of
 *              operation. The task of this function is to decipher the
 *              command and act on it immediately, ie. a cancel command
 *              would abort any ProcessRequest that is in process and
 *              clean up.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void    java_ProcessOOB( UCHAR    nCommand    )    /* I: OOB Command */
{
    /* Local variables.
    */

    /* Decipher command and perform actions accordingly.
    */
    switch(nCommand)
    {
        /* Request to abort current ProcessRequest command and return daemon
         * to a waiting-for-request state.
        */
        case SDD_ABORT:
            break;

        /* Request to close down and exit.
        */
        case SDD_EXIT:
            break;

        default:
            break;
    }


    /* Return to caller.
    */
    return;
}
