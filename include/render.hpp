#pragma once

#include "texture.hpp"

class Rect;
class RectGFX;

void render_texture( Texture texture, const Rect& src, const Rect& dest, int palette, bool flip_x = false, bool flip_y = false, float rotation = 0.0f, float alpha = 1.0f, float rotation_origin_x = 0.0f, float rotation_origin_y = 0.0f );
void render_rect( const Rect& rect, int color );

Texture render_get_texture( const char* name );

bool render_init_window();
int render_window_closed();
void render_present();
void render_start();
void render_init_gfx();