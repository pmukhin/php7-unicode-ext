/* unicode extension for PHP */

#ifndef PHP_UNICODE_H
# define PHP_UNICODE_H

#include <unistd.h>

#include "php.h"
#include "utf8proc.h"

extern zend_module_entry unicode_module_entry;

typedef struct _unicode_string {
    zend_object std; // zend_object

    const char *str; // data
    const utf8proc_int32_t *ubuffer; // data is stored as uint32 array

    size_t b_len; // bytes len 
    size_t u_len; // runes len
} unicode_string; 

# define phpext_unicode_ptr &unicode_module_entry
# define PHP_UNICODE_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_UNICODE)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_UNICODE_H */
