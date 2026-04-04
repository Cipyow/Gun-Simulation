#include "src/screens/glock.h"
#include "raylib.h"
#include "screen_type.h"
#include "src/algo/bresenham.h"
#include "src/algo/dda.h"
#include "src/algo/midcircle.h"
#include "src/ui/back_button.h"
#include "src/ui/audio_manager.h"
#include <math.h>
#include <stdlib.h>

#define S 2.4f
#define P(v) ((int)((v) * S))

/* ── Colors ───────────────────────────────────────────────────────────── */
#define C_SL (Color){50, 50, 52, 145}     /* slide transparent */
#define C_SLE (Color){38, 38, 40, 255}    /* slide edge */
#define C_SLH (Color){68, 68, 72, 255}    /* slide highlight */
#define C_SLT (Color){58, 58, 60, 200}    /* slide top */
#define C_FR (Color){40, 40, 38, 255}     /* frame */
#define C_FRL (Color){50, 50, 48, 255}    /* frame light */
#define C_FRD (Color){30, 30, 28, 255}    /* frame dark */
#define C_GR (Color){38, 38, 36, 155}     /* grip transparent */
#define C_GRT (Color){32, 32, 30, 155}    /* grip texture */
#define C_GRH (Color){46, 46, 44, 155}    /* grip highlight */
#define C_BAR (Color){78, 78, 80, 255}    /* barrel */
#define C_BORE (Color){18, 18, 18, 255}   /* bore */
#define C_SPR (Color){128, 128, 126, 255} /* spring */
#define C_ROD (Color){148, 148, 146, 255} /* guide rod */
#define C_STK (Color){108, 108, 106, 255} /* striker */
#define C_BR (Color){212, 172, 52, 255}   /* brass */
#define C_BRD (Color){182, 142, 42, 255}  /* brass dark */
#define C_COP (Color){192, 122, 56, 255}  /* copper */
#define C_PRM (Color){172, 158, 132, 255} /* primer */
#define C_MAG (Color){42, 42, 42, 130}    /* mag transparent */
#define C_MAGE (Color){55, 55, 52, 200}   /* mag edge */
#define C_TRG (Color){40, 40, 38, 255}    /* trigger */
#define C_CTL (Color){55, 55, 52, 255}    /* controls */
#define C_SDT (Color){28, 28, 28, 255}    /* sight */
#define C_SDW (Color){225, 225, 225, 255} /* sight dot */
#define C_LCK (Color){62, 62, 58, 255}    /* slide lock lever */
#define C_LCKH (Color){78, 78, 74, 255}   /* slide lock highlight */
#define C_LCKD (Color){42, 42, 38, 255}   /* slide lock dark edge */

/* ── State ────────────────────────────────────────────────────────────── */
typedef enum {
  IDLE,
  PULLING,
  FIRE,
  RECOIL,
  EJECT,
  FEEDING,
  LOCKED,
  MAG_DROP,
  MAG_INS
} St;
static St st = IDLE;
static float sldX = 0, trgP = 0, bulX = 0;
static float csX = 0, csY = 0, csR = 0, csVX = 0, csVY = 0;
static float fdT = 0, magY = 0;
static float kX = 0, kY = 0;
static int mag = 7, chm = 1, safeMode = 0;
static float ejT = 0;
static float lockT = 0;

/* particles */
#define MP 25
typedef struct {
  float x, y, vx, vy, l, ml;
  int t;
  Color c;
} Pt;
static Pt pts[MP];
static int pn = 0;
static void AddP(float x, float y, float vx, float vy, float l, int t,
                 Color c) {
  if (pn >= MP)
    return;
  pts[pn++] = (Pt){x, y, vx, vy, l, l, t, c};
}
static void FlashP(float x, float y) {
  for (int i = 0; i < 6; i++) {
    float a = -0.2f + (float)(rand() % 40) / 100.0f;
    float sp = 200 + (float)(rand() % 250);
    AddP(x, y, cosf(a) * sp, sinf(a) * sp - 30,
         0.04f + (float)(rand() % 25) / 1000.0f, 0,
         (Color){255, (unsigned char)(210 + rand() % 45),
                 (unsigned char)(60 + rand() % 60), 255});
  }
  for (int i = 0; i < 3; i++)
    AddP(x, y, 20 + (float)(rand() % 40), -(float)(rand() % 25) - 10,
         0.3f + (float)(rand() % 20) / 100.0f, 1, (Color){140, 140, 140, 90});
}

/* ── Helpers ──────────────────────────────────────────────────────────── */
static void FR(int x, int y, int w, int h, Color c) {
  for (int r = 0; r < h; r++)
    DDALine(x, y + r, x + w, y + r, c);
}

