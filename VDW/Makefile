#******************************************************************************
#* Product:       #     # ######  #     #
#*                #     # #     # #  #  #
#*                #     # #     # #  #  #
#*                #     # #     # #  #  #
#*                 #   #  #     # #  #  #
#*                  # #   #     # #  #  #
#*                   #    ######   ## ##
#*
#* File:          Makefile
#* Description:   Build description file for the Virtual Data Warehouse
#*                Daemon.
#* Version:       %I%
#* Dated:         %D%
#* Copyright:     P.D. Smart, 1996-2019.
#*
#* History:       1.0  - Initial Release.
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
TITLE          = "Virtual Data Warehouse Daemon"
COPYRIGHT      = "(C) P.D.Smart, %D%, Vers %I%"
PROJ           = 
PURIFY         = #purify -best-effort -follow-child-processes=yes
PROJPATH       = ../VDW
GNUINCLUDE     = #-I/apps/gnu/$(ARCH)/include
MDCINCLUDE     = -I../MDC
SDDINCLUDE     = -I../SDD
UXINCLUDE      = -I../ux
INCLUDEDIR     = -I. $(UXINCLUDE) $(MDCINCLUDE) $(SDDINCLUDE) $(GNUINCLUDE)
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
LDFLAGS        = #-static
MDCLIBS        = -L../MDC/${OSVER}lib -lmdc
ODBCLIBS       = -L../odbc/dlls -lodbc
SDDLIBS        = -L../SDD/${OSVER}lib -lsdd
1SYBLIBS       =
4SYBLIBS       = -L/apps/sybase/lib -lsybdb
5SYBLIBS       = -L/apps/sybase/lib -lsybdb
UXLIBS         = -L../ux/${OSVER}lib -lux
1LIBS          = -lm
4LIBS          = -lm
5LIBS          = -L/usr/ucblib -lsocket -lnsl -lucb #-liberty -lucb
LIBS           = $(MDCLIBS) $(SDDLIBS) $(UXLIBS) $(${OSVER}SYBLIBS) $(${OSVER}LIBS)
SCCSFLAGS      = -d$(PROJPATH)
SCCSGETFLAGS   =

ifeq ($(ZPU_BUILD),)
BASE           = 
else
BASE	       = zpu-elf-
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
			@echo "Use 'build' command to make Virtual DataWarehouse Daemon."

VDWD:		Begin \
			vdwd \
			End

# How to clean up the directory... make it look pretty!
#
clean:		Begin \
			DoClean \
			End

# How to perform an installation of the resultant software.
#
install:	Begin \
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

# Build the Virtual Data Warehouse Daemon Process.
#
vdwd:	vdwd.o config.o
		$(PURIFY) $(CC) $(LDFLAGS) -o vdwd \
		vdwd.o \
		config.o \
		$(LIBS)

vdwd.o:		vdwd.c vdwd.h

config.o:	config.c vdwd.h
