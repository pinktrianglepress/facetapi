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

    // Themes the item list.
    $output .= '<div' . drupal_attributes($element['#attributes']) . '>';
    $output .= theme('item_list', $element[$field_alias], $title);
    $output .= '</div>';
  }

  return $output;
}

/**
 * Themes the facet title.
 *
 * @param $title
 *   A string containing the facet title.
 *
 * @return
 *   A string containing the themed title.
 */
function theme_facetapi_title($title) {
  return t('Filter by @title:', array('@title' => drupal_strtolower($title)));
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
 *
 * @return
 *   A string containing the themed link.
 *
 * @see facetapi_l()
 */
function theme_facetapi_link($text, $path, array $options = array()) {
  return facetapi_l($text, $path, $options);
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
 *
 * @return
 *   A string containing the themed link.
 *
 * @see facetapi_l()
 */
function theme_facetapi_link_inactive($text, $path, array $options = array(), $count = 0) {
  if ($count) {
    $text .= ' ' . theme('facetapi_count', $count);
  }
  return facetapi_l($text, $path, $options);
}

/**
 * Themes the count for the facet item.  For example, this number shows how many
 * results will be returned after clicking on a link.
 *
 * @param $count
 *   An integer containing the count.
 *
 * @return
 *   The themes number.
 */
function theme_facetapi_count($count) {
  return '(' . check_plain($count) . ')';
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
 *
 * @return
 *   A string containing the themed link.
 *
 * @see facetapi_l()
 */
function theme_facetapi_link_active($text, $path, array $options = array()) {
  return facetapi_l('(-) ' . $text, $path, $options);
}
