#include "UHash"
#include "Verific.h"
#include <stdio.h>
#include <string.h>
#include <typeinfo>

void dump_node(void *n)
{
  Verific::VhdlTreeNode *vtn = (Verific::VhdlTreeNode *) n;
  printf("Node: %p, type = %s\n", vtn, typeid(*vtn).name());
}


/*static*/void dump_array(Verific::Array *a)
{
  printf("Array %p: size = %d\n", a, a->Size());
  for (unsigned int i = 0; i < a->Size(); i++) {
    printf("%d: %p", i, a->At(i));
    printf("\t");
    dump_node(a->At(i));
  }
}

typedef UHashMap<Verific::VhdlIdDef*, Verific::VhdlDeclaration*> IdDeclMap;

class MyVisitor : public Verific::VhdlVisitor
{
public:
  MyVisitor(IdDeclMap *map)
    : VhdlVisitor(), d_map(map)
  {
  }
  ~MyVisitor()
  {
  }
  void doit(Verific::VhdlDeclaration *decl)
  {
    Verific::Array *ids = decl->GetIds();
    int i;
    Verific::VhdlIdDef *id;

    switch (decl->GetClassId()) {
    case Verific::ID_VHDLFULLTYPEDECL:
      id = ((Verific::VhdlFullTypeDecl *) decl)->GetId();
      printf("full type decl: '%s'\n", id->Name());
      printf("map['%s'] = '%p'\n", id->Name(), decl);
      (*d_map)[id] = decl;
      break;
    case Verific::ID_VHDLSUBTYPEDECL:
      id = ((Verific::VhdlSubtypeDecl *) decl)->GetId();
      printf("subtype decl: '%s'\n", id->Name());
      printf("map['%s'] = '%p'\n", id->Name(), decl);
      (*d_map)[id] = decl;
      break;
    case Verific::ID_VHDLALIASDECL:
      id = ((Verific::VhdlAliasDecl *) decl)->GetDesignator();
      printf("subtype decl: '%s'\n", id->Name());
      printf("map['%s'] = '%p'\n", id->Name(), decl);
      (*d_map)[id] = decl;
      break;
    case Verific::ID_VHDLATTRIBUTEDECL:
      id = ((Verific::VhdlAttributeDecl *) decl)->GetId();
      printf("attribute decl: '%s'\n", id->Name());
      printf("map['%s'] = '%p'\n", id->Name(), decl);
      (*d_map)[id] = decl;
      break;
    case Verific::ID_VHDLCOMPONENTDECL:
      id = ((Verific::VhdlComponentDecl *) decl)->GetId();
      printf("attribute decl: '%s'\n", id->Name());
      printf("map['%s'] = '%p'\n", id->Name(), decl);
      (*d_map)[id] = decl;
      break;
    default:
      FOREACH_ARRAY_ITEM(ids, i, id) {
	printf("map['%s'] = '%p'\n", id->Name(), decl);
	(*d_map)[id] = decl;
      }
      break;
    }
  }

  void Visit(Verific::VhdlDeclaration &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlDeclaration &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlTypeDef &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlTypeDef &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlScalarTypeDef &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlScalarTypeDef &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlArrayTypeDef &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlArrayTypeDef &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlRecordTypeDef &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlRecordTypeDef &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlAccessTypeDef &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlAccessTypeDef &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlFileTypeDef &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlFileTypeDef &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlEnumerationTypeDef &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlEnumerationTypeDef &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlPhysicalTypeDef &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlPhysicalTypeDef &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlElementDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlElementDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlPhysicalUnitDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlPhysicalUnitDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlUseClause &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlUseClause &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlLibraryClause &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlLibraryClause &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlInterfaceDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlInterfaceDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlSubprogramDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlSubprogramDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlSubprogramBody &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlSubprogramBody &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlSubtypeDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlSubtypeDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlFullTypeDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlFullTypeDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlIncompleteTypeDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlIncompleteTypeDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlConstantDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlConstantDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlSignalDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlSignalDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlVariableDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlVariableDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlFileDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlFileDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlAliasDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlAliasDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlComponentDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlComponentDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlAttributeDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlAttributeDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlAttributeSpec &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlAttributeSpec &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlConfigurationSpec &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlConfigurationSpec &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlDisconnectionSpec &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlDisconnectionSpec &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlGroupTemplateDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlGroupTemplateDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }
  void Visit(Verific::VhdlGroupDecl &decl)
  {
    printf("Called %s\n", "  void Visit(Verific::VhdlGroupDecl &decl)");
    doit((Verific::VhdlDeclaration *) &decl);
  }

private:
  IdDeclMap *d_map;
};

class StrBuf
{
 public:
  StrBuf();
  ~StrBuf();
  void add(const char *);
  void add(char);
  void addnum(int);
  void addnum(long long int);
  const char *str();
  int length();
  void clear();
 private:
  void resize(int);
  char *d_buf;
  int d_capacity;
  int d_strlen;
};

class Pretty
{
 public:
  Pretty();
  ~Pretty();

  void put(const char *);
  void put(int);
  void put(long long);
  void put(double);
  void nl();
  void indent() {d_indent_level++;}
  void deindent() {d_indent_level--;}
  void comment(const char *);

  void unimplemented(const char *, Verific::VhdlTreeNode *);

  void lib(Verific::VhdlLibrary *);

  void unit(Verific::VhdlDesignUnit *);
  void unit(Verific::VhdlPackageDecl *);
  void unit(Verific::VhdlPackageBody *);
  void unit(Verific::VhdlEntityDecl *);
  void unit(Verific::VhdlArchitectureBody *);
  void unit(Verific::VhdlConfigurationDecl *);

