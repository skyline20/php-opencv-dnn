/* image_recognition extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>

#include "php.h"
#include "ext/standard/info.h"
#include "php_image_recognition.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>

using namespace cv;
using namespace cv::dnn;
using std::string;

static zend_class_entry * image_ce;



extern zend_module_entry image_recognition_module_entry;

typedef struct _image_data
{
	string config;			//yolo的配置文件路径
	string weights;			//yolo的权重文件路径
	Size imageSize;			//输入图片的大小
	Net net;				//yolo网络

	Mat imageData;			//读取后的图片bgr数据
	Mat imageInput;			//imageData缩放后的网络数据

	Mat prob;				//预测后的返回值
	
	Point classIdPoint;		//forward后的预测值. x是类别值,y是可信度值
	double confidence;		//保存forward最大的可信度值	
} image_data;

typedef struct _image_recognition_object
{
	image_data * data;
	zend_object std;
	
} image_recognition_object;

static zend_object_handlers image_recognition_handlers;


static inline image_recognition_object * image_recognition_obj_from_obj(zend_object *obj) {
	return (image_recognition_object*)((char*)(obj) - XtOffsetOf(image_recognition_object, std));
}

#define Z_IMAGE_P(zv)  image_recognition_obj_from_obj(Z_OBJ_P((zv)))

static int init_cpp_object(image_recognition_object * image_object)
{
	//php_printf("new image data\n");
	image_object->data = new image_data();

	if(!image_object->data)
	{
		php_printf("new image data is null\n");
	}
	return 0;
}

static void free_cpp_object(image_recognition_object * image_object)
{
	//php_printf("entry free_cpp_object\n");
	delete image_object->data;
}

/*
// 多线程并发可能有问题
//参考:https://stackoverflow.com/questions/58434675/how-to-perform-deep-copy-with-cvdnnnet
static void clone_cpp_object(image_recognition_object * image_object, image_recognition_object * image_clone)
{
	image_clone.config = image_object->config;
	image_clone.weights = image_object->weights;

	image_clone.imageSize = image_object->imageSize;
	image_clone.net = image_object->net.clone();

}
*/

static image_recognition_object * image_recognition_objects_set_class(zend_class_entry *class_type) /* {{{ */
{
	//php_printf("entry image_recognition_objects_set_class\n");
	image_recognition_object *intern = (image_recognition_object *)zend_object_alloc(sizeof(image_recognition_object), class_type);
	if(!intern)
		return NULL;

	zend_class_entry *base_class = class_type;
	while ((base_class->type != ZEND_INTERNAL_CLASS || base_class->info.internal.module->module_number != image_recognition_module_entry.module_number) && base_class->parent != NULL) {
		base_class = base_class->parent;
	}

	//intern->prop_handler = zend_hash_find_ptr(&classes, base_class->name);

	zend_object_std_init(&intern->std, class_type);
	object_properties_init(&intern->std, class_type);

	init_cpp_object(intern);
	return intern;
}


//创建对象时,调用该函数分配空间和初始化
static zend_object * image_recognition_objects_new(zend_class_entry *class_type)
{
	//php_printf("entry image_recognition_objects_new\n");
	image_recognition_object *intern = image_recognition_objects_set_class(class_type);

	if(!intern)
		return NULL;
	
	intern->std.handlers = &image_recognition_handlers;

	return &intern->std;
}


static void image_recognition_objects_free_storage(zend_object *object)
{
	image_recognition_object *intern = image_recognition_obj_from_obj(object);
	zend_object_std_dtor(&intern->std);
	free_cpp_object(intern);
}



/* For compatibility with older PHP versions */
#ifndef ZEND_PARSE_PARAMETERS_NONE
#define ZEND_PARSE_PARAMETERS_NONE() \
	ZEND_PARSE_PARAMETERS_START(0, 0) \
	ZEND_PARSE_PARAMETERS_END()
#endif


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



