#include "InputBuffer.h"

#include <iostream>
#include <fstream>
#include <sstream>

InputBuffer::InputBuffer()
{
    pos = 0;
    line = 1;
}

bool InputBuffer::loadFile(const string& fileName)
{
    ifstream fin(fileName.c_str(), ios::in);
    if (!fin.is_open())
    {
        cerr << "无法打开源文件: " << fileName << endl;
        return false;
    }
    stringstream buffer;
    buffer << fin.rdbuf();
    source = buffer.str();
    fin.close();
    pos = 0;
    line = 1;

    return true;
}

char InputBuffer::advance()
{
    if (pos >= source.size())
    {
        return '\0';
    }
    char ch = source[pos];
    pos++;
    if (ch == '\n')
    {
        line++;
    }
    return ch;
}

void InputBuffer::backup(char ch)
{
    if (ch == '\0' || pos == 0) return;
    pos--;
    if (source[pos] == '\n') line--;
}

int InputBuffer::getLine() const
{
    return line;
}