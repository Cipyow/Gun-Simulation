#include "src/screens/kar98k.h"
#include "src/algo/dda.h"
#include "src/algo/bresenham.h"
#include "src/algo/midcircle.h"
#include "src/ui/back_button.h"
#include "src/ui/audio_manager.h"
#include "screen_type.h"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define S 2.4f
#define P(v) ((int)((v)*S))

/* ── Colors ───────────────────────────────────────────────────────────── */
#define C_WOOD  (Color){115, 65, 35, 255}
#define C_WOODD (Color){85, 45, 25, 255}
#define C_WOODH (Color){135, 80, 45, 255}
#define C_STL   (Color){38, 38, 40, 255}   /* dark steel */
#define C_STLH  (Color){68, 68, 72, 255}   /* steel highlight */
#define C_BOLT  (Color){120, 120, 122, 255} /* polished steel bolt */
#define C_STK   (Color){108, 108, 106, 255}
#define C_BR    (Color){212, 172, 52, 255} /* brass */
#define C_BRD   (Color){182, 142, 42, 255}
#define C_COP   (Color){192, 122, 56, 255} /* copper tip */
#define C_CLIP  (Color){80, 85, 90, 255}   /* stripper clip */
#define C_SDW   (Color){225, 225, 225, 255}

/* ── State ────────────────────────────────────────────────────────────── */
typedef enum { K_IDLE, K_PULLING, K_FIRE, K_RECOIL, K_BOLT_UP, K_BOLT_BACK, K_BOLT_FWD, K_BOLT_DOWN, K_CLIP_IN, K_PUSH_AMMO, K_CLIP_OUT } KSt;
static KSt   st = K_IDLE;
static float boltX = 0, boltR = 0; /* bolt translation and rotation */
static float trgP = 0, bulX = 0;
static float csX = 0, csY = 0, csR = 0, csVX = 0, csVY = 0;
static float kX = 0, kY = 0; /* recoil kick */
static int   mag = 5, chm = 1, fired = 0;
static int   isReloading = 0;
static float fdT = 0, ejectDelay = 0;
static float clipY = -P(150), ammoY = 0; /* reload anims */

/* particles */
#define MP 25
typedef struct{float x,y,vx,vy,l,ml;int t;Color c;}Pt;
static Pt pts[MP]; static int pn=0;

static void AddP(float x,float y,float vx,float vy,float l,int t,Color c){
    if(pn>=MP) return;
    pts[pn++]=(Pt){x,y,vx,vy,l,l,t,c};
}
static void FlashP(float x,float y){
    for(int i=0;i<6;i++){
        float a=-0.2f+(float)(rand()%40)/100.0f;
        float sp=300+(float)(rand()%250);
        AddP(x,y,cosf(a)*sp,sinf(a)*sp-30,0.06f+(float)(rand()%25)/1000.0f,0,(Color){255,(unsigned char)(210+rand()%45),(unsigned char)(60+rand()%60),255});
    }
    for(int i=0;i<3;i++) AddP(x,y,30+(float)(rand()%40),-(float)(rand()%25)-10, 0.4f+(float)(rand()%30)/100.0f,1,(Color){140,140,140,90});
}

static void FR(int x,int y,int w,int h,Color c){for(int r=0;r<h;r++)DDALine(x,y+r,x+w,y+r,c);}
static float smoothstep(float t){if(t<0)t=0;if(t>1)t=1;return t*t*(3-2*t);}

/* ═══════════════════════════════════════════════════════════════════════ */

