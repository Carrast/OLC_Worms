#pragma once

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
    bool bStable = false; // has object stopped moving
    float fFriction = 0.8f;  // energy loss

    int nBounceBeforeDeath = -1;
    bool bDead = false;

    virtual void Draw(olcConsoleGameEngine* engine, float fOffsetX, float fOffsetY) = 0;

private:
};

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