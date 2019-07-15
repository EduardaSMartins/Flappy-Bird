#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include "gameutils.h"

// -----------------------------------------------------------------------
// MACROS / CONSTANTES
// -----------------------------------------------------------------------

#define STATE_GAME_TITLE 0
#define STATE_GAME_RUN 1
#define STATE_GAME_OVER 2

#define TRUE 1
#define FALSE 0

#define MAX_TUBES 3
#define TUBES_SIZE 0.7f

// -----------------------------------------------------------------------
// GLOBAL GAME OBJECTS
// -----------------------------------------------------------------------
GameObject bird;

GameObject tubes[MAX_TUBES];
Texture2D tubeTexture;

ScrollingBackground trees;

int state;
int launchCounter;
int launchTime; // tempo em frames (1 seg => 60 frames)

// -----------------------------------------------------------------------
// FUNCTION PROTOTYPES
// -----------------------------------------------------------------------

// ------------------------------------------------------------------------
// INITIALIZATION
// -----------------------------------------------------------------------

void initScreen(){
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "STUDENT BIRD");
    SetTargetFPS(60);

    initGame();
}

void initGame(){
    Texture2D bgTexture = LoadTexture("assets/cenario.png");
    Rectangle bgBounds = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    stars = createScrollingBackground(bgTexture, 2, 0, bgBounds, 1, 1, WHITE);
    //    stars.tint.a = 150;

    Texture2D birdTexture = LoadTexture("assets/bat.png");
    bird = createGameObject(birdTexture, SCREEN_WIDTH * 1 / 3, SCREEN_HEIGHT * 2 / 3, -1, -1, 0.5f, 0.7f);
    setScale(&bird, 0.15f);
    //bird.health = 5;

    for (int i = 0; i < MAX_TUBES; i++){
        tubes[i].active = false;
    }

    state = STATE_GAME_RUN;
    launchCounter = 0;
    launchTime = 30;
}

// -----------------------------------------------------------------------
// LOGIC UPDATE
// -----------------------------------------------------------------------

void updatePlayer(GameObject *bird, int up){
    if (bird->active == FALSE)
        return;

    bird->hitCounter--;
    if (bird->hitCounter <= 0){
        bird->tint = WHITE;
    }

    bird->speed.x = 0;

    // mover o passaro
    if (IsKeyPressed(up))
        bird->speed.y = -10;
    /*
    else
    if (IsKeyDown(down))
        bird->speed.y +=  5;
*/
    // corrige diagonais ~ os catetos (speedX e speedY) recebem 71% da hipotenusa
    //if (bird->speed.x != 0 && bird->speed.y != 0) {
    //    bird->speed.x *= 0.71f;
    //    bird->speed.y *= 0.71f;
    //}

    bird->speed.y += 0.2f;
    if (bird->speed.y > 10)
        bird->speed.y = 10;
    //bird->speed.y = MIN(10, bird->speed.y);

    bird->position.x = bird->position.x + bird->speed.x;
    bird->position.y = bird->position.y + bird->speed.y;

    screenWrap(bird, 0);
    //screenBounds(bird);
}

void hitBird(GameObject *bird, GameObject *other){
    if (!bird->active || !other->active)
        return;

    if (bird->hitCounter > 0)
        return;

    bird->health--;
    bird->hitCounter = 30;
    bird->tint = RED;

    if (bird->health <= 0){
        bird->active = FALSE;
        bird->visible = FALSE;
    }
}

void addToEmptyPosition(GameObject vet[], int max, GameObject newObject){
    int i;
    for (i = 0; i < max; i++){
        if (vet[i].active == FALSE)
            break;
    }
    // nao tem mais espaco... todos os tubos estao sendo usadas
    if (i == max)
        return;

    vet[i] = newObject;
}

void launchBall(){
    launchCounter++;
    if (launchCounter < launchTime)
        return;
    launchCounter = 0;

    // bolas aparecem na parte superior da tela
    GameObject tubes = createGameObject(ballTexture, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, -1, -1, 0.7f, 0.7f);
    setScale(&tubes, TUBES_SIZE);

    while (tubes.speed.x == 0 && tubes.speed.y == 0)
    {
        tubes.speed.x = randomBetween(-2, 2);
        //tubes.speed.y = randomBetween(-2, 2);
    }

    // encontrar a proxima posicao disponivel
    addToEmptyPosition(tubes, MAX_TUBES, tubes);
}

void updateGame(){
    if (state == STATE_GAME_OVER){
        if (IsKeyPressed(KEY_ENTER)){
            initGame();
        }
    }
    else if (state == STATE_GAME_RUN){
        // atualiza logica de todos os objetos do jogo
        stars.offset.x += stars.speed.x;

        updatePlayer(&bird, KEY_W);

        for (int i = 0; i < MAX_TUBES; i++){
            if (tubes[i].active){
                tubes[i].position.x += tubes[i].speed.x;
                tubes[i].position.y += tubes[i].speed.y;
                screenWrap(&tubes[i], tubes[i].scaledWidth / 2);
            }
        }

        launchBall();

        // verifica colisoes e responde
        for (int i = 0; i < MAX_TUBES; i++){
            if (tubes[i].active){
                if (CheckCollision(&bird, &tubes[i])){
                    //hitBird(&bird, &tubes[i]);
                    tubes[i].active = FALSE;
                    continue;
                }
                else if (tubes[i].active){
                    if (CheckCollision(&bird, &tubes[i])){
                        //hitBird(&bird, &tubes[i]);
                        tubes[i].active = TRUE;
                        break;
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------
    // GAME DRAW
    // -----------------------------------------------------------------------

void drawGameOver(){
    drawText("GAME OVER", TEXTUI_CENTERX, SCREEN_HEIGHT / 2, 80, GREEN, BLACK);
    drawText("Press [ENTER] to restart", TEXTUI_CENTERX, SCREEN_HEIGHT / 2 + 160, 35, DARKGREEN, BLACK);
        // escrever a mensagem de GAMEOVER
        if (!bird.active){
            drawText("Draw Game!", TEXTUI_CENTERX, SCREEN_HEIGHT / 2 + 90, 45, WHITE, BLUE);
        }
        else if (!bird.active){
            drawText("Player B Wins!", TEXTUI_CENTERX, SCREEN_HEIGHT / 2 + 90, 45, WHITE, BLUE);
        } //else {
        // drawText("Player A Wins!", TEXTUI_CENTERX, SCREEN_HEIGHT/2 + 90, 45, WHITE, BLUE);
        //}
    }

    void drawGameRun(){
        //ClearBackground(BLACK);

        drawBackground(&stars);

        drawGameObject(&bird);

        for (int i = 0; i < MAX_TUBES; i++){
            if (tubes[i].active){
                drawGameObject(&tubes[i]);
            }
        }

        // HUD
        char str[70];
        sprintf(str, "Score: %03d\n", (int)bird.health);
        drawText(str, 20, 20, 25, WHITE, DARKGREEN);
    }

    void drawGame(){
        BeginDrawing();

        drawGameRun();

        if (state == STATE_GAME_OVER){
            drawGameOver();
        }

        EndDrawing();
    }

    // -----------------------------------------------------------------------
    // GAME LOOP
    // -----------------------------------------------------------------------

int main(){
    initScreen();

    // Main game loop: Detect window close button or ESC key
    while (!WindowShouldClose()){
            updateGame();
            drawGame();
    }
        CloseWindow(); // Close window and OpenGL context
        return 0;
    }

    /*  Criar os tubos
    Fazer pontuação
    Morrer se tocar o chão ou colidir no tubo
*/