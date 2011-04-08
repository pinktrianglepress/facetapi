<?php

/**
 * @file
 * Theme functions for the Facet API module.
 */

/**
 * Wrapper around theme_item_list(), accepts render array as an argument.
 *
 * @param $variables
 *
 * @see theme_item_list()
 * @todo Remove in Drupal 7.
 */
function theme_facetapi_item_list($element) {
  $output = '';

  $field_alias = $element['#facet']['field alias'];
  if (!empty($element[$field_alias])) {
    // Themes title, initializes attributes if necessary.
    $title = theme('facetapi_title', $element['#title']);
    if (empty($element['#attributes'])) {
      $element['#attributes'] = array();
    }

    // Variables to pass to theme('item_list');
    $variables = array(
      'title' => $title,
      'items' => $element[$field_alias],
    );

    // Themes the item list.
    $output .= '<div' . drupal_attributes($element['#attributes']) . '>';
    $output .= theme('item_list', $variables);
    $output .= '</div>';
  }

  return $output;
}

/**
 * Themes the facet title.
 */
function theme_facetapi_title($variables) {
  return t('Filter by @title:', array('@title' => drupal_strtolower($variables['title'])));
}

/**
 * Themes a facet link for a value that is currently being searched.
 */
function theme_facetapi_link($variables) {
  return facetapi_l('', $path, $options);
}

/**
 * Themes a facet link with an optional count.
 */
function theme_facetapi_link_inactive($variables) {
  if (!empty($variables['count'])) {
    $variables['text'] .= ' ' . theme('facetapi_count', $variables);
  }
  return theme('link', $variables);
}

/**
 * Themes the count for the facet item.  For example, this number shows how many
 * results will be returned after clicking on a link.
 */
function theme_facetapi_count($variables) {
  return '(' . check_plain($variables['count']) . ')';
}

/**
 * Themes a facet link for a value that is currently being searched.
 */
function theme_facetapi_link_active($variables) {
  if (isset($variables['text'])) {
    $variables['text'] = '(-) ' . $variables['text'];
  }
  return theme('link', $variables);
}
