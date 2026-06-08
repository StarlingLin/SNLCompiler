#include "LL1Parser.h"

#include <iostream>

LL1Parser::LL1Parser(const vector<Token>& tokens)
{
    this->tokens = tokens;
    this->current = 0;
    this->errorFlag = false;
    this->rootPointer = nullptr;
    this->currentP = nullptr;
    this->saveP = nullptr;
    this->temp = nullptr;
    createLL1Table();
}

bool LL1Parser::hasError() const
{
    return errorFlag;
}

void LL1Parser::initializeParser()
{
    current = 0;
    errorFlag = false;
    symbolStack.clear();
    treeStack.clear();
    opStack.clear();
    numStack.clear();
    expTargetStack.clear();
    rootPointer = newRootNode();
    currentP = rootPointer;
    saveP = nullptr;
    temp = nullptr;
    currentVarNode = nullptr;
    pushSymbol(LLSymbol::ENDFILE);
    pushSymbol(LLSymbol::NT_PROGRAM);
}

TreeNode* LL1Parser::parseLL()
{
    initializeParser();
    while (!isSymbolStackEmpty() && !errorFlag)
    {
        LLSymbol top = readStackTop();
        LLSymbol currentSymbol = tokenToSymbol(peek().type);
        if (isTerminal(top))
        {
            if (top == currentSymbol)
            {
                popSymbol();
                Token token = advance();
                if (top == LLSymbol::ENDFILE)
                {
                    break;
                }
            }
            else
            {
                syntaxError("终极符不匹配，期望 " + symbolToString(top) +
                            "，当前为 " + symbolToString(currentSymbol));
                break;
            }
        }
        else if (isNonTerminal(top))
        {
            int productionNumber = getTableProduction(top, currentSymbol);
            if (productionNumber == 0)
            {
                syntaxError("LL(1)分析表中找不到产生式，非终极符 " +
                            symbolToString(top) +
                            "，当前单词 " +
                            symbolToString(currentSymbol));
                break;
            }
            popSymbol();
            predict(productionNumber);
        }
        else
        {
            syntaxError("符号栈栈顶出现非法符号");
            break;
        }
    }
    if (!errorFlag && !isAtEnd())
    {
        syntaxError("语法分析结束后源程序仍有未处理单词");
    }
    if (errorFlag)
    {
        return nullptr;
    }

    return rootPointer;
}

