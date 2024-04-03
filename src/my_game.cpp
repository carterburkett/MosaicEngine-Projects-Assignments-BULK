//@IMPORTANT YOU CHANGED screen settings in GAMEINIT() and Config.txt to make screen work better
// Do not forget this when you go to hw for other Mosaic based classes. Original values were
// Config: 1600x900
// Init: 16 & 9 for cam->w/h 

const uint32 ServerAddress = MakeAddressIPv4(192, 168, 1, 78);
const uint16 ServerPort = 30000;
const int32 PacketID = Hash("NetworkSimple");

enum FlightPattern {bezier, linear, sinWave}; 

struct ClientInfo {
	uint32 address;
	uint16 port;
	int16 id;
	real32 pingTime;
	bool connected;
};

struct ServerInfo { //Do I need this if I'm not port fwding??
	uint32 address;
	uint16 port;
};

struct Bullet {
	vec2 pos;
	bool running = true;
	real32 startTime;
	int16 fromID = -1;
	int indexID;
};

struct InputPacket {
	vec2 position;
	int16 playerID;

	bool gunActive;
	DynamicArray <Bullet> playerBullets = {};
};

struct ClientData {
	vec2 pos; //try using this for the "inputPacket"
	real32 pingTime;
	bool connected;
	bool playing;
	bool isPlayer;
	int16 id = -1;

	Bullet primaryWeapon;
};

struct GameSprites {
	Sprite playerBullet;
	Sprite allyBullet;
	Sprite enemyBullet;

	Sprite playerShip;
	Sprite allyShip;
	Sprite enemy[3];
};

struct NPCData {
	//NPCType type;
	FlightPattern attackStyle;
	
	vec2 startPos; //this will be first array item for pathing
	vec2 currentPos;
	vec2 targetPos;	//last pathing item, this is not followed. Just stamped
	vec2 bezPoints[4]; //Fix this for scalability later

	int16 degBez;
	bool isTypeBez;
	bool alive;
	
	real32 startTime = 0;
	real32 timeSinceLastAttack;
	real32 lifeTime;
};

struct EnemyTypes {
	NPCData ratthewLeft;
	NPCData ratthewRight;
	NPCData sinEnemy;
	NPCData slowEnemy;
	NPCData randomBezEnemy;
};

struct Server {
	DynamicArray <ClientInfo> clients = {};
	int16 clientCount;
	DynamicArray <int16> assignedIDs = {}; //Make ID's a random number between 1 and 1000. Compare the proposed assignation to those stored in the array and then reroll if taken. Assign if not. Or serialize it. IDK

	int16 maxEnemies = 8;
};

struct GameData {
	bool gameActive;
	
	ClientData player;
	Server server;
		
	GameSprites sprites;
	EnemyTypes enemyTypes;
	
	DynamicArray <NPCData> enemies = {};
	DynamicArray <Bullet> playerBullets = {};
	Bullet allyBullet;
	
	SoundClip sound;
	DynamicArray<vec2> posToDraw = {};
	DynamicArray<Bullet> bulletToDraw = {};
};

ClientData player = {};

real32 tempStartTime = 0.0f; //DO NOT LEAVE THIS HERE
real32 tempStartTime2 = 0.0f; //DO NOT LEAVE THIS HERE

GameData hwData = {};
MemoryArena arena = {};
MemoryArena drawArena = {};
MemoryArena serverArena = {};

//=======================================================================================
//									Pathing & Positioning
//=======================================================================================
vec2 MoveTowards(vec2 current, vec2 target, float speed = 2.0f) {
	vec2 outPos = V2(0);
	real32 t = speed * DeltaTime;

	outPos.x = Lerp(current.x, target.x, t);
	outPos.y = Lerp(current.y, target.y, t);

	return outPos;
} //This is not finished Yet

