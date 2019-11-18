/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_odbc.c
 * Description:   Server Data-source Driver library driver to handle odbc
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
#define     SDD_ODBC_C

/* Bring in local specific header files.
*/
#include    "sdd.h"

/* Bring in required ODBC header files.
*/
#include    <rdodbc.h>
/* #include    <sql.h> */
#include    <sqlext.h>
#include    "sdd_odbc.h"

/******************************************************************************
 * Function:    _ODBC_GetArg
 * Description: Function to scan an input buffer and extract a required 
 *              argument from it.
 * 
 * Returns:     SDD_FAIL- Couldnt obtain argument.
 *              SDD_OK    - Argument obtained and validated.
 ******************************************************************************/
int    _ODBC_GetArg( UCHAR    *szArgType,       /* I: Type of Arg to scan for */
                     UCHAR    *snzDataBuf,      /* I: Input buffer */
                     int      nDataLen,         /* I: Len of data */
                     UCHAR    **pszArg )        /* O: Pointer to Arg */
{
    /* Local variables.
    */
    int            nCmpLen;
    int            nPos;
    int            nReturn = SDD_FAIL;
    UCHAR          *szFunc = "_ODBC_GetArg";

    /* Go through the input buffer looking for a 'szArgType' within it. If it
     * exists, then note the position just after it as this contains the
     * database name.
    */
    for(nPos=0, nCmpLen=strlen(szArgType); nPos < nDataLen; nPos++)
    {
        /* If a match occurs, then setup the pointer to correct location.
        */
        if(strncmp(&snzDataBuf[nPos], szArgType, nCmpLen) == 0)
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
        *pszArg = &snzDataBuf[nPos];
        nReturn=SDD_OK;
    }

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _ODBC_LogODBCError
 * Description: Function to dump an error message/code from the ODBC driver
 *              to the log device. Typically used for debugging.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void    _ODBC_LogODBCError(    HSTMT    hStmt )
{
    /* Local variables.
    */
    SDWORD       lNativeError;
    SWORD        nActualMsgLen;
    UCHAR        szErrMsg[MAX_TMPBUFLEN];
    UCHAR        szSqlState[10];
    UCHAR        *szFunc = "_ODBC_LogODBCError";

    /* Call ODBC underlying to extract the error message/code.
    */
    SQLError(ODBC.hEnv, ODBC.hDbc, hStmt, szSqlState,
             &lNativeError, szErrMsg, SQL_MAX_MESSAGE_LENGTH -1,
             &nActualMsgLen);

    /* Log to logger.
    */
    Lgr(LOG_ALERT, szFunc, "ODBC Error: Msg=%s, State=%s, NativeError=%ld",
        szErrMsg, szSqlState, lNativeError);

    /* All done..
    */
    return;
}

/******************************************************************************
 * Function:    _ODBC_RunSql
 * Description: Function to execute a given buffer of SQL on the current
 *              database and return resultant data to the original caller.
 * 
 * Returns:     SDD_FAIL- SQL execution failed, see error message.
 *              SDD_OK    - SQL execution succeeded.
 ******************************************************************************/
int    _ODBC_RunSql( UCHAR    *snzDataBuf,               /* I: Input data */
                     int        nDataLen,                /* I: Len of data */
                     int        (*fSendDataCB)(UCHAR *, UINT),
                                                         /* I: CB to send reply */
                     UCHAR    *szErrMsg )                /* O: Error text */
{
    /* Local variables.
    */
    UINT        nFirstRow;
    UINT        nOffSet;
    UINT        nRowBufLen;
    UINT        nRowBufPos;
    int         nReturn = SDD_OK;
    RETCODE     nResult;
    SWORD       nBufLen;
    SWORD       nColScale;
    SWORD       nColType;
    SWORD       nNdx;
    SWORD       nNumCols;
    SWORD       nNullType;
    SDWORD      nMaxColLen;
    SDWORD      nValType;
    SDWORD      *panColLen = NULL;
    UDWORD      nColPrecision;
    UCHAR       *pszSqlBuf = NULL;
    UCHAR       *pszRowBuf = NULL;
    UCHAR       *pacRowCol = NULL;
    UCHAR       *szFunc = "_ODBC_RunSql";
    HSTMT       hStmt;

    /* Allocate memory for incoming statement storage within odbc.
    */
    nResult=SQLAllocStmt(ODBC.hDbc, &hStmt);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt allocate SQL statement memory",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_RunSql_Exit;
    }

    /* Allocate memory to null terminate inbound SQL prior to preparing it.
    */
    if((pszSqlBuf=(UCHAR *)malloc(nDataLen+1)) == NULL)
    {
        /* Build exit message to let caller know why we failed.
        */
        sprintf(szErrMsg, "%s: Out of memory allocating SQL buffer",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_RunSql_Exit;
    }

    /* Copy original SQL into buffer and terminate it.
    */
    memcpy(pszSqlBuf, snzDataBuf, nDataLen);
    pszSqlBuf[nDataLen] = '\0';

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "SQL: %s", pszSqlBuf);

    /* Load the SQL into the odbc drivers memory.
    */
    nResult=SQLPrepare(hStmt, pszSqlBuf, (SDWORD)nDataLen);

    /* Free up memory, no need to hang on to it..
    */
    free(pszSqlBuf);
    pszSqlBuf = NULL;

    /* Check result code to see if prepare created errors.
    */
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: SQL syntax error reported by ODBC driver",
                SDD_EMSG_SQLLOAD);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_RunSql_Exit;
    }

    /* Get the number of result columns that will be returned.
    */
    nResult=SQLNumResultCols(hStmt, &nNumCols);
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt determine number of result columns",
                SDD_EMSG_SQLSYNTAX);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_RunSql_Exit;
    }

    /* Allocate memory to bind the returning column lengths on.
    */
    if((panColLen=(SDWORD *)malloc(nNumCols * sizeof(SDWORD))) == NULL)
    {
        /* Build exit message to let caller know why we failed.
        */
        sprintf(szErrMsg, "%s: Out of memory allocating column lengths",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_RunSql_Exit;
    }

    /* Work out the lengths of each column. All data will be returned as
     * ASCII characters, so work with this basis. Within the same loop, work
     * out the maximum column name length to be used for buffer allocation.
    */
    for(nNdx=0, nMaxColLen=0; nNdx < nNumCols; nNdx++)
    {
        /* Request the ODBC driver to return the length of the column.
        */
        nResult=SQLColAttributes(hStmt, nNdx+1, SQL_COLUMN_DISPLAY_SIZE,
                                  NULL, 0, NULL, panColLen+nNdx);
        if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
        {
            /* Log message to indicate reason.
            */
            _ODBC_LogODBCError( hStmt );

            /* Build up exit message and get out.
            */
            sprintf(szErrMsg, "%s: Couldnt determine length of a result column",
                    SDD_EMSG_SQLSYNTAX);

            /* Goto exit point, setting error code on way out.
            */
            nReturn = SDD_FAIL;
            goto _ODBC_RunSql_Exit;
        }

        /* Is this column length greater than our current maximum?
        */
        if(*(panColLen+nNdx) > nMaxColLen)
            nMaxColLen = *(panColLen+nNdx) + 1;
    }

    /* Allocate memory to bind the returning columns on.
    */
    if((pacRowCol=(UCHAR *)malloc(nNumCols*nMaxColLen)) == NULL)
    {
        /* Build exit message to let caller know why we failed.
        */
        sprintf(szErrMsg, "%s: Out of memory allocating column bindings",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_RunSql_Exit;
    }

    /* Bind the expected columns to the allocated memory.
    */
    for(nNdx=0, nOffSet=0, nValType=SQL_NO_TOTAL; nNdx < nNumCols;
        nNdx++, nOffSet+=nMaxColLen)
    {
        /* Bind column according to index value.
        */
        nResult=SQLBindCol(hStmt, nNdx+1, SQL_C_CHAR, pacRowCol+nOffSet,
                           nMaxColLen, &nValType);
        if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
        {
            /* Log message to indicate reason.
            */
            _ODBC_LogODBCError( hStmt );

            /* Build up exit message and get out.
            */
            sprintf(szErrMsg, "%s: Couldnt bind a column to memory",
                    SDD_EMSG_SQLRUNERR);

            /* Goto exit point, setting error code on way out.
            */
            nReturn = SDD_FAIL;
            goto _ODBC_RunSql_Exit;
        }
    }

    /* Allocate memory to hold all column data as one continous character
     * row, seperated by the pipe symbol.
    */
    nRowBufLen=nMaxColLen * nNumCols;
    if((pszRowBuf=(UCHAR *)malloc(nRowBufLen+1)) == NULL)
    {
        /* Build exit message to let caller know why we failed.
        */
        sprintf(szErrMsg, "%s: Out of memory allocating row return buffer",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_RunSql_Exit;
    }

    /* OK, lets go for it, execute SQL time.
    */
    nResult=SQLExecute(hStmt);
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Runtime error during SQL execution",
                SDD_EMSG_SQLRUNERR);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_RunSql_Exit;
    }

    /* If there is no data to be fetched, then just fast forward.
    */
    if(nNumCols == 0)
    {
        goto _ODBC_RunSql_Exit;
    }

    /* Initialise any required variables.
    */
    ODBC.nAbortPending = FALSE;
    nFirstRow = TRUE;

    /* Loop until we have exhausted the number of rows or an ABORT condition
     * is generated.
    */
    do {
        /* If this is the first row then get the column header, else get the
         * actual rows.
        */
        if( nFirstRow == TRUE )
        {
            /* To get the names of each column header, go in a loop and load
             * the name into the row array buffers.
            */
            for(nNdx=0, nOffSet=0; nNdx < nNumCols; nNdx++, nOffSet+=nMaxColLen)
            {
                /* Get the name of one column, equal to nNdx+1.
                */
                nResult=SQLDescribeCol(hStmt, nNdx+1,
                                       (UCHAR *)pacRowCol+nOffSet,
                                       nMaxColLen, &nBufLen, &nColType,
                                       &nColPrecision, &nColScale,
                                       &nNullType);
                if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
                {
                    /* Log message to indicate reason.
                    */
                    _ODBC_LogODBCError( hStmt );

                    /* Build up exit message and get out.
                    */
                    sprintf(szErrMsg, "%s: Couldnt get name of a column",
                            SDD_EMSG_SQLSYNTAX);

                    /* Goto exit point, setting error code on way out.
                    */
                    nReturn = SDD_FAIL;
                    goto _ODBC_RunSql_Exit;
                }
            }

            /* Toggle flag to ensure that we dont send the header again.
            */
            nFirstRow = FALSE;
        } else
         {
            /* Fetch a row of results from the ODBC driver.
            */
            nResult=SQLFetch(hStmt);

            /* If we dont have a success status or a no more data status,
             * then exit, as we hit an error that we dont intend to handle.
            */
            if(nResult != SQL_SUCCESS && nResult != SQL_NO_DATA_FOUND &&
               nResult != SQL_SUCCESS_WITH_INFO)
            {
                /* Log message to indicate reason.
                */
                _ODBC_LogODBCError( hStmt );

                /* Build up exit message and get out.
                */
                sprintf(szErrMsg, "%s: Runtime error whilst fetching data",
                        SDD_EMSG_SQLRUNERR);

                /* Goto exit point, setting error code on way out.
                */
                nReturn = SDD_FAIL;
                goto _ODBC_RunSql_Exit;
            }

            /* If there are any error messages (or info) associated with 
             * this operation then log it.
            */
            if(nResult == SQL_SUCCESS_WITH_INFO)
            {
                _ODBC_LogODBCError( hStmt );
            }
        }

        /* If there is any data to be transmitted, ie. nResult is set to
         * SQL_SUCCESS or SQL_SUCCESS_WITH_INFO then transmit it to the
         * client.
        */
        if(nResult == SQL_SUCCESS || nResult == SQL_SUCCESS_WITH_INFO)
        {
            /* Clear out the row buffer prior to assembling the array buffers
             * into it.
            */
            memset(pszRowBuf, ' ', nRowBufLen);
            pszRowBuf[nRowBufLen] = '\0';

            /* Place the data in each array buffer into the row buffer with
             * appropriate seperation characters.
            */
            for(nNdx=0, nOffSet=0, nRowBufPos=0; nNdx < nNumCols;
                nNdx++, nOffSet+=nMaxColLen)
            {
                /* Copy actual data into the row buffer.
                */
                memcpy(&pszRowBuf[nRowBufPos], pacRowCol+nOffSet,
                       strlen((UCHAR *)pacRowCol+nOffSet));

                /* Update row position.
                */
                if(*(panColLen+nNdx) > strlen((UCHAR *)pacRowCol+nOffSet))
                {
                    nRowBufPos += *(panColLen+nNdx);
                } else
                 {
                    nRowBufPos += strlen((UCHAR *)pacRowCol+nOffSet);
                }

                /* Insert seperation character if this is not the last col.
                */
                if( nNdx < nNumCols-1 )
                {
                    strncpy(&pszRowBuf[nRowBufPos++], DEF_COLSEP,
                            strlen(DEF_COLSEP));
                }
            }

            /* Terminate string prior to transmission and set correct length.
            */
            pszRowBuf[nRowBufPos] = '\0';

            /* Log row if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc, "%s", pszRowBuf);

            /* Call function to transmit row to original caller.
            */
            if(fSendDataCB(pszRowBuf, nRowBufPos) == SDD_FAIL)
            {
                /* Build error message for return.
                */
                sprintf(szErrMsg,
                        "%s: Failed to send row back to client",
                        SDD_EMSG_SENDROW);

                /* Goto exit point, setting error code on way out.
                */
                nReturn = SDD_FAIL;
                goto _ODBC_RunSql_Exit;
            }
        }
    } while( (ODBC.nAbortPending == FALSE) && 
             (nResult == SQL_SUCCESS || nResult == SQL_SUCCESS_WITH_INFO) );

    /* If an abort command arrived halfway through processing then tidy up
     * for exit.
    */
    if(ODBC.nAbortPending == TRUE)
    {
        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: ABORT received, query abandoned",
                SDD_EMSG_ABORTRCVD);
        nReturn = SDD_FAIL;
    }

