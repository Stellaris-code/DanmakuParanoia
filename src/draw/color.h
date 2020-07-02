#ifndef COLOR_H
#define COLOR_H

#include "utils/utils.h"

#include <string.h>
#include <ctype.h>

// from raylib.h
#if defined(__cplusplus)
#define COL_LITERAL(type)      type
#else
#define COL_LITERAL(type)      (type)
#endif

#define COL_LIGHTGRAY  COL_LITERAL(color_t){ 200, 200, 200, 255 }   // Light Gray
#define COL_GRAY       COL_LITERAL(color_t){ 130, 130, 130, 255 }   // Gray
#define COL_DARKGRAY   COL_LITERAL(color_t){ 80, 80, 80, 255 }      // Dark Gray
#define COL_YELLOW     COL_LITERAL(color_t){ 253, 249, 0, 255 }     // Yellow
#define COL_GOLD       COL_LITERAL(color_t){ 255, 203, 0, 255 }     // Gold
#define COL_ORANGE     COL_LITERAL(color_t){ 255, 161, 0, 255 }     // Orange
#define COL_PINK       COL_LITERAL(color_t){ 255, 109, 194, 255 }   // Pink
#define COL_RED        COL_LITERAL(color_t){ 230, 41, 55, 255 }     // Red
#define COL_MAROON     COL_LITERAL(color_t){ 190, 33, 55, 255 }     // Maroon
#define COL_GREEN      COL_LITERAL(color_t){ 0, 228, 48, 255 }      // Green
#define COL_LIME       COL_LITERAL(color_t){ 0, 158, 47, 255 }      // Lime
#define COL_DARKGREEN  COL_LITERAL(color_t){ 0, 117, 44, 255 }      // Dark Green
#define COL_SKYBLUE    COL_LITERAL(color_t){ 102, 191, 255, 255 }   // Sky Blue
#define COL_BLUE       COL_LITERAL(color_t){ 0, 121, 241, 255 }     // Blue
#define COL_DARKBLUE   COL_LITERAL(color_t){ 0, 82, 172, 255 }      // Dark Blue
#define COL_PURPLE     COL_LITERAL(color_t){ 200, 122, 255, 255 }   // Purple
#define COL_VIOLET     COL_LITERAL(color_t){ 135, 60, 190, 255 }    // Violet
#define COL_DARKPURPLE COL_LITERAL(color_t){ 112, 31, 126, 255 }    // Dark Purple
#define COL_BEIGE      COL_LITERAL(color_t){ 211, 176, 131, 255 }   // Beige
#define COL_BROWN      COL_LITERAL(color_t){ 127, 106, 79, 255 }    // Brown
#define COL_DARKBROWN  COL_LITERAL(color_t){ 76, 63, 47, 255 }      // Dark Brown

#define COL_WHITE      COL_LITERAL(color_t){ 255, 255, 255, 255 }   // White
#define COL_BLACK      COL_LITERAL(color_t){ 0, 0, 0, 255 }         // Black
#define COL_BLANK      COL_LITERAL(color_t){ 0, 0, 0, 0 }           // Blank (Transparent)
#define COL_MAGENTA    COL_LITERAL(color_t){ 255, 0, 255, 255 }     // Magenta

typedef struct color_t {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} color_t;

static inline color_t color_lerp(color_t from, color_t to, float t)
{
    color_t col;
    col.r = lerp(from.r, to.r, t);
    col.g = lerp(from.g, to.g, t);
    col.b = lerp(from.b, to.b, t);
    col.a = lerp(from.a, to.a, t);

    return col;
}

// TODO : named colors
static inline color_t str_to_color(const char* str)
{
    int has_alpha = 0;
    if (strncmp(str, "rgba(", 5) == 0)
    {
        has_alpha = 1;
        str += 4;
    }
    else if (strncmp(str, "rgb(", 4) == 0)
    {
        str += 3;
    }
    else
        return COL_WHITE;

    if (*str != '(')
        return COL_WHITE;
    ++str;
    char* endptr;

    while (isspace(*str))
        ++str;
    int r = strtol(str, &endptr, 0);
    str = endptr;
    if (str == NULL)
        return COL_WHITE;
    while (isspace(*str))
        ++str;
    if (*str != ',')
        return COL_WHITE;
    ++str;

    while (isspace(*str))
        ++str;
    int g = strtol(str, &endptr, 0);
    str = endptr;
    if (str == NULL)
        return COL_WHITE;
    while (isspace(*str))
        ++str;
    if (*str != ',')
        return COL_WHITE;
    ++str;

    while (isspace(*str))
        ++str;
    int b = strtol(str, &endptr, 0);
    str = endptr;
    if (str == NULL)
        return COL_WHITE;
    while (isspace(*str))
        ++str;
    if (has_alpha && *str != ',')
        return COL_WHITE;

    int a = 255;
    if (has_alpha)
    {
        ++str;
        while (isspace(*str))
            ++str;
        a = strtol(str, &endptr, 0);
        str = endptr;
        if (str == NULL)
            return COL_WHITE;
        while (isspace(*str))
            ++str;
    }

    if (*str != ')')
        return COL_WHITE;

    color_t col;
    col.r = r; col.b = b; col.g = g; col.a = a;

    return col;
}

#endif // COLOR_H
