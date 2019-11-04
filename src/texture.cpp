#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "glad.h"
#include "glfw3.h"
#include "ogl_error.hpp"
#include "texture.hpp"
#include "rect.hpp"

static unsigned int vertex_indices[ 6 ] =
{
    0, 1, 2,
    2, 3, 0
};

Texture texture_create( Rect rect )
{
    float vertex_positions[ 16 ] = {
        rect.x, rect.y, 0.0f, 1.0f,// Left Bottom
        rect_right( rect ), rect.y, 1.0f, 1.0f, // Right Bottom
        rect_right( rect ), rect_bottom( rect ), 1.0f, 0.0f, // Right Top
        rect.x, rect_bottom( rect ), 0.0f, 0.0f // Left Top
    };

    unsigned int vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    unsigned int buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, 4 * 4 * sizeof( float ), vertex_positions, GL_STATIC_DRAW );\

    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, nullptr );

    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 4, ( const void* )( sizeof( float ) * 2 ) );

    unsigned int ibo;
    glGenBuffers( 1, &ibo );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof( unsigned int ), vertex_indices, GL_STATIC_DRAW );

    unsigned int texture_id;
    glGenTextures( 1, &texture_id );
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, texture_id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

    FILE* gfx_file;
    long file_size;
    unsigned char* file_buffer;
    size_t fread_flag;
    gfx_file = fopen( "bin/autumn.jwi", "rb" );
    if ( !gfx_file )
    {
        printf( "File didn’t load: %s\n", "bin/autumn.jwi" );
    }
    else
    {
        fseek( gfx_file, 0, SEEK_END );
        file_size = ftell( gfx_file );
        rewind( gfx_file );
        file_buffer = ( unsigned char* )( malloc( sizeof( unsigned char ) * file_size ) );
        if ( !file_buffer )
        {
            printf( "Somehow run out o’ memory for loading file %s\n", "bin/autumn.jwi" );
        }
        fread_flag = fread( file_buffer, 1, file_size, gfx_file );
        if ( ( long )( fread_flag ) != file_size )
        {
            fputs( "Reading error", stderr );
        }

        int texture_width = ( ( unsigned int )( file_buffer[ 0 ] ) << 8 ) | file_buffer[ 1 ];
        int texture_height = ( ( unsigned int )( file_buffer[ 2 ] ) << 8 ) | file_buffer[ 3 ];
        unsigned char* texture_buffer = nullptr;

        const size_t image_data_size = file_size - 4;

        if ( image_data_size != ( size_t )( texture_width * texture_height ) )
        {
            printf( "GFX Load Error: File data doesn’t match width & height given!\n" );
        }
        else
        {
            texture_buffer = ( unsigned char* )( calloc( image_data_size, sizeof( unsigned char ) ) );
            memcpy( ( void* )( texture_buffer ), ( const void* )( &file_buffer[ 4 ] ), image_data_size );
        }

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, texture_width, texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, texture_buffer );
        glBindTexture( GL_TEXTURE_2D, 1 );

        fclose( gfx_file );
        free( file_buffer );
    }

    return
    {
        texture_id,
        vao,
        buffer,
        ibo
    };
};