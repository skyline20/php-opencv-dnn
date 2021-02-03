dnl config.m4 for extension image_recognition

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary.

dnl If your extension references something external, use 'with':

dnl PHP_ARG_WITH([image_recognition],
dnl   [for image_recognition support],
dnl   [AS_HELP_STRING([--with-image_recognition],
dnl     [Include image_recognition support])])

dnl Otherwise use 'enable':

PHP_ARG_ENABLE([image_recognition],
  [whether to enable image_recognition support],
  [AS_HELP_STRING([--enable-image_recognition],
    [Enable image_recognition support])],
  [no])

if test "$PHP_IMAGE_RECOGNITION" != "no"; then
  dnl Write more examples of tests here...

  dnl Remove this code block if the library does not support pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl PHP_EVAL_INCLINE($LIBFOO_CFLAGS)
  dnl PHP_EVAL_LIBLINE($LIBFOO_LIBS, IMAGE_RECOGNITION_SHARED_LIBADD)

  dnl If you need to check for a particular library version using PKG_CHECK_MODULES,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-image_recognition -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/image_recognition.h"  # you most likely want to change this
  dnl if test -r $PHP_IMAGE_RECOGNITION/$SEARCH_FOR; then # path given as parameter
  dnl   IMAGE_RECOGNITION_DIR=$PHP_IMAGE_RECOGNITION
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for image_recognition files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       IMAGE_RECOGNITION_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$IMAGE_RECOGNITION_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the image_recognition distribution])
  dnl fi

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-image_recognition -> add include path
  dnl PHP_ADD_INCLUDE($IMAGE_RECOGNITION_DIR/include)

  dnl Remove this code block if the library supports pkg-config.
  dnl --with-image_recognition -> check for lib and symbol presence
  dnl LIBNAME=IMAGE_RECOGNITION # you may want to change this
  dnl LIBSYMBOL=IMAGE_RECOGNITION # you most likely want to change this

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   AC_DEFINE(HAVE_IMAGE_RECOGNITION_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your image_recognition library.])
  dnl ], [
  dnl   $LIBFOO_LIBS
  dnl ])

  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY($LIBNAME, $LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $IMAGE_RECOGNITION_DIR/$PHP_LIBDIR, IMAGE_RECOGNITION_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_IMAGE_RECOGNITION_FEATURE, 1, [ ])
  dnl ],[
  dnl   AC_MSG_ERROR([FEATURE not supported by your image_recognition library.])
  dnl ],[
  dnl   -L$IMAGE_RECOGNITION_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(IMAGE_RECOGNITION_SHARED_LIBADD)

  dnl In case of no dependencies
  dnl PKG_CHECK_MODULES([OPENCV], [opencv4 >= 4.0.0])
  dnl PHP_EVAL_INCLINE($OPENCV_CFLAGS)
  dnl PHP_EVAL_LIBLINE($OPENCV_LIBS, OPENCV_SHARED_LIBADD)
  dnl

  PHP_ADD_INCLUDE(/usr/local/opencv-4.5.1/include/opencv4)
  PHP_ADD_LIBRARY_WITH_PATH(opencv_world, /usr/local/opencv-4.5.1/lib64, IMAGE_RECOGNITION_SHARED_LIBADD)

  PHP_REQUIRE_CXX()
  AC_DEFINE(HAVE_IMAGE_RECOGNITION, 1, [ Have image_recognition support ])

  PHP_NEW_EXTENSION(image_recognition, image_recognition.cpp, $ext_shared, ,"-Wall")
  PHP_SUBST(IMAGE_RECOGNITION_SHARED_LIBADD)
fi
