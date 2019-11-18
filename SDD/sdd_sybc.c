/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_sybc.c
 * Description:   Server Data-source Driver library driver to handle sybase
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
#define     SDD_SYBC_C

/* Bring in local specific header files.
*/
#include    "sdd.h"
#include    <sybfront.h>
#include    <sybdb.h>
#include    "sdd_sybc.h"

/******************************************************************************
 * Function:    _SYBC_GetArg
 * Description: Function to scan an input buffer and extract a required 
 *              argument from it.
 * 
 * Returns:     SDD_FAIL- Couldnt obtain argument.
 *              SDD_OK    - Argument obtained and validated.
 ******************************************************************************/
int    _SYBC_GetArg( UCHAR    *szArgType,     /* I: Type of Arg to scan for */
                     UCHAR    *snzDataBuf,    /* I: Input buffer */
                     int      nDataLen,       /* I: Len of data */
                     UCHAR    **pszArg )      /* O: Pointer to Arg */
{
    /* Local variables.
    */
    int          nCmpLen;
    int          nPos;
    int          nReturn = SDD_FAIL;
    UCHAR        *szFunc = "_SYBC_GetArg";

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
 * Function:    _SYBC_RunSql
 * Description: Function to execute a given buffer of SQL on the current
 *              database and return resultant data to the original caller.
 * 
 * Returns:     SDD_FAIL- SQL execution failed, see error message.
 *              SDD_OK    - SQL execution succeeded.
 ******************************************************************************/
int    _SYBC_RunSql( UCHAR    *snzDataBuf,               /* I: Input data */
                     int      nDataLen,                  /* I: Len of data */
                     int      (*fSendDataCB)(UCHAR *, UINT),
                                                         /* I: CB to send reply */
                     UCHAR    *szErrMsg )                /* O: Error text */
{
    /* Local variables.
    */
    DBINT       lResultBufLen;
    UINT        nFirstRow;
    UCHAR       *pszTmpBuf;
    UCHAR       *szFunc = "_SYBC_RunSql";

    /* Need to NULL terminate the buffer prior to executing it, so allocate
     * some memory and copy the original buffer into it.
    */
    if((pszTmpBuf=(UCHAR *)malloc(nDataLen+1)) == NULL)
    {
        sprintf(szErrMsg,
                "%s: Out of memory trying to exec SQL buffer",
                SDD_EMSG_MEMORY);
        return(SDD_FAIL);
    }
    memcpy(pszTmpBuf, snzDataBuf, nDataLen);
    pszTmpBuf[nDataLen] = '\0';

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "SQL: %s", pszTmpBuf);

    /* Load SQL into sybase's SQL buffer.
    */
    if(dbcmd(SYB.dbProc, pszTmpBuf) == FAIL)
    {
        /* Free up memory as it can no longer be used.
        */
        free(pszTmpBuf);
        sprintf(szErrMsg,
                "%s: Couldnt load SQL into sybase exec buffer",
                SDD_EMSG_SQLLOAD);
        return(SDD_FAIL);
    }

    /* Free up memory, no longer needed.
    */
    free(pszTmpBuf);

    /* Execute the SQL on the server, paying attention to the results.
    */
    if(dbsqlexec(SYB.dbProc) == FAIL)
    {
        sprintf(szErrMsg,
                "%s: Couldnt execute SQL due to syntax errors",
                SDD_EMSG_SQLSYNTAX);
        return(SDD_FAIL);
    }

    /* Initiate sybase into the return-us-results mode, trapping any errors.
    */
    if(dbresults(SYB.dbProc) == FAIL)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build an error message and exit.
        */
        sprintf(szErrMsg,
                "%s: Runtime error during SQL execution",
                SDD_EMSG_SQLRUNERR);
        return(SDD_FAIL);
    }

    /* Initialise any required variables.
    */
    SYB.nAbortPending = FALSE;

    /* Loop until we have exhausted the number of rows or an ABORT condition
     * occurs.
    */
    do {
        /* Initialise any loop variables.
        */
        nFirstRow = TRUE;

        /* Work out the maximum size of a memory buffer we need to hold returned
         * results.
        */
        lResultBufLen=dbspr1rowlen(SYB.dbProc);

        /* Allocate memory to hold returned results.
        */
        if((pszTmpBuf=(UCHAR *)malloc(lResultBufLen+1)) == NULL)
        {
            /* Cancel the query to avoid future results-pending errors.
            */
            dbcancel(SYB.dbProc);

            /* Build error message for return.
            */
            sprintf(szErrMsg,
                    "%s: Out of memory trying to alloc result buffer",
                    SDD_EMSG_MEMORY);
            return(SDD_FAIL);
        }

        do {
            /* If this is the first row then get the column header, else get
             * the actual rows.
            */
            if( nFirstRow == TRUE )
            {
                /* Fetch a list of all the columns we are returning.
                */
                if(dbsprhead(SYB.dbProc, pszTmpBuf, lResultBufLen) == FAIL)
                {
                    /* Cancel the query to avoid future results-pending errors.
                    */
                    dbcancel(SYB.dbProc);

                    /* Free up memory used.
                    */
                    free(pszTmpBuf);

                    /* Build error message for return.
                    */
                    sprintf(szErrMsg,
                            "%s: Error in fetching row header from server",
                            SDD_EMSG_FETCHHEAD);
                    return(SDD_FAIL);
                }

                /* Update flag so we dont do this again.
                */
                nFirstRow = FALSE;
            } else
             {
                /* Fetch one row of data from the server.
                */
                if(dbspr1row(SYB.dbProc, pszTmpBuf, lResultBufLen) == FAIL)
                {
                    /* Cancel the query to avoid future results-pending errors.
                    */
                    dbcancel(SYB.dbProc);

                    /* Free up memory used.
                    */
                    free(pszTmpBuf);

                    /* Build error message for return.
                    */
                    sprintf(szErrMsg,
                            "%s: Error in fetching a result row from server",
                            SDD_EMSG_FETCHROW);
                    return(SDD_FAIL);
                }
            }

            /* Log row if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc, "%s", pszTmpBuf);

            /* Call function to transmit row to original caller.
            */
            if(fSendDataCB(pszTmpBuf, strlen(pszTmpBuf)) == SDD_FAIL)
            {
                /* Cancel the query to avoid future results-pending errors.
                */
                dbcancel(SYB.dbProc);

                /* Free up memory used.
                */
                free(pszTmpBuf);

                /* Build error message for return.
                */
                sprintf(szErrMsg,
                        "%s: Failed to send row back to client",
                        SDD_EMSG_SENDROW);
                return(SDD_FAIL);
            }
        } while( (SYB.nAbortPending == FALSE) && 
                 (dbnextrow(SYB.dbProc) != NO_MORE_ROWS) );

        /* Free up memory used.
        */
        free(pszTmpBuf);
    } while(SYB.nAbortPending == FALSE && 
            dbresults(SYB.dbProc) != NO_MORE_RESULTS);

    /* If an abort command arrived halfway through processing then tidy up
     * and exit.
    */
    if(SYB.nAbortPending == TRUE)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: ABORT received, query abandoned",
                SDD_EMSG_ABORTRCVD);
        return(SDD_FAIL);
    }

    /* Return success to caller as everything worked.
    */
    return(SDD_OK);
}

