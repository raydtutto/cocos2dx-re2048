#include "GameplayScene.h"

#include "CustomButton.h"
#include "MenuScene.h"
#include "TileWidget.h"

#include "cocostudio/ActionTimeline/CSLoader.h"
#include "cocostudio/ActionTimeline/CCActionTimeline.h"
#include "ui/UIListView.h"
#include "utils/NodeUtils.h"
#include "ui/UIText.h"
#include "editor-support/cocostudio/SimpleAudioEngine.h"

USING_NS_CC;

using namespace cocos2d;

namespace {
    constexpr int _gridSizeX = 4;
    constexpr int _gridSizeY = 4;
    auto touchSwipeThreshold = 70.f;
}

// Sounds
#define PLAY_1 CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/click_1.ogg", false, 1.0f, 1.0f, 1.0f)
#define PLAY_2 CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/click_2.ogg", false, 1.0f, 1.0f, 1.0f)
#define PLAY_3 CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/click_3.ogg", false, 1.0f, 1.0f, 1.0f)
#define PLAY_4 CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/click_4.ogg", false, 1.0f, 1.0f, 1.0f)
#define PLAY_NULL CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/click_0.ogg", false, 1.0f, 1.0f, 1.0f)
#define PLAY_MAX CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/ding_deep.ogg", false, 1.0f, 1.0f, 1.0f)

GameplayScene* GameplayScene::create() {
    auto pRet = new(std::nothrow) GameplayScene();
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        return nullptr;
    }
}

GameplayScene::~GameplayScene() {
    if (mTouchListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(mTouchListener);
        mTouchListener = nullptr;
    }
    if (mKeyboardListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(mKeyboardListener);
        mKeyboardListener = nullptr;
    }
}

bool GameplayScene::init() {
    if (!Scene::init())
        return false;

    // Background color
    const auto colorBg = Color4B(40, 40, 45, 255);
    const auto layerColor = LayerColor::create(colorBg);
    addChild(layerColor);

    mRoot = CSLoader::createNodeWithVisibleSize("GUI/cocosstudio/widgets/gameplayScene.csb");
    if (mRoot) {
        addChild(mRoot);
    } else {
        return false;
    }

    // Create gameboard
    if (const auto gameboard = NodeUtils::getNodeByName(mRoot, "gameboard")) {
        const auto visibleSize = gameboard->getContentSize();
        const auto boardBg = Sprite::create("img/boardBg.png");
        const auto boardSize = boardBg->getContentSize();
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
        updateScore(mGameScore);
    } else {
        return false;
    }

    // Menu button
    if (const auto menuButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "menuBtn"))) {
        menuButton->addClickEventListener([](cocos2d::Ref*) {
            CCLOG("Back to menu.");
            if (cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true)) {
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);
            }
            Director::getInstance()->replaceScene(MenuScene::create());
        });
    } else {
        return false;
    }

    // Reset button
    if (const auto resetButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "resetBtn"))) {
        resetButton->addClickEventListener([this](cocos2d::Ref*) {
            CCLOG("Reset board.");
            if (cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true)) {
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);
            }
            reinitBoard();
        });
    }

    reinitBoard();
    initListeners();
    scheduleUpdate();

    return true;
}

void GameplayScene::reinitBoard() {
    mGameOver = false;
    mIsRestarting = false;

    const auto gameOverHolder = NodeUtils::getNodeByName(mRoot, "gameOverHolder");
    gameOverHolder->setVisible(false);

    for (int x = 0; x < _gridSizeX; ++x) {
        for (int y = 0; y < _gridSizeY; ++y) {
            if (mTileGrid[{x, y}] != nullptr) {
                mTileGrid[{x, y}]->removeFromParentAndCleanup(true);
            }
        }
    }
    mTileGrid.clear();

    // Reset score
    mGameScore = 0;
    updateScore(0);

    if (const auto bestScoreHolder = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "bestScoreHolder")))
        bestScoreHolder->setScrollBarEnabled(false);

    mMoveTimer = TileWidget::getTimeDelay();

    debugGenerateUnsolvableBoard();
    // generateTile();
    // generateTile();
}

