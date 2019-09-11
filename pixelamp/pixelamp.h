#ifndef __PIXELAMP_H__
#define __PIXELAMP_H__

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

#define NUM_LEDS    (kMatrixWidth * kMatrixHeight)
#define LED_PIN     D6
#define COLOR_ORDER GRB
#define CHIPSET     WS2812

//used by fireworks animation
#define MODEL_BORDER 1
#define MODEL_WIDTH  (MODEL_BORDER + kMatrixWidth  + MODEL_BORDER)
#define MODEL_HEIGHT (MODEL_BORDER + kMatrixHeight + MODEL_BORDER)
#define PIXEL_X_OFFSET ((MODEL_WIDTH  - kMatrixWidth ) / 2)
#define PIXEL_Y_OFFSET ((MODEL_HEIGHT - kMatrixHeight) / 2)
#define WINDOW_X_MIN (PIXEL_X_OFFSET)
#define WINDOW_X_MAX (WINDOW_X_MIN + kMatrixWidth - 1)
#define WINDOW_Y_MIN (PIXEL_Y_OFFSET)
#define WINDOW_Y_MAX (WINDOW_Y_MIN + kMatrixHeight - 1)
#define NONE 0
#define SHELL 1
#define SPARK 2
#define NUM_SPARKS 12

//used by firepit animation
//play with this value. Range: from 0 (almost no fire) to 10 (biiig flames) or even more.
#define FIRE_HEIGHT 0.5

template <uint32_t N> struct Sprite {
  uint8_t w, h;
  uint8_t x, y;
  int8_t dx, dy;
  uint32_t data[N];
};

/************************************************************************
 * Controls                                                             *
 ************************************************************************/
#define POT_ANIM              A0
#define POT_ANIM_MIN          10
#define POT_ANIM_MAX          100
#define POT_BRIGHTNESS        A0
#define POT_BRIGHTNESS_MIN    200
#define POT_BRIGHTNESS_MAX    400
#define MAX_BRIGHTNESS        150

/************************************************************************
 * Animations                                                           *
 ************************************************************************/
void xyTester();
void hueRotationEffect();
void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8);
void animatePacChase();
void animatePacman();
void minecraft();
void hue();
void fireworks();
void firepit();
void nothing();

/************************************************************************
 * Helper functions                                                     *
 ************************************************************************/
uint16_t XY(uint8_t x, uint8_t y, bool wrapX, bool wrapY);
template <uint32_t N> void showSprite(const Sprite<N> &sprite);
template <uint32_t N> void moveSprite(Sprite<N> &sprite);
void wipeMatrices();
//fireworks
void screenscale( accum88 a, byte N, byte& screen, byte& screenerr);
void plot88( byte x, byte y, CRGB& color);
static int16_t scale15by8_local( int16_t i, fract8 scale );

class Dot {
public:
  byte    show;
  byte    theType;
  accum88 x;
  accum88 y;
  saccum78 xv;
  saccum78 yv;
  accum88 r;
  CRGB    color;
  
  Dot();
  void Draw();
  void Move();
  void GroundLaunch();
  void Skyburst( accum88 basex, accum88 basey, saccum78 basedv, CRGB& basecolor);
};


struct Sprite<72> pacmanOpenMouth = {
  9, 8,
  8, 0,
  1, 0,
  {
    0, 0, 0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0, 
    0, 0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0,
    0, CRGB::Gold, CRGB::Gold, CRGB::Blue, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0,
    0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0, 0,
    0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0, 0,
    0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0, 
    0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0,
    0, 0, 0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0, 
  }
};

struct Sprite<72> pacmanClosedMouth = {
  9, 8,
  8, 0,
  1, 0,
  {
    0, 0, 0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0, 
    0, 0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0,
    0, CRGB::Gold, CRGB::Gold, CRGB::Blue, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0,
    0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0,
    0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0,
    0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 
    0, 0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 
    0, 0, 0, CRGB::Gold, CRGB::Gold, CRGB::Gold, CRGB::Gold, 0, 0, 
  }
};

struct Sprite<72> pinkGhost = {
  9, 8,
  0, 0,
  1, 0,
  {
    0, 0, 0, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, 0, 0, 0,
    0, 0, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, 0, 0,
    0, CRGB::HotPink, CRGB::White, CRGB::White, CRGB::HotPink, CRGB::White, CRGB::White, CRGB::HotPink, 0,
    0, CRGB::HotPink, CRGB::Blue, CRGB::White, CRGB::HotPink, CRGB::Blue, CRGB::White, CRGB::HotPink, 0,
    0, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, 0,
    0, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, 0,
    0, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, CRGB::HotPink, 0,
    0, CRGB::HotPink, 0, CRGB::HotPink, 0, CRGB::HotPink, 0, CRGB::HotPink, 0
  }
};

