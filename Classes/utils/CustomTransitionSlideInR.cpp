#include "CustomTransitionSlideInR.h"

#include "CCDirector.h"
#include "2d/CCActionEase.h"
#include "2d/CCActionInterval.h"

#define ADJUST_FACTOR 0.5f

CustomTransitionSlideInR* CustomTransitionSlideInR::create(float t, Scene *scene) {
    auto newScene = new (std::nothrow) CustomTransitionSlideInR();
    if(newScene && newScene->initWithDuration(t, scene))
    {
        newScene->autorelease();
        return newScene;
    }
    CC_SAFE_DELETE(newScene);
    return nullptr;
}

cocos2d::ActionInterval* CustomTransitionSlideInR::action() {
    const cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();

    const auto move = cocos2d::MoveBy::create(_duration, cocos2d::Vec2(-(s.width-ADJUST_FACTOR),0.0f));
    const auto ease = cocos2d::EaseQuinticActionInOut::create(move);

    return ease;
}
