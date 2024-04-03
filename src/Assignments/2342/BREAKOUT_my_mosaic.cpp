enum PowerUp { extraLife, speedUp, speedDown };

struct Brick {
    vec2 pos;
    vec2 scale;
    vec4 color;

    bool destroyed;
    real32 deathTime;
};

struct Ball {
    vec4 color;
    vec2 pos;
    vec2 nPos;
    vec2 lPos[15];
    int lPosCount;

    vec2 lastPos;
    vec2 velocity;
    vec2 min;
    vec2 max;

    real32 timeSinceLastHit; //Fix so that is compared to as a a stamp - currTIme

    real32 startTime;
    vec2 endPos;
    vec2 startPos;
};

struct Player {
    vec2 pos;
    vec2 mousePos;
    vec2 scale;
    vec4 color;

    bool alive;
    bool hit;
    int speed;
    int speedMod = 1;
    PowerUp status;
};

struct Score {
    int16 highScore; //Make sure not to reinit this on lvl replay
    int16 score;
};

struct LevelData {
    Player player;
    Score score;
    DynamicArray<Brick> bricks;
    DynamicArray<Ball> balls;
    vec2 boardMax;

    vec2 refBrickScale = V2(6,2);
    int rowCount = 3;

    bool active;
    bool win;
    bool restart;
    bool paused;
    bool menuActive = false;

    bool nextLevel = false;
};

LevelData* level = NULL;
MemoryArena arena = {};

float ballStartSpeed = 30;
float maxBallSpeed = 40;
float minBallSpeed = 20;
real32 paddleColorTime = 0.0f;
vec4 tempColor = V4(NULL);
vec4 bgColor = BLACK;
float TimeAlive;
bool inputMouse = false;
int highScore;







//==================================BALLS-======================
void InitBall(vec2 pos = V2(NULL), vec4 color = WHITE) {
    Ball* ball = new Ball;

    if (pos != V2(NULL)) { ball->pos = pos; }
    else { ball->pos = V2(level->boardMax.x / 2, 0); }

    ball->velocity = V2(0, 1) * ballStartSpeed;
    ball->nPos = ball->pos;
    ball->color = WHITE;
    if (color != WHITE) { ball->color = color; }
    ball->timeSinceLastHit = 0.0f;

    PushBack(&level->balls, *ball);
}

void InitBallPowerUp() {
    int startSlot = level->balls.count;
    InitBall();
    InitBall();

    for (int i = startSlot; i < level->balls.count; i++) {
        Ball* b = &level->balls[i];
        b->pos.y = level->player.pos.y - 1;
        b->velocity.y *= -1;
        b->velocity.x *= RandiRange(-1, 1);
    }
}

void CullBalls() {
    for (int i = 0; i < level->balls.count; i++) {
        Ball* b = &level->balls[i];

        if (b->pos.y > level->boardMax.y) {
            RemoveAtIndex(&level->balls, i);
            Print("Ball Culled");
        }
    }
}

//============================Collision==================================
void ResolveClip(Ball* ball) {
    Player* player = &level->player;
    float32 left = player->pos.x;
    float32 top = player->pos.y;
    float32 right = player->pos.x + player->scale.x-1;
    float32 bottom = player->pos.y + player->scale.y;

    if (PointRectTest(MakeRect(V2(top,left), V2(bottom, right)), ball->pos)) {
        ball->pos.y = top -1;
        //ball->velocity.y *= -1; // -Abs(ball->velocity.y);
    }
}

