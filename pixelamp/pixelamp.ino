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
  MerryGoOn,
  tournesol,
  flower,
//  pulse,
  MatrixSwirl,
  crossfade,
  soulmate1,
  circus_marque,
  pacifica_loop,
  fredplasma,
  invader2,
  tetris,
  mario,
  heliox,
  matrix,
  matrix2,
  invader,
  minecraft,
  animatePacChase,       //#2, pink
  animatePacman,
  hueRotationEffect,      //#1
  do_noise,
  hue,
  fireworks,
  xyTester,               //#0
  firepit,
  all
};

void fixPal(CRGB pal[], uint8_t maxPal)
{
    int i;
    uint8_t t;

  for (i = 0; i < maxPal; i++) // 11
  {
    t = pal[i][0];
    pal[i][0] = pal[i][2];
    pal[i][2] = t;
  }
}


void setup() {
  Serial.begin(115200);
  fixPal(flower_pal, sizeof(flower_pal)/sizeof(CRGB)); //7
  fixPal(tournesol_pal, sizeof(tournesol_pal)/sizeof(CRGB)); //7
  fixPal(MerryGoOn_pal, sizeof(MerryGoOn_pal)/sizeof(CRGB)); //7

  pinMode(POT_ANIM1,INPUT_PULLUP);
  pinMode(POT_ANIM2,INPUT_PULLUP);
  timeanimA=millis();
  lastanimA=digitalRead(POT_ANIM1);


/*
#define  PIN_ADDR_A D5
  pinMode(PIN_ADDR_A, OUTPUT);
  digitalWrite(PIN_ADDR_A, LOW);
*/
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
//  static int oldValue = currentEffect;

  animA = digitalRead(POT_ANIM1);

  if( animA != lastanimA ){ 
    // controle du temps pour eviter des erreurs 
//    if( abs(millis() - timeanimA) > 50 )
    {
      // Si B different de l'ancien état de A alors
      if(digitalRead(POT_ANIM2) != lastanimA){
        Serial.println("Moins");
        if (currentEffect>0) 
          currentEffect--;
        else
          currentEffect=ARRAY_SIZE(effects)-1;
        wipeMatrices();
      }
      else{
        Serial.println("plus");
        if ( currentEffect < ARRAY_SIZE(effects)-1) 
          currentEffect++;
        else
          currentEffect=0;
        wipeMatrices(); 
      }
      // memorisation du temps pour A
      timeanimA = millis();
    } 
    // memorisation de l'état de A
    lastanimA = animA ;
    
    //affichage du compteur
    Serial.print("currentEffect :");
    Serial.println(currentEffect);
  }
/*  
  int potValue = analogRead(POT_ANIM);
//  Serial.print (potValue); Serial.print ("\n");
  potValue = constrain(potValue, POT_ANIM_MIN, POT_ANIM_MAX);
  int newValue = map(potValue, POT_ANIM_MIN, POT_ANIM_MAX, 0, ARRAY_SIZE(effects)-1);

 Serial.printf("ANI %d : %d : %d\n",potValue, newValue, oldValue);

  if (newValue != oldValue) {
    oldValue = newValue;
    currentEffect = newValue;
    wipeMatrices();
  }
*/
}

