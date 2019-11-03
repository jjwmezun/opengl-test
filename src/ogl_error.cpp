#include <cstdio>
#include "glad.h"
#include "glfw3.h"
#include "ogl_error.hpp"

void ogl_clear_error()
{
    while( glGetError() != GL_NO_ERROR );
};
bool ogl_log_call( const char* function, const char* file, int line )
{
    if ( GLenum error = glGetError() )
    {
        printf( "OpenGL Error: %s in %s in %s on line %d\n", error, function, file, line );
        return false;
    }
    return true;
};