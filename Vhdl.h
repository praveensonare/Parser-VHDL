// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************

#ifndef VHDL_H
#define VHDL_H

#include "Verific"
#include "VhdlLib.h"
#include "VhdlUtils.h"

#include "UString"
#include "UStack"
#include "UControl"
#include "UHashBin"

struct VhdlLibKey {
  const UString& operator()(const VhdlLib* l) const {return l->name();}
};

class Vhdl
{
 public:
  typedef UHashList<VhdlLib*, UString, VhdlLibKey>     VhdlLibHash;
  typedef UHashSet<Verific::VhdlTypeId*>               BitTypes;
  typedef UStack<void*>                                HookStack;
  typedef UHashMap<Verific::VhdlTreeNode*, HookStack*> VhdlHooks;
  typedef UHashSet<Verific::VhdlIdDef*>                RamSet;
  typedef UHashMapList<Verific::VhdlTreeNode*, int>    NodeIntMap;
  typedef UHashMapMap<Verific::VhdlTreeNode*, UString, Verific::VhdlIdDef*> ReverseEnumMap;
 public:
  Vhdl();
  ~Vhdl();
  static void init(UMemPool* = NULL);
  static bool initPredefinedLibraries(bool);
  static void exit();

  static void reset();

  static int registerVhdl(const char *);
  static int parseVhdl(UControl&);

  static void writeObfuscated(const char*, const UHashSet<UString>&, UHashMapList<UString,UString>&);

  static void setVhdl87(bool b)                   {s_vhdl87 = b;}
  static void setVhdl2008(bool b)                 {s_vhdl2008 = b;}
  static void setVhdlLibrary(const char *);
  static void clearVhdlLibrary();
  static VhdlLib *workLib()                       {return s_vhdlLibrary;}

  static void initLibraries();

  static const VhdlLibHash& libraries()           {return s_libs;}

  static void initMemPool();
  static void exitMemPool();

  static void initVhdlMessages();

  static Verific::VhdlTreeNode *getDecl(Verific::VhdlIdDef *);
  static Verific::VhdlBindingIndication *getBinding(Verific::VhdlComponentInstantiationStatement *);
  static bool getPredefPack(Verific::VhdlTreeNode*, UString&);
  static bool getPredefPack(Verific::VhdlTreeNode*);

  static void setBitType(Verific::VhdlTypeId *);
  static bool isBitType(Verific::VhdlTypeId *);

  static void pushHook(Verific::VhdlTreeNode *, void *);
  static void *popHook(Verific::VhdlTreeNode *);
  static void *hook(Verific::VhdlTreeNode *);
  static void disHooks(Verific::VhdlTreeNode *);
  static void clearHooks();

  static void setRamVar(Verific::VhdlIdDef*);
  static void delRamVar(Verific::VhdlIdDef*);
  static bool isRamVar(Verific::VhdlIdDef*);

  static void addReverseEncoding(Verific::VhdlIdDef*, const UString&, Verific::VhdlIdDef*);
  static Verific::VhdlIdDef* reverseEncoding(Verific::VhdlIdDef*, const UString&);
  static void clearReverseEncodings();

#ifdef RT_FPGA
  static bool isNotRamVarHard(Verific::VhdlIdDef*) {return true;}
#endif

  static int  getNodeIndex(Verific::VhdlTreeNode*);

  static VhdlMod *findUnit(const char *);
  static VhdlMod *findUnit(Verific::VhdlName *);
  static VhdlMod *findUnit(const char *, Verific::VhdlIdDef*, const char * = NULL, const char * = NULL);

  static UStrBuf toLower(const char *);
  static UString signature(const char *, const char *);

  static Verific::VhdlIdDef* getStdId(const char *);

  static void dis();

  static UMemPool* memPool()                      {return s_vhdlPool;}   
  
 private:
  static VhdlLib *addVhdlLibrary(const char *);

  static bool        s_initialized;
  static bool        s_allocated;
  static Verific::vhdl_file *s_verificVhdl;
  static bool        s_vhdl87;
  static bool        s_vhdl2008;
  static VhdlLibHash s_libs;
  static UMemPool*   s_vhdlPool;
  static VhdlLib*    s_vhdlLibrary;
  static IdDeclMap   s_decls;
  static InstBindingMap s_bindings;
  static PredefNodeMap s_predefs;
  static StdIdMap    s_stdIds;
  static BitTypes    s_bitTypes;
  static VhdlHooks   s_hooks;
  static RamSet      s_ramVars;
  static NodeIntMap  s_nodeIndices;
  static ReverseEnumMap s_reverseEnums;

  friend class VhdlLib;
};

#endif
