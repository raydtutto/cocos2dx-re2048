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
        auto action = cocos2d::MoveTo::create(getTimeDelay() / 2, nextPos);
        runAction(action);
    } else {
        setPosition(nextPos);
    }

    _pos = pos;
}

void TileWidget::setNumber(int num) {
    _num = num;
    std::string path = "GUI/cocosstudio/img/tile" + std::to_string(num) + ".png";
    if (cocos2d::FileUtils::getInstance()->isFileExist(path)) {
        loadTexture(path);
    } else {
        CCLOGERROR("Image path %s is invalid", path.c_str());
    }
}
