#include "Lexer.h"
#include "Parser.h"
#include "LL1Parser.h"
#include "CompilerOptions.h"
#include "SemanticAnalyzer.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void printUsage()
{
    cout << "用法:" << endl;
    cout << "  snlc.exe <source.snl> [options]" << endl;
    cout << "输出控制参数:" << endl;
    cout << "  -EchoSource       输出源程序" << endl;
    cout << "  -TraceScan        输出词法分析结果" << endl;
    cout << "  -TraceParse       输出语法树" << endl;
    cout << "  -TraceTable       输出符号表信息" << endl;
    cout << "编译控制参数:" << endl;
    cout << "  -NO_PARSE         只进行词法分析，不进行语法分析" << endl;
    cout << "  -NO_LL1           使用递归下降语法分析，不使用 LL(1)" << endl;
    cout << "  -NO_ANALYZE       不进行语义分析" << endl;
}

bool parseArguments(int argc, char* argv[], CompilerOptions& options)
{
    if (argc < 2)
    {
        printUsage();
        return false;
    }
    options.sourceFile = argv[1];
    for (int i = 2; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "-EchoSource")
        {
            options.echoSource = true;
        }
        else if (arg == "-TraceScan")
        {
            options.traceScan = true;
        }
        else if (arg == "-TraceParse")
        {
            options.traceParse = true;
        }
        else if (arg == "-TraceTable")
        {
            options.traceTable = true;
        }
        else if (arg == "-NO_PARSE")
        {
            options.noParse = true;
        }
        else if (arg == "-NO_LL1")
        {
            options.noLL1 = true;
        }
        else if (arg == "-NO_ANALYZE")
        {
            options.noAnalyze = true;
        }
        else if (arg == "-h" || arg == "--help" || arg == "/?")
        {
            printUsage();
            return false;
        }
        else
        {
            cerr << "未知参数: " << arg << endl;
            printUsage();
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[])
{
    CompilerOptions options;
    if (!parseArguments(argc, argv, options)) return EXIT_FAILURE;
    Lexer lexer;
    if (!lexer.loadFile(options.sourceFile)) return EXIT_FAILURE;
    vector<Token> tokens = lexer.getTokenList();
    if (options.traceScan)
    {
        cout << endl;
        cout << "========== 词法分析结果 ==========" << endl;
        lexer.printTokenList();
    }
    lexer.writeTokenListToFile(options.tokenOutputFile);
    if (options.noParse)
    {
        cout << endl;
        cout << "检测到 -NO_PARSE，只完成词法分析。" << endl;
        return EXIT_SUCCESS;
    }
    TreeNode* root = nullptr;
    bool parseError = false;
    if (options.noLL1)
    {
        Parser parser(tokens);
        root = parser.parse();
        parseError = parser.hasError();
    }
    else
    {
        LL1Parser parser(tokens);
        root = parser.parseLL();
        parseError = parser.hasError();
    }
    if (options.traceParse)
    {
        cout << endl;
        cout << "========== 语法树 ==========" << endl;
        printTree(root);
    }
    writeTreeToFile(root, options.parseOutputFile);
    if (parseError)
    {
        cout << endl;
        cout << "语法分析完成，但存在语法错误。" << endl;
        freeTree(root);
        return EXIT_FAILURE;
    }
    cout << endl;
    cout << "语法分析成功。" << endl;
    bool semanticError = false;
    if (!options.noAnalyze)
    {
        SemanticAnalyzer semanticAnalyzer;
        semanticAnalyzer.Analyze(root, options.traceTable);
        if (semanticAnalyzer.hasError())
        {
            semanticError = true;
            cout << endl;
            semanticAnalyzer.printErrors();
        }
        else
        {
            cout << endl;
            cout << "语义分析成功。" << endl;
        }
    }
    else
    {
        cout << endl;
        cout << "检测到 -NO_ANALYZE，跳过语义分析。" << endl;
    }
    freeTree(root);
    if (semanticError)
    {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}