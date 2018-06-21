<?php

$myString = new UnicodeString('empty');
$myString->foreach('array_walk');

// $charArray = UnicodeString::empty()
//     ->concat(Rune::fromInt32(60))
//     ->concat(Rune::fromInt32(52))
//     ->toCharArray();

// $myArrayOfString = ['sdf', 'sdf', 'sdf'];

// $data = UnicodeString::empty()
//     ->join($myArrayOfString);

// $data = map();