static void FillQuad(int x1, int y1, int x2, int y2, int x3, int y3, int x4,
                     int y4, Color c) {
  int minY = y1, maxY = y1;
  if (y2 < minY)
    minY = y2;
  if (y2 > maxY)
    maxY = y2;
  if (y3 < minY)
    minY = y3;
  if (y3 > maxY)
    maxY = y3;
  if (y4 < minY)
    minY = y4;
  if (y4 > maxY)
    maxY = y4;
  int ex[] = {x1, x2, x3, x4}, ey[] = {y1, y2, y3, y4};
  for (int yy = minY; yy <= maxY; yy++) {
    int xn = 9999, xx = -9999;
    for (int e = 0; e < 4; e++) {
      int e2 = (e + 1) % 4, ya = ey[e], yb = ey[e2], xa = ex[e], xb = ex[e2];
      if (ya > yb) {
        int t = ya;
        ya = yb;
        yb = t;
        t = xa;
        xa = xb;
        xb = t;
      }
      if (yy >= ya && yy <= yb && ya != yb) {
        int ix = xa + (xb - xa) * (yy - ya) / (yb - ya);
        if (ix < xn)
          xn = ix;
        if (ix > xx)
          xx = ix;
      }
    }
    if (xn <= xx)
      DDALine(xn, yy, xx, yy, c);
  }
}

static float smoothstep(float t) {
  if (t < 0)
    t = 0;
  if (t > 1)
    t = 1;
  return t * t * (3 - 2 * t);
}

#define SL P(130) /* slide length */
#define SH P(20)  /* slide height */
#define FH P(6)   /* frame rail height */

/* Grip corners (parallelogram, angled ~20° backward) */
#define G_TLX P(5)     /* top-left (backstrap top) */
#define G_TRX P(46)    /* top-right (front-strap top) */
#define G_TY FH        /* top Y = frame bottom */
#define G_BLX (-P(16)) /* bottom-left (shifted left by grip angle) */
#define G_BRX P(25)    /* bottom-right */
#define G_BY P(72)     /* bottom Y */

/* Trigger guard */
#define TG_LX P(44)
#define TG_RX P(65)
#define TG_TY FH
#define TG_BY P(32)

/* Dust cover */
#define DC_LX P(65)
#define DC_RX P(122)

static void DrawInternals(int ox, int oy, int so) {
  int barY = oy - P(12);
  /* barrel */
  Bres_ThickLine(ox + P(28), barY, ox + SL - P(2), barY, P(7), C_BAR);
  DDA_ThickLine(ox + P(28), barY, ox + SL - P(2), barY, P(3), C_BORE);
  /* barrel hood */
  Bres_ThickLine(ox + P(26), barY, ox + P(40), barY, P(10), C_BAR);
  /* muzzle */
  MidcircleThick(ox + SL - P(2), barY, P(4), P(1), C_BAR);
  MidcircleFilled(ox + SL - P(2), barY, P(2), C_BORE);

  /* guide rod */
  DDA_ThickLine(ox + P(38), oy - P(4), ox + P(118), oy - P(4), P(1),
                (Color){148, 148, 146, 200});
  /* recoil spring */
  float sLen = (float)(P(75)) + (float)so;
  int ss = ox + P(42);
  for (int i = 0; i < 16; i++) {
    float t0 = (float)i / 16.0f, t1 = (float)(i + 1) / 16.0f;
    int cx0 = ss + (int)(sLen * t0), cx1 = ss + (int)(sLen * t1);
    int dy = (i % 2 == 0) ? P(3) : -P(3);
    BresenhamLine(cx0, oy - P(4) + dy, cx1, oy - P(4) - dy, C_SPR);
  }
  /* striker */
  int skX = ox + P(22) + so;
  DDA_ThickLine(skX, oy - P(13), skX + P(16), oy - P(13), P(2), C_STK);
  MidcircleFilled(skX + P(16), oy - P(13), P(1), (Color){140, 140, 138, 255});
}

