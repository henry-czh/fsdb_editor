
ScopeTree *newScopeTree(Scope origin, char *introduce)
{
    ScopeTree *ret = (ScopeTree *)malloc(sizeof(ScopeTree));
    if (introduce != NULL)
        strcpy(ret->introduce, introduce);
    ret->root = (ScopeNode *)malloc(sizeof(ScopeNode));
    memset(ret->root, 0, sizeof(ScopeNode));
    ret->root->scope = origin;
    ret->root->deep = 1;
    return ret;
}

int insertNewBaby(ScopeNode *parent, Scope baby)
{
    if (parent == NULL)
        return 0;

    ScopeNode *chld = (ScopeNode *)malloc(sizeof (ScopeNode));
    memset(chld, 0, sizeof(ScopeNode));

    chld->scope = baby;
    chld->parent = parent;
    // 该baby是第1个child
    if (parent->children == NULL)
    {
        parent->children = chld;
        chld->deep = parent->deep + 1;
    }
    // 该baby是第n个child
    else
    {
        ScopeNode *tmp = parent->children;
        while (tmp->brother != NULL)
            tmp = tmp->brother;
        tmp->brother = chld;
        chld->deep = tmp->deep;
    }
    K_INFOMATION("%s新增子scope %s, 端口信号ID范围 %d - %d .\n",
                 parent->scope.name,
                 chld->scope.name,
                 chld->scope.signalStartID,
                 chld->scope.signalFinishID);
    return 1;
}

Scope createScope(char *name, int start_id, int signalFinishID)
{
    Scope ret;
    strcpy(ret.name, name);
    ret.signalStartID = start_id;
    ret.signalFinishID = finish_id;

    return ret;
}

ScopeNode *findScopeNodeByName(ScopeNode *node, Scope *scope)
{
    ScopeNode *ret = null;
    if (node == NULL)
        return NULL;
    
    if(compareScope(&node->scope, scope) == 0)
        return node;
    else
    {
        if((ret = findScopeNodeByName(node->brother, scope)) == NULL)
            ret = findScopeNodeByName(node-children, scope);
    }

    return ret;
}

// Scope比对只比对scope name，上层路径的比对在主函数实现
int compareScope(Scope *scope0, Scope *scope1)
{
    int ret = -1;
    if((ret = strcmp(scope0->name, scope1->name)) == 0)
    {
        return 0;
    }
    else{
        return ret;
    }
}