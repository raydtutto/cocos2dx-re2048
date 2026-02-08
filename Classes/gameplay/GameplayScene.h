#ifndef COCOS2DX_RE2048_GAMEPLAYSCENE_H
#define COCOS2DX_RE2048_GAMEPLAYSCENE_H


#include "cocos2d.h"

// forward
class TileWidget;

enum class eDirection {
    UNDEFINED,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

// GameplayScene - Main gameplay logic, game board owner, handling touches and game logic
class GameplayScene : public cocos2d::Scene {
public:
    // implement the "static create()" method manually
    static GameplayScene *create() {
        GameplayScene *pRet = new(std::nothrow) GameplayScene();
        if (pRet && pRet->init()) {
            pRet->autorelease();
            return pRet;
        } else {
            delete pRet;
            pRet = nullptr;
            return nullptr;
        }
    }
    bool init() override;

private:
    void initListeners();
    std::pair<int, int> getRandomPos();
    void generateTile();
    void onMove(eDirection dir);
    void matchTileRow(std::vector<TileWidget*> buffer, eDirection dir);

    cocos2d::Node* mRoot{nullptr};
    cocos2d::Node* mBoard{nullptr};
    std::map<std::pair<int, int>, TileWidget*> mTileGrid;
    cocos2d::EventListenerTouchOneByOne* mTouchListener = nullptr;
    cocos2d::Vec2 mInitTouchPos;

};


#endif //COCOS2DX_RE2048_GAMEPLAYSCENE_H