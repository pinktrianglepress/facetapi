<?php

/**
 * @file
 * Widget callbacks and building functions.
 */

/**
 * Builds items as array of links that can be passed to theme_item_list().
 *
 * @param &$build
 *   The facet's render array.
 * @param &$key
 *   A string containing the key the facet's render array will be appended to.
 * @param &$settings
 *   An array containing the JavaScript settings.
 */
function facetapi_widget_links(array &$build, &$key, &$settings) {
  drupal_add_js(drupal_get_path('module', 'facetapi') .'/js/facetapi.block.js');

  // Captures facet information for code readability,
  $facet_name  = $build['#facet']['name'];
  $field_alias = $build['#facet']['field alias'];

  // Sets key to facet name since facets shoudln't be grouped by alias.
  $key = $build['#facet']['name'];

  // Adds theme functions to each item dependent on whether it is active or not.
  facetapi_theme_hooks_set($build[$field_alias], 'facetapi_link_active', 'facetapi_link_inactive');

  // Converts the array to something that can be read by theme_item_list().
  $build['#theme'] = 'facetapi_item_list';
  $build[$field_alias] = facetapi_item_list_build($build[$field_alias]);

  // Adds "soft limit" to the javascript settings.
  $searcher = $build['#adapter']->getSearcher();
  $limit = facetapi_setting_get('soft_limit', $searcher, $build['#realm_name'], $build['#facet']['name']);
  $settings['limit'] = (NULL === $limit) ? 10 : $limit;
}

/**
 * Renders facets as links with checkboxes, useful for facets using the OR
 * operator.
 *
 * @param &$build
 *   The facet's render array.
 * @param &$key
 *   A string containing the key the facet's render array will be appended to.
 * @param &$settings
 *   An array containing the JavaScript settings.
 */
function facetapi_widget_link_checkboxes(array &$build, &$key, &$settings) {
  drupal_add_js(drupal_get_path('module', 'facetapi') .'/js/facetapi.checkboxes.js');
  facetapi_widget_links($build, $key, $settings);
}

/**
 * Builds items as checkbox form elemets.
 *
 * @param &$build
 *   The facet's render array.
 * @param &$key
 *   A string containing the key the facet's render array will be appended to.
 * @param &$settings
 *   An array containing the JavaScript settings.
 */
function facetapi_widget_checkboxes(array &$build, &$key, &$settings) {
  $build['#type'] = 'checkboxes';
  $build['#default_value'] = $build['#adapter']->getActiveValues($build['#facet']);
  $build['#options'] = array_map('check_plain', facetapi_form_options_get($build));
  $build['#prefix'] = '<div class="criterion">';
  $build['#suffix'] = '</div>';
  unset($build[$build['#facet']['field alias']]);
}

/**
 * Builds items as radio for elements.
 *
 * @param &$build
 *   The facet's render array.
 * @param &$key
 *   A string containing the key the facet's render array will be appended to.
 * @param &$settings
 *   An array containing the JavaScript settings.
 */
function facetapi_widget_radios(array &$build, &$key, &$settings) {

  // Gets the default value, on;y one can be passed for radio widget.
  $active_values = $build['#adapter']->getActiveValues($build['#facet']);
  if (!empty($active_values)) {
    $build['#default_value'] = array_shift($active_values);
  }
  else {
    $build['#default_value'] = '__all__';
  }

  // Builds radio element.
  $build['#type'] = 'radios';
  $build['#options'] = array_map('check_plain', facetapi_form_options_get($build, TRUE));
  $build['#prefix'] = '<div class="criterion">';
  $build['#suffix'] = '</div>';
  unset($build[$build['#facet']['field alias']]);
}

/**
 * Builds items as a select list element.
 *
 * @param &$build
 *   The facet's render array.
 * @param &$key
 *   A string containing the key the facet's render array will be appended to.
 * @param &$settings
 *   An array containing the JavaScript settings.
 */
function facetapi_widget_select(array &$build, &$key, &$settings) {
  $build['#type'] = 'select';
  $build['#default_value'] = $build['#adapter']->getActiveValues($build['#facet']);
  $build['#options'] = facetapi_form_options_get($build, TRUE);
  $build['#prefix'] = '<div class="criterion">';
  $build['#suffix'] = '</div>';
  unset($build[$build['#facet']['field alias']]);
}

/**
 * Builds items as a multiple value select list element.
 *
 * @param &$build
 *   The facet's render array.
 * @param &$key
 *   A string containing the key the facet's render array will be appended to.
 * @param &$settings
 *   An array containing the JavaScript settings.
 */
function facetapi_widget_multiselect(array &$build, &$key, &$settings) {
  $build['#type'] = 'select';
  $build['#multiple'] = TRUE;
  $build['#default_value'] = $build['#adapter']->getActiveValues($build['#facet']);
  $build['#options'] = facetapi_form_options_get($build);
  $build['#prefix'] = '<div class="criterion">';
  $build['#suffix'] = '</div>';
  $build['#size'] = 10;
  unset($build[$build['#facet']['field alias']]);
}

