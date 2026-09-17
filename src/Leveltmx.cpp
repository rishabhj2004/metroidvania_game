#include "Leveltmx.hpp"
#include <iostream>
#include <tmxlite/TileLayer.hpp>

Leveltmx::Leveltmx(const std::string& filename)
{
    if (!map.load(filename))
    {
        std::cerr << "Failed to load TMX map: "
                  << filename << '\n';
        return;
    }

    std::cout << "Loaded TMX map successfully!\n";

    std::cout << "Map size: "
              << map.getTileCount().x
              << " x "
              << map.getTileCount().y
              << '\n';

    std::cout << "Tile size: "
              << map.getTileSize().x
              << " x "
              << map.getTileSize().y
              << '\n';

    for (const auto& layer : map.getLayers())
    {
        std::cout << "Layer: "
                  << layer->getName()
                  << '\n';
    }

    for (const auto& ts : map.getTilesets())
    {
        std::cout << "\nTileset: "
                  << ts.getName()
                  << '\n';

        std::cout << "Tile size: "
                  << ts.getTileSize().x
                  << " x "
                  << ts.getTileSize().y
                  << '\n';

        std::cout << "Tile count: "
                  << ts.getTileCount()
                  << '\n';

        std::cout << "Columns: "
                  << ts.getColumnCount()
                  << '\n';

        std::cout << "Image: "
                  << ts.getImagePath()
                  << '\n';

        std::cout << "First GID: "
                  << ts.getFirstGID()
                  << '\n';


        TilesetData data;
        data.tileset = ts;

        if (!data.texture.loadFromFile(ts.getImagePath()))
        {
            std::cerr << "Failed to load tileset texture: "
                      << ts.getImagePath()
                      << '\n';

            continue;
        }

        tilesets.push_back(std::move(data));

            for (const auto& layer : map.getLayers())
            {
                if (layer->getType() != tmx::Layer::Type::Tile)
                    continue;

                if (layer->getName() != "Ground")
                    continue;

                const auto& tileLayer =
                    layer->getLayerAs<tmx::TileLayer>();

                const auto& tiles = tileLayer.getTiles();

                const unsigned int mapWidth = map.getTileCount().x;
                const unsigned int tileWidth = map.getTileSize().x;
                const unsigned int tileHeight = map.getTileSize().y;

                for (std::size_t i = 0; i < tiles.size(); ++i)
                {
                    if (tiles[i].ID == 0)
                        continue;

                    unsigned int x = i % mapWidth;
                    unsigned int y = i / mapWidth;

                    CollisionRect collision;

                    collision.bounds = sf::FloatRect(
                        x * tileWidth,
                        y * tileHeight,
                        tileWidth,
                        tileHeight
                    );

                    collisions.push_back(collision);
                }
            }

        std::cout << "Collision tiles: "
          << collisions.size()
          << '\n';

            for (const auto& layer : map.getLayers())
            {
                if (layer->getType() != tmx::Layer::Type::Tile)
                    continue;

                if (layer->getName() != "DamageTiles")
                    continue;

                const auto& tileLayer =
                    layer->getLayerAs<tmx::TileLayer>();

                const auto& tiles = tileLayer.getTiles();

                const unsigned int mapWidth = map.getTileCount().x;
                const unsigned int tileWidth = map.getTileSize().x;
                const unsigned int tileHeight = map.getTileSize().y;

                for (std::size_t i = 0; i < tiles.size(); ++i)
                {
                    if (tiles[i].ID == 0)
                        continue;

                    unsigned int x = i % mapWidth;
                    unsigned int y = i / mapWidth;

                    DamageRect damage;

                    damage.bounds = sf::FloatRect(
                        x * tileWidth,
                        y * tileHeight,
                        tileWidth,
                        tileHeight
                    );

                    damageTiles.push_back(damage);
                }
            }

            std::cout << "Damage tiles: "
                      << damageTiles.size()
                      << '\n';
    }
    if (sf::Shader::isAvailable())
    {
        platformShader.loadFromFile("assets/shaders/wind.frag", sf::Shader::Fragment);
    
        if (noiseTexture.loadFromFile("assets/textures/noise.png"))
        {
            noiseTexture.setRepeated(true);
            noiseTexture.setSmooth(true);
        }
    }
}