void NormalizeCursor(vec2* mPos) {
	mPos->x = mPos->x * (Game->camera.width * Game->camera.size * 0.5f);
	mPos->y = mPos->y * (Game->camera.height * Game->camera.size * 0.5f);
	mPos->x = mPos->x + Game->camera.position.x;
	mPos->y = mPos->y + Game->camera.position.y;
}

void EnemyPath_BezAttack(NPCData* enemy, float speed = 2.0f) {
	int deg = enemy->degBez;
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
		enemy->currentPos = BezierByDegree(enemy->bezPoints, t, deg); 
		NormalizeCursor(&enemy->currentPos);
	}
}

void EnemyPath_SinAttack(NPCData* enemy, float amplitude, float speed = 2.0f) {
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
		NormalizeCursor(&enemy->currentPos);

		enemy->currentPos.x = -cosf(elapsedTime * speed) * amplitude;
		enemy->currentPos.y = Lerp(enemy->startPos.y, enemy->targetPos.y, t);
		NormalizeCursor(&enemy->currentPos);

	}
}

void MoveBullet(Bullet* bullet, float speed, int dir, int index = NULL) {
	real32 elapsedTime = Time - bullet->startTime;
	real32 t = elapsedTime / (speed / 2);

	bullet->pos.y = Lerp(bullet->pos.y, bullet->pos.y + (dir * speed), t);
	NormalizeCursor(&bullet->pos);

	if (t > 1 || bullet->pos.y > 1) {
		bullet->running = false;
	}

	GamePacket bulletPacket = {};
	bulletPacket.id = PacketID;
	bulletPacket.type = GamePacketType_ClientProjectile;
	
	

	memcpy(bulletPacket.data, bullet, sizeof(Bullet));
	PushBack(&Game->networkInfo.packetsToSend, bulletPacket);
	//if (IS_SERVER) { DrawSprite(bullet->pos, V2(0.5f, 0.5f), &hwData.sprites.allyShip); } //this is temp for debugging
}

//===================================================================================
//									GamePlay
//===================================================================================
void PlayerBullet() {
	//@TODO add another weapon type with different behavior and allow for identifying params
	/*GamePacket bulletPacket = {};
	bulletPacket.id = PacketID;
	bulletPacket.type = GamePacketType_ClientProjectile;*/

	if (InputPressed(Mouse, Input_MouseLeft) && !IS_SERVER) {
		Bullet* bullet = new Bullet;
		bullet = &player.primaryWeapon;

		bullet->running = true;
		bullet->pos = hwData.player.pos;
		bullet->startTime = Time;

		PushBack(&hwData.playerBullets, *bullet);
	}
	
	for (int i = 0; i < hwData.playerBullets.count; i++) {
		Bullet b = hwData.playerBullets[i];
		/*if (b.pos.y > 1) {
			RemoveAtIndex(&hwData.playerBullets, i);
		}*/

		if (!IS_SERVER) {
			hwData.playerBullets[i].indexID = i;
			hwData.playerBullets[i].fromID = hwData.player.id;
			//Print("Bullet is from %i", hwData.playerBullets[i].fromID);
		}

		if (b.running) {
			MoveBullet(&b, 2, 1, i);
			DrawSprite(b.pos, V2(0.5f, 0.5f), &hwData.sprites.playerBullet);
		}
		//else { RemoveAtIndex(&hwData.playerBullets, i); }

		/*if (!IS_SERVER) {
			hwData.playerBullets[i].indexID = i;
			hwData.playerBullets[i].fromID = hwData.player.id;
			Print("Bullet is from %i", hwData.playerBullets[i].fromID);
		}*/

		//memcpy(bulletPacket.data, &hwData.playerBullets[i], sizeof(Bullet));
		//PushBack(&Game->networkInfo.packetsToSend, bulletPacket);

	}
	if (hwData.playerBullets.count > 20) {
		RemoveAtIndex(&hwData.playerBullets, 0);
	}
}

