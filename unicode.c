/* unicode extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_unicode.h"
#include "zend.h"
#include "zend_API.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"
#include "utf8proc.h"

#define DEBUG_USTRING(obj) 	printf("=============================\n"); \
							printf("b_len: %d\n", (int)obj->b_len); \
							printf("u_len: %d\n", (int)obj->u_len); \
							if (obj->str[0] == '\0') { \
								printf("str: 0\n"); \
							} else { \
								printf("str: %s\n", obj->str); \
							} \
							printf("ubuffer:\n"); \
							for (int i = 0; i <= (int)obj->u_len; ++i) { \
								printf("%d ->", obj->ubuffer[i]); \
							} \
							printf("\n=============================\n");

typedef enum casing_option {
    UTF8_LOWER,
    UTF8_UPPER,
	UTF8_TITLE,
} casing_option_t;

static zend_class_entry *php_unicode_ce;
static zend_object_handlers php_unicode_handlers;

static unicode_string *php_unicode_create(zend_class_entry *ce) {
	unicode_string *str = (unicode_string*)emalloc(sizeof(unicode_string));
	memset(str, 0, sizeof(unicode_string));

	zend_object_std_init(&str->std, ce);
	str->std.handlers = &php_unicode_handlers;

	return str;
}

static zend_object *php_unicode_new(zend_class_entry *ce) {
	unicode_string *str = php_unicode_create(ce);
	return &str->std;
}

static unicode_string *unicode_string_new(const char* data, size_t b_len) {
	unicode_string *ustr = php_unicode_create(php_unicode_ce);
	
	ustr->str = data;
	ustr->b_len = b_len;

	utf8proc_int32_t *ubuf = (utf8proc_int32_t *)ecalloc(b_len + 1, sizeof(utf8proc_int32_t));
	ubuf[b_len + 1] = '\0';
	size_t u_len = utf8proc_decompose((utf8proc_int8_t *)ustr->str, b_len, ubuf, b_len + 1, UTF8PROC_NULLTERM);

	ustr->ubuffer = ubuf;
	ustr->u_len = u_len;

	return ustr;
}

static void php_unicode_free(zend_object *object) {
	unicode_string *this = (unicode_string *) object;
	efree((void *)this->str);
	efree((void *)this->ubuffer);
	zend_object_std_dtor(&this->std);
}

static char *utf8_case(const unicode_string *obj, casing_option_t casing, utf8proc_option_t options) {
	size_t result = obj->u_len;
	utf8proc_int32_t *buffer = (utf8proc_int32_t *)ecalloc(result + 1, sizeof(utf8proc_int32_t));

    for (utf8proc_ssize_t i = 0; i < result; i++) {
        utf8proc_int32_t uc = obj->ubuffer[i];
        utf8proc_int32_t norm = uc;

		if (casing == UTF8_TITLE && i == 0) {
			norm = utf8proc_toupper(uc);
		} else if (casing == UTF8_LOWER) {
            norm = utf8proc_tolower(uc);
        } else if (casing == UTF8_UPPER) {
            norm = utf8proc_toupper(uc);
        }

        buffer[i] = norm;
    }

    result = utf8proc_reencode(buffer, result, options);
    if (result < 0) {
        efree(buffer);
        return NULL;
    }

    utf8proc_int32_t *newptr = (utf8proc_int32_t *) erealloc(buffer, (size_t)result+1);
    if (newptr != NULL) {
        buffer = newptr;
    } else {
        efree(buffer);
        return NULL;
    }

    return (char *)buffer;
}

static void drop_nchars(unicode_string *dest, unicode_string *obj, size_t n) {
	utf8proc_int32_t *buffer = (utf8proc_int32_t*)ecalloc(obj->u_len - n + 1, sizeof(utf8proc_int32_t));
	utf8proc_int32_t *cp = buffer;

	size_t b_len = 0;
	size_t u_len = 0;

	for (utf8proc_ssize_t i = 0; i < obj->u_len; i++) {
		if (n > 0) {
			--n;
			continue;
		}
		utf8proc_int32_t uc = obj->ubuffer[i];
		*cp = uc;
		cp++;
		b_len += utf8proc_charwidth(uc);
		++u_len;
	}

	dest->ubuffer = buffer;
	utf8proc_int32_t *str = (utf8proc_int32_t*) ecalloc(b_len + 1, sizeof(utf8proc_int32_t));
	memcpy( (void*)str, (void*)dest->ubuffer, sizeof(utf8proc_int32_t)*b_len + 1);

	size_t result = utf8proc_reencode(str, b_len, UTF8PROC_NULLTERM);

    if ((int)result < 0) {
        efree(buffer);
        return;
    }

	dest->str = str;
	dest->b_len = result;
	dest->u_len = u_len;
}

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(unicode)
{
#if defined(ZTS) && defined(COMPILE_DL_UNICODE)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(unicode)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "unicode support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */


ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_new, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()



ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_substring, 0, 0, 2)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_drop, 0, 0, 1)
	ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_startsWith, 0, 0, 1)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_none, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