static void DrawKarWood(int ox, int oy) {
    /* Main body */
    // Buttstock to Grip
    Bres_ThickLine(ox-P(150), oy-P(2), ox-P(60), oy+P(5), P(28), C_WOOD);
    // Smooth transition to grip
    MidcircleFilled(ox-P(60), oy+P(5), P(14), C_WOOD);
    Bres_ThickLine(ox-P(60), oy+P(5), ox-P(35), oy+P(18), P(22), C_WOOD);
    MidcircleFilled(ox-P(35), oy+P(18), P(11), C_WOOD);
    Bres_ThickLine(ox-P(35), oy+P(18), ox-P(15), oy+P(10), P(16), C_WOOD);
    MidcircleFilled(ox-P(15), oy+P(10), P(8), C_WOOD);
    
    // Belly area (Internal Magazine)
    Bres_ThickLine(ox-P(15), oy+P(10), ox+P(45), oy+P(10), P(18), C_WOOD);
    
    // Fore-end stock
    Bres_ThickLine(ox+P(45), oy+P(10), ox+P(160), oy+P(5), P(14), C_WOOD);
    
    // Top Handguard (wood over the barrel)
    Bres_ThickLine(ox+P(65), oy-P(5), ox+P(145), oy-P(5), P(8), C_WOODD);

    // Stock Bolt disk (the distinct metal ring in the buttstock)
    MidcircleFilled(ox-P(110), oy+P(3), P(4), C_STL);
    Midcircle(ox-P(110), oy+P(3), P(4), C_STLH);
    MidcircleFilled(ox-P(110), oy+P(3), P(1), (Color){20,20,20,255});
    
    // Sling slot (horizontal hole in buttstock)
    Bres_ThickLine(ox-P(130), oy+P(6), ox-P(120), oy+P(6), P(3), (Color){20,15,10,200});

    // Bolt handle cutout (curved wood removal)
    MidcircleFilled(ox+P(2), oy+P(2), P(6), C_WOODD);

    // Highlights for realism
    DDALine(ox-P(150), oy-P(14), ox-P(60), oy-P(8), C_WOODH); // top edge
    DDALine(ox+P(45), oy-P(1), ox+P(160), oy-P(1), C_WOODH); // fore-end top edge
    DDALine(ox-P(150), oy+P(10), ox-P(60), oy+P(17), (Color){50,25,10,150}); // buttstock shadow
    
    // Metal Buttplate
    Bres_ThickLine(ox-P(152), oy-P(13), ox-P(152), oy+P(12), P(4), C_STL);
    Bres_ThickLine(ox-P(152), oy-P(13), ox-P(148), oy-P(15), P(3), C_STL);

    // Barrel Bands (metal rings holding stock and barrel)
    // H-Band (Rear)
    Bres_ThickLine(ox+P(100), oy-P(8), ox+P(100), oy+P(11), P(6), C_STL);
    // Front Band & Bayonet Lug
    Bres_ThickLine(ox+P(145), oy-P(8), ox+P(145), oy+P(9), P(6), C_STL);
    Bres_ThickLine(ox+P(145), oy+P(9), ox+P(155), oy+P(9), P(3), C_STL); // bayonet lug extension
    // Cleaning rod under barrel
    Bres_ThickLine(ox+P(155), oy+P(6), ox+P(190), oy+P(6), P(2), C_STL);
}

static void DrawKarBarrelAndReceiver(int ox, int oy) {
    /* Barrel */
    // Stepped barrel (gets thinner)
    Bres_ThickLine(ox+P(40), oy-P(3), ox+P(150), oy-P(3), P(8), C_STL);
    Bres_ThickLine(ox+P(150), oy-P(3), ox+P(205), oy-P(3), P(5), C_STL);
    
    // Front sight (Ramp and post)
    Bres_ThickLine(ox+P(195), oy-P(6), ox+P(200), oy-P(8), P(5), C_STL);
    Bres_ThickLine(ox+P(200), oy-P(8), ox+P(200), oy-P(11), P(2), C_STL); // post
    
    /* Rear sight block (Ramp shape) */
    Bres_ThickLine(ox+P(40), oy-P(6), ox+P(65), oy-P(6), P(8), C_STL);
    Bres_ThickLine(ox+P(40), oy-P(10), ox+P(60), oy-P(6), P(4), C_STLH); // ramp angle
    
    /* Receiver Body - Made semi-transparent so internal chamber & bolt are visible */
    Color r_stl = C_STL; r_stl.a = 120;
    Color r_stlh = C_STLH; r_stlh.a = 120;

    FR(ox-P(20), oy-P(7), P(60), P(11), r_stl);
    
    // Smooth internal cavity (Ejection port & Mag Well opening)
    FR(ox+P(5), oy-P(5), P(25), P(8), (Color){20,20,18,120}); // receiver cavity
    
    // Chamber cut (where bullet seats)
    Bres_ThickLine(ox+P(30), oy-P(2), ox+P(40), oy-P(2), P(6), (Color){15,15,15,120});
    // Feed ramp (sloped incline into chamber)
    Bres_ThickLine(ox+P(25), oy+P(2), ox+P(32), oy-P(2), P(3), r_stlh);
    
    // Upper receiver lip above the open action
    Bres_ThickLine(ox+P(4), oy-P(7), ox+P(35), oy-P(7), P(2), r_stl);
    
    // Metal block behind bolt
    FR(ox-P(25), oy-P(5), P(5), P(10), r_stl); 
}

