#include "HelloWorldScene.h"

USING_NS_CC;


bool HelloWorld::init()
{
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // Board background
    auto boardBg = Sprite::create("GUI/cocosstudio/img/boardBg.png");
    boardBg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    addChild(boardBg);

    return true;
}

