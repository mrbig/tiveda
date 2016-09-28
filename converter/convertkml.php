<?php

if (count($_SERVER['argv']) != 3 || !is_readable($_SERVER['argv'][1]) || (file_exists($_SERVER['argv'][2]) && !is_writable($_SERVER['argv'][2]))) {
    die("Usage:\n\tconvertkml.php [filename] [outputfile]\n");
}

$xml = simplexml_load_file($_SERVER['argv'][1]);


$pois = [];
foreach ($xml->Document->Folder as $folder) {


    // First collect all the polygons
    $folder_pois = [];
    foreach ($folder->Placemark as $place) {

        if (empty($place->Polygon)) continue;
        
        $poi = parsePoly($place);
        if (isset($folder_pois[$poi['n']])) {
            die('Error: duplicate poi name '.$poi['n']."\n");
        }
        $folder_pois[$poi['n']] = $poi;
    }

    // Now collect, and add the directionial lines
    foreach ($folder->Placemark as $place) {
        if (empty($place->LineString)) continue;
        
        $line = parseLine($place);
        if (!$line) continue;
        
        if (isset($folder_pois[$line['name']])) {
            $folder_pois[$line['name']]['h'] = $line['hdg'];
        } else {
            logError('WARNING: line named "'. $line['name']. "\" has no matching polygon\n");
        }
    }
    
    $pois = array_merge($pois, array_values($folder_pois));
}


var_dump($pois);

//$splitters = findSplitters($pois);

$output = fopen($_SERVER['argv'][2], 'w');
writePois($output, $pois);
fclose($output);
echo 'done', "\n";

$version = time() - strtotime('2000-01-01 00:00:00');

$output = fopen(dirname($_SERVER['argv'][2]).'/version.dat', 'w');
fwrite($output, pack('L', $version));
fclose($output);

echo 'New version is: 0x', dechex($version), "\n";

exit();

/**
 * Parsing a Polygon placemark into a poit
 * @param object the placemark object from KML
 * @return array poi description
 */
function parsePoly($place) {
        $poi = [
            'n' => (string)$place->name,
        ];
        if (preg_match('/limit: ([0-9]+)/', $place->description, $regs)) {
            $poi['l'] = (int)$regs[1];
        } else {
            logError('WARNING: No limit found in place '.$place->name. "\n");
        }
        
        $poly = [];
        foreach (explode(' ', trim($place->Polygon->outerBoundaryIs->LinearRing->coordinates)) as $coords) {
            list($lng, $lat) = explode(',', $coords);
            $poly[] = [
                'lng' => (float)$lng,
                'lat' => (float)$lat,
            ];
        }
        
        $lines = [];
        
        // Last point should repeat the first one
        for ($i=0; $i<count($poly) - 1; $i++) {
            $j = $i+1;
            
            // Converting the coordinates into line equation constants
            $line = [
                'lng1' => $poly[$i]['lng'],
                'lng2' => $poly[$j]['lng'],
            ];
            
            $line['m'] = ($poly[$j]['lat']-$poly[$i]['lat']) / ($poly[$j]['lng'] - $poly[$i]['lng']);
            
            $line['c'] = $poly[$i]['lat'] - $poly[$i]['lng'] * $line['m'];
            
            $lines[] = $line;
        }
        
        $poi['edges'] = $lines;
        
        return $poi;
}

/**
 * Parsing a line placemark into name and heading values
 * @param object the placemark object from KML
 * @return array name and heading of the line
 */
function parseLine($place) {
    $coords = explode(' ', trim($place->LineString->coordinates));
    if (count($coords) != 2) return null; // handling only two point vectors
    
    list($start, $end) = array_map(function($c) {list($lng, $lat) = explode(',', $c); return ['lng' => $lng, 'lat' => $lat];}, $coords);
    
    
    $hdg = atan2($end['lng'] - $start['lng'], $end['lat']-$start['lat']) * 180 / M_PI;
    if ($hdg < 0) $hdg += 360;
    
    return ['name' => (string)$place->name, 'hdg' => $hdg];
}


/**
 * Write the pois into the output file
 * @param resource the output file handle
 * @param array list of pois
 */
