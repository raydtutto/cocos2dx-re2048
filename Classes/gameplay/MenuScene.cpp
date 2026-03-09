#include "MenuScene.h"

#include "GameplayScene.h"
#include "CustomButton.h"
#include "cocostudio/ActionTimeline/CCActionTimeline.h"
#include "cocostudio/ActionTimeline/CCActionTimelineCache.h"
#include "cocostudio/ActionTimeline/CSLoader.h"
#include "utils/NodeUtils.h"
#include "ui/UIText.h"
#include "ui/UIListView.h"
#include "editor-support/cocostudio/SimpleAudioEngine.h"

using namespace cocos2d;

void scaleAnimation(cocos2d::Node* image);

MenuScene* MenuScene::create() {
    auto pRet = new(std::nothrow) MenuScene();
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
        pRet = nullptr;
        return nullptr;
    }
}

MenuScene::~MenuScene() {
    // Destroy listeners
    if (mTouchListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(mTouchListener);
        mTouchListener = nullptr;
    }
    if (mKeyboardListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(mKeyboardListener);
        mKeyboardListener = nullptr;
    }
}

bool MenuScene::init() {
    if (!Scene::init()) {
        return false;
    }

    // Background color
    const auto colorBg = Color4B(40, 40, 45, 255);
    const auto layerColor = LayerColor::create(colorBg);
    addChild(layerColor);

    mRoot = CSLoader::createNodeWithVisibleSize("widgets/menuScene.csb");
    if (mRoot) {
        addChild(mRoot);
    } else {
        return false;
    }

    // Title rotation
    const auto titleRotationTimeline = CSLoader::createTimeline("widgets/menuScene.csb");
    mRoot->runAction(titleRotationTimeline);
    titleRotationTimeline->play("titleRotation", true);

    // Title animation
    if (const auto titleImg = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "titleHolder"))) {
        titleImg->setScrollBarEnabled(false);
        const std::vector<std::string> nodeNames = { "num2Holder", "num0Holder", "num4Holder", "num8Holder" };

        // Apply animation on tiles
        for (const auto& name : nodeNames) {
            if (const auto numHolder = titleImg->getChildByName(name))
                scaleAnimation(numHolder);
        }
    } else {
        return false;
    }

    // Update best user score
    if (const auto bestScoreHolder = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "bestScoreHolder"))) {
        bestScoreHolder->setScrollBarEnabled(false);

        if (const auto bestScoreLabel = dynamic_cast<cocos2d::ui::Text*>(NodeUtils::getNodeByName(mRoot, "bestScore"))) {
            const int bestScore = cocos2d::UserDefault::getInstance()->getIntegerForKey("best_score", 0);
            bestScoreLabel->setString(std::to_string(bestScore));
        }
    }

    // Remove textLabel scrollbar
    if (const auto textHolder = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "textHolder"))) {
        textHolder->setScrollBarEnabled(false);
    }

    // Manage sound
    const auto audioEnabled = cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true);
    auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
    if (!audioEnabled) {
        audio->pauseAllEffects();
    } else {
        audio->resumeAllEffects();
    }

    // Mute all sounds
    if (const auto iconSoundHolder = dynamic_cast<cocos2d::ui::Button *>(NodeUtils::getNodeByName(
        mRoot, "iconSoundHolder"))) {
        const auto iconSoundOnImage = dynamic_cast<cocos2d::Sprite*>(NodeUtils::getNodeByName(mRoot, "imageSoundOn"));
        const auto iconSoundOffImage = dynamic_cast<cocos2d::Sprite*>(NodeUtils::getNodeByName(mRoot, "imageSoundOff"));
        if (iconSoundOnImage && iconSoundOffImage) {
            iconSoundOnImage->setVisible(audioEnabled);
            iconSoundOffImage->setVisible(!audioEnabled);
        }
        iconSoundHolder->addTouchEventListener([&](Ref* obj, cocos2d::ui::Widget::TouchEventType type) {
            switch (type) {
                case cocos2d::ui::Widget::TouchEventType::BEGAN:
                case ui::Widget::TouchEventType::MOVED:
                case ui::Widget::TouchEventType::CANCELED:
                    break;
                case ui::Widget::TouchEventType::ENDED: {
                    CCLOG("Mute clicked");
                    const auto iconSoundOnImage = dynamic_cast<cocos2d::Sprite*>(NodeUtils::getNodeByName(mRoot, "imageSoundOn"));
                    const auto iconSoundOffImage = dynamic_cast<cocos2d::Sprite*>(NodeUtils::getNodeByName(mRoot, "imageSoundOff"));
                    if (iconSoundOnImage && iconSoundOffImage) {
                        const auto audio = toggleAudio();
                        iconSoundOnImage->setVisible(audio);
                        iconSoundOffImage->setVisible(!audio);
                    }
                } break;
                default:
                    break;
            }
        });
    }

    initListeners();

    return true;
}

void MenuScene::initListeners() {
    // Start game on Enter
    if (mKeyboardListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(mKeyboardListener);
    }
    mKeyboardListener = cocos2d::EventListenerKeyboard::create();
    if (mKeyboardListener) {
        mKeyboardListener->onKeyReleased = [](const EventKeyboard::KeyCode keyCode, Event*) {
            if (keyCode == EventKeyboard::KeyCode::KEY_ENTER || keyCode == EventKeyboard::KeyCode::KEY_KP_ENTER) {
                CCLOG("Button clicked.");
                auto audio = cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true);
                if (audio) {
                    CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);
                }
                auto gameplayScene = GameplayScene::create();
                Director::getInstance()->replaceScene(gameplayScene);
            }
        };
        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mKeyboardListener, this);
        CCLOG("Event keyboard listener created.");
    }

    // Touch listener
    if (mTouchListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(mTouchListener);
    }
    mTouchListener = EventListenerTouchOneByOne::create();
    if (mTouchListener) {
        // Push down trigger
        mTouchListener->onTouchBegan = [](Touch *touch, Event *event) {
            return true;
        };

        // Start game
        mTouchListener->onTouchEnded = [] (Touch *touch, Event *event) {
            CCLOG("Start game");
            auto audio = cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true);
            if (audio) {
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);
            }
            Director::getInstance()->replaceScene(GameplayScene::create());
        };

        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mTouchListener, this);
        CCLOG("Event touch listener created.");
    }
}

bool MenuScene::toggleAudio() {
    const auto audioEnabled = cocos2d::UserDefault::getInstance()->getBoolForKey("audioEnabled", true);
    cocos2d::UserDefault::getInstance()->setBoolForKey("audioEnabled", !audioEnabled);
    auto audio = CocosDenshion::SimpleAudioEngine::getInstance();
    if (audioEnabled) {
        audio->pauseAllEffects();
    } else {
        audio->resumeAllEffects();
    }
    return !audioEnabled;
}

void scaleAnimation(cocos2d::Node* image) {
    image->setScale(.9f);
    image->setAnchorPoint(Vec2(0.5, 0.5));
    const auto scaleTo = cocos2d::ScaleTo::create(1.f, 1.f); // Scale
    const auto scaleEaseOut = EaseElasticOut::create(scaleTo->clone());
    image->runAction(scaleEaseOut);
}