void DetectCollisions() {
    Player* player = &level->player;

    for (int i = 0; i < level->balls.count; i++) {
        Ball* ball = &level->balls[i];

        float32 left = player->pos.x;
        float32 top = player->pos.y ;
        float32 right = player->pos.x + player->scale.x - 1;
        float32 bottom = player->pos.y + player->scale.y + 1;

        /*SetTileColor(left, top, RED);
        SetTileColor(left, bottom, YELLOW);
        SetTileColor(right, bottom, BLUE);
        SetTileColor(right, top, GREEN);*/

        vec2 speed = (ball->pos - ball->lPos[0]) / DeltaTime;
        ball->nPos = ball->pos + (ball->velocity * DeltaTime);
        SetTileColor(ball->pos, YELLOW);

        ball->lastPos = ball->pos - (DeltaTime * ball->velocity);

        
        if (PointRectTest(MakeRect(player->pos + V2(1), V2(player->scale.x, player->scale.y)), ball->nPos)) {
            Print("NextPosCollided");
            level->active = true;

            /*if (ball->color == BLUE) { 
                RemoveAtIndex(&level->balls, i); 
                InitBallPowerUp();
                continue;
            }*/


            if (PointRectTest(MakeRect(V2(top, left), V2(bottom, right)), ball->pos)){
                ResolveClip(ball);
            }


            player->pos = player->pos; //freezing paddle for the collision
            player->hit = true;
            ball->velocity.y *= -1;

            if (player->speed > 0) {
                if (ball->velocity.x < 0) {
                    ball->velocity.x *= .8;
                }
                else if (ball->velocity.x == 0) {
                    ball->velocity.x = ballStartSpeed * RandiRange(-1, 1);
                }
                else if (ball->velocity.x > 0) {
                    if (TilePositionsOverlap(V2(right, top), ball->nPos) || TilePositionsOverlap(V2(left, top), ball->nPos) || TilePositionsOverlap(V2(right, top), ball->pos) || TilePositionsOverlap(V2(left, top), ball->pos)) {
                        //Corner Behavior                                               There should not be a situation where the ball is inside the paddle at this point but I'm leaving the case here just in case.
                        ball->velocity.x *= -1.5;
                        ball->velocity.y *= 1.5;
                    }
                    else {
                        ball->velocity.y *= 1.5;
                        ball->velocity.x *= 1.5;
                    }
                }
                continue;
            }

            if (player->speed < 0) {
                if (ball->velocity.x > 0) {
                    ball->velocity.x *= .8;
                }

                else if (ball->velocity.x == 0) {
                    ball->velocity.x = ballStartSpeed * RandiRange(-1, 1);;
                }

                else if (ball->velocity.x < 0) {
                    if (TilePositionsOverlap(V2(right, bottom), ball->nPos) || TilePositionsOverlap(V2(left, bottom), ball->nPos) || TilePositionsOverlap(V2(right, bottom), ball->pos) || TilePositionsOverlap(V2(left, bottom), ball->pos)) {
                        //Corner Behavior
                        ball->velocity.x *= -1.5;
                        ball->velocity.y *= 1.5;

                    }
                    else {
                        ball->velocity.x *= 1.5;
                        ball->velocity.y *= 1.5;
                    }
                }
                continue;

            }

            if(player->speed == 0) {
                //Corner Behavior
                if (TilePositionsOverlap(V2(right, top), ball->pos) || TilePositionsOverlap(V2(left, top), ball->pos) || TilePositionsOverlap(V2(right, top), ball->nPos) || TilePositionsOverlap(V2(left, top), ball->nPos)) {
                    if (ball->velocity.x > 0) {
                        ball->velocity.x *= -1;
                    }
                }
                if (ball->velocity.x == 0) {
                    ball->velocity.x = ballStartSpeed * RandiRange(-1,1);
                }
            }
        }
        
    }
}

void CullBricks() {
    //poorly named bc this also handles color but I guess that's also part of the culling
    for (int f = 0; f < level->bricks.count; f++) {
        Brick* b = &level->bricks[f];

        if (tempColor == V4(NULL)) { tempColor = b->color; }
        if (b->destroyed) {
            b->deathTime += DeltaTime;
            b->destroyed = true;
            float colorPos;

            if (b->deathTime <= 0.5) {
                colorPos += b->deathTime * 2;
                b->color = Lerp(tempColor, RED, colorPos);
            }
            else { 
                colorPos -= b->deathTime * 2; 
                b->color = V4(b->deathTime, 0.0f, 0.0f, 1.0f);
            }

            if (b->deathTime >= 1) {
                tempColor = V4(NULL);
                RemoveAtIndex(&level->bricks, f);
            }
        }
    }
}

void CheckPowerUpSpawn(vec2 pos) {
    int rand = RandiRange(0, 11);
    SeedRand(rand);

    if (rand == 3) {
        InitBall(pos, BLUE);
    }
}

