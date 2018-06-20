# php7-unicode-ext [WIP]

This extension does not require `mbstring`.

## What's in guts

```c
struct unicode_string {
    const char *str;    // string
    int32 *ubuffer;     // int32 buffer
    size_t b_len;       // bytes length (not including terminal null)
    size_t u_len;       // characters length
};
```

## Examples
### Modification
```php
<?php

$uString = new UnicodeString('Привет, мир');
$uString
    ->toUpperCase() // ПРИВЕТ, МИР
    ->toLowerCase() // привет, мир
    ->drop(8)       // мир
    ->capitalize()  // Мир
    ->length();     // 3
```
### Examination
```php
<?php

$uString = new UnicodeString('Привет, мир');
if ($uString->startsWith('Привет') && $uString->endsWith('мир')) {
    // do your awesome things
}
if ($uString->empty()) {
    // won't ever happen
}
if ($uString->nonEmpty()) {
    // will definitely happen
}
```
### Decompostion
```php
<?php

$uString = new UnicodeString('Привет, мир');
$ints = $uString->map(function(Rune $rune) {
    return $rune->toInt();
});
print_r($ints); [
            1055, // П
            1088, // р
            1080, // и
            1074, // в
            1077, // е
            1090, // т
            32,   // SPACE
            1084, // м
            1080, // и
            1088, // р
        ];
```