_ODBC_RunSql_Exit:
    /* Free memory prior to exit.
    */
    if(panColLen != NULL)
        free(panColLen);
    if(pacRowCol != NULL)
        free(pacRowCol);
    if(pszRowBuf != NULL)
        free(pszRowBuf);

    /* Clear SQL Cursor prior to exit.
    */
    nResult=SQLFreeStmt(hStmt, SQL_CLOSE);
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Failed to close SQL cursor",
                SDD_EMSG_SQLRUNERR);

        /* Set up failure code to indicate condition.    
        */
        nReturn = SDD_FAIL;
    }

    /* Free up statement memory.
    */
    nResult=SQLFreeStmt(hStmt, SQL_DROP);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Failed to free ODBC handle",
                SDD_EMSG_SQLRUNERR);

        /* Setup failure code to indicate condition.    
        */
        nReturn = SDD_FAIL;
    }

    /* Return success/fail to caller depending on execution results.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _ODBC_ListDB
 * Description: Function to list all the names of databases available on the
 *              currently open data source.
 * 
 * Returns:     SDD_FAIL- SQL execution failed, see error message.
 *              SDD_OK    - SQL execution succeeded.
 ******************************************************************************/
int    _ODBC_ListDB( int      (*fSendDataCB)(UCHAR *, UINT),
                                                         /* I: CB to send reply */
                     UCHAR    *szErrMsg )                /* O: Error text */
{
    /* Local variables.
    */
    UINT        nFirstRow;
    int         nReturn = SDD_OK;
    RETCODE     nResult;
    UWORD       nDirection;
    SWORD       nDescrBufLen;
    SWORD       nDSNBufLen;
    UCHAR       szDataSourceName[SQL_MAX_DSN_LENGTH+1];
    UCHAR       szDescription[MAX_DSN_DESCR_LEN+1];
    UCHAR       szRowBuf[SQL_MAX_DSN_LENGTH + MAX_DSN_DESCR_LEN + 5];
    UCHAR       *szFunc = "_ODBC_ListDB";

    /* Initialise any required variables.
    */
    ODBC.nAbortPending = FALSE;
    nFirstRow = TRUE;

    /* Go in a loop, reading each database name and returning it to the caller
     * until all have been exhausted. 
    */
    do {
        /* If this is the first row then start the query at the beginning.
        */
        nDirection=(nFirstRow == TRUE ? SQL_FETCH_FIRST : SQL_FETCH_NEXT);
        nFirstRow = FALSE;

        /* Query the driver for the next database name.
        */
        nResult=SQLDataSources(ODBC.hEnv, nDirection, szDataSourceName,
                               SQL_MAX_DSN_LENGTH, &nDSNBufLen,
                               szDescription, MAX_DSN_DESCR_LEN,
                               &nDescrBufLen);
        if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO &&
           nResult != SQL_NO_DATA_FOUND)
        {
            /* Log message to indicate reason.
            */
            _ODBC_LogODBCError( 0 );

            /* Build up exit message and get out.
            */
            sprintf(szErrMsg, "%s: Couldnt get data source name",
                    SDD_EMSG_BADDBASE);

            /* Goto exit point, setting error code on way out.
            */
            nReturn = SDD_FAIL;
            goto _ODBC_ListDB_Exit;
        }

        /* If data is available, return it to calling client.
        */
        if(nResult == SQL_SUCCESS || nResult == SQL_SUCCESS_WITH_INFO)
        {
            /* Build up the row for return from components.
            */
            sprintf(szRowBuf, "%s", szDataSourceName);

            /* Call function to transmit row to original caller.
            */
            if(fSendDataCB(szRowBuf, strlen(szRowBuf)) == SDD_FAIL)
            {
                /* Build error message for return.
                */
                sprintf(szErrMsg,
                        "%s: Failed to send row back to client",
                        SDD_EMSG_SENDROW);

                /* Goto exit point, setting error code on way out.
                */
                nReturn = SDD_FAIL;
                goto _ODBC_ListDB_Exit;
            }
        }
    } while( (ODBC.nAbortPending == FALSE) && (nResult != SQL_NO_DATA_FOUND) );

    /* If an abort command arrived halfway through processing then tidy up
     * for exit.
    */
    if(ODBC.nAbortPending == TRUE)
    {
        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: ABORT received, query abandoned",
                SDD_EMSG_ABORTRCVD);
        nReturn = SDD_FAIL;
    }

    /* Complete processing by freeing/closing used resources and exitting
     * with pre-set return code.
    */