void LL1Parser::createLL1Table()
{
    productions.clear();
    ll1Table.clear();

    addProduction(
        1,
        LLSymbol::NT_PROGRAM,
        {
            LLSymbol::NT_PROGRAM_HEAD,
            LLSymbol::NT_DECLARE_PART,
            LLSymbol::NT_PROGRAM_BODY,
            LLSymbol::DOT
        },
        {
            LLSymbol::PROGRAM
        }
    );

    addProduction(
        2,
        LLSymbol::NT_PROGRAM_HEAD,
        {
            LLSymbol::PROGRAM,
            LLSymbol::NT_PROGRAM_NAME
        },
        {
            LLSymbol::PROGRAM
        }
    );

    addProduction(
        3,
        LLSymbol::NT_PROGRAM_NAME,
        {
            LLSymbol::ID
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        4,
        LLSymbol::NT_DECLARE_PART,
        {
            LLSymbol::NT_TYPE_DEC,
            LLSymbol::NT_VAR_DEC,
            LLSymbol::NT_PROC_DEC
        },
        {
            LLSymbol::TYPE,
            LLSymbol::VAR,
            LLSymbol::PROCEDURE,
            LLSymbol::BEGIN
        }
    );

    addProduction(
        5,
        LLSymbol::NT_TYPE_DEC,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::VAR,
            LLSymbol::PROCEDURE,
            LLSymbol::BEGIN
        }
    );
        
    addProduction(
        6,
        LLSymbol::NT_TYPE_DEC,
        {
            LLSymbol::NT_TYPE_DECLARATION
        },
        {
            LLSymbol::TYPE
        }
    );

    addProduction(
        7,
        LLSymbol::NT_TYPE_DECLARATION,
        {
            LLSymbol::TYPE,
            LLSymbol::NT_TYPE_DEC_LIST
        },
        {
            LLSymbol::TYPE
        }
    );

    addProduction(
        8,
        LLSymbol::NT_TYPE_DEC_LIST,
        {
            LLSymbol::NT_TYPE_ID,
            LLSymbol::EQ,
            LLSymbol::NT_TYPE_DEF,
            LLSymbol::SEMI,
            LLSymbol::NT_TYPE_DEC_MORE
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        9,
        LLSymbol::NT_TYPE_DEC_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::VAR,
            LLSymbol::PROCEDURE,
            LLSymbol::BEGIN
        }
    );

    addProduction(
        10,
        LLSymbol::NT_TYPE_DEC_MORE,
        {
            LLSymbol::NT_TYPE_DEC_LIST
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        11,
        LLSymbol::NT_TYPE_ID,
        {
            LLSymbol::ID
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        12,
        LLSymbol::NT_TYPE_DEF,
        {
            LLSymbol::NT_BASE_TYPE
        },
        {
            LLSymbol::INTEGER,
            LLSymbol::CHAR
        }
    );

    addProduction(
        13,
        LLSymbol::NT_TYPE_DEF,
        {
            LLSymbol::NT_STRUCTURE_TYPE
        },
        {
            LLSymbol::ARRAY,
            LLSymbol::RECORD
        }
    );

    addProduction(
        14,
        LLSymbol::NT_TYPE_DEF,
        {
            LLSymbol::ID
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        15,
        LLSymbol::NT_BASE_TYPE,
        {
            LLSymbol::INTEGER
        },
        {
            LLSymbol::INTEGER
        }
    );

    addProduction(
        16,
        LLSymbol::NT_BASE_TYPE,
        {
            LLSymbol::CHAR
        },
        {
            LLSymbol::CHAR
        }
    );

    addProduction(
        17,
        LLSymbol::NT_STRUCTURE_TYPE,
        {
            LLSymbol::NT_ARRAY_TYPE
        },
        {
            LLSymbol::ARRAY
        }
    );

    addProduction(
        18,
        LLSymbol::NT_STRUCTURE_TYPE,
        {
            LLSymbol::NT_REC_TYPE
        },
        {
            LLSymbol::RECORD
        }
    );

    addProduction(
        19,
        LLSymbol::NT_ARRAY_TYPE,
        {
            LLSymbol::ARRAY,
            LLSymbol::LMIDPAREN,
            LLSymbol::NT_LOW,
            LLSymbol::UNDERANGE,
            LLSymbol::NT_TOP,
            LLSymbol::RMIDPAREN,
            LLSymbol::OF,
            LLSymbol::NT_BASE_TYPE
        },
        {
            LLSymbol::ARRAY
        }
    );

    addProduction(
        20,
        LLSymbol::NT_LOW,
        {
            LLSymbol::INTC
        },
        {
            LLSymbol::INTC
        }
    );

    addProduction(
        21,
        LLSymbol::NT_TOP,
        {
            LLSymbol::INTC
        },
        {
            LLSymbol::INTC
        }
    );

    addProduction(
        22,
        LLSymbol::NT_REC_TYPE,
        {
            LLSymbol::RECORD,
            LLSymbol::NT_FIELD_DEC_LIST,
            LLSymbol::END
        },
        {
            LLSymbol::RECORD
        }
    );

    addProduction(
        23,
        LLSymbol::NT_FIELD_DEC_LIST,
        {
            LLSymbol::NT_BASE_TYPE,
            LLSymbol::NT_ID_LIST,
            LLSymbol::SEMI,
            LLSymbol::NT_FIELD_DEC_MORE
        },
        {
            LLSymbol::INTEGER,
            LLSymbol::CHAR
        }
    );

    addProduction(
        24,
        LLSymbol::NT_FIELD_DEC_LIST,
        {
            LLSymbol::NT_ARRAY_TYPE,
            LLSymbol::NT_ID_LIST,
            LLSymbol::SEMI,
            LLSymbol::NT_FIELD_DEC_MORE
        },
        {
            LLSymbol::ARRAY
        }
    );

    addProduction(
        25,
        LLSymbol::NT_FIELD_DEC_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::END
        }
    );

    addProduction(
        26,
        LLSymbol::NT_FIELD_DEC_MORE,
        {
            LLSymbol::NT_FIELD_DEC_LIST
        },
        {
            LLSymbol::INTEGER,
            LLSymbol::CHAR,
            LLSymbol::ARRAY
        }
    );

    addProduction(
        27,
        LLSymbol::NT_ID_LIST,
        {
            LLSymbol::ID,
            LLSymbol::NT_ID_MORE
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        28,
        LLSymbol::NT_ID_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::SEMI
        }
    );

    addProduction(
        29,
        LLSymbol::NT_ID_MORE,
        {
            LLSymbol::COMMA,
            LLSymbol::NT_ID_LIST
        },
        {
            LLSymbol::COMMA
        }
    );

    addProduction(
        30,
        LLSymbol::NT_VAR_DEC,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::PROCEDURE,
            LLSymbol::BEGIN
        }
    );

    addProduction(
        31,
        LLSymbol::NT_VAR_DEC,
        {
            LLSymbol::NT_VAR_DECLARATION
        },
        {
            LLSymbol::VAR
        }
    );

    addProduction(
        32,
        LLSymbol::NT_VAR_DECLARATION,
        {
            LLSymbol::VAR,
            LLSymbol::NT_VAR_DEC_LIST
        },
        {
            LLSymbol::VAR
        }
    );

    addProduction(
        33,
        LLSymbol::NT_VAR_DEC_LIST,
        {
            LLSymbol::NT_TYPE_DEF,
            LLSymbol::NT_VAR_ID_LIST,
            LLSymbol::SEMI,
            LLSymbol::NT_VAR_DEC_MORE
        },
        {
            LLSymbol::INTEGER,
            LLSymbol::CHAR,
            LLSymbol::ARRAY,
            LLSymbol::RECORD,
            LLSymbol::ID
        }
    );

    addProduction(
        34,
        LLSymbol::NT_VAR_DEC_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::PROCEDURE,
            LLSymbol::BEGIN
        }
    );

    addProduction(
        35,
        LLSymbol::NT_VAR_DEC_MORE,
        {
            LLSymbol::NT_VAR_DEC_LIST
        },
        {
            LLSymbol::INTEGER,
            LLSymbol::CHAR,
            LLSymbol::ARRAY,
            LLSymbol::RECORD,
            LLSymbol::ID
        }
    );

    addProduction(
        36,
        LLSymbol::NT_VAR_ID_LIST,
        {
            LLSymbol::ID,
            LLSymbol::NT_VAR_ID_MORE
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        37,
        LLSymbol::NT_VAR_ID_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::SEMI
        }
    );

    addProduction(
        38,
        LLSymbol::NT_VAR_ID_MORE,
        {
            LLSymbol::COMMA,
            LLSymbol::NT_VAR_ID_LIST
        },
        {
            LLSymbol::COMMA
        }
    );

    addProduction(
        39,
        LLSymbol::NT_PROC_DEC,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::BEGIN
        }
    );

    addProduction(
        40,
        LLSymbol::NT_PROC_DEC,
        {
            LLSymbol::NT_PROC_DECLARATION
        },
        {
            LLSymbol::PROCEDURE
        }
    );

    addProduction(
        41,
        LLSymbol::NT_PROC_DECLARATION,
        {
            LLSymbol::PROCEDURE,
            LLSymbol::NT_PROC_NAME,
            LLSymbol::LPAREN,
            LLSymbol::NT_PARAM_LIST,
            LLSymbol::RPAREN,
            LLSymbol::SEMI,
            LLSymbol::NT_PROC_DEC_PART,
            LLSymbol::NT_PROC_BODY,
            LLSymbol::NT_PROC_DEC_MORE
        },
        {
            LLSymbol::PROCEDURE
        }
    );

    addProduction(
        42,
        LLSymbol::NT_PROC_DEC_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::BEGIN
        }
    );

    addProduction(
        43,
        LLSymbol::NT_PROC_DEC_MORE,
        {
            LLSymbol::NT_PROC_DECLARATION
        },
        {
            LLSymbol::PROCEDURE
        }
    );

    addProduction(
        44,
        LLSymbol::NT_PROC_NAME,
        {
            LLSymbol::ID
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        45,
        LLSymbol::NT_PARAM_LIST,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::RPAREN
        }
    );

    addProduction(
        46,
        LLSymbol::NT_PARAM_LIST,
        {
            LLSymbol::NT_PARAM_DEC_LIST
        },
        {
            LLSymbol::INTEGER,
            LLSymbol::CHAR,
            LLSymbol::ARRAY,
            LLSymbol::RECORD,
            LLSymbol::ID,
            LLSymbol::VAR
        }
    );

    addProduction(
        47,
        LLSymbol::NT_PARAM_DEC_LIST,
        {
            LLSymbol::NT_PARAM,
            LLSymbol::NT_PARAM_MORE
        },
        {
            LLSymbol::INTEGER,
            LLSymbol::CHAR,
            LLSymbol::ARRAY,
            LLSymbol::RECORD,
            LLSymbol::ID,
            LLSymbol::VAR
        }
    );

    addProduction(
        48,
        LLSymbol::NT_PARAM_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::RPAREN
        }
    );

    addProduction(
        49,
        LLSymbol::NT_PARAM_MORE,
        {
            LLSymbol::SEMI,
            LLSymbol::NT_PARAM_DEC_LIST
        },
        {
            LLSymbol::SEMI
        }
    );

    addProduction(
        50,
        LLSymbol::NT_PARAM,
        {
            LLSymbol::NT_TYPE_DEF,
            LLSymbol::NT_FORM_LIST
        },
        {
            LLSymbol::INTEGER,
            LLSymbol::CHAR,
            LLSymbol::ARRAY,
            LLSymbol::RECORD,
            LLSymbol::ID
        }
    );

    addProduction(
        51,
        LLSymbol::NT_PARAM,
        {
            LLSymbol::VAR,
            LLSymbol::NT_TYPE_DEF,
            LLSymbol::NT_FORM_LIST
        },
        {
            LLSymbol::VAR
        }
    );

    addProduction(
        52,
        LLSymbol::NT_FORM_LIST,
        {
            LLSymbol::ID,
            LLSymbol::NT_FID_MORE
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        53,
        LLSymbol::NT_FID_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::SEMI,
            LLSymbol::RPAREN
        }
    );

    addProduction(
        54,
        LLSymbol::NT_FID_MORE,
        {
            LLSymbol::COMMA,
            LLSymbol::NT_FORM_LIST
        },
        {
            LLSymbol::COMMA
        }
    );

    addProduction(
        55,
        LLSymbol::NT_PROC_DEC_PART,
        {
            LLSymbol::NT_DECLARE_PART
        },
        {
            LLSymbol::TYPE,
            LLSymbol::VAR,
            LLSymbol::PROCEDURE,
            LLSymbol::BEGIN
        }
    );

    addProduction(
        56,
        LLSymbol::NT_PROC_BODY,
        {
            LLSymbol::NT_PROGRAM_BODY
        },
        {
            LLSymbol::BEGIN
        }
    );

    addProduction(
        57,
        LLSymbol::NT_PROGRAM_BODY,
        {
            LLSymbol::BEGIN,
            LLSymbol::NT_STM_LIST,
            LLSymbol::END
        },
        {
            LLSymbol::BEGIN
        }
    );

    addProduction(
        58,
        LLSymbol::NT_STM_LIST,
        {
            LLSymbol::NT_STM,
            LLSymbol::NT_STM_MORE
        },
        {
            LLSymbol::ID,
            LLSymbol::IF,
            LLSymbol::WHILE,
            LLSymbol::RETURN,
            LLSymbol::READ,
            LLSymbol::WRITE
        }
    );

    addProduction(
        59,
        LLSymbol::NT_STM_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::ELSE,
            LLSymbol::FI,
            LLSymbol::END,
            LLSymbol::ENDWH
        }
    );

    addProduction(
        60,
        LLSymbol::NT_STM_MORE,
        {
            LLSymbol::SEMI,
            LLSymbol::NT_STM_LIST
        },
        {
            LLSymbol::SEMI
        }
    );

    addProduction(
        61,
        LLSymbol::NT_STM,
        {
            LLSymbol::NT_CONDITIONAL_STM
        },
        {
            LLSymbol::IF
        }
    );

    addProduction(
        62,
        LLSymbol::NT_STM,
        {
            LLSymbol::NT_LOOP_STM
        },
        {
            LLSymbol::WHILE
        }
    );

    addProduction(
        63,
        LLSymbol::NT_STM,
        {
            LLSymbol::NT_INPUT_STM
        },
        {
            LLSymbol::READ
        }
    );

    addProduction(
        64,
        LLSymbol::NT_STM,
        {
            LLSymbol::NT_OUTPUT_STM
        },
        {
            LLSymbol::WRITE
        }
    );

    addProduction(
        65,
        LLSymbol::NT_STM,
        {
            LLSymbol::NT_RETURN_STM
        },
        {
            LLSymbol::RETURN
        }
    );

    addProduction(
        66,
        LLSymbol::NT_STM,
        {
            LLSymbol::ID,
            LLSymbol::NT_ASS_CALL
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        67,
        LLSymbol::NT_ASS_CALL,
        {
            LLSymbol::NT_ASSIGNMENT_REST
        },
        {
            LLSymbol::ASSIGN,
            LLSymbol::LMIDPAREN,
            LLSymbol::DOT
        }
    );

    addProduction(
        68,
        LLSymbol::NT_ASS_CALL,
        {
            LLSymbol::NT_CALL_STM_REST
        },
        {
            LLSymbol::LPAREN
        }
    );

    addProduction(
        69,
        LLSymbol::NT_ASSIGNMENT_REST,
        {
            LLSymbol::NT_VARI_MORE,
            LLSymbol::ASSIGN,
            LLSymbol::NT_EXP
        },
        {
            LLSymbol::ASSIGN,
            LLSymbol::LMIDPAREN,
            LLSymbol::DOT
        }
    );

    addProduction(
        70,
        LLSymbol::NT_CONDITIONAL_STM,
        {
            LLSymbol::IF,
            LLSymbol::NT_EXP,
            LLSymbol::THEN,
            LLSymbol::NT_STM_LIST,
            LLSymbol::ELSE,
            LLSymbol::NT_STM_LIST,
            LLSymbol::FI
        },
        {
            LLSymbol::IF
        }
    );

    addProduction(
        71,
        LLSymbol::NT_LOOP_STM,
        {
            LLSymbol::WHILE,
            LLSymbol::NT_EXP,
            LLSymbol::DO,
            LLSymbol::NT_STM_LIST,
            LLSymbol::ENDWH
        },
        {
            LLSymbol::WHILE
        }
    );

    addProduction(
        72,
        LLSymbol::NT_INPUT_STM,
        {
            LLSymbol::READ,
            LLSymbol::LPAREN,
            LLSymbol::NT_IN_VAR,
            LLSymbol::RPAREN
        },
        {
            LLSymbol::READ
        }
    );

    addProduction(
        73,
        LLSymbol::NT_IN_VAR,
        {
            LLSymbol::ID,
            LLSymbol::NT_VARI_MORE
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        74,
        LLSymbol::NT_OUTPUT_STM,
        {
            LLSymbol::WRITE,
            LLSymbol::LPAREN,
            LLSymbol::NT_EXP,
            LLSymbol::RPAREN
        },
        {
            LLSymbol::WRITE
        }
    );

    addProduction(
        75,
        LLSymbol::NT_RETURN_STM,
        {
            LLSymbol::RETURN
        },
        {
            LLSymbol::RETURN
        }
    );

    addProduction(
        76,
        LLSymbol::NT_CALL_STM_REST,
        {
            LLSymbol::LPAREN,
            LLSymbol::NT_ACT_PARAM_LIST,
            LLSymbol::RPAREN
        },
        {
            LLSymbol::LPAREN
        }
    );

    addProduction(
        77,
        LLSymbol::NT_ACT_PARAM_LIST,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::RPAREN
        }
    );

    addProduction(
        78,
        LLSymbol::NT_ACT_PARAM_LIST,
        {
            LLSymbol::NT_EXP,
            LLSymbol::NT_ACT_PARAM_MORE
        },
        {
            LLSymbol::ID,
            LLSymbol::INTC,
            LLSymbol::LPAREN
        }
    );

    addProduction(
        79,
        LLSymbol::NT_ACT_PARAM_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::RPAREN
        }
    );

    addProduction(
        80,
        LLSymbol::NT_ACT_PARAM_MORE,
        {
            LLSymbol::COMMA,
            LLSymbol::NT_ACT_PARAM_LIST
        },
        {
            LLSymbol::COMMA
        }
    );

    addProduction(
        81,
        LLSymbol::NT_REL_EXP,
        {
            LLSymbol::NT_EXP,
            LLSymbol::NT_OTHER_REL_E
        },
        {
            LLSymbol::ID,
            LLSymbol::INTC,
            LLSymbol::LPAREN
        }
    );

    addProduction(
        82,
        LLSymbol::NT_OTHER_REL_E,
        {
            LLSymbol::NT_CMP_OP,
            LLSymbol::NT_EXP
        },
        {
            LLSymbol::LT,
            LLSymbol::EQ
        }
    );

    addProduction(
        83,
        LLSymbol::NT_EXP,
        {
            LLSymbol::NT_TERM,
            LLSymbol::NT_OTHER_TERM
        },
        {
            LLSymbol::ID,
            LLSymbol::INTC,
            LLSymbol::LPAREN
        }
    );

    addProduction(
        84,
        LLSymbol::NT_OTHER_TERM,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::LT,
            LLSymbol::EQ,
            LLSymbol::THEN,
            LLSymbol::DO,
            LLSymbol::RPAREN,
            LLSymbol::RMIDPAREN,
            LLSymbol::SEMI,
            LLSymbol::COMMA,
            LLSymbol::ELSE,
            LLSymbol::FI,
            LLSymbol::END,
            LLSymbol::ENDWH
        }
    );

    addProduction(
        85,
        LLSymbol::NT_OTHER_TERM,
        {
            LLSymbol::NT_ADD_OP,
            LLSymbol::NT_EXP
        },
        {
            LLSymbol::PLUS,
            LLSymbol::MINUS
        }
    );

    addProduction(
        86,
        LLSymbol::NT_TERM,
        {
            LLSymbol::NT_FACTOR,
            LLSymbol::NT_OTHER_FACTOR
        },
        {
            LLSymbol::ID,
            LLSymbol::INTC,
            LLSymbol::LPAREN
        }
    );

    addProduction(
        87,
        LLSymbol::NT_OTHER_FACTOR,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::PLUS,
            LLSymbol::MINUS,
            LLSymbol::LT,
            LLSymbol::EQ,
            LLSymbol::THEN,
            LLSymbol::DO,
            LLSymbol::RPAREN,
            LLSymbol::RMIDPAREN,
            LLSymbol::SEMI,
            LLSymbol::COMMA,
            LLSymbol::ELSE,
            LLSymbol::FI,
            LLSymbol::END,
            LLSymbol::ENDWH
        }
    );

    addProduction(
        88,
        LLSymbol::NT_OTHER_FACTOR,
        {
            LLSymbol::NT_MULT_OP,
            LLSymbol::NT_TERM
        },
        {
            LLSymbol::TIMES,
            LLSymbol::OVER
        }
    );

    addProduction(
        89,
        LLSymbol::NT_FACTOR,
        {
            LLSymbol::LPAREN,
            LLSymbol::NT_EXP,
            LLSymbol::RPAREN
        },
        {
            LLSymbol::LPAREN
        }
    );

    addProduction(
        90,
        LLSymbol::NT_FACTOR,
        {
            LLSymbol::INTC
        },
        {
            LLSymbol::INTC
        }
    );

    addProduction(
        91,
        LLSymbol::NT_FACTOR,
        {
            LLSymbol::NT_VARIABLE
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        92,
        LLSymbol::NT_VARIABLE,
        {
            LLSymbol::ID,
            LLSymbol::NT_VARI_MORE
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        93,
        LLSymbol::NT_VARI_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::ASSIGN,
            LLSymbol::TIMES,
            LLSymbol::OVER,
            LLSymbol::PLUS,
            LLSymbol::MINUS,
            LLSymbol::LT,
            LLSymbol::EQ,
            LLSymbol::THEN,
            LLSymbol::ELSE,
            LLSymbol::FI,
            LLSymbol::DO,
            LLSymbol::ENDWH,
            LLSymbol::END,
            LLSymbol::RPAREN,
            LLSymbol::RMIDPAREN,
            LLSymbol::SEMI,
            LLSymbol::COMMA
        }
    );

    addProduction(
        94,
        LLSymbol::NT_VARI_MORE,
        {
            LLSymbol::LMIDPAREN,
            LLSymbol::NT_EXP,
            LLSymbol::RMIDPAREN
        },
        {
            LLSymbol::LMIDPAREN
        }
    );

    addProduction(
        95,
        LLSymbol::NT_VARI_MORE,
        {
            LLSymbol::DOT,
            LLSymbol::NT_FIELD_VAR
        },
        {
            LLSymbol::DOT
        }
    );

    addProduction(
        96,
        LLSymbol::NT_FIELD_VAR,
        {
            LLSymbol::ID,
            LLSymbol::NT_FIELD_VAR_MORE
        },
        {
            LLSymbol::ID
        }
    );

    addProduction(
        97,
        LLSymbol::NT_FIELD_VAR_MORE,
        {
            LLSymbol::NIL
        },
        {
            LLSymbol::ASSIGN,
            LLSymbol::TIMES,
            LLSymbol::OVER,
            LLSymbol::PLUS,
            LLSymbol::MINUS,
            LLSymbol::LT,
            LLSymbol::EQ,
            LLSymbol::THEN,
            LLSymbol::ELSE,
            LLSymbol::FI,
            LLSymbol::DO,
            LLSymbol::ENDWH,
            LLSymbol::END,
            LLSymbol::RPAREN,
            LLSymbol::RMIDPAREN,
            LLSymbol::SEMI,
            LLSymbol::COMMA
        }
    );

    addProduction(
        98,
        LLSymbol::NT_FIELD_VAR_MORE,
        {
            LLSymbol::LMIDPAREN,
            LLSymbol::NT_EXP,
            LLSymbol::RMIDPAREN
        },
        {
            LLSymbol::LMIDPAREN
        }
    );

    addProduction(
        99,
        LLSymbol::NT_CMP_OP,
        {
            LLSymbol::LT
        },
        {
            LLSymbol::LT
        }
    );

    addProduction(
        100,
        LLSymbol::NT_CMP_OP,
        {
            LLSymbol::EQ
        },
        {
            LLSymbol::EQ
        }
    );

    addProduction(
        101,
        LLSymbol::NT_ADD_OP,
        {
            LLSymbol::PLUS
        },
        {
            LLSymbol::PLUS
        }
    );

    addProduction(
        102,
        LLSymbol::NT_ADD_OP,
        {
            LLSymbol::MINUS
        },
        {
            LLSymbol::MINUS
        }
    );

    addProduction(
        103,
        LLSymbol::NT_MULT_OP,
        {
            LLSymbol::TIMES
        },
        {
            LLSymbol::TIMES
        }
    );

    addProduction(
        104,
        LLSymbol::NT_MULT_OP,
        {
            LLSymbol::OVER
        },
        {
            LLSymbol::OVER
        }
    );
}

