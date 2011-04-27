<?php

/**
 * @file
 * Block realm code and hook implementations.
 */

/**
 * Returns block information.
 */
function facetapi_get_block_info($realm_name = 'block') {
  $blocks = array();

  // Gets delta map, iterates over all enabled facets.
  $map = facetapi_get_delta_map();
  foreach (facetapi_get_searcher_info() as $searcher => $info) {
    $facets = facetapi_get_enabled_facets($searcher, $realm_name);
    foreach ($facets as $facet_name => $facet) {

      // Gets the delta from the delta map.
      $string = $searcher . ':' . $realm_name .':' . $facet_name;
      $delta = array_search($string, $map);

      // Defines the block.
      // @todo Explore more efficient caching options.
      $blocks[$delta] = array(
        'info' => 'Facet API: ' . $info['label'] . ' : ' . $facet['label'],
        //'cache' => BLOCK_NO_CACHE,
      );
    }
  }

  // Returns available blocks.
  return $blocks;
}

/**
 * Returns the content for a facet based on the delta.
 */
function facetapi_get_block_content($delta) {
  $builds = &drupal_static(__FUNCTION__, array());

  // Bails if delta isn't mapped.
  $map = facetapi_get_delta_map();
  if (!isset($map[$delta])) {
    return;
  }
  // Extracts the searcher, realm name, and facet name from $delta.
  // Process the parts from the end in case the searcher includes a ':'.
  $parts = explode(':', $map[$delta]);
  $facet_name = array_pop($parts);
  $realm_name = array_pop($parts);
  $searcher = implode(':', $parts);

  $group = $searcher . ':' . $realm_name;

  // Bails if output is suppressed, such as if a search hasn't been executed.
  if ((!$adapter = facetapi_adapter_load($searcher)) || $adapter->suppressOutput($realm_name)) {
    return;
  }

  // Builds and caches the entire realm per searcher / realm combination.
  if (!isset($builds[$group])) {
    $builds[$group] = facetapi_build_realm($searcher, $realm_name);
  }

  // Returns the individual block.
  if (isset($builds[$group][$facet_name])) {
    // Add a contextual link to edit this facet.
    $builds[$group][$facet_name]['#contextual_links'] = array(
      'facetapi' => array("admin/config/search/facetapi/$searcher/$realm_name/$facet_name/edit", array()),
    );
    return $builds[$group][$facet_name];
  }
}

/**
 * Returns a cached delta map of hashes to names.
 *
 * Sometimes our deltas are longer than 32 chars and need to be passed to hash().
 * Due to the block table's schema, deltas cannot be longer than 32 characters.
 * However, hashes are nasty as CSS IDs, so we can use the map to convert
 * the hashes back to a nicer value in facetapi_preprocess_block().
 *
 * @return
 *   An array containing the delta map.
 */
function facetapi_get_delta_map() {
  $map = &drupal_static(__FUNCTION__);
  if (NULL === $map) {
    if ($data = cache_get('facetapi:delta_map')) {
      $map = $data->data;
    }
    else {
      $map = array();

      // Calculates deltas for facets in the block realm.
      foreach (facetapi_get_searcher_info() as $searcher => $info) {
        // @todo - some other way to define realms that are block-like.
        foreach (array('block') as $realm_name) {
          foreach (facetapi_get_enabled_facets($searcher, $realm_name) as $facet) {
            $string = $searcher . ':' . $realm_name . ':' . $facet['name'];
            // NOTE: We don't need drupal_strlen() becuase there will be no UTF8
            // characters in this string, and strlen() is much faster.
            // @todo Is this a bad assumption?
            $key = (strlen($string) <= 32) ? $string : substr(base64_encode(hash('sha256', $string, TRUE)), 0, 32);
            $map[$key] = $string;
          }
        }
      }

      // Caches the map so we don't have to do this repeatedly.
      cache_set('facetapi:delta_map', $map, 'cache', CACHE_TEMPORARY);
    }
  }

  return $map;
}