void Leveltmx::draw(sf::RenderWindow& window)
{
    const unsigned int tileWidth = map.getTileSize().x;
    const unsigned int tileHeight = map.getTileSize().y;
    const unsigned int mapWidth = map.getTileCount().x;

    for (const auto& layer : map.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Tile)
            continue;
        if(layer->getName() == "Platforms" || layer->getName() == "Ground"|| layer->getName() == "DamageTiles"|| layer->getName() == "Foreground")
            continue;
        const auto& tileLayer =
            layer->getLayerAs<tmx::TileLayer>();

        const auto& tiles = tileLayer.getTiles();

        for (std::size_t i = 0; i < tiles.size(); ++i)
        {
            unsigned int gid = tiles[i].ID;

            if (gid == 0)
                continue;

            const TilesetData* tilesetData = nullptr;

            for (const auto& data : tilesets)
            {
                if (gid >= data.tileset.getFirstGID())
                {
                    tilesetData = &data;
                }
            }

            if (tilesetData == nullptr)
                continue;

            const auto& tileset = tilesetData->tileset;

            unsigned int tileIndex =
                gid - tileset.getFirstGID();

            unsigned int columns =
                tileset.getColumnCount();

            unsigned int column =
                tileIndex % columns;

            unsigned int row =
                tileIndex / columns;

            sf::Sprite sprite;

            sprite.setTexture(tilesetData->texture);

            sprite.setTextureRect(
                sf::IntRect(
                    column * tileWidth,
                    row * tileHeight,
                    tileWidth,
                    tileHeight
                )
            );

            unsigned int x = i % mapWidth;
            unsigned int y = i / mapWidth;

            sprite.setPosition(
                x * tileWidth,
                y * tileHeight
            );

            window.draw(sprite);
        }
    }
}

void Leveltmx::drawForeground(sf::RenderWindow& window)
{
    const unsigned int tileWidth = map.getTileSize().x;
    const unsigned int tileHeight = map.getTileSize().y;
    const unsigned int mapWidth = map.getTileCount().x;

    for (const auto& layer : map.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Tile)
            continue;

        if (layer->getName() != "Foreground")
            continue;

        const auto& tileLayer =
            layer->getLayerAs<tmx::TileLayer>();

        const auto& tiles = tileLayer.getTiles();

        for (std::size_t i = 0; i < tiles.size(); ++i)
        {
            unsigned int gid = tiles[i].ID;

            if (gid == 0)
                continue;

            const TilesetData* tilesetData = nullptr;

            for (const auto& data : tilesets)
            {
                if (gid >= data.tileset.getFirstGID())
                {
                    tilesetData = &data;
                }
            }

            if (tilesetData == nullptr)
                continue;

            const auto& tileset = tilesetData->tileset;

            unsigned int tileIndex =
                gid - tileset.getFirstGID();

            unsigned int columns =
                tileset.getColumnCount();

            unsigned int column =
                tileIndex % columns;

            unsigned int row =
                tileIndex / columns;

            sf::Sprite sprite;

            sprite.setTexture(tilesetData->texture);

            sprite.setTextureRect(
                sf::IntRect(
                    column * tileWidth,
                    row * tileHeight,
                    tileWidth,
                    tileHeight
                )
            );

            unsigned int x = i % mapWidth;
            unsigned int y = i / mapWidth;

            sprite.setPosition(
                x * tileWidth,
                y * tileHeight
            );

            window.draw(sprite);
        }
    }
}

void Leveltmx::drawPlatforms(sf::RenderWindow& window, float totalTime)
{
    if (sf::Shader::isAvailable())
    {
        platformShader.setUniform("time", totalTime);
        platformShader.setUniform("texture", sf::Shader::CurrentTexture);
        platformShader.setUniform("noiseTex", noiseTexture);
    }

    const unsigned int tileW = map.getTileSize().x;
    const unsigned int tileH = map.getTileSize().y;
    const unsigned int mapW = map.getTileCount().x;
    const unsigned int mapH = map.getTileCount().y;

    const tmx::TileLayer* groundLayer = nullptr;
    for (const auto& layer : map.getLayers())
    {
        if (layer->getType() == tmx::Layer::Type::Tile && layer->getName() == "Ground")
        {
            groundLayer = &layer->getLayerAs<tmx::TileLayer>();
            break;
        }
    }
    for (const auto& layer : map.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Tile || layer->getName() != "Platforms") continue;
        
        const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
        const auto& platformTiles = tileLayer.getTiles();
        
        const auto& skeletonTiles = groundLayer ? groundLayer->getTiles() : platformTiles;

        for (std::size_t i = 0; i < platformTiles.size(); ++i)
        {
            unsigned int gid = platformTiles[i].ID;
            if (gid == 0) continue;

            unsigned int x = i % mapW;
            unsigned int y = i / mapW;
            if (groundLayer && skeletonTiles[y * mapW + x].ID != 0)
            {
                const TilesetData* tsData = nullptr;
                for (const auto& data : tilesets) {
                    if (gid >= data.tileset.getFirstGID()) tsData = &data;
                }
                if (!tsData) continue;

                unsigned int idx = gid - tsData->tileset.getFirstGID();
                unsigned int cols = tsData->tileset.getColumnCount();

                sf::Sprite sprite;
                sprite.setTexture(tsData->texture);
                sprite.setTextureRect(sf::IntRect((idx % cols) * tileW, (idx / cols) * tileH, tileW, tileH));
                sprite.setPosition(x * tileW, y * tileH);

                window.draw(sprite);
                continue;
            }

            //Border Detection
            bool airTop    = (y == 0 || skeletonTiles[(y - 1) * mapW + x].ID == 0);
            bool airBottom = (y == mapH - 1 || skeletonTiles[(y + 1) * mapW + x].ID == 0);
            bool airLeft   = (x == 0 || skeletonTiles[y * mapW + (x - 1)].ID == 0);
            bool airRight  = (x == mapW - 1 || skeletonTiles[y * mapW + (x + 1)].ID == 0);

            bool isLeafTile = (airTop || airBottom || airLeft || airRight);

            const TilesetData* tsData = nullptr;
            for (const auto& data : tilesets) {
                if (gid >= data.tileset.getFirstGID()) tsData = &data;
            }
            if (!tsData) continue;

            unsigned int idx = gid - tsData->tileset.getFirstGID();
            unsigned int cols = tsData->tileset.getColumnCount();
            
            sf::Sprite sprite;
            sprite.setTexture(tsData->texture);
            sprite.setTextureRect(sf::IntRect((idx % cols) * tileW, (idx / cols) * tileH, tileW, tileH));
            sprite.setPosition(x * tileW, y * tileH);

            //Drawing Logic 
            if (isLeafTile && sf::Shader::isAvailable())
            {
                platformShader.setUniform("tilePos", sprite.getPosition());
                platformShader.setUniform("borders", sf::Glsl::Vec4(
                    airTop ? 1.0f : 0.0f,
                    airBottom ? 1.0f : 0.0f,
                    airLeft ? 1.0f : 0.0f,
                    airRight ? 1.0f : 0.0f
                ));

                sf::Vector2u tSize = tsData->texture.getSize();
                float rawLeft   = (float)((idx % cols) * tileW) / tSize.x;
                float rawTop    = (float)((idx / cols) * tileH) / tSize.y;
                float rawRight  = rawLeft + ((float)tileW / tSize.x);
                float rawBottom = rawTop + ((float)tileH / tSize.y);
                float halfU = 0.5f / tSize.x;
                float halfV = 0.5f / tSize.y;
                platformShader.setUniform("uvBounds", sf::Glsl::Vec4(
                    rawLeft   + halfU, 
                    rawTop    + halfV, 
                    rawRight  - halfU, 
                    rawBottom - halfV
                ));
                
                window.draw(sprite, &platformShader);
            }
            else
            {
                window.draw(sprite); 
            }
        }
    }
}


