#ifndef PARSER_H
#define PARSER_H

// 定义语法分析器，递归下降方法

#include "Token.h"
#include "TreeNode.h"

#include <vector>
#include <string>

class Parser
{
private:
    vector<Token> tokens;   // 记号流
    size_t current;         // 当前记号的索引
    bool errorFlag;         // 标记语法错误

public:
    Parser(const vector<Token>& tokens);
    TreeNode* parse();
    bool hasError() const;

private:
    Token peek() const;     // 查看当前记号
    Token previous() const; // 查看上一个记号
    bool check(LexType type) const; // 检查当前记号类型是否匹配
    bool isAtEnd() const;   // 检查是否到达记号流尾
    Token advance();        // 记号前进
    Token consume(LexType type, const string& message); // 如果类型匹配则记号前进，否则报错
    void syntaxError(const string& message);            // 报语法错误

private:
    // 递归下降部分，每个函数对应非终结符，返回语法树节点
    TreeNode* parseProgram();   // 入口
    TreeNode* program();        // 总程序处理分析程序
    TreeNode* programHead();    // 程序头部分处理分析程序
    TreeNode* programName();    //
    TreeNode* declarePart();    // 程序声明部分处理分析程序
    TreeNode* typeDec();        // 类型声明处理分析程序
    TreeNode* typeDeclaration();// 类型声明中的其他函数
    TreeNode* typeDecList();    // 类型声明中的其他函数
    TreeNode* typeDecMore();    // 类型声明中的其他函数
    TreeNode* typeId();         // 类型声明中新声明的类型名称处理分析程序
    TreeNode* typeDef();        // 类型处理分析程序
    TreeNode* baseType();       // 基本类型处理分析程序
    TreeNode* structureType();  // 结构类型处理分析程序
    TreeNode* arrayType();      // 数组类型处理分析程序
    TreeNode* recType();        // 记录类型处理分析程序
    TreeNode* fieldDecList();   // 记录类型中域声明处理分析程序
    TreeNode* fieldDecMore();   // 记录类型中其他域声明处理分析程序
    TreeNode* idList();         // 记录类型域中标识符名处理分析程序
    TreeNode* idMore();         // 记录类型域中其他标识符名处理分析程序
    TreeNode* varDec();         // 变量声明处理分析程序
    TreeNode* varDeclaration(); // 变量声明部分处理程序
    TreeNode* varDecList();     // 变量声明部分处理程序
    TreeNode* varDecMore();     // 变量声明部分处理程序
    TreeNode* varIdList();      // 变量声明部分变量名部分处理程序
    TreeNode* varIdMore();      // 变量声明部分变量名部分处理程序
    TreeNode* procDec();        // 过程声明部分总处理分析程序
    TreeNode* procDeclaration();// 过程声明部分具体处理分析程序
    TreeNode* procName();
    TreeNode* paramList();      // 过程声明中的参数声明部分的处理分析程序
    TreeNode* paramDecList();   // 过程声明中的参数声明其他部分的处理分析程序
    TreeNode* paramMore();      // 过程声明中的参数声明其他部分的处理分析程序
    TreeNode* param();          // 过程声明中的参数声明中的参数部分的处理分析程序
    TreeNode* formList();       // 过程声明中的参数声明中的参数名部分的处理分析程序
    TreeNode* fidMore();        // 过程声明中的参数声明中的参数名部分的处理分析程序
    TreeNode* procDecPart();    // 过程声明部分处理处理程序
    TreeNode* procBody();       // 过程体部分处理处理程序
    TreeNode* programBody();    // 主程序体部分处理分析程序
    TreeNode* stmList();        // 语句序列部分处理分析程序
    TreeNode* stmMore();        // 更多语句部分处理分析程序
    TreeNode* stm();            // 语句递归处理分析程序
    TreeNode* assCall();        // 赋值语句和函数调用语句部分的处理分析程序
    TreeNode* assignmentRest(); // 赋值语句处理分析程序
    TreeNode* conditionalStm(); // 条件语句处理分析程序
    TreeNode* loopStm();        // 循环语句处理分析程序
    TreeNode* inputStm();       // 输入语句处理分析程序
    TreeNode* outputStm();      // 输出语句处理分析程序
    TreeNode* returnStm();      // 返回语句处理分析程序
    TreeNode* callStmRest();    // 函数调用语句处理分析程序 
    TreeNode* actParamList();   // 实参部分处理分析程序
    TreeNode* actParamMore();   // 更多实参部分处理分析程序
    TreeNode* exp();            // 表达式递归处理分析程序
    TreeNode* simpleExp();      // 简单表达式递归处理分析程序
    TreeNode* term();           // 项递归处理分析程序
    TreeNode* factor();         // 因子递归处理分析程序
    TreeNode* variable();       // 变量处理分析程序
    TreeNode* variMore();       // 变量其他部分处理分析程序
    TreeNode* fieldVar();       // 域变量处理分析程序
    TreeNode* fieldVarMore();   // 域变量其他部分处理分析程序
};

#endif