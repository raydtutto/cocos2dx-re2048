#ifndef COCOS2DX_RE2048_GAMEPLAYSCENE_H
#define COCOS2DX_RE2048_GAMEPLAYSCENE_H


#include "cocos2d.h"

class GameplayScene : public cocos2d::Scene
{
public:
    // implement the "static create()" method manually
    static GameplayScene *create() {
        GameplayScene *pRet = new(std::nothrow) GameplayScene();
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = nullptr;
            return nullptr;
        }
    }
    virtual bool init();

private:
    cocos2d::Node* mRoot{nullptr};
    cocos2d::Node* mBoard{nullptr};
};


#endif //COCOS2DX_RE2048_GAMEPLAYSCENE_H