// OLC_Worms.cpp
// re written by Carrast for educational purpose
// 2022.09.09.
// thanks javidx9!

#include <iostream>
#include <string>
#include <algorithm>

#include "olcconsolegameengine.h"
#include "olc_worms.h"

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

    std::list<std::unique_ptr<cPhysicsObject>> listObjects;

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
        {
            listObjects.clear();
            CreateMap();
        }

        // mouse button left
        if (m_mouse[0].bReleased)
        {
            Boom(m_mousePosX + fCameraPosX, m_mousePosY + fCameraPosY, 10);
        }

        // mouse button right
        if (m_mouse[1].bReleased)
        {
            //cDummy* p = new cDummy(m_mousePosX + fCameraPosX, m_mousePosY + fCameraPosY);
            //listObjects.push_back(std::unique_ptr<cDummy>(p));
            cWorm* p = new cWorm(m_mousePosX + fCameraPosX, m_mousePosY + fCameraPosY);
            listObjects.push_back(std::unique_ptr<cWorm>(p));
        }

        // mouse button middle
        if (m_mouse[2].bReleased)
        {
            cMissile* p = new cMissile(m_mousePosX + fCameraPosX, m_mousePosY + fCameraPosY);
            listObjects.push_back(std::unique_ptr<cMissile>(p));
        }

        //  mouse map scroll
        float fMapScrollSpeed = 400.0f;
        if (m_mousePosX < 5) fCameraPosX -= fMapScrollSpeed * fElapsedTime;
        if (m_mousePosX > ScreenWidth() - 5) fCameraPosX += fMapScrollSpeed * fElapsedTime;
        if (m_mousePosY < 5) fCameraPosY -= fMapScrollSpeed * fElapsedTime;
        if (m_mousePosY > ScreenHeight() - 5) fCameraPosY += fMapScrollSpeed * fElapsedTime;

        // limit camera boundaries
        if (fCameraPosX < 0) fCameraPosX = 0;
        if (fCameraPosX >= nMapWidth - ScreenWidth()) fCameraPosX = (float)(nMapWidth - ScreenWidth());
        if (fCameraPosY < 0) fCameraPosY = 0;
        if (fCameraPosY >= nMapHeight - ScreenHeight()) fCameraPosY = (float)(nMapHeight - ScreenHeight());

        // iterate thru object 10 times a frame
        for (int z = 0; z < 10; z++)
        {
            // update pfhisycs of all objects
            for (auto& p : listObjects)
            {
                // apply gravity
                p->ay += 2.0f;

                // update velocity
                p->vx += p->ax * fElapsedTime;
                p->vy += p->ay * fElapsedTime;

                // update position
                float fPotentialX = p->px + p->vx * fElapsedTime;
                float fPotentialY = p->py + p->vy * fElapsedTime;

                // reset accelleration
                p->ax = 0.0f;
                p->ay = 0.0f;
                p->bStable = false;

                // collision detection
                float fAngle = atan2f(p->vy, p->vx);
                float fResponseX = 0.0f;
                float fResponseY = 0.0f;
                bool bCollision = false;

                // iterate through semicircle of objects radius rotated to direction of travel
                for (float r = fAngle - 3.14159f / 2.0f; r < fAngle + 3.14159f / 2.0f; r += 3.14159f / 8.0f)
                {
                    // Calculate test point on circumference of circle
                    float fTestPosX = (p->radius) * cosf(r) + fPotentialX;
                    float fTestPosY = (p->radius) * sinf(r) + fPotentialY;

                    // Constrain to test within map boundary
                    if (fTestPosX >= nMapWidth) fTestPosX = nMapWidth - 1;
                    if (fTestPosY >= nMapHeight) fTestPosY = nMapHeight - 1;
                    if (fTestPosX < 0) fTestPosX = 0;
                    if (fTestPosY < 0) fTestPosY = 0;

                    // test if any points on semicircle intersect with terrain
                    if (map[(int)fTestPosY * nMapWidth + (int)fTestPosX] != 0)
                    {
                        // accumulate collision points to give an escape response vector
                        // effectively, normal to the areas of contact
                        fResponseX += fPotentialX - fTestPosX;
                        fResponseY += fPotentialY - fTestPosY;
                        bCollision = true;
                    }
                }

                // Calculate magnitudes of response and velocity vectors
                float fMagVelocity = sqrtf(p->vx * p->vx + p->vy * p->vy);
                float fMagResponse = sqrtf(fResponseX * fResponseX + fResponseY * fResponseY);

                // find angle of collision
                if (bCollision)
                {
                    // Force object to be stable, this stops the object penetrating the terrain
                    p->bStable = true;

                    // calculate reflection vector of objects velocity vector, using response vector as normal
                    float dot = p->vx * (fResponseX / fMagResponse) + p->vy * (fResponseY / fMagResponse);

                    // use friction coefficient to dampen response (approximating energy loss)
                    p->vx = p->fFriction * (-2.0f * dot * (fResponseX / fMagResponse) + p->vx);
                    p->vy = p->fFriction * (-2.0f * dot * (fResponseY / fMagResponse) + p->vy);

                    // die after 3 collisions
                    if (p->nBounceBeforeDeath > 0)
                    {
                        p->nBounceBeforeDeath--;
                        p->bDead = p->nBounceBeforeDeath == 0;
                        
                        // if the object is dead, work out what to do next
                        // = 0 nothing
                        // > 0 explosion
                        int nResponse = p->BounceDeathAction();
                        if (nResponse > 0)
                            Boom(p->px, p->py, nResponse);
                    }
                }
                else
                {
                    // no collision so update objects position
                    p->px = fPotentialX;
                    p->py = fPotentialY;
                }

                // turn off movement when tiny
                if (fMagVelocity < 0.1f) p->bStable = true;
            }

            // remove objects flagged with bDead
            listObjects.remove_if([](std::unique_ptr<cPhysicsObject> &po) {return po->bDead; });
        }

        // draw landscape
        for (int x = 0; x < ScreenWidth(); x++)
        {
            for (int y = 0; y < ScreenHeight(); y++)
            {
                switch (map[(y + (int)fCameraPosY) * nMapWidth + (x + (int)fCameraPosX)])
                {
                case 0: // sky
                    Draw(x, y, PIXEL_SOLID, FG_CYAN);
                    break;
                case 1: // land
                    Draw(x, y, PIXEL_SOLID, FG_DARK_GREEN);
                    break;
                default:
                    break;
                }
            }
        }

        // draw objects
        for (auto& p : listObjects)
            p->Draw(this, fCameraPosX, fCameraPosY);

        // monitor quantity of objects
        DrawString(2, 2, L"Number of objects: " + std::to_wstring(listObjects.size()));

        return true;
    }

