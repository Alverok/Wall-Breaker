#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define PLAYER_MAX_LIFE         5
#define LINES_OF_BRICKS         4
#define BRICKS_PER_LINE        20

typedef struct Paddle {
    Vector2 position;
    Vector2 size;
    int life;
} Paddle;

typedef struct Ball {
    Vector2 position;
    Vector2 speed;
    int radius;
    bool active;
} Ball;

typedef struct Brick {
    Vector2 position;
    bool active;
} Brick;


static const int screenWidth = 1600;
static const int screenHeight = 900;

static bool gameOver = false;
static bool pause = false;
Texture2D background;   
Texture2D heartImage;
Texture2D brickTexture1;
Texture2D brickTexture2;
Texture2D ballTexture;


static Paddle player = { 0 };
static Ball ball = { 0 };
static Brick brick[LINES_OF_BRICKS][BRICKS_PER_LINE] = { 0 };
static Vector2 brickSize = { 0 };

static Music bgMusic;
static Sound brickBreakSound;


static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)


int main(void)
{
    InitWindow(screenWidth, screenHeight, "Wall Breaker!");
    InitAudioDevice();

    InitGame();

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateMusicStream(bgMusic);
        UpdateDrawFrame();
    }
    UnloadGame();
    CloseWindow();

    return 0;
}


// Initialize game variables
void InitGame(void)
{
    
    bgMusic = LoadMusicStream("C:/Users/Akhil/Downloads/Littleroot Town.mp3"); 
    PlayMusicStream(bgMusic);
    SetMusicVolume(bgMusic, 0.1f);
    SetSoundVolume(brickBreakSound, 0.1f);
    
    brickSize = (Vector2){ GetScreenWidth()/BRICKS_PER_LINE, 80 };  //width, height

    // Initialize player
    player.position = (Vector2){ screenWidth/2, screenHeight*7/8 };
    player.size = (Vector2){ screenWidth/10, 20 };
    player.life = PLAYER_MAX_LIFE;

    // Initialize ball
    ball.position = (Vector2){ player.position.x, player.position.y - player.size.y/2 - ball.radius };
    ball.speed = (Vector2){ 0, 0 };
    ball.radius = 15;
    ball.active = false;
    
    // Initialize Textures
    background = LoadTexture("C:/Users/Akhil/Downloads/background_image.png");  
    heartImage = LoadTexture("C:/Users/Akhil/Downloads/heart.png");  
    // Load brick textures
    brickTexture1 = LoadTexture("C:/Users/Akhil/Downloads/brick1.png");  
    brickTexture2 = LoadTexture("C:/Users/Akhil/Downloads/brick2.png");  
    ballTexture = LoadTexture("C:/Users/Akhil/Downloads/ball.png");  
    brickBreakSound = LoadSound("C:/Users/Akhil/Downloads/brick_break.ogg");  


    // Initialize bricks
    int initialDownPosition = 50
    ;

    for (int i = 0; i < LINES_OF_BRICKS; i++)
    {
        for (int j = 0; j < BRICKS_PER_LINE; j++)
        {
            brick[i][j].position = (Vector2){ j*brickSize.x + brickSize.x/2, i*brickSize.y + initialDownPosition };
            brick[i][j].active = true;
        }
    }
}

