#pragma once

// -----------------------------------------------------------------------------
// Physics object
// -----------------------------------------------------------------------------
class cPhysicsObject
{
public:
    cPhysicsObject(float x = 0.0f, float y = 0.0f)
    {
        px = x;
        py = y;
    }

    float px = 0.0f;        // position
    float py = 0.0f;
    float vx = 0.0f;        // velocity
    float vy = 0.0f;
    float ax = 0.0f;        // accelleration
    float ay = 0.0f;

    float radius = 4.0f;    // circle for collision check
    bool bStable = false;   // has object stopped moving
    float fFriction = 0.8f; // energy loss

    int nBounceBeforeDeath = -1;
    bool bDead = false;

    virtual void Draw(olcConsoleGameEngine* engine, float fOffsetX, float fOffsetY) = 0;
    virtual int BounceDeathAction() = 0;

private:
};

// -----------------------------------------------------------------------------
// Dummy object
// -----------------------------------------------------------------------------
class cDummy : public cPhysicsObject
{
public:
    cDummy(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y)
    {

    }

    virtual void Draw(olcConsoleGameEngine* engine, float fOffsetX, float fOffsetY)
    {
        engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, atan2f(vx, vy), radius, FG_WHITE);
    }

    virtual int BounceDeathAction()
    {
        return 0; // do nothing just fade
    }

private:
    static std::vector<std::pair<float, float>> vecModel;
};

std::vector<std::pair<float, float>> DefineDummy()
{
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({ 0.0f, 0.0f });
    for (int i = 0; i < 10; i++)
    {
        vecModel.push_back({ cosf(i / 9.0f * 2.0f * 3.14159f), sinf(i / 9.0f * 2.0f * 3.14159f) });
    }
    return vecModel;
}

std::vector<std::pair<float, float>> cDummy::vecModel = DefineDummy();

// -----------------------------------------------------------------------------
// Debris
// -----------------------------------------------------------------------------
class cDebris : public cPhysicsObject
{
public:
    cDebris(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y)
    {
        vx = 10.0f * cosf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
        vy = 10.0f * sinf(((float)rand() / (float)RAND_MAX) * 2.0f * 3.14159f);
        radius = 1;
        fFriction = 0.8f;
        nBounceBeforeDeath = 5;
    }

    virtual void Draw(olcConsoleGameEngine* engine, float fOffsetX, float fOffsetY)
    {
        engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, atan2f(vx, vy), radius, FG_DARK_GREEN);
    }

    virtual int BounceDeathAction()
    {
        return 0; // do nothing just fade
    }

private:
    static std::vector<std::pair<float, float>> vecModel;
};

std::vector<std::pair<float, float>> DefineDebris()
{
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({0.0f, 0.0f});
    vecModel.push_back({0.0f, 1.0f});
    vecModel.push_back({1.0f, 1.0f});
    vecModel.push_back({1.0f, 0.0f});
    return vecModel;
}

std::vector<std::pair<float, float>> cDebris::vecModel = DefineDebris();

// -----------------------------------------------------------------------------
// Missile
// -----------------------------------------------------------------------------
class cMissile : public cPhysicsObject
{
public:
    cMissile(float x = 0.0f, float y = 0.0f, float _vx = 0.0f, float _vy = 0.0f) : cPhysicsObject(x, y)
    {
        radius = 2.5f;
        fFriction = 0.5f;
        vx = _vx;
        vy = _vy;
        bDead = false;
        nBounceBeforeDeath = 1;
    }

    virtual void Draw(olcConsoleGameEngine* engine, float fOffsetX, float fOffsetY)
    {
        engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, atan2f(vy, vx), radius, FG_YELLOW);
    }

    virtual int BounceDeathAction()
    {
        return 20; // big BOOM
    }

private:
    static std::vector<std::pair<float, float>> vecModel;
};

std::vector<std::pair<float, float>> DefineMissile()
{
    std::vector<std::pair<float, float>> vecModel;
    vecModel.push_back({ 0.0f, 0.0f });
    vecModel.push_back({ 1.0f, 1.0f });
    vecModel.push_back({ 2.0f, 1.0f });
    vecModel.push_back({ 2.5f, 0.0f });
    vecModel.push_back({ 2.0f, -1.0f });
    vecModel.push_back({ 1.0f, -1.0f });
    vecModel.push_back({ 0.0f, 0.0f });
    vecModel.push_back({ -1.0f, -1.0f });
    vecModel.push_back({ -2.5f, -1.0f });
    vecModel.push_back({ -2.0f, 0.0f });
    vecModel.push_back({ -2.5f, 1.0f });
    vecModel.push_back({ -1.0f, 1.0f });

    // Scale points to make shape unit sized
    for (auto& v : vecModel)
    {
        v.first /= 2.5f; v.second /= 2.5f;
    }
    return vecModel;
}

std::vector<std::pair<float, float>> cMissile::vecModel = DefineMissile();

class cWorm : public cPhysicsObject
{
public:
    cWorm(float x = 0.0f, float y = 0.0f) : cPhysicsObject(x, y)
    {
        radius = 3.5f;
        fFriction = 0.2f;
        bDead = false;
        nBounceBeforeDeath = -1;

        if (sprWorm == nullptr)
            sprWorm = new olcSprite(L"worms.spr");
    }

    virtual void Draw(olcConsoleGameEngine* engine, float fOffsetX, float fOffsetY)
    {
        engine->DrawPartialSprite(px - fOffsetX - radius, py - fOffsetY - radius, sprWorm, 0, 0, 8, 8);
    }

    virtual int BounceDeathAction()
    {
        return 0; // do nothing just fade
    }

public:
    float fShootAngle = 0.0f;

private:
    static olcSprite* sprWorm;
};

olcSprite* cWorm::sprWorm = nullptr;

