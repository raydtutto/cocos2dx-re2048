#ifndef COCOS2DX_RE2048_OVERLAYNODE_H
#define COCOS2DX_RE2048_OVERLAYNODE_H


#include "cocos2d.h"

enum class eOverlayType {
    GAME_OVER = 0, WIN
};

class OverlayNode : public cocos2d::Node {
public:
    static OverlayNode* create(const std::function<void()>& onRestart);
    bool initWithCallback(const std::function<void()>& onRestart);
    void showOverlay(eOverlayType overlayType);
    void hideOverlay();
private:
    cocos2d::Node* mRoot = nullptr;
    bool mIsRestarting = false;
    std::function<void()> mOnRestart = nullptr;
};

#endif //COCOS2DX_RE2048_OVERLAYNODE_H