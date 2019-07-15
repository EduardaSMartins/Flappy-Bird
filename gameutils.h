#ifndef GAMEUTILS_H_INCLUDED
#define GAMEUTILS_H_INCLUDED

#include <string.h>
#include <math.h>

//#define DEBUG

// caso necessite, basta aumentar estes limites
#define MAX_TEXTURES    20
#define MAX_MUSIC       10
#define MAX_ANIMATIONS  5
#define MAX_FRAMES      10
#define MAX_TEXT        70

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

#define ADD_VECTOR(vecR, vecA, vecB) {vecR.x = vecA.x + vecB.x; vecR.y = vecA.y + vecB.y;}
#define SET_VECTOR(vec, a, b)  {vec.x = a; vec.y = b;}
#define RESET_ZERO(vec)        {vec.x = 0; vec.y = 0;}
#define SET_RECTANGLE(rec, a, b, w, h) {rec.x = a; rec.y = b; rec.width = w; rec.height = h;}

#define TRUE  1
#define FALSE 0

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

#define TEXTUI_CENTERX -1

// -----------------------------------------------------------------------
// COMMON GAME OBJECTS
// -----------------------------------------------------------------------

typedef struct Box {
    float left;
    float right;
    float top;
    float bottom;
} Box;

typedef struct Animation {
    int counter;
    int current;
    int speed;
    int size;
    int frames[MAX_FRAMES];
    int loop; // roda em loop?
} Animation;


/* Backgrounds tem origm em top-left */
typedef struct ScrollingBackground {
    Vector2 tiling;
    Vector2 offset;
    Vector2 speed;
    Rectangle destRect;
    Color tint;
    Texture2D texture;
} ScrollingBackground;


/* Game objects tem origem no centro */
typedef struct GameObject {
    // posicao e movimento
    Vector2 position;
    Vector2 speed;
    float rotation;
    float maxSpeed;

    // colisao e dimensoes
    float scale;
    int width;
    int height;
    float scaledWidth;
    float scaledHeight;
    int collidable;

    // logica/estado do personagem
    int active;
    float health;
    int stateCounter;   // contador generico
    int state;          // controle de estado generico
    int type;           // tipo generico
    int blinkCounter;   // controla intervalo de blink do objeto
    int hitCounter;     // controla tempo para ficar invencivel apos hit

    // renderizacao
    int visible;
    Color tint;
    Texture2D texture;
    Rectangle textureRect;
    Animation animations[MAX_ANIMATIONS];
} GameObject;

typedef struct TextUI {
    char text[MAX_TEXT];
    int active;
    Vector2 position;
    float fontSize;
    int stateCounter;   // contador generico
    int state;          // controle de estado generico
    Color tint;
    Color tintShadow;
} TextUI;


// -----------------------------------------------------------------------
// FUNCTION PROTOTYPES
// -----------------------------------------------------------------------

GameObject createGameObject(Texture2D texture, float x, float y, int frameWidth, int frameHeight,
                            float collisionRatioX, float collisionRatioY);
ScrollingBackground createScrollingBackground(Texture2D texture, float speedX, float speedY, Rectangle bounds,
                                              float tilingX, float tilingY, Color tint);
TextUI createTextUI(char text[], Vector2 position, int fontSize, Color tint, Color tintShadow);

void addAnimation(GameObject* obj, int animationID, int frames[], int size, int speed, int loop);

void animate(GameObject* obj, int animationID);
void setScale(GameObject* obj, float scale);
int getCenterTextX(TextUI* textUI);

float angleBetween(Vector2 a, Vector2 b);
float lerp(float start, float end, float time);
Box getCollisionBox(GameObject* obj);

void drawBackground(ScrollingBackground* bg);
void drawGameObject(GameObject* obj);
void drawTextUI(TextUI* textUI);

// ------------------------------------------------------------------------
// FUNCTION IMPLEMENTATIONS
// -----------------------------------------------------------------------