struct Sprite<72> blueGhost = {
  9, 8,
  0, 0,
  1, 0,
  {
    0, 0, 0, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, 0, 0, 0,
    0, 0, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, 0, 0,
    0, CRGB::DeepSkyBlue, CRGB::White, CRGB::White, CRGB::DeepSkyBlue, CRGB::White, CRGB::White, CRGB::DeepSkyBlue, 0,
    0, CRGB::DeepSkyBlue, CRGB::HotPink, CRGB::White, CRGB::DeepSkyBlue, CRGB::HotPink, CRGB::White, CRGB::DeepSkyBlue, 0,
    0, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, 0,
    0, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, 0,
    0, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, CRGB::DeepSkyBlue, 0,
    0, CRGB::DeepSkyBlue, 0, CRGB::DeepSkyBlue, 0, CRGB::DeepSkyBlue, 0, CRGB::DeepSkyBlue, 0
  }
};

struct Sprite<72> orangeGhost = {
  9, 8,
  0, 0,
  1, 0,
  {
    0, 0, 0, CRGB::Orange, CRGB::Orange, CRGB::Orange, 0, 0, 0,
    0, 0, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, 0, 0,
    0, CRGB::Orange, CRGB::White, CRGB::White, CRGB::Orange, CRGB::White, CRGB::White, CRGB::Orange, 0,
    0, CRGB::Orange, CRGB::Blue, CRGB::White, CRGB::Orange, CRGB::Blue, CRGB::White, CRGB::Orange, 0,
    0, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, 0,
    0, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, 0,
    0, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange, 0,
    0, CRGB::Orange, 0, CRGB::Orange, 0, CRGB::Orange, 0, CRGB::Orange, 0
  }
};

struct Sprite<72> redGhost = {
  9, 8,
  0, 0,
  1, 0,
  {
    0, 0, 0, CRGB::Red, CRGB::Red, CRGB::Red, 0, 0, 0,
    0, 0, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 0, 0,
    0, CRGB::Red, CRGB::White, CRGB::White, CRGB::Red, CRGB::White, CRGB::White, CRGB::Red, 0,
    0, CRGB::Red, CRGB::Blue, CRGB::White, CRGB::Red, CRGB::Blue, CRGB::White, CRGB::Red, 0,
    0, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 0,
    0, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 0,
    0, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red, 0,
    0, CRGB::Red, 0, CRGB::Red, 0, CRGB::Red, 0, CRGB::Red, 0
  }
};

struct Sprite<4> pacGum = {
  2, 2 ,
  0, 3,
  0, 0,
  {
    CRGB::White, CRGB::White,
    CRGB::White, CRGB::White,
  }
};

struct Sprite<56> R2D2 = {
  8, 7 ,
  0, 0,
  1, 0,
  {
    0, 0, 0, CRGB::DarkSlateBlue, CRGB::DarkSlateBlue, 0, 0, 0,
    0, 0, CRGB::LightSlateGray, CRGB::Gainsboro, CRGB::LightSlateGray, CRGB::LightSlateGray, 0, 0,
    0, 0, CRGB::DarkSlateBlue, CRGB::Crimson, CRGB::Gray, CRGB::DarkSlateBlue, 0, 0,
    0, CRGB::Gainsboro, CRGB::LightSlateGray, CRGB::Gainsboro, CRGB::Gainsboro, CRGB::LightSlateGray, CRGB::Gainsboro, 0,
    0, CRGB::LightSlateGray, CRGB::LightSlateGray, CRGB::DeepSkyBlue, CRGB::Gainsboro, CRGB::LightSlateGray, CRGB::LightSlateGray, 0,
    0, CRGB::Gainsboro, CRGB::LightSlateGray, CRGB::DarkSlateBlue, CRGB::Gainsboro, CRGB::LightSlateGray, CRGB::Gainsboro, 0,
    0, CRGB::LightSlateGray, 0, CRGB::Gainsboro, CRGB::DarkSlateBlue, 0, CRGB::LightSlateGray, 0,
  }
};

struct Sprite<32> testSprite = {
  4, 8,
  -1, 0,
  1, 0,
  {
    0, 0, CRGB::Red, 0,
    0, CRGB::Red, 0, 0,
    0, 0, CRGB::Red, 0,
    0, CRGB::Red, 0, 0,
    0, 0, CRGB::Red, 0,
    0, CRGB::Red, 0, 0,
    0, 0, CRGB::Red, 0,
    0, CRGB::Red, 0, 0,
  }
};



/* EMPTY SPRITE
struct Sprite<64> testSprite = {
  8, 8,
  1, 0,
  {
    0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 0
  }
};
*/
#endif
