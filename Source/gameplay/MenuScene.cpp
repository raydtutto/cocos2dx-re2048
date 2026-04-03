#include "MenuScene.h"

#include "GameplayScene.h"
#include "CustomButton.h"
#include "utils/CustomTransitionSlideInR.h"
#include "cocostudio/ActionTimeline/ActionTimeline.h"
#include "cocostudio/ActionTimeline/CSLoader.h"
#include "utils/NodeUtils.h"
#include "ui/UIText.h"
#include "ui/UIListView.h"
#include "audio/AudioEngine.h"

using namespace ax;

void scaleAnimation(ax::Node* image);

MenuScene* MenuScene::create() {
    auto pRet = new(std::nothrow) MenuScene();
    if (pRet && pRet->init()) {
        pRet->autorelease();
        return pRet;
    } else {
        delete pRet;
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

    // Score reset
    // ax::UserDefault::getInstance()->setIntegerForKey("best_score", 0);

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
    if (const auto titleImg = dynamic_cast<ax::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "titleHolder"))) {
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
    if (const auto bestScoreHolder = dynamic_cast<ax::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "bestScoreHolder"))) {
        bestScoreHolder->setScrollBarEnabled(false);

        if (const auto bestScoreLabel = dynamic_cast<ax::ui::Text*>(NodeUtils::getNodeByName(mRoot, "bestScore"))) {
            const int bestScore = ax::UserDefault::getInstance()->getIntegerForKey("best_score", 0);
            bestScoreLabel->setString(std::to_string(bestScore));
        }
    }

    // Remove textLabel scrollbar
    if (const auto textHolder = dynamic_cast<ax::ui::ListView*>(NodeUtils::getNodeByName(mRoot, "textHolder"))) {
        textHolder->setScrollBarEnabled(false);
    }

    // Manage sound
    const auto audioEnabled = ax::UserDefault::getInstance()->getBoolForKey("audioEnabled", true);
    if (!audioEnabled) {
        AudioEngine::pauseAll();
    } else {
        AudioEngine::resumeAll();
    }

    // Mute all sounds
    if (const auto iconSoundHolder = dynamic_cast<ax::ui::Button *>(NodeUtils::getNodeByName(
        mRoot, "iconSoundHolder"))) {
        const auto iconSoundOnImage = dynamic_cast<ax::Sprite*>(NodeUtils::getNodeByName(mRoot, "imageSoundOn"));
        const auto iconSoundOffImage = dynamic_cast<ax::Sprite*>(NodeUtils::getNodeByName(mRoot, "imageSoundOff"));
        if (iconSoundOnImage && iconSoundOffImage) {
            iconSoundOnImage->setVisible(audioEnabled);
            iconSoundOffImage->setVisible(!audioEnabled);
        }
        iconSoundHolder->addTouchEventListener([this](Object* obj, ax::ui::Widget::TouchEventType type) {
            switch (type) {
                case ax::ui::Widget::TouchEventType::BEGAN:
                case ui::Widget::TouchEventType::MOVED:
                case ui::Widget::TouchEventType::CANCELED:
                    break;
                case ui::Widget::TouchEventType::ENDED: {
                    AXLOG("Mute clicked");
                    const auto iconSoundOnImg = dynamic_cast<ax::Sprite*>(NodeUtils::getNodeByName(mRoot, "imageSoundOn"));
                    const auto iconSoundOffImg = dynamic_cast<ax::Sprite*>(NodeUtils::getNodeByName(mRoot, "imageSoundOff"));
                    if (iconSoundOnImg && iconSoundOffImg) {
                        const auto audio = toggleAudio();
                        iconSoundOnImg->setVisible(audio);
                        iconSoundOffImg->setVisible(!audio);
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

void MenuScene::onEnterTransitionDidFinish() {
    Scene::onEnterTransitionDidFinish();
}

void MenuScene::initListeners() {
    // Start game on Enter
    if (mKeyboardListener) {
        Director::getInstance()->getEventDispatcher()->removeEventListener(mKeyboardListener);
    }
    mKeyboardListener = ax::EventListenerKeyboard::create();
    if (mKeyboardListener) {
        mKeyboardListener->onKeyReleased = [](const EventKeyboard::KeyCode keyCode, Event*) {
            if (keyCode == EventKeyboard::KeyCode::KEY_ENTER || keyCode == EventKeyboard::KeyCode::KEY_KP_ENTER) {
                AXLOG("Button clicked.");
                if (ax::UserDefault::getInstance()->getBoolForKey("audioEnabled", true)) {
                    AudioEngine::play2d("sounds/button.ogg", false);
                }
                Director::getInstance()->replaceScene(CustomTransitionSlideInR::create(.4f, GameplayScene::create()));
            }
        };
        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mKeyboardListener, this);
        AXLOG("Event keyboard listener created.");
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
            AXLOG("Start game");
            if (ax::UserDefault::getInstance()->getBoolForKey("audioEnabled", true)) {
                AudioEngine::play2d("sounds/button.ogg", false);
            }
            Director::getInstance()->replaceScene(CustomTransitionSlideInR::create(.4f, GameplayScene::create()));
        };

        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mTouchListener, this);
        AXLOG("Event touch listener created.");
    }
}

bool MenuScene::toggleAudio() {
    const auto audioEnabled = ax::UserDefault::getInstance()->getBoolForKey("audioEnabled", true);
    ax::UserDefault::getInstance()->setBoolForKey("audioEnabled", !audioEnabled);
    if (audioEnabled) {
        AudioEngine::pauseAll();
    } else {
        AudioEngine::resumeAll();
    }
    return !audioEnabled;
}

void scaleAnimation(ax::Node* image) {
    const auto bg = NodeUtils::getNodeByName(image, "bg");
    const auto num = NodeUtils::getNodeByName(image, "num");

    if (bg && num) {
        bg->setScale(.9f);
        num->setScale(.9f);

        const auto scaleTo = ax::ScaleTo::create(1.2f, 1.f);
        const auto scaleEaseOut = EaseElasticOut::create(scaleTo);

        bg->runAction(scaleEaseOut);
        num->runAction(scaleEaseOut->clone());
    }
}
