#pragma once
#include <SFML/Graphics.hpp>
#include "Player.hpp"
#include <vector>
#include "assetManager.hpp"
#include "Leveltmx.hpp"
#include "SaveManager.hpp"
#include "EnemyManager.hpp"
#include "HealthBar.hpp"
#include <memory>

enum class TransitionState
{
    None,
    FadeOut,
    FadeIn
};

enum class DeathTransitionState
{
    None,
    FadeOut,
    FadeIn
};

class Game{
    private:
        sf::View camera;
        void processEvents();
        void update(float dt);
        void render();
        sf::RenderWindow window;
        Player player;
        EnemyManager enemyManager;
        float gravity;
        void resolveHorizontalCollisions();
        void resolveVerticalCollisions();
        void updateCamera(float dt);
        assetManager assets;
        sf::Sprite bgSprite;
        std::string currentLevel;
        std::unique_ptr<Leveltmx> level;
        SaveData saveData;
        void saveGame();
        sf::Clock absoluteClock;
        float cameraLookAhead;
        HealthBar healthBar;
        void checkLevelExit();
        void loadLevel(
            const std::string& levelName,
            const std::string& spawnPoint
        );

        TransitionState transitionState;

        sf::RectangleShape transitionOverlay;

        float transitionAlpha;

        std::string nextLevel;
        std::string nextSpawnPoint;
        void startLevelTransition(
            const std::string& levelName,
            const std::string& spawnPoint
        );
        void updateTransition(float dt);
        sf::Vector2f lastSafePosition;
        void checkDamageTiles();
        float damageStunTimer;

        DeathTransitionState deathTransitionState;
        float deathTransitionAlpha;
        void updateDeathTransition(float dt);

        float cameraShakeTimer;
        float cameraShakeDuration;
        float cameraShakeMagnitude;
        void updateCameraShake(float dt);
        void startCameraShake(float duration, float magnitude);


    public:
        Game();
        void run();
};
