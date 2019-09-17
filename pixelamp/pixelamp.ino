/*********************************************************************************************************
 * Credits:
 * Firepit Effect: FastLED Fire 2018 by Stefan Petrick, see https://www.youtube.com/watch?v=SWMu-a9pbyk
 * Firework Effect: FireworksXY by Mark Kriegsman, July 2013
 */

#include <FastLED.h>
#include "pixelamp.h"

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
  mario,
  do_noise,
  matrix,
  heliox,
  invader,
  minecraft,
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

  // MAtrix init
  for (int8_t col=0; col<kMatrixWidth; col++) 
    tab[col]=0;

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

// cheap correction with gamma 2.0
void adjust_gamma() // for do_noise
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i].r = dim8_video(leds[i].r);
    leds[i].g = dim8_video(leds[i].g);
    leds[i].b = dim8_video(leds[i].b);
  }
}

//as shown on youtube
//a noise controlled & modulated by itself
void do_noise() {

  CRGBPalette16 Pal( pit );

  //modulate the position so that it increases/decreases x
  //(here based on the top left pixel - it could be any position else)
  //the factor "2" defines the max speed of the x movement
  //the "-255" defines the median moving direction
  x = x + (2 * noise[0][0]) - 255;
  //modulate the position so that it increases/decreases y
  //(here based on the top right pixel - it could be any position else)
  y = y + (2 * noise[kMatrixWidth-1][0]) - 255;
  //z just in one direction but with the additional "1" to make sure to never get stuck
  //in case the movement is stopped by a crazy parameter (noise data) combination
  //(here based on the down left pixel - it could be any position else)
  z += 1 + ((noise[0][kMatrixHeight-1]) / 4);
  //set the scaling based on left and right pixel of the middle line
  //here you can set the range of the zoom in both dimensions
  scale_x = 8000 + (noise[0][CentreY] * 16);
  scale_y = 8000 + (noise[kMatrixWidth-1][CentreY] * 16);

  //calculate the noise data
 // uint8_t layer = 0;
  for (uint8_t i = 0; i < kMatrixWidth; i++) {
    uint32_t ioffset = scale_x * (i - CentreX);
    for (uint8_t j = 0; j < kMatrixHeight; j++) {
      uint32_t joffset = scale_y * (j - CentreY);
      uint16_t data = inoise16(x + ioffset, y + joffset, z);
      // limit the 16 bit results to the interesting range
      if (data < 11000) data = 11000;
      if (data > 51000) data = 51000;
      // normalize
      data = data - 11000;
      // scale down that the result fits into a byte
      data = data / 161;
      // store the result in the array
      noise[i][j] = data;
    }
  }

  //map the colors
  for (uint8_t y = 0; y < kMatrixHeight; y++) {
    for (uint8_t x = 0; x < kMatrixWidth; x++) {
      //I will add this overlay CRGB later for more colors
      //it´s basically a rainbow mapping with an inverted brightness mask
      CRGB overlay = CHSV(noise[y][x], 255, noise[x][y]);
      //here the actual colormapping happens - note the additional colorshift caused by the down right pixel noise[0][15][15]
      leds[XY(x, y, false,false)] = ColorFromPalette( Pal, noise[kMatrixWidth-1][kMatrixHeight-1] + noise[x][y]) + overlay;
    }
  }

  //make it looking nice
  adjust_gamma();

  //and show it!
  FastLED.show();
}

void invader() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t pos=0;

  for( x = 0; x<15; x++)
    for( y = 0; y<8; y++)
      leds[XY( x, y,false, false)]=leds[XY( x+1, y,false, false)];
           
  for (y = 0; y < 8; y++) 
  {
        idx = invader_map[y][pos>>1];
        if(pos & 0x01 == 1)
          idx=idx & 0x0F;
        else
          idx=idx>>4;

        leds[XY( 15, y,false, false)] = invader_pal[idx];
  }
    FastLED.show();
    delay(500);
    
  pos++;
  if(pos==192) pos=0;
}



void heliox() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t pos=0;

  for( x = 0; x<15; x++)
    for( y = 0; y<8; y++)
      leds[XY( x, y,false, false)]=leds[XY( x+1, y,false, false)];
      
  for (y = 0; y < 8; y++) 
  {
        idx = heliox_map[y][pos>>1];

        if(pos & 0x01 == 1)
          idx=idx & 0x0F;
        else
          idx=idx>>4;

        leds[XY( 15, y,false, false)] = heliox_pal[idx];
  }
    FastLED.show();
    delay(500);
    
  pos++;
  if(pos==41) pos=0;
}


