#******************************************************************************
#* Product:     #     # #     #         #         ###   ######
#*              #     #  #   #          #          #    #     #
#*              #     #   # #           #          #    #     #
#*              #     #    #            #          #    ######
#*              #     #   # #           #          #    #     #
#*              #     #  #   #          #          #    #     #
#*               #####  #     # ####### #######   ###   ######
#*
#* File:        Makefile
#* Description: Build description file for UX lib.
#* Version:     %I%
#* Dated:       %D%
#* Copyright:   P.D. Smart, 1994-2019
#*
#* History:     1.0  - Initial Release.
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
TITLE          = "UniX Library"
COPYRIGHT      = "(C) P.D. Smart %D%, Version %I%"
PROJ           = 
PURIFY         = #purify
PROJPATH       = /users/bopropsm/ux
INCLUDEDIR     = -I.
IM_INCLUDE     = -I/usr/5include
1DEBUGFLAGS    = -g # -DUX_DEBUG #-E
4DEBUGFLAGS    = -g # -DUX_DEBUG #-E
5DEBUGFLAGS    = -g # -DUX_DEBUG #-E
1OPTIMIZEFLAGS = #-O2
4OPTIMIZEFLAGS = #-O2
5OPTIMIZEFLAGS = #-O2
1OPTIONFLAGS   = -D${OS} #-DUX_MONITOR #-ansi -Wall
4OPTIONFLAGS   = -D${OS} #-DUX_MONITOR #-ansi -Wall
5OPTIONFLAGS   = -D${OS} -D_REENTRANT 
CFLAGS         = $(${OSVER}DEBUGFLAGS) $(${OSVER}OPTIMIZEFLAGS) \
                 $(${OSVER}OPTIONFLAGS)
LDFLAGS        = -static
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
	@echo "Use 'build' command to make LIBUX library."

libux:      Begin \
            libux.a \
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

# Perform all cleanup operations to ensure future builds occur with
# completeness.
#
DoClean:
	rm -f *.o *.bak *.a *.BAK *.sav core

# Perform installation of software as per spec.
#
DoInstall:

# Build the UniX Library.
#
libux.a:	ux_cli.o ux_cmprs.o ux_comms.o ux_lgr.o ux_linkl.o \
		ux_mon.o ux_str.o ux_thrd.o
		$(AR) rcv libux.a \
		ux_cli.o ux_cmprs.o ux_comms.o ux_lgr.o \
		ux_linkl.o ux_mon.o ux_str.o ux_thrd.o

ux_cli.o:	ux_cli.c ux_comon.h ux_dtype.h ux_comms.h

ux_cmprs.o:	ux_cmprs.c

ux_comms.o:	ux_comms.c ux_comms.h ux_dtype.h ux_comon.h

ux_lgr.o:	ux_lgr.c ux_comon.h ux_dtype.h ux_comms.h

ux_linkl.o:	ux_linkl.c ux_comon.h ux_dtype.h ux_comms.h

ux_mon.o:	ux_mon.c ux_mon.h ux_comon.h ux_dtype.h ux_comms.h

ux_str.o:	ux_str.c ux_comon.h ux_dtype.h ux_comms.h

ux_thrd.o:	ux_thrd.c ux_comon.h ux_dtype.h ux_comms.h
