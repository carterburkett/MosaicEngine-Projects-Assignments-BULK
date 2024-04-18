// @TODO: separate level and runtime state
// @TODO: acceleration grid structure
// @TODO: undo
// @TODO: inheritance

enum EntityType {
    EntityType_Entity,
    EntityType_Player,
    EntityType_Wall,
    EntityType_Box,

    EntityType_Count,
};

struct Entity {
    EntityType type;
    vec2 position;
    vec3 color;
    bool inactive;
};

// Player inherits from Entity, which means that Player just has all the contents of Entity
// copy-pasted into the beginning of its definition.
// When we inherit from a type we can use that type as a generic pointer to anything
// that has inherited from it.
struct Player : Entity {

};

struct Wall : Entity {

};

struct Box : Entity {

};

// offline version of data
struct LevelDefinition {
    int32 width;
    int32 height;

    vec2 playerPosition;
    DynamicArray<vec2> walls;
    DynamicArray<vec2> boxs;
};

struct LevelCell {
    Entity* entity;
};

struct LevelGrid {
    int32 width;
    int32 height;

    int32 cellCount;
    LevelCell* cells;
};

// runtime version of data
struct LevelState {
    LevelGrid grid;

    Player player;
    DynamicArray<Wall> walls;
    DynamicArray<Box> boxes;
};

enum LevelName {
    LevelName_Intro,
    LevelName_Count,
};

struct GameState {
    MemoryArena permanentArena;
    MemoryArena levelArena;

    LevelState activeLevel;

    LevelDefinition levels[LevelName_Count];
};

GameState GS = {};

void GenerateLevel1(LevelDefinition* level) {
    level->width = 48;
    level->height = 27;
    level->walls = MakeDynamicArray<vec2>(&GS.permanentArena, 64);

    {
        level->playerPosition = V2(5, 5);
    }

    {
        PushBack(&level->walls, V2(0, 0));
    }
}

void GenerateLevel2(LevelDefinition* level) {
    level->width = 16;
    level->height = 9;
    level->walls = MakeDynamicArray<vec2>(&GS.permanentArena, 64);

    {
        level->playerPosition = V2(5, 5);
    }

    {
        PushBack(&level->walls, V2(0, 0));
    }
}



int32 GetCellIndex(LevelGrid* grid, vec2 position) {
    // the pitch of our array of cells is the grid's width
    int32 index = (int32)position.x + ((int32)position.y * grid->width);
    return index;
}

LevelCell* GetCell(LevelGrid* grid, vec2 position) {
    int32 index = GetCellIndex(grid, position);

    if (index < 0 || index >= grid->cellCount) { return NULL; }

    return &grid->cells[index];
}

void AddEntityToCell(LevelGrid* grid, Entity* entity) {
    LevelCell* cell = GetCell(grid, entity->position);
    cell->entity = entity;
}

void DrawPlayer() {
    LevelState* level = &GS.activeLevel;
    Player* p = &level->player;

	SetTileColor(p->position.x, p->position.y - 3, GREEN);
	SetTileColor(p->position.x, p->position.y - 2, BLUE);
	SetTileColor(p->position.x, p->position.y - 1, BLUE);
	SetTileColor(p->position, BLACK);
}

void MovePlayer() {
    LevelState* level = &GS.activeLevel;
    Player* p = &level->player;
	
	if (InputPressed(Keyboard, Input_W) || InputPressed(Keyboard, Input_UpArrow)) {
        p->position.y -= 1;
	}
	else if (InputPressed(Keyboard, Input_S) || InputPressed(Keyboard, Input_DownArrow)) {
        p->position.y += 1;
	}
	else if (InputPressed(Keyboard, Input_A) || InputPressed(Keyboard, Input_LeftArrow)) {
        p->position.x -= 1;
	}
	else if (InputPressed(Keyboard, Input_D) || InputPressed(Keyboard, Input_RightArrow)) {
        p->position.x += 1;
	}
}

void DrawLevelBounds(int16 width, int16 height) {
	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			x == 0 || x == width - 1 || y == 0 || y == height - 1 ? SetTileColor(x, y, MEDBROWN) : SetTileColor(x, y, LITEBROWN);
		}
	}
}

void SpawnLevel(LevelDefinition* definition) {
    MemoryArena* arena = &GS.levelArena;
    ClearMemoryArena(arena);

    LevelState* level = &GS.activeLevel;
    LevelGrid* grid = &level->grid;

    grid->width = definition->width;
    grid->height = definition->height;

    SetMosaicGridSize(grid->width, grid->height);

    grid->cellCount = grid->width * grid->height;
    grid->cells = PushArray(arena, LevelCell, grid->cellCount);

    for (int i = 0; i < grid->cellCount; i++) {
        grid->cells[i] = {};
    }

    level->walls = MakeDynamicArray<Wall>(arena, definition->walls.count);

    {
        Player* player = &level->player;
        player->type = EntityType_Player;
        player->color = V3(1, 1, 1);
        player->position = definition->playerPosition;

        AddEntityToCell(grid, player);
    }

    for (int i = 0; i < definition->walls.count; i++) {

        // Wall wall = {};
        // wall.type = EntityType_Wall;
        // wall.position = definition->walls[i];
        // wall.color = V3(0.35f, 0.35f, 0.4f);

        // int32 index = PushBack(&level->walls, wall);
        // AddEntityToCell(grid, &level->walls[index]);

        Wall* wall = PushBackPtr(&level->walls);
        wall->type = EntityType_Wall;
        wall->position = definition->walls[i];
        wall->color = V3(0.35f, 0.35f, 0.4f);
        AddEntityToCell(grid, wall);
    }
}

