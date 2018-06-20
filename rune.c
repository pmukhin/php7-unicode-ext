#include "rune.h"
#include "utf8proc.h"

static zend_class_entry *php_unicode_rune_ce;
static zend_object_handlers php_unicode_rune_handlers;

static unicode_rune *php_rune_create(zend_class_entry *ce) {
    unicode_rune *rune = (unicode_rune*)emalloc(sizeof(unicode_rune));
	memset(rune, 0, sizeof(unicode_rune));

	zend_object_std_init(&rune->std, ce);
	rune->std.handlers = &php_unicode_rune_handlers;

	return rune;
}

static unicode_rune *php_rune_internal_ctor(int32_t value) {
    unicode_rune *rune = php_rune_create(php_unicode_rune_ce);
    rune->rune = value;

    return rune;
}

static zend_object *php_rune_new(zend_class_entry *ce) {
    unicode_rune *rune = php_rune_create(ce);
    return &rune->std;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_fromInt, 0, 0, 1)
	ZEND_ARG_INFO(0, i32)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_none, 0, 0, 0)
ZEND_END_ARG_INFO()

PHP_METHOD(Rune, fromInt32) {
    zend_long i;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l", &i) == FAILURE) {
        return;
    }
    unicode_rune *r = php_rune_internal_ctor((int32_t)i);
    RETVAL_OBJ((zend_object *) r);
}

PHP_METHOD(Rune, __toString) {
    unicode_rune *this;
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    zend_string *zstr;
    size_t mal_b = 0;
    
    this = (unicode_rune *) Z_OBJ_P(getThis());
    utf8proc_int32_t uc = (utf8proc_int32_t) this->rune;

    if (uc < 0x00) {
        zend_throw_exception(zend_exceptions_get_default(), "empty char", 0);
        return;
    } else if (uc < 0x80) {
        mal_b = 1;
    } else if (uc < 0x800) {
        mal_b = 2;
    } else if (uc < 0x10000) {
        mal_b = 3;
    } else if (uc < 0x110000) {
        mal_b = 4;
    }

    char buf[5];
    utf8proc_encode_char(uc, buf);
    buf[mal_b] = '\0';

    RETVAL_STRINGL(buf, mal_b);
}

static const zend_function_entry php_rune_ce_methods[] = {
    PHP_ME(Rune, fromInt32,    arginfo_unicode_fromInt,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(Rune, __toString,   arginfo_unicode_none,       ZEND_ACC_PUBLIC)
    PHP_FE_END
};

void php_register_rune_class() {
    zend_class_entry ce;

    memcpy(&php_unicode_rune_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	php_unicode_rune_handlers.free_obj = NULL;
	php_unicode_rune_handlers.clone_obj = NULL;

	INIT_CLASS_ENTRY(ce, "Rune", php_rune_ce_methods);
	php_unicode_rune_ce = zend_register_internal_class(&ce);
	php_unicode_rune_ce->ce_flags |= ZEND_ACC_FINAL;
    php_unicode_rune_ce->create_object = php_rune_new;
}