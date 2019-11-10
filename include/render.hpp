#pragma once

class Rect;
class RectGFX;
class Texture;

void render_texture( const Texture& texture, Rect rect );
void render_rect( const RectGFX& rect_gfx );
bool render_init_window();
int render_window_closed();
void render_present();
void render_start();
void render_init_gfx();