/******************************************************************************
 * Function:    sybc_InitService
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
int    sybc_InitService( SERVICEDETAILS   *sServiceDet,    /* I: Init data */
                         UCHAR            *szErrStr )      /* O: Error message */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "sybc_InitService";

    /* Copy all configuration data out of the service structure.
    */
    strcpy(SYB.szUserName, sServiceDet->uServiceInfo.sSybaseInfo.szUser);
    strcpy(SYB.szPassword, sServiceDet->uServiceInfo.sSybaseInfo.szPassword);
    strcpy(SYB.szServer,   sServiceDet->uServiceInfo.sSybaseInfo.szServer);
    strcpy(SYB.szDatabase, sServiceDet->uServiceInfo.sSybaseInfo.szDatabase);

    /* Initialise the sybase db library.
    */
    if( dbinit() == FAIL )
    {
        sprintf(szErrStr,
                "%s: Couldnt initialise DBLIBRARY",
                SDD_EMSG_SRCINIT);
        return(SDD_FAIL);
    }

    /* Initialise the login maintence record.
    */
    if( (SYB.sLoginRec=dblogin()) == (LOGINREC *)NULL )
    {
        sprintf(szErrStr,
                "%s: Couldnt initialise Login Maintenance Record",
                SDD_EMSG_SRCINIT);
        return(SDD_FAIL);
    }

    /* Setup options and user id info within the login record.
    */
    DBSETLUSER(SYB.sLoginRec, SYB.szUserName);
    DBSETLPWD(SYB.sLoginRec,  SYB.szPassword);
    DBSETLAPP(SYB.sLoginRec,  DEF_APPNAME);

    /* Now perform the actual login to the server.
    */
    SYB.dbProc = dbopen(SYB.sLoginRec, SYB.szServer);
    if( SYB.dbProc == (DBPROCESS *)NULL )
    {
        sprintf(szErrStr,
                "%s: Couldnt login to named data server (%s)",
                SDD_EMSG_BADLOGIN, SYB.szServer);
        return(SDD_FAIL);
    }

    /* Setup options on the database specific for this modules usage.
    */
    dbsetopt(SYB.dbProc, DBPRCOLSEP, DEF_COLSEP, 1);
    dbsetopt(SYB.dbProc, DBPRLINELEN, "1024", 4);
    dbsetopt(SYB.dbProc, DBPRPAD, DEF_PADCHR, DBPADON);

    /* OK, server connection is valid, so lets connect to the required db.
    */
    if( dbuse(SYB.dbProc, SYB.szDatabase) == FAIL )
    {
        sprintf(szErrStr,
                "%s: Couldnt open named database (%s)",
                SDD_EMSG_BADDBASE, SYB.szDatabase);
        return(SDD_FAIL);
    }

    /* Soak up any/all init results.
    */
    while(dbresults(SYB.dbProc) != NO_MORE_RESULTS)
    {
        while(dbnextrow(SYB.dbProc) != NO_MORE_ROWS);
    }

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc,
        "Sybase Driver: Initialised: (User=%s, Pwd=<hidden>, Srv=%s, DB=%s)",
        SYB.szUserName, SYB.szServer, SYB.szDatabase);

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SYBC_ListDB
 * Description: Function to list all the names of databases available on the
 *              currently open data source.
 * 
 * Returns:     SDD_FAIL- SQL execution failed, see error message.
 *              SDD_OK    - SQL execution succeeded.
 ******************************************************************************/
