// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************

#include "Vhdl.h"
#include "VhdlMsg.h"
#include <ctype.h>
#include "oasys_port.h"
#include "HdlParam.h"
#include "UFile"

using namespace Verific;

// Static variables

bool                Vhdl::s_initialized = false;
bool                Vhdl::s_allocated = false;
bool                Vhdl::s_vhdl87 = false;
bool                Vhdl::s_vhdl2008 = false;
Vhdl::VhdlLibHash   Vhdl::s_libs;
UMemPool*           Vhdl::s_vhdlPool = NULL;
VhdlLib*            Vhdl::s_vhdlLibrary = NULL;
vhdl_file*          Vhdl::s_verificVhdl = NULL;
IdDeclMap           Vhdl::s_decls;
InstBindingMap      Vhdl::s_bindings;
PredefNodeMap       Vhdl::s_predefs;
StdIdMap            Vhdl::s_stdIds;
Vhdl::BitTypes      Vhdl::s_bitTypes;
Vhdl::VhdlHooks     Vhdl::s_hooks;
Vhdl::RamSet        Vhdl::s_ramVars;
Vhdl::NodeIntMap    Vhdl::s_nodeIndices;
Vhdl::ReverseEnumMap Vhdl::s_reverseEnums;

static Vhdl vhdlGlobal;		// to control vhdl cleanup

Vhdl::Vhdl()
{
}

Vhdl::~Vhdl()
{
}

void Vhdl::init(UMemPool* pool)
{
  if (!s_initialized) {
#ifndef NO_MEMPOOL
    if (!pool) {
      s_vhdlPool = new UMemPool("vhdl");
      s_allocated = true;
    } else {
      s_vhdlPool = pool;
    }
#endif
    // In development mode we keep a separate verific mempool to avoid
    // errors for verific memory memory leaks.  Otherwise, we keep
    // verific in the general parsing/elaboration pool.
    initVhdlMessages();

    s_verificVhdl = new vhdl_file;

    s_decls.clear();
    s_bindings.clear();
    s_predefs.clear();
    s_stdIds.clear();
    s_bitTypes.clear();
    clearReverseEncodings();

    bool leftovers = false;
    for (VhdlHooks::iterator i = s_hooks.beginRandom(); i; i++) {
      HookStack *s = i.value();
      if (s) {
	for (HookStack::iterator j = s->top(); j; j++) {
	  leftovers = true;
	}
      }
    }
    uassert(!leftovers);

    s_hooks.clear();
    s_ramVars.clear();
    s_initialized = true;
  }
}

static void debugVdbMismatch(const char* libpath)
{
  UStrBuf path = libpath;
  path << "/std/standard.vdb";

  SaveRestore tmp(path.str(), 1);
  printf("Native sizeof(long) = %d\n", (int)sizeof(long));
  printf("Vdb    sizeof(long) = %d for %s\n", tmp.GetSizeofLong(), path.str());

  if (tmp.GetSizeofLong() > sizeof(long)) {
    printf("error:  size mismatch for %s\n", path.str());
  }

}

bool Vhdl::initPredefinedLibraries(bool checkConflict)
{
  UStrBuf path = getenv("RT_LIBPATH");
#ifndef RT_FPGA
  path << "/share/vdbs";
#else
  path << "/vdbs";
#endif
  if (s_vhdl2008) {
    path << "2008";
  } else {
    path << "1993";
  }

  const char* libPath = s_verificVhdl->GetDefaultLibraryPath();
  if (libPath) {
    if (checkConflict && 0 != strcmp(libPath, path.str())) {
      VHMSG(changedLanguage);
      uprintf("Old: %s\n", libPath);
      uprintf("New: %s\n", path.str());
      return false;
    }
    return true;
  }

  s_verificVhdl->SetDefaultLibraryPath(path);
  addVhdlLibrary("default");
  addVhdlLibrary("std");
  addVhdlLibrary("ieee");
  addVhdlLibrary("synopsys");
  addVhdlLibrary("arithmetic");
  addVhdlLibrary("dware");
  addVhdlLibrary("dw");
  addVhdlLibrary("dw01");
  addVhdlLibrary("dw02");

  if (UParam::debugVdbMismatch()) {
    debugVdbMismatch(s_verificVhdl->GetDefaultLibraryPath());
  }

  return true;
}

