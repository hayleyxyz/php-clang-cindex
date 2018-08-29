#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include <clang-c/Index.h>
#include <clang-c/CXString.h>

#define PHP_MY_EXTENSION_VERSION "1.0"
#define PHP_MY_EXTENSION_EXTNAME "hello"

ZEND_BEGIN_ARG_INFO_EX(arginfo_clang_getInstantiationLocation, 0, 0, 5)
    ZEND_ARG_INFO(0, z_sourceLocation)
    ZEND_ARG_INFO(1, z_file)
    ZEND_ARG_INFO(1, z_line)
    ZEND_ARG_INFO(1, z_column)
    ZEND_ARG_INFO(1, z_offset)
ZEND_END_ARG_INFO()

PHP_FUNCTION(clang_createIndex);
PHP_FUNCTION(clang_parseTranslationUnit);
PHP_FUNCTION(clang_getTranslationUnitCursor);
PHP_FUNCTION(clang_visitChildren);
PHP_FUNCTION(clang_isCursorDefinition);
PHP_FUNCTION(clang_getCursorExtent);
PHP_FUNCTION(clang_getRangeStart);
PHP_FUNCTION(clang_getRangeEnd);
PHP_FUNCTION(clang_getInstantiationLocation);
PHP_FUNCTION(clang_getFileName);
PHP_FUNCTION(clang_getCursorKind);
PHP_FUNCTION(clang_getCursorKindSpelling);
PHP_FUNCTION(clang_getCursorSpelling);
PHP_FUNCTION(clang_getCursorDisplayName);
PHP_MINIT_FUNCTION(hello);

extern zend_module_entry hello_module_entry;
#define phpext_my_extension_ptr &hello_module_entry

