#include "image.h"

#include <stdint.h>
#include <png.h>
#include <stdlib.h>

int image_create_table(image_table* table_p, 
        const png_uint_32 height, const png_uint_32 width){
    rgba8_t** table = table_p->table = malloc(height * sizeof(rgba8_t*));
    if(!table){
        return -1;
    }
    
    rgba8_t* buf = malloc(height * width * sizeof(rgba8_t));
    if(!buf){
        return -1;
    }
    table_p->height = height;
    table_p->width = width;
    for(size_t i=0; i<height; i++){
        table[i] = &(buf[i * width]);
    }
    return 0;
}
void image_destroy_table(image_table* table_p){
    free(table_p->table[0]);
    free(table_p->table);
    table_p->table = NULL;
}

int image_read_png(FILE* file_rb, image_table* table_p){
    int ret = 0;
    png_structrp png_ptr = NULL;
    png_inforp info_ptr = NULL;

    png_ptr = (png_structrp) 
        png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr){
        return -1;
    }

    info_ptr = (png_inforp) png_create_info_struct(png_ptr);
    if(!info_ptr){
        png_destroy_read_struct((png_structpp)&png_ptr, NULL, NULL);
        return -1;
    }

    int iRetVal = setjmp(png_jmpbuf(png_ptr));
    if(iRetVal){
        ret = -1;
        goto free_png;
    }


    png_init_io(png_ptr, file_rb);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width, height;
    int bit_depth, color_type;
    png_get_IHDR(png_ptr, info_ptr, 
        &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

    if(image_create_table(table_p, height, width)){
        ret = -1;
        goto free_png;
    }

    if(color_type == PNG_COLOR_TYPE_PALETTE){
        png_set_palette_to_rgb(png_ptr);
    }
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8){
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }
    if(bit_depth == 16){
        png_set_strip_16(png_ptr);
    }
    if(png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)){
        png_set_tRNS_to_alpha(png_ptr);
    }
    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA){
        png_set_gray_to_rgb(png_ptr);
    }

    png_read_image(png_ptr, (png_bytepp) table_p->table);
    png_read_end(png_ptr, info_ptr);

    free_png:
    png_destroy_read_struct((png_structpp)&png_ptr, (png_infopp)&info_ptr, NULL);
    return ret;
}

int image_write_png(FILE* file_wb, image_table* table_p){
    int ret = 0;
    png_structrp png_ptr = NULL;
    png_inforp info_ptr = NULL;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr){
        return -1;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(!info_ptr){
        ret = -1;
        goto free_png;
    }

    int iRetVal = setjmp(png_jmpbuf(png_ptr));
    if(iRetVal){
        printf("err:%d\n", iRetVal);
        goto free_png;
    }

    png_init_io(png_ptr, file_wb);
    png_set_IHDR(png_ptr, info_ptr, table_p->width, table_p->height, 8, 
        PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, 
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    png_write_image(png_ptr, (png_bytepp) table_p->table);
    png_write_end(png_ptr, info_ptr);

    free_png:
    png_destroy_write_struct((png_structpp)&png_ptr, (png_infopp)&info_ptr);
    return ret;
}