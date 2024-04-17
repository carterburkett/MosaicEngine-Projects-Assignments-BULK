struct Player {
	vec2 tiles[4];
	vec2 pos;
	vec4 color;
	bool push;
};

struct TurnActions {

};

struct Box {};

struct Map {
	int16 height;
	int16 width;
	vec2 tilePos[256];
	vec4 tileColors;
};

struct LevelData {
	Player player;
	Map map;
};

LevelData data = {};

MemoryArena levelArena = {};

void GetSet_PlayerTiles() {
	for (int i = 0; i < sizeof(data.player.tiles); i++) {
		data.player.tiles[i] = V2(data.player.pos.x, data.player.pos.y - i);
	}
}

void InitPlayer() {
	data.player.pos = V2(1, 5);
	GetSet_PlayerTiles();
}

void MovePlayer() {
	Player* p = &data.player;
	
	if (InputPressed(Keyboard, Input_W) || InputPressed(Keyboard, Input_UpArrow)) {
		p->pos.y -= 1;

	}
	else if (InputPressed(Keyboard, Input_S) || InputPressed(Keyboard, Input_DownArrow)) {
		p->pos.y += 1;

	}
	else if (InputPressed(Keyboard, Input_A) || InputPressed(Keyboard, Input_LeftArrow)) {
		p->pos.x -= 1;

	}
	else if (InputPressed(Keyboard, Input_D) || InputPressed(Keyboard, Input_RightArrow)) {
		p->pos.x += 1;
	}

}


void DrawPlayer() {
	SetTileColor(data.player.pos.x, data.player.pos.y - 3, PASTEL_RED);
	SetTileColor(data.player.pos.x, data.player.pos.y - 2, BLUE);
	SetTileColor(data.player.pos.x, data.player.pos.y - 1, BLUE);
	SetTileColor(data.player.pos, BLACK);
}



void DrawLevelBounds() {
	for (int x = 0; x < data.map.width; x++) {
		for (int y = 0; y < data.map.height; y++) {
			(x == 0 || x == data.map.width - 1 || y == 0 || y == data.map.height - 1) ? SetTileColor(x, y, MEDBROWN) : SetTileColor(x, y, LITEBROWN);
		}
	
	}
}

void InitLevel() {}

void RenderLevel() {
//	ClearTiles(PASTEL_BLUE);
	DrawLevelBounds();
	DrawPlayer();
}

void MyMosaicInit() {
	data.map.width = 64;
	data.map.height = 36;
	SetMosaicGridSize(data.map.width, data.map.height);
	InitPlayer();
}

void MyMosaicUpdate() {
	MovePlayer();
	RenderLevel();
}