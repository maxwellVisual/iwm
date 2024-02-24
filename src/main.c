
#include <stdio.h>
#include <argp.h>
#include <string.h>
#include <stdlib.h>

#include "image.h"
#include "libiwm.h"

#define true ((char)1)
#define false ((char)0)
typedef char bool;

static char doc[] = "Welcome to IWM";
static char args_doc[] = "FILE SIGNATURE\nFILE";

const char *argp_program_version =
  "iwm version 0.0.1 20240224 (IWM)";
// const char *argp_program_bug_address =
//   "<bug-gnu-utils@gnu.org>";

typedef struct{
    const char* outputFile;
    const char* imageFile;
    const char* signature;
    bool verbose;
    bool decode;
} arguments;

static struct argp_option options[]={
    {"decode", 'd', 0, 0, "Decode watermark"},
    {"verbose", 'v', 0, 0, "Be verbose"},
    {"output", 'o', "FILE", 0, "Output to FILE"},
    {0},
};

static error_t parse_opt(int key,  char* arg, struct argp_state* state){
    arguments* arguments = state->input;

    switch(key){
        case 'o':
            arguments->outputFile = arg;
            if(!arg){
                argp_usage(state);
            }
            break;
        case 'd':
            arguments->decode = 1;
            break;
        case 'v':
            arguments->verbose = 1;
            break;
        case ARGP_KEY_ARG:
            if(!(arguments->imageFile)){
                arguments->imageFile = arg;
            }else if(!(arguments->signature)){
                arguments->signature = arg;
            }else{
                argp_usage(state);
            }
            break;
        case ARGP_KEY_END:
            if(!arguments->imageFile){
                goto end_err;
            }else if(!arguments->decode && !arguments->outputFile){
                goto end_err;
            }
            break;
            end_err:
            argp_usage(state);
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp={options, parse_opt, "FILE SIGNATURE\nFILE", doc};

#define vlog ;if(args.verbose) printf

int main(int argc, char **argv)
{
    arguments args;

    args.decode = 0;
    args.imageFile = NULL;
    args.signature = NULL;
    args.verbose = 0;
    args.outputFile = NULL;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    size_t length = strlen(args.imageFile);
    char outputFileName[length+8];//signed_
    if(!args.imageFile){
        goto err;
    }else if(!args.decode && !args.signature){
        goto err;
    }


    if(!args.outputFile){
    }
    FILE* inputFile = fopen(args.imageFile, "rb");
    if(!inputFile){
        fprintf(stderr, "Can't open file '%s'\n", args.imageFile);
        goto err;
    }
    FILE* outputFile;
    if(!args.outputFile){
        if(args.decode){
            outputFile = stdout;
        }else{
            char* begin=(char*)args.imageFile+length;
            while(*begin != '/' && *begin != '\\' && begin >=args.imageFile){
                begin--;
            }
            begin++;

            sprintf(outputFileName ,"signed_%s", begin);
            args.outputFile = outputFileName;

            outputFile = fopen(args.outputFile, "wb");
        }
    }else{
        outputFile = fopen(args.outputFile, "wb");
    }
    if(!outputFile){
        fprintf(stderr, "Can't open output file\n");
        goto err;
    }

    image_table table;

    if(image_read_png(inputFile, &table)){
        fprintf(stderr, "Can't read image from file '%s'\n", args.imageFile);
        goto err;
    }


    if(args.decode){
        char* buf;
        register size_t size = iwm_decode_table(&table, &buf);
        if(size == (size_t)-1){
            goto err;
        }
        fprintf(outputFile, "%s\n",buf);
        free(buf);
        vlog("signature length: %d\n", size);
    }else{
        iwm_sign_table(&table, args.signature, strlen(args.signature));
        if(image_write_png(outputFile, &table)){
            fprintf(stderr, "Can't write to file '%s'\n", args.outputFile);
            goto err;
        }
        vlog("encryped image of '%s' was created\n", args.outputFile);
    }

    return 0;
    err:
    return 1;
}
