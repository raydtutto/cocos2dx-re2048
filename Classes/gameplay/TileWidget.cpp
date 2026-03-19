#include "gameplay/TileWidget.h"

using namespace cocos2d;

float TileWidget::getTimeDelay() {
    return .5f;
}

TileWidget *TileWidget::create(int num) {
    auto pTile = new TileWidget();
    if (pTile && pTile->init()) {
        pTile->loadTexture("GUI/cocosstudio/img/tile2.png");
        pTile->autorelease();
        pTile->setNumber(num);
        return pTile;
    }
    CC_SAFE_DELETE(pTile);
    return nullptr;
}

void TileWidget::setBoardPos(const std::pair<int, int>& pos, bool animate) {
    auto tileSize = getContentSize();
    auto nextPos = Vec2(
        static_cast<float>(pos.first) * tileSize.width + tileSize.width / 2,
        static_cast<float>(pos.second) * tileSize.height + tileSize.height / 2
    );
    if (animate) {
        const auto action = cocos2d::MoveTo::create(getTimeDelay() / 2, nextPos);
        const auto ease = EaseQuadraticActionInOut::create(action);
        runAction(ease);
    } else {
        setPosition(nextPos);
    }

    _pos = pos;
}

void TileWidget::setNumber(int num) {
    bool isChanged = _num != num;
    _num = num;

    // Update image path
    std::string path = "GUI/cocosstudio/img/tile" + std::to_string(num) + ".png";
    if (cocos2d::FileUtils::getInstance()->isFileExist(path)) {
        loadTexture(path);
    } else {
        CCLOGERROR("Image path %s is invalid", path.c_str());
    }
    if (isChanged) {
        // Scale animation
        setScale(.9f);
        const auto scaleTo = cocos2d::ScaleTo::create(.8f, 1.f);
        const auto scaleEaseOut = EaseElasticOut::create(scaleTo->clone());
        runAction(scaleEaseOut);
    }
}

void TileWidget::removeWidget() {
    // removeFromParentAndCleanup(true);
    auto scale = cocos2d::ScaleBy::create(getTimeDelay()/4, 0.8f);
    auto fade = cocos2d::FadeOut::create(getTimeDelay()/4);
    auto removeSelf = RemoveSelf::create(true);
    auto seq = Sequence::create(Spawn::create(scale, fade, nullptr), removeSelf, nullptr);
    runAction(seq);
}
