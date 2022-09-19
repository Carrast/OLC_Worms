// OLC_Worms.cpp
// re written by Carrast for educational purpose
// 2022.09.09.
// thanks javidx9!

#include <iostream>
#include <string>
#include <algorithm>

#include "olcConsoleGameEngine.h"

class cPhysicsObject
{
public:
    cPhysicsObject(float x = 0.0f, float y = 0.0f)
    {
        px = x;
        py = y;
    }

    float px = 00.f;        // position
    float py = 00.f;
    float vx = 00.f;        // velocity
    float vy = 00.f;
    float ax = 00.f;        // accelleration
    float ay = 00.f;

    float radius = 4.0f;     // circle for collision check
    bool bIsStable = false;  // has object stopped moving

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
        engine->DrawWireFrameModel(vecModel, px - fOffsetX, py - fOffsetY, atan2(vx, vy), radius, FG_WHITE);
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

class Worms : public olcConsoleGameEngine
{
public:
    Worms();
    ~Worms() {}

private:
    int nMapWidth = 1024;
    int nMapHeight = 512;
    unsigned char* map = nullptr;

    float fCameraPosX = 0.0f;
    float fCameraPosY = 0.0f;

    std::list<cPhysicsObject*> listObjects;

protected:
    // Called by olcConsoleGameEngine
    virtual bool OnUserCreate()
    {
        // create 2d map
        map = new unsigned char[nMapWidth * nMapHeight];
        memset(map, 0, nMapWidth * nMapHeight * sizeof(unsigned char));
        CreateMap();

        
        return true;
    }

    // Called by olcConsoleGameEngine
    virtual bool OnUserUpdate(float fElapsedTime)
    {
        if (m_keys[L'M'].bReleased)
            CreateMap();
        
        if (m_mouse[1].bReleased)
        {
            cDummy* p = new cDummy(m_mousePosX + fCameraPosX, m_mousePosY + fCameraPosY);
            listObjects.push_back(p);
        }

        //  mouse map scroll
        float fMapScrollSpeed = 400.0f;
        if (m_mousePosX < 5) fCameraPosX -= fMapScrollSpeed * fElapsedTime;
        if (m_mousePosX > ScreenWidth() - 5) fCameraPosX += fMapScrollSpeed * fElapsedTime;
        if (m_mousePosY < 5) fCameraPosY -= fMapScrollSpeed * fElapsedTime;
        if (m_mousePosY > ScreenHeight() - 5) fCameraPosY += fMapScrollSpeed * fElapsedTime;

        // limit camera boundaries
        if (fCameraPosX < 0) fCameraPosX = 0;
        if (fCameraPosX >= nMapWidth - ScreenWidth()) fCameraPosX = nMapWidth;
        if (fCameraPosY < 0) fCameraPosY = 0;
        if (fCameraPosY >= nMapHeight - ScreenHeight()) fCameraPosY = nMapHeight;
        
        // iterate thru object 10 times a frame
        for (int z = 0; z < 10; z++)
        {
            // update pfhisycs of all objects
            for (auto& o : listObjects)
            {
                // apply gravity
                o->ay += 2.0f;

                // update velocity
                o->vx += o->ax * fElapsedTime;
                o->vy += o->ay * fElapsedTime;

                // update position
                float fPotentialX = o->px + o->vx * fElapsedTime;
                float fPotentialY = o->py + o->vy * fElapsedTime;

                // reset accelleration
                o->ax = 0;
                o->ay = 0;
                o->bIsStable = false;

                o->px = fPotentialX;
                o->py = fPotentialY;
            }
        }

        // remove objects from gamespace
        //for (auto& o : listObjects)
        //{
        //    auto i = std::remove_if(listObjects.begin(), listObjects.end(), [&](cPhysicsObject p) {return (p.py >= nMapHeight); });
        //    if (i != listObjects.end())
        //    {
        //        listObjects.erase(i);
        //    }
        //}

        // draw landscape
        for (int x = 0; x < ScreenWidth(); x++)
        {
            for (int y = 0; y < ScreenHeight(); y++)
            {
                switch (map[(y + (int)fCameraPosY) * nMapWidth + (x + (int)fCameraPosX)])
                {
                case 0:
                    Draw(x, y, PIXEL_SOLID, FG_CYAN);
                    break;
                case 1:
                    Draw(x, y, PIXEL_SOLID, FG_DARK_GREEN);
                    break;
                default:
                    break;
                }
            }
        }

        // draw objects
        for (auto& o : listObjects)
            o->Draw(this, fCameraPosX, fCameraPosY);

        // monitor quantity of objects
        DrawString(fCameraPosX + 2, fCameraPosY + 2, L"Number of objects: " + std::to_wstring(listObjects.size()));

        return true;
    }

// --- SPECIAL FUNCTIONS ---
protected:
    void CreateMap()
    {
        // used 1d perlin noise
        float* fSurface = new float[nMapWidth];
        float* fNoiseSpeed = new float[nMapWidth];

        for (int i = 0; i < nMapWidth; i++)
            fNoiseSpeed[i] = (float)std::rand() / (float)RAND_MAX;

        fNoiseSpeed[0] = 0.5f;
        PerlinNoise1D(nMapWidth, fNoiseSpeed, 8, 2.0f, fSurface);

        for (int x = 0; x < nMapWidth; x++)
        {
            for (int y = 0; y < nMapHeight; y++)
            {
                if (y >= fSurface[x] * nMapHeight)
                    map[y * nMapWidth + x] = 1;
                else
                    map[y * nMapWidth + x] = 0;
            }
        }

        delete[] fSurface;
        delete[] fNoiseSpeed;
    }

    // Taken from Perlin Noise Video https://youtu.be/6-0UaeJBumA
    void PerlinNoise1D(int nCount, float* fSeed, int nOctaves, float fBias, float* fOutput)
    {
        // Used 1D Perlin Noise
        for (int x = 0; x < nCount; x++)
        {
            float fNoise = 0.0f;
            float fScaleAcc = 0.0f;
            float fScale = 1.0f;

            for (int o = 0; o < nOctaves; o++)
            {
                int nPitch = nCount >> o;
                int nSample1 = (x / nPitch) * nPitch;
                int nSample2 = (nSample1 + nPitch) % nCount;
                float fBlend = (float)(x - nSample1) / (float)nPitch;
                float fSample = (1.0f - fBlend) * fSeed[nSample1] + fBlend * fSeed[nSample2];
                fScaleAcc += fScale;
                fNoise += fSample * fScale;
                fScale = fScale / fBias;
            }

            // Scale to seed range
            fOutput[x] = fNoise / fScaleAcc;
        }
    }

    // demonic Quake fast inverse square root
    float Q_rsqrt(float number)
    {
        long i;
        float x2, y;
        const float threehalfs = 1.5f;

        x2 = number * 0.5f;
        y = number;
        i = *(long*)&y;                             // evil floating point bit hack
        i = 0x5f3759df - (i >> 1);                  // what the fuck?
        y = *(float*)&i;
        y = y * (threehalfs - (x2 * y * y));        // 1st iteration
        //y = y * (threehalfs - (x2 * y * y));      // 2dn iteration, can be removed

        return y;
    }
};

Worms::Worms()
{
    m_sAppName = L"Worms";
}

int main()
{
    Worms game;
    game.ConstructConsole(256, 160, 6, 6);
    game.Start();
    return 0;
}
