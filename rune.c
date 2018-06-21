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

unicode_rune *php_rune_internal_ctor(int32_t value) {
    unicode_rune *rune = php_rune_create(php_unicode_rune_ce);
    rune->rune = value;

    return rune;
}

static zend_object *php_rune_new(zend_class_entry *ce) {
    unicode_rune *rune = php_rune_create(ce);
    return &rune->std;
}

int get_ch_width(int32_t uc) {
    if (uc < 0x00) {
        return 0;
    } else if (uc < 0x80) {
        return 1;
    } else if (uc < 0x800) {
        return 2;
    } else if (uc < 0x10000) {
        return 3;
    } else if (uc < 0x110000) {
        return 4;
    }
    return -1;
}

ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_fromInt, 0, 0, 1)
	ZEND_ARG_INFO(0, i32)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_unicode_none, 0, 0, 0)
ZEND_END_ARG_INFO()

/* {{{ proto Rune Rune::fromInt32() 
        returns Rune corresponding to an int  */
PHP_METHOD(Rune, fromInt32) {
    zend_long i;
    if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l", &i) == FAILURE) {
        return;
    }
    unicode_rune *r = php_rune_internal_ctor((int32_t)i);
    RETVAL_OBJ((zend_object *) r);
}
/* }}} */

/* {{{ proto string Rune::__toString() 
        returns string representation of Rune  */
PHP_METHOD(Rune, __toString) {
    unicode_rune *this;
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    zend_string *zstr;
    this = (unicode_rune *) Z_OBJ_P(getThis());
    int32_t uc = this->rune;
    int mal_b = get_ch_width(uc);

    char buf[mal_b + 1];
    utf8proc_encode_char(uc, buf);
    buf[mal_b] = '\0';

    RETVAL_STRINGL(buf, mal_b);
}
/* }}} */

/* {{{ proto int Rune::toInt() 
        returns int representation of Rune  */
PHP_METHOD(Rune, toInt) {
    unicode_rune *this;
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    this = (unicode_rune *) Z_OBJ_P(getThis());
    RETVAL_LONG(this->rune);
}
/* }}} */

/* {{{ proto bool Rune::isAscii() 
        returns true if rune is in ASCII range (< 128)
        otherwise false */
PHP_METHOD(Rune, isAscii) {
    unicode_rune *this;
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    this = (unicode_rune *) Z_OBJ_P(getThis());
    if ((unsigned)this->rune < 128) {
        RETVAL_TRUE;
        return;
    }
    RETVAL_FALSE;
}
/* }}} */

/* {{{ proto bool Rune::isDigit() 
        returns true if rune is a digit
        otherwise false */
PHP_METHOD(Rune, isDigit) {
    unicode_rune *this;
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    this = (unicode_rune *) Z_OBJ_P(getThis());
    if (this->rune >= '0' && this->rune <= '9') {
        RETVAL_TRUE;
        return;
    }
    RETVAL_FALSE;
}
/* }}} */

/* {{{ proto bool Rune::isCyrillic() 
        returns true if rune is in cyrrilic range
        otherwise false */
PHP_METHOD(Rune, isCyrillic) {
    unicode_rune *this;
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    this = (unicode_rune *) Z_OBJ_P(getThis());
    if ((unsigned)this->rune >= 1024 && (unsigned)this->rune <= 1327) {
        RETVAL_TRUE;
        return;
    }
    RETVAL_FALSE;
}
/* }}} */

/* {{{ proto Rune Rune::toUpper() 
        returns uppercased rune */
PHP_METHOD(Rune, toUpper) {
    unicode_rune *this, *ret;
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    this = (unicode_rune *) Z_OBJ_P(getThis());
    int32_t upper = utf8proc_toupper(this->rune);
    ret = php_rune_internal_ctor(upper);

    RETVAL_OBJ((zend_object *) ret);
}
/* }}} */

/* {{{ proto Rune Rune::toLower() 
        returns lowercased rune */
PHP_METHOD(Rune, toLower) {
    unicode_rune *this, *ret;
    if (zend_parse_parameters_none_throw() == FAILURE) {
        return;
    }
    this = (unicode_rune *) Z_OBJ_P(getThis());
    int32_t lower = utf8proc_tolower(this->rune);
    ret = php_rune_internal_ctor(lower);

    RETVAL_OBJ((zend_object *) ret);
}
/* }}} */

static const zend_function_entry php_rune_ce_methods[] = {
    PHP_ME(Rune, fromInt32,    arginfo_unicode_fromInt,    ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    PHP_ME(Rune, __toString,   arginfo_unicode_none,       ZEND_ACC_PUBLIC)
    PHP_ME(Rune, toInt,        arginfo_unicode_none,       ZEND_ACC_PUBLIC)
    PHP_ME(Rune, isDigit,      arginfo_unicode_none,       ZEND_ACC_PUBLIC)
    PHP_ME(Rune, isAscii,      arginfo_unicode_none,       ZEND_ACC_PUBLIC)
    PHP_ME(Rune, isCyrillic,   arginfo_unicode_none,       ZEND_ACC_PUBLIC)

    PHP_ME(Rune, toUpper,      arginfo_unicode_none,       ZEND_ACC_PUBLIC)
    PHP_ME(Rune, toLower,      arginfo_unicode_none,       ZEND_ACC_PUBLIC)
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