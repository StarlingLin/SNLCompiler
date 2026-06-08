#include "SymbolTable.h"

SymbolTableManager::SymbolTableManager()
{
    level = -1;
}

SymbolTableManager::~SymbolTableManager()
{
    while (!scopeStack.empty())
    {
        DestroyTable();
    }
}

void SymbolTableManager::CreateTable()
{
    level++;
    scopeStack.push_back(nullptr);
    offsetStack.push_back(0);
}

void SymbolTableManager::DestroyTable()
{
    if (scopeStack.empty())
    {
        return;
    }
    SymbTable* table = scopeStack.back();
    freeTable(table);
    scopeStack.pop_back();
    offsetStack.pop_back();
    level--;
}

bool SymbolTableManager::Enter(const string& id, AttributeIR* attribP, SymbTable** entry)
{
    if (entry != nullptr)
    {
        *entry = nullptr;
    }
    if (scopeStack.empty())
    {
        CreateTable();
    }
    SymbTable* exist = nullptr;
    if (SearchOneTable(id, level, &exist))
    {
        return false;
    }
    SymbTable* node = new SymbTable();
    node->id = id;
    if (attribP != nullptr)
    {
        node->attr = *attribP;
    }
    node->level = level;
    node->offset = offsetStack[level];
    node->attr.level = level;
    node->attr.off = node->offset;
    offsetStack[level]++;
    node->next = nullptr;
    if (scopeStack[level] == nullptr)
    {
        scopeStack[level] = node;
    }
    else
    {
        SymbTable* p = scopeStack[level];
        while (p->next != nullptr)
        {
            p = p->next;
        }
        p->next = node;
    }
    if (entry != nullptr)
    {
        *entry = node;
    }
    return true;
}

bool SymbolTableManager::FindEntry(const string& id, SearchFlag flag, SymbTable** entry)
{
    if (entry != nullptr)
    {
        *entry = nullptr;
    }
    if (scopeStack.empty())
    {
        return false;
    }
    if (flag == SearchFlag::OneTable)
    {
        return SearchOneTable(id, level, entry);
    }
    for (int i = level; i >= 0; i--)
    {
        if (SearchOneTable(id, i, entry))
        {
            return true;
        }
    }
    return false;
}

bool SymbolTableManager::SearchOneTable(const string& id, int currentLevel, SymbTable** entry)
{
    if (entry != nullptr)
    {
        *entry = nullptr;
    }
    if (currentLevel < 0 || currentLevel >= static_cast<int>(scopeStack.size()))
    {
        return false;
    }
    SymbTable* p = scopeStack[currentLevel];
    while (p != nullptr)
    {
        if (p->id == id)
        {
            if (entry != nullptr)
            {
                *entry = p;
            }

            return true;
        }

        p = p->next;
    }
    return false;
}

bool SymbolTableManager::FindField(const string& id, FieldChain* head, FieldChain** entry)
{
    if (entry != nullptr)
    {
        *entry = nullptr;
    }
    FieldChain* p = head;
    while (p != nullptr)
    {
        if (p->id == id)
        {
            if (entry != nullptr)
            {
                *entry = p;
            }
            return true;
        }
        p = p->next;
    }
    return false;
}

void SymbolTableManager::PrintSymbTable() const
{
    cout << "========== 符号表 ==========" << endl;
    for (int i = 0; i < static_cast<int>(scopeStack.size()); i++)
    {
        cout << "Level " << i << ":" << endl;
        SymbTable* p = scopeStack[i];
        while (p != nullptr)
        {
            cout << "  "
                 << "ID: " << p->id
                 << ", 标识符种类: " << idKindToString(p->attr.idKind)
                 << ", 类型: " << typeKindToString(p->attr.type.typeKind);
            if (!p->attr.type.typeName.empty())
            {
                cout << ", 类型名: " << p->attr.type.typeName;
            }
            if (p->attr.type.typeKind == TypeKind::ArrayTy)
            {
                cout << ", 范围: [" << p->attr.type.low << ".." << p->attr.type.top << "]";
            }
            cout << ", 偏移: " << p->offset;

            if (p->attr.isVarParam)
            {
                cout << ", 变量参数";
            }
            cout << endl;
            p = p->next;
        }
    }
}

int SymbolTableManager::getCurrentLevel() const
{
    return level;
}

void SymbolTableManager::freeTable(SymbTable* table)
{
    SymbTable* p = table;
    while (p != nullptr)
    {
        SymbTable* q = p->next;
        if (p->attr.type.fieldList != nullptr)
        {
            freeFieldChain(p->attr.type.fieldList);
            p->attr.type.fieldList = nullptr;
        }
        delete p;
        p = q;
    }
}

void SymbolTableManager::freeFieldChain(FieldChain* head)
{
    FieldChain* p = head;
    while (p != nullptr)
    {
        FieldChain* q = p->next;
        if (p->attr.type.fieldList != nullptr)
        {
            freeFieldChain(p->attr.type.fieldList);
            p->attr.type.fieldList = nullptr;
        }
        delete p;
        p = q;
    }
}

string SymbolTableManager::idKindToString(IdKind kind) const
{
    switch (kind)
    {
    case IdKind::TypeKind:
        return "TypeKind";
    case IdKind::VarKind:
        return "VarKind";
    case IdKind::ProcKind:
        return "ProcKind";
    case IdKind::ParamKind:
        return "ParamKind";
    case IdKind::FieldKind:
        return "FieldKind";
    default:
        return "UnknownKind";
    }
}

string SymbolTableManager::typeKindToString(TypeKind kind) const
{
    switch (kind)
    {
    case TypeKind::IntegerTy:
        return "IntegerTy";
    case TypeKind::CharTy:
        return "CharTy";
    case TypeKind::ArrayTy:
        return "ArrayTy";
    case TypeKind::RecordTy:
        return "RecordTy";
    case TypeKind::IdTy:
        return "IdTy";
    case TypeKind::BoolTy:
        return "BoolTy";
    default:
        return "UnknownTy";
    }
}