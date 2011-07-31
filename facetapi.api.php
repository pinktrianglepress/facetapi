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
 *
 */
function hook_facetapi_searcher_info_alter(array &$searcher_info) {

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
 * Allows backends to handle query types, such as term queries or date queries.
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

/**
 * Defines display widgets.
 */
function hook_facetapi_widgets() {
  return array(
    'facetapi_links' => array(
      'handler' => array(
        'label' => t('Links'),
        'class' => 'FacetapiWidgetLinks',
      ),
    ),
  );
}

/**
 * Defines facet filters.
 */
function facetapi_facetapi_filters() {
  return array(
    'active_items' => array(
      'handler' => array(
        'label' => t('Do not display active items'),
        'class' => 'FacetapiFilterActiveItems',
      ),
    ),
  );
}

/**
 * Defines facet realms.
 */
function hook_facetapi_realm_info() {

}

/**
 * Alter realm definitions.
 */
function hook_facetapi_realm_info_alter(array &$realm_info) {

}

/**
 * Defines available facets.
 */
function hook_facetapi_facet_info(array $searcher_info) {
  $facets = array();
  if ('node' == $searcher_info['type']) {

    $facets['created'] = array(
      'label' => t('Post date'),
      'description' => t('Filter by the date the node was posted.'),
      'field' => 'created',
      'field alias' => 'created',
      'query type' => 'date',
      'allowed operators' => array(FACETAPI_OPERATOR_AND => TRUE),
      'weight' => 0,
      'values callback' => FALSE,
      'map callback' => 'facetapi_map_date',
      'min callback' => 'facetapi_get_min_date',
      'max callback' => 'facetapi_get_max_date',
      'default sorts' => array(
        array('active', SORT_DESC),
        array('indexed', SORT_ASC),
      ),
    );
  }

  return $facets;
}

/**
 * Alter facet definitions.
 */
function hook_facetapi_facet_info_alter(array &$facet_info, array $searcher_info) {

}

/**
 * Define sorting algorithms for facets.
 */
function hook_facetapi_sort_info() {

}

/**
 * Alter sorting algorithms.
 */
function hook_facetapi_sort_info_alter(array &$sort_info) {

}

/**
 * Forces delta mapping of a facet block.
 *
 * This obscure hook is useful for cases where facets are disabled, but their
 * block positioning needs to be set anyways. If a facet is enabled via the
 * facetapi_set_facet_enabled() API function, its block needs to be enabled
 * and assigned to a region despite the facet not being enabled in the Facet API
 * interface, which would normally prevent the block from being listed.
 */
function hook_facetapi_force_delta_mapping() {
  return array(
    // The machine-readable name of the searcher.
    'my_searcher' => array(
      // The realm we are mapping, usually block.
      'block' => array(
        // Machine readable names of facets whose mappping is forced. Regardless
        // of whether they are enabled via the Facet API interface, their blocks
        // will be available to enable and position via admin/structure/block.
        'facet_one',
        'facet_two',
      ),
    ),
  );
}
