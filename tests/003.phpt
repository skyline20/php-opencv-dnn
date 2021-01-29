--TEST--
image_recognition_test2() Basic test
--SKIPIF--
<?php
if (!extension_loaded('image_recognition')) {
	echo 'skip';
}
?>
--FILE--
<?php
var_dump(image_recognition_test2());
var_dump(image_recognition_test2('PHP'));
?>
--EXPECT--
string(11) "Hello World"
string(9) "Hello PHP"