static void DrawKarTrigger(int ox, int oy, float pull) {
    /* Trigger guard */
    int tL = ox-P(20), tR = ox+P(10), tY = oy+P(15), tB = oy+P(32);
    Bres_ThickLine(tL, tY, tL, tB-P(4), P(3), C_STL);
    Bres_ThickLine(tR, tY, tR, tB-P(4), P(3), C_STL);
    Bres_ThickLine(tL+P(4), tB, tR-P(4), tB, P(3), C_STL);
    BresenhamLine(tL, tB-P(4), tL+P(4), tB, C_STL);
    BresenhamLine(tR, tB-P(4), tR-P(4), tB, C_STL);
    BresenhamLine(tL+P(1), tB-P(4), tL+P(4), tB-P(1), C_STL);
    BresenhamLine(tR-P(1), tB-P(4), tR-P(4), tB-P(1), C_STL);
    
    /* Trigger */
    float tang = (15 + pull*25) * DEG2RAD;
    int px = ox-P(8), py = oy+P(18);
    int tx = px - (int)(sinf(tang)*P(15));
    int ty = py + (int)(cosf(tang)*P(15));
    // Trigger thickness smoothing
    MidcircleFilled(tx, ty, P(2), C_STLH);
    Bres_ThickLine(px, py, tx, ty, P(4), C_STLH);
}

static void DrawRound(int cx, int cy, float a) {
    float ca = cosf(a), sa = sinf(a);
    int px = cx + (int)(ca*P(14)), py = cy + (int)(sa*P(14)); // tip
    int bp = cx - (int)(ca*P(10)), by = cy - (int)(sa*P(10)); // base
    
    Bres_ThickLine(bp, by, cx, cy, P(5), C_BRD);  // casing body
    Bres_ThickLine(cx, cy, px-(int)(ca*P(2)), py-(int)(sa*P(2)), P(3), C_COP); // bullet tip
    // Primer groove
    BresenhamLine(bp+(int)(sa*P(3)), by-(int)(ca*P(3)), bp-(int)(sa*P(3)), by+(int)(ca*P(3)), C_BR);
}

static void DrawKarInternalMag(int ox, int oy, int rds, float aY) {
    /* Mag box sits inside the belly at oy+10 to oy+19 */
    int mX = ox+P(15), mY = oy-P(1); // Top of mag well
    int bY = oy+P(16); // Bottom of the internal cavity
    
    // Follower plate
    int fY = mY + P(2) + rds*P(4) + (int)aY;
    if(fY > bY - P(2)) fY = bY - P(2);
    Bres_ThickLine(mX-P(10), fY, mX+P(12), fY, P(2), C_STLH);
    
    // ZigZag spring (compresses between follower plate fY and bottom bY)
    int sH = (bY - fY) / 4;
    if (sH > 0) {
        BresenhamLine(mX-P(6), fY, mX+P(6), fY+sH, C_STK);
        BresenhamLine(mX+P(6), fY+sH, mX-P(6), fY+sH*2, C_STK);
        BresenhamLine(mX-P(6), fY+sH*2, mX+P(6), fY+sH*3, C_STK);
        BresenhamLine(mX+P(6), fY+sH*3, mX, bY, C_STK);
    }

    /* Draw rounds in mag */
    for(int i=0; i<rds; i++) {
        int rY = mY + P(2) + (rds - 1 - i)*P(4) + (int)aY;
        if(rY > bY) continue; // Don't draw if pushed below bottom
        
        int rx = mX;
        if (rY > mY + P(2)) {
            rx += ((i%2==0)?-P(2):P(2)); // start zig-zagging once inside
        }
        DrawRound(rx, rY, 0); 
    }
}

