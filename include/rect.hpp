#pragma once

#define rect_right( r ) ( ( r.x ) + ( r.w ) )
#define rect_bottom( r ) ( ( r.y ) + ( r.h ) )

struct Rect
{
    float x;
    float y;
    float w;
    float h;
};