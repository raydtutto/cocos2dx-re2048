#include "OverlayNode.h"

#include "CustomButton.h"
#include "MenuScene.h"
#include "audio/AudioEngine.h"
#include "cocostudio/ActionTimeline/ActionTimeline.h"
#include "cocostudio/ActionTimeline/CSLoader.h"
#include "ui/UIListView.h"
#include "ui/UIText.h"
#include "utils/CustomTransitionSlideInL.h"
#include "utils/NodeUtils.h"

using namespace ax;

OverlayNode * OverlayNode::create(const std::function<void()>& onRestart) {
    auto pRet = new(std::nothrow) OverlayNode();
    if (pRet && pRet->initWithCallback(onRestart)) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        return nullptr;
    }
}

bool OverlayNode::initWithCallback(const std::function<void()>& onRestart) {
    if (!Node::init()) {
        return false;
    }

    mOnRestart = onRestart;

    mRoot = CSLoader::createNodeWithVisibleSize("widgets/overlayNode.csb");
    if (mRoot) {
        addChild(mRoot);
    } else {
        return false;
    }
    mRoot->setAnchorPoint({0.5f, 0.5f});

    const auto gameOverHolder = NodeUtils::getNodeByName(mRoot, "gameOverHolder");
    if (!gameOverHolder) {
        AXLOGERROR("Game over missed.");
        return false;
    }

    // Remove scrollbar
    if (const auto buttonHolder = dynamic_cast<ax::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "buttonHolder"))) {
        buttonHolder->setScrollBarEnabled(false);
    }

    // Restart button
    if (const auto restartButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "restartBtnGameOver"))) {
        restartButton->addClickEventListener([this](ax::Object*) {
            if (mIsRestarting)
                return;

            mIsRestarting = true;

            if (const auto audioEnabled = ax::UserDefault::getInstance()->getBoolForKey("audioEnabled", true))
                AudioEngine::play2d("sounds/button.ogg", false);

            // Sequence animation
            if (const auto timeline = CSLoader::createTimeline("widgets/overlayNode.csb")) {
                mRoot->runAction(timeline);
                timeline->play("gameOverFadeOut", false);

                Vector<FiniteTimeAction *> list;
                list.pushBack(ax::DelayTime::create(.5f));
                list.pushBack(ax::CallFunc::create([this]() {
                    if (mOnRestart) {
                        mOnRestart();
                    }
                }));
                list.pushBack(ax::EaseOut::create(ax::FadeOut::create(.2f), .2f));
                list.pushBack(ax::CallFunc::create([this]() {
                    mIsRestarting = false;
                    hideOverlay();
                }));
                runAction(Sequence::create(list));
            } else {
                if (mOnRestart) {
                    mOnRestart();
                }
                mIsRestarting = false;
                hideOverlay();
            }
        });
    }

    // Menu button
    if (const auto menuButton = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(mRoot, "menuBtnGameOver"))) {
        menuButton->addClickEventListener([this](ax::Object*) {
            if (mIsRestarting)
                return;
            AXLOG("Back to menu.");

            if (const auto audioEnabled = ax::UserDefault::getInstance()->getBoolForKey("audioEnabled", true))
                AudioEngine::play2d("sounds/button.ogg", false);

            // Scene transition
            Director::getInstance()->replaceScene(CustomTransitionSlideInL::create(.4f, MenuScene::create()));
        });
    }

    return true;
}

void OverlayNode::showOverlay(eOverlayType overlayType) {
    setVisible(true);
    setOpacity(0);
    stopAllActions();
    runAction(ax::FadeIn::create(.2f));

    const auto image = NodeUtils::getNodeByName(mRoot, "image");
    const auto glow = dynamic_cast<ax::Sprite*>(NodeUtils::getNodeByName(mRoot, "glow"));
    const auto txtLabel = dynamic_cast<ax::ui::Text*>(NodeUtils::getNodeByName(mRoot, "labelMainTxt"));

    if (image && glow && txtLabel) {
        // Win overlay
        if (overlayType == eOverlayType::WIN) {
                AXLOG("Congratulations!");

                image->setColor(Color3B(ax::Color4B(190, 47, 78, 255)));
                glow->setColor(Color3B::RED);
                constexpr  ax::BlendFunc additive = {
                    ax::backend::BlendFactor::ONE,
                    ax::backend::BlendFactor::ONE
                };
                glow->setBlendFunc(additive);
                txtLabel->setString("CONGRATS!");
        } else {
            image->setColor(Color3B(ax::Color4B(40, 40, 45, 255)));
            glow->setColor(Color3B::WHITE);
            constexpr  ax::BlendFunc regular = {
                ax::backend::BlendFactor::ONE,
                ax::backend::BlendFactor::ONE_MINUS_SRC_ALPHA
            };
            glow->setBlendFunc(regular);
            txtLabel->setString("GAME OVER!");
        }
    }

    // Load timeline animation
    if (const auto timeline = CSLoader::createTimeline("widgets/overlayNode.csb")) {
        mRoot->runAction(timeline);
        timeline->play("gameOverFadeIn", false);
    }
    AXLOG("Timeline start.");
}

void OverlayNode::hideOverlay() {
    setVisible(false);
}
