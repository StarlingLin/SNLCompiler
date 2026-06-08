#include "SemanticAnalyzer.h"

SemanticAnalyzer::SemanticAnalyzer()
{
    integerType = nullptr;
    charType = nullptr;
    boolType = nullptr;
    unknownType = nullptr;
}

SemanticAnalyzer::~SemanticAnalyzer()
{
    for (TypeIR* type : typePool)
    {
        delete type;
    }
    typePool.clear();
}

void SemanticAnalyzer::Analyze(TreeNode* currentP, bool traceTable)
{
    errors.clear();
    currentLevel = 0;
    initOff = 7;
    currentOff = initOff;
    offStack.clear();
    symbolTable.CreateTable();
    initialize();
    if (currentP == nullptr)
    {
        semanticError(nullptr, "语法树为空，无法进行语义分析。");
        symbolTable.DestroyTable();
        return;
    }
    analyzeDeclarePart(currentP);
    for (TreeNode* item : currentP->children)
    {
        if (item != nullptr && item->name == "StmtK" && item->value == "StmListK")
        {
            Body(item);
        }
    }
    if (traceTable)
    {
        cout << endl;
        symbolTable.PrintSymbTable();
    }
    symbolTable.DestroyTable();
}

void SemanticAnalyzer::initialize()
{
    integerType = makeType(TypeKind::IntegerTy);
    charType = makeType(TypeKind::CharTy);
    boolType = makeType(TypeKind::BoolTy);
    unknownType = makeType(TypeKind::UnknownTy);
    AttributeIR integerAttr;
    integerAttr.idKind = IdKind::TypeKind;
    integerAttr.type = *integerType;
    AttributeIR charAttr;
    charAttr.idKind = IdKind::TypeKind;
    charAttr.type = *charType;
    AttributeIR boolAttr;
    boolAttr.idKind = IdKind::TypeKind;
    boolAttr.type = *boolType;
    SymbTable* entry = nullptr;
    symbolTable.Enter("integer", &integerAttr, &entry);
    symbolTable.Enter("char", &charAttr, &entry);
    symbolTable.Enter("bool", &boolAttr, &entry);
}

TypeIR* SemanticAnalyzer::TypeProcess(TreeNode* t)
{
    if (t == nullptr)
    {
        semanticError(nullptr, "类型节点为空。");
        return unknownType;
    }
    if (isNode(t, "TypeK", "IntegerK"))
    {
        return integerType;
    }
    if (isNode(t, "TypeK", "CharK"))
    {
        return charType;
    }
    if (isNode(t, "TypeK", "IdTypeK"))
    {
        return nameType(t);
    }
    if (isNode(t, "TypeK", "ArrayK"))
    {
        return arrayType(t);
    }
    if (isNode(t, "TypeK", "RecordK"))
    {
        return recordType(t);
    }
    semanticError(t, "未知的类型节点: " + t->name + " " + t->value);
    return unknownType;
}

TypeIR* SemanticAnalyzer::nameType(TreeNode* t)
{
    TreeNode* idNode = child(t, 0);
    if (idNode == nullptr)
    {
        semanticError(t, "类型标识符缺失。");
        return unknownType;
    }
    SymbTable* entry = nullptr;
    if (!symbolTable.FindEntry(idNode->value, SearchFlag::AllTable, &entry))
    {
        semanticError(idNode, "类型 " + idNode->value + " 未声明。");
        return unknownType;
    }
    if (entry->attr.idKind != IdKind::TypeKind)
    {
        semanticError(idNode, idNode->value + " 不是类型标识符。");
        return unknownType;
    }
    return cloneType(&entry->attr.type);
}