void Vhdl::exit()
{
  if (s_initialized) {
    s_nodeIndices.clear();
    clearHooks();
    s_verificVhdl->ClearDefaultLibraryPath();

    // When shutting down, we don't want to see messages about 
    // units being out of date.
    Message::SetMessageType("VHDL-1201", VERIFIC_IGNORE);
    for (VhdlLibHash::iterator i = s_libs.begin(); i; i++) {
      VhdlLib* lib = *i;
      UString nm = lib->name();
      delete lib;
      s_verificVhdl->DeleteLibrary(nm);
    }
    s_verificVhdl->DeleteLibrary("work");
    s_verificVhdl->RemoveAllUnits(1);
    s_libs.clear();
    delete s_verificVhdl;
    if (s_allocated) {
#ifndef NO_MEMPOOL
      delete s_vhdlPool;
#endif
      s_allocated = false;
    }
    s_vhdlPool = NULL;
    clearReverseEncodings();
    s_initialized = false;
  }
}

int Vhdl::registerVhdl(const char *file)
{
  init();
  if (!initPredefinedLibraries(true)) {
    return 1;
  }
  unsigned vhdlMode = vhdl_file::VHDL_93;
  if (s_vhdl2008) {
    vhdlMode = vhdl_file::VHDL_2008;
  } else if (s_vhdl87) {
    vhdlMode = vhdl_file::VHDL_93;
  }

  if (!vhdl_sort::RegisterFile(file, (str_t) s_vhdlLibrary->name(), vhdlMode)) {
    return 1;
  }

  return 0;
}

int Vhdl::parseVhdl(UControl& control)
{
  init();
  if (!initPredefinedLibraries(true)) {
    return 1;
  }
  int rsl = 0;

  UTask* vhdlTask = control.lock("Reading Vhdl", vhdl_sort::Size());

  unsigned sortMode = vhdl_sort::VHDL_93;
  if (UParam::vhdlSortCommandLine() || UParam::vhdl_sort_files()) {
    vhdl_sort::Sort();
  }
  FILE* fp = NULL;
  if (!UParam::vhdlSortDebug().empty()) {
    fp = UFile::open(UParam::vhdlSortDebug().str(), "a");
    if (!fp) {
      printf("ERROR: cannot open file '%s'\n", UParam::vhdlSortDebug().str());
    } else {
      fprintf(fp, "\n\n");
    }
  }
  unsigned vhdlMode = vhdl_file::VHDL_93;
  const char* option = "";
  if (s_vhdl2008) {
    vhdlMode = vhdl_file::VHDL_2008;
    option = "-vhdl2008 ";
  } else if (s_vhdl87) {
    vhdlMode = vhdl_file::VHDL_87;
    option = "-vhdl87 ";
  }
  const char *fileName;
  const char *library;
  for (int i = 0; vhdl_sort::FileAt(i, &fileName, &library, &sortMode); i++) {
    if (fp) {
      fprintf(fp, "    read_vhdl %s-library %s %s\n", option, library, fileName);
    }
    if (!s_verificVhdl->Analyze(fileName, library, vhdlMode)) {
      rsl = 1;
      break;
    }
    vhdlTask->incrProgress(1);
  }
  if (fp) {
    UFile::close(fp);
    fp = NULL;
  }
  vhdl_sort::ClearRegisteredFiles();
  delete vhdlTask;
  return rsl;
}

void Vhdl::writeObfuscated(const char* file, 
			   const UHashSet<UString>&,
			   UHashMapList<UString,UString>&)
{
  // not implemented
}

void Vhdl::setVhdlLibrary(const char *arg)
{
  init();
  if (!initPredefinedLibraries(false)) {
    return;
  }

  if (0 == strcasecmp(arg, "") ||
      0 == strcasecmp(arg, "work")) {
    arg = "default";
  }
  UStrBuf lc = arg;
  lc.lower();
  const char* lower = lc.str();
  
  VhdlLib *lib = NULL;
  const char *eq = strchr(lower, '=');
  if (eq) {
    int aliasLen = eq - lower;
    char *alias = new char [aliasLen+1];
    strncpy(alias, lower, aliasLen);
    alias[aliasLen] = '\0';
    int libLen = strlen(lower) - aliasLen - 1;
    char *libName = new char [libLen+1];
    strncpy(libName, eq+1, libLen);
    libName[libLen] = '\0';

    if (s_libs.find((UString) alias)) {
      VHMSG(libraryAlias, alias);
    } else {
      s_verificVhdl->SetLibraryAlias(alias, libName);
    }

    lib = s_libs.find((UString) libName);
    if (!lib) {
      lib = addVhdlLibrary((UString) libName);
    }

    delete [] libName;
    delete [] alias;
  } else {

    lib = s_libs.find((UString) lower);
    if (!lib) {
      lib = addVhdlLibrary((UString) lower);
    }
  }
  s_vhdlLibrary = lib;
}

