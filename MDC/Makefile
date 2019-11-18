#******************************************************************************
#* Product:       #     # ######   #####          #         ###   ######
#*                ##   ## #     # #     #         #          #    #     #
#*                # # # # #     # #               #          #    #     #
#*                #  #  # #     # #               #          #    ######
#*                #     # #     # #               #          #    #     #
#*                #     # #     # #     #         #          #    #     #
#*                #     # ######   #####  ####### #######   ###   ######
#*
#* File:          Makefile
#* Description:   Build description file for the virtual data warehouse
#*                Meta Data Communications API library.
#* Version:       %I%
#* Dated:         %D%
#* Copyright:     P.D. Smart, 1996-2019.
#*
#* History:        1.0  - Initial Release.
#*
#******************************************************************************
#* This source file is free software: you can redistribute it and#or modify
#* it under the terms of the GNU General Public License as published
#* by the Free Software Foundation, either version 3 of the License, or
#* (at your option) any later version.
#*
#* This source file is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#* GNU General Public License for more details.
#*
#* You should have received a copy of the GNU General Public License
#* along with this program.  If not, see <http://www.gnu.org/licenses/>.
#******************************************************************************
TITLE          = "Meta Data Communications API Library"
COPYRIGHT      = "(C) P.D.Smart, %D%, Ver %I%"
PROJ           = 
PURIFY         = #purify
PROJPATH       = /dvlp/MDC_LIB
GNUINCLUDE     = #-I/apps/gnu/$(ARCH)/include
UXINCLUDE      = -I../ux
INCLUDEDIR     = -I. $(UXINCLUDE) $(GNUINCLUDE)
1DEBUGFLAGS    = -g #-DMDC_DEBUG #-E
4DEBUGFLAGS    = -g #-DMDC_DEBUG #-E
5DEBUGFLAGS    = -g #-DMDC_DEBUG #-E
1OPTIMIZEFLAGS = #-O2
4OPTIMIZEFLAGS = #-O2
5OPTIMIZEFLAGS = #-O2
1OPTIONFLAGS   = -D${OS} #-ansi -Wall
4OPTIONFLAGS   = -D${OS} #-ansi -Wall
5OPTIONFLAGS   = -D${OS} -D_REENTRANT #-ansi -Wall
CFLAGS         = $(${OSVER}DEBUGFLAGS) $(${OSVER}OPTIMIZEFLAGS) \
                 $(${OSVER}OPTIONFLAGS)
LDFLAGS        = -static
SYBLIBS        = -L/apps/sybase/lib -lsybdb
UXLIBS         = -L../ux/${OSVER}lib -lux
LIBS           = $(UXLIBS) -lm
SCCSFLAGS      = -d$(PROJPATH)
SCCSGETFLAGS   =

ifeq ($(ZPU_BUILD),)
BASE           = 
else
BASE           = zpu-elf-
endif

CC             = $(BASE)gcc
LD             = $(BASE)gcc
AS             = $(BASE)as
AR             = $(BASE)ar
CP             = $(BASE)objcopy
DUMP           = $(BASE)objdump    
RANLIB         = $(BASE)ranlib

# Suffixes where interested in for this project.
#
.SUFFIXES:
.SUFFIXES: .o .c .h

# Our way of making an object file.
#
.c.o:
	$(PURIFY) $(CC) $(INCLUDEDIR) $(CFLAGS) -c $<

# All, ie: all programs to be built
#
all:
	@echo $(TITLE)
	@echo $(COPYRIGHT)
	@echo
	@echo "Use 'build' command to make LIBMDC library."

libmdc:     Begin \
            libmdc.a \
            End

# How to clean up the directory... make it look pretty!
#
clean:      Begin \
            DoClean \
            End

# How to perform an installation of the resultant software.
#
install:    Begin \
            DoInstall \
            End

#
# Pre-make start sequence.
#
Begin:
	@echo $(TITLE)
	@echo $(COPYRIGHT)
	@echo
	@echo "Operation commencing @ `date`"
	@echo

#
# Post-make completion sequence.
#
End:
	@echo
	@echo "Completed @ `date`"

# Perform all cleanup operations to ensure future builds occur
# with completeness.
#
DoClean:
	rm -f *.o *.bak *.a *.BAK *.sav core 

# Perform installation of software as per spec.
#
DoInstall:

# Build the virtual data warehouse Meta Data Communications
# API library.
#
libmdc.a:	mdc_common.o mdc_server.o mdc_client.o
		$(AR) rcv libmdc.a mdc_common.o mdc_server.o mdc_client.o

mdc_client.o:	mdc.h mdc_common.h

mdc_server.o:	mdc.h mdc_common.h

mdc_common.o:	mdc.h mdc_common.h
