#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include<windows.h>
#define ARRAYWIDTH 4
#define SCREENOFFSET 2
#define CHARACTER_A 65
using namespace std;

wstring tetromino[7];

// Grid Size
int nGridWidth = 12;
int nGridHeight = 18;
/*
* Set your default console display to 80 * 30 for proper display
* Right Click on the title window of the console and click on default
* Set the console size to this size
*/
int nScreenWidth = 80;			
int nScreenHeight = 30;

unsigned char* pGrid = nullptr;

int Rotate(int px, int py, int r)
{
    int piece = 0;
    switch (r % 4)   // Clamp Roatation to 4 states
    {
        case 0:
            piece = py * ARRAYWIDTH + px;         // 0 degress
            break;
        case 1:
            piece = 12 + py - (px * ARRAYWIDTH);  // 90 degress
            break;
        case 2:
            piece = 15 - (py * ARRAYWIDTH) - px;  // 180 degress
            break;
        case 3:
            piece = 3 + (px * ARRAYWIDTH) - py;   // 270 degress
            break;
    }
    return piece;
}

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
    for (int px = 0; px < ARRAYWIDTH; px++)
    {
        for (int py = 0; py < ARRAYWIDTH; py++)
        {
            // Index conversion
            int piece = Rotate(px, py, nRotation);

            // Setting index in field
            int field = (nPosY + py) * nGridWidth + (nPosX + px);

            if (nPosX + px >= 0 && nPosX + px < nGridWidth)
            {
                if (nPosY + py >= 0 && nPosY + py)
                {
                    if (tetromino[nTetromino][piece] != L'.' && pGrid[field] != 0)
                    {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

int main()
{
    // Screen Buffer to Display
    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    

    // Current Piece Information
    int nCurrentPiece = 0;
    int nCurrentRotation = 0;
    int nCurrentX = nGridWidth / 2;
    int nCurrentY = 0;


    //Game Control Variables
    bool bGameOver = false;
    bool bKey[4];
    bool bForceDown = false;
    bool bRotateHold = true;  // To disable logic when holding the button 
    int nPieceCount = 0;
    int nScore = 0;
    int nSpeed = 20;
    int nSpeedCount = 0;
    vector<int> vLines;

    //Initialize Screen Buffer With Empty Space
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++)
    {
        screen[i] = L' ';
    }
    
    // Setting up Display
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    //Game Asset
    tetromino[0].append(L"..X...X...X...X.");
    tetromino[1].append(L"..X..XX...X.....");
    tetromino[2].append(L".....XX..XX.....");
    tetromino[3].append(L"..X..XX..X......");
    tetromino[4].append(L".X...XX...X.....");
    tetromino[5].append(L".X...X...XX.....");
    tetromino[6].append(L"..X...X..XX.....");

    // Setting up the grid
    pGrid = new unsigned char[nGridWidth * nGridHeight];

    //Intializing the grid with empty space and boundaries
    for (int x = 0; x < nGridWidth; x++)
    {
        for (int y = 0; y < nGridHeight; y++)
        {
            pGrid[y * nGridWidth + x] = (x == 0 || x == nGridWidth - 1 || y == nGridHeight - 1) ? 9 : 0;
        }
    }
    
    //GAME LOOP
    while (!bGameOver) {

        //Game Timing==========================================================

        this_thread::sleep_for(50ms);  // Single game tick
        nSpeedCount++;
        bForceDown = (nSpeed == nSpeedCount); // Set it to push automatically according to game speed

        //Input=================================================================

        for (int k = 0; k < 4; k++)
        {
            bKey[k] = (0x8000 && GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
            /*
            * x27 -> Left Arrow Key
            * x25 -> Right Arrow Key
            * x28 -> Down arrow Key
            * Z -> Z (xD)
            */
        }
        //Game Logic============================================================

        nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
        nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
        nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

        if (bKey[3])
        {
            nCurrentRotation += (bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
            bRotateHold = false;  
        }
        else
            bRotateHold = true;

        // This will push down block automatically
        if (bForceDown)
        {
            nSpeedCount = 0;
            nPieceCount++;
            if (nPieceCount % 30 == 0)
            {
                if (nSpeed >= 10)
                    nSpeed--;
            }

            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) //Keep pushing down if it's possible
                nCurrentY++;
            else
            {
                for (int px = 0; px < ARRAYWIDTH; px++)     //Setting the current Piece to Grid
                {
                    for (int py = 0; py < ARRAYWIDTH; py++)
                    {
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
                        {
                            pGrid[(nCurrentY + py) * nGridWidth + (nCurrentX + px)] = nCurrentPiece + 1;
                        }
                    }
                }

                // Line Check!
                for (int py = 0; py < ARRAYWIDTH; py++)
                {
                    if (nCurrentY + py < nGridHeight - 1)
                    {
                        bool bLine = true;
                        for (int px = 1; px < nGridWidth - 1; px++)
                        {
                            bLine &= (pGrid[(nCurrentY + py) * nGridWidth + px]) != 0;
                        }
                        if (bLine)
                        {
                            for (int px = 1; px < nGridWidth - 1; px++)
                            {
                                pGrid[(nCurrentY + py) * nGridWidth + px] = 8;
                            }
                            vLines.push_back(nCurrentY + py);
                        }
                    }
                }
                nScore += 35;
                if (!vLines.empty())
                    nScore += (vLines.size()) * 150;

                // New Piece Time
                nCurrentX = nGridWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;
                nCurrentPiece = rand() % 7;

                //Game Over Condition
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);

            }
        }

//Display OUTPUT=========================================================================== 
        
        
        //Draw Grid
        for (int x = 0; x < nGridWidth; x++)
        {
            for (int y = 0; y < nGridHeight; y++)
            {
                screen[(y + SCREENOFFSET) * nScreenWidth + (x + SCREENOFFSET)] = L" ABCDEFG=#"[pGrid[y * nGridWidth + x]];
            }
        }

        //Draw Current Piece(Not A part of grid yet)
        for (int px = 0; px < ARRAYWIDTH; px++)
        {
            for (int py = 0; py < ARRAYWIDTH; py++)
            {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] != L'.')
                {
                    screen[(nCurrentY + py + SCREENOFFSET) * nScreenWidth + (px + nCurrentX + SCREENOFFSET)] = nCurrentPiece + CHARACTER_A;
                }
            }
        }
        // Score Display

        swprintf_s(&screen[2 * nScreenWidth + nGridWidth + 6], 16, L"Score: %8d", nScore);


        //Animate Blocks dissapearing 

        if (!vLines.empty())
        {
            WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth* nScreenHeight, { 0,0 }, & dwBytesWritten);
            this_thread::sleep_for(400ms);

            for (auto &v: vLines)   //Every Lines in the vector Lines
            {
                for (int px = 1; px < nGridWidth - 1; px++)
                {
                    for (int py = v; py > 0; py--)
                    {
                        pGrid[py * nGridWidth + px] = pGrid[(py - 1) * nGridWidth + px];
                        pGrid[px] = 0;
                    }
                }
            }

            vLines.clear();
        }


        //Display Frame
        WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, {0,0}, &dwBytesWritten);

    }
    //Game Over
    CloseHandle(hConsole);
    cout << "Game Over!! \n Try Again Next Time!! \n Score: " << nScore<<"\n";
    system("pause");

    return 0;
}