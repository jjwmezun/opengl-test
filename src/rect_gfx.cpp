#include "glad.h"
#include "glfw3.h"
#include "ogl_error.hpp"
#include "rect_gfx.hpp"
#include "rect.hpp"

static unsigned int vertex_indices[ 6 ] =
{
    0, 1, 2,
    2, 3, 0
};

RectGFX rect_gfx_create( Rect rect, Color color )
{
    float vertex_positions[ 16 ] = {
        rect.x, rect.y, // Left Bottom
        rect_right( rect ), rect.y, // Right Bottom
        rect_right( rect ), rect_bottom( rect ), // Right Top
        rect.x, rect_bottom( rect ) // Left Top
    };

    unsigned int vao;
    ogl_call( glGenVertexArrays( 1, &vao ) );
    ogl_call( glBindVertexArray( vao ) );

    unsigned int buffer;
    ogl_call( glGenBuffers( 1, &buffer ) );
    ogl_call( glBindBuffer( GL_ARRAY_BUFFER, buffer ) );
    ogl_call( glBufferData( GL_ARRAY_BUFFER, 2 * 4 * sizeof( float ), vertex_positions, GL_STATIC_DRAW ) );

    ogl_call( glEnableVertexAttribArray( 0 ) );
    ogl_call( glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 2, nullptr ) );

    unsigned int ibo;
    ogl_call( glGenBuffers( 1, &ibo ) );
    ogl_call( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo ) );
    ogl_call( glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof( unsigned int ), vertex_indices, GL_STATIC_DRAW ) );

    return
    {
        color,
        vao,
        buffer,
        ibo
    };
};

void rect_gfx_update_position( RectGFX& rect_gfx, Rect position )
{
    float vertex_positions[ 8 ] = {
        position.x, position.y, // Left Bottom
        rect_right( position ), position.y, // Right Bottom
        rect_right( position ), rect_bottom( position ), // Right Top
        position.x, rect_bottom( position ) // Left Top
    };
    ogl_call( glBindBuffer( GL_ARRAY_BUFFER, rect_gfx.buffer ) );
    ogl_call( glBufferData( GL_ARRAY_BUFFER, 2 * 4 * sizeof( float ), vertex_positions, GL_STATIC_DRAW ) );
};

void rect_gfx_delete( RectGFX& rect_gfx )
{
    glDeleteBuffers( 1, &rect_gfx.ibo );
    glDeleteBuffers( 1, &rect_gfx.buffer );
    glDeleteVertexArrays( 1, &rect_gfx.vao );
}