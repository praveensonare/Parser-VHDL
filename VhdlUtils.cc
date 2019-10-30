// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************
#include "VhdlUtils.h"
#include "VhdlMsg.h"
#include "Vhdl.h"
#include "SynParam.h"
#include "HdlParam.h"
#include <typeinfo>
#include <iostream>
#include "oasys_port.h"

using namespace Verific;


VhdlSubtypeIndication *getObjectSubtype(VhdlDeclaration *decl)
{
  if (!decl) {
    return NULL;
  }
  switch (decl->GetClassId()) {
  case ID_VHDLALIASDECL:
    return ((VhdlAliasDecl *) decl)->GetSubtypeIndication();
  case ID_VHDLATTRIBUTEDECL:
    return ((VhdlAttributeDecl *) decl)->GetTypeMark();
  case ID_VHDLCONSTANTDECL:
    return ((VhdlConstantDecl *) decl)->GetSubtypeIndication();
  case ID_VHDLFILEDECL:
    return ((VhdlFileDecl *) decl)->GetSubtypeIndication();
  case ID_VHDLINTERFACEDECL:
    return ((VhdlInterfaceDecl *) decl)->GetSubtypeIndication();
  case ID_VHDLSIGNALDECL:
    return ((VhdlSignalDecl *) decl)->GetSubtypeIndication();
  case ID_VHDLVARIABLEDECL:
    return ((VhdlVariableDecl *) decl)->GetSubtypeIndication();
  default:
    return NULL;
  }
}

VhdlSubtypeIndication *getObjectSubtype(VhdlElementDecl *decl)
{
  return decl->GetSubtypeIndication();
}

VhdlIdDef *getTypeId(VhdlName *e)
{
  switch (e->GetClassId()) {
  case ID_VHDLIDREF:
    return ((VhdlIdRef *) e)->GetId();
  case ID_VHDLSELECTEDNAME:
    return unExpand((VhdlSelectedName *)e);
  default:
    uassert(0);
    return NULL;
  }
}

static UTrace getVhdlTrace(linefile_type lf)
{
  if (!lf) {
    return UTrace();
  }
  unsigned int fileId = lf->GetFileId();
  int line = lf->GetLeftLine();
  int dline = lf->GetRightLine();
  // subtract 1 because verific is 1-based, oasys is 0-based
  int col = lf->GetLeftCol() - 1;
  int dcol = lf->GetRightCol();
  return UTrace(fileId, line, col, dline, dcol);
}

bool getOtherTraces(VhdlTreeNode *n, VhdlDesignUnitSet& units, IntHashList& files, bool includeMe)
{
  if (!n) {
    return false;
  }
  switch (n->GetClassId()) {
  case ID_VHDLENTITYDECL:
  case ID_VHDLCONFIGURATIONDECL:
  case ID_VHDLPACKAGEDECL:
  case ID_VHDLARCHITECTUREBODY:
  case ID_VHDLPACKAGEBODY: {
    VhdlDesignUnit* du = static_cast<VhdlDesignUnit*>(n);
    VhdlLibrary* lib = du->GetOwningLib();
    if (units.find(du)) {
      break;
    }
    units.add(du);
    if (lib && (0 == strcmp(lib->Name(), "std") || 0 == strcmp(lib->Name(), "ieee"))) {
      break;
    }

    if (includeMe) {
      UTrace t(getVhdlTrace(du));
      for (int j = 0; j < t.count(); j++) {
	files.append(t.fileNameID(j));
      }
    }

    VhdlScope* lc = du->LocalScope();
    Set* used = lc->GetUsing();
    SetIter si;
    VhdlScope* scope;
    FOREACH_SET_ITEM(used, si, &scope) {
      VhdlIdDef* usedId = scope->GetContainingDesignUnit();
      if (!usedId) {
	continue;
      }
      VhdlDesignUnit* usedUnit = usedId->GetDesignUnit();
      getOtherTraces(usedUnit, units, files, true);
    }
    if (du->GetClassId() == ID_VHDLPACKAGEDECL) {
      // pull in package body, if any
      VhdlPackageDecl* pd = static_cast<VhdlPackageDecl*>(du);
      VhdlSecondaryUnit* su = pd->GetSecondaryUnit(NULL);
      if (su) {
	getOtherTraces(su, units, files, true);
      }
    }
    int i;
    long fd;
    FOREACH_ARRAY_ITEM(du->GetOtherTraces(), i, fd) {
      files.append(fd);
    }
    break;
  }
  default:
    break;
  }
  return true;
}

UTrace getOtherTraces(IntHashList& files)
{
  UTrace t;
  for (IntHashList::iterator it = files.begin(); it; it++) {
    int fd = *it;
    t.add(UTrace(fd, 0, 0, 0, 0));
  }
  return t;
}

UTrace getVhdlTrace(VhdlTreeNode *n)
{
  UTrace t;
  if (!n) {
    return t;
  }
  t.add(getVhdlTrace(n->Linefile()));
  return t;
}

UTrace getTrace(VhdlTreeNode* n)
{
  return getVhdlTrace(n);
}

void copyLinefile(VhdlTreeNode *dest, VhdlTreeNode *src)
{
  dest->SetLinefile(src->Linefile());
}

bool isEscapedName(const char* n)
{
  return n[0] == '\\';
}

UStrBuf unEscapeName(const char* n)
{
  uassert(isEscapedName(n));
  UStrBuf buf = &n[1];
  buf.setTail(strlen(n) - 2);
  return buf;
}

const char* getName(VhdlIdDef* n)
{
  bool useOrig = UParam::vhdlDowncase() == false;

  return useOrig ? n->OrigName() : n->Name();
}


class IdDeclMapper : public VhdlVisitor
{
public:
  IdDeclMapper(IdDeclMap *map)
    : VhdlVisitor(), d_map(map)
  {
  }
  ~IdDeclMapper()
  {
  }
  void connect(VhdlDeclaration *decl, VhdlIdDef *id)
  {
    (*d_map)[id] = decl;
  }

  void connect(VhdlElementDecl *decl, VhdlIdDef *id)
  {
    (*d_map)[id] = decl;
  }

  void connect(VhdlPhysicalUnitDecl *decl, VhdlIdDef *id)
  {
    (*d_map)[id] = decl;
  }

  void doit(VhdlDeclaration *decl)
  {
    VhdlIdDef *id;

    switch (decl->GetClassId()) {
    case ID_VHDLFULLTYPEDECL:
      id = ((VhdlFullTypeDecl *) decl)->GetId();
      connect(decl, id);
      break;
    case ID_VHDLSUBTYPEDECL:
      id = ((VhdlSubtypeDecl *) decl)->GetId();
      connect(decl, id);
      break;
    case ID_VHDLALIASDECL:
      id = ((VhdlAliasDecl *) decl)->GetDesignator();
      connect(decl, id);
      break;
    case ID_VHDLATTRIBUTEDECL:
      id = ((VhdlAttributeDecl *) decl)->GetId();
      connect(decl, id);
      break;
    case ID_VHDLCOMPONENTDECL:
      // use compId->GetComponentDecl() instead
      break;
    case ID_VHDLINTERFACETYPEDECL:
      id = ((VhdlInterfaceTypeDecl *) decl)->GetId();
      connect(decl, id);
      break;
    default: {
      int i;
      FOREACH_ARRAY_ITEM(decl->GetIds(), i, id) {
	connect(decl, id);
      }
      break;
    }
    }
  }

  void doit(VhdlElementDecl *decl)
  {
    // strangely enough, a VhdlElementDecl is not derived from VhdlDeclaration
    int i;
    VhdlIdDef* id;
    FOREACH_ARRAY_ITEM(decl->GetIds(), i, id) {
      connect(decl, id);
    }
  }

  void doit(VhdlPhysicalUnitDecl *decl)
  {
    // strangely enough, a VhdlPhysicalUnitDecl is not derived from VhdlDeclaration
    connect(decl, decl->GetId());
  }


  void Visit(VhdlDeclaration &decl)
  {
    doit(&decl);
  }
  void Visit(VhdlElementDecl &decl)
  {
    doit(&decl);
  }
  void Visit(VhdlPhysicalUnitDecl &decl)
  {
    doit(&decl);
  }
  void Visit(VhdlUseClause &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlLibraryClause &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlInterfaceDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlSubprogramDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlSubprogramBody &decl)
  {
    TraverseNode(decl.GetSubprogramSpec()) ;
    TraverseArray(decl.GetDeclPart()) ;
    TraverseArray(decl.GetStatementPart()) ;
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlSubtypeDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlFullTypeDecl &decl)
  {
    TraverseNode(decl.GetTypeDef());
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlIncompleteTypeDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlConstantDecl &decl)
  {
   doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlSignalDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlVariableDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlFileDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlAliasDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlComponentDecl &decl)
  {
    TraverseArray(decl.GetGenericClause());
    TraverseArray(decl.GetPortClause());
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlAttributeDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlAttributeSpec &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlConfigurationSpec &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlDisconnectionSpec &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlGroupTemplateDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlGroupDecl &decl)
  {
    doit((VhdlDeclaration *) &decl);
  }
  void Visit(VhdlInterfaceTypeDecl &decl)
  {
    doit((VhdlInterfaceTypeDecl*) &decl);
  }
  void Visit(VhdlPackageInstantiationDecl &decl)
  {
    TraverseNode(decl.InitialPkg());
  }

private:
  IdDeclMap *d_map;
};


void mapIdDecls(IdDeclMap &map, VhdlTreeNode *n)
{
  IdDeclMapper mapper(&map);
  n->Accept(mapper);
}


class InstBindingMapper : public VhdlVisitor
{
public:
  InstBindingMapper(InstBindingMap *map)
    : VhdlVisitor(), d_map(map)
  {
  }
  ~InstBindingMapper()
  {
  }

