#ifndef COCOS2DX_RE2048_MENUSCENE_H
#define COCOS2DX_RE2048_MENUSCENE_H


#include "axmol.h"

// MenuScene - Start menu
class MenuScene : public ax::Scene {
public:
    static MenuScene* create();
    ~MenuScene() override;
    bool init() override;
    void onEnterTransitionDidFinish() override;
private:
    void initListeners();
    static bool toggleAudio();

    ax::Node* mRoot{nullptr};
    ax::EventListenerKeyboard* mKeyboardListener = nullptr;
    ax::EventListenerTouchOneByOne* mTouchListener = nullptr;
};


#endif //COCOS2DX_RE2048_MENUSCENE_H