  void decls(Verific::Array *);
  void decl(Verific::VhdlDeclaration *);
  void decl(Verific::VhdlConstantDecl *);
  void decl(Verific::VhdlSignalDecl *);
  void decl(Verific::VhdlVariableDecl *);
  void decl(Verific::VhdlFileDecl *);
  void decl(Verific::VhdlFullTypeDecl *);
  void decl(Verific::VhdlPhysicalUnitDecl *);
  void decl(Verific::VhdlElementDecl *);
  void decl(Verific::VhdlSubtypeDecl *);
  void decl(Verific::VhdlAttributeDecl *);
  void decl(Verific::VhdlAttributeSpec *);
  void decl(Verific::VhdlSubprogramDecl *);
  void decl(Verific::VhdlSubprogramBody *);
  void decl(Verific::VhdlInterfaceDecl *);
  void decl(Verific::VhdlAliasDecl *);
  void decl(Verific::VhdlUseClause *);
  void decl(Verific::VhdlLibraryClause *);

  void kind(unsigned int);
  void mode(unsigned int);

  void spec(Verific::VhdlEntitySpec *);

  void subp(Verific::VhdlSpecification *);
  void subp(Verific::VhdlFunctionSpec *);
  void subp(Verific::VhdlProcedureSpec *);
  void subp_formals(Verific::Array *);

  void entity_class(unsigned int);

  void pretty_object_decl(Verific::VhdlDeclaration *);
  void pretty_ids(Verific::Array *);
  
  void stmts(Verific::Array *);

  void stmt(Verific::VhdlStatement *);
  void stmt(Verific::VhdlLoopStatement *);
  void stmt(Verific::VhdlCaseStatement *);
  void stmt(Verific::VhdlIfStatement *);
  void stmt(Verific::VhdlSignalAssignmentStatement *);
  void stmt(Verific::VhdlWaitStatement *);
  void stmt(Verific::VhdlAssertionStatement *);
  void stmt(Verific::VhdlProcessStatement *);
  void stmt(Verific::VhdlBlockStatement *);
  void stmt(Verific::VhdlGenerateStatement *);
  void stmt(Verific::VhdlComponentInstantiationStatement *);
  void stmt(Verific::VhdlProcedureCallStatement *);
  void stmt(Verific::VhdlConditionalSignalAssignment *);
  void stmt(Verific::VhdlSelectedSignalAssignment *);
  void stmt(Verific::VhdlVariableAssignmentStatement *);

  void waveforms(Verific::Array *);

  void exprs(Verific::Array *);
  void expr(Verific::VhdlDiscreteRange *);
  void expr(Verific::VhdlAttributeName *);
  void expr(Verific::VhdlOperator *);
  void expr(Verific::VhdlIdRef *);
  void expr(Verific::VhdlIdDef *);
  void expr(Verific::VhdlExplicitSubtypeIndication *);
  void expr(Verific::VhdlIndexedName *);
  void expr(Verific::VhdlSelectedName *);
  void expr(Verific::VhdlAggregate *);
  void expr(Verific::VhdlElementAssoc *);
  void expr(Verific::VhdlWaveformElement *);

  void choices(Verific::Array *);

  void type(Verific::VhdlTypeDef *);

  static void enter_comment();
  static void leave_comment();

 private:
  void do_indent();
  StrBuf d_put;
  StrBuf d_comment;
  int d_indent_level;
  int d_pos;
  FILE *d_fp;
  static int d_comment_level;
};

class Comment
{
 public:
  Comment() { Pretty::enter_comment(); }
  ~Comment() { Pretty::leave_comment(); }
};


#define UNIMP(v) unimplemented(__PRETTY_FUNCTION__, v)

StrBuf::StrBuf()
{
  const int init_size = 4;
  d_buf = new char [init_size];
  d_buf[0] = '\0';
  d_capacity = init_size;
  d_strlen = 0;
}

StrBuf::~StrBuf()
{
  delete [] d_buf;
}

void StrBuf::resize(int size)
{
  if (size >= d_capacity) {
    while (size >= d_capacity) {
      d_capacity *= 2;
    }
    char *new_buf = new char [d_capacity];
    strcpy(new_buf, d_buf);
    delete [] d_buf;
    d_buf = new_buf;
  }
}

void StrBuf::add(const char *s)
{
  int len = strlen(s);
  resize(len + d_strlen);
  strcpy(&d_buf[d_strlen], s);
  d_strlen += len;
}

void StrBuf::add(char c)
{
  char buf[2];
  buf[0] = c;
  buf[1] = '\0';
  add(buf);
}

void StrBuf::addnum(int i)
{
  char buf[30];
  sprintf(buf, "%d", i);
  add(buf);
}

void StrBuf::addnum(long long i)
{
  char buf[60];
  sprintf(buf, "%lld", i);
  add(buf);
}

const char *StrBuf::str()
{
  return d_buf;
}

int StrBuf::length()
{
  return d_strlen;
}

void StrBuf::clear()
{
  d_buf[0] = '\0';
  d_strlen = 0;
}

int Pretty::d_comment_level = 0;

Pretty::Pretty()
: d_indent_level(0), d_fp(stdout)
{
  
}

Pretty::~Pretty()
{
  if (d_fp != NULL) {
    fclose(d_fp);
    d_fp = NULL;
  }
}

void Pretty::nl()
{
  if (d_put.length() > 0) {
    fprintf(d_fp, "%s", d_put.str());
    if (d_comment.length() > 0) {
      fprintf(d_fp, " -- %s", d_comment.str());
    }
  }
  else if (d_comment.length() > 0) {
    fprintf(d_fp, "-- %s", d_comment.str());
  }
  fprintf(d_fp, "\n");
  d_put.clear();
  d_comment.clear();
}

void Pretty::do_indent()
{
  const char *indent_str = "  ";
  if (d_put.length() == 0) {
    for (int i = 1; i <= d_indent_level; i++) {
      d_put.add(indent_str);
    }
  }
}

void Pretty::enter_comment()
{
  d_comment_level++;
}

void Pretty::leave_comment()
{
  d_comment_level--;
}

void Pretty::put(const char *s)
{
  if (d_put.length() + d_comment.length() + strlen(s) > 78) {
    nl();
  }
  do_indent();
  if (d_comment_level > 0) {
    d_comment.add(s);
  }
  else {
    d_put.add(s);
  }
}