  void link(Array *decls, Array *stmts)
  {
    typedef UHashMap<UString, VhdlComponentInstantiationStatement *> InstTable;
    InstTable table;
    int i;
    VhdlStatement *stmt;
    FOREACH_ARRAY_ITEM(stmts, i, stmt) {
      if (stmt->GetClassId() == ID_VHDLCOMPONENTINSTANTIATIONSTATEMENT) {
	VhdlComponentInstantiationStatement *inst = static_cast<VhdlComponentInstantiationStatement*>(stmt);
	VhdlIdDef *label = inst->GetLabel();
	table[label->Name()] = inst;
      }
    }
    int j;
    VhdlDeclaration *decl;
    FOREACH_ARRAY_ITEM(decls, j, decl) {
      if (decl->GetClassId() == ID_VHDLCONFIGURATIONSPEC) {
	VhdlConfigurationSpec *configSpec = static_cast<VhdlConfigurationSpec *>(decl);
	VhdlBindingIndication *binding = configSpec->GetBinding();
	VhdlComponentSpec *compSpec = static_cast<VhdlComponentSpec*>(configSpec->GetComponentSpec());
	VhdlIdDef *boundComponent = derefAliasId(compSpec->GetComponent());

	int k;
	VhdlName *id;
	FOREACH_ARRAY_ITEM(compSpec->GetIds(), k, id) {
	  id = derefAlias(id);
	  if (id->IsAll() || id->IsOthers()) {
	    for (InstTable::iterator it = table.beginRandom(); it; it++) {
	      VhdlComponentInstantiationStatement *inst = it.value();
	      VhdlIdDef *component = derefAliasId(inst->GetInstantiatedUnit());
	      if (component == boundComponent) {
		if ((*d_map)[inst]) {
		  uassert(id->IsOthers());	// verific should have caught this
		} else {
		  (*d_map)[inst] = binding;
		}
	      }
	    }
	  } else {
	    UString name = id->Name();
	    VhdlComponentInstantiationStatement *inst = table[name];
	    (*d_map)[inst] = binding;
	  }
	}
      }
    }
  }

  void Visit(VhdlArchitectureBody &body)
  {
    link(body.GetDeclPart(), body.GetStatementPart());
    TraverseArray(body.GetStatementPart());
  }

  void Visit(VhdlGenerateStatement &stmt)
  {
    link(stmt.GetDeclPart(), stmt.GetStatementPart());
    TraverseArray(stmt.GetStatementPart());
  }

  void Visit(VhdlBlockStatement &stmt)
  {
    link(stmt.GetDeclPart(), stmt.GetStatements());
    TraverseArray(stmt.GetStatements());
  }

private:
  InstBindingMap *d_map;
};

void mapInstBindings(InstBindingMap &map, VhdlTreeNode *n)
{
  InstBindingMapper mapper(&map);
  n->Accept(mapper);
}

class PredefNodeMapper : public VhdlVisitor
{
public:
  PredefNodeMapper(PredefNodeMap *map, const UString &packName)
    : VhdlVisitor(), d_map(map), d_packName(packName)
  {
  }
  ~PredefNodeMapper()
  {
  }

  void Visit(VhdlSpecification &spec)
  {
    (*d_map)[&spec] = d_packName;
  }

  void Visit(VhdlIdDef &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlAttributeId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlComponentId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlAliasId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlFileId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlVariableId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlSignalId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlConstantId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlTypeId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlSubtypeId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlSubprogramId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlOperatorId &id)
  {
    (*d_map)[&id] = d_packName;
  }

  void Visit(VhdlEnumerationId &id)
  {
    (*d_map)[&id] = d_packName;
  }

private:
  PredefNodeMap *d_map;
  UString        d_packName;
};

void mapPredefNodes(PredefNodeMap &map, const UString &packName, VhdlTreeNode *n)
{
  PredefNodeMapper mapper(&map, packName);
  n->Accept(mapper);
}


class StdIdMapper : public VhdlVisitor
{
public:
  StdIdMapper(StdIdMap *map)
    : VhdlVisitor(), d_map(map)
  {
  }

  void Visit(VhdlIdDef& id)
  {
    // to avoid conflict between bit and character '0' values, don't
    // store character literals
    if (id.Name()[0] != '\'') {
      d_map->append(id.Name()).value(&id);
    }
  }

  void Visit(VhdlOperatorId& id)
  {
    // do nothing
  }

  void Visit(VhdlSubprogramId& id)
  {
    // do nothing
  }

private:
  StdIdMap *d_map;
};

void mapStdIds(StdIdMap &map, VhdlTreeNode *n)
{
  StdIdMapper mapper(&map);
  n->Accept(mapper);
}

class VhdlMemoryVisitor : public VhdlVisitor
{
public:
  typedef UHashMap<VhdlIdDef*, int> CountTable;

  VhdlMemoryVisitor()
    : VhdlVisitor(), d_clocked(false), d_assign(false), d_constIdx(false), 
      d_idx(false), d_constAccesses(), d_varWrites(), d_targets()
  {
  }
  ~VhdlMemoryVisitor()
  {
  }

  void incrConstAccess(VhdlIdDef *id)
  {
    CountTable::iterator i = d_constAccesses.find(id);
    if (!i) {
      d_constAccesses[id] = 1;
    } else {
      i.value()++;
    }
  }

  void incrVarWrite(VhdlIdDef *id)
  {
    CountTable::iterator i = d_varWrites.find(id);
    if (!i) {
      d_varWrites[id] = 1;
    } else {
      i.value()++;
    }
  }

  void TraverseTarget(VhdlTreeNode *target)
  {
    uassert(!d_assign);
    d_assign = true;
    TraverseNode(target);
    d_assign = false;
  }

  void TraverseExpr(VhdlTreeNode *expr)
  {
    if (d_assign) {
      d_assign = false;
      TraverseNode(expr);
      d_assign = true;
    } else {
      TraverseNode(expr);
    }
  }

  void Visit(VhdlArchitectureBody& body)
  {
    TraverseArray(body.GetStatementPart());

    for (Vhdl::RamSet::iterator i = d_targets.beginRandom(); i; i++) {
      VhdlIdDef *id = i.object();
      Vhdl::setRamVar(id);
    }

    for (CountTable::iterator i = d_varWrites.beginRandom(); i; i++) {
      VhdlIdDef *id = i.object();
      int count = i.value();
      if (count >= UParam::inferMemoryIfVariableIndexWritesLessThan()) {
	Vhdl::delRamVar(id);
      }
    }

    for (CountTable::iterator i = d_constAccesses.beginRandom(); i; i++) {
      VhdlIdDef *id = i.object();
      if (!UParam::inferMemoryAllowConstantIndex()) {
	Vhdl::delRamVar(id);
      }
    }

    for (Vhdl::RamSet::iterator i = d_rejects.beginRandom(); i; i++) {
      VhdlIdDef *id = i.object();
      Vhdl::delRamVar(id);
    }
  }

  void Visit(VhdlProcessStatement& stmt)
  {
    int i;
    VhdlStatement *s;
    d_clocked = false;
    d_constIdx = false;
    d_idx = false;
    FOREACH_ARRAY_ITEM(stmt.GetStatementPart(), i, s) {
      if (!s) {
	continue;
      }
      if (s->GetClassId() == ID_VHDLWAITSTATEMENT) {
	VhdlWaitStatement *ws = static_cast<VhdlWaitStatement*>(s);
	if (ws->UntilClause() && ws->UntilClause()->FindIfEdge()) {
	  d_clocked = true;
	}
      }
      TraverseNode(s);
    }
    d_clocked = false;
  }

  void Visit(VhdlIfStatement& stmt)
  {
    if (stmt.GetIfCondition()->FindIfEdge()) {
      d_clocked = true;
      TraverseArray(stmt.GetIfStatements());
      d_clocked = false;
    } else {
      TraverseNode(stmt.GetIfCondition());
      TraverseArray(stmt.GetIfStatements());      
    }
    TraverseArray(stmt.GetElsifList());
    TraverseArray(stmt.GetElseStatments());
  }

  void Visit(VhdlElsif& stmt)
  {
    if (stmt.Condition()->FindIfEdge()) {
      d_clocked = true;
      TraverseArray(stmt.GetStatements());
      d_clocked = false;
    } else {
      TraverseNode(stmt.Condition());
      TraverseArray(stmt.GetStatements());      
    }
  }

  void Visit(VhdlConditionalSignalAssignment& stmt)
  {
    TraverseTarget(stmt.GetTarget());  
    TraverseArray(stmt.GetConditionalWaveforms());
  }

  void Visit(VhdlSelectedSignalAssignment& stmt)
  {
    TraverseTarget(stmt.GetTarget());    
    TraverseArray(stmt.GetSelectedWaveforms());
  }

  void Visit(VhdlSignalAssignmentStatement& stmt)
  {
    TraverseTarget(stmt.GetTarget());
    TraverseArray(stmt.GetWaveform());
  }

  void Visit(VhdlVariableAssignmentStatement& stmt)
  {
    TraverseTarget(stmt.GetTarget());
    TraverseNode(stmt.GetValue());
  }

  void Visit(VhdlIndexedName& name)
  {
    int i;
    VhdlExpression *idx;
    bool constIdx = false;
    FOREACH_ARRAY_ITEM(name.GetAssocList(), i, idx) {
      if (!idx) {
	// open association in function call
      } else if (i == 0 && idx->IsGloballyStatic()) {
	constIdx = true;
      } else {
	TraverseExpr(idx);
      }
    }

    bool oldIdx = d_idx;
    bool oldConstIdx = d_constIdx;
    if (name.IsArrayIndex()) {
      d_idx = true;
    }
    d_constIdx = constIdx;

    TraverseNode(name.GetPrefix());

    d_constIdx = oldConstIdx;
    d_idx = oldIdx;
  }

  void Visit(VhdlSelectedName& name)
  {
    if (!name.IsExpandedName()) {
      TraverseNode(name.GetPrefix());
    }
  }

  void Visit(VhdlAttributeName& name)
  {
    // ignore references to mem'range, etc.
  }

  void Visit(VhdlIdRef& name)
  {
    VhdlIdDef *id = name.GetId();

    if (d_assign && d_idx && !d_constIdx) {
      d_targets.add(id);
    }

    if (!d_idx) {
      d_rejects.add(id);
    } else if (d_assign && !d_clocked) {
      if (!d_constIdx) {
	d_rejects.add(id);
      }
    } else if (d_assign && d_clocked) {
      if (d_constIdx) {
	incrConstAccess(id);
      } else {
	incrVarWrite(id);
      }
    } else if (!d_assign) {
      if (d_constIdx) {
	incrConstAccess(id);
      }
    }
  }

private:
  bool        d_clocked;
  bool        d_assign;
  bool        d_constIdx;
  bool        d_idx;
  CountTable  d_constAccesses;
  CountTable  d_varWrites;
  Vhdl::RamSet d_targets;
  Vhdl::RamSet d_rejects;
};


