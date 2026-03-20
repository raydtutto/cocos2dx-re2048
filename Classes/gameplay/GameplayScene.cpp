#include "GameplayScene.h"

#include "CustomButton.h"
#include "utils/CustomTransitionSlideInL.h"
#include "MenuScene.h"
#include "TileWidget.h"
#include "OverlayNode.h"

#include "cocostudio/ActionTimeline/CSLoader.h"
#include "cocostudio/ActionTimeline/CCActionTimeline.h"
#include "ui/UIListView.h"
#include "utils/NodeUtils.h"
#include "ui/UIText.h"
#include "editor-support/cocostudio/SimpleAudioEngine.h"

#include <json/document.h>
#include <json/writer.h>

#include "json/prettywriter.h"

USING_NS_CC;

using namespace cocos2d;

namespace {
    constexpr int _gridSizeX = 4;
    constexpr int _gridSizeY = 4;
    auto touchSwipeThreshold = 70.f;
}

// Sound macros
#define PLAY(name) if (cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true)) {  \
CocosDenshion::SimpleAudioEngine::getInstance()->playEffect(name, false, 1.0f, 1.0f, 1.0f);         \
}

#define PLAY_1 PLAY("sounds/click_1.ogg")
#define PLAY_2 PLAY("sounds/click_2.ogg")
#define PLAY_3 PLAY("sounds/click_3.ogg")
#define PLAY_4 PLAY("sounds/click_4.ogg")
#define PLAY_NULL PLAY("sounds/click_0.ogg")
#define PLAY_FAIL PLAY("sounds/move_fail.ogg")
#define PLAY_MAX PLAY("sounds/ding_deep.ogg")

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
        const auto visibleSize = cocos2d::Size(gameboard->getContentSize().width, gameboard->getContentSize().height);
        const auto boardBg = Sprite::create("img/boardBg.png");
        boardBg->setPosition(visibleSize.width / 2, visibleSize.height / 2);
        boardBg->setAnchorPoint({.5f, .5f});
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
            Director::getInstance()->replaceScene(CustomTransitionSlideInL::create(.6f, MenuScene::create()));
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
            resetBoard();
            reinitBoard();
        });
    }

    // Game over overlay
    if (const auto gameOverNode = NodeUtils::getNodeByName(mRoot, "gameOverNode")) {
        mOverlayNode = OverlayNode::create([this]() {
            resetBoard();
            reinitBoard();
            mGameOver = false;
            mUserWon = false;
        });
        gameOverNode->addChild(mOverlayNode);
        mOverlayNode->hideOverlay();
    }

    initListeners();
    scheduleUpdate();

    // Debug unsolvable board
#if defined(DEBUG)
    // Debug unsolvable board
    const auto debugUnsolvableBtn = CustomButton::create();
    debugUnsolvableBtn->setTitleText("Debug unsolvable");
    debugUnsolvableBtn->setTitleFontSize(32);
    debugUnsolvableBtn->setTitleColor(cocos2d::Color3B::RED);
    addChild(debugUnsolvableBtn);

    const auto size = Director::getInstance()->getWinSize();
    debugUnsolvableBtn->setPosition({size.width / 2, 100});

    debugUnsolvableBtn->addClickEventListener([&](Ref*) {
        resetBoard();
        const auto jsonData = FileUtils::getInstance()->getStringFromFile("devUnsolvableBoard.json");
        UserDefault::getInstance()->setStringForKey("saved_board", jsonData);
        reinitBoard();
    });

    // Debug winner board
    const auto debugWinBtn = CustomButton::create();
    debugWinBtn->setTitleText("Debug win");
    debugWinBtn->setTitleFontSize(32);
    debugWinBtn->setTitleColor(cocos2d::Color3B::RED);
    addChild(debugWinBtn);

    debugWinBtn->setPosition({size.width / 2, 50});

    debugWinBtn->addClickEventListener([&](Ref*) {
        resetBoard();
        const auto jsonData = FileUtils::getInstance()->getStringFromFile("devWinBoard.json");
        UserDefault::getInstance()->setStringForKey("saved_board", jsonData);
        reinitBoard();
    });
#endif

    return true;
}

void GameplayScene::onExit() {
    Scene::onExit();
    saveBoard();
}

void GameplayScene::update(float dt) {
    Scene::update(dt);
    mMoveTimer += dt;
}