function writePois($output, $pois) {
    fwrite($output, 'POIS'); // Magic string
    fwrite($output, pack('v', count($pois)));
    
    foreach ($pois as $poi) {
        writePoi($output, $poi);
    }
}

/**
 * Write a single poi into the output file
 * @param resource the output file handle
 * @param array the poi definition
 */
function writePoi($output, $poi) {
    // Write poi header
    fwrite($output,
        pack('vvf',
            $poi['l'],
            count($poi['edges']),
            isset($poi['h']) ? $poi['h'] : -1
        )
    );
    
    // Write the edges
    foreach ($poi['edges'] as $edge) {
        fwrite($output,
            pack('f*',
                $edge['lng1'],
                $edge['lng2'],
                $edge['m'],
                $edge['c']
            )
        );
    }
}

/**
 * Find the longitudes that split the pois into equals sized areas
 * @param array list of pois
 * @return array longitudes for the splitter lines
 */
function findSplitters($pois) {
    $intervals = findIntervals($pois);
    var_dump($intervals);
    
    var_dump(splitIntervals($intervals, 3));
}

/**
 * Split the intervals into two, mostly equal parts
 * @param array list of intervals
 * @return array splitters: list of splitters
 */
function splitIntervals($intervals, $level) {
    // find the best place to split
    $split = $splitkey = null; $mindiff = 1000;
    foreach ($intervals as $key => $interval) {
        $diff = abs($interval['eCount'] - $interval['wCount']);
        if ($diff < $mindiff) {
            $split = $interval;
            $splitkey = $key;
            $mindiff = $diff;
        }
    }
    
    $splitter = ($split['west'] + $split['east']) / 2;
    
    $west = array_map(
        function ($item) use ($split) {
            $item['eCount'] -= $split['eCount'];
            return $item;
        },
        array_slice($intervals, 0, $splitkey)
    );
    $east = array_map(
        function ($item) use ($split) {
            $item['wCount'] -= $split['wCount'];
            return $item;
        },
        array_slice($intervals, $splitkey + 1)
    );
    
    $ret = [];
    if ($level && $west) {
        $ret = splitIntervals($west, $level - 1);
    }
    $ret[] = $splitter;
    if ($level && $east) {
        $ret = array_merge($ret, splitIntervals($east, $level -1));
    }
    
    return $ret;
}

/**
 * Calculate the intervals between pois. Each interval should
 * be at 0.1 grad wide
 * @param array list of pois (polygons)
 * @return array list of intervals
 */
function findIntervals($pois) {
    $pois = calculateBorders($pois);
    
    usort($pois, function($a, $b) {
        if ($a['west'] == $b['west']) return 0;
        return $a['west'] < $b['west'] ? -1 : 1;
    });
    var_dump($pois);
    
    $intervals = [];
    for ($i=1; $i<count($pois); $i++) {
        if ($pois[$i]['west'] < $pois[$i-1]['east']) continue; // overlapping intervals
        
        if ($pois[$i]['west'] - $pois[$i-1]['east'] < 0.1) {
            echo 'not enough gap: ', $i-1, ' => ', $i,': ',($pois[$i]['west'] - $pois[$i-1]['east']) *75000, "\n";
            continue;
        }
        
        $intervals[] = [
            'west' => $pois[$i-1]['east'],
            'east' => $pois[$i]['west'],
            'wCount' => $i,
            'eCount' => count($pois) - $i,
        ];
        
    }
    return $intervals;
}

/**
 * Calculate the borders (east-west) of each poi, and set them
 * in west, east properties
 * @param array list of pois
 * @return array list of pois with the borders calculated
 */
function calculateBorders($pois) {
    foreach ($pois as &$poi) {
        $res = array_reduce(
            $poi['edges'],
            function($carry, $edge) {
                $carry['west'] = min($carry['west'], $edge['lng1'], $edge['lng2']);
                $carry['east'] = max($carry['east'], $edge['lng1'], $edge['lng2']);
                return $carry;
            },
            ['west' => 24, 'east' => 15]
        );
        
        $poi['west'] = $res['west'];
        $poi['east'] = $res['east'];
    }
    
    return $pois;
}


/**
 * Output error message to the stderr
 * @param string the error message
 */
function logError($error) {
    $stderr = fopen('php://stderr', 'w');
    fwrite($stderr, $error);
    fclose($stderr);
}
