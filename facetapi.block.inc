<?php
// $Id$

/**
 * @file
 * Functions related to the block realm.
 */

/**
 * Renders facets as links.
 *
 * @param $facet_items
 *   An array containing the facet items.
 * @param $searcher
 *   A string containing the machine readable name of the searcher module.
 * @param $realm_name
 *   A string containing the machine readable name of the realm.
 * @param $facet_name
 *   A string containing the machine readable name of the facet.
 * @param $widget
 *   An optional string containig the widget name. This is useful for other
 *   widgets that render the facets through this function, such as the
 *   "facetapi_checkboxes" widget.
 *
 * @return
 *   A string containig the themed facet.
 */
function facetapi_links_widget_callback(array $facet_items, $searcher, $realm_name, $facet_name, $widget = NULL) {
  // Gets the facet's soft limit, adds to javascript variables.
  $limit = facetapi_setting_get('soft_limit', $searcher, $realm_name, $facet_name);
  if (NULL === $limit) {
    $limit = 10;
  }
  $settings['facetapi'][$searcher][$realm_name][$facet_name] = array(
    'limit' => $limit,
    'widget' => (NULL === $widget) ? 'facetapi_links' : $widget,
  );

  // Adds settings as javascript variables.
  drupal_add_js($settings, 'setting');

  // Builds links, themes links as an item list.
  $links = facetapi_links_get($facet_items);
  return theme('facetapi_block', $links);
}

/**
 * Renders facets as links with checkboxes, useful for facets using the OR
 * operator.
 *
 * @param $facet_items
 *   An array containing the facet items.
 * @param $searcher
 *   A string containing the machine readable name of the searcher module.
 * @param $realm_name
 *   A string containing the machine readable name of the realm.
 * @param $facet_name
 *   A string containing the machine readable name of the facet.
 *
 * @return
 *   A string containig the themed facet.
 */
function facetapi_checkboxes_widget_callback(array $facet_items, $searcher, $realm_name, $facet_name) {
  array_walk($facet_items, 'facetapi_active_checkbox_links_replace');
  return facetapi_links_widget_callback(
    $facet_items, $searcher, $realm_name, $facet_name, 'facetapi_checkboxes'
  );
}

/**
 * Replaces active links when being displayed via the "facetapi_checkboxes"
 * widget.  Useful as an array_walk() callback.
 *
 * @param &$item
 *   An array containing the facet item.
 * @param $key
 *   The array key of the item being modified.
 */
function facetapi_active_checkbox_links_replace(array &$item, $key) {
  // Overrides the default "active" link.
  if ('facetapi_link_active' == $item['theme hook']) {
    $item['theme hook'] = 'facetapi_link';
  }
  // Replaces links for children as well.
  if (!empty($item['children'])) {
    array_walk($item['children'], 'facetapi_active_checkbox_links_replace');
  }
}

/**
 * Returns data for the "list" operation of hook_block().
 *
 * @return
 *   An array of block information.
 */
function facetapi_block_info() {
  $blocks = array();

  // Gets "delta map", iterates over available searcher modules.
  $map = facetapi_delta_map_get();
  foreach (facetapi_adapter_definitions_get() as $searcher => $definition) {
    $facets = facetapi_enabled_facets_get($searcher, 'block');
    foreach ($facets as $facet_name => $facet) {

      // Gets the delta from the delta map.
      $string = $searcher .':block:'. $facet_name;
      $delta = array_search($string, $map);

      // Defines the block.
      // @todo explore more efficient caching options.
      $blocks[$delta] = array(
        'info' => sprintf('Facet API: %s: %s', $searcher, $facet_name),
        'cache' => BLOCK_NO_CACHE, // FOR DEVELOPMENT!!!
      );
    }
  }

  // Returns available blocks.
  return $blocks;
}

/**
 * Returns data for the "view" operation of hook_block().
 *
 * @param $delta
 *   A string containing the identifier for the block.
 *
 * @return
 *   An array containing the block subject and content.
 */