GameObject createGameObject(Texture2D texture, float x, float y, int frameWidth, int frameHeight,
                            float collisionRatioX, float collisionRatioY) {
    GameObject obj;

    obj.active = TRUE;
    obj.texture = texture;
    if (frameWidth == -1) {
        frameWidth = obj.texture.width;
        frameHeight = obj.texture.height;
    }
    // retangulo para desenho
    SET_RECTANGLE(obj.textureRect, 0, 0, frameWidth, frameHeight);

    obj.stateCounter = 0;
    obj.state = 0;
    obj.health = 0;
    obj.type = 0;
    SET_VECTOR(obj.position, x, y);
    obj.rotation = 0;
    obj.tint = WHITE;
    obj.visible = TRUE;
    obj.collidable = TRUE;
    obj.blinkCounter = 0;
    obj.hitCounter = 0;
    obj.maxSpeed = 0;
    SET_VECTOR(obj.speed, 0, 0);

    // tamanho considerado para colisao
    obj.width = frameWidth * collisionRatioX;
    obj.height= frameHeight * collisionRatioY;
    setScale(&obj, 1);
    return obj;
}

ScrollingBackground createScrollingBackground(Texture2D texture, float speedX, float speedY, Rectangle bounds,
                                              float tilingX, float tilingY, Color tint) {
    ScrollingBackground bg;
    bg.texture = texture;
    SET_VECTOR(bg.tiling, tilingX, tilingY);
    SET_VECTOR(bg.speed, speedX/SCREEN_WIDTH, speedY/SCREEN_HEIGHT);
    SET_VECTOR(bg.offset, 0, 0);
    bg.destRect = bounds;
    bg.tint = tint;
    return bg;
}

TextUI createTextUI(char text[], Vector2 position, int fontSize, Color tint, Color tintShadow) {
    TextUI textUI;
    strcpy(textUI.text, text);
    textUI.active  = TRUE;
    textUI.position = position;
    textUI.fontSize = fontSize;
    textUI.stateCounter = 0;
    textUI.state = 0;
    textUI.tint = tint;
    textUI.tintShadow = tintShadow;

    if (textUI.position.x == TEXTUI_CENTERX) {
        textUI.position.x = getCenterTextX(&textUI);
    }
    return textUI;
}

void addAnimation(GameObject* obj, int animationID, int frames[], int size, int speed, int loop) {
    Animation anim;
    anim.size = size;
    anim.speed = speed;
    anim.counter = 0;
    anim.current = 0;
    anim.loop = loop;
    for (int i = 0; i < size && i < MAX_FRAMES; i++) {
        anim.frames[i] = frames[i];
    }
    obj->animations[animationID] = anim;
}

/** Interpolacao linear.
 *  @param time in 0-1 range (normalized)
**/
float lerp(float start, float end, float time) {
    return start + time * (end - start);
}

float angleBetween(Vector2 a, Vector2 b) {
    return atan2(a.y - b.y, a.x - b.x) * 180/PI;
}

void setScale(GameObject* obj, float scale) {
    obj->scale = scale;
    obj->scaledWidth = obj->width * scale;
    obj->scaledHeight = obj->height * scale;
}

int animationEnded(GameObject *obj, int animationID) {
    return obj->animations[animationID].current == obj->animations[animationID].size;
}

void animate(GameObject* obj, int animationID) {

    Animation* anim = &obj->animations[animationID];

    // if it doesnt loop, stop increasing current value
    if (!anim->loop && anim->current > anim->size) {
        return;
    }
    anim->counter++;
    if (anim->counter >= anim->speed) {
        anim->counter = 0;
        anim->current++;

        if (anim->current >= anim->size || anim->current >= MAX_FRAMES) {
            if (anim->loop) {
                anim->current = 0;
            } else {
                obj->visible = FALSE;
            }
        }
        obj->textureRect.x = (float)anim->frames[anim->current]*(float)obj->textureRect.width;
    }
}

Box getBoundingBox(GameObject* obj) {
    Box box;
    box.left = obj->position.x - obj->scaledWidth/2;
    box.right = obj->position.x + obj->scaledWidth/2;
    box.top = obj->position.y - obj->scaledHeight/2;
    box.bottom = obj->position.y + obj->scaledHeight/2;
    return box;
}

