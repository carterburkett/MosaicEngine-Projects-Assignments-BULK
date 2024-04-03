

const int16 gridWidth = 36;
const int16 gridHeight = 24;
const int16 totalTiles = gridWidth * gridHeight;

vec2 targetHitBox[4] = { V2(100,100) };
vec2 poopLoc = V2(100, 100);
vec2 crossSpawn;
vec2 crossPos[5] = { V2(100,100) };
vec2 cursorPos = V2(0);
vec2 clickedPos = V2(0);

int16 growthFactor = NULL;
int16 crossCount = 0;
real32 growTimer = 0;

bool headCol = false;
bool drawCross = false;
bool shoulderCol = false;
bool pooCollided = false;

bool startGrow = false;
bool mouseLeftClicked = false;
bool mouseLeftHeld = false;
vec2 pooTargetPos = V2(10, 16);


real32 SlerpTimer;
real32 colorTimer;
int16 colorCount = 1;
int16 modeCount = 0;

real32 colorIncrement = 0.0f;

float xPos = 1.0f;
float yPos = 11.0f;

bool birdDone = false;

struct LevelTiles {
	//@INFO I obviously should have stored a tiles struct inside of the image struct but here we are
	vec4 color[totalTiles];
	vec2 tilePos[totalTiles];
	real32 startTime[totalTiles];
	real32 duration[totalTiles];
};
LevelTiles* level = { nullptr };

vec2 tilePos;

void InitBirdGame() {
	SetMosaicGridSize(36, 24);

	level = (LevelTiles*)malloc(sizeof(LevelTiles));
	memset(level, 0, sizeof(LevelTiles));

	crossCount = 0;
	int16 count = 0;

	for (int w = 0; w < gridWidth; w++) {
		for (int h = 0; h < gridHeight; h++) {
			level->tilePos[count] = V2(w, h);
			count++;
		}
	}

	for (int i = 0; i < totalTiles; i++) {
		level->startTime[i] = 0;
		level->color[i] = V4(.5f, .5f, .9f, 1.0f);
	}
}

void DrawThreeBG() {
	for (int i = 0; i < totalTiles; i++) {
		if (level->tilePos[i].y >= 20) {
			level->color[i] = GREEN;
		}
		SetTileColor(level->tilePos[i], level->color[i]);
	}
}

void CheckCursor() {
	cursorPos = V2(GetMousePositionX() + 1, GetMousePositionY());
	if (InputPressed(Mouse, Input_MouseLeft)) {
		clickedPos = cursorPos;
	}
}

void DrawBird() {
	CheckCursor();
	SetBlockColor(cursorPos - V2(4, 1), 4, 3, BLACK);
	SetBlockColor(cursorPos - V2(5, 0), 4, 1, RED);
	SetTileColor(cursorPos - V2(0, 0), YELLOW);

	if (InputPressed(Mouse, Input_MouseLeft)) {
		poopLoc = cursorPos - V2(3, -2);
	}

	if (poopLoc.y < 26 && !pooCollided) { //Move the poo
		poopLoc.y += DeltaTime * 15;
		SetTileColor(poopLoc, WHITE);
		if (pooCollided) {
			poopLoc = V2(100, 100);
		}
	}
	else { poopLoc = V2(100, 100); }
}

void DrawCross() {
	for (int i = 0; i < crossCount; i++) {
		crossPos[crossCount - 1] = crossSpawn;
		vec4 GRAVECROSS = V4(.3f, .3f, 0.0f, 1.0f); //Temporary until transfer
		SetBlockColor(crossPos[i], 1, 5, GRAVECROSS);
		SetBlockColor(crossPos[i] - V2(1, -1), 3, 1, GRAVECROSS);
	}
}

void PooCollision() {
	headCol = (TilePositionsOverlap(poopLoc + V2(0, 1), pooTargetPos - V2(-1, 3)) ||
		TilePositionsOverlap(poopLoc + V2(0, 1), pooTargetPos - V2(0, 3))) ? true : false;

	shoulderCol = (TilePositionsOverlap(poopLoc + V2(0, 1), pooTargetPos - V2(-2, 1)) ||
		TilePositionsOverlap(poopLoc + V2(0, 1), pooTargetPos - V2(1, 1))) ? true : false;

	pooCollided = (headCol || shoulderCol) ? true : false;

	if (pooCollided) {
		crossSpawn = pooTargetPos;
		crossCount++;
	}
}


void DrawPoopTarget() {
	SetBlockColor(pooTargetPos - V2(0, 0), 2, 5, PASTEL_ORANGE);
	SetBlockColor(pooTargetPos - V2(0, 2), 2, 2, PASTEL_ORANGE);
	SetBlockColor(pooTargetPos - V2(1, 0), 1, 3, PASTEL_ORANGE);
	SetBlockColor(pooTargetPos - V2(-2, 0), 1, 3, PASTEL_ORANGE);

	pooTargetPos.x = PingPong(Time * 10.0f, 30);

}

void RunBirdGame() {
	if (crossCount == 10) {
		crossCount = 10;
		DrawThreeBG();
		DrawBird();
		PooCollision();
		DrawCross();
		DrawTextTop(RED, "You killed the farmer and his family. Click R to return to the hub...");
		birdDone = true;
	}
	else {
		DrawTextTop(GOLD, "Move your mouse and click the LMB to poop on the people!");
		DrawThreeBG();
		DrawPoopTarget();
		DrawBird();
		PooCollision();
		DrawCross();
	}
}