static void image_forward(INTERNAL_FUNCTION_PARAMETERS, zval * predict,  zval * elapsed)
{
	image_data * data = NULL;

	double forward_start = 0;
	double forward_end = 0;
	double forward_elapsed = 0;

	data = Z_IMAGE_P(ZEND_THIS)->data;
	data->imageInput = blobFromImage(data->imageData, 1.0/255.0, data->imageSize, Scalar(), true, false);


	if(data->imageInput.empty())
	{
		zend_error(E_ERROR, "unable to normalize image data");
		RETURN_NULL();
	}

	try {
		data->net.setInput(data->imageInput);

		//如果需要记录计算的时间,则先记录开始的时间
		if(UNEXPECTED(elapsed)){
			forward_start = (double)cvGetTickCount();
			data->prob = data->net.forward();
			forward_end = (double)cvGetTickCount();
		}
		else
		{
			data->prob = data->net.forward();
		}

		
	} catch (const std::exception & e) {
		zend_error(E_ERROR, "forward error : %s", e.what());
		RETURN_NULL();
	}

	Mat re = data->prob.reshape(0, 1);
	minMaxLoc(re, 0, &data->confidence, 0, &data->classIdPoint);
	if(predict)
	{
		convert_to_array(predict);
		zend_hash_clean(Z_ARRVAL_P(predict));
		
		for(int index = 0; index < re.cols; index++)
		{			
			if(SUCCESS != add_index_double(predict, index, re.at<float>(index)))
				php_printf("add_index_double failed\n");

		}
	}


	if(elapsed)
	{
		//单位ms
		forward_elapsed = (forward_end - forward_start) / (cvGetTickFrequency() * 1000);
		ZEND_TRY_ASSIGN_REF_DOUBLE(elapsed, forward_elapsed);
	}

	RETURN_LONG(data->classIdPoint.x);

}


ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 2)
	ZEND_ARG_INFO(0, config)
	ZEND_ARG_INFO(0, weights)
	ZEND_ARG_INFO(0, imageSize)
ZEND_END_ARG_INFO()



ZEND_BEGIN_ARG_INFO_EX(arginfo_forward, 0, 0, 1)
	ZEND_ARG_INFO(0, image)
	ZEND_ARG_ARRAY_INFO(1, predict, 1)
	ZEND_ARG_INFO(1, elapsed)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_forward_data, 0, 0, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_ARRAY_INFO(1, predict, 1)
	ZEND_ARG_INFO(1, elapsed)
ZEND_END_ARG_INFO()


ZEND_METHOD(ImageRecog, forward_data)
{
	char *input = NULL;
	size_t inputlen = 0;
	zval * predict = NULL;
	zval * elapsed = NULL;
	image_data * data = NULL;
	
	
	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_STRING(input, inputlen)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(predict)
		Z_PARAM_ZVAL(elapsed)
	ZEND_PARSE_PARAMETERS_END();

	data = Z_IMAGE_P(ZEND_THIS)->data;
	std::vector<char> formatData;

	if(!input || 0>= inputlen)
	{
		zend_error(E_ERROR, "invalid image file data, should be jpg or png raw data");
		RETURN_NULL();
	}

	try {
		std::vector<char> formatData(inputlen);
		std::memcpy(formatData.data(), input, inputlen);
		data->imageData = imdecode(formatData, IMREAD_COLOR);
	}catch(const std::exception & e)
	{
		zend_error(E_ERROR, "unalbe to copy image raw data");
		RETURN_NULL();
	}
	
	if(data->imageData.empty())
	{
		zend_error(E_ERROR, "unable to get image data");
		RETURN_NULL();
	}
	
	image_forward(INTERNAL_FUNCTION_PARAM_PASSTHRU, predict, elapsed);

}