void mapMemories(VhdlTreeNode *n)
{
  VhdlMemoryVisitor mapper;
  n->Accept(mapper);
}

VhdlExpression *getAttribute(VhdlIdDef *id, const char *name)
{
  VhdlExpression *rsl = NULL;
  Map *attrs = id->GetAttributes();

  MapIter i;
  VhdlIdDef *attr_id;
  VhdlExpression *val;

  // Map is VhdlAttributeId* -> VhdlExpression*.  Can't
  // look up by string
  FOREACH_MAP_ITEM(attrs, i, &attr_id, &val) {
    if (0 == strcmp(attr_id->Name(), name)) {
      rsl = val;
      break;
    }
  }
  return rsl;
}

UStrBuf getStringValue(VhdlStringLiteral *n)
{
  // Verific includes the quotes around a string
  UStrBuf rsl;
  const char *quoted_value = n->Name();
  uint_t len = strlen(quoted_value);
  for (uint_t i = 1; i < len - 1; i++) {
    rsl << quoted_value[i];
  }
  return rsl;
}

UStrBuf getStringValue(VhdlCharacterLiteral *n)
{
  // Verific includes the quotes around a character literal
  UStrBuf rsl;
  const char *quoted_value = n->Name();
  rsl << quoted_value[1];
  return rsl;
}

// For parsing the enum_encoding attribute
UStrBuf getStringValue(VhdlExpression *n)
{
  UStrBuf rsl;
  if (!n) {
    return rsl;
  }
  switch (n->GetClassId()) {
  case ID_VHDLSTRINGLITERAL:
    rsl = getStringValue((VhdlStringLiteral *) n);
    break;
  case ID_VHDLCHARACTERLITERAL:
    rsl = getStringValue((VhdlCharacterLiteral *) n);
    break;
  case ID_VHDLAGGREGATE: {
    Array *elems = ((VhdlAggregate *) n)->GetElementAssocList();
    int i;
    VhdlExpression *elem;
    FOREACH_ARRAY_ITEM(elems, i, elem) {
      rsl << getStringValue(elem);
    }
    break;
  }
  case ID_VHDLASSOCELEMENT:
    VHMSG(unimplemented, "association element in aggregate for enum_encoding");
    break;
  case ID_VHDLOPERATOR: {
    VhdlOperator *op = static_cast<VhdlOperator *>(n);
    if (op->GetOperatorToken() == VHDL_AMPERSAND) {
      rsl << getStringValue(n->GetLeftExpression());
      rsl << getStringValue(n->GetRightExpression());
    } else {
      VHMSG(unimplemented, "unexpected operator for enum_encoding");
    }
    break;
  }
  case ID_VHDLIDREF: {
    VhdlIdDef *id = ((VhdlIdRef*)n)->GetId();
    if (id && id->IsConstantId()) {
      VhdlConstantDecl *decl = static_cast<VhdlConstantDecl*>(Vhdl::getDecl(id));
      if (decl) {
	return getStringValue(decl->GetInitAssign());
      }
    }
    break;
  }
    
  default:
    VHMSG(unimplemented, "unexpected expression for enum_encoding");
    break;
  }
  return rsl;
}


UStrBuf getSubprogramName(VhdlTreeNode *n)
{
  UStrBuf rsl;
  switch (n->GetClassId()) {
  case ID_VHDLSUBPROGRAMID: {
    const char *name = getName((VhdlSubprogramId *)n);
    if (name[0] == '"') {
      // convert operator name to alphabetic (for dot output)
      switch (name[1]) {
      case '+':
	rsl << "plus";
	break;
      case '-':
	rsl << "minus";
	break;
      case '*':
	if (name[2] == '*') {
	  rsl << "exp";
	} else {
	  rsl << "mult";
	}
	break;
      case '/':
	if (name[2] == '=') {
	  rsl << "neq";
	} else {
	  rsl << "div";
	}
	break;
      case '=':
	rsl << "eq";
	break;
      case '>':
	if (name[2] == '=') {
	  rsl << "geq";
	} else {
	  rsl << "gt";
	}
	break;
      case '<':
	if (name[2] == '=') {
	  rsl << "leq";
	} else {
	  rsl << "lt";
	}
	break;
      default:
	rsl << &name[1];
	rsl.setTail(strlen(name)-2);
	break;
      }
    } else {
      rsl << name;
    }
    return rsl;
  }
  case ID_VHDLFUNCTIONSPEC:
    return getSubprogramName(((VhdlFunctionSpec *)n)->GetDesignator());
  case ID_VHDLPROCEDURESPEC:
    return getSubprogramName(((VhdlProcedureSpec *)n)->GetDesignator());
  default:
    uassert(0);
    return rsl;
  }
}


bool isBinaryOperator(VhdlExpression *e)
{
  if (!e) {
    return false;
  }
  if (e->GetClassId() == ID_VHDLOPERATOR && getRightArg(e) != NULL) {
    return true;
  }
  if (e->GetClassId() == ID_VHDLINDEXEDNAME && getRightArg(e) != NULL) {
    return true;
  }
  return false;
}

static int strToOp(const char *s) {
  // ?? does verific not have a way to do this?
  if (0 == strcmp(s, "\"and\"")) {
    return VHDL_and;
  } else if (0 == strcmp(s, "\"or\"")) {
    return VHDL_or;
  } else if (0 == strcmp(s, "\"xor\"")) {
    return VHDL_xor;
  } else if (0 == strcmp(s, "\"nand\"")) {
    return VHDL_nand;
  } else if (0 == strcmp(s, "\"nor\"")) {
    return VHDL_nor;
  } else if (0 == strcmp(s, "\"xnor\"")) {
    return VHDL_xnor;
  } else if (0 == strcmp(s, "\"<\"")) {
    return VHDL_STHAN;
  } else if (0 == strcmp(s, "\"<=\"")) {
    return VHDL_SEQUAL;
  } else if (0 == strcmp(s, "\">\"")) {
    return VHDL_GTHAN;
  } else if (0 == strcmp(s, "\">=\"")) {
    return VHDL_GEQUAL;
  } else if (0 == strcmp(s, "\"=\"")) {
    return VHDL_EQUAL;
  } else if (0 == strcmp(s, "\"/=\"")) {
    return VHDL_NEQUAL;
  } else if (0 == strcmp(s, "\"sll\"")) {
    return VHDL_sll;
  } else if (0 == strcmp(s, "\"sla\"")) {
    return VHDL_sla;
  } else if (0 == strcmp(s, "\"srl\"")) {
    return VHDL_srl;
  } else if (0 == strcmp(s, "\"sra\"")) {
    return VHDL_sra;
  } else if (0 == strcmp(s, "\"ror\"")) {
    return VHDL_ror;
  } else if (0 == strcmp(s, "\"rol\"")) {
    return VHDL_rol;
  } else if (0 == strcmp(s, "\"+\"")) {
    return VHDL_PLUS;
  } else if (0 == strcmp(s, "\"-\"")) {
    return VHDL_MINUS;
  } else if (0 == strcmp(s, "\"&\"")) {
    return VHDL_AMPERSAND;
  } else if (0 == strcmp(s, "\"*\"")) {
    return VHDL_STAR;
  } else if (0 == strcmp(s, "\"/\"")) {
    return VHDL_SLASH;
  } else if (0 == strcmp(s, "\"mod\"")) {
    return VHDL_mod;
  } else if (0 == strcmp(s, "\"rem\"")) {
    return VHDL_rem;
  } else if (0 == strcmp(s, "\"**\"")) {
    return VHDL_EXPONENT;
  } else if (0 == strcmp(s, "\"not\"")) {
    return VHDL_not;
  } else if (0 == strcmp(s, "\"abs\"")) {
    return VHDL_abs;
  } else {
    uassert(0);
    return -1;
  }
}

static int getOperatorToken(VhdlExpression *e)
{
  if (e->GetClassId() == ID_VHDLOPERATOR) {
    return ((VhdlOperator *)e)->GetOperatorToken();
  } else {
    uassert(e->GetClassId() == ID_VHDLINDEXEDNAME);
    VhdlIndexedName *in = static_cast<VhdlIndexedName*>(e);
    uassert(in->IsFunctionCall());
    VhdlName *prefix = in->GetPrefix();
    if (prefix->GetClassId() == ID_VHDLSELECTEDNAME) {
      prefix = ((VhdlSelectedName*)prefix)->GetSuffix();
    }
    if (prefix->GetClassId() == ID_VHDLSTRINGLITERAL) {
      VhdlStringLiteral *sl = static_cast<VhdlStringLiteral *>(prefix);
      return strToOp(sl->Name());
    } else {
      VhdlIdDef *id = prefix->GetId();
      if (id) {
	uassert(id->GetClassId() == ID_VHDLOPERATORID);
	return ((VhdlOperatorId *)id)->GetOperatorType();
      } else {
	if (0 == strcmp(prefix->Name(), "rising_edge")) {
	  return VHDL_rising_edge;
	} else if (0 == strcmp(prefix->Name(), "falling_edge")) {
	  return VHDL_falling_edge;
	} else {
	  return 0;
	}
      }
    }
  }
}

UKeyword getBinaryOperatorType(VhdlExpression *e)
{
  return getBinaryOperatorType(getOperatorToken(e));
}