PHP_METHOD(UnicodeString, __construct) {
	unicode_string *u_obj;
	char* zstr;
	size_t zstr_len;
	size_t u_len;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &zstr, &zstr_len) == FAILURE) {
		return;
	}

	u_obj = (unicode_string*)Z_OBJ_P(getThis());
	u_obj->str = (char*)emalloc(1+(sizeof(char)*zstr_len));

	memcpy((void*)u_obj->str, zstr, zstr_len+1);

	u_obj->b_len = zstr_len;

	utf8proc_int32_t *ubuf = (utf8proc_int32_t *)ecalloc( zstr_len + 1, sizeof(utf8proc_int32_t));
	u_len = utf8proc_decompose( (utf8proc_int8_t *)u_obj->str, zstr_len, ubuf, zstr_len + 1, UTF8PROC_NULLTERM );

	if (u_len == -1) {
		zend_throw_exception(zend_exception_get_default(), "can not handle", 1);
		return;
	}
	u_obj->ubuffer = ubuf;
	u_obj->u_len = u_len;
}

PHP_METHOD(UnicodeString, length) {
	unicode_string *u_obj;
	if (zend_parse_parameters_none_throw() == FAILURE) {
		return;
	}
	u_obj = (unicode_string*)Z_OBJ_P(getThis());
	RETVAL_LONG((long)u_obj->u_len);
}

PHP_METHOD(UnicodeString, toLowerCase) {
	unicode_string *this;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	this = (unicode_string *)Z_OBJ_P(getThis());
	char *str_cased = utf8_case(this, UTF8_LOWER, UTF8PROC_NULLTERM);

	unicode_string *copy = unicode_string_new(str_cased, this->b_len);
	RETVAL_OBJ((zend_object*)copy);
}

PHP_METHOD(UnicodeString, toUpperCase) {
	unicode_string *this;

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	this = (unicode_string *)Z_OBJ_P(getThis());
	char *str_cased = utf8_case(this, UTF8_UPPER, UTF8PROC_NULLTERM);

	unicode_string *copy = unicode_string_new(str_cased, this->b_len);
	RETVAL_OBJ((zend_object*)copy);
}

PHP_METHOD(UnicodeString, capitalize) {
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	unicode_string *this = (unicode_string *)Z_OBJ_P(getThis());
	const char *str_cased = utf8_case(this, UTF8_TITLE, UTF8PROC_NULLTERM);
	
	unicode_string *copy = unicode_string_new(str_cased, this->b_len);
	RETVAL_OBJ((zend_object*)copy);
}

PHP_METHOD(UnicodeString, substring) {
	long start, end;
	unicode_string *this;
	unicode_string *ret;
	char *data;
	char *dst;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "ll", &start, &end) == FAILURE) {
		return;
	}
	this = (unicode_string *)Z_OBJ_P(getThis());

	if (start == end) {
		zend_throw_exception(zend_exception_get_default(), "start == end", 0);
		return;
	}
	if (start >= end) {
		zend_throw_exception(zend_exception_get_default(), "start >= end", 0);
		return;
	} 
	if (end >= (int)this->u_len) {
		zend_throw_exception(zend_exception_get_default(), "out of bounds", 0);
		return;
	}

	ret = (unicode_string*)zend_objects_new(php_unicode_ce);
	printf("allocating %lu bytes for str\n", this->b_len);
	ret->str = (char *)emalloc(1 + (sizeof(char)*this->b_len));

	do {
		end--;
	} while (end > 0);

	RETVAL_NULL();
}

