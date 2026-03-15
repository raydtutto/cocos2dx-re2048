#ifndef COCOS2DX_RE2048_MENUSCENE_H
#define COCOS2DX_RE2048_MENUSCENE_H


#include "cocos2d.h"

// MenuScene - Start menu
class MenuScene : public cocos2d::Scene {
public:
    static MenuScene* create();
    ~MenuScene() override;
    bool init() override;
    void onEnterTransitionDidFinish() override;
private:
    void initListeners();
    static bool toggleAudio();

    cocos2d::Node* mRoot{nullptr};
    cocos2d::EventListenerKeyboard* mKeyboardListener = nullptr;
    cocos2d::EventListenerTouchOneByOne* mTouchListener = nullptr;
};


#endif //COCOS2DX_RE2048_MENUSCENE_H