void GameplayScene::initListeners() {
    if (mTouchListener)
        Director::getInstance()->getEventDispatcher()->removeEventListener(mTouchListener);

    mTouchListener = EventListenerTouchOneByOne::create(); // Create event listener
    if (mTouchListener) {
        // Push down trigger
        mTouchListener->onTouchBegan = [this](const Touch* touch, Event* event) {
            CCLOG("Touch BEGAN");
            mInitTouchPos = touch->getStartLocation();
            return true;
        };

        // Let up trigger
        mTouchListener->onTouchEnded = [this](const Touch* touch, Event* event) {
            auto dir = eDirection::UNDEFINED;
            const auto loc = touch->getLocation();

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

    if (mKeyboardListener)
        Director::getInstance()->getEventDispatcher()->removeEventListener(mKeyboardListener);

    mKeyboardListener = cocos2d::EventListenerKeyboard::create();
    if (mKeyboardListener) {
        mKeyboardListener->onKeyReleased = [this](EventKeyboard::KeyCode keyCode, auto /*event*/) {
            if (mGameOver)
                return;

            switch (keyCode) {
                case EventKeyboard::KeyCode::KEY_UP_ARROW:
                    onMove(eDirection::UP);
                    break;
                case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
                    onMove(eDirection::DOWN);
                    break;
                case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
                    onMove(eDirection::LEFT);
                    break;
                case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
                    onMove(eDirection::RIGHT);
                    break;
                case EventKeyboard::KeyCode::KEY_R:
                    reinitBoard();
                    break;
                case EventKeyboard::KeyCode::KEY_ESCAPE: {
                    CCLOG("Back to menu.");
                    if (cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true))
                        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);

                    Director::getInstance()->replaceScene(MenuScene::create());
                    break;
                }
                default:
                    break;
            }
        };
        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mKeyboardListener, this);
    }
}

std::pair<int, int> GameplayScene::getRandomPos() {
    // Store available places on the mTileGrid
    std::vector<std::pair<int, int> > buffer;
    for (int x = 0; x < _gridSizeX; ++x) {
        for (int y = 0; y < _gridSizeY; ++y) {
            if (mTileGrid[{x, y}] == nullptr)
                buffer.emplace_back(std::pair<int, int>(x, y));
        }
    }
    if (buffer.empty())
        return {-1, -1};

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

    const auto tile = TileWidget::create(2);
    mBoard->addChild(tile);
    mTileGrid[pos] = tile;
    tile->setBoardPos(pos, false);

    // Scale animation
    tile->setScale(.9f);
    const auto scaleTo = cocos2d::ScaleTo::create(.8f, 1.f);
    const auto scaleEaseOut = EaseElasticOut::create(scaleTo);
    tile->runAction(scaleEaseOut);
}

void GameplayScene::onMove(const eDirection dir) {
    if (dir == eDirection::UNDEFINED || mGameOver || mIsRestarting) {
        CCLOGERROR("Cannot move tiles.");
        return;
    }

    if (mMoveTimer < TileWidget::getTimeDelay())
        return;

    mMoveTimer = 0.f;

    auto findCell = [this](const std::pair<int, int>& key) {
        return mTileGrid.find(key);
    };

    int movedCells = 0;
    int numMergedMax = 0;

    if (dir == eDirection::DOWN) {
        for (int x = 0; x < _gridSizeX; ++x) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({x, 0}));
            col.push_back(findCell({x, 1}));
            col.push_back(findCell({x, 2}));
            col.push_back(findCell({x, 3}));

            auto result = matchTileRow(col);  // <1 moved cells, 2 max merged number>

            movedCells = std::max(movedCells, result.first);
            numMergedMax = (numMergedMax > result.second) ? numMergedMax : result.second;
        }
    }

    if (dir == eDirection::UP) {
        for (int x = 0; x < _gridSizeX; ++x) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({x, 3}));
            col.push_back(findCell({x, 2}));
            col.push_back(findCell({x, 1}));
            col.push_back(findCell({x, 0}));

            auto result = matchTileRow(col); // <1 moved cells, 2 max merged number>

            movedCells = std::max(movedCells, result.first);
            numMergedMax = (numMergedMax > result.second) ? numMergedMax : result.second;
        }
    }

    if (dir == eDirection::LEFT) {
        for (int y = 0; y < _gridSizeY; ++y) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({0, y}));
            col.push_back(findCell({1, y}));
            col.push_back(findCell({2, y}));
            col.push_back(findCell({3, y}));

            auto result = matchTileRow(col); // <1 moved cells, 2 max merged number>

            movedCells = std::max(movedCells, result.first);
            numMergedMax = (numMergedMax > result.second) ? numMergedMax : result.second;
        }
    }

    if (dir == eDirection::RIGHT) {
        for (int y = 0; y < _gridSizeY; ++y) {
            std::vector<TileGrid::iterator> col;
            col.push_back(findCell({3, y}));
            col.push_back(findCell({2, y}));
            col.push_back(findCell({1, y}));
            col.push_back(findCell({0, y}));

            auto result = matchTileRow(col); // <1 moved cells, 2 max merged number>

            movedCells = std::max(movedCells, result.first);
            numMergedMax = (numMergedMax > result.second) ? numMergedMax : result.second;
        }
    }

    playSound(numMergedMax);

    if (movedCells > 0) {
        auto seq = Sequence::create(DelayTime::create(TileWidget::getTimeDelay()/2), CallFunc::create([this]() {
            generateTile();
        }), nullptr);
        runAction(seq);
    } else {
        if (checkUnsolvableBoard()) {
            CCLOG("Game over!");
            gameOver();
        }
    }
}

