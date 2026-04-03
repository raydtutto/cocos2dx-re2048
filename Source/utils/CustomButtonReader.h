#ifndef COCOS2DX_RE2048_CUSTOMBUTTONREADER_H
#define COCOS2DX_RE2048_CUSTOMBUTTONREADER_H

#include "cocostudio/WidgetReader/ButtonReader/ButtonReader.h"

class CustomButtonReader : public cocostudio::ButtonReader {
    DECLARE_CLASS_NODE_READER_INFO
public:
    static ButtonReader* getInstance();
    ax::Node* createNodeWithFlatBuffers(const flatbuffers::Table* buttonOptions) override;

};


#endif //COCOS2DX_RE2048_CUSTOMBUTTONREADER_H