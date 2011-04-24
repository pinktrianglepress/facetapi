<?php

/**
 * @file
 *
 */

/**
 * Widget that renders facets as a list of clickable links as checkboxes.
 */
class FacetapiWidgetCheckboxLinks extends FacetapiWidgetLinks {

  /**
   * Recursive function that converts the render array into an array that can be
   * passed to theme_item_list().
   *
   * @param array $build
   *   The facet's render array.
   *
   * @return array
   *   The "items" parameter for theme_item_list().
   */
  function buildListItems($build) {
    $items = array();
    foreach ($build as $value => $item) {
      $row = array('class' => array());

      // Initializes variables passed to theme hook.
      $variables = array(
        'text' => $item['#markup'],
        'path' => $_GET['q'],
        'count' => $item['#count'],
        'options' => array(
          'attributes' => array('class' => array('facet-checkbox')),
          'html' => FALSE,
          'query' => $item['#query'],
        ),
      );

      // We don't display children unless the parent is clicked.
      if (!empty($item['#item_children'])) {
        if ($item['#active']) {
          $row['class'][] = 'expanded';
          $row['children'] = $this->buildListItems($item['#item_children']);
        }
        else {
          $row['class'][] = 'collapsed';
        }
      }

      // Gets theme hook, adds last minute classes.
      if ($item['#active']) {
        $variables['options']['attributes']['class'][] = 'facet-unclick';
      }
      else {
        $variables['options']['attributes']['class'][] = 'facet-click';
      }

      // Themes the link, adds row to items.
      $row['data'] = theme($item['#theme'], $variables);
      $items[] = $row;
    }

    return $items;
  }
}
