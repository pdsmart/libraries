/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_thrd.c
 * Description:   UX Thread Management routines. A module with the specific aim
 *                of invoking, charting and termination of threads required by
 *                programs built with this library.
 *
 * Version:       %I%
 * Dated:         %D%
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
 ******************************************************************************/

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
#define        UX_THREADS_C

/* Bring in specific header files.
*/
#include    "ux.h"

/******************************************************************************
 * Internal Library functions.
 ******************************************************************************/

/* Internal structures: A global Mutex lock set, split up into sections,
 *                        which the threads examine and wait on according to
 *                        their state.
 *
 * No lock in all _SL functions, can only be entered by the SL library
 * thread. During initialisation, store id of SL thread, compare upon entry
 * to the library and exit with error if id incorrect.
 * 
 * Single lock for all SL functions. Upon entry, thread checks lock,
 * if its free, it sets it and proceeds. If its locked, it waits till it
 * becomes free.
 *
 *
*/

/* Internal Function: Create Thread and Add to Linked List. Linked List to
 *                       contain Mutex Locks for local locks and global lock set.
 *
 *                      Delete Thread.
*/

/******************************************************************************
 * Function:    _TM_<INTERNAL FUNCTION NAME>
 * Description: <Description of Function>
 *                 
 * Returns:     <RETURN PARAMETERS>.
 ******************************************************************************/




/******************************************************************************
 * User API functions.
 ******************************************************************************/

/* API Function: Create New Thread. Calls Internal function with Mutex
 *                 protection.
 *
 * API Function: Delete or Terminate existing thread. Calls Internal function.
 *
 *
*/

/******************************************************************************
 * Function:    TM_<API FUNCTION NAME>
 * Description: <Description of Function>
 *                 
 * Returns:     <RETURN PARAMETERS>.
 ******************************************************************************/

/******************************************************************************
 * Function:    TM_Init
 * Description: Function to initialise the Thread Management Library.
 *                 
 * Returns:     R_OK    - Succeeded.
 *              R_FAIL  - Failed to init, see errno and szErrMsg.
 ******************************************************************************/
int    TM_Init( UCHAR        *szErrMsg    )    /* O: Error message if function fails */
{
    /* Local variables.
    */
    char        *szFunc = "TM_Init";

    /* Initialise control&tracking linked list.
    */

    /* Got this far, exit with success.
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    TM_Exit
 * Description: Function to terminate all threads and clear the 
 *              control&tracking records. Must be the main parent thread
 *              ie. Thread0 that calls this function otherwise an error
 *              is returned.
 *                 
 * Returns:     R_OK    - Succeeded.
 *              R_FAIL  - Failed to exit, see errno and szErrMsg.
 ******************************************************************************/
int    TM_Exit( UCHAR        *szErrMsg    )    /* O: Error message if function fails */
{
    /* Local variables.
    */
    char        *szFunc = "TM_Init";

    /* Free up all resources used by the control&tracking linked list.
    */

    /* Got this far, exit with success.
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    TM_CreateThread
 * Description: Function to create a new thread. Uses current thread as 
 *              base, allocates a new control&tracking record which gets
 *              stored in the Thread Management linked list and releases
 *              the thread onto the supplied start function.
 *                 
 * Author:      P.D. Smart
 * Returns:     R_OK    - Succeeded.
 *              R_FAIL  - Failed to create thread, see errno and szErrMsg.
 ******************************************************************************/
int    TM_CreateThread(                                /* I: Start Function */
                        UCHAR        *szErrMsg    )    /* O: Error msg if func fails */
{
    /* Local variables.
    */
    char        *szFunc = "TM_CreateThread";


    /* Got this far, exit with success.
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    TM_KillThread
 * Description: Function to kill a requested thread. Once killed, the 
 *              control&tracking lists are updated.
 *                 
 * Returns:     R_OK    - Succeeded.
 *              R_FAIL  - Failed to kill thread, see errno and szErrMsg.
 ******************************************************************************/
int    TM_Create(                                /* I: Thread Id */
                  UCHAR        *szErrMsg    )    /* O: Error msg if func fails */
{
    /* Local variables.
    */
    char        *szFunc = "TM_KillThread";


    /* Got this far, exit with success.
    */
    return(R_OK);
}

