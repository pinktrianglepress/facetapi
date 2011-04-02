<?php

/**
 * @file
 */

/**
 *
 */
function hook_facetapi_searcher_info() {
  return array(
    // The name of the searcher.
    'apachesolr_search' => array(
      // The adapter plugin used by this searcher.
      'adapter' => 'apachesolr',
      // This is usually the name of the index instance.
      'instance' => 'apachesolr',
      // The pase path to the admin settings page.
      'path' => 'admin/config/search/apachesolr',
    ),
  );
}

/**
 *
 */
function hook_facetapi_adapters() {
  return array(
    'apachesolr' => array(
      'handler' => array(
        'class' => 'FacetapiApachesolrFacetapiAdapter',
      ),
    ),
  );
}

/**
 *
 */
function hook_facetapi_query_types() {
  return array(
    // The name of the query type plugin.
    'apachesolr_term' => array(
      'handler' => array(
        // The name of the plugin class.
        'class' => 'FacetapiApachesolrTerm',
        // The adapter plugin used by this searcher.
        'adapter' => 'apachesolr',
      ),
    ),
  );
}

