/******************************************************************************
 * Product:       #     # #     #         #         ###   ######
 *                #     #  #   #          #          #    #     #
 *                #     #   # #           #          #    #     #
 *                #     #    #            #          #    ######
 *                #     #   # #           #          #    #     #
 *                #     #  #   #          #          #    #     #
 *                 #####  #     # ####### #######   ###   ######
 *
 * File:          ux_linkl.c
 * Description:   A library of linked list functions for creating, deleting,
 *                searching (etc..) linked lists.
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
#define        UX_LINKEDLIST_C

/* Bring in specific header files.
*/
#include    "ux.h"

/******************************************************************************
 * Function:    AddItem
 * Description: A simplistic mechanism to compose a linked list. The link
 *              is only singly linked, and items are only added to the tail
 *              of the list.
 * Returns:     R_OK      - Item added successfully.
 *              R_FAIL    - Failure in addition, see Errno.
 * <Errno>      E_NOMEM   - Memory exhaustion.
 *              E_BADHEAD - Head pointer is bad.
 *              E_BADTAIL - Tail pointer is bad.
 *              E_NOKEY   - No search key provided.
 ******************************************************************************/
int AddItem( LINKLIST    **spHead,    /* IO: Pointer to head of list */
             LINKLIST    **spTail,    /* IO: Pointer to tail of list */
             int         nMode,       /* I: Mode of addition to link */
             UINT        *nKey,       /* I: Integer based search key */
             ULNG        *lKey,       /* I: Long based search key */
             UCHAR       *szKey,      /* I: String based search key */
             void        *spData )    /* I: Address of carried data */
{
    /* Local variables.
    */
    char        *szFunc = "AddItem";
    LINKLIST    *pTmpLRec;
    LINKLIST    *pTailRec;
    LINKLIST    *spCur;
    LINKLIST    *spPrev;

    /* Quick check, no point adding to list if there is no data.
    */
    if(spData == NULL)
    {
        Errno = E_NODATA;
        return(R_FAIL);
    }

    /* Allocate enough memory for a linklist control block. This will be
     * tagged on to the end of the list... eventually!
    */
    if( (pTmpLRec=(LINKLIST *)malloc(sizeof(LINKLIST))) == NULL)
    {
        Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
            sizeof(LINKLIST));
        Errno = E_NOMEM;
        return(R_FAIL);
    }
    memset((UCHAR *)pTmpLRec, '\0', sizeof(LINKLIST));

    /* If a text based search key provided, need to allocate space in which
     * to store it. If allocation succeeds, dup key.
    */
    if(szKey != NULL)
    {
        if( (pTmpLRec->szKey=(UCHAR *)malloc(strlen(szKey)+1)) == NULL)
        {
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                strlen(szKey)+1);
            free(pTmpLRec);
            Errno = E_NOMEM;
            return(R_FAIL);
        } else
         {
            strcpy(pTmpLRec->szKey, szKey);
        }
    }
    
    /* Populate linklist.
    */
    pTmpLRec->nKey   = (nKey == NULL ? 0  : *nKey);
    pTmpLRec->lKey   = (lKey == NULL ? 0L : *lKey);
    pTmpLRec->spData = spData;

    /* Right, we have a record, so where do we add it.
    */
    if(*spHead == NULL)
    {
        /* Both pointers look at new element.
        */
        *spHead = pTmpLRec;
        *spTail = pTmpLRec;
        pTmpLRec->spNext = NULL;
    } else
     {
        /* If were sorting the list as we go along, then we need to scan it and
         * find the required location.
        */
        if(nMode != SORT_NONE)
        {
            for(spPrev=NULL, spCur= *spHead; spCur != NULL; 
                spPrev=spCur, spCur=spCur->spNext)
            {
                if(nMode == SORT_INT_UP    && pTmpLRec->nKey < spCur->nKey)
                    break;
                else
                if(nMode == SORT_INT_DOWN  && pTmpLRec->nKey > spCur->nKey)
                    break;
                else
                if(nMode == SORT_LONG_UP   && pTmpLRec->lKey < spCur->lKey)
                    break;
                else
                if(nMode == SORT_LONG_DOWN && pTmpLRec->lKey > spCur->lKey)
                    break;
            }

            /* Error condition, should not occur?
            */
            if(spPrev == NULL && spCur == NULL)
            {
                if(pTmpLRec->szKey != NULL)
                    free(pTmpLRec->szKey);
                free(pTmpLRec);
                return(R_FAIL);
            } else

            /* Insert at very beginning of list?
            */
            if(spPrev == NULL && spCur != NULL)
            {
                pTmpLRec->spNext = *spHead;
                *spHead = pTmpLRec;
            } else

            /* Insert in the middle of the list?
            */
            if(spPrev != NULL && spCur != NULL)
            {
                pTmpLRec->spNext = spPrev->spNext;
                spPrev->spNext   = pTmpLRec;
            } else

            /* Insert at the end of the list!
            */
             {
                /* Add to tail of list by making tail point to new item, then 
                 * new item becomes the tail.
                */
                pTailRec         = *spTail;
                pTailRec->spNext = pTmpLRec;
                *spTail          = pTmpLRec;
                pTmpLRec->spNext = NULL;
            }
        } else
         {
            /* Add to tail of list by making tail point to new item, then 
             * new item becomes the tail.
            */
            pTailRec         = *spTail;
            pTailRec->spNext = pTmpLRec;
            *spTail          = pTmpLRec;
            pTmpLRec->spNext = NULL;
        }
    }

    /* Return success or fail...?
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    DelItem
 * Description: Delete an element from a given linked list. The underlying
 *              carried data is not freed, it is assumed that the caller
 *              will free that, as it was the caller that allocated it.
 * Returns:     R_OK      - Item deleted successfully.
 *              R_FAIL    - Failure in deletion, see Errno.
 * <Errno>      E_BADHEAD - Head pointer is bad.
 *              E_BADTAIL - Tail pointer is bad.
 *              E_MEMFREE - Couldnt free memory to sys pool.
 *              E_NOKEY   - No search key provided.
 ******************************************************************************/
