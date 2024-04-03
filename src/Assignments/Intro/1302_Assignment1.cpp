

real32 colorIncrement = 0.0f;
float xPos = 1.0f;
float yPos = 11.0f;
real32 cDirection = 0.1f;
vec2 tilePos;
vec4 playerColor;

void MoveTile() {
    bool checkHoriz = (xPos >= 0 && xPos <= 31) ? true : false;
    bool checkVert = (yPos >= 0 && yPos <= 31) ? true : false;

    //============Left Right Boundary Check=========
    if (!checkHoriz) {
        xPos > 31 ? xPos = 31 : xPos = 0;
    }
    else {
        if (InputPressed(Keyboard, Input_D) || InputPressed(Keyboard, Input_RightArrow)) {
            xPos += 1;
        }
        else if (InputPressed(Keyboard, Input_A) || InputPressed(Keyboard, Input_LeftArrow)) {
            xPos -= 1;
        }
    }

    //==============Top Bottom Boundary Check===================
    if (!checkVert) {
        yPos > 31 ? yPos = 31 : yPos = 0;
    }
    else {
        if (InputPressed(Keyboard, Input_W) || InputPressed(Keyboard, Input_UpArrow)) {
            yPos -= 1;
        }
        else if (InputPressed(Keyboard, Input_S) || InputPressed(Keyboard, Input_DownArrow)) {
            yPos += 1;
        }
    }

    tilePos = V2(xPos, yPos);
}

void IncrementColor() {
    if (InputPressed(Keyboard, Input_Space) && (colorIncrement <= 1.0f || colorIncrement >= 0.0f)) {
        colorIncrement += cDirection;

        if (colorIncrement >= 1.0f || colorIncrement <= 0.0f) { //@INFO_condition :: inverts direction so it doesn't just get stuck on a single color
            cDirection *= -1.0f;
        }
    }

    playerColor = V4(colorIncrement, 0.0f, 1.0f - colorIncrement, 1.0f);
}

void MyMosaicInit() {
    SetMosaicGridSize(32, 32);
}

void MyMosaicUpdate() {
    ClearTiles(BLACK);
    SetTileColor(0.0f, 0.0f, RED);
    SetTileColor(1.0f, 0.0f, GREEN);
    SetTileColor(2.0f, 0.0f, BLUE);
    SetTileColor(31.0f, 0.0f, YELLOW);
    SetTileColor(31.0f, 31.0f, CYAN);
    SetTileColor(0.0f, 31.0f, MAGENTA);
    
    IncrementColor();
    MoveTile();
    SetTileColor(tilePos, playerColor);
}