TypeIR* SemanticAnalyzer::arrayType(TreeNode* t)
{
    TypeIR* result = makeType(TypeKind::ArrayTy);
    TreeNode* lowNode = nullptr;
    TreeNode* topNode = nullptr;
    TreeNode* elemTypeNode = nullptr;
    for (TreeNode* item : t->children)
    {
        if (isNode(item, "LowK"))
        {
            lowNode = item;
        }
        else if (isNode(item, "TopK"))
        {
            topNode = item;
        }
        else if (item != nullptr && item->name == "TypeK")
        {
            elemTypeNode = item;
        }
    }
    if (lowNode == nullptr || topNode == nullptr)
    {
        semanticError(t, "数组上下界缺失。");
        return unknownType;
    }
    int low = stoi(lowNode->value);
    int top = stoi(topNode->value);
    if (low > top)
    {
        semanticError(t, "数组下界大于上界。");
        return unknownType;
    }
    result->low = low;
    result->top = top;
    if (elemTypeNode == nullptr)
    {
        semanticError(t, "数组元素类型缺失。");
        result->elemType = unknownType;
        return result;
    }
    result->elemType = TypeProcess(elemTypeNode);
    return result;
}

TypeIR* SemanticAnalyzer::recordType(TreeNode* t)
{
    TypeIR* result = makeType(TypeKind::RecordTy);
    FieldChain* head = nullptr;
    FieldChain* tail = nullptr;
    for (TreeNode* fieldDecNode : t->children)
    {
        if (fieldDecNode == nullptr)
        {
            continue;
        }
        if (!(fieldDecNode->name == "DeclareK" && fieldDecNode->value == "FieldDecK"))
        {
            continue;
        }
        TreeNode* typeNode = nullptr;
        for (TreeNode* item : fieldDecNode->children)
        {
            if (item != nullptr && item->name == "TypeK")
            {
                typeNode = item;
                break;
            }
        }
        TypeIR* fieldType = TypeProcess(typeNode);
        for (TreeNode* item : fieldDecNode->children)
        {
            if (item == nullptr || item->name != "IdK")
            {
                continue;
            }
            FieldChain* exist = nullptr;
            if (symbolTable.FindField(item->value, head, &exist))
            {
                semanticError(item, "记录域 " + item->value + " 重复声明。");
                continue;
            }
            FieldChain* field = new FieldChain();
            field->id = item->value;
            field->attr.idKind = IdKind::FieldKind;
            field->attr.type = *fieldType;
            field->next = nullptr;
            if (head == nullptr)
            {
                head = field;
                tail = field;
            }
            else
            {
                tail->next = field;
                tail = field;
            }
        }
    }
    result->fieldList = head;
    return result;
}

void SemanticAnalyzer::TypeDecPart(TreeNode* t)
{
    if (t == nullptr) return;
    for (TreeNode* decNode : t->children)
    {
        if (decNode == nullptr) continue;
        TreeNode* typeNode = nullptr;
        for (TreeNode* item : decNode->children)
        {
            if (item == nullptr) continue;
            if (item->name == "TypeK")
            {
                typeNode = item;
                break;
            }
        }
        if (typeNode == nullptr)
        {
            semanticError(decNode, "类型声明缺少类型定义部分。");
            continue;
        }
        TypeIR* typePtr = TypeProcess(typeNode);
        vector<TreeNode*> ids = collectIdNodes(decNode);
        for (TreeNode* idNode : ids)
        {
            AttributeIR attr;
            attr.idKind = IdKind::TypeKind;
            attr.type = *typePtr;
            SymbTable* entry = nullptr;
            if (!symbolTable.Enter(idNode->value, &attr, &entry))
            {
                semanticError(idNode, "类型名重复声明: " + idNode->value);
            }
        }
    }
}

