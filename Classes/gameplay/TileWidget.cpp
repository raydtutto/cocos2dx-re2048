#include "gameplay/TileWidget.h"

using namespace cocos2d;

TileWidget *TileWidget::create(int num, const std::string &info) {
    auto pTile = new TileWidget();
    if (pTile && pTile->init()) {
        pTile->loadTexture("GUI/cocosstudio/img/tile2.png");
        pTile->autorelease();
        return pTile;
    }
    CC_SAFE_DELETE(pTile);
    return nullptr;
}

void TileWidget::setBoardPos(const std::pair<int, int>& pos) {
    auto tileSize = getContentSize();
    setPosition(Vec2(
        static_cast<float>(pos.first) * tileSize.width + tileSize.width / 2,
        static_cast<float>(pos.second) * tileSize.height + tileSize.height / 2
    ));
    _pos = pos;
}
