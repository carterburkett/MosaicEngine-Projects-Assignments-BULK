//@TODO reset difficulty on restart
//@TODO take end sounds of if statements
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
    real32 speed;
    bool hit; 
};

struct PongBall {
    vec4 color;
    vec2 pos;
    vec2 nPos;
    vec2 lastPos;
    vec2 velocity;
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
};


BoardData* board = NULL;

//I know the audio stuff should probably be in it's own struct or processed through DynamicArrays but I'm new to the way you set up Mosaic to handle Audio
//and this gets the job done. @INFO
SoundClip hitSound;
SoundClip scoreSound;
SoundClip bgMusic;
SoundClip endSound;

SoundHandle handle;
SoundHandle endHandle;

bool gameOver;

real32 difficultyMod = 0;
real32 difficultyLevel = 2;
real32 ballStartSpeed = 55.0f;
real32 maxBallSpeed = 70.0f;

float paddleColorTime = 0.0f;
real32 paddleSpeed = 100.0f;


//=========================================================================
//                              Audio
//=========================================================================
    void PlayEndSound() {
        Sound* sound = GetSound(&Game->audioPlayer, handle);
        sound->volume = 0.0f;
        if (endHandle.generation == 0) {
            endHandle = PlaySound(&Game->audioPlayer, endSound, 1.0f, false);
        }
    }

    void PlayBGMusic() {
        if (gameOver == false && handle.generation == 0) {
            handle = PlaySound(&Game->audioPlayer, bgMusic, .3f, true);
        }
    }

    void PlayHitSound() {
        PlaySound(&Game->audioPlayer, hitSound, 0.75f, false);
    }

    void PlayScoreSound() {
        ScoreCount* score;
        for (int i = 0; i < 2; i++) {
            score = &board->scores[i];
        }
        if (score->count < 11) {
            PlaySound(&Game->audioPlayer, scoreSound, .7f, false);
        }
    }

//=========================================================================
//                   Initializing Components and Math
//=========================================================================
Rect MakeBetterRect(vec2 position, vec2 halfSize) {
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
        board->paddle[i].speed = 80.0f;
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
        //if - again?
    }

    board->gameBall.pos = V2(60.0f, randY);
    board->gameBall.velocity = V2(xDir, yDir) * ballStartSpeed;
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
        ClearTiles(PASTEL_BLUE);
        DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 32, WHITE, true, "YOU WIN!");
        DrawTextScreenPixel(&Game->serifFont, V2(800, 700), 32, WHITE, true, "Hit Space To Play Again or ESC to Quit");

        LoadSoundClip("data/sfx/Pong/Pong_Success_01.wav", &endSound);
        PlayEndSound();

    }
    if (compScore->count >= 11) {
        playTime = DeltaTime;
        ball->velocity = V2(0);
        ball->pos = V2(0);

        gameOver = true;
        ClearTiles(PASTEL_RED);
        DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 32, WHITE, true, "YOU LOSE...");
        DrawTextScreenPixel(&Game->serifFont, V2(800, 700), 32, WHITE, true, "Hit Space To Play Again or ESC to Quit");

        LoadSoundClip("data/sfx/Pong/Pong_Failure_01.wav", &endSound);
        PlayEndSound();
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
void ResolveClip(Paddle* paddle) {
    PongBall* ball = &board->gameBall;

    float32 left = paddle->pos.x - paddle->min.x;
    float32 top = paddle->pos.y - paddle->min.y;
    float32 right = paddle->pos.x + paddle->min.x - ball->size.x;
    float32 bottom = paddle->pos.y + paddle->min.y - ball->size.y;
    
    paddle->pos = paddle->pos; //freezing the paddle until the clip has been resolved

    //Checking Corner Clip Cases and changing velocity to not destroy game feel/player feedback
    if (TilePositionsOverlap(V2(right, top), ball->pos) || TilePositionsOverlap(V2(left, top), ball->pos)) {
        if (ball->velocity.y > 0) {
            ball->velocity.y *= -1;
        }
    }
    else if (TilePositionsOverlap(V2(right, bottom), ball->pos) || TilePositionsOverlap(V2(left, bottom), ball->pos)) {
        if (ball->velocity.y < 0) {
            ball->velocity.y *= -1;
        }
    }

    if (ball->lastPos.x <= 116 && ball->lastPos.x >= 3) { //making sure the ball isn't somehow rebounding from the endzone
        if (ball->pos.x == 115 || ball->pos.x == 4) {
            ball->pos.x = ball->lastPos.x;
            ball->velocity.x *= -1;
        }
        else if(ball->pos.x == 3) { //if it's stuck in the end-side of the paddle i need to correct it's pos to be out of the paddle not just to the lastPos
            ball->pos.x = 5;
            ball->velocity.x *= -1;
        }
        else if(ball->pos.x == 116) {
            ball->pos.x = 114;
            ball->velocity.x *= -1;
        }
    }
}