void LL1Parser::addProduction(int number,
                              LLSymbol left,
                              const vector<LLSymbol>& right,
                              const vector<LLSymbol>& predictSet)
{
    Production production;
    production.number = number;
    production.left = left;
    production.right = right;
    productions[number] = production;
    for (size_t i = 0; i < predictSet.size(); i++)
    {
        int key = makeTableKey(left, predictSet[i]);
        ll1Table[key] = number;
    }
}

int LL1Parser::makeTableKey(LLSymbol nonTerminal, LLSymbol terminal) const
{
    int row = static_cast<int>(nonTerminal);
    int col = static_cast<int>(terminal);
    return row * 1000 + col;
}

int LL1Parser::getTableProduction(LLSymbol nonTerminal, LLSymbol terminal) const
{
    int key = makeTableKey(nonTerminal, terminal);
    unordered_map<int, int>::const_iterator it = ll1Table.find(key);
    if (it == ll1Table.end()) return 0;
    return it->second;
}

void LL1Parser::predict(int number)
{
    switch (number)
    {
#define CASE_PROCESS(n) \
    case n:             \
        process##n();   \
        break;
        LL1_PROCESS_LIST(CASE_PROCESS)
#undef CASE_PROCESS
    default:
        syntaxError("没有对应的产生式处理函数: process" + to_string(number));
        break;
    }
}

void LL1Parser::pushSymbol(LLSymbol symbol)
{
    if (symbol != LLSymbol::NIL)
    {
        symbolStack.push_back(symbol);
    }
}