VhdlLib *Vhdl::addVhdlLibrary(const char *l)
{
  VhdlLib *lib = new IN_POOL(s_vhdlPool) VhdlLib(l);
  s_libs.append(lib);
  return lib;
}

void Vhdl::clearVhdlLibrary()
{
  init();
  if (!initPredefinedLibraries(false)) {
    return;
  }

  setVhdlLibrary("default");
}

void Vhdl::reset()
{
  initLibraries();
  s_decls.clear();
  s_bindings.clear();
  s_predefs.clear();
  s_stdIds.clear();
  s_bitTypes.clear();
  s_hooks.clear();
  s_ramVars.clear();
  s_nodeIndices.clear();
}

void Vhdl::initLibraries()
{
  for (VhdlLibHash::iterator i = s_libs.begin(); i; i++) {
    VhdlLib* lib = *i;
    lib->reset();
    Verific::VhdlLibrary *vlib = s_verificVhdl->GetLibrary((str_t) lib->name());
    lib->init(vlib);
  }
}

VhdlTreeNode *Vhdl::getDecl(VhdlIdDef *id)
{
  return s_decls[id];
}

VhdlBindingIndication* Vhdl::getBinding(VhdlComponentInstantiationStatement *stmt)
{
  return s_bindings[stmt];
}

bool Vhdl::getPredefPack(VhdlTreeNode *node, UString &pack)
{
  if (!node) {
    return false;
  }
  if (node->GetClassId() == ID_VHDLIDREF) {
    node = ((VhdlIdRef*)node)->GetId();
  }
  PredefNodeMap::iterator i = s_predefs.find(node);
  if (i) {
    pack = i.value();
    return true;
  } else {
    pack = UString();
    return false;
  }
}

bool Vhdl::getPredefPack(VhdlTreeNode *node)
{
  UString dummy;
  return getPredefPack(node, dummy);
}

void Vhdl::setBitType(VhdlTypeId *id)
{
  s_bitTypes.add(id);
}

bool Vhdl::isBitType(VhdlTypeId *id)
{
  return s_bitTypes.find(id);
}

VhdlIdDef* Vhdl::getStdId(const char *s)
{
  StdIdMap::iterator it = s_stdIds.find(s);
  if (it) {
    return it.value();
  } else {
    return NULL;
  }
}

void Vhdl::pushHook(VhdlTreeNode *n, void *p)
{
  HookStack *stack = s_hooks[n];
  if (! stack) {
    stack = new HookStack;
    s_hooks[n] = stack;
  }
  stack->push(p);
}

void *Vhdl::popHook(VhdlTreeNode *n)
{
  HookStack *stack = s_hooks[n];
  uassert(stack);
  void *rsl = stack->top();
  stack->pop();
  if (stack->empty()) {
    s_hooks.remove(n);
    delete stack;
  }
  return rsl;
}

void *Vhdl::hook(VhdlTreeNode *n)
{
  HookStack *stack = s_hooks[n];
  if (!stack) {
    return NULL;
  }
  return stack->top();
}

void Vhdl::disHooks(VhdlTreeNode* n)
{
  printf("n = %p\n", n);
  HookStack *stack = s_hooks[n];
  if (!stack) {
    printf("  stack = NULL\n");
  } else {
    int idx = 1;
    for (HookStack::iterator it = stack->top(); it; it++, idx++) {
      printf("  %d: %p\n", idx, it.object());
    }
  }
}

void Vhdl::clearHooks()
{
  for (VhdlHooks::iterator i = s_hooks.beginRandom(); i; i++) {
    delete i.value();
  }
}

void Vhdl::setRamVar(VhdlIdDef* id)
{
  s_ramVars.add(id);
}

void Vhdl::delRamVar(VhdlIdDef* id)
{
  s_ramVars.remove(id);
}