ZEND_METHOD(ImageRecog, forward)
{
	char * file;

	image_data * data = NULL;
	zend_string * image = NULL;
	zval * predict = NULL;
	zval * elapsed = NULL;


	
	ZEND_PARSE_PARAMETERS_START(1,3)
		Z_PARAM_PATH_STR(image)
		Z_PARAM_OPTIONAL
		Z_PARAM_ZVAL(predict)
		Z_PARAM_ZVAL(elapsed)
	ZEND_PARSE_PARAMETERS_END();


	
	data = Z_IMAGE_P(ZEND_THIS)->data;
	file = ZSTR_VAL(image);

	//php_printf("predict type : %d, elapsed type : %d\n", Z_TYPE_P(predict), Z_TYPE_P(predict));

	if(!data)
	{
		zend_error(E_ERROR, "Object is not initialized");
		RETURN_NULL();
	}


	if(0 != ::access(file, F_OK))
	{
		zend_error(E_ERROR, "%s is not accessable", file);
		RETURN_NULL();
	}

	data->imageData = imread(file);

	if(data->imageData.empty())
	{
		zend_error(E_ERROR, "unable to read image %s", file);
		RETURN_NULL();
	}


	image_forward(INTERNAL_FUNCTION_PARAM_PASSTHRU, predict, elapsed);

#if 0
    try {
        data->net.setInput(data->imageInput);

		//如果需要记录计算的时间,则先记录开始的时间
		if(UNEXPECTED(elapsed)){
			forward_start = (double)cvGetTickCount();
			data->prob = data->net.forward();
			forward_end = (double)cvGetTickCount();
		}
		else
		{
        	data->prob = data->net.forward();
		}

		
    } catch (const std::exception & e) {
    	zend_error(E_ERROR, "%s forward error : %s", file, e.what());
		RETURN_NULL();
    }

	Mat re = data->prob.reshape(0, 1);
	minMaxLoc(re, 0, &data->confidence, 0, &data->classIdPoint);
	if(predict)
	{
		php_printf("111 type = %d\n", Z_TYPE_P(predict));
		convert_to_array(predict);
		php_printf("222 type = %d\n", Z_TYPE_P(predict));
		zend_hash_clean(Z_ARRVAL_P(predict));
		
		for(int index = 0; index < re.cols; index++)
		{			
			if(SUCCESS != add_index_double(predict, index, re.at<float>(index)))
				php_printf("add_index_double failed\n");

		}
	}

	
/*
	Mat re1 = data->prob.reshape(0, 1);
	Mat re2 = data->prob.reshape(1, 1);

	php_printf("data->confidence = %f\n", data->confidence);

	php_printf("re1 dims = %d, re2 dims = %d\n", re1.dims, re2.dims);

	php_printf("re1 row = %d, colu = %d\n", re1.rows, re1.cols);
	php_printf("re2 row = %d, colu = %d\n", re2.rows, re2.cols);

	for(int index = 0; index < re1.cols; index++)
	{
		php_printf("[%d] = %f\n", index, re1.at<float>(index));
	}
	php_printf("\n");
*/

	if(elapsed)
	{
		//单位ms
		forward_elapsed = (forward_end - forward_start) / (cvGetTickFrequency() * 1000);
		ZEND_TRY_ASSIGN_REF_DOUBLE(elapsed, forward_elapsed);
	}

	RETURN_LONG(data->classIdPoint.x);

	#endif
}



static int check_input_image_size(int width, int height)
{
	//图片的大小为720x586
	if(0 > width || 720 < width || 0 > height || 586 < height)
		return -1;

	return 0;
}

static int request_image_input_size_from_args(image_data * data, zend_array * imageSize)
{
	zval * width = NULL;
	zval * height = NULL;
	unsigned int emement_count = 2;
	
	if(!data)
		return -1;


	if(!imageSize || zend_array_count(imageSize) != emement_count)
		return -1;


	width = zend_hash_index_find_deref(imageSize, 0);
	height = zend_hash_index_find_deref(imageSize, 1);

	if(!width || !height)
		return -1;

	if(0 != check_input_image_size(Z_LVAL_P(width), Z_LVAL_P(height)))
		return -1;

	data->imageSize = Size(Z_LVAL_P(width), Z_LVAL_P(height));

	return 0;
}

