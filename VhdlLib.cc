// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************

#include "VhdlLib.h"
#include "Vhdl.h"

using namespace Verific;

VhdlLib::VhdlLib(const UString &n)
  : d_lib(NULL), d_name(n), d_mods()
{
}

VhdlLib::~VhdlLib()
{
  reset();
}

void VhdlLib::reset()
{
  VhdlModHashList::iterator i = d_mods.begin();
  while (i) {
    VhdlMod* mod = i++.value();
    delete mod;
  }
  d_lib = NULL;
}

void VhdlLib::init(Verific::VhdlLibrary *vlib)
{
  if (!vlib) {
    // error
    return;
  } else if (d_lib) {
    // already initialized
    return;
  }
  
  d_lib = vlib;

  MapIter mip, mis;
  VhdlPrimaryUnit *prim;
  VhdlSecondaryUnit *sec;
  FOREACH_VHDL_PRIMARY_UNIT(d_lib, mip, prim) {
    VhdlMod *ent = NULL;
    if (prim->GetClassId() == ID_VHDLCONFIGURATIONDECL) {
      //      uprintf("Adding configuration '%s' to library '%s'.\n",
      //      	      getName(prim->Id()), d_lib->Name());
      VhdlConfigurationDecl *conf = (VhdlConfigurationDecl*) prim;
      VhdlEntityId* entityId = static_cast<VhdlEntityId*>(conf->Id()->GetEntity());
      VhdlEntityDecl* entDecl = static_cast<VhdlEntityDecl*>(entityId->GetPrimaryUnit());
      VhdlBlockConfiguration *blockConfig = conf->GetBlockConfiguration();
      VhdlName *archName = blockConfig->GetBlockSpec();
      VhdlArchitectureBody *body = static_cast<VhdlArchitectureBody*>(entDecl->GetSecondaryUnit(archName->Name()));
      ent = new VhdlMod(this, conf, entDecl, body);
      d_mods[ent->key()] = ent;
    } else if (prim->GetClassId() == ID_VHDLENTITYDECL) {
      //      uprintf("Adding entity '%s' to library '%s'.\n",
      //      	      getName(prim->Id()), d_lib->Name());
      ent = new VhdlMod(this, NULL, (VhdlEntityDecl *) prim);
      d_mods[ent->key()] = ent;
    } else if (prim->GetClassId() == ID_VHDLPACKAGEDECL) {
      if (0 == strcmp(d_lib->Name(), "std") || 
	  0 == strcmp(d_lib->Name(), "ieee") ||
	  0 == strcmp(d_lib->Name(), "dw") ||
	  0 == strcmp(d_lib->Name(), "dw01") ||
	  0 == strcmp(d_lib->Name(), "dw02") ||
	  0 == strcmp(d_lib->Name(), "dware")) {
	mapPredefNodes(Vhdl::s_predefs, prim->Name(), prim);
      }
      if (0 == strcmp(d_lib->Name(), "std") &&
	  0 == strcmp(prim->Id()->Name(), "standard")) {
	mapStdIds(Vhdl::s_stdIds, prim);
      }
    }

    if (prim->GetClassId() == ID_VHDLPACKAGEDECL) {
      initPackage((VhdlPackageDecl*)prim);
    } else if (prim->GetClassId() == ID_VHDLPACKAGEINSTANTIATIONDECL) {
      initPackage((VhdlPackageInstantiationDecl*)prim);
    } else {
      
      //    uprintf("Primary unit %s:%s\n", d_lib->Name(), getName(prim->Id()));
      mapIdDecls(Vhdl::s_decls, prim);
      FOREACH_VHDL_SECONDARY_UNIT(prim, mis, sec) {    
	//      uprintf("  Secondary unit %s-%s\n", getName(prim->Id()), getName(sec->Id()));
	mapIdDecls(Vhdl::s_decls, sec);
	mapInstBindings(Vhdl::s_bindings, sec);
	mapMemories(sec);
	if (sec->GetClassId() == ID_VHDLARCHITECTUREBODY) {
	  uassert(prim->GetClassId() == ID_VHDLENTITYDECL);
	  VhdlMod *arch = new VhdlMod(this, NULL, (VhdlEntityDecl *) prim, (VhdlArchitectureBody *) sec);
	  d_mods[arch->key()] = arch;
	  ent->architectures().append(arch);
	  arch->setParent(ent);
	}
      }
    }
  }
}

void VhdlLib::initPackage(VhdlPrimaryUnit* pkg)
{
  mapIdDecls(Vhdl::s_decls, pkg);
  MapIter mi;
  VhdlSecondaryUnit* sec;
  FOREACH_VHDL_SECONDARY_UNIT(pkg, mi, sec) {    
    mapIdDecls(Vhdl::s_decls, sec);
  }
}

VhdlMod* VhdlLib::findModule(const UString& mod) const
{
  UString key = (UString) Vhdl::toLower((str_t)mod);
  VhdlModHashList::iterator i = d_mods.find(key);
  return i ? i.value() : NULL;
}


VhdlMod* VhdlLib::findModule(const UString& ent, const UString& arc) const
{
  UString key = (UString) Vhdl::toLower(Vhdl::signature(ent, arc));
  VhdlModHashList::iterator i = d_mods.find(key);
  return i ? i.value() : NULL;
}


void VhdlLib::dis()
{
  if (!d_lib) {
    uprintf("Uninitialized vhdl library '%s'.\n", (str_t) name());
  } else {
    uprintf("Vhdl library '%s':\n", d_lib->Name());
    for (VhdlModHashList::iterator i = d_mods.begin(); i; i++) {
      i.value()->dis();
    }
  }
}

void VhdlLib::remove(VhdlMod* m)
{
  if (!d_mods.find(m->key())) {
    printf("Could not find vhdl module '%s'\n", m->key().str());
  }
  d_mods.remove(m->key());
}
