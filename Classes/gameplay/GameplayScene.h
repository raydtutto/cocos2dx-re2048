#ifndef COCOS2DX_RE2048_GAMEPLAYSCENE_H
#define COCOS2DX_RE2048_GAMEPLAYSCENE_H


#include "cocos2d.h"

// forward
class TileWidget;
class GameOverOverlay;

enum class eDirection {
    UNDEFINED,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

using TileGrid = std::map<std::pair<int, int>, TileWidget*>;

// GameplayScene - Main gameplay logic, game board owner, handling touches and game logic
class GameplayScene : public cocos2d::Scene {
public:
    static GameplayScene *create();
    ~GameplayScene() override;
    bool init() override;
    void onExit() override;

private:
    void initListeners();
    void reinitBoard();
    std::pair<int, int> getRandomPos();
    void generateTile();
    void onMove(eDirection dir);
    std::pair<int, int> matchTileRow(std::vector<TileGrid::iterator>& buffer); // <1 moved cells, 2 max merged number>
    void updateScore(int num);
    bool checkUnsolvableBoard();
    static void playSound(int num);
    void gameOver();

    // Save loading state
    void saveBoard();
    void tryLoadBoard();
    void resetBoard();

public:
    void update(float delta) override;

    void onEnterTransitionDidFinish() override;

private:
    cocos2d::Node* mRoot{nullptr};
    cocos2d::Node* mBoard{nullptr};
    TileGrid mTileGrid;
    cocos2d::EventListenerTouchOneByOne* mTouchListener = nullptr;
    cocos2d::EventListenerKeyboard* mKeyboardListener = nullptr;
    cocos2d::Vec2 mInitTouchPos;
    float mMoveTimer{0.f};
    int mGameScore{0};
    bool mGameOver = false;
    GameOverOverlay* mGameOverOverlay{nullptr};
};


#endif //COCOS2DX_RE2048_GAMEPLAYSCENE_H