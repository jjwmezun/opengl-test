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

    const Rect autumn_rect = { 128.0f, 96.0f, 16.0f, 25.0f };
    const Rect hydrant_rect = { 192.0f, 32.0f, 48.0f, 32.0f };
    Texture autumn_texture = render_get_texture( "autumn" );
    Texture hydrant_texture = render_get_texture( "hydrant" );

    while ( !render_window_closed() )
    {
        render_start();
        render_texture( autumn_texture, autumn_rect, 0 );
        render_texture( hydrant_texture, hydrant_rect, 1 );
        render_rect( { 16.0f, 16.0f, 16.0f, 16.0f }, 2 );
        render_present();

        /* Poll for and process events */
        glfwPollEvents();
    }

    game_close();
    return 0;
}