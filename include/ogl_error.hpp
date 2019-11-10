#pragma once

#define dassert( x ) if ( !( x ) ) __builtin_trap();
#define ogl_call( x ) ogl_clear_error();\
    x;\
    dassert( ogl_log_call( #x, __FILE__, __LINE__ ) )

void ogl_clear_error();
bool ogl_log_call( const char* function, const char* file, int line );

void ogl_check_error();