_ODBC_ListDB_Exit:

    /* Return success/fail to caller depending on execution results.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _ODBC_ListTables
 * Description: Function to list all names of tables in a given database
 *              (or current database if no database name given).
 * 
 * Returns:     SDD_FAIL- SQL execution failed, see error message.
 *              SDD_OK    - SQL execution succeeded.
 ******************************************************************************/
int    _ODBC_ListTables( UCHAR    *snzDataBuf,               /* I: Input data */
                         int        nDataLen,                /* I: Len of data */
                         int        (*fSendDataCB)(UCHAR *, UINT),
                                                             /* I: CB to send reply */
                         UCHAR    *szErrMsg )                /* O: Error text */
{
    /* Local variables.
    */
    int          nReturn = SDD_OK;
    RETCODE      nResult;
    SDWORD       nOwnerLen;
    SDWORD       nQualifierLen;
    SDWORD       nRemarksLen;
    SDWORD       nTableLen;
    SDWORD       nTypeLen;
    UCHAR        *pszRowBuf = NULL;
    UCHAR        szOwner[MAX_TABLE_DESCR_LEN];
    UCHAR        szQualifier[MAX_TABLE_DESCR_LEN];
    UCHAR        *pszDbName;
    UCHAR        szRemarks[MAX_TABLE_DESCR_LEN];
    UCHAR        szTableName[MAX_TABLE_DESCR_LEN];
    UCHAR        szTableType[MAX_TABLE_DESCR_LEN];
    UCHAR        *szFunc = "_ODBC_ListTables";
    HSTMT        hStmt;

    /* See if the caller has provided the name of a new database. If HE hasnt
     * then assume current database.
    */
    if(_ODBC_GetArg(ODBC_DBNAME, snzDataBuf, nDataLen, &pszDbName) == SDD_FAIL)
    {
        /* If the length of the input buffer indicates that there is data within
         * it, yet it does not contain a database name, then it is illegal.
        */
        if(nDataLen > 1)
        {
            sprintf(szErrMsg, "%s: Invalid DATABASE NAME provided",
                    SDD_EMSG_BADCMD);
            return(SDD_FAIL);
        } else
         {
            /* Setup the variable so that the select below will work on the
             * current database.
            */
            pszDbName = NULL;
        }
    } else
     {
        /* AAAAAHHHHH! Database name switching not yet supported... tee
         * hee, message for the sucker!!
        */
        sprintf(szErrMsg,
                "%s: ODBC switch to database function not yet implemented",
                SDD_EMSG_NOTYI);
        return(SDD_FAIL);
    }

    /* Allocate memory for the statement handle.
    */
    nResult=SQLAllocStmt(ODBC.hDbc, &hStmt);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt allocate Statement memory",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_ListTables_Exit;
    }

    /* Call odbc function to generate a listing of all the tables in the
     * data source.
    */
    nResult=SQLTables(hStmt, NULL, SQL_NTS, NULL, SQL_NTS, NULL, SQL_NTS,
                      NULL,SQL_NTS);
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt request table details (%d)",
                SDD_EMSG_SQLSYNTAX, nResult);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_ListTables_Exit;
    }

    /* Bind the expected columns to the allocated memory.
    */
    SQLBindCol(hStmt, 1, SQL_C_CHAR, szQualifier, MAX_TABLE_DESCR_LEN,
               &nQualifierLen);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, szOwner, MAX_TABLE_DESCR_LEN,
               &nOwnerLen);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, szTableName, MAX_TABLE_DESCR_LEN,
               &nTableLen);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, szTableType, MAX_TABLE_DESCR_LEN,
               &nTypeLen);
    SQLBindCol(hStmt, 5, SQL_C_CHAR, szRemarks, MAX_TABLE_DESCR_LEN,
               &nRemarksLen);

    /* Allocate memory to hold all column data as one continous character
     * row, seperated by the pipe symbol.
    */
    if((pszRowBuf=(UCHAR *)malloc( ((MAX_TABLE_DESCR_LEN+1)*5) +1)) == NULL)
    {
        /* Build exit message to let caller know why we failed.
        */
        sprintf(szErrMsg, "%s: Out of memory allocating row return buffer",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_ListTables_Exit;
    }

    /* Initialise any required variables.
    */
    ODBC.nAbortPending = FALSE;

    /* Loop until we have exhausted the number of rows or an ABORT condition
     * is generated.
    */
    do {
        /* Fetch a row of results from the ODBC driver.
        */
        nResult=SQLFetch(hStmt);

        /* If we dont have a success status or a no more data status,
         * then exit, as we hit an error that we dont intend to handle.
        */
        if(nResult != SQL_SUCCESS && nResult != SQL_NO_DATA_FOUND &&
           nResult != SQL_SUCCESS_WITH_INFO)
        {
            /* Log message to indicate reason.
            */
            _ODBC_LogODBCError( hStmt );

            /* Build up exit message and get out.
            */
            sprintf(szErrMsg, "%s: Runtime error whilst fetching data",
                    SDD_EMSG_SQLRUNERR);

            /* Goto exit point, setting error code on way out.
            */
            nReturn = SDD_FAIL;
            goto _ODBC_ListTables_Exit;
        }

        /* If there are any error messages (or info) associated with 
         * this operation then log it.
        */
        if(nResult == SQL_SUCCESS_WITH_INFO)
        {
            _ODBC_LogODBCError( hStmt );
        }

        /* If there is any data to be transmitted, ie. nResult is set to
         * SQL_SUCCESS or SQL_SUCCESS_WITH_INFO then transmit it to the
         * client.
        */
        if(nResult == SQL_SUCCESS || nResult == SQL_SUCCESS_WITH_INFO)
        {
            /* Place the appropriate data in the return buffer.
            */
            sprintf(pszRowBuf, "%s%s%s",
                    szTableName, DEF_COLSEP, szTableType);

            /* Log row if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc, "%s", pszRowBuf);

            /* Call function to transmit row to original caller.
            */
            if(fSendDataCB(pszRowBuf, strlen(pszRowBuf)) == SDD_FAIL)
            {
                /* Build error message for return.
                */
                sprintf(szErrMsg,
                        "%s: Failed to send row back to client",
                        SDD_EMSG_SENDROW);

                /* Goto exit point, setting error code on way out.
                */
                nReturn = SDD_FAIL;
                goto _ODBC_ListTables_Exit;
            }
        }
    } while( (ODBC.nAbortPending == FALSE) && 
             (nResult == SQL_SUCCESS || nResult == SQL_SUCCESS_WITH_INFO) );

    /* If an abort command arrived halfway through processing then tidy up
     * for exit.
    */
    if(ODBC.nAbortPending == TRUE)
    {
        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: ABORT received, query abandoned",
                SDD_EMSG_ABORTRCVD);
        nReturn = SDD_FAIL;
    }

