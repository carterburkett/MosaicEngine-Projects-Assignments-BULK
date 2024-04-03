
int mode;

void DrawLine(vec2 scale, vec4 color = WHITE) {
    //Doing this for proof of concept but SetBlock Color exists also
    for (int x = 0; x < scale.x; x++) {
        for (int y = 0; y < scale.y; y++) {
            SetTileColor(x, y, color);
        }
    }
}
void DrawLine() {
    DrawLine(V2(1,16), WHITE);
}

void DrawCheckerLine(vec2 scale, vec4 oddColor, vec4 evenColor) {
    for (int x = 0; x < scale.x; x++) {
        for (int y = 0; y < scale.y; y++) {
            vec4 tempColor;

            if(scale.x > 1 && scale.y > 1){
                if (y % 2 == 0) {
                    if (x % 2 == 0) { SetTileColor(x, y, evenColor); }
                    else { SetTileColor(x, y, oddColor); }
                }
                else {
                    if (x % 2 == 0) { SetTileColor(x, y, oddColor); }
                    else { SetTileColor(x, y, evenColor); }
                }
            }
            else {
                if (scale.x > 1) {
                    if (x % 2 == 0) { SetTileColor(x, y, evenColor); }
                    else { SetTileColor(x, y, oddColor); }
                }
                if (scale.y > 1) {
                    if (y % 2 == 0) { SetTileColor(x, y, evenColor); }
                    else { SetTileColor(x, y, oddColor); }
                }
            }
        }
    }
}
void DrawCheckerLine(vec2 scale) {
    DrawCheckerLine(scale, BLACK, WHITE);
}

void DrawThirds(vec2 scale, vec4 color1, vec4 color2, vec4 color3) {
    for (int x = 0; x < scale.x; x++) {
        for (int y = 0; y < scale.y; y++) {
            if (x < scale.x * .333) { SetTileColor(x, y, color1); }
            else if (x < scale.x * .667) { SetTileColor(x, y, color2); }
            else { SetTileColor(x, y, color3); }
        }
    }
}

void DrawScalingColor(vec2 scale, vec4 startColor, vec4 endColor, bool row, bool column) {
    for (int x = 0; x < scale.x; x++) {
        for (int y = 0; y < scale.y; y++) {
            vec4 colorOut;

            if (row && column) {
                int avg = (x + y) / 2;
                colorOut = Lerp(startColor, endColor, avg / ((scale.x + scale.y) / 2.0f));
            }
            else if (row) {
                colorOut = Lerp(startColor, endColor, x / scale.x);
            }
            else if (column) {
                colorOut = Lerp(startColor, endColor, y / scale.y);
            }

            SetTileColor(x, y, colorOut);
        }
    }
}

void PeakColorAtMid(vec2 scale, vec4 startColor, vec4 endColor) {
    //This is terrible execution
    for (int x = 0; x < scale.x; x++) {
        for (int y = 0; y < scale.y; y++) {
            vec4 colorOut;
            float midPoint = scale.x / 2.0f;
            if (x <= midPoint) {
                colorOut = Lerp(startColor, endColor, x / midPoint);
            }
            else { colorOut = Lerp(endColor, startColor, x / scale.x); }
            SetTileColor(x, y, colorOut);
        }
    }
}

void AlternateRowColor(vec2 scale, vec4 color1, vec4 color2) {
    for (int x = 0; x < scale.x; x++) {
        for (int y = 0; y < scale.y; y++) {
            if (y % 2 == 0) { SetTileColor(x, y, color1); }
            else { SetTileColor(x, y, color2); }
        }
    }
}

void MyMosaicInit() {
    SetMosaicGridSize(16, 16);
    mode = 0;
}

void MyMosaicUpdate() {
    switch (mode) {
    case 0:
        DrawLine();
        break;
    case 1:
        DrawCheckerLine(V2(16,1));
        break;
    case 2:
        DrawCheckerLine(V2(16, 1), RED, BLUE);
        break;
    case 3:
        DrawThirds(V2(16, 2), RED, YELLOW, GREEN);
        break;
    case 4:
        DrawLine(V2(3), YELLOW);
        break;
    case 5:
        AlternateRowColor(V2(10), GREEN, PASTEL_RED);
        break;
    case 6:
        DrawCheckerLine(V2(8), PASTEL_ORANGE, GREEN);
        break;
    case 7:
        DrawScalingColor(V2(16), SKYBLUE, YELLOW, false, true);
        break;
    case 8:
        DrawScalingColor(V2(16), SKYBLUE, YELLOW, true, false);
        break;
    case 9:
        PeakColorAtMid(V2(15), YELLOW, BLUE);
        break;
    }

    if (InputPressed(Keyboard, Input_Space)) {
        mode++;
        if (mode > 9) { mode = 0; }

    }

}