UKeyword getBinaryOperatorType(int op)
{
  switch (op) {
    // 7.2.1 Logical operators
  case VHDL_and:
    return S_band;
  case VHDL_or:
    return S_bor;
  case VHDL_nand:
    return K_nand;
  case VHDL_nor:
    return K_nor;
  case VHDL_xnor:
    return S_bxnor;
  case VHDL_xor:
    return S_bxor;

    // 7.2.2 Relational operators
    
  case VHDL_EQUAL:
    return S_eq;
  case VHDL_NEQUAL:
    return S_neq;
  case VHDL_STHAN:
    return S_lt;
  case VHDL_SEQUAL:
    return S_leq;
  case VHDL_GTHAN:
    return S_gt;
  case VHDL_GEQUAL:
    return S_geq;

    // 7.2.3 Shift operators

  case VHDL_sll:
    return S_lshift;
  case VHDL_srl:
    return S_rshift;
  case VHDL_sla:
    return S_alshift;
  case VHDL_sra:
    return S_arshift;
  case VHDL_rol:
    return K_vhdl_rol;
  case VHDL_ror:
    return K_vhdl_ror;

    // 7.2.4 Adding operators
  case VHDL_PLUS:
    return S_add;
  case VHDL_MINUS:
    return S_sub;
  case VHDL_AMPERSAND:
    return K_vhdl_concat;

    // 7.2.6 Multiplying operators
  case VHDL_STAR:
    return S_mult;
  case VHDL_SLASH:
    return S_div;
  case VHDL_mod:
    return K_vhdl_mod;	// differs from verilog %
  case VHDL_rem:
    return S_mod;	// vhdl rem behaves like verilog %

    // 7.2.7 Miscellanous operators
  case VHDL_EXPONENT:
    return S_power;

  case VHDL_matching_equal:
    return S_vhdl_qeq;

  case VHDL_matching_nequal:
    return S_vhdl_qneq;

  case VHDL_matching_gthan:
    return S_vhdl_qgt;

  case VHDL_matching_sthan:
    return S_vhdl_qlt;

  case VHDL_matching_gequal:
    return S_vhdl_qgeq;

  case VHDL_matching_sequal:
    return S_vhdl_qleq;

  default:
    uassert(0);
    return K_null;
  }
}

UKeyword getUnaryOperatorType(VhdlExpression *e)
{
  return getUnaryOperatorType(getOperatorToken(e));
}

UKeyword getUnaryOperatorType(int op)
{
  switch (op) {
    // 7.2.1 Logical operators
  case VHDL_not:
    return S_bnot;

    // 7.2.5 Sign operators
  case VHDL_PLUS:
    return S_uplus;
  case VHDL_MINUS:
  case VHDL_UMINUS:
    return S_uminus;

    // 7.2.7 Miscellaneous operators
  case VHDL_abs:
    return K_vhdl_abs;

  case VHDL_and:
  case VHDL_REDAND:
    return S_rand;
  case VHDL_nand:
  case VHDL_REDNAND:
    return S_rnand;
  case VHDL_or:
  case VHDL_REDOR:
    return S_ror;
  case VHDL_nor:
  case VHDL_REDNOR:
    return S_rnor;
  case VHDL_xor:
  case VHDL_REDXOR:
    return S_rxor;
  case VHDL_xnor:
  case VHDL_REDXNOR:
    return S_rxnor;

  case VHDL_condition:
    return S_vhdl_qq;	// "??"

  default:
    uassert(0);
    return K_null;
  }
}


bool isOperatorFunction(VhdlExpression *e)
{
  if (e->GetClassId() == ID_VHDLOPERATOR) {
    VhdlIdDef *op = e->GetOperator();
    if (op) {
      return op->GetClassId() == ID_VHDLSUBPROGRAMID;
    }
  }
  return false;
}


bool isBoxConstraint(VhdlDiscreteRange *rng)
{
  if (rng->GetClassId() == ID_VHDLEXPLICITSUBTYPEINDICATION) {
    VhdlExplicitSubtypeIndication *esi = static_cast<VhdlExplicitSubtypeIndication *>(rng);
    return esi->GetRangeConstraint()->GetClassId() == ID_VHDLBOX;
  }
  return false;
}

bool isRange(VhdlDiscreteRange *e)
{
  return e->IsRange();
}


VhdlConcatType getConcatType(VhdlOperatorId *opId)
{
  uassert(opId->GetOperatorType() == VHDL_AMPERSAND);

  Array *args = opId->GetArgs();
  uassert(args->Size() == 2);
  
  VhdlIdDef *lType = (VhdlIdDef *) args->At(0);
  VhdlIdDef *rType = (VhdlIdDef *) args->At(1);
  VhdlIdDef *rslType = opId->BaseType();

  if (lType == rType) {
    if (rType == rslType) {
      return VCT_ArrayArray;
    } else {
      return VCT_ElemElem;
    }
  } else if (lType == rslType) {
    return VCT_ArrayElem;
  } else {
    return VCT_ElemArray;
  }
}

VhdlDiscreteRange *getSliceRange(VhdlIndexedName *name)
{
  uassert(name->IsArraySlice());
  Array *assocs = name->GetAssocList();
  return (VhdlDiscreteRange *) assocs->GetFirst();
}

VhdlIdDef *unExpand(VhdlSelectedName *name)
{
  // Replace expanded name like pkg.x with x.  Unfortunately verific doesn't
  // do this automatically.
  if (!name || name->GetClassId() != ID_VHDLSELECTEDNAME) {
    return NULL;
  }
  VhdlSelectedName* sn = static_cast<VhdlSelectedName*>(name);
  if (sn->GetSuffix()->IsAll()) {
    return NULL;
  }
  VhdlIdDef *id = sn->GetUniqueId();
  if (id && id->GetClassId() != ID_VHDLELEMENTID) {
    return id;
  }
  return NULL;
}

VhdlName *derefAlias(VhdlName* n)
{
  if (!n) {
    return NULL;
  }
  if (n->GetClassId() == ID_VHDLIDREF) {
    VhdlIdRef *ref = static_cast<VhdlIdRef*>(n);
    VhdlIdDef *id = ref->GetId();
    if (id) {
      VhdlName* target = derefAlias(id);
      if (target) {
	return target;
      }
    }
  }
  return n;
}

VhdlName *derefAlias(VhdlIdDef* id)
{
  if (!id) {
    return NULL;
  }
  if (id->GetClassId() == ID_VHDLALIASID) {
    VhdlAliasDecl *alias = static_cast<VhdlAliasDecl*>(Vhdl::getDecl(id));
    VhdlName *target = alias->GetAliasTargetName();
    if (target) {
      return derefAlias(target);
    }
  }
  return NULL;
}

VhdlIdDef *derefAliasId(VhdlIdDef* id)
{
  VhdlName* target = derefAlias(id);
  if (!target) {
    return id;
  } else {
    return target->GetId();
  }
}

VhdlExpression *getIfCondition(VhdlIfStatement *stmt, int idx)
{
  VhdlExpression *cond = NULL;
  Array *elsifs = stmt->GetElsifList();
  int elsifCount = elsifs ? elsifs->Size() : 0; 
  if (idx == 0) {
    cond = stmt->GetIfCondition();
  } else if (idx <= elsifCount) {
    VhdlElsif *ei = (VhdlElsif *) elsifs->At(idx-1);
    cond = ei->Condition();
  }
  return cond;
}

Array *getIfClause(VhdlIfStatement *stmt, int idx)
{
  Array *ifClause = NULL;
  Array *elsifs = stmt->GetElsifList();
  int elsifCount = elsifs ? elsifs->Size() : 0; 
  if (idx == 0) {
    ifClause = stmt->GetIfStatements();
  } else if (idx <= elsifCount) {
    VhdlElsif *ei = (VhdlElsif *) elsifs->At(idx-1);
    ifClause = ei->GetStatements();
  } else if (idx == elsifCount + 1) {
    ifClause = stmt->GetElseStatments();
  }
  return ifClause;
}

VhdlDiscreteRange *nthIndexSubtype(VhdlIdDef *id, int idx)
{
  uassert(id->IsArrayType());
  VhdlFullTypeDecl *decl = (VhdlFullTypeDecl *) Vhdl::getDecl(id);
  VhdlTypeDef *td = decl->GetTypeDef();
  uassert(td->GetClassId() == ID_VHDLARRAYTYPEDEF);
  Array *constraints = ((VhdlArrayTypeDef *) td)->GetIndexConstraint();
  return (VhdlDiscreteRange *) constraints->At(idx);
}

VhdlName *baseName(VhdlName *name)
{
  switch (name->GetClassId()) {
  case ID_VHDLINDEXEDNAME: {
    VhdlIndexedName *in = static_cast<VhdlIndexedName *>(name);
    if (in->IsFunctionCall()) {
      return in;
    } else if (in->IsArrayIndex() || in->IsArraySlice()) {
      return baseName(in->GetPrefix());
    } else if (in->IsIndexConstraint()) {
      return baseName(in->GetPrefix());
    } else if (!in->IsTypeConversion()) {
      // Assume this is subelement association to a verilog module; 
      // verific does not know what the prefix represents and so cannot
      // mark it as an array index or slice.
      return baseName(in->GetPrefix());
    } else {
      uassert(0);
      return NULL;
    }
  }
  case ID_VHDLSELECTEDNAME: {
    VhdlSelectedName *sn = static_cast<VhdlSelectedName *>(name);
    VhdlIdDef *id = unExpand(sn);
    if (id) {
      return sn;
    } else {
      return baseName(sn->GetPrefix());
    }
  }

  case ID_VHDLATTRIBUTENAME:
    return name;

  case ID_VHDLEXTERNALNAME:
    return name;

  case ID_VHDLIDREF: {
    VhdlIdRef* ref = static_cast<VhdlIdRef*>(name);
    if (!ref->GetId()) {
      return name;
    } else if (ref->GetId()->IsAlias()) {
      return baseName(ref->GetId()->GetAliasTarget());
    } else {
      return name;
    }
  }

  default:
    uassert(0);
    return NULL;
  }
}

VhdlIdDef* getSubprogramId(VhdlProcedureCallStatement *call)
{
  return getSubprogramId(call->GetCall());
}

VhdlIdDef *getSubprogramId(VhdlExpression *n)
{
  VhdlIdDef *id;
  switch (n->GetClassId()) {
  case ID_VHDLINDEXEDNAME:
    id = ((VhdlIndexedName *) n)->GetPrefixId();
    break;
  case ID_VHDLOPERATOR:
    id = ((VhdlOperator *) n)->GetOperator();
    break;
  case ID_VHDLIDREF:
    id = n->GetId();
    break;
  case ID_VHDLSELECTEDNAME:
    id = unExpand((VhdlSelectedName *)n);
    break;
  default:
    id = NULL;
    uassert(0);
    break;
  }

  return id;
}

