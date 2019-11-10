#include "config.hpp"
#include <cstdio>
#include "game.hpp"
#include "glad.h"
#include "glfw3.h"
#include "rect.hpp"
#include "render.hpp"
#include "texture.hpp"

#define TIMER_LIMIT 1000
#define NUMBER_OF_SQUARES 255

int main( int argc, char** argv )
{
    if ( !game_init() )
    {
        printf( "Game failed to initialize.\n" );
        return -1;
    }

    const Rect texture_rect = { 128.0f, 96.0f, 16.0f, 25.0f };
    Texture texture = texture_create();

    double times_list[ TIMER_LIMIT ];
    int times_counter = 0;
    double current_time = glfwGetTime();
    while ( !render_window_closed() )
    {
        current_time = glfwGetTime();
        render_start();
        render_texture( texture, texture_rect );
        render_rect( { 16.0f, 16.0f, 16.0f, 16.0f }, 2 );
        render_present();

        double render_time = glfwGetTime() - current_time;
        times_list[ times_counter ] = render_time;
        ++times_counter;
        if ( times_counter == TIMER_LIMIT )
        {
            double max = 0;
            double min = 9999.9;
            double total = 0;
            for ( int i = 0; i < TIMER_LIMIT; ++i )
            {
                if ( times_list[ i ] > max )
                {
                    max = times_list[ i ];
                }
                if ( times_list[ i ] < min )
                {
                    min = times_list[ i ];
                }
                total += times_list[ i ];
            }
            double average = total / TIMER_LIMIT;

            printf( "MAX: %f\n", max );
            printf( "MIN: %f\n", min );
            printf( "AVG: %f\n", average );
            break;
        }

        /* Poll for and process events */
        glfwPollEvents();  
    }

    game_close();
    return 0;
}