bool Vhdl::isRamVar(VhdlIdDef* id)
{
  return s_ramVars.find(id);
}

void Vhdl::addReverseEncoding(VhdlIdDef* typeId, const UString& encoding, VhdlIdDef* enumId)
{
  // Don't overwrite existing value
  s_reverseEnums.add(typeId, encoding, enumId, false);
}

VhdlIdDef* Vhdl::reverseEncoding(VhdlIdDef* typeId, const UString& encoding)
{
  UHashMapList<UString, VhdlIdDef*>::iterator it = s_reverseEnums.find(typeId, encoding);
  if (it) {
    return it.value();
  } else {
    return NULL;
  }
}

void Vhdl::clearReverseEncodings()
{
  for (ReverseEnumMap::iterator it = s_reverseEnums.begin(); it; it++) {
    UHashMapList<UString, VhdlIdDef*>* val = it.value();
    delete val;
  }

  s_reverseEnums.clear();
}

int Vhdl::getNodeIndex(Verific::VhdlTreeNode* n)
{
  // maps any node to a unique small integer
  NodeIntMap::iterator i = s_nodeIndices.find(n);
  if (!i) {
    i = s_nodeIndices.append(n);
    i.value(s_nodeIndices.size());
  }
  return i.value();
}


VhdlMod *Vhdl::findUnit(const char *name)
{
  VhdlMod* mod = NULL;
  for (Vhdl::VhdlLibHash::iterator i = Vhdl::libraries().begin(); i; i++) {
    VhdlLib* l = *i;
    VhdlMod *found = l->findModule(name);
    if (found) {
      if (mod) {
	// ?? message
	uprintf("error: multiple modules found for '%s'\n", name);
	return mod;
      } else {
	mod = found;
      }
    }
  }
  return mod;
}

VhdlMod *Vhdl::findUnit(VhdlName *name)
{
  switch (name->GetClassId()) {
  case ID_VHDLENTITYID: {
    VhdlEntityId *entityId = static_cast<VhdlEntityId *>(name->GetId());
    VhdlLibrary *lib = entityId->GetDesignUnit()->GetOwningLib();
    return findUnit(lib->Name(), entityId, NULL);
  }
  case ID_VHDLARCHITECTUREID: {
    VhdlArchitectureId *archId = static_cast<VhdlArchitectureId *>(name->GetId());
    VhdlEntityId *entityId = dynamic_cast<VhdlEntityId *>(archId->GetEntity());
    VhdlLibrary *lib = entityId->GetDesignUnit()->GetOwningLib();
    return findUnit(lib->Name(), entityId, entityId->Name(), archId->Name());
  }
  case ID_VHDLCONFIGURATIONID: {
    VhdlConfigurationId* configId = static_cast<VhdlConfigurationId *>(name->GetId());
    VhdlLibrary *lib = configId->GetDesignUnit()->GetOwningLib();
    return findUnit(lib->Name(), configId);
  }
  default:
    uassert(0);
    break;
  }
  return NULL;
}

VhdlMod *Vhdl::findUnit(const char *lib, VhdlIdDef *id, const char* entity, const char *archName)
{
  if (!id && !entity) {
    return NULL;
  }

  VhdlMod* rsl = NULL;
  if (id && entity) {
    uassert(0 == strcasecmp(id->Name(), entity));
  }
  if (id && !entity) {
    entity = id->Name();
  }
      
  VhdlLib *l = NULL;
  if (lib) {
    l = s_libs.find(lib);
  }
  if (l) {
    rsl = l->findModule(entity, archName);
    if (rsl) {
      if (id && id != rsl->entityId() && id != rsl->configId()) {
	rsl = NULL;
      }
    }
  }
  return rsl;
}

UStrBuf Vhdl::toLower(const char *s)
{
  UStrBuf rsl;
  for (const char *cp = s; *cp; cp++) {
    rsl << (char) tolower(*cp);
  }
  return rsl;
}

UString Vhdl::signature(const char *entName, const char *archName)
{
  UStrBuf buf;

  if (archName && strlen(archName) != 0) {
    buf.sprint("%s__%s", entName, archName);
  } else {
    buf << entName;
  }
  return (UString) buf;
}

void Vhdl::dis()
{
  for (VhdlLibHash::iterator i = libraries().begin(); i; i++) {
    (*i)->dis();
  }
}
