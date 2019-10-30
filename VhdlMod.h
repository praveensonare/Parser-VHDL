// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************

#ifndef VHDLMOD_H
#define VHDLMOD_H

#include "Verific"
#include "UString"
#include "UTrace"
#include "../HdlMod.h"

class VhdlLib;

class VhdlMod : public HdlMod
{
  typedef UList<VhdlMod*> ArchList;
 public:
  VhdlMod(VhdlLib *lib, Verific::VhdlConfigurationDecl *conf, Verific::VhdlEntityDecl *ent, Verific::VhdlArchitectureBody *arc = NULL);
  ~VhdlMod();

  const UString& name()                 const {return d_name;}
  const UString& signature()            const {return d_signature;}
  const UString& key()                  const {return d_key;}
  VhdlLib *library()                    const {return d_lib;}
  const UString& libName()              const;
  UStrBuf script()                      const;

  Verific::VhdlConfigurationDecl *config() const {return d_config;}
  Verific::VhdlConfigurationId *configId() const;
  Verific::VhdlEntityDecl *entity()     const {return d_entity;}
  Verific::VhdlEntityId* entityId()     const;
  Verific::VhdlArchitectureId* archId() const;
  Verific::VhdlArchitectureBody *arch() const {return d_arch;}
  const char* archName()                const {return d_arch ? d_arch->Name() : NULL;}

  VhdlMod* synthArch()                  const;
  VhdlMod* defaultArch()                const;
  ArchList &architectures()                   {return d_archs;}

  void setParent(VhdlMod *m)                  {d_parentMod = m;}
  VhdlMod *getParent()                  const {return d_parentMod;}

  const UTrace& trace()                 const {return d_trace;}
  const UTrace& otherTraces()           const {return d_otherTraces;}

  bool top()                            const {return d_top;}
  void setTop(bool t)                         {d_top = t;}

  void unlink();
  void flush();
  void cleanArch(Verific::VhdlArchitectureBody*);

  void dis();
 private:
  UString d_name;
  UString d_key;
  UString d_signature;
  VhdlLib *d_lib;
  Verific::VhdlConfigurationDecl *d_config;
  Verific::VhdlEntityDecl *d_entity;
  Verific::VhdlArchitectureBody *d_arch;
  ArchList d_archs;
  VhdlMod *d_parentMod;
  bool d_top;
  UTrace d_trace;
  UTrace d_otherTraces;
};

#endif
