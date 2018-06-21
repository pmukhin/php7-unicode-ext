#ifndef PHP_UNICODE_RUNE_H
#define PHP_UNICODE_RUNE_H

#include "php.h"

typedef struct unicode_rune {
    zend_object std; // zend_object
    int32_t rune;    // content
} unicode_rune;

unicode_rune *php_rune_internal_ctor(int32_t value);

void php_register_rune_class();

int get_ch_width(int32_t);

#endif