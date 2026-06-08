#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

// 定义一个简单的输入缓冲区类，读源代码文件并提供字符级访问接口

#include <string>

using namespace std;

class InputBuffer
{
private:
    string source;
    size_t pos;
    int line;

public:
    InputBuffer();
    bool loadFile(const string& fileName);
    char advance();         // 返回字符并光标后移
    void backup(char ch);   // 光标前移
    int getLine() const;    // 获取行号
};

#endif