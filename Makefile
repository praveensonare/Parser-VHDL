# ****************************************************************************
#             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
#                             All rights reserved.
# ****************************************************************************
SRCROOT := ../..
DIRNAME := vhdl
LIBNAME := vhdl
SRC     := Vhdl.cc VhdlLib.cc VhdlMod.cc VhdlUtils.cc VhdlMessage.cc
#EXEC    := test_vhdl
#LIBS    := vhdl util mem
#LCLSRC  := main.cc
ENABLEGCOV := 1

ifdef TOPDOWN
  MYCXXFLAGS := -Ipkg/verific

  CUROBJDIR  := $(OBJDIR)/$(CURROOT)/$(DIRNAME)
  VHDLOBJS   := $(patsubst %.cc,$(CUROBJDIR)/%$(OBJSUFFIX),$(SRC))

  include Makefile.rec
else
  include $(SRCROOT)/Makefile.lcl
endif

