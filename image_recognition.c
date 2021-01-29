/* image_recognition extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "php.h"
#include "ext/standard/info.h"
#include "php_image_recognition.h"



static zend_class_entry * image_ce;
static const char  w[] = "width";
static const char  h[] = "height";
static const char  c[] = "config";
static const char  w1[] = "weights";
static const char  n[] = "net";

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

ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 4)
	ZEND_ARG_INFO(0, cfg)
	ZEND_ARG_INFO(0, wht)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, height)
ZEND_END_ARG_INFO()

ZEND_METHOD(ImageRecog, __construct)
{
	zend_string * cfg;
	zend_string * wht;
	zend_long  width;
	zend_long height;

	return_value = ZEND_THIS;
	
	php_printf("The extension %s is loaded and working__%s!\r\n", "image_recognition", "__construct");


	ZEND_PARSE_PARAMETERS_START(4, 4)
	Z_PARAM_PATH_STR(cfg)
	Z_PARAM_PATH_STR(wht)
	Z_PARAM_LONG(width)
	Z_PARAM_LONG(height)
	ZEND_PARSE_PARAMETERS_END();

	
	php_printf("config : %s\nweights : %s\nwidth : %ld, height : %ld\n", ZSTR_VAL(cfg), ZSTR_VAL(wht), width, height);

	zend_update_property_string(image_ce, return_value, c, sizeof(c)-1, ZSTR_VAL(cfg));
	zend_update_property_string(image_ce, return_value, w1, sizeof(w1)-1, ZSTR_VAL(wht));
	zend_update_property_long(image_ce, return_value, h, sizeof(h)-1, height);
	zend_update_property_long(image_ce, return_value, w, sizeof(w)-1, width);

}

static zend_function_entry image_recognition_methods[] = {
	
	PHP_ME(ImageRecog, __construct, arginfo_construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	
	PHP_FE_END
	};

PHP_MINIT_FUNCTION(image_recognition)
{
	zend_class_entry ce;


	INIT_CLASS_ENTRY(ce, "ImageRecog", image_recognition_methods);

	image_ce = zend_register_internal_class(&ce TSRMLS_CC);
	image_ce->ce_flags |= ZEND_ACC_FINAL;
	
	//zend_declare_property_null(image_ce, w, sizeof(w), ZEND_ACC_PUBLIC, TSRMLS_CC); 
	//zend_declare_property_null(image_ce, h, sizeof(h), ZEND_ACC_PUBLIC, TSRMLS_CC); 
	//zend_declare_property_null(image_ce, cfg, sizeof(cfg), ZEND_ACC_PUBLIC, TSRMLS_CC); 
	//zend_declare_property_null(image_ce, wht, sizeof(wht), ZEND_ACC_PUBLIC, TSRMLS_CC); 
	//zend_declare_property_null(image_ce, net, sizeof(net), ZEND_ACC_PUBLIC, TSRMLS_CC); 
	//
	zend_declare_property_long(image_ce, w, sizeof(w)-1, 0, ZEND_ACC_PUBLIC);
	zend_declare_property_long(image_ce, h, sizeof(h)-1, 0, ZEND_ACC_PUBLIC);
	zend_declare_property_string(image_ce, c, sizeof(c)-1, "", ZEND_ACC_PUBLIC);
	zend_declare_property_string(image_ce, w1, sizeof(w1)-1, "", ZEND_ACC_PUBLIC);
	
	return SUCCESS;
}




/* {{{ image_recognition_module_entry
 */
zend_module_entry image_recognition_module_entry = {
	STANDARD_MODULE_HEADER,
	"image_recognition",					/* Extension name */
	image_recognition_functions,			/* zend_function_entry */
	PHP_MINIT(image_recognition),			/* PHP_MINIT - Module initialization */
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
