#include "CustomButton.h"

using namespace cocos2d;

void CustomButton::onPressStateChangedToNormal() {
    this->stopActionByTag(1);

    const auto scale = ScaleTo::create(0.2f, 1.0f);
    const auto scaleEase = EaseBackOut::create(scale);
    scaleEase->setTag(1);

    this->runAction(scaleEase);
}

void CustomButton::onPressStateChangedToPressed() {
    this->stopActionByTag(2);

    const auto scale = ScaleTo::create(0.1f, 0.9f);
    const auto scaleEase = EaseSineOut::create(scale);
    scaleEase->setTag(2);

    this->runAction(scaleEase);
}