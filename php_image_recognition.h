/* image_recognition extension for PHP */

#ifndef PHP_IMAGE_RECOGNITION_H
# define PHP_IMAGE_RECOGNITION_H

extern zend_module_entry image_recognition_module_entry;
# define phpext_image_recognition_ptr &image_recognition_module_entry

# define PHP_IMAGE_RECOGNITION_VERSION "0.1.0"

# if defined(ZTS) && defined(COMPILE_DL_IMAGE_RECOGNITION)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_IMAGE_RECOGNITION_H */