static void DrawMagInt(int ox, int oy, float mOff, int rds) {
  /* Angle matching grip */
  float ang = 20.0f * DEG2RAD;
  float sa = sinf(ang), ca = cosf(ang);

  int myTop = oy + P(12) + (int)mOff;
  int mH = P(66);
  int myBot = myTop + (int)(ca * mH);
  if (mOff > P(150))
    return;
  int segs = 12;
  int centerTopX = ox + P(22);
  int centerBotX = centerTopX - (int)(sa * mH);

  int fY = myTop + P(6) + rds * P(7);
  if (fY > myBot - P(10))
    fY = myBot - P(10);

  // Follower Center X
  float tFol = (float)(fY - myTop) / (float)(myBot - myTop);
  int fX = centerTopX - (int)(tFol * (centerTopX - centerBotX));

  int sB_Y = myBot - P(4);
  int sT_Y = fY + P(2);

  for (int i = 0; i < segs; i++) {
    float t1 = (float)i / segs, t2 = (float)(i + 1) / segs;
    int sy1 = sB_Y - (int)((sB_Y - sT_Y) * t1);
    int sx1 = centerBotX + (int)((fX - centerBotX) * t1);
    int sy2 = sB_Y - (int)((sB_Y - sT_Y) * t2);
    int sx2 = centerBotX + (int)((fX - centerBotX) * t2);

    int dx1 = (i % 2 == 0) ? P(6) : -P(6);
    int dx2 = (i % 2 == 0) ? -P(6) : P(6);
    DDALine(sx1 + dx1, sy1, sx2 + dx2, sy2, (Color){118, 118, 115, 255});
  }

  /* follower */
  Bres_ThickLine(fX - P(10), fY, fX + P(10), fY, P(3),
                 (Color){88, 88, 85, 255});
  /* rounds */
  for (int i = 0; i < rds && i < 7; i++) {
    int ry = myTop + P(6) + (rds - 1 - i) * P(7);
    if (ry < myTop - P(4))
      break;
    float tBul = (float)(ry - myTop) / (float)(myBot - myTop);
    int cx = centerTopX - (int)(tBul * (centerTopX - centerBotX));
    int rx = cx - P(3);
    int stg = (i % 2 == 0) ? 0 : P(3);
    Bres_ThickLine(rx - P(6) + stg, ry, rx + P(6) + stg, ry, P(5), C_BRD);
    MidcircleFilled(rx + P(8) + stg, ry, P(3), C_COP);
    BresenhamLine(rx - P(7) + stg, ry - P(2), rx - P(7) + stg, ry + P(2), C_BR);
  }
}

static void DrawChRound(int ox, int oy) {
  int cy = oy - P(12), cx = ox + P(28);
  DDA_ThickLine(cx, cy, cx + P(12), cy, P(5), C_BR);
  MidcircleFilled(cx + P(14), cy, P(3), C_COP);
  MidcircleFilled(cx - P(1), cy, P(2), C_PRM);
}

static void DrawFeedRound(int ox, int oy, float lerp) {
  float t = smoothstep(lerp > 1 ? 1 : lerp);

  int sx = ox + P(16), sy = oy + P(6);  // magazine feed lips
  int ex = ox + P(28), ey = oy - P(12); // chamber

  int fx = sx + (int)((ex - sx) * t);
  int fy = sy - (int)((sy - ey) * t);

  float ang = sinf(t * M_PI) * (18.0f * DEG2RAD);
  float ca = cosf(ang), sa = sinf(ang);

  int endX = fx + (int)(ca * P(12));
  int endY = fy - (int)(sa * P(12));

  Bres_ThickLine(fx, fy, endX, endY, P(5), C_BR);
  MidcircleFilled(endX + (int)(ca * P(2)), endY - (int)(sa * P(2)), P(3),
                  C_COP);
}

static void DrawSlideShape(int ox, int oy, int so) {
  int sx = ox + so, sY = oy - SH;
  FR(sx, sY, SL, SH, C_SL);
  DDA_ThickLine(sx, sY, sx + SL, sY, P(1), C_SLT);
  DDALine(sx, oy, sx + SL, oy, C_SLE);
  BresenhamLine(sx + SL, sY, sx + SL, oy, C_SLH);
  BresenhamLine(sx, sY, sx, oy, C_SLH);
  int epX = sx + P(26), epY = sY + P(3), epW = P(20), epH = P(14);
  BresenhamLine(epX, epY, epX + epW, epY, C_SLE);
  BresenhamLine(epX, epY + epH, epX + epW, epY + epH, C_SLE);
  int ntX = sx + P(88), ntY = oy - P(1);
  FR(ntX, ntY, P(8), P(3), (Color){22, 22, 20, 255});
  BresenhamLine(ntX, ntY, ntX, ntY + P(3), C_SLE);
  BresenhamLine(ntX + P(8), ntY, ntX + P(8), ntY + P(3), C_SLE);
  DDALine(ntX + 1, ntY, ntX + P(8) - 1, ntY, (Color){14, 14, 12, 255});
  /* rear serrations */
  for (int i = 0; i < 8; i++) {
    int gx = sx + P(4) + i * P(2);
    DDALine(gx, sY + P(3), gx, sY + SH - P(3), C_SLE);
    DDALine(gx + 1, sY + P(3), gx + 1, sY + SH - P(3),
            (Color){62, 62, 65, 180});
  }
  /* front serrations */
  for (int i = 0; i < 5; i++) {
    int gx = sx + SL - P(7) - i * P(3);
    DDALine(gx, sY + P(3), gx, sY + SH - P(3), C_SLE);
  }
  /* front sight */
  FR(sx + SL - P(14), sY - P(5), P(3), P(5), C_SDT);
  MidcircleFilled(sx + SL - P(13), sY - P(4), P(1), C_SDW);
  /* rear sight */
  FR(sx + P(18), sY - P(6), P(15), P(6), C_SDT);
  FR(sx + P(23), sY - P(6), P(5), P(3), (Color){12, 12, 12, 255});
  MidcircleFilled(sx + P(20), sY - P(3), P(1), C_SDW);
  MidcircleFilled(sx + P(31), sY - P(3), P(1), C_SDW);
  /* markings */
  Bres_DashedLine(sx + P(40), sY + P(9), sx + P(56), sY + P(9), P(2), P(1),
                  (Color){62, 62, 65, 160});
  /* chamber indicator */
  if (chm)
    MidcircleFilled(sx + P(52), sY + P(3), P(1), (Color){180, 50, 50, 200});
}

