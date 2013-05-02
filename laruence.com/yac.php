<?php

// MurmurHash : 
// 双散列法 : 
// 共享内存 : 
// 
$yac = new Yac();
 
for ($i = 0; $i<1000; $i++) {
    $key =  "xxx" . rand(1, 10000);
    $value = str_repeat("x", rand(1, 10000));
 
    if (!$yac->set($key, $value)) {
        var_dump("write " . $i);
    }
 
    if ($value != ($new = $yac->get($key))) {
        var_dump("read " . $i);
    }
}
 
var_dump($i);