// -----------------------------------------------------------------------------
// --- SPECIAL FUNCTIONS ---
// -----------------------------------------------------------------------------
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
                    map[y * nMapWidth + x] = 1;     // land
                else
                    map[y * nMapWidth + x] = 0;     // sky
            }
        }

        delete[] fSurface;
        delete[] fNoiseSpeed;
    }

    void Boom(float fWorldX, float fWorldY, float fRadius)
    {
        auto CircleBresenham = [&](int xc, int yc, int r)
        {
            // taken from wikipedia
            int x = 0;
            int y = r;
            int p = 3 - 2 * r;
            if (!r) return;

			auto drawline = [&](int sx, int ex, int ny)
			{
				for (int i = sx; i < ex; i++)
					if (ny >= 0 && ny < nMapHeight && i >= 0 && i < nMapWidth)
						map[ny*nMapWidth + i] = 0;
			};

            while (y >= x) // only formulate 1/8 of circle
            {
                // filled circle
				drawline(xc - x, xc + x, yc - y);
				drawline(xc - y, xc + y, yc - x);
				drawline(xc - x, xc + x, yc + y);
				drawline(xc - y, xc + y, yc + x);
                if (p < 0) p += 4 * x++ + 6;
                else p += 4 * (x++ - y--) + 10;
            }
        };

        // erase terrain to form crater
        CircleBresenham(fWorldX, fWorldY, fRadius);

        // shockwave other entities in range
        for (auto& p : listObjects)
        {
            float dx = p->px - fWorldX;
            float dy = p->py - fWorldY;
            float fDist = sqrt(dx * dx + dy * dy);
            if (fDist < 0.0001f) fDist = 0.0001f;
            if (fDist < fRadius)
            {
                p->vx = (dx / fDist) * fRadius;
                p->vy = (dy / fDist) * fRadius;
                p->bStable = false;
            }
        }
        
        // launch debris
        for (int i = 0; i < (int)fRadius; i++)
        {
            cDebris* p = new cDebris(fWorldX, fWorldY);
            listObjects.push_back(std::unique_ptr<cDebris>(p));
        }
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