void DrawEnemies() {
	for (int i = 0; i < hwData.enemies.count; i++) {
		NPCData* e = &hwData.enemies[i];

		if (e->attackStyle == bezier) {
			EnemyPath_BezAttack(e, 4);
			DrawSprite(e->currentPos, V2(0.75f), &hwData.sprites.enemy[0]);
		}
		if (e->attackStyle == sinWave) {
			EnemyPath_SinAttack(e, .25, 9);
			DrawSprite(e->currentPos, V2(0.75f), &hwData.sprites.enemy[0]);
		}
	}
}

//===================================================================================
//								GameInit
//===================================================================================
void InitEnemies() {
	//=========RatthewLeft
	NPCData ratthewLeft = hwData.enemyTypes.ratthewLeft;
	ratthewLeft.bezPoints[0] = V2(-1, 1);
	ratthewLeft.bezPoints[1] = V2(4, -.6f);
	ratthewLeft.bezPoints[2] = V2(-3, .85f);
	ratthewLeft.bezPoints[3] = V2(0, -1);

	ratthewLeft.startPos = V2(-1, 1);
	ratthewLeft.targetPos = V2(0, -1);
	ratthewLeft.degBez = 3;
	ratthewLeft.isTypeBez = true;
	ratthewLeft.attackStyle = bezier;
	PushBack(&hwData.enemies, ratthewLeft);


	//======RatthewRight
	NPCData ratthewRight = hwData.enemyTypes.ratthewRight;
	ratthewRight.bezPoints[0] = V2(1, 1);
	ratthewRight.bezPoints[1] = V2(-4, -.6f);
	ratthewRight.bezPoints[2] = V2(3, .85f);
	ratthewRight.bezPoints[3] = V2(0, -1);

	ratthewRight.startPos = hwData.enemyTypes.ratthewRight.bezPoints[0];
	ratthewRight.targetPos = hwData.enemyTypes.ratthewRight.bezPoints[3];
	ratthewRight.degBez = 3;
	ratthewRight.isTypeBez = true;
	ratthewRight.attackStyle = bezier;
	PushBack(&hwData.enemies, ratthewRight);


	//=====Sine Enemy
	NPCData sin = hwData.enemyTypes.sinEnemy;
	sin.attackStyle = sinWave;
	sin.startPos = V2(1, 1);
	sin.targetPos = V2(0, -1);

	PushBack(&hwData.enemies, sin);

	//=======Random Bez
			//@TODO Create Default Params
				//This will probably be best done in a function that randomly instantiates a degree and the randomly fills coords out within a clamp

}
 
void LoadAssets() {
	LoadSprite(&hwData.sprites.playerShip, "data/galaga_ship.png");
	LoadSprite(&hwData.sprites.allyShip, "data/galaga_ship_alt.png");
	LoadSprite(&hwData.sprites.enemy[0], "data/ratthewSide.png");

	LoadSprite(&hwData.sprites.allyBullet, "data/BlasterShot_Green.png");
	LoadSprite(&hwData.sprites.playerBullet, "data/BlasterShot_Blue.png");
	LoadSprite(&hwData.sprites.enemyBullet, "data/BlasterShot_Red.png");

	LoadSoundClip("data/sfx/flute_breathy_c4.wav", &hwData.sound);
}

void InitGameNetwork() {
	NetworkInfo* network = &Game->networkInfo;
	InitNetwork(&Game->permanentArena);

	if (IS_SERVER) {
		InitSocket(&network->socket, GetMyAddress(), ServerPort, true);
	}
	else {
		InitSocket(&network->socket, GetMyAddress(), 0, true);
	}
}

