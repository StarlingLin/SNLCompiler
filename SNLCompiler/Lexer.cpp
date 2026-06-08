#include "Lexer.h"

#include <iostream>
#include <fstream>
#include <cctype>

Lexer::Lexer()
{
    initReservedWords();
}

bool Lexer::loadFile(const string& fileName)
{
    tokens.clear();
    return input.loadFile(fileName);
}

vector<Token> Lexer::getTokenList()
{
    int count = 0;
    while (true)
    {
        Token token = getToken();
        tokens.push_back(token);
        count++;
        if (count > 100000)
        {
            Token errorToken;
            errorToken.line = input.getLine();
            errorToken.type = LexType::ERROR;
            errorToken.sem = "词法分析器生成过多记号，可能死循环";
            tokens.push_back(errorToken);
            break;
        }
        if (token.type == LexType::ENDFILE)
        {
            break;
        }
    }

    return tokens;
}

void Lexer::printTokenList() const
{
    cout << "行号\t词法信息\t\t语义信息" << endl;

    for (size_t i = 0; i < tokens.size(); i++)
    {
        cout << tokens[i].line << "\t"
                  << lexTypeToString(tokens[i].type) << "\t\t"
                  << tokens[i].sem << endl;
    }
}

bool Lexer::writeTokenListToFile(const string& fileName) const
{
    ofstream fout(fileName.c_str(), ios::out);
    if (!fout.is_open())
    {
        cerr << "无法打开输出文件: " << fileName << endl;
        return false;
    }
    fout << "行号\t词法信息\t\t语义信息" << endl;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        fout << tokens[i].line << "\t"
             << lexTypeToString(tokens[i].type) << "\t\t"
             << tokens[i].sem << endl;
    }
    fout.close();

    return true;
}

void Lexer::initReservedWords()
{
    reservedWords["program"] = LexType::PROGRAM;
    reservedWords["type"] = LexType::TYPE;
    reservedWords["var"] = LexType::VAR;
    reservedWords["procedure"] = LexType::PROCEDURE;
    reservedWords["begin"] = LexType::BEGIN;
    reservedWords["end"] = LexType::END;
    reservedWords["array"] = LexType::ARRAY;
    reservedWords["of"] = LexType::OF;
    reservedWords["record"] = LexType::RECORD;
    reservedWords["if"] = LexType::IF;
    reservedWords["then"] = LexType::THEN;
    reservedWords["else"] = LexType::ELSE;
    reservedWords["fi"] = LexType::FI;
    reservedWords["while"] = LexType::WHILE;
    reservedWords["do"] = LexType::DO;
    reservedWords["endwh"] = LexType::ENDWH;
    reservedWords["read"] = LexType::READ;
    reservedWords["write"] = LexType::WRITE;
    reservedWords["return"] = LexType::RETURN;
    reservedWords["integer"] = LexType::INTEGER;
    reservedWords["char"] = LexType::CHAR;
}

LexType Lexer::reservedLookup(const string& word)
{
    unordered_map<string, LexType>::iterator it = reservedWords.find(word);
    if (it != reservedWords.end()) return it->second;
    return LexType::ID;
}

