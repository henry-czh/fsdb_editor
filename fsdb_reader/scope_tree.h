#ifndef _SCOPE_TREE_H
#define _SCOPE_TREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "include.h"

typedef struct _Scope
{
    char name[30];
    int signalStartID;
    int signalFinishID;
} Scope;

// def Scope node 定义节点
typedef struct _ScopeNode
{
    struct _ScopeNode* parent;
    struct _ScopeNode* brother;
    struct _ScopeNode* children;
    Scope scope;
} ScopeNode;

// def Scope Tree 定义族谱
typedef struct _ScopeTree
{
    ScopeNode* root;
    int deep;
    char introduce[200]; //关于Scope Tree的简单描述
} ScopeTree;

ScopeTree *newScopeTree(Scope origin);
void *deleteScopeTree(ScopeTree *tree);

int insertNewBaby(ScopeNode *parent, Scope baby);
int editScopeNode(ScopeNode *old, ScopeNode *newone);
void removeScopeNode(ScopeTree *tree, ScopeNode *node);

ScopeNode *findScopeNodee(Scope man);
ScopeNode *findOrigin(ScopeNode *node, int generation);
ScopeNode *findParent(ScopeNode *node);

void printBrother(ScopeNode *node);
void printCousin(ScopeNode *node);

#ifdef __cplusplus
}
#endif

#endif // _SCOPE_TREE_H