_ODBC_ListTables_Exit:
    /* Free memory prior to exit.
    */
    if(pszRowBuf != NULL)
        free(pszRowBuf);

    /* Clear SQL Cursor prior to exit.
    */
    nResult=SQLFreeStmt(hStmt, SQL_CLOSE);
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Failed to close SQL cursor",
                SDD_EMSG_SQLRUNERR);

        /* Set up failure code to indicate condition.    
        */
        nReturn = SDD_FAIL;
    }

    /* Free up statement memory.
    */
    nResult=SQLFreeStmt(hStmt, SQL_DROP);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Failed to free ODBC handle",
                SDD_EMSG_SQLRUNERR);

        /* Setup failure code to indicate condition.    
        */
        nReturn = SDD_FAIL;
    }


    /* Return success/fail to caller depending on execution results.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _ODBC_ListCols
 * Description: Function to list all names and attributes of columns in a
 *              given table in a given database (or current database/table if
 *              no database name given).
 * 
 * Returns:     SDD_FAIL- SQL execution failed, see error message.
 *              SDD_OK    - SQL execution succeeded.
 ******************************************************************************/
int    _ODBC_ListCols( UCHAR    *snzDataBuf,               /* I: Input data */
                       int      nDataLen,                  /* I: Len of data */
                       int      (*fSendDataCB)(UCHAR *, UINT),
                                                           /* I: CB to send reply */
                       UCHAR    *szErrMsg )                /* O: Error text */
{
    /* Local variables.
    */
    UINT        nRowCount = 0;
    int         nReturn = SDD_OK;
    RETCODE     nResult;
    SWORD       nDataType;
    SWORD       nScale;
    SWORD       nRadix;
    SWORD       nNullable;
    SDWORD      lLength;
    SDWORD      lPrecision;
    SDWORD      nOwnerLen;
    SDWORD      nQualifierLen;
    SDWORD      nRemarksLen;
    SDWORD      nTableLen;
    SDWORD      nColLen;
    SDWORD      nDataTypeLen;
    SDWORD      nNameTypeLen;
    SDWORD      nPrecisionLen;
    SDWORD      nLengthLen;
    SDWORD      nScaleLen;
    SDWORD      nRadixLen;    
    SDWORD      nNullableLen;
    UCHAR       *pszRowBuf = NULL;
    UCHAR       *pszTableName;
    UCHAR       *pszDbName;
    UCHAR       *pszColFilter;
    UCHAR       szColName[MAX_TABLE_DESCR_LEN];
    UCHAR       szQualifier[MAX_TABLE_DESCR_LEN];
    UCHAR       szOwner[MAX_TABLE_DESCR_LEN];
    UCHAR       szTmpTableName[MAX_TABLE_DESCR_LEN];
    UCHAR       szTypeName[MAX_TABLE_DESCR_LEN];
    UCHAR       szRemarks[MAX_TABLE_DESCR_LEN];
    UCHAR       *szFunc = "_ODBC_ListTables";
    HSTMT       hStmt;

    /* See if the caller has provided the name of a new database. If HE hasnt
     * then assume current database.
    */
    if(_ODBC_GetArg(ODBC_DBNAME, snzDataBuf, nDataLen, &pszDbName) == SDD_FAIL)
    {
        /* Setup the variable so that the select below will work on the
         * current database.
        */
        pszDbName = NULL;
    } else
     {
        /* AAAAAHHHHH! Database name switching not yet supported... tee
         * hee, message for the sucker!!
        */
        sprintf(szErrMsg,
                "%s: ODBC switch to database function not yet implemented",
                SDD_EMSG_NOTYI);
        return(SDD_FAIL);
    }

    /* Caller must provide the name of a table... else what do we scan for...
     * a French Empire, vapourware... tell me more!!
    */
    if(_ODBC_GetArg(ODBC_TABLENAME, snzDataBuf, nDataLen, &pszTableName)
                                                                    == SDD_FAIL)
    {
        /* Always generate an error... French Empire.. bah humbug!
        */
        sprintf(szErrMsg, "%s: Invalid TABLE NAME provided",
                SDD_EMSG_BADARGS);
        return(SDD_FAIL);
    }

    /* See if the caller has provided the name of a specific column. If HE has
     * then he requires data limited to just that one column.
    */
    if(_ODBC_GetArg(ODBC_COLFILTER,snzDataBuf,nDataLen,&pszColFilter)==SDD_FAIL)
    {
        /* Setup the variable so that the column filter logic below wont work
         * as no specific column name has been provided, hence the caller
         * wants all columns.
        */
        pszColFilter = NULL;
    } 

    /* Allocate memory for the statement handle.
    */
    nResult=SQLAllocStmt(ODBC.hDbc, &hStmt);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt allocate Statement memory",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_ListColumns_Exit;
    }

    /* Call odbc function to generate a listing of all the columns in a
     * given table in the current data source.
    */
    nResult=SQLColumns(hStmt, NULL, SQL_NTS, NULL, SQL_NTS, 
                        pszTableName, SQL_NTS, NULL, SQL_NTS);
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt request column details (%d)",
                SDD_EMSG_SQLSYNTAX, nResult);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_ListColumns_Exit;
    }

    /* Bind the expected columns to the allocated memory.
    */
    SQLBindCol(hStmt, 1, SQL_C_CHAR, szQualifier, MAX_TABLE_DESCR_LEN,
               &nQualifierLen);
    SQLBindCol(hStmt, 2, SQL_C_CHAR, szOwner, MAX_TABLE_DESCR_LEN,
               &nOwnerLen);
    SQLBindCol(hStmt, 3, SQL_C_CHAR, szTmpTableName, MAX_TABLE_DESCR_LEN,
               &nTableLen);
    SQLBindCol(hStmt, 4, SQL_C_CHAR, szColName, MAX_TABLE_DESCR_LEN,
               &nColLen);
    SQLBindCol(hStmt, 5, SQL_C_SSHORT, &nDataType, 0, &nDataTypeLen);
    SQLBindCol(hStmt, 6, SQL_C_CHAR, szTypeName, MAX_TABLE_DESCR_LEN,
               &nNameTypeLen);
    SQLBindCol(hStmt, 7, SQL_C_SLONG, &lPrecision, 0, &nPrecisionLen);
    SQLBindCol(hStmt, 8, SQL_C_SLONG, &lLength, 0, &nLengthLen);
    SQLBindCol(hStmt, 9, SQL_C_SSHORT, &nScale, 0, &nScaleLen);
    SQLBindCol(hStmt, 10, SQL_C_SSHORT, &nRadix, 0, &nRadixLen);
    SQLBindCol(hStmt, 11, SQL_C_SSHORT, &nNullable, 0, &nNullableLen);
    SQLBindCol(hStmt, 12, SQL_C_CHAR, szRemarks, MAX_TABLE_DESCR_LEN,
               &nRemarksLen);

    /* Allocate memory to hold all column data as one continous character
     * row, seperated by the pipe symbol.
    */
    if((pszRowBuf=(UCHAR *)malloc( ((MAX_TABLE_DESCR_LEN+1)*12) +1)) == NULL)
    {
        /* Build exit message to let caller know why we failed.
        */
        sprintf(szErrMsg, "%s: Out of memory allocating row return buffer",
                SDD_EMSG_MEMORY);

        /* Goto exit point, setting error code on way out.
        */
        nReturn = SDD_FAIL;
        goto _ODBC_ListColumns_Exit;
    }

    /* Initialise any required variables.
    */
    ODBC.nAbortPending = FALSE;

    /* Loop until we have exhausted the number of rows or an ABORT condition
     * is generated.
    */
    do {
        /* Fetch a row of results from the ODBC driver.
        */
        nResult=SQLFetch(hStmt);

        /* If we dont have a success status or a no more data status,
         * then exit, as we hit an error that we dont intend to handle.
        */
        if(nResult != SQL_SUCCESS && nResult != SQL_NO_DATA_FOUND &&
           nResult != SQL_SUCCESS_WITH_INFO)
        {
            /* Log message to indicate reason.
            */
            _ODBC_LogODBCError( hStmt );

            /* Build up exit message and get out.
            */
            sprintf(szErrMsg, "%s: Runtime error whilst fetching data",
                    SDD_EMSG_SQLRUNERR);

            /* Goto exit point, setting error code on way out.
            */
            nReturn = SDD_FAIL;
            goto _ODBC_ListColumns_Exit;
        }

        /* If there are any error messages (or info) associated with 
         * this operation then log it.
        */
        if(nResult == SQL_SUCCESS_WITH_INFO)
        {
            _ODBC_LogODBCError( hStmt );
        }

        /* If there is any data to be transmitted, ie. nResult is set to
         * SQL_SUCCESS or SQL_SUCCESS_WITH_INFO then run through the column
         * filter if applicable and transmit it to the client.
        */
        if(nResult == SQL_SUCCESS || nResult == SQL_SUCCESS_WITH_INFO)
        {
            /* Is the column filter active? If so, only pass the column name
             * which has been specified. If it hasnt, pass all columns.
            */
            if( (pszColFilter != NULL && strcmp(pszColFilter, szColName) == 0) ||
                (pszColFilter == NULL) )
            {
                /* Place the appropriate data in the return buffer.
                */
                sprintf(pszRowBuf,
                        "%s%s%s%s%ld%s%ld%s%d%s%d%s%d",
                        szColName,        DEF_COLSEP,
                        szTypeName,        DEF_COLSEP,
                        lPrecision,        DEF_COLSEP,
                        lLength,        DEF_COLSEP,
                        nScale,            DEF_COLSEP,
                        nRadix,            DEF_COLSEP,
                        nNullable);

                /* Log row if debugging switched on.
                */
                Lgr(LOG_DEBUG, szFunc, "%s", pszRowBuf);

                /* Call function to transmit row to original caller.
                */
                if(fSendDataCB(pszRowBuf, strlen(pszRowBuf)) == SDD_FAIL)
                {
                    /* Build error message for return.
                    */
                    sprintf(szErrMsg,
                            "%s: Failed to send row back to client",
                            SDD_EMSG_SENDROW);

                    /* Goto exit point, setting error code on way out.
                    */
                    nReturn = SDD_FAIL;
                    goto _ODBC_ListColumns_Exit;
                } else
                 {
                    /* Increment row counter to indicate number of rows returned.
                    */
                    nRowCount++;
                }
            }
        }
    } while( (ODBC.nAbortPending == FALSE) && 
             (nResult == SQL_SUCCESS || nResult == SQL_SUCCESS_WITH_INFO) );

    /* If no rows where detected or returned, then there was an error with the
     * column, table or database name.
    */
    if(nRowCount == 0)
    {
        /* Build error message for return.
        */
        sprintf(szErrMsg,
                (pszColFilter == NULL ? "%s: Table name not known by system" :
                                        "%s: Column name not valid for table"),
                SDD_EMSG_SQLRUNERR);

        /* Setting error code to indicate condition.
        */
        nReturn = SDD_FAIL;
    }

    /* If an abort command arrived halfway through processing then tidy up
     * for exit.
    */
    if(ODBC.nAbortPending == TRUE)
    {
        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: ABORT received, query abandoned",
                SDD_EMSG_ABORTRCVD);
        nReturn = SDD_FAIL;
    }