int    _SYBC_ListDB( int      (*fSendDataCB)(UCHAR *, UINT),
                                                         /* I: CB to send reply */
                     UCHAR    *szErrMsg )                /* O: Error text */
{
    /* Local variables.
    */
    int         nReturn = SDD_OK;
    ULNG        lResultBufLen;
    UCHAR       szDbName[MAX_SYBC_DBNAME];
    UCHAR       *pszTmpBuf;
    UCHAR       *szFunc = "_SYBC_ListDB";

    /* Load SQL into sybase's SQL buffer which requests a list of all known
     * databases.
    */
    if(dbcmd(SYB.dbProc, "sp_databases") == FAIL)
    {
        /* Build error message to indicate problem.
        */
        sprintf(szErrMsg,
                "%s: Couldnt issue request to Sybase", SDD_EMSG_SQLLOAD);
        return(SDD_FAIL);
    }

    /* Execute the SQL on the server, paying attention to the results.
    */
    if(dbsqlexec(SYB.dbProc) == FAIL)
    {
        sprintf(szErrMsg,
                "%s: Issued request to sybase generated syntax errors",
                SDD_EMSG_SQLSYNTAX);
        return(SDD_FAIL);
    }

    /* Initiate sybase into the return-us-results mode, trapping any errors.
    */
    if(dbresults(SYB.dbProc) == FAIL)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build an error message and exit.
        */
        sprintf(szErrMsg, "%s: Runtime error during request execution",
                SDD_EMSG_SQLRUNERR);
        return(SDD_FAIL);
    }

    /* Initialise any required variables.
    */
    SYB.nAbortPending = FALSE;

    /* Work out the maximum size of a memory buffer we need to hold returned
     * results.
    */
    lResultBufLen=dbspr1rowlen(SYB.dbProc);

    /* Allocate memory to hold returned results.
    */
    if((pszTmpBuf=(UCHAR *)malloc(lResultBufLen+1)) == NULL)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: Out of memory trying to alloc result buffer",
                SDD_EMSG_MEMORY);
        return(SDD_FAIL);
    }

    /* Loop until we have exhausted the number of result sets or an ABORT
     * condition occurs.
    */
    do {
        /* Bind to relevant return columns.
        */
        dbbind(SYB.dbProc, 1, NTBSTRINGBIND, (DBINT)0, szDbName);

        /* Loop to extract all rows returned.
        */
        while( (SYB.nAbortPending == FALSE) && 
                 (dbnextrow(SYB.dbProc) != NO_MORE_ROWS) )
        {
            /* Build up output line.
            */
            sprintf(pszTmpBuf, "%s", szDbName);

            /* Call function to transmit row to original caller.
            */
            if(fSendDataCB(pszTmpBuf, strlen(pszTmpBuf)) == SDD_FAIL)
            {
                /* Cancel the query to avoid future results-pending errors.
                */
                dbcancel(SYB.dbProc);

                /* Free up memory used.
                */
                free(pszTmpBuf);

                /* Build error message for return.
                */
                sprintf(szErrMsg,
                        "%s: Failed to send row back to client",
                        SDD_EMSG_SENDROW);
                return(SDD_FAIL);
            }
        }
    } while(SYB.nAbortPending == FALSE && 
            dbresults(SYB.dbProc) != NO_MORE_RESULTS);

    /* Free up memory used.
    */
    free(pszTmpBuf);

    /* If an abort command arrived halfway through processing then tidy up
     * and exit.
    */
    if(SYB.nAbortPending == TRUE)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: ABORT received, query abandoned",
                SDD_EMSG_ABORTRCVD);
        return(SDD_FAIL);
    }

    /* Return success/fail to caller depending on execution results.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SYBC_ListTables
 * Description: Function to list all names of tables in a given database
 *              (or current database if no database name given).
 * 
 * Returns:     SDD_FAIL- SQL execution failed, see error message.
 *              SDD_OK    - SQL execution succeeded.
 ******************************************************************************/