function facetapi_block_view($delta = '') {
  static $blocks;
  $add_js = FALSE;

  // Bails if delta isn't mapped.
  $map = facetapi_delta_map_get();
  if (!isset($map[$delta])) {
    return array();
  }

  // Extracts the searcher, realm name, and facet name from $delta. Caches
  // rendered blocks by group (searcher - realm name), because we cannot
  // render facets individually, only the realm as a whole.
  list($searcher, $realm_name, $facet_name) = explode(':', $map[$delta]);
  $group = $searcher .':'. $realm_name;
  if (!isset($blocks[$group])) {
    $settings = array('facetapi' => array());

    // @todo We shouldn't have to load the adapter.  There needs to be a
    // generic API function that flags which searches were executed.
    if (facetapi_adapter_load($searcher)->searchExecuted()) {
      $widgets = facetapi_widgets_get();

      // Renders the facet via facetapi_realm_render().
      $data = facetapi_realm_render($searcher, $realm_name);
      foreach ($data as $facet_data) {
        if (!empty($facet_data['items'])) {
          $add_js = TRUE;

          // Determines which widget will render the realm.
          $widget = facetapi_setting_get('widget', $searcher, $realm_name, $facet_data['name']);
          if (NULL !== $widget && isset($widgets[$widget])) {
            $callback = $widgets[$widget]['callback'];
          }
          else {
            $callback = 'facetapi_links_widget_callback';
          }

          // Builds the facet links, passes to the widget callback
          $blocks[$group][$facet_data['name']] = array(
            'subject' => $facet_data['title'],
            'content' => $callback($facet_data['items'], $searcher, $realm_name, $facet_data['name'])
          );
        }
      }

      // Adds javascript if at least one facet is rendered.
      if ($add_js) {
        drupal_add_js(drupal_get_path('module', 'facetapi') .'/facetapi.js');
      }
    }
    else {
      $blocks[$group] = array();
    }
  }

  // Returns the individual block.
  return isset($blocks[$group][$facet_name]) ? $blocks[$group][$facet_name] : array();
}

/**
 * Returns a "delta map", because sometimes our deltas are longer than 32 chars
 * and need to be passed to md5(). Due to the block table's schema, deltas
 * cannot be longer than 32 characters.  However, md5 hashes are nasty as CSS
 * IDs, so we can use the map to convert the md5 back to a nicer value in
 * facetapi_preprocess_block().
 *
 * @param $reset
 *   A boolean flagging whether the static should be reset.
 *
 * @return
 *   An array containing the delta map.
 */