LLSymbol LL1Parser::popSymbol()
{
    if (symbolStack.empty())
    {
        syntaxError("符号栈为空，无法弹栈");
        return LLSymbol::NIL;
    }
    LLSymbol symbol = symbolStack.back();
    symbolStack.pop_back();
    return symbol;
}

LLSymbol LL1Parser::readStackTop() const
{
    if (symbolStack.empty())
    {
        return LLSymbol::NIL;
    }
    return symbolStack.back();
}

bool LL1Parser::isSymbolStackEmpty() const
{
    return symbolStack.empty();
}

bool LL1Parser::readStackFlag() const
{
    return isTerminal(readStackTop());
}

LLSymbol LL1Parser::readStackN() const
{
    LLSymbol top = readStackTop();
    if (isNonTerminal(top))
    {
        return top;
    }
    return LLSymbol::NIL;
}

LLSymbol LL1Parser::readStackT() const
{
    LLSymbol top = readStackTop();
    if (isTerminal(top))
    {
        return top;
    }
    return LLSymbol::NIL;
}

void LL1Parser::pushTreeNode(TreeNode* node)
{
    if (node != nullptr)
    {
        treeStack.push_back(node);
    }
}

TreeNode* LL1Parser::popTreeNode()
{
    if (treeStack.empty()) return nullptr;
    TreeNode* node = treeStack.back();
    treeStack.pop_back();
    return node;
}

TreeNode* LL1Parser::readTreeNode() const
{
    if (treeStack.empty()) return nullptr;
    return treeStack.back();
}

bool LL1Parser::isTreeStackEmpty() const
{
    return treeStack.empty();
}

void LL1Parser::pushOp(TreeNode* node)
{
    if (node != nullptr)
    {
        opStack.push_back(node);
    }
}

TreeNode* LL1Parser::popOp()
{
    if (opStack.empty()) return nullptr;
    TreeNode* node = opStack.back();
    opStack.pop_back();
    return node;
}

TreeNode* LL1Parser::readOpStack() const
{
    if (opStack.empty()) return nullptr;
    return opStack.back();
}

bool LL1Parser::isOpStackEmpty() const
{
    return opStack.empty();
}

void LL1Parser::pushNum(TreeNode* node)
{
    if (node != nullptr)
    {
        numStack.push_back(node);
    }
}

TreeNode* LL1Parser::popNum()
{
    if (numStack.empty()) return nullptr;
    TreeNode* node = numStack.back();
    numStack.pop_back();
    return node;
}

TreeNode* LL1Parser::readNumStack() const
{
    if (numStack.empty()) return nullptr;
    return numStack.back();
}

bool LL1Parser::isNumStackEmpty() const
{
    return numStack.empty();
}

void LL1Parser::pushExpTarget(TreeNode* node)
{
    expTargetStack.push_back(node);
}

TreeNode* LL1Parser::popExpTarget()
{
    if (expTargetStack.empty()) return nullptr;
    TreeNode* node = expTargetStack.back();
    expTargetStack.pop_back();
    return node;
}

TreeNode* LL1Parser::readExpTarget() const
{
    if (expTargetStack.empty()) return nullptr;
    return expTargetStack.back();
}

void LL1Parser::attachExpressionResult(TreeNode* expNode)
{
    if (expNode == nullptr) return;
    TreeNode* target = popExpTarget();
    if (target != nullptr)
    {
        attachChild(target, expNode);
    }
    else
    {
        attachToTreeTop(expNode);
    }
}

int LL1Parser::priority(LexType op) const
{
    if (op == LexType::ENDFILE) return 0; 
    if (op == LexType::LT || op == LexType::EQ) return 1; 
    if (op == LexType::PLUS || op == LexType::MINUS) return 2; 
    if (op == LexType::TIMES || op == LexType::OVER) return 3; 
    return -1;
}

void LL1Parser::reduceExpressionOnce()
{
    if (isOpStackEmpty())
    {
        syntaxError("表达式归约失败，操作符栈为空");
        return;
    }
    TreeNode* opNode = popOp();
    if (opNode == nullptr)
    {
        syntaxError("表达式归约失败，操作符为空");
        return;
    }
    if (opNode->value == "END" || opNode->value == "(")
    {
        syntaxError("表达式归约失败，遇到非法归约标记 " + opNode->value);
        return;
    }
    TreeNode* right = popNum();
    TreeNode* left = popNum();
    if (left == nullptr || right == nullptr)
    {
        syntaxError("表达式归约失败，操作符或操作数不足");
        return;
    }
    opNode->addChild(left);
    opNode->addChild(right);
    pushNum(opNode);
}

void LL1Parser::reduceExpressionByPriority(LexType op)
{
    int currentPriority = priority(op);
    while (!isOpStackEmpty())
    {
        TreeNode* top = readOpStack();
        if (top == nullptr)
        {
            return;
        }
        if (top->value == "END" || top->value == "(")
        {
            return;
        }
        LexType topOp = stringToOperator(top->value);
        int topPriority = priority(topOp);
        if (topPriority < currentPriority)
        {
            return;
        }
        reduceExpressionOnce();
    }
}

LexType LL1Parser::stringToOperator(const string& op) const
{
    if (op == "+") return LexType::PLUS;
    if (op == "-") return LexType::MINUS;
    if (op == "*") return LexType::TIMES;
    if (op == "/") return LexType::OVER;
    if (op == "<") return LexType::LT;
    if (op == "=") return LexType::EQ;
    return LexType::ERROR;
}

Token LL1Parser::peek() const
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

Token LL1Parser::advance()
{
    if (!isAtEnd()) current++;
    return tokens[current - 1];
}

bool LL1Parser::isAtEnd() const
{
    return peek().type == LexType::ENDFILE;
}

bool LL1Parser::isTerminal(LLSymbol symbol) const
{
    return symbol >= LLSymbol::PROGRAM && symbol <= LLSymbol::ENDFILE;
}

bool LL1Parser::isNonTerminal(LLSymbol symbol) const
{
    return symbol >= LLSymbol::NT_PROGRAM &&
           symbol <= LLSymbol::NT_MULT_OP;
}

void LL1Parser::syntaxError(const string& message)
{
    errorFlag = true;
    Token token = peek();
    cerr << "LL(1)语法错误：第 " << token.line << " 行，"
         << message << "，当前单词为 "
         << lexTypeToString(token.type) << " "
         << token.sem << endl;
}

TreeNode* LL1Parser::newRootNode()
{
    return new TreeNode("Program");
}

TreeNode* LL1Parser::newPheadNode()
{
    return new TreeNode("ProgramHead");
}

TreeNode* LL1Parser::newDecANode(const string& kind)
{
    return new TreeNode("DeclareK", kind, peek().line);
}

TreeNode* LL1Parser::newTypeNode(const string& kind)
{
    return new TreeNode("TypeK", kind, peek().line);
}

TreeNode* LL1Parser::newStmNode(const string& kind)
{
    return new TreeNode("StmtK", kind, peek().line);
}

TreeNode* LL1Parser::newExpNode(const string& kind)
{
    return new TreeNode("ExpK", kind, peek().line);
}

TreeNode* LL1Parser::newTerminalNode(const Token& token)
{
    return new TreeNode(lexTypeToString(token.type), token.sem, token.line);
}

TreeNode* LL1Parser::newIdNode(const Token& token)
{
    return new TreeNode("IdK", token.sem, token.line);
}

TreeNode* LL1Parser::newConstNode(const Token& token)
{
    return new TreeNode("ConstK", token.sem, token.line);
}

TreeNode* LL1Parser::newOpNode(const Token& token)
{
    return new TreeNode("OpK", token.sem, token.line);
}

void LL1Parser::attachChild(TreeNode* parent, TreeNode* child)
{
    if (parent != nullptr && child != nullptr)
    {
        parent->addChild(child);
    }
}

void LL1Parser::attachToCurrent(TreeNode* child)
{
    attachChild(currentP, child);
}

void LL1Parser::attachToTreeTop(TreeNode* child)
{
    TreeNode* top = readTreeNode();
    if (top != nullptr)
    {
        top->addChild(child);
    }
}

bool LL1Parser::isStatementNode(TreeNode* node) const
{
    if (node == nullptr) return false;
    if (node->name != "StmtK") return false;
    return true;
}

void LL1Parser::closeCurrentStatement()
{
    TreeNode* top = readTreeNode();
    if (isStatementNode(top))
    {
        popTreeNode();
    }
    currentP = readTreeNode();
    if (currentP == nullptr)
    {
        currentP = rootPointer;
    }
    temp = currentP;
}

TreeNode* LL1Parser::getDeclareAttachTarget()
{
    for (int i = static_cast<int>(treeStack.size()) - 1; i >= 0; i--)
    {
        if (treeStack[i] != nullptr && treeStack[i]->name == "ProcDecK")
        {
            return treeStack[i];
        }
    }
    return rootPointer;
}

TreeNode* LL1Parser::getCurrentStatementNode() const
{
    for (int i = static_cast<int>(treeStack.size()) - 1; i >= 0; i--)
    {
        if (treeStack[i] != nullptr &&
            treeStack[i]->name == "StmtK" &&
            treeStack[i]->value != "StmListK")
        {
            return treeStack[i];
        }
    }
    return nullptr;
}

