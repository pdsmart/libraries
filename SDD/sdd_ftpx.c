/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_ftpx.c
 * Description:   Server Data-source Driver library driver to handle direct
 *                FTP transfers to an FTP server.
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
#include    <sys/types.h>
#include    <sys/stat.h>

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
#include    <sys/wait.h>
#include    <unistd.h>
#endif

/* Indicate that we are a C module for any header specifics.
*/
#define     SDD_FTPX_C

/* Bring in local specific header files.
*/
#include    "sdd.h"
#include    "sdd_ftpx.h"

/******************************************************************************
 * Function:    _FTPX_GetStrArg
 * Description: Function to scan an input buffer and extract a string based
 *              argument.
 * 
 * Returns:     SDD_FAIL- Couldnt obtain argument.
 *              SDD_OK    - Argument obtained.
 ******************************************************************************/
int    _FTPX_GetStrArg( UCHAR    *snzDataBuf,       /* I: Input buffer */
                        int      nDataLen,          /* I: Len of data */
                        UCHAR    *szArg,            /* I: Arg to look for */
                        UCHAR    **pszPath )        /* O: Pointer to argument */
{
    /* Local variables.
    */
    int          nCmpLen;
    int          nPos;
    int          nReturn = SDD_FAIL;
    UCHAR        *szFunc = "_FTPX_GetStrArg";

    /* Go through the input buffer looking for 'szArg' within the
     * input buffer. If it exists, then note the position just after it
     * as this contains the string argument.
    */
    for(nPos=0, nCmpLen=strlen(szArg); nPos < nDataLen; nPos++)
    {
        /* If a match occurs, then setup the pointer to correct location.
        */
        if(strncmp(&snzDataBuf[nPos], szArg, nCmpLen) == 0)
        {
            /* Make sure that the match is not a sub-match as some of the
             * variables we are scanning for are similar.
            */
            if( (nPos == 0) || 
                (nPos > 0 && snzDataBuf[nPos-1] == '\0') ||
                (nPos > 0 && isspace(snzDataBuf[nPos-1])) )
            {
                nPos += nCmpLen;
                break;
            }
        }
    }

    /* If the pointer did not reach the end of the buffer then we have
     * located a valid name.
    */
    if(nPos < nDataLen)
    {
        /* Setup the callers pointer to point to the correct location
         * in the buffer.
        */
        *pszPath = &snzDataBuf[nPos];
        nReturn=SDD_OK;
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_GetMode
 * Description: Function to scan an input buffer and determine the mode 
 *              of FTP operation that the caller requires (Binary or Ascii). If
 *              no mode is provided then default to binary.
 * 
 * Returns:     Mode Flag - 1 = Binary mode selected.
 *                        - 0 = Ascii mode selected.
 ******************************************************************************/
int    _FTPX_GetMode( UCHAR    *snzDataBuf,     /* I: Input buffer */
                      int      nDataLen    )    /* I: Len of data */
{
    /* Local variables.
    */
    int          nCmpLen;
    int          nPos;
    int          nReturn = FTP_BINARY_MODE;
    UCHAR        *szFunc = "_FTPX_GetMode";

    /* Go through the input buffer looking for the MODE directive. If it
     * exists then note the position just after it as this is the flag
     * value indicating BINARY or ASCII.
    */
    for(nPos=0, nCmpLen=strlen(FTP_MODE); nPos < nDataLen; nPos++)
    {
        /* If a match occurs, then setup the pointer to correct location.
        */
        if(strncmp(&snzDataBuf[nPos], FTP_MODE, nCmpLen) == 0)
        {
            /* Make sure that the match is not a sub-match as some of the
             * variables we are scanning for are similar.
            */
            if( (nPos == 0) || 
                (nPos > 0 && snzDataBuf[nPos-1] == '\0') ||
                (nPos > 0 && isspace(snzDataBuf[nPos-1])) )
            {
                nPos += nCmpLen;
                break;
            }
        }
    }

    /* If the pointer did not reach the end of the buffer then we have
     * located the beginning of the argument list.
    */
    if(nPos < nDataLen)
    {
        /* Does the user require Ascii mode? If he doesnt, then assume binary
         * as its pointless error trapping here.
        */
        if(strcasecmp(&snzDataBuf[nPos], FTP_ASCII) == 0)
        {
            nReturn = FTP_ASCII_MODE;
        }
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_GetWriteData
 * Description: Function to scan an input buffer, verify that it has data in it,
 *              extract the data and store in the opened file stream and
 *              set the start flag if the block is the final block.
 * 
 * Returns:     SDD_FAIL- Bad block of data or error writing to file.
 *              SDD_OK    - Block obtained and stored.
 ******************************************************************************/
int    _FTPX_GetWriteData( UCHAR    *snzDataBuf,       /* I: Input buffer */
                           int      nDataLen,          /* I: Len of data */
                           FILE     *fpFile,           /* IO: Opened file stream */
                           int      *nLast,            /* O: Last block flag */
                           UCHAR    *szErrMsg )        /* O: Any resultant error msg */
{
    /* Local variables.
    */
    int          nBlockSize;
    int          nCmpDataLen;
    int          nCmpEndLen;
    int          nPos;
    int          nReturn = SDD_FAIL;
    UCHAR        *szFunc = "_FTPX_GetWriteData";

    /* Initialise any required variables.
    */
    *nLast = FALSE;
    nCmpDataLen=strlen(FTP_DATA);
    nCmpEndLen=strlen(FTP_END);

    /* Go through the input buffer looking for FTP_DATA or FTP_END within the
     * input buffer. If either exist then note position just after it
     * as this contains the size of the data block as an ascii number. If 
     * FTP_END is detected then set the nLast flag to indicate last block.
    */
    for(nPos=0; nPos < nDataLen; nPos++)
    {
        /* If a match occurs, then setup the pointer to correct location.
        */
        if(strncmp(&snzDataBuf[nPos], FTP_DATA, nCmpDataLen) == 0 ||
           strncmp(&snzDataBuf[nPos], FTP_END, nCmpEndLen) == 0)
        {
            /* Make sure that the match is not a sub-match as some of the
             * variables we are scanning for are similar.
            */
            if( (nPos == 0) || 
                (nPos > 0 && snzDataBuf[nPos-1] == '\0') ||
                (nPos > 0 && isspace(snzDataBuf[nPos-1])) )
            {
                if(strncmp(&snzDataBuf[nPos], FTP_DATA, nCmpDataLen) == 0)
                {
                    nPos += nCmpDataLen;
                } else
                 {
                    /* Setup the nLast flag as this is the last data block.
                    */    
                    *nLast = TRUE;
                    nPos += nCmpEndLen;
                }
                break;
            }
        }
    }

    /* If the pointer did not reach the end of the buffer then we have
     * located a valid data block.
    */
    if(nPos < nDataLen)
    {
        /* The following location upto a NULL byte should contain the
         * size of the data block in ASCII representation so use scanf
         * to extract the data accordingly.
        */
        if(sscanf(&snzDataBuf[nPos], "%d", &nBlockSize) != 1)
        {
            /* Create an error message to indicate the problem.
            */
            sprintf(szErrMsg, "%s: Invalid Block Size provided",
                    SDD_EMSG_BADBLOCKSZ);
            nReturn = SDD_FAIL;
            goto _FTPX_GetWriteData_EXIT;
        }

        /* Update our index variable.
        */
        nPos += (strlen(&snzDataBuf[nPos]) + 1);

        /* Quick test before we go any further, ensure that the data
         * provided is sufficient.
        */ 
        if((nPos+nBlockSize) != nDataLen)
        {
            /* Create an error message to indicate the problem.
            */
            sprintf(szErrMsg,
                    "%s: Data block size does not match data provided",
                    SDD_EMSG_BADBLOCKSZ);
            nReturn = SDD_FAIL;
            goto _FTPX_GetWriteData_EXIT;
        }

        /* Read data byte at a time and write to the opened file stream.
        */
        while(nPos < nDataLen)
        {
            if(fputc(snzDataBuf[nPos], fpFile) == EOF)
            {
                /* Create an error message to indicate the problem.
                */
                sprintf(szErrMsg,
                        "%s: Couldnt write to temporary file to store data",
                        SDD_EMSG_FILEERROR);
                nReturn = SDD_FAIL;
                break;
            }

            /* Time for pointer update... Could write the same data forever
             * I suppose....
            */
            nPos += 1;
        }
    }

    /* I suppose we are successful.... this time!
    */
    nReturn=SDD_OK;

_FTPX_GetWriteData_EXIT:

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_PutReadData
 * Description: Function to read an open stream and transmit the data
 *              contents to the caller via the callback mechanism. 
 * 
 * Returns:     SDD_FAIL- Couldnt obtain PATH.
 *              SDD_OK    - PATH obtained.
 ******************************************************************************/
int    _FTPX_PutReadData( FILE    *fpFile,       /* I: Stream to read from */
                          int     (*fSendDataCB)(UCHAR *, UINT),
                                                 /* I: CB to send data to */
                          UCHAR   *szErrMsg )    /* O: Error text */
{
    /* Local variables.
    */
    int          nChar;
    int          nEnd = FALSE;
    int          nNdx;
    int          nReturn;
    UCHAR        szRetBuf[MAX_RETURN_BUF];
    UCHAR        *szFunc = "_FTPX_PutReadData";

    /* Loop, extracting data from the input stream and sending it to the 
     * users callback in MAX_RETURN_BUF (ID + MAX_RETURN_BUF-1 bytes data)
     * byte blocks until we reach the end.
    */
    while(nEnd == FALSE)
    {
        /* Initialise any loop variables.
        */
        nNdx=1;

        /* Loop until the return buffer becomes full or we run out of data.
        */
        while(nNdx < (MAX_RETURN_BUF-1) && (nChar=fgetc(fpFile)) != EOF)
        {
            szRetBuf[nNdx] = (UCHAR)nChar;
            nNdx++;
        }

        /* Is this the end of the input stream?
        */
        if(nChar == EOF)
        {
            /* Set flag as we've got to end of the file, time to get out.
            */
            nEnd = TRUE;

            /* Set the first location in the input buffer to indicate last
             * block.
            */
            szRetBuf[0] = SDD_ENDBLOCK;
        } else
         {
            /* Set the first location in the input buffer to indicate that
             * this is one of many data blocks.
            */
            szRetBuf[0] = SDD_DATABLOCK;
        }

        /* Call function to transmit row to original caller.
        */
        if(fSendDataCB(szRetBuf, nNdx) == SDD_FAIL)
        {
            /* Create an error message to indicate the problem.
            */
            sprintf(szErrMsg,
                    "%s: Failed to send row back to client", SDD_EMSG_SENDROW);
            nReturn = SDD_FAIL;
            nEnd = TRUE;
        }
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_PIDataCB
 * Description: Function to handle any control information passed back from
 *              the FTP server.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void _FTPX_PIDataCB( UINT    nChanId,      /* I: Channel data arrived on */
                     UCHAR   *szData,      /* I: Actual data */
                     UINT    nDataLen )    /* I: Length of data */
{
    /* Local variables.
    */
    UINT      nA1, nA2, nA3, nA4;
    UINT      nNdx;
    UINT      nP1, nP2;
    UCHAR    *spNewBuf = NULL;
    UCHAR    *spTmp = NULL;
    UCHAR    *spTmp2 = NULL;
    UCHAR    *szFunc = "_FTPX_PIDataCB";

    /* If needed, create initial FTP buffer.
    */
    if(FTPX.snzPIDataBuf == NULL)
    {
        if((FTPX.snzPIDataBuf=(UCHAR *)malloc(DEF_FTP_BUF_SIZE)) == NULL)
        {
            Lgr(LOG_DEBUG, szFunc,
                "Couldnt malloc initial FTP buffer, wasting data");
            return;
        }

        /* Setup initial size.
        */
        FTPX.nPIDataBufLen = DEF_FTP_BUF_SIZE;
        FTPX.nPIDataBufPos = 0;
    }

    /* If the data will overfill our buffer, then grow it to accomodate.
    */
    if( (FTPX.nPIDataBufPos+nDataLen) > FTPX.nPIDataBufLen)
    {
        /* For safety's sake, an upper limit on the size of the
         * receive buffer has to be implemented. If this ceiling
         * is hit, then assume something is going wrong, so keep
         * the current buffer, but dump the contents.
        */
        if( FTPX.nPIDataBufLen >= MAX_FTP_BUF_SIZE )
        {
            Lgr(LOG_DEBUG, szFunc,
                "Exceeded maximum size of recv buffer, dumping");
            FTPX.nPIDataBufPos = 0;
            return;
        }

        /* OK, lets grow the buffer by a fixed size. If no memory
         * is available, then keep the current buffer, just stop
         * reading. Later functionality will prune the data.
        */
        if((spNewBuf=(UCHAR *)malloc(FTPX.nPIDataBufLen+DEF_FTP_BUF_INCSIZE))
                                                                        == NULL)
        {
            Lgr(LOG_DEBUG, szFunc, "Couldnt malloc (%d) bytes",
                DEF_FTP_BUF_INCSIZE);
            return;
        }

        /* Copy the old buffer onto the new, so we can release it
        * back to the sys pool.
        */
        for(spTmp=FTPX.snzPIDataBuf, spTmp2=spNewBuf, nNdx=FTPX.nPIDataBufLen;
            nNdx != 0; nNdx--, *spTmp2 = *spTmp, spTmp++, spTmp2++);

        /* Get rid of old buffer, and update pointers to accomodate the new
         * buffer and new size values.
        */
        free(FTPX.snzPIDataBuf);
        FTPX.snzPIDataBuf = spNewBuf;
        FTPX.nPIDataBufLen += DEF_FTP_BUF_INCSIZE;

        /* Log a message indicating that the buffer has grown in size.
        */
        Lgr(LOG_DEBUG, szFunc,
            "Allocated new receive buffer of %d bytes", FTPX.nPIDataBufLen);
    }

    /* Copy data from passed buffer into our buffer.
    */
    for(nNdx=FTPX.nPIDataBufPos; nNdx < (FTPX.nPIDataBufPos+nDataLen); nNdx++)
    {
        FTPX.snzPIDataBuf[nNdx] = szData[nNdx-FTPX.nPIDataBufPos];
    }
    FTPX.nPIDataBufPos += nNdx;

    /* Scan through the buffer looking for the start of a number as
     * this will represent the FTP Server response code.
    */
    for(nNdx=0; nNdx < FTPX.nPIDataBufPos; nNdx++)
    {
        if(isdigit(FTPX.snzPIDataBuf[nNdx]))
            break;
    }

    /* Digit detected? If not, fall through and wait for more data to
     * arrive.
    */
    if(nNdx < FTPX.nPIDataBufPos)
    {
        /* Get number.
        */
        sscanf(&FTPX.snzPIDataBuf[nNdx], "%d", &FTPX.nPIResponseCode);

        /* Special case handling if weve entered passive mode.
        */
        if(FTPX.nPIResponseCode == FTP_RESPONSE_PASSIVE)
        {
            /* Extract the IP address and port number from the response.
            */
            if(sscanf(&FTPX.snzPIDataBuf[nNdx+27], "%d,%d,%d,%d,%d,%d",
                      &nA1, &nA2, &nA3, &nA4, &nP1, &nP2) == 6)
            {
                /* Build up address and port number in useable form.
                */
                FTPX.lDTPIPaddr = ((ULNG)nA1*16777216L) + ((ULNG)nA2*65536L) +
                             ((ULNG)nA3*256L) + (ULNG)nA4;
                FTPX.nDTPPortNo = (nP1*256) + nP2;
            } else
             {
                FTPX.nPIResponseCode == FTP_RESPONSE_NONE;
            }
        }

        /* Clear buffer, data no longer needed.
        */
        FTPX.nPIDataBufPos = 0;
    }

    /* Return to caller.
    */
    return;
}

/******************************************************************************
 * Function:    _FTPX_PICtrlCB
 * Description: Function to handle any control callbacks during connectivity 
 *              with the FTP server.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void _FTPX_PICtrlCB( int        nType,        /* I: Type of callback */
                     ...                )     /* I: Var args */
{
    /* Local variables.
    */
    UINT        nChanId;
    UINT        nPortNo;
    ULNG        lIPaddr;
    va_list     pArgs;
    UCHAR       *szFunc = "_FTPX_PICtrlCB";

    /* Start var-arg list by making pArgs point to first arg in list.
    */
    va_start(pArgs, nType);

    /* What type of callback is it....?
    */
    switch(nType)
    {
        /* A new connection has arrived.
        */
        case SLC_NEWSERVICE:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log link going down event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "New Service with client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));
            break;

        /* A connection has been made
        */
        case SLC_CONNECT:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            Lgr(LOG_DEBUG, szFunc,
                "Connected to client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));

            SL_RawMode(nChanId, TRUE);
            break;

        /* Given connection has become temporarily unavailable.
        */
        case SLC_LINKDOWN:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log link going down event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Link down to client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));

            /* Get rid of connection, no longer needed.
            */
            SL_DelClient(FTPX.nPIChanId);
            FTPX.nPIChanId = 0;
            break;

        /* Given connection has died.
        */
        case SLC_LINKFAIL:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log link failure event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Link closed to client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));
            break;

        default:
            /* Log a message as this condition shouldnt occur.
            */
            Lgr(LOG_DEBUG, szFunc, "Unrecognised message type (%d)", nType);
            break;
    }

    /* Return to caller.
    */
    return;
}

/******************************************************************************
 * Function:    _FTPX_DTPDataCB
 * Description: Function to handle any data passed back from the FTP server on
 *              the data transfer connection..
 * 
 * Returns:     No returns.
 ******************************************************************************/
void _FTPX_DTPDataCB( UINT    nChanId,    /* I: Channel data arrived on */
                      UCHAR   *szData,    /* I: Actual data */
                      UINT    nDataLen )  /* I: Length of data */
{
    /* Local variables.
    */
    UINT     nNdx;
    UCHAR    *szFunc = "_FTPX_DTPDataCB";

    /* Simply store the data into our opened file buffer.
    */
    if(FTPX.fDataFile == NULL)
    {
        return;
    }

    /* Copy data byte at a time into the storage file.
    */
    for(nNdx=0; nNdx < nDataLen; nNdx++)
    {
        fputc(szData[nNdx], FTPX.fDataFile);
    }

    /* Return to caller.
    */
    return;
}

/******************************************************************************
 * Function:    _FTPX_DTPCtrlCB
 * Description: Function to handle any control callbacks on the Data Transfer
 *              connection with the FTP server.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void _FTPX_DTPCtrlCB( int        nType,       /* I: Type of callback */
                      ...                )    /* I: Var args */
{
    /* Local variables.
    */
    UINT        nChanId;
    UINT        nPortNo;
    ULNG        lIPaddr;
    va_list     pArgs;
    UCHAR       *szFunc = "_FTPX_DTPCtrlCB";

    /* Start var-arg list by making pArgs point to first arg in list.
    */
    va_start(pArgs, nType);

    /* What type of callback is it....?
    */
    switch(nType)
    {
        /* A new connection has arrived.
        */
        case SLC_NEWSERVICE:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log link going down event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "New Service with client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));

            /* Set channel into raw mode.
            */
            SL_RawMode(nChanId, TRUE);

            /* Store the channel id, because a server connection only
             * liberates the channel id at this point.
            */
            FTPX.nDTPChanId=nChanId;
            FTPX.nDTPConnected=TRUE;
            break;

        /* A connection has been made
        */
        case SLC_CONNECT:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            Lgr(LOG_DEBUG, szFunc,
                "Connected to client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));

            SL_RawMode(nChanId, TRUE);
            FTPX.nDTPConnected = TRUE;
            break;

        /* Given connection has become temporarily unavailable.
        */
        case SLC_LINKDOWN:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log link going down event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Link down to client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));

            /* Mark channel as closed.
            */
            FTPX.nDTPConnected = FALSE;

            /* On a link down, remove the connection entry as its no longer
             * needed.
            */
            SL_DelClient(FTPX.nDTPChanId);
            FTPX.nDTPChanId = 0;
            break;

        /* Given connection has died.
        */
        case SLC_LINKFAIL:
            nChanId = va_arg(pArgs, UINT);
            nPortNo = va_arg(pArgs, UINT);
            lIPaddr = va_arg(pArgs, ULNG);

            /* Log link failure event for bug catching.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Link closed to client: nChanId=%d, nPortNo=%d, IP=%s",
                nChanId, nPortNo, SL_HostIPtoString(lIPaddr));

            /* Mark channel as closed.
            */
            FTPX.nDTPConnected = FALSE;
            break;

        default:
            /* Log a message as this condition shouldnt occur.
            */
            Lgr(LOG_DEBUG, szFunc, "Unrecognised message type (%d)", nType);
            break;
    }

    /* Return to caller.
    */
    return;
}
 
/******************************************************************************
 * Function:    _FTPX_PIGetResponse
 * Description: Function to get a response code from the FTP server.
 * 
 * Returns:     Response Code.
 ******************************************************************************/
int _FTPX_PIGetResponse( void )
{
    /* Local variables.
    */
    UINT        nTotalTime;
    UCHAR       *szFunc = "_FTPX_PIGetResponse";

    /* Setup what type of data we are looking for from FTP server.
    */
    FTPX.nPIResponseCode = FTP_RESPONSE_NONE;

    /* OK, lets go into a loop state until we get a required response.
    */
    nTotalTime = 0;
    while(FTP_CONNECT_TIME > nTotalTime &&
          FTPX.nPIResponseCode==FTP_RESPONSE_NONE)
    {
        SL_Poll((ULNG)1);
        nTotalTime += 1;
    }

    /* Return to caller.
    */
    return(FTPX.nPIResponseCode);
}

/******************************************************************************
 * Function:    _FTPX_PISendCmd
 * Description: Function to send a command to an FTP server.
 * 
 * Returns:     SDD_FAIL - FTP Server failed to respond, see szErrMsg.
 *              SDD_OK   - Command sent successfully.
 ******************************************************************************/
int _FTPX_PISendCmd( UCHAR    *szCmd,             /* I: Command to send */
                     UINT    *panReqResponses,    /* I: Array of req resp */
                     UCHAR    *szErrMsg )         /* O: Error message */
{
    /* Local variables.
    */
    UINT        nNdx;
    UINT        nResponse;
    UINT        nReturn = SDD_FAIL;
    UCHAR       *szFunc = "_FTPX_PISendCmd";

    /* Send command to FTP server.
    */
    if(SL_BlockSendData(FTPX.nPIChanId, (UCHAR *)szCmd, strlen(szCmd)) != R_OK)
    {
        sprintf(szErrMsg, "Couldnt send command '%s' to FTP server", szCmd);
        return(SDD_FAIL);
    }

    /* If caller requires a certain set of response to the issued command, then
     * obtain a response and match it.
    */
    if(panReqResponses == NULL || panReqResponses[0] != FTP_RESPONSE_NONE)
    {
        /* Get the response from the executed command.
        */
        nResponse=_FTPX_PIGetResponse();

        /* See if the response exists in the array that the caller has
         * passed.
        */
        for(nNdx=0; nResponse != FTP_RESPONSE_NONE &&
                    panReqResponses[nNdx] != FTP_RESPONSE_NONE; nNdx++)
        {
            if(panReqResponses[nNdx] == nResponse)
                break;
        }

        /* If there was no response or rthe response did not match any of
         * the callers then indicate error.
        */
        if(nResponse == FTP_RESPONSE_NONE ||
           panReqResponses[nNdx] == FTP_RESPONSE_NONE)
        {
            sprintf(szErrMsg, "Illegal response '%d' obtained for command '%s'",
                    nResponse, szCmd);
        } else
         {
            nReturn = SDD_OK;
        }
    } else
     {
        nReturn = SDD_OK;
    }

    /* Return to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_PIGetDTPResponse
 * Description: Function to get a response code during a DTP transfer.
 * 
 * Returns:     Response Code.
 ******************************************************************************/
int _FTPX_PIGetDTPResponse( void )
{
    /* Local variables.
    */
    UINT        nTotalTime;
    UCHAR       *szFunc = "_FTPX_PIGetResponse";

    /* Setup what type of data we are looking for from FTP server.
    */
    FTPX.nPIResponseCode = FTP_RESPONSE_NONE;

    /* OK, lets go into a loop state until we get a required response.
    */
    nTotalTime = 0;
    while(    FTP_CONNECT_TIME > nTotalTime )
    {
        SL_Poll((ULNG) 1);
        nTotalTime += 1;

        if(FTPX.nPIResponseCode!=FTP_RESPONSE_NONE && FTPX.nDTPConnected==FALSE)
            break;
    }

    /* Return to caller.
    */
    return(FTPX.nPIResponseCode);
}


/******************************************************************************
 * Function:    _FTPX_PISendDTPCmd
 * Description: Function to send a command to an FTP server which will invoke
 *              a DTP channel for data transfer.
 * 
 * Returns:     SDD_FAIL - FTP Server failed to respond, see szErrMsg.
 *              SDD_OK   - Command sent successfully.
 ******************************************************************************/
int _FTPX_PISendDTPCmd( UCHAR   *szCmd,              /* I: Command to send */
                        UINT    *panReqResponses,    /* I: Allowed responses */
                        UCHAR   *szErrMsg )          /* O: Error message */
{
    /* Local variables.
    */
    UINT        anResponses[3];
    UINT        nPortNo;
    UINT        nReturn = SDD_OK;
    UINT        nTotalTime;
    ULNG        lIPAddr;
    UCHAR       szTmp[MAX_TMP_BUF_SIZE];
    UCHAR       *szFunc = "_FTPX_PISendDTPCmd";

    /* Setup allowed responses.
    */
    anResponses[0] = FTP_RESPONSE_PASSIVE;
    anResponses[1] = FTP_RESPONSE_NONE;

    /* Tel FTP server to enter into passive mode.
    */
    if(_FTPX_PISendCmd("PASV\r\n", anResponses, szErrMsg) == SDD_OK)
    {
        /* OK, FTP server has entered passive mode, so lets build a Data
         * Transfer link to it.
        */
        if((FTPX.nDTPChanId = SL_AddClient(FTPX.nDTPPortNo, FTPX.lDTPIPaddr,
                                           "DTP Channel", _FTPX_DTPDataCB,
                                           _FTPX_DTPCtrlCB)) < 0)
        {
            sprintf(szErrMsg, "Couldnt build DTP connection");
            return(SDD_FAIL);
        }

        /* Reset flag so that when connection is built we can detect it.
        */
        FTPX.nDTPConnected = FALSE;

        /* OK, lets go into a loop state until we get a required response.
        */
        nTotalTime = 0;
        while(FTP_CONNECT_TIME > nTotalTime && FTPX.nDTPConnected == FALSE)
        {
            SL_Poll((ULNG) 1);
            nTotalTime += 1;
        }

        /* Was the connection made?
        */
        if(FTPX.nDTPConnected == FALSE)
        {
            sprintf(szErrMsg, "Couldnt build DTP connection");
            return(SDD_FAIL);
        }
    } else
     {
        /* Look for a port we can hang on to.
        */
        for(nPortNo=DEF_SRV_START_PORT; nPortNo < DEF_SRV_END_PORT; nPortNo++)
        {
            /* Try and add a service on this port, if ok, then exit and run
             * with it.
            */
            if(SL_AddServer(nPortNo, FALSE, _FTPX_DTPDataCB,
                                                    _FTPX_DTPCtrlCB) == SDD_OK)
            break;
        }

        /* Have we reached the limit? If we have, give up, saturated server
         * or what!!
        */
        if(nPortNo == DEF_SRV_END_PORT)
        {
            sprintf(szErrMsg, "Couldnt add a service port for DTP channel");
            return(SDD_FAIL);
        }

        /* OK, lets tell the FTP server that we are listening for a 
         * connection. Firstly, work out who we are.
        */
        gethostname(szTmp, MAX_TMP_BUF_SIZE);
        if(SL_GetIPaddr(szTmp, &lIPAddr) != R_OK)
        {
            sprintf(szErrMsg, "Couldnt get our IP address");
            return(SDD_FAIL);
        }

        /* Convert the IP address and Port number into easy blocks for
         * inserting into our command.
        */
        PutCharFromLong(szTmp, lIPAddr);
        PutCharFromInt(&szTmp[4], nPortNo);

        /* Build up the command to instruct the FTP server.
        */
        sprintf(&szTmp[6], "PORT %d,%d,%d,%d,%d,%d\r\n",
                (UINT)szTmp[0], (UINT)szTmp[1], (UINT)szTmp[2],
                (UINT)szTmp[3], (UINT)szTmp[4], (UINT)szTmp[5]);

        /* Setup allowed responses.
        */
        anResponses[0] = FTP_RESPONSE_CWDOK;
        anResponses[1] = FTP_RESPONSE_CMDOK;
        anResponses[2] = FTP_RESPONSE_NONE;

        /* Fire the command at the server and stand back.....Ce'st la vie.
        */
        if(_FTPX_PISendCmd(&szTmp[6], anResponses, szErrMsg) == SDD_FAIL)
        {
            sprintf(szErrMsg, "Couldnt instruct FTP server to use port %d",
                    nPortNo);
            return(SDD_FAIL);
        }
    }

    /* Send command to FTP server
    */
    if(_FTPX_PISendCmd(szCmd, panReqResponses, szErrMsg) != SDD_OK)
    {
        sprintf(&szErrMsg[strlen(szErrMsg)],
                ": Failed to obtain valid response from FTP server");
        return(SDD_FAIL);
    }

    /* Return to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_SetMode
 * Description: Function to setup the transfer mode between the FTP server and
 *              the driver.
 * 
 * Returns:     SDD_FAIL - Failed to set transfer mode, critical error.
 *              SDD_OK     - Mode set.
 ******************************************************************************/
int    _FTPX_SetMode( UINT     nBinaryMode,    /* I: Select binary mode = TRUE */
                      UCHAR    *szErrMsg )     /* O: Generated error messages */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UINT     anResponses[3];
    UCHAR    *szFunc = "_FTPX_SetMode";

    /* Setup allowed responses.
    */
    anResponses[0] = FTP_RESPONSE_CMDOK;
    anResponses[1] = FTP_RESPONSE_CWDOK;
    anResponses[2] = FTP_RESPONSE_NONE;

    /* Enable binary mode or ascii?
    */
    if( nBinaryMode == TRUE)
    {
        nReturn=_FTPX_PISendCmd("TYPE I\r\n", anResponses, szErrMsg);
    } else
     {
        nReturn=_FTPX_PISendCmd("TYPE A\r\n", anResponses, szErrMsg);
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_SetCwd
 * Description: Function to set the FTP servers current working directory.
 * 
 * Returns:     SDD_FAIL - Failed to set directory to that specified.
 *              SDD_OK     - Current Working Directory set.
 ******************************************************************************/
int    _FTPX_SetCwd( UCHAR    *szPath,        /* I: Path to set CWD */
                     UCHAR    *szErrMsg )     /* O: Generated error messages */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UINT     anResponses[3];
    UCHAR    *pszCmd;
    UCHAR    *szFunc = "_FTPX_SetCwd";

    /* Allocate required memory for the command buffer.
    */
    if((pszCmd=(UCHAR *)malloc(strlen(szPath)+7)) == NULL)
    {
        sprintf(szErrMsg, "Couldnt allocate memory for CWD command");
        return(SDD_FAIL);
    }

    /* Setup command to change the remote directory to that required.
    */
    sprintf(pszCmd, "CWD %s\r\n", szPath);

    /* Setup allowed responses.
    */
    anResponses[0] = FTP_RESPONSE_CMDOK;
    anResponses[1] = FTP_RESPONSE_CWDOK;
    anResponses[2] = FTP_RESPONSE_NONE;

    /* Send command to FTP server.
    */
    nReturn=_FTPX_PISendCmd(pszCmd, anResponses, szErrMsg);

    /* Free up resources, no longer needed.
    */
    free(pszCmd);

    /* If a failure occurred, tag on our reason code as well.
    */
    if(nReturn == SDD_FAIL)
    {
        sprintf(&szErrMsg[strlen(szErrMsg)],
                ": Couldnt change CWD to '%' on remote", szPath);
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_FTPInit
 * Description: Function to initialise a connection with an FTP server. The
 *              caller provides the name/IP address of the server and the 
 *              user name/password to complete the connection.
 * 
 * Returns:     SDD_FAIL - Couldnt make connection with given details.
 *              SDD_OK     - FTP connection made.
 ******************************************************************************/
int    _FTPX_FTPInit( UCHAR    *szFTPServer,   /* I: Name of FTP server */
                      UCHAR    *szUserName,    /* I: User name to login with */
                      UCHAR    *szPassword,    /* I: Password for login */
                      UCHAR    *szErrMsg )     /* O: Error message if failed */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UINT     anResponses[2];
    UINT     nFTPPortNo;
    ULNG     lIPAddr;
    UCHAR    *pszCmd;
    UCHAR    *szFunc = "_FTPX_FTPInit";

    /* Setup any module variables to an initial state.
    */
    FTPX.nPIResponseCode = FTP_RESPONSE_NONE;
    FTPX.nPIChanId = 0;
    FTPX.nDTPChanId = 0;
    FTPX.nPIDataBufLen = 0;
    FTPX.nPIDataBufPos = 0;
    FTPX.snzPIDataBuf = NULL;
    FTPX.lDTPIPaddr = 0;
    FTPX.nDTPPortNo = 0;
    FTPX.nDTPConnected = FALSE;
    FTPX.fDataFile = NULL;
    
    /* Get the IP address of the host we are connecting with.
    */
    if(SL_GetIPaddr(szFTPServer, &lIPAddr) != R_OK)
    {
        sprintf(szErrMsg, "Server %s does not appear in hosts file",
                szFTPServer);
        return(SDD_FAIL);
    }

    /* Lookup port number for FTP service.
    */
    if(SL_GetService(DEF_FTP_SERVICE_NAME, &nFTPPortNo) != R_OK)
    {
        sprintf(szErrMsg, "FTP service '%s' does not appear in services file",
                DEF_FTP_SERVICE_NAME);
        return(SDD_FAIL);
    }

    /* OK, got IP address and port number, so lets build a connection to
     * it.
    */
    if((FTPX.nPIChanId = SL_AddClient(nFTPPortNo, lIPAddr, szFTPServer,
                                  _FTPX_PIDataCB, _FTPX_PICtrlCB)) < 0)
    {
        sprintf(szErrMsg, "Couldnt build IP connection to %s", szFTPServer);
        return(SDD_FAIL);
    }

    /* Have we connected?
    */
    if(_FTPX_PIGetResponse() != FTP_RESPONSE_CONNECTED)
    {
        sprintf(szErrMsg, "Couldnt build IP connection to %s", szFTPServer);
        return(SDD_FAIL);
    }

    /* Allocate required memory for the command buffer.
    */
    if((pszCmd=(UCHAR *)malloc(strlen(szUserName)+strlen(szPassword)+8))==NULL)
    {
        sprintf(szErrMsg, "Couldnt allocate memory during FTP initialisation");
        return(SDD_FAIL);
    }

    /* OK, send the FTP server the User name.
    */
    sprintf(pszCmd, "USER %s\r\n", szUserName);

    /* Setup allowed responses.
    */
    anResponses[0] = FTP_RESPONSE_USEROK;
    anResponses[1] = FTP_RESPONSE_NONE;

    /* Send command to FTP server.
    */
    if(_FTPX_PISendCmd(pszCmd, anResponses, szErrMsg) != SDD_OK)
    {
        sprintf(&szErrMsg[strlen(szErrMsg)],
                ": Couldnt login to FTP server with user '%s'", szUserName);
        free(pszCmd);
        return(SDD_FAIL);
    }

    /* OK, send the FTP server the Password for above user name.
    */
    sprintf(pszCmd, "PASS %s\r\n", szPassword);

    /* Setup allowed responses.
    */
    anResponses[0] = FTP_RESPONSE_PWDOK;
    anResponses[1] = FTP_RESPONSE_NONE;

    /* Send command to FTP server.
    */
    nReturn=_FTPX_PISendCmd(pszCmd, anResponses, szErrMsg);

    /* Free up memory buffer, no longer needed.
    */
    free(pszCmd);

    /* If a failure occurred on last command, tag on our reason code.
    */
    if(nReturn == SDD_FAIL)
    {
        sprintf(&szErrMsg[strlen(szErrMsg)],
                ": Couldnt login to FTP server with password '%s'", szPassword);
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_FTPClose
 * Description: Function to close a connected FTP connection and tidy up
 *              in preparation for next task.
 * 
 * Returns:     SDD_FAIL - Failed to close properly, library wont work again.
 *              SDD_OK     - Closed.
 ******************************************************************************/
int    _FTPX_FTPClose( UCHAR    *szErrMsg )        /* O: Generated error messages */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UINT     anResponses[2];
    UCHAR   *szFunc = "_FTPX_FTPClose";

    /* If the PI channel is open, close it by issuing the QUIT command to
     * the FTP server then deleting the connection.
    */
    if(FTPX.nPIChanId != 0)
    {
        /* Setup allowed responses.
        */
        anResponses[0] = FTP_RESPONSE_GOODBYE;
        anResponses[1] = FTP_RESPONSE_NONE;

        /* Send command to FTP server.
        */
        _FTPX_PISendCmd("QUIT\r\n", anResponses, szErrMsg);

        /* Delete the physical connection.
        */
        SL_DelClient(FTPX.nPIChanId);
        FTPX.nPIChanId = 0;
    }

    /* If the DTP channel is open (shouldnt be!!!), delete the connection.
    */
    if(FTPX.nDTPChanId != 0)
    {
        /* Delete the physical connection.
        */
        SL_DelClient(FTPX.nDTPChanId);
        FTPX.nDTPChanId = 0;
    }

    /* Free the PI data buffer if still allocated.
    */
    if(FTPX.snzPIDataBuf != NULL)
    {
        free(FTPX.snzPIDataBuf);
    }

    /* If the data file is still open, close it and reset.
    */
    if(FTPX.fDataFile != NULL)
    {
        fclose(FTPX.fDataFile);
        FTPX.fDataFile = NULL;
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_FTPRenFile
 * Description: Function to rename a file on a remote FTP server.
 * 
 * Returns:     SDD_FAIL - Failed to rename the required file.
 *              SDD_OK     - File renamed successfully.
 ******************************************************************************/
int    _FTPX_FTPRenFile( UCHAR    *szPath,       /* I: Path to remote file */
                         UCHAR    *szSrcFile,    /* I: Original remote file name */
                         UCHAR    *szDstFile,    /* I: New remote file name */
                         UCHAR    *szErrMsg )    /* O: Generated error messages */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UINT     anResponses[3];
    UCHAR    *pszCmd;
    UCHAR    *szFunc = "_FTPX_FTPRenFile";

    /* Change to correct directory on remote.
    */
    if(_FTPX_SetCwd(szPath, szErrMsg) == SDD_FAIL)
    {
        return(SDD_FAIL);
    }

    /* Allocate required memory for the command buffer.
    */
    if((pszCmd=(UCHAR *)malloc(strlen(szSrcFile)+strlen(szDstFile)+10))==NULL)
    {
        sprintf(szErrMsg, "Couldnt allocate memory for command buffer");
        return(SDD_FAIL);
    }

    /* Setup command to issue the FROM portion of the rename command.
    */
    sprintf(pszCmd, "RNFR %s\r\n", szSrcFile);

    /* Setup allowed responses.
    */
    anResponses[0] = FTP_RESPONSE_RENFROK;
    anResponses[1] = FTP_RESPONSE_NONE;

    /* Issue part 1 of the rename sequence.
    */
    if(_FTPX_PISendCmd(pszCmd, anResponses, szErrMsg) == SDD_FAIL)
    {
        sprintf(&szErrMsg[strlen(szErrMsg)],
                ": FTP server couldnt locate file '%s'", szSrcFile);
        free(pszCmd);
        return(SDD_FAIL);
    }

    /* Setup command to issue the TO portion of the rename command.
    */
    sprintf(pszCmd, "RNTO %s\r\n", szDstFile);

    /* Setup allowed responses.
    */
    anResponses[0] = FTP_RESPONSE_CWDOK;
    anResponses[1] = FTP_RESPONSE_CMDOK;
    anResponses[2] = FTP_RESPONSE_NONE;

    /* Send command to FTP server.
    */
    nReturn=_FTPX_PISendCmd(pszCmd, anResponses, szErrMsg);

    /* Free up command buffer, no longer needed.
    */
    free(pszCmd);

    /* If the above command failed, tag on our reason code and exit.
    */
    if(nReturn == SDD_FAIL)
    {
        sprintf(&szErrMsg[strlen(szErrMsg)],
                ": FTP server couldnt rename to file '%s'", szDstFile);
        return(SDD_FAIL);
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_FTPRcvFile
 * Description: Function to initiate a file transfer from the FTP server
 *              to the current machine file system.
 * 
 * Returns:     SDD_FAIL - Failed to complete file transfer.
 *              SDD_OK     - File received successfully.
 ******************************************************************************/
int    _FTPX_FTPRcvFile( UCHAR    *szRcvFile,    /* I: Name of file to store in */
                         UCHAR    *szPath,       /* I: Path to remote file */
                         UCHAR    *szFile,       /* I: Remote file */
                         UINT     nBinaryMode,   /* I: Select binary transfer mode */
                         UCHAR    *szErrMsg )    /* O: Generated error messages */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UINT     anResponses[2];
    UCHAR    *pszCmd;
    UCHAR    *szFunc = "_FTPX_FTPRcvFile";

    /* Setup transfer mode.
    */
    if(_FTPX_SetMode(nBinaryMode, szErrMsg) == SDD_FAIL)
    {
        return(SDD_FAIL);
    }

    /* Change to correct directory on remote.
    */
    if(_FTPX_SetCwd(szPath, szErrMsg) == SDD_FAIL)
    {
        return(SDD_FAIL);
    }

    /* Open file to store data into.
    */
    if((FTPX.fDataFile=fopen(szRcvFile, "w")) == NULL)
    {
        sprintf(szErrMsg, "Could not open '%s' for writing", szRcvFile);
        return(SDD_FAIL);
    }

    /* Allocate required memory for the command buffer.
    */
    if((pszCmd=(UCHAR *)malloc(strlen(szFile)+8))==NULL)
    {
        sprintf(szErrMsg, "Couldnt allocate memory for command buffer");
        return(SDD_FAIL);
    }

    /* Setup command to retrieve the required file.
    */
    sprintf(pszCmd, "RETR %s\r\n", szFile);

    /* Setup required responses.    
    */
    anResponses[0] = FTP_RESPONSE_DATASTART;
    anResponses[1] = FTP_RESPONSE_NONE;

    /* Send command to FTP server.
    */
    nReturn=_FTPX_PISendDTPCmd(pszCmd, anResponses, szErrMsg);

    /* Free up command buffer, no longer needed.
    */
    free(pszCmd);

    /* If the above command failed, tag on our reason code and exit.
    */
    if(nReturn == SDD_FAIL)
    {
        sprintf(&szErrMsg[strlen(szErrMsg)],
                ": Failed to receive data file '%s'", szFile);
        return(SDD_FAIL);
    }

    /* Have we connected?
    */
    if(_FTPX_PIGetDTPResponse() != FTP_RESPONSE_DATAEND)
    {
        sprintf(szErrMsg, "Failure to receive data file '%s'", szFile);
        return(SDD_FAIL);
    }

    /* OK, all done, so close the file and prepare to exit.
    */
    fclose(FTPX.fDataFile);
    FTPX.fDataFile=NULL;

    /* Ensure that the link between us and FTP server is severed.
    */
    SL_DelClient(FTPX.nDTPChanId);

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _FTPX_FTPXmitFile
 * Description: Function to initiate a file transfer from the current machine
 *              file system to the FTP server.
 * 
 * Returns:     SDD_FAIL - Failed to complete file transfer.
 *              SDD_OK     - File transmitted successfully.
 ******************************************************************************/
int    _FTPX_FTPXmitFile( UCHAR   *szXmitFile,   /* I: Name of file to transmit */
                         UCHAR    *szPath,       /* I: Path to remote destination */
                         UCHAR    *szFile,       /* I: Remote file */
                         UINT     nBinaryMode,   /* I: Select binary transfer Mode */
                         UCHAR    *szErrMsg )    /* O: Generated error messages */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UINT     nChar;
    UINT     nNdx;
    UINT     anResponses[2];
    UINT     nSending;
    UCHAR    *pszCmd;
    UCHAR    *pszDataBuf;
    UCHAR    *szFunc = "_FTPX_FTPXmitFile";

    /* Setup transfer mode.
    */
    if(_FTPX_SetMode(nBinaryMode, szErrMsg) == SDD_FAIL)
    {
        return(SDD_FAIL);
    }

    /* Change to correct directory on remote.
    */
    if(_FTPX_SetCwd(szPath, szErrMsg) == SDD_FAIL)
    {
        return(SDD_FAIL);
    }

    /* Open the file for transmission.
    */
    if((FTPX.fDataFile=fopen(szXmitFile, "r")) == NULL)
    {
        sprintf(szErrMsg, "Could not open '%s' for reading", szXmitFile);
        return(SDD_FAIL);
    }

    /* Allocate required memory for the command buffer.
    */
    if((pszCmd=(UCHAR *)malloc(strlen(szFile)+8))==NULL)
    {
        sprintf(szErrMsg, "Couldnt allocate memory for command buffer");
        return(SDD_FAIL);
    }

    /* Setup command to store the required file.
    */
    sprintf(pszCmd, "STOR %s\r\n", szFile);

    /* Setup required responses.
    */
    anResponses[0] = FTP_RESPONSE_DATASTART;
    anResponses[1] = FTP_RESPONSE_NONE;

    /* Send command to FTP server.
    */
    nReturn=_FTPX_PISendDTPCmd(pszCmd, anResponses, szErrMsg);

    /* Free up command buffer, no longer needed.
    */
    free(pszCmd);

    /* If last command failed, tag on our reason code and exit.
    */
    if(nReturn == SDD_FAIL)
    {
        sprintf(&szErrMsg[strlen(szErrMsg)],
                ": Failed to transmit data file '%s'", szFile);
        return(SDD_FAIL);
    }

    /* Allocate required memory for the transmit data buffer.
    */
    if((pszDataBuf=(UCHAR *)malloc(DEF_FTP_XMIT_SIZE+1))==NULL)
    {
        sprintf(szErrMsg, "Couldnt allocate memory for xmit data buffer");
        return(SDD_FAIL);
    }

    /* Transmit the contents of the file by going in a tight loop and sending
     * packets of a predetermined size to the FTP server.
    */
    for(nSending=TRUE, nNdx=0; nSending == TRUE; nNdx=0)
    {
        /* Fill our buffer with data from the file, cutting short if the
         * file end is reached.
        */
        while(nNdx < DEF_FTP_XMIT_SIZE && (nChar=fgetc(FTPX.fDataFile)) != EOF)
        {
            pszDataBuf[nNdx] = (UCHAR)nChar;
            nNdx++;
        }

        /* If we have reached the end of the file then set the flag such
         * that we fall out of this loop.
        */
        if(nChar == EOF)
        {
            nSending=FALSE;
        }

        /* If there is any data in the buffer, then send it, regardless of
         * wether we are at the end of the file.
        */
        if(nNdx > 0)
        {
            if(SL_BlockSendData(FTPX.nDTPChanId,(UCHAR *)pszDataBuf,nNdx)!=R_OK)
            {
                sprintf(szErrMsg, "Couldnt send data to FTP server");
                free(pszDataBuf);
                return(SDD_FAIL);
            }
        }
    }

    /* Free up memory, no longer needed.
    */
    free(pszDataBuf);

    /* Close the DTP channel, which in turn indicates to the FTP server that
     * data transmission is complete.
    */    
    SL_DelClient(FTPX.nDTPChanId);
    FTPX.nDTPChanId = 0;
    FTPX.nDTPConnected=FALSE;

    /* Wait for the end of data transmission response from the server. If it
     * doesnt arrive or another response arrives in its place then something
     * went wrong.
    */
    if(_FTPX_PIGetDTPResponse() != FTP_RESPONSE_DATAEND)
    {
        sprintf(szErrMsg, "Failed to send data file '%s'", szFile);
        return(SDD_FAIL);
    }

    /* OK, all done, so close the file and prepare to exit.
    */
    fclose(FTPX.fDataFile);
    FTPX.fDataFile=NULL;

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    ftpx_InitService
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
int    ftpx_InitService( SERVICEDETAILS   *sServiceDet,      /* I: Init data */
                         UCHAR            *szErrStr )        /* O: Error message */
{
    /* Local variables.
    */
    int       nReturn = SDD_OK;
    UCHAR    *szFunc = "ftpx_InitService";

    /* Prepare error string.
    */
    sprintf(szErrStr, "%s: ", SDD_EMSG_SRCINIT);

    /* Initialise a connection with the FTP server using the data provided
     * in the service details record.
    */
    if(_FTPX_FTPInit(sServiceDet->uServiceInfo.sFTPInfo.szServer,
                     sServiceDet->uServiceInfo.sFTPInfo.szUser,
                     sServiceDet->uServiceInfo.sFTPInfo.szPassword,
                     &szErrStr[strlen(szErrStr)]) == SDD_FAIL)
    {
        return(SDD_FAIL);
    }

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "FTPX Driver: Initialised:");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    ftpx_CloseService
 * Description: Entry point which performs a drive closedown. The closedown
 *              procedure ensure that the driver returns to a virgin state
 *              (ie.like at power up) so that InitService can be called again.
 * 
 * Returns:     SDD_FAIL- An error occurred in closing the driver and an
 *                        error message is stored in szErrStr.
 *              SDD_OK    - Driver successfully closed.
 ******************************************************************************/
int    ftpx_CloseService( UCHAR        *szErrMsg )    /* O: Error message if failed */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "ftpx_CloseService";

    /* Prepare error string.
    */
    sprintf(szErrMsg, "%s: ", SDD_EMSG_BADEXIT);

    /* Close connection with FTP server and tidy up.
    */
    if(_FTPX_FTPClose( &szErrMsg[strlen(szErrMsg)] ) == SDD_FAIL)
    {
        return(SDD_FAIL);
    }

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "FTPX Driver: Closed.");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    ftpx_ProcessRequest
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
int    ftpx_ProcessRequest( UCHAR    *snzDataBuf,           /* I: Input data */
                            int      nDataLen,              /* I: Len of data */
                            int      (*fSendDataCB)(UCHAR *, UINT),
                                                            /* I: CB to send reply*/
                            UCHAR    *szErrMsg )            /* O: Error text */
{
    /* Static variables.
    */
    static int        nXmitMode = FTP_BINARY_MODE;
    static UCHAR      *pszRemoteFile = NULL;
    static UCHAR      *pszRemotePath = NULL;
    static UCHAR      szXmitFile[MAX_FTP_FILENAME] = "";
    static FILE       *fXmitFile = NULL;

    /* Local variables.
    */
    int        nLastBlock;
    int        nRcvMode;
    int        nReturn = SDD_OK;
    UCHAR    *szSrcFile;
    UCHAR    *szDstFile;
    UCHAR    *szPath;
    UCHAR    szRcvFile[MAX_FTP_FILENAME] = "";
    UCHAR    *szFunc = "ftpx_ProcessRequest";
    FILE    *fpRcvFile;

    /* If the request block doesnt contain any data, something went wrong
     * somewhere?
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
        case SDD_FTPX_XMIT:
            /* If this is the first time, then create a temporary file and
             * process any options.
            */
            if(fXmitFile == NULL)
            {
                /* Create a filename using a constant with the current pid.
                */
                sprintf(szXmitFile, "/tmp/XmitFile.%d", getpid());

                /* Try to open the file.
                */
                if((fXmitFile=fopen(szXmitFile, "w")) == NULL)
                {
                    /* Create a message to indicate failure.
                    */
                    sprintf(szErrMsg,
                            "%s: Failed to create a temporary storage file",
                            SDD_EMSG_FILEERROR);
        
                    /* Exit directly as we have nothing open to tidy up.
                    */
                    return(SDD_FAIL);
                }

                /* Get the path of the file to on the remote.
                */
                if(_FTPX_GetStrArg(&snzDataBuf[1], (nDataLen-1), FTP_SRCPATH,
                                                        &szPath) == SDD_FAIL)
                {
                    /* Create a message to indicate failure.
                    */
                    sprintf(szErrMsg,
                            "%s: Illegal or no Path given in command",
                            SDD_EMSG_BADPATH);

                    /* Close file, no longer needed.
                    */
                    fclose(fXmitFile);
                    fXmitFile = NULL;
        
                    /* Exit directly as we have nothing open to tidy up.
                    */
                    return(SDD_FAIL);
                }

                /* Get the name of the file to be created on the remote.
                */
                if(_FTPX_GetStrArg(&snzDataBuf[1], (nDataLen-1), FTP_SRCFILE,
                                                     &szSrcFile) == SDD_FAIL)
                {
                    /* Create a message to indicate failure.
                    */
                    sprintf(szErrMsg,
                            "%s: Illegal or no Filename given in command",
                            SDD_EMSG_BADFILE);

                    /* Close file, no longer needed.
                    */
                    fclose(fXmitFile);
                    fXmitFile = NULL;
        
                    /* Exit directly as we have nothing open to tidy up.
                    */
                    return(SDD_FAIL);
                }

                /* Allocate memory to store the Path and File until they
                 * are needed.
                */
                if((pszRemotePath=(UCHAR *)malloc(strlen(szPath)+1)) == NULL ||
                   (pszRemoteFile=(UCHAR *)malloc(strlen(szSrcFile)+1)) == NULL)
                {
                    /* Create a message to indicate failure.
                    */
                    sprintf(szErrMsg,
                            "%s: No more memory whilst allocating storage",
                            SDD_EMSG_MEMORY);

                    /* Close file, no longer needed.
                    */
                    fclose(fXmitFile);
                    fXmitFile = NULL;
        
                    /* Exit directly as we have nothing open to tidy up.
                    */
                    return(SDD_FAIL);
                }

                /* Copy temporary contents into the permanent memory we've
                 * just allocated.
                */
                strcpy(pszRemotePath, szPath);
                strcpy(pszRemoteFile, szSrcFile);

                /* Get the mode of transfer, if specified, else default to
                 * binary.
                */
                nXmitMode = _FTPX_GetMode(&snzDataBuf[1], (nDataLen-1));
            }

            /* Extract the data which has been passed in the block and store
             * in the temporary file opened earlier.
            */
            if(_FTPX_GetWriteData(&snzDataBuf[1], (nDataLen-1), fXmitFile,
                                  &nLastBlock, szErrMsg) == SDD_FAIL)
            {
                /* Tidy up as we are not coming back!!!
                */
                fclose(fXmitFile);
                fXmitFile = NULL;
                free(pszRemotePath);
                free(pszRemoteFile);
                pszRemotePath = NULL;
                pszRemoteFile = NULL;

                /* Delete temporary file as we dont want thousands of these
                 * populating the temporary directory.
                */
                unlink(szXmitFile);

                /* Exit directly as we have nothing left to tidy up.
                */
                return(SDD_FAIL);
            }

            /* If we wrote the last block successfully, then prepare for FTP
             * takeoff.
            */
            if(nLastBlock == TRUE)
            {
                /* Firstly, close the opened file and reset the pointer for
                 * next time.
                */
                fclose(fXmitFile);
                fXmitFile = NULL;

                /* Fire the data into hyper space... we assume the FTP link
                 * is still open, it will fail otherwise.
                */
                nReturn=_FTPX_FTPXmitFile(szXmitFile, pszRemotePath,
                                          pszRemoteFile, nXmitMode, szErrMsg);

                /* Free the permanent path and file store, no longer needed.
                */
                free(pszRemotePath);
                free(pszRemoteFile);
                pszRemotePath = NULL;
                pszRemoteFile = NULL;

                /* Delete temporary file as we dont want thousands of these
                 * populating the temporary directory.
                */
                unlink(szXmitFile);
            }
            break;

        case SDD_FTPX_RCV:
            /* Get the path of the file to be received.
            */
            if(_FTPX_GetStrArg(&snzDataBuf[1], (nDataLen-1), FTP_SRCPATH,
                                                        &szPath) == SDD_FAIL)
            {
                /* Create a message to indicate failure.
                */
                sprintf(szErrMsg,
                        "%s: Illegal or no Path given in command",
                        SDD_EMSG_BADPATH);
    
                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }

            /* Get the name of the file to be received.
            */
            if(_FTPX_GetStrArg(&snzDataBuf[1], (nDataLen-1), FTP_SRCFILE,
                                                        &szSrcFile) == SDD_FAIL)
            {
                /* Create a message to indicate failure.
                */
                sprintf(szErrMsg,
                        "%s: Illegal or no Filename given in command",
                        SDD_EMSG_BADFILE);
    
                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }

            /* Get the mode of transfer, if specified, else default to
             * binary.
            */
            nRcvMode = _FTPX_GetMode(&snzDataBuf[1], (nDataLen-1));

            /* Create the name of the temporary file for holding FTP'd data.
            */
            sprintf(szRcvFile, "/tmp/RcvFile.%d", getpid());

            /* Get file from remote site.. lets hope it hasnt closed down,
             * maybe the client is the ?A.dasd?.
            */
            if(_FTPX_FTPRcvFile(szRcvFile, szPath, szSrcFile, nRcvMode,
                                                        szErrMsg) == SDD_FAIL)
            {
                /* Delete temporary file as we dont want thousands of these
                 * populating the temporary directory.
                */
                unlink(szRcvFile);

                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }

            /* Open the file just populated from the FTP receive ready for
             * transmission back to the host.
            */
            if((fpRcvFile=fopen(szRcvFile, "r")) == NULL)
            {
                /* Create a message to indicate failure.
                */
                sprintf(szErrMsg,
                        "%s: Illegal or no Filename given in command",
                        SDD_EMSG_BADFILE);

                /* Delete temporary file as we dont want thousands of these
                 * populating the temporary directory.
                */
                unlink(szRcvFile);
    
                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }

            /* Transmit data back to caller...... 
            */
            nReturn=_FTPX_PutReadData(fpRcvFile, fSendDataCB, szErrMsg);

            /* Close file and remove it...
            */
            fclose(fpRcvFile);
            unlink(szRcvFile);
            break;

        case SDD_FTPX_REN:
            /* Get the path where the file to be renamed exists.
            */
            if(_FTPX_GetStrArg(&snzDataBuf[1], (nDataLen-1), FTP_SRCPATH,
                                                        &szPath) == SDD_FAIL)
            {
                /* Create a message to indicate failure.
                */
                sprintf(szErrMsg, "%s: Illegal or no Path given in command",
                        SDD_EMSG_BADPATH);
    
                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }

            /* Get the source file name.
            */
            if(_FTPX_GetStrArg(&snzDataBuf[1], (nDataLen-1), FTP_SRCFILE,
                                                        &szSrcFile) == SDD_FAIL)
            {
                /* Create a message to indicate failure.
                */
                sprintf(szErrMsg,
                        "%s: Illegal or no source file given in command",
                        SDD_EMSG_BADPATH);
    
                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }

            /* Get the destination file name.
            */
            if(_FTPX_GetStrArg(&snzDataBuf[1], (nDataLen-1), FTP_DSTFILE,
                                                        &szDstFile) == SDD_FAIL)
            {
                /* Create a message to indicate failure.
                */
                sprintf(szErrMsg,
                        "%s: Illegal or no source file given in command",
                        SDD_EMSG_BADPATH);
    
                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }

            /* Call the FTP function to perform the rename.
            */
            if(_FTPX_FTPRenFile(szPath,szSrcFile,szDstFile,szErrMsg)==SDD_FAIL)
            {
                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }
            break;

        default:
            sprintf(szErrMsg,
                    "%s: Illegal command in request buffer (%x)",
                    SDD_EMSG_BADREQ, snzDataBuf[0]);
            return(SDD_FAIL);
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    ftpx_ProcessOOB
 * Description: Entry point into driver to process an out of band command
 *              that may or may not be relevant to current state of
 *              operation. The task of this function is to decipher the
 *              command and act on it immediately, ie. a cancel command
 *              would abort any ProcessRequest that is in process and
 *              clean up.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void    ftpx_ProcessOOB( UCHAR    nCommand    )    /* I: OOB Command */
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
