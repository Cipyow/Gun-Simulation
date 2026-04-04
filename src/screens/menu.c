#include "src/screens/menu.h"
#include "screen_type.h"
#include "raylib.h"
#include <math.h>
#include <string.h>

static int clickedProgram = 0;
static int showStartChoices = 0;

#define MENU_CMD_ABOUT 100
#define MENU_CMD_EXIT  -1

int GetClickedProgram(void) {
    int result = clickedProgram;
    clickedProgram = 0;
    return result;
}

// ═══════════════════════════════════════════════════════════════════════════
// WEAPON DATA
// ═══════════════════════════════════════════════════════════════════════════
typedef struct {
    const char *name;
    const char *type;
    const char *caliber;
    const char *desc;
    int price;
    int key;           // 1-10
    const char *keyStr;
    Color typeColor;
} WeaponInfo;

static const WeaponInfo weapons[2] = {
    {"GLOCK 19", "PISTOL", "9mm", "Animasi mekanik tembakan", 750, 1, "1", {220,60,60,255}},
    {"KAR-98K",   "RIFLE",  "7.92mm", "Bolt-Action mekanik manual", 850, 2, "2", {180,110,60,255}},
};

// ═══════════════════════════════════════════════════════════════════════════
// DRAWING HELPERS — gun silhouettes (pure raylib)
// ═══════════════════════════════════════════════════════════════════════════

static void DrawPistolSil(int x, int y, float s, Color c) {
    DrawRectangle(x, y, (int)(42*s), (int)(8*s), c);                         // barrel
    DrawRectangle(x-(int)(6*s), y-(int)(4*s), (int)(48*s), (int)(4*s), c);    // slide top
    DrawRectangle(x-(int)(6*s), y+(int)(8*s), (int)(26*s), (int)(12*s), c);   // frame
    DrawRectangle(x-(int)(6*s), y+(int)(20*s), (int)(14*s), (int)(18*s), c);  // grip
    DrawRectangle(x-(int)(8*s), y+(int)(34*s), (int)(18*s), (int)(3*s), c);   // grip base
    DrawRectangleLines(x+(int)(8*s), y+(int)(14*s), (int)(12*s), (int)(14*s), c); // trigger guard
    DrawRectangle(x+(int)(42*s), y+(int)(1), (int)(3*s), (int)(8*s)-2, Fade(c,0.4f)); // muzzle
}

static void DrawRifleSil(int x, int y, float s, Color c) {
    DrawRectangle(x, y, (int)(80*s), (int)(6*s), c);                          // barrel
    DrawRectangle(x-(int)(5*s), y-(int)(3*s), (int)(85*s), (int)(3*s), c);     // rail top
    DrawRectangle(x+(int)(12*s), y+(int)(6*s), (int)(22*s), (int)(8*s), c);    // body
    DrawRectangle(x+(int)(20*s), y+(int)(14*s), (int)(6*s), (int)(14*s), c);   // magazine
    DrawRectangle(x-(int)(30*s), y-(int)(2*s), (int)(30*s), (int)(10*s), c);   // stock
    DrawRectangle(x-(int)(34*s), y+(int)(4*s), (int)(10*s), (int)(10*s), c);   // stock end
    DrawRectangle(x+(int)(30*s), y-(int)(7*s), (int)(18*s), (int)(5*s), Fade(c,0.7f)); // scope
    DrawCircle(x+(int)(39*s), y-(int)(5*s), 3*s, Fade(c,0.5f));               // scope lens
}

static void DrawKar98kSil(int x, int y, float s, Color c) {
    DrawRectangle(x, y, (int)(92*s), (int)(4*s), c);                          // barrel
    DrawRectangle(x+(int)(14*s), y-(int)(3*s), (int)(40*s), (int)(5*s), c);    // receiver
    DrawRectangle(x+(int)(10*s), y-(int)(5*s), (int)(18*s), (int)(6*s), c);    // bolt housing
    DrawRectangle(x-(int)(22*s), y-(int)(2*s), (int)(24*s), (int)(8*s), c);    // stock body
    DrawRectangle(x-(int)(40*s), y+(int)(2*s), (int)(18*s), (int)(8*s), c);    // butt plate
    DrawRectangle(x+(int)(46*s), y+(int)(2*s), (int)(32*s), (int)(4*s), Fade(c, 0.8f)); // fore-end
    DrawRectangle(x+(int)(56*s), y-(int)(1*s), (int)(8*s), (int)(6*s), Fade(c, 0.6f)); // cleaning rod detail
    DrawLine(x+(int)(16*s), y-(int)(1*s), x+(int)(32*s), y-(int)(1*s), Fade(c, 0.6f)); // bolt line
}

