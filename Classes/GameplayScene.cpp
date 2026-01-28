#include "GameplayScene.h"

USING_NS_CC;


bool GameplayScene::init() {
    if ( !Scene::init() )
    {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();

    // Background color
    const Color4B colorBg = Color4B(40, 40, 45, 255);
    const auto layerColor = LayerColor::create(colorBg);
    addChild(layerColor);

    // Board background
    auto boardBg = Sprite::create("GUI/cocosstudio/img/boardBg.png");
    auto boardSize = boardBg->getContentSize();
    boardBg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
    boardBg->setScale(std::min(visibleSize.width / boardSize.width, visibleSize.height / boardSize.height));
    addChild(boardBg);

    return true;
}

