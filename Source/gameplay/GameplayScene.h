#ifndef COCOS2DX_RE2048_GAMEPLAYSCENE_H
#define COCOS2DX_RE2048_GAMEPLAYSCENE_H


#include "axmol.h"

// forward
class TileWidget;
class OverlayNode;

enum class eDirection {
    UNDEFINED,
    UP,
    DOWN,
    LEFT,
    RIGHT
};

using TileGrid = std::map<std::pair<int, int>, TileWidget*>;

// GameplayScene - Main gameplay logic, game board owner, handling touches and game logic
class GameplayScene : public ax::Scene {
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
    void animateMoveFail(const eDirection dir) const;

    // Save loading state
    void saveBoard();
    void tryLoadBoard();
    void resetBoard();

public:
    void update(float delta) override;
    void onEnterTransitionDidFinish() override;

private:
    ax::Node* mRoot{nullptr};
    ax::Node* mBoard{nullptr};
    TileGrid mTileGrid;
    ax::EventListenerTouchOneByOne* mTouchListener = nullptr;
    ax::EventListenerKeyboard* mKeyboardListener = nullptr;
    ax::Vec2 mInitTouchPos;
    float mMoveTimer{0.f};
    int mGameScore{0};
    bool mGameOver = false;
    bool mUserWon = false;
    OverlayNode* mOverlayNode{nullptr};
};


#endif //COCOS2DX_RE2048_GAMEPLAYSCENE_H