#ifndef COCOS2DX_RE2048_CUSTOMBUTTON_H
#define COCOS2DX_RE2048_CUSTOMBUTTON_H

#include "cocos2d.h"
#include "ui/UIButton.h"

class CustomButton : public cocos2d::ui::Button {
public:
    static CustomButton *create() {
        auto* pRet = new(std::nothrow) CustomButton();
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            return nullptr;
        }
    }

protected:
    void onPressStateChangedToNormal() override;
    void onPressStateChangedToPressed() override;
};


#endif //COCOS2DX_RE2048_CUSTOMBUTTON_H