void SemanticAnalyzer::VarDecList(TreeNode* t, bool isParam)
{
    if (t == nullptr) return;
    for (TreeNode* decNode : t->children)
    {
        if (decNode == nullptr) continue;
        TreeNode* typeNode = nullptr;
        for (TreeNode* item : decNode->children)
        {
            if (item == nullptr) continue;
            if (item->name == "TypeK")
            {
                typeNode = item;
                break;
            }
        }
        if (typeNode == nullptr)
        {
            semanticError(decNode, "变量声明缺少类型定义部分。");
            continue;
        }
        TypeIR* typePtr = TypeProcess(typeNode);
        vector<TreeNode*> ids = collectIdNodes(decNode);
        bool isVarParam = false;
        if (isParam)
        {
            if (decNode->value == "VarParamK")
            {
                isVarParam = true;
            }
        }
        for (TreeNode* idNode : ids)
        {
            AttributeIR attr;
            attr.idKind = IdKind::VarKind;
            attr.type = *typePtr;
            attr.level = currentLevel;
            attr.off = currentOff;
            attr.isParam = isParam;
            attr.isVarParam = isVarParam;
            SymbTable* entry = nullptr;
            if (!symbolTable.Enter(idNode->value, &attr, &entry))
            {
                semanticError(idNode, "变量重复声明: " + idNode->value);
                continue;
            }
            if (isVarParam)
            {
                currentOff += 1;
            }
            else
            {
                currentOff += widthOfType(typePtr);
            }
        }
    }
}

void SemanticAnalyzer::ProcDecPart(TreeNode* t)
{
    if (t == nullptr) return;
    vector<TreeNode*> procNodes;
    if (t->name == "ProcDecK")
    {
        procNodes.push_back(t);
    }
    else
    {
        for (TreeNode* item : t->children)
        {
            if (item != nullptr && item->name == "ProcDecK")
            {
                procNodes.push_back(item);
            }
        }
    }
    for (TreeNode* procNode : procNodes)
    {
        HeadProcess(procNode);
        analyzeDeclarePart(procNode);
        for (TreeNode* item : procNode->children)
        {
            if (item != nullptr && item->name == "StmtK" && item->value == "StmListK")
            {
                Body(item);
            }
        }
        if (symbolTable.getCurrentLevel() > 0)
        {
            symbolTable.DestroyTable();
            currentLevel--;
        }
        if (!offStack.empty())
        {
            currentOff = offStack.back();
            offStack.pop_back();
        }
        else
        {
            currentOff = initOff;
        }
    }
}

SymbTable* SemanticAnalyzer::HeadProcess(TreeNode* t)
{
    if (t == nullptr) return nullptr;
    string procName = t->value;
    if (procName.empty() || procName == "ProcK" || procName == "ProcDecK")
    {
        TreeNode* idNode = findFirstChild(t, "IdK");
        if (idNode != nullptr)
        {
            procName = idNode->value;
        }
    }
    if (procName.empty())
    {
        semanticError(t, "过程名缺失。");
        return nullptr;
    }
    AttributeIR attr;
    attr.idKind = IdKind::ProcKind;
    attr.level = currentLevel;
    attr.param = nullptr;
    SymbTable* entry = nullptr;
    if (!symbolTable.Enter(procName, &attr, &entry))
    {
        semanticError(t, "过程名重复声明: " + procName);
    }
    offStack.push_back(currentOff);
    symbolTable.CreateTable();
    currentLevel++;
    currentOff = initOff;
    ParamTable* paramHead = ParaDecList(t);
    if (entry != nullptr)
    {
        entry->attr.param = paramHead;
    }
    return entry;
}

