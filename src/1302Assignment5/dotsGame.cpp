//#include "my_mosaic.cpp"


enum PowerUpType { extraLife, speedUp, speedDown };
enum EnemyNames { Enemy_BlueR = 0, Enemy_BlueL = 1, Enemy_Red = 2, Enemy_Green = 3, Enemy_Yellow = 4, Enemy_Pink = 5 };

struct BossOptions {
    vec4 attackColor;
    vec2 swordTiles[8];

    int growDir;
    int growSpeed;

    int spinDir = 1;
    int spinSpeed = 8;
    int swordLength;

    bool alive;
    int health;

    vec2 size;
    vec2 minGrowSize;
    vec2 maxGrowSize;
    real32 eventTime = 0;

    bool startEvent;

    bool growing;
    bool swordAttack;
    bool bees;
};

struct Enemy {
    vec4 color;
    vec2 startPos;
    vec2 endPos;
    vec2 pos;
    EnemyNames type;
    bool alive;

    DynamicArray<vec2> bezPoints = {};
    int speed;
    int amp;

    BossOptions boss;

    real32 t = 0.0f;
    real32 startTime = 0.0f; //probably remove this
    real32 deathTime = 0.0f;
};

struct PowerUp {
    vec2 pos;
    vec4 color;

    PowerUpType type;
    real32 startTime;

    bool active;
};

struct EnemyTypes {
    Enemy blueRight;
    Enemy blueLeft;
    Enemy redBez;
    Enemy greenFollow;
    Enemy yellowTrig;
    Enemy pinkBastard;
};

struct Player {
    vec2 pos = V2(0);
    vec4 color;
    bool alive;
    int speed;
    int speedMod = 1;
    PowerUp status;
};

struct Bullet {
    vec2 pos;
    vec2 dir;
    int speed = 60;
    real32 t;
};

struct LevelData {
    Player player;
    DynamicArray<Bullet> playerBullets;

    EnemyTypes eType;
    DynamicArray<Enemy> opp;

    vec2 boardMax;
    int score;

    bool win;
    bool active;
    bool restart;
    bool paused;

    bool initBoss;
    bool bossFight = false;
};

LevelData* hwLevel = NULL;
MemoryArena arena = {};

int speed = 2;

bool dotsDone = false;
bool bossDone = false;


//===========================================================================
//                                  Math
//===========================================================================
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


void ListenSwordCollisions(vec2 recPos) {
    if (PointRectTest(MakeRect(hwLevel->player.pos, V2(1)), recPos)) {
        hwLevel->player.alive = false;
    }
    for (int i = 0; i < hwLevel->playerBullets.count; i++) {
        if (TilePositionsOverlap(recPos, hwLevel->playerBullets[i].pos)) { RemoveAtIndex(&hwLevel->playerBullets, i); }
    }
}
//===========================================================================
//                      Path Behavior
//===========================================================================
void DrawPointSlerp(vec2 start, vec2 end, vec4 color, real32 timer) {
    /*real32 elapsedTime = Time - timer;
    real32 rt = elapsedTime / duration;*/

    float ox = (start.x + end.x) / 2;
    float oy = (start.y + end.y) / 2;
    vec2 origin = V2(ox, oy);

    real32 rad = Distance(origin, start);

    float angle = Lerp(0, _2PI, timer);

    float xCPos = (rad * cosf(angle - (_PI / 2))) + origin.x;
    float yCPos = (rad * sinf(angle - (_PI / 2))) + origin.y;

    vec2 circPos = V2(xCPos, yCPos);
    ListenSwordCollisions(circPos);


    SetTileColor(circPos, color);
}

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

