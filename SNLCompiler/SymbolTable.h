#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

// 符号表实现

#include <iostream>
#include <string>
#include <vector>

using namespace std;

// 标识符种类
enum class IdKind
{
    TypeKind,
    VarKind,
    ProcKind,
    ParamKind,
    FieldKind,
    UnknownKind
};

// 类型种类
enum class TypeKind
{
    IntegerTy,
    CharTy,
    ArrayTy,
    RecordTy,
    IdTy,
    BoolTy,
    UnknownTy
};

// 搜索范围
enum class SearchFlag
{
    OneTable,
    AllTable
};

// 字段链表
struct FieldChain;

// 类型信息
struct TypeIR
{
    TypeKind typeKind = TypeKind::UnknownTy;
    string typeName;
    int low = 0;
    int top = 0;
    TypeIR* elemType = nullptr;
    FieldChain* fieldList = nullptr;
};

struct ParamTable
{
    string name;
    TypeIR* type;
    bool isVarParam;
    ParamTable* next;
    ParamTable()
    {
        type = nullptr;
        isVarParam = false;
        next = nullptr;
    }
};

// 标识符属性
struct AttributeIR
{
    IdKind idKind;
    TypeIR type;
    int level;
    int off;
    bool isParam;
    bool isVarParam;
    ParamTable* param;
    AttributeIR()
    {
        level = 0;
        off = 0;
        isParam = false;
        isVarParam = false;
        param = nullptr;
    }
};

struct FieldChain
{
    string id;
    AttributeIR attr;
    FieldChain* next = nullptr;
};

// 符号表项
struct SymbTable
{
    string id;
    AttributeIR attr;
    int level = 0;
    int offset = 0;
    SymbTable* next = nullptr;
};

class SymbolTableManager
{
public:
    SymbolTableManager();
    ~SymbolTableManager();
    void CreateTable();
    void DestroyTable();
    bool Enter(const string& id, AttributeIR* attribP, SymbTable** entry);
    bool FindEntry(const string& id, SearchFlag flag, SymbTable** entry);
    bool SearchOneTable(const string& id, int currentLevel, SymbTable** entry);
    bool FindField(const string& id, FieldChain* head, FieldChain** entry);
    void PrintSymbTable() const;
    int getCurrentLevel() const;

private:
    vector<SymbTable*> scopeStack;
    vector<int> offsetStack;
    int level;

private:
    void freeTable(SymbTable* table);
    void freeFieldChain(FieldChain* head);
    string idKindToString(IdKind kind) const;
    string typeKindToString(TypeKind kind) const;
};

#endif