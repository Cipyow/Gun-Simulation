#include "src/ui/back_button.h"
#include "raylib.h"

static Rectangle backBtn = { 12, 12, 110, 34 };

int BackButtonPressed(void) {
    Vector2 mouse = GetMousePosition();
    int hover = CheckCollisionPointRec(mouse, backBtn);
    if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return 1;
    return 0;
}

void DrawBackButton(void) {
    Vector2 mouse = GetMousePosition();
    int hover = CheckCollisionPointRec(mouse, backBtn);
    Color bg  = hover ? (Color){180,80,50,255} : (Color){90,30,28,220};
    Color brd = hover ? WHITE : (Color){140,80,60,255};
    DrawRectangleRounded(backBtn, 0.3f, 6, bg);
    DrawRectangleRoundedLines(backBtn, 0.3f, 6, brd);
    DrawText("BACK", (int)backBtn.x+18, (int)backBtn.y+9, 16, WHITE);
}