inline vec2 BezierByDegree_DynArray(DynamicArray<vec2>* arr, real32 time, int deg) {
    //@TODO have the degree be direclty tied to the number of points fed. instead of an input
    ASSERT((*arr).count > deg);

    DynamicArray<real32> t = MakeDynamicArray<real32>(&Game->frameMem, sizeof(real32));
    DynamicArray<real32> mt = MakeDynamicArray<real32>(&Game->frameMem, sizeof(real32));

    vec2 outValue = V2(0);

    t[0] = 1;
    mt[0] = 1;

    for (int i = 1; i <= deg; i++) {

        real32 tStor = pow(time, i);
        real32 mtStor = pow(1 - time, i);
        PushBack(&t, tStor);
        PushBack(&mt, mtStor);
    }

    InsertAtIndex(&t, 0, (real32)1);
    InsertAtIndex(&mt, 0, (real32)1);


    for (int i = 0; i <= deg; i++) {
        real32 tempDeg = deg;
        vec2 pieceOut = V2(0);

        if (i == 0 || i == deg) { tempDeg = 1; }

        pieceOut.x = mt[deg - i] * (*arr)[i].x * t[i] * tempDeg;
        pieceOut.y = mt[deg - i] * (*arr)[i].y * t[i] * tempDeg;
        outValue.x += pieceOut.x;
        outValue.y += pieceOut.y;

        //Print("DynArray output (%f, %f)", outValue.x, outValue.y);
    }
    return outValue;
}

void EnemeyPath_MakeRandomBezArray(DynamicArray<vec2>* arr, int deg, bool useEnds, vec2 startPos, vec2 endPos) {
    //@TODO should support dynArrays or lists
    DynamicArrayClear(arr);

    for (int i = 0; i <= deg; i++) {
        if (i == 0 || i == deg) {
            PushBack(arr, Vec2RandiRange(V2(0), hwLevel->boardMax));
        }
        else {
            PushBack(arr, Vec2RandiRange(V2(0, 5), hwLevel->boardMax - V2(0, 5)));
        }
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
            arr[i] = Vec2RandiRange(V2(0), hwLevel->boardMax); //this was an oversight board need to be square for this to work properly rn. That

            //clamp this to the bounds of the border with a variable instead of hard numbers
            //Doing this bc i never want the starting point to not be on the board. the tile should always be somewhat trackable
        }

        arr[i] = Vec2RandiRange(V2(-4), hwLevel->boardMax + V2(4));
        //Print("(%i, %i)", arr[i].x, arr[i].y);
    }
    if (useEnds) {
        arr[0] = startPos;
        arr[deg] = endPos;

        Print("RandomBez last element output (%i, %i)", arr[deg].x, arr[deg].y);
    }
}
void MakeRandomBez(vec2 arr[], int degree, bool useEnds = false) {
    MakeRandomBez(arr, degree, useEnds, V2(0), V2(0));
}

void EnemyPath_SinDownAttack(Enemy* enemy, float amplitude, float speed = 2.0f) {
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
        enemy->pos.x = (-cosf(elapsedTime * speed) * amplitude) + enemy->startPos.x;
        enemy->pos.y = Lerp(enemy->startPos.y, enemy->endPos.y, t);
    }
}

//===========================================================================
//                             Boss Behavior
//===========================================================================


void PinkBossGrow(Enemy* e) {
    real32 t;
    if (e->boss.startEvent) {
        e->boss.eventTime += (DeltaTime / e->boss.growSpeed) * e->boss.growDir;
        // e->boss.startEvent = false;
    }

    if (t > 1) { e->boss.growDir *= -1; }
    if (t < 0) { e->boss.growing = false; }

    if (t > 0 && t < 1) {
        e->boss.size = Lerp(e->boss.minGrowSize, e->boss.maxGrowSize, t);
    }
}

void PinkBossSword(Enemy* e, int swordLength) {
    if (e->boss.eventTime >= 1 || e->boss.eventTime < 0) {
        e->boss.spinDir++;
        e->t = 0;
        e->boss.eventTime = 0;
    }
    //e->boss.spinDir % 2 == 0 ? sT -= DeltaTime : sT += DeltaTime;

    e->t += Time - DeltaTime;
    e->boss.eventTime = (e->t / e->boss.spinSpeed);

    for (int i = 0; i <= swordLength; i++) {
        DrawPointSlerp(V2(e->pos.x - swordLength + i, e->pos.y), V2(e->pos.x + swordLength - i, e->pos.y), YELLOW, e->boss.eventTime);
    }
}


//===========================================================================
//                          Event Behavior
//===========================================================================
void CheckPlayerDeath() {
    Player* player = &hwLevel->player;
    //Need solution if adding boss
    for (int i = 0; i < hwLevel->opp.count; i++) {
        Enemy* e = &hwLevel->opp[i];
        if (e->alive && PointRectTest(MakeRect(player->pos, V2(1)), e->pos)) {
            player->alive = false;
        }
    }
}