char * trim(char *str)
{
        char *p = str;
        char *p1;
        if(p)
        {
            p1 = p + strlen(str) - 1;
            while(*p && isspace(*p)) 
                p++;
            while(p1 > p && isspace(*p1)) 
                *p1--=0;
        }
        return p;
}


static int request_image_input_size_from_cfg(image_data * data, zend_string * config)
{
	char * line = NULL;
	size_t size = 1024;

	size_t read_chars;
	
	char * str;
	char * key;
	char * value;

	char * delim;
	int width = -1;
	int height = -1;

	
	FILE * f = NULL;


	if(!data || !config)
	{
		php_printf("image_object or config is null\n");
		return -1;
	}


	if(!ZSTR_VAL(config) || 0 >= ZSTR_LEN(config))
	{
		php_printf("Invalid config file agrument\n");
		return -1;
	}


	if(!(line = (char *)malloc(size)))
	{
		php_printf("unable to alloc buffer\n");
		return -1;
	}


	if(!(f = fopen(ZSTR_VAL(config), "r")))
	{
		php_printf("unable to open config file : %s\n", ZSTR_VAL(config));
		free(line);
		return -1;
	}



	while(0 < (read_chars = getline((char **)&line, &size, f)))
	{
		line[read_chars] = '\0';
		str = trim(line);
		
		//php_printf("read : %s, %lu\n", line, read_chars);
		
		if(str[0] == '#')
			continue;

		
		if(!(delim = strchr(str, '=')))
			continue;

		
		*delim = '\0';

		key = str, value = delim + 1;

		//php_printf("key = %s, value = %s\n", key, value);
		if(0 == strncasecmp(key, "width", 5))
		{
			width = strtol(value, NULL, 10);
			if(errno == 0)
				continue;
		}

		if(0 == strncasecmp(key, "height", 6))
		{
			height = strtol(value, NULL, 10);
			if(errno == 0)
				continue;
		}

		bzero(line, sizeof(*line));

		// 都找到之后退出		
		if(0 < width && 0 < height)
			break;
		
	}

	fclose(f);
	free(line);

	//php_printf("width = %d height = %d\n", width, height);

	if(0 != check_input_image_size(width, height))
		return -1;

	data->imageSize = Size(width, height);

	return 0;


}


static int request_image_input_size(image_data * data, zend_string * config, zend_array * imageSize)
{

	if(0 == request_image_input_size_from_args(data, imageSize))
		return 0;


	if(0 == request_image_input_size_from_cfg(data, config))
		return 0;

	return -1;
}




static int load_network(image_data * data)
{

	if(0 != access(data->config.c_str(), F_OK) || 0 != access(data->weights.c_str(), F_OK))
	{
		php_printf("%s or %s is not readable.\n", data->config.c_str(), data->weights.c_str());
		return -1;
	}


	try {
		data->net = readNetFromDarknet(data->config, data->weights);
	} catch (const std::exception & e) {
		php_printf("unable to load model %s\n", e.what());
		//zend_throw_error(NULL, "load %s and %s error : %s", ZSTR_VAL(cfg), ZSTR_VAL(wht), e.what());
		//zend_error(E_ERROR, "load %s and %s error : %s", ZSTR_VAL(cfg), ZSTR_VAL(wht), e.what());
		//RETURN_NULL();
		return -1;
	}

	if(data->net.empty())
	{
		php_printf("load model failed\n");
		return -1;
	}
	
	return 0;
}