LLSymbol LL1Parser::tokenToSymbol(LexType type) const
{
    switch (type)
    {
    case LexType::PROGRAM: return LLSymbol::PROGRAM;
    case LexType::TYPE: return LLSymbol::TYPE;
    case LexType::VAR: return LLSymbol::VAR;
    case LexType::PROCEDURE: return LLSymbol::PROCEDURE;
    case LexType::BEGIN: return LLSymbol::BEGIN;
    case LexType::END: return LLSymbol::END;
    case LexType::ARRAY: return LLSymbol::ARRAY;
    case LexType::OF: return LLSymbol::OF;
    case LexType::RECORD: return LLSymbol::RECORD;
    case LexType::IF: return LLSymbol::IF;
    case LexType::THEN: return LLSymbol::THEN;
    case LexType::ELSE: return LLSymbol::ELSE;
    case LexType::FI: return LLSymbol::FI;
    case LexType::WHILE: return LLSymbol::WHILE;
    case LexType::DO: return LLSymbol::DO;
    case LexType::ENDWH: return LLSymbol::ENDWH;
    case LexType::READ: return LLSymbol::READ;
    case LexType::WRITE: return LLSymbol::WRITE;
    case LexType::RETURN: return LLSymbol::RETURN;
    case LexType::INTEGER: return LLSymbol::INTEGER;
    case LexType::CHAR: return LLSymbol::CHAR;
    case LexType::ID: return LLSymbol::ID;
    case LexType::INTC: return LLSymbol::INTC;
    case LexType::CHARC: return LLSymbol::CHARC;
    case LexType::ASSIGN: return LLSymbol::ASSIGN;
    case LexType::EQ: return LLSymbol::EQ;
    case LexType::LT: return LLSymbol::LT;
    case LexType::PLUS: return LLSymbol::PLUS;
    case LexType::MINUS: return LLSymbol::MINUS;
    case LexType::TIMES: return LLSymbol::TIMES;
    case LexType::OVER: return LLSymbol::OVER;
    case LexType::LPAREN: return LLSymbol::LPAREN;
    case LexType::RPAREN: return LLSymbol::RPAREN;
    case LexType::LMIDPAREN: return LLSymbol::LMIDPAREN;
    case LexType::RMIDPAREN: return LLSymbol::RMIDPAREN;
    case LexType::SEMI: return LLSymbol::SEMI;
    case LexType::COMMA: return LLSymbol::COMMA;
    case LexType::DOT: return LLSymbol::DOT;
    case LexType::COLON: return LLSymbol::COLON;
    case LexType::UNDERANGE: return LLSymbol::UNDERANGE;
    case LexType::ENDFILE: return LLSymbol::ENDFILE;
    default: return LLSymbol::NIL;
    }
}

string LL1Parser::symbolToString(LLSymbol symbol) const
{
    switch (symbol)
    {
    case LLSymbol::NIL: return "Empty";
    case LLSymbol::PROGRAM: return "PROGRAM";
    case LLSymbol::TYPE: return "TYPE";
    case LLSymbol::VAR: return "VAR";
    case LLSymbol::PROCEDURE: return "PROCEDURE";
    case LLSymbol::BEGIN: return "BEGIN";
    case LLSymbol::END: return "END";
    case LLSymbol::ARRAY: return "ARRAY";
    case LLSymbol::OF: return "OF";
    case LLSymbol::RECORD: return "RECORD";
    case LLSymbol::IF: return "IF";
    case LLSymbol::THEN: return "THEN";
    case LLSymbol::ELSE: return "ELSE";
    case LLSymbol::FI: return "FI";
    case LLSymbol::WHILE: return "WHILE";
    case LLSymbol::DO: return "DO";
    case LLSymbol::ENDWH: return "ENDWH";
    case LLSymbol::READ: return "READ";
    case LLSymbol::WRITE: return "WRITE";
    case LLSymbol::RETURN: return "RETURN";
    case LLSymbol::INTEGER: return "INTEGER";
    case LLSymbol::CHAR: return "CHAR";
    case LLSymbol::ID: return "ID";
    case LLSymbol::INTC: return "INTC";
    case LLSymbol::CHARC: return "CHARC";
    case LLSymbol::ASSIGN: return "ASSIGN";
    case LLSymbol::EQ: return "EQ";
    case LLSymbol::LT: return "LT";
    case LLSymbol::PLUS: return "PLUS";
    case LLSymbol::MINUS: return "MINUS";
    case LLSymbol::TIMES: return "TIMES";
    case LLSymbol::OVER: return "OVER";
    case LLSymbol::LPAREN: return "LPAREN";
    case LLSymbol::RPAREN: return "RPAREN";
    case LLSymbol::LMIDPAREN: return "LMIDPAREN";
    case LLSymbol::RMIDPAREN: return "RMIDPAREN";
    case LLSymbol::SEMI: return "SEMI";
    case LLSymbol::COMMA: return "COMMA";
    case LLSymbol::DOT: return "DOT";
    case LLSymbol::COLON: return "COLON";
    case LLSymbol::UNDERANGE: return "UNDERANGE";
    case LLSymbol::ENDFILE: return "ENDFILE";
    case LLSymbol::NT_PROGRAM: return "Program";
    case LLSymbol::NT_PROGRAM_HEAD: return "ProgramHead";
    case LLSymbol::NT_PROGRAM_NAME: return "ProgramName";
    case LLSymbol::NT_DECLARE_PART: return "DeclarePart";
    case LLSymbol::NT_TYPE_DEC: return "TypeDec";
    case LLSymbol::NT_TYPE_DECLARATION: return "TypeDeclaration";
    case LLSymbol::NT_TYPE_DEC_LIST: return "TypeDecList";
    case LLSymbol::NT_TYPE_DEC_MORE: return "TypeDecMore";
    case LLSymbol::NT_TYPE_ID: return "TypeId";
    case LLSymbol::NT_TYPE_DEF: return "TypeDef";
    case LLSymbol::NT_BASE_TYPE: return "BaseType";
    case LLSymbol::NT_STRUCTURE_TYPE: return "StructureType";
    case LLSymbol::NT_ARRAY_TYPE: return "ArrayType";
    case LLSymbol::NT_REC_TYPE: return "RecType";
    case LLSymbol::NT_FIELD_DEC_LIST: return "FieldDecList";
    case LLSymbol::NT_FIELD_DEC_MORE: return "FieldDecMore";
    case LLSymbol::NT_ID_LIST: return "IdList";
    case LLSymbol::NT_ID_MORE: return "IdMore";
    case LLSymbol::NT_VAR_DEC: return "VarDec";
    case LLSymbol::NT_VAR_DECLARATION: return "VarDeclaration";
    case LLSymbol::NT_VAR_DEC_LIST: return "VarDecList";
    case LLSymbol::NT_VAR_DEC_MORE: return "VarDecMore";
    case LLSymbol::NT_VAR_ID_LIST: return "VarIdList";
    case LLSymbol::NT_VAR_ID_MORE: return "VarIdMore";
    case LLSymbol::NT_PROC_DEC: return "ProcDec";
    case LLSymbol::NT_PROC_DECLARATION: return "ProcDeclaration";
    case LLSymbol::NT_PROC_DEC_MORE: return "ProcDecMore";
    case LLSymbol::NT_PROC_NAME: return "ProcName";
    case LLSymbol::NT_PARAM_LIST: return "ParamList";
    case LLSymbol::NT_PARAM_DEC_LIST: return "ParamDecList";
    case LLSymbol::NT_PARAM_MORE: return "ParamMore";
    case LLSymbol::NT_PARAM: return "Param";
    case LLSymbol::NT_FORM_LIST: return "FormList";
    case LLSymbol::NT_FID_MORE: return "FidMore";
    case LLSymbol::NT_PROC_DEC_PART: return "ProcDecPart";
    case LLSymbol::NT_PROC_BODY: return "ProcBody";
    case LLSymbol::NT_PROGRAM_BODY: return "ProgramBody";
    case LLSymbol::NT_STM_LIST: return "StmList";
    case LLSymbol::NT_STM_MORE: return "StmMore";
    case LLSymbol::NT_STM: return "Stm";
    case LLSymbol::NT_ASS_CALL: return "AssCall";
    case LLSymbol::NT_ASSIGNMENT_REST: return "AssignmentRest";
    case LLSymbol::NT_CONDITIONAL_STM: return "ConditionalStm";
    case LLSymbol::NT_LOOP_STM: return "LoopStm";
    case LLSymbol::NT_INPUT_STM: return "InputStm";
    case LLSymbol::NT_OUTPUT_STM: return "OutputStm";
    case LLSymbol::NT_RETURN_STM: return "ReturnStm";
    case LLSymbol::NT_CALL_STM_REST: return "CallStmRest";
    case LLSymbol::NT_ACT_PARAM_LIST: return "ActParamList";
    case LLSymbol::NT_ACT_PARAM_MORE: return "ActParamMore";
    case LLSymbol::NT_EXP: return "Exp";
    case LLSymbol::NT_SIMPLE_EXP: return "SimpleExp";
    case LLSymbol::NT_TERM: return "Term";
    case LLSymbol::NT_FACTOR: return "Factor";
    case LLSymbol::NT_VARIABLE: return "Variable";
    case LLSymbol::NT_VARI_MORE: return "VariMore";
    case LLSymbol::NT_FIELD_VAR: return "FieldVar";
    case LLSymbol::NT_FIELD_VAR_MORE: return "FieldVarMore";
    case LLSymbol::NT_LOW: return "Low";
    case LLSymbol::NT_TOP: return "Top";
    case LLSymbol::NT_IN_VAR: return "InVar";
    case LLSymbol::NT_REL_EXP: return "RelExp";
    case LLSymbol::NT_OTHER_REL_E: return "OtherRelE";
    case LLSymbol::NT_OTHER_TERM: return "OtherTerm";
    case LLSymbol::NT_ADD_OP: return "AddOp";
    case LLSymbol::NT_CMP_OP: return "CmpOp";
    case LLSymbol::NT_OTHER_FACTOR: return "OtherFactor";
    case LLSymbol::NT_MULT_OP: return "MultOp";
    default: return "Unknown";
    }
}

void LL1Parser::process1()
{
    // <Program> ::= ProgramHead DeclarePart ProgramBody .
    pushSymbol(LLSymbol::DOT);
    pushSymbol(LLSymbol::NT_PROGRAM_BODY);
    pushSymbol(LLSymbol::NT_DECLARE_PART);
    pushSymbol(LLSymbol::NT_PROGRAM_HEAD);
}

