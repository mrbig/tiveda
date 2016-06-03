<?php

$fileSketch = 'tiveda.ino.nodemcu.bin';
$fileSpiffs = 'tiveda.spiffs.bin';
$version = "1.2";
$mapVersion = 0x1edba629;

file_put_contents('/tmp/esp_debug.txt', print_r($_SERVER, true));

if (
    !isset($_SERVER['HTTP_X_ESP8266_VERSION']) ||
    empty($_SERVER['HTTP_X_ESP8266_MODE']) ||
    empty($_GET['id'])
)
{
    header($_SERVER["SERVER_PROTOCOL"].' 403 Forbidden', true, 403);
    echo "only for tiveda updater!\n";
    exit();
            
}


if ($_SERVER['HTTP_X_ESP8266_MODE'] == 'spiffs') {
    $file = $fileSpiffs;
    if ($mapVersion <= hexdec($_SERVER['HTTP_X_ESP8266_VERSION']) || !is_readable($file)) {
        header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
        exit();
    }
} else {
    $file = $fileSketch;
    if (version_compare($_SERVER['HTTP_X_ESP8266_VERSION'], $version) >=0 || !is_readable($file)) {
        header($_SERVER["SERVER_PROTOCOL"].' 304 Not Modified', true, 304);
        exit();
    }

}

header('Content-Type: application/octet-stream');
header('Content-Disposition: attachment; filename='.basename($file));
header('Content-Length: '.filesize($file));
header('x-MD5: '.md5_file($file));

readfile($file);