_ODBC_ListColumns_Exit:
    /* Free memory prior to exit.
    */
    if(pszRowBuf != NULL)
        free(pszRowBuf);

    /* Clear SQL Cursor prior to exit.
    */
    nResult=SQLFreeStmt(hStmt, SQL_CLOSE);
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( hStmt );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Failed to close SQL cursor",
                SDD_EMSG_SQLRUNERR);

        /* Set up failure code to indicate condition.    
        */
        nReturn = SDD_FAIL;
    }

    /* Free up statement memory.
    */
    nResult=SQLFreeStmt(hStmt, SQL_DROP);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Failed to free ODBC handle",
                SDD_EMSG_SQLRUNERR);

        /* Setup failure code to indicate condition.    
        */
        nReturn = SDD_FAIL;
    }

    /* Return success/fail to caller depending on execution results.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    odbc_InitService
 * Description: Entry point which initialises the driver into a defined state.
 *              It is mandatory that this function is called before any other
 *              in order for the driver to function correctly. The caller
 *              provides it with two types of data, 1) A structure containing
 *              data for it to use in initialising itself, 2) a pointer to a
 *              buffer which the driver uses to place an error message should
 *              it not be able to complete initialisation.
 * 
 * Returns:     SDD_FAIL- An error occurred in initialising the driver and an
 *                        error message is stored in szErrMsg.
 *              SDD_OK    - Driver initialised successfully.
 ******************************************************************************/
