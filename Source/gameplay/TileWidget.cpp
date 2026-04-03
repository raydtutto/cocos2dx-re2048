#include "gameplay/TileWidget.h"

using namespace ax;

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
    AX_SAFE_DELETE(pTile);
    return nullptr;
}

void TileWidget::setBoardPos(const std::pair<int, int>& pos, bool animate) {
    auto tileSize = getContentSize();
    auto nextPos = Vec2(
        static_cast<float>(pos.first) * tileSize.width + tileSize.width / 2,
        static_cast<float>(pos.second) * tileSize.height + tileSize.height / 2
    );
    if (animate) {
        const auto action = ax::MoveTo::create(getTimeDelay() / 2, nextPos);
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
    if (ax::FileUtils::getInstance()->isFileExist(path)) {
        loadTexture(path);
    } else {
        AXLOGERROR("Image path %s is invalid", path.c_str());
    }
    if (isChanged) {
        // Scale animation
        setScale(.9f);
        const auto scaleTo = ax::ScaleTo::create(.8f, 1.f);
        const auto scaleEaseOut = EaseElasticOut::create(scaleTo->clone());
        runAction(scaleEaseOut);
    }
}

void TileWidget::removeWidget() {
    auto scale = ax::ScaleBy::create(getTimeDelay()/4, 0.8f);
    auto fade = ax::FadeOut::create(getTimeDelay()/4);
    auto removeSelf = RemoveSelf::create(true);
    auto seq = Sequence::create(Spawn::create(scale, fade, nullptr), removeSelf, nullptr);
    runAction(seq);
}