void DetectBrickCol() {
    for (int f = 0; f < level->bricks.count; f++) {
        Brick* br = &level->bricks[f];
        for (int i = 0; i < level->balls.count; i++) {
            Ball* b = &level->balls[i];


            if (!br->destroyed  && PointRectTest(MakeRect(br->pos, br->scale), b->nPos)) {
                bool canHit = (Time - b->timeSinceLastHit) >= 0.06 ? true : false; //approx t.length of 1 frame

                if (canHit) {
                    b->velocity.y *= -1;
                    b->timeSinceLastHit = Time;
                    br->destroyed = true;
                    break;
                }
                else { Print("Ball collided more than twice in 0.06s"); }
            }
        }
    }
}

void BallBoundaries() {
    Player* player = &level->player;
    for (int i = 0; i < level->balls.count; i++) {
        Ball* ball = &level->balls[i];

        if ((ball->velocity.y > 0) && (ball->nPos.y > level->boardMax.y)) {
            ball->nPos.y = ball->pos.y;
        }
        else if ((ball->velocity.y < 0) && (ball->nPos.y < 0)) {
            ball->nPos.y = ball->pos.y;
        }

         
        if ((ball->velocity.x > 0) && (ball->nPos.x < level->boardMax.x)) {
            ball->nPos.x = ball->pos.x;
        }
        else if ((ball->velocity.x < 0) && (ball->nPos.x < 0)) {
            ball->nPos.x = ball->pos.x;
        }


        if ((ball->velocity.x > 0) && (ball->pos.x >= level->boardMax.x)) {
            ball->pos.x = level->boardMax.x;
            ball->nPos.x = ball->pos.x + 1;
            ball->velocity.x *= -1;
        }
        else if ((ball->velocity.x < 0) && (ball->pos.x <= 0)) {
            ball->pos.x = 0;
            ball->nPos.x = ball->pos.x - 1;
            ball->velocity.x *= -1;
        }

        if ((ball->pos.y >= level->boardMax.y)) {
            Print("Ball Died");
            RemoveAtIndex(&level->balls, i);
            continue;
        }
        else if ((ball->velocity.y < 0) && (ball->pos.y <= 0)) {
            ball->pos.y = 0;
            //ball->nPos.y = ball->pos.y + 1;
            ball->velocity.y *= -1;

        } 
    }
}

//========================Ball=====================================
void RunBall() {
    for (int i = 0; i < level->balls.count; i++) {
        Ball* ball = &level->balls[i];

        for (int i = 14; i > 0; i--) {
            ball->lPos[i] = ball->lPos[i - 1];
        }
        ball->lPos[0] = ball->pos;

        ball->velocity.x = Clamp(ball->velocity.x, -maxBallSpeed, maxBallSpeed);
        ball->velocity.y = Clamp(ball->velocity.y, -maxBallSpeed, maxBallSpeed);

        if (ball->velocity.x < minBallSpeed && ball->velocity.x > 0) {
            ball->velocity.x = minBallSpeed;
        }
        if (ball->velocity.x > -minBallSpeed && ball->velocity.x < 0) {
            ball->velocity.x = -minBallSpeed;
        }
        if (ball->velocity.y < minBallSpeed && ball->velocity.y > 0) {
            ball->velocity.y = minBallSpeed;
        }
        if (ball->velocity.y > -minBallSpeed && ball->velocity.y < 0) {
            ball->velocity.y = -minBallSpeed;
        }

        ball->pos.x += ball->velocity.x * DeltaTime;
        ball->pos.y += ball->velocity.y * DeltaTime;
        
        ball->pos.x = Clamp(ball->pos.x, 0.0f, level->boardMax.x);
        ball->pos.y = Clamp(ball->pos.y, 0.0f, level->boardMax.y);
        DetectCollisions();
    }
}
//================================Bricks==================================
void InitBrick(vec2 pos, vec2 scale, vec4 color) {
    Brick* brick = new Brick;
    brick->color = color;
    brick->scale = scale;
    brick->pos = pos;
    brick->deathTime = 0;
    brick->destroyed = false;

    PushBack(&level->bricks, *brick);
}