Verific::VhdlIdDef* getNthFormal(Verific::VhdlFunctionSpec *spec, int n)
{
  // n is zero-based
  int k = 0;
  int i;
  VhdlInterfaceDecl *decl;
  FOREACH_ARRAY_ITEM(spec->GetFormalParamList(), i, decl) {
    int j;
    VhdlInterfaceId *id;
    FOREACH_ARRAY_ITEM(decl->GetIds(), j, id) {
      if (k == n) {
	return id;
      } else {
	k++;
      }
    }
  }
  return NULL;
}

VhdlSpecification* getSpec(VhdlProcedureCallStatement *call)
{
  return getSpec(call->GetCall());
}

VhdlSpecification* getSpec(VhdlExpression *n)
{
  VhdlIdDef *subp = getSubprogramId(n);
  return subp ? subp->Spec() : NULL;
}

VhdlSubprogramBody* getBody(VhdlProcedureCallStatement *call)
{
  return getBody(call->GetCall());
}

VhdlSubprogramBody* getBody(VhdlExpression *n)
{
  VhdlIdDef *subp = getSubprogramId(n);
  return subp ? subp->Body() : NULL;
}

unsigned int getFunctionPragma(VhdlExpression *e)
{
  VhdlIdDef *id = getSubprogramId(e);
  int pragmaFn = id->GetPragmaFunction();
  if (pragmaFn != 0) {
    return pragmaFn;
  }
  VhdlSpecification *spec = getSpec(e);
  if (!spec) {
    return 0;
  }

  if (Vhdl::getPredefPack(spec)) {
    // ?? hack: check for rising_edge/falling_edge from std_logic_1164.
    // Verific's version doesn't have pragmas for these functions.
    if (0 == strcasecmp(id->Name(), "rising_edge")) {
      return VHDL_rising_edge;
    } else if (0 == strcasecmp(id->Name(), "falling_edge")) {
      return VHDL_falling_edge;
    }
  }
  return 0;
}

bool isSignedFunctionPragma(VhdlExpression *e)
{
  VhdlIdDef *id = getSubprogramId(e);
  return id->GetPragmaSigned();
}

unsigned int getProcedurePragma(VhdlProcedureCallStatement *s)
{
  VhdlIdDef *id = getSubprogramId(s);
  return id->GetPragmaFunction();
}

VhdlExpression* getLeftArg(VhdlExpression *e)
{
  switch (e->GetClassId()) {
  case ID_VHDLINDEXEDNAME:
    return (VhdlExpression *) ((VhdlIndexedName *) e)->GetAssocList()->GetFirst();
  case ID_VHDLOPERATOR:
    return ((VhdlOperator *) e)->GetLeftExpression();
  default:
    uassert(0);
    return NULL;
  }
}

VhdlExpression* getRightArg(VhdlExpression *e)
{
  switch (e->GetClassId()) {
  case ID_VHDLINDEXEDNAME: {
    VhdlIndexedName *in = static_cast<VhdlIndexedName *>(e);
    if (e->GetAssocList()->Size() <= 1) {
      return NULL;
    }
    return (VhdlExpression *) in->GetAssocList()->At(1);
  }
  case ID_VHDLOPERATOR:
    return ((VhdlOperator *) e)->GetRightExpression();
  default:
    uassert(0);
    return NULL;
  }
}

VhdlName* getReturnType(VhdlExpression *e)
{
  uassert(e->GetClassId() == ID_VHDLINDEXEDNAME);
  VhdlSubprogramBody *body = getBody(e);
  VhdlSpecification *spec = body->GetSubprogramSpec();
  uassert(spec->GetClassId() == ID_VHDLFUNCTIONSPEC);
  return ((VhdlFunctionSpec *)spec)->GetReturnType();
}

bool isTemplate(VhdlEntityId* m)
{
  return m->GetAttribute("template") != NULL;
}

bool isBlackbox(VhdlIdDef* m)
{
  if (!m) {
    return false;
  }
  return (m->GetAttribute("black_box") != NULL || 
	  m->GetAttribute("syn_black_box") != NULL);
}

bool isPrimitive(VhdlIdDef* m)
{
  if (!m) {
    return false;
  }
  VhdlExpression *expr = getAttribute(m, "box_type");
  if (!expr) {
    return false;
  }
  UStrBuf rsl = getStringValue(expr);
  if (0 == strcasecmp(rsl.str(), "PRIMITIVE")) {
    return true;
  }
  return false;
}

bool isInferMux(VhdlIdDef* id)
{
  return id->GetAttribute("infer_mux");
}

bool isPredefType(VhdlIdDef* id, const char* type)
{
  if (!id) {
    return false;
  }
  switch (id->GetClassId()) {
  case ID_VHDLTYPEID:
    return Vhdl::getPredefPack(id) && 0 == strcasecmp(type, id->Name());
  case ID_VHDLALIASID:
  case ID_VHDLVARIABLEID:
  case ID_VHDLSIGNALID:
  case ID_VHDLCONSTANTID:
  case ID_VHDLINTERFACEID:
  case ID_VHDLSUBTYPEID:
    return isPredefType(id->Type(), type);
  default:
    return false;
  }
}

bool isPredefType(VhdlExpression* e, const char* type)
{
  if (!e) {
    return false;
  } else if (e->GetClassId() == ID_VHDLAGGREGATE) {
    // can't call TypeInfer without context because it may 
    // report an error
    return false;
  }
  VhdlIdDef* typeId = e->TypeInfer(NULL, NULL, 0, NULL);
  return isPredefType(typeId, type);
}

bool isStringType(VhdlIdDef* id)
{
  return isPredefType(id, "string");
}

bool isStringType(VhdlExpression* e)
{
  return isPredefType(e, "string");
}

bool isBooleanType(VhdlIdDef* id)
{
  return isPredefType(id, "boolean");
}

bool isBooleanType(VhdlExpression* e)
{
  return isPredefType(e, "boolean");
}

bool isRealType(VhdlIdDef* id)
{
  return isPredefType(id, "real");
}

bool isRealType(VhdlExpression* e)
{
  return isPredefType(e, "real");
}

static VhdlName* getTypeMark(VhdlSubtypeIndication* subtype)
{
  switch (subtype->GetClassId()) {
  case ID_VHDLINDEXEDNAME:
    return subtype->GetPrefix();
  case ID_VHDLEXPLICITSUBTYPEINDICATION:
    return subtype->GetTypeMark();
  case ID_VHDLIDREF:
    return static_cast<VhdlIdRef*>(subtype);
  default:
    return NULL;
  }
}

static VhdlIdDef* getBaseTypeId(VhdlIdDef* id)
{
  if (!id->IsType()) {
    return NULL;
  } else if (!id->IsSubtype()) {
    return id;
  } else {
    VhdlSubtypeDecl* s = static_cast<VhdlSubtypeDecl*>(Vhdl::getDecl(id));
    if (!s || s->GetClassId() != ID_VHDLSUBTYPEDECL) {
      return NULL;
    }
    VhdlSubtypeIndication* subtype = s->GetSubtypeIndication();
    if (subtype->GetClassId() == ID_VHDLSELECTEDNAME) {
      return static_cast<VhdlSelectedName*>(subtype)->GetUniqueId();
    }
    VhdlName* type = getTypeMark(subtype);
    return getBaseTypeId(type->GetId());
  }
}

bool isStdSubtype(VhdlDeclaration* decl)
{
  // returns true for subtypes declared in the standard packages
  if (!decl) {
    return false;
  }
  VhdlSubtypeIndication* subtype = getObjectSubtype(decl);
  if (!subtype) {
    return false;
  }
  VhdlIdDef* typeId;
  if (subtype->GetClassId() == ID_VHDLSELECTEDNAME) {
    typeId = static_cast<VhdlSelectedName*>(subtype)->GetUniqueId();
  } else {
    VhdlName* typeMark = getTypeMark(subtype);
    typeId = getBaseTypeId(typeMark->GetId());
  }
  if (typeId && Vhdl::getPredefPack(typeId)) {
    return true;
  }
  return false;
}

static DFPort::DFPortType getMode(int veriMode)
{
  DFPort::DFPortType mode = DFPort::DF_input;
  switch (veriMode) {
  case 0:	// no mode specified: defaults to 'in'
  case VHDL_in:
    mode = DFPort::DF_input;
    break;
  case VHDL_out:
  case VHDL_buffer:
    mode = DFPort::DF_output;
    break;
  case VHDL_inout:
  case VHDL_linkage:
    mode = DFPort::DF_inout;
    break;
  default:
    uassert(0);
    break;
  }
  return mode;
}

DFPort::DFPortType getMode(VhdlInterfaceDecl *decl)
{
  return getMode(decl->GetMode());
}

DFPort::DFPortType getMode(VhdlInterfaceId *id)
{
  return getMode(id->Mode());
}

void parseComponentInst(VhdlComponentInstantiationStatement *inst, 
			VhdlComponentId *&compId, 
			const char *&library,
			VhdlEntityId *&entityId, 
			const char *&archName, 
			VhdlConfigurationId *&configId)
{
  compId = NULL;
  library = NULL;
  entityId = NULL;
  archName = NULL;
  configId = NULL;

  VhdlName *unit = inst->GetInstantiatedUnitNameNode();
  if (unit->GetClassId() == ID_VHDLINSTANTIATEDUNIT) {
    VhdlInstantiatedUnit *instUnit = static_cast<VhdlInstantiatedUnit*>(unit);
    switch (instUnit->GetEntityClass()) {
    case VHDL_component:
      compId = static_cast<VhdlComponentId*>(derefAliasId(instUnit->GetId()));
      break;
    case VHDL_entity:
      entityId = static_cast<VhdlEntityId*>(derefAliasId(instUnit->GetId()));
      if (entityId) {
	library = entityId->GetDesignUnit()->GetOwningLib()->Name();
      }
      archName = instUnit->ArchitectureNameAspect();
      break;
    case VHDL_configuration:
      configId = static_cast<VhdlConfigurationId*>(derefAliasId(instUnit->GetId()));
      if (configId) {
	library = configId->GetDesignUnit()->GetOwningLib()->Name();
      }
      break;
    }
  } else if (unit->GetClassId() == ID_VHDLSELECTEDNAME) {
    VhdlSelectedName *sn = static_cast<VhdlSelectedName *>(unit);
    compId = static_cast<VhdlComponentId*>(derefAliasId(unExpand(sn)));
  } else {
    compId = static_cast<VhdlComponentId*>(derefAliasId(unit->GetId()));
  }
}

