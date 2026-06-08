#ifndef COMPILER_OPTIONS_H
#define COMPILER_OPTIONS_H

// 定义编译器接受的命令行参数，文档P26

#include <string>

struct CompilerOptions
{
    std::string sourceFile;

    bool echoSource = false;    // 回显源代码
    bool traceScan = false;     // 词法分析跟踪
    bool traceParse = false;     // 语法分析跟踪
    bool traceTable = false;     // 符号表跟踪

    bool noParse = false;       // 仅进行词法分析
    bool noLL1 = false;         // false就LL1，true就递归下降
    bool noAnalyze = false;     // 在语义分析前停止

    std::string tokenOutputFile = "tokenlist.csv";
    std::string parseOutputFile = "parsetree.txt";
};

#endif