static void DrawKarBolt(int ox, int oy, float bx, float bRot, int cSt, float cFd) {
    int bX = ox-P(10) + (int)bx;
    int bY = oy-P(6);
    
    /* Bolt body cylinder */
    FR(bX, bY, P(40), P(6), C_BOLT);
    DDALine(bX, bY+P(1), bX+P(40), bY+P(1), (Color){220,220,225,180}); // shine
    
    /* Bolt Head / Extractor claw */
    Bres_ThickLine(bX+P(40), bY+P(3), bX+P(42), bY+P(3), P(4), C_STLH);
    
    /* Bolt handle (rotates down 90 deg -> 0 deg) 
       bRot: 0 is locked down, 1 is unlocked up */
    int hX = bX + P(8);
    int hY = bY + P(3); 
    float hAng = (90 - bRot*90) * DEG2RAD; 
    
    // Perspective simulated bolt handle drop
    int hLen = P(18); // longer handle
    int endX = hX + (int)(cosf(hAng)*P(6)); // tilts forward slightly
    int endY = hY + (int)(sinf(hAng)*hLen);
    
    // Bolt handle stem and ball
    Bres_ThickLine(hX, hY, endX, endY, P(5), C_STLH);
    MidcircleFilled(endX, endY, P(6), C_STL); // larger metallic ball knob
    MidcircleFilled(endX-P(2), endY-P(2), P(2), (Color){150,150,155,200}); // highlight on ball
    
    /* Safety catch assembly at rear of bolt */
    FR(bX-P(8), bY-P(2), P(8), P(10), C_STL);
    
    /* Chambered round or Feeding round attached to bolt face */
    if (cSt == 1 && bx > -P(5)) {
        // Round in chamber
        DrawRound(ox+P(44), oy-P(3), 0);
    } else if (cSt == 2) {
        // Fired casing in chamber, or being extracted by bolt
        int exX = ox+P(34) + (bx < 0 ? (int)bx : 0);
        Bres_ThickLine(exX-P(10), oy-P(3), exX, oy-P(3), P(5), C_BRD);
        BresenhamLine(exX-P(10), oy-P(6), exX-P(10), oy, C_BR);
    } else if (cSt == 3) {
        // Feeding round moving with bolt
        float pX = ox+P(12) + cFd*P(22);
        float pY = oy+P(4) - cFd*P(7);
        float ang = sinf(cFd * M_PI) * (-5.0f * DEG2RAD);
        DrawRound((int)pX, (int)pY, ang);
    }
}

static void DrawReloadClip(int ox, int oy) {
    if (clipY < -P(140)) return;
    int cx = ox+P(15);
    int cy = oy-P(30) + (int)clipY;
    
    /* Draw the brass stripper clip */
    Bres_ThickLine(cx+P(11), cy-P(2), cx+P(11), cy-P(2)+P(30), P(3), C_CLIP);
    
    /* Thumb pushing down */
    if (st == K_PUSH_AMMO) {
        int thY = cy - P(4) + (int)ammoY;
        MidcircleFilled(cx-P(2), thY, P(8), (Color){200, 150, 120, 255}); // simple thumb
    }
}

/* ═════════════════════════════════════════════════════════════════════ */