void parseComponentInst(VhdlComponentInstantiationStatement *inst, 
			UString& modName, UString& libName)
{
  modName = "";
  libName = "";
  VhdlName* instUnit = inst->GetInstantiatedUnitNameNode();
  if (instUnit->GetClassId() == ID_VHDLINSTANTIATEDUNIT) {
    VhdlInstantiatedUnit* iu = static_cast<VhdlInstantiatedUnit*>(instUnit);
    instUnit = iu->GetName();
  }
  if (instUnit->GetClassId() == ID_VHDLINDEXEDNAME) {
    VhdlIndexedName* in = static_cast<VhdlIndexedName*>(instUnit);
    instUnit = in->GetPrefix();
  }
  if (instUnit->GetClassId() == ID_VHDLSELECTEDNAME) {
    VhdlSelectedName* sn = static_cast<VhdlSelectedName*>(instUnit);
    VhdlName* prefix = sn->GetPrefix();
    if (prefix->GetClassId() == ID_VHDLSELECTEDNAME) {
      if (!sn->GetId() || !sn->GetId()->IsComponent()) {
        VHVMSG(unimplemented, prefix, "complex selected name for component/entity");
      }
    } else {
      libName = sn->GetPrefix()->Name();
    }
    modName = sn->GetSuffix()->Name();
  } else if (instUnit->GetClassId() == ID_VHDLIDREF) {
    modName = instUnit->Name();
  }
  if (isEscapedName(modName.str())) {
    modName = unEscapeName(modName.str());
  }
}


void parseBindingIndication(VhdlBindingIndication *binding, 
			    const char *&library,
			    VhdlEntityId *&entityId,
			    const char *&entity,
			    const char *&archName,
			    VhdlConfigurationId *&configId)
{
  library = NULL;
  entityId = NULL;
  entity = NULL;
  archName = NULL;
  configId = NULL;
  VhdlBlockConfiguration* blockConfig = NULL;

  VhdlIdDef *primary = derefAliasId(binding->EntityAspect());
  if (!primary) {
    VhdlName* bindingName = binding->GetEntityAspectNameNode();
    if (bindingName) {
      int entityClass = 0;
      if (bindingName->GetClassId() == ID_VHDLENTITYASPECT) {
	VhdlEntityAspect* ea = static_cast<VhdlEntityAspect*>(bindingName);
	entityClass = ea->GetEntityClass();
	bindingName = ea->GetName();
      }
      if (bindingName->GetClassId() == ID_VHDLINDEXEDNAME) {
	VhdlIndexedName* in = static_cast<VhdlIndexedName*>(bindingName);
	VhdlExpression* arch = static_cast<VhdlExpression*>(in->GetAssocList()->At(0));
	archName = arch->Name();
	bindingName = in->GetPrefix();
      }
      if (bindingName->GetClassId() == ID_VHDLSELECTEDNAME) {
	VhdlSelectedName* sn = static_cast<VhdlSelectedName*>(bindingName);
	library = sn->GetPrefix()->Name();
	if (entityClass == VHDL_configuration) {
	  configId = static_cast<VhdlConfigurationId*>(sn->GetId());
	} else {
	  entityId = static_cast<VhdlEntityId*>(sn->GetId());
	  entity = sn->GetSuffix()->Name();
	}
      } else {
	if (entityClass == VHDL_configuration) {
	  configId = static_cast<VhdlConfigurationId*>(bindingName->GetId());
	} else {
	  entityId = static_cast<VhdlEntityId*>(bindingName->GetId());
	  entity = bindingName->Name();
	}
      }
    }
  } else if (primary->GetClassId() == ID_VHDLCONFIGURATIONID) {
    configId = static_cast<VhdlConfigurationId *>(primary);
  } else {
    uassert(primary->GetClassId() == ID_VHDLENTITYID);
    entityId = static_cast<VhdlEntityId *>(primary);
    archName = binding->ArchitectureNameAspect();
  }

  if (configId) {
    parseConfigDecl(configId, entityId, archName, blockConfig);
  }

  if (entityId) {
    entity = entityId->Name();
    library = entityId->GetPrimaryUnit()->GetOwningLib()->Name();
  }

  if (library && entity && !entityId) {
    VhdlLibrary* lib = vhdl_file::GetLibrary(library);
    if (lib) {
      VhdlPrimaryUnit* unit = lib->GetPrimUnit(entity, false);
      if (unit) {
	entityId = static_cast<VhdlEntityId*>(unit->Id());
      }
    }
  }
}

VhdlBlockConfiguration* getBlockConfiguration(VhdlConfigurationId* configId)
{
  VhdlEntityId* entityId;
  const char* archName;
  VhdlBlockConfiguration* blockConfig;

  parseConfigDecl(configId, entityId, archName, blockConfig);
  return blockConfig;
}

void parseConfigDecl(VhdlConfigurationId* configId, VhdlEntityId*& entityId, const char *&archName, VhdlBlockConfiguration*& blockConfig)
{
  entityId = static_cast<VhdlEntityId*>(configId->GetEntity());
  VhdlConfigurationDecl* configDecl = static_cast<VhdlConfigurationDecl*>(configId->GetPrimaryUnit());
  blockConfig = configDecl->GetBlockConfiguration();
  VhdlName *blockSpec = blockConfig->GetBlockSpec();
  archName = blockSpec->Name();
}

VhdlComponentId* findComponentId(VhdlComponentInstantiationStatement *stmt)
{
  VhdlIdDef *rsl = NULL;
  VhdlName *unit = stmt->GetInstantiatedUnitNameNode();
  if (unit->GetClassId() == ID_VHDLINSTANTIATEDUNIT) {
    VhdlInstantiatedUnit *instUnit = static_cast<VhdlInstantiatedUnit*>(unit);
    if (instUnit->GetEntityClass() == VHDL_component) {
      rsl = instUnit->GetId();
    }
  } else if (unit->GetClassId() == ID_VHDLSELECTEDNAME) {
    VhdlSelectedName *sn = static_cast<VhdlSelectedName *>(unit);
    rsl = derefAliasId(unExpand(sn));
  } else {
    rsl = derefAliasId(unit->GetId());
  }
  if (rsl) {
    return static_cast<VhdlComponentId*>(rsl);
  } else {
    return NULL;
  }
}

VhdlComponentConfiguration *findCompConfig(VhdlBlockConfiguration* blockConfig, const char *label, VhdlComponentId* compId)
{
  int i;
  VhdlConfigurationItem *item;
  FOREACH_ARRAY_ITEM(blockConfig->GetConfigurationItemList(), i, item) {
    if (item->GetClassId() == ID_VHDLCOMPONENTCONFIGURATION) {
      VhdlComponentConfiguration *cc = static_cast<VhdlComponentConfiguration *>(item);
      VhdlSpecification *spec = cc->GetComponentSpec();
      VhdlComponentSpec *cSpec = static_cast<VhdlComponentSpec *>(spec);
      if (cSpec->GetId() != compId) {
	continue;
      }
      int j;
      VhdlName *id;
      FOREACH_ARRAY_ITEM(cSpec->GetIds(), j, id) {
	switch (id->GetClassId()) {
	case ID_VHDLIDREF:
	  if (0 == strcasecmp(id->Name(), label)) {
	    return cc;
	  }
	  break;
	case ID_VHDLALL:
	case ID_VHDLOTHERS:
	  return cc;
	default:
	  uassert(0);
	  break;
	}
      }
    }
  }
  return NULL;
}


VhdlBlockConfiguration *findBlockConfig(VhdlBlockConfiguration* blockConfig, const char *label)
{
  int i;
  VhdlConfigurationItem *item;
  FOREACH_ARRAY_ITEM(blockConfig->GetConfigurationItemList(), i, item) {
    if (item->GetClassId() == ID_VHDLBLOCKCONFIGURATION) {
      VhdlBlockConfiguration *bc = static_cast<VhdlBlockConfiguration *>(item);
      VhdlName* blockName = bc->GetBlockSpec();
      if (blockName->GetClassId() == ID_VHDLINDEXEDNAME) {
	blockName = blockName->GetPrefix();
      }
      VhdlIdDef* blockId = blockName->GetId();
      if (0 == strcasecmp(blockId->Name(), label)) {
	return bc;
      }
    }
  }
  return NULL;
}

UString defaultBindingLibrary(VhdlScope* scope, VhdlComponentId* compId)
{
  VhdlIdDef* unit;
  if (scope) {
    unit = scope->FindDefaultBinding(compId->Name(), compId);
    if (unit) {
      VhdlDesignUnit* designUnit = unit->GetDesignUnit();
      if (designUnit) {
	VhdlLibrary* vhdlLib = designUnit->GetOwningLib();
	return vhdlLib->Name();
      }
    }
  }
  return "";
}

VhdlPrimaryUnit* getUnit(VhdlName* nm)
{
  // Accepts either lib.prim or prim
  VhdlLibrary* lib = NULL;
  const char* primName = NULL;
  if (nm->GetClassId() == ID_VHDLSELECTEDNAME) {
    const char* libName = nm->GetPrefix()->Name();
    if (0 == strcasecmp("work", libName)) {
      lib = vhdl_file::GetWorkLib();
    } else {
      lib = vhdl_file::GetLibrary(libName);
    }
    primName = nm->GetSuffix()->Name();
  } else {
    lib = vhdl_file::GetWorkLib();
    primName = nm->Name();
  }
  if (!lib) {
    return NULL;
  }
  return lib->GetPrimUnit(primName, 0, 0);
}

bool isTextIOFile(VhdlDeclaration* decl)
{
  VhdlSubtypeIndication* subtype = getObjectSubtype(decl);
  if (!subtype || subtype->GetClassId() != ID_VHDLIDREF) {
    return false;
  }
  VhdlIdDef* id = subtype->GetId();
  if (!id || !id->IsFileType()) {
    return false;
  }
  if (0 != strcmp("text", id->Name())) {
    return false;
  }
  UString pack;
  if (!Vhdl::getPredefPack(id, pack) || !pack.eql("textio")) {
    return false;
  }
  return true;
}