float Leveltmx::getWidth() const
{
    return map.getTileCount().x * map.getTileSize().x;
}

float Leveltmx::getHeight() const
{
    return map.getTileCount().y * map.getTileSize().y;
}

sf::Vector2f Leveltmx::getPlayerSpawn(
    const std::string& spawnPoint
) const
{
    for(const auto& layer : map.getLayers())
    {
        if(layer->getType() != tmx::Layer::Type::Object)
            continue;

        const auto& objectLayer =
            layer->getLayerAs<tmx::ObjectGroup>();

        for(const auto& object : objectLayer.getObjects())
        {
            if(object.getName() != "PlayerSpawn")
                continue;

            for(const auto& property : object.getProperties())
            {
                if(property.getName() == "spawnPoint")
                {
                    if(property.getStringValue() == spawnPoint)
                    {
                        auto bounds = object.getAABB();

                        return sf::Vector2f(
                            bounds.left,
                            bounds.top
                        );
                    }
                }
            }
        }
    }

    return sf::Vector2f(100.f, 100.f);
}

const std::vector<CollisionRect>& Leveltmx::getCollisions() const
{
    return collisions;
}

const std::vector<DamageRect>& Leveltmx::getDamageTiles() const
{
    return damageTiles;
}

std::vector<EnemySpawn> Leveltmx::getEnemySpawns() const
{
    std::vector<EnemySpawn> enemySpawns;

    for (const auto& layer : map.getLayers())
    {
        if (layer->getType() != tmx::Layer::Type::Object)
            continue;

        if (layer->getName() != "Enemies")
            continue;
        const auto& objectLayer =
            layer->getLayerAs<tmx::ObjectGroup>();

        for (const auto& object : objectLayer.getObjects())
        {
            EnemySpawn spawn;

            spawn.type = object.getName();

            spawn.position = sf::Vector2f(
                object.getPosition().x,
                object.getPosition().y
            );

            enemySpawns.push_back(spawn);
        }
    }

    return enemySpawns;
}

std::vector<LevelExit> Leveltmx::getLevelExits() const
{
    std::vector<LevelExit> exits;

    for(const auto& layer : map.getLayers())
    {
        if(layer->getType() != tmx::Layer::Type::Object)
            continue;

        const auto& objectLayer =
            layer->getLayerAs<tmx::ObjectGroup>();

        for(const auto& object : objectLayer.getObjects())
        {
            if(object.getName() != "LevelExit")
                continue;

            LevelExit exit;

            auto bounds = object.getAABB();

            exit.bounds = sf::FloatRect(
                bounds.left,
                bounds.top,
                bounds.width,
                bounds.height
            );

            for(const auto& property : object.getProperties())
            {
                if(property.getName() == "nextLevel")
                {
                    exit.nextLevel =
                        property.getStringValue();
                }

                if(property.getName() == "spawnPoint")
                {
                    exit.spawnPoint =
                        property.getStringValue();
                }
            }

            exits.push_back(exit);
        }
    }

    return exits;
}