void InitRows() {
    float bricksPerRow = level->boardMax.x / level->refBrickScale.x;
    vec2 loc = V2(0);
    Print("boardMax %f", level->boardMax.x);
    Print("refBrickScale %f", level->refBrickScale.x);


    for (int i = 0; i < level->rowCount; i++) {
        Print("Drawing Row %i", i);
        Print("There are %f bricksPerRow", bricksPerRow);

        vec4 color;

        switch (i) {
        case 0:
            color = PASTEL_RED;
            break;
        case 1:
            color = PASTEL_ORANGE;
            break;
        case 2:
            color = PASTEL_YELLOW;
            break;
        case 3:
            color = GREEN;
            break;
        case 4:
            color = PASTEL_BLUE;
            break;
        case 5:
            color = PASTEL_PURPLE;
            break;
        default:
            color = WHITE;
            break;
        }
        
        for (int f = 0; f < bricksPerRow; f++) {
            InitBrick(loc, level->refBrickScale, color);
            loc.x += level->refBrickScale.x;
        }

        loc.y += level->refBrickScale.y;
        loc.x = 0;
    }
}

void DrawBricks() {
    for (int i = 0; i < level->bricks.count; i++) {
        Brick* b = &level->bricks[i];
        SetBlockColor(b->pos, b->scale.x, b->scale.y, b->color);
    }
}

//==========================Player==================================
void InitPlayer() {
    Player* player = &level->player;
    player->speed = 40;
    player->speedMod = 1;
    player->color = WHITE;
    player->scale = V2(7, 2);
    player->pos.x = (level->boardMax.x / 2) - (level->player.scale.x / 2);
    player->pos.y = level->boardMax.y - player->scale.y - 1;

}

void PaddleColorOnHit() {
    vec4 hitColor = BLUE;
    vec4 defColor = WHITE;
    bool dirSwitched;

    Player* paddle = &level->player;
    if (paddle->hit == true) {
        paddleColorTime += DeltaTime;
        real32 t = paddleColorTime / .3f;

        if (t < 1.0f) {
            paddle->color = Lerp(defColor, hitColor, t);
        }
        else { paddle->color = Lerp(hitColor, defColor, t / 2); }

        if (t > 2.0f) {
            paddleColorTime = 0;
            paddle->hit = false;
        }
    }
}

void NormalizeCursor(vec2* mPos) {
    Player* player = &level->player;
    mPos->x = mPos->x * (Game->camera.width * Game->camera.size * 0.5) + (level->boardMax.x/2) - (player->scale.x/2);
    mPos->x = mPos->x + Game->camera.position.x;
    mPos->x = rint(mPos->x);
}

void LevelInputManager() {
    PaddleColorOnHit();
    Player* player = &level->player;
    vec2 mouseMoved = player->mousePos;
    
    player->mousePos = Input->mousePosNormSigned;
    NormalizeCursor(&player->mousePos);

    if (player->mousePos != mouseMoved && !inputMouse) {
        inputMouse = true;
    }

    if (InputPressed(Keyboard, Input_R)) {
        level->restart = true;
    }        

    if (InputHeld(Keyboard, Input_A) || InputHeld(Keyboard, Input_LeftArrow)) {
        player->speed = -80;
        player->pos.x += DeltaTime * player->speed * player->speedMod;
        inputMouse = false;
    }
    else if (InputHeld(Keyboard, Input_D) || InputHeld(Keyboard, Input_RightArrow)) {
        player->speed = 80;
        player->pos.x += DeltaTime * player->speed * player->speedMod;
        inputMouse = false;
    }
    else { player->speed = 0; }

    if(inputMouse) {
        if (player->mousePos.x > player->pos.x) {
            player->speed = 80;
            player->pos.x += DeltaTime * player->speed * player->speedMod;
            player->pos.x = ceil(player->pos.x);

        }
        else if (player->mousePos.x < player->pos.x) {
            player->speed = -80;
            player->pos.x += DeltaTime * player->speed * player->speedMod;
            player->pos.x = floor(player->pos.x);

        }
        else {
            player->speed = 0;
            player->pos.x = floor(player->pos.x);
        }
    }

    player->pos.y = level->boardMax.y - player->scale.y - 1;
    player->pos.x = Clamp(player->pos.x, 0.0f, level->boardMax.x - player->scale.x);
}