void GameOver() {
    Player* player = &hwLevel->player;
    if (!player->alive) {
        Print("==========================================");
        Print("                PLAYER DIED");
        Print("==========================================");

        vec4 screenColor = V4(0.19f, 0.03f, 0.05f, 1.0f);
        ClearTiles(0.21, 0.02, 0.02);

        DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 32, WHITE, true, "YOU LOST...");
        DrawTextScreenPixel(&Game->serifFont, V2(800, 400), 20, WHITE, true, "Hit 'R' To Return to the Hub World. ESC to Quit");
    }
}

void LevelInputManager() {
    Player* player = &hwLevel->player;
    Bullet bullet;
    bullet.pos = player->pos;

    if (InputHeld(Keyboard, Input_W)) {
        player->pos.y -= DeltaTime * player->speed * player->speedMod;
    }
    if (InputHeld(Keyboard, Input_A)) {
        player->pos.x -= DeltaTime * player->speed * player->speedMod;
    }
    if (InputHeld(Keyboard, Input_D)) {
        player->pos.x += DeltaTime * player->speed * player->speedMod;
    }
    if (InputHeld(Keyboard, Input_S)) {
        player->pos.y += DeltaTime * player->speed * player->speedMod;
    }

    if (InputPressed(Keyboard, Input_UpArrow)) {
        bullet.dir = V2(0, -1);
        PushBack(&hwLevel->playerBullets, bullet);
    }
    else if (InputPressed(Keyboard, Input_DownArrow)) {
        bullet.dir = V2(0, 1);
        PushBack(&hwLevel->playerBullets, bullet);
    }
    else if (InputPressed(Keyboard, Input_RightArrow)) {
        bullet.dir = V2(1, 0);
        PushBack(&hwLevel->playerBullets, bullet);
    }
    else if (InputPressed(Keyboard, Input_LeftArrow)) {
        bullet.dir = V2(-1, 0);
        PushBack(&hwLevel->playerBullets, bullet);
    }

    player->pos.y = Clamp(player->pos.y, 0.0f, hwLevel->boardMax.x - 2);
    player->pos.x = Clamp(player->pos.x, 0.0f, hwLevel->boardMax.y - 2);
}

void BulletManager() {
    for (int i = 0; i < hwLevel->playerBullets.count; i++) {
        Bullet* bullet = &hwLevel->playerBullets[i];
        bullet->pos.x += DeltaTime * bullet->speed * bullet->dir.x;
        bullet->pos.y += DeltaTime * bullet->speed * bullet->dir.y;

        SetTileColor(bullet->pos, V4(0.61f, 0.39f, 0.60f, 1.0f));

        if (bullet->pos.x > hwLevel->boardMax.x || bullet->pos.x < 0 || bullet->pos.y > hwLevel->boardMax.y || bullet->pos.y < 0) {
            RemoveAtIndex(&hwLevel->playerBullets, i);
        }
    }
}

void ListenEnemyDeath() {
    for (int f = 0; f < hwLevel->opp.count; f++) {
        Enemy* e = &hwLevel->opp[f];

        for (int i = 0; i < hwLevel->playerBullets.count; i++) {
            Bullet* b = &hwLevel->playerBullets[i];

            if (e->type != Enemy_Pink && e->alive) {
                if (TilePositionsOverlap(e->pos, b->pos)) {
                    hwLevel->score++;
                    e->alive = false;
                    e->color = BLACK;
                    RemoveAtIndex(&hwLevel->playerBullets, i);
                }
            }
            else {
                if (PointRectTest(MakeRect(e->pos, V2(floor(e->boss.size.x / 2), ceil(e->boss.size.y / 2))), b->pos)) {
                    e->boss.health--;
                    RemoveAtIndex(&hwLevel->playerBullets, i);

                    if (e->boss.health <= 0) {
                        e->alive = false;
                        hwLevel->win = true;
                    }
                }
            }
        }

        if (!e->alive) {
            if (&e->bezPoints != nullptr && e->bezPoints.chunkCount > 0) { //Checking to make sure that bezPoints exists and has data stored within it.
                DeallocateDynamicArray(&e->bezPoints);
            }

            e->deathTime += DeltaTime;

            if (e->deathTime >= 3.0f) {
                RemoveAtIndex(&hwLevel->opp, f);
                Print("Index Size is now: %i", hwLevel->opp.count);

            } //This adds a makeshift buffer for the amount of Time 
        }
    }
}