/**
 * Builds items as a textfield element.
 *
 * @param &$build
 *   The facet's render array.
 * @param &$key
 *   A string containing the key the facet's render array will be appended to.
 * @param &$settings
 *   An array containing the JavaScript settings.
 */
function facetapi_widget_textfield(array &$build, &$key, &$settings) {
  $build['#type'] = 'textfield';
  $build['#default_value'] = drupal_implode_tags($build['#adapter']->getActiveValues($build['#facet']));
  $build['#prefix'] = '<div class="criterion">';
  $build['#suffix'] = '</div>';
  $build['#size'] = 30;
  unset($build[$build['#facet']['field alias']]);
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
  foreach (facetapi_adapter_info_get() as $searcher => $definition) {
    $facets = facetapi_enabled_facets_get($searcher, 'block');
    foreach ($facets as $facet_name => $facet) {

      // Gets the delta from the delta map.
      $string = $searcher .':block:'. $facet_name;
      $delta = array_search($string, $map);

      // Defines the block.
      // @todo explore more efficient caching options.
      $blocks[$delta] = array(
        'info' => sprintf('Facet API: %s: %s', $searcher, $facet_name),
        'cache' => BLOCK_NO_CACHE,
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
  static $builds = array();

  // Bails if delta isn't mapped.
  $map = facetapi_delta_map_get();
  if (!isset($map[$delta])) {
    return array();
  }

  // Extracts the searcher, realm name, and facet name from $delta.
  list($searcher, $realm_name, $facet_name) = explode(':', $map[$delta]);
  $group = $searcher .':'. $realm_name;

  // Bails if a search hasn't been executed.
  if ((!$adapter = facetapi_adapter_load($searcher)) || !$adapter->searchExecuted()) {
    return array();
  }

  // Builds and caches the entire realm.
  if (!isset($builds[$group])) {
    $builds[$group] = facetapi_realm_build($searcher, $realm_name);
  }

  // Returns the individual block.
  if (isset($builds[$group][$facet_name])) {
    return array(
      'subject' => NULL,
      'content' => drupal_render($builds[$group][$facet_name])
    );
  }

  return array();
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
      foreach (facetapi_adapter_info_get() as $searcher => $definition) {
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
 * Recursive function that converts the render array into an array that can be
 * passed to theme_item_list().
 *
 * NOTE: In Drupal 6 theme_item_list() does not accept an array, so the wrapper
 * function theme_facetapi_item_list() must be used.
 *
 * @param $build
 *   The render array for the facet's items.
 *
 * @return
 *   An array that can be passed the theme_item_list().
 *
 * @see theme_item_list()
 * @see theme_facetapi_item_list()
 */
function facetapi_item_list_build($build) {
  $items = array();
  foreach ($build as $value => $item) {
    $row = array();

    $options = array(
      'attributes' => array(),
      'query' => drupal_query_string_encode($item['#query'], array('q', 'page')),
    );

    // We don't display children unless the parent is clicked.
    if (!empty($item['#item_children'])) {
      if ($item['#active']) {
        $row['class'] = 'expanded';
        $row['children'] = facetapi_item_list_build($item['#item_children']);
      }
      else {
        $row['class'] = 'collapsed';
      }
    }

    // Gets theme hook, adds last minute classes.
    if ($item['#active']) {
      $options['attributes']['class'] = 'active';
    }

    // Themes the link.
    // @todo Change #value to #markup in D7.
    $row['data'] = theme(
      $item['#theme'], $item['#value'], $_GET['q'], $options, $item['#count']
    );

    // Adds links to array.
    $items[] = $row;
  }

  return $items;
}

/**
 * Builds array of options, pulls values from the facet's "values callback"
 * function or builds from the facet's items of the callback is not set.
 *
 * NOTE: The option values are return unescaped. It is the responsibility of the
 * widget to escape the values.
 *
 * @param $build
 *   The facet's render array.
 * @param $all_option
 *   A boolean flagging whether to append an "include all" option.
 * @param
 *
 * @return
 *   An array containing the FAPI options.
 */
function facetapi_form_options_get(&$build, $all_option = FALSE) {
  $field_alias = $build['#facet']['field alias'];

  // Initializes options.
  if (!$all_option) {
    $options = array();
  }
  else {
    // @todo The text should be a setting.
    $options = array('__all__' => t('<All>'));
  }

  // Builds options.
  if (!empty($build['#facet']['values callback']) && function_exists($build['#facet']['values callback'])) {
    $options += $build['#facet']['values callback']($build['#facet']);
  }
  else {
    foreach ($build[$field_alias] as $value => $item) {
      $options[$value] = $item['#value'];
    }
  }

  return $options;
}
