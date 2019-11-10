#pragma once

#include "texture.hpp"

class Rect;
class RectGFX;

void render_texture( Texture texture, const Rect& dest, int palette );
void render_rect( const Rect& rect, int color );

Texture render_get_texture( const char* name );

bool render_init_window();
int render_window_closed();
void render_present();
void render_start();
void render_init_gfx();