void LL1Parser::process2()
{
    // <ProgramHead> ::= PROGRAM ProgramName
    pushSymbol(LLSymbol::NT_PROGRAM_NAME);
    pushSymbol(LLSymbol::PROGRAM);
    currentP = newPheadNode();
    attachChild(rootPointer, currentP);
    pushTreeNode(currentP);
}

void LL1Parser::process3()
{
    // <ProgramName> ::= ID
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    TreeNode* idNode = newIdNode(token);
    attachToTreeTop(idNode);
    popTreeNode();
}

void LL1Parser::process4()
{
    // <DeclarePart> ::= TypeDec VarDec ProcDec
    pushSymbol(LLSymbol::NT_PROC_DEC);
    pushSymbol(LLSymbol::NT_VAR_DEC);
    pushSymbol(LLSymbol::NT_TYPE_DEC);
}

void LL1Parser::process5()
{
    // <TypeDec> ::= ε
}

void LL1Parser::process6()
{
    // <TypeDec> ::= TypeDeclaration
    pushSymbol(LLSymbol::NT_TYPE_DECLARATION);
}

void LL1Parser::process7()
{
    // <TypeDeclaration> ::= TYPE TypeDecList
    pushSymbol(LLSymbol::NT_TYPE_DEC_LIST);
    pushSymbol(LLSymbol::TYPE);
    currentP = newDecANode("TypeDecK");
    attachChild(getDeclareAttachTarget(), currentP);
    pushTreeNode(currentP);
}

void LL1Parser::process8()
{
    // <TypeDecList> ::= TypeId = TypeDef ; TypeDecMore
    pushSymbol(LLSymbol::NT_TYPE_DEC_MORE);
    pushSymbol(LLSymbol::SEMI);
    pushSymbol(LLSymbol::NT_TYPE_DEF);
    pushSymbol(LLSymbol::EQ);
    pushSymbol(LLSymbol::NT_TYPE_ID);
    TreeNode* typeItemNode = newDecANode("TypeDecItemK");
    attachToTreeTop(typeItemNode);
    pushTreeNode(typeItemNode);
    currentP = typeItemNode;
}

void LL1Parser::process9()
{
    // <TypeDecMore> ::= ε
    popTreeNode();
    popTreeNode();
    currentP = rootPointer;
}

void LL1Parser::process10()
{
    // <TypeDecMore> ::= TypeDecList
    popTreeNode();
    pushSymbol(LLSymbol::NT_TYPE_DEC_LIST);
    currentP = readTreeNode();
}

void LL1Parser::process11()
{
    // <TypeId> ::= ID
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    TreeNode* idNode = newIdNode(token);
    attachToTreeTop(idNode);
}

void LL1Parser::process12()
{
    // <TypeDef> ::= BaseType
    pushSymbol(LLSymbol::NT_BASE_TYPE);
    temp = readTreeNode();
}

void LL1Parser::process13()
{
    // <TypeDef> ::= StructureType
    pushSymbol(LLSymbol::NT_STRUCTURE_TYPE);
    temp = readTreeNode();
}

void LL1Parser::process14()
{
    // <TypeDef> ::= ID
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    TreeNode* typeNode = newTypeNode("IdTypeK");
    TreeNode* idNode = newIdNode(token);
    attachChild(typeNode, idNode);
    if (temp != nullptr)
    {
        attachChild(temp, typeNode);
    }
    else
    {
        attachToTreeTop(typeNode);
    }
}

void LL1Parser::process15()
{
    // <BaseType> ::= INTEGER
    pushSymbol(LLSymbol::INTEGER);
    TreeNode* typeNode = newTypeNode("IntegerK");
    if (temp != nullptr)
    {
        attachChild(temp, typeNode);
    }
    else
    {
        attachToTreeTop(typeNode);
    }
}

void LL1Parser::process16()
{
    // <BaseType> ::= CHAR
    pushSymbol(LLSymbol::CHAR);
    TreeNode* typeNode = newTypeNode("CharK");
    if (temp != nullptr)
    {
        attachChild(temp, typeNode);
    }
    else
    {
        attachToTreeTop(typeNode);
    }
}

void LL1Parser::process17()
{
    // <StructureType> ::= ArrayType
    pushSymbol(LLSymbol::NT_ARRAY_TYPE);
}

void LL1Parser::process18()
{
    // <StructureType> ::= RecType
    pushSymbol(LLSymbol::NT_REC_TYPE);
}

void LL1Parser::process19()
{
    // <ArrayType> ::= ARRAY [ Low .. Top ] OF BaseType
    pushSymbol(LLSymbol::NT_BASE_TYPE);
    pushSymbol(LLSymbol::OF);
    pushSymbol(LLSymbol::RMIDPAREN);
    pushSymbol(LLSymbol::NT_TOP);
    pushSymbol(LLSymbol::UNDERANGE);
    pushSymbol(LLSymbol::NT_LOW);
    pushSymbol(LLSymbol::LMIDPAREN);
    pushSymbol(LLSymbol::ARRAY);
    TreeNode* arrayNode = newTypeNode("ArrayK");
    if (temp != nullptr)
    {
        attachChild(temp, arrayNode);
    }
    else
    {
        attachToTreeTop(arrayNode);
    }
    temp = arrayNode;
}

void LL1Parser::process20()
{
    // <Low> ::= INTC
    pushSymbol(LLSymbol::INTC);
    Token token = peek();
    TreeNode* lowNode = new TreeNode("LowK", token.sem, token.line);
    if (temp != nullptr)
    {
        attachChild(temp, lowNode);
    }
    else
    {
        attachToTreeTop(lowNode);
    }
}

void LL1Parser::process21()
{
    // <Top> ::= INTC
    pushSymbol(LLSymbol::INTC);
    Token token = peek();
    TreeNode* topNode = new TreeNode("TopK", token.sem, token.line);
    if (temp != nullptr)
    {
        attachChild(temp, topNode);
    }
    else
    {
        attachToTreeTop(topNode);
    }
}

void LL1Parser::process22()
{
    // <RecType> ::= RECORD FieldDecList END
    pushSymbol(LLSymbol::END);
    pushSymbol(LLSymbol::NT_FIELD_DEC_LIST);
    pushSymbol(LLSymbol::RECORD);
    TreeNode* recordNode = newTypeNode("RecordK");
    if (temp != nullptr)
    {
        attachChild(temp, recordNode);
    }
    else
    {
        attachToTreeTop(recordNode);
    }
    saveP = recordNode;
    currentP = recordNode;
    temp = recordNode;
    pushTreeNode(recordNode);
}