void changeBrightness() {
  static int oldValue = brightness;
//  digitalWrite(PIN_ADDR_A, HIGH);
  int potValue = analogRead(POT_BRIGHTNESS);
  potValue = constrain(potValue, POT_BRIGHTNESS_MIN, POT_BRIGHTNESS_MAX);
  int newValue = map(potValue, POT_BRIGHTNESS_MIN, POT_BRIGHTNESS_MAX, 0, 255);
//  Serial.printf("BRI %d : %d : %d\n",potValue, newValue, oldValue);

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

 
////////////////////////////////////////////
// INVADER2


void shoot()
{
    static int d = 0, p = 5, dir = 1 ;
    static int tirX[4], tirY[4];
    static bool tir[4] = {false, false, false, false};


      

    if (++d == 32)
    {

        for (int y = 0; y < 4; y++)
          if (tir[y] == true)
          {
              tirY[y]--;
              if (tirY[y] == -1)
                  tir[y] = false;
          }

        if (p == 2)
        {
            tirX[0] = p + 1;
            tirY[0] = 5;
            tir[0] = true;
        }
        if (p == 13)
        {
            tirX[1] = p + 1;
            tirY[1] = 5;
            tir[1] = true;
        }
        if (p == 9)
        {
            tirX[2] = p + 1;
            tirY[2] = 5;
            tir[2] = true;
        }
        if (p == 5)
        {
            tirX[3] = p + 1;
            tirY[3] = 5;
            tir[3] = true;
        }

        if (dir == 1 && p == 13)
            dir = -1;
        if (dir == -1 && p == 0)
            dir = 1;

        if (dir == 1 && p < 13)
            p++;
        if (dir == -1 && p > 0)
            p--;

        d = 0;
    }

    leds[XY(p, 7,false,false)] = CRGB(255, 255, 255);
    leds[XY(p + 1, 7,false,false)] = CRGB(255, 255, 255);
    leds[XY(p + 2, 7,false,false)] = CRGB(255, 255, 255);
    leds[XY(p + 1, 6,false,false)] = CRGB(255, 255, 255);

    for (int y = 0; y < 4; y++)
      if (tir[y] == true)
            leds[XY(tirX[y], tirY[y], false,false)] = CRGB(128, 128, 255);


}

void displayImage(uint64_t image, int c)
{
   int y;
   static  int posi = 0, b = 0, p = 5, dir = 1;
           
    for (y = 0; y < 8; y++)
    {
        byte row = (image >> y * 8) & 0xFF;
        for (int x = 0; x < 8; x++)
        {
            if (bitRead(row, x))
            {
                if(c>255) c=255;
                leds[XY(x + posi/50, y, true, true)] = CRGB(c, c, c);
            }
        }
    }
    
    if (dir==1){
      posi ++;
      if (posi == 400) dir = -1;
    }
    else if (dir == -1) {
      posi--;
      if (posi == 0) dir = 1;
    }
}

void DrawOneFrame( byte startHue8, int8_t yHueDelta8, int8_t xHueDelta8) {
  byte lineStartHue = startHue8;
  for( byte y = 0; y < kMatrixHeight; y++) {
    lineStartHue += yHueDelta8;
    byte pixelHue = lineStartHue;      
    for( byte x = 0; x < kMatrixWidth; x++) {
      pixelHue += xHueDelta8;
      leds[XY(x, y, true, true)]  = CHSV( pixelHue, 200, 200);
    }
  }
}

void hueRotationEffect() {
  uint32_t ms = millis();
  int32_t yHueDelta32 = ((int32_t)cos16( ms * (27/1) ) * (350 / kMatrixWidth));
  int32_t xHueDelta32 = ((int32_t)cos16( ms * (39/1) ) * (310 / kMatrixHeight));
  DrawOneFrame( ms / 65536, yHueDelta32 / 32768, xHueDelta32 / 32768);
  FastLED.show();
}

void invader2()
{
    uint32_t ms = millis();
    static int nbi=0 , col=0;
    static bool way = true;

    DrawOneFrame(col, 8, 16 );
    if(nbi&0x01)
      displayImage(INVADER[nbi>>1], col);
    else
      shoot();
    FastLED.show();

    if (way)
    {
      if(++col==400) {
        way=false; 
      }
    }
    else
    {
      if(--col==0) {
        way=true; 
      if(++nbi==IMAGES_LEN<<1) nbi=0;
      }
    }


}    


////////////////////////////////////////////


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
  if(pos==44) pos=0;
}

void tetris() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t anim=0;

    for (y = 0; y < 8; y++) 
    {
      for (x = 0; x < 4; x++) 
      {
        idx = tetris_map[y+(8*anim)][x];
        p1=idx>>4;
        p2=idx & 0x0F;
        leds[XY( 2*x, y,true, false)] = tetris_pal[p1];
        leds[XY( 2*x+1, y, true, false)] = tetris_pal[p2];
        if(p1==1) p1=11;
        if(p2==1) p2=11;
        leds[XY( 2*x+8, y,true, false)] = tetris_pal[p1];
        leds[XY( 2*x+9, y, true, false)] = tetris_pal[p2];
      }
    }
    FastLED.show();
    delay(500);
    anim++;
    if(anim==91) anim=0;
 
}

void flower() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t anim=0;

    for (y = 0; y < 8; y++) 
    {
      for (x = 0; x < 8; x++) 
      {
        idx = flower_map[y+(8*anim)][x];
        p1=idx>>4;
        p2=idx & 0x0F;
        leds[XY( 2*x, y,true, false)] = flower_pal[p1];
        leds[XY( 2*x+1, y,true, false)] = flower_pal[p2];
      }
    }
    FastLED.show();
    delay(500);
    anim++;
    if(anim==18) anim=0;
 
}

