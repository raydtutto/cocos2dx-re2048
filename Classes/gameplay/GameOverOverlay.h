#ifndef COCOS2DX_RE2048_GAMEOVEROVERLAY_H
#define COCOS2DX_RE2048_GAMEOVEROVERLAY_H


#include "cocos2d.h"

class GameOverOverlay : public cocos2d::Node {
public:
    static GameOverOverlay* create(std::function<void()> onRestart);
    bool initWithCallback(std::function<void()> onRestart);
    void showGameOver();
    void hideGameOver();
private:
    cocos2d::Node* mRoot = nullptr;
    bool mIsRestarting = false;
    std::function<void()> mOnRestart = nullptr;
};

#endif //COCOS2DX_RE2048_GAMEOVEROVERLAY_H