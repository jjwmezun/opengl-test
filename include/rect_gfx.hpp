#pragma once

#include "color.hpp"

class Rect;

struct RectGFX
{
    Color color;
    unsigned int vao;
    unsigned int buffer;
    unsigned int ibo;
};

RectGFX rect_gfx_create( Rect rect, Color color );
void rect_gfx_update_position( RectGFX& rect_gfx, Rect position );
void rect_gfx_delete( RectGFX& rect_gfx );