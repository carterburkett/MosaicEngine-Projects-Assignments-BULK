//Cloud Variables
struct Cloud {
	int16 shape;
	vec4 color;
	vec2 spawn;
	vec2 pos;
	real32 duration;
	real32 startTime;
};
Cloud* clouds[6] = { nullptr };
bool cloudsRunning = false;

Cloud* BuildCloud(int shape, vec2 loc, vec4 color) {
	Cloud* cloud = new Cloud;
	cloud->shape = shape;
	cloud->spawn.x = loc.x;
	cloud->spawn.y = loc.y;
	cloud->color = color;
	cloud->duration = RandiRange(18, 45);

	return cloud;
}

void InitClouds() {
	for (int i = 0; i < 6; i++) {
		clouds[i] = (Cloud*)malloc(sizeof(Cloud));
		memset(clouds[i], 0, sizeof(Cloud));
	}

	(clouds[0]) = BuildCloud(4, V2(14, 3), WHITE);
	(clouds[1]) = BuildCloud(2, V2(4, 7), WHITE);
	(clouds[2]) = BuildCloud(3, V2(21, 6), WHITE);
	(clouds[3]) = BuildCloud(3, V2(33, 8), WHITE);
	(clouds[4]) = BuildCloud(2, V2(-5, 4), WHITE);
	(clouds[5]) = BuildCloud(1, V2(44, 2), WHITE);
}

void MoveCloud() {
	for (int i = 0; i < 6; i++) {
		real32 t;
		float elapsedTime;

		int a = (clouds[i])->spawn.x;
		int b = 70;

		if ((clouds[i])->startTime >= 0.0f) {
			elapsedTime = Time - (clouds[i])->startTime;
			t = InverseLerp(0, (clouds[i])->duration, elapsedTime);

			if (t >= 1.0f) {
				(clouds[i])->spawn.x = 0.0f;
				(clouds[i])->duration = RandiRange(4, 16);
				(clouds[i])->startTime = Time;
			}

			(clouds[i])->pos.x = Lerp(a, b, t);
			(clouds[i])->pos.y = (clouds[i])->spawn.y;
		}

		switch ((clouds[i])->shape) {
		case 1:
			SetBlockColor((clouds[i])->pos, 5, 1, (clouds[i])->color);
			SetBlockColor((clouds[i])->pos + V2(1, -1), 4, 1, (clouds[i])->color);
			SetBlockColor((clouds[i])->pos + V2(2, -2), 2, 1, (clouds[i])->color);
			break;
		case 2:
			SetBlockColor((clouds[i])->pos, 6, 1, (clouds[i])->color);
			SetBlockColor((clouds[i])->pos + V2(1, -1), 5, 1, (clouds[i])->color);
			SetBlockColor((clouds[i])->pos + V2(2, -2), 3, 1, (clouds[i])->color);
			SetBlockColor((clouds[i])->pos + V2(4, -3), 2, 1, (clouds[i])->color);
			break;
		case 3:         //this cloud is fucky, why didn't I start at the spawn point??
			SetBlockColor((clouds[i])->pos + V2(-1, 1), 3, 1, (clouds[i])->color);
			SetBlockColor((clouds[i])->pos + V2(1, -1), 2, 1, (clouds[i])->color);
			SetBlockColor((clouds[i])->pos, 4, 1, (clouds[i])->color);
			SetTileColor((clouds[i])->pos + V2(4, -1), (clouds[i])->color);
			break;
		default:
			SetTileColor((clouds[i])->pos, (clouds[i])->color);
			SetTileColor((clouds[i])->pos + V2(1, -1), (clouds[i])->color);
			break;
		}
	}
}