void InitMemory() {
	AllocateMemoryArena(&arena, Bytes(512));
	AllocateMemoryArena(&drawArena, Megabytes(1));
	AllocateMemoryArena(&serverArena, Megabytes(1));

	hwData.playerBullets = MakeDynamicArray<Bullet>(&serverArena, 2);
	hwData.server.clients = MakeDynamicArray<ClientInfo>(&arena, 4);
	
	hwData.posToDraw = MakeDynamicArray<vec2>(&drawArena, 10);
	hwData.bulletToDraw = MakeDynamicArray<Bullet>(&serverArena, 2);
	
	hwData.enemies = MakeDynamicArray<NPCData>(&drawArena, 5);
}

//===================================================================================
//								Server & Client
//===================================================================================
void ServerUpdate() {
	NetworkInfo* network = &Game->networkInfo;
	Server* server = &hwData.server;
	int16 currentID;

	GamePacket gamePacket = {};
	gamePacket.id = PacketID;
	gamePacket.type = GamePacketType_Ping;
	gamePacket.frame = Game->frame;

	memcpy(gamePacket.data, &Game->time, sizeof(real32));

	PushBack(&network->packetsToSend, gamePacket);

	DrawTextScreen(&Game->serifFont, V2(0.5f, 0.9f), 0.02f, V4(1), true, "%u Clients Connected", server->clients.count);


	DynamicArrayClear(&hwData.playerBullets);
	for (int i = 0; i < network->packetsReceived.count; i++) {
		ReceivedPacket* received = &network->packetsReceived[i];

		if (received->packet.id != PacketID) {
			continue;
		}

		ClientInfo* validClient = NULL;
		GamePacket cInfoPacket = {};
		cInfoPacket.type = GamePacketType_ID;
		int16 clientIndex = 0;

		for (int16 f = 0; f < server->clients.count; f++) {

			ClientInfo* client = &server->clients[f];
			if (received->packet.type == GamePacketType_Ping) {
				if (received->fromAddress == client->address && received->fromPort == client->port) {
					validClient = client;
					if (!client->id) {
						int tempRand = Randi();
						SeedRand(tempRand);
						client->id = Randi();
						memcpy(cInfoPacket.data, &client->id, sizeof(int16));
						PushBack(&network->packetsToSend, cInfoPacket);
					}
					
					Print("ClientPort %u", client->port);
					Print("ClientID =  %u", client->id);
					break;
				}
			}
		}

		if (received->packet.type == GamePacketType_Ping) {
			if (validClient != NULL) {
				validClient->pingTime = Game->time;
				if (received->packet.data[0]) {
					validClient->connected = true;
				}
			}
			else {
				ClientInfo c = {};
				c.address = received->fromAddress;
				c.port = received->fromPort;
				c.pingTime = *((real32*)&received->packet.data);

				PushBack(&server->clients, c);
				validClient = &server->clients[server->clients.count - 1];
			}
			continue;
		}

		if (received->packet.type == GamePacketType_ClientProjectile) {
			Bullet b = *((Bullet*)received->packet.data);
			PushBack(&hwData.playerBullets, b);
			Print("bullet fromID = %i", b.fromID);

			continue;
		}

		if (received->packet.type == GamePacketType_Input) {
			GamePacket inputOut;
			inputOut.type = GamePacketType_Homework;
				
			InputPacket inputIn;
			
			inputIn = *((InputPacket*)(received->packet.data));
			//memcpy(&inputIn.playerBullets, &(*((InputPacket*)(received->packet.data))).playerBullets, sizeof(InputPacket));

			vec2* mPos = &inputIn.position;
			NormalizeCursor(mPos);
				
			memcpy(inputOut.data, &inputIn, sizeof(InputPacket));
			PushBack(&network->packetsToSend, inputOut);
		}
	}

	if (hwData.gameActive) {

		DrawEnemies();
		for (int i = 0; i < hwData.playerBullets.count; i++) {
			Bullet b = hwData.playerBullets[i];

			if (b.fromID != -1) {
				DrawSprite(hwData.playerBullets[i].pos, V2(0.5f, 0.5f), &hwData.sprites.playerBullet);


				if (b.pos.y > 1) {
					hwData.playerBullets[i].running = false;
				}
			
				GamePacket bulletPacket = {};
				bulletPacket.id = PacketID;
				bulletPacket.type = GamePacketType_ClientProjectile;

				memcpy(bulletPacket.data, &hwData.playerBullets[i], sizeof(Bullet));
				PushBack(&Game->networkInfo.packetsToSend, bulletPacket);
			
				if (hwData.playerBullets[i].running = false) {
					RemoveAtIndex(&hwData.playerBullets, i);
				}
			}
		}

		DrawTextScreen(&Game->serifFont, V2(0.5f, 0.5f), 0.02f, V4(1), true, "GameActive!");

		for (int i = server->clients.count - 1; i >= 0; i--) {
			ClientInfo* c = &server->clients[i];

			if (Game->time - c->pingTime > 5.0f) {
				RemoveAtIndex(&server->clients, i);
			}
		}
	}

	if (server->clients.count > 0) {
		hwData.gameActive = true;

		for (int i = 0; i < network->packetsToSend.count; i++) {
			GamePacket* p = &network->packetsToSend[i];

			for (int f = 0; f < server->clients.count; f++) {
				ClientInfo* client = &server->clients[f];
				uint32 bytesSent = SendPacket(&network->socket, client->address, client->port, p, sizeof(GamePacket));

				if (bytesSent != sizeof(GamePacket)) {
					DrawTextScreen(&Game->serifFont, V2(0.5f, 0.1f), 0.02f, V4(1), true, "PacketSize mismatch");
				}
			}
		}
	}
}

