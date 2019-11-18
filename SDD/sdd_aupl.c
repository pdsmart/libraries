/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_aupl.c
 * Description:   Server Data-source Driver library driver to handle playback
 *                of 16bit Stereo Audio WAV file to the DSP circuitry of 
 *                an underlying sound card.
 *
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1997-2019.
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
#endif

/* Indicate that we are a C module for any header specifics.
*/
#define     SDD_AUPL_C

/* Bring in local specific header files.
*/
#include    "sdd.h"
#include    "sdd_aupl.h"

/******************************************************************************
 * Function:    _AUPL_GetStrArg
 * Description: Function to scan an input buffer and extract a string based
 *              argument.
 * 
 * Returns:     SDD_FAIL- Couldnt obtain argument.
 *              SDD_OK    - Argument obtained.
 ******************************************************************************/
int    _AUPL_GetStrArg( UCHAR    *snzDataBuf,       /* I: Input buffer */
                        int      nDataLen,          /* I: Len of data */
                        UCHAR    *szArg,            /* I: Arg to look for */
                        UCHAR    **pszPath )        /* O: Pointer to argument */
{
    /* Local variables.
    */
    int          nCmpLen;
    int          nPos;
    int          nReturn = SDD_FAIL;
    UCHAR        *szFunc = "_AUPL_GetStrArg";

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
 * Function:    _AUPL_ValidatePath
 * Description: Function to validate the existence of a path.
 * 
 * Returns:     SDD_FAIL- Couldnt validate PATH.
 *              SDD_OK    - PATH validated.
 ******************************************************************************/
int    _AUPL_ValidatePath( UCHAR    *pszPath    )    /* I: Path to validate */
{
    /* Local variables.
    */
    int            nReturn = SDD_FAIL;
    UCHAR          *szFunc = "_AUPL_ValidatePath";
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
 * Function:    _AUPL_ValidateFile
 * Description: Function to validate the existence of a file or to validate
 *              that a file can be created.
 * 
 * Returns:     SDD_FAIL- Couldnt obtain Filename or validate it.
 *              SDD_OK    - Filename obtained and validated.
 ******************************************************************************/
int    _AUPL_ValidateFile( UCHAR    *pszPath,         /* I: Path to file */
                           UCHAR    *pszFile,         /* I: File to validate */
                           UINT    nWriteFlag    )    /* I: Read = 0, Write = 1 */
{
    /* Local variables.
    */
    int         nReturn = SDD_FAIL;
    UCHAR       szTmpBuf[MAX_TMPBUFLEN];
    UCHAR       *szFunc = "_AUPL_ValidateFile";
    struct stat sStat;
    FILE        *fpTest;

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

#if defined(SOLARIS) || defined(SUNOS) || defined(LINUX)
/******************************************************************************
 * Function:    _AUPL_PlayZ
 * Description: Function to play a compressed audio file. Method of attach
 *              is to launch a child which is the actual decompressor, this
 *              feeds data back via the stdout of the child to our stdin. The
 *              data is then buffered in a round robin fashion and fed to the
 *              audio DSP hardware.
 * 
 * Returns:     SDD_FAIL- Command failed during execution.
 *              SDD_OK    - Command executed successfully.
 ******************************************************************************/
int    _AUPL_PlayZ( UCHAR    *pszAudioPath,        /* I: Path to Audio File */
                    UCHAR    *pszAudioFile,        /* I: Audio Filename */
                    int      (*fSendDataCB)(UCHAR *, UINT),
                                                   /* I: Func for returning data */
                    UCHAR    *szErrMsg )           /* O: Error message generated */
{
    /* Local variables.
    */
    int            nChar;
    int            nEnd = FALSE;
    int            nNdx;
    int            nResult;
/*    int            nRetBufSize; */
    int            nReturn = SDD_OK;
    UCHAR          *pszTmpBuf;
/*    UCHAR        *pszRetBuf; */
    UCHAR          *szFunc = "_AUPL_Exec";
    FILE           *fpStdIn;
    FILE           *fpStdOut;


    /* Allocate memory for return buffer.
    if((pszRetBuf=(UCHAR *)malloc(nRetBufSize)) == NULL)
    {
        return(SDD_FAIL);
    }
    */

    /* Concatenate the decompressors file name plus the path and filename of
     * the audio file to enable the decompression of the audio file to go
     * ahead.
    */
    nNdx=strlen(AUPL.szDecompExec)+strlen(pszAudioPath)+strlen(pszAudioFile)+10;
    if((pszTmpBuf=(UCHAR *)malloc(nNdx)) == NULL)
    {
        return(SDD_FAIL);
    }
    sprintf(pszTmpBuf, "%s %s//%s", AUPL.szDecompExec, pszAudioPath,
            pszAudioFile);

    /* Execute the program as a child with its stdout connected to a read
     * stream such that we can capture its output.
    */
    if((fpStdIn=popen(pszTmpBuf, "r")) == NULL)
    {
        /* Build up an error message to indicate that the decompression
         * command failed.
        */
        sprintf(szErrMsg, "%s: Failed to de-compress file (%s//%s).",
                SDD_EMSG_BADEXEC, pszAudioPath, pszAudioFile);

        /* Set exit code to indicate failure.
        */
        nReturn = SDD_FAIL;
    } else
    if((fpStdOut=popen(AUPL.szAudioPlayer, "w")) == NULL)
    {
        /* Build up an error message to indicate that the Audio Player
         * command failed.
        */
        sprintf(szErrMsg, "%s: Failed to start Audio Player.",
                SDD_EMSG_BADEXEC);

        /* Set exit code to indicate failure.
        */
        nReturn = SDD_FAIL;
    }

    while((nChar=fgetc(fpStdIn)) != EOF)
    {
        fputc(nChar, fpStdOut);
    }

    /* Close the Decompressor and get its exit code.
    */
    nResult=pclose(fpStdIn);

    /* Close the Audio Player and get its exit code.
    */
    nResult=pclose(fpStdOut);


    /* Free up allocated memory block.
    */
    free(pszTmpBuf);

    /* Return result code to caller.
    */
    return(nReturn);
}
#endif

/******************************************************************************
 * Function:    aupl_InitService
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
int    aupl_InitService( SERVICEDETAILS   *sServiceDet,    /* I: Init data */
                         UCHAR            *szErrStr )      /* O: Error message */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "aupl_InitService";

    /* Setup any required constants, variables etc.
    */
    strcpy(AUPL.szAudioPlayer, AUPL_AUDIO_PLAYER);
    strcpy(AUPL.szDecompExec, AUPL_DECOMPRESSOR);

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "AUPL Driver: Initialised:");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    aupl_CloseService
 * Description: Entry point which performs a drive closedown. The closedown
 *              procedure ensure that the driver returns to a virgin state
 *              (ie.like at power up) so that InitService can be called again.
 * 
 * Returns:     SDD_FAIL- An error occurred in closing the driver and an
 *                        error message is stored in szErrStr.
 *              SDD_OK    - Driver successfully closed.
 ******************************************************************************/
int    aupl_CloseService( UCHAR        *szErrMsg )    /* O: Error message if failed */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "aupl_CloseService";

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "AUPL Driver: Closed.");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    aupl_ProcessRequest
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
int    aupl_ProcessRequest( UCHAR    *snzDataBuf,           /* I: Input data */
                            int      nDataLen,              /* I: Len of data */
                            int      (*fSendDataCB)(UCHAR *, UINT),
                                                            /* I: CB to send reply*/
                            UCHAR    *szErrMsg )            /* O: Error text */
{
    /* Static variables.
    */

    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *pszAudioFile;
    UCHAR    *pszAudioPath;
    UCHAR    *szFunc = "aupl_ProcessRequest";

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

    /* If we have a PLAY_Z command, then extract relevant information from
     * buffer.
    */
    if(snzDataBuf[0] == SDD_PLAY_Z)
    {
        /* Locate the path argument and validate it.
        */
        if(_AUPL_GetStrArg(&snzDataBuf[1], nDataLen-1, AUPL_AUDIOPATH,
                                                &pszAudioPath) == SDD_FAIL|| 
           _AUPL_ValidatePath(pszAudioPath) == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid PATH to Audio File provided",
                    SDD_EMSG_BADPATH);
            return(SDD_FAIL);
        }

        /* Locate the Audio File Name argument and validate it.
        */
        if(_AUPL_GetStrArg(&snzDataBuf[1], nDataLen-1, AUPL_AUDIOFILE,
                                                &pszAudioFile) == SDD_FAIL ||
           _AUPL_ValidateFile(pszAudioPath, pszAudioFile, FALSE) == SDD_FAIL)
        {
            sprintf(szErrMsg, "%s: Invalid Audio Filename provided",
                    SDD_EMSG_BADFILE);
            return(SDD_FAIL);
        }
    }

    /* First byte of the request data block indicates actions required, so
     * decipher it.    
    */
    switch(snzDataBuf[0])
    {
        case SDD_PLAY_Z:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "AUPL Driver: Req to execute Play Compressed File");
            Lgr(LOG_DEBUG, szFunc,
                "File Details: Path=%s, File=%s", pszAudioPath, pszAudioFile);

            /* Execute the function. The final exit status dictates wether the
             * command was a success or failure.
            */
            if(_AUPL_PlayZ(pszAudioPath, pszAudioFile, fSendDataCB,
                                                        szErrMsg) == SDD_FAIL)
            {
                return(SDD_FAIL);
            }
            break;

        default:
            sprintf(szErrMsg,
                    "%s: Illegal request in request buffer (%x)",
                    SDD_EMSG_BADREQ, snzDataBuf[0]);
            return(SDD_FAIL);
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    aupl_ProcessOOB
 * Description: Entry point into driver to process an out of band command
 *              that may or may not be relevant to current state of
 *              operation. The task of this function is to decipher the
 *              command and act on it immediately, ie. a cancel command
 *              would abort any ProcessRequest that is in process and
 *              clean up.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void    aupl_ProcessOOB( UCHAR    nCommand    )    /* I: OOB Command */
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