void GameplayScene::reinitBoard() {
    mGameOver = false;

    if (mOverlayNode) {
        mOverlayNode->hideOverlay();
    }

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

    tryLoadBoard();
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
                case EventKeyboard::KeyCode::KEY_R: {
                    resetBoard();
                    reinitBoard();
                    break;
                }
                case EventKeyboard::KeyCode::KEY_ESCAPE: {
                    CCLOG("Back to menu.");
                    if (cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true))
                        CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);

                    Director::getInstance()->replaceScene(CustomTransitionSlideInL::create(.4f, MenuScene::create()));
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
    if (dir == eDirection::UNDEFINED || mGameOver) {
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
        PLAY("sounds/click_0.ogg")

        // Win condition
        if (numMergedMax == 4096 && !mUserWon) {
            mUserWon = true;
            if (mOverlayNode) {
                mOverlayNode->showOverlay(eOverlayType::WIN);
            }
        }

        // Continue generate tile
        if (!mUserWon) {
            auto seq = Sequence::create(DelayTime::create(TileWidget::getTimeDelay()/2), CallFunc::create([this]() {
               generateTile();
           }), nullptr);
            runAction(seq);
        }
    } else {
        PLAY("sounds/move_fail.ogg")
        animateMoveFail(dir);

        if (checkUnsolvableBoard()) {
            CCLOG("Game over!");
            gameOver();
        }
    }
}

void GameplayScene::playSound(const int num) {
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
        default:
            break;
    }
}

void GameplayScene::gameOver() {
    if (mGameOver || !mOverlayNode)
        return;
    mGameOver = true;
    mOverlayNode->showOverlay(eOverlayType::GAME_OVER);
}

void GameplayScene::animateMoveFail(const eDirection dir) const {
    const auto bg = NodeUtils::getNodeByName(mRoot,"gameboard");

    const bool isHorizontal = (dir == eDirection::LEFT || dir == eDirection::RIGHT);
    const cocos2d::Vec2 offset = isHorizontal ? cocos2d::Vec2(8, 0) : cocos2d::Vec2(0, 8);
    constexpr float duration = 0.05f;

    const auto moveR = cocos2d::MoveBy::create(duration, offset);
    const auto moveL = moveR->reverse();
    const auto moveBack = cocos2d::MoveTo::create(duration, bg->getPosition());

    const auto seq = Sequence::create(
        cocos2d::EaseSineInOut::create(moveR),
        cocos2d::EaseSineInOut::create(moveL),
        cocos2d::EaseSineInOut::create(moveR->clone()),
        cocos2d::EaseSineInOut::create(moveL->clone()),
        cocos2d::EaseSineInOut::create(moveBack),
        nullptr
    );

    bg->runAction(seq);
}

void GameplayScene::saveBoard() {
    // Create document
    rapidjson::Document doc;
    doc.SetArray();
    auto& allocator = doc.GetAllocator();

    // Write to doc
    for (int x = 0; x < _gridSizeX; ++x) {
        for (int y = 0; y < _gridSizeY; ++y) {
            rapidjson::Value row(rapidjson::kObjectType);
            row.AddMember("x", x, allocator);
            row.AddMember("y", y, allocator);
            row.AddMember("val", !mTileGrid[{x, y}] ? 0 : mTileGrid[{x, y}]->getNumber(), allocator);
            doc.PushBack(row, allocator);
        }
    }

    // Convert to .json
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    const char* output = buffer.GetString();

    // Save string into UserDefault
    cocos2d::UserDefault::getInstance()->setStringForKey("saved_board", output);
    CCLOG("%s", output);
}

void GameplayScene::tryLoadBoard() {
    // Extract data from a file
    const auto jsonData = UserDefault::getInstance()->getStringForKey("saved_board", "");
    CCLOG("%s", jsonData.c_str());
    if (jsonData.empty()) {
        resetBoard();
        generateTile();
        generateTile();
        return;
    }

    // Parse data
    rapidjson::Document document;
    if (document.Parse(jsonData.c_str()).HasParseError() || !document.IsArray()) {
        CCLOGERROR("Failed to parse json %d", document.GetParseError());
        resetBoard();
        generateTile();
        generateTile();
        return;
    }

    int count = 0;
    const auto array = document.GetArray();
    for (auto it = array.Begin(); it != array.End(); ++it) {
        if (it->IsObject() && it->HasMember("x") && it->HasMember("y") && it->HasMember("val")) {
            auto obj = it->GetObject();
            int x = obj["x"].GetInt();
            int y = obj["y"].GetInt();
            int val = obj["val"].GetInt();

            // Add tile to board
            if (val == 0) {
                mTileGrid[std::pair<int, int>(x, y)] = nullptr;
            } else {
                const auto tile = TileWidget::create(val);
                mBoard->addChild(tile);
                mTileGrid[std::pair<int, int>(x, y)] = tile;
                tile->setBoardPos(std::pair<int, int>(x, y), false);
                count++;
            }
        } else {
            CCLOGERROR("No value for x,y in array.");
            return;
        }
    }

    if (count == 0) {
        resetBoard();
        generateTile();
        generateTile();
    }
}

void GameplayScene::resetBoard() {
    UserDefault::getInstance()->setStringForKey("saved_board", "");
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

void GameplayScene::onEnterTransitionDidFinish() {
    Scene::onEnterTransitionDidFinish();
    reinitBoard();
}