void MerryGoOn() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t anim=0;

    for (y = 0; y < 8; y++) 
    {
      for (x = 0; x < 8; x++) 
      {
        idx = MerryGoOn_map[y+(8*anim)][x];
        p1=idx>>4;
        p2=idx & 0x0F;
        leds[XY( 2*x, y,true, false)] = MerryGoOn_pal[p1];
        leds[XY( 2*x+1, y,true, false)] = MerryGoOn_pal[p2];
      }
    }
    FastLED.show();
    delay(500);
    anim++;
    if(anim==4) anim=0;
 
}



void tournesol() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t anim=0;

    for (y = 0; y < 8; y++) 
    {
      for (x = 0; x < 8; x++) 
      {
        idx = tournesol_map[y+(8*anim)][x];
        p1=idx>>4;
        p2=idx & 0x0F;
        leds[XY( 2*x, y,true, false)] = tournesol_pal[p1];
        leds[XY( 2*x+1, y,true, false)] = tournesol_pal[p2];
      }
    }
    FastLED.show();
    delay(500);
    anim++;
    if(anim==16) anim=0;

}


void mario() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t pos=0, anim=0;

    for (y = 0; y < 8; y++) 
    {
      for (x = 0; x < 4; x++) 
      {
        idx = mario_map[y+(8*anim)][x];
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
    anim++;
    if(pos==15) pos=0;
    if(anim==39) anim=0;
 
}

void matrix() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t pos=0, anim=0;

    for (y = 0; y < 8; y++) 
    {
      for (x = 0; x < 8; x++) 
      {
        idx = matrix_map[y+(8*anim)][x];
        p1=idx>>4;
        p2=idx & 0x0F;
        leds[XY( 2*x, y,true, false)] = matrix_pal[p1];
        leds[XY( 2*x+1, y, true, false)] = matrix_pal[p2];
      }
    }
    FastLED.show();
    delay(50);
    anim++;
    if(anim==32) anim=0;
}

