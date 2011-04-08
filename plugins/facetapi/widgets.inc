<?php

/**
 * @file
 */

abstract class FacetapiWidget {

  /**
   * Abstract public function.
   *
   * @param &$build
   *   The render array.
   * @param &$key
   *   The element name.
   * @param &$js_settings
   *   An array of JavaScript settings.
   */
  abstract public function execute(array &$build, &$key, &$js_settings);

}

class FacetapiWidgetLinks extends FacetapiWidget {

  /**
   *
   */
  public function execute(array &$build, &$key, &$js_settings) {
    drupal_add_js(drupal_get_path('module', 'facetapi') .'/js/facetapi.block.js');

    // Captures facet information for code readability,
    $facet_name  = $build['#facet']['name'];
    $field_alias = $build['#facet']['field alias'];

    // Sets key to facet name since facets shouldn't be grouped by alias.
    $key = $build['#facet']['name'];

    // Adds theme functions to each item dependent on whether it is active or not.
    $this->setThemeHooks($build[$field_alias], 'facetapi_link_active', 'facetapi_link_inactive');

    // Converts the array to something that can be read by theme_item_list().
    $build['#theme'] = 'item_list';
    $build[$field_alias] = $this->buildItemList($build[$field_alias]);

    // Adds "soft limit" to the javascript settings.
    $js_settings['limit'] = $build['#settings']->settings['soft_limit'];
  }

  /**
   * Recursive function that sets theme hooks.
   *
   * @param &$build
   *   A render array containing the facet items.
   * @param $active_hook
   *   A string containing the theme hook to use when the facet is active.
   * @param $inactive_hook
   *   A string containing the theme hook to use when the facet is inactive.
   */
  protected function setThemeHooks(array &$build, $active_hook = NULL, $inactive_hook = NULL) {
    foreach ($build as $value => &$item) {
      if (empty($item['#active']) && NULL !== $inactive_hook) {
        $item['#theme'] = $inactive_hook;
      }
      if (!empty($item['#active']) && NULL !== $active_hook) {
        $item['#theme'] = $active_hook;
      }
      if (!empty($item['#item_children'])) {
        $this->setThemeHooks($item['#item_children'], $active_hook, $inactive_hook);
      }
    }
  }

  /**
   * Recursive function that converts the render array into an array that can be
   * passed to theme_item_list().
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
  function buildItemList($build) {
    $items = array();
    foreach ($build as $value => $item) {
      $row = array();

      $options = array(
        'attributes' => array(),
        //'query' => drupal_http_build_query($item['#query'], array('q', 'page')),
      );

      // We don't display children unless the parent is clicked.
      if (!empty($item['#item_children'])) {
        if ($item['#active']) {
          $row['class'] = 'expanded';
          $row['children'] = $this->buildItemList($item['#item_children']);
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
      $row['data'] = theme(
        $item['#theme'], $item['#markup'], $_GET['q'], $options, $item['#count']
      );

      // Adds links to array.
      $items[] = $row;
    }

    return $items;
  }
}
