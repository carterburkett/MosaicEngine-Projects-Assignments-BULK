float r, g, b;
bool gameRunning = true;
bool rFlip, gFlip, bFlip;
real32 SlerpTimer, SlerpTimerSec, SlerpTimerMin, SlerpTimerHr;
int count = 1;

//=========================================================================
//                              Draw
//=========================================================================

void DrawCircle(vec2 start, vec2 end, vec4 color) {
    float ox = (start.x + end.x) / 2;
    float oy = (start.y + end.y) / 2;

    vec2 origin = V2(ox, oy);
    real32 rad = Distance(origin, start);

    vec2 minValPoint = origin - V2(rad);

    vec2 maxValPoint = origin + V2(rad - 1); //@Debug This miiiiight create a problem if there's a perfect center???

    for (real32 i = 0; i < _2PI; i += (_PI / 256)) { //256 will probably need to be bigger if the aspect ratio goes up
        float xCirc = (rad * cosf(i)) + origin.x;
        float yCirc = (rad * sinf(i)) + origin.y;
        
        vec2 circPos = V2(xCirc, yCirc);
        
        circPos.x = Clamp(circPos.x, minValPoint.x, maxValPoint.x);
        circPos.y = Clamp(circPos.y, minValPoint.y, maxValPoint.y);

        SetTileColor(circPos, color);

        Print("DrawingPos: (%f, %f)", circPos.x, circPos.y);
        Print("%f %f %f", r, g, b);
    }
}

void DrawPointSlerp(vec2 start, vec2 end, vec4 color, float duration = 5.0f) {
    real32 elapsedTime = Time - SlerpTimer;
    real32 t = elapsedTime / duration;

    if (t >= 1.0f) {
        SlerpTimer = Time;
        elapsedTime = 0.0f;
        t = 0.0f;
    }

    float ox = (start.x + end.x) / 2;
    float oy = (start.y + end.y) / 2;
    vec2 origin = V2(ox, oy);

    real32 rad = Distance(origin, start);

    float angle = Lerp(0, _2PI, t);

    float xCPos = rad * cosf(angle) + origin.x;
    float yCPos = rad * sinf(angle) + origin.y;

    vec2 circPos = V2(xCPos, yCPos);

    SetTileColor(circPos, color);
}

void DrawClockHands(vec2 start, vec2 end, vec4 color, int duration) {
    real32 elapsedTime;
    
    switch (duration) {
    case(60):
        elapsedTime = Time - SlerpTimerSec;
        Print("Second Hand Initiated");
        break;
    case(3600):
        elapsedTime = Time - SlerpTimerMin;
        Print("Minute Hand Initiated");
        break;
    case(43200):
        elapsedTime = Time - SlerpTimerHr;
        Print("Hour Hand Initiated");
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

    float ox = (start.x + end.x) / 2;
    float oy = (start.y + end.y) / 2;
    vec2 origin = V2(ox, oy);

    real32 rad = Distance(origin, start);

    float angle = Lerp(0, _2PI, t);

    float xCPos = (rad * cosf(angle - (_PI / 2))) + origin.x;
    float yCPos = (rad * sinf(angle - (_PI / 2))) + origin.y;

    vec2 circPos = V2(xCPos, yCPos);

    SetTileColor(circPos, color);
}


//=========================================================================
//                          Game Start and Run
//=========================================================================
void MyMosaicInit() {
    SetMosaicGridSize(100, 100);
    gameRunning = true;
    g = 1.0f;

    rFlip = gFlip = bFlip = false;
}

void MyMosaicUpdate() {
    vec2 s = V2(50, 2);
    vec2 e = V2(50, 98);

    ClearTiles(BLACK);

    Print("Time == %u", Time);
    Print("DeltaTime == %u", DeltaTime);

    r >= 1.0f || r < 0.0f ? rFlip = !rFlip : rFlip = rFlip;
    g >= 1.0f || g < 0.0f ? gFlip = !gFlip : gFlip = gFlip;
    b >= 1.0f || b < 0.0f ? bFlip = !bFlip : bFlip = bFlip;

    !rFlip ? r += DeltaTime / 8.0f : r -= DeltaTime / 8.0f;
     gFlip ? g += DeltaTime / 5.0f : g -= DeltaTime / 5.0f;
    !bFlip ? b += DeltaTime / 11.0f : b -= DeltaTime / 11.0f;

    for (int i = 0; i <= 48; i++) {
        DrawCircle(s + V2(0, i), e - V2(0, i), WHITE);
    }

    DrawCircle(s, e, V4(r, g, b, 1.0f));
    DrawCircle(s + V2(0,1), e - V2(0,1), V4(r, g, b, 1.0f));
    DrawCircle(s + V2(0,2), e - V2(0,2), V4(r, g, b, 1.0f));

    for (int i = 0; i <= 50; i++) {
        DrawClockHands(V2(49, 12 + i), V2(49, 88 - i), BLACK, 43200); //Hours
        DrawClockHands(V2(10 + i, 50), V2(90 - i, 50), BLACK, 43200); //Hours
        DrawClockHands(V2(51, 12 + i), V2(51, 88 - i), BLACK, 43200); //Hours
        
        DrawClockHands(V2(10 + i, 50), V2(90 - i, 50), BLUE, 3600); //Minute
        DrawClockHands(V2(10 + i, 50), V2(90 - i, 50), RED, 60);      //Seconds
    }
    count++;
}










//===============================================================================================
//                  Code I made but didn't use but also still want access to
//===============================================================================================

void DrawLeftSemiCircle(vec2 start, vec2 end, vec4 color) {   //Accidentally made this trying to fix my circle funciton. There's probably a better way
                                                                //to draw a semi circle but this Does work too...
    float ox = (start.x + end.x) / 2;
    float oy = (start.y + end.y) / 2;
    vec2 origin = V2(ox, oy);
    real32 rad = Distance(origin, start);

    for (real32 i = 0; i < (_2PI * r + 1); i += (_PI / 256)) {
        float xCirc = (rad * cosf(i)) + origin.x;
        float yCirc = (rad * sinf(i)) + origin.y;

        vec2 circPos = V2(xCirc, yCirc);
        vec2 minValPoint = origin - V2(rad);


        //vec2 min = Min(start, end);
        vec2 max = Max(start, end);

        //circPos = Clamp(circPos, min, max);
        circPos.x = Clamp(circPos.x, minValPoint.x, max.x);
        circPos.y = Clamp(circPos.y, minValPoint.y, max.y);




        SetTileColor(circPos, color);

        Print("DrawingPos: (%f, %f)", circPos.x, circPos.y);
        Print("%f %f %f", r, g, b);
    }
}