void GameplayScene::playSound(const int num) {
    const auto audioEnabled = cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true);
    if (!audioEnabled)
        return;

    switch (num) {
        case 4:
        case 8:
            PLAY_1;
            break;
        case 16:
        case 32:
        case 64:
            PLAY_2;
            break;
        case 128:
        case 256:
        case 512:
            PLAY_3;
            break;
        case 1024:
        case 2048:
            PLAY_4;
            break;
        case 4096:
            PLAY_MAX;
            break;
        case 0:
            PLAY_NULL;
            break;
        default:
            break;
    }
}

void GameplayScene::gameOver() {
    if (mGameOver)
        return;
    mGameOver = true;

    const auto gameOverHolder = NodeUtils::getNodeByName(mRoot, "gameOverHolder");
    if (!gameOverHolder) {
        CCLOGERROR("GameOverHolder node lost.");
        return;
    }

    if (const auto buttonHolder = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "buttonHolder")))
        buttonHolder->setScrollBarEnabled(false);

    gameOverHolder->setVisible(true);
    gameOverHolder->setOpacity(0);
    gameOverHolder->stopAllActions();
    gameOverHolder->runAction(cocos2d::FadeIn::create(.2f));

    // Load timeline animation
    if (const auto timeline = CSLoader::createTimeline("widgets/gameplayScene.csb")) {
        mRoot->runAction(timeline);
        timeline->play("gameOverFadeIn", false);
    }

    const auto audioEnabled = cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true);

    // Restart button
    if (const auto restartButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "restartBtnGameOver"))) {
        restartButton->addClickEventListener([this, audioEnabled, gameOverHolder](cocos2d::Ref*) {
            if (mIsRestarting)
                return;

            mIsRestarting = true;

            if (audioEnabled)
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);

            // Sequence animation
            if (const auto timeline = CSLoader::createTimeline("widgets/gameplayScene.csb")) {
                mRoot->runAction(timeline);
                timeline->play("gameOverFadeOut", false);

                Vector<FiniteTimeAction *> list;
                list.pushBack(cocos2d::DelayTime::create(.5f));
                list.pushBack(cocos2d::CallFunc::create([this]() {
                    this->reinitBoard();
                    this->mGameOver = false;
                }));
                list.pushBack(cocos2d::EaseOut::create(cocos2d::FadeOut::create(.2f), .2f));
                list.pushBack(cocos2d::CallFunc::create([gameOverHolder]() {
                    gameOverHolder->setVisible(false);
                }));
                gameOverHolder->runAction(Sequence::create(list));
            } else {
                this->reinitBoard();
                this->mGameOver = false;
                gameOverHolder->setVisible(false);
            }
        });
    }

    // Menu button
    if (const auto menuButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "menuBtnGameOver"))) {
        menuButton->addClickEventListener([this, audioEnabled](cocos2d::Ref*) {
            if (mIsRestarting)
                return;
            CCLOG("Back to menu.");

            if (audioEnabled)
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);

            Director::getInstance()->replaceScene(MenuScene::create());
        });
    }
}