// Update game (one frame)
void UpdateGame(void)
{   
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            // Player movement logic
            if (IsKeyDown(KEY_LEFT)) player.position.x -= 5;
            if ((player.position.x - player.size.x/2) <= 0) player.position.x = player.size.x/2;
            if (IsKeyDown(KEY_RIGHT)) player.position.x += 5;
            if ((player.position.x + player.size.x/2) >= screenWidth) player.position.x = screenWidth - player.size.x/2;

            // Ball launching logic
            if (!ball.active)
            {
                if (IsKeyPressed(KEY_SPACE))
                {
                    ball.active = true;
                    ball.speed = (Vector2){ 0, -8 };
                }
            }

            // Ball movement logic
            if (ball.active)
            {
                ball.position.x += ball.speed.x;
                ball.position.y += ball.speed.y;
            }
            else
            {
                ball.position = (Vector2){ player.position.x, player.position.y - player.size.y/2 - ball.radius };
            }

            // Collision logic: ball vs walls
            if (((ball.position.x + ball.radius) >= screenWidth) || ((ball.position.x - ball.radius) <= 0)) ball.speed.x *= -1;
            if ((ball.position.y - ball.radius) <= 0) ball.speed.y *= -1;
            if ((ball.position.y + ball.radius) >= screenHeight)
            {
                ball.speed = (Vector2){ 0, 0 };
                ball.active = false;

                player.life--;
            }

            // Collision logic: ball vs player
            if (CheckCollisionCircleRec(ball.position, ball.radius,
                (Rectangle){ player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y}))
            {
                if (ball.speed.y > 0)
                {
                    ball.speed.y *= -1;
                    ball.speed.x = (ball.position.x - player.position.x)/(player.size.x/2)*5;
                }
            }

            // Collision logic: ball vs bricks
            for (int i = 0; i < LINES_OF_BRICKS; i++)
            {
                for (int j = 0; j < BRICKS_PER_LINE; j++)
                {
                    if (brick[i][j].active)
                    {
                        // Hit below
                        if (((ball.position.y - ball.radius) <= (brick[i][j].position.y + brickSize.y/2)) &&
                            ((ball.position.y - ball.radius) > (brick[i][j].position.y + brickSize.y/2 + ball.speed.y)) &&
                            ((fabs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*0.707)) && (ball.speed.y < 0))
                        {
                            brick[i][j].active = false;
                            ball.speed.y *= -1;
                            PlaySound(brickBreakSound);
                        }
                        // Hit above
                        else if (((ball.position.y + ball.radius) >= (brick[i][j].position.y - brickSize.y/2)) &&
                                ((ball.position.y + ball.radius) < (brick[i][j].position.y - brickSize.y/2 + ball.speed.y)) &&
                                ((fabs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*0.707)) && (ball.speed.y > 0))
                        {
                            brick[i][j].active = false;
                            ball.speed.y *= -1;
                            PlaySound(brickBreakSound);
                        }
                        // Hit left
                        else if (((ball.position.x + ball.radius) >= (brick[i][j].position.x - brickSize.x/2)) &&
                                ((ball.position.x + ball.radius) < (brick[i][j].position.x - brickSize.x/2 + ball.speed.x)) &&
                                ((fabs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*0.707)) && (ball.speed.x > 0))
                        {
                            brick[i][j].active = false;
                            ball.speed.x *= -1;
                            PlaySound(brickBreakSound);
                        }
                        // Hit right
                        else if (((ball.position.x - ball.radius) <= (brick[i][j].position.x + brickSize.x/2)) &&
                                ((ball.position.x - ball.radius) > (brick[i][j].position.x + brickSize.x/2 + ball.speed.x)) &&
                                ((fabs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*0.707)) && (ball.speed.x < 0))
                        {
                            brick[i][j].active = false;
                            ball.speed.x *= -1;
                            PlaySound(brickBreakSound);
                        }
                    }
                }
            }

            // Game over logic
            if (player.life <= 0) gameOver = true;  // If all lives lost
            else                                    // Checks if all bricks are broken
            {
                gameOver = true;

                for (int i = 0; i < LINES_OF_BRICKS; i++)
                {
                    for (int j = 0; j < BRICKS_PER_LINE; j++)
                    {
                        if (brick[i][j].active) gameOver = false;
                    }
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);
        
        if (background.id != 0) {
        // Create a destination rectangle with screen dimensions
        Rectangle destRect = { 0, 0, screenWidth, screenHeight };

        // Draw the background texture scaled to fit the entire screen
        DrawTexturePro(background, (Rectangle){ 0, 0, background.width, background.height },
                       destRect, (Vector2){ 0, 0 }, 0.0f, WHITE);  
        }

        if (!gameOver)
        {
            // Draw player bar
            Rectangle paddleRect = { player.position.x - player.size.x / 2, player.position.y - player.size.y / 2, player.size.x, player.size.y };
            // Define the roundness and segments for the paddle
            float roundness = 1.0f;  // Roundness
            int segments = 16;       // Increase segments for smoother curves
            // Draw the paddle with rounded corners
            DrawRectangleRounded(paddleRect, roundness, segments, BLACK);



            // Draw player lives
            float scale = 0.065f;  // Increase scale to make the hearts visible
            for (int i = 0; i < player.life; i++) {
                Vector2 heartPosition = (Vector2){ 0 + 50 * i, screenHeight - 75 };  // Adjust position
                DrawTextureEx(heartImage, heartPosition, 0.0f, scale, WHITE);
            }
            
            // Draw ball
            // DrawCircleV(ball.position, ball.radius, MAROON);  //(Alternate code to generate ball)
            // Vector2 ballPosition = { ball.position.x - ball.radius, ball.position.y - ball.radius };  // Position top-left of the ball
            // DrawTextureV(ballTexture, ballPosition, WHITE);
            
            DrawTexturePro(ballTexture, 
               (Rectangle){ 0, 0, ballTexture.width, ballTexture.height }, // Source rectangle
               (Rectangle){ ball.position.x - ballTexture.width * 0.040f / 2, ball.position.y - ballTexture.height * 0.040f / 2, ballTexture.width * 0.040f, ballTexture.height * 0.040f }, // Destination rectangle with scaling
               (Vector2){ 0, 0 }, 
               0.0f, 
               WHITE); 

            // Draw bricks
            for (int i = 0; i < LINES_OF_BRICKS; i++)
            {
                for (int j = 0; j < BRICKS_PER_LINE; j++)
                {
                    if (brick[i][j].active)
                    {
                        //  alternate brick textures
                        Texture2D currentBrickTexture = (i + j) % 2 == 0 ? brickTexture1 : brickTexture2; // Alternates between brick1 and brick2
                        Rectangle destRect = { brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y };
                        DrawTexturePro(currentBrickTexture, (Rectangle){ 0, 0, currentBrickTexture.width, currentBrickTexture.height }, destRect, (Vector2){ 0, 0 }, 0.0f, WHITE);
                    }
                }
            }

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 50)/2, screenHeight/2 - 40, 50, BLACK);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 40)/2, GetScreenHeight()/2 - 50, 40, BLACK);

    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    // Unload all dynamic loaded data (textures, sounds, models...)
    UnloadTexture(background);
    UnloadTexture(heartImage);
    UnloadTexture(brickTexture1);
    UnloadTexture(brickTexture2);
    UnloadTexture(ballTexture);
    UnloadMusicStream(bgMusic);
    UnloadSound(brickBreakSound);
    CloseAudioDevice(); 
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}