bool isTextIOLine(VhdlDeclaration* decl)
{
  VhdlSubtypeIndication* subtype = getObjectSubtype(decl);
  if (!subtype || subtype->GetClassId() != ID_VHDLIDREF) {
    return false;
  }
  VhdlIdDef* id = subtype->GetId();
  if (!id || !id->IsAccessType()) {
    return false;
  }
  if (0 != strcmp("line", id->Name())) {
    return false;
  }
  UString pack;
  if (!Vhdl::getPredefPack(id, pack) || !pack.eql("textio")) {
    return false;
  }
  return true;
}

UStrBuf getDebugName(VhdlTreeNode *n)
{
  // Try to get descriptive name of this object.
  // Need to check *every* damn object type!
  UStrBuf rsl;
  if (!n) {
    return rsl;
  }

  switch (n->GetClassId()) {

  case ID_VHDLALIASID:
  case ID_VHDLARCHITECTUREID:
  case ID_VHDLATTRIBUTEID:
  case ID_VHDLCOMPONENTID:
  case ID_VHDLCONFIGURATIONID:
  case ID_VHDLCONSTANTID:
  case ID_VHDLELEMENTID:
  case ID_VHDLENTITYID:
  case ID_VHDLENUMERATIONID:
  case ID_VHDLFILEID:
  case ID_VHDLGROUPID:
  case ID_VHDLGROUPTEMPLATEID:
  case ID_VHDLIDDEF:
  case ID_VHDLINTERFACEID:
  case ID_VHDLLABELID:
  case ID_VHDLLIBRARYID:
  case ID_VHDLOPERATORID:
  case ID_VHDLPACKAGEBODYID:
  case ID_VHDLPACKAGEID:
  case ID_VHDLPHYSICALUNITID:
  case ID_VHDLSIGNALID:
  case ID_VHDLSUBPROGRAMID:
  case ID_VHDLSUBTYPEID:
  case ID_VHDLTYPEID:
  case ID_VHDLVARIABLEID:
    rsl << getName((VhdlIdDef *) n);
    break;

  case ID_VHDLIDREF:
    rsl << getDebugName(((VhdlIdRef *) n)->GetId());
    break;

  case ID_VHDLCHARACTERLITERAL:
    rsl << ((VhdlCharacterLiteral *) n)->Name();
    break;
  case ID_VHDLSTRINGLITERAL:
    rsl << ((VhdlStringLiteral *) n)->Name();
    break;
  case ID_VHDLBITSTRINGLITERAL:
    rsl << ((VhdlBitStringLiteral *) n)->GetBasedBitString();
    break;
  case ID_VHDLREAL:
    rsl << ((VhdlReal *) n)->GetValue();
    break;
  case ID_VHDLINTEGER:
    rsl << (int) ((VhdlInteger *) n)->GetValue();
    break;

  case ID_VHDLINDEXEDNAME: {
    rsl << getDebugName(((VhdlIndexedName *) n)->GetPrefix());
    rsl << "(";
    Array *indices = ((VhdlIndexedName *) n)->GetAssocList();
    int i;
    VhdlDesignator *idx;
    FOREACH_ARRAY_ITEM(indices, i, idx) {
      if (i > 0) {
	rsl << ", ";
      }
      rsl << getDebugName(idx);
    }
    rsl << ")";
    break;
  }

  case ID_VHDLSELECTEDNAME:
    rsl << getDebugName(((VhdlSelectedName *) n)->GetPrefix());
    rsl << ".";
    rsl << getDebugName(((VhdlSelectedName *) n)->GetSuffix());
    break;

  case ID_VHDLATTRIBUTENAME:
    rsl << getDebugName(((VhdlAttributeName *) n)->GetPrefix());
    rsl << "'";
    rsl << getDebugName(((VhdlAttributeName *) n)->GetDesignator());
    if (((VhdlAttributeName *) n)->GetExpression()) {
      rsl << "(";
      rsl << getDebugName(((VhdlAttributeName *) n)->GetExpression());
      rsl << ")";
    }
    break;

  case ID_VHDLOPERATOR:
    rsl << getName(((VhdlOperator *) n)->GetOperator());
    break;

  case ID_VHDLNULL:
    rsl << "null";
    break;
  case ID_VHDLALL:
    rsl << "all";
    break;
  case ID_VHDLOTHERS:
    rsl << "others";
    break;
  case ID_VHDLOPEN:
    rsl << "open";
    break;
  case ID_VHDLBOX:
    rsl << "<>";
    break;

  case ID_VHDLACCESSTYPEDEF:
    rsl << "access type def";
    break;
  case ID_VHDLARRAYTYPEDEF:
    rsl << "array type def";
    break;
  case ID_VHDLENUMERATIONTYPEDEF:
    rsl << "enum type def";
    break;
  case ID_VHDLFILETYPEDEF:
    rsl << "file type def";
    break;
  case ID_VHDLPHYSICALTYPEDEF:
    rsl << "physical type def";
    break;
  case ID_VHDLRECORDTYPEDEF:
    rsl << "record type def";
    break;
  case ID_VHDLSCALARTYPEDEF:
    rsl << "scalar type def";
    break;
  case ID_VHDLTYPEDEF:
    rsl << "?? type def";
    break;
  case ID_VHDLUSECLAUSE:
    rsl << "use clause";
    break;
  case ID_VHDLINCOMPLETETYPEDECL:
    rsl << "incomplete type decl";
    break;
  case ID_VHDLFILEDECL:
    rsl << "file ";
    break;
  case ID_VHDLSIGNALDECL:
    rsl << "signal ";
    break;
  case ID_VHDLVARIABLEDECL:
    rsl << "variable ";
    break;


  case ID_VHDLAGGREGATE:
  case ID_VHDLALIASDECL:
  case ID_VHDLALLOCATOR:
  case ID_VHDLANONYMOUSTYPE:
  case ID_VHDLARCHITECTUREBODY:
  case ID_VHDLARRAYCONSTRAINT:
  case ID_VHDLASSERTIONSTATEMENT:
  case ID_VHDLASSOCELEMENT:
  case ID_VHDLATTRIBUTEDECL:
  case ID_VHDLATTRIBUTESPEC:
  case ID_VHDLBINDINGINDICATION:
  case ID_VHDLBLOCKCONFIGURATION:
  case ID_VHDLBLOCKGENERICS:
  case ID_VHDLBLOCKPORTS:
  case ID_VHDLBLOCKSTATEMENT:
  case ID_VHDLCASESTATEMENT:
  case ID_VHDLCASESTATEMENTALTERNATIVE:
  case ID_VHDLCOMMENTNODE:
  case ID_VHDLCOMPONENTCONFIGURATION:
  case ID_VHDLCOMPONENTDECL:
  case ID_VHDLCOMPONENTINSTANTIATIONSTATEMENT:
  case ID_VHDLCOMPONENTSPEC:
  case ID_VHDLCOMPOSITEVALUE:
  case ID_VHDLCONDITIONALSIGNALASSIGNMENT:
  case ID_VHDLCONDITIONALWAVEFORM:
  case ID_VHDLCONFIGURATIONDECL:
  case ID_VHDLCONFIGURATIONITEM:
  case ID_VHDLCONFIGURATIONSPEC:
  case ID_VHDLCONSTANTDECL:
  case ID_VHDLCONSTRAINT:
  case ID_VHDLDATAFLOW:
  case ID_VHDLDECLARATION:
  case ID_VHDLDELAYMECHANISM:
  case ID_VHDLDESIGNATOR:
  case ID_VHDLDESIGNUNIT:
  case ID_VHDLDISCONNECTIONSPEC:
  case ID_VHDLDISCRETERANGE:
  case ID_VHDLDOUBLE:
  case ID_VHDLEDGE:
  case ID_VHDLELEMENTASSOC:
  case ID_VHDLELEMENTDECL:
  case ID_VHDLELSIF:
  case ID_VHDLENTITYASPECT:
  case ID_VHDLENTITYCLASSENTRY:
  case ID_VHDLENTITYDECL:
  case ID_VHDLENTITYSPEC:
  case ID_VHDLENUM:
  case ID_VHDLEVENT:
  case ID_VHDLEXITSTATEMENT:
  case ID_VHDLEXPLICITSUBTYPEINDICATION:
  case ID_VHDLEXPRESSION:
  case ID_VHDLFILEOPENINFO:
  case ID_VHDLFORSCHEME:
  case ID_VHDLFULLTYPEDECL:
  case ID_VHDLFUNCTIONSPEC:
  case ID_VHDLGENERATESTATEMENT:
  case ID_VHDLGOTOAREA:
  case ID_VHDLGROUPDECL:
  case ID_VHDLGROUPTEMPLATEDECL:
  case ID_VHDLGUARDEDSIGNALSPEC:
  case ID_VHDLIFSCHEME:
  case ID_VHDLIFSTATEMENT:
  case ID_VHDLINERTIALDELAY:
  case ID_VHDLINSTANTIATEDUNIT:
  case ID_VHDLINT:
  case ID_VHDLINTERFACEDECL:
  case ID_VHDLITERSCHEME:
  case ID_VHDLLIBRARY:
  case ID_VHDLLIBRARYCLAUSE:
  case ID_VHDLLITERAL:
  case ID_VHDLLOOPSTATEMENT:
  case ID_VHDLNAME:
  case ID_VHDLNEXTSTATEMENT:
  case ID_VHDLNODE:
  case ID_VHDLNONCONST:
  case ID_VHDLNONCONSTBIT:
  case ID_VHDLNULLSTATEMENT:
  case ID_VHDLOPTIONS:
  case ID_VHDLPACKAGEBODY:
  case ID_VHDLPACKAGEDECL:
  case ID_VHDLPHYSICALLITERAL:
  case ID_VHDLPHYSICALUNITDECL:
  case ID_VHDLPRIMARYUNIT:
  case ID_VHDLPROCEDURECALLSTATEMENT:
  case ID_VHDLPROCEDURESPEC:
  case ID_VHDLPROCESSSTATEMENT:
  case ID_VHDLQUALIFIEDEXPRESSION:
  case ID_VHDLRANGE:
  case ID_VHDLRANGECONSTRAINT:
  case ID_VHDLRECORDCONSTRAINT:
  case ID_VHDLREPORTSTATEMENT:
  case ID_VHDLRETURNSTATEMENT:
  case ID_VHDLSCOPE:
  case ID_VHDLSECONDARYUNIT:
  case ID_VHDLSELECTEDSIGNALASSIGNMENT:
  case ID_VHDLSELECTEDWAVEFORM:
  case ID_VHDLSIGNALASSIGNMENTSTATEMENT:
  case ID_VHDLSIGNATURE:
  case ID_VHDLSIGNATUREDNAME:
  case ID_VHDLSPECIFICATION:
  case ID_VHDLSTATEMENT:
  case ID_VHDLSUBPROGRAMBODY:
  case ID_VHDLSUBPROGRAMDECL:
  case ID_VHDLSUBTYPEDECL:
  case ID_VHDLSUBTYPEINDICATION:
  case ID_VHDLTRANSPORT:
  case ID_VHDLTREENODE:
  case ID_VHDLUNAFFECTED:
  case ID_VHDLUNIVERSALINTEGER:
  case ID_VHDLUNIVERSALREAL:
  case ID_VHDLVALUE:
  case ID_VHDLVARIABLEASSIGNMENTSTATEMENT:
  case ID_VHDLWAITSTATEMENT:
  case ID_VHDLWAVEFORMELEMENT:
  case ID_VHDLWHILESCHEME:
  case ID_VHDLWRITE:
  case ID_VHDLZVALUE:
  case ID_VHDL_FILE:
    break;
  default:
    break;
  }
  return rsl;
}


