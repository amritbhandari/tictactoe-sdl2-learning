#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <cstdlib> // For rand() and srand()
#include <ctime>   // For time()

using namespace std;

const auto FONT_PATH = "images/consolas.ttf";
constexpr int FONT_SIZE = 32;
const auto SPRITES_FOLDER = "images/";
const auto CROSS_PATH = SPRITES_FOLDER + string("cross.png");
const auto CIRCLE_PATH = SPRITES_FOLDER + string("circle.jpg");

constexpr int SCREEN_WIDTH = 600;
constexpr int SCREEN_HEIGHT = 600;

constexpr int GRID_SIZE = 3;
constexpr int CELL_SIZE = SCREEN_WIDTH / GRID_SIZE;
int gridsEmpty = 9;

enum Player
{
    NONE, PLAYER_X, PLAYER_O
};

Player currentPlayer = PLAYER_X;
Player winner = NONE;

struct Cell
{
    Player owner;

    Cell() : owner(NONE)
    {
    }
};

vector<vector<Cell>> board(GRID_SIZE, vector<Cell>(GRID_SIZE));

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

SDL_Texture* crossTexture = nullptr;
SDL_Texture* circleTexture = nullptr;

SDL_Texture* gameOverTexture = nullptr;
SDL_Texture* replayTexture = nullptr;

TTF_Font* font = nullptr;

bool continueGame = true;
bool continuePlaying = true;

void DrawGrid()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 1; i < GRID_SIZE; i++)
    {
        SDL_RenderDrawLine(renderer, i * CELL_SIZE, 0, i * CELL_SIZE, SCREEN_HEIGHT);
        SDL_RenderDrawLine(renderer, 0, i * CELL_SIZE, SCREEN_WIDTH, i * CELL_SIZE);
    }
}

void DrawBoard()
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            SDL_Texture* texture = nullptr;
            switch (board[i][j].owner)
            {
            case PLAYER_X:
                texture = crossTexture;
                break;
            case PLAYER_O:
                texture = circleTexture;
                break;
            case NONE:
                break;
            }

            if (texture)
            {
                SDL_Rect cellRect = {i * CELL_SIZE + 5, j * CELL_SIZE + 5, CELL_SIZE - 5, CELL_SIZE - 5};
                SDL_RenderCopy(renderer, texture, nullptr, &cellRect);
            }
        }
    }
}

bool playerMatch(Player p1, Player p2)
{
    return p1 == p2;
}

Player checkWinner()
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        if (board[i][0].owner != NONE) // across rows
        {
            if (Player winner = board[i][0].owner; playerMatch(winner, board[i][1].owner) && playerMatch(
                winner, board[i][2].owner))
            {
                return winner;
            }
        }
        if (board[0][i].owner != NONE) // across columns
        {
            if (Player winner = board[0][i].owner; playerMatch(winner, board[1][i].owner) && playerMatch(
                winner, board[2][i].owner))
            {
                return winner;
            }
        }
    }
    if (board[0][0].owner != NONE) // diagonal from top left
    {
        if (Player winner = board[0][0].owner; playerMatch(winner, board[1][1].owner) && playerMatch(
            winner, board[2][2].owner))
        {
            return winner;
        }
    }
    if (board[0][2].owner != NONE) // diagonal from bottom left
    {
        if (Player winner = board[0][2].owner; playerMatch(winner, board[1][1].owner) && playerMatch(
            winner, board[0][2].owner))
        {
            return winner;
        }
    }

    return NONE;
}

bool initialiseSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return false;

    if (TTF_Init() == -1)
    {
        cerr << "TTF_Init error" << TTF_GetError() << endl;
        return false;
    }

    window = SDL_CreateWindow("Tic Tac Toe",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                              SDL_WINDOW_SHOWN);
    if (!window)
    {
        cerr << "SDL_CreateWindow error: " << SDL_GetError() << endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        cerr << "SDL_CreateRenderer error: " << SDL_GetError() << endl;
        SDL_Quit();
        return false;
    }

    return true;
}

bool loadMedia()
{
    circleTexture = IMG_LoadTexture(renderer, CIRCLE_PATH.c_str());
    if (!circleTexture)
    {
        cerr << "IMG_LoadTexture images/circle.png error: " << IMG_GetError() << endl;
        return false;
    }

    crossTexture = IMG_LoadTexture(renderer, CROSS_PATH.c_str());
    if (!crossTexture)
    {
        cerr << "IMG_LoadTexture images/cross.png error: " << IMG_GetError() << endl;
        return false;
    }

    gameOverTexture = IMG_LoadTexture(renderer, (SPRITES_FOLDER + string("gameover.png")).c_str());
    if (!gameOverTexture)
    {
        cerr << "IMG_LoadTexture images/gameover.png error: " << IMG_GetError() << endl;
        return false;
    }

    font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
    if (!font)
    {
        cerr << "TTF_OpenFont error: " << TTF_GetError() << endl;
        return false;
    }

    return true;
}

void RenderText(const char* text, SDL_Texture*& texture, SDL_Rect& destRect)
{
    SDL_Color textColour = {255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColour);
    texture = SDL_CreateTextureFromSurface(renderer, textSurface);

    destRect.w = textSurface->w;
    destRect.h = textSurface->h;

    SDL_FreeSurface(textSurface);

    destRect.x = (SCREEN_WIDTH - destRect.w) / 2;
}

void RenderPlayAgainText()
{
    SDL_Rect replayRect;

    replayRect.y = SCREEN_HEIGHT / 2 + 50;

    RenderText("Press Spacebar to Play Again", replayTexture, replayRect);

    SDL_RenderCopy(renderer, replayTexture, nullptr, &replayRect);
}

void ResetGame()
{
    currentPlayer = PLAYER_X;
    gridsEmpty = 9;

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
            board[i][j].owner = NONE;
    }

    continueGame = true;
}

void handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        switch (event.type)
        {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                int x = event.button.x / CELL_SIZE; // which column
                int y = event.button.y / CELL_SIZE; // which row

                if (board[x][y].owner == NONE)
                {
                    board[x][y].owner = currentPlayer;
                    currentPlayer = (currentPlayer == PLAYER_X) // switch current player
                                        ? PLAYER_O
                                        : PLAYER_X;

                    gridsEmpty--;
                }
            }
            break;
        case SDL_QUIT:
            continuePlaying = false;
            continueGame = false;
            break;
        case SDL_KEYUP:
            if (!continueGame)
            {
                if (event.key.keysym.sym == SDLK_SPACE)
                    ResetGame();
            }
            break;
        }
    }
}

void Destroy()
{
    SDL_DestroyTexture(circleTexture);
    SDL_DestroyTexture(crossTexture);

    SDL_DestroyTexture(gameOverTexture);
    SDL_DestroyTexture(replayTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
}

int main()
{
    if (!initialiseSDL())
        return -1;

    if (!loadMedia())
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    while (continuePlaying)
    {
        handleEvents();

        if (continueGame)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);

            DrawGrid();

            DrawBoard();

            SDL_RenderPresent(renderer);

            if (checkWinner() != NONE || gridsEmpty == 0)
            {
                SDL_Delay(1500);
                continueGame = false;
            }
        }
        else
        {
            SDL_Rect gameOverRect = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
            SDL_RenderCopy(renderer, gameOverTexture, nullptr, &gameOverRect);

            RenderPlayAgainText();

            SDL_RenderPresent(renderer);
        }
    }

    Destroy();
}
