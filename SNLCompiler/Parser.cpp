#include "Parser.h"

#include <iostream>

Parser::Parser(const vector<Token>& tokens)
{
    this->tokens = tokens;
    this->current = 0;
    this->errorFlag = false;
}

TreeNode* Parser::parse()
{
    TreeNode* root = parseProgram();
    if (root == nullptr) return nullptr;
    if (!check(LexType::ENDFILE))
    {
        syntaxError("程序结束标志\'.\'后不应该还有其他内容");
        return nullptr;
    }

    return root;
}

bool Parser::hasError() const
{
    return errorFlag;
}

Token Parser::peek() const
{
    if (current >= tokens.size())
    {
        Token token;
        token.line = -1;
        token.type = LexType::ENDFILE;
        token.sem = "EOF";
        return token;
    }

    return tokens[current];
}

Token Parser::previous() const
{
    if (current == 0 || tokens.empty())
    {
        Token token;
        token.line = -1;
        token.type = LexType::ERROR;
        token.sem = "";
        return token;
    }

    return tokens[current - 1];
}

bool Parser::check(LexType type) const
{
    return peek().type == type;
}

bool Parser::isAtEnd() const
{
    return check(LexType::ENDFILE);
}

Token Parser::advance()
{
    if (!isAtEnd()) current++;
    return previous();
}

Token Parser::consume(LexType type, const string& message)
{
    if (check(type))
    {
        return advance();
    }
    syntaxError(message);
    Token token;
    token.line = peek().line;
    token.type = LexType::ERROR;
    token.sem = peek().sem;
    if (!isAtEnd())
    {
        advance();
    }
    return token;
}

void Parser::syntaxError(const string& message)
{
    errorFlag = true;
    Token token = peek();
    cerr << "语法错误：第 " << token.line << " 行，"
              << message << "，当前单词为 "
              << lexTypeToString(token.type) << " "
              << token.sem << endl;
}

TreeNode* Parser::parseProgram()
{
    TreeNode* root = program();
    if (root == nullptr)
    {
        syntaxError("总程序处理失败");
        return nullptr;
    }
    return root;
}

TreeNode* Parser::program()
{
    TreeNode* root = new TreeNode("Program");
    TreeNode* headNode = programHead();
    if (headNode == nullptr)
    {
        syntaxError("程序头处理失败");
        delete root;
        return nullptr;
    }
    root->addChild(headNode);
    TreeNode* declareNode = declarePart();
    if (declareNode == nullptr)
    {
        syntaxError("程序声明部分处理失败");
        delete root;
        return nullptr;
    }
    root->addChild(declareNode);
    TreeNode* bodyNode = programBody();
    if (bodyNode == nullptr)
    {
        syntaxError("程序体处理失败");
        delete root;
        return nullptr;
    }
    root->addChild(bodyNode);
    Token dotToken = consume(LexType::DOT, "总程序结尾缺少\'.\'");
    if (dotToken.type == LexType::ERROR)
    {
        delete root;
        return nullptr;
    }
    root->addChild(new TreeNode("DOT", dotToken.sem, dotToken.line));

    return root;
}

TreeNode* Parser::programHead()
{
    TreeNode* node = new TreeNode("ProgramHead");
    Token programToken = consume(LexType::PROGRAM, "程序头应以 program 开始");
    if (programToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("PROGRAM", programToken.sem, programToken.line));
    TreeNode* nameNode = programName();
    if (nameNode == nullptr)
    {
        syntaxError("program 后面缺少程序名");
        delete node;
        return nullptr;
    }
    node->addChild(nameNode);

    return node;
}

TreeNode* Parser::programName()
{
    TreeNode* node = new TreeNode("ProgramName");
    Token idToken = consume(LexType::ID, "program 后面应为程序名标识符");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));

    return node;
}

