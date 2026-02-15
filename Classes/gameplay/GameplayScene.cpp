#include "GameplayScene.h"
#include "TileWidget.h"

#include "cocostudio/ActionTimeline/CSLoader.h"
#include "utils/NodeUtils.h"

USING_NS_CC;

using namespace cocos2d;

namespace {
    const int _gridSizeX = 4;
    const int _gridSizeY = 4;
    auto touchSwipeThreshold = 70.f;
}

bool GameplayScene::init() {
    if (!Scene::init()) {
        return false;
    }

    // Background color
    const Color4B colorBg = Color4B(40, 40, 45, 255);
    const auto layerColor = LayerColor::create(colorBg);
    addChild(layerColor);

    mRoot = CSLoader::createNodeWithVisibleSize("GUI/cocosstudio/widgets/gameplayScene.csb");
    if (mRoot) {
        addChild(mRoot);
    } else {
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
    generateTile();
    generateTile();
    generateTile();
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
        mTouchListener->onTouchBegan = [this](Touch *touch, Event *event) {
            CCLOG("Touch BEGAN");
            mInitTouchPos = touch->getStartLocation();
            return true;
        };

        // Let up trigger
        mTouchListener->onTouchEnded = [=](Touch *touch, Event *event) {
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
            onMove(dir);
        };

        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mTouchListener, this);
        // Add listener
        CCLOG("Event listener created.");
    }
}

std::pair<int, int> GameplayScene::getRandomPos() {
    // Store available places on the mTileGrid
    std::vector<std::pair<int, int> > buffer;
    for (int x = 0; x < _gridSizeX; ++x) {
        for (int y = 0; y < _gridSizeY; ++y) {
            if (mTileGrid[{x, y}] == nullptr) {
                buffer.emplace_back(std::pair<int, int>(x, y));
            }
        }
    }
    if (buffer.empty()) {
        return {-1, -1};
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
    if (dir == eDirection::UNDEFINED)
        return;

    // for each row/column 0[] 1[X] 2[] 3[X] -> 0[X] 1[X] 2[] 3[]

    if (dir == eDirection::DOWN) {
        for (int x = 0; x < _gridSizeX; ++x) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({x, 0}));
            col.push_back(findCell({x, 1}));
            col.push_back(findCell({x, 2}));
            col.push_back(findCell({x, 3}));
            movedCells = matchTileRow(col);
        }
    }


    // DOWN up->down
    // create arrays for each column
    // - X - -
    // - - - -
    // - - - -
    // - - - -

    // UP down->up
    // create arrays for each column
    // - - - -
    // - - - -
    // - - - -
    // - X - -

    // LEFT right to left
    // create arrays for each row
    // - - - -
    // - - - -
    // - - - -
    // X - - -
}

int GameplayScene::matchTileRow(std::vector<TileGrid::iterator>& buffer) {
    int moved = 0;
    // Get pointers
    std::vector<TileWidget*> list;
    for (auto item : buffer) {
        if (item->second != nullptr) {
            list.push_back(item->second);
        }
    }

    // Merge elements in the list
    std::vector<TileWidget*> merged;
    for (int i = 0; i < list.size(); ++i) {
        if (i + 1 < list.size() && list[i + 1]->getNumber() == list[i]->getNumber()) {
            // Merge
            list[i]->setNumber(list[i]->getNumber()*2);
            list[i + 1]->removeFromParentAndCleanup(true);
            list[i + 1] = nullptr;
            merged.push_back(list[i]);
            i++; // Skip removed element
            moved++;

            // todo score placement

        } else {
            merged.push_back(list[i]);
        }
    }

    // Update buffer
    for (int i = 0; i < buffer.size(); ++i) {
        if (i < merged.size()) {
            // Write
            buffer[i]->second = merged[i];
            buffer[i]->second->setBoardPos(buffer[i]->first);
            moved++;
        } else {
            // Clean
            buffer[i]->second = nullptr;
        }
    }
    return moved;
}