int DelItem( LINKLIST    **spHead,   /* IO: Pointer to head of list */
             LINKLIST    **spTail,   /* IO: Pointer to tail of list */
             void        *spKey,     /* I: Addr of item, direct update */
             UINT        *nKey,      /* I: Integer based search key */
             ULNG        *lKey,      /* I: Long based search key */
             UCHAR       *szKey )    /* I: String based search key */
{
    /* Local variables.
    */
    int         nResult = R_FAIL;
    LINKLIST    *spCur;
    LINKLIST    *spPrev;

    /* Check input values. Is head valid?
    */
    if(*spHead == NULL)
    {
        Errno = E_BADHEAD;
        return(nResult);
    }

    /* Is tail valid?
    */
    if(*spTail == NULL)
    {
        Errno = E_BADTAIL;
        return(nResult);
    }

    /* Have search keys been provided.
    */
    if(spKey == NULL && nKey == NULL && lKey == NULL && szKey == NULL)
    {
        Errno = E_NOKEY;
        return(nResult);
    }

    /* Locate item by scanning the list. This may get updated in years to
     * come to be a hash/btree lookup/delete.... dream on!!
    */
    for(spPrev=NULL, spCur= *spHead; spCur != NULL; 
        spPrev=spCur, spCur=spCur->spNext)
    {
        /* See if we have a match!
        */
        if( (spKey != NULL && spCur->spData != spKey) ||
            (nKey != NULL && *nKey != spCur->nKey) ||
            (lKey != NULL && *lKey != spCur->lKey) ||
            (szKey != NULL && spCur->szKey != NULL &&
             strcmp(szKey, spCur->szKey) != 0))
            continue;

        /* OK, found the one.
        */
        break;
    }

    /* If records not null, then we have located the required record, remove
     * it.
    */
    if(spCur != NULL)
    {
        /* Item at beginning of list?
        */
        if(spPrev == NULL)
        {
            /* Point head at next in list. If next is NULL, then list empty,
             * so update Tail.
            */
            if((*spHead = spCur->spNext) == NULL)
                *spTail = NULL;
        } else
         {
            if((spPrev->spNext = spCur->spNext) == NULL)
                *spTail = spPrev;
        }

        /* Free memory used by removed element.
        */
        if(spCur->szKey != NULL)
            free(spCur->szKey);
        free(spCur);
        nResult = R_OK;
    }

    /* Return success or fail...?
    */
    return(nResult);
}

/******************************************************************************
 * Function:    FindItem
 * Description: Find an element in a given linked list.
 * Returns:     NOTNULL    - Item found, address returned.
 *              NULL       - Item not found, see Errno.
 * <Errno>      E_BADHEAD - Head pointer is bad.
 *              E_BADTAIL - Tail pointer is bad.
 *              E_NOKEY   - No search key provided.
 ******************************************************************************/
