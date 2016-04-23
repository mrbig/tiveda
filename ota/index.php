<?php

$file = 'tiveda.ino.nodemcu.bin';
$version = "1.1";

file_put_contents('/tmp/esp_debug.txt', print_r($_SERVER, true));

if (
    empty($_SERVER['HTTP_X_ESP8266_VERSION']) ||
    empty($_GET['id'])
)
{
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for tiveda updater!\n";
    exit();
            
}


if (version_compare($_SERVER['HTTP_X_ESP8266_VERSION'], $version) >=0 || !is_readable($file)) {
    header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
    exit();
}

header('Content-Type: application/octet-stream');
header('Content-Disposition: attachment; filename='.basename($file));
header('Content-Length: '.filesize($file));
header('x-MD5: '.md5_file($file));

readfile($file);
