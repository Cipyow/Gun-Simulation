#ifndef MIDCIRCLE_H
#define MIDCIRCLE_H

#include "raylib.h"

/*
 * Midpoint Circle Algorithm (Bresenham-style)
 * 
 * Menggambar lingkaran menggunakan 8-way symmetry tanpa operasi floating-point.
 * Hanya menggunakan DrawPixel() sebagai primitif dasar.
 * 
 * Konsep:
 *   - Mulai dari titik (0, radius) pada oktan pertama
 *   - Decision parameter d menentukan pilih titik E (East) atau SE (South-East)
 *   - 8 titik simetri digambar sekaligus untuk setiap langkah
 */

void Midcircle(int centerX, int centerY, int radius, Color color);
void MidcircleFilled(int centerX, int centerY, int radius, Color color);
void MidcircleThick(int centerX, int centerY, int radius, int thickness, Color color);
void MidcircleDashed(int centerX, int centerY, int radius, int dashLen, int gapLen, Color color);

#endif