void *FindItem( LINKLIST    *spHead,    /* I: Pointer to head of list */
                UINT        *nKey,      /* I: Integer based search key */
                ULNG        *lKey,      /* I: Long based search key */
                UCHAR       *szKey )    /* I: String based search key */
{
    /* Local variables.
    */
    UCHAR        *spResult = NULL;
    LINKLIST    *spCur;

    /* Quite simple at the momoko, just loop through the list and see if an
     * entry exists. Eventually, (he hopes) this could be enhanced to inc
     * btree/hash lookup.
    */
    for(spCur=spHead; spCur != NULL; spCur=spCur->spNext)
    {
        if(nKey != NULL && spCur->nKey == *nKey)
            break;

        if(lKey != NULL && spCur->lKey == *lKey)
            break;

        if(szKey != NULL && strcmp(szKey, spCur->szKey) == 0)
            break;
    }

    /* Found a record?
    */
    if( spCur != NULL )
        spResult = spCur->spData;

    /* Return success or fail...?
    */
    return(spResult);
}

/******************************************************************************
 * Function:    StartItem
 * Description: Setup pointers for a complete list scan. Return the top most
 *              list 'data item' to caller.
 * Returns:     NOTNULL    - Item found, address returned.
 *              NULL       - Item not found, see Errno.
 * <Errno>      E_BADHEAD - Head pointer is bad.
 ******************************************************************************/
void *StartItem( LINKLIST    *spHead,      /* I: Pointer to head of list */
                 LINKLIST    **spNext )    /* O: Pointer to next item in list */
{
    /* Local variables.
    */
    void    *spResult = NULL;

    /* Does the list exist yet?
    */
    if( spHead == NULL )
    {
        Errno = E_BADHEAD;
    } else
     {
        /* Setup pointer to next in link and return pointer to data to caller.
        */
        *spNext = (LINKLIST *)spHead->spNext;
        spResult = (UCHAR *)spHead->spData;
    }

    /* Return success or fail...?
    */
    return(spResult);
}

/******************************************************************************
 * Function:    NextItem
 * Description: Move to next item in a given list. Return the current 'data
 *              item' to caller.
 * Returns:     NOTNULL    - Item found, address returned.
 *              NULL       - Item not found, see Errno.
 * <Errno>      E_BADPARM - Bad parameter passed to function.
 ******************************************************************************/
void *NextItem( LINKLIST    **spNext )    /* O: Pointer to next item in list */
{
    /* Local variables.
    */
    void        *spResult = NULL;
    LINKLIST    *spLink = *spNext;

    /* Check parameter, maybe at end of list already.
    */
    if( *spNext == NULL )
    {
        Errno = E_BADPARM;
    } else
     {
        /* Extract pointer to data and move down link.
        */
        spResult = (UCHAR *)spLink->spData;
        *spNext = (LINKLIST *)spLink->spNext;
    }

    /* Return success or fail...?
    */
    return(spResult);
}

/******************************************************************************
 * Function:    MergeLists
 * Description: Merge two list together. The Source list is merged into the
 *              target list. Lists are re-sorted if required.
 * Returns:     R_OK      - Item added successfully.
 *              R_FAIL    - Failure in addition, see Errno.
 * <Errno>      E_NOMEM   - Memory exhaustion.
 *              E_BADHEAD - Head pointer is bad.
 *              E_BADTAIL - Tail pointer is bad.
 *              E_NOKEY   - No search key provided.
 ******************************************************************************/