TreeNode* Parser::declarePart()
{
    TreeNode* node = new TreeNode("DeclarePart");
    TreeNode* typeNode = typeDec();
    if (typeNode == nullptr)
    {
        syntaxError("类型声明部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(typeNode);
    TreeNode* varNode = varDec();
    if (varNode == nullptr)
    {
        syntaxError("变量声明部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(varNode);
    TreeNode* procNode = procDec();
    if (procNode == nullptr)
    {
        syntaxError("过程声明部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(procNode);

    return node;
}

TreeNode* Parser::typeDec()
{
    TreeNode* node = new TreeNode("TypeDec");
    if (check(LexType::TYPE))
    {
        TreeNode* typeDeclarationNode = typeDeclaration();
        if (typeDeclarationNode == nullptr)
        {
            syntaxError("类型声明处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(typeDeclarationNode);
    }
    else if (check(LexType::VAR) ||
             check(LexType::PROCEDURE) ||
             check(LexType::BEGIN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else
    {
        syntaxError("类型声明部分出现非法单词");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::typeDeclaration()
{
    TreeNode* node = new TreeNode("TypeDeclaration");
    Token typeToken = consume(LexType::TYPE, "类型声明应以 type 开始");
    if (typeToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("TYPE", typeToken.sem, typeToken.line));
    TreeNode* listNode = typeDecList();
    if (listNode == nullptr)
    {
        syntaxError("type 后面缺少类型声明列表");
        delete node;
        return nullptr;
    }
    node->addChild(listNode);

    return node;
}

TreeNode* Parser::typeDecList()
{
    TreeNode* node = new TreeNode("TypeDecList");
    TreeNode* idNode = typeId();
    if (idNode == nullptr)
    {
        syntaxError("类型声明中缺少类型标识符");
        delete node;
        return nullptr;
    }
    node->addChild(idNode);
    Token eqToken = consume(LexType::EQ, "类型标识符后面缺少 =");
    if (eqToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("EQ", eqToken.sem, eqToken.line));
    TreeNode* defNode = typeDef();
    if (defNode == nullptr)
    {
        syntaxError("= 后面缺少类型定义");
        delete node;
        return nullptr;
    }
    node->addChild(defNode);
    Token semiToken = consume(LexType::SEMI, "类型声明结尾缺少 ;");
    if (semiToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("SEMI", semiToken.sem, semiToken.line));
    TreeNode* moreNode = typeDecMore();
    if (moreNode == nullptr)
    {
        syntaxError("类型声明后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::typeDecMore()
{
    TreeNode* node = new TreeNode("TypeDecMore");
    if (check(LexType::ID))
    {
        TreeNode* listNode = typeDecList();
        if (listNode == nullptr)
        {
            syntaxError("后续类型声明处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else if (check(LexType::VAR) ||
             check(LexType::PROCEDURE) ||
             check(LexType::BEGIN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else
    {
        syntaxError("类型声明后出现非法单词");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::typeId()
{
    TreeNode* node = new TreeNode("TypeId");
    Token idToken = consume(LexType::ID, "类型声明中应出现类型标识符");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));

    return node;
}

TreeNode* Parser::typeDef()
{
    TreeNode* node = new TreeNode("TypeDef");
    if (check(LexType::INTEGER) || check(LexType::CHAR))
    {
        TreeNode* baseNode = baseType();
        if (baseNode == nullptr)
        {
            syntaxError("基本类型处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(baseNode);
    }
    else if (check(LexType::ARRAY) || check(LexType::RECORD))
    {
        TreeNode* structureNode = structureType();
        if (structureNode == nullptr)
        {
            syntaxError("结构类型处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(structureNode);
    }
    else if (check(LexType::ID))
    {
        Token idToken = consume(LexType::ID, "类型定义中应出现类型名");
        if (idToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("ID", idToken.sem, idToken.line));
    }
    else
    {
        syntaxError("类型定义非法，应为已有类型名");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::baseType()
{
    TreeNode* node = new TreeNode("BaseType");
    if (check(LexType::INTEGER))
    {
        Token token = consume(LexType::INTEGER, "基本类型应为 integer");
        if (token.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("INTEGER", token.sem, token.line));
    }
    else if (check(LexType::CHAR))
    {
        Token token = consume(LexType::CHAR, "基本类型应为 char");
        if (token.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("CHAR", token.sem, token.line));
    }
    else
    {
        syntaxError("基本类型应为 integer 或 char");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::structureType()
{
    TreeNode* node = new TreeNode("StructureType");
    if (check(LexType::ARRAY))
    {
        TreeNode* arrayNode = arrayType();
        if (arrayNode == nullptr)
        {
            syntaxError("数组类型处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(arrayNode);
    }
    else if (check(LexType::RECORD))
    {
        TreeNode* recNode = recType();
        if (recNode == nullptr)
        {
            syntaxError("记录类型处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(recNode);
    }
    else
    {
        syntaxError("结构类型应为 array 或 record");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::arrayType()
{
    TreeNode* node = new TreeNode("ArrayType");
    Token arrayToken = consume(LexType::ARRAY, "数组类型应以 array 开始");
    if (arrayToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ARRAY", arrayToken.sem, arrayToken.line));
    Token lmidToken = consume(LexType::LMIDPAREN, "array 后缺少 [");
    if (lmidToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("LMIDPAREN", lmidToken.sem, lmidToken.line));
    Token lowToken = consume(LexType::INTC, "数组下界应为无符号整数");
    if (lowToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("LOW", lowToken.sem, lowToken.line));
    Token rangeToken = consume(LexType::UNDERANGE, "数组下界后缺少 ..");
    if (rangeToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("UNDERANGE", rangeToken.sem, rangeToken.line));
    Token topToken = consume(LexType::INTC, "数组上界应为无符号整数");
    if (topToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("TOP", topToken.sem, topToken.line));
    Token rmidToken = consume(LexType::RMIDPAREN, "数组上界后缺少 ]");
    if (rmidToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("RMIDPAREN", rmidToken.sem, rmidToken.line));
    Token ofToken = consume(LexType::OF, "数组类型中缺少 of");
    if (ofToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("OF", ofToken.sem, ofToken.line));
    TreeNode* baseNode = baseType();
    if (baseNode == nullptr)
    {
        syntaxError("数组元素类型处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(baseNode);

    return node;
}

TreeNode* Parser::recType()
{
    TreeNode* node = new TreeNode("RecType");
    Token recordToken = consume(LexType::RECORD, "记录类型应以 record 开始");
    if (recordToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("RECORD", recordToken.sem, recordToken.line));
    TreeNode* fieldNode = fieldDecList();
    if (fieldNode == nullptr)
    {
        syntaxError("record 中缺少域声明");
        delete node;
        return nullptr;
    }
    node->addChild(fieldNode);
    Token endToken = consume(LexType::END, "record 结尾缺少 end");
    if (endToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("END", endToken.sem, endToken.line));

    return node;
}

TreeNode* Parser::fieldDecList()
{
    TreeNode* node = new TreeNode("FieldDecList");
    if (check(LexType::INTEGER) || check(LexType::CHAR))
    {
        TreeNode* baseNode = baseType();
        if (baseNode == nullptr)
        {
            syntaxError("记录域中的基本类型处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(baseNode);
        TreeNode* idNode = idList();
        if (idNode == nullptr)
        {
            syntaxError("记录域中缺少标识符列表");
            delete node;
            return nullptr;
        }
        node->addChild(idNode);
        Token semiToken = consume(LexType::SEMI, "记录域声明后缺少 ;");
        if (semiToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("SEMI", semiToken.sem, semiToken.line));
        TreeNode* moreNode = fieldDecMore();
        if (moreNode == nullptr)
        {
            syntaxError("记录域后续声明处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(moreNode);
    }
    else if (check(LexType::ARRAY))
    {
        TreeNode* arrayNode = arrayType();
        if (arrayNode == nullptr)
        {
            syntaxError("记录域中的数组类型处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(arrayNode);
        TreeNode* idNode = idList();
        if (idNode == nullptr)
        {
            syntaxError("记录域中缺少标识符列表");
            delete node;
            return nullptr;
        }
        node->addChild(idNode);
        Token semiToken = consume(LexType::SEMI, "记录域声明后缺少 ;");
        if (semiToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("SEMI", semiToken.sem, semiToken.line));
        TreeNode* moreNode = fieldDecMore();
        if (moreNode == nullptr)
        {
            syntaxError("记录域后续声明处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(moreNode);
    }
    else
    {
        syntaxError("记录域声明应以 integer、char 或 array 开始");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::fieldDecMore()
{
    TreeNode* node = new TreeNode("FieldDecMore");
    if (check(LexType::END))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::INTEGER) ||
             check(LexType::CHAR) ||
             check(LexType::ARRAY))
    {
        TreeNode* fieldNode = fieldDecList();
        if (fieldNode == nullptr)
        {
            syntaxError("记录类型中的后续域声明处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(fieldNode);
    }
    else
    {
        syntaxError("记录域声明后应为 end、integer、char 或 array");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::idList()
{
    TreeNode* node = new TreeNode("IdList");
    Token idToken = consume(LexType::ID, "标识符列表中缺少标识符");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));
    TreeNode* moreNode = idMore();
    if (moreNode == nullptr)
    {
        syntaxError("标识符列表后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::idMore()
{
    TreeNode* node = new TreeNode("IdMore");
    if (check(LexType::SEMI))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::COMMA))
    {
        Token commaToken = consume(LexType::COMMA, "标识符之间缺少 ,");
        if (commaToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("COMMA", commaToken.sem, commaToken.line));
        TreeNode* listNode = idList();
        if (listNode == nullptr)
        {
            syntaxError(", 后面缺少标识符");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else
    {
        syntaxError("标识符后应为 ; 或 ,");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::varDec()
{
    TreeNode* node = new TreeNode("VarDec");
    if (check(LexType::VAR))
    {
        TreeNode* varDeclarationNode = varDeclaration();
        if (varDeclarationNode == nullptr)
        {
            syntaxError("变量声明处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(varDeclarationNode);
    }
    else if (check(LexType::PROCEDURE) || check(LexType::BEGIN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else
    {
        syntaxError("变量声明部分应为 var、procedure 或 begin");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::varDeclaration()
{
    TreeNode* node = new TreeNode("VarDeclaration");
    Token varToken = consume(LexType::VAR, "变量声明应以 var 开始");
    if (varToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("VAR", varToken.sem, varToken.line));
    TreeNode* listNode = varDecList();
    if (listNode == nullptr)
    {
        syntaxError("var 后面缺少变量声明列表");
        delete node;
        return nullptr;
    }
    node->addChild(listNode);

    return node;
}

TreeNode* Parser::varDecList()
{
    TreeNode* node = new TreeNode("VarDecList");
    TreeNode* typeNode = typeDef();
    if (typeNode == nullptr)
    {
        syntaxError("变量声明中缺少类型定义");
        delete node;
        return nullptr;
    }
    node->addChild(typeNode);
    TreeNode* idListNode = varIdList();
    if (idListNode == nullptr)
    {
        syntaxError("变量声明中缺少变量名列表");
        delete node;
        return nullptr;
    }
    node->addChild(idListNode);
    Token semiToken = consume(LexType::SEMI, "变量声明结尾缺少 ;");
    if (semiToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("SEMI", semiToken.sem, semiToken.line));
    TreeNode* moreNode = varDecMore();
    if (moreNode == nullptr)
    {
        syntaxError("变量声明后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::varDecMore()
{
    TreeNode* node = new TreeNode("VarDecMore");
    if (check(LexType::PROCEDURE) || check(LexType::BEGIN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::INTEGER) ||
             check(LexType::CHAR) ||
             check(LexType::ARRAY) ||
             check(LexType::RECORD) ||
             check(LexType::ID))
    {
        TreeNode* listNode = varDecList();

        if (listNode == nullptr)
        {
            syntaxError("后续变量声明处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else
    {
        syntaxError("变量声明后应为 procedure、begin 或新的变量声明");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::varIdList()
{
    TreeNode* node = new TreeNode("VarIdList");
    Token idToken = consume(LexType::ID, "变量声明中缺少变量名");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));
    TreeNode* moreNode = varIdMore();
    if (moreNode == nullptr)
    {
        syntaxError("变量名列表后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::varIdMore()
{
    TreeNode* node = new TreeNode("VarIdMore");
    if (check(LexType::SEMI))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::COMMA))
    {
        Token commaToken = consume(LexType::COMMA, "变量名之间缺少 ,");
        if (commaToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("COMMA", commaToken.sem, commaToken.line));
        TreeNode* listNode = varIdList();
        if (listNode == nullptr)
        {
            syntaxError(", 后面缺少变量名");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else
    {
        syntaxError("变量名后应为 ; 或 ,");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::procDec()
{
    TreeNode* node = new TreeNode("ProcDec");
    if (check(LexType::BEGIN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::PROCEDURE))
    {
        TreeNode* declarationNode = procDeclaration();
        if (declarationNode == nullptr)
        {
            syntaxError("过程声明处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(declarationNode);
    }
    else
    {
        syntaxError("过程声明部分应为 procedure 或 begin");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::procDeclaration()
{
    TreeNode* node = new TreeNode("ProcDeclaration");
    Token procedureToken = consume(LexType::PROCEDURE, "过程声明应以 procedure 开始");
    if (procedureToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("PROCEDURE", procedureToken.sem, procedureToken.line));
    TreeNode* nameNode = procName();
    if (nameNode == nullptr)
    {
        syntaxError("procedure 后缺少过程名");
        delete node;
        return nullptr;
    }
    node->addChild(nameNode);
    Token lparenToken = consume(LexType::LPAREN, "过程名后缺少 (");
    if (lparenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("LPAREN", lparenToken.sem, lparenToken.line));
    TreeNode* paramListNode = paramList();
    if (paramListNode == nullptr)
    {
        syntaxError("过程参数列表处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(paramListNode);
    Token rparenToken = consume(LexType::RPAREN, "参数列表后缺少 )");
    if (rparenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("RPAREN", rparenToken.sem, rparenToken.line));
    Token semiToken = consume(LexType::SEMI, "过程头后缺少 ;");
    if (semiToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("SEMI", semiToken.sem, semiToken.line));
    TreeNode* decPartNode = procDecPart();
    if (decPartNode == nullptr)
    {
        syntaxError("过程内部声明部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(decPartNode);
    TreeNode* bodyNode = procBody();
    if (bodyNode == nullptr)
    {
        syntaxError("过程体处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(bodyNode);

    return node;
}

TreeNode* Parser::procName()
{
    TreeNode* node = new TreeNode("ProcName");
    Token idToken = consume(LexType::ID, "过程名应为标识符");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));

    return node;
}

TreeNode* Parser::paramList()
{
    TreeNode* node = new TreeNode("ParamList");
    if (check(LexType::RPAREN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::INTEGER) ||
             check(LexType::CHAR) ||
             check(LexType::ARRAY) ||
             check(LexType::RECORD) ||
             check(LexType::ID) ||
             check(LexType::VAR))
    {
        TreeNode* listNode = paramDecList();
        if (listNode == nullptr)
        {
            syntaxError("参数声明列表处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else
    {
        syntaxError("参数列表应为空，或以类型名 / var 开始");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::paramDecList()
{
    TreeNode* node = new TreeNode("ParamDecList");
    TreeNode* paramNode = param();
    if (paramNode == nullptr)
    {
        syntaxError("参数声明处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(paramNode);
    TreeNode* moreNode = paramMore();
    if (moreNode == nullptr)
    {
        syntaxError("参数声明后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::paramMore()
{
    TreeNode* node = new TreeNode("ParamMore");
    if (check(LexType::RPAREN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::SEMI))
    {
        Token semiToken = consume(LexType::SEMI, "参数声明之间缺少 ;");
        if (semiToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("SEMI", semiToken.sem, semiToken.line));
        TreeNode* listNode = paramDecList();
        if (listNode == nullptr)
        {
            syntaxError("; 后面缺少参数声明");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else
    {
        syntaxError("参数声明后应为 ) 或 ;");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::param()
{
    TreeNode* node = new TreeNode("Param");
    if (check(LexType::VAR))
    {
        Token varToken = consume(LexType::VAR, "引用参数应以 var 开始");
        if (varToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("VAR", varToken.sem, varToken.line));
        TreeNode* typeNode = typeDef();
        if (typeNode == nullptr)
        {
            syntaxError("var 后面缺少参数类型");
            delete node;
            return nullptr;
        }
        node->addChild(typeNode);
        TreeNode* formNode = formList();
        if (formNode == nullptr)
        {
            syntaxError("参数类型后缺少形式参数名");
            delete node;
            return nullptr;
        }
        node->addChild(formNode);
    }
    else if (check(LexType::INTEGER) ||
             check(LexType::CHAR) ||
             check(LexType::ARRAY) ||
             check(LexType::RECORD) ||
             check(LexType::ID))
    {
        TreeNode* typeNode = typeDef();
        if (typeNode == nullptr)
        {
            syntaxError("参数声明中缺少参数类型");
            delete node;
            return nullptr;
        }
        node->addChild(typeNode);
        TreeNode* formNode = formList();
        if (formNode == nullptr)
        {
            syntaxError("参数类型后缺少形式参数名");
            delete node;
            return nullptr;
        }
        node->addChild(formNode);
    }
    else
    {
        syntaxError("参数声明应以类型名或 var 开始");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::formList()
{
    TreeNode* node = new TreeNode("FormList");
    Token idToken = consume(LexType::ID, "形式参数列表中缺少标识符");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));
    TreeNode* moreNode = fidMore();
    if (moreNode == nullptr)
    {
        syntaxError("形式参数列表后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::fidMore()
{
    TreeNode* node = new TreeNode("FidMore");
    if (check(LexType::SEMI) || check(LexType::RPAREN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::COMMA))
    {
        Token commaToken = consume(LexType::COMMA, "形式参数之间缺少 ,");
        if (commaToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("COMMA", commaToken.sem, commaToken.line));
        TreeNode* listNode = formList();
        if (listNode == nullptr)
        {
            syntaxError(", 后面缺少形式参数");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else
    {
        syntaxError("形式参数后应为 ,、; 或 )");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::procDecPart()
{
    TreeNode* node = new TreeNode("ProcDecPart");
    TreeNode* declareNode = declarePart();
    if (declareNode == nullptr)
    {
        syntaxError("过程声明中的内部声明部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(declareNode);

    return node;
}

TreeNode* Parser::procBody()
{
    TreeNode* node = new TreeNode("ProcBody");
    TreeNode* bodyNode = programBody();
    if (bodyNode == nullptr)
    {
        syntaxError("过程体处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(bodyNode);

    return node;
}

TreeNode* Parser::programBody()
{
    TreeNode* node = new TreeNode("ProgramBody");
    Token beginToken = consume(LexType::BEGIN, "程序体应以 begin 开始");
    if (beginToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("BEGIN", beginToken.sem, beginToken.line));
    TreeNode* stmListNode = stmList();
    if (stmListNode == nullptr)
    {
        syntaxError("语句序列处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(stmListNode);
    Token endToken = consume(LexType::END, "程序体应以 end 结束");
    if (endToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("END", endToken.sem, endToken.line));

    return node;
}

TreeNode* Parser::stmList()
{
    TreeNode* node = new TreeNode("StmList");
    TreeNode* stmNode = stm();
    if (stmNode == nullptr)
    {
        syntaxError("语句序列中缺少语句");
        delete node;
        return nullptr;
    }
    node->addChild(stmNode);
    TreeNode* moreNode = stmMore();
    if (moreNode == nullptr)
    {
        syntaxError("语句序列后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::stmMore()
{
    TreeNode* node = new TreeNode("StmMore");
    if (check(LexType::END) ||
        check(LexType::ENDWH) ||
        check(LexType::ELSE) ||
        check(LexType::FI))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::SEMI))
    {
        Token semiToken = consume(LexType::SEMI, "语句之间缺少 ;");
        if (semiToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("SEMI", semiToken.sem, semiToken.line));
        TreeNode* listNode = stmList();
        if (listNode == nullptr)
        {
            syntaxError("; 后面缺少语句");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else
    {
        syntaxError("语句后应为 ;、end、endwh、else 或 fi");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::stm()
{
    TreeNode* node = new TreeNode("Stm");
    if (check(LexType::IF))
    {
        TreeNode* conditionalNode = conditionalStm();
        if (conditionalNode == nullptr)
        {
            delete node;
            return nullptr;
        }
        node->addChild(conditionalNode);
    }
    else if (check(LexType::WHILE))
    {
        TreeNode* loopNode = loopStm();
        if (loopNode == nullptr)
        {
            delete node;
            return nullptr;
        }
        node->addChild(loopNode);
    }
    else if (check(LexType::RETURN))
    {
        TreeNode* returnNode = returnStm();
        if (returnNode == nullptr)
        {
            delete node;
            return nullptr;
        }
        node->addChild(returnNode);
    }
    else if (check(LexType::READ))
    {
        TreeNode* inputNode = inputStm();
        if (inputNode == nullptr)
        {
            delete node;
            return nullptr;
        }
        node->addChild(inputNode);
    }
    else if (check(LexType::WRITE))
    {
        TreeNode* outputNode = outputStm();
        if (outputNode == nullptr)
        {
            delete node;
            return nullptr;
        }
        node->addChild(outputNode);
    }
    else if (check(LexType::ID))
    {
        TreeNode* assCallNode = assCall();
        if (assCallNode == nullptr)
        {
            syntaxError("标识符开头的语句应为赋值语句或过程调用语句");
            delete node;
            return nullptr;
        }
        node->addChild(assCallNode);
    }
    else
    {
        syntaxError("非法语句开始符");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::assCall()
{
    TreeNode* node = new TreeNode("AssCall");
    Token idToken = consume(LexType::ID, "赋值语句或过程调用语句应以标识符开始");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));
    if (check(LexType::LPAREN))
    {
        TreeNode* callNode = callStmRest();
        if (callNode == nullptr)
        {
            syntaxError("过程调用语句处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(callNode);
    }
    else if (check(LexType::ASSIGN) ||
             check(LexType::LMIDPAREN) ||
             check(LexType::DOT))
    {
        TreeNode* moreNode = variMore();
        if (moreNode == nullptr)
        {
            syntaxError("赋值语句左部变量处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(moreNode);
        TreeNode* assignNode = assignmentRest();
        if (assignNode == nullptr)
        {
            syntaxError("赋值语句右部处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(assignNode);
    }
    else
    {
        syntaxError("标识符后应为 :=、[、. 或 (");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::assignmentRest()
{
    TreeNode* node = new TreeNode("AssignmentRest");
    Token assignToken = consume(LexType::ASSIGN, "赋值语句中缺少 :=");
    if (assignToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ASSIGN", assignToken.sem, assignToken.line));
    TreeNode* expNode = exp();
    if (expNode == nullptr)
    {
        syntaxError("赋值号后缺少表达式");
        delete node;
        return nullptr;
    }
    node->addChild(expNode);

    return node;
}

TreeNode* Parser::conditionalStm()
{
    TreeNode* node = new TreeNode("ConditionalStm");
    Token ifToken = consume(LexType::IF, "条件语句应以 if 开始");
    if (ifToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("IF", ifToken.sem, ifToken.line));
    TreeNode* expNode = exp();
    if (expNode == nullptr)
    {
        syntaxError("if 后面缺少条件表达式");
        delete node;
        return nullptr;
    }
    node->addChild(expNode);
    Token thenToken = consume(LexType::THEN, "条件表达式后缺少 then");
    if (thenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("THEN", thenToken.sem, thenToken.line));
    TreeNode* thenStmList = stmList();
    if (thenStmList == nullptr)
    {
        syntaxError("then 后面缺少语句序列");
        delete node;
        return nullptr;
    }
    node->addChild(thenStmList);
    Token elseToken = consume(LexType::ELSE, "then 分支后缺少 else");
    if (elseToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ELSE", elseToken.sem, elseToken.line));
    TreeNode* elseStmList = stmList();
    if (elseStmList == nullptr)
    {
        syntaxError("else 后面缺少语句序列");
        delete node;
        return nullptr;
    }
    node->addChild(elseStmList);
    Token fiToken = consume(LexType::FI, "条件语句结尾缺少 fi");
    if (fiToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("FI", fiToken.sem, fiToken.line));

    return node;
}

TreeNode* Parser::loopStm()
{
    TreeNode* node = new TreeNode("LoopStm");
    Token whileToken = consume(LexType::WHILE, "循环语句应以 while 开始");
    if (whileToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("WHILE", whileToken.sem, whileToken.line));
    TreeNode* expNode = exp();
    if (expNode == nullptr)
    {
        syntaxError("while 后面缺少循环条件表达式");
        delete node;
        return nullptr;
    }
    node->addChild(expNode);
    Token doToken = consume(LexType::DO, "循环条件后缺少 do");
    if (doToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("DO", doToken.sem, doToken.line));
    TreeNode* stmListNode = stmList();
    if (stmListNode == nullptr)
    {
        syntaxError("do 后面缺少循环体语句序列");
        delete node;
        return nullptr;
    }
    node->addChild(stmListNode);
    Token endwhToken = consume(LexType::ENDWH, "while 语句结尾缺少 endwh");
    if (endwhToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ENDWH", endwhToken.sem, endwhToken.line));

    return node;
}

TreeNode* Parser::inputStm()
{
    TreeNode* node = new TreeNode("InputStm");
    Token readToken = consume(LexType::READ, "输入语句应以 read 开始");
    if (readToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("READ", readToken.sem, readToken.line));
    Token lparenToken = consume(LexType::LPAREN, "read 后缺少 (");
    if (lparenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("LPAREN", lparenToken.sem, lparenToken.line));
    TreeNode* varNode = variable();
    if (varNode == nullptr)
    {
        syntaxError("read 的参数应为变量");
        delete node;
        return nullptr;
    }
    node->addChild(varNode);
    Token rparenToken = consume(LexType::RPAREN, "read 参数后缺少 )");
    if (rparenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("RPAREN", rparenToken.sem, rparenToken.line));

    return node;
}

TreeNode* Parser::outputStm()
{
    TreeNode* node = new TreeNode("OutputStm");
    Token writeToken = consume(LexType::WRITE, "输出语句应以 write 开始");
    if (writeToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("WRITE", writeToken.sem, writeToken.line));
    Token lparenToken = consume(LexType::LPAREN, "write 后缺少 (");
    if (lparenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("LPAREN", lparenToken.sem, lparenToken.line));
    TreeNode* expNode = exp();
    if (expNode == nullptr)
    {
        syntaxError("write 的参数中缺少表达式");
        delete node;
        return nullptr;
    }
    node->addChild(expNode);
    Token rparenToken = consume(LexType::RPAREN, "write 参数后缺少 )");
    if (rparenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("RPAREN", rparenToken.sem, rparenToken.line));

    return node;
}

TreeNode* Parser::returnStm()
{
    TreeNode* node = new TreeNode("ReturnStm");
    Token returnToken = consume(LexType::RETURN, "返回语句应为 return");
    if (returnToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("RETURN", returnToken.sem, returnToken.line));

    return node;
}

TreeNode* Parser::callStmRest()
{
    TreeNode* node = new TreeNode("CallStmRest");
    Token lparenToken = consume(LexType::LPAREN, "过程调用缺少 (");
    if (lparenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("LPAREN", lparenToken.sem, lparenToken.line));
    TreeNode* paramNode = actParamList();
    if (paramNode == nullptr)
    {
        syntaxError("过程调用实参列表处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(paramNode);
    Token rparenToken = consume(LexType::RPAREN, "过程调用缺少 )");
    if (rparenToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("RPAREN", rparenToken.sem, rparenToken.line));

    return node;
}

TreeNode* Parser::actParamList()
{
    TreeNode* node = new TreeNode("ActParamList");
    if (check(LexType::RPAREN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::ID) ||
             check(LexType::INTC) ||
             check(LexType::LPAREN) ||
             check(LexType::CHARC))
    {
        TreeNode* expNode = exp();
        if (expNode == nullptr)
        {
            syntaxError("实参表达式处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(expNode);
        TreeNode* moreNode = actParamMore();
        if (moreNode == nullptr)
        {
            syntaxError("实参列表后续部分处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(moreNode);
    }
    else
    {
        syntaxError("实参列表应为空，或以表达式开始");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::actParamMore()
{
    TreeNode* node = new TreeNode("ActParamMore");
    if (check(LexType::RPAREN))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::COMMA))
    {
        Token commaToken = consume(LexType::COMMA, "实参之间缺少 ,");
        if (commaToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("COMMA", commaToken.sem, commaToken.line));
        TreeNode* listNode = actParamList();
        if (listNode == nullptr)
        {
            syntaxError(", 后面缺少实参");
            delete node;
            return nullptr;
        }
        node->addChild(listNode);
    }
    else
    {
        syntaxError("实参后应为 , 或 )");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::exp()
{
    TreeNode* node = new TreeNode("Exp");
    TreeNode* leftNode = simpleExp();
    if (leftNode == nullptr)
    {
        syntaxError("表达式左部处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(leftNode);
    if (check(LexType::LT) || check(LexType::EQ))
    {
        Token opToken = advance();
        node->addChild(new TreeNode("RelOp", opToken.sem, opToken.line));
        TreeNode* rightNode = simpleExp();
        if (rightNode == nullptr)
        {
            syntaxError("关系运算符右侧缺少简单表达式");
            delete node;
            return nullptr;
        }
        node->addChild(rightNode);
    }

    return node;
}

TreeNode* Parser::simpleExp()
{
    TreeNode* node = new TreeNode("SimpleExp");
    TreeNode* leftNode = term();
    if (leftNode == nullptr)
    {
        syntaxError("简单表达式中缺少项");
        delete node;
        return nullptr;
    }
    node->addChild(leftNode);
    while (check(LexType::PLUS) || check(LexType::MINUS))
    {
        Token opToken = advance();
        node->addChild(new TreeNode("AddOp", opToken.sem, opToken.line));
        TreeNode* rightNode = term();
        if (rightNode == nullptr)
        {
            syntaxError("加减运算符右侧缺少项");
            delete node;
            return nullptr;
        }
        node->addChild(rightNode);
    }

    return node;
}

TreeNode* Parser::term()
{
    TreeNode* node = new TreeNode("Term");
    TreeNode* leftNode = factor();
    if (leftNode == nullptr)
    {
        syntaxError("项中缺少因子");
        delete node;
        return nullptr;
    }
    node->addChild(leftNode);
    while (check(LexType::TIMES) || check(LexType::OVER))
    {
        Token opToken = advance();
        if (opToken.type == LexType::TIMES)
        {
            node->addChild(new TreeNode("TIMES", opToken.sem, opToken.line));
        }
        else
        {
            node->addChild(new TreeNode("OVER", opToken.sem, opToken.line));
        }
        TreeNode* rightNode = factor();
        if (rightNode == nullptr)
        {
            syntaxError("乘除运算符右侧缺少因子");
            delete node;
            return nullptr;
        }
        node->addChild(rightNode);
    }

    return node;
}

TreeNode* Parser::factor()
{
    TreeNode* node = new TreeNode("Factor");
    if (check(LexType::LPAREN))
    {
        Token lparenToken = consume(LexType::LPAREN, "表达式因子缺少 (");
        if (lparenToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("LPAREN", lparenToken.sem, lparenToken.line));
        TreeNode* expNode = exp();
        if (expNode == nullptr)
        {
            syntaxError("( 后面缺少表达式");
            delete node;
            return nullptr;
        }
        node->addChild(expNode);
        Token rparenToken = consume(LexType::RPAREN, "表达式因子缺少 )");
        if (rparenToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("RPAREN", rparenToken.sem, rparenToken.line));
    }
    else if (check(LexType::INTC))
    {
        Token intToken = consume(LexType::INTC, "因子应为整数常量");
        if (intToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("INTC", intToken.sem, intToken.line));
    }
    else if (check(LexType::CHARC))
    {
        Token charToken = consume(LexType::CHARC, "因子应为字符常量");
        if (charToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("CHARC", charToken.sem, charToken.line));
    }
    else if (check(LexType::ID))
    {
        TreeNode* varNode = variable();
        if (varNode == nullptr)
        {
            syntaxError("变量因子处理失败");
            delete node;
            return nullptr;
        }
        node->addChild(varNode);
    }
    else
    {
        syntaxError("因子应为变量、常量或括号表达式");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::variable()
{
    TreeNode* node = new TreeNode("Variable");
    Token idToken = consume(LexType::ID, "变量应以标识符开始");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));
    TreeNode* moreNode = variMore();
    if (moreNode == nullptr)
    {
        syntaxError("变量后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::variMore()
{
    TreeNode* node = new TreeNode("VariMore");
    if (check(LexType::ASSIGN) ||
        check(LexType::TIMES) ||
        check(LexType::EQ) ||
        check(LexType::LT) ||
        check(LexType::PLUS) ||
        check(LexType::MINUS) ||
        check(LexType::OVER) ||
        check(LexType::RPAREN) ||
        check(LexType::RMIDPAREN) ||
        check(LexType::SEMI) ||
        check(LexType::COMMA) ||
        check(LexType::THEN) ||
        check(LexType::ELSE) ||
        check(LexType::FI) ||
        check(LexType::DO) ||
        check(LexType::ENDWH) ||
        check(LexType::END))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::LMIDPAREN))
    {
        Token lmidToken = consume(LexType::LMIDPAREN, "数组变量缺少 [");
        if (lmidToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("LMIDPAREN", lmidToken.sem, lmidToken.line));
        TreeNode* expNode = exp();
        if (expNode == nullptr)
        {
            syntaxError("数组下标中缺少表达式");
            delete node;
            return nullptr;
        }
        node->addChild(expNode);
        Token rmidToken = consume(LexType::RMIDPAREN, "数组变量缺少 ]");
        if (rmidToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("RMIDPAREN", rmidToken.sem, rmidToken.line));
    }
    else if (check(LexType::DOT))
    {
        Token dotToken = consume(LexType::DOT, "记录变量缺少 .");
        if (dotToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("DOT", dotToken.sem, dotToken.line));
        TreeNode* fieldNode = fieldVar();
        if (fieldNode == nullptr)
        {
            syntaxError(". 后面缺少域变量");
            delete node;
            return nullptr;
        }
        node->addChild(fieldNode);
    }
    else
    {
        syntaxError("变量后续部分非法");
        delete node;
        return nullptr;
    }

    return node;
}

TreeNode* Parser::fieldVar()
{
    TreeNode* node = new TreeNode("FieldVar");
    Token idToken = consume(LexType::ID, "记录域变量应为标识符");
    if (idToken.type == LexType::ERROR)
    {
        delete node;
        return nullptr;
    }
    node->addChild(new TreeNode("ID", idToken.sem, idToken.line));
    TreeNode* moreNode = fieldVarMore();
    if (moreNode == nullptr)
    {
        syntaxError("记录域变量后续部分处理失败");
        delete node;
        return nullptr;
    }
    node->addChild(moreNode);

    return node;
}

TreeNode* Parser::fieldVarMore()
{
    TreeNode* node = new TreeNode("FieldVarMore");
    if (check(LexType::ASSIGN) ||
        check(LexType::TIMES) ||
        check(LexType::EQ) ||
        check(LexType::LT) ||
        check(LexType::PLUS) ||
        check(LexType::MINUS) ||
        check(LexType::OVER) ||
        check(LexType::RPAREN) ||
        check(LexType::SEMI) ||
        check(LexType::COMMA) ||
        check(LexType::THEN) ||
        check(LexType::ELSE) ||
        check(LexType::FI) ||
        check(LexType::DO) ||
        check(LexType::ENDWH) ||
        check(LexType::END))
    {
        node->addChild(new TreeNode("Empty"));
    }
    else if (check(LexType::LMIDPAREN))
    {
        Token lmidToken = consume(LexType::LMIDPAREN, "域变量数组下标缺少 [");
        if (lmidToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("LMIDPAREN", lmidToken.sem, lmidToken.line));
        TreeNode* expNode = exp();
        if (expNode == nullptr)
        {
            syntaxError("域变量数组下标中缺少表达式");
            delete node;
            return nullptr;
        }
        node->addChild(expNode);
        Token rmidToken = consume(LexType::RMIDPAREN, "域变量数组下标缺少 ]");
        if (rmidToken.type == LexType::ERROR)
        {
            delete node;
            return nullptr;
        }
        node->addChild(new TreeNode("RMIDPAREN", rmidToken.sem, rmidToken.line));
    }
    else
    {
        syntaxError("域变量后续部分非法");
        delete node;
        return nullptr;
    }

    return node;
}