std::pair<int, int> GameplayScene::matchTileRow(std::vector<TileGrid::iterator>& buffer) {
    int moved = 0;

    // Get pointers
    std::vector<TileWidget*> list;
    for (const auto item : buffer) {
        if (item->second != nullptr)
            list.push_back(item->second);
    }

    // Merge elements in the list
    std::vector<TileWidget*> merged;
    int numMaxMerged = 0;
    for (int i = 0; i < list.size(); ++i) {
        if (i + 1 < list.size() && list[i + 1]->getNumber() == list[i]->getNumber()) {
            numMaxMerged = std::max(numMaxMerged, list[i]->getNumber()*2);

            // Update score
            updateScore(list[i]->getNumber()*2);

            // Merge
            list[i]->setNumber(list[i]->getNumber()*2);
            // list[i + 1]->removeFromParentAndCleanup(true);
            list[i + 1]->removeWidget();
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

    return {moved, numMaxMerged};
}

void GameplayScene::updateScore(const int num) {
    // Score reset
    cocos2d::UserDefault::getInstance()->setIntegerForKey("best_score", 0);

    // Score update
    mGameScore += num;
    if (const auto gameScore = dynamic_cast<cocos2d::ui::Text*>(NodeUtils::getNodeByName(mRoot, "score"))) {
        gameScore->setString(std::to_string(mGameScore));
    } else {
        CCLOGERROR("No game score node.");
    }

    // Update user score
    if (const auto bestScoreLabel = dynamic_cast<cocos2d::ui::Text*>(NodeUtils::getNodeByName(mRoot, "bestScore"))) {
        int bestScore = cocos2d::UserDefault::getInstance()->getIntegerForKey("best_score", 0);

        // Update best score
        if (mGameScore > bestScore) {
            bestScore = mGameScore;
            cocos2d::UserDefault::getInstance()->setIntegerForKey("best_score", bestScore);
        }
        bestScoreLabel->setString(std::to_string( bestScore));

        // Center the best score label
        if (const auto bestScoreHolder = NodeUtils::getNodeByName(mRoot, "bestScoreHolder")) {
            float totalWidth = 0;
            for (const auto child : bestScoreHolder->getChildren()) {
                totalWidth += child->getContentSize().width;
            }
            bestScoreHolder->setContentSize(cocos2d::Size(totalWidth, bestScoreHolder->getContentSize().height));
        }
    } else {
        CCLOGERROR("No game score.");
    }
}

void GameplayScene::debugGenerateUnsolvableBoard() {
    std::vector<std::tuple<int, int, int>> list = { // x y num
        {0, 0, 2},
        {0, 1, 4},
        {0, 2, 8},
        {0, 3, 16},

        {1, 0, 32},
        {1, 1, 64},
        {1, 2, 128},
        {1, 3, 256},

        {2, 0, 512},
        {2, 1, 1024},
        {2, 2, 2048},
        {2, 3, 4096},

        {3, 0, 2},
        {3, 1, 4},
        {3, 2, 8},
        {3, 3, 16}
    };

    for (const auto& [x, y, num] : list) {
        auto tile = TileWidget::create(num);
        mBoard->addChild(tile);
        mTileGrid[std::pair<int, int>(x, y)] = tile;
        tile->setBoardPos(std::pair<int, int>(x, y), false);
    }
}

bool GameplayScene::checkUnsolvableBoard() {
    for (int x = 0; x < _gridSizeX; ++x) {
        for (int y = 0; y < _gridSizeY; ++y) {
            if (mTileGrid[{x, y}] == nullptr)
                return false;

            int nextX = x + 1;
            if (nextX <= _gridSizeX && mTileGrid[{nextX, y}] != nullptr) {
                if (mTileGrid[{x, y}]->getNumber() == mTileGrid[{nextX, y}]->getNumber())
                    return false;
            }

            int nextY = y + 1;
            if (nextY <= _gridSizeY && mTileGrid[{x, nextY}] != nullptr) {
                if (mTileGrid[{x, y}]->getNumber() == mTileGrid[{x, nextY}]->getNumber())
                    return false;
            }
        }
    }

    return true;
}

void GameplayScene::update(float delta) {
    Scene::update(delta);
    mMoveTimer += delta;
}
