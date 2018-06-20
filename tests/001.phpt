--TEST--
Check if unicode is loaded
--SKIPIF--
<?php
if (!extension_loaded('unicode')) {
	echo 'skip';
}
?>
--FILE--
<?php 

$uString = new UnicodeString('Hello world');
echo (string)$uString->length() . PHP_EOL;
echo $uString->toLower() . PHP_EOL;
echo $uString->toUpper() . PHP_EOL;
echo $uString->substring(0, 3) . PHP_EOL;
?>
--EXPECT--
11
hello world
HELLO WORLD
Hel
