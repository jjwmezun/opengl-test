#include "config.hpp"
#include <cstdio>
#include "game.hpp"
#include "glad.h"
#include "glfw3.h"
#include "render.hpp"

bool game_init()
{
    if ( !glfwInit() )
    {
        printf( "GLFW Failed to initialize!\n" );
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if ( !render_init_window() )
    {
        printf( "Window failed to initialize.\n" );
        game_close();
        return false;
    }

    // Load all OpenGL functions using the glfw loader function
    if ( !gladLoadGLLoader( ( GLADloadproc )( glfwGetProcAddress ) ) )
    {
        printf( "Failed to initialize OpenGL context\n" );
        return false;
    }

    render_init_gfx();

    if ( CONFIG_SHOW_OPENGL_INIT_INFO )
    {
        printf( "%s\n", glGetString( GL_VERSION ) );
        printf( "%s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    }

    return true;
}

void game_close()
{
    glfwTerminate();
}