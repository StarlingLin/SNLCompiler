#ifndef TOKEN_H
#define TOKEN_H

// 定义词法分析器输出的记号流结构体
// 以及词法单元类型的枚举

#include <string>

using namespace std;

enum class LexType
{
    // 保留字
    PROGRAM,
    TYPE,
    VAR,
    PROCEDURE,
    BEGIN,
    END,
    ARRAY,
    OF,
    RECORD,
    IF,
    THEN,
    ELSE,
    FI,
    WHILE,
    DO,
    ENDWH,
    READ,
    WRITE,
    RETURN,
    INTEGER,
    CHAR,

    ID,     // 标识符
    INTC,   // 无符号整数
    CHARC,  // 字符标识符

    ASSIGN, // 双字符分界符
    EQ,
    LT,
    PLUS,
    MINUS,
    TIMES,
    OVER,

    LPAREN,
    RPAREN,
    LMIDPAREN,
    RMIDPAREN,

    SEMI,
    COMMA,
    DOT,
    COLON,
    UNDERANGE,

    ENDFILE,
    ERROR
};

struct Token
{
    int line;           // 行号
    LexType type;       // 词法信息
    string sem;         // 语义信息
};

string lexTypeToString(LexType type);

#endif