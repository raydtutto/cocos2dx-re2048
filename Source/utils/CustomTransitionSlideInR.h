#ifndef COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINR_H
#define COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINR_H

#include "2d/Transition.h"


class CustomTransitionSlideInR : public ax::TransitionSlideInR {
public:
    static CustomTransitionSlideInR* create(float t, ax::Scene* scene);
    ax::ActionInterval* action() override;
};


#endif //COCOS2DX_RE2048_CUSTOMTRANSITIONSLIDEINR_H