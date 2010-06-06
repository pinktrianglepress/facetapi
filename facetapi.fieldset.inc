<?php
// $Id$

/**
 * @file
 *
 */

/**
 * Converts a facet to a FAPI array.
 *
 * @param $facet
 *   An array containing the full facet definition.
 * @return
 *   A FAPI array.
 */
function luceneapi_facet_to_fapi_convert(array $facet) {
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