//@TODO only run based off of clientID, distribute sprites based on Client ID, have another case if Client is not a player

int16 activePlayerID = -1;
vec2 stableImagePos;
int16 clientSideCount = 0;
DynamicArray<vec2> spriteBuffer = {};
DynamicArray<Bullet> bulletBuffer = {};

void ClientUpdate() {
	NetworkInfo* network = &Game->networkInfo;
	Server* server = &hwData.server;
	bool hwReceived = false;

	GamePacket packet = {};
	packet.id = PacketID;
	packet.type = GamePacketType_Ping;
	packet.frame = Game->frame;

	memcpy(packet.data, &Game->time, sizeof(real32));
	PushBack(&network->packetsToSend, packet);

	GamePacket inputOut = {};
	inputOut.id = PacketID;
	inputOut.type = GamePacketType_Input;

	real32 pingTime;

	DynamicArrayClear(&hwData.posToDraw);
	DynamicArrayClear(&hwData.bulletToDraw);

	if (hwData.player.id == -1 && activePlayerID != -1) {
		hwData.player.id = activePlayerID;
		Print("ClientSide ID Assign = %u", player.id);
	}

	if (network->packetsReceived.count > 0) {
		player.connected = true;
		player.pingTime = Game->time;
	}

	for (int i = 0; i < network->packetsReceived.count; i++) {
		ReceivedPacket* received = &network->packetsReceived[i];

		if (received->packet.type == GamePacketType_ID) {
			activePlayerID = *((int16*)(received->packet.data));
			continue;
		}

		if (received->packet.type == GamePacketType_ClientProjectile) {
			Bullet b = *((Bullet*)received->packet.data);
			Print("BulletRec");
			//if (b.fromID == hwData.player.id) {
			//	//if (b.running == false && b.indexID < hwData.playerBullets.count) { RemoveAtIndex(&hwData.playerBullets, b.indexID); }

			//	if (memcmp(&hwData.playerBullets[b.indexID], &b, sizeof(hwData.playerBullets[b.indexID])) != 0) {
			//		if (b.running) { hwData.playerBullets[b.indexID] = b; }

			//		RemoveAtIndex(&hwData.playerBullets, b.indexID);
			//		InsertAtIndex(&hwData.playerBullets,b.indexID, b);

			//		Print("Bullet no. %i, fired by player %i, desync!", b.indexID, b.fromID);
			//		Print("Swapping Bullets\n");
			//	}
			//}

			if (b.fromID != hwData.player.id && b.fromID != -1) {
				
				Print("Ally Bullet Rec from %i", b.fromID);
				PushBack(&hwData.bulletToDraw, b);
			}

			continue;
		}

		if (received->packet.type == GamePacketType_Homework) {
			InputPacket temp = *((InputPacket*)received->packet.data);

			if (temp.playerID != hwData.player.id) {
				PushBack(&hwData.posToDraw, *((vec2*)received->packet.data));
			}
			else {
				if (temp.position != hwData.player.pos) {
					hwData.player.pos = temp.position;
				}
			}
		}
	}

	if (hwData.player.id != -1) {

		if (hwData.posToDraw.count >= 1) {
			memcpy(&spriteBuffer, &hwData.posToDraw, sizeof(hwData.posToDraw));
		}
		if (hwData.bulletToDraw.count >= 1) {
			memcpy(&bulletBuffer, &hwData.bulletToDraw, sizeof(hwData.bulletToDraw));
		}

		if (player.connected && &player != NULL) {

			for (int i = 0; i < spriteBuffer.count; i++) {
				DrawSprite(spriteBuffer[i], V2(0.5f, 0.5f), &hwData.sprites.allyShip);
			}
			for (int i = 0; i < bulletBuffer.count; i++) {
				DrawSprite(bulletBuffer[i].pos, V2(0.5f, 0.5f), &hwData.sprites.allyBullet);
			}
			Print("BulletBuffer Count %f", bulletBuffer.count);
			InputPacket input = { NULL };
			vec2 mousePos = Input->mousePosNormSigned;
			hwData.player.pos = mousePos;

			PlayerBullet();

			input.position = mousePos;
			input.playerID = hwData.player.id;

			
			DrawTextScreen(&Game->serifFont, V2(0.5f, 0.97f), 0.02f, V4(1), true, "(%f, %f)", mousePos.x, mousePos.y);

			memcpy(inputOut.data, &input, sizeof(InputPacket));
			PushBack(&network->packetsToSend, inputOut);
			pingTime = Game->time - player.pingTime;
			DrawTextScreen(&Game->serifFont, V2(0.5f, 0.1f), 0.02f, V4(1), true, "Last Ping Time: %f", pingTime);


			NormalizeCursor(&hwData.player.pos);
			DrawSprite(hwData.player.pos, V2(0.5f, 0.5f), &hwData.sprites.playerShip);

		}

		DrawEnemies();
	}

	for (int i = 0; i < network->packetsToSend.count; i++) {
		GamePacket* gamePacket = &network->packetsToSend[i];
		uint32 bytesSent = SendPacket(&network->socket, ServerAddress, ServerPort, gamePacket, sizeof(GamePacket));
		if (bytesSent != sizeof(GamePacket)) {
			DrawTextScreen(&Game->serifFont, V2(0.5f, 0.1f), 0.02f, V4(1), true, "PacketSize mismatch");
		}
	}
	 
	if (pingTime > 5.0f) {
		player.connected = false;
		DrawTextScreen(&Game->serifFont, V2(0.5f, 0.1f), 0.02f, V4(1), true, "Disconnected");
	}
	
}

//================================================================================================================
//												Game Run and Init
//================================================================================================================
void MyInit() {
	InitMemory();
	InitGameNetwork();
	InitEnemies();
	LoadAssets();
}

void MyGameUpdate() {
	NetworkInfo* network = &Game->networkInfo;

	DynamicArrayClear(&network->packetsToSend);
	ReceivePackets(&network->socket);

	if (IS_SERVER) {
		ServerUpdate(); //@DEBUG There'a problem with updating this every frame since it's causing the slower client
						//to flash on faster client's screen because the functions aren't being called at the same rate
	}
	else {
		ClientUpdate();
	}
}