//===========================================================================
//                          Create Enemies
//===========================================================================
void PlayerWin() {
    Player* player = &hwLevel->player;
    if (hwLevel->win) {
        Print("==========================================");
        Print("                PLAYER WON");
        Print("==========================================");

        vec4 screenColor = V4(0.03f, 0.19f, 0.05f, 1.0f);
        ClearTiles(screenColor);

        DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 32, WHITE, true, "YOU WIN!");
        DrawTextScreenPixel(&Game->serifFont, V2(800, 400), 20, WHITE, true, "Take the Key to return to the Hub World!");
    }
}

void DrawEnemies() {
    //This function could have been better but I'm running out of time. If next hw invovles this one fix this it's shitty.
    for (int i = 0; i < hwLevel->opp.count; i++) {
        Enemy* e = &hwLevel->opp[i];
        if (e->alive) {
            if (e->type == Enemy_BlueL) {
                e->t += DeltaTime / e->speed;

                if (e->t >= 1) {
                    e->t = 0;
                }
                e->pos = BezierByDegree_DynArray(&e->bezPoints, e->t, e->bezPoints.count - 1);
            }
            if (e->type == Enemy_BlueR) {
                e->t += DeltaTime / e->speed;

                if (e->t >= 1) {
                    e->t = 0;
                }
                e->pos = BezierByDegree_DynArray(&e->bezPoints, e->t, e->bezPoints.count - 1);
            }
            if (e->type == Enemy_Red) {
                e->t += DeltaTime / e->speed;

                if (e->t >= 1) {
                    e->t = 0;
                }
                e->pos = BezierByDegree_DynArray(&e->bezPoints, e->t, e->bezPoints.count - 1);
            }
            if (e->type == Enemy_Yellow) {
                EnemyPath_SinDownAttack(e, e->amp, e->speed);
            }
            if (e->type == Enemy_Green) {
                vec2 dir = DirFromPosToPlayer(e->pos, hwLevel->player.pos);

                e->pos.x += DeltaTime * e->speed * dir.x;
                e->pos.y += DeltaTime * e->speed * dir.y;
            }
            SetTileColor(e->pos, e->color);
        }
    }
}

