#ifndef LL1_PARSER_H
#define LL1_PARSER_H

// 定义语法分析器，LL(1)方法

#include "Token.h"
#include "TreeNode.h"
#include "LL1ProcessList.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <stack>

using namespace std;

// LL(1)分析表中的符号枚举
enum class LLSymbol
{
    NIL = 0,

    // 终极符
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

    ID,
    INTC,
    CHARC,

    ASSIGN,
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

    // 非终极符
    NT_PROGRAM,
    NT_PROGRAM_HEAD,
    NT_PROGRAM_NAME,
    NT_DECLARE_PART,
    NT_TYPE_DEC,
    NT_TYPE_DECLARATION,
    NT_TYPE_DEC_LIST,
    NT_TYPE_DEC_MORE,
    NT_TYPE_ID,
    NT_TYPE_DEF,
    NT_BASE_TYPE,
    NT_STRUCTURE_TYPE,
    NT_ARRAY_TYPE,
    NT_REC_TYPE,
    NT_FIELD_DEC_LIST,
    NT_FIELD_DEC_MORE,
    NT_ID_LIST,
    NT_ID_MORE,
    NT_VAR_DEC,
    NT_VAR_DECLARATION,
    NT_VAR_DEC_LIST,
    NT_VAR_DEC_MORE,
    NT_VAR_ID_LIST,
    NT_VAR_ID_MORE,
    NT_PROC_DEC,
    NT_PROC_DECLARATION,
    NT_PROC_DEC_MORE,
    NT_PROC_NAME,
    NT_PARAM_LIST,
    NT_PARAM_DEC_LIST,
    NT_PARAM_MORE,
    NT_PARAM,
    NT_FORM_LIST,
    NT_FID_MORE,
    NT_PROC_DEC_PART,
    NT_PROC_BODY,
    NT_PROGRAM_BODY,
    NT_STM_LIST,
    NT_STM_MORE,
    NT_STM,
    NT_ASS_CALL,
    NT_ASSIGNMENT_REST,
    NT_CONDITIONAL_STM,
    NT_LOOP_STM,
    NT_INPUT_STM,
    NT_OUTPUT_STM,
    NT_RETURN_STM,
    NT_CALL_STM_REST,
    NT_ACT_PARAM_LIST,
    NT_ACT_PARAM_MORE,
    NT_EXP,
    NT_SIMPLE_EXP,
    NT_TERM,
    NT_FACTOR,
    NT_VARIABLE,
    NT_VARI_MORE,
    NT_FIELD_VAR,
    NT_FIELD_VAR_MORE,
    NT_LOW,
    NT_TOP,
    NT_IN_VAR,
    NT_REL_EXP,
    NT_OTHER_REL_E,
    NT_OTHER_TERM,
    NT_ADD_OP,
    NT_CMP_OP,
    NT_OTHER_FACTOR,
    NT_MULT_OP
};

// 产生式结构体
struct Production
{
    int number;
    LLSymbol left; 
    vector<LLSymbol> right;
};

class LL1Parser
{
private:
    vector<Token> tokens;   // 记号流
    size_t current;         // 当前记号的索引
    bool errorFlag;         // 标记语法错误
    vector<LLSymbol> symbolStack;   // 符号栈
    vector<TreeNode*> treeStack;    // 语法树栈
    vector<TreeNode*> opStack;      // 操作符栈
    vector<TreeNode*> numStack;     // 操作数栈
    vector<TreeNode*> expTargetStack; // 表达式目标栈
    TreeNode* rootPointer;  // 语法树根节点
    TreeNode* currentP;     // 当前语法树节点
    TreeNode* saveP;        // 临时保存语法树节点
    TreeNode* temp;         // 临时语法树节点
    TreeNode* currentVarNode;
    unordered_map<int, Production> productions; // 产生式集合
    unordered_map<int, int> ll1Table;           // LL(1)分析表，键为非终极符和终极符的组合

public:
    LL1Parser(const vector<Token>& tokens);
    TreeNode* parseLL();
    bool hasError() const;

private:
    void initializeParser();
    void createLL1Table();  // 构建分析表
    void addProduction(int number,
                       LLSymbol left,
                       const vector<LLSymbol>& right,
                       const vector<LLSymbol>& predictSet); // 添加产生式，并将Predict集对应项填入LL(1)分析表
    int makeTableKey(LLSymbol nonTerminal,
                     LLSymbol terminal) const;              // 根据非终极符和终极符构造LL(1)分析表键值
    int getTableProduction(LLSymbol nonTerminal,
                           LLSymbol terminal) const;        // 根据符号栈顶非终极符和当前输入终极符查表，返回产生式编号
    void predict(int number);                               // 调用对应process函数