int    _SYBC_ListTables( UCHAR    *snzDataBuf,             /* I: Input data */
                         int      nDataLen,                /* I: Len of data */
                         int      (*fSendDataCB)(UCHAR *, UINT),
                                                           /* I: CB to send reply */
                         UCHAR    *szErrMsg )              /* O: Error text */
{
    /* Local variables.
    */
    int          nReturn = SDD_OK;
    UCHAR        *pszDbName;
    UCHAR        szTableName[MAX_SYBC_TABLENAME];
    UCHAR        szTableType[MAX_SYBC_TABLENAME];
    ULNG         lResultBufLen;
    UCHAR        *pszTmpBuf;
    UCHAR        *szFunc = "_SYBC_ListTables";

    /* See if the caller has provided the name of a new database. If HE hasnt
     * then assume current database.
    */
    if(_SYBC_GetArg(SYBC_DBNAME, snzDataBuf, nDataLen, &pszDbName) == SDD_FAIL)
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
                "%s: Sybase switch to database function not yet implemented",
                SDD_EMSG_NOTYI);
        return(SDD_FAIL);
    }

    /* Load SQL into sybase's SQL buffer which requests a list of all known
     * tables.
    */
    if(dbcmd(SYB.dbProc, "sp_tables") == FAIL)
    {
        /* Build error message to indicate problem.
        */
        sprintf(szErrMsg,
                "%s: Couldnt issue request to Sybase", SDD_EMSG_SQLLOAD);
        return(SDD_FAIL);
    }

    /* Execute the SQL on the server, paying attention to the results.
    */
    if(dbsqlexec(SYB.dbProc) == FAIL)
    {
        sprintf(szErrMsg,
                "%s: Issued request to sybase generated syntax errors",
                SDD_EMSG_SQLSYNTAX);
        return(SDD_FAIL);
    }

    /* Initiate sybase into the return-us-results mode, trapping any errors.
    */
    if(dbresults(SYB.dbProc) == FAIL)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build an error message and exit.
        */
        sprintf(szErrMsg, "%s: Runtime error during request execution",
                SDD_EMSG_SQLRUNERR);
        return(SDD_FAIL);
    }

    /* Initialise any required variables.
    */
    SYB.nAbortPending = FALSE;

    /* Work out the maximum size of a memory buffer we need to hold returned
     * results.
    */
    lResultBufLen=dbspr1rowlen(SYB.dbProc);

    /* Allocate memory to hold returned results.
    */
    if((pszTmpBuf=(UCHAR *)malloc(lResultBufLen+1)) == NULL)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: Out of memory trying to alloc result buffer",
                SDD_EMSG_MEMORY);
        return(SDD_FAIL);
    }

    /* Loop until we have exhausted the number of result sets or an ABORT
     * condition occurs.
    */
    do {
        /* Bind to relevant return columns.
        */
        dbbind(SYB.dbProc, 3, NTBSTRINGBIND, (DBINT)0, szTableName);
        dbbind(SYB.dbProc, 4, NTBSTRINGBIND, (DBINT)0, szTableType);

        /* Loop to extract all rows returned.
        */
        while( (SYB.nAbortPending == FALSE) && 
                 (dbnextrow(SYB.dbProc) != NO_MORE_ROWS) )
        {
            /* Build up output line.
            */
            sprintf(pszTmpBuf, "%s%s%s", szTableName, DEF_COLSEP, szTableType);

            /* Call function to transmit row to original caller.
            */
            if(fSendDataCB(pszTmpBuf, strlen(pszTmpBuf)) == SDD_FAIL)
            {
                /* Cancel the query to avoid future results-pending errors.
                */
                dbcancel(SYB.dbProc);

                /* Free up memory used.
                */
                free(pszTmpBuf);

                /* Build error message for return.
                */
                sprintf(szErrMsg,
                        "%s: Failed to send row back to client",
                        SDD_EMSG_SENDROW);
                return(SDD_FAIL);
            }
        }
    } while(SYB.nAbortPending == FALSE && 
            dbresults(SYB.dbProc) != NO_MORE_RESULTS);

    /* Free up memory used.
    */
    free(pszTmpBuf);

    /* If an abort command arrived halfway through processing then tidy up
     * and exit.
    */
    if(SYB.nAbortPending == TRUE)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: ABORT received, query abandoned",
                SDD_EMSG_ABORTRCVD);
        return(SDD_FAIL);
    }

    /* Return success/fail to caller depending on execution results.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    _SYBC_ListCols
 * Description: Function to list all names and attributes of columns in a
 *              given table in a given database (or current database/table if
 *              no database name given).
 * 
 * Returns:     SDD_FAIL- SQL execution failed, see error message.
 *              SDD_OK    - SQL execution succeeded.
 ******************************************************************************/
