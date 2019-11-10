#include "config.hpp"
#include <cstdio>
#include "game.hpp"
#include "glad.h"
#include "glfw3.h"
#include "rect.hpp"
#include "render.hpp"
#include "texture.hpp"

int main( int argc, char** argv )
{
    if ( !game_init() )
    {
        printf( "Game failed to initialize.\n" );
        return -1;
    }

    const Rect texture_rect = { 128.0f, 96.0f, 16.0f, 25.0f };
    Texture texture = texture_create();

    while ( !render_window_closed() )
    {
        render_start();
        render_texture( texture, texture_rect );
        render_rect( { 16.0f, 16.0f, 16.0f, 16.0f }, 2 );
        render_present();

        /* Poll for and process events */
        glfwPollEvents();  
    }

    game_close();
    return 0;
}