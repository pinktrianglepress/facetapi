<?php
// $Id$

/**
 * @file
 *
 */

/**
 * Renders facets in the fieldset realm.
 *
 * @param $adapter
 *   A FacetapiAdapter object extended by the searcher module.
 * @param $realm_name
 *   A string containing the machine readable name of the realm.
 * @param $facets
 *   An array containing the full facet definitions.
 * @return
 *   A FAPI array.
 */
function facetapi_realm_fieldset_render(FacetapiAdapter $adapter, $realm_name, array $facets) {
  $form = array();
  foreach ($facets as $facet) {
    $form = array_merge_recursive($form, facetapi_facet_to_fapi($facet));
  }
  return $form;
}
drupal_set_breadcrumb();
/**
 * Converts a facet to a FAPI array.
 *
 * @param $facet
 *   An array containing the full facet definition.
 * @return
 *   A FAPI array.
 */
function facetapi_facet_to_fapi(array $facet) {
  $base_form = array(
    '#type' => $facet['form type'],
    '#title' => check_plain($facet['title']),
    '#weight' => $facet['weight'],
    '#description' => filter_xss($facet['description']),
  );
  switch ($facet['form type']) {
    case 'checkbox':
    case 'date':
    case 'textarea':
    case 'textfield':
      $form[$facet['field alias']] = array_merge($base_form, array(
        '#default_value' => '', // @todo get facet value passed through form.
      ));
      break;
    case 'checkboxes':
    case 'radios':
    case 'select':
      $form[$facet['field alias']] = array_merge($base_form, array(
        '#options' => array(),
        '#default_value' => '', // @todo get facet value passed through form.
      ));
      break;
    case 'markup':
      $form[$facet['field alias']] = array_merge($base_form, array(
        '#value' => $base_form['#title']
      ));
      break;
    default:
      $form = array();
      break;
  }
  return $form;
}
