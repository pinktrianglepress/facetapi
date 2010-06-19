<?php
// $Id$

/**
 * @file
 * Defines and gives example implementations of all Facet API hooks.
 */

/**
 * Defines Facet API adapters that abstract various functionality of the search
 * backend.
 *
 * @return
 *   An associative array containing:
 *   - name: The machine readable name of the adapter, usually the name of the
 *     searcher module connecting to the backend (i.e. apachesolr_search,
 *     luceneapi_node).
 *     - class: The name of the adapter class, which must extend the
 *       FacetapiAdapter class.
 *     - type: The type of content being indexed by the backend the adapter is
 *       connected to.
 *     - file: Optionally specify the file containing the adapter.
 *     - file path: The path to the folder containing the file specified in
         "file". This defaults to the path to the module implementing the hook.
 *
 * @ingroup hooks
 */
function hook_facetapi_adapters() {
}

/**
 * Allows modules to alter the adapter definitions.
 *
 * @param &$adapters
 *   An associative array of all adapter definitions.
 *
 * @ingroup hooks
 */
function hook_facetapi_adapters_alter(&$adapters) {
}

/**
 * Defines realms that facets can be rendered in.
 *
 * A facet realm is a group of facets that are displayed in a similar fashion
 * and rendered through the same function.  For example, facets in the "block"
 * realm are rendered via the facetapi_realm_block_render() function. This
 * architecture allows us to define a single facet but display it in
 * different ways.
 *
 * @return
 *   An associative array containing:
 *   - name: The machine readable name of the realm.
 *     - title: Human readable name of the realm, i.e. label.
 *     - description: The realm description.
 *     - weight: The default weight of the realm, defaults to 0.
 *     - sortable: Whether or not the facet weights can be set in the Facet API
 *     - callback => The realm rendering function, defaults to
 *       "MODULE_realm_REALM_NAME_render".
 *     - file: Optionally specify the file containing the callback function.
 *     - file path: The path to the folder containing the file specified in
 *       "file". This defaults to the path to the module implementing the hook.
 *     - theme hook: Optionally specify a theme hook to pass the rendered facets
 *       to prior to being returned by facetapi_realm_remner().
 *
 * @ingroup hooks
 */
function hook_facetapi_realms() {
}

/**
 * Allows modules to alter the realm definitions.
 *
 * @param &$realms
 *   An associative array of all realm definitions.
 *
 * @ingroup hooks
 */
function hook_facetapi_realms_alter(array &$realms) {
}

/**
 * Defines facets.
 *
 * Facets are filters that can be used to refine a search.  Although most facets
 * are attached to a search field, some facets may be straight markup, such as
 * one that returns to the original search query.
 *
 * @param $searcher
 *   A string containing the machine readable name of the searcher module.
 * @param $type
 *   A string containing the type of content $searcher indexes.
 *
 * @return
 *   An associative array containing:
 *   - name: The machine readable name of the realm.
 *     - title: Human readable name of the realm, i.e. label. Defaults to
 *       "name".
 *     - description: The realm description.
 *     - field: The search field associated with the filter. Defaults to "name".
 *     - field alias: The variable name used in the query string as well as the
 *       name of the form element when the facet is rendered as a FAPI element.
 *       Defaults to "name".
 *     - query type: The type of query used to filter the search results when
 *       the facet is active, i.e. "range" or "wildcard", defaults to "term".
 *       Each adapter must implement hook_facetapi_query_TYPE_process() to add
 *       the facet filter to the search uery.
 *     - fetch type: Determines which adapter's fetch*() method is used to
 *       retrieve the facet data from the search backend. Defaults to whatever
 *       is passed in "query type".
 *       meaning that the detchDefault() method is used to retrieve the data.
 *     - form type: The form element type when rendering the facet as a for
 *       element i.e. "checkboxes", "select".
 *     - weight: The default weight of the facet when rendered.
 *     - map callback: Callback function that maps raw values stored in the =
 *       search backend to something human readable, for example user IDs to
 *       usernames.
 *     - hierarchy callback: Callback function that adds perent information to
 *       the facets. For example, the facetapi_taxonomy_hierarchy_callback()
 *       function maps term IDs to their parent IDs.
 *     - file: Optionally specify the file containing the callback functions.
 *     - file path: The path to the folder containing the file specified in
 *       "file". This defaults to the path to the module implementing the hook.
 *
 * @ingroup hooks
 */
function hook_facetapi_facets($searcher, $type) {
}

/**
 * Allows modules to alter the facet definitions.
 *
 * @param &$facets
 *   An array of facet definitions.
 * @param $searcher
 *   A string containing the machine readable name of the searcher module.
 * @param $type
 *   A string containing the type of content $searcher indexes.
 *
 * @ingroup hooks
 */
function hook_facetapi_facets_alter(array &$facets, $searcher, $type) {
}

/**
 *
 * @ingroup hooks
 */
function hook_facetapi_postrender_alter(&$rendered, $searcher, $type, $realm_name) {
}

/**
 *
 * @ingroup hooks
 */
function hook_facetapi_widget() {
}

/**
 *
 * @ingroup hooks
 */
function hook_facetapi_facets_export_alter(&$export, $searcher, $realm_name) {
}

/**
 *
 * @ingroup hooks
 */
function hook_facetapi_facets_import_alter(&$export, $searcher, $realm_name) {
}

/**
 *
 * @ingroup hooks
 */
function hook_facetapi_realms_export_alter(&$export, $searcher) {
}

/**
 *
 * @ingroup hooks
 */
function hook_facetapi_realms_import_alter(&$export, $searcher) {
}

