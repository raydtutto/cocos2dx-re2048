#include "CustomTransitionSlideInL.h"

#include "Director.h"
#include "2d/ActionEase.h"
#include "2d/ActionInterval.h"

#define ADJUST_FACTOR 0.5f

CustomTransitionSlideInL* CustomTransitionSlideInL::create(float t, ax::Scene *scene) {
    auto newScene = new (std::nothrow) CustomTransitionSlideInL();
    if(newScene && newScene->initWithDuration(t, scene))
    {
        newScene->autorelease();
        return newScene;
    }
    AX_SAFE_DELETE(newScene);
    return nullptr;
}

ax::ActionInterval* CustomTransitionSlideInL::action() {
    const ax::Size s = ax::Director::getInstance()->getWinSize();

    const auto move = ax::MoveBy::create(_duration, ax::Vec2(s.width-ADJUST_FACTOR,0.0f));
    const auto ease = ax::EaseQuinticActionInOut::create(move);

    return ease;
}
