#pragma once

#include <stdint.h>
#include <stddef.h>

#include "image.h"

void iwm_sign_table(image_table* table, const char* signature, size_t size);

size_t iwm_decode_table(image_table* table, char** buf);

int iwm_sign(const char* srcFileName, const char* destFileName, const char* signature, size_t size);

int iwm_decode(const char* srcFileName, const char** signature_p, size_t* length_p);