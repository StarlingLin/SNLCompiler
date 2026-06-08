#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

// 定义语义分析器类

#include "TreeNode.h"
#include "SymbolTable.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

enum class AccessKind
{
    Dir,
    Indir
};

class SemanticAnalyzer
{
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    void Analyze(TreeNode* currentP, bool traceTable = false);
    bool hasError() const;
    void printErrors() const;

private:
    SymbolTableManager symbolTable;
    vector<string> errors;
    vector<TypeIR*> typePool;
    TypeIR* integerType;
    TypeIR* charType;
    TypeIR* boolType;
    TypeIR* unknownType;
    int currentLevel;
    int currentOff;
    int initOff;
    vector<int> offStack;

private:
    void initialize();
    TypeIR* TypeProcess(TreeNode* t);   // 类型分析处理函数
    TypeIR* nameType(TreeNode* t);      // 自定义类型内部结构分析函数
    TypeIR* arrayType(TreeNode* t);     // 数组类型内部表示处理函数
    TypeIR* recordType(TreeNode* t);    // 记录类型内部表示处理函数
    void TypeDecPart(TreeNode* t);
    void VarDecList(TreeNode* t, bool isParam = false);
    void ProcDecPart(TreeNode* t);
    SymbTable* HeadProcess(TreeNode* t);
    ParamTable* ParaDecList(TreeNode* t);
    void Body(TreeNode* t);
    void statement(TreeNode* t);
    void ifstatement(TreeNode* t);
    void whilestatement(TreeNode* t);
    void assignstatement(TreeNode* t);
    void readstatement(TreeNode* t);
    void writestatement(TreeNode* t);
    void callstatement(TreeNode* t);
    void returnstatement(TreeNode* t);
    void analyzeDeclarePart(TreeNode* t);
    TypeIR* makeType(TypeKind kind);
    TypeIR* cloneType(const TypeIR* source);
    TreeNode* child(TreeNode* node, int index) const;
    TreeNode* findFirstChild(TreeNode* node, const string& name, const string& value = "") const;
    vector<TreeNode*> collectIdNodes(TreeNode* node) const;
    bool isNode(TreeNode* node, const string& name, const string& value = "") const;
    int widthOfType(TypeIR* type) const;
    void semanticError(TreeNode* node, const string& message);
    TypeIR* Expr(TreeNode* t, AccessKind* Ekind);
    TypeIR* arrayVar(TreeNode* t);
    TypeIR* recordVar(TreeNode* t);
    bool sameType(TypeIR* a, TypeIR* b) const;
    bool isIntegerType(TypeIR* t) const;
    bool isBoolType(TypeIR* t) const;
    string getBaseVarName(const string& name) const;
};

#endif