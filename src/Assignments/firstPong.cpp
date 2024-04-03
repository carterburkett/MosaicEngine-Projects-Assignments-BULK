//struct CollisionData {
//    vec2 posCorrection;
//    int32 sideHit;
//    bool collided;
//};

struct Boundaries {
    vec4 color;
    vec2 min;
    vec2 max;
    vec2 scale;
    int32 midLine;
};

struct Paddle {
    vec2 scale;
    vec2 pos;
    vec2 min;
    vec2 max;
    vec4 color;
    bool hit; 
};

struct PongBall {
    vec4 color;
    //int32 accel;
    vec2 pos;
    vec2 nPos;
    vec2 velocity;
    //vec2 correction;
    vec2 size;
    vec2 min;
    vec2 max;
};

struct ScoreCount {
    int32 count;
    int32 height;
    int32 yPos;
};

struct BoardData {
    Boundaries perimeter;
    Paddle paddle[2];
    PongBall gameBall;
    ScoreCount scores[2];
    //CollisionData collision;
};


BoardData* board = NULL;
SoundClip soundFX;
//I wanted to use more sounds but everytime I tried to pull a sound I added to the sfx folder
//the entire game crashed. I made sure the file was mono and had a similar size to the flute sound
//I'm not really sure what I was doing wrong

bool gameOver;
real32 difficultyMod = 0;
real32 difficultyLevel = 2;
real32 ballStartSpeed = 55.0f;
real32 maxBallSpeed = 70.0f;

float paddleColorTime = 0.0f;
real32 paddleSpeed = 80.0f;

//=========================================================================
//                   Initializing Components and Math
//=========================================================================
Rect MakeBetterRect(vec2 position, vec2 halfSize) {
    //When you use this normal MakeRect() it doesn't take into account the starting tile's position it just adds to it
    //I subtracted one so that it would line up with the boundaries of my paddle
    Rect r;
    r.min = position - halfSize - V2(1);
    r.max = position + halfSize;
    return r;
}

void InitPaddles() {
    //player (left) Paddle is paddle[0]
    board->paddle[0].pos = V2(4.0f, 39.0f);
    board->paddle[1].pos = V2(116.0f, 39.0f);
    for (int i = 0; i < 2; i++) {
        board->paddle[i].max = V2(-1.0f, -3.0f);
        board->paddle[i].min = V2(1.0f, 3.0f);
        board->paddle[i].scale = V2(2.0f, 6.0f);
        board->paddle[i].color = WHITE;
    }

}

void InitBall() { 
    board->gameBall.size = V2(1, 1);
    board->gameBall.min = board->perimeter.min + board->gameBall.size;
    board->gameBall.max = board->perimeter.max - board->gameBall.size;
    
    board->gameBall.color = WHITE;
    

    float32 randY = RandfRange(board->gameBall.min.y -1 , board->gameBall.max.y + 1);

    bool even = RandiRange(1, 6) % 2 == 0;
    int xDir = 1;
    int yDir = RandiRange(-1, 1);
    if (even) {
        xDir = -1;
    }
    if (yDir == 0) {
        yDir += RandiRange(-1, 1);
    }

    board->gameBall.pos = V2(60.0f, randY);
    board->gameBall.velocity = V2(xDir, yDir) * ballStartSpeed;
    //board->gameBall.accel = 25;
}

void InitScores() {
    for (int i = 0; i < 2; i++) {
        board->scores[i].count = 0;
        board->scores[i].yPos = 2;
    }
}

void EndGame() {
    ScoreCount* playerScore = &board->scores[0];
    ScoreCount* compScore = &board->scores[1];
    PongBall* ball = &board->gameBall;
    float playTime;
    
    if (playerScore->count >= 11) {
        playTime = DeltaTime;
        ball->velocity = V2(0);
        ball->pos = V2(0);

        gameOver = true;
        ClearTiles(BLUE);
        DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 32, BLUE, true, "YOU WIN!");
        DrawTextScreenPixel(&Game->serifFont, V2(800, 700), 32, BLUE, true, "Hit Space To Play Again or ESC to Quit");
    }
    if (compScore->count >= 11) {
        playTime = DeltaTime;
        ball->velocity = V2(0);
        ball->pos = V2(0);

        gameOver = true;
        ClearTiles(RED);
        DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 32, WHITE, true, "YOU LOSE...");
        DrawTextScreenPixel(&Game->serifFont, V2(800, 700), 32, WHITE, true, "Hit Space To Play Again or ESC to Quit");
    }
}