static int check_config_model(image_data * data, zend_string * config, zend_string * weights)
{

	if(0 >= ZSTR_LEN(config) || 0 >= ZSTR_LEN(weights))
	{
		php_printf("invalid model config and weights arguments\n");
		return -1;
	}

	if(0 != access(ZSTR_VAL(config), F_OK) || 0 != access(ZSTR_VAL(weights), F_OK))
	{
		php_printf("unable to read %s or %s file\n", ZSTR_VAL(config), ZSTR_VAL(weights));
		return -1;
	}

	if(!data)
	{
		php_printf("image_object is empty\n");
		return -1;
	}
	

	//php_printf("config = %s, weights = %s\n", ZSTR_VAL(config), ZSTR_VAL(weights));
	data->config = string(ZSTR_VAL(config), ZSTR_LEN(config));
	data->weights = string(ZSTR_VAL(weights), ZSTR_LEN(weights));
	return 0;
}

ZEND_METHOD(ImageRecog, __construct)
{
	zend_string * config = NULL;
	zend_string * weights = NULL;
	zend_array *  imageSize = NULL;
	
	unsigned int emement_count = 2;
	
	image_recognition_object * image_object = Z_IMAGE_P(ZEND_THIS);


	ZEND_PARSE_PARAMETERS_START(2, 4)
	Z_PARAM_PATH_STR(config)
	Z_PARAM_PATH_STR(weights)
	Z_PARAM_OPTIONAL
	Z_PARAM_ARRAY_HT(imageSize)
	ZEND_PARSE_PARAMETERS_END();

	if(imageSize && zend_array_count(imageSize) != emement_count)
	{
		zend_error(E_ERROR, "Invalid image size dim");
		RETURN_NULL();
	}


	if(0 != request_image_input_size(image_object->data, config, imageSize))
	{
		zend_error(E_ERROR, "unable to get input image size");
		RETURN_NULL();
	}



	if(0 != check_config_model(image_object->data, config, weights))
	{
		zend_error(E_ERROR, "Invalid config or weights file");
		RETURN_NULL();
	}



	if(0 != load_network(image_object->data))
	{
		zend_error(E_ERROR, "unable to load yolo model");
		RETURN_NULL();
	}

	//php_printf("end construct\n");
}




static zend_function_entry image_recognition_methods[] = {
	
	PHP_ME(ImageRecog,		__construct,		arginfo_construct,		ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_MALIAS(ImageRecog,	__invoke,			forward,				arginfo_forward,	ZEND_ACC_PUBLIC)
	PHP_ME(ImageRecog,		forward,			arginfo_forward,		ZEND_ACC_PUBLIC)
	PHP_ME(ImageRecog,		forward_data,		arginfo_forward_data,	ZEND_ACC_PUBLIC)
	PHP_FE_END
};

PHP_MINIT_FUNCTION(image_recognition)
{
	zend_class_entry ce;

	//php_printf("image_recognition class function\n");
	
	memcpy(&image_recognition_handlers, &std_object_handlers, sizeof(zend_object_handlers));
	image_recognition_handlers.free_obj = image_recognition_objects_free_storage;
	INIT_CLASS_ENTRY(ce, "ImageRecog", image_recognition_methods);
	image_ce = zend_register_internal_class(&ce TSRMLS_CC);
	image_ce->create_object = image_recognition_objects_new;
	
	return SUCCESS;
}




/* {{{ image_recognition_module_entry
 */
zend_module_entry image_recognition_module_entry = {
	STANDARD_MODULE_HEADER,
	"image_recognition",					/* Extension name */
	NULL,									/* zend_function_entry */
	PHP_MINIT(image_recognition),			/* PHP_MINIT - Module initialization */
	NULL,									/* PHP_MSHUTDOWN - Module shutdown */
	PHP_RINIT(image_recognition),			/* PHP_RINIT - Request initialization */
	NULL,									/* PHP_RSHUTDOWN - Request shutdown */
	PHP_MINFO(image_recognition),			/* PHP_MINFO - Module info */
	PHP_IMAGE_RECOGNITION_VERSION,			/* Version */
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_IMAGE_RECOGNITION
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(image_recognition)
#endif