void DrawKar98k(void) {
    float dt = GetFrameTime();
    if(dt > 0.033f) dt = 0.033f;
    ClearBackground((Color){16, 16, 18, 255});
    DrawBackButton();
    DrawText("KAR-98K | 7.92x57mm Mauser | BOLT-ACTION", P(55), P(7), P(7), (Color){200, 180, 120, 255});
    DrawText("[SPASI] Tembak   [B] Siklus Bolt   [R] Reload Clip", P(55), P(18), P(5), (Color){140, 135, 120, 160});

    float bSpd = 5.0f;
    float pullSpd = 10.0f;
    
    /* State Machine */
    switch(st) {
        case K_IDLE:
            kX += (0-kX)*8*dt; kY += (0-kY)*8*dt;
            if (IsKeyPressed(KEY_SPACE) && chm && !fired) {
                st = K_PULLING; trgP = 0;
            } else if (IsKeyPressed(KEY_SPACE) && (!chm || fired)) {
                // Dry fire click - chamber empty or fired casing still in chamber
                PlayAudioEffect(SOUND_TRIGGER);
            } else if (IsKeyPressed(KEY_R) && mag < 5) {
                isReloading = 1; st = K_BOLT_UP;
            } else if (IsKeyPressed(KEY_B) && fired) {
                st = K_BOLT_UP;
            }
            break;
        case K_PULLING:
            trgP += pullSpd*dt;
            if(trgP >= 1){ trgP=1; st=K_FIRE; }
            kX += (0-kX)*8*dt; kY += (0-kY)*8*dt;
            break;
        case K_FIRE:
            st = K_RECOIL; bulX = 0; chm = 0; fired = 1;
            kX = -P(8); kY = -P(4);
            FlashP((float)(SCREEN_W/2 + P(190)), (float)(SCREEN_H/2 - P(4)));
            PlayAudioEffect(SOUND_GUNSHOT);
            break;
        case K_RECOIL:
            bulX += 2500.0f * dt;
            kX += (0-kX)*12*dt; kY += (0-kY)*12*dt;
            if (bulX > SCREEN_W) {
                st = K_IDLE; trgP = 0;
            }
            break;
        case K_BOLT_UP:
            PlayAudioEffect(SOUND_BOLT_UP);
            boltR += bSpd*dt*2; // fast unlock
            kX += (-P(1)-kX)*10*dt; // slight nudge
            if (boltR >= 1.0f) { boltR = 1.0f; st = K_BOLT_BACK; }
            break;
        case K_BOLT_BACK:
            PlayAudioEffect(SOUND_BOLT_BACK);
            boltX -= P(40)*dt*bSpd;
            if (boltX <= -P(40)) {
                boltX = -P(40);
                if (fired || chm) {
                    // Eject brass
                    fired = 0; chm = 0; 
                    csX = 0; csY = 0; csVX = -100*S; csVY = -250*S; csR = 0;
                }
                ejectDelay = 0;
                fdT = 0; // Reset feed timer
                if (isReloading) {
                    st = K_CLIP_IN; clipY = -P(150); ammoY = -P(30); mag = 5;
                    isReloading = 0;
                } else {
                    st = K_BOLT_FWD; // auto return for smooth anim
                }
            }
            break;
        case K_BOLT_FWD:
            ejectDelay += dt; // wait briefly for ejection to clear
            if(ejectDelay > 0.1f) {
                if (mag > 0 && !chm) {
                    fdT += dt * bSpd;
                    boltX = -P(40) + smoothstep(fdT)*P(40);
                    if (boltX >= 0) { boltX = 0; mag--; chm=1; st = K_BOLT_DOWN; }
                } else {
                    boltX += P(40)*dt*bSpd;
                    if (boltX >= 0) { boltX = 0; st = K_BOLT_DOWN; }
                }
            }
            break;
        case K_BOLT_DOWN:
            PlayAudioEffect(SOUND_BOLT_DOWN);
            boltR -= bSpd*dt*2;
            if (boltR <= 0) { boltR = 0; st = K_IDLE; }
            break;
        case K_CLIP_IN:
            PlayAudioEffect(SOUND_CLIP_IN);
            clipY += P(300)*dt;
            if (clipY >= 0) { clipY = 0; st = K_PUSH_AMMO; }
            break;
        case K_PUSH_AMMO:
            ammoY += P(100)*dt;
            if (ammoY >= 0) { ammoY = 0; st = K_CLIP_OUT; }
            break;
        case K_CLIP_OUT:
            clipY -= P(400)*dt;
            if (clipY < -P(150)) { ejectDelay = 0; st = K_BOLT_FWD; }
            break;
    }

    /* physics for ejected casing */
    if (csY < P(500) && (csVX!=0 || csVY!=0)) {
        csVY += 600*dt; csX += csVX*dt; csY += csVY*dt; csR += 15*dt;
    }
    
    /* particles */
    for(int i=0;i<pn;i++){
        pts[i].x+=pts[i].vx*dt; pts[i].y+=pts[i].vy*dt;
        if(pts[i].t==1){pts[i].vy-=30*dt;pts[i].vx*=(1-2*dt);}
        pts[i].l-=dt;
        if(pts[i].l<=0){pts[i]=pts[--pn];i--;}
    }

    /* ── DRAWING ─────────────────────────────────────────────────── */
    int ox = SCREEN_W/2 - P(40) + (int)kX;
    int oy = SCREEN_H/2 + (int)kY;
    
    DrawKarWood(ox, oy);
    DrawKarInternalMag(ox, oy, mag, (st==K_PUSH_AMMO || st==K_CLIP_IN)? ammoY : 0.0f);
    DrawKarTrigger(ox, oy, trgP);
    
    int cStatus = 0;
    if (chm && !fired) cStatus = 1; // loaded round
    else if (fired) cStatus = 2; // fired casing
    else if (st == K_BOLT_FWD && mag > 0) cStatus = 3; // feeding round

    DrawKarBolt(ox, oy, boltX, boltR, cStatus, fdT);
    DrawKarBarrelAndReceiver(ox, oy);
    
    DrawReloadClip(ox, oy);

    /* Vfx Muzzle flash */
    if (st == K_RECOIL && bulX < P(20)) {
        int fx = ox+P(200), fy = oy-P(2);
        MidcircleFilled(fx, fy, P(8), (Color){255,200,50,200});
        MidcircleFilled(fx, fy, P(4), (Color){255,255,200,255});
    }
    /* Vfx Bullet flying */
    if (st == K_RECOIL && bulX < SCREEN_W) {
        int bx2 = ox+P(200)+(int)bulX, by2 = oy-P(2);
        Bres_ThickLine(bx2-P(10), by2, bx2+P(2), by2, P(3), C_COP);
        DDA_DashedLine(bx2-P(30), by2, bx2-P(10), by2, P(4), P(2), (Color){200,180,100,50});
    }
    /* Vfx Ejected Casing */
    if ((csVX != 0 || csVY != 0) && csY < P(300)) {
        int ex2 = ox+P(24) + (int)csX, ey2 = oy-P(6) + (int)csY;
        float cr = cosf(csR), sr = sinf(csR);
        int l = P(10);
        Bres_ThickLine(ex2-(int)(cr*l/2), ey2-(int)(sr*l/2),
                       ex2+(int)(cr*l/2), ey2+(int)(sr*l/2), P(4), C_BRD);
    }
    
    for(int i=0;i<pn;i++){
        float lr=pts[i].l/pts[i].ml; int a=(int)(lr*pts[i].c.a); if(a<0)a=0;
        Color pc=pts[i].c; pc.a=(unsigned char)a;
        if(pts[i].t==0) MidcircleFilled((int)pts[i].x,(int)pts[i].y,P(2)+(int)(P(2)*lr),pc);
        else Midcircle((int)pts[i].x,(int)pts[i].y,P(3)+(int)(P(3)*(1-lr)),pc);
    }

    /* ── HUD ─────────────────────────────────────────────────────── */
    int hx=P(8), hy=SCREEN_H-P(55);
    FR(hx,hy,P(108),P(48),(Color){12,12,12,200});
    BresenhamLine(hx,hy,hx+P(108),hy,(Color){80,70,40,180});
    
    const char* ss = "READY"; Color sc = C_SDW;
    if(st == K_PULLING) {ss="FIRING"; sc=ORANGE;}
    if(st == K_FIRE || st == K_RECOIL) {ss="RECOIL"; sc=YELLOW;}
    if(st == K_BOLT_UP || st == K_BOLT_DOWN) {ss="UNLOCK/LOCK BOLT"; sc=SKYBLUE;}
    if(st == K_BOLT_BACK) {ss="EXTRACTING"; sc=ORANGE;}
    if(st == K_BOLT_FWD) {ss="FEEDING"; sc=SKYBLUE;}
    if(st == K_IDLE && fired) {ss="BOLT CYCLE REQUIRED [B]"; sc=RED;}
    if(st == K_CLIP_IN || st == K_PUSH_AMMO || st == K_CLIP_OUT) {ss="STRIPPER CLIP RELOAD"; sc=GREEN;}
    
    DrawText("STATUS",hx+P(3),hy+P(2),P(4),(Color){120,115,95,180});
    DrawText(ss,hx+P(3),hy+P(9),P(5),sc);
    
    DrawText("AMMO",hx+P(3),hy+P(19),P(4),(Color){120,115,95,180});
    int tot = mag;
    DrawText(TextFormat("%d / 5",tot),hx+P(3),hy+P(26),P(7),tot>0?(Color){80,220,80,255}:RED);
    for(int i=0;i<5;i++){
        Color bc=(i<tot)?C_BR:(Color){35,35,33,255};
        FR(hx+P(38)+i*P(5),hy+P(27),P(3),P(8),bc);
        if(i<tot) MidcircleFilled(hx+P(39)+i*P(5),hy+P(26),P(1),C_COP);
    }
    
    DrawText("CHAMBER",hx+P(3),hy+P(38),P(4),(Color){120,115,95,180});
    MidcircleFilled(hx+P(38),hy+P(41),P(3),chm?(fired?ORANGE:(Color){50,200,50,255}):RED);
    DrawText(chm?(fired?"SPENT CASING":"LIVE ROUND"):"EMPTY",hx+P(45),hy+P(38),P(4),
             chm?(fired?ORANGE:(Color){80,200,80,180}):(Color){200,80,80,180});
}
