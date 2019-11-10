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

    const Rect autumn_dest_rect = { 128.0f, 96.0f, 16.0f, 25.0f };
    const Rect autumn_src_rect = { 0.0f, 0.0f, 16.0f, 25.0f };
    const Rect hydrant_dest_rect = { 192.0f, 32.0f, 16.0f, 16.0f };
    const Rect hydrant_src_rect = { 16.0f, 16.0f, 16.0f, 16.0f };
    //Texture autumn_texture = render_get_texture( "autumn" );
    Texture hydrant_texture = render_get_texture( "hydrant" );

    float rotation = 0.0f;

    while ( !render_window_closed() )
    {
        render_start();
        //render_rect( hydrant_dest_rect, 2 );
        //render_texture( autumn_texture, autumn_src_rect, autumn_dest_rect, 0 );
        render_texture( hydrant_texture, hydrant_src_rect, hydrant_dest_rect, 1 );
        render_present();

        rotation += 1.0f;

        /* Poll for and process events */
        glfwPollEvents();
    }

    game_close();
    return 0;
}