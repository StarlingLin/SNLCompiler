#ifndef LEXER_H
#define LEXER_H

// 定义词法分析器类，读取字符并生成记号流

#include "Token.h"
#include "InputBuffer.h"

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

// DFA的几个状态
enum class State
{
    START,
    INASSIGN,
    INCOMMENT,
    INNUM,
    INID,
    INCHAR,
    INRANGE,
    DONE
};

class Lexer
{
private:
    InputBuffer input;
    vector<Token> tokens;
    unordered_map<string, LexType> reservedWords;   // 保留字

public:
    Lexer();
    bool loadFile(const string& fileName);
    vector<Token> getTokenList();
    void printTokenList() const;
    bool writeTokenListToFile(const string& fileName) const;

private:
    void initReservedWords();   // 初始化保留字表
    LexType reservedLookup(const string& word);
    Token getToken();
};

#endif