PHP_METHOD(UnicodeString, __toString) {
	if (zend_parse_parameters_none_throw() == FAILURE) {
		return;
	}
	unicode_string *this = (unicode_string *)Z_OBJ_P(getThis());
	zend_string *str = zend_string_init(this->str, this->b_len, 0);

	RETVAL_STR(str);
}

PHP_METHOD(UnicodeString, empty) {
	if (zend_parse_parameters_none_throw() == FAILURE) {
		return;
	}
	unicode_string *this = (unicode_string *)Z_OBJ_P(getThis());
	if (0 == this->b_len) {
		RETVAL_TRUE;
		return;
	}
	RETVAL_FALSE;
}

PHP_METHOD(UnicodeString, nonEmpty) {
	if (zend_parse_parameters_none_throw() == FAILURE) {
		return;
	}
	unicode_string *this = (unicode_string *)Z_OBJ_P(getThis());
	if (0 == this->b_len) {
		RETVAL_FALSE;
		return;
	}
	RETVAL_TRUE;
}

PHP_METHOD(UnicodeString, drop) {
	zend_long n;
	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l", &n) == FAILURE) {
		return;
	}

	unicode_string *this = (unicode_string *)Z_OBJ_P(getThis());
	unicode_string *ret = php_unicode_create(php_unicode_ce);

	drop_nchars(ret, this, (size_t)n);

	RETVAL_OBJ((zend_object*)ret);
}

PHP_METHOD(UnicodeString, startsWith) {
	unicode_string *this;
	char *substr;
	size_t substr_len;
	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &substr, &substr_len) == FAILURE) {
		return;
	}
	this = (unicode_string *) Z_OBJ_P(getThis());
	if (substr_len > this->b_len) {
		RETVAL_FALSE;
		return;
	}
	for (int i = 0; i < (int)substr_len; ++i) {
		if (this->str[i] != substr[i]) {
			RETVAL_FALSE;
			return;
		}
	}
	RETVAL_TRUE;
}

PHP_METHOD(UnicodeString, endsWith) {
	unicode_string *this;
	char *substr;
	size_t substr_len;
	int cmp_r;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &substr, &substr_len) == FAILURE) {
		return;
	}
	this = (unicode_string *) Z_OBJ_P(getThis());
	if (substr_len > this->b_len) {
		RETVAL_FALSE;
		return;
	}

	char *copy = this->str;
	copy += this->b_len - substr_len;

	if (cmp_r == strcmp(copy, substr)) {
		RETVAL_TRUE;
		return;
	}
	RETVAL_FALSE;
}

static const zend_function_entry php_unicode_ce_methods[] = {
	PHP_ME(UnicodeString, __construct, 	arginfo_unicode_new, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, __toString, 	arginfo_unicode_none, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, empty, 		arginfo_unicode_none, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, nonEmpty, 	arginfo_unicode_none, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, length, 		arginfo_unicode_none, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, toLowerCase,	arginfo_unicode_none, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, toUpperCase,	arginfo_unicode_none, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, capitalize,	arginfo_unicode_none, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, drop,			arginfo_unicode_drop, 		ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, startsWith,	arginfo_unicode_startsWith, ZEND_ACC_PUBLIC)
	PHP_ME(UnicodeString, endsWith,		arginfo_unicode_startsWith, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

PHP_MINIT_FUNCTION(unicode) {
	zend_class_entry ce;

	memcpy(&php_unicode_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_unicode_handlers.free_obj = php_unicode_free;
	php_unicode_handlers.clone_obj = NULL;

	INIT_CLASS_ENTRY(ce, "UnicodeString", php_unicode_ce_methods);
	php_unicode_ce = zend_register_internal_class(&ce);
	php_unicode_ce->ce_flags |= ZEND_ACC_FINAL;
	php_unicode_ce->create_object = php_unicode_new;

	return SUCCESS;
}

/* {{{ unicode_module_entry
 */
zend_module_entry unicode_module_entry = {
	STANDARD_MODULE_HEADER,
	"unicode",					/* Extension name */
	NULL,						/* zend_function_entry */
	PHP_MINIT(unicode),			/* PHP_MINIT - Module initialization */
	NULL,						/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(unicode),			/* PHP_RINIT - Request initialization */
	NULL,						/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(unicode),			/* PHP_MINFO - Module info */
	PHP_UNICODE_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES 
};
/* }}} */

#ifdef COMPILE_DL_UNICODE
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(unicode)
#endif
