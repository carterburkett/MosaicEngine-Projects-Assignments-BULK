enum PowerUp { extraLife, speedUp, speedDown };
enum EnemyNames { Enemy_BlueR = 0, Enemy_BlueL = 1, Enemy_Red = 2, Enemy_Green = 3, Enemy_Yellow = 4, Enemy_Pink = 5 };

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

    real32 t;
    real32 startTime; //probably remove this
    real32 deathTime;
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
};

LevelData* hwLevel = NULL;
MemoryArena arena = {};

real32 t;
int speed = 2;

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

//I haven't actually fixed my bezbydeg function to handle DA's yet so this can't be higher than 8
void EnemeyPath_MakeRandomBezArray(DynamicArray<vec2>* arr, int deg, bool useEnds, vec2 startPos, vec2 endPos) {
    //@TODO should support dynArrays or lists
    DynamicArrayClear(arr);

    for (int i = 0; i <= deg; i++) {
        if (i == 0 || i == deg) {
            PushBack(arr, Vec2RandiRange(V2(0), hwLevel->boardMax));
        }

        PushBack(arr, Vec2RandiRange(V2(-4), hwLevel->boardMax + V2(4)));
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
        Print("(%i, %i)", arr[i].x, arr[i].y);
    }
    if (useEnds) {
        arr[0] = startPos;
        arr[deg] = endPos;
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

void CheckPlayerDeath() {
    Player* player = &hwLevel->player;
    //Need solution if adding boss
    for (int i = 0; i < hwLevel->opp.count; i++) {
        Enemy* e = &hwLevel->opp[i];
        if (e->alive && PointRectTest(MakeRect(player->pos, V2(2)), e->pos)) {
            player->alive = false;
        }
    }
}

void GameOver() {
    Player* player = &hwLevel->player;
    if (!player->alive) {
        real32 t, dir;
        if (t < 0.2f) {
            dir = 1.0f;
        }
        else if (t > 1.0f) {
            dir = -1.0f;
        }

        t += (DeltaTime / 8.0f) * dir;


        vec4 screenColor = V4(0.19f, 0.03f, 0.05f, t);


        ClearTiles(0.21, 0.02, 0.02);

        DrawTextScreenPixel(&Game->serifFont, V2(800, 300), 32, WHITE, true, "YOU LOSE...");
        DrawTextScreen(&Game->serifFont, V2(800, 700), 32, WHITE, true, "Hit 'R' To Play Again or 'ESC' to Quit");
    }
}

void LevelInputManager() {
    Player* player = &hwLevel->player;
    Bullet bullet;
    bullet.pos = player->pos;
    if (InputPressed(Keyboard, Input_R)) {
        hwLevel->restart = true;
    }
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

            if (TilePositionsOverlap(e->pos, b->pos)) {
                if (e->alive) { hwLevel->score++; }

                e->alive = false;
                e->color = BLACK;

            }
        }

        if (!e->alive) {
            e->deathTime += DeltaTime;

            if (e->deathTime >= 5.0f) { RemoveAtIndex(&hwLevel->opp, f); } //This adds a makeshift buffer for the amount of Time 
            //in b/w enemy respawns so it still feels like there's some reward to killing enemies and
        }
    }
}



void DrawEnemies() {
    //This function could have been better but I'm running out of time. If next hw invovles this one fix this it's shitty.
    for (int i = 0; i < hwLevel->opp.count; i++) {
        Enemy* e = &hwLevel->opp[i];

        if (e->type == Enemy_BlueL) {
            e->t += DeltaTime / speed;

            if (e->t >= 1) {
                e->t = 0;
            }

            e->pos = BezierByDegree_DynArray(&e->bezPoints, e->t, e->bezPoints.count - 1);
        }

        if (e->type == Enemy_BlueR) {
            e->t += DeltaTime / speed;

            if (e->t >= 1) {
                e->t = 0;
            }

            e->pos = BezierByDegree_DynArray(&e->bezPoints, e->t, e->bezPoints.count - 1);
        }

        if (e->type == Enemy_Red) {
            e->t += DeltaTime / speed;

            if (e->t >= 1) {
                e->t = 0;
                //MakeRandomBez(testArr, 3, true, V2(0), hwLevel->boardMax);
            }

            e->pos = BezierByDegree_DynArray(&e->bezPoints, e->t, e->bezPoints.count - 1);
        }

        if (e->type == Enemy_Yellow) {
            EnemyPath_SinDownAttack(e, e->amp, e->speed);
        }

        if (e->type == Enemy_Green) {
            vec2 dir = DirFromPosToPlayer(e->pos, hwLevel->player.pos);
            Print("Green Direction is: %f, %f", dir.x, dir.y);

            e->pos.x += DeltaTime * e->speed * dir.x;
            e->pos.y += DeltaTime * e->speed * dir.y;
        }

        Print("Drawing Enemy\n pos is (%f, %f)", e->pos.x, e->pos.y);

        SetTileColor(e->pos, e->color);
    }
}

