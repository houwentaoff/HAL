################################################################################
################################################################################
##
## Makefile -- makefile for ADI test application
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

  TOPDIR = ../..
  SUBDIR = adi/test
#  INSTALL_DIR = $(ROOTFS_TOP)/usr/local/bin

include $(TOPDIR)/env/make/Makefile.config

################################################################################
# source files
################################################################################

SOURCES += src/main.c
SOURCES += src/video.c

################################################################################
# define define user cc flags
################################################################################

CCFLAGS_USER += -O3 -DDEBUG
CCFLAGS_USER += -I $(TOPDIR)/legacy/build/include
CCFLAGS_USER += -I $(TOPDIR)/adi/include
CCFLAGS_USER += -I $(TOPDIR)/env/temp/headers

################################################################################
# define define user ld flags
################################################################################

LDFLAGS_USER += -lrt 
#LDFLAGS_USER += -ladi
LDFLAGS_USER += -L $(TOPDIR)/legacy/prebuild/imgproc
LDFLAGS_USER += -limg_algo_a5s -limg_dsp_a5s

################################################################################
# define library used
################################################################################

LIBRARIES = adi

################################################################################
# define target name
################################################################################

PROGRAM = adi_test

################################################################################
# define target install location
# if this was commented out, the target will be installed into the default 
# location:
#   $(ROOTFS_TOP)/usr/local/bin/
################################################################################

# INSTALL_BIN_DIR = 

################################################################################
# define user targets
################################################################################

default: install

clean: clean-objects clean-all

build:  build-objects build-program

install: build install-program install-debug-program

headers:

all: clean build install

include $(TOPDIR)/env/make/Makefile.rules

-include $(TOPDIR)/env/make/Makfile.dep