//=========================================================================
//                           Draw Functions
//=========================================================================
void PaddleColorOnHit() {
    vec4 hitColor;
    vec4 defColor = WHITE;
    
    for (int i = 0; i < 2; i++) {
        Paddle* paddle = &board->paddle[i];
        if (paddle->hit == true) {

            paddleColorTime += DeltaTime;
            real32 duration = .2;

            float t = paddleColorTime / duration;

            if (t >= 1.0) {
                t *= -1;
            }
            
            if (paddleColorTime > duration) {
                paddleColorTime = 0;
            }
            
            

            if (i == 0) {
                hitColor = BLUE;
            }
            if (i == 1) {
                hitColor = RED;
            }

            paddle->color = Lerp(defColor, hitColor, t);

            if (t < 0) {
                paddleColorTime = 0;
                paddle->hit = false;
            }
        }
    }
}

void DrawScores() {
    for (int i = 0; i < 2; i++) {
        ScoreCount* score = &board->scores[i];
        if (score->count >= 11) {
            
        }

        if (i == 0) {
            for (int s = 0; s < (score->count * 2); s += 2) {
                SetBlockColor(s + 1, score->yPos, 1, 2, BLUE);
            }
        }

        else if (i == 1) {
            for (int s = 0; s < (score->count * 2); s+=2) {
                SetBlockColor(118 - s, score->yPos, 1, 2, RED);
            }  
        }
    }
}

void DrawPaddles() {
    PaddleColorOnHit();
    for (int i = 0; i < 2; i++) {
        Paddle* pongPaddle = &board->paddle[i];
        SetBlockColor(pongPaddle->pos - pongPaddle->min, 2, 6, pongPaddle->color);
    }
}

void DrawCourt() {
    //I defined a new color in color.h for the color here.
    Boundaries* walls = &board->perimeter;
    for (int y = 6; y <= walls->max.y; y++) {   
        for (int x = 0; x <= walls->max.x; x++) {
            if((x == walls->min.x || x == walls->max.x) || (y == walls->min.y || y == walls->max.y) ){
                SetTileColor(x,y, GREY);
            }
            if (x == 60 && y % 2 == 0) {
                SetTileColor(x, y, GREY);
            }
        }      
    }
}