int    odbc_InitService( SERVICEDETAILS   *sServiceDet,    /* I: Init data */
                         UCHAR            *szErrMsg )      /* O: Error message */
{
    /* Local variables.
    */
    RETCODE  nResult;
    UCHAR    *szFunc = "odbc_InitService";

    /* Copy all configuration data out of the service structure.
    */
    strcpy(ODBC.szUserName, sServiceDet->uServiceInfo.sODBCInfo.szUser);
    strcpy(ODBC.szPassword, sServiceDet->uServiceInfo.sODBCInfo.szPassword);
    strcpy(ODBC.szServer, sServiceDet->uServiceInfo.sODBCInfo.szServer);
    strcpy(ODBC.szDatabase,
            sServiceDet->uServiceInfo.sODBCInfo.szDatabase);

    /* Initialise memory etc within the ODBC API.
    */
    nResult=SQLAllocEnv(&ODBC.hEnv);
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( (HSTMT)0 );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt initialise ODBC API env memory",
                SDD_EMSG_MEMORY);
        return(SDD_FAIL);
    }

    /* If there is information with previos command, display it during
     * debug mode.
    */
    if(nResult == SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( (HSTMT)0 );
    }

    /* Initialise and allocate ODBC API connection memory.
    */
    nResult=SQLAllocConnect( ODBC.hEnv, &ODBC.hDbc );
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( (HSTMT)0 );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt initialise ODBC API connection memory",
                SDD_EMSG_MEMORY);
        return(SDD_FAIL);
    }

    /* If there is information with previous command, display it during
     * debug mode.
    */
    if(nResult == SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( (HSTMT)0 );
    }

    /* Create a connection to the data source.
    */
    nResult=SQLConnect(ODBC.hDbc,     ODBC.szServer,        SQL_NTS,
                                    ODBC.szUserName,    SQL_NTS,
                                    ODBC.szPassword,    SQL_NTS );
    if(nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( (HSTMT)0 );

        /* Build up exit message and get out.
        */
        sprintf(szErrMsg, "%s: Couldnt connect to data source",
                SDD_EMSG_BADLOGIN);
        return(SDD_FAIL);
    }

    /* If there is information with previos command, display it during
     * debug mode.
    */
    if(nResult == SQL_SUCCESS_WITH_INFO)
    {
        /* Log message to indicate reason.
        */
        _ODBC_LogODBCError( (HSTMT)0 );
    }

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc,
        "ODBC Driver: Initialised: (User=%s, Pwd=<hidden>, Srv=%s, DB=%s)",
        ODBC.szUserName, ODBC.szServer, ODBC.szDatabase);

    /* Return result code to caller.
    */
    return(SDD_OK);
}

