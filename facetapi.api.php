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

    $facets['my_field'] = array(
      // Machine readable name fo the facet, defaults to array key.
      'name' => 'my_field',
      // Human readable name of the facet as displayed in settings forms.
      'label' => t('My field'),
      // Brief description of the facet.
      'description' => t('My field index some content we can facet by.'),
      // The field name used by the actual index, defaults to the array key.
      'field' => 'my_field_index_field_name',
      // The key used when passing filter data through the query string,
      // defaults the the value in "field" above.
      'field alias' => isset($info['field']) ? $info['field'] : $facet_name,
      // The machine readable name of the Field API field this data is
      // associated with, FALSE if it is not associated with a field.
      'field api name' => FALSE,
      // The ID of the query type plugin the backend uses to execute the facet
      // query.
      'query type' => 'term',
      // The dependency plugins this facet supports.
      'dependency plugins' => array('role'),
      // The default widget plugin used if no plugin has been selected or the
      // one selected is not valid.
      'default widget' => FALSE,
      // A list of operators supported by the facet, defaults to "AND".
      'allowed operators' => array(FACETAPI_OPERATOR_AND => TRUE, FACETAPI_OPERATOR_OR => TRUE),
      // Whether or not missing facets are allowed.  Default to FALSE.
      'facet missing allowed' => FALSE,
      // Whether or not the facet supports the "minimum facet count" setting.
      // Defaults to FALSE.
      'facet mincount allowed' => FALSE,
      // If the order of the facets is controlled in the Facet API GUI, the
      // default weight of the facet.  Defaults to 0.
      'weight' => 0,
      // The map callback used to map the raw values returned by the index to
      // something human readable, defaults to FALSE.
      'map callback' => FALSE,
      // An array of options passed to the map callback.
      'map options' => array(),
      // A callback that maps the parent / child relationships of the facet
      // data, defaults to FALSE meaning the list is flat.
      'hierarchy callback' => FALSE,
      // In instances where facet data is not returned by the backend, provide a
      // list of values that can be used.
      'values callback' => FALSE,
      // For facets containing ranges, a callback returning the minimum value in
      // the index. Defaults to FALSE meaning the facet does not contain ranges.
      'min callback' => FALSE,
      // For facets containing ranges, a callback returning the maximum value in
      // the index. Defaults to FALSE meaning the facet does not contain ranges.
      'max callback' => FALSE,
      // An array of available sorts that are executed during display.
      'default sorts' => array(
        array('active', SORT_DESC),
        array('count', SORT_DESC),
        array('display', SORT_ASC),
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
