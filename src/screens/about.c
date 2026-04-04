#include "src/screens/about.h"
#include "src/algo/dda.h"
#include "src/algo/bresenham.h"
#include "src/ui/back_button.h"
#include "screen_type.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

static void DrawTextWrap(const char *text, int x, int y, int maxWidth, float fontSize, float spacing, Color color) {
    Font font = GetFontDefault();
    char buffer[1024];
    strncpy(buffer, text, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char current[1024] = "";
    int lineHeight = (int)(fontSize + spacing);
    const char *p = buffer;

    while (*p) {
        if (*p == '\n') {
            if (current[0] != '\0') {
                DrawTextEx(font, current, (Vector2){(float)x, (float)y}, fontSize, spacing, color);
                y += lineHeight;
                current[0] = '\0';
            }
            y += lineHeight / 2;
            p++;
            continue;
        }

        while (*p == ' ') p++;
        if (!*p) break;
        char word[256] = "";
        int wi = 0;
        while (*p && *p != ' ' && *p != '\n' && wi < (int)sizeof(word) - 1) {
            word[wi++] = *p++;
        }
        word[wi] = '\0';

        char test[1024];
        if (current[0] == '\0') {
            snprintf(test, sizeof(test), "%s", word);
        } else {
            snprintf(test, sizeof(test), "%s %s", current, word);
        }

        Vector2 size = MeasureTextEx(font, test, fontSize, spacing);
        if ((int)size.x > maxWidth && current[0] != '\0') {
            DrawTextEx(font, current, (Vector2){(float)x, (float)y}, fontSize, spacing, color);
            y += lineHeight;
            strcpy(current, word);
        } else {
            strcpy(current, test);
        }
    }

    if (current[0] != '\0') {
        DrawTextEx(font, current, (Vector2){(float)x, (float)y}, fontSize, spacing, color);
    }
}

void DrawAbout(void) {
    ClearBackground((Color){12, 12, 18, 255});

    // Back button
    DrawBackButton();

    // Header bar (positioned to the right of back button)
    DrawRectangle(128, 14, SCREEN_W-146, 72, (Color){16, 16, 22, 220});
    DrawRectangleLinesEx((Rectangle){128, 14, SCREEN_W-146, 72}, 2, (Color){170, 130, 60, 170});
    DrawText("ABOUT - Tentang Aplikasi", 144, 24, 28, (Color){245, 235, 190, 255});
    DrawText("Armory & Gun Shop", SCREEN_W - 320, 24, 22, (Color){255, 170, 80, 215});
    DrawText("Simulasi senjata api interaktif", SCREEN_W - 320, 50, 14, (Color){210, 190, 150, 180});

    // Subtle pegboard-style pattern
    for (int py = 110; py < SCREEN_H - 100; py += 28) {
        for (int px = 44; px < SCREEN_W - 44; px += 28) {
            DrawCircle(px, py, 1.0f, (Color){24, 24, 28, 170});
        }
    }

    // Main content card
    int cardX = SCREEN_W/2 - 320;
    int cardY = 112;
    int cardW = 640;
    int cardH = 500;
    DrawRectangleRounded((Rectangle){cardX, cardY, cardW, cardH}, 0.08f, 8, (Color){22, 24, 30, 220});
    DrawRectangleRoundedLines((Rectangle){cardX, cardY, cardW, cardH}, 0.08f, 8, (Color){190, 150, 70, 170});

    // Accent bars
    DrawRectangle(cardX + 18, cardY + 18, 6, cardH - 36, (Color){180, 140, 70, 180});
    DrawRectangle(cardX + cardW - 24, cardY + 18, 6, cardH - 36, (Color){180, 140, 70, 180});

    int leftX = cardX + 44;
    int rightX = cardX + cardW/2 + 16;
    int leftW = (cardW - 132) / 2;
    int rightW = leftW;
    int titleY = cardY + 38;

    DrawText("Deskripsi Aplikasi", leftX, titleY, 22, (Color){255, 210, 130, 255});
    DrawText("Fitur Utama", rightX, titleY, 22, (Color){255, 210, 130, 255});

    int bodyY = titleY + 34;
    const char *descText = "Aplikasi simulasi senjata api interaktif dengan suasana toko persenjataan. "
                           "Pengguna dapat menguji mekanik senjata seperti Glock 19 pistol "
                           "dan Kar-98k bolt-action rifle.";
    DrawTextWrap(descText, leftX, bodyY, leftW, 18.0f, 4.0f, (Color){220, 220, 225, 230});

    const char *featureText = "- Tes tembakan Glock 19 dan Kar-98k\n"
                              "- Mekanik bolt-action dan reload\n"
                              "- Sistem pengamanan (safe mode) pada Glock\n"
                              "- Visualisasi recoil, ejection casing, muzzle flash, dan sound effects\n";
    DrawTextWrap(featureText, rightX, bodyY, rightW, 16.0f, 4.0f, LIGHTGRAY);

    int techY = cardY + cardH - 200;
    DrawText("Teknologi", leftX, techY, 22, (Color){255, 210, 130, 255});
    const char *techText = "Dikembangkan dengan Raylib untuk rendering 2D dan input handling. "
                           "Menggunakan algoritma grafis komputer seperti Bresenham Line, DDA Line, dan Midpoint Circle "
                           "untuk rendering geometri senjata yang akurat. Ditulis dalam bahasa C sebagai bahasa pemrograman utama.";
    DrawTextWrap(techText, leftX, techY + 30, cardW - 88, 16.0f, 4.0f, LIGHTGRAY);

    // Bottom cartridges accent
    for (int i = 0; i < 6; i++) {
        int cx = cardX + 72 + i * 96;
        DrawRectangle(cx-3, cardY + cardH - 28, 6, 18, (Color){180, 140, 70, 200});
        DrawTriangle((Vector2){(float)(cx-4), (float)(cardY + cardH - 30)},
                     (Vector2){(float)cx, (float)(cardY + cardH - 44)},
                     (Vector2){(float)(cx+4), (float)(cardY + cardH - 30)},
                     (Color){215, 190, 120, 200});
    }

    DrawText("Klik BACK untuk kembali", 28, SCREEN_H-24, 14, (Color){170, 170, 190, 200});
}
