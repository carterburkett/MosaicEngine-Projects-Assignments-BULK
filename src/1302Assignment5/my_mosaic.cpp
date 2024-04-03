#include "birdGame_1302.cpp"
#include "dotsGame.cpp"
#include "CloudMaker.cpp"

struct EventAccess {
    bool doorLocked;

    bool key;
    bool keyInWorld;
    vec2 keySpawn;

    bool firstTime;
    vec2 doorLoc;
    vec2 doorScale;
};

struct EventData {
    EventAccess hubEvent[4] = {NULL};

    bool npcAlive;
    bool npcSpeaking;
    int keyCount;
};

struct OverWorld {
    int width;
    int height;

    vec2 npcPos;
    vec2 playerPos;//this should all be in a player struct be we're here now
    vec2 playerSpeed;
    int jumpMax = 0;
    bool airborne;
};

OverWorld overworld;
EventData* owEvents;


float speedStorage = NULL;
int activeLevel = 0;
int jumpActive = 0;
int tempPos = -1;
bool isInitCall = true;

void InitMemory() {
    Mosaic->myData = malloc(sizeof(EventData));
    memset(Mosaic->myData, 0, sizeof(EventData));
    owEvents = (EventData*)Mosaic->myData;
}

void InitEvents() {
    //I am VERY well aware of the fact this is a horrible way to run this function but it started using pointers and then i changed it for simplicity's sake
        //The assignment is already past due I just want to get in and this code, while poorly optimized does clearly work :)
    EventAccess hub;
    EventAccess dots;
    EventAccess bird;
    EventAccess boss;

    boss.doorLocked = true;
    boss.key = false;
    boss.firstTime = true;
    boss.doorLoc = V2(48, overworld.height - 10);
    boss.keyInWorld = false;

    bird = boss;
    bird.doorLoc = V2(59, overworld.height - 10);

    dots = boss;
    dots.doorLoc = V2(35, overworld.height - 10);
    dots.keyInWorld = true;

    hub.firstTime = true;
    hub.key = true;
    hub.doorLocked = false;


    owEvents->hubEvent[0] = hub;
    owEvents->hubEvent[1] = dots;
    owEvents->hubEvent[2] = boss;
    owEvents->hubEvent[3] = bird;

    owEvents->npcSpeaking = false;
    owEvents->npcAlive = true;
    owEvents->keyCount = 0;  //if you make an inventory put this in there.
}

void DrawKey(vec2 pos, vec4 color) {
    SetBlockColor(pos, 3, 3, color);
    SetBlockColor(pos + V2(1, 0), 1, 7, color);
    SetTileColor(pos + V2(2, 6), color);
    SetTileColor(pos + V2(2, 4), color);
}

void KeyHandler(vec2 pos, vec4 color, int keyIndex) {
    if (keyIndex < 4 && keyIndex > 0) {
        if (owEvents->hubEvent[keyIndex].keyInWorld) {
            vec2 playerMin = overworld.playerPos - V2(1, 2);
            vec2 playerMax = overworld.playerPos + V2(3, 3);
            
            if (activeLevel == 1 || activeLevel == 2) {
                playerMin = hwLevel->player.pos;
                playerMax = hwLevel->player.pos + V2(2);
            }
            
            DrawKey(pos, color);

            if (TestAABBAABB(playerMin, playerMax, pos, pos + V2(3, 7))) {
                owEvents->hubEvent[keyIndex].key = true;
                owEvents->hubEvent[keyIndex].keyInWorld = false;
                activeLevel = 0;
            }
        }
    }
}

void DoorEvents() {
    vec2 playerMin = overworld.playerPos - V2(1, 2);
    vec2 playerMax = overworld.playerPos + V2(3, 3);

    for (int i = 1; i < 4; i++) {
        EventAccess e = owEvents->hubEvent[i];
        if (TestAABBAABB(playerMin, playerMax, e.doorLoc, e.doorLoc + V2(4, 7))) {
            if (i == 1) { 
                if (e.key) {
                    DrawTextTop(RED, 2, "Press W or Up to challenge the Game!"); 
                    e.doorLocked = false;
                }
                else { DrawTextTop(RED, 2, "That's the dot minigame! Grab that key off the ground to play it!"); }

            }
            else if (i == 2) { 
                if (e.key) { 
                    DrawTextTop(RED, 2, "The Boss is behind that door. Don't get too close! He has a sword."); 
                    e.doorLocked = false;
                }
                else { DrawTextTop(RED, 2, "The Boss is behind that door. You don't have the key though..."); }

            }
            else if (i == 3) { 
                
                if (e.key) { 
                    DrawTextTop(RED, 4, "Please don't open that. That's where the birds are..."); 
                    e.doorLocked = false;
                }
                else { DrawTextTop(RED, 2, "Don't worry about that door. There's nothing interesting behind it."); }
            }

            if (!e.doorLocked && (InputPressed(Keyboard, Input_W) ||(InputPressed(Keyboard,Input_UpArrow)))) {
                if (i == 1) { DotsGameInit(); }
                else if (i == 2) { DotsGameInit(); }
                else if (i == 3) { InitBirdGame(); }
                activeLevel = i;
            }
        }
    }
}

void NPCMessage(char* message) {
    vec2 playerMin = overworld.playerPos - V2(1,2);
    vec2 playerMax = overworld.playerPos + V2(3,3);
    vec2 npcMin = overworld.npcPos - V2(1,2);
    vec2 npcMax = overworld.npcPos + V2(3);

    if (TestAABBAABB(playerMin, playerMax, npcMin, npcMax)) {
        DrawTextTop(RED, 2, message);
    }
}