void RenderLevel(LevelState* level) {
    DrawLevelBounds(GS.activeLevel.grid.width, GS.activeLevel.grid.height);
    SetTileColor(level->player.position, V4(level->player.color, 1.0f));
    //DrawPlayer();
    for (int i = 0; i < level->walls.count; i++) {
        SetTileColor(level->walls[i].position,
            V4(level->walls[i].color, 1.0f));
    }
}

void MyMosaicInit() {
    AllocateMemoryArena(&GS.levelArena, Megabytes(8));
    AllocateMemoryArena(&GS.permanentArena, Megabytes(8));

    GenerateLevel1(&GS.levels[LevelName_Intro]);

    SpawnLevel(&GS.levels[LevelName_Intro]);
}

void MyMosaicUpdate() {
    MovePlayer();
    RenderLevel(&GS.activeLevel);
}



//#define IN_EDITOR 
//
//#if IN_EDITOR 0
//#include "mosaic.cpp"
//#endif
//
//struct Player {
//	vec2 tiles[4];
//	vec2 pos;
//	vec4 color;
//	bool push;
//};
//
//struct TurnActions {
//
//};
//
//struct Box {
//
//
//};
//
//struct Wall {
//	vec2 start;
//	vec2 end;
//	vec4 color;
//};
//
//struct Map {
//	int height = 64;
//	int width = 36;
//	vec2 tilePos[256];
//	vec4 tileColors;
//};
//
//struct LevelData {
//	Player player;
//	Map map;
//	Wall walls[10];
//};
//
//LevelData* data = {};
//MemoryArena levelArena = {};
//
//void GetSet_PlayerTiles() {
//	for (int i = 0; i < sizeof(data->player.tiles); i++) {
//		data->player.tiles[i] = V2(data->player.pos.x, data->player.pos.y - i);
//	}
//}
//
//void InitPlayer() {
//	data->player.pos = V2(1, 8);
//	GetSet_PlayerTiles();
//}
//
//void MovePlayer() {
//	Player* p = &data->player;
//	
//	if (InputPressed(Keyboard, Input_W) || InputPressed(Keyboard, Input_UpArrow)) {
//		p->pos.y -= 1;
//	}
//	else if (InputPressed(Keyboard, Input_S) || InputPressed(Keyboard, Input_DownArrow)) {
//		p->pos.y += 1;
//	}
//	else if (InputPressed(Keyboard, Input_A) || InputPressed(Keyboard, Input_LeftArrow)) {
//		p->pos.x -= 1;
//
//	}
//	else if (InputPressed(Keyboard, Input_D) || InputPressed(Keyboard, Input_RightArrow)) {
//		p->pos.x += 1;
//	}
//}
//
//
//void DrawPlayer() {
//	SetTileColor(data->player.pos.x, data->player.pos.y - 3, GREEN);
//	SetTileColor(data->player.pos.x, data->player.pos.y - 2, BLUE);
//	SetTileColor(data->player.pos.x, data->player.pos.y - 1, BLUE);
//	SetTileColor(data->player.pos, BLACK);
//}
//
//void DrawLevelBounds() {
//	for (int x = 0; x < 64; x++) {
//		for (int y = 0; y < 36; y++) {
//			x == 0 || x == 64 - 1 || y == 0 || y == 36 - 1 ? SetTileColor(x, y, MEDBROWN) : SetTileColor(x, y, LITEBROWN);
//		}
//	}
//}
//
//void DrawWall(vec2 start, vec2 end, vec4 color) {
//	for (int x = start.x; x < end.x; x++){
//		for (int y = start.y; x < end.y; y++) {
//			SetTileColor(x, y, MEDBROWN);
//		}
//	}
//}
//
//void AddWall(vec2 start, vec2 end) {
//	for (int i = 0; i < sizeof(data->walls); i++) {
//		if (&data->walls[i].start != nullptr) { DrawWall(data->walls[i].start, data->walls[i].end, data->walls[i].color); }
//	}
//
//	Wall w = new Wall;
//	w.start = start;
//	w.end = end;
//
//	memse
//}
//
//void InitLevel() {
//	SetTileColor(V2(0), RED);
//	data->walls[0].start = V2(0, 5);
//	data->walls[0].end = V2(0, 7);
//
//	
//}
//
//void RenderLevel() {
//	ClearTiles(PASTEL_BLUE);
//	DrawLevelBounds();
//	for (int i = 0; i < sizeof(data->walls); i++) {
//		if (&data->walls[i].start != nullptr) { DrawWall(data->walls[i].start, data->walls[i].end, data->walls[i].color); }
//	}
//	DrawPlayer();
//}
//void MyMosaicInit() {
//	Mosaic->myData = malloc(sizeof(LevelData));
//	memset(Mosaic->myData, 0, sizeof(LevelData));
//	data = (LevelData*)Mosaic->myData;
//
//	data->map.width = 64;
//	data->map.height = 36;
//	SetMosaicGridSize(data->map.width, data->map.height);
//	InitPlayer();
//	DrawLevelBounds();
//}
//
//void MyMosaicUpdate() {
//	MovePlayer();
//	RenderLevel();
//	Print("%f", data->player.pos.x);
//}