void DetectCollisions() {
    PongBall* ball = &board->gameBall;

    for (int i = 0; i < 2; i++) {
        Paddle* paddle = &board->paddle[i];
        
        float32 left = paddle->pos.x - paddle->min.x;
        float32 top = paddle->pos.y - paddle->min.y;
        float32 right = paddle->pos.x + paddle->min.x -ball->size.x;
        float32 bottom = paddle->pos.y + paddle->min.y - ball->size.y;

        SetTileColor(left, top, RED);
        SetTileColor(left, bottom, YELLOW);
        SetTileColor(right, bottom, BLUE);
        SetTileColor(right, top, GREEN);
        SetTileColor(paddle->pos, MAGENTA);

        ball->nPos = ball->pos + (DeltaTime * ball->velocity);
        ball->lastPos = ball->pos - (DeltaTime * ball->velocity);

        //@TODO account for cases where the paddle isn't moving
        if (PointRectTest(MakeBetterRect(paddle->pos, V2(1, 3)), ball->pos)) {
        //              if (TestPointAABB(ball->pos, V2(left, bottom), V2(top, right))) {
            ResolveClip(paddle); //checking to make sure the ball isn't already inside the paddle before checking the nPos. So we can resolve it if true
            PlayHitSound();
            break;
        }
        else if (PointRectTest(MakeBetterRect(paddle->pos, V2(1,3)), ball->nPos) ) {
        //          else if (TestPointAABB(ball->nPos, V2(left, bottom), V2(top, right))) {
            paddle->pos = paddle->pos; //freezing paddle for the collision
            paddle->hit = true;
            ball->velocity.x *= -1;
            PlayHitSound();

            if (paddle->speed > 0) {
                if (ball->velocity.y < 0) {
                    ball->velocity.y *= 1.5;
                }
                else if(ball->velocity.y == 0) {
                    ball->velocity.y = ballStartSpeed;
                }
                else if (ball->velocity.y > 0) {
                    if (TilePositionsOverlap(V2(right, top), ball->nPos) || TilePositionsOverlap(V2(left, top), ball->nPos) || TilePositionsOverlap(V2(right, top), ball->pos) || TilePositionsOverlap(V2(left, top), ball->pos)) {
                        //Corner Behavior                                               There should not be a situation where the ball is inside the paddle at this point but I'm leaving the case here just in case.
                        ball->velocity.y *= -1.5;
                        ball->velocity.x *= 1.5;

                    }
                    else {
                        ball->velocity.x *= 1.5;
                    }
                }
            }
            else if (paddle->speed < 0) {
                if (ball->velocity.y > 0) {
                    ball->velocity.y *= 1.5; //This would increase the velocity it the paddle hits the ball WITH it's momentum
                }

                else if (ball->velocity.y == 0) {
                    ball->velocity.y = ballStartSpeed;
                }

                else if (ball->velocity.y < 0) {
                    if (TilePositionsOverlap(V2(right, bottom), ball->nPos) || TilePositionsOverlap(V2(left, bottom), ball->nPos) || TilePositionsOverlap(V2(right, bottom), ball->pos) || TilePositionsOverlap(V2(left, bottom), ball->pos)) {
                        //Corner Behavior
                        ball->velocity.y *= -1.5;
                        ball->velocity.x *= 1.5;

                    }
                    else {
                        ball->velocity.x *= 1.5;
                    }
                }
            }
            else {
                //Corner Behavior
                if (TilePositionsOverlap(V2(right, top), ball->nPos) || TilePositionsOverlap(V2(left, top), ball->nPos)) {
                    if (ball->velocity.y > 0) {
                        ball->velocity.y *= -1;
                    }
                }
                else if (TilePositionsOverlap(V2(right, bottom), ball->nPos) || TilePositionsOverlap(V2(left, bottom), ball->nPos)) {
                    if (ball->velocity.y < 0) {
                        ball->velocity.y *= -1;
                    }
                }
            }
            if (ball->pos.x == 3 || ball->pos.x == 4) { ball->pos.x = 5; }
            if (ball->pos.x == 116 || ball->pos.x == 115) { ball->pos.x = 114; }
        }
    }
}