void Pretty::put(int i)
{
  char buf[30];
  sprintf(buf, "%d", i);
  put(buf);
}

void Pretty::put(long long i)
{
  char buf[40];
  sprintf(buf, "%lld", i);
  put(buf);
}

void Pretty::put(double d)
{
  char buf[30];
  sprintf(buf, "%10.5g", d);
  put(buf);
}

void Pretty::comment(const char *s)
{
  d_comment.add(s);
}

void Pretty::unimplemented(const char *fn, Verific::VhdlTreeNode *v)
{
  printf("Unimplemented: %s\n", fn);
  printf("\t");
  if (v != 0) {
    dump_node(v);
  }
}

void Pretty::lib(Verific::VhdlLibrary *l)
{
  comment("Contents of library ");
  comment(l->Name());
  nl();

  Verific::MapIter prim_iter;
  Verific::VhdlPrimaryUnit *u;
  FOREACH_VHDL_PRIMARY_UNIT(l, prim_iter, u) {
    unit(u);

    Verific::MapIter sec_iter;
    Verific::VhdlSecondaryUnit *s;
    FOREACH_VHDL_SECONDARY_UNIT(u, sec_iter, s) {
      unit(s);
    }
  }
}


void Pretty::unit(Verific::VhdlDesignUnit *du)
{
  decls(du->GetContextClause());

  switch (du->GetClassId()) {
  case Verific::ID_VHDLPACKAGEDECL:
    unit((Verific::VhdlPackageDecl *) du);
    break;
  case Verific::ID_VHDLPACKAGEBODY:
    unit((Verific::VhdlPackageBody *) du);
    break;
  case Verific::ID_VHDLENTITYDECL:
    unit((Verific::VhdlEntityDecl *) du);
    break;
  case Verific::ID_VHDLARCHITECTUREBODY:
    unit((Verific::VhdlArchitectureBody *) du);
    break;
  case Verific::ID_VHDLCONFIGURATIONDECL:
    unit((Verific::VhdlConfigurationDecl *) du);
    break;
  default:
    UNIMP(du);
    assert(0);
    break;
  }
}

void Pretty::unit(Verific::VhdlPackageDecl *du)
{
  put("package ");
  put(du->Name());
  put( " is");
  nl();
  indent();
  decls(du->GetDeclPart());
  deindent();
  put("end;");
  nl();
}

void Pretty::unit(Verific::VhdlPackageBody *du)
{
  put("package body ");
  put(du->Name());
  put( " is");
  nl();
  indent();
  decls(du->GetDeclPart());
  deindent();
  put("end;");
  nl();
}


void Pretty::unit(Verific::VhdlEntityDecl *du)
{
  put("entity ");
  expr(du->Id());
  put(" is");
  nl();
  indent();
  decls(du->GetDeclPart());
  deindent();
  if (du->GetStatementPart()) {
    put("begin");
    nl();
    indent();
    stmts(du->GetStatementPart());
    deindent();
  }
  put("end;");
  nl();
}

void Pretty::unit(Verific::VhdlArchitectureBody *du)
{
  put("architecture ");
  expr(du->Id());
  put(" of ");
  expr(du->GetEntityName());
  put(" is");
  nl();
  indent();
  decls(du->GetDeclPart());
  deindent();
  put("begin");
  nl();
  indent();
  stmts(du->GetStatementPart());
  deindent();
  put("end;");
  nl();
}

void Pretty::unit(Verific::VhdlConfigurationDecl *du)
{
  UNIMP(du);
}


void Pretty::decls(Verific::Array *a)
{
  if (! a) {
    return;
  }
  int i;
  Verific::VhdlDeclaration *n;
  FOREACH_ARRAY_ITEM(a, i, n) {
    decl(n);
  }
}


void Pretty::decl(Verific::VhdlDeclaration *d)
{
  switch (d->GetClassId()) {
  case Verific::ID_VHDLCONSTANTDECL:
    decl((Verific::VhdlConstantDecl *) d);
    break;
  case Verific::ID_VHDLSIGNALDECL:
    decl((Verific::VhdlSignalDecl *) d);
    break;
  case Verific::ID_VHDLVARIABLEDECL:
    decl((Verific::VhdlVariableDecl *) d);
    break;
  case Verific::ID_VHDLFILEDECL:
    decl((Verific::VhdlFileDecl *) d);
    break;
  case Verific::ID_VHDLFULLTYPEDECL:
    decl((Verific::VhdlFullTypeDecl *) d);
    break;
  case Verific::ID_VHDLPHYSICALUNITDECL:
    decl((Verific::VhdlPhysicalUnitDecl *) d);
    break;
  case Verific::ID_VHDLELEMENTDECL:
    decl((Verific::VhdlElementDecl *) d);
    break;
  case Verific::ID_VHDLSUBTYPEDECL:
    decl((Verific::VhdlSubtypeDecl *) d);
    break;
  case Verific::ID_VHDLATTRIBUTEDECL:
    decl((Verific::VhdlAttributeDecl *) d);
    break;
  case Verific::ID_VHDLATTRIBUTESPEC:
    decl((Verific::VhdlAttributeSpec *) d);
    break;
  case Verific::ID_VHDLSUBPROGRAMDECL:
    decl((Verific::VhdlSubprogramDecl *) d);
    break;
  case Verific::ID_VHDLSUBPROGRAMBODY:
    decl((Verific::VhdlSubprogramBody *) d);
    break;
  case Verific::ID_VHDLINTERFACEDECL:
    decl((Verific::VhdlInterfaceDecl *) d);
    break;
  case Verific::ID_VHDLALIASDECL:
    decl((Verific::VhdlAliasDecl *) d);    
    break;
  case Verific::ID_VHDLLIBRARYCLAUSE:
    decl((Verific::VhdlLibraryClause *) d);
    break;
  case Verific::ID_VHDLUSECLAUSE:
    decl((Verific::VhdlUseClause *) d);
    break;
  default:
    UNIMP(d);
    break;
  }
}

