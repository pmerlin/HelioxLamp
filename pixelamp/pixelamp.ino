/*********************************************************************************************************
 * Credits:
 * Firepit Effect: FastLED Fire 2018 by Stefan Petrick, see https://www.youtube.com/watch?v=SWMu-a9pbyk
 * Firework Effect: FireworksXY by Mark Kriegsman, July 2013
 */

#include <FastLED.h>
#include "pixelamp.h"

const uint8_t kMatrixWidth = 16;
const uint8_t kMatrixHeight = 8;
uint8_t CentreX =  (kMatrixWidth / 2) - 1;
uint8_t CentreY = (kMatrixHeight / 2) - 1;

uint8_t brightness = 1;
uint32_t currentEffect = 0;

CRGB leds[NUM_LEDS];

/*********************************************************************************************************
 * Firework Effect data
 */
CRGB overrun;
saccum78 gGravity = 10;
fract8  gBounce = 200;
fract8  gDrag = 255;
bool gSkyburst = 0;
accum88 gBurstx;
accum88 gBursty;
saccum78 gBurstxv;
saccum78 gBurstyv;
CRGB gBurstcolor;
Dot gDot;
Dot gSparks[NUM_SPARKS];

/*********************************************************************************************************
 * Firepit Effect data
 */
uint32_t x;
uint32_t y;
uint32_t z;
uint32_t scale_x;
uint32_t scale_y;
uint8_t noise[16][8];
// heatmap data with the size matrix kMatrixWidth * kMatrixHeight
uint8_t heat[128];
CRGBPalette16 Pal;

/*********************************************************************************************************
 * Effects list
 */
void (*effects[])() = {
  xyTester,               //#0
  hueRotationEffect,      //#1
  animatePacChase,       //#2, pink
  animatePacman,
  hue,
  fireworks,
  firepit,
  nothing
};


void setup() {
  Serial.begin(115200);
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(Halogen);
  FastLED.setBrightness(brightness);
  FastLED.setDither(DISABLE_DITHER);

  //changer cette palette pour avoir des effets super sympas, comme un feu sous l'océan avec OceanColors_p
  Pal = LavaColors_p;
}

void loop() {
  effects[currentEffect]();

  changeAnimation();
  changeBrightness();
}

/*********************************************************************************************************
 * Controls
 */
void changeAnimation() {
  static int oldValue = currentEffect;
  int potValue = analogRead(POT_ANIM);
//  Serial.print (potValue); Serial.print ("\n");
  potValue = constrain(potValue, POT_ANIM_MIN, POT_ANIM_MAX);
  int newValue = map(potValue, POT_ANIM_MIN, POT_ANIM_MAX, 0, ARRAY_SIZE(effects)-1);

 Serial.printf("%d : %d : %d\n",potValue, newValue, oldValue);

  if (newValue != oldValue) {
    oldValue = newValue;
    currentEffect = newValue;
    wipeMatrices();
  }
}

void changeBrightness() {
  static int oldValue = brightness;
  int potValue = 220; //analogRead(POT_BRIGHTNESS);
  potValue = constrain(potValue, POT_BRIGHTNESS_MIN, POT_BRIGHTNESS_MAX);
  int newValue = map(potValue, POT_BRIGHTNESS_MIN, POT_BRIGHTNESS_MAX, 0, 255);

  if (newValue != oldValue) {
    oldValue = newValue;
    brightness = newValue;
    brightness = constrain(brightness, 0, MAX_BRIGHTNESS);
    FastLED.setBrightness(brightness);
  }
}

/*********************************************************************************************************
 * Effects
 */
void xyTester() {
  static uint8_t x=0;
  static uint8_t y=0;
  static uint8_t hue = 0;
  
  leds[XY(x, y, false, false)] = CHSV(hue, 255, 255);
  FastLED.show();
  FastLED.delay(20);

  x++;
  if (x >= kMatrixWidth) {
    y++;
    x = 0;
    hue++;
  }
  if (y >= kMatrixHeight) {
    y = x = 0;
    wipeMatrices();
  }
}

void hueRotationEffect() {
  uint32_t ms = millis();
  int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / kMatrixWidth));
  int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / kMatrixHeight));
  DrawOneFrame( ms / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
  FastLED.show();
}

void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8) {
  byte lineStartHue = startHue8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      leds[XY(x, y, true, true)]  = CHSV( pixelHue, 255, 255);
    }
  }
}