ParamTable* SemanticAnalyzer::ParaDecList(TreeNode* t)
{
    if (t == nullptr) return nullptr;
    ParamTable* head = nullptr;
    ParamTable* tail = nullptr;
    for (TreeNode* decNode : t->children)
    {
        if (decNode == nullptr) continue;
        if (!(decNode->name == "DeclareK" &&
              (decNode->value == "VarParamK" || decNode->value == "ValParamK")))
        {
            continue;
        }
        TreeNode* typeNode = nullptr;
        for (TreeNode* item : decNode->children)
        {
            if (item != nullptr && item->name == "TypeK")
            {
                typeNode = item;
                break;
            }
        }
        if (typeNode == nullptr)
        {
            semanticError(decNode, "形参声明缺少类型定义部分。");
            continue;
        }
        TypeIR* typePtr = TypeProcess(typeNode);
        bool isVarParam = decNode->value == "VarParamK";
        for (TreeNode* item : decNode->children)
        {
            if (item == nullptr || item->name != "IdK")
            {
                continue;
            }
            AttributeIR attr;
            attr.idKind = IdKind::ParamKind;
            attr.type = *typePtr;
            attr.level = currentLevel;
            attr.off = currentOff;
            attr.isParam = true;
            attr.isVarParam = isVarParam;
            SymbTable* entry = nullptr;
            if (!symbolTable.Enter(item->value, &attr, &entry))
            {
                semanticError(item, "形参重复声明: " + item->value);
                continue;
            }
            ParamTable* paramNode = new ParamTable();
            paramNode->name = item->value;
            paramNode->type = typePtr;
            paramNode->isVarParam = isVarParam;
            paramNode->next = nullptr;
            if (head == nullptr)
            {
                head = paramNode;
                tail = paramNode;
            }
            else
            {
                tail->next = paramNode;
                tail = paramNode;
            }
            if (isVarParam)
            {
                currentOff += 1;
            }
            else
            {
                currentOff += widthOfType(typePtr);
            }
        }
    }
    return head;
}

void SemanticAnalyzer::Body(TreeNode* t)
{
    if (t == nullptr) return;
    TreeNode* stmtList = t;
    if (stmtList->name != "StmtK" && stmtList->value != "StmListK")
    {
        TreeNode* candidate = findFirstChild(t, "StmtK", "StmListK");
        if (candidate != nullptr)
        {
            stmtList = candidate;
        }
    }
    for (TreeNode* stmtNode : stmtList->children)
    {
        if (stmtNode == nullptr)
        {
            continue;
        }
        statement(stmtNode);
    }
}

void SemanticAnalyzer::statement(TreeNode* t)
{
    if (t == nullptr) return;
    if (t->name != "StmtK") return;
    if (t->value == "IfK")
    {
        ifstatement(t);
    }
    else if (t->value == "WhileK")
    {
        whilestatement(t);
    }
    else if (t->value == "AssignK")
    {
        assignstatement(t);
    }
    else if (t->value == "ReadK")
    {
        readstatement(t);
    }
    else if (t->value == "WriteK")
    {
        writestatement(t);
    }
    else if (t->value == "CallK")
    {
        callstatement(t);
    }
    else if (t->value == "ReturnK")
    {
        returnstatement(t);
    }
    else
    {
        semanticError(t, "未知语句类型: " + t->value);
    }
}

void SemanticAnalyzer::analyzeDeclarePart(TreeNode* t)
{
    if (t == nullptr) return;
    for (TreeNode* item : t->children)
    {
        if (item == nullptr) continue;
        if (item->name == "DeclareK" && item->value == "TypeDecK")
        {
            TypeDecPart(item);
        }
        else if (item->name == "DeclareK" && item->value == "VarDecK")
        {
            VarDecList(item, false);
        }
        else if (item->name == "ProcDecK")
        {
            ProcDecPart(item);
        }
    }
}

TypeIR* SemanticAnalyzer::makeType(TypeKind kind)
{
    TypeIR* type = new TypeIR();
    type->typeKind = kind;
    typePool.push_back(type);
    return type;
}

TypeIR* SemanticAnalyzer::cloneType(const TypeIR* source)
{
    if (source == nullptr)
    {
        return unknownType;
    }
    TypeIR* result = makeType(source->typeKind);
    result->typeName = source->typeName;
    result->low = source->low;
    result->top = source->top;
    result->elemType = source->elemType;
    result->fieldList = source->fieldList;
    return result;
}

TreeNode* SemanticAnalyzer::child(TreeNode* node, int index) const
{
    if (node == nullptr) return nullptr;
    if (index < 0 || index >= static_cast<int>(node->children.size())) return nullptr;
    return node->children[index];
}

