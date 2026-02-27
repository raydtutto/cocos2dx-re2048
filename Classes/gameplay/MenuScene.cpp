#include "MenuScene.h"

#include "GameplayScene.h"
#include "CustomButton.h"
#include "cocostudio/ActionTimeline/CSLoader.h"
#include "utils/NodeUtils.h"
#include "ui/UIText.h"

using namespace cocos2d;

bool MenuScene::init() {
    if (!Scene::init()) {
        return false;
    }

    // Background color
    const Color4B colorBg = Color4B(80, 80, 85, 255);
    const auto layerColor = LayerColor::create(colorBg);
    addChild(layerColor);

    auto root = CSLoader::createNodeWithVisibleSize("widgets/menuScene.csb");
    if (root) {
        addChild(root);
    } else {
        return false;
    }

    // Update user score
    if (auto bestScoreLabel = dynamic_cast<cocos2d::ui::Text*>(NodeUtils::getNodeByName(root, "bestScore"))) {
        int bestScore = cocos2d::UserDefault::getInstance()->getIntegerForKey("best_score", 0);
        bestScoreLabel->setString(std::to_string(bestScore));
    }

    if (auto btnMenuPlay = dynamic_cast<CustomButton*>(NodeUtils::getNodeByName(root, "playBtn"))) {
        btnMenuPlay->addClickEventListener([](Ref* eve) {
            CCLOG("Button clicked.");
            auto gameplayScene = GameplayScene::create();
            Director::getInstance()->replaceScene(gameplayScene);
        });
        CCLOG("Button found.");
        btnMenuPlay->setPressedActionEnabled(true);
    }

    return true;
}
