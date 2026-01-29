#include "GameplayScene.h"

#include "cocostudio/ActionTimeline/CSLoader.h"
#include "utils/NodeUtils.h"

USING_NS_CC;


bool GameplayScene::init() {
    if ( !Scene::init() )
    {
        return false;
    }

    // Background color
    const Color4B colorBg = Color4B(40, 40, 45, 255);
    const auto layerColor = LayerColor::create(colorBg);
    addChild(layerColor);

    mRoot = CSLoader::createNode("GUI/cocosstudio/widgets/gameplayScene.csb");
    if (mRoot)
        addChild(mRoot);
    else
        return false;


    if (auto gameboard = NodeUtils::getNodeByName(mRoot, "gameboard")) {
        // Board background
        auto visibleSize = Director::getInstance()->getVisibleSize();
        auto boardBg = Sprite::create("GUI/cocosstudio/img/boardBg.png");
        auto boardSize = boardBg->getContentSize();
        boardBg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        boardBg->setScale(std::min(visibleSize.width / boardSize.width, visibleSize.height / boardSize.height));
        gameboard->addChild(boardBg);
    }

    return true;
}