bool SemanticAnalyzer::isNode(TreeNode* node, const string& name, const string& value) const
{
    if (node == nullptr) return false;
    if (node->name != name) return false;
    if (!value.empty() && node->value != value) return false;
    return true;
}

void SemanticAnalyzer::semanticError(TreeNode* node, const string& message)
{
    string errorMessage;
    if (node != nullptr)
    {
        errorMessage = "第 " + to_string(node->line) + " 行，" + message;
    }
    else
    {
        errorMessage = message;
    }
    errors.push_back(errorMessage);
    cerr << "语义错误：" << errorMessage << endl;
}

bool SemanticAnalyzer::hasError() const
{
    return !errors.empty();
}

void SemanticAnalyzer::printErrors() const
{
    if (errors.empty())
    {
        cout << "语义分析成功，没有发现语义错误。" << endl;
        return;
    }
    cout << "========== 语义错误 ==========" << endl;
    for (const string& error : errors)
    {
        cout << error << endl;
    }
}

TreeNode* SemanticAnalyzer::findFirstChild(TreeNode* node, const string& name, const string& value) const
{
    if (node == nullptr) return nullptr;
    for (TreeNode* item : node->children)
    {
        if (item == nullptr) continue;
        if (item->name != name) continue;
        if (!value.empty() && item->value != value) continue;
        return item;
    }
    return nullptr;
}

vector<TreeNode*> SemanticAnalyzer::collectIdNodes(TreeNode* node) const
{
    vector<TreeNode*> result;
    if (node == nullptr) return result;
    for (TreeNode* item : node->children)
    {
        if (item != nullptr && item->name == "IdK")
        {
            result.push_back(item);
        }
    }
    return result;
}

int SemanticAnalyzer::widthOfType(TypeIR* type) const
{
    if (type == nullptr) return 1;
    switch (type->typeKind)
    {
    case TypeKind::IntegerTy:
    case TypeKind::CharTy:
    case TypeKind::BoolTy:
        return 1;
    case TypeKind::ArrayTy:
        if (type->elemType == nullptr)
        {
            return 1;
        }
        return (type->top - type->low + 1) * widthOfType(type->elemType);
    case TypeKind::RecordTy:
    {
        int total = 0;
        FieldChain* p = type->fieldList;
        while (p != nullptr)
        {
            total += widthOfType(&(p->attr.type));
            p = p->next;
        }
        return total > 0 ? total : 1;
    }
    default:
        return 1;
    }
}

bool SemanticAnalyzer::sameType(TypeIR* a, TypeIR* b) const
{
    if (a == nullptr || b == nullptr) return false;
    if (a->typeKind == TypeKind::UnknownTy || b->typeKind == TypeKind::UnknownTy) return true;
    if (a->typeKind != b->typeKind) return false;
    if (a->typeKind == TypeKind::ArrayTy)
    {
        return a->low == b->low &&
               a->top == b->top &&
               sameType(a->elemType, b->elemType);
    }
    return true;
}

bool SemanticAnalyzer::isIntegerType(TypeIR* t) const
{
    return t != nullptr && t->typeKind == TypeKind::IntegerTy;
}

bool SemanticAnalyzer::isBoolType(TypeIR* t) const
{
    return t != nullptr && t->typeKind == TypeKind::BoolTy;
}

string SemanticAnalyzer::getBaseVarName(const string& name) const
{
    string result = name;
    size_t pos = result.find("[]");
    if (pos != string::npos)
    {
        result = result.substr(0, pos);
    }
    pos = result.find(".");
    if (pos != string::npos)
    {
        result = result.substr(0, pos);
    }
    return result;
}

