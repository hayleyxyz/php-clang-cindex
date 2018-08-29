<?php

const CXChildVisit_Recurse = 2;

const CXTranslationUnit_DetailedPreprocessingRecord = 0x01;
const CXTranslationUnit_KeepGoing = 0x200;

$index = clang_createIndex(0, 0);
var_dump($index);

//$unit = clang_parseTranslationUnit($index);
$options = 0;
$unit = clang_parseTranslationUnit($index, realpath("../zircon/kernel/arch/x86/smp.cpp"), [], $options);
var_dump($unit, $options);

$cursor = clang_getTranslationUnitCursor($unit);
var_dump($cursor);

clang_visitChildren($cursor, function($cursor, $parentCursor) {
    $extent = clang_getCursorExtent($cursor);
    $start = clang_getRangeStart($extent);
    $end = clang_getRangeStart($extent);

    clang_getInstantiationLocation($start, $file, $line, $column, $offset);
    $rangeStart = [ 'file' => clang_getFileName($file), 'line' => $line, 'column' => $column, 'offset' => $offset ];
    clang_getInstantiationLocation($end, $file, $line, $column, $offset);
    $rangeEnd = [ 'file' => clang_getFileName($file), 'line' => $line, 'column' => $column, 'offset' => $offset ];

    $kind = clang_getCursorKind($cursor);

    var_export([
        'isDefinition' => clang_isCursorDefinition($cursor),
        'kind' => $kind,
        'kindSpelling' => clang_getCursorKindSpelling($kind),
        'cursorSpelling' => clang_getCursorSpelling($cursor),
        'cursorDisplayName' => clang_getCursorDisplayName($cursor),
        'rangeStart' => $rangeStart,
        'rangeEnd' => $rangeEnd,
    ]);

    return CXChildVisit_Recurse;
});