void matrix2() {
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
 
void minecraft() {
  uint8_t x,y, idx,p1,p2;
  static uint8_t pos=0;

  for( x = 0; x<15; x++)
    for( y = 0; y<8; y++)
      leds[XY( x, y,false, false)]=leds[XY( x+1, y,false, false)];
      
  for (y = 0; y < 8; y++) 
  {
        idx = minecraftpic_map[y][pos>>1];

        if(pos & 0x01 == 1)
          idx=idx & 0x0F;
        else
          idx=idx>>4;

        leds[XY( 15, y,false, false)] = minecraftpic_pal[idx];
  }
    FastLED.show();
    delay(500);
    
  pos++;
  if(pos==108) pos=0;

/*
  uint8_t x,y, idx,p1,p2;
  static uint8_t pos=0, anim=0;


 if(anim==0)
 {
  for (y = 0; y < 8; y++) {
    for (x = 0; x < 8; x++) {
      idx = minecraft_map[y][x];
      p1=idx>>4;
      p2=idx & 0x0F;
      leds[XY( 2*x+pos, y,true, false)] = pal[p1];
      leds[XY( 2*x+1+pos, y, true, false)] = pal[p2];
    }
  }
 }
 else
 {
    for ( y = 0; y < 8; y++) {
    for ( x = 0; x < 4; x++) {
      idx = creeper[y][x];
      p1=idx>>4;
      p2=idx & 0x0F;
      leds[XY( 2*x  +(pos), y,true, false)] = pal[p1];
      leds[XY( 2*x+1+(pos), y, true, false)] = pal[p2];
      leds[XY( 2*x+(pos+8), y,true, false)] = pal[p1];
      leds[XY( 2*x+1+(pos+8), y, true, false)] = pal[p2];
//      Serial.printf (" %#x  %#x ",p1, p2); 
    }
  }
 }
  
  pos++;
  if(pos==15) { pos=0; anim= 1-anim;}
  
  FastLED.show();
  delay(500);
  */
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

#define DELAY 100
void all() {
  static uint8_t cur=0, i=0;
   
  effects[cur]();
  i++;
  if ( (cur<8 && i==DELAY) || (cur>=8 && i==DELAY<<1) ) {cur ++; i=0; wipeMatrices(); }
 
  if (cur == sizeof (effects)-1 ) cur=0;
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

uint16_t XY(uint8_t x, uint8_t y) {
  return (XY(x,y,false,false)) ;
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

////////////////////////////

// fred''s plasma
static int PlasmaTime = 0;
static int PlasmaShift = 1;
#define PLASMA_X_FACTOR     16
#define PLASMA_Y_FACTOR     8
void fredplasma() 
{
  for (int y = 0; y < kMatrixHeight; y++) 
  {
    for (int x = 0; x < kMatrixWidth ; x++) 
    {
      // XY tells us the index of a given X/Y coordinate
      int index = XY(x, y, false, false);
      int r = sin16(PlasmaTime) / 256;
      int hue = sin16(x * r * PLASMA_X_FACTOR + PlasmaTime) +
          cos16(y * (-r) * PLASMA_Y_FACTOR + PlasmaTime) +
          sin16(y * x * (cos16(-PlasmaTime) / 256) / 2);
      PlasmaTime += PlasmaShift;
      leds[index] = CHSV(hue / 256, 255, 255);
    }
  }
  FastLED.show();
}


/////////////////////////////


CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };


void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011,10,13));
  sCIStart2 -= (deltams21 * beatsin88(777,8,11));
  sCIStart3 -= (deltams1 * beatsin88(501,5,7));
  sCIStart4 -= (deltams2 * beatsin88(257,4,6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();

  FastLED.show();
}

// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}

//////////////////////////////////////////////////////////////////////////////

// 3 is normal (1/3rd of lights lit), but 4 or 5 work.
#define CYCLE_LENGTH 3


void circus_marque() 
{
  uint8_t knob = 60;
//#ifdef KNOB_PIN
  knob = analogRead( A0) / 4;
//#endif
  knob = (knob & 0xF0) + 0x0F;
  uint16_t time_divider = knob * 4;

  // fade like the way incandescent bulbs fade
  fadeUsingColor( leds, NUM_LEDS, CRGB(200,185,150));

  uint8_t stepN = (millis() / time_divider) % CYCLE_LENGTH;
  uint8_t modN = 0;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t modN = addmod8( modN, 1, CYCLE_LENGTH);
    if( modN != stepN ) {
      // add some power to the bulbs that are lit right now
      leds[i] += CRGB( 80, 50, 20);
    }
  }
  
  FastLED.delay(10);
}


/////////////////////////////////////////////

float offsetX = 0;
float offsetY = 0;
uint8_t hue2 = 0;

void soulmate1() 
{
  offsetX = beatsin16(6, -180, 180);
  offsetY = beatsin16(6, -180, 180, 12000);

  EVERY_N_MILLISECONDS(10) {
    hue2++;
  }

  for (int x = 0; x < kMatrixHeight; x++) {
    for (int y = 0; y < kMatrixWidth; y++) {
      int16_t index = XY(y, x);

      if (index < 0) break;

      float hue2 = x * beatsin16(10, 1, 10) + offsetY;
      leds[index] = CHSV(hue2, 200, sin8(x * 30 + offsetX));
      hue2 = y * 3 + offsetX;
      leds[index] += CHSV(hue2, 200, sin8(y * 30 + offsetY));
    }
  }
  FastLED.show();
}

///////////////////////////////////////////////////

const uint8_t kSquareWidth = 8;
const uint8_t kBorderWidth = 1;

void MatrixSwirl()
{
  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = dim8_raw( beatsin8(3,64,192) );
  blur2d( leds, kSquareWidth, kSquareWidth, blurAmount);

  // Use two out-of-sync sine waves
  uint8_t  i = beatsin8(  91, kBorderWidth, kSquareWidth-kBorderWidth);
  uint8_t  j = beatsin8( 109, kBorderWidth, kSquareWidth-kBorderWidth);
  uint8_t  k = beatsin8(  73, kBorderWidth, kSquareWidth-kBorderWidth);
  
  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();  
  leds[XY( i, j)] += CHSV( ms / 29, 200, 255);
  leds[XY( j, k)] += CHSV( ms / 41, 200, 255);
  leds[XY( k, i)] += CHSV( ms / 73, 200, 255);

  for (int a=0; a<64; a++) leds[a+64]=leds[a];
  
  FastLED.show();
  FastLED.delay(50);
}

//////////////////////////////////////////////////////////////////

#define UPDATES_PER_SECOND 100

CRGBPalette16 currentPalette( CRGB::Black);
CRGBPalette16 targetPalette( PartyColors_p );

void crossfade()
{
  ChangePalettePeriodically();

  // Crossfade current palette slowly toward the target palette
  //
  // Each time that nblendPaletteTowardPalette is called, small changes
  // are made to currentPalette to bring it closer to matching targetPalette.
  // You can control how many changes are made in each call:
  //   - the default of 24 is a good balance
  //   - meaningful values are 1-48.  1=veeeeeeeery slow, 48=quickest
  //   - "0" means do not change the currentPalette at all; freeze
  
  uint8_t maxChanges = 24; 
  nblendPaletteTowardPalette( currentPalette, targetPalette, maxChanges);


  static uint8_t startIndex = 0;
  startIndex = startIndex + 1; /* motion speed */
  FillLEDsFromPaletteColors( startIndex);

  FastLED.show();
  FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  uint8_t brightness = 255;
  
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex + sin8(i*16), brightness);
    colorIndex += 3;
  }
}


