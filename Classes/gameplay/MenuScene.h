#ifndef COCOS2DX_RE2048_MENUSCENE_H
#define COCOS2DX_RE2048_MENUSCENE_H


#include "cocos2d.h"

// MenuScene - Start menu
class MenuScene : public cocos2d::Scene {
public:
    // implement the "static create()" method manually
    static MenuScene *create() {
        MenuScene *pRet = new(std::nothrow) MenuScene();
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = nullptr;
            return nullptr;
        }
    }
    bool init() override;
};


#endif //COCOS2DX_RE2048_MENUSCENE_H