void Pretty::decl(Verific::VhdlConstantDecl *d)
{
  put("constant ");
  pretty_ids(d->GetIds());
  put(" : ");
  expr(d->GetSubtypeIndication());
  if (d->GetInitAssign()) {
    put(" := ");
    expr(d->GetInitAssign());
  }
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlSignalDecl *d)
{
  put("signal ");
  pretty_ids(d->GetIds());
  put(" : ");
  expr(d->GetSubtypeIndication());
  if (d->GetInitAssign()) {
    put(" := ");
    expr(d->GetInitAssign());
  }
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlVariableDecl *d)
{
  put("variable ");
  pretty_ids(d->GetIds());
  put(" : ");
  expr(d->GetSubtypeIndication());
  if (d->GetInitAssign()) {
    put(" := ");
    expr(d->GetInitAssign());
  }
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlFileDecl *d)
{
  put("file ");
  pretty_ids(d->GetIds());
  put(" : ");
  expr(d->GetSubtypeIndication());
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlFullTypeDecl *d)
{
  put("type ");
  expr(d->GetId());
  put(" is ");
  type(d->GetTypeDef());
  put(";");  
  nl();
}

void Pretty::decl(Verific::VhdlPhysicalUnitDecl *d)
{
  expr(d->GetId());
  if (d->GetPhysicalLiteral()) {
    put(" = ");
    expr(d->GetPhysicalLiteral());
  }
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlElementDecl *d)
{
  pretty_ids(d->GetIds());
  put(" : ");
  expr(d->GetSubtypeIndication());
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlSubtypeDecl *d)
{
  put("subtype ");
  expr(d->GetId());
  put(" is ");
  expr(d->GetSubtypeIndication());
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlAttributeDecl *d)
{
  put("attribute ");
  expr(d->GetId());
  put(" : ");
  expr(d->GetTypeMark());
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlAttributeSpec *d)
{
  put("attribute ");
  expr(d->GetDesignator());
  put(" of ");
  spec((Verific::VhdlEntitySpec *) d->GetEntitySpec());
  put(" is ");
  expr(d->GetValue());
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlSubprogramDecl *d)
{
  subp((Verific::VhdlSpecification *) d->GetSubprogramSpec());
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlSubprogramBody *d)
{
  subp((Verific::VhdlSpecification *) d->GetSubprogramSpec());
  put(" is");
  nl();
  indent();
  decls(d->GetDeclPart());
  deindent();
  put("begin");
  nl();
  indent();
  stmts(d->GetStatementPart());
  deindent();
  put("end;");
  nl();
}

void Pretty::decl(Verific::VhdlInterfaceDecl *d)
{
  kind(d->GetInterfaceKind());
  pretty_ids(d->GetIds());
  put(" : ");
  mode(d->GetMode());
  expr(d->GetSubtypeIndication());
  if (d->GetInitAssign()) {
    put(" := ");
    expr(d->GetInitAssign());
  }
}

void Pretty::decl(Verific::VhdlAliasDecl *d)
{
  put("alias ");
  expr(d->GetDesignator());
  if (d->GetSubtypeIndication()) {
    put(" : ");
    expr(d->GetSubtypeIndication());
  }
  put(" is ");
  expr(d->GetAliasTargetName());
  put(";");
  nl();
}

void Pretty::decl(Verific::VhdlUseClause *d)
{
  if (d->IsImplicit()) {
    enter_comment();
  }
  put("use ");
  exprs(d->GetSelectedNameList());
  put(";");
  if (d->IsImplicit()) {
    leave_comment();
  }
  nl();
}

void Pretty::decl(Verific::VhdlLibraryClause *d)
{
  if (d->IsImplicit()) {
    enter_comment();
  }
  put("library ");
  pretty_ids(d->GetIds());
  put(";");
  if (d->IsImplicit()) {
    leave_comment();
  }
  nl();
}

void Pretty::kind(unsigned int k)
{
  switch (k) {
  case VHDL_constant:
    put("constant ");
    break;
  case VHDL_signal:
    put("signal ");
    break;
  case VHDL_variable:
    put("variable ");
    break;
  case VHDL_file:
    put("file ");
    break;
  case 0:
    break;
  default:
    UNIMP(0);
    put((int) k);
    break;
  }
}

void Pretty::mode(unsigned int dir)
{
  switch (dir) {
  case VHDL_in: 
    put("in ");
    break;
  case VHDL_out: 
    put("out ");
    break;
  case VHDL_inout: 
    put("inout ");
    break;
  case VHDL_buffer: 
    put("buffer ");
    break;
  case VHDL_linkage: 
    put("linkage ");
    break;
  case 0:
    break;
  default:
    UNIMP(0);
    put((int) dir);
    break;
  }
}




void Pretty::entity_class(unsigned int i)
{
  switch (i) {
  case VHDL_variable:
    put("variable");
    break;
  case VHDL_signal:
    put("signal");
    break;
  case VHDL_constant:
    put("constant");
    break;
  case VHDL_subtype:
    put("subtype");
    break;
  case VHDL_type:;
    put("type");
    break;
  default:
    put((int) i);
    break;
  }
}


void Pretty::spec(Verific::VhdlEntitySpec *s)
{
  Verific::Array *a = s->GetEntityNameList();
  int i;
  Verific::VhdlName *n;
  FOREACH_ARRAY_ITEM(a, i, n) {
    if (i > 0) {
      put(", ");
    }
    expr(n);
  }
  put(" : ");
  entity_class(s->GetEntityClass());
}

void Pretty::subp(Verific::VhdlSpecification *s)
{
  switch (s->GetClassId()) {
  case Verific::ID_VHDLFUNCTIONSPEC:
    subp((Verific::VhdlFunctionSpec *) s);
    break;
  case Verific::ID_VHDLPROCEDURESPEC:
    subp((Verific::VhdlProcedureSpec *) s);
    break;
  default:
    dump_node(s);
    assert(0);
    break;
  }
}

