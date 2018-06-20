# php7-unicode-ext

## Examples
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