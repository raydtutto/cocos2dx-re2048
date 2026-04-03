#ifndef COCOS2DX_RE2048_NODEUTILS_H
#define COCOS2DX_RE2048_NODEUTILS_H


#include "axmol.h"

class NodeUtils {
public:
    static ax::Node* getNodeByName(ax::Node *parent, const std::string &name);
};



#endif //COCOS2DX_RE2048_NODEUTILS_H