void ChangePalettePeriodically()
{
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;
  
  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    CRGB p = CHSV( HUE_PURPLE, 255, 255);
    CRGB g = CHSV( HUE_GREEN, 255, 255);
    CRGB b = CRGB::Black;
    CRGB w = CRGB::White;
    if( secondHand ==  0)  { targetPalette = RainbowColors_p; }
    if( secondHand == 10)  { targetPalette = CRGBPalette16( g,g,b,b, p,p,b,b, g,g,b,b, p,p,b,b); }
    if( secondHand == 20)  { targetPalette = CRGBPalette16( b,b,b,w, b,b,b,w, b,b,b,w, b,b,b,w); }
    if( secondHand == 30)  { targetPalette = LavaColors_p; }
    if( secondHand == 40)  { targetPalette = CloudColors_p; }
    if( secondHand == 50)  { targetPalette = PartyColors_p; }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////
/*

void drawCircle(int16_t x0, int16_t y0, uint16_t radius, const CRGB& color)
{
  int a = radius, b = 0;
  int radiusError = 1 - a;

  if (radius == 0) {
    leds[XY(x0, y0)] = color;
    return;
  }

  while (a >= b)
  {
    leds[XY(a + x0, b + y0)] = color;
    leds[XY(b + x0, a + y0)] = color;
    leds[XY(-a + x0, b + y0)] = color;
    leds[XY(-b + x0, a + y0)] = color;
    leds[XY(-a + x0, -b + y0)] = color;
    leds[XY(-b + x0, -a + y0)] = color;
    leds[XY(a + x0, -b + y0)] = color;
    leds[XY(b + x0, -a + y0)] = color;

    b++;
    if (radiusError < 0)
      radiusError += 2 * b + 1;
    else
    {
      a--;
      radiusError += 2 * (b - a + 1);
    }
  }
}

void dimAll(byte value)
{
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value);
  }
}

CRGB solidColor = CRGB::White;

typedef uint16_t(*PatternFunctionPointer)();
typedef PatternFunctionPointer PatternList [];

int currentPatternIndex = 0;
PatternFunctionPointer currentPattern;

CRGB w(85, 85, 85), W(CRGB::White);
CRGBPalette16 snowColors = CRGBPalette16( W, W, W, W, w, w, w, w, w, w, w, w, w, w, w, w );

CRGBPalette16 incandescentColors = CRGBPalette16( l, l, l, l, l, l, l, l, l, l, l, l, l, l, l, l );

const CRGBPalette16 palettes[] = {
  RainbowColors_p,
  RainbowStripeColors_p,
  OceanColors_p,
  CloudColors_p,
  ForestColors_p,
  PartyColors_p,
  HeatColors_p,
  LavaColors_p,
  snowColors,
};

const int paletteCount = ARRAY_SIZE(palettes);

int currentPaletteIndex = 0;
CRGBPalette16 palette = palettes[0];

void  pulse() {
//  palette = RainbowColors_p;

  static uint8_t hue = 0;
  static uint8_t centerX = 0;
  static uint8_t centerY = 0;
  static uint8_t step = 0;

  static const uint8_t maxSteps = 16;
  static const float fadeRate = 0.8;

  dimAll(235);

  if (step == 0) {
    centerX = random(16);
    centerY = random(8);
    hue = random(256); // 170;

    drawCircle(centerX, centerY, step, ColorFromPalette(palette, hue));
    step++;
  }
  else {
    if (step < maxSteps) {
      // initial pulse
      drawCircle(centerX, centerY, step, ColorFromPalette(palette, hue, pow(fadeRate, step - 2) * 255));

      // secondary pulse
      if (step > 3) {
        drawCircle(centerX, centerY, step - 3, ColorFromPalette(palette, hue, pow(fadeRate, step - 2) * 255));
      }
      step++;
    }
    else {
      step = 0;
    }
  }

  //return 30;
}

*/