TypeIR* SemanticAnalyzer::Expr(TreeNode* t, AccessKind* Ekind)
{
    if (Ekind != nullptr) *Ekind = AccessKind::Dir;
    if (t == nullptr)
    {
        semanticError(nullptr, "表达式节点为空。");
        return unknownType;
    }
    if (t->name == "ConstK")
    {
        if (Ekind != nullptr)
        {
            *Ekind = AccessKind::Dir;
        }
        return integerType;
    }
    if (t->name == "VarK" || t->name == "IdK")
    {
        if (t->value.find("[]") != string::npos)
        {
            if (Ekind != nullptr)
            {
                *Ekind = AccessKind::Indir;
            }
            return arrayVar(t);
        }
        if (t->value.find(".") != string::npos)
        {
            if (Ekind != nullptr)
            {
                *Ekind = AccessKind::Indir;
            }
            return recordVar(t);
        }
        SymbTable* entry = nullptr;
        if (!symbolTable.FindEntry(t->value, SearchFlag::AllTable, &entry))
        {
            semanticError(t, "变量未声明: " + t->value);
            return unknownType;
        }
        if (entry->attr.idKind != IdKind::VarKind && entry->attr.idKind != IdKind::ParamKind)
        {
            semanticError(t, t->value + " 不是变量标识符。");
            return unknownType;
        }
        if (Ekind != nullptr)
        {
            *Ekind = AccessKind::Indir;
        }
        return cloneType(&entry->attr.type);
    }
    if (t->name == "OpK")
    {
        TreeNode* leftNode = child(t, 0);
        TreeNode* rightNode = child(t, 1);
        AccessKind leftKind;
        AccessKind rightKind;
        TypeIR* leftType = Expr(leftNode, &leftKind);
        TypeIR* rightType = Expr(rightNode, &rightKind);
        if (t->value == "+" || t->value == "-" || t->value == "*" || t->value == "/")
        {
            if (!isIntegerType(leftType) || !isIntegerType(rightType))
            {
                semanticError(t, "算术运算符 " + t->value + " 两侧必须是 integer 类型。");
                return unknownType;
            }
            if (Ekind != nullptr)
            {
                *Ekind = AccessKind::Dir;
            }
            return integerType;
        }
        if (t->value == "<" || t->value == "=")
        {
            if (!sameType(leftType, rightType))
            {
                semanticError(t, "关系运算符 " + t->value + " 两侧类型不一致。");
                return unknownType;
            }
            if (Ekind != nullptr)
            {
                *Ekind = AccessKind::Dir;
            }
            return boolType;
        }
        semanticError(t, "未知运算符: " + t->value);
        return unknownType;
    }
    semanticError(t, "未知表达式节点: " + t->name + " " + t->value);
    return unknownType;
}

TypeIR* SemanticAnalyzer::arrayVar(TreeNode* t)
{
    if (t == nullptr) return unknownType;
    string baseName = getBaseVarName(t->value);
    SymbTable* entry = nullptr;
    if (!symbolTable.FindEntry(baseName, SearchFlag::AllTable, &entry))
    {
        semanticError(t, "数组变量未声明: " + baseName);
        return unknownType;
    }
    if (entry->attr.idKind != IdKind::VarKind && entry->attr.idKind != IdKind::ParamKind)
    {
        semanticError(t, baseName + " 不是变量。");
        return unknownType;
    }
    TypeIR* arrayTypePtr = cloneType(&entry->attr.type);
    if (arrayTypePtr->typeKind != TypeKind::ArrayTy)
    {
        semanticError(t, baseName + " 不是数组变量。");
        return unknownType;
    }
    TreeNode* indexNode = child(t, 0);
    if (indexNode == nullptr)
    {
        semanticError(t, "数组变量 " + baseName + " 缺少下标表达式。");
        return unknownType;
    }
    AccessKind indexKind;
    TypeIR* indexType = Expr(indexNode, &indexKind);
    if (!isIntegerType(indexType))
    {
        semanticError(indexNode, "数组下标必须是 integer 类型。");
        return unknownType;
    }
    if (arrayTypePtr->elemType == nullptr)
    {
        return unknownType;
    }
    return arrayTypePtr->elemType;
}

