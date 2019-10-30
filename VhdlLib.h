// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************

#ifndef VHDLLIB_H
#define VHDLLIB_H

#include "Verific"
#include "UString"
#include "VhdlMod.h"


class VhdlLib
{
 public:
  typedef UHashMapList<UString, VhdlMod*> VhdlModHashList;
 public:
  VhdlLib(const UString &);
  ~VhdlLib();

  const UString&   name() const                    {return d_name;}
  void init(Verific::VhdlLibrary *);
  VhdlMod* findModule(const UString&) const;
  VhdlMod* findModule(const UString&, const UString&) const;
  const VhdlModHashList& modules() const           {return d_mods;}
  void remove(VhdlMod*);

  void reset();

  static void initPackage(Verific::VhdlPrimaryUnit*);

  void dis();

 private:
  Verific::VhdlLibrary *d_lib;
  UString          d_name;
  VhdlModHashList  d_mods;
};

#endif