void Pretty::subp(Verific::VhdlFunctionSpec *f)
{
  if (f->IsImpure()) {
    put("impure ");
  }
  put("function ");
  expr(f->GetId());
  subp_formals(f->GetFormalParamList());
  put(" return ");
  expr(f->GetReturnType());
}

void Pretty::subp(Verific::VhdlProcedureSpec *p)
{
  put("procedure ");
  expr(p->GetId());
  subp_formals(p->GetFormalParamList());
}

void Pretty::subp_formals(Verific::Array *a)
{
  if (! a || a->Size() == 0) {
    return;
  }
  put("(");
  int i;
  Verific::VhdlInterfaceDecl *d;
  FOREACH_ARRAY_ITEM(a, i, d) {
    if (i > 0) {
      put("; ");
    }
    decl(d);
  }
  put(")");
}


void Pretty::pretty_ids(Verific::Array *ids)
{
  int i;
  Verific::VhdlIdDef *id;
  FOREACH_ARRAY_ITEM(ids, i, id) {
    if (i > 0) {
      put(", ");
    }
    put(id->Name());
  }
}


void Pretty::stmts(Verific::Array *a)
{
  int i;
  Verific::VhdlStatement *s;
  FOREACH_ARRAY_ITEM(a, i, s) {
    stmt(s);
  }
}

void Pretty::stmt(Verific::VhdlStatement *s)
{
  if (s->GetLabel()) {
    expr(s->GetLabel());
    put(": ");
  }
  if (s->IsPostponed()) {
    put("postponed ");
  }

  switch (s->GetClassId()) {
  case Verific::ID_VHDLNULLSTATEMENT:
    put("null;");
    nl();
    break;
  case Verific::ID_VHDLRETURNSTATEMENT:
    put("return");
    if (((Verific::VhdlReturnStatement *) s)->GetExpression()) {
      put(" ");
      expr(((Verific::VhdlReturnStatement *) s)->GetExpression());
    }
    put(";");
    nl();
    break;
  case Verific::ID_VHDLEXITSTATEMENT:
    put("exit");
    if (((Verific::VhdlExitStatement *) s)->GetTarget()) {
      put(" ");
      expr(((Verific::VhdlExitStatement *) s)->GetTarget());
    }
    if (((Verific::VhdlExitStatement *) s)->GetCondition()) {
      put(" when ");
      expr(((Verific::VhdlExitStatement *) s)->GetCondition());
    }
    put(";");
    nl();
    break;
  case Verific::ID_VHDLNEXTSTATEMENT:
    put("next");
    if (((Verific::VhdlNextStatement *) s)->GetTarget()) {
      put(" ");
      expr(((Verific::VhdlNextStatement *) s)->GetTarget());
    }
    if (((Verific::VhdlNextStatement *) s)->GetCondition()) {
      put(" when ");
      expr(((Verific::VhdlNextStatement *) s)->GetCondition());
    }
    put(";");
    nl();
    break;
  case Verific::ID_VHDLLOOPSTATEMENT:
    stmt((Verific::VhdlLoopStatement *) s);
    break;
  case Verific::ID_VHDLCASESTATEMENT:
    stmt((Verific::VhdlCaseStatement *) s);
    break;
  case Verific::ID_VHDLIFSTATEMENT:
    stmt((Verific::VhdlIfStatement *) s);
    break;
  case Verific::ID_VHDLSIGNALASSIGNMENTSTATEMENT:
    stmt((Verific::VhdlSignalAssignmentStatement *) s);
    break;
  case Verific::ID_VHDLWAITSTATEMENT:
    stmt((Verific::VhdlWaitStatement *) s);
    break;
  case Verific::ID_VHDLASSERTIONSTATEMENT:
    stmt((Verific::VhdlAssertionStatement *) s);
    break;
  case Verific::ID_VHDLPROCESSSTATEMENT:
    stmt((Verific::VhdlProcessStatement *) s);
    break;
  case Verific::ID_VHDLBLOCKSTATEMENT:
    stmt((Verific::VhdlBlockStatement *) s);
    break;
  case Verific::ID_VHDLGENERATESTATEMENT:
    stmt((Verific::VhdlGenerateStatement *) s);
    break;
  case Verific::ID_VHDLCOMPONENTINSTANTIATIONSTATEMENT:
    stmt((Verific::VhdlComponentInstantiationStatement *) s);
    break;
  case Verific::ID_VHDLPROCEDURECALLSTATEMENT:
    stmt((Verific::VhdlProcedureCallStatement *) s);
    break;
  case Verific::ID_VHDLCONDITIONALSIGNALASSIGNMENT:
    stmt((Verific::VhdlConditionalSignalAssignment *) s);
    break;
  case Verific::ID_VHDLSELECTEDSIGNALASSIGNMENT:
    stmt((Verific::VhdlSelectedSignalAssignment *) s);
    break;
  case Verific::ID_VHDLVARIABLEASSIGNMENTSTATEMENT:
    stmt((Verific::VhdlVariableAssignmentStatement *) s);
    break;
  default:
    UNIMP(s);
    assert(0);
    break;
  }
}

void Pretty::stmt(Verific::VhdlLoopStatement *s)
{
  Verific::VhdlIterScheme *is = s->GetIterScheme();
  switch (is->GetClassId()) {
  case Verific::ID_VHDLWHILESCHEME: {
    Verific::VhdlWhileScheme *ws = (Verific::VhdlWhileScheme *) is;
    put("while ");
    expr(ws->GetCondition());
    put(" loop");
    { Comment c;
      if (ws->GetExitLabel()) {
	put("exit: ");
	expr(ws->GetExitLabel());
      }
      if (ws->GetNextLabel()) {
	put("next: ");
	expr(ws->GetNextLabel());
      }
    }
    nl();
    indent();
    stmts(s->GetStatements());
    deindent();
    put("end loop;");
    nl();
  }
    break;
  case Verific::ID_VHDLFORSCHEME: {
    Verific::VhdlForScheme *fs = (Verific::VhdlForScheme *) is;
    put("for ");
    expr(fs->GetId());
    put(" in ");
    expr(fs->GetRange());
    put(" loop ");
    { Comment c;
      if (fs->GetExitLabel()) {
	put("exit: ");
	expr(fs->GetExitLabel());
      }
      if (fs->GetNextLabel()) {
	put(" next: ");
	expr(fs->GetNextLabel());
      }
    }
    nl();
    indent();
    stmts(s->GetStatements());
    deindent();
    put("end loop;");
    nl();
  }
    break;
  default:
    UNIMP(s);
    break;
  }
}