/******************************************************************************
 * Function:    odbc_CloseService
 * Description: Entry point which performs a drive closedown. The closedown
 *              procedure ensure that the driver returns to a virgin state
 *              (ie.like at power up) so that InitService can be called again.
 * 
 * Returns:     SDD_FAIL- An error occurred in closing the driver and an
 *                        error message is stored in szErrMsg.
 *              SDD_OK    - Driver successfully closed.
 ******************************************************************************/
int    odbc_CloseService( UCHAR        *szErrMsg )    /* O: Error message if failed */
{
    /* Local variables.
    */
    RETCODE      nResult;
    UCHAR        *szFunc = "odbc_CloseService";

    /* Drop connection to data source.
    */
    nResult=SQLDisconnect(ODBC.hDbc);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        sprintf(szErrMsg, "%s: Couldnt drop connection to data source",
                SDD_EMSG_BADEXIT);
        return(SDD_FAIL);
    }

    /* Free up connection memory.
    */
    nResult=SQLFreeConnect(ODBC.hDbc);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        sprintf(szErrMsg, "%s: Couldnt free connection memory",
                SDD_EMSG_BADFREE);
        return(SDD_FAIL);
    }

    /* Free up environment memory.
    */
    nResult=SQLFreeEnv(ODBC.hEnv);
    if( nResult != SQL_SUCCESS && nResult != SQL_SUCCESS_WITH_INFO )
    {
        sprintf(szErrMsg, "%s: Couldnt free environment memory",
                SDD_EMSG_BADFREE);
        return(SDD_FAIL);
    }

    /* Tidy up variables.
    */
    ODBC.nAbortPending = FALSE;
    strcpy(ODBC.szUserName, "");
    strcpy(ODBC.szPassword, "");
    strcpy(ODBC.szServer, "");
    strcpy(ODBC.szDatabase, "");
 
    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "ODBC Driver: Closed.");

    /* Return result code to caller.
    */
    return(SDD_OK);
}