void animatePacChase() {  
  static boolean openMouth = true;
  
  showSprite(pinkGhost);
  showSprite(openMouth? pacmanOpenMouth:pacmanClosedMouth);

  openMouth = !openMouth;

  FastLED.delay(200);
  moveSprite(pacmanOpenMouth);
  moveSprite(pacmanClosedMouth);
  pinkGhost.x = pacmanOpenMouth.x + 8;
}

void animatePacman() {
  static boolean openMouth = true;
  struct Sprite<4> pacGum2 = pacGum;
  struct Sprite<4> pacGum3 = pacGum;
  struct Sprite<4> pacGum4 = pacGum;

  pacGum.x = 1;
  pacGum2.x = 5;
  pacGum3.x = 9;
  pacGum4.x = 13;
    
  showSprite(openMouth? pacmanOpenMouth:pacmanClosedMouth);

  int pacmanPosition = pacmanOpenMouth.x%16;

  //pacgum dissapearance handling
  if (pacmanPosition > 2 && pacmanPosition < 11) showSprite(pacGum);
  if (pacmanPosition > 6 && pacmanPosition < 15) showSprite(pacGum2);
  if (pacmanPosition > 10 || pacmanPosition < 3) showSprite(pacGum3);
  if (pacmanPosition > 14 || pacmanPosition < 7) showSprite(pacGum4);
  
  openMouth = !openMouth;

  FastLED.delay(200);
  moveSprite(pacmanOpenMouth);
  moveSprite(pacmanClosedMouth);
}

void hue() { 
  static uint8_t hue = 0;
  for(uint8_t y=0; y<kMatrixWidth; y++) {
    fill_solid(&leds[y*kMatrixHeight], kMatrixHeight, CHSV(hue+(kMatrixHeight*y), 255, 255));
  }
  FastLED.delay(10);
  FastLED.show();
  hue++;
}

void fireworks() {
  random16_add_entropy( random(300) );
  CRGB sky1(0,0,2);
  CRGB sky2(2,0,2);

  memset8( leds, 0, NUM_LEDS * 3);

#if 1
   for( uint16_t v = 0; v < NUM_LEDS; v++) {
     leds[v] = sky1;
   }
   for( byte u = 0; u < 1; u++) {
    leds[random8(NUM_LEDS)] = sky2;
  }
#endif
  
  gDot.Move();
  gDot.Draw();
  for( byte b = 0; b < NUM_SPARKS; b++) {
    gSparks[b].Move();
    gSparks[b].Draw();
  }
  
  LEDS.show();
  static uint16_t launchcountdown = 0;
  if( gDot.show == 0 ) {
    if( launchcountdown == 0) {
      gDot.GroundLaunch();
      gDot.theType = SHELL;
      launchcountdown = random16( 350) + 1;
    } else {
      launchcountdown --;
    }
  }
  
  if( gSkyburst) {
    byte nsparks = random8( NUM_SPARKS / 2, NUM_SPARKS + 1);
    for( byte b = 0; b < nsparks; b++) {
      gSparks[b].Skyburst( gBurstx, gBursty, gBurstyv, gBurstcolor);
      gSkyburst = 0;
    }
  }

  FastLED.delay(10);
}

