// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************
#ifndef VHDLUTILS_H
#define VHDLUTILS_H

#include "UTrace"
#include "UHash"
#include "Verific"
#include "DbDf"



typedef UHashMapList<Verific::VhdlComponentInstantiationStatement*, Verific::VhdlBindingIndication*> InstBindingMap;
typedef UHashMapList<Verific::VhdlIdDef*, Verific::VhdlTreeNode*> IdDeclMap;
typedef UHashMapList<Verific::VhdlTreeNode*, UString> PredefNodeMap;
typedef UHashMapList<UString, Verific::VhdlIdDef*> StdIdMap;

Verific::VhdlSubtypeIndication *getObjectSubtype(Verific::VhdlDeclaration *);
Verific::VhdlSubtypeIndication *getObjectSubtype(Verific::VhdlElementDecl *);

Verific::VhdlIdDef *getTypeId(Verific::VhdlName*);


typedef UHashSet<Verific::VhdlDesignUnit*> VhdlDesignUnitSet;
typedef UHashList<int> IntHashList;

bool getOtherTraces(Verific::VhdlTreeNode *, VhdlDesignUnitSet&, IntHashList&, bool);
UTrace getOtherTraces(IntHashList&);
UTrace getVhdlTrace(Verific::VhdlTreeNode *);
UTrace getTrace(Verific::VhdlTreeNode *);
void copyLinefile(Verific::VhdlTreeNode *, Verific::VhdlTreeNode *);

bool isEscapedName(const char*);
UStrBuf unEscapeName(const char*);
const char* getName(Verific::VhdlIdDef*);

void mapIdDecls(IdDeclMap &, Verific::VhdlTreeNode *);
void mapInstBindings(InstBindingMap&, Verific::VhdlTreeNode *);
void mapPredefNodes(PredefNodeMap &, const UString &, Verific::VhdlTreeNode *);
void mapMemories(Verific::VhdlTreeNode *);
void mapStdIds(StdIdMap&, Verific::VhdlTreeNode *);

Verific::VhdlExpression *getAttribute(Verific::VhdlIdDef *, const char *);

UStrBuf getStringValue(Verific::VhdlStringLiteral *);
UStrBuf getStringValue(Verific::VhdlCharacterLiteral *);
UStrBuf getStringValue(Verific::VhdlExpression *);

UStrBuf getSubprogramName(Verific::VhdlTreeNode *);

UStrBuf getDebugName(Verific::VhdlTreeNode *);

bool isBinaryOperator(Verific::VhdlExpression *);
UKeyword getBinaryOperatorType(Verific::VhdlExpression *);
UKeyword getBinaryOperatorType(int);
UKeyword getUnaryOperatorType(Verific::VhdlExpression *);
UKeyword getUnaryOperatorType(int);

bool isOperatorFunction(Verific::VhdlExpression *);

bool isBoxConstraint(Verific::VhdlDiscreteRange *);
bool isRange(Verific::VhdlDiscreteRange *);


enum VhdlConcatType {VCT_ElemElem, VCT_ElemArray, VCT_ArrayElem, VCT_ArrayArray};
VhdlConcatType getConcatType(Verific::VhdlOperatorId *);

Verific::VhdlDiscreteRange *getSliceRange(Verific::VhdlIndexedName *);

Verific::VhdlIdDef *unExpand(Verific::VhdlSelectedName *);
Verific::VhdlName *derefAlias(Verific::VhdlName*);
Verific::VhdlName *derefAlias(Verific::VhdlIdDef*);
Verific::VhdlIdDef *derefAliasId(Verific::VhdlIdDef*);

Verific::VhdlExpression *getIfCondition(Verific::VhdlIfStatement *, int);
Verific::Array *getIfClause(Verific::VhdlIfStatement *, int);

Verific::VhdlDiscreteRange *nthIndexSubtype(Verific::VhdlIdDef *, int);

Verific::VhdlName* baseName(Verific::VhdlName *);

