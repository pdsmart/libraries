/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_scmd.c
 * Description:   Server Data-source Driver library driver to handle system
 *                command execution on the local host.
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
#define        SDD_SCMD_C

/* Bring in local specific header files.
*/
#include    "sdd.h"
#include    "sdd_scmd.h"

/******************************************************************************
 * Function:    _SCMD_GetStrArg
 * Description: Function to scan an input buffer and extract a string based
 *              argument.
 * 
 * Returns:     SDD_FAIL- Couldnt obtain argument.
 *              SDD_OK    - Argument obtained.
 ******************************************************************************/
int    _SCMD_GetStrArg( UCHAR    *snzDataBuf,    /* I: Input buffer */
                        int      nDataLen,       /* I: Len of data */
                        UCHAR    *szArg,         /* I: Arg to look for */
                        UCHAR    **pszPath )     /* O: Pointer to argument */
{
    /* Local variables.
    */
    int          nCmpLen;
    int          nPos;
    int          nReturn = SDD_FAIL;
    UCHAR        *szFunc = "_SCMD_GetStrArg";

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
 * Function:    _SCMD_ValidatePath
 * Description: Function to validate the existence of a path.
 * 
 * Returns:     SDD_FAIL- Couldnt validate PATH.
 *              SDD_OK    - PATH validated.
 ******************************************************************************/
int    _SCMD_ValidatePath( UCHAR    *pszPath    )    /* I: Path to validate */
{
    /* Local variables.
    */
    int            nReturn = SDD_FAIL;
    UCHAR          *szFunc = "_SCMD_ValidatePath";
    struct stat    sStat;

    /* Test the path to ensure its valid.
    */
    if( (stat(pszPath, &sStat) == -1) ||
        ((sStat.st_mode & S_IFDIR) == 0) )
    {
        nReturn=SDD_FAIL;
    } else
     {
        nReturn=SDD_OK;
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SCMD_ValidateFile
 * Description: Function to validate the existence of a file or to validate
 *              that a file can be created.
 * 
 * Returns:     SDD_FAIL- Couldnt obtain Filename or validate it.
 *              SDD_OK    - Filename obtained and validated.
 ******************************************************************************/
int    _SCMD_ValidateFile( UCHAR   *pszPath,          /* I: Path to file */
                           UCHAR   *pszFile,          /* I: File to validate */
                           UINT    nWriteFlag    )    /* I: Read = 0, Write = 1 */
{
    /* Local variables.
    */
    int          nReturn = SDD_FAIL;
    UCHAR        szTmpBuf[MAX_TMPBUFLEN];
    UCHAR        *szFunc = "_SCMD_ValidateFile";
    struct stat  sStat;
    FILE         *fpTest;

    /* Concatenate the file and path together, ready to perform a test.
    */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
    sprintf(szTmpBuf, "%s//%s", pszPath, pszFile);
#endif
#if defined(_WIN32)
    sprintf(szTmpBuf, "%s%c%s", pszPath, 0x5c, pszFile);
#endif

    /* Is the file to be created?
    */
    if(nWriteFlag == TRUE)
    {
        /* Test the file by trying to create it, see if the underlying OS
         * can perform this operation.
        */
        if((fpTest=fopen(szTmpBuf, "w")) != NULL)
        {
            /* Close and remove the file we created, not needed yet.
            */
            fclose(fpTest);
            unlink(szTmpBuf);
            nReturn = SDD_OK;
        } else
         {
            nReturn = SDD_FAIL;
        }
    } else
     {
        /* Test the file to ensure that it exists and is readable.
        */
        if( (stat(szTmpBuf, &sStat) == -1) ||
            ((sStat.st_mode & S_IFREG) == 0) )
        {
            nReturn=SDD_FAIL;
        } else
         {
            nReturn=SDD_OK;
        }
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SCMD_ValidateTime
 * Description: Function to validate a time value given as an ascii string.
 * 
 * Returns:     SDD_FAIL- Couldnt obtain a TIME or validate it.
 *              SDD_OK    - TIME obtained and validated.
 ******************************************************************************/
int    _SCMD_ValidateTime( UCHAR   *pszTime,        /* I: Time to verify */
                           ULNG    *lTime    )      /* O: Time in seconds */
{
    /* Local variables.
    */
    int          nReturn = SDD_FAIL;
    UCHAR        *szFunc = "_SCMD_ValidateTime";

    /* Convert the time value into a long value which represents the
     * number of seconds since the beginning of the machine date start
     * point (ie. 0:00 Jan 1970 for PCDOS).
    */
    nReturn=SDD_OK;

    /* Return result code to caller.
    */
    return(SDD_OK);
}

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
/******************************************************************************
 * Function:    _SCMD_Exec
 * Description: Function to execute a given command via a fork and exec,
 *              attaching the parent to the childs I/O so that any data
 *              output by the child can be captured by the parent and fed
 *              back to the caller.
 * 
 * Returns:     SDD_FAIL- Command failed during execution.
 *              SDD_OK    - Command executed successfully.
 ******************************************************************************/
int    _SCMD_Exec( int      nTimedExec,        /* I: Is this a timed exec (T/F)? */
                   UCHAR    *pszPath,          /* I: Path to command */
                   UCHAR    *pszCmd,           /* I: Command name */
                   UCHAR    *pszArgs,          /* I: Arguments to command */
                   ULNG     lTimeToExec,       /* I: Time to execution */
                   int      (*fSendDataCB)(UCHAR *, UINT),
                                               /* I: Func for returning data */
                   UCHAR    *szErrMsg )        /* O: Error message generated */
{
    /* Local variables.
    */
    int          nChar;
    int          nEnd = FALSE;
    int          nNdx;
    int          nResult;
    int          nRetBufSize;
    int          nReturn = SDD_OK;
    UCHAR        *pszTmpBuf;
    UCHAR        *pszRetBuf;
    UCHAR        *szFunc = "_SCMD_Exec";
    FILE         *fpStdIn;

    /* Work out return buffer memory based on mode and return buffer size.
    */
    if(SCMD.nRetMode == SCMD_BINMODE)
    {
        nRetBufSize = SCMD.nRetBufSize+1;
    } else
     {
        nRetBufSize = MAX_RETURN_BUF_SIZE+1;
    }

    /* Allocate memory for return buffer.
    */
    if((pszRetBuf=(UCHAR *)malloc(nRetBufSize)) == NULL)
    {
        return(SDD_FAIL);
    }

    /* Is this a timed execution? 
    */
    if(nTimedExec == TRUE)
    {
        sleep(1);
    }

    /* Concatenate the file, command and arguments to build up a string
     * for the system command to use.
    */
    nNdx=strlen(pszPath)+strlen(pszCmd)+strlen(pszArgs)+10;
    if((pszTmpBuf=(UCHAR *)malloc(nNdx)) == NULL)
    {
        return(SDD_FAIL);
    }
    sprintf(pszTmpBuf, "%s//%s %s", pszPath, pszCmd, pszArgs);

    /* Execute the program as a child with its stdout connected to a read
     * stream such that we can capture its output.
    */
    if((fpStdIn=popen(pszTmpBuf, "r")) != NULL)
    {
        while(nEnd == FALSE)
        {
            /* Initialise loop variables.
            */
            nNdx=1;

            /* Loop until our return buffer becomes full or we run out of data
             * to read.
            */
            while(nNdx < nRetBufSize && (nChar=fgetc(fpStdIn)) != EOF)
            {
                pszRetBuf[nNdx] = (UCHAR)nChar;
                nNdx++;

                /* If we are in ascii mode and we detect an end of line then
                 * exit from the loop.
                */
                if(SCMD.nRetMode == SCMD_ASCIIMODE && pszRetBuf[nNdx-1] == '\n')
                    break;
            }

            /* Has the child completed, i.e. no more data?
            */
            if(nChar == EOF)
            {
                /* Set flag to indicate that we have reached end of input.
                */
                nEnd = TRUE;
            }

            /* If data exists in the buffer for xmit, then transmit it.
            */
            if(nNdx > 1)
            {
                /* Set the first location in the return buffer to indicate
                 * that this is just one of many data blocks in consecutive
                 * order.
                */
                pszRetBuf[0] = SDD_DATABLOCK;

                /* Call function to transmit row to original caller.
                */
                if(fSendDataCB(pszRetBuf, nNdx) == SDD_FAIL)
                {
                    /* Create an error message to indicate the problem.
                    */
                    sprintf(szErrMsg,
                            "%s: Failed to send row back to client",
                            SDD_EMSG_SENDROW);
                    nReturn = SDD_FAIL;
                    nEnd = TRUE;
                }
            }
        }

        /* Close the process and get its exit code.
        */
        nResult=pclose(fpStdIn);

        /* If we completed without error, then build a final packet
         * to indicate the exit code of the process.
        */
        if(nEnd == TRUE && nReturn != SDD_FAIL)
        {
            /* Create the packet for transmit based on the end of data
             * indicator and an ascii representation of the exit code.
            */
            pszRetBuf[0] = SDD_ENDBLOCK;
            sprintf(&pszRetBuf[1], "%d", nResult);

            /* Send the packet and see what happens.
            */
            if(fSendDataCB(pszRetBuf, (strlen(&pszRetBuf[1])+1)) == SDD_FAIL)
            {
                /* Create an error message to indicate the problem.
                */
                sprintf(szErrMsg,
                        "%s: Failed to send row back to client",
                        SDD_EMSG_SENDROW);
                nReturn = SDD_FAIL;
            }
        }
    } else
     {
        /* Build up an error message to indicate that the command was
         * invalid.
        */
        sprintf(szErrMsg, "%s: Command execution failed", SDD_EMSG_BADCMD);

        /* Set exit code to indicate failure.
        */
        nReturn = SDD_FAIL;
    }

    /* Free up allocated memory block.
    */
    free(pszTmpBuf);

    /* Return result code to caller.
    */
    return(nReturn);
}
#endif

/******************************************************************************
 * Function:    _SCMD_GetWriteData
 * Description: Function to scan an input buffer, verify that it has data in it,
 *              extract the data and store in the opened file stream and
 *              set the start flag if the block is the final block.
 * 
 * Returns:     SDD_FAIL- Bad block of data or error writing to file.
 *              SDD_OK    - Block obtained and stored.
 ******************************************************************************/
int    _SCMD_GetWriteData( UCHAR   *snzDataBuf,    /* I: Input buffer */
                           int     nDataLen,       /* I: Len of data */
                           FILE    *fpFile,        /* IO: Opened file stream */
                           int     *nLast,         /* O: Last block flag */
                           UCHAR   *szErrMsg )     /* O: Any resultant error msg */
{
    /* Local variables.
    */
    int          nBlockSize;
    int          nCmpDataLen;
    int          nCmpEndLen;
    int          nPos;
    int          nReturn = SDD_FAIL;
    UCHAR        *szFunc = "_SCMD_GetWriteData";

    /* Initialise any required variables.
    */
    *nLast = FALSE;
    nCmpDataLen=strlen(SCMD_DATA);
    nCmpEndLen=strlen(SCMD_END);

    /* Go through the input buffer looking for SCMD_DATA or SCMD_END within the
     * input buffer. If either exist then note position just after it
     * as this contains the size of the data block as an ascii number. If 
     * SCMD_END is detected then set the nLast flag to indicate last block.
    */
    for(nPos=0; nPos < nDataLen; nPos++)
    {
        /* If a match occurs, then setup the pointer to correct location.
        */
        if(strncmp(&snzDataBuf[nPos], SCMD_DATA, nCmpDataLen) == 0 ||
           strncmp(&snzDataBuf[nPos], SCMD_END, nCmpEndLen) == 0)
        {
            /* Make sure that the match is not a sub-match as some of the
             * variables we are scanning for are similar.
            */
            if( (nPos == 0) || 
                (nPos > 0 && snzDataBuf[nPos-1] == '\0') ||
                (nPos > 0 && isspace(snzDataBuf[nPos-1])) )
            {
                if(strncmp(&snzDataBuf[nPos], SCMD_DATA, nCmpDataLen) == 0)
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
            goto _SCMD_GetWriteData_EXIT;
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
            goto _SCMD_GetWriteData_EXIT;
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

_SCMD_GetWriteData_EXIT:

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SCMD_PutReadData
 * Description: Function to read an open stream and transmit the data
 *              contents to the caller via the callback mechanism. 
 * 
 * Returns:     SDD_FAIL- Couldnt obtain PATH.
 *              SDD_OK    - PATH obtained.
 ******************************************************************************/
int    _SCMD_PutReadData( FILE    *fpFile,       /* I: Stream to read from */
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
    UCHAR        szRetBuf[SCMD.nRetBufSize+1];
    UCHAR        *szFunc = "_SCMD_PutReadData";

    /* Loop, extracting data from the input stream and sending it to the 
     * users callback in packets of size indicated by SCMD.nRetBufSize.
    */
    while(nEnd == FALSE)
    {
        /* Initialise any loop variables.
        */
        nNdx=1;

        /* Loop until the return buffer becomes full or we run out of data.
        */
        while(nNdx < SCMD.nRetBufSize && (nChar=fgetc(fpFile)) != EOF)
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
 * Function:    _SCMD_MoveFile
 * Description: Function to move a file from one location/name to another.
 *              This is performed as a copy and unlink operation because the
 *              underlying may not support moves across file systems.
 * 
 * Returns:     SDD_FAIL- An error whilst moving file, see szErrMsg.
 *              SDD_OK    - File moved successfully.
 ******************************************************************************/
int    _SCMD_MoveFile( UCHAR            *pszSrcPath,    /* I: Path to source file */
                       UCHAR            *pszSrcFile,    /* I: Source File */
                       UCHAR            *pszDstPath,    /* I: Path to dest file */
                       UCHAR            *pszDstFile,    /* I: Dest File */
                       UCHAR            *szErrMsg )     /* O: Error message */
{
    /* Local variables.
    */
    int     nChar;
    int     nReturn = SDD_OK;
    UCHAR   szSourceFile[MAX_FILENAME_LEN+1];
    UCHAR   szDestFile[MAX_FILENAME_LEN+1];
    UCHAR   *szFunc = "_SCMD_MoveFile";
    FILE    *fpDestFile;
    FILE    *fpSourceFile;

    /* Combine path and filename to create an absolute source filename.
    */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
                sprintf(szSourceFile, "%s//%s", pszSrcPath, pszSrcFile);
#endif
#if defined(_WIN32)
                sprintf(szSourceFile, "%s%c%s", pszSrcPath, 0x5c, pszSrcFile);
#endif

    /* Combine path and filename to create an absolute destination filename.
    */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
                sprintf(szDestFile, "%s//%s", pszDstPath, pszDstFile);
#endif
#if defined(_WIN32)
                sprintf(szDestFile, "%s%c%s", pszDstPath, 0x5c, pszDstFile);
#endif

    /* Open source file ready for copy operation.
    */
    if((fpSourceFile=fopen(szSourceFile, "r")) == NULL)
    {
        sprintf(szErrMsg, "%s: Couldnt open source file (%s)", szSourceFile);
        return(SDD_FAIL);
    }

    /* Open destination file ready to accept copied data.
    */
    if((fpDestFile=fopen(szDestFile, "w")) == NULL)
    {
        fclose(fpSourceFile);
        sprintf(szErrMsg, "%s: Couldnt create destination file (%s)",
                szDestFile);
        return(SDD_FAIL);
    }

    /* Copy data byte by byte, checking for accuracy.
    */
    while((nChar=fgetc(fpSourceFile)) != EOF)
    {
        /* Put the newly read byte into the destination file and check return
         * status. If the put fails, exit with error.
        */
        if(fputc((UCHAR)nChar, fpDestFile) == EOF)
        {
            /* Close files, and delete destination.
            */
            fclose(fpSourceFile);
            fclose(fpDestFile);
            unlink(szDestFile);

            /* Build up error message and exit.
            */
            sprintf(szErrMsg, "%s: Error writing to destination file (%s)",
                    szDestFile);
            return(SDD_FAIL);
        }
    }

    /* All done, first close and confirm destination file, then close and
     * delete source file.
    */
    fclose(fpSourceFile);
    if(fclose(fpDestFile) == EOF)
    {
        sprintf(szErrMsg, "%s: Error writing to destination file (%s)",
                szDestFile);
        return(SDD_FAIL);
    }
    unlink(szSourceFile);

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SCMD_DeleteFile
 * Description: Function to delete a file from the given path.
 * 
 * Returns:     SDD_FAIL- An error whilst moving file, see szErrMsg.
 *              SDD_OK    - File moved successfully.
 ******************************************************************************/
int    _SCMD_DeleteFile( UCHAR        *pszDelPath,    /* I: Path to file */
                         UCHAR        *pszDelFile,    /* I: File to delete */
                         UCHAR        *szErrMsg )     /* O: Error message */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    szDeleteFile[MAX_FILENAME_LEN+1];
    UCHAR    *szFunc = "_SCMD_DeleteFile";

    /* Combine path and filename to create an absolute filename for deletion.
    */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
                sprintf(szDeleteFile, "%s//%s", pszDelPath, pszDelFile);
#endif
#if defined(_WIN32)
                sprintf(szDeleteFile, "%s%c%s", pszDelPath, 0x5c, pszDelFile);
#endif

    /* Call the operating system to perform the actual operation.
    */
    unlink(szDeleteFile);

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    scmd_InitService
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
int    scmd_InitService( SERVICEDETAILS   *sServiceDet,    /* I: Init data */
                         UCHAR            *szErrStr )      /* O: Error message */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "scmd_InitService";

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "SCMD Driver: Initialised:");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    scmd_CloseService
 * Description: Entry point which performs a drive closedown. The closedown
 *              procedure ensure that the driver returns to a virgin state
 *              (ie.like at power up) so that InitService can be called again.
 * 
 * Returns:     SDD_FAIL- An error occurred in closing the driver and an
 *                        error message is stored in szErrStr.
 *              SDD_OK    - Driver successfully closed.
 ******************************************************************************/
int    scmd_CloseService( UCHAR        *szErrMsg )    /* O: Error message if failed */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "scmd_CloseService";

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "SCMD Driver: Closed.");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    scmd_ProcessRequest
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
int    scmd_ProcessRequest( UCHAR    *snzDataBuf,           /* I: Input data */
                            int      nDataLen,              /* I: Len of data */
                            int      (*fSendDataCB)(UCHAR *, UINT),
                                                            /* I: CB to send reply*/
                            UCHAR    *szErrMsg )            /* O: Error text */
{
    /* Static variables.
    */
    static UCHAR    szWriteFile[MAX_FILENAME_LEN+1];
    static FILE     *fpWriteFile = NULL;

    /* Local variables.
    */
    int      nLastBlock;
    int      nReturn = SDD_OK;
    ULNG     lTime = 0;
    UCHAR    *pszDstFile;
    UCHAR    *pszSrcFile;
    UCHAR    *pszDstPath;
    UCHAR    *pszSrcPath;
    UCHAR    *pszArgs;
    UCHAR    *pszBufSize;
    UCHAR    *pszCmd;
    UCHAR    *pszMode;
    UCHAR    *pszTime;
    UCHAR    szReadFile[MAX_FILENAME_LEN+1];
    UCHAR    *szFunc = "scmd_ProcessRequest";
    FILE     *fpReadFile;

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

    /* If we have an EXECUTE command, then extract relevant information from
     * buffer.
    */
    if(snzDataBuf[0] == SDD_CMD_EXEC || snzDataBuf[0] == SDD_CMD_EXEC_TIMED)
    {
        /* Locate the path argument and validate it.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_SRCPATH,
                                                    &pszSrcPath) == SDD_FAIL|| 
           _SCMD_ValidatePath(pszSrcPath) == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid PATH to command provided",
                    SDD_EMSG_BADPATH);
            return(SDD_FAIL);
        }

        /* Locate the command argument and validate it.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_CMD, &pszCmd)
                                                                == SDD_FAIL ||
           _SCMD_ValidateFile(pszSrcPath, pszCmd, FALSE) == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid COMMAND provided",
                    SDD_EMSG_BADCMD);
            return(SDD_FAIL);
        }

        /* Locate the mode flag (ASCII or BINARY) and if present ensure
         * it is valid.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_MODE, &pszMode)
                                                                    == SDD_OK)
        {
            /* Is the parameter valid?
            */
            if(    strcmp(pszMode, SCMD_BINARY) != 0 && 
                strcmp(pszMode, SCMD_ASCII) !=0 )
            {
                sprintf(szErrMsg, "%s: Invalid MODE provided", SDD_EMSG_BADCMD);
                return(SDD_FAIL);
            }

            /* Setup parameter accordingly.
            */
            if( strcmp(pszMode, SCMD_BINARY) == 0 )
            {
                SCMD.nRetMode = SCMD_BINMODE;
            } else
                SCMD.nRetMode = SCMD_ASCIIMODE;
        } else
         {
            /* Setup the default return buffer mode.
            */
            SCMD.nRetMode = DEF_RETURN_MODE;
        }

        /* Locate the buffer size flag and if present, ensure its within range.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1,SCMD_BUFSIZE,&pszBufSize)
                                                                    == SDD_OK )
        {
            /* Get the value passed to verify and store.
            */
            sscanf(pszBufSize, "%d", &SCMD.nRetBufSize);
            if(    SCMD.nRetBufSize < MIN_RETURN_BUF_SIZE ||
                SCMD.nRetBufSize > MAX_RETURN_BUF_SIZE )
            {
                sprintf(szErrMsg, "%s: Invalid MODE provided", SDD_EMSG_BADCMD);
                return(SDD_FAIL);
            }
        } else
         {
            /* Setup the default return buffer size.
            */
            SCMD.nRetBufSize = DEF_RETURN_BUF_SIZE;
        }

        /* Locate the Args argument.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_ARGS, &pszArgs)
                                                                    == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid command ARGS provided",
                    SDD_EMSG_BADARGS);
            return(SDD_FAIL);
        }

        /* For timed EXECUTION commands, get the time value.
        */
        if(snzDataBuf[0] == SDD_CMD_EXEC_TIMED)
        {
            /* Locate the Time argument in the input buffer, validate that it
             * is for a time in the future and setup our pointer into it.
            */
            if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_TIME, &pszTime)
                                                                == SDD_FAIL ||
               _SCMD_ValidateTime(pszTime, &lTime) == SDD_FAIL)
            {
                sprintf(szErrMsg, "%s: Invalid TIME value provided",
                        SDD_EMSG_BADARGS);
                return(SDD_FAIL);
            }
        }
    }

    /* If we have a READ/MOVE command, then extract relevant information from
     * buffer.
    */
    if(snzDataBuf[0] == SDD_CMD_READ || snzDataBuf[0] == SDD_CMD_MOVE)
    {
        /* Locate the path argument and validate it.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_SRCPATH,
                                                    &pszSrcPath) == SDD_FAIL ||
           _SCMD_ValidatePath(pszSrcPath) == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid Source PATH provided",
                    SDD_EMSG_BADPATH);
            return(SDD_FAIL);
        }

        /* Locate the file argument and validate it.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_SRCFILE,
                                                    &pszSrcFile) == SDD_FAIL ||
           _SCMD_ValidateFile(pszSrcPath, pszSrcFile, FALSE) == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid Source FILE provided",
                    SDD_EMSG_BADPATH);
            return(SDD_FAIL);
        }
    }

    /* If we have a DELETE/MOVE command, then extract relevant information
     * from buffer.
    */
    if(snzDataBuf[0] == SDD_CMD_DEL || snzDataBuf[0] == SDD_CMD_MOVE)
    {
        /* Locate the path argument and validate it.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_DSTPATH,
                                                    &pszDstPath) == SDD_FAIL ||
           _SCMD_ValidatePath(pszDstPath) == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid Destination PATH provided",
                    SDD_EMSG_BADPATH);
            return(SDD_FAIL);
        }

        /* Locate the file argument and validate it.
        */
        if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_DSTFILE,
                                                     &pszDstFile) == SDD_FAIL ||
           _SCMD_ValidateFile(pszDstPath, pszDstFile, 
                              (snzDataBuf[0] == SDD_CMD_DEL ? FALSE : TRUE))
                               == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid Destination FILE provided",
                    SDD_EMSG_BADPATH);
            return(SDD_FAIL);
        }
    }

    /* First byte of the request data block indicates actions required, so
     * decipher it.    
    */
    switch(snzDataBuf[0])
    {
        case SDD_CMD_EXEC:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "SCMD Driver: Req to execute COMMAND");
            Lgr(LOG_DEBUG, szFunc,
                "Command: Path=%s, Cmd=%s, Args=%s",
                pszSrcPath, pszCmd, pszArgs);

            /* Execute the required command. Any output generated by the
             * command is fed back to the caller row-at-a-time. The final
             * exit status dictates wether the command was a success or 
             * failure.
            */
            if(_SCMD_Exec(FALSE,pszSrcPath,pszCmd,pszArgs,0,fSendDataCB,
                                                        szErrMsg) == SDD_FAIL)
            {
                return(SDD_FAIL);
            }
            break;

        case SDD_CMD_EXEC_TIMED:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "SCMD Driver: Req to execute timed COMMAND");
            Lgr(LOG_DEBUG, szFunc,
                "Command: Path=%s, Cmd=%s, Args=%s, Time=%ld",
                pszSrcPath, pszCmd, pszArgs, lTime);

            /* Execute the required command at the given time, and upon
             * command completion return the status to the caller via a return
             * code and text message.
            */
            if(_SCMD_Exec(TRUE,pszSrcPath,pszCmd,pszArgs,lTime,fSendDataCB,
                                                        szErrMsg) == SDD_FAIL)
            {
                return(SDD_FAIL);
            }
            break;

        case SDD_CMD_READ:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "SCMD Driver: Requested to perform a READ operation");

            /* Open the file as indicated by the command parameters.
            */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
                sprintf(szReadFile, "%s//%s", pszSrcPath, pszSrcFile);
#endif
#if defined(_WIN32)
                sprintf(szReadFile, "%s%c%s", pszSrcPath, 0x5c, pszSrcFile);
#endif
            if((fpReadFile=fopen(szReadFile, "r")) == NULL)
            {
                /* Create a message to indicate failure.
                */
                sprintf(szErrMsg,
                        "%s: Illegal Filename given in command",
                        SDD_EMSG_BADFILE);

                /* Exit directly as we have nothing open to tidy up.
                */
                return(SDD_FAIL);
            }

            /* Copy the requested file from opened file stream to the remote
             * via a return byte stream.
            */
            nReturn=_SCMD_PutReadData(fpReadFile, fSendDataCB, szErrMsg);

            /* Close the file as were all done, then exit.
            */
            fclose(fpReadFile);
            break;

        case SDD_CMD_WRITE:
            /* If this is the first time we've been called then process
             * options and open the indicated file for writing.
            */
            if(fpWriteFile == NULL)
            {
                /* Log if debugging switched on.
                */
                Lgr(LOG_DEBUG, szFunc,
                    "SCMD Driver: Requested to perform a WRITE operation");

                /* Locate the path argument and validate it.
                */
                if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_DSTPATH,
                                                    &pszDstPath) == SDD_FAIL ||
                   _SCMD_ValidatePath(pszDstPath) == SDD_FAIL)
                {
                    sprintf(szErrMsg, "%s: Invalid Destination PATH provided",
                            SDD_EMSG_BADPATH);
                    return(SDD_FAIL);
                }

                /* Locate the file argument and validate it.
                */
                if(_SCMD_GetStrArg(&snzDataBuf[1], nDataLen-1, SCMD_DSTFILE,
                                                     &pszDstFile) == SDD_FAIL ||
                   _SCMD_ValidateFile(pszDstPath, pszDstFile, TRUE) == SDD_FAIL)
                {
                    sprintf(szErrMsg, "%s: Invalid Destination FILE provided",
                            SDD_EMSG_BADPATH);
                    return(SDD_FAIL);
                }

                /* Create new file for writing.
                */
#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
                sprintf(szWriteFile, "%s//%s", pszDstPath, pszDstFile);
#endif
#if defined(_WIN32)
                sprintf(szWriteFile, "%s%c%s", pszDstPath, 0x5c, pszDstFile);
#endif
                if((fpWriteFile=fopen(szWriteFile, "w")) == NULL)
                {
                    /* Create message to indicate failure.
                    */
                    sprintf(szErrMsg, "%s: Couldnt create file",    
                            SDD_EMSG_BADPATH);
                    return(SDD_FAIL);
                }
            } else
             {
                /* Extract the data from the passed buffer and store in the
                 * opened file. Make a note as to wether this is the last
                 * block or not.
                */
                if(_SCMD_GetWriteData(&snzDataBuf[1], (nDataLen-1), fpWriteFile,
                                      &nLastBlock, szErrMsg) == SDD_FAIL)
                {
                    /* Tidy up as we are not coming back!!!!
                    */
                    fclose(fpWriteFile);
                    fpWriteFile = NULL;

                    /* Delete the file as theres no point in keeping half a
                     * file.
                    */
                    unlink(szWriteFile);

                    /* Exit with the bad tidings in szErrMsg.
                    */
                    return(SDD_FAIL);
                }

                /* If we wrote the last block successfully then alles klare,
                 * wunderbar!.
                */
                if(nLastBlock == TRUE)
                {
                    /* Firstly, close the opened file and reset the pointer for
                     * next time.
                    */
                    fclose(fpWriteFile);
                    fpWriteFile = NULL;
                }
            }
            break;

        case SDD_CMD_MOVE:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "SCMD Driver: Requested to perform a MOVE operation");

            /* Call function to perform the move and use its return code
             * as our return code.
            */
            nReturn=_SCMD_MoveFile(pszSrcPath, pszSrcFile, pszDstPath,
                                   pszDstFile, szErrMsg);
            break;

        case SDD_CMD_DEL:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "SCMD Driver: Requested to perform a DELETE operation");

            /* Call function to perform the move and use its return code
             * as our return code.
            */
            nReturn=_SCMD_DeleteFile(pszDstPath, pszDstFile, szErrMsg);
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
 * Function:    scmd_ProcessOOB
 * Description: Entry point into driver to process an out of band command
 *              that may or may not be relevant to current state of
 *              operation. The task of this function is to decipher the
 *              command and act on it immediately, ie. a cancel command
 *              would abort any ProcessRequest that is in process and
 *              clean up.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void    scmd_ProcessOOB( UCHAR    nCommand    )    /* I: OOB Command */
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
