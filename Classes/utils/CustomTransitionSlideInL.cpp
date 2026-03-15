#include "CustomTransitionSlideInL.h"

#include "CCDirector.h"
#include "2d/CCActionEase.h"
#include "2d/CCActionInterval.h"

#define ADJUST_FACTOR 0.5f

CustomTransitionSlideInL* CustomTransitionSlideInL::create(float t, cocos2d::Scene *scene) {
    auto newScene = new (std::nothrow) CustomTransitionSlideInL();
    if(newScene && newScene->initWithDuration(t, scene))
    {
        newScene->autorelease();
        return newScene;
    }
    CC_SAFE_DELETE(newScene);
    return nullptr;
}

cocos2d::ActionInterval* CustomTransitionSlideInL::action() {
    const cocos2d::Size s = cocos2d::Director::getInstance()->getWinSize();

    const auto move = cocos2d::MoveBy::create(_duration, cocos2d::Vec2(s.width-ADJUST_FACTOR,0.0f));
    const auto ease = cocos2d::EaseQuinticActionInOut::create(move);

    return ease;
}