TypeIR* SemanticAnalyzer::recordVar(TreeNode* t)
{
    if (t == nullptr) return unknownType;
    size_t dotPos = t->value.find(".");
    if (dotPos == string::npos)
    {
        semanticError(t, "记录变量格式错误: " + t->value);
        return unknownType;
    }
    string baseName = t->value.substr(0, dotPos);
    string fieldName = t->value.substr(dotPos + 1);
    size_t arrayPos = fieldName.find("[]");
    if (arrayPos != string::npos)
    {
        fieldName = fieldName.substr(0, arrayPos);
    }
    SymbTable* entry = nullptr;
    if (!symbolTable.FindEntry(baseName, SearchFlag::AllTable, &entry))
    {
        semanticError(t, "记录变量未声明: " + baseName);
        return unknownType;
    }
    if (entry->attr.idKind != IdKind::VarKind && entry->attr.idKind != IdKind::ParamKind)
    {
        semanticError(t, baseName + " 不是变量。");
        return unknownType;
    }
    TypeIR* recordTypePtr = cloneType(&entry->attr.type);
    if (recordTypePtr->typeKind != TypeKind::RecordTy)
    {
        semanticError(t, baseName + " 不是记录变量。");
        return unknownType;
    }
    FieldChain* fieldEntry = nullptr;
    if (!symbolTable.FindField(fieldName, recordTypePtr->fieldList, &fieldEntry))
    {
        semanticError(t, fieldName + " 不是记录 " + baseName + " 的合法域名。");
        return unknownType;
    }
    TypeIR* fieldType = cloneType(&fieldEntry->attr.type);
    if (t->value.find("[]") != string::npos)
    {
        if (fieldType->typeKind != TypeKind::ArrayTy)
        {
            semanticError(t, fieldName + " 不是数组域。");
            return unknownType;
        }
        TreeNode* indexNode = child(t, 0);
        if (indexNode == nullptr)
        {
            semanticError(t, "数组域 " + fieldName + " 缺少下标表达式。");
            return unknownType;
        }
        AccessKind indexKind;
        TypeIR* indexType = Expr(indexNode, &indexKind);
        if (!isIntegerType(indexType))
        {
            semanticError(indexNode, "数组域下标必须是 integer 类型。");
            return unknownType;
        }
        return fieldType->elemType;
    }
    return fieldType;
}

void SemanticAnalyzer::assignstatement(TreeNode* t)
{
    if (t == nullptr) return;
    TreeNode* leftNode = child(t, 0);
    TreeNode* rightNode = child(t, 1);
    if (leftNode == nullptr || rightNode == nullptr)
    {
        semanticError(t, "赋值语句缺少左值或右值。");
        return;
    }
    AccessKind leftKind;
    AccessKind rightKind;
    TypeIR* leftType = Expr(leftNode, &leftKind);
    TypeIR* rightType = Expr(rightNode, &rightKind);
    if (leftNode->name == "ConstK")
    {
        semanticError(leftNode, "赋值语句左侧不能是常量。");
        return;
    }
    if (!sameType(leftType, rightType))
    {
        semanticError(t, "赋值语句左右两侧类型不一致。");
    }
}

void SemanticAnalyzer::callstatement(TreeNode* t)
{
    if (t == nullptr) return;
    TreeNode* procNode = child(t, 0);
    if (procNode == nullptr)
    {
        semanticError(t, "过程调用缺少过程名。");
        return;
    }
    string procName = procNode->value;
    SymbTable* entry = nullptr;
    if (!symbolTable.FindEntry(procName, SearchFlag::AllTable, &entry))
    {
        semanticError(procNode, "过程未声明: " + procName);
        return;
    }
    if (entry->attr.idKind != IdKind::ProcKind)
    {
        semanticError(procNode, procName + " 不是过程标识符。");
        return;
    }
    ParamTable* param = entry->attr.param;
    int argIndex = 1;
    while (param != nullptr && argIndex < static_cast<int>(t->children.size()))
    {
        TreeNode* argNode = child(t, argIndex);
        AccessKind argKind;
        TypeIR* argType = Expr(argNode, &argKind);
        if (!sameType(param->type, argType))
        {
            semanticError(argNode, "过程 " + procName + " 的实参与形参类型不匹配。");
        }
        if (param->isVarParam && argKind != AccessKind::Indir)
        {
            semanticError(argNode, "过程 " + procName + " 的 var 参数必须传入变量。");
        }
        param = param->next;
        argIndex++;
    }
    if (param != nullptr)
    {
        semanticError(t, "过程 " + procName + " 的实参数量过少。");
        return;
    }
    if (argIndex < static_cast<int>(t->children.size()))
    {
        semanticError(t, "过程 " + procName + " 的实参数量过多。");
    }
}

