/******************************************************************************
 * Product:        #####  ######  ######          #         ###   ######
 *                #     # #     # #     #         #          #    #     #
 *                #       #     # #     #         #          #    #     #
 *                 #####  #     # #     #         #          #    ######
 *                      # #     # #     #         #          #    #     #
 *                #     # #     # #     #         #          #    #     #
 *                 #####  ######  ######  ####### #######   ###   ######
 *
 * File:          sdd_aupl.h
 * Description:   Server Data-source Driver library driver header file for
 *                the audio player driver.
 * Version:       %I%
 * Dated:         %D%
 * Copyright:     P.D. Smart, 1996-2019.
 *
 * History:       1.0 - Initial Release.
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

/* Ensure file is only included once - avoid compile loops.
*/
#ifndef    SDD_AUPL_H
#define    SDD_AUPL_H

/* Definitions for constants, configurable options etc.
*/
#define    AUPL_AUDIOPATH            "PATH="
#define    AUPL_AUDIOFILE            "FILE="
#define    AUPL_DECOMPRESSOR         "/home/vdwd/bin/zcat"
#define    AUPL_AUDIO_PLAYER         "/home/vdwd/bin/play"

/* Definitions for maxims and defaults etc.
*/
#define    MAX_AUDIOPLAYER_LEN       sizeof(AUPL_AUDIO_PLAYER)
#define    MAX_DECOMPRESSOR_LEN      sizeof(AUPL_DECOMPRESSOR)

/* Structure for variables needed within this module.
*/
typedef struct {
    UCHAR    szAudioPlayer[MAX_AUDIOPLAYER_LEN+1];
    UCHAR    szDecompExec[MAX_DECOMPRESSOR_LEN+1];
} AUPL_DRIVER;

/* Allocate global variables by instantiating the above structure.
*/
static    AUPL_DRIVER        AUPL;

/* Prototypes of internal functions, not seen by any outside module.
*/
int    _AUPL_GetStrArg( UCHAR *, int, UCHAR *, UCHAR ** );
int    _AUPL_ValidatePath( UCHAR * );
int    _AUPL_ValidateFile( UCHAR *, UCHAR *, UINT );
#if defined(SOLARIS) || defined(SUNOS)
int    _AUPL_PlayZ(UCHAR *, UCHAR *, int(*)(UCHAR *, UINT), UCHAR * );
#endif

#endif    /* SDD_AUPL_H */
