#pragma once

#include <stdio.h>
#include <stdint.h>
#include <png.h>

typedef struct{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
} rgba8_t;

typedef struct{
    rgba8_t** table;
    png_uint_32 width;
    png_uint_32 height;
} image_table;

int image_create_table(image_table* table_p, 
        const png_uint_32 height, const png_uint_32 width);

void image_destroy_table(image_table* table_p);

int image_read_png(FILE* file_rb, image_table* table_p);

int image_write_png(FILE* file_wb, image_table* table_p);