void Pretty::stmt(Verific::VhdlCaseStatement *s)
{
  put("case ");
  expr(s->GetExpression());
  put(" is");
  nl();
  Verific::Array *a = s->GetAlternatives();
  int i;
  Verific::VhdlCaseStatementAlternative *sa;
  FOREACH_ARRAY_ITEM(a, i, sa) {
    put("when ");
    choices(sa->GetChoices());
    put(" =>");
    nl();
    indent();
    stmts(sa->GetStatements());
    deindent();
  }
  put("end case;");
  nl();
}

void Pretty::stmt(Verific::VhdlIfStatement *s)
{
  put("if ");
  expr(s->GetIfCondition());
  put(" then");
  nl();
  indent();
  stmts(s->GetIfStatements());
  deindent();
  Verific::Array *es = s->GetElsifList();
  if (es) {
    int i;
    Verific::VhdlElsif *eif;
    FOREACH_ARRAY_ITEM(es, i, eif) {
      put("elseif ");
      expr(eif->Condition());
      put(" then");
      nl();
      indent();
      stmts(eif->GetStatements());
      deindent();
    }
  }
  if (s->GetElseStatments()) {
    put("else");
    nl();
    indent();
    stmts(s->GetElseStatments());
    deindent();
  }
  put("end if;");
  nl();
}

void Pretty::stmt(Verific::VhdlSignalAssignmentStatement *s)
{
  expr(s->GetTarget());
  put(" <= ");
  Verific::VhdlDelayMechanism *dm = s->GetDelayMechanism();
  if (dm) {
    if (dm->GetClassId() == Verific::ID_VHDLTRANSPORT) {
      put("transport ");
    }
    else if (dm->GetClassId() == Verific::ID_VHDLINERTIALDELAY) {
      Verific::VhdlInertialDelay *id = (Verific::VhdlInertialDelay *) dm;
      put("inertial ");
      {
	Comment c; 
	expr(id->GetRejectExpression()); 
      }
    }
    else {
      UNIMP(dm);
      assert(0);
    }
  }
  waveforms(s->GetWaveform());
  put(";");
  nl();
}

void Pretty::stmt(Verific::VhdlWaitStatement *s)
{
  UNIMP(s);
}

void Pretty::stmt(Verific::VhdlAssertionStatement *s)
{
  if (s->GetCondition()) {
    put("assert ");
    expr(s->GetCondition());
    nl();
    indent();
  }
  if (s->GetReport()) {
    put("report ");
    expr(s->GetReport());
    nl();
  }
  if (s->GetSeverity()) {
    put("severity ");
    expr(s->GetSeverity());
  }
  put(";");
  if (s->GetCondition()) {
    deindent();
  }
  nl();
}

void Pretty::stmt(Verific::VhdlProcessStatement *s)
{
  put("process");
  if (s->GetSensitivityList()) {
    put("(");
    exprs(s->GetSensitivityList());
    put(")");
  }
  nl();
  indent();
  decls(s->GetDeclPart());
  deindent();
  put("begin");
  nl();
  indent();
  stmts(s->GetStatementPart());
  deindent();
  put("end process;");
  nl();
}

void Pretty::stmt(Verific::VhdlBlockStatement *s)
{
  UNIMP(s);
}

void Pretty::stmt(Verific::VhdlGenerateStatement *s)
{
  UNIMP(s);
}

void Pretty::stmt(Verific::VhdlComponentInstantiationStatement *s)
{
  UNIMP(s);
}

void Pretty::stmt(Verific::VhdlProcedureCallStatement *s)
{
  UNIMP(s);
}

void Pretty::stmt(Verific::VhdlConditionalSignalAssignment *s)
{
  expr(s->GetTarget());
  put(" <= ");
  if (s->GetGuardId()) {
    put("guarded ");
  }
  waveforms(s->GetConditionalWaveforms());
  put(";");
  nl();
}

void Pretty::stmt(Verific::VhdlSelectedSignalAssignment *s)
{
  UNIMP(s);
}

void Pretty::stmt(Verific::VhdlVariableAssignmentStatement *s)
{
  expr(s->GetTarget());
  put(" := ");
  expr(s->GetValue());
  put(";");
  nl();
}

void Pretty::waveforms(Verific::Array *a)
{
  int i;
  Verific::VhdlTreeNode *e;
  FOREACH_ARRAY_ITEM(a, i, e) {
    if (i > 0) {
      put(", ");
    }
    
    switch (e->GetClassId()) {
    case Verific::ID_VHDLCONDITIONALWAVEFORM: {
      Verific::VhdlConditionalWaveform *cw = (Verific::VhdlConditionalWaveform *) e;
      exprs(cw->GetWaveform());
      if (cw->Condition()) {
	put(" when ");
	expr(cw->Condition());
      }
    }
      break;
    case Verific::ID_VHDLSELECTEDWAVEFORM: {
      //Verific::VhdlSelectedWaveform *sw = (Verific::VhdlSelectedWaveform *) e;
      
    }
      break;
    default:
      expr((Verific::VhdlExpression *)e);
      break;
    }
  }
}

void Pretty::exprs(Verific::Array *a)
{
  int i;
  Verific::VhdlDiscreteRange *r;
  indent();
  FOREACH_ARRAY_ITEM(a, i, r) {
    if (i > 0) {
      put(", ");
    }
    expr(r);
  }
  deindent();
}