void firepit() {
  // get one noise value out of a moving noise space
  uint16_t ctrl1 = inoise16(11 * millis(), 0, 0);
  // get another one
  uint16_t ctrl2 = inoise16(13 * millis(), 100000, 100000);
  // average of both to get a more unpredictable curve
  uint16_t  ctrl = ((ctrl1 + ctrl2) / 2);

  // this factor defines the general speed of the heatmap movement
  // high value = high speed
  uint8_t speed = 27;

  // here we define the impact of the wind
  // high factor = a lot of movement to the sides
  x = 3 * ctrl * speed;

  // this is the speed of the upstream itself
  // high factor = fast movement
  y = 15 * millis() * speed;

  // just for ever changing patterns we move through z as well
  z = 3 * millis() * speed ;

  // ...and dynamically scale the complete heatmap for some changes in the
  // size of the heatspots.
  // The speed of change is influenced by the factors in the calculation of ctrl1 & 2 above.
  // The divisor sets the impact of the size-scaling.
  scale_x = ctrl1 / 2;
  scale_y = ctrl2 / 2;

  // Calculate the noise array based on the control parameters.
  uint8_t layer = 0;
  for (uint8_t i = 0; i < kMatrixWidth; i++) {
    uint32_t ioffset = scale_x * (i - CentreX);
    for (uint8_t j = 0; j < kMatrixHeight; j++) {
      uint32_t joffset = scale_y * (j - CentreY);
      uint16_t data = ((inoise16(x + ioffset, y + joffset, z)) + 1);
      noise[i][j] = data >> 8;
    }
  }


  // Draw the first (lowest) line - seed the fire.
  // It could be random pixels or anything else as well.
  for (uint8_t x = 0; x < kMatrixWidth; x++) {
    // draw
    leds[XY(x, kMatrixHeight-1, false, false)] = ColorFromPalette( Pal, noise[x][0]);
    // and fill the lowest line of the heatmap, too
    heat[XY(x, kMatrixHeight-1, false, false)] = noise[x][0];
  }

  // Copy the heatmap one line up for the scrolling.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {
      heat[XY(x, y, false, false)] = heat[XY(x, y + 1, false, false)];
    }
  }

  // Scale the heatmap values down based on the independent scrolling noise array.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {

      // get data from the calculated noise field
      uint8_t dim = noise[x][y];

      // This number is critical
      // If it´s to low (like 1.1) the fire dosn´t go up far enough.
      // If it´s to high (like 3) the fire goes up too high.
      // It depends on the framerate which number is best.
      // If the number is not right you loose the uplifting fire clouds
      // which seperate themself while rising up.
      dim = dim / FIRE_HEIGHT;

      dim = 255 - dim;

      // here happens the scaling of the heatmap
      heat[XY(x, y, false, false)] = scale8(heat[XY(x, y, false, false)] , dim);
    }
  }

  // Now just map the colors based on the heatmap.
  for (uint8_t y = 0; y < kMatrixHeight - 1; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {
      leds[XY(x, y, false, false)] = ColorFromPalette( Pal, heat[XY(x, y, false, false)]);
    }
  }

  // Done. Bring it on!
  FastLED.show();

  // I hate this delay but with 8 bit scaling there is no way arround.
  // If the framerate gets too high the frame by frame scaling doesn´s work anymore.
  // Basically it does but it´s impossible to see then...

  // If you change the framerate here you need to adjust the
  // y speed and the dim divisor, too.
  FastLED.delay(10);

}

void nothing() {
  wipeMatrices();
}


/*********************************************************************************************************
 * Helping functions
 */
//[0;0] is ont top-left-hand corner
uint16_t XY(uint8_t x, uint8_t y, bool wrapX, bool wrapY) {
  const uint8_t matrixMaxX = kMatrixWidth - 1;
  const uint8_t matrixMaxY = kMatrixHeight - 1;

  //comment this line to put [0;0] on bottom-left-hand corner.
  //y = (kMatrixHeight-1) - y;
  
  if (wrapX == true) {
    while(x > matrixMaxX) x -= kMatrixWidth;
  }
  if (wrapY == true) {
    while(y > matrixMaxY) y -= kMatrixHeight;
  }

  //uncomment if LEDs are arranged in zigzag
  if (x%2 == 0) {
    y = (kMatrixHeight-1) - y;
  }

  return (x * kMatrixHeight) + y;
}


template <uint32_t N> void showSprite(const Sprite<N> &sprite) {
  for (uint8_t y=0; y<sprite.h; y++) {
    for (uint8_t x=0; x<sprite.w; x++) {
      uint8_t nx = sprite.x+x;
      uint8_t ny = sprite.y+y;
      leds[XY(nx, ny, true, false)] = CRGB(sprite.data[x+(y*sprite.w)]);
    }
  }
  FastLED.show();
}


template <uint32_t N> void moveSprite(Sprite<N> &sprite) {
  sprite.x += sprite.dx;
  sprite.y += sprite.dy;
}

void wipeMatrices() {
  for (uint16_t n=0; n<NUM_LEDS; n++) {
    leds[n]  = CHSV(0, 0, 0);
  }  
}

//fireworks
void screenscale( accum88 a, byte N, byte& screen, byte& screenerr) {
  byte ia = a >> 8;
  screen = scale8( ia, N);
  byte m = screen * (256 / N);
  screenerr = (ia - m) * scale8(255,N);
  return;
}


void plot88( byte x, byte y, CRGB& color) {
  byte ix = scale8( x, MODEL_WIDTH);
  byte iy = scale8( y, MODEL_HEIGHT);
  CRGB& px = leds[XY(ix, iy, false, false)];
  px = color;
}