static void DrawFrameAndGrip(int ox, int oy) {
  /* frame body */
  FR(ox, oy, P(65), FH, C_FR);
  DDALine(ox, oy, ox + P(65), oy, C_FRL);
  /* dust cover */
  FR(ox + DC_LX, oy, DC_RX - DC_LX, FH, C_FR);
  DDALine(ox + DC_LX, oy + FH, ox + DC_RX, oy + FH, C_FRD);
  /* picatinny rail */
  for (int i = 0; i < 4; i++) {
    int rx = ox + P(78) + i * P(10);
    FR(rx, oy + FH, P(7), P(3), C_FRL);
  }
  /* beavertail */
  FR(ox - P(3), oy - P(2), P(20), P(4), C_FR);
  MidcircleFilled(ox + P(4), oy - P(2), P(6), C_FR);
  /* trigger guard */
  int tL = ox + TG_LX, tR = ox + TG_RX, tT = oy + TG_TY, tB = oy + TG_BY;
  Bres_ThickLine(tL, tT, tL, tB, P(2), C_FR);
  Bres_ThickLine(tL, tB, tR, tB, P(2), C_FR);
  Bres_ThickLine(tR, tB, tR, tT, P(2), C_FR);
  MidcircleThick(tL + P(4), tB - P(4), P(4), P(1), C_FR);
  FR(tL + P(2), tT + P(1), tR - tL - P(4), tB - tT - P(2),
     (Color){12, 12, 12, 255});
  /* grip */
  FillQuad(ox + G_TLX, oy + G_TY, ox + G_TRX, oy + G_TY, ox + G_BRX, oy + G_BY,
           ox + G_BLX, oy + G_BY, C_GR);
  /* finger grooves */
  for (int i = 0; i < 3; i++) {
    float t = 0.30f + (float)i * 0.18f;
    int gx = ox + G_TRX - (int)((G_TRX - G_BRX) * t) + P(2);
    int gy = oy + G_TY + (int)((G_BY - G_TY) * t);
    MidcircleFilled(gx, gy, P(4), C_GRH);
    Midcircle(gx, gy, P(4), C_GR);
  }
  /* stippling */
  for (int r = 0; r < 12; r++)
    for (int c = 0; c < 4; c++) {
      float tv = 0.12f + (float)r * 0.06f, th = 0.25f + (float)c * 0.16f;
      int dx = ox + G_TLX + (int)((G_TRX - G_TLX) * th) -
               (int)((G_TLX - G_BLX) * tv);
      int dy = oy + G_TY + (int)((G_BY - G_TY) * tv);
      MidcircleFilled(dx, dy, P(1), C_GRT);
    }
  /* backstrap ridges */
  for (int i = 0; i < 8; i++) {
    float t = 0.2f + (float)i * 0.08f;
    int lx = ox + G_TLX - (int)((G_TLX - G_BLX) * t) + P(1);
    int ly = oy + G_TY + (int)((G_BY - G_TY) * t);
    Bres_DashedLine(lx, ly, lx + P(5), ly, P(2), P(1), C_GRT);
  }
  /* grip base plate */
  int bpL = ox + G_BLX - P(1), bpR = ox + G_BRX + P(1);
  FR(bpL, oy + G_BY, bpR - bpL, P(5), C_FRL);
  BresenhamLine(bpL, oy + G_BY + P(5), bpR, oy + G_BY + P(5), C_FRD);
  
  MidcircleFilled(ox + P(52), oy + P(4), P(3), (Color){62, 62, 60, 255});
  Midcircle(ox + P(52), oy + P(4), P(3), C_FRD);
  MidcircleFilled(ox + P(38), oy + P(8), P(3), (Color){52, 52, 50, 255});
}

