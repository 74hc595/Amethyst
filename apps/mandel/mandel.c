#include "app.h"

#define XR 80
#define YR 100
static const int C1 = 256;
static uint8_t screen[XR*YR];
static uint8_t backbuf[XR*YR];

static const uint8_t color_order[256] PROGMEM = {
  0, 80, 160, 240, 5, 10, 85, 90, 165, 170, 245, 250, 15, 95, 175, 255, 192, 148, 12, 76, 44, 60, 92, 172, 197, 108, 204, 236, 124, 252, 158, 207, 72, 104, 200, 232, 28, 140, 202, 156, 213, 188, 220, 77, 109, 205, 237, 143, 208, 88, 120, 133, 138, 248, 218, 13, 45, 141, 93, 173, 125, 221, 253, 223, 128, 8, 40, 24, 136, 56, 168, 152, 184, 216, 201, 233, 29, 61, 157, 189, 144, 9, 41, 73, 137, 149, 154, 105, 89, 169, 153, 121, 185, 217, 249, 159, 16, 176, 1, 129, 17, 193, 145, 209, 177, 25, 57, 139, 75, 203, 155, 219, 81, 161, 21, 26, 241, 181, 186, 11, 27, 91, 171, 235, 187, 31, 251, 191, 65, 33, 97, 49, 225, 113, 131, 19, 147, 211, 179, 43, 59, 107, 123, 63, 48, 3, 35, 67, 99, 51, 53, 58, 83, 163, 195, 115, 227, 243, 151, 183, 2, 112, 34, 18, 130, 50, 146, 82, 114, 178, 210, 37, 42, 7, 23, 55, 32, 66, 98, 162, 242, 117, 122, 39, 135, 87, 167, 119, 215, 247, 47, 127, 96, 194, 6, 226, 38, 22, 54, 86, 150, 101, 106, 182, 71, 103, 199, 231, 36, 52, 116, 70, 134, 102, 166, 198, 118, 230, 214, 246, 46, 110, 62, 111, 4, 68, 20, 100, 228, 180, 244, 14, 78, 94, 174, 238, 126, 254, 79, 239, 64, 224, 132, 84, 164, 196, 212, 69, 74, 229, 234, 142, 30, 206, 222, 190
};
static uint8_t color(uint8_t c) { return pgm_read_byte(color_order+c); }

static int CA = 16;
static int CB = 16;
static int C4 = 5*C1;
static int RN = -2*C1;
static int RX = 2*C1;
static int IN = -5*C1/4;
static int IX = 5*C1/4;
static int MC = 12;
static uint8_t PAL = 0;

static void retint(uint8_t t) {
  uint8_t *p = screen;
  uint8_t *b = backbuf;
  uint16_t n = sizeof(screen);
  while (--n) {
    *p = color((*b)+t);
    p++; b++;
  }
}


static void mandel(void) {
  int RD = (RX - RN)/(XR - 1);
  int ID = (IX - IN)/(YR - 1);
  int R;
  int I = IN;
  int TR, TI;
  char *px = backbuf;
  char *spx = screen;
  for (uint8_t y = 0; y < YR; y++) {
    R = RN;
    for (uint8_t x = 0; x < XR; x++) {
      int ZR = 0;
      int ZI = 0;
      int R2 = 0;
      int I2 = 0;
      uint8_t CT = 0;
      do {
        TR = R2 - I2 + R;
        TI = ZR*2/CA*ZI/CB + I;
        ZR = TR;
        ZI = TI;
        R2 = ZR/CA*ZR/CB;
        I2 = ZI/CA*ZI/CB;
        CT++;
      } while ((CT<MC) && (R2+I2 < C4));
      *px = CT & 15;
      *spx = color((CT&15) + PAL);
      px++;
      spx++;
      R += RD;
    }
    I += ID;
  }
}


#define Q 64

APP_MAIN(mandel)
{
  init_80x100x256_bitmap_mode(screen);
  video_start();
  struct keyreader kr = {0};

  mandel();
  uint8_t n = 0;
  while (frame_sync()) {
    if (n == 7) { PAL++; retint(PAL); n = 0; } n++;
    switch (get_key_ascii(&kr)) {
      case KC_LEFT: RN-=Q; RX-=Q; goto draw;
      case KC_RIGHT: RN+=Q; RX+=Q; goto draw;
      case KC_UP: IN-=Q; IX-=Q; goto draw;
      case KC_DOWN: IN+=Q; IX+=Q; goto draw;
      case 'a': RN+=Q; RX-=Q; IN+=Q; IX-=Q; goto draw;
      case 'z': RN-=Q; RX+=Q; IN-=Q, IX+=Q; goto draw;
      case 's': PAL+=1; goto tint;
      case 'x': PAL-=1; goto tint;
      default: break;
      draw: mandel(); break;
      tint: retint(PAL);
    }
  }
}
