#include "MenuScene.h"

#include "GameplayScene.h"
#include "CustomButton.h"
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

    const auto root = CSLoader::createNodeWithVisibleSize("widgets/menuScene.csb");
    if (root) {
        addChild(root);
    } else {
        return false;
    }

    // Title animation
    if (const auto titleImg = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(root, "titleHolder"))) {
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
    if (const auto bestScoreHolder = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(root, "bestScoreHolder"))) {
        bestScoreHolder->setScrollBarEnabled(false);

        if (const auto bestScoreLabel = dynamic_cast<cocos2d::ui::Text*>(NodeUtils::getNodeByName(root, "bestScore"))) {
            const int bestScore = cocos2d::UserDefault::getInstance()->getIntegerForKey("best_score", 0);
            bestScoreLabel->setString(std::to_string(bestScore));
        }
    }

    // Remove textLabel scrollbar
    if (const auto textHolder = dynamic_cast<cocos2d::ui::ListView*>(NodeUtils::getNodeByName(root, "textHolder"))) {
        textHolder->setScrollBarEnabled(false);
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
                CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);
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
            CocosDenshion::SimpleAudioEngine::getInstance()->playEffect("sounds/button.ogg", false, 1.0f, 1.0f, 1.0f);
            Director::getInstance()->replaceScene(GameplayScene::create());
        };

        Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(mTouchListener, this);
        CCLOG("Event touch listener created.");
    }
}

void scaleAnimation(cocos2d::Node* image) {
    image->setScale(.9f);
    image->setAnchorPoint(Vec2(0.5, 0.5));
    const auto scaleTo = cocos2d::ScaleTo::create(1.f, 1.f); // Scale
    const auto scaleEaseOut = EaseElasticOut::create(scaleTo->clone());
    image->runAction(scaleEaseOut);
}
