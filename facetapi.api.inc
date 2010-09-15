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
function hook_facetapi_adapter_info() {
}

/**
 * Allows modules to alter the adapter definitions.
 *
 * @param &$adapters
 *   An associative array of all adapter definitions.
 *
 * @ingroup hooks
 */
function hook_facetapi_adapter_info_alter(&$adapters) {
}

/**
 * Defines realms that facets can be built in.
 *
 * A facet realm is a group of facets that are displayed in a similar fashion
 * and built through the same function.  For example, facets in the "block"
 * realm are built via the facetapi_realm_block_build() function. This
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
 *     - callback => The realm building function, defaults to
 *       "MODULE_realm_REALM_NAME_build".
 *     - file: Optionally specify the file containing the callback function.
 *     - file path: The path to the folder containing the file specified in
 *       "file". This defaults to the path to the module implementing the hook.
 *
 * @ingroup hooks
 */
function hook_facetapi_realm_info() {
}

/**
 * Allows modules to alter the realm definitions.
 *
 * @param &$realms
 *   An associative array of all realm definitions.
 *
 * @ingroup hooks
 */
function hook_facetapi_realm_info_alter(array &$realms) {
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
 *       name of the form element when the facet is built as a FAPI element.
 *       Defaults to "name".
 *     - query type: The type of query used to filter the search results when
 *       the facet is active, i.e. "range" or "wildcard", defaults to "term".
 *       Each adapter must implement hook_facetapi_query_TYPE_process() to add
 *       the facet filter to the search uery.
 *     - fetch type: Determines which adapter's fetch*() method is used to
 *       retrieve the facet data from the search backend. Defaults to whatever
 *       is passed in "query type".
 *       meaning that the detchDefault() method is used to retrieve the data.
 *     - form type: The form element type when building the facet as a for
 *       element i.e. "checkboxes", "select".
 *     - weight: The default weight of the facet when built.
 *     - map callback: Callback function that maps raw values stored in the =
 *       search backend to something human readable, for example user IDs to
 *       usernames.
 *     - hierarchy callback: Callback function that adds perent information to
 *       the facets. For example, the facetapi_taxonomy_hierarchy_callback()
 *       function maps term IDs to their parent IDs.
 *     - values callabck: Callback function that returns the values for a field.
 *       This callback is useful for returning form options when facets are
 *       built as select boxes, radios, etc.
 *     - default range callabck: Callback function that returns the default
 *       range values for a field.
 *     - file: Optionally specify the file containing the callback functions.
 *     - file path: The path to the folder containing the file specified in
 *       "file". This defaults to the path to the module implementing the hook.
 *
 * @ingroup hooks
 */
function hook_facetapi_facet_info($searcher, $type) {
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
function hook_facetapi_facet_info_alter(array &$facets, $searcher, $type) {
}

/**
 * Allows modules to alter the built facets prior to being returned by
 * facetapi_realm_build().
 *
 * @ingroup hooks
 */
function hook_facetapi_facets_alter(&$built, $searcher, $type, $realm_name) {
}

/**
 * Defines display widgets the block realm uses to theme facets.
 *
 * @return
 *   An associative array containing:
 *   - name: The machine readable name of the widget.
 *     - title: Human readable name of the widget.
 *     - callback: Callback function used to theme the facets.
 *     - realms: An array of realms the widget applies to.
 *     - query types: An array of query types the widget applies to.
 *     - weight: The weight of the widget when displayed in forms.
 *
 * @ingroup hooks
 */
function hook_facetapi_widget_info() {
}

/**
 * Allows modules to alter the widget definitions.
 *
 * @param &$widgets
 *   An associative array of widget definitions.
 *
 * @ingroup hooks
 */
function hook_facetapi_widget_info_alter() {
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

