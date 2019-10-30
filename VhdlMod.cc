// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************

#include "Vhdl.h"
#include "VhdlMod.h"
#include "VhdlUtils.h"
#include "VhdlLib.h"

using namespace Verific;

VhdlMod::VhdlMod(VhdlLib *lib, VhdlConfigurationDecl *conf, VhdlEntityDecl *ent, VhdlArchitectureBody *arc)
  : HdlMod(""), d_lib(lib), d_config(conf), d_entity(ent), d_arch(arc), d_archs(), d_parentMod(this), d_top(false),
    d_trace(), d_otherTraces()
{
  if (d_config) {
    d_name = getName(d_config->Id());
    d_signature = Vhdl::signature(d_name, NULL);
  } else {
    d_name = getName(d_entity->Id());
    d_signature = Vhdl::signature(d_name, arc ? getName(arc->Id()) : NULL);
  }
  setModuleName(d_signature); 
  d_key = Vhdl::toLower(d_signature);

  // Important trace is architecture body, then entity, then config.
  // otherTraces contains traces of used packages from all of them.

  VhdlDesignUnitSet units;
  IntHashList files;
  if (arch()) {
    d_trace.add(getTrace(arch()));
    getOtherTraces(arch(), units, files, false);
  }

  d_trace.add(getTrace(d_entity));
  getOtherTraces(d_entity, units, files, false);

  if (d_config) {
    d_trace.add(getTrace(d_config));
    getOtherTraces(d_config, units, files, false);
  }
  d_otherTraces.add(getOtherTraces(files));
}

VhdlMod::~VhdlMod()
{
  d_lib->remove(this);
}

VhdlMod *VhdlMod::synthArch() const
{
  if (d_arch) {
    return const_cast<VhdlMod*>(this);
  }
  return defaultArch();
}

VhdlMod* VhdlMod::defaultArch() const
{
  int num = entity()->NumOfSecondaryUnits();
  for (int i = num - 1; i >= 0; i--) {
    VhdlSecondaryUnit *arc = entity()->SecondaryUnitByIndex(i);
    if (arc) {
      UString sig = Vhdl::signature(getParent()->key(), getName(arc->Id()));
      return library()->findModule(sig);
    }
  }
  return const_cast<VhdlMod*>(this);
}

VhdlEntityId* VhdlMod::entityId() const
{
  VhdlEntityDecl *ent = entity();
  if (!ent) {
    return NULL;
  }
  VhdlIdDef* id = ent->Id();
  return static_cast<VhdlEntityId*>(id);
}

VhdlArchitectureId* VhdlMod::archId() const
{
  VhdlArchitectureBody *t = arch();
  if (!t) {
    return NULL;
  }
  VhdlIdDef* id = t->Id();
  return static_cast<VhdlArchitectureId*>(id);
}

VhdlConfigurationId* VhdlMod::configId() const
{
  VhdlConfigurationDecl *conf = config();
  if (!conf) {
    return NULL;
  }
  VhdlIdDef* id = conf->Id();
  return static_cast<VhdlConfigurationId*>(id);
}

void VhdlMod::unlink()
{
  d_entity = NULL;
  d_arch = NULL;
  d_config = NULL;
}

void VhdlMod::flush()
{
  // Don't flush architecture for a config, since several configs
  // (not to mention an entity) could share an architecture.
  if (!d_config) {
    cleanArch(d_arch);
  }
}

void VhdlMod::cleanArch(VhdlArchitectureBody* body)
{
  // Delete statements & declarations within architecture.  Leave
  // architecture itself since it is referenced in the
  // _secondary_units set of its entity.
  if (!body) {
    return;
  }

  uprintf("Flushing vhdl architecture: %s-%s\n", body->GetEntityName()->Name(), body->Name());

  Array* stmts = body->GetStatementPart();
  while (stmts && stmts->Size() > 0) {
    VhdlStatement* stmt = static_cast<VhdlStatement*>(stmts->RemoveLast());
    delete stmt;
  }

  Array* decls = body->GetDeclPart();
  while (decls && decls->Size() > 0) {
    VhdlDeclaration* decl = static_cast<VhdlDeclaration*>(decls->RemoveLast());
    delete decl;
  }
}

void VhdlMod::dis()
{
  uprintf("Unit: %s (%s)", (str_t) name(), (str_t) (signature()));
  if (d_top) {
    uprintf("  top");
  }
  uprintf("\n");
}

const UString& VhdlMod::libName() const 
{
  return library()->name();
}

UStrBuf VhdlMod::script() const
{
  UStrBuf buf;
  if (config() && config()->ScriptBuf()) {
    buf << *config()->ScriptBuf();
  }
  if (arch() && arch()->ScriptBuf()) {
    buf << *arch()->ScriptBuf();
  }
  if (entity() && entity()->ScriptBuf()) {
    buf << *entity()->ScriptBuf();
  }
  return buf;
}