//============================Level Controllers========================
void DrawMenu() {
    ClearTiles(V4(0.0f, .4f, .1f, 1.0f));
    DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 15, WHITE, true, "Use A&D to move the paddle!");
    DrawTextScreenPixel(&Game->serifFont, V2(800, 350), 15, WHITE, true, "If the balls hits the bottom of the screen you lose!");
    DrawTextScreenPixel(&Game->serifFont, V2(800, 400), 15, WHITE, true, "Press Esc to Quit, Tab to toggle this menu, and R to Reload");
}

void DrawLose() {
    ClearTiles(V4(0.6f,0.0f,0.2f, 1.0f));

    DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 15, WHITE, true, "You Lose. Your Score was %i !", level->score.score);
    DrawTextScreenPixel(&Game->serifFont, V2(800, 350), 15, WHITE, true, "Press R to Play Again!");

}

void CheckPlayerDeath() {
    if (level->balls.count <= 0) {
        level->active = false;
        DrawLose();
    }
}

void LevelUp() {
    if (level->bricks.count <= 0) {
        level->score.score++;

        if (level->score.score > level->score.highScore) {
            level->score.highScore = level->score.score;
            highScore++;
        }

        level->active = false;
        level->nextLevel = true;;
    }

    if (level->nextLevel) {
        if (level->rowCount % 2 == 0 && maxBallSpeed < 65) {
            maxBallSpeed += 3;
            ballStartSpeed += 3;
        }
        if (level->rowCount % 5 == 0) {
            level->boardMax.x += 6;
            level->boardMax.y += 2;
        }
        
        bgColor.x = RandfRange(0.2f, 0.5f);
        bgColor.y = RandfRange(0.2f, 0.5f);
        bgColor.z = RandfRange(0.2f, 0.5f);

        level->player.pos.x = (level->boardMax.x / 2) - (level->player.scale.x / 2);
        
        if (level->rowCount < 7) {
            level->rowCount++;
        }
        else {
            level->rowCount = 3;
            level->boardMax.x += 6;
        }

        InitRows();
        DynamicArrayClear(&level->balls);
        InitBall();
        level->nextLevel = false;
    }
}

void InitLevelMemory() {
    Mosaic->myData = malloc(sizeof(LevelData));
    memset(Mosaic->myData, 0, sizeof(LevelData));
    level = (LevelData*)Mosaic->myData;

    AllocateMemoryArena(&arena, Megabytes(1));
    level->bricks = MakeDynamicArray<Brick>(&arena, 2);
    level->balls = MakeDynamicArray<Ball>(&arena, 2);
}



//====================Mosaic=================================
void MyMosaicInit() {
    InitLevelMemory();
    level->boardMax = V2(60, 60);
    InitPlayer();

    SetMosaicGridSize(level->boardMax.x, level->boardMax.y);
    level->rowCount = 3;
    level->refBrickScale = V2(6, 2);
    level->active = false;
    level->nextLevel = false;
    level->restart = false;

    inputMouse = false;

    InitRows();
    InitBall();

    bgColor.x = RandfRange(0.0f, 0.3f);
    bgColor.y = RandfRange(0.0f, 0.3f);
    bgColor.z = RandfRange(0.0f, 0.3f);
    bgColor.w = 0.65;
}

void MosaicGameUpdate() {
    DrawTextTop(WHITE, 2, "Time: %f        High Score : % i", TimeAlive, highScore);
    ClearTiles(bgColor);
    RunBall();
    DrawBricks();
    LevelUp();
    LevelInputManager();
    CullBalls();
    CheckPlayerDeath();
    BallBoundaries();

    if (level->restart) {
        TimeAlive = 0;
        DynamicArrayClear(&level->balls);
        DynamicArrayClear(&level->bricks);
        MyMosaicInit();
        level->restart = false;
    }

    if (level->active) {
        TimeAlive += DeltaTime;
        DetectBrickCol();
        CullBricks();
    }

    for (int i = 0; i < level->balls.count; i++) {
        Ball b = level->balls[i];

        real32 colorSpeed = Length(b.velocity) / (Length(b.pos * 3));
        b.color = Lerp(WHITE, YELLOW, colorSpeed);
        
        for (int i = 0; i < 12; i++) {
            //SetTileColor(b.lPos[i], V4(RandfRange(0, 1), RandfRange(0, 1), RandfRange(0, 1), 1.0f));
            vec4 trailColor = b.color;
            trailColor.w = 0.8f;
            SetTileColor(b.lPos[i], trailColor);
        }
        
        SetTileColor(b.pos, b.color);
        //SetTileColor(b.nPos, b.color);
    }

    SetBlockColor(level->player.pos, level->player.scale.x, level->player.scale.y, level->player.color);
}

