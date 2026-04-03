#ifndef COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINL_H
#define COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINL_H

#include "2d/Transition.h"


class CustomTransitionSlideInL : public ax::TransitionSlideInL {
public:
    static CustomTransitionSlideInL* create(float t, Scene* scene);
    ax::ActionInterval* action() override;
};


#endif //COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINL_H