void InitEnemies(EnemyNames enemy = Enemy_BlueL) {
    Enemy* blueR = &hwLevel->eType.blueRight;
    Enemy* blueL = &hwLevel->eType.blueLeft;
    Enemy* red = &hwLevel->eType.redBez;
    Enemy* green = &hwLevel->eType.greenFollow;
    Enemy* yellow = &hwLevel->eType.yellowTrig;
    Enemy* pink = &hwLevel->eType.pinkBastard;

    int boardMid = hwLevel->boardMax.x / 2;
    int redCorner = RandiRange(-1, 2);

    Enemy* e = new Enemy;

    switch (enemy) {
    case(Enemy_BlueL):
        blueL->alive = true;
        blueL->speed = 2.0;
        blueL->type = Enemy_BlueL;
        blueL->startPos = V2(0);
        blueL->endPos = V2(boardMid, hwLevel->boardMax.y);
        blueL->color = V4(0.14f, 0.42f, 0.84f, 1.0f);
        blueL->bezPoints = MakeDynamicArray<vec2>(&arena, 2);
        EnemeyPath_MakeRandomBezArray(&blueL->bezPoints, 3, true, blueL->startPos, blueL->endPos);

        e = blueL;
        PushBack(&hwLevel->opp, *e);
        break;

    case(Enemy_BlueR):
        blueR->alive = true;
        blueR->speed = 2.0;
        blueR->type = Enemy_BlueR;
        blueR->startPos = V2(hwLevel->boardMax.x - 1, 0);
        blueR->endPos = V2(boardMid, hwLevel->boardMax.y);
        blueR->color = V4(0.14f, 0.42f, 0.84f, 1.0f);
        blueR->bezPoints = MakeDynamicArray<vec2>(&arena, 2);
        EnemeyPath_MakeRandomBezArray(&blueR->bezPoints, 3, true, blueR->startPos, blueR->endPos);

        e = blueR;
        PushBack(&hwLevel->opp, *e);
        break;

    case(Enemy_Red):
        red->alive = true;
        red->speed = 9.0;
        red->type = Enemy_Red;
        red->color = RED;
        red->startPos = Vec2RandiRange(V2(0), V2(hwLevel->boardMax.x, 5));
        red->endPos = redCorner == 0 ? Vec2RandiRange(V2(0, hwLevel->boardMax.y - 5), V2(5, hwLevel->boardMax.y)) :
            Vec2RandiRange(V2(hwLevel->boardMax.x - 5, hwLevel->boardMax.y - 5), V2(hwLevel->boardMax.x, hwLevel->boardMax.y));
        red->bezPoints = MakeDynamicArray<vec2>(&arena, 2);
        EnemeyPath_MakeRandomBezArray(&red->bezPoints, RandiRange(3, 7), true, red->startPos, red->endPos);

        e = red;
        PushBack(&hwLevel->opp, *e);
        break;

    case(Enemy_Green):
        green->alive = true;
        green->type = Enemy_Green;
        green->color = GREEN;
        green->speed = 15;
        green->startPos = Vec2RandiRange(V2(0), V2(hwLevel->boardMax.x, 4));

        e = green;
        PushBack(&hwLevel->opp, *e);
        break;

    case(Enemy_Yellow):
        yellow->alive = true;
        yellow->startTime = Time;
        yellow->speed = 4.0f;
        yellow->amp = 10.0f;
        yellow->type = Enemy_Yellow;
        yellow->color = PASTEL_YELLOW;
        yellow->startPos = V2(boardMid, 0);
        yellow->endPos = V2(boardMid, hwLevel->boardMax.y);

        e = yellow;
        PushBack(&hwLevel->opp, *e);
        break;

    case(Enemy_Pink):
        pink->alive = true;
        pink->boss.spinSpeed = 2.5;
        pink->boss.spinDir = 1;
        pink->startTime = Time;
        pink->speed = 2.0f;
        pink->type = Enemy_Pink;
        pink->color = PASTEL_RED;
        pink->pos = V2(boardMid, hwLevel->boardMax.y / 3);
        pink->boss.size = V2(7);
        pink->boss.health = 100;
        pink->boss.eventTime = 0;
        pink->boss.swordLength = 10;

        e = pink;
        PushBack(&hwLevel->opp, *e);
        break;

    default:
        Print("Defualt Reached: Error has occured");
        break;
    }
}

void EnemyInstanceHandler() {
    int spawnChance = RandiRange(0, 10);

    //This seems like a bad way to do this but If i want to add more enemies later it makes life a little easier
    switch (spawnChance) {
    case 0:
    case 1:
        InitEnemies(Enemy_BlueL);
        ClearTiles(PASTEL_BLUE);
        break;
    case 2:
    case 3:
        InitEnemies(Enemy_BlueR);
        ClearTiles(PASTEL_BLUE);
        break;
    case 4:
    case 5:
    case 6:
        InitEnemies(Enemy_Red);
        ClearTiles(PASTEL_RED);
        break;
    case 7:
    case 8:
        InitEnemies(Enemy_Yellow);
        ClearTiles(PASTEL_YELLOW);
        break;
    case 9:
        InitEnemies(Enemy_Green);
        ClearTiles(GREEN);
        break;
    }

    Print("EnemySpawned");
}


//===========================================================================
//                     Misc. Initiates & Game Update
//===========================================================================
void InitLevelMemory() {
    Mosaic->myData = malloc(sizeof(LevelData));
    memset(Mosaic->myData, 0, sizeof(LevelData));
    hwLevel = (LevelData*)Mosaic->myData;

    AllocateMemoryArena(&arena, Megabytes(1));
    hwLevel->playerBullets = MakeDynamicArray<Bullet>(&arena, 2);
    hwLevel->opp = MakeDynamicArray<Enemy>(&arena, 2);
}

