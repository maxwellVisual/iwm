#include "libiwm.h"

#include <stdint.h>
#include <stdlib.h>


typedef struct{
    const char* string;
    size_t size;
    size_t chr;
    char bit;
} cb_stream;

void cb_reset(cb_stream* stream){
    stream->bit=0;
    stream->chr=0;
}
void cb_create(cb_stream* stream, const char* string, size_t size){
    stream = malloc(sizeof(cb_stream));
    cb_reset(stream);
    stream->size = size;
    stream->string = string;
}

void cb_destroy(cb_stream* stream){
    free(stream);
}

char cb_next(cb_stream* stream){
    if(stream->bit >= 8){
        stream->bit = 0;
        stream->chr++;
        if(stream->chr>=stream->size){
            cb_reset(stream);
        }
    }

    return 1 & stream->string[stream->chr] >> stream->bit++;
}
char cb_eof(cb_stream* stream){
    return stream->chr>=stream->size || (stream->chr == stream->size-1 &&stream->bit == 8);
}
void cb_cseek(cb_stream* stream, size_t chr){
    stream->chr = chr;
    stream->bit=0;
}

char _pixmod(png_uint_32* color, char bit){
    if(bit){
        *color|=1;
    }else{
        *color&=(~1);
    }
    return bit;
}

void iwm_sign_table(image_table* table, const char* signature, size_t size){
    cb_stream stream;
    stream.string = signature;
    stream.size=size;

    char color_bit=0;
    size_t sign_width = size / table->height * table->width;

    for(size_t i=0; i<table->height; i++){
        for(size_t j=0; j<table->width; j++){
            register rgba8_t* pixel = &table->table[i][j];

            (void)_pixmod((png_uint_32*)&pixel->blue, cb_eof(&stream));
            register char next = cb_next(&stream);
            (void)_pixmod((png_uint_32*)&pixel->green, next);
            (void)_pixmod((png_uint_32*)&pixel->red, next);
        }
    }
}

typedef struct{
    char* buf;
    char bit_p;
} bc_stream;

void bc_create(bc_stream* stream, char* buf){
    stream->bit_p=0;
    stream->buf=buf;
}

void bc_push(bc_stream* stream, char bit){
    if(stream->bit_p>=8){
        stream->buf++;
        *(stream->buf) = 0;
        stream->bit_p=0;
    }

    if(bit){
        *(stream->buf) |= (1<<stream->bit_p);
    }

    stream->bit_p++;
}

size_t iwm_decode_table(image_table* table, char** buf){
    size_t length=table->width * table->height/8+1;
    char raw[length];
    char err[length];
    size_t dists[length];
    size_t count=0;
    size_t greatestDist=0;
    bc_stream stream_raw;
    bc_stream stream_err;
    bc_create(&stream_raw, (char*)&raw);
    bc_create(&stream_err, (char*)&err);

    size_t dist=0;
    for(size_t i=0; i<table->height; i++){
        for(size_t j=0; j<table->width; j++){
            register rgba8_t pixel = table->table[i][j];
            if(pixel.blue & 1){
                if(greatestDist<dist){
                    greatestDist=dist>>3;
                }
                dists[count-1] = dist>>3;
                count++;
                dist=1;
            }else{
                dist++;
            }
            bc_push(&stream_err, (pixel.green^pixel.red)&1);
            bc_push(&stream_raw, (pixel.green|pixel.red)&1);
        }
    }

    size_t majorityCount=0;
    size_t majoritySize=0;

    for(size_t i=0; i<count; i++){
        if(majoritySize == dists[i]){
            majorityCount++;
        }else if(majorityCount == 0){
            majoritySize = dists[i];
            majorityCount = 1;
        }else{
            majorityCount--;
        }
    }

    if(majorityCount == 0 || majoritySize == 0){// signature length undetermined
        return -1;
    }

    *buf = malloc(majoritySize+1);
    
    for(size_t i=0; i<majoritySize; i++){
        char major_char=0;
        size_t major_count=0;
        for(size_t layer = 0; layer*majoritySize < length; layer++){
            if(err[i+layer*majoritySize]){
                continue;
            }
            if(major_char == raw[i+layer*majoritySize]){
                major_count++;
            }else if(major_count == 0){
                major_char = raw[i+layer*majoritySize];
                major_count = 1;
            }else{
                major_count--;
            }
        }
        if(major_count == 0){
            goto err_free;
        }
        (*buf)[i] = major_char;
    }

    (*buf)[majoritySize]='\0';
    return majoritySize;

    err_free:
    free(*buf);
    *buf = NULL;
    return -1;
}


int iwm_sign(const char* srcFileName, const char* destFileName, const char* signature, size_t size);

int iwm_decode(const char* srcFileName, const char** signature_p, size_t* length_p);