Token Lexer::getToken()
{
    State state = State::START;
    string tokenString;
    int tokenLine = input.getLine();
    LexType currentType = LexType::ERROR;

    while (state != State::DONE)
    {
        char ch = input.advance();
        bool save = true;   // 当前字符是否需要保存输出
        switch (state)
        {
        case State::START:
            tokenLine = input.getLine();
            if (ch == '\0')
            {
                save = false;
                currentType = LexType::ENDFILE;
                tokenString = "EOF";
                state = State::DONE;
            }
            else if (isspace(static_cast<unsigned char>(ch)))
            {
                save = false;
            }
            else if (isalpha(static_cast<unsigned char>(ch)))
            {
                state = State::INID;
            }
            else if (isdigit(static_cast<unsigned char>(ch)))
            {
                state = State::INNUM;
            }
            else
            {
                switch (ch)
                {
                case ':':
                    state = State::INASSIGN;
                    break;

                case '{':
                    save = false;
                    tokenString.clear();
                    state = State::INCOMMENT;
                    break;

                case '.':
                    state = State::INRANGE;
                    break;

                case '\'':
                    save = false;
                    tokenString.clear();
                    state = State::INCHAR;
                    break;

                case '+':
                    currentType = LexType::PLUS;
                    state = State::DONE;
                    break;

                case '-':
                    currentType = LexType::MINUS;
                    state = State::DONE;
                    break;

                case '*':
                    currentType = LexType::TIMES;
                    state = State::DONE;
                    break;

                case '/':
                    currentType = LexType::OVER;
                    state = State::DONE;
                    break;

                case '=':
                    currentType = LexType::EQ;
                    state = State::DONE;
                    break;

                case '<':
                    currentType = LexType::LT;
                    state = State::DONE;
                    break;

                case '(':
                    currentType = LexType::LPAREN;
                    state = State::DONE;
                    break;

                case ')':
                    currentType = LexType::RPAREN;
                    state = State::DONE;
                    break;

                case '[':
                    currentType = LexType::LMIDPAREN;
                    state = State::DONE;
                    break;

                case ']':
                    currentType = LexType::RMIDPAREN;
                    state = State::DONE;
                    break;

                case ';':
                    currentType = LexType::SEMI;
                    state = State::DONE;
                    break;

                case ',':
                    currentType = LexType::COMMA;
                    state = State::DONE;
                    break;

                default:
                    currentType = LexType::ERROR;
                    state = State::DONE;
                    break;
                }
            }
            break;

        case State::INID:
            if (!isalnum(static_cast<unsigned char>(ch)))
            {
                input.backup(ch);
                save = false;
                currentType = reservedLookup(tokenString);
                state = State::DONE;
            }
            break;

        case State::INNUM:
            if (!isdigit(static_cast<unsigned char>(ch)))
            {
                input.backup(ch);
                save = false;
                currentType = LexType::INTC;
                state = State::DONE;
            }
            break;

        case State::INASSIGN:
            if (ch == '=')
            {
                currentType = LexType::ASSIGN;
            }
            else
            {
                input.backup(ch);
                save = false;
                currentType = LexType::COLON;
            }

            state = State::DONE;
            break;

        case State::INCOMMENT:
            save = false;
            if (ch == '\0')
            {
                currentType = LexType::ERROR;
                tokenString = "注释未闭合";
                state = State::DONE;
            }
            else if (ch == '}')
            {
                tokenString.clear();
                state = State::START;
            }
            break;

        case State::INRANGE:
            if (ch == '.')
            {
                currentType = LexType::UNDERANGE;
            }
            else
            {
                input.backup(ch);
                save = false;
                currentType = LexType::DOT;
            }

            state = State::DONE;
            break;

        case State::INCHAR:
            save = false;

            if (ch == '\0' || ch == '\n')
            {
                currentType = LexType::ERROR;
                tokenString = "字符未闭合";
                state = State::DONE;
            }
            else if (isalnum(static_cast<unsigned char>(ch)))
            {
                char value = ch;
                char next = input.advance();
                if (next == '\'')
                {
                    currentType = LexType::CHARC;
                    tokenString = string(1, value);
                }
                else
                {
                    input.backup(next);
                    currentType = LexType::ERROR;
                    tokenString = "必须是单个字符";
                }
                state = State::DONE;
            }
            else
            {
                currentType = LexType::ERROR;
                tokenString = "无效的字符常量";
                state = State::DONE;
            }
            break;

        case State::DONE:
            break;
        }
        if (save)
        {
            tokenString += ch;
        }
    }

    Token token;
    token.line = tokenLine;
    token.type = currentType;
    token.sem = tokenString;

    return token;
}