Rectangle getBoundingRect(GameObject* obj) {
    Rectangle rect;
    rect.x = obj->position.x - obj->scaledWidth/2;
    rect.y = obj->position.y - obj->scaledHeight/2;
    rect.width = obj->scaledWidth;
    rect.height= obj->scaledHeight;
    return rect;
}

int CheckCollision(GameObject* obj1, GameObject* obj2) {
    return CheckCollisionRecs(getBoundingRect(obj1), getBoundingRect(obj2));
}

void drawBackground(ScrollingBackground* bg) {
    DrawTextureQuad(bg->texture, bg->tiling, bg->offset, bg->destRect, bg->tint);
}

void drawCollisionBox(GameObject* obj) {
    Box box = getBoundingBox(obj);
    DrawRectangleLines(box.left, box.top, box.right - box.left, box.bottom - box.top, GREEN);
}

void drawGameObject(GameObject* obj) {
    if (!obj->visible) {
        return;
    }
    // prepara rect fonte e destino e origem, e adequa pela escala
    Rectangle destRect = {obj->position.x, obj->position.y,
                          obj->textureRect.width*obj->scale, obj->textureRect.height*obj->scale};

    Vector2 origin = {obj->textureRect.width*obj->scale/2, obj->textureRect.height*obj->scale/2};

    //DrawTextureRec(obj->tex, rect, pos, WHITE);
    DrawTexturePro(obj->texture, obj->textureRect, destRect, origin, obj->rotation, obj->tint);

    #ifdef DEBUG
    drawCollisionBox(obj);
    #endif // DEBUG
}

int getCenterTextX(TextUI* textUI) {
    return (SCREEN_WIDTH-MeasureText(textUI->text,textUI->fontSize))/2;
}

void drawText(const char* str, int x, int y, int fontSize, Color textColor, Color shadowColor) {
    if (x == TEXTUI_CENTERX)
        x= (SCREEN_WIDTH-MeasureText(str, fontSize))/2;

    DrawText(str, x+2, y+2, fontSize, shadowColor);
    DrawText(str, x, y, fontSize, textColor);
}

void drawTextUI(TextUI* textUI) {
    if (!textUI->active) return;

    drawText(textUI->text, textUI->position.x, textUI->position.y, textUI->fontSize, textUI->tint, textUI->tintShadow);
    //DrawText(textUI->text, textUI->position.x+2, textUI->position.y+2, textUI->fontSize, textUI->tintShadow);
    //DrawText(textUI->text, textUI->position.x, textUI->position.y, textUI->fontSize, textUI->tint);
}

/** Teste de comentario */
void screenWrap(GameObject *obj, int offset) {
    if (obj->position.x < 0 - offset) {
        obj->position.x = SCREEN_WIDTH - 1 + offset;
    }
    else
    if (obj->position.x > SCREEN_WIDTH + offset) {
        obj->position.x = 1 - offset;
    }

    if (obj->position.y  < 0 - offset) {
        obj->position.y = SCREEN_HEIGHT - 1 + offset;
    }
    else
    if (obj->position.y > SCREEN_HEIGHT + offset) {
        obj->position.y = 1 - offset;
    }
}

void screenBounds(GameObject *obj) {
    Box box = getBoundingBox(obj);

    if (box.left < 0) {
        obj->position.x = 0 + obj->scaledWidth/2;
    }
    else
    if (box.right > SCREEN_WIDTH) {
        obj->position.x = SCREEN_WIDTH - obj->scaledWidth/2;
    }

    if (box.top < 0) {
        obj->position.y = 0 + obj->scaledHeight/2;
    }
    else
    if (box.bottom > SCREEN_HEIGHT) {
        obj->position.y = SCREEN_HEIGHT - obj->scaledHeight/2;
    }
}

int randomBetween(int min, int max) {
    return rand() % (max - min + 1) + min;
}

#endif // GAMEUTILS_H_INCLUDED
