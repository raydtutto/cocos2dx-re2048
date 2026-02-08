#ifndef COCOS2DX_RE2048_TILEWIDGET_H
#define COCOS2DX_RE2048_TILEWIDGET_H


#include "cocos2d.h"
#include "ui/UIImageView.h"

#include <string>

// TileWidget - Represents each tile, max amount 4x4. Can be created by GameplayScene
class TileWidget : public cocos2d::ui::ImageView {
public:
    static TileWidget* create(int num = 0, const std::string& info = "");
    void setBoardPos(std::pair<int, int> pos);
};


#endif //COCOS2DX_RE2048_TILEWIDGET_H