static void DrawMagShell(int ox, int oy, float mOff) {
  float ang = 20.0f * DEG2RAD;
  float sa = sinf(ang), ca = cosf(ang);

  int mxTop = ox + P(10);
  int myTop = oy + P(12) + (int)mOff;
  int mW = P(24);
  int mH = P(66);

  int mxBot = mxTop - (int)(sa * mH);
  int myBot = myTop + (int)(ca * mH);

  if (mOff > P(150))
    return;

  FillQuad(mxTop, myTop, mxTop + mW, myTop, mxBot + mW, myBot, mxBot, myBot,
           C_MAG);
  BresenhamLine(mxTop, myTop, mxBot, myBot, C_MAGE);
  BresenhamLine(mxTop + mW, myTop, mxBot + mW, myBot, C_MAGE);

  /* Floor plate */
  FillQuad(mxBot - P(1), myBot, mxBot + mW + P(2), myBot,
           mxBot + mW + P(2) - (int)(sa * P(4)), myBot + P(4),
           mxBot - P(1) - (int)(sa * P(4)), myBot + P(4),
           (Color){50, 50, 48, 200});

  for (int i = 0; i < 5; i++) {
    int hy = myBot - P(6) - i * P(9);
    int hx = mxBot + mW - P(1) + (int)(sa * (P(6) + i * P(9)));
    if (hy < myTop + P(3))
      break;
    FR(hx, hy, P(3), P(2), (Color){18, 18, 18, 180});
  }
}

static void DrawTrigger(int ox, int oy, float pull) {
  int px = ox + P(50), py = oy + P(10);
  float ang = (80 + pull * 22) * DEG2RAD;
  int mx = px + (int)(cosf(ang) * P(12)), my = py + (int)(sinf(ang) * P(12));
  float ta = ang - 5 * DEG2RAD;
  int tx = mx + (int)(cosf(ta) * P(8)), ty = my + (int)(sinf(ta) * P(8));
  Bres_ThickLine(px, py, mx, my, P(3), C_TRG);
  Bres_ThickLine(mx, my, tx, ty, P(3), C_TRG);
  float pa = ta + (float)M_PI * 0.5f;
  Bres_ThickLine(tx + (int)(cosf(pa) * P(3)), ty + (int)(sinf(pa) * P(3)),
                 tx - (int)(cosf(pa) * P(3)), ty - (int)(sinf(pa) * P(3)), P(2),
                 (Color){48, 48, 46, 255});
  MidcircleFilled(px, py, P(2), C_CTL);
}

/* ── Slide Lock Lever ─────────────────────────────────────────────── */
static float lockBounce = 0;

