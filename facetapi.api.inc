<?php
// $Id$

/**
 * @file
 * Defines and gives example implementations of all Facet API hooks.
 */


function hook_facetapi_adapters() {
}


function hook_facetapi_adapters_alter(&$adapters) {
}


function hook_facetapi_realms() {
}


function hook_facetapi_realms_alter(array &$realms) {
}


function hook_facetapi_facets($searcher, $type) {
}


function hook_facetapi_facets_alter(array &$facets, $searcher, $type) {
}


function hook_facetapi_postrender_alter(&$rendered, $searcher, $type, $realm_name) {
}


function hook_facetapi_facets_export_alter(&$export, $searcher, $realm_name) {
}


function hook_facetapi_facets_import_alter(&$export, $searcher, $realm_name) {
}


function hook_facetapi_realms_export_alter(&$export, $searcher) {
}


function hook_facetapi_realms_import_alter(&$export, $searcher) {
}

