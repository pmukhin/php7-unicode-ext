<?php

$emptyString = new UnicodeString('');
echo ($emptyString->empty() ? "empty" : "non empty") . PHP_EOL;
echo ($emptyString->nonEmpty() ? "non empty" : "empty") . PHP_EOL;
echo $emptyString->length() . PHP_EOL;

$uString = new UnicodeString('Hello world');
echo "len:   '" . (string)$uString->length() . " keeping " . $uString . PHP_EOL;
echo "lower: '" . $uString->toLowerCase() . " keeping " . $uString . PHP_EOL;
echo "upper: '" . $uString->toUpperCase() . " keeping " . $uString . PHP_EOL;
echo "title: '" . $uString->capitalize() . " keeping " . $uString . PHP_EOL;
echo "drop5: '" . $uString->drop(5) . " keeping " . $uString . PHP_EOL;