//=========================================================================
//                              Collision
//=========================================================================
void DetectCollisions() {
    PongBall* ball = &board->gameBall;

    for (int i = 0; i < 2; i++) {
        Paddle* paddle = &board->paddle[i];
        
        float32 left = paddle->pos.x - paddle->min.x;
        float32 top = paddle->pos.y - paddle->min.y;
        float32 right = paddle->pos.x + paddle->min.x -ball->size.x;
        float32 bottom = paddle->pos.y + paddle->min.y - ball->size.y;

        /*SetTileColor(left, top, RED);
        SetTileColor(left, bottom, YELLOW);
        SetTileColor(right, bottom, BLUE);
        SetTileColor(right, top, GREEN);*/


        ball->nPos = ball->pos + (DeltaTime * ball->velocity);

        if (PointRectTest(MakeBetterRect(paddle->pos, V2(1,3)), ball->nPos)) {
            paddle->hit = true;


            PlaySound(&Game->audioPlayer, soundFX, 1.0f);
            if (InputHeld(Keyboard, Input_UpArrow)) {

                if (ball->velocity.y < 0) {
                    ball->velocity.y *= 1.5;
                }

                else if(ball->velocity.y == 0) {
                    ball->velocity.y *= ballStartSpeed;
                }

                else if (ball->velocity.y > 0) {
                    if (TilePositionsOverlap(V2(right, top), ball->nPos) || TilePositionsOverlap(V2(left, top), ball->nPos)) {
                        //Corner Behavior
                        ball->velocity.y *= -1;
                    }
                    else {
                        ball->velocity.y *= 0.5;
                    }
                }
            }

            else if (InputHeld(Keyboard, Input_DownArrow)) {
                
                if (ball->velocity.y > 0) {
                    ball->velocity.y *= 1.5;
                }

                else if (ball->velocity.y == 0) {
                    ball->velocity.y *= ballStartSpeed;
                }

                else if (ball->velocity.y < 0) {
                    if (TilePositionsOverlap(V2(right, bottom), ball->nPos) || TilePositionsOverlap(V2(left, bottom), ball->nPos)) {
                        //Corner Behavior
                        ball->velocity.y *= -1;               
                    }
                    else {
                        ball->velocity.y *= 0.5;
                    }
                }

            }
            //ball->correction = V2(0);
            ball->velocity.x *= -1;
        }
        //if (paddle->hit == false) {
        //    if (ball->velocity.y > 0) {
        //        //bottom side(s)
        //        DetectionAlogrithm(ball->pos, ball->nPos, V2(left, bottom), V2(right, bottom), 3, paddle);
        //        if (ball->correction.y != 0) {
        //            ball->pos.y = ball->correction.y;
        //        }
        //    }
        //    else if (ball->velocity.y < 0) {
        //        //top
        //        DetectionAlogrithm(ball->pos, ball->nPos, V2(left, top), V2(right, top), 2, paddle);
        //        if (ball->correction.y != 0) {
        //            ball->pos.y = ball->correction.y;
        //        }
        //    }
        //}
    
    
    }

    
}

void BallBoundaries() {
    PongBall* ball = &board->gameBall;
    ScoreCount* player = &board->scores[0];
    ScoreCount* comp = &board->scores[1];

    if ((ball->velocity.x > 0) && (ball->pos.x > ball->max.x)) {
        ball->pos.x = ball->max.x;
        ball->velocity.x *= -1;

        InitBall();
        player->count++;
    }
    else if ((ball->velocity.x < 0) && (ball->pos.x < ball->min.x)) {
        ball->pos.x = ball->min.x;
        ball->velocity.x *= -1;
        
        InitBall();
        comp->count++;
    }


    if ((ball->velocity.y > 0) && (ball->pos.y > ball->max.y)) {
        ball->pos.y = ball->max.y;
        ball->velocity.y *= -1;
        
    }
    else if ((ball->velocity.y < 0) && (ball->pos.y < ball->min.y)) {
        ball->pos.y = ball->min.y;
        ball->velocity.y *= -1;
    }
}


//=========================================================================
//                            Object Movement
//=========================================================================
void PongAI() {
    //AI gets faster as you score more. I'm not sure I like this but it's it what I went with for now
    PongBall* ball = &board->gameBall;
    Paddle* paddle = &board->paddle[1];
    ScoreCount* playerScore = &board->scores[0];

    if (playerScore->count > difficultyLevel && playerScore->count < 10) {
        difficultyMod += 5;
        difficultyLevel += 2;
    }

    if (ball->velocity.x > 0) {
        if ((paddle->pos.y < ball->pos.y) && (paddle->pos.y < 76)) {
            paddle->pos.y += DeltaTime * ( (paddleSpeed/2) + difficultyMod);
        }
        if ((paddle->pos.y > ball->pos.y) && (paddle->pos.y > 11)) {
            paddle->pos.y -= DeltaTime * ( (paddleSpeed/2) + difficultyMod);
        }
        if ((paddle->pos.y == ball->pos.y) && (paddle->pos.y < 76) && (paddle->pos.y > 11)) {
            paddle->pos.y = paddle->pos.y;
        }
    }
}

void MovePlayerPaddle() {
    Paddle* playerPaddle = &board->paddle[0];

    if (InputHeld(Keyboard, Input_UpArrow) && (playerPaddle->pos.y > 11)) {
        playerPaddle->pos.y -= paddleSpeed * DeltaTime;
    }

    if (InputHeld(Keyboard, Input_DownArrow) && (playerPaddle->pos.y < 76)) {
        playerPaddle->pos.y += paddleSpeed * DeltaTime;
    }

}

