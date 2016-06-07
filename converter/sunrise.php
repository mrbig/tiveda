<?php

/**
 * Generates the sunrise/sunset table
 */
date_default_timezone_set('UTC');
$timestamp = new DateTime('2016-01-01 12:00:00');


$values = [];
for ($i=0; $i<365; $i++) {
    $day = ($timestamp->format('m')-1) * 30 + $timestamp->format('d') -1;

    $sunrise = date_sunrise($timestamp->getTimestamp(), SUNFUNCS_RET_STRING, 47.33, 19);
    $sunset  = date_sunset($timestamp->getTimestamp(), SUNFUNCS_RET_STRING, 47.33, 19);
    
    $timestamp->add(new DateInterval('P1D'));
    
    $values[$day] = [toInt($sunrise), toInt($sunset)];
}

// feb 29
$values[59] = $values[58];
ksort($values);

echo '{';
echo implode(", ", array_map(
    function ($v) {
        return '{' . implode(', ', $v) . '}';
    },
    $values
));
echo '}', "\n";
function toInt($str) {
    list($hour, $minute) = explode(':', $str);
    return $hour * 60 + $minute;
}