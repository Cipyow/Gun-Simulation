#include "src/algo/midcircle.h"

/*
 * Helper: Plot 8 titik simetri sekaligus (8-way symmetry)
 * Lingkaran memiliki simetri pada sumbu X, Y, dan diagonal
 * Dengan menggambar 8 titik sekaligus, kita hanya perlu
 * menghitung 1/8 lingkaran (oktan pertama)
 */
static void PlotSymmetryPoints(int cx, int cy, int x, int y, Color color) {
    DrawPixel(cx + x, cy + y, color); // Oktan 1
    DrawPixel(cx + y, cy + x, color); // Oktan 2
    DrawPixel(cx + y, cy - x, color); // Oktan 3
    DrawPixel(cx + x, cy - y, color); // Oktan 4
    DrawPixel(cx - x, cy - y, color); // Oktan 5
    DrawPixel(cx - y, cy - x, color); // Oktan 6
    DrawPixel(cx - y, cy + x, color); // Oktan 7
    DrawPixel(cx - x, cy + y, color); // Oktan 8
}

/*
 * Midpoint Circle Algorithm (Bresenham-style)
 * 
 * Algoritma ini menggambar lingkaran menggunakan hanya operasi integer.
 * 
 * Konsep matematika:
 *   Persamaan lingkaran: x² + y² = r²
 *   Decision parameter d menentukan apakah titik selanjutnya
 *   lebih dekat ke E (x+1, y) atau SE (x+1, y-1)
 * 
 * Inisialisasi:
 *   x = 0, y = r
 *   d = 3 - 2r  (atau d = 1 - r untuk versi lain)
 * 
 * Iterasi:
 *   Jika d < 0:  pilih E,  d = d + 4x + 6
 *   Jika d >= 0: pilih SE, d = d + 4(x - y) + 10, y--
 *   x++
 */
void Midcircle(int centerX, int centerY, int radius, Color color) {
    if (radius <= 0) {
        DrawPixel(centerX, centerY, color);
        return;
    }
    
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;  // Decision parameter awal
    
    while (y >= x) {
        PlotSymmetryPoints(centerX, centerY, x, y, color);
        
        if (d < 0) {
            // Pilih titik E (East) - bergerak ke kanan
            d = d + 4 * x + 6;
        } else {
            // Pilih titik SE (South-East) - bergerak ke kanan-bawah
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

/*
 * Midcircle Filled - Lingkaran berisi (solid)
 * 
 * Menggunakan pendekatan yang sama dengan Midcircle,
 * tetapi menggambar garis horizontal untuk setiap baris y
 * alih-alih hanya titik tunggal.
 */
void MidcircleFilled(int centerX, int centerY, int radius, Color color) {
    if (radius <= 0) {
        DrawPixel(centerX, centerY, color);
        return;
    }
    
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    while (y >= x) {
        // Gambar garis horizontal untuk mengisi lingkaran
        // Menggunakan DrawPixel untuk setiap titik pada garis
        for (int i = centerX - x; i <= centerX + x; i++) {
            DrawPixel(i, centerY + y, color);
            DrawPixel(i, centerY - y, color);
        }
        for (int i = centerX - y; i <= centerX + y; i++) {
            DrawPixel(i, centerY + x, color);
            DrawPixel(i, centerY - x, color);
        }
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

/*
 * Midcircle Thick - Lingkaran dengan ketebalan
 * 
 * Menggambar beberapa lingkaran konsentris dengan radius
 * yang berbeda untuk menciptakan efek tebal.
 */
void MidcircleThick(int centerX, int centerY, int radius, int thickness, Color color) {
    if (radius <= 0 || thickness <= 0) {
        DrawPixel(centerX, centerY, color);
        return;
    }
    
    // Gambar lingkaran dari radius - thickness/2 hingga radius + thickness/2
    int innerR = radius - thickness / 2;
    if (innerR < 0) innerR = 0;
    int outerR = radius + thickness / 2;
    
    for (int r = innerR; r <= outerR; r++) {
        Midcircle(centerX, centerY, r, color);
    }
}

/*
 * Midcircle Dashed - Lingkaran putus-putus
 * 
 * Menggunakan counter untuk mengatur pola dash dan gap.
 * Karena lingkaran tidak linear dalam x atau y, kita menggunakan
 * jumlah total titik yang diplot sebagai pengukur.
 */
void MidcircleDashed(int centerX, int centerY, int radius, int dashLen, int gapLen, Color color) {
    if (radius <= 0) {
        DrawPixel(centerX, centerY, color);
        return;
    }
    
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;
    
    int counter = 0;
    int drawing = 1;
    int current_limit = dashLen;
    
    while (y >= x) {
        // Plot 8 titik dengan kondisi dash
        if (drawing) {
            DrawPixel(centerX + x, centerY + y, color);
            DrawPixel(centerX + y, centerY + x, color);
            DrawPixel(centerX + y, centerY - x, color);
            DrawPixel(centerX + x, centerY - y, color);
            DrawPixel(centerX - x, centerY - y, color);
            DrawPixel(centerX - y, centerY - x, color);
            DrawPixel(centerX - y, centerY + x, color);
            DrawPixel(centerX - x, centerY + y, color);
        }
        
        // Update dash/gap state
        counter += 8;  // 8 titik per iterasi
        if (counter >= current_limit) {
            counter = 0;
            drawing = !drawing;
            current_limit = drawing ? dashLen : gapLen;
        }
        
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}