int    _SYBC_ListCols( UCHAR    *snzDataBuf,               /* I: Input data */
                       int      nDataLen,                  /* I: Len of data */
                       int      (*fSendDataCB)(UCHAR *, UINT),
                                                           /* I: CB to send reply */
                       UCHAR    *szErrMsg )                /* O: Error text */
{
    /* Local variables.
    */
    int          nNullable;
    int          nRadix;
    int          nScale;
    int          nReturn = SDD_OK;
    UINT         nRowCount = 0;
    long         lLength;
    long         lPrecision;
    ULNG         lResultBufLen;
    UCHAR        szColumnName[MAX_SYBC_COLNAME];
    UCHAR        szTypeName[MAX_SYBC_TYPENAME];
    UCHAR        *pszColFilter;
    UCHAR        *pszDbName;
    UCHAR        *pszTableName;
    UCHAR        *pszTmpBuf;
    UCHAR        *szFunc = "_SYBC_ListCols";

    /* See if the caller has provided the name of a new database. If HE hasnt
     * then assume current database.
    */
    if(_SYBC_GetArg(SYBC_DBNAME, snzDataBuf, nDataLen, &pszDbName) == SDD_FAIL)
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
                "%s: Sybase switch to database function not yet implemented",
                SDD_EMSG_NOTYI);
        return(SDD_FAIL);
    }

    /* Caller must provide the name of a table... else what do we scan for...
     * a French Empire, vapourware... tell me more!!
    */
    if(_SYBC_GetArg(SYBC_TABLENAME, snzDataBuf, nDataLen, &pszTableName)
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
    if(_SYBC_GetArg(SYBC_COLFILTER,snzDataBuf,nDataLen,&pszColFilter)==SDD_FAIL)
    {
        /* Setup the variable so that the column filter logic below wont work
         * as no specific column name has been provided, hence the caller
         * wants all columns.
        */
        pszColFilter = NULL;
    } 

    /* Load SQL into sybase's SQL buffer which requests a list of all known
     * columns for a given table.
    */
    if(dbfcmd(SYB.dbProc, "sp_columns %s", pszTableName) == FAIL)
    {
        /* Build error message to indicate problem.
        */
        sprintf(szErrMsg,
                "%s: Couldnt issue request to list columns for %s", 
                SDD_EMSG_SQLLOAD, pszTableName);
        return(SDD_FAIL);
    }

    /* Execute the SQL on the server, paying attention to the results.
    */
    if(dbsqlexec(SYB.dbProc) == FAIL)
    {
        sprintf(szErrMsg,
                "%s: Issued request to sybase generated syntax errors",
                SDD_EMSG_SQLSYNTAX);
        return(SDD_FAIL);
    }

    /* Initiate sybase into the return-us-results mode, trapping any errors.
    */
    if(dbresults(SYB.dbProc) == FAIL)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build an error message and exit.
        */
        sprintf(szErrMsg, "%s: Runtime error during request execution",
                SDD_EMSG_SQLRUNERR);
        return(SDD_FAIL);
    }

    /* Initialise any required variables.
    */
    SYB.nAbortPending = FALSE;

    /* Work out the maximum size of a memory buffer we need to hold returned
     * results.
    */
    lResultBufLen=dbspr1rowlen(SYB.dbProc);

    /* Allocate memory to hold returned results.
    */
    if((pszTmpBuf=(UCHAR *)malloc(lResultBufLen+1)) == NULL)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: Out of memory trying to alloc result buffer",
                SDD_EMSG_MEMORY);
        return(SDD_FAIL);
    }

    /* Loop until we have exhausted the number of result sets or an ABORT
     * condition occurs.
    */
    do {
        /* Bind to relevant return columns.
        */
        dbbind(SYB.dbProc, 4, NTBSTRINGBIND, (DBINT)0, szColumnName);
        dbbind(SYB.dbProc, 6, NTBSTRINGBIND, (DBINT)0, szTypeName);
        dbbind(SYB.dbProc, 7, INTBIND, (DBINT)0, (BYTE *)&lPrecision);
        dbbind(SYB.dbProc, 8, INTBIND, (DBINT)0, (BYTE *)&lLength);
        dbbind(SYB.dbProc, 9, INTBIND, (DBINT)0, (BYTE *)&nScale);
        dbbind(SYB.dbProc, 10, INTBIND, (DBINT)0, (BYTE *)&nRadix);
        dbbind(SYB.dbProc, 11, INTBIND, (DBINT)0, (BYTE *)&nNullable);

        /* Loop to extract rows returned.
        */
        while( (SYB.nAbortPending == FALSE) && 
                 (dbnextrow(SYB.dbProc) != NO_MORE_ROWS) )
        {
            /* If there is any data to be transmitted the run through the
             * column filter if applicable and transmit it to the client.
            */
            if( (pszColFilter != NULL && strcmp(pszColFilter, szColumnName)==0)
                ||
                (pszColFilter == NULL) )
            {
                /* Place the appropriate data in the return buffer.
                */
                sprintf(pszTmpBuf,
                        "%s%s%s%s%ld%s%ld%s%d%s%d%s%d",
                        szColumnName,    DEF_COLSEP,
                        szTypeName,        DEF_COLSEP,
                        lPrecision,        DEF_COLSEP,
                        lLength,        DEF_COLSEP,
                        nScale,            DEF_COLSEP,
                        nRadix,            DEF_COLSEP,
                        nNullable);

                /* Call function to transmit row to original caller.
                */
                if(fSendDataCB(pszTmpBuf, strlen(pszTmpBuf)) == SDD_FAIL)
                {
                    /* Cancel the query to avoid future results-pending errors.
                    */
                    dbcancel(SYB.dbProc);

                    /* Free up memory used.
                    */
                    free(pszTmpBuf);

                    /* Build error message for return.
                    */
                    sprintf(szErrMsg,
                            "%s: Failed to send row back to client",
                            SDD_EMSG_SENDROW);
                    return(SDD_FAIL);
                } else
                 {
                    /* Increment row counter to indicate number of rows
                     * returned.
                    */
                    nRowCount++;
                }
            }
        }
    } while(SYB.nAbortPending == FALSE && 
            dbresults(SYB.dbProc) != NO_MORE_RESULTS);

    /* Free up memory used.
    */
    free(pszTmpBuf);

    /* If no rows where detected or returned then there was an error with 
     * the column, table or database name.
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
    if(SYB.nAbortPending == TRUE)
    {
        /* Cancel the query to avoid future results-pending errors.
        */
        dbcancel(SYB.dbProc);

        /* Build error message for return.
        */
        sprintf(szErrMsg,
                "%s: ABORT received, query abandoned",
                SDD_EMSG_ABORTRCVD);
        nReturn = SDD_FAIL;
    }

    /* Return success/fail to caller depending on execution results.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    sybc_CloseService
 * Description: Entry point which performs a drive closedown. The closedown
 *              procedure ensure that the driver returns to a virgin state
 *              (ie.like at power up) so that InitService can be called again.
 * 
 * Returns:     SDD_FAIL- An error occurred in closing the driver and an
 *                        error message is stored in szErrStr.
 *              SDD_OK    - Driver successfully closed.
 ******************************************************************************/