void InitEnemies(EnemyNames enemy = Enemy_BlueL) {
    Enemy* blueR = &hwLevel->eType.blueRight;
    Enemy* blueL = &hwLevel->eType.blueLeft;
    Enemy* red = &hwLevel->eType.redBez;
    Enemy* green = &hwLevel->eType.greenFollow;
    Enemy* yellow = &hwLevel->eType.yellowTrig;

    int boardMid = hwLevel->boardMax.x / 2;
    int redCorner = RandiRange(0, 1);

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

        PushBack(&hwLevel->opp, *blueL);
        break;

    case(Enemy_BlueR):
        blueR->alive = true;
        blueL->speed = 2.0;
        blueR->type = Enemy_BlueR;
        blueR->startPos = V2(hwLevel->boardMax.x, 0);
        blueR->endPos = V2(boardMid, hwLevel->boardMax.y);
        blueR->color = V4(0.14f, 0.42f, 0.84f, 1.0f);
        blueR->bezPoints = MakeDynamicArray<vec2>(&arena, 2);
        EnemeyPath_MakeRandomBezArray(&blueR->bezPoints, 3, true, blueR->startPos, blueR->endPos);

        PushBack(&hwLevel->opp, *blueR);
        break;

    case(Enemy_Red):
        red->alive = true;
        red->speed = 1.0;
        red->type = Enemy_Red;
        red->color = RED;
        red->startPos = Vec2RandiRange(V2(0), V2(hwLevel->boardMax.x, 6));
        red->endPos = redCorner == 0 ? Vec2RandiRange(V2(0), V2(5)) :
            Vec2RandiRange(V2(hwLevel->boardMax.x, 0), V2(hwLevel->boardMax.x - 5, 0));

        red->bezPoints = MakeDynamicArray<vec2>(&arena, 2);
        EnemeyPath_MakeRandomBezArray(&red->bezPoints, 5, false);
        PushBack(&hwLevel->opp, *red);

        break;
    case(Enemy_Green):
        green->alive = true;
        green->type = Enemy_Green;
        green->color = GREEN;
        green->speed = 15;
        green->startPos = Vec2RandiRange(V2(0), V2(hwLevel->boardMax.x, 4));
        PushBack(&hwLevel->opp, *green);
        break;
    case(Enemy_Yellow):
        yellow->startTime = Time;
        yellow->speed = 4.0f;
        yellow->amp = 10.0f;
        yellow->alive = true;
        yellow->type = Enemy_Yellow;
        yellow->color = PASTEL_YELLOW;
        yellow->startPos = V2(boardMid, 0);
        yellow->endPos = V2(boardMid, hwLevel->boardMax.y);

        PushBack(&hwLevel->opp, *yellow);
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
}
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

void MyMosaicInit() {
    InitLevelMemory();
    hwLevel->boardMax = V2(40);
    hwLevel->score = 0;

    InitPlayer();

    InitEnemies(Enemy_BlueL);
    InitEnemies(Enemy_BlueR);
    InitEnemies(Enemy_Red);
    InitEnemies(Enemy_Green);

    SetMosaicGridSize(hwLevel->boardMax.x, hwLevel->boardMax.y);
}

void MyMosaicUpdate() {
    if (hwLevel->score <= 15) {
        DrawTextTile(V2(0, 0), 0.3f, WHITE, "Your score is: %i", hwLevel->score);
        DrawEnemies();

        LevelInputManager();
        BulletManager();
        ListenEnemyDeath();
        CheckPlayerDeath();

        SetBlockColor(hwLevel->player.pos, 2, 2, hwLevel->player.color);

        if (hwLevel->opp.count < 4) {
            EnemyInstanceHandler();
        }
    }
    if (!hwLevel->player.alive) { GameOver(); }
    if (hwLevel->score >= 15) {
        ClearTiles(BLACK);
        DrawTextTile(V2(20, 20), 2.0f, WHITE, "You Win!");


    }
    if (hwLevel->restart) { MyMosaicInit(); }
}