static void DrawSlideLock(int ox, int oy, float lt) {
  int pvX = ox + P(40), pvY = oy - P(1);
  int leverLen = P(16);
  int leverW = P(5);
  float baseAng = 5.0f * DEG2RAD;
  float engAng = -45.0f * DEG2RAD;
  float bounce = lockBounce * sinf(lockBounce * 10.0f) * 2.5f * DEG2RAD;
  float ang = baseAng + lt * engAng + bounce;
  float ca = cosf(ang), sa = sinf(ang);
  int exX = pvX + (int)(ca * leverLen);
  int exY = pvY + (int)(sa * leverLen);
  int tbLen = P(10);
  int tbX = pvX - (int)(ca * tbLen);
  int tbY = pvY - (int)(sa * tbLen);

  Bres_ThickLine(tbX, tbY, exX, exY, leverW, C_LCK);
  float pa = ang + (float)M_PI * 0.5f;
  int hdW = leverW / 2 + P(1);
  Bres_ThickLine(exX - (int)(cosf(pa) * hdW), exY - (int)(sinf(pa) * hdW),
                 exX + (int)(cosf(pa) * hdW), exY + (int)(sinf(pa) * hdW), P(3),
                 C_LCK);
  /* Highlight edge on top */
  DDALine(tbX + (int)(cosf(pa) * leverW / 2),
          tbY + (int)(sinf(pa) * leverW / 2),
          exX + (int)(cosf(pa) * leverW / 2),
          exY + (int)(sinf(pa) * leverW / 2), C_LCKH);
  /* Dark edge on bottom */
  DDALine(tbX - (int)(cosf(pa) * leverW / 2),
          tbY - (int)(sinf(pa) * leverW / 2),
          exX - (int)(cosf(pa) * leverW / 2),
          exY - (int)(sinf(pa) * leverW / 2), C_LCKD);
  /* Pivot pin — visible circle at the rotation center */
  MidcircleFilled(pvX, pvY, P(3), (Color){70, 70, 66, 255});
  Midcircle(pvX, pvY, P(3), C_LCKD);
  MidcircleFilled(pvX, pvY, P(1), C_LCKH);
  /* Serrations on thumb tab for grip texture */
  for (int i = 1; i <= 4; i++) {
    float t2 = (float)i / 5.0f;
    int ssx = pvX - (int)(ca * tbLen * t2);
    int ssy = pvY - (int)(sa * tbLen * t2);
    /* Perpendicular serration lines */
    int dx = (int)(sinf(ang) * leverW / 2), dy = -(int)(cosf(ang) * leverW / 2);
    DDALine(ssx - dx, ssy - dy, ssx + dx, ssy + dy, (Color){48, 48, 44, 255});
  }
  /* Engagement indicator — pulsing glow when locked */
  if (lt > 0.5f) {
    float gPulse = 0.6f + 0.4f * sinf((float)GetTime() * 6.0f);
    int ga = (int)(80.0f * (lt - 0.5f) * 2 * gPulse);
    MidcircleFilled(exX, exY, P(3),
                    (Color){220, 80, 40, (unsigned char)(ga > 255 ? 255 : ga)});
    MidcircleThick(
        exX, exY, P(5), P(1),
        (Color){220, 80, 40, (unsigned char)(ga / 3 > 255 ? 255 : ga / 3)});
  }
  /* Lever engagement label when fully locked */
  if (lt > 0.8f) {
    int la = (int)(200 * (lt - 0.8f) * 5);
    DrawText("LOCKED", pvX - P(12), pvY + P(8), P(4),
             (Color){200, 80, 50, (unsigned char)(la > 200 ? 200 : la)});
  }
}

/* ═════════════════════════════════════════════════════════════════════ */

