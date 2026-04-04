#include "raylib.h"
#include "screen_type.h"
#include "src/ui/back_button.h"
#include "src/ui/audio_manager.h"
#include "src/screens/menu.h"
#include "src/screens/glock.h"
#include "src/screens/kar98k.h"
#include "src/screens/about.h"

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "Armory & Gun Shop - Grafika Komputer");
    InitAudioManager();
    SetTargetFPS(60);

    Screen current = MENU;

    while (!WindowShouldClose()) {
        if (current == MENU) {
            int clicked = GetClickedProgram();
            if (clicked == -1) {
                CloseWindow();
                return 0;
            }
            if (clicked == 100) current = ABOUT;
            if (clicked == 1) current = GLOCK;
            if (clicked == 2) current = KAR98K;
        }
        if (current != MENU && BackButtonPressed()) current = MENU;

        BeginDrawing();
        switch (current) {
            case MENU:     DrawMenu();     break;
            case GLOCK:    DrawGlock();    break;
            case KAR98K:   DrawKar98k();   break;
            case ABOUT:    DrawAbout();    break;
            default:
                // Placeholder for programs not yet implemented
                ClearBackground((Color){15, 15, 15, 255});
                DrawBackButton();
                DrawText("COMING SOON", SCREEN_W/2 - 100, SCREEN_H/2 - 20, 30, (Color){200,170,60,255});
                DrawText("Program ini belum tersedia", SCREEN_W/2 - 120, SCREEN_H/2 + 20, 16, GRAY);
                break;
        }
        EndDrawing();
    }

    CloseAudioManager();
    CloseWindow();
    return 0;
}