void mario() {
  uint8_t i,x,y, idx,p1,p2;
  static uint8_t pos=0;

  for( i = 0; i<39; i++)
  {
    for (y = 0; y < 8; y++) 
    {
      for (x = 0; x < 4; x++) 
      {
        idx = mario_map[y+(8*i)][x];
        p1=idx>>4;
        p2=idx & 0x0F;
        leds[XY( 2*x+pos, y,true, false)] = mario_pal[p1];
        leds[XY( 2*x+1+pos, y, true, false)] = mario_pal[p2];
        if(p1==1) p1=11;
        if(p2==1) p2=11;
        leds[XY( 2*x+8+pos, y,true, false)] = mario_pal[p1];
        leds[XY( 2*x+9+pos, y, true, false)] = mario_pal[p2];
      }
    }
    FastLED.show();
    delay(500);
    pos++;
    if(pos==15) pos=0;
  }
}

void matrix() {
{
  int8_t spawnX;
  
  EVERY_N_MILLIS(150) // falling speed
  {
    // move code downward
    // start with lowest row to allow proper overlapping on each column
    for (int8_t row=kMatrixHeight-1; row>=0; row--)
    {
      for (int8_t col=0; col<kMatrixWidth; col++)
      {
        if (leds[XY(col, row,false,false)] == CRGB(175,255,175))
        {
          leds[XY(col, row, false, false)] = CRGB(27,130,39); // create trail
          if (row < kMatrixHeight-1) leds[XY(col, row+1, false, false)] = CRGB(175,255,175);
        }
      }
    }

    // fade all leds
    for(int i = 0; i < NUM_LEDS; i++) {
      if (leds[i].g != 255) leds[i].nscale8(192); // only fade trail
    }

    // check for empty screen to ensure code spawn
    bool emptyScreen = true;
    for(int i = 0; i < NUM_LEDS; i++) {
      if (leds[i])
      {
        emptyScreen = false;
        break;
      }
    }

    // spawn new falling code
    if (random8(1) == 0 || emptyScreen) // lower number == more frequent spawns initial 3
    {
      do
      {
        spawnX = random8(kMatrixWidth);
        Serial.printf("you have SX: %d, NB: %d tab: %d\n", spawnX, nb, tab[spawnX]  );
        for (int j=0; j<kMatrixWidth; j++) Serial.printf("%2d ", tab[j]);
        Serial.printf("\n");
      }
      while (nb < kMatrixWidth && tab[spawnX] != 0);

      if (nb < kMatrixWidth && tab[spawnX] ==0)
      {
        nb++;      
        tab[spawnX]=kMatrixHeight;
        leds[XY(spawnX, 0,false, false)] = CRGB(175,255,175 );

      }

      for (int8_t col=0; col<kMatrixWidth; col++) 
      {
        if (tab[col]==1)
          nb--;
        if (tab[col]>0)
          tab[col]-- ;
      }
    }

    FastLED.show();
  }
}

}
 
