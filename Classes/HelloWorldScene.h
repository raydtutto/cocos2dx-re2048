#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"

class HelloWorld : public cocos2d::Scene
{
public:
    // implement the "static create()" method manually
    static HelloWorld *create() {
        HelloWorld *pRet = new(std::nothrow) HelloWorld();
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

};

#endif // __HELLOWORLD_SCENE_H__
