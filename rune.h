#ifndef PHP_UNICODE_RUNE_H
#define PHP_UNICODE_RUNE_H

#include "php.h"

typedef struct unicode_rune {
    zend_object std; // zend_object
    int32_t rune;    // content
} unicode_rune;

void php_register_rune_class();

#endif