void InitPlayer() {
    Player* player = &hwLevel->player;
    player->speed = 25; //move this to an init or something
    player->pos = V2(hwLevel->boardMax.x / 2, hwLevel->boardMax.y);
    player->color = V4(0.58f, 0.25f, 0.84f, 1.0f);
    player->alive = true;
    player->speedMod = 1;
}


bool bossRunning = false;

void DotsGameInit() {
    bossDone = false;
    dotsDone = false;
    InitLevelMemory();
    hwLevel->boardMax = V2(40);
    hwLevel->score = 0;
    hwLevel->initBoss = false; //@TODO SET THIS TO FLASE B4 SUBMIT
    hwLevel->bossFight = false;

    InitPlayer();

    InitEnemies(Enemy_BlueL);
    InitEnemies(Enemy_BlueR);
    InitEnemies(Enemy_Red);
    InitEnemies(Enemy_Green);
    

    bossRunning = false;
    SetMosaicGridSize(hwLevel->boardMax.x, hwLevel->boardMax.y);
}


void DotsGameUpdate(bool bossFight) {
    ClearTiles(BLACK);
    PlayerWin();

    if (hwLevel->player.alive) {
        DrawTextTile(V2(0, 0), 0.3f, WHITE, "Your score is: %i", hwLevel->score);

        if (bossFight && !bossRunning) {
            bossRunning = true;
            hwLevel->initBoss = true;
        }

        if (hwLevel->bossFight) {
            for (int i = 0; i < hwLevel->opp.count; i++) {
                Enemy* e = &hwLevel->opp[i];
                if (e->alive) {
                    if (e->type == Enemy_Pink) {
                        SetBlockColor(e->pos.x - floor(e->boss.size.x / 2), e->pos.y - floor(e->boss.size.x / 2), e->boss.size.x, e->boss.size.y, e->color);

                        if (e->boss.health <= 25) {
                            e->boss.size = V2(2);
                            e->boss.swordLength = 6;
                            e->speed = 10;
                            PinkBossSword(e, e->boss.swordLength);

                        }
                        else if (e->boss.health < 40) {
                            e->speed = 6;
                            e->boss.size = V2(3);
                            e->boss.swordLength = 8;
                            PinkBossSword(e, e->boss.swordLength);
                        }
                        else if (e->boss.health <= 60) {
                            e->speed = 4;
                            e->boss.size = V2(5);
                            PinkBossSword(e, e->boss.swordLength);

                        }
                        else if (e->boss.health <= 80) {
                            e->speed = 4;
                        }

                        vec2 dir = DirFromPosToPlayer(e->pos, hwLevel->player.pos);

                        e->pos.x += DeltaTime * e->speed * dir.x;
                        e->pos.y += DeltaTime * e->speed * dir.y;
                    }
                    else {
                        bossDone = true;
                        hwLevel->win = true;
                        DynamicArrayClear(&hwLevel->opp);
                        DynamicArrayClear(&hwLevel->playerBullets);
                        e->alive = false;
                    }
                }

                else { bossDone = true; }
            }
        }
        else {
            if (hwLevel->opp.count < 4) {
                EnemyInstanceHandler();
            }

            DrawEnemies();

            if (hwLevel->score >= 9) {
                dotsDone = true;
                hwLevel->bossFight = true;
                hwLevel->win = true;
                DynamicArrayClear(&hwLevel->opp);
                DynamicArrayClear(&hwLevel->playerBullets);
            }
        }
        LevelInputManager();
        BulletManager();
        ListenEnemyDeath();
        CheckPlayerDeath();

        SetBlockColor(hwLevel->player.pos, 2, 2, hwLevel->player.color);
    }

    if (!hwLevel->player.alive) { GameOver(); }

    if (hwLevel->initBoss) {
        Print("InitiatingBoss");
        DynamicArrayClear(&hwLevel->opp);
        DynamicArrayClear(&hwLevel->playerBullets);
        InitEnemies(Enemy_Pink);
        hwLevel->bossFight = true;
        hwLevel->initBoss = false;
    }

    if (InputPressed(Keyboard, Input_R)) { 
        dotsDone = true; 
        bossDone = true;
    }
    if (hwLevel->restart) { DotsGameInit(); }
    
}