void RunBall() {
    PongBall* ball = &board->gameBall;
    ball->velocity.x = Clamp(ball->velocity.x, -maxBallSpeed, maxBallSpeed);
    ball->velocity.y = Clamp(ball->velocity.y, -maxBallSpeed, maxBallSpeed);

    ball->pos.x += ball->velocity.x * DeltaTime;
    ball->pos.y += ball->velocity.y * DeltaTime;
    DetectCollisions();


}


//=========================================================================
//                          Game Start and Run
//=========================================================================
void MyMosaicInit() {
    SetMosaicGridSize(120, 80);    

    Mosaic->myData = malloc(sizeof(BoardData));
    memset(Mosaic->myData, 0, sizeof(BoardData));
    board = (BoardData*)Mosaic->myData;

    LoadSoundClip("data/sfx/flute_breathy_c4.wav", &soundFX);    

    board->perimeter.min = V2(0.0, 6.0);
    board->perimeter.max = V2(119.0, 79.0);
    board->perimeter.midLine = 119 / 2;
    board->perimeter.scale = V2(79, 119);
    
    gameOver = false;
    
    InitPaddles();
    InitBall();
    InitScores();
}

void MyMosaicUpdate() {
    ClearTiles(BLACK);
    
    PongBall* ball = &board->gameBall;
    
    DrawCourt();
    DrawPaddles();
    MovePlayerPaddle();
    PongAI();
    RunBall();
    
    BallBoundaries();

    real32 colorSpeed = Length(ball->velocity) / (Length(ball->pos)*2);
    ball->color = Lerp(BLUE, RED, colorSpeed);
    SetTileColor(ball->pos, ball->color);

    DrawScores();
    EndGame();

    if (InputPressed(Keyboard, Input_Space) && gameOver == true) {
        MyMosaicInit();
    }
}











===================================================================================
  Math and Collision Code I did not end up using but still want to have access to
===================================================================================
 //If you de-comment this code, the program won't work because I screwed up the order of
 //Operations when I moved it down here. I also may have deleted some variables that it used originally
 //because I didn't need them for the current code.
 
 
inline vec2 CrossProductCompensator(vec2 a, vec2 b) {
    //I ended up not needing to use this but I am leaving it in so I can refrence it later if i Need it.
    //The function stores two halves of a vector's cross product equation into result.x and result.y
    //I did this so that I would be able to multiply vectors if I need to. 
    vec2 result;
    result.x = a.x * b.y;
    result.y = -(b.x * a.y);
    
    return result;
}

void AccelBall(){ //vec2 pos, vec2 velocity, int32 accel, int32 timeInterval) {
    PongBall* ball = &board->gameBall;

    vec2 updatedPos;
    vec2 updatedVelocity;
    int32 xDir;
    int32 yDir;

    float t = DeltaTime/2.0;

    updatedVelocity.x = ball->velocity.x + (ball->accel * t) *
                        (ball->velocity.x > 0 ? 1 : -1);

    updatedVelocity.y = ball->velocity.y + (ball->accel * t) *
        (ball->velocity.y > 0 ? 1 : -1);

    
    updatedPos.x = ball->pos.x + (t * ball->velocity.x) +
                   (ball->accel + (t*t) * .5);
    updatedPos.y = ball->pos.y + (t * ball->velocity.y) +
                   (ball->accel + (t*t) * .5);
    
    //ball->pos = updatedPos;
    ball->nPos = updatedPos - ball->pos;

    ball->velocity.x = updatedVelocity.x;
    ball->velocity.y = updatedVelocity.y;

}