void BallBoundaries() {  //@TODO need to calculate nPos here as well so the ball doesn't glitch into the paddle on certain corners and hitboxes
    PongBall* ball = &board->gameBall;
    ScoreCount* player = &board->scores[0];
    ScoreCount* comp = &board->scores[1];

    { //nPos edge case
        if ((ball->velocity.x > 0) && (ball->nPos.x > ball->max.x)) {
            ball->nPos.x = ball->max.x;
        }
        else if ((ball->velocity.x < 0) && (ball->nPos.x < ball->min.x)) {
            ball->nPos.x = ball->min.x;
        }

        if ((ball->velocity.y > 0) && (ball->nPos.y > ball->max.y)) {
            ball->nPos.y = ball->max.y;
        }
        else if ((ball->velocity.y < 0) && (ball->nPos.y < ball->min.y)) {
            ball->nPos.y = ball->min.y;
        }
    }


    if ((ball->velocity.x > 0) && (ball->pos.x > ball->max.x)) { //@Q am i switching x dir here? i'm very confused why am i switching x dir on wall impact, and why does the game not switch???
        ball->pos.x = ball->max.x;
        ball->nPos.x = ball->pos.x + 1;

        InitBall();
        player->count++;

        LoadSoundClip("data/sfx/Pong/Player_Score_01.wav", &scoreSound);
        PlayScoreSound();

    }
    else if ((ball->velocity.x < 0) && (ball->pos.x < ball->min.x)) {
        ball->pos.x = ball->min.x;
        ball->nPos.x = ball->pos.x - 1;

        InitBall();
        comp->count++;

        LoadSoundClip("data/sfx/Pong/AI_Score_01.wav", &scoreSound);
        PlayScoreSound();
    }


    if ((ball->velocity.y > 0) && (ball->pos.y > ball->max.y)) {
        ball->pos.y = ball->max.y;
        ball->nPos.y = ball->pos.y - 1;

        ball->velocity.y *= -1;
        
    }
    else if ((ball->velocity.y < 0) && (ball->pos.y < ball->min.y)) {
        ball->pos.y = ball->min.y;
        ball->nPos.y = ball->pos.y + 1;

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

    if (playerScore->count > difficultyLevel && playerScore->count < 10) { //@TODO Fix starting difficulty the AI is brain dead in the beginning
        difficultyMod += 5;
        difficultyLevel += 2;
    }

    if (ball->velocity.x > 0) {
        if ((paddle->pos.y < ball->pos.y) && (paddle->pos.y < 76)) {
            paddle->speed = 100;
            paddle->pos.y += DeltaTime * ( (paddle->speed/2) + difficultyMod);
        }
        if ((paddle->pos.y > ball->pos.y) && (paddle->pos.y > 11)) {
            paddle->speed = -100;
            paddle->pos.y += DeltaTime * ( (paddle->speed/2) - difficultyMod);
        }
        if ((paddle->pos.y == ball->pos.y) && (paddle->pos.y < 76) && (paddle->pos.y > 11)) {
            paddle->pos.y = paddle->pos.y;
            paddle->speed = 0;
        }
    }
}

void MovePlayerPaddle() {
    PongBall* ball = &board->gameBall;
    Paddle* playerPaddle = &board->paddle[0];

    if (InputHeld(Keyboard, Input_UpArrow) && (playerPaddle->pos.y > 11)) {
        playerPaddle->speed = -100;
        playerPaddle->pos.y += playerPaddle->speed * DeltaTime;
    }

    if (InputHeld(Keyboard, Input_DownArrow) && (playerPaddle->pos.y < 76)) {
        playerPaddle->speed = 100;
        playerPaddle->pos.y += playerPaddle->speed * DeltaTime;
    }
    else {
        playerPaddle->speed = 0;
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

    LoadSoundClip("data/sfx/Pong/BassLine_Game44100.wav", &bgMusic);
    LoadSoundClip("data/sfx/Pong/paddleHit_01.wav", &hitSound);
    Sound* sound = GetSound(&Game->audioPlayer, handle);
    sound->volume = 1.0f;

    board->perimeter.min = V2(0.0, 6.0);
    board->perimeter.max = V2(119.0, 79.0);
    board->perimeter.midLine = 119 / 2;
    board->perimeter.scale = V2(79, 119);

    difficultyMod = 0;
    gameOver = false;
    InitPaddles();
    InitBall();
    InitScores();
}

void MyMosaicUpdate() {
    ClearTiles(BLACK);
    PongBall* ball = &board->gameBall;

    PlayBGMusic();

    DrawCourt();
    DrawPaddles();
    MovePlayerPaddle();
    PongAI();
    RunBall();
    
    BallBoundaries();

    real32 colorSpeed = Length(ball->velocity) / (Length(ball->pos)*2);
    
    if (ball->pos.x < 2 || ball->pos.x > 116) { //I did this this way because I noticed the Ball felt like it was clipping through the paddle when it barely passed by the paddle on a diagonal
                                                //The better thing to do would be to change the score zones but This accomplishes the same thing for less work.
        ball->color = BLACK;
    }
    else {
        ball->color = Lerp(BLUE, RED, colorSpeed);
    }

    SetTileColor(ball->pos, ball->color);

    DrawScores();
    EndGame();

    if (InputPressed(Keyboard, Input_Space) && gameOver == true) {
        MyMosaicInit();
    }
}