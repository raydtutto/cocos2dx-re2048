#ifndef COCOS2DX_RE2048_NODEUTILS_H
#define COCOS2DX_RE2048_NODEUTILS_H


#include "cocos2d.h"

class NodeUtils {
public:
    static cocos2d::Node* getNodeByName(cocos2d::Node *parent, const std::string &name);
};



#endif //COCOS2DX_RE2048_NODEUTILS_H