/******************************************************************************
 * Function:    odbc_ProcessRequest
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
int    odbc_ProcessRequest( UCHAR    *snzDataBuf,           /* I: Input data */
                            int      nDataLen,              /* I: Len of data */
                            int      (*fSendDataCB)(UCHAR *, UINT),
                                                            /* I: CB to send reply*/
                            UCHAR    *szErrMsg )            /* O: Error text */
{
    /* Local variables.
    */
    int          nReturn = SDD_OK;
    UCHAR        *szFunc = "odbc_ProcessRequest";

    /* If the request block doesnt contain any data, something went wrong
     * somewhere??
    */
    if(nDataLen < 1)
    {
        sprintf(szErrMsg,
                "%s: Illegal request, has size of %d Bytes",
                SDD_EMSG_BADREQ, nDataLen);
        return(SDD_FAIL);
    }

    /* First byte of the request data block indicates actions required, so
     * decipher it.
    */
    switch(snzDataBuf[0])
    {
        /* Request to execute a block of SQL code. SQL immediately follows the
         * command byte in the input buffer.
        */
        case SDD_RUNSQL:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc, "ODBC Driver: Requested to execute SQL");

            /* Execute the SQL.
            */
            nReturn=_ODBC_RunSql(&snzDataBuf[1],nDataLen,fSendDataCB,szErrMsg);
            break;

        case SDD_LIST_DB:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "ODBC Driver: Requested to list all Databases in data source");

            /* Call function to extract names of all databases in data source.
            */
            nReturn=_ODBC_ListDB(fSendDataCB, szErrMsg);
            break;

        case SDD_LIST_TABLES:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "ODBC Driver: Requested to list all tables for a database");

            /* Call function to extract names of all tables in a database.
            */
            nReturn=_ODBC_ListTables(&snzDataBuf[1], nDataLen, fSendDataCB,
                                     szErrMsg);
            break;

        case SDD_LIST_COLS:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "ODBC Driver: Requested to list all columns for a table");

            /* Call function to extract details of all columns in a table.
            */
            nReturn=_ODBC_ListCols(&snzDataBuf[1], nDataLen, fSendDataCB,
                                   szErrMsg);
            break;
 
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
 * Function:    odbc_ProcessOOB
 * Description: Entry point into driver to process an out of band command
 *              that may or may not be relevant to current state of
 *              operation. The task of this function is to decipher the
 *              command and act on it immediately, ie. a cancel command
 *              would abort any ProcessRequest that is in process and
 *              clean up.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void    odbc_ProcessOOB( UCHAR    nCommand    )    /* I: OOB Command */
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

        default:
            break;
    }

    /* Return to caller.
    */
    return;
}
