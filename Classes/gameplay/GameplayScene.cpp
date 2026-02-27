#include "GameplayScene.h"

#include "CustomButton.h"
#include "MenuScene.h"
#include "TileWidget.h"

#include "cocostudio/ActionTimeline/CSLoader.h"
#include "ui/UIButton.h"
#include "ui/UIListView.h"
#include "utils/NodeUtils.h"
#include "ui/UIText.h"

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
        auto boardBg = Sprite::create("img/boardBg.png");
        auto boardSize = boardBg->getContentSize();
        boardBg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        boardBg->setScale(std::min(visibleSize.width / boardSize.width, visibleSize.height / boardSize.height));
        gameboard->addChild(boardBg);
        mBoard = boardBg;
    } else {
        return false;
    }

    // Create score
    if (auto gameScore = dynamic_cast<cocos2d::ui::Text*>(NodeUtils::getNodeByName(mRoot, "score"))) {
        gameScore->setString("0");
        gameScore->setFontSize(128.f);

        // Access user default
        auto userDefaults = cocos2d::UserDefault::getInstance();
        updateScore(mGameScore);
    } else {
        return false;
    }

    // Menu button
    // if (auto menuButton = dynamic_cast<cocos2d::ui::Button*>(NodeUtils::getNodeByName(mRoot, "menuBtn"))) {
    if (auto menuButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "menuBtn"))) {
        CCLOG("Back to menu.");
        menuButton->addClickEventListener([](cocos2d::Ref*) {
            CCLOG("Back to menu.");
            Director::getInstance()->replaceScene(MenuScene::create());
        });
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

    scheduleUpdate();

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

void GameplayScene::generateTile(bool animate) {
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
    if (mMoveTimer < TileWidget::getTimeDelay()) {
        return;
    }
    mMoveTimer = 0.f;

    // for each row/column 0[] 1[X] 2[] 3[X] -> 0[X] 1[X] 2[] 3[]

    if (dir == eDirection::DOWN) {
        for (int x = 0; x < _gridSizeX; ++x) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({x, 0}));
            col.push_back(findCell({x, 1}));
            col.push_back(findCell({x, 2}));
            col.push_back(findCell({x, 3}));
            movedCells = std::max(movedCells, matchTileRow(col));
        }
    }

    if (dir == eDirection::UP) {
        for (int x = 0; x < _gridSizeX; ++x) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({x, 3}));
            col.push_back(findCell({x, 2}));
            col.push_back(findCell({x, 1}));
            col.push_back(findCell({x, 0}));
            movedCells = std::max(movedCells, matchTileRow(col));
        }
    }

    // UP down->up
    // create arrays for each column
    // - - - -
    // - - - -
    // - - - -
    // - X - -

    if (dir == eDirection::LEFT) {
        for (int y = 0; y < _gridSizeY; ++y) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({0, y}));
            col.push_back(findCell({1, y}));
            col.push_back(findCell({2, y}));
            col.push_back(findCell({3, y}));
            movedCells = std::max(movedCells, matchTileRow(col));
        }
    }

    if (dir == eDirection::RIGHT) {
        for (int y = 0; y < _gridSizeY; ++y) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({3, y}));
            col.push_back(findCell({2, y}));
            col.push_back(findCell({1, y}));
            col.push_back(findCell({0, y}));
            movedCells = std::max(movedCells, matchTileRow(col));
        }
    }

    if (movedCells > 0) {
        auto seq = Sequence::create(DelayTime::create(TileWidget::getTimeDelay()/2), CallFunc::create([this]() {
            generateTile();
        }), nullptr);
        runAction(seq);
    }
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
            // Update score
            updateScore(list[i]->getNumber()*2);

            // Merge
            list[i]->setNumber(list[i]->getNumber()*2);
            list[i + 1]->removeFromParentAndCleanup(true);
            list[i + 1] = nullptr;
            merged.push_back(list[i]);
            i++; // Skip removed element
            moved++;
        } else {
            merged.push_back(list[i]);
        }
    }

    // Update buffer
    for (int i = 0; i < buffer.size(); ++i) {
        if (i < merged.size()) {
            // Write
            buffer[i]->second = merged[i];
            if (buffer[i]->first != merged[i]->getBoardPos())
                moved++;
            buffer[i]->second->setBoardPos(buffer[i]->first, true);
        } else {
            // Clean
            buffer[i]->second = nullptr;
        }
    }
    return moved;
}

void GameplayScene::updateScore(const int num) {
    // Score reset
    // cocos2d::UserDefault::getInstance()->setIntegerForKey("best_score", 0);

    // Score update
    mGameScore += num;
    if (auto gameScore = dynamic_cast<cocos2d::ui::Text*>(NodeUtils::getNodeByName(mRoot, "score"))) {
        gameScore->setString(std::to_string(mGameScore));

        // Scale animation
        if (mGameScore != 0) {
            const auto scaleBy = cocos2d::ScaleBy::create(.14f, .9f);
            const auto scaleEaseIn = EaseQuadraticActionInOut::create(scaleBy->clone());
            const auto scaleSeq = Sequence::create(scaleEaseIn, scaleEaseIn->reverse(), nullptr);
            gameScore->runAction(scaleSeq);
        }
    } else {
        CCLOGERROR("No game score node.");
    }

    // Update user score
    if (auto bestScoreLabel = dynamic_cast<cocos2d::ui::Text*>(NodeUtils::getNodeByName(mRoot, "bestScore"))) {
        int bestScore = cocos2d::UserDefault::getInstance()->getIntegerForKey("best_score", 0);

        // Update best score
        if (mGameScore > bestScore) {
            bestScore = mGameScore;
            cocos2d::UserDefault::getInstance()->setIntegerForKey("best_score", bestScore);
        }
        bestScoreLabel->setString(std::to_string( bestScore));

        // Center the best score label
        if (auto bestScoreHolder = NodeUtils::getNodeByName(mRoot, "bestScoreHolder")) {
            float totalWidth = 0;
            for (auto child : bestScoreHolder->getChildren()) {
                totalWidth += child->getContentSize().width;
            }
            bestScoreHolder->setContentSize(cocos2d::Size(totalWidth, bestScoreHolder->getContentSize().height));
        }
    } else {
        CCLOGERROR("No game score.");
    }
}

void GameplayScene::update(float delta) {
    Scene::update(delta);
    mMoveTimer += delta;
}