function facetapi_delta_map_get($reset = FALSE) {
  static $map;
  if (NULL === $map || $reset) {
    if ($data = cache_get('facetapi:delta_map')) {
      $map = $data->data;
    }
    else {
      $map = array();

      // Calculates deltas for each facet in each realms for each searcher.
      foreach (facetapi_adapter_definitions_get() as $searcher => $definition) {
        foreach (facetapi_realms_get() as $realm) {
          foreach (facetapi_enabled_facets_get($searcher, $realm['name']) as $facet) {
            $string = $searcher .':'. $realm['name'] .':'. $facet['name'];
            // NOTE: We don't need drupal_strlen() becuase there will be no UTF8
            // characters in this string, and strlen() is much faster.
            $key = (strlen($string) <= 32) ? $string : md5($string);
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

/**
 * Renders facets in the block realm.
 *
 * @param $adapter
 *   A FacetapiAdapter object extended by the searcher module.
 * @param $realm_name
 *   A string containing the machine readable name of the realm.
 * @param $facets
 *   An array containing the full facet definitions.
 *
 * @return
 *   A structured array that can be passed to a theming function.
 */
function facetapi_realm_block_render(FacetapiAdapter $adapter, $realm_name, array $facets) {

  // @todo Get cached data here???
  // cache_get();

  // Initializes breadcrumb trail.
  $breadcrumb = array();
  if ($keys = $adapter->getSearchKeys()) {
    $breadcrumb[] = l($keys, $_GET['q']);
  }

  // Gets facet counts, iterates over collection.
  $data = $adapter->getFacetData($realm_name, $facets);
  foreach ($data as &$facet_data) {

    // Maps the IDs to human readable values via the mapping callback.
    if (!empty($facet_data['map callback'])) {
      $map = call_user_func($facet_data['map callback'], array_keys($facet_data['items']));
      array_walk($facet_data['items'], 'facetapi_ids_replace', $map);
    }

    // Adds breadcrumb items.
    foreach ($adapter->getActiveFacets($facet_data['field alias']) as $value => $filter) {
      $breadcrumb[] = l($facet_data['items'][$value]['text'], $_GET['q']);
    }

    // Builds hierarchical relationships.
    facetapi_hierarchy_process($facet_data['items'], $facets[$facet_data['name']]);
  }
  unset($facet_data);


  // Finalizes breadcrumb trail and sets.
  if (!empty($breadcrumb)) {
    $last = count($breadcrumb) - 1;
    $breadcrumb[$last] = strip_tags($breadcrumb[$last]);
    drupal_set_breadcrumb(array_merge(menu_get_active_breadcrumb(), $breadcrumb));
  }

  // @todo Set cached data here??
  // cache_set();

  // Builds query string for each item. We don't want to cache query string data
  // otherwise the sort parameters won't persist.
  // @see http://drupal.org/node/664440
  foreach ($data as &$facet_data) {
    facetapi_paths_set($facet_data['items'], $adapter, $realm_name, $facets[$facet_data['name']]);
  }
  unset($facet_data);

  // Returns the rendered facet data.
  return $data;
}

/**
 * Replaces ID's with a mapped value, useful as a callback for array_walk().
 *
 * @param &$item
 *   An array containing the facet item.
 * @param $key
 *   An integer containing the array key, or the ID being mapped to a value.
 * @param $map
 *   An array containing the mapped values.
 */
function facetapi_ids_replace(array &$item, $key, array $map) {
  if (isset($map[$key])) {
    $item['text'] = $map[$key];
  }
}

/**
 * Recursive function that sets the link paths for all facets items and their
 * descendants. In addition, this function sorts the facet list by the
 * parameters set in the facet's administrative settings page.
 *
 * @param &$facet_items
 *   An array of facet items whose path is being set.
 * @param $adapter
 *   A FacetapiAdapter object extended by the searcher module.
 * @param $realm_name
 *   A string containing the machine readable name of the realm.
 * @param $facet
 *   An array containing the full facet definition.
 */
function facetapi_paths_set(array &$facet_items, FacetapiAdapter $adapter, $realm_name, array $facet) {

  // Iterates over items, builds link paths.
  foreach ($facet_items as $value => &$item) {
    $values = array($value);

    // Sets current page as the link's path.
    $item['path'] = $_GET['q'];

    // If there are active children, builds links for decendents.
    if ($item['active']) {
      if (!empty($item['children'])) {
        $active_children = facetapi_paths_set($item['children'], $adapter, $realm_name, $facet);
        // Make sure unclicking the parent deactivates the children as well.
        $values = array_merge(facetapi_child_values_get($item['children']), $values);
      }
    }

    // Formats query string dependent on whether the facet is active.
    $item['options']['query'] = drupal_query_string_encode(
      $adapter->getFacetQueryString($facet, $values, $item['active']),
      array('q', 'page')
    );
  }
  unset($item);

  // Gets the facet's sort settings, parses into an array.
  // @todo statically cache the settings for performance?
  $sort_array = array_filter(explode(':', facetapi_setting_get(
    'sort', $adapter->getSearcher(), $realm_name, $facet['name']
  )));

  // This logic prevetns an "undefined index" notice when NULL is returned as
  // the setting value (in other words it hasn't bee set yet).
  list($sort, $flow) = (2 == count($sort_array)) ? $sort_array : array(0, 0);

  // Sorts the facets by text or key, calls adapters sortDefault() hook if
  // no sorting parameters are defined.
  switch ($sort) {
    case SORT_STRING:
      $cmp_function = (SORT_DESC != $flow) ? 'facetapi_sort_text' : 'facetapi_rsort_text';
      uasort($facet_items, $cmp_function);
      break;

    case SORT_NUMERIC:
      $sort_function = (SORT_ASC == $flow) ? 'ksort' : 'krsort';
      $sort_function($facet_items);
      break;

    default:
      $adapter->sortDefault($facet_items);
      break;
  }
}

/**
 * Recursive function that returns an array of values for all descendants of a
 * facet item.
 *
 * @param $facet_items
 *   An array containing the child facet items.
 *
 * @return
 *   An array containing the values of all descendants.
 */
function facetapi_child_values_get(array $facet_items) {
  $values = array_keys($facet_items);
  foreach ($facet_items as $item) {
    if (!empty($item['children'])) {
      $values = array_merge(facetapi_child_values_get($item['children']), $values);
    }
  }
  return $values;
}

/**
 * Builds a list of links that can be passed to theme('item_list').
 *
 * @param $facet_items
 *   An array containing the facet items.
 *
 * @return
 *  An array containing the hierarchical links that can be passed directly to
 *  theme('item_list').
 *
 * @see theme_item_list()
 */
function facetapi_links_get(array $facet_items) {
  $links = array();
  foreach ($facet_items as $value => $item) {
    $link = array();

    // We don't display children unless the parent is clicked.
    if (!empty($item['children'])) {
      if ($item['active']) {
        $link['class'] = 'expanded-facet';
        $link['children'] = facetapi_links_get($item['children']);
      }
      else {
        $link['class'] = 'collapsed';
      }
    }

    // Themes the link.
    $link['data'] = theme(
      $item['theme hook'], $item['text'], $item['path'], $item['options'], $item['count']
    );

    // Adds links to array.
    $links[] = $link;
  }

  return $links;
}

/**
 * Converts UIDs to username.
 *
 * @param $uids
 *   An array containing the user IDs.
 *
 * @return
 *   An array mapping user IDs to usernames.
 */
function facetapi_node_uid_map(array $uids) {
  $sql = '
    SELECT uid AS id, name
    FROM {users}
    WHERE uid IN (!placeholders)
  ';

  $map = facetapi_map_query($sql, $uids);
  if (isset($map[0])) {
    $map[0] = variable_get('anonymous', t('Anonymous'));
  }
  return $map;
}

/**
 * Converts machine readable content types to display names.
 *
 * @param $types
 *   An array containing the machine readable content types.
 *
 * @return
 *   An array mapping the machine readable content types to the display names.
 */
function facetapi_node_type_map(array $types) {
  $sql = '
    SELECT type AS id, name
    FROM {node_type}
    WHERE type IN (!placeholders)
  ';
  return facetapi_map_query($sql, $types, 'varchar');
}

/**
 * Maps a taxonomy ID to a term name.
 *
 * @param $tids
 *   An array containing the term IDs.
 *
 * @return
 *   An array mapping the term IDs to the display name.
 */
function facetapi_taxonomy_term_map(array $tids) {
  $sql = '
    SELECT tid AS id, name
    FROM {term_data}
    WHERE tid IN (!placeholders)
  ';
  return facetapi_map_query($sql, $tids);
}

/**
 * Converts machine readable languages to
 *
 * @param $languages
 *   An array containing the machine readable content types.
 *
 * @return
 *   An array mapping the machine readable content types to the display names.
 */
function facetapi_node_language_map(array $languages) {
  $language_list = language_list();
  $map = array();
  foreach ($languages as $language) {
    if (isset($language_list[$language])) {
      $map[$language] = $language_list[$language]->name;
    }
    else {
      $map[$language] = t('Language neutral');
    }
  }
  return $map;
}

/**
 * Converts date ranges to human readable dates.
 *
 * @param $ranges
 *   An array containing the date ranges.
 *
 * @return
 *   An array mapping the ranges to nice display dates.
 */
function facetapi_date_range_map(array $ranges) {
  $map = array();
  foreach ($ranges as $value) {
    $range = explode(' TO ', trim($value, '{[]}'));
    if (2 == count($range)) {
      $gap = facetapi_date_gap_get($range[0], $range[1]);
      $map[$value] = facetapi_date_format($range[0], $gap);
    }
  }
  return $map;
}

/**
 * Gets parent information for taxonomy terms.
 *
 * @param &$tids
 *   An array containing the term ids.
 *
 * @return
 *   An associative array keyed by term ID to parent ID.
 */
function facetapi_taxonomy_hierarchy_callback(array $tids) {
  $placeholders = db_placeholders($tids);

  $sql = "
    SELECT tid, parent
    FROM {term_hierarchy}
    WHERE parent > 0 AND (tid IN ($placeholders) OR parent IN ($placeholders))
  ";

  // Executes query to get parents, relates the terms to one another.
  $parents = array();
  if ($result = db_query($sql, array_merge($tids, $tids))) {
    while ($record = db_fetch_object($result)) {
      $parents[$record->tid][] = $record->parent;
    }
  }
  return $parents;
}