void DrawNPC() {
    vec4 farmerSkin;
    if (!owEvents->npcAlive) { farmerSkin = RGBHex(2c5e25); }
    else {farmerSkin = PASTEL_ORANGE;}

    SetBlockColor(overworld.npcPos - V2(0, 0), 2, 5, BLUE);
    SetBlockColor(overworld.npcPos - V2(0, 2), 2, 2, farmerSkin);
    SetBlockColor(overworld.npcPos - V2(1, 0), 1, 3, farmerSkin);
    SetBlockColor(overworld.npcPos - V2(-2, 0), 1, 3, farmerSkin);

    SetTileColor(overworld.npcPos-V2(1,2), RED);
}

void JumpPlayer() {
    if (overworld.airborne) {
        if (overworld.playerPos.y >= overworld.jumpMax && jumpActive == 1) { 
            if (speedStorage == NULL) { speedStorage = overworld.playerSpeed.x; }

            overworld.playerPos.y -= DeltaTime * overworld.playerSpeed.y; 
            overworld.playerSpeed.x = speedStorage * 0.5;
            
            if (overworld.playerPos.y <= overworld.jumpMax) {
                jumpActive = 0;
            }
        }

        if (jumpActive == 0) { 
            overworld.playerPos.y += DeltaTime * overworld.playerSpeed.y; 
            if (overworld.playerPos.y >= 9) { 
                overworld.playerSpeed.x = speedStorage;
                overworld.airborne = false; 
            }
        }
    }
}

void MovePlayer() {
    if (InputHeld(Keyboard, Input_D) || InputHeld(Keyboard, Input_RightArrow)) {
        overworld.playerPos.x +=  DeltaTime * overworld.playerSpeed.x;
    }
    else if (InputHeld(Keyboard, Input_A) || InputHeld(Keyboard, Input_LeftArrow)) {
        overworld.playerPos.x -=  DeltaTime * overworld.playerSpeed.x;
    }

    if (InputPressed(Keyboard, Input_Space) && !overworld.airborne) {
        jumpActive = 1;
        overworld.airborne = true;
    }

    JumpPlayer();

    overworld.playerPos.x = Clamp(overworld.playerPos.x, 1.0f, 61.0f);
}

void DrawPlayer() {
    SetBlockColor(overworld.playerPos - V2(0, 0), 2, 5, RED);
    SetBlockColor(overworld.playerPos - V2(0, 2), 2, 2, LITEBROWN);
    SetBlockColor(overworld.playerPos - V2(1, 0), 1, 3, LITEBROWN);
    SetBlockColor(overworld.playerPos - V2(-2, 0), 1, 3, LITEBROWN);

    SetTileColor(overworld.playerPos - V2(1, 2), RED);
}

void DrawOverWorld() {    
    for (int x = 0; x < overworld.width; x++) {
        for (int y = 0; y < overworld.height; y++) {
            if (y < overworld.height - 3) { SetTileColor(x, y, SKYBLUE); }
            else { SetTileColor(x, y, GRASSY_GREEN); }
        }
    }
    if (cloudsRunning == false) {
        for (int i = 0; i < 6; i++) {
            (clouds[i])->startTime = Time;
        }
        cloudsRunning = true;
    }

    MoveCloud();
    KeyHandler(V2(17, 6), RGBHex(ffa200), 0);
    for (int i = 1; i < 4; i++) {
        SetBlockColor(owEvents->hubEvent[i].doorLoc, 4, 8, BROWN);
    }
}

void MyMosaicInit() {
    overworld.width = 64;
    overworld.height = 16;

    SetMosaicGridSize(overworld.width, overworld.height);
    InitMemory();
    InitEvents();

    overworld.npcPos = V2(11, 9);
    overworld.playerPos = V2(2, 9);
    overworld.playerSpeed = V2(15, 13);
    overworld.airborne = false;
    overworld.jumpMax = 0;

    InitClouds();//Maybe set this up in an init for overworld only
}

void MyMosaicUpdate() {
    if (activeLevel == 0) {
        SetMosaicGridSize(overworld.width, overworld.height);
        owEvents->npcAlive ? NPCMessage("Hi! Beat the doors to get the keys!") : NPCMessage("*Zombie Sounds*");
        DoorEvents();
        MovePlayer();
        DrawOverWorld();
        DrawNPC();
        DrawPlayer();
        KeyHandler(V2(18, 9), GOLD, 1);
    }
    else if (activeLevel == 1) {
        DotsGameUpdate(false);
        Print("Dots RUnning");
        if (dotsDone) {
            if (&hwLevel->player.alive) {
                owEvents->hubEvent[2].keyInWorld = true;
                KeyHandler(V2(16, 19), GOLD, 2);
            }

            if (InputPressed(Keyboard, Input_R)) { activeLevel = 0; }
        }
        else {
            DrawTextTop(RED, 2, "Use WASD to Move and Arrow Keys to Shoot!");
        }
    }
    else if (activeLevel == 2) {
        DotsGameUpdate(true);
        if (bossDone) {
            if (&hwLevel->player.alive) {
                owEvents->hubEvent[3].keyInWorld = true;
                KeyHandler(V2(16,19), GOLD, 3);
            }

            if (InputPressed(Keyboard, Input_R)) { activeLevel = 0; }
        }
        else {
            DrawTextTop(RED, 2, "Use WASD to Move and Arrow Keys to Shoot!");
        }
    }
    else if (activeLevel == 3) {
        RunBirdGame();
        if (birdDone) {
            owEvents->npcAlive = false;
            if (InputPressed(Keyboard, Input_R)) { activeLevel = 0; }
        }
    }
}
