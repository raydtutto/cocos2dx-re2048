#ifndef COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINL_H
#define COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINL_H

#include "2d/CCTransition.h"


class CustomTransitionSlideInL : public cocos2d::TransitionSlideInL {
public:
    static CustomTransitionSlideInL* create(float t, Scene* scene);
    cocos2d::ActionInterval* action() override;
};


#endif //COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINL_H