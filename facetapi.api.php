<?php

/**
 * @file
 */

/**
 * Defines the available searchers, or search pages.
 */
function hook_facetapi_searcher_info() {
  return array(
    // The name of the searcher.
    'apachesolr_search' => array(
      // The adapter plugin used by this searcher.
      'adapter' => 'apachesolr',
      // The type of content being indexed by the backend.
      'type' => 'node',
      // The pase path to the admin settings page.
      'path' => 'admin/config/search/apachesolr',
    ),
  );
}

/**
 * Defines the available adapters implemented by the backends.
 */
function hook_facetapi_adapters() {
  return array(
    // The plugin ID.
    'apachesolr' => array(
      // The class containing the plugin implementation.
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