void minecraft() {
  uint8_t i,x,y, idx,p1,p2;
const CRGB pal[] = {
  {0x21, 0x94, 0x52},  /*Color of index 0*/
  {0x29, 0x39, 0x5a},   /*Color of index 1*/
  {0x6b, 0x6b, 0x6b},   /*Color of index 2*/
  {0x39, 0x52, 0x7b},   /*Color of index 3*/
  {0x42, 0x63, 0x8c},   /*Color of index 4*/
  {0x42, 0x6b, 0x94},   /*Color of index 5*/
  {0x84, 0x84, 0x84},   /*Color of index 6*/
  {0x52, 0x77, 0xa8},   /*Color of index 7*/
  {0x2d, 0x98, 0x56},   /*Color of index 8*/
  {0x31, 0xa5, 0x63},   /*Color of index 9*/
  {0x39, 0xa5, 0x63},   /*Color of index 10*/
  {0x39, 0xad, 0x6b},   /*Color of index 11*/
  {0x42, 0xad, 0x6b},   /*Color of index 12*/
  {0x42, 0xb5, 0x73},   /*Color of index 13*/
  {0x4a, 0xb5, 0x73},   /*Color of index 14*/
  {0x5d, 0xc0, 0x8d}   /*Color of index 15*/
};

const uint8_t minecraft_map[8][8] = {
  0xee, 0xea, 0xac, 0x9c, 0xee, 0xca, 0xca, 0x0c, 
  0xec, 0xfe, 0xe1, 0xca, 0x9f, 0xfe, 0xac, 0xac, 
  0xf1, 0xfa, 0xc1, 0xc1, 0xee, 0xff, 0x18, 0xa1, 
  0x12, 0x11, 0xc1, 0x11, 0x91, 0xc1, 0x31, 0x13, 
  0x53, 0x57, 0x15, 0x33, 0x11, 0x12, 0x35, 0x13, 
  0x31, 0x55, 0x35, 0x11, 0x13, 0x31, 0x33, 0x37, 
  0x71, 0x33, 0x63, 0x37, 0x73, 0x77, 0x35, 0x35, 
  0x73, 0x77, 0x55, 0x33, 0x51, 0x55, 0x33, 0x45, 
};

const uint8_t creeper[8][4] = {
  0x0a, 0xa0, 0x00, 0x0d, 
  0x0d, 0x00, 0xad, 0x0d, 
  0xa1, 0x1a, 0x01, 0x10, 
  0xd1, 0x1a, 0x01, 0x1a, 
  0xd0, 0x01, 0x10, 0xdd, 
  0x0a, 0x11, 0x11, 0x00, 
  0x00, 0x11, 0x11, 0xa0, 
  0x0a, 0x1d, 0x01, 0x00, 
};

 for(uint8_t i = 0; i<128; i++)
   leds[i]=CRGB(255,0,0);
 FastLED.show();
 delay(500);
/*
 wipeMatrices(); 
 for(uint8_t i = 0; i<128; i++)
   leds[i]=CRGB(0,255,0);
 FastLED.show();
 delay(5000);

 wipeMatrices(); 
for(uint8_t i = 0; i<128; i++)
   leds[i]=CRGB(0,0,255);
 FastLED.show();
 delay(5000);

 wipeMatrices(); 
 for(uint8_t i = 0; i<128; i++)
   leds[i]=pal[i/8];
 FastLED.show();
 delay(5000);
 
 wipeMatrices(); 
 leds[XY(0,0,false,false)]=pal[0];
 leds[XY(0,1,false,false)]=pal[1];
 leds[XY(1,1,false,false)]=pal[1];
 leds[XY(0,2,false,false)]=pal[2];
 leds[XY(1,2,false,false)]=pal[2];
 leds[XY(2,2,false,false)]=pal[2];
 FastLED.show();
 delay(5000);

 wipeMatrices(); 
 */
  for( i = 0; i<15; i++)
  {
  for (y = 0; y < 8; y++) {
    for (x = 0; x < 8; x++) {
      idx = minecraft_map[y][x];
 //     p1,p2;
      p1=idx>>4;
      p2=idx & 0x0F;
      leds[XY( 2*x+i, y,true, false)] = pal[p1];
      leds[XY( 2*x+1+i, y, true, false)] = pal[p2];
    }
  }
  FastLED.show();
  delay(500);
  }
  
  for( i = 0; i<15; i++)
  {
  for ( y = 0; y < 8; y++) {
    for ( x = 0; x < 4; x++) {
      idx = creeper[y][x];
      p1,p2;
      p1=idx>>4;
      p2=idx & 0x0F;
      leds[XY( 2*x  +(15-i), y,true, false)] = pal[p1];
      leds[XY( 2*x+1+(15-i), y, true, false)] = pal[p2];
      leds[XY( 2*x+(23-i), y,true, false)] = pal[p1];
      leds[XY( 2*x+1+(23-i), y, true, false)] = pal[p2];
//      Serial.printf (" %#x  %#x ",p1, p2); 
    }
  }
  FastLED.show();
  delay(500);
  }
//  FastLED.show();
//  delay(5000);
}
 
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
//    while(x > matrixMaxX) x -= kMatrixWidth;
      x=x%kMatrixWidth;
  }
  if (wrapY == true) {
//    while(y > matrixMaxY) y -= kMatrixHeight;
    y=y%kMatrixHeight;
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
