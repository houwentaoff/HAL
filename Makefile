################################################################################
################################################################################
##
## Makefile -- makefile for ADI library
##
## (C) Copyright 2012-2014 by Goke Microelectronics Co.,Ltd
##
## Version: $Id$
##
## Mandatory settings:
##
## o TOPDIR      = the toplevel directory (using slashes as path separator)
## o TARGET      = the machine qualifier to generate code for
##
################################################################################
################################################################################

  TOPDIR = ..
  SUBDIR = adi

include $(TOPDIR)/env/make/Makefile.config

################################################################################
# source files
################################################################################

SOURCES += src/adi_ain.c
SOURCES += src/adi_aout.c
SOURCES += src/adi_cap.c
SOURCES += src/adi_fd.c
SOURCES += src/adi_gpio.c
SOURCES += src/adi_i2c.c
SOURCES += src/adi_image.c
SOURCES += src/adi_md.c
SOURCES += src/adi_osd.c
SOURCES += src/adi_pm.c
SOURCES += src/adi_priv.c
SOURCES += src/adi_pwm.c
SOURCES += src/adi_spi.c
SOURCES += src/adi_sys.c
SOURCES += src/adi_vdec.c
SOURCES += src/adi_venc.c
SOURCES += src/adi_vin.c
SOURCES += src/adi_vout.c
SOURCES += src/adi_ptz.c
SOURCES += src/adi_ptz_pelcod.c

################################################################################
# headers to be installed
################################################################################

HEADERS += include/*.h

################################################################################
# sub directory name for installed header
################################################################################

HEADER_SUBDIR = adi

################################################################################
# define library name
################################################################################

LIBRARY = libadi

################################################################################
# define user cc flags
################################################################################

CCFLAGS_USER += -O3 -DDEBUG
CCFLAGS_USER += -march=armv6k -mtune=arm1136j-s -msoft-float -mlittle-endian
#CCFLAGS_USER += -DNDEBUG
CCFLAGS_USER += -MMD

CCFLAGS_USER += -I ./include
CCFLAGS_USER += -I $(TOPDIR)/legacy/build/include
CCFLAGS_USER += -I $(TOPDIR)/legacy/build/include/arch
CCFLAGS_USER += -I $(TOPDIR)/env/temp/headers
CCFLAGS_USER += -I ./params

################################################################################
# define user lint flags
################################################################################

LINTFLAGS_USER +=

################################################################################
# define user ld flags
################################################################################

################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-library clean-all

build:  build-objects build-library

headers: install-headers

install: build install-library install-headers

all: clean build install

################################################################################
# include internal definitions and rules
################################################################################

  RELEASE_SUBDIR = $(SUBDIR)
  RELEASE_FILES += $(SOURCES)
  RELEASE_FILES += $(wildcard include/*.h)
  RELEASE_FILES += Makefile

release: doc release-files

doc:

include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep

