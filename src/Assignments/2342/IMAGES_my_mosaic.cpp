struct MosTile {
    int16 index;
    vec2 pos;
    vec4 color;
};

struct Image {
    int16 quadrant;
    vec2 size;
    int16 capacity;
    real32 startTime;
    
    vec4* data;
    MosTile tiles = {};
};

Image img[4] = {};
vec2 boardSize = V2(20);

float r, g, b;
bool gameRunning = true;
bool rFlip, gFlip, bFlip;
real32 SlerpTimer, SlerpTimerSec, SlerpTimerMin, SlerpTimerHr;
real32 acidTime, checkerTimer;
int count = 1;
float tDir = 1;
int checkerMod;

//=========================================================================
//                              Clock
//=========================================================================
void DrawTESTCircle(int origin, int radius, vec4 color, vec4* data) { //Ok This finally fucking works. DONT CHANGE IT
                                                                                        //Now need to adapt to move. This is way harder without coords
    for (int i = 0; i < 360; ++i) {
        float angle = i * _PI / 180.0;
        int x = origin + radius * cos(angle);
        int y = origin + radius * sin(angle);

        x = (x % 40 + 40) % 40;
        y = (y % 40 + 40) % 40;

        int index = y * 40 + x;
        data[index] = color;
        //Print("Index = %i", index);
    }
}

void DrawClockHandsFromOrigin(real32 origin, real32 radius, vec4 color, int duration, vec4* data) {
    real32 elapsedTime;

    switch (duration) {
    case(60):
        elapsedTime = Time - SlerpTimerSec;
        break;
    case(3600):
        elapsedTime = Time - SlerpTimerMin;
        break;
    case(43200):
        elapsedTime = Time - SlerpTimerHr;
        break;
    default:
        elapsedTime = Time - SlerpTimerSec;
        duration = 5.0f;
        Print("Using Clock Hand Function with no duration value. \n Initializing 'Seconds' Timer and setting duration to 5.0f \n Did you mean to use DrawPointSlerp() ?\n");
        break;
    }

    real32 t = elapsedTime / duration;

    if (t >= 1.0f) {
        switch (duration) {
        case(60):
            SlerpTimerSec = Time;
            Print("Minute Completed");
            break;
        case(3600):
            SlerpTimerMin = Time;
            Print("Hour Completed");
            break;
        case(43200):
            SlerpTimerHr = Time;
            Print("12 Hours Completed");
            break;
        default:
            SlerpTimerSec = Time;
            Print("Resetting Clock Hand Function with no duration value. \n Initializing 'Seconds' Timer and setting duration to 5.0f \n Did you mean to use DrawPointSlerp() ?\n");
            break;
        }

        elapsedTime = 0.0f;
        t = 0.0f;
    }

    float angle = Lerp(0, 360, t);
    int x = origin + radius * cos(angle * _PI / 180.0);
    int y = origin + radius * sin(angle * _PI / 180.0);

    // Normalize x and y to be within the grid bounds
    x = (x % 40 + 40) % 40;
    y = (y % 40 + 40) % 40;

    int index = y * 40 + x;
    data[index] = color;
    //Print("Index = %i", index);
}

void ClockImage(Image* m) {
    //ClearTiles(BLACK); //@REMOVE

    r >= 1.0f || r < 0.0f ? rFlip = !rFlip : rFlip = rFlip;
    g >= 1.0f || g < 0.0f ? gFlip = !gFlip : gFlip = gFlip;
    b >= 1.0f || b < 0.0f ? bFlip = !bFlip : bFlip = bFlip;

    !rFlip ? r += DeltaTime / 8.0f : r -= DeltaTime / 8.0f;
    gFlip ? g += DeltaTime / 5.0f : g -= DeltaTime / 5.0f;
    !bFlip ? b += DeltaTime / 11.0f : b -= DeltaTime / 11.0f;

    for (int i = 0; i <= m->size.x / 2; i++) {
        DrawTESTCircle(820, i, WHITE, m->data);
    }
    DrawTESTCircle(820, 20, V4(r, g, b, 1.0f), m->data);

    for (int i = 0; i <= (m->size.x / 2) - 2; i++) {
        DrawClockHandsFromOrigin(820, 19 - i, BLACK, 43200, m->data);

        DrawClockHandsFromOrigin(820, 19 - i, RED, 60, m->data);
        DrawClockHandsFromOrigin(820, 19 - i, BLUE, 3600, m->data);

    }

    count++;
}

void ClockInit() {
    g = 1.0f;
    rFlip = gFlip = bFlip = false;
}


//=========================================================================
//                           Acid
//=========================================================================
void AcidSheet(int width, int height, vec4* pixels) { //random RGB values using different trig Functions changes over time
    float t, elapsedTime;

    if (acidTime >= 0) {
        elapsedTime = Time - acidTime;
        t = elapsedTime;

        //if (t > 1.0f) {
        //    acidTime = Time;
        //}
    }

    int32 index = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[index] = V4(0, 0, 0, 1);
            pixels[index].r = (0 + sinf(Time + x * y * 0.2f)) * t;
            pixels[index].g = (0 + cosf(Time + x * y * 0.1f)) * t;
            pixels[index].b = (1 + cosf(cosf(Time + x * y * 0.6f))) * t;

            index++;
        }
    }
}

//=========================================================================
//                              Game
//=========================================================================

bool swap = true;

void Checkers(Image* m) {
    
    if (checkerMod == NULL) {
        checkerMod = 2;
    }

    if (InputPressed(Keyboard, Input_Space)) {
        checkerMod = RandiRange(1, 7);
    }
    
    int x, y;
    int height = m->size.y;
    int width = m->size.x;

    checkerTimer += DeltaTime * tDir;

    //vec4 random = V4(RandfRange(0, 1), RandfRange(0, 1), RandfRange(0, 1), 1.0f);
    vec4 random = WHITE;

    vec4 c1 = Lerp(GREY, BLACK, checkerTimer);
    vec4 c2 = Lerp(BLACK, GREY, checkerTimer);

    if (checkerTimer < 0 || checkerTimer > 1) {
        swap = !swap;  
    }
    
    if (!swap) {
        tDir *= -1;
        
    }

    int32 index = 0;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if (x % checkerMod == 0) {
                m->data[index] = c1;
            }
            else{
                
                if (!swap) { c2 = V4(RandfRange(0, 1), RandfRange(0, 1), RandfRange(0, 1), 1.0f); }
                m->data[index] = c2;
            }

            index++;
        }
    }
}
//=========================================================================
//                              Rain
//=========================================================================
void RandomColorsRandomPos(Image* img) {
    for (int f = 0; f < RandiRange(0,img->capacity-1); f++) {
        int i = RandiRange(0, img->capacity-1);
        img->data[i].r = RandfRange(0, 1);
        img->data[i].g = RandfRange(0, 1);
        img->data[i].b = RandfRange(0, 1);
        img->data[i].w = RandfRange(0, 1);
    }
}

//=========================================================================
//                              Game
//=========================================================================
void InitImages() {
    for (int i = 0; i < 4; i++) {
        img[i].quadrant = i;
        img[i].capacity = img[i].size.x * img[i].size.y;
        
        img[i].data = (vec4 *)malloc(sizeof(vec4) * img[i].capacity);
    }
}


void DrawImage(vec2 anchor, vec2 size, Image m) {
    int16 index = 0;
    for (int x = 0; x < size.x; x++) {
        if (m.quadrant == 0) {
            for (int y = anchor.y; y >= size.y; y--) {
                SetTileColor(x, y, m.data[index]);
                index++;
            }
        }
        else {
            for (int y = 0; y < size.y; y++) {
                SetTileColor(anchor.x + x, anchor.y + y, m.data[index]);
                index++;
            }
        }
    }
}

void MyMosaicInit() {
    acidTime = Time;
    boardSize = V2(80, 80);
    SetMosaicGridSize(boardSize.x, boardSize.y);
    
    img[0].size.x = boardSize.x / 2;
    img[0].size.y = boardSize.y / 2;
    
    img[1].size.x = 40;
    img[1].size.y = 80;
    
    img[2].size.x = 15;
    img[2].size.y = 40;
    
    img[3].size.x = 25;
    img[3].size.y = 40;

    InitImages();
    ClockInit();
    RandomColorsRandomPos(&img[3]);
}

void MyMosaicUpdate() {
    DrawTextTop(WHITE, 8, "The Clock moves in real time.");
    ClearTiles(RED);
    ClockImage(&img[0]);
    AcidSheet(40, 80, img[1].data);
    Checkers(&img[2]);

    DrawImage(V2(0,39), V2(40, 0), img[0]);

    DrawImage(V2(40,0), V2(40, 80), img[1]);

    DrawImage(V2(0, 40), V2(15, 40), img[2]);
    DrawImage(V2(15, 40), V2(25, 40), img[3]);
    
    
    if (InputPressed(Keyboard, Input_Space)) {
        acidTime = Time;
        SlerpTimer = Time;
        SlerpTimerSec = Time;
        SlerpTimerMin = Time;
        SlerpTimerHr = Time;
        RandomColorsRandomPos(&img[3]);
    }
}