int    sybc_CloseService( UCHAR        *szErrMsg )    /* O: Error message if failed */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "sybc_CloseService";

    /* Shutdown any dbase connection.
    */
    dbexit();

    /* Tidy up variables.
    */
    SYB.nAbortPending = FALSE;
    strcpy(SYB.szUserName, "");
    strcpy(SYB.szPassword, "");
    strcpy(SYB.szServer, "");
    strcpy(SYB.szDatabase, "");

    /* Log if debugging switched on.
    */
    Lgr(LOG_MESSAGE, szFunc, "Sybase Driver: Closed.");

    /* Return result code to caller.
    */
    return(nReturn);
}

/******************************************************************************
 * Function:    sybc_ProcessRequest
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
int    sybc_ProcessRequest( UCHAR    *snzDataBuf,       /* I: Input data */
                            int      nDataLen,          /* I: Len of data */
                            int      (*fSendDataCB)(UCHAR *, UINT),
                                                        /* I: CB to send reply */
                            UCHAR    *szErrMsg )        /* O: Error text */
{
    /* Local variables.
    */
    int      nReturn = SDD_OK;
    UCHAR    *szFunc = "sybc_ProcessRequest";

    /* If the request block doesnt contain any data, something went wrong
     * somewhere??
    */
    if(nDataLen < 1)
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
        /* Request to execute a block of SQL code. SQL immediately follows the
         * command byte in the input buffer.
        */
        case SDD_RUNSQL:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Sybase Driver: Requested to execute SQL");

            /* Execute the SQL.
            */
            nReturn=_SYBC_RunSql(&snzDataBuf[1],nDataLen,fSendDataCB,szErrMsg);
            break;

        /* Request to list all the names of the databases on the current open
         * data source.
        */
        case SDD_LIST_DB:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Sybase Driver: Requested to list all Databases on source");

            /* Call function to extract names of all databases in data source.
            */
            nReturn=_SYBC_ListDB(fSendDataCB, szErrMsg);
            break;

        /* Request to list all the table names of a given database on the
         * current open data source.
        */
        case SDD_LIST_TABLES:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Sybase Driver: Requested to list all columns on a table");

            /* Call function to extract names of all tables in a database.
            */
            nReturn=_SYBC_ListTables(&snzDataBuf[1], nDataLen, fSendDataCB,
                                     szErrMsg);
            break;

        /* Request to list all the column names and their attributes of a
         * given database/table.
        */
        case SDD_LIST_COLS:
            /* Log if debugging switched on.
            */
            Lgr(LOG_DEBUG, szFunc,
                "Sybase Driver: Requested to list all tables in a database");

            /* Call function to extract details of all columns in a table.
            */
            nReturn=_SYBC_ListCols(&snzDataBuf[1], nDataLen, fSendDataCB,
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
 * Function:    sybc_ProcessOOB
 * Description: Entry point into driver to process an out of band command
 *              that may or may not be relevant to current state of
 *              operation. The task of this function is to decipher the
 *              command and act on it immediately, ie. a cancel command
 *              would abort any ProcessRequest that is in process and
 *              clean up.
 * 
 * Returns:     No returns.
 ******************************************************************************/
void    sybc_ProcessOOB( UCHAR    nCommand    )    /* I: OOB Command */
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
            SYB.nAbortPending = TRUE;
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