static void DrawShotgunSil(int x, int y, float s, Color c) {
    DrawRectangle(x, y, (int)(85*s), (int)(5*s), c);                         // barrel
    DrawRectangle(x, y+(int)(6*s), (int)(85*s), (int)(4*s), c);              // under barrel
    DrawRectangle(x-(int)(5*s), y-(int)(2*s), (int)(45*s), (int)(14*s), c);  // receiver
    DrawRectangle(x-(int)(35*s), y+(int)(0), (int)(32*s), (int)(10*s), c);   // stock
    DrawRectangle(x-(int)(38*s), y+(int)(6*s), (int)(8*s), (int)(8*s), c);   // stock pad
    DrawRectangle(x+(int)(50*s), y+(int)(10*s), (int)(6*s), (int)(10*s), c); // grip
}

static void DrawSmgSil(int x, int y, float s, Color c) {
    DrawRectangle(x, y, (int)(55*s), (int)(6*s), c);                           // barrel
    DrawRectangle(x-(int)(4*s), y-(int)(3*s), (int)(60*s), (int)(3*s), c);      // top rail
    DrawRectangle(x+(int)(5*s), y+(int)(6*s), (int)(20*s), (int)(8*s), c);      // body
    DrawRectangle(x+(int)(10*s), y+(int)(14*s), (int)(5*s), (int)(16*s), c);    // magazine
    DrawRectangle(x-(int)(15*s), y+(int)(0), (int)(15*s), (int)(8*s), c);       // stock fold
    DrawRectangle(x+(int)(25*s), y+(int)(8*s), (int)(10*s), (int)(12*s), c);    // grip
}

static void DrawSniperSil(int x, int y, float s, Color c) {
    DrawRectangle(x, y, (int)(95*s), (int)(4*s), c);                            // long barrel
    DrawRectangle(x+(int)(20*s), y-(int)(8*s), (int)(25*s), (int)(7*s), c);      // scope body
    DrawCircle(x+(int)(22*s), y-(int)(5*s), 4*s, Fade(c,0.5f));                  // scope front
    DrawCircle(x+(int)(43*s), y-(int)(5*s), 3*s, Fade(c,0.4f));                  // scope rear
    DrawRectangle(x-(int)(5*s), y-(int)(2*s), (int)(40*s), (int)(10*s), c);      // receiver
    DrawRectangle(x-(int)(35*s), y+(int)(0), (int)(32*s), (int)(8*s), c);        // stock
    DrawRectangle(x-(int)(38*s), y+(int)(4*s), (int)(6*s), (int)(6*s), c);       // stock pad
    DrawRectangle(x+(int)(15*s), y+(int)(8*s), (int)(5*s), (int)(12*s), c);      // magazine
    DrawRectangle(x+(int)(30*s), y+(int)(6*s), (int)(10*s), (int)(14*s), c);     // grip
}

// Draw weapon silhouette based on name and type string
static void DrawWeaponByType(const char *name, const char *type, int x, int y, float s, Color c) {
    if (strcmp(type, "PISTOL") == 0 || strcmp(type, "SPECIAL") == 0)
        DrawPistolSil(x, y, s, c);
    else if (strcmp(type, "RIFLE") == 0) {
        if (strcmp(name, "KAR-98K") == 0)
            DrawKar98kSil(x, y, s, c);
        else
            DrawRifleSil(x, y, s, c);
    } else if (strcmp(type, "SHOTGUN") == 0)
        DrawShotgunSil(x, y, s, c);
    else if (strcmp(type, "SMG") == 0)
        DrawSmgSil(x, y, s, c);
    else if (strcmp(type, "SNIPER") == 0)
        DrawSniperSil(x, y, s, c);
}

// ═══════════════════════════════════════════════════════════════════════════
// DECORATIVE HELPERS
// ═══════════════════════════════════════════════════════════════════════════

// Crosshair reticle
static void DrawReticle(int cx, int cy, int r, Color c) {
    DrawCircleLines(cx, cy, (float)r, c);
    DrawCircleLines(cx, cy, (float)(r-3), Fade(c, 0.3f));
    DrawLine(cx-r-4, cy, cx-3, cy, c);
    DrawLine(cx+3, cy, cx+r+4, cy, c);
    DrawLine(cx, cy-r-4, cx, cy-3, c);
    DrawLine(cx, cy+3, cx, cy+r+4, c);
    DrawCircle(cx, cy, 1.5f, c);
}

