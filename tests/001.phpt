--TEST--
Check if image_recognition is loaded
--SKIPIF--
<?php
if (!extension_loaded('image_recognition')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "image_recognition" is available';
?>
--EXPECT--
The extension "image_recognition" is available
