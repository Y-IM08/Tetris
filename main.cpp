#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

const int Screen_height = 20;
const int Screen_width = 10;

string tetropiece[7] = {
    "..X...X...X...X.", // I
    "..X..XX...X.....", // T
    ".....XX..XX.....", // O
    "..X..XX...X.....", // Z
    ".X...XX...X.....", // S
    ".X....X..XX.....", // L
    "..X....X..XX...."  // J
};

int board_piece[Screen_height][Screen_width] = {0}; // 0 for empty space and 1 for block is in their.

void setTerminalInput(bool enable)
{
    static struct termios oldt, newt;
    static int oldFlags;
    if (enable)
    {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        oldFlags=fcntl(STDIN_FILENO, F_GETFL,0);
        fcntl(STDIN_FILENO,F_SETFL,oldFlags| O_NONBLOCK);
    }
    else
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO,F_GETFL,oldFlags);
    }
}

int rotate(int x, int y, int r)
{
    switch (r % 4)
    {
    case 0:
        return y * 4 + x;
    case 1:
        return 12 + y - (4 * x);
    case 2:
        return 15 - (y * 4) - x;
    case 3:
        return 3 - y + (x * 4);
    };
}

bool doesPieceFit(int tetrisId, int r, int bx, int by)
{
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            int pieceIndex = rotate(x, y, r);
            int boardx = x + r;
            int boardy = y + r;

            if (tetropiece[tetrisId][pieceIndex] == 'X')
            {
                if (boardx < 0 || boardx >= Screen_width || boardy >= Screen_height)
                {
                    return false;
                }
                if (boardy >= 0 && board_piece[by][bx] != 0)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

int main()
{
    srand(time(0));
    setTerminalInput(true);

    int currentpiece = rand() % 7;
    int currentrotation = 0;
    int currentX = Screen_width / 2 - 2;
    int currentY = 0;

    int score = 0;
    bool gameover = false;
    int gamespeed = 400;

    while (!gameover)
    {
        // time scheduling
        this_thread ::sleep_for(chrono::milliseconds(50));
        static int ticks = 0;
        ticks++;
        bool forcedown = (ticks >= (gamespeed / 50));

        // input
        char key;
        if (read(STDIN_FILENO, &key, 1) > 0)
        {
            if (key == 'a' && doesPieceFit(currentpiece, currentrotation, currentX - 1, currentY))
                currentX--;
            if (key == 'd' && doesPieceFit(currentpiece, currentrotation, currentX + 1, currentY))
                currentX++;
            if (key == 's' && doesPieceFit(currentpiece, currentrotation, currentX, currentY + 1))
                currentY++;
            if (key == 'w' && doesPieceFit(currentpiece, currentrotation + 1, currentX, currentY))
                currentrotation++;
        }
        // forcepiecedown
        if (forcedown)
        {
            ticks = 0;
        }
        if (doesPieceFit(currentpiece, currentrotation, currentX, currentY + 1))
        {
            currentY++;
        }
        else
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    if (tetropiece[currentpiece][rotate(i, j, currentrotation)] == 'X')
                    { // i and j here and in rotate fn are same
                        if (currentY + j >= 0)
                        {
                            board_piece[currentY + j][currentX + i] = 1;
                        }
                    }
                }
            }

            for (int y = 0; y < 4; y++)
            {
                if (currentY + y >= Screen_height)
                {
                    bool line = true;
                    for (int x = 0; x < Screen_width; x++)
                    {
                        if (board_piece[currentY + y][x] == 0)
                        {
                            line = false;
                            break;
                        }
                    }

                    if (line)
                    {
                        for (int targetY = currentY + y; targetY > 0; targetY--)
                        {
                            for (int x = 0; x < Screen_width; x++)
                            {
                                board_piece[targetY][x] = board_piece[targetY-1][x];
                            }
                        
                            for (int x = 0; x < Screen_width; x++)
                            {
                                board_piece[0][x]=0;
                            }
                        }
                        score += 100;
                    }
                }
            }

            currentpiece = rand() % 7;
            currentrotation = 0;
            currentX = Screen_width / 2 - 2;
            currentY = 0;

            if (!doesPieceFit(currentpiece, currentrotation, currentX, currentY))
            {
                gameover = true;
            }
        }
    }

    cout << "_____Tetris____" << endl;
    cout << "Score: " << ' ' << score << endl;
    cout << "Controls: A=Left, D=Right, W=Rotate, S=Drop" << endl;

    for (int y = 0; y < Screen_height; y++)
    {
        cout << '|';
        for (int x = 0; x < Screen_width; x++)
        {
            bool isPiecePart = false;
            if (x >= currentX && x < currentX + 4 && y >= currentY && y < currentY + 4)
            {
                if (tetropiece[currentpiece][rotate(x - currentX, y - currentY, currentrotation)] == 'X')
                {
                    isPiecePart = true;
                }
            }
            if (board_piece[y][x] != 0)
                cout << "#";
            else if (isPiecePart)
                cout << "X";
            else
                cout << " ";
        }
        cout << "|" << endl;
    }
    cout << "___________" << endl;

    setTerminalInput(false);

    cout << "\nGame Over!\nFinal Score: " << " " << score << endl;

    return 0;
}