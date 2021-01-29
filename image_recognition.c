/* image_recognition extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_image_recognition.h"

/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif

/* {{{ void image_recognition_test1()
 */
PHP_FUNCTION(image_recognition_test1)
{
	ZEND_PARSE_PARAMETERS_NONE();

	php_printf("The extension %s is loaded and working!\r\n", "image_recognition");
}
/* }}} */

/* {{{ string image_recognition_test2( [ string $var ] )
 */
PHP_FUNCTION(image_recognition_test2)
{
	char *var = "World";
	size_t var_len = sizeof("World") - 1;
	zend_string *retval;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(var, var_len)
	ZEND_PARSE_PARAMETERS_END();

	retval = strpprintf(0, "Hello %s", var);

	RETURN_STR(retval);
}
/* }}}*/

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(image_recognition)
{
#if defined(ZTS) && defined(COMPILE_DL_IMAGE_RECOGNITION)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(image_recognition)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "image_recognition support", "enabled");
	php_info_print_table_end();
}
/* }}} */

/* {{{ arginfo
 */
ZEND_BEGIN_ARG_INFO(arginfo_image_recognition_test1, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_image_recognition_test2, 0)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ image_recognition_functions[]
 */
static const zend_function_entry image_recognition_functions[] = {
	PHP_FE(image_recognition_test1,		arginfo_image_recognition_test1)
	PHP_FE(image_recognition_test2,		arginfo_image_recognition_test2)
	PHP_FE_END
};
/* }}} */

/* {{{ image_recognition_module_entry
 */
zend_module_entry image_recognition_module_entry = {
	STANDARD_MODULE_HEADER,
	"image_recognition",					/* Extension name */
	image_recognition_functions,			/* zend_function_entry */
	NULL,							/* PHP_MINIT - Module initialization */
	NULL,							/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(image_recognition),			/* PHP_RINIT - Request initialization */
	NULL,							/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(image_recognition),			/* PHP_MINFO - Module info */
	PHP_IMAGE_RECOGNITION_VERSION,		/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_IMAGE_RECOGNITION
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(image_recognition)
#endif
