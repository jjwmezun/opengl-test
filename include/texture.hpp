#pragma once

class Rect;

struct Texture
{
    unsigned int id;
    unsigned int vao;
    unsigned int buffer;
    unsigned int ibo;
};

Texture texture_create();