void DrawGlock(void) {
  float dt = GetFrameTime();
  if (dt > 0.033f)
    dt = 0.033f;
  ClearBackground((Color){16, 16, 18, 255});
  DrawBackButton();
  DrawText("GLOCK 19 | 9x19mm | PISTOL", P(55), P(7), P(7),
           (Color){200, 180, 120, 255});
  DrawText("[SPASI] Tembak   [R] Reload   [F] Pengaman", P(55),
           P(18), P(5), (Color){140, 135, 120, 160});

  /* ── physics constants ── */
  float sldMax = P(48);
  float sldSpdBack = 800.0f;
  float sldSpdFwd = 500.0f;
  float trgSpd = 10.0f;
  float bulSpd = 2400.0f;
  float magSpd = 400.0f;
  float flash = 0;
  if (IsKeyPressed(KEY_F)) {
    safeMode = !safeMode;
    PlayAudioEffect(SOUND_LOCK);
  }

  if (IsKeyPressed(KEY_SPACE) && (safeMode || st == LOCKED)) {
    PlayAudioEffect(SOUND_TRIGGER);
  }

  switch (st) {
  case IDLE:
    if (sldX < 0) {
      sldX += sldSpdFwd * dt;
      if (sldX > 0) sldX = 0;
    }
    lockT += (0 - lockT) * 14 * dt;
    kX += (0 - kX) * 6 * dt;
    kY += (0 - kY) * 8 * dt;
    if (IsKeyPressed(KEY_SPACE) && chm && !safeMode && st != LOCKED) {
      st = PULLING;
      trgP = 0;
    }
    if (IsKeyPressed(KEY_R) && mag < 7)
      st = MAG_DROP;
    break;
  case PULLING:
    trgP += trgSpd * dt;
    if (trgP >= 1) {
      trgP = 1;
      st = FIRE;
    }
    kX += (0 - kX) * 6 * dt;
    kY += (0 - kY) * 8 * dt;
    break;
  case FIRE:
    st = RECOIL;
    bulX = 0;
    csX = 0;
    csY = 0;
    csR = 0;
    chm = 0;
    kX = -P(4);
    kY = -P(2);
    csVX = -(60 + (rand() % 40)) * S;
    csVY = -(180 + (rand() % 60)) * S;
    FlashP((float)(SCREEN_W / 2 + P(80)), (float)(SCREEN_H / 2 - P(12)));
    PlayAudioEffect(SOUND_GUNSHOT);
    break;
  case RECOIL: {
    sldX -= sldSpdBack * dt;
    bulX += bulSpd * dt;
    kX += (0 - kX) * 10 * dt;
    kY += (0 - kY) * 12 * dt;
    if (sldX < -sldMax * 0.8f) {
      csVY += 500 * dt;
      csX += csVX * dt;
      csY += csVY * dt;
      csR += 20 * dt;
    } else {
      csX = sldX;
    }
    if (sldX <= -sldMax) {
      sldX = -sldMax;
      st = EJECT;
      ejT = 0;
    }
    flash = (-sldX < sldMax * 0.4f) ? 1 + sldX / (sldMax * 0.4f) : 0;
    if (flash < 0)
      flash = 0;
    break;
  }
  case EJECT:
    bulX += bulSpd * dt;
    kX += (0 - kX) * 10 * dt;
    kY += (0 - kY) * 10 * dt;
    ejT += dt;
    if (ejT > 0.1f) {
      if (mag > 0) {
        st = FEEDING;
        fdT = 0;
        mag--;
      } else {
        st = LOCKED;
        lockT = 0;
        lockBounce = 1.0f;
        PlayAudioEffect(SOUND_LOCK);
      }
    }
    break;
  case FEEDING: {
    sldX += sldSpdFwd * dt;
    fdT = 1.0f - (-sldX / sldMax);
    if (fdT < 0) fdT = 0;
    if (fdT > 1) fdT = 1;
    kX += (0 - kX) * 10 * dt;
    kY += (0 - kY) * 10 * dt;
    trgP += (0 - trgP) * 10 * dt;
    if (sldX >= 0) {
      sldX = 0;
      chm = 1;
      st = IDLE;
    }
    break;
  }
  case LOCKED:
    trgP += (0 - trgP) * 5 * dt;
    lockT += (1 - lockT) * 10 * dt;
    if (lockT > 0.99f)
      lockT = 1;
    lockBounce *= (1 - 7 * dt);
    if (lockBounce < 0.01f)
      lockBounce = 0;
    kX += (0 - kX) * 6 * dt;
    kY += (0 - kY) * 8 * dt;
    if (IsKeyPressed(KEY_R))
      st = MAG_DROP;
    break;
  case MAG_DROP:
    magY += magSpd * dt * 2.5f;
    lockT += (0 - lockT) * 14 * dt;
    lockBounce = 0;
    kX += (0 - kX) * 6 * dt;
    kY += (0 - kY) * 8 * dt;
    if (magY > P(160)) {
      mag = 7;
      st = MAG_INS;
    }
    break;
  case MAG_INS:
    magY += (0 - magY) * 5 * dt;
    kX += (0 - kX) * 6 * dt;
    kY += (0 - kY) * 8 * dt;
    if (magY < 1) {
      magY = 0;
      if (sldX < -1) {
        st = FEEDING;
        fdT = 0;
        mag--;
      } else {
        st = IDLE;
      }
    }
    break;
  }

  /* Global casing physics while it falls */
  if (st != RECOIL && st != FIRE && st != PULLING && csY < P(500) && csY != 0) {
    csVY += 500 * dt;
    csX += csVX * dt;
    csY += csVY * dt;
    csR += 20 * dt;
  }

  /* particles */
  for (int i = 0; i < pn; i++) {
    pts[i].x += pts[i].vx * dt;
    pts[i].y += pts[i].vy * dt;
    if (pts[i].t == 1) {
      pts[i].vy -= 25 * dt;
      pts[i].vx *= (1 - 2 * dt);
    }
    pts[i].l -= dt;
    if (pts[i].l <= 0) {
      pts[i] = pts[--pn];
      i--;
    }
  }

  /* ── DRAWING ─────────────────────────────────────────────────── */
  int ox = SCREEN_W / 2 - P(45) + (int)kX;
  int oy = SCREEN_H / 2 - P(8) + (int)kY;
  int so = (int)sldX;
  FR(ox + P(5), oy + G_BY + P(10), P(60), P(4), (Color){8, 8, 8, 30});
  DrawInternals(ox, oy, so);
  DrawMagInt(ox, oy, magY, mag);
  if (chm && sldX > -P(3) && st != FEEDING)
    DrawChRound(ox, oy);
  if (st == FEEDING)
    DrawFeedRound(ox, oy, fdT);
  DrawMagShell(ox, oy, magY);
  DrawFrameAndGrip(ox, oy);
  DrawSlideLock(ox, oy, lockT);
  DrawTrigger(ox, oy, trgP);
  DrawSlideShape(ox, oy, so);
  if (flash > 0) {
    int fx = ox + SL - P(2), fy = oy - P(12);
    MidcircleFilled(fx, fy, (int)(P(5) * flash),
                    (Color){255, 255, 225, (unsigned char)(255 * flash)});
    MidcircleThick(fx, fy, (int)(P(9) * flash), P(2),
                   (Color){255, 175, 45, (unsigned char)(180 * flash)});
    Midcircle(fx, fy, (int)(P(14) * flash),
              (Color){255, 195, 75, (unsigned char)(70 * flash)});
  }
  if ((st == RECOIL || st == EJECT) && bulX < SCREEN_W) {
    int bx2 = ox + SL + (int)bulX, by2 = oy - P(12);
    MidcircleFilled(bx2, by2, P(3), C_COP);
    DDA_DashedLine(bx2 - P(18), by2, bx2 - P(3), by2, P(3), P(2),
                   (Color){200, 180, 100, 50});
  }
  if ((st == RECOIL || csY != 0) && csY < P(300)) {
    int ex2 = ox + P(34) + (int)csX, ey2 = oy - P(12) + (int)csY;
    float cr = cosf(csR), sr = sinf(csR);
    int l = P(12);
    Bres_ThickLine(ex2 - (int)(cr * l / 2), ey2 - (int)(sr * l / 2),
                   ex2 + (int)(cr * l / 2), ey2 + (int)(sr * l / 2), P(5),
                   C_BR);
    MidcircleFilled(ex2 - (int)(cr * l / 2), ey2 - (int)(sr * l / 2), P(2),
                    C_PRM);
  }
  for (int i = 0; i < pn; i++) {
    float lr = pts[i].l / pts[i].ml;
    int a = (int)(lr * pts[i].c.a);
    if (a < 0)
      a = 0;
    Color pc = pts[i].c;
    pc.a = (unsigned char)a;
    if (pts[i].t == 0)
      MidcircleFilled((int)pts[i].x, (int)pts[i].y, P(2) + (int)(P(2) * lr),
                      pc);
    else {
      int r = P(2) + (int)(P(4) * (1 - lr));
      Midcircle((int)pts[i].x, (int)pts[i].y, r, pc);
    }
  }

  /* ── HUD ─────────────────────────────────────────────────────── */
  int hx = P(8), hy = SCREEN_H - P(55);
  FR(hx, hy, P(108), P(48), (Color){12, 12, 12, 200});
  BresenhamLine(hx, hy, hx + P(108), hy, (Color){80, 70, 40, 180});
  const char *ss = "READY";
  Color sc = C_SDW;
  if (st == PULLING) {
    ss = "FIRING...";
    sc = ORANGE;
  }
  if (st == RECOIL) {
    ss = "RECOIL";
    sc = (Color){255, 180, 50, 255};
  }
  if (st == EJECT) {
    ss = "EJECTING";
    sc = YELLOW;
  }
  if (st == FEEDING) {
    ss = "CHAMBERING";
    sc = SKYBLUE;
  }
  if (st == LOCKED) {
    ss = "SLIDE LOCKED [R]";
    sc = RED;
  }
  if (st == MAG_DROP) {
    ss = "MAG DROP";
    sc = ORANGE;
  }
  if (st == MAG_INS) {
    ss = "MAG INSERT";
    sc = ORANGE;
  }
  DrawText("STATUS", hx + P(3), hy + P(2), P(4), (Color){120, 115, 95, 180});
  DrawText(ss, hx + P(3), hy + P(9), P(5), sc);
  DrawText(safeMode ? "[SAFE]" : "[FIRE]", hx + P(75), hy + P(9), P(4),
           safeMode ? (Color){50, 200, 50, 255} : (Color){200, 50, 50, 255});
  int tot = mag + (chm ? 1 : 0) + ((st == FEEDING) ? 1 : 0);
  DrawText("AMMO", hx + P(3), hy + P(19), P(4), (Color){120, 115, 95, 180});
  DrawText(TextFormat("%d / 8", tot), hx + P(3), hy + P(26), P(7),
           tot > 0 ? (Color){80, 220, 80, 255} : RED);
  for (int i = 0; i < 8; i++) {
    Color bc = (i < tot) ? C_BR : (Color){35, 35, 33, 255};
    FR(hx + P(48) + i * P(7), hy + P(27), P(5), P(10), bc);
    if (i < tot)
      MidcircleFilled(hx + P(50) + i * P(7), hy + P(25), P(2), C_COP);
  }
  DrawText("CHAMBER", hx + P(3), hy + P(38), P(4), (Color){120, 115, 95, 180});
  MidcircleFilled(hx + P(38), hy + P(41), P(3),
                  chm ? (Color){50, 200, 50, 255} : RED);
  DrawText(chm ? "LOADED" : "EMPTY", hx + P(45), hy + P(38), P(4),
           chm ? (Color){80, 200, 80, 180} : (Color){200, 80, 80, 180});
}