VhdlExpression *getWaveform(Array *waveform)
{
  uassert(waveform && waveform->Size() > 0);
  if (waveform->Size() > 1) {
    VHVMSG(ignoring, (VhdlTreeNode *) waveform->GetFirst(), "extra waveform elements");
  }    
  VhdlExpression *waveform_elem = (VhdlExpression *) waveform->GetFirst();
  VhdlExpression *e;
  if (waveform_elem->GetClassId() == ID_VHDLWAVEFORMELEMENT) {
    e = ((VhdlWaveformElement *) waveform_elem)->GetValue();
  } else {
    e = waveform_elem;
  }
  return e;
}


bool matchExpr(VhdlDiscreteRange* l, VhdlDiscreteRange* r)
{
  if (!l && !r) {
    return true;
  } else if (!l || !r) {
    return false;
  } else if (l->GetClassId() != r->GetClassId()) {
    return false;
  }

  switch (l->GetClassId()) {
  case ID_VHDLIDREF: {
    VhdlIdRef* lid = static_cast<VhdlIdRef*>(l);
    VhdlIdRef* rid = static_cast<VhdlIdRef*>(r);
    return (lid->GetId() == rid->GetId());
  }
  case ID_VHDLINTEGER: {
    VhdlInteger* lint = static_cast<VhdlInteger*>(l);
    VhdlInteger* rint = static_cast<VhdlInteger*>(r);
    return (lint->GetValue() == rint->GetValue());
  }
  case ID_VHDLOPERATOR: {
    VhdlOperator* lop = static_cast<VhdlOperator*>(l);
    VhdlOperator* rop = static_cast<VhdlOperator*>(r);
    return (lop->GetOperator() == rop->GetOperator() &&
	    matchExpr(lop->GetLeftExpression(), rop->GetLeftExpression()) &&
	    matchExpr(lop->GetRightExpression(), rop->GetRightExpression()));
  }
  case ID_VHDLINDEXEDNAME: {
    VhdlIndexedName* lin = static_cast<VhdlIndexedName*>(l);
    VhdlIndexedName* rin = static_cast<VhdlIndexedName*>(r);
    if (!matchExpr(lin->GetPrefix(), rin->GetPrefix())) {
      return false;
    }
    Array* la = lin->GetAssocList();
    Array* ra = rin->GetAssocList();
    if (la->Size() != ra->Size()) {
      return false;
    }
    int i;
    VhdlDiscreteRange* ldr;
    FOREACH_ARRAY_ITEM(la, i, ldr) {
      VhdlDiscreteRange* rdr = static_cast<VhdlDiscreteRange*>(ra->At(i));
      VhdlExpression* le = static_cast<VhdlExpression*>(ldr);
      VhdlExpression* re = static_cast<VhdlExpression*>(rdr);
      VhdlRange* lr = static_cast<VhdlRange*>(ldr);
      VhdlRange* rr = static_cast<VhdlRange*>(rdr);
      if (le && re) {
	if (!matchExpr(le, re)) {
	  return false;
	}
      } else if (lr && rr) {
	return (matchExpr(lr->GetLeftExpression(), rr->GetLeftExpression()) &&
		lr->GetDir() == rr->GetDir() &&
		matchExpr(lr->GetRightExpression(), rr->GetRightExpression()));
      } else {
	return false;
      }
    }
    return true;
  }
  case ID_VHDLQUALIFIEDEXPRESSION: {
    VhdlQualifiedExpression* ql = static_cast<VhdlQualifiedExpression*>(l);
    VhdlQualifiedExpression* qr = static_cast<VhdlQualifiedExpression*>(r);
    return (matchExpr(ql->GetPrefix(), qr->GetPrefix()) &&
	    matchExpr(ql->GetAggregate(), qr->GetAggregate()));
  }
  case ID_VHDLATTRIBUTENAME: {
    VhdlAttributeName* al = static_cast<VhdlAttributeName*>(l);
    VhdlAttributeName* ar = static_cast<VhdlAttributeName*>(r);
    return (matchExpr(al->GetPrefix(), ar->GetPrefix()) &&
	    al->GetAttributeEnum() == ar->GetAttributeEnum() &&
	    matchExpr(al->GetExpression(), ar->GetExpression()));
  }
  case ID_VHDLSELECTEDNAME: {
    VhdlSelectedName* sl = static_cast<VhdlSelectedName*>(l);
    VhdlSelectedName* sr = static_cast<VhdlSelectedName*>(r);
    return (matchExpr(sl->GetPrefix(), sr->GetPrefix()) &&
	    matchExpr(sl->GetSuffix(), sr->GetSuffix()));
  }
  case ID_VHDLRANGE: {
    VhdlRange* rl = static_cast<VhdlRange*>(l);
    VhdlRange* rr = static_cast<VhdlRange*>(r);
    return (matchExpr(rl->GetLeftExpression(),  rr->GetLeftExpression()) &&
	    rl->GetDir() == rr->GetDir() &&
	    matchExpr(rl->GetRightExpression(), rr->GetRightExpression()));
  }
  // add more cases as needed
  default:
    uprintf("matchExpr: unsupported case:\n");
#ifdef DEV_PRINT
    dump_node(l);
#endif
    break;
  }
  return false;
}

bool evalInteger(VhdlExpression* expr, int& val)
{
  if (!expr) {
    return false;
  }
  switch (expr->GetClassId()) {
  case ID_VHDLINTEGER: {
    VhdlInteger* i = static_cast<VhdlInteger*>(expr);
    val = (int) i->GetValue();
    return true;
  }
  case ID_VHDLOPERATOR: {
    if (isOperatorFunction(expr)) {
      return false;
    }
    VhdlOperator* op = static_cast<VhdlOperator*>(expr);
    int lval, rval;
    if (!evalInteger(op->GetLeftExpression(), lval)) {
      return false;
    }
    if (op->GetRightExpression()) {
      if (!evalInteger(op->GetRightExpression(), rval)) {
	return false;
      }
      switch (op->GetOperatorToken()) {
      case VHDL_PLUS:
	val = lval + rval;
	return true;
      case VHDL_MINUS:
	val = lval - rval;
	return true;
      case VHDL_STAR:
	val = lval * rval;
	return true;
      case VHDL_SLASH:
	if (rval == 0) {
	  return false;
	}
	val = lval / rval;
	return true;
      case VHDL_rem:
	// mod is a different matter...
	if (rval == 0) {
	  return false;
	}
	val = lval % rval;
	return true;
      case VHDL_EXPONENT:
	if (rval < 0) {
	  return false;
	}
	val = 1;
	for (int i = 0; i < rval; i++) {
	  val *= lval;
	}
	return true;
      case VHDL_EQUAL:
        val = lval == rval;
	return true;
      case VHDL_NEQUAL:
        val = lval != rval;
	return true;
      case VHDL_STHAN:
        val = lval < rval;
	return true;
      case VHDL_SEQUAL:
        val = lval <= rval;
	return true;
      case VHDL_GTHAN:
        val = lval > rval;
	return true;
      case VHDL_GEQUAL:
        val = lval == rval;
	return true;
      default:
	return false;
      }
    } else {
      switch (op->GetOperatorToken()) {
      case VHDL_PLUS:
	val = lval;
	return true;
      case VHDL_UMINUS:
	val = -lval;
	return true;
      case VHDL_not:
	val = !lval;
	return true;
      case VHDL_abs:
	val = lval >= 0 ? lval : -lval;
	return true;
      default:
	return false;
      }
    }
  }
  default:
    return false;
  }
}

bool evalRangeLength(VhdlDiscreteRange* r, int& val)
{
  int lval, rval;
  if (evalInteger(r->GetLeftExpression(), lval) &&
      evalInteger(r->GetRightExpression(), rval)) {
    if (r->GetDir() == VHDL_to) {
      if (lval <= rval) {
	val = rval - lval + 1;
	return true;
      }
    } else {
      if (lval >= rval) {
	val = lval - rval + 1;
	return true;
      }
    }
  }
  return false;
}



void dump_node(VhdlTreeNode* n)
{
  if (!n) {
    printf("Node: <nil>\n");
    return;
  }
  // Parse the type: N7Verific10VhdlTypeIdE
  int len1, len2;
  char typeBuf[256];
  sscanf(typeid(*n).name(), "N%dVerific%d%s", &len1, &len2, typeBuf);
  typeBuf[len2] = '\0';  

  printf("Node: (%s*) %p", typeBuf, n);
  linefile_type lf = n->Linefile();
  printf("\tline %d in file %s\n", LineFile::GetLineNo(lf), LineFile::GetFileName(lf));
  n->PrettyPrint(std::cout, 0);
  std::cout << "\n";
}
