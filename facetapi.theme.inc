<?php
// $Id$

/**
 * @file
 * Theme functions for the Facet API module.
 */

/**
 * Themes facets displayed in a block.
 *
 * @param $items
 *   An array of items as returned by the facetapi_block_realm_render().
 * @param $searcher
 *   A string containing the machine readable name of the searcher module
 *   executing the search.
 * @return
 *   A string containing the themed list.
 */
function theme_facetapi_block($items) {
  $links = facetapi_links_get($items);
  // @todo Do we need a title?
  return theme('item_list', $links);
}

/**
 * Themes a facet link with an optional count.
 *
 * @param $text
 *   A string containing the facet title.
 * @param $path
 *   A string containing the Drupal path being linked to, usually $_GET['q'].
 * @param $options
 *   An associative array of additional options.
 * @param $count
 *   An optional integer containing the count of the facet item.
 * @return
 *   A string containing the themed link.
 * @see l()
 */
function theme_facetapi_link($text, $path, $options = array(), $count = 0) {
  if ($count) {
    $text .= ' '. theme('facetapi_count', $count);
  }
  $link = facetapi_l($text, $path, $options);
  return $link;
}

/**
 * Themes the count for the facet item.  For example, this number shows how many
 * results will be returned after clicking on a link.
 *
 * @param $count
 *   An integer containing the count.
 * @return
 *   The themes number.
 */
function theme_facetapi_count($count) {
  return sprintf(' (%d)', $count);
}

/**
 * Themes a facet link for a value that is currently being searched.
 *
 * @param $text
 *   A string containing the facet title.
 * @param $path
 *   A string containing the Drupal path being linked to, usually $_GET['q'].
 * @param $options
 *   An associative array of additional options.
 * @return
 *   A string containing the themed link.
 * @see l()
 */
function theme_facetapi_link_active($text, $path, $options = array()) {
  return facetapi_l(sprintf('(-) %s', $text), $path, $options);
}