void LL1Parser::process23()
{
    // <FieldDecList> ::= BaseType IdList ; FieldDecMore
    pushSymbol(LLSymbol::NT_FIELD_DEC_MORE);
    pushSymbol(LLSymbol::SEMI);
    pushSymbol(LLSymbol::NT_ID_LIST);
    pushSymbol(LLSymbol::NT_BASE_TYPE);
    currentP = newDecANode("FieldDecK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
    temp = currentP;
}

void LL1Parser::process24()
{
    // <FieldDecList> ::= ArrayType IdList ; FieldDecMore
    pushSymbol(LLSymbol::NT_FIELD_DEC_MORE);
    pushSymbol(LLSymbol::SEMI);
    pushSymbol(LLSymbol::NT_ID_LIST);
    pushSymbol(LLSymbol::NT_ARRAY_TYPE);
    currentP = newDecANode("FieldDecK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
    temp = currentP;
}

void LL1Parser::process25()
{
    // <FieldDecMore> ::= ε
    popTreeNode();
    if (!isTreeStackEmpty() && readTreeNode() == saveP)
    {
        popTreeNode();
    }
    currentP = readTreeNode();
    temp = currentP;
}

void LL1Parser::process26()
{
    // <FieldDecMore> ::= FieldDecList
    popTreeNode();
    currentP = readTreeNode();
    temp = currentP;
    pushSymbol(LLSymbol::NT_FIELD_DEC_LIST);
}

void LL1Parser::process27()
{
    // <IdList> ::= ID IdMore
    pushSymbol(LLSymbol::NT_ID_MORE);
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    TreeNode* idNode = newIdNode(token);
    attachToTreeTop(idNode);
}

void LL1Parser::process28()
{
    // <IdMore> ::= ε
}

void LL1Parser::process29()
{
    // <IdMore> ::= COMMA IdList
    pushSymbol(LLSymbol::NT_ID_LIST);
    pushSymbol(LLSymbol::COMMA);
}

void LL1Parser::process30()
{
    // <VarDec> ::= ε
}

void LL1Parser::process31()
{
    // <VarDec> ::= VarDeclaration
    pushSymbol(LLSymbol::NT_VAR_DECLARATION);
}

void LL1Parser::process32()
{
    // <VarDeclaration> ::= VAR VarDecList
    pushSymbol(LLSymbol::NT_VAR_DEC_LIST);
    pushSymbol(LLSymbol::VAR);
    currentP = newDecANode("VarDecK");
    attachChild(getDeclareAttachTarget(), currentP);
    pushTreeNode(currentP);
}

void LL1Parser::process33()
{
    // <VarDecList> ::= TypeDef VarIdList ; VarDecMore
    pushSymbol(LLSymbol::NT_VAR_DEC_MORE);
    pushSymbol(LLSymbol::SEMI);
    pushSymbol(LLSymbol::NT_VAR_ID_LIST);
    pushSymbol(LLSymbol::NT_TYPE_DEF);
    currentP = newDecANode("VarDecItemK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
    temp = currentP;
}

void LL1Parser::process34()
{
    // <VarDecMore> ::= ε
    popTreeNode();
    if (!isTreeStackEmpty())
    {
        popTreeNode();
    }
    currentP = rootPointer;
    temp = currentP;
}

void LL1Parser::process35()
{
    // <VarDecMore> ::= VarDecList
    popTreeNode();
    currentP = readTreeNode();
    temp = currentP;
    pushSymbol(LLSymbol::NT_VAR_DEC_LIST);
}

void LL1Parser::process36()
{
    // <VarIdList> ::= ID VarIdMore
    pushSymbol(LLSymbol::NT_VAR_ID_MORE);
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    TreeNode* idNode = newIdNode(token);
    attachToTreeTop(idNode);
}

void LL1Parser::process37()
{
    // <VarIdMore> ::= ε
}

void LL1Parser::process38()
{
    // <VarIdMore> ::= COMMA VarIdList
    pushSymbol(LLSymbol::NT_VAR_ID_LIST);
    pushSymbol(LLSymbol::COMMA);
}

void LL1Parser::process39()
{
    // <ProcDec> ::= ε
}

void LL1Parser::process40()
{
    // <ProcDec> ::= ProcDeclaration
    pushSymbol(LLSymbol::NT_PROC_DECLARATION);
}

void LL1Parser::process41()
{
    // <ProcDeclaration> ::= PROCEDURE ProcName ( ParamList ) ; ProcDecPart ProcBody ProcDecMore
    pushSymbol(LLSymbol::NT_PROC_DEC_MORE);
    pushSymbol(LLSymbol::NT_PROC_BODY);
    pushSymbol(LLSymbol::NT_PROC_DEC_PART);
    pushSymbol(LLSymbol::SEMI);
    pushSymbol(LLSymbol::RPAREN);
    pushSymbol(LLSymbol::NT_PARAM_LIST);
    pushSymbol(LLSymbol::LPAREN);
    pushSymbol(LLSymbol::NT_PROC_NAME);
    pushSymbol(LLSymbol::PROCEDURE);
    currentP = new TreeNode("ProcDecK", "", peek().line);
    attachChild(rootPointer, currentP);
    pushTreeNode(currentP);
    saveP = currentP;
    temp = currentP;
}

void LL1Parser::process42()
{
    // <ProcDecMore> ::= ε
    popTreeNode();
    currentP = rootPointer;
    temp = currentP;
}

void LL1Parser::process43()
{
    // <ProcDecMore> ::= ProcDeclaration
    popTreeNode();
    currentP = rootPointer;
    temp = currentP;
    pushSymbol(LLSymbol::NT_PROC_DECLARATION);
}

void LL1Parser::process44()
{
    // <ProcName> ::= ID
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    TreeNode* idNode = newIdNode(token);
    attachToTreeTop(idNode);
}

void LL1Parser::process45()
{
    // <ParamList> ::= ε
}

void LL1Parser::process46()
{
    // <ParamList> ::= ParamDecList
    pushSymbol(LLSymbol::NT_PARAM_DEC_LIST);
}

void LL1Parser::process47()
{
    // <ParamDecList> ::= Param ParamMore
    pushSymbol(LLSymbol::NT_PARAM_MORE);
    pushSymbol(LLSymbol::NT_PARAM);
}

void LL1Parser::process48()
{
    // <ParamMore> ::= ε
    popTreeNode();
    currentP = saveP;
    temp = saveP;
}

void LL1Parser::process49()
{
    // <ParamMore> ::= ; ParamDecList
    popTreeNode();
    currentP = saveP;
    temp = saveP;
    pushSymbol(LLSymbol::NT_PARAM_DEC_LIST);
    pushSymbol(LLSymbol::SEMI);
}

void LL1Parser::process50()
{
    // <Param> ::= TypeDef FormList
    pushSymbol(LLSymbol::NT_FORM_LIST);
    pushSymbol(LLSymbol::NT_TYPE_DEF);
    currentP = newDecANode("ValParamK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
    temp = currentP;
}

void LL1Parser::process51()
{
    // <Param> ::= VAR TypeDef FormList
    pushSymbol(LLSymbol::NT_FORM_LIST);
    pushSymbol(LLSymbol::NT_TYPE_DEF);
    pushSymbol(LLSymbol::VAR);
    currentP = newDecANode("VarParamK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
    temp = currentP;
}

void LL1Parser::process52()
{
    // <FormList> ::= ID FidMore
    pushSymbol(LLSymbol::NT_FID_MORE);
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    TreeNode* idNode = newIdNode(token);
    attachToTreeTop(idNode);
}

void LL1Parser::process53()
{
    // <FidMore> ::= ε
}

void LL1Parser::process54()
{
    // <FidMore> ::= COMMA FormList
    pushSymbol(LLSymbol::NT_FORM_LIST);
    pushSymbol(LLSymbol::COMMA);
}

void LL1Parser::process55()
{
    // <ProcDecPart> ::= DeclarePart
    pushSymbol(LLSymbol::NT_DECLARE_PART);
}

void LL1Parser::process56()
{
    // <ProcBody> ::= ProgramBody
    pushSymbol(LLSymbol::NT_PROGRAM_BODY);
}

void LL1Parser::process57()
{
    // <ProgramBody> ::= BEGIN StmList END
    pushSymbol(LLSymbol::END);
    pushSymbol(LLSymbol::NT_STM_LIST);
    pushSymbol(LLSymbol::BEGIN);
    currentP = newStmNode("StmListK");
    if (!isTreeStackEmpty())
    {
        attachToTreeTop(currentP);
    }
    else
    {
        attachChild(rootPointer, currentP);
    }
    pushTreeNode(currentP);
}

void LL1Parser::process58()
{
    // <StmList> ::= Stm StmMore
    pushSymbol(LLSymbol::NT_STM_MORE);
    pushSymbol(LLSymbol::NT_STM);
}

void LL1Parser::process59()
{
    // <StmMore> ::= ε
    closeCurrentStatement();
    TreeNode* top = readTreeNode();
    if (top != nullptr && top->name == "StmtK" && top->value == "StmListK")
    {
        popTreeNode();
    }
    currentP = readTreeNode();
    if (currentP == nullptr)
    {
        currentP = rootPointer;
    }
    temp = currentP;
}

void LL1Parser::process60()
{
    // <StmMore> ::= ; StmList
    closeCurrentStatement();
    pushSymbol(LLSymbol::NT_STM_LIST);
    pushSymbol(LLSymbol::SEMI);
}

void LL1Parser::process61()
{
    // <Stm> ::= ConditionalStm
    pushSymbol(LLSymbol::NT_CONDITIONAL_STM);
    currentP = newStmNode("IfK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
}

void LL1Parser::process62()
{
    // <Stm> ::= LoopStm
    pushSymbol(LLSymbol::NT_LOOP_STM);
    currentP = newStmNode("WhileK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
}

void LL1Parser::process63()
{
    // <Stm> ::= InputStm
    pushSymbol(LLSymbol::NT_INPUT_STM);
    currentP = newStmNode("ReadK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
}

void LL1Parser::process64()
{
    // <Stm> ::= OutputStm
    pushSymbol(LLSymbol::NT_OUTPUT_STM);
    currentP = new TreeNode("StmtK", "WriteK", peek().line);
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
}

void LL1Parser::process65()
{
    // <Stm> ::= ReturnStm
    pushSymbol(LLSymbol::NT_RETURN_STM);
    currentP = newStmNode("ReturnK");
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
}

void LL1Parser::process66()
{
    // <Stm> ::= ID AssCall
    pushSymbol(LLSymbol::NT_ASS_CALL);
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    currentP = newStmNode("IdStmK");
    TreeNode* idNode = newIdNode(token);
    attachChild(currentP, idNode);
    attachToTreeTop(currentP);
    pushTreeNode(currentP);
    temp = currentP;
    currentVarNode = idNode;
}

void LL1Parser::process67()
{
    // <AssCall> ::= AssignmentRest
    pushSymbol(LLSymbol::NT_ASSIGNMENT_REST);
    if (currentP != nullptr)
    {
        currentP->value = "AssignK";
    }
}

void LL1Parser::process68()
{
    // <AssCall> ::= CallStmRest
    pushSymbol(LLSymbol::NT_CALL_STM_REST);
    if (currentP != nullptr)
    {
        currentP->value = "CallK";
    }
}

void LL1Parser::process69()
{
    // <AssignmentRest> ::= VariMore := Exp
    pushSymbol(LLSymbol::NT_EXP);
    pushSymbol(LLSymbol::ASSIGN);
    pushSymbol(LLSymbol::NT_VARI_MORE);
    TreeNode* endNode = new TreeNode("OpK", "END", peek().line);
    pushOp(endNode);
    pushExpTarget(currentP);
}

void LL1Parser::process70()
{
    // <ConditionalStm> ::= IF Exp THEN StmList ELSE StmList FI
    pushSymbol(LLSymbol::FI);
    pushSymbol(LLSymbol::NT_STM_LIST);
    pushSymbol(LLSymbol::ELSE);
    pushSymbol(LLSymbol::NT_STM_LIST);
    pushSymbol(LLSymbol::THEN);
    pushSymbol(LLSymbol::NT_REL_EXP);
    pushSymbol(LLSymbol::IF);
    pushExpTarget(currentP);
}

void LL1Parser::process71()
{
    // <LoopStm> ::= WHILE Exp DO StmList ENDWH
    pushSymbol(LLSymbol::ENDWH);
    pushSymbol(LLSymbol::NT_STM_LIST);
    pushSymbol(LLSymbol::DO);
    pushSymbol(LLSymbol::NT_REL_EXP);
    pushSymbol(LLSymbol::WHILE);
    pushExpTarget(currentP);
}

void LL1Parser::process72()
{
    // <InputStm> ::= READ ( InVar )
    pushSymbol(LLSymbol::RPAREN);
    pushSymbol(LLSymbol::NT_IN_VAR);
    pushSymbol(LLSymbol::LPAREN);
    pushSymbol(LLSymbol::READ);
}

void LL1Parser::process73()
{
    // <InVar> ::= ID VariMore
    pushSymbol(LLSymbol::NT_VARI_MORE);
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    currentP = new TreeNode("VarK", token.sem, token.line);
    attachToTreeTop(currentP);
    currentVarNode = currentP;
}

void LL1Parser::process74()
{
    // <OutputStm> ::= WRITE ( Exp )
    pushSymbol(LLSymbol::RPAREN);
    pushSymbol(LLSymbol::NT_EXP);
    pushSymbol(LLSymbol::LPAREN);
    pushSymbol(LLSymbol::WRITE);
    TreeNode* endNode = new TreeNode("OpK", "END", peek().line);
    pushOp(endNode);
    TreeNode* target = getCurrentStatementNode();
    if (target == nullptr)
    {
        target = currentP;
    }
    pushExpTarget(target);
}

void LL1Parser::process75()
{
    // <ReturnStm> ::= RETURN
    pushSymbol(LLSymbol::RETURN);
}

void LL1Parser::process76()
{
    // <CallStmRest> ::= ( ActParamList )
    pushSymbol(LLSymbol::RPAREN);
    pushSymbol(LLSymbol::NT_ACT_PARAM_LIST);
    pushSymbol(LLSymbol::LPAREN);
}

void LL1Parser::process77()
{
    // <ActParamList> ::= ε
}

void LL1Parser::process78()
{
    // <ActParamList> ::= Exp ActParamMore
    pushSymbol(LLSymbol::NT_ACT_PARAM_MORE);
    pushSymbol(LLSymbol::NT_EXP);
    TreeNode* endNode = new TreeNode("OpK", "END", peek().line);
    pushOp(endNode);
    TreeNode* target = getCurrentStatementNode();
    if (target == nullptr)
    {
        target = currentP;
    }
    pushExpTarget(target);
}

void LL1Parser::process79()
{
    // <ActParamMore> ::= ε
    while (!isOpStackEmpty() && readOpStack()->value != "END")
    {
        reduceExpressionOnce();
    }
    if (!isOpStackEmpty() && readOpStack()->value == "END")
    {
        popOp();
    }
    TreeNode* expNode = popNum();
    attachExpressionResult(expNode);
}

void LL1Parser::process80()
{
    // <ActParamMore> ::= , ActParamList
    while (!isOpStackEmpty() && readOpStack()->value != "END")
    {
        reduceExpressionOnce();
    }
    if (!isOpStackEmpty() && readOpStack()->value == "END")
    {
        popOp();
    }
    TreeNode* expNode = popNum();
    attachExpressionResult(expNode);
    pushSymbol(LLSymbol::NT_ACT_PARAM_LIST);
    pushSymbol(LLSymbol::COMMA);
}

void LL1Parser::process81()
{
    // <RelExp> ::= Exp OtherRelE
    pushSymbol(LLSymbol::NT_OTHER_REL_E);
    pushSymbol(LLSymbol::NT_EXP);
    TreeNode* endNode = new TreeNode("OpK", "END", peek().line);
    pushOp(endNode);
}

void LL1Parser::process82()
{
    // <OtherRelE> ::= CmpOp Exp
    pushSymbol(LLSymbol::NT_EXP);
    pushSymbol(LLSymbol::NT_CMP_OP);
}

void LL1Parser::process83()
{
    // <Exp> ::= Term OtherTerm
    pushSymbol(LLSymbol::NT_OTHER_TERM);
    pushSymbol(LLSymbol::NT_TERM);
}

void LL1Parser::process84()
{
    // <OtherTerm> ::= ε
    if (peek().type == LexType::LT || peek().type == LexType::EQ)
    {
        return;
    }
    if (peek().type == LexType::RPAREN)
    {
        while (!isOpStackEmpty() &&
               readOpStack()->value != "(" &&
               readOpStack()->value != "END")
        {
            reduceExpressionOnce();
        }
        if (!isOpStackEmpty() && readOpStack()->value == "(")
        {
            popOp();
            return;
        }
        TreeNode* target = readExpTarget();
        if (!isOpStackEmpty() &&
            readOpStack()->value == "END" &&
            target != nullptr &&
            target->name == "StmtK" &&
            target->value == "WriteK")
        {
            popOp();

            TreeNode* expNode = popNum();

            attachExpressionResult(expNode);
        }

        return;
    }
    while (!isOpStackEmpty() &&
           readOpStack()->value != "END" &&
           readOpStack()->value != "(")
    {
        reduceExpressionOnce();
    }
    if (!isOpStackEmpty() && readOpStack()->value == "END")
    {
        popOp();
        TreeNode* expNode = popNum();
        attachExpressionResult(expNode);
    }
}

void LL1Parser::process85()
{
    // <OtherTerm> ::= AddOp Exp
    Token token = peek();
    TreeNode* opNode = newOpNode(token);
    reduceExpressionByPriority(token.type);
    pushOp(opNode);
    pushSymbol(LLSymbol::NT_EXP);
    pushSymbol(LLSymbol::NT_ADD_OP);
}

void LL1Parser::process86()
{
    // <Term> ::= Factor OtherFactor
    pushSymbol(LLSymbol::NT_OTHER_FACTOR);
    pushSymbol(LLSymbol::NT_FACTOR);
}

void LL1Parser::process87()
{
    // <OtherFactor> ::= ε
}

void LL1Parser::process88()
{
    // <OtherFactor> ::= MultOp Term
    Token token = peek();
    TreeNode* opNode = newOpNode(token);
    reduceExpressionByPriority(token.type);
    pushOp(opNode);
    pushSymbol(LLSymbol::NT_TERM);
    pushSymbol(LLSymbol::NT_MULT_OP);
}

void LL1Parser::process89()
{
    // <Factor> ::= ( Exp )
    pushSymbol(LLSymbol::RPAREN);
    pushSymbol(LLSymbol::NT_EXP);
    pushSymbol(LLSymbol::LPAREN);
    TreeNode* leftParenNode = new TreeNode("OpK", "(", peek().line);
    pushOp(leftParenNode);
}

void LL1Parser::process90()
{
    // <Factor> ::= INTC
    pushSymbol(LLSymbol::INTC);
    Token token = peek();
    TreeNode* constNode = new TreeNode("ConstK", token.sem, token.line);
    pushNum(constNode);
}

void LL1Parser::process91()
{
    // <Factor> ::= Variable
    pushSymbol(LLSymbol::NT_VARIABLE);
}

void LL1Parser::process92()
{
    // <Variable> ::= ID VariMore
    pushSymbol(LLSymbol::NT_VARI_MORE);
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    currentP = new TreeNode("VarK", token.sem, token.line);
    pushNum(currentP);
    currentVarNode = currentP;
}

void LL1Parser::process93()
{
    // <VariMore> ::= ε
}

void LL1Parser::process94()
{
    // <VariMore> ::= [ Exp ]
    pushSymbol(LLSymbol::RMIDPAREN);
    pushSymbol(LLSymbol::NT_EXP);
    pushSymbol(LLSymbol::LMIDPAREN);
    TreeNode* target = currentVarNode;
    if (target != nullptr)
    {
        target->value = target->value + "[]";
        pushExpTarget(target);
    }
    TreeNode* endNode = new TreeNode("OpK", "END", peek().line);
    pushOp(endNode);
}

void LL1Parser::process95()
{
    // <VariMore> ::= . FieldVar
    pushSymbol(LLSymbol::NT_FIELD_VAR);
    pushSymbol(LLSymbol::DOT);
    if (currentP != nullptr)
    {
        currentP->value = currentP->value + ".";
    }
}

void LL1Parser::process96()
{
    // <FieldVar> ::= ID FieldVarMore
    pushSymbol(LLSymbol::NT_FIELD_VAR_MORE);
    pushSymbol(LLSymbol::ID);
    Token token = peek();
    TreeNode* fieldNode = new TreeNode("FieldVarK", token.sem, token.line);
    if (currentP != nullptr)
    {
        attachChild(currentP, fieldNode);
    }
    else
    {
        pushNum(fieldNode);
    }
    currentP = fieldNode;
}

void LL1Parser::process97()
{
    // <FieldVarMore> ::= ε
}

void LL1Parser::process98()
{
    // <FieldVarMore> ::= [ Exp ]
    pushSymbol(LLSymbol::RMIDPAREN);
    pushSymbol(LLSymbol::NT_EXP);
    pushSymbol(LLSymbol::LMIDPAREN);
    TreeNode* target = currentVarNode;
    if (target != nullptr)
    {
        target->value = target->value + "[]";
        pushExpTarget(target);
    }
    TreeNode* endNode = new TreeNode("OpK", "END", peek().line);
    pushOp(endNode);
}

void LL1Parser::process99()
{
    // <CmpOp> ::= LT
    pushSymbol(LLSymbol::LT);
    Token token = peek();
    TreeNode* opNode = newOpNode(token);
    reduceExpressionByPriority(token.type);
    pushOp(opNode);
}

void LL1Parser::process100()
{
    // <CmpOp> ::= EQ
    pushSymbol(LLSymbol::EQ);
    Token token = peek();
    TreeNode* opNode = newOpNode(token);
    reduceExpressionByPriority(token.type);
    pushOp(opNode);
}

void LL1Parser::process101()
{
    // <AddOp> ::= PLUS
    pushSymbol(LLSymbol::PLUS);
}

void LL1Parser::process102()
{
    // <AddOp> ::= MINUS
    pushSymbol(LLSymbol::MINUS);
}

void LL1Parser::process103()
{
    // <MultOp> ::= TIMES
    pushSymbol(LLSymbol::TIMES);
}

void LL1Parser::process104()
{
    // <MultOp> ::= OVER
    pushSymbol(LLSymbol::OVER);
}