// Bullet cartridge standing up
static void DrawCartridge(int x, int y, float s, Color brass, Color tip) {
    int cw = (int)(5*s), ch = (int)(12*s);
    DrawRectangle(x-cw/2, y, cw, ch, brass);
    DrawTriangle(
        (Vector2){(float)(x-cw/2), (float)y},
        (Vector2){(float)x, (float)(y-(int)(8*s))},
        (Vector2){(float)(x+cw/2), (float)y},
        tip
    );
    DrawRectangle(x-cw/2, y+ch-(int)(2*s), cw, (int)(2*s), Fade(brass, 0.6f));
}

// Rivet / bolt
static void DrawRivet(int x, int y, float r) {
    DrawCircle(x, y, r, (Color){90,95,100,255});
    DrawCircleLines(x, y, r, (Color){40,42,48,255});
    DrawCircle(x-(int)(r*0.3f), y-(int)(r*0.3f), r*0.3f, (Color){120,125,130,80});
}

// Horizontal metal strip with brushed texture
static void DrawMetalStrip(int x, int y, int w, int h, Color base) {
    DrawRectangle(x, y, w, h, base);
    for (int i = 0; i < h; i += 2) {
        int a = 8 + (i*13)%16;
        DrawLine(x, y+i, x+w, y+i, (Color){255,255,255,(unsigned char)a});
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// MAIN DRAW
// ═══════════════════════════════════════════════════════════════════════════

void DrawMenu(void) {
    float t = (float)GetTime();
    Vector2 mouse = GetMousePosition();
    int mPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    // ── 1. BACKGROUND — dark concrete with subtle noise ────────────────
    ClearBackground((Color){18,18,22,255});
    for (int row = 0; row < SCREEN_H; row += 3) {
        int v = 18 + ((row*7 + row*row/200) % 6);
        DrawRectangle(0, row, SCREEN_W, 2, (Color){(unsigned char)v,(unsigned char)v,(unsigned char)(v+1),255});
    }
    // Vertical streaks (water stains on concrete)
    for (int col = 80; col < SCREEN_W; col += 160) {
        DrawRectangle(col, 0, 1, SCREEN_H, (Color){255,255,255,4});
        DrawRectangle(col+40, 60, 1, SCREEN_H-120, (Color){255,255,255,3});
    }

    // ── 2. AMBIENT GLOW — warm spotlight from above ────────────────────
    float glowPulse = 0.85f + 0.15f * sinf(t * 1.8f);
    // Central warm spotlight
    for (int i = 6; i >= 0; i--) {
        float r = 200.0f + i*60;
        int a = (int)(8.0f * glowPulse * (7-i) / 7.0f);
        DrawCircle(SCREEN_W/2, -30, r, (Color){255,180,80,(unsigned char)a});
    }

    // ── 3. PEGBOARD WALL (background texture for weapon area) ──────────
    int pegY = 108, pegH = 480;
    DrawRectangle(20, pegY, SCREEN_W-40, pegH, (Color){28,26,24,255});
    // Pegboard holes pattern
    for (int py = pegY+15; py < pegY+pegH-10; py += 20) {
        for (int px = 35; px < SCREEN_W-35; px += 20) {
            DrawCircle(px, py, 1.5f, (Color){15,14,12,255});
        }
    }
    // Left and right rails
    DrawMetalStrip(20, pegY, 8, pegH, (Color){50,52,55,255});
    DrawMetalStrip(SCREEN_W-28, pegY, 8, pegH, (Color){50,52,55,255});
    DrawRivet(24, pegY+10, 3);
    DrawRivet(24, pegY+pegH-10, 3);
    DrawRivet(SCREEN_W-24, pegY+10, 3);
    DrawRivet(SCREEN_W-24, pegY+pegH-10, 3);

    // ── 4. TOP HEADER — neon signboard ─────────────────────────────────
    // Metal plate background
    DrawMetalStrip(30, 8, SCREEN_W-60, 92, (Color){35,38,42,255});
    DrawRectangleLines(30, 8, SCREEN_W-60, 92, (Color){60,62,68,255});
    DrawRivet(40, 18, 3.5f);
    DrawRivet(SCREEN_W-40, 18, 3.5f);
    DrawRivet(40, 90, 3.5f);
    DrawRivet(SCREEN_W-40, 90, 3.5f);

    // Neon sign effect — "ARMORY & GUN SHOP"
    float flicker = (sinf(t*12.0f) > -0.1f) ? 1.0f : 0.3f; // occasional flicker
    float neonGlow = glowPulse * flicker;
    Color neonOrange = {255, 140, 30, (unsigned char)(220*neonGlow)};
    Color neonOrangeDim = {255, 140, 30, (unsigned char)(50*neonGlow)};
    Color neonWhite = {255, 220, 180, (unsigned char)(255*neonGlow)};
    // Glow bloom layers
    DrawText("ARMORY  &  GUN  SHOP", SCREEN_W/2-200, 18, 34, neonOrangeDim);
    DrawText("ARMORY  &  GUN  SHOP", SCREEN_W/2-198, 20, 34, neonOrange);
    DrawText("ARMORY  &  GUN  SHOP", SCREEN_W/2-199, 19, 34, neonWhite);

    // Sub-banner
    DrawRectangle(SCREEN_W/2-200, 58, 400, 1, Fade(neonOrange, 0.5f));
    DrawText("SENJATA  *  AMUNISI  *  AKSESORIS", SCREEN_W/2-148, 64, 12, (Color){180,170,140,(unsigned char)(180*neonGlow)});

    // Open/closed sign indicator
    float openBlink = (sinf(t*3.0f) > 0) ? 1.0f : 0.6f;
    DrawRectangleRounded((Rectangle){SCREEN_W-150.0f, 20, 70, 22}, 0.4f, 4, (Color){0,180,60,(unsigned char)(200*openBlink)});
    DrawText("OPEN", SCREEN_W-143, 24, 14, (Color){220,255,220,255});

    // Left/right neon accent bars
    DrawRectangle(35, 20, 3, 70, Fade(neonOrange, 0.6f*neonGlow));
    DrawRectangle(SCREEN_W-38, 20, 3, 70, Fade(neonOrange, 0.6f*neonGlow));

    // Decorative reticles on sign
    DrawReticle(90, 54, 14, Fade(neonOrange, 0.3f*neonGlow));
    DrawReticle(SCREEN_W-90, 54, 14, Fade(neonOrange, 0.3f*neonGlow));

    // ── 5. SEPARATOR — brass strip with cartridges ─────────────────────
    DrawRectangle(30, 102, SCREEN_W-60, 3, (Color){160,130,50,200});
    DrawRectangle(30, 105, SCREEN_W-60, 1, (Color){100,80,30,120});
    // Decorative cartridges along the strip
    for (int ci = 0; ci < 8; ci++) {
        int cx = 70 + ci * 125;
        DrawCartridge(cx, 86, 0.7f, (Color){200,170,70,(unsigned char)(60+ci*10)}, (Color){180,120,50,(unsigned char)(50+ci*10)});
    }

    Color btnBase = {30, 32, 28, 220};
    Color btnHover = {210, 160, 70, 255};
    Color btnText = {235, 230, 215, 255};

    int btnW = 360, btnH = 90;
    int btnX = SCREEN_W/2 - btnW/2;
    int btnY = 170;
    Rectangle startRect = {(float)btnX, (float)btnY, (float)btnW, (float)btnH};
    Rectangle aboutRect = {(float)btnX, (float)(btnY + btnH + 20), (float)btnW, (float)btnH};
    Rectangle exitRect = {(float)btnX, (float)(btnY + 2*(btnH + 20)), (float)btnW, (float)btnH};

    int startHover = CheckCollisionPointRec(mouse, startRect);
    int aboutHover = CheckCollisionPointRec(mouse, aboutRect);
    int exitHover = CheckCollisionPointRec(mouse, exitRect);

    if (!showStartChoices) {
        DrawRectangleRounded(startRect, 0.18f, 4, startHover ? (Color){42,44,38,240} : btnBase);
        DrawRectangleRoundedLines(startRect, 0.18f, 4, startHover ? btnHover : (Color){70,72,65,200});
        DrawText("START", btnX + 28, btnY + 18, 36, btnText);
        DrawText("Tes senjata api interaktif", btnX + 28, btnY + 54, 14, (Color){200,190,170,215});
        if (startHover && mPressed) showStartChoices = 1;

        DrawRectangleRounded(aboutRect, 0.18f, 4, aboutHover ? (Color){42,44,38,240} : btnBase);
        DrawRectangleRoundedLines(aboutRect, 0.18f, 4, aboutHover ? btnHover : (Color){70,72,65,200});
        DrawText("ABOUT", btnX + 28, btnY + btnH + 38, 36, btnText);
        DrawText("Tentang aplikasi", btnX + 28, btnY + btnH + 74, 14, (Color){200,190,170,215});
        if (aboutHover && mPressed) clickedProgram = MENU_CMD_ABOUT;

        DrawRectangleRounded(exitRect, 0.18f, 4, exitHover ? (Color){42,44,38,240} : btnBase);
        DrawRectangleRoundedLines(exitRect, 0.18f, 4, exitHover ? btnHover : (Color){70,72,65,200});
        DrawText("EXIT", btnX + 28, btnY + 2*(btnH + 20) + 18, 36, btnText);
        DrawText("Tutup toko dan kembali ke realitas", btnX + 28, btnY + 2*(btnH + 20) + 54, 14, (Color){200,190,170,215});
        if (exitHover && mPressed) clickedProgram = MENU_CMD_EXIT;

        (void)mPressed; // intentionally ignore keyboard input for menu navigation
    } else {
        int overlayX = 56;
        int overlayY = 146;
        int overlayW = SCREEN_W - 112;
        int overlayH = 346;
        DrawRectangle(overlayX, overlayY, overlayW, overlayH, (Color){16, 16, 18, 230});
        DrawRectangleLines(overlayX, overlayY, overlayW, overlayH, (Color){180, 140, 65, 180});
        DrawText("PILIH SENJATA", SCREEN_W/2 - 120, overlayY + 14, 36, (Color){235, 210, 140, 255});
        DrawText("Klik kartu di bawah untuk memilih Glock atau Kar-98k", SCREEN_W/2 - 260, overlayY + 56, 16, (Color){200, 190, 160, 220});

        int cardW = 360;
        int cardH = 200;
        int cardY = overlayY + 96;
        int leftX = overlayX + 28;
        int rightX = SCREEN_W - overlayX - cardW - 28;
        Rectangle cards[2] = {{(float)leftX, (float)cardY, (float)cardW, (float)cardH}, {(float)rightX, (float)cardY, (float)cardW, (float)cardH}};

        for (int i = 0; i < 2; i++) {
            Rectangle card = cards[i];
            int hover = CheckCollisionPointRec(mouse, card);
            Color bg = hover ? (Color){42, 44, 40, 220} : (Color){28, 28, 30, 220};
            Color border = hover ? btnHover : (Color){80, 78, 70, 200};
            DrawRectangleRounded(card, 0.16f, 4, bg);
            DrawRectangleRoundedLines(card, 0.16f, 4, border);
            if (hover) DrawRectangleRounded(card, 0.16f, 4, (Color){255,220,100,14});

            DrawText(weapons[i].name, card.x + 22, card.y + 18, 28, btnText);
            DrawText(weapons[i].type, card.x + 22, card.y + 48, 14, weapons[i].typeColor);
            DrawText(weapons[i].caliber, card.x + 22, card.y + 68, 14, (Color){190, 180, 150, 200});
            DrawText(weapons[i].desc, card.x + 22, card.y + 96, 14, (Color){200, 190, 170, 220});
            DrawWeaponByType(weapons[i].name, weapons[i].type, card.x + 264, card.y + 106, 1.0f, Fade(weapons[i].typeColor, hover ? 0.85f : 0.4f));

            Rectangle selectRect = {(float)(card.x + 22), (float)(card.y + cardH - 46), 140, 30};
            DrawRectangleRounded(selectRect, 0.2f, 3, hover ? btnHover : (Color){32, 34, 30, 220});
            DrawText("PILIH", selectRect.x + 44, selectRect.y + 8, 14, btnText);

            if (hover && mPressed) {
                clickedProgram = weapons[i].key;
                showStartChoices = 0;
            }
        }

        Rectangle closeRect = {(float)(SCREEN_W - overlayX - 80), (float)(overlayY + 12), 72, 28};
        int closeHover = CheckCollisionPointRec(mouse, closeRect);
        DrawRectangleRounded(closeRect, 0.35f, 3, closeHover ? (Color){180, 80, 50, 255} : (Color){90, 30, 28, 220});
        DrawText("BACK", closeRect.x + 14, closeRect.y + 6, 14, btnText);
        if (closeHover && mPressed) showStartChoices = 0;
    }

    // ── 8. FOOTER — HUD-style info bar ─────────────────────────────────
    int footY = SCREEN_H - 42;
    DrawRectangle(0, footY, SCREEN_W, 42, (Color){22,22,20,250});
    DrawRectangle(0, footY, SCREEN_W, 1, (Color){120,100,40,180});
    DrawRectangle(0, footY+1, SCREEN_W, 1, (Color){60,50,20,80});

    // Left: credits display
    DrawText("CREDITS:", 30, footY+8, 10, (Color){120,115,90,180});
    float creditPulse = 0.8f + 0.2f * sinf(t * 2.0f);
    DrawText("$ 99,999", 90, footY+6, 16, (Color){80,(unsigned char)(220*creditPulse),80,255});

    // Center: controls
DrawText("Gunakan mouse untuk memilih: START / ABOUT / EXIT", SCREEN_W/2-260, footY+6, 12, (Color){160,155,130,200});
        DrawText("Klik kartu senjata untuk membuka Glock atau Kar-98k", SCREEN_W/2-260, footY+22, 10, (Color){120,115,100,150});

    // Right: ammo counter display
    DrawRectangleRounded((Rectangle){(float)(SCREEN_W-170), (float)(footY+5), 140, 30}, 0.15f, 3, (Color){15,15,12,230});
    DrawRectangleRoundedLines((Rectangle){(float)(SCREEN_W-170), (float)(footY+5), 140, 30}, 0.15f, 3, (Color){60,80,60,200});
    DrawText("AMMO", SCREEN_W-165, footY+8, 9, (Color){80,120,80,150});
    // Animated counter
    int ammoDisp = 30 + (int)(sinf(t*0.5f)*5);
    DrawText(TextFormat("%03d / 090", ammoDisp), SCREEN_W-160, footY+19, 14, (Color){60,220,60,255});

    // ── 9. ANIMATED OVERLAYS ───────────────────────────────────────────
    // Scanline effect
    int scanY = ((int)(t * 60)) % SCREEN_H;
    DrawRectangle(0, scanY, SCREEN_W, 1, (Color){255,255,255,8});
    DrawRectangle(0, (scanY+233)%SCREEN_H, SCREEN_W, 1, (Color){255,100,30,5});

    // Corner brackets (HUD frame)
    Color cornerCol = Fade((Color){200,170,60,255}, 0.4f * glowPulse);
    int cLen = 25, cOff = 14;
    // Top-left
    DrawLineEx((Vector2){(float)cOff, (float)(cOff+8)}, (Vector2){(float)cOff, (float)(cOff+8+cLen)}, 2, cornerCol);
    DrawLineEx((Vector2){(float)cOff, (float)(cOff+8)}, (Vector2){(float)(cOff+cLen), (float)(cOff+8)}, 2, cornerCol);
    // Top-right
    DrawLineEx((Vector2){(float)(SCREEN_W-cOff), (float)(cOff+8)}, (Vector2){(float)(SCREEN_W-cOff), (float)(cOff+8+cLen)}, 2, cornerCol);
    DrawLineEx((Vector2){(float)(SCREEN_W-cOff), (float)(cOff+8)}, (Vector2){(float)(SCREEN_W-cOff-cLen), (float)(cOff+8)}, 2, cornerCol);
    // Bottom-left
    DrawLineEx((Vector2){(float)cOff, (float)(footY-5)}, (Vector2){(float)cOff, (float)(footY-5-cLen)}, 2, cornerCol);
    DrawLineEx((Vector2){(float)cOff, (float)(footY-5)}, (Vector2){(float)(cOff+cLen), (float)(footY-5)}, 2, cornerCol);
    // Bottom-right
    DrawLineEx((Vector2){(float)(SCREEN_W-cOff), (float)(footY-5)}, (Vector2){(float)(SCREEN_W-cOff), (float)(footY-5-cLen)}, 2, cornerCol);
    DrawLineEx((Vector2){(float)(SCREEN_W-cOff), (float)(footY-5)}, (Vector2){(float)(SCREEN_W-cOff-cLen), (float)(footY-5)}, 2, cornerCol);

    // Animated reticle that follows mouse subtly
    float mx = mouse.x, my = mouse.y;
    DrawReticle((int)mx, (int)my, 18, (Color){255,200,60,25});
}
