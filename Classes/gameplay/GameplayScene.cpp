#include "GameplayScene.h"
#include "TileWidget.h"

#include "cocostudio/ActionTimeline/CSLoader.h"
#include "utils/NodeUtils.h"

USING_NS_CC;

using namespace cocos2d;

namespace {
    const std::pair<int, int> gridSize{4, 4};
    auto touchSwipeThreshold = 70.f;
}

bool GameplayScene::init() {
    if ( !Scene::init() )
    {
        return false;
    }

    // Background color
    const Color4B colorBg = Color4B(40, 40, 45, 255);
    const auto layerColor = LayerColor::create(colorBg);
    addChild(layerColor);

    mRoot = CSLoader::createNodeWithVisibleSize("GUI/cocosstudio/widgets/gameplayScene.csb");
    if (mRoot) {
        addChild(mRoot);
    }
    else {
        return false;
    }

    if (auto gameboard = NodeUtils::getNodeByName(mRoot, "gameboard")) {
        auto visibleSize = gameboard->getContentSize();
        auto boardBg = Sprite::create("GUI/cocosstudio/img/boardBg.png");
        auto boardSize = boardBg->getContentSize();
        boardBg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        boardBg->setScale(std::min(visibleSize.width / boardSize.width, visibleSize.height / boardSize.height));
        gameboard->addChild(boardBg);
        mBoard = boardBg;
    } else {
        return false;
    }

    // Tiles
    generateTile();
    generateTile();

    // Event listener
    initListeners();

    return true;
}

void GameplayScene::initListeners() {
    mTouchListener = EventListenerTouchOneByOne::create(); // Create event listener
    if (mTouchListener) {
        // Push down trigger
        mTouchListener->onTouchBegan = [this](Touch* touch, Event* event){
            CCLOG("Touch BEGAN");
            mInitTouchPos = touch->getStartLocation();
            return true;
        };

        // Let up trigger
        mTouchListener->onTouchEnded = [=](Touch* touch, Event* event){
            auto dir = eDirection::UNDEFINED;
            auto loc = touch->getLocation();

            if (loc.x > mInitTouchPos.x + touchSwipeThreshold) {
                dir = eDirection::RIGHT;
                CCLOG("Right");
            } else if (loc.x < mInitTouchPos.x - touchSwipeThreshold) {
                dir = eDirection::LEFT;
                CCLOG("Left");
            } else if (loc.y > mInitTouchPos.y + touchSwipeThreshold) {
                dir = eDirection::UP;
                CCLOG("Up");
            } else if (loc.y < mInitTouchPos.y - touchSwipeThreshold) {
                dir = eDirection::DOWN;
                CCLOG("Down");
            }

            mInitTouchPos = loc;
            CCLOG("Touch ENDED");
        };

        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mTouchListener, this); // Add listener
        CCLOG("Event listener created.");
    }
}

std::pair<int, int> GameplayScene::getRandomPos() {
    // Store available places on the mTileGrid
    std::vector<std::pair<int, int>> buffer;
    for (int x = 0; x < gridSize.first; ++x) {
        for (int y = 0; y < gridSize.second; ++y) {
            if (mTileGrid[{x,y}] == nullptr) {
                buffer.push_back(std::pair<int, int>(x, y));
            }
        }
    }
    if (buffer.empty()) {
        return {-1,-1};
    }

    // Pick random place from the store
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, buffer.size() - 1);
    return buffer[dist(rd)];
}

void GameplayScene::generateTile() {
    auto pos = getRandomPos();
    if (pos == std::pair<int, int>{-1, -1}) {
        CCLOGERROR("No empty space on the grid.");
        return;
    }

    auto tile = TileWidget::create(2, "test");
    mBoard->addChild(tile);
    mTileGrid[pos] = tile;
    tile->setBoardPos(pos);
}

void GameplayScene::onMove(eDirection dir) {
    //
}