void MyMosaicUpdate() {
    if (InputPressed(Keyboard, Input_Tab)) {
        level->menuActive = !level->menuActive;
    }

    if (level->menuActive) {
        MosaicGameUpdate();
    }
    else { DrawMenu(); }
}



//Code I didn't use but still want access to==================================================================
        //I made these functions with the intent  of adding powerups but I didn't end up having time to do so :/        
//BezierFunctions work off a function I placed in bezier.h

void EnemeyPath_MakeRandomBezArray(DynamicArray<vec2>* arr, int deg, bool useEnds, vec2 startPos, vec2 endPos) {
    //@TODO should support dynArrays or lists
    DynamicArrayClear(arr);

    for (int i = 0; i <= deg; i++) {
        if (i == 0 || i == deg) {
            PushBack(arr, Vec2RandiRange(V2(0), level->boardMax));
        }

        PushBack(arr, Vec2RandiRange(V2(-4), level->boardMax + V2(4)));
    }

    if (useEnds) {
        (*arr)[0] = startPos;
        (*arr)[deg] = endPos;
    }
}
void EnemeyPath_MakeRandomBezArray(DynamicArray<vec2>* arr, int degree, bool useEnds = false) {
    EnemeyPath_MakeRandomBezArray(arr, degree, useEnds, V2(0), V2(0));
}


void MakeRandomBez(vec2 arr[], int deg, bool useEnds, vec2 startPos, vec2 endPos) {
    //@TODO should support dynArrays or lists
    //int arrCap = arr.size();
    for (int i = 0; i <= deg; i++) {
        if (i == 0 || i == deg) {
            arr[i] = Vec2RandiRange(V2(0), level->boardMax);

                //clamp this to the bounds of the border with a variable instead of hard numbers
                //Doing this bc i never want the starting point to not be on the board. the tile should always be somewhat trackable
        }
        
        arr[i] = Vec2RandiRange(V2(-4), level->boardMax + V2(4));
        Print("(%i, %i)", arr[i].x, arr[i].y);
    }
    if (useEnds){
        arr[0] = startPos;
        arr[deg] = endPos;
    }
}
void MakeRandomBez(vec2 arr[], int degree, bool useEnds = false) {
    MakeRandomBez(arr, degree, useEnds, V2(0), V2(0));
}
void EnemyPath_SinDownAttack(Ball* enemy, float amplitude, float speed = 2.0f) {
    if (enemy->startTime <= 0.0f) {
        enemy->startTime = Time;
    }

    real32 elapsedTime = Time - enemy->startTime;
    real32 t = elapsedTime / speed;

    if (t >= 1.0f) {
        enemy->startTime = 0.0f;
        elapsedTime = 0.0f;
        t = 0.0f;
    }

    if (t < 1.0f) {
        enemy->pos.x = (- cosf(elapsedTime * speed) * amplitude) + enemy->startPos.x;
        enemy->pos.y = Lerp(enemy->startPos.y, enemy->endPos.y, t);
    }
}

//====================Custom Math - Not Necessarily Related To Project Needs a Home=====================================
vec2 DirFromPosToPlayer(vec2 pos, vec2 playerPos) {
    vec2 dir = V2(0);
    if (playerPos.x - pos.x > 0) {
        dir.x = 1;
    }
    else {
        dir.x = -1;
    }
    if (playerPos.y - pos.y > 0) {
        dir.y = 1;
    }
    else {
        dir.y = -1;
    }

    return dir;
}
vec2 Vec2RandiRange(vec2 min, vec2 max, int seed = NULL) {
    if (seed != NULL) {
        SeedRand(seed);
    }

    vec2 out;
    out.x = RandiRange(min.x, max.x);
    out.y = RandiRange(min.y, max.y);
    Print("Random Vec2 = (%f, %f)", out.x, out.y);

    return out;
}