static int16_t scale15by8_local( int16_t i, fract8 scale ) {
    int16_t result;
    result = (int32_t)((int32_t)i * scale) / 256;
    return result;
}

Dot::Dot() {
  show = 0;
  theType = 0;
  x =  0;
  y =  0;
  xv = 0;
  yv = 0;
  r  = 0;
  color.setRGB( 0, 0, 0);
}
  
void Dot::Draw() {
  if( !show) return;
  byte ix, xe, xc;
  byte iy, ye, yc;
  screenscale( x, MODEL_WIDTH, ix, xe);
  screenscale( y, MODEL_HEIGHT, iy, ye);
  yc = 255 - ye;
  xc = 255 - xe;
  
  CRGB c00 = CRGB( dim8_video( scale8( scale8( color.r, yc), xc)), 
                   dim8_video( scale8( scale8( color.g, yc), xc)), 
                   dim8_video( scale8( scale8( color.b, yc), xc))
                   );
  CRGB c01 = CRGB( dim8_video( scale8( scale8( color.r, ye), xc)), 
                   dim8_video( scale8( scale8( color.g, ye), xc)), 
                   dim8_video( scale8( scale8( color.b, ye), xc))
                   );

  CRGB c10 = CRGB( dim8_video( scale8( scale8( color.r, yc), xe)), 
                   dim8_video( scale8( scale8( color.g, yc), xe)), 
                   dim8_video( scale8( scale8( color.b, yc), xe))
                   );
  CRGB c11 = CRGB( dim8_video( scale8( scale8( color.r, ye), xe)), 
                   dim8_video( scale8( scale8( color.g, ye), xe)), 
                   dim8_video( scale8( scale8( color.b, ye), xe))
                   );

  leds[XY(ix, iy, true, false)] += c00;
  leds[XY(ix, iy + 1, true, false)] += c01;
  leds[XY(ix + 1, iy, true, false)] += c10;
  leds[XY(ix + 1, iy + 1, true, false)] += c11;
}

void Dot::Move() {
  saccum78 oyv = yv;
  
  if( !show) return;
  yv -= gGravity;
  xv = scale15by8_local( xv, gDrag);    
  yv = scale15by8_local( yv, gDrag);

  if( theType == SPARK) {
    xv = scale15by8_local( xv, gDrag);    
    yv = scale15by8_local( yv, gDrag);
    color.nscale8( 255);
    if( !color) {
      show = 0;
    }
  }

  // if we'd hit the ground, bounce
  if( yv < 0 && (y < (-yv)) ) {
    if( theType == SPARK ) {
      show = 0;
    } else {
      yv = -yv;
      yv = scale15by8_local( yv, gBounce);
      if( yv < 500 ) {
        show = 0;
      }
    }
  }
  
  if( (yv < -300) /* && (!(oyv < 0))*/ ) {
    // pinnacle
    if( theType == SHELL ) {

      if( (y > (uint16_t)(0x8000)) /*&& (random8() < 64)*/) {
        // boom
        LEDS.showColor( CRGB::White);
        //FastLED.delay( 1);
        LEDS.showColor( CRGB::Black);
      }

      show = 0;

      gSkyburst = 1;
      gBurstx = x;
      gBursty = y;
      gBurstxv = xv;
      gBurstyv = yv;
      gBurstcolor = color;        
    }
  }
  if( theType == SPARK) {
    if( ((xv >  0) && (x > xv)) ||
        ((xv < 0 ) && (x < (0xFFFF + xv))) )  {
      x += xv;
    } else {
      show = 0;
    }
  } else {
    x += xv;
  }
  y += yv;
  
}
  
void Dot::GroundLaunch() {
  yv = 600 + random16(300 + (25 * kMatrixHeight));
  xv = (int16_t)random16(600) - (int16_t)300;
  y = 0;
  x = 0x8000;
  hsv2rgb_rainbow( CHSV( random8(), 240, 200), color);
  show = 1;
}

void Dot::Skyburst( accum88 basex, accum88 basey, saccum78 basedv, CRGB& basecolor) {
  yv = (int16_t)0 + (int16_t)random16(1500) - (int16_t)500;
  xv = basedv + (int16_t)random16(2000) - (int16_t)1000;
  y = basey;
  x = basex;
  color = basecolor;
  color *= 4;
  theType = SPARK;
  show = 1;
}
