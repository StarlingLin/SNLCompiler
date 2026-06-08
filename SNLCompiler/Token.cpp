#include "Token.h"

string lexTypeToString(LexType type)
{
    switch (type)
    {
    case LexType::PROGRAM: return "PROGRAM";
    case LexType::TYPE: return "TYPE";
    case LexType::VAR: return "VAR";
    case LexType::PROCEDURE: return "PROCEDURE";
    case LexType::BEGIN: return "BEGIN";
    case LexType::END: return "END";
    case LexType::ARRAY: return "ARRAY";
    case LexType::OF: return "OF";
    case LexType::RECORD: return "RECORD";
    case LexType::IF: return "IF";
    case LexType::THEN: return "THEN";
    case LexType::ELSE: return "ELSE";
    case LexType::FI: return "FI";
    case LexType::WHILE: return "WHILE";
    case LexType::DO: return "DO";
    case LexType::ENDWH: return "ENDWH";
    case LexType::READ: return "READ";
    case LexType::WRITE: return "WRITE";
    case LexType::RETURN: return "RETURN";
    case LexType::INTEGER: return "INTEGER";
    case LexType::CHAR: return "CHAR";

    case LexType::ID: return "ID";
    case LexType::INTC: return "INTC";
    case LexType::CHARC: return "CHARC";

    case LexType::ASSIGN: return "ASSIGN";
    case LexType::EQ: return "EQ";
    case LexType::LT: return "LT";
    case LexType::PLUS: return "PLUS";
    case LexType::MINUS: return "MINUS";
    case LexType::TIMES: return "TIMES";
    case LexType::OVER: return "OVER";

    case LexType::LPAREN: return "LPAREN";
    case LexType::RPAREN: return "RPAREN";
    case LexType::LMIDPAREN: return "LMIDPAREN";
    case LexType::RMIDPAREN: return "RMIDPAREN";

    case LexType::SEMI: return "SEMI";
    case LexType::COMMA: return "COMMA";
    case LexType::DOT: return "DOT";
    case LexType::COLON: return "COLON";
    case LexType::UNDERANGE: return "UNDERANGE";

    case LexType::ENDFILE: return "ENDFILE";
    case LexType::ERROR: return "ERROR";
    default: return "UNKNOWN";
    }
}