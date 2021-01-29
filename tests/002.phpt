--TEST--
image_recognition_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('image_recognition')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = image_recognition_test1();

var_dump($ret);
?>
--EXPECT--
The extension image_recognition is loaded and working!
NULL
