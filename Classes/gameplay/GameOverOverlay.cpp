#include "GameOverOverlay.h"

#include "CustomButton.h"
#include "MenuScene.h"
#include "cocostudio/SimpleAudioEngine.h"
#include "cocostudio/ActionTimeline/CCActionTimeline.h"
#include "cocostudio/ActionTimeline/CSLoader.h"
#include "ui/UIListView.h"
#include "utils/CustomTransitionSlideInL.h"
#include "utils/NodeUtils.h"

using namespace cocos2d;

GameOverOverlay * GameOverOverlay::create(std::function<void()> onRestart) {
    auto pRet = new(std::nothrow) GameOverOverlay();
    if (pRet && pRet->initWithCallback(onRestart)) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        return nullptr;
    }
}

bool GameOverOverlay::initWithCallback(std::function<void()> onRestart) {
    if (!Node::init()) {
        return false;
    }

    mOnRestart = onRestart;

    mRoot = CSLoader::createNodeWithVisibleSize("widgets/gameOverOverlay.csb");
    if (mRoot) {
        addChild(mRoot);
    } else {
        return false;
    }
    mRoot->setAnchorPoint({0.5f, 0.5f});

    const auto gameOverHolder = NodeUtils::getNodeByName(mRoot, "gameOverHolder");
    if (!gameOverHolder) {
        CCLOGERROR("Game over missed.");
        return false;
    }

    // Remove scrollbar
    if (const auto buttonHolder = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "buttonHolder"))) {
        buttonHolder->setScrollBarEnabled(false);
    }

    // Restart button
    if (const auto restartButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "restartBtnGameOver"))) {
        restartButton->addClickEventListener([this](cocos2d::Ref*) {
            if (mIsRestarting)
                return;

            mIsRestarting = true;

            if (const auto audioEnabled = cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true))
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);

            // Sequence animation
            if (const auto timeline = CSLoader::createTimeline("widgets/gameOverOverlay.csb")) {
                mRoot->runAction(timeline);
                timeline->play("gameOverFadeOut", false);

                Vector<FiniteTimeAction *> list;
                list.pushBack(cocos2d::DelayTime::create(.5f));
                list.pushBack(cocos2d::CallFunc::create([this]() {
                    if (mOnRestart) {
                        mOnRestart();
                    }
                }));
                list.pushBack(cocos2d::EaseOut::create(cocos2d::FadeOut::create(.2f), .2f));
                list.pushBack(cocos2d::CallFunc::create([this]() {
                    mIsRestarting = false;
                    hideGameOver();
                }));
                runAction(Sequence::create(list));
            } else {
                if (mOnRestart) {
                    mOnRestart();
                }
                mIsRestarting = false;
                hideGameOver();
            }
        });
    }

    // Menu button
    if (const auto menuButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "menuBtnGameOver"))) {
        menuButton->addClickEventListener([this](cocos2d::Ref*) {
            if (mIsRestarting)
                return;
            CCLOG("Back to menu.");

            if (const auto audioEnabled = cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true))
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);

            // Scene transition
            Director::getInstance()->replaceScene(CustomTransitionSlideInL::create(.4f, MenuScene::create()));
        });
    }

    return true;
}

void GameOverOverlay::showGameOver() {
    setVisible(true);
    setOpacity(0);
    stopAllActions();
    runAction(cocos2d::FadeIn::create(.2f));

    // Load timeline animation
    if (const auto timeline = CSLoader::createTimeline("widgets/gameOverOverlay.csb")) {
        mRoot->runAction(timeline);
        timeline->play("gameOverFadeIn", false);
    }
    CCLOG("Timeline start.");
}

void GameOverOverlay::hideGameOver() {
    setVisible(false);
}
