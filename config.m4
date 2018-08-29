PHP_ARG_ENABLE(hello, whether to enable hello support,
[ --enable-hello   Enable hello support])

if test "$PHP_HELLO" = "yes"; then
    CLANG_LIBS="-L/usr/local/Cellar/llvm/6.0.1/lib -lclang"
    CLANG_INCL="-I/usr/local/Cellar/llvm/6.0.1/include"

    PHP_EVAL_LIBLINE($CLANG_LIBS, HELLO_SHARED_LIBADD)
    PHP_EVAL_INCLINE($CLANG_INCL)

    PHP_ADD_INCLUDE(/usr/local/Cellar/llvm/6.0.1/include)    
    PHP_ADD_LIBRARY_WITH_PATH(clang, /usr/local/Cellar/llvm/6.0.1/lib, HELLO_SHARED_LIBADD)

    PHP_SUBST(HELLO_SHARED_LIBADD)

    AC_DEFINE(HAVE_HELLO, 1, [Whether you have hello])
    PHP_NEW_EXTENSION(hello, hello.c, $ext_shared)
fi