void Pretty::expr(Verific::VhdlDiscreteRange *e)
{
  if (! e) {
    put("<null>");
    return;
  }
  switch (e->GetClassId()) {
  case Verific::ID_VHDLBOX:
    put("<>");
    break;
  case Verific::ID_VHDLRANGE:
    expr(((Verific::VhdlRange *) e)->GetLeftExpression());
    put(((Verific::VhdlRange *) e)->GetDir() ? " to " : " downto ");
    expr(((Verific::VhdlRange *) e)->GetRightExpression());
    break;
  case Verific::ID_VHDLQUALIFIEDEXPRESSION:
    expr(((Verific::VhdlQualifiedExpression *) e)->GetPrefix());
    put("'");
    expr(((Verific::VhdlQualifiedExpression *) e)->GetAggregate());
    break;
  case Verific::ID_VHDLOPERATOR:
    expr((Verific::VhdlOperator *) e);
    break;
  case Verific::ID_VHDLINTEGER:
    put(((Verific::VhdlInteger *) e)->GetValue());
    break;
  case Verific::ID_VHDLREAL:
    put(((Verific::VhdlReal *) e)->GetValue());
    break;
  case Verific::ID_VHDLNULL:
    put("null");
    break;
  case Verific::ID_VHDLSTRINGLITERAL:
  case Verific::ID_VHDLBITSTRINGLITERAL:
    put(((Verific::VhdlStringLiteral *) e)->Name());
    break;
  case Verific::ID_VHDLCHARACTERLITERAL:
    put(((Verific::VhdlCharacterLiteral *) e)->Name());
    break;
  case Verific::ID_VHDLOPEN:
    put("open");
    break;
  case Verific::ID_VHDLATTRIBUTENAME:
    expr((Verific::VhdlAttributeName *) e);
    break;
  case Verific::ID_VHDLIDREF:
    expr((Verific::VhdlIdRef *) e);
    break;
  case Verific::ID_VHDLEXPLICITSUBTYPEINDICATION:
    expr((Verific::VhdlExplicitSubtypeIndication *) e);
    break;
  case Verific::ID_VHDLINDEXEDNAME:
    expr((Verific::VhdlIndexedName *) e);
    break;
  case Verific::ID_VHDLSELECTEDNAME:
    expr((Verific::VhdlSelectedName *) e);
    break;
  case Verific::ID_VHDLPHYSICALLITERAL:
    expr(((Verific::VhdlPhysicalLiteral *) e)->GetValueExpr());
    put(" ");
    expr(((Verific::VhdlPhysicalLiteral *) e)->GetUnit());
    break;
  case Verific::ID_VHDLAGGREGATE:
    expr((Verific::VhdlAggregate *) e);
    break;
  case Verific::ID_VHDLELEMENTASSOC:
    expr((Verific::VhdlElementAssoc *) e);
    break;
  case Verific::ID_VHDLOTHERS:
    put("others");
    break;
  case Verific::ID_VHDLALL:
    put("all");
    break;
  case Verific::ID_VHDLWAVEFORMELEMENT:
    expr((Verific::VhdlWaveformElement *) e);
    break;
  default:
    UNIMP(e);
    break;
  }
}

void Pretty::expr(Verific::VhdlAttributeName *n)
{
  Verific::VhdlName *prefix = n->GetPrefix();
  expr(prefix);
  put("'");
  Verific::VhdlIdRef *desig = n->GetDesignator();
  expr(desig);
  Verific::VhdlExpression *e = n->GetExpression();
  if (e) {
    put("(");
    expr(e);
    put(")");
  }
}


void Pretty::expr(Verific::VhdlOperator *e)
{
  Verific::VhdlExpression *l = e->GetLeftExpression();
  Verific::VhdlExpression *r = e->GetRightExpression();
  Verific::VhdlIdDef *op = e->GetOperator();
  char *name = strdup(op->Name());
  name[strlen(name)-1] = '\0';	// remove quotes

  put("(");
  if (r) {
    expr(l);
    put(" ");
    put(&name[1]);
    put(" ");
    expr(r);
  }
  else {
    put(&name[1]);
    put(" ");
    expr(l);
  }
  put(")");
  free(name);
}

void Pretty::expr(Verific::VhdlIdRef *id)
{
  if (! id) {
    put("<null>");
    return;
  }
  put(id->Name());
  {
    Comment c;
    put("   ");
    Verific::VhdlIdDef *def = id->GetId();
    if (def) {
      expr(def);
      put(" ");
      Verific::VhdlIdDef *type_def = def->Type();
      if (type_def) {
	put("Type:");
	expr(type_def);
      }
    }    
  }
}

void Pretty::expr(Verific::VhdlIdDef *id)
{
  if (! id) {
    put("<null>");
    return;
  }
  put(getName(id));
  {
    Comment c;
    Verific::MapIter i;
    Verific::VhdlIdDef *attr_id;
    Verific::VhdlExpression *val;

    FOREACH_MAP_ITEM((id)->GetAttributes(),i,&(attr_id),&(val)) {
      put("attribute ");
      put(attr_id->Name());
      put(" of ");
      put(getName(id));
      put(": whatever is ");
      expr(val);

      if (! id->GetAttributes()->GetValue(attr_id->Name())) {
	printf("Something is wrong!\n");
      }

    }
  }
}

void Pretty::expr(Verific::VhdlExplicitSubtypeIndication *st)
{
  bool empty = true;
  if (st->GetResolutionFunction()) {
    expr(st->GetResolutionFunction());
    empty = false;
  }
  if (st->GetTypeMark()) {
    if (! empty) {
      put(" ");
    }
    expr(st->GetTypeMark());
    empty = false;
  }
  if (st->GetRangeConstraint()) {
    if (! empty) {
      put(" ");
    }
    expr(st->GetRangeConstraint());
  }
}