static zend_function_entry hello_functions[] = {
    PHP_FE(clang_createIndex, NULL)
    PHP_FE(clang_parseTranslationUnit, NULL)
    PHP_FE(clang_getTranslationUnitCursor, NULL)
    PHP_FE(clang_visitChildren, NULL)
    PHP_FE(clang_isCursorDefinition, NULL)
    PHP_FE(clang_getCursorExtent, NULL)
    PHP_FE(clang_getRangeStart, NULL)
    PHP_FE(clang_getRangeEnd, NULL)
    PHP_FE(clang_getInstantiationLocation, arginfo_clang_getInstantiationLocation)
    PHP_FE(clang_getFileName, NULL)
    PHP_FE(clang_getCursorKind, NULL)
    PHP_FE(clang_getCursorKindSpelling, NULL)
    PHP_FE(clang_getCursorSpelling, NULL)
    PHP_FE(clang_getCursorDisplayName, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry hello_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_MY_EXTENSION_EXTNAME,
    hello_functions,
    PHP_MINIT(hello),
    NULL,
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_MY_EXTENSION_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#define CLANG_DEFINE_RES(name, type) \
    struct clang_##name { type name; }; \
    static int le_clang_##name; \
    char * le_clang_##name##_name = #type;

#define CLANG_RES_DTOR(name) \
    le_clang_##name = zend_register_list_destructors_ex(NULL, php_clang_##name##_dtor, le_clang_##name##_name, module_number);

#define CLANG_GET_ARG(name, type) \
    zval *user_##name; \
    struct clang_##name *res_##name; \
    \
    ZEND_PARSE_PARAMETERS_START(1, 1) \
        Z_PARAM_RESOURCE(user_##name) \
    ZEND_PARSE_PARAMETERS_END(); \
    type name; \
    \
    if((res_##name = (struct clang_##name *)zend_fetch_resource(Z_RES_P(user_##name), le_clang_##name##_name, le_clang_##name)) == NULL) { \
        RETURN_FALSE; \
    } \
    name = res_##name -> name; \

#define CLANG_REGISTER_RES(type, value, zval) \
    struct clang_##type *res_struct_##type##value = (struct clang_##type *)emalloc(sizeof(struct clang_##type)); \
    res_struct_##type##value->type = value; \
    ZVAL_RES(zval, zend_register_resource(res_struct_##type##value, le_clang_##type))

#define CLANG_ZVAL_TO_RES_VALUE(zval, name, type, var) \
    struct clang_##name * res_##type_##name = NULL; \
    if((res_##type_##name = (struct clang_##name *)zend_fetch_resource(Z_RES_P(zval), le_clang_##name##_name, le_clang_##name)) == NULL) { \
        RETURN_FALSE; \
    } \
    \
    type var = res_##type_##name -> name;

CLANG_DEFINE_RES(index, CXIndex);
CLANG_DEFINE_RES(unit, CXTranslationUnit);
CLANG_DEFINE_RES(cursor, CXCursor);
CLANG_DEFINE_RES(source_range, CXSourceRange);
CLANG_DEFINE_RES(source_location, CXSourceLocation);
CLANG_DEFINE_RES(file, CXFile);

ZEND_GET_MODULE(hello)

PHP_FUNCTION(clang_createIndex) {
    long excludeDeclarationsFromPCH;
    long displayDiagnostics;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &excludeDeclarationsFromPCH, &displayDiagnostics) == FAILURE) {
        RETURN_NULL();
    }

    CXIndex index = clang_createIndex(excludeDeclarationsFromPCH, displayDiagnostics);
    CLANG_REGISTER_RES(index, index, return_value);
}

PHP_FUNCTION(clang_parseTranslationUnit) {
    zval *userIndex;
    struct clang_index *actualIndex;
    zend_string *sourceFilename;
    zval *commandLineArgs;
    zend_long options;

    ZEND_PARSE_PARAMETERS_START(4, 4)
        Z_PARAM_RESOURCE(userIndex)
        Z_PARAM_STR(sourceFilename)
        Z_PARAM_ARRAY(commandLineArgs)
        Z_PARAM_LONG(options)
    ZEND_PARSE_PARAMETERS_END();

    if ((actualIndex = (struct clang_index *)zend_fetch_resource(Z_RES_P(userIndex), le_clang_index_name, le_clang_index)) == NULL) {
        RETURN_FALSE;
    }

    CXTranslationUnit unit;
    enum CXErrorCode err = clang_parseTranslationUnit2(
            actualIndex->index,
            ZSTR_VAL(sourceFilename),
            NULL, 0,
            NULL, 0,
            options,
            &unit);

    if(err != CXError_Success) {
        RETURN_FALSE;
    }

    CLANG_REGISTER_RES(unit, unit, return_value);
}

PHP_FUNCTION(clang_getTranslationUnitCursor) {
    CLANG_GET_ARG(unit, CXTranslationUnit);

    CXCursor cursor = clang_getTranslationUnitCursor(unit);

    CLANG_REGISTER_RES(cursor, cursor, return_value);
}

struct cursor_visitor_user_data {
    zend_fcall_info *callbackInfo;
    zend_fcall_info_cache *callbackCache;
};

enum CXChildVisitResult cursor_visitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    zval result, cursorVal, parentCursorVal;
    zval callback_params[2];
    struct cursor_visitor_user_data * user_data = (struct cursor_visitor_user_data *)client_data;

    CLANG_REGISTER_RES(cursor, cursor, &cursorVal);
    CLANG_REGISTER_RES(cursor, parent, &parentCursorVal);

    callback_params[0] = cursorVal;
    callback_params[1] = parentCursorVal;

    user_data->callbackInfo->params = &callback_params[0];
    user_data->callbackInfo->param_count = 2;
    user_data->callbackInfo->retval = &result;

    if(zend_call_function(user_data->callbackInfo, user_data->callbackCache) != SUCCESS) {
        return CXChildVisit_Break;
    }

    if(Z_TYPE_P(&result) == IS_LONG) {
        long visitResult = Z_LVAL_P(&result);
        switch(visitResult) {
            case CXChildVisit_Break:
            case CXChildVisit_Continue:
            case CXChildVisit_Recurse:
                return visitResult;
        }
    }

    return CXChildVisit_Break;
}

PHP_FUNCTION(clang_visitChildren) {
    zval *userCursor;
    struct clang_cursor *actualCursor;
    zend_fcall_info callbackInfo;
    zend_fcall_info_cache callbackCache;

    ZEND_PARSE_PARAMETERS_START(2, 2)
        Z_PARAM_RESOURCE(userCursor)
        Z_PARAM_FUNC(callbackInfo, callbackCache)
    ZEND_PARSE_PARAMETERS_END();

    if((actualCursor = (struct clang_cursor *)zend_fetch_resource(Z_RES_P(userCursor), le_clang_cursor_name, le_clang_cursor)) == NULL) {
        RETURN_FALSE;
    }

    CXCursor cursor = actualCursor->cursor;

    struct cursor_visitor_user_data user_data;
    user_data.callbackInfo = &callbackInfo;
    user_data.callbackCache = &callbackCache;

    clang_visitChildren(cursor, cursor_visitor, (CXClientData)&user_data);

    RETURN_TRUE;
}

PHP_FUNCTION(clang_isCursorDefinition) {
    CLANG_GET_ARG(cursor, CXCursor);

    RETURN_BOOL(clang_isCursorDefinition(cursor));
}

PHP_FUNCTION(clang_getCursorExtent) {
    CLANG_GET_ARG(cursor, CXCursor);

    CXSourceRange sourceRange = clang_getCursorExtent(cursor);

    CLANG_REGISTER_RES(source_range, sourceRange, return_value);
}

PHP_FUNCTION(clang_getRangeStart) {
    CLANG_GET_ARG(source_range, CXSourceRange);

    CXSourceLocation sourceLocation = clang_getRangeStart(source_range);

    CLANG_REGISTER_RES(source_location, sourceLocation, return_value);
}

PHP_FUNCTION(clang_getRangeEnd) {
    CLANG_GET_ARG(source_range, CXSourceRange);

    CXSourceLocation sourceLocation = clang_getRangeEnd(source_range);

    CLANG_REGISTER_RES(source_location, sourceLocation, return_value);
}

/*
 CINDEX_LINKAGE void clang_getInstantiationLocation(CXSourceLocation location,
                                                   CXFile *file,
                                                   unsigned *line,
                                                   unsigned *column,
                                                   unsigned *offset);
 */
PHP_FUNCTION(clang_getInstantiationLocation) {
    zval * z_sourceLocation = NULL;
    zval * z_file = NULL;
    zval * z_line = NULL;
    zval * z_column = NULL;
    zval * z_offset = NULL;

    CXFile file;
    unsigned int line, column, offset;

    ZEND_PARSE_PARAMETERS_START(1, 5)
        Z_PARAM_RESOURCE(z_sourceLocation)
        Z_PARAM_ZVAL_DEREF(z_file)
        Z_PARAM_ZVAL_DEREF(z_line)
        Z_PARAM_ZVAL_DEREF(z_column)
        Z_PARAM_ZVAL_DEREF(z_offset)
    ZEND_PARSE_PARAMETERS_END();

    CLANG_ZVAL_TO_RES_VALUE(z_sourceLocation, source_location, CXSourceLocation, sourceLocation);

    clang_getInstantiationLocation(sourceLocation, &file, &line, &column, &offset);

    CLANG_REGISTER_RES(file, file, z_file);

    ZVAL_LONG(z_line, line);
    ZVAL_LONG(z_column, column);
    ZVAL_LONG(z_offset, offset);

    RETURN_TRUE;
}

/**
 * /**
 * \brief Retrieve the complete file and path name of the given file.
 * CINDEX_LINKAGE CXString clang_getFileName(CXFile SFile);
 */
PHP_FUNCTION(clang_getFileName) {
    CLANG_GET_ARG(file, CXFile);
    CXString cx_file = clang_getFileName(file);
    zend_string * file_str;
    const char * str = NULL;

    str = clang_getCString(cx_file);

    if(str == NULL) {
        RETURN_NULL();
    }

    file_str = zend_string_init(str, strlen(str), 0);
    ZVAL_STR_COPY(return_value, file_str);

    clang_disposeString(cx_file);
}

PHP_FUNCTION(clang_getCursorKind) {
    CLANG_GET_ARG(cursor, CXCursor);

    enum CXCursorKind kind = clang_getCursorKind(cursor);

    RETURN_LONG(kind);
}

PHP_FUNCTION(clang_getCursorKindSpelling) {
    zend_long kind = 0;
    zend_string * temp_str = NULL;
    const char * c_spelling = NULL;
    CXString spelling;

    ZEND_PARSE_PARAMETERS_START(1, 1)
            Z_PARAM_LONG(kind)
    ZEND_PARSE_PARAMETERS_END();

    spelling = clang_getCursorKindSpelling(kind);
    c_spelling = clang_getCString(spelling);

    if(c_spelling == NULL) {
        RETURN_NULL();
    }

    temp_str = zend_string_init(c_spelling, strlen(c_spelling), 0);
    ZVAL_STR_COPY(return_value, temp_str);

    clang_disposeString(spelling);
}

PHP_FUNCTION(clang_getCursorSpelling) {
    zend_string * temp_str = NULL;
    const char * c_spelling = NULL;
    CXString spelling;

    CLANG_GET_ARG(cursor, CXCursor);

    spelling = clang_getCursorSpelling(cursor);
    c_spelling = clang_getCString(spelling);

    if(c_spelling == NULL) {
        RETURN_NULL();
    }

    temp_str = zend_string_init(c_spelling, strlen(c_spelling), 0);
    ZVAL_STR_COPY(return_value, temp_str);

    clang_disposeString(spelling);
}

PHP_FUNCTION(clang_getCursorDisplayName) {
    zend_string * temp_str = NULL;
    const char * c_spelling = NULL;
    CXString spelling;

    CLANG_GET_ARG(cursor, CXCursor);

    spelling = clang_getCursorDisplayName(cursor);
    c_spelling = clang_getCString(spelling);

    if(c_spelling == NULL) {
        RETURN_NULL();
    }

    temp_str = zend_string_init(c_spelling, strlen(c_spelling), 0);
    ZVAL_STR_COPY(return_value, temp_str);

    clang_disposeString(spelling);
}

ZEND_RSRC_DTOR_FUNC(php_clang_index_dtor) {
    struct clang_index *index = (struct clang_index *)res->ptr;
    clang_disposeIndex(index->index);
}

ZEND_RSRC_DTOR_FUNC(php_clang_unit_dtor) {
    struct clang_unit *unit = (struct clang_unit *)res->ptr;
    clang_disposeTranslationUnit(unit->unit);
}

ZEND_RSRC_DTOR_FUNC(php_clang_cursor_dtor) {

}

ZEND_RSRC_DTOR_FUNC(php_clang_source_range_dtor) {

}

ZEND_RSRC_DTOR_FUNC(php_clang_source_location_dtor) {

}

ZEND_RSRC_DTOR_FUNC(php_clang_file_dtor) {

}

PHP_MINIT_FUNCTION(hello) {
    CLANG_RES_DTOR(index)
    CLANG_RES_DTOR(unit)
    CLANG_RES_DTOR(cursor)
    CLANG_RES_DTOR(source_range)
    CLANG_RES_DTOR(source_location)
    CLANG_RES_DTOR(file)

    return SUCCESS;
}
