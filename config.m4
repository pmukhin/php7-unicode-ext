PHP_ARG_ENABLE(unicode, whether to enable unicode support,
[  --enable-unicode          Enable unicode support], no)

if test "$PHP_UNICODE" != "no"; then
  AC_DEFINE(HAVE_UNICODE, 1, [ Have unicode support ])
  PHP_NEW_EXTENSION(unicode, unicode.c utf8proc.c rune.c, $ext_shared)
fi