void DetectionAlogrithm(vec2 a, vec2 b, vec2 c, vec2 d, int32 e, Paddle* paddle) {
    //The first few times i tried to use RectTest() i think i was using it wrong. I don't know if that is
    //something i could have used for this assignment or not
    
    PongBall* ball = &board->gameBall;
    CollisionData* intersect = &board->collision;
    
    //intersect->collided = false;
    intersect->sideHit = 1000;
    vec2 correction;

    vec2 q = (b - a);
    vec2 r = (d - c);
    vec2 s = (a - c);

    real32 qr = -Cross(q, r);
    real32 rs = -Cross(r, s);
    real32 qs = -Cross(q, s);
    
    if (qr != 0) {
        
        real32 u = rs / qr;
        
        if (u >= 0 && u <= 1) {
            
            real32 v = qs / qr;
            
            if (v >= 0 && v <= 1) {                  
                correction.x = a.x + (u * q.x);
                correction.y = a.y + (u * q.y);

               
                //intersect->collided = true;
                intersect->posCorrection = correction;
                //intersect->sideHit = e;
                paddle->hit = true;
                //ClearTiles(WHITE);
            }
        }
    }
    else {
        intersect->collided = false;
        paddle->hit = false;
    }
}

void CollidePaddleBall(Paddle* paddle, PongBall* ball) {
    CollisionData* intersect = &board->collision;
    if (ball->velocity.x <= 0) {
        paddle = &board->paddle[0];
    }
    else {
        paddle = &board->paddle[1];
    }

    float32 left = paddle->pos.x;
    float32 top = paddle->pos.y;
    float32 right = paddle->pos.x + paddle->scale.x - ball->size.x;
    float32 bottom = paddle->pos.y  + paddle->scale.y - ball->size.y;
    
    //Don't delete this yet this is to test for boundaries of paddle
    SetTileColor(left, top, RED);
    SetTileColor(left, bottom, YELLOW);
    SetTileColor(right, bottom, BLUE);
    SetTileColor(right, top, GREEN);
    ball->nPos = 

    //if (ball->velocity.x > 0) {
    //    //checking for collision with left side(s)
    //    DetectionAlogrithm(ball->pos, ball->nPos, V2(left, top), V2(left, bottom), 1, paddle);
    //}
    //else if (ball->velocity.x < 0) {
    //    //right side(s)
    //    DetectionAlogrithm(ball->pos, ball->nPos, V2(right, top), V2(right, bottom), 0, paddle);
    }
    if (paddle->hit == false) {
        if (ball->velocity.y > 0) {
            //bottom side(s)
            DetectionAlogrithm(ball->pos, ball->nPos, V2(left, bottom), V2(right, bottom), 3, paddle);
        }
        else if (ball->velocity.y < 0) {
            //top
            DetectionAlogrithm(ball->pos, ball->nPos, V2(left, top), V2(right, top), 2, paddle);
        }
    }
}

void PaddleBallResponse() {
    PongBall* ball = &board->gameBall;
    Boundaries* walls = &board->perimeter;
    CollisionData* intersect = &board->collision;
    Paddle* paddle;
    
    AccelBall();

    if ((ball->velocity.y > 0) && (ball->pos.y > ball->max.y)) {
        ball->pos.y = ball->max.y;
        ball->velocity.y *= -1;
    }
    else if ((ball->velocity.y < 0) && (ball->pos.y < ball->min.y)) {
        ball->pos.y = ball->min.y;
        ball->velocity.y *= -1;
    }

    CollidePaddleBall(paddle, ball);

    if (intersect->collided == true) {
        //ClearTiles(RED);
        switch (intersect->sideHit) {
            case 0:
            case 1:
                ball->pos.x = intersect->posCorrection.x;
                ball->velocity.x *= -1;
                //ClearTiles(GREEN);
                break;
            case 2:
            case 3:
                ball->pos.y = intersect->posCorrection.y;
                ball->velocity.y *= -1;
                //ClearTiles(YELLOW);
                break;
        }

        if (InputHeld(Keyboard, Input_UpArrow)){
            if (ball->velocity.y < 0) {
                ball->velocity.y *= 0.5;
            }
            else if (ball->velocity.y > 0) {
                ball->velocity.y *= 1.5;
            }
        }
        else if (InputHeld(Keyboard, Input_DownArrow)) {
            if (ball->velocity.y > 0) {
                ball->velocity.y *= 0.5;
            }
            else if (ball->velocity.y < 0) {
                ball->velocity.y *= 1.5;
            }
        }
    }
    intersect->collided = false;
}