    // 符号栈
    void pushSymbol(LLSymbol symbol); 
    LLSymbol popSymbol();  
    LLSymbol readStackTop() const; 
    bool isSymbolStackEmpty() const;
    bool readStackFlag() const;         // 栈顶是否为终极符
    LLSymbol readStackN() const;        // 读栈顶非终极符
    LLSymbol readStackT() const;        // 读栈顶终极符

    // 语法树栈
    void pushTreeNode(TreeNode* node); 
    TreeNode* popTreeNode();   
    TreeNode* readTreeNode() const;  
    bool isTreeStackEmpty() const;

    // 操作符栈
    void pushOp(TreeNode* node);      
    TreeNode* popOp(); 
    TreeNode* readOpStack() const;   
    bool isOpStackEmpty() const; 

    // 操作数栈
    void pushNum(TreeNode* node); 
    TreeNode* popNum();   
    TreeNode* readNumStack() const; 
    bool isNumStackEmpty() const;   

    // 表达式目标栈
    void pushExpTarget(TreeNode* node);
    TreeNode* popExpTarget();
    TreeNode* readExpTarget() const;
    void attachExpressionResult(TreeNode* expNode);

    int priority(LexType op) const;                         // 返回操作符优先级
    void reduceExpressionOnce();                            // 操作符栈操作数栈归约生成表达式树
    void reduceExpressionByPriority(LexType incomingOp);    // 根据操作符优先级规约表达式
    LexType stringToOperator(const string& op) const;

    Token peek() const;  
    Token advance();  
    bool isAtEnd() const; 

    LLSymbol tokenToSymbol(LexType type) const;     // 将记号类型转换为LLSymbol
    bool isTerminal(LLSymbol symbol) const;         // 判断终极符
    bool isNonTerminal(LLSymbol symbol) const;      // 判断非终极符
    string symbolToString(LLSymbol symbol) const;   // LLSymbol转换为字符串
    void syntaxError(const string& message); 

    // 语法树节点创建
    TreeNode* newRootNode();                        // 语法分析树根节点
    TreeNode* newPheadNode();                       // 程序头
    TreeNode* newDecANode(const string& kind);      // 声明
    TreeNode* newTypeNode(const string& kind);      // 类型
    TreeNode* newStmNode(const string& kind);       // 语句
    TreeNode* newExpNode(const string& kind);       // 表达式
    TreeNode* newTerminalNode(const Token& token);  // 终极符
    TreeNode* newIdNode(const Token& token);        // 标识符
    TreeNode* newConstNode(const Token& token);     // 常量
    TreeNode* newOpNode(const Token& token);        // 操作符

    void attachChild(TreeNode* parent, TreeNode* child);    // 连接到parent
    void attachToCurrent(TreeNode* child);                  // 连接到currentP
    void attachToTreeTop(TreeNode* child);                  // 连接到语法树栈顶

    bool isStatementNode(TreeNode* node) const;
    void closeCurrentStatement();
    TreeNode* getDeclareAttachTarget();
    TreeNode* getCurrentStatementNode() const;

    // 生成process函数声明
#define DECLARE_PROCESS(n) void process##n();
    LL1_PROCESS_LIST(DECLARE_PROCESS)
#undef DECLARE_PROCESS
};

#endif