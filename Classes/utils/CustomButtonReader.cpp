#include "CustomButtonReader.h"
#include "gameplay/CustomButton.h"

static CustomButtonReader* instanceButtonReader = nullptr;

// Implements CLASS_NODE_READER_INFO, using it instead of macro IMPLEMENT_CLASS_NODE_READER_INFO
cocos2d::Ref *CustomButtonReader::createInstance() { return CustomButtonReader::getInstance(); }
cocos2d::ObjectFactory::TInfo CustomButtonReader::__Type("ButtonReader", &CustomButtonReader::createInstance);

cocostudio::ButtonReader* CustomButtonReader::getInstance() {
    if (!instanceButtonReader)
    {
        instanceButtonReader = new (std::nothrow) CustomButtonReader();
    }
    return instanceButtonReader;
}

// Override createNodeWithFlatBuffers() from original Button createNodeWithFlatBuffers()
cocos2d::Node* CustomButtonReader::createNodeWithFlatBuffers(const flatbuffers::Table* buttonOptions) {
    // Replace Button with CustomButton
    CustomButton* button = CustomButton::create();

    setPropsWithFlatBuffers(button, buttonOptions);

    return button;
}