Verific::VhdlSpecification* getSpec(Verific::VhdlProcedureCallStatement *);	// procedure
Verific::VhdlSpecification* getSpec(Verific::VhdlExpression *);		// function
Verific::VhdlSubprogramBody* getBody(Verific::VhdlProcedureCallStatement *);	// procedure
Verific::VhdlSubprogramBody* getBody(Verific::VhdlExpression *);		// function
Verific::VhdlIdDef* getSubprogramId(Verific::VhdlProcedureCallStatement *);	// procedure
Verific::VhdlIdDef* getSubprogramId(Verific::VhdlExpression *);		// function

Verific::VhdlIdDef* getNthFormal(Verific::VhdlFunctionSpec*, int);

unsigned int getFunctionPragma(Verific::VhdlExpression *);
bool isSignedFunctionPragma(Verific::VhdlExpression *);
unsigned int getProcedurePragma(Verific::VhdlProcedureCallStatement *);

Verific::VhdlExpression* getLeftArg(Verific::VhdlExpression *);
Verific::VhdlExpression* getRightArg(Verific::VhdlExpression *);
Verific::VhdlName* getReturnType(Verific::VhdlExpression *);

bool isTemplate(Verific::VhdlEntityId*);
bool isBlackbox(Verific::VhdlIdDef*);
bool isPrimitive(Verific::VhdlIdDef*);

bool isInferMux(Verific::VhdlIdDef*);

bool isStringType(Verific::VhdlIdDef*);
bool isStringType(Verific::VhdlExpression*);

bool isBooleanType(Verific::VhdlIdDef*);
bool isBooleanType(Verific::VhdlExpression*);

bool isRealType(Verific::VhdlIdDef* id);
bool isRealType(Verific::VhdlExpression* e);

bool isStdSubtype(Verific::VhdlDeclaration*);

DFPort::DFPortType getMode(Verific::VhdlInterfaceDecl *);
DFPort::DFPortType getMode(Verific::VhdlInterfaceId *);

Verific::VhdlExpression *getWaveform(Verific::Array *);

void parseComponentInst(Verific::VhdlComponentInstantiationStatement*, Verific::VhdlComponentId*&, const char *&, Verific::VhdlEntityId*&, const char*&, Verific::VhdlConfigurationId*&);
void parseComponentInst(Verific::VhdlComponentInstantiationStatement*, UString&, UString&);

void parseBindingIndication(Verific::VhdlBindingIndication *, const char *&, Verific::VhdlEntityId *&, const char *&, const char *&, Verific::VhdlConfigurationId *&);

Verific::VhdlBlockConfiguration* getBlockConfiguration(Verific::VhdlConfigurationId*);
void parseConfigDecl(Verific::VhdlConfigurationId*, Verific::VhdlEntityId*&, const char *&, Verific::VhdlBlockConfiguration*&);
Verific::VhdlComponentId* findComponentId(Verific::VhdlComponentInstantiationStatement*);
Verific::VhdlComponentConfiguration *findCompConfig(Verific::VhdlBlockConfiguration*, const char *, Verific::VhdlComponentId*);
Verific::VhdlBlockConfiguration* findBlockConfig(Verific::VhdlBlockConfiguration*, const char *);

UString defaultBindingLibrary(Verific::VhdlScope*, Verific::VhdlComponentId*);

Verific::VhdlPrimaryUnit* getUnit(Verific::VhdlName* nm);

bool isTextIOFile(Verific::VhdlDeclaration*);
bool isTextIOLine(Verific::VhdlDeclaration*);

bool matchExpr(Verific::VhdlDiscreteRange*, Verific::VhdlDiscreteRange*);

bool evalInteger(Verific::VhdlExpression*, int& val);
bool evalRangeLength(Verific::VhdlDiscreteRange* r, int& val); 

// for verific debugging
void dump_node(Verific::VhdlTreeNode*);

#endif