int MergeLists( LINKLIST    **spDstHead,   /* IO: Pointer to head of dest list */
                LINKLIST    **spDstTail,   /* IO: Pointer to tail of dest list */
                LINKLIST    *spSrcHead,    /* I: Pointer to head of src list */
                LINKLIST    *spSrcTail,    /* I: Pointer to tail of src list */
                int          nMode )       /* I: Mode of list merging  */
{
    /* Local variables.
    */
    LINKLIST    *spNext;
    LINKLIST    *spSrc;
    LINKLIST    *pTailRec;
    LINKLIST    *spCur;
    LINKLIST    *spPrev;

    /* Go through each record in the source list and add onto the destination
     * list.
    */
    if((spSrc = spSrcHead) != NULL)
        spNext = spSrcHead->spNext;
    else
        spNext = NULL;
    
    /* Loop through the entire source list and merge into the destination list.
    */
    while(spSrc != NULL)
    {
        /* Right, we have a record, so where do we add it.
        */
        if(*spDstHead == NULL)
        {
            /* Both pointers look at new element.
            */
            *spDstHead = spSrc;
            *spDstTail = spSrc;
            spSrc->spNext = NULL;
        } else
         {
            /* If were sorting the list as we go along, then we need to scan it
             * and find the required location.
            */
            if(nMode != SORT_NONE)
            {
                for(spPrev=NULL, spCur= *spDstHead; spCur != NULL; 
                    spPrev=spCur, spCur=spCur->spNext)
                {
                    if(nMode == SORT_INT_UP    && spSrc->nKey < spCur->nKey)
                        break;
                    else
                    if(nMode == SORT_INT_DOWN  && spSrc->nKey > spCur->nKey)
                        break;
                    else
                    if(nMode == SORT_LONG_UP   && spSrc->lKey < spCur->lKey)
                        break;
                    else
                    if(nMode == SORT_LONG_DOWN && spSrc->lKey > spCur->lKey)
                        break;
                }

                /* Insert at very beginning of list?
                */
                if(spPrev == NULL && spCur != NULL)
                {
                    spSrc->spNext = *spDstHead;
                    *spDstHead = spSrc;
                } else

                /* Insert in the middle of the list?
                */
                if(spPrev != NULL && spCur != NULL)
                {
                    spSrc->spNext  = spPrev->spNext;
                    spPrev->spNext = spSrc;
                } else

                /* Insert at the end of the list!
                */
                {
                    /* Add to tail of list by making tail point to new item,
                     * then new item becomes the tail.
                    */
                    pTailRec         = *spDstTail;
                    pTailRec->spNext = spSrc;
                    *spDstTail       = spSrc;
                    spSrc->spNext = NULL;
                }
            } else
             {
                /* Add to tail of list by making tail point to new item, then 
                 * new item becomes the tail.
                */
                pTailRec         = *spDstTail;
                pTailRec->spNext = spSrc;
                *spDstTail       = spSrc;
                spSrc->spNext    = NULL;
            }
        }

        /* Move to next element.
        */
        if((spSrc = spNext) != NULL)
            spNext = spSrc->spNext;
    }

    /* Return success or fail...?
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    DelList
 * Description: Delete an entire list and free memory used by the list and the
 *              underlying carried data.
 * Returns:     R_OK      - List deleted successfully.
 *              R_FAIL    - Failed to delete list, see Errno.
 * <Errno>      E_BADHEAD - Head pointer is bad.
 *              E_BADTAIL - Tail pointer is bad.
 ******************************************************************************/
int DelList( LINKLIST    **spHead,     /* IO: Pointer to head of list */
             LINKLIST    **spTail )    /* IO: Pointer to tail of list */
{
    /* Local variables.
    */
    LINKLIST    *spTmp;
    LINKLIST    *spNext;

    /* Check input values. Is head valid?
    */
    if(*spHead == NULL)
    {
        Errno = E_BADHEAD;
        return(R_FAIL);
    }

    /* Is tail valid?
    */
    if(*spTail == NULL)
    {
        Errno = E_BADTAIL;
        return(R_FAIL);
    }

    /* Quite simple, breeze through list, deleting everything.
    */
    for(spTmp= *spHead; spTmp != NULL; spTmp=spNext)
    {
        /* Free any memory allocated for text search buffer.
        */
        if(spTmp->szKey != NULL)
            free(spTmp->szKey);

        /* Free any memory allocated for data record.
        */
        if(spTmp->spData != NULL)
            free(spTmp->spData);

        /* Get next element then release element memory.
        */
        spNext=spTmp->spNext;
        free(spTmp);
    }

    /* Tidy up callers pointers.
    */
    *spHead = *spTail = NULL;

    /* Got here, perhaps everything worked...!
    */
    return(R_OK);
}

/******************************************************************************
 * Function:    SizeList
 * Description: Find the total number of elements in a given list by scanning
 *              it.
 * Returns:     R_OK      - List size calculated.
 *              R_FAIL    - Failed to calculate list size, see Errno.
 * <Errno>      E_BADHEAD - Head pointer is bad.
 ******************************************************************************/
int SizeList( LINKLIST    *spHead,       /* I: Pointer to head of list */
              UINT        *nCnt )        /* O: Count of elements in list */
{
    /* Local variables.
    */
    LINKLIST    *spTmp;

    /* Check input values. Is head valid?
    */
    if(spHead == NULL)
    {
        Errno = E_BADHEAD;
        return(R_FAIL);
    }

    /* Quite simple, breeze through list, counting.
    */
    for(*nCnt=0,spTmp=spHead; spTmp != NULL; *nCnt += 1, spTmp=spTmp->spNext);

    /* Got here, perhaps everything worked...!
    */
    return(R_OK);
}
