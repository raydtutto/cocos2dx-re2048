#ifndef COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINR_H
#define COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINR_H

#include "2d/CCTransition.h"


class CustomTransitionSlideInR : public cocos2d::TransitionSlideInR {
public:
    static CustomTransitionSlideInR* create(float t, cocos2d::Scene* scene);
    cocos2d::ActionInterval* action() override;
};


#endif //COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINR_H