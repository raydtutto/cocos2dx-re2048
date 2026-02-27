#ifndef COCOS2DX_RE2048_CUSTOMBUTTON_H
#define COCOS2DX_RE2048_CUSTOMBUTTON_H

#include "cocos2d.h"
#include "ui/UIButton.h"

class CustomButton : public cocos2d::ui::Button {
    public:
    static CustomButton *create() {
        CustomButton *pRet = new(std::nothrow) CustomButton();
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = nullptr;
            return nullptr;
        }
    }
};


#endif //COCOS2DX_RE2048_CUSTOMBUTTON_H