void SemanticAnalyzer::ifstatement(TreeNode* t)
{
    if (t == nullptr) return;
    TreeNode* condNode = nullptr;
    TreeNode* thenNode = nullptr;
    TreeNode* elseNode = nullptr;
    for (TreeNode* item : t->children)
    {
        if (item == nullptr) continue;
        if (condNode == nullptr && (item->name == "OpK" || item->name == "VarK" || item->name == "ConstK"))
        {
            condNode = item;
        }
        else if (item->name == "StmtK" && item->value == "StmListK")
        {
            if (thenNode == nullptr)
            {
                thenNode = item;
            }
            else
            {
                elseNode = item;
            }
        }
    }
    AccessKind condKind;
    TypeIR* condType = Expr(condNode, &condKind);
    if (!isBoolType(condType))
    {
        semanticError(condNode, "if 条件表达式必须是 bool 类型。");
    }
    Body(thenNode);
    Body(elseNode);
}

void SemanticAnalyzer::whilestatement(TreeNode* t)
{
    if (t == nullptr) return;
    TreeNode* condNode = nullptr;
    TreeNode* bodyNode = nullptr;
    for (TreeNode* item : t->children)
    {
        if (item == nullptr) continue;
        if (condNode == nullptr && (item->name == "OpK" || item->name == "VarK" || item->name == "ConstK"))
        {
            condNode = item;
        }
        else if (item->name == "StmtK" && item->value == "StmListK")
        {
            bodyNode = item;
        }
    }
    AccessKind condKind;
    TypeIR* condType = Expr(condNode, &condKind);
    if (!isBoolType(condType))
    {
        semanticError(condNode, "while 条件表达式必须是 bool 类型。");
    }
    Body(bodyNode);
}

void SemanticAnalyzer::readstatement(TreeNode* t)
{
    if (t == nullptr) return;
    TreeNode* varNode = child(t, 0);
    if (varNode == nullptr)
    {
        semanticError(t, "read 语句缺少读入变量。");
        return;
    }
    if (varNode->name != "VarK" && varNode->name != "IdK")
    {
        semanticError(varNode, "read 语句的对象必须是变量。");
        return;
    }
    AccessKind kind;
    TypeIR* type = Expr(varNode, &kind);
    if (type == nullptr || type->typeKind == TypeKind::UnknownTy) return;
    if (kind != AccessKind::Indir)
    {
        semanticError(varNode, "read 语句的对象必须是可赋值变量。");
    }
}

void SemanticAnalyzer::writestatement(TreeNode* t)
{
    if (t == nullptr) return;
    TreeNode* expNode = child(t, 0);
    if (expNode == nullptr)
    {
        semanticError(t, "write 语句缺少输出表达式。");
        return;
    }
    AccessKind kind;
    TypeIR* expType = Expr(expNode, &kind);
    if (expType == nullptr || expType->typeKind == TypeKind::UnknownTy) return;
    if (isBoolType(expType))
    {
        semanticError(expNode, "write 语句不能输出 bool 类型表达式。");
    }
}

void SemanticAnalyzer::returnstatement(TreeNode* t)
{
    if (t == nullptr) return;
}