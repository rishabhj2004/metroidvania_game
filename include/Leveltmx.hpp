#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <string>
#include <tmxlite/ImageLayer.hpp>
#include <tmxlite/LayerGroup.hpp>

struct TilesetData
{
    tmx::Tileset tileset;
    sf::Texture texture;
};

struct BackgroundData
{
    tmx::ImageLayer* layer;
    sf::Texture texture;
    sf::Vector2f startPosition;
};

struct CollisionRect
{
    sf::FloatRect bounds;
};

struct DamageRect
{
    sf::FloatRect bounds;
};

struct EnemySpawn
{
    std::string type;
    sf::Vector2f position;
};

struct LevelExit
{
    sf::FloatRect bounds;
    std::string nextLevel;
    std::string spawnPoint;
};

class Leveltmx
{
private:
    tmx::Map map;
    std::vector<TilesetData> tilesets;
    std::vector<CollisionRect> collisions;
    sf::Shader platformShader;
    sf::Texture noiseTexture;
    std::vector<DamageRect> damageTiles;
    std::vector<BackgroundData> backgrounds;


public:
    Leveltmx(const std::string& filename);
    void draw(sf::RenderWindow& window, float totalTime);
    void drawPlatforms(sf::RenderWindow& window, float totalTime);
    void drawForeground(sf::RenderWindow& window);

    float getWidth() const;
    float getHeight() const;
    sf::Vector2f getPlayerSpawn(const std::string& spawnPoint) const;
    const std::vector<CollisionRect>& getCollisions() const;
    std::vector<EnemySpawn> getEnemySpawns() const;
    std::vector<LevelExit> getLevelExits() const;
    const std::vector<DamageRect>& getDamageTiles() const;
    void drawBackground(
        sf::RenderWindow& window,
        const sf::Vector2f& cameraStart
    );
};
