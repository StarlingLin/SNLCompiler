#ifndef TREE_NODE_H
#define TREE_NODE_H

// 树节点类，用于构建语法树

#include <string>
#include <iostream>
#include <vector>

using namespace std;

class TreeNode
{
public:
    string name;
    string value;
    int line;
    vector<TreeNode*> children;

public:
    TreeNode(const string& name);
    TreeNode(const string& name, const string& value, int line);

    void addChild(TreeNode* child);
};

void printTree(TreeNode* root, int depth = 0);
void printTreeToStream(TreeNode* root, ostream& out, int depth = 0);
bool writeTreeToFile(TreeNode* root, const string& fileName);
void freeTree(TreeNode* root);

#endif