void Pretty::expr(Verific::VhdlIndexedName *e)
{
  expr(e->GetPrefixId());
  put("(");
  Verific::Array *a = e->GetAssocList();
  int i;
  Verific::VhdlDiscreteRange *r;
  FOREACH_ARRAY_ITEM(a, i, r) {
    if (i > 0) {
      put(", ");
    }
    expr(r);
  }
  put(")");
}

void Pretty::expr(Verific::VhdlSelectedName *e)
{
  expr(e->GetPrefix());
  put(".");
  expr(e->GetSuffix());
  if (e->GetUniqueId()) {
    Comment c;
    expr(e->GetUniqueId());
    put(" ");
  }
}

void Pretty::expr(Verific::VhdlAggregate *e)
{
  put("(");
  exprs(e->GetElementAssocList());
  put(")");
}

void Pretty::expr(Verific::VhdlElementAssoc *e)
{
  if (e->GetChoices()) {
    choices(e->GetChoices());
    put(" => ");
  }
  expr(e->GetExpression());
}

void Pretty::expr(Verific::VhdlWaveformElement *e)
{
  expr(e->GetValue());
  if (e->GetAfter()) {
    put(" after ");
    expr(e->GetAfter());
  }
}

void Pretty::choices(Verific::Array *c)
{
  int i;
  Verific::VhdlDiscreteRange *r;
  FOREACH_ARRAY_ITEM(c, i, r) {
    if (i > 0) {
      put(" | ");
    }
    expr(r);
  }
}

void Pretty::type(Verific::VhdlTypeDef *t)
{
  switch (t->GetClassId()) {
  case Verific::ID_VHDLACCESSTYPEDEF:
    put("access ");
    expr(((Verific::VhdlAccessTypeDef *) t)->GetSubtypeIndication());
    break;
  case Verific::ID_VHDLARRAYTYPEDEF: {
    put("array ");
    Verific::Array *ic = ((Verific::VhdlArrayTypeDef *) t)->GetIndexConstraint();
    int i;
    Verific::VhdlDiscreteRange *r;
    FOREACH_ARRAY_ITEM(ic, i, r) {
      expr(r);
    }
    put(" of ");
    expr(((Verific::VhdlArrayTypeDef *) t)->GetSubtypeIndication());
  }
    break;
  case Verific::ID_VHDLENUMERATIONTYPEDEF: {
    Verific::Array *enums = ((Verific::VhdlEnumerationTypeDef *) t)->GetEnumLiteralList();
    int i;
    Verific::VhdlEnumerationId *id;

    put("(");
    indent();
    pretty_ids(enums);
    {
      Comment c;
      FOREACH_ARRAY_ITEM(enums, i, id) {
	char buf[80];
	sprintf(buf, "%s->'%d'/\"%s\"  ", id->Name(), id->GetBitEncoding(), id->GetEncoding());
	put(buf);
      }
    }
    deindent();
    put(")");
  }
    break;
  case Verific::ID_VHDLFILETYPEDEF:
    put("file of ");
    expr(((Verific::VhdlFileTypeDef *) t)->GetFileTypeMark());
    break;
  case Verific::ID_VHDLPHYSICALTYPEDEF: {
    put("range ");
    expr(((Verific::VhdlPhysicalTypeDef *) t)->GetRangeConstraint());
    put(" units ");
    indent();
    Verific::Array *units = ((Verific::VhdlPhysicalTypeDef *) t)->GetPhysicalUnitDeclList();
    int i;
    Verific::VhdlPhysicalUnitDecl *n;
    FOREACH_ARRAY_ITEM(units, i, n) {
      decl(n);
    }
    deindent();
    put("end units");
  }
    break;
  case Verific::ID_VHDLRECORDTYPEDEF: {
    put("record");
    nl();
    indent();
    Verific::Array *ed = ((Verific::VhdlRecordTypeDef *) t)->GetElementDeclList();
    int i;
    Verific::VhdlDeclaration *d;
    FOREACH_ARRAY_ITEM(ed, i, d) {
      decl(d);
    }
    deindent();
    put("end record");
    nl();
  }
    break;
  case Verific::ID_VHDLSCALARTYPEDEF:
    put("range ");
    expr(((Verific::VhdlScalarTypeDef *) t)->GetScalarRange());
    break;
  default:
    UNIMP(t);
    assert(0);
  }
}



int main(int argc, char **argv)
{
  if (argc == 1) {
    printf("Usage: test {<vhdl-file>}\n");
    exit(1);
  }

  Verific::vhdl_file::SetDefaultLibraryPath("/export/home/pgraham/verific/eval/vhdl_packages/vdbs") ;

  for (int i = 1; i < argc; i++) {
    Verific::vhdl_sort::RegisterFile(argv[i], "work");
  }
  
  Verific::vhdl_file vhdlReader;

  unsigned int i;
  const char *filename, *libname;
  FOREACH_SORTED_FILE(i, filename, libname) {
    if (! vhdlReader.Analyze(filename, libname)) {
      printf("Error: failed to read file %s:%s.\n", libname, filename);
      exit(1);
    }
    else {
      printf("Successfully read file %s:%s.\n", libname, filename);
    }
  }
  
  Verific::MapIter m1, m2, m3 ;
  Verific::VhdlLibrary *lib ;
  Verific::VhdlPrimaryUnit *primunit ;
  Verific::VhdlSecondaryUnit *secunit ;
  IdDeclMap map;
  MyVisitor v(&map);

  FOREACH_VHDL_LIBRARY(m1, lib) {
    FOREACH_VHDL_PRIMARY_UNIT(lib, m2, primunit) {
      primunit->Accept(v);
      FOREACH_VHDL_SECONDARY_UNIT(primunit, m3, secunit) {
	secunit->Accept(v);
      }
    }
  }

  Pretty p;
  
  Verific::MapIter lib_iter;
  FOREACH_VHDL_LIBRARY(lib_iter, lib) {
    p.lib(lib);
  }
}
