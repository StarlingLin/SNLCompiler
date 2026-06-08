#include "TreeNode.h"

#include <iostream>
#include <fstream>

TreeNode::TreeNode(const string& name)
{
    this->name = name;
    this->value = "";
    this->line = -1;
}

TreeNode::TreeNode(const string& name, const string& value, int line)
{
    this->name = name;
    this->value = value;
    this->line = line;
}

void TreeNode::addChild(TreeNode* child)
{
    if (child != nullptr)
    {
        children.push_back(child);
    }
}

void printTree(TreeNode* root, int depth)
{
    printTreeToStream(root, cout, depth);
}

void printTreeToStream(TreeNode* root, ostream& out, int depth)
{
    if (root == nullptr) return;
    for (int i = 0; i < depth; i++)
    {
        out << "  ";
    }
    out << root->name;
    if (!root->value.empty())
    {
        out << ": " << root->value;
    }
    if (root->line != -1)
    {
        out << "  <line " << root->line << ">";
    }
    out << endl;
    for (size_t i = 0; i < root->children.size(); i++)
    {
        printTreeToStream(root->children[i], out, depth + 1);
    }
}

bool writeTreeToFile(TreeNode* root, const string& fileName)
{
    ofstream fout(fileName.c_str(), ios::out);
    if (!fout.is_open())
    {
        cerr << "无法打开语法树输出文件: " << fileName << endl;
        return false;
    }
    fout << "========== 语法树 ==========" << endl;
    printTreeToStream(root, fout, 0);
    fout.close();
    return true;
}

void freeTree(TreeNode* root)
{
    if (root == nullptr)
    {
        return;
    }
    for (size_t i = 0; i < root->children.size(); i++)
    {
        freeTree(root->children[i]);
    }
    delete root;
}