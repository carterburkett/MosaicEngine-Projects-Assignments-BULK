
#define RGB(r, g, b) V4(r, g, b, 1.0f)

#define RGB8(r, g, b) V4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f)


// Usage like RGBHex(ffeeaa)
#define RGBHex(h) V4(((0x##h >> 16) & 0xff) / 255.0f,       \
                     ((0x##h >> 8) & 0xff) / 255.0f,        \
                     (0x##h & 0xff) / 255.0f,               \
                     1.0f)

// Usage like RGBH(0xffeeaa)
#define RGBH(h) V4(((h >> 16) & 0xff) / 255.0f, \
                   ((h >> 8) & 0xff) / 255.0f,  \
                   (h & 0xff) / 255.0f,         \
                   1.0f)

#define WHITE RGB(1.0f, 1.0f, 1.0f)
#define BLACK RGB(0.0f, 0.0f, 0.0f)
#define GREY RGBHex(808080)

#define RED RGB(1.0f, 0.0f, 0.0f)
#define GREEN RGB(0.0f, 1.0f, 0.0f)
#define BLUE RGB(0.0f, 0.0f, 1.0f)

#define MAGENTA RGB(1.0f, 0.0f, 1.0f)
#define YELLOW RGB(1.0f, 1.0f, 0.0f)
#define CYAN RGB(0.0f, 1.0f, 1.0f)
#define GOLD RGBHex(ffa200)

#define PASTEL_RED RGBHex(fea3aa)
#define PASTEL_GREEN RGBHex(baed91)
#define PASTEL_BLUE RGBHex(b2cefe)

#define PASTEL_ORANGE RGBHex(f8b88b)
#define PASTEL_YELLOW RGBHex(faf884)
#define PASTEL_PURPLE RGBHex(f2a2e8)

#define SKYBLUE RGBHex(9cbcf0)
#define GRASSY_GREEN RGBHex(485e23)

#define LITEBROWN RGBHex(8c7865)
#define MEDBROWN RGBHex(6e563f)
#define DARKBROWN RGBHex(2b231c)


#define SKIN_COLOR RGB(.4f, .4f, .1f, 1.0f)
#define CLOTHES RGB(1.0f, .8f, .8f, 1.0f)
#define SKY_BLUE RGB(0.3f, 0.3f, 0.7f, 1.0f)
#define GRASS RGB(0.5f, 0.8f, .3f, 1.0f)
#define DARK_LEAF RGB(.2f, .6f, .2f, 1.0f)
#define LEAF RGB(.3f, .7f, .2f, 1.0f)
#define BROWN RGB(0.58f, .29f, 0.0f, 1.0f)
    
