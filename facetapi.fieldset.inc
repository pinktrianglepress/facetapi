<?php
// $Id$

/**
 * @file
 * Functions related to the fieldset realm.
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
 *
 * @return
 *   A FAPI array.
 */
function facetapi_realm_fieldset_render(FacetapiAdapter $adapter, $realm_name, array $facets) {
  $form = array();
  foreach ($facets as $facet) {
    $form = array_merge_recursive($form, facetapi_facet_to_fapi($facet, $adapter));
  }
  return $form;
}

/**
 * Converts a facet to a FAPI array.
 *
 * @param $facet
 *   An array containing the full facet definition.
 *
 * @return
 *   A FAPI array.
 */
function facetapi_facet_to_fapi(array $facet, FacetapiAdapter $adapter) {
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
        '#default_value' => '', // @todo get facet value passed through form.
      ));
      if (!empty($facet['values callback'])) {
        $form[$facet['field alias']] = array_merge($base_form, array(
          '#options' => $facet['values callback']($facet),
        ));
      }
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

/**
 * Returns values for vocabulary facets.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   An array of values passed as options to the form element.
 */
function facetapi_values_taxonomy(array $facet) {
  // Extracts vid, loads vocabulary object, returns empty array on any error.
  if (!preg_match('/^vocabulary_(\d+)$/', $facet['name'], $match)) {
    return array();
  }
  if (!$vocabulary = taxonomy_vocabulary_load($match[1])) {
    return array();
  }

  // Builds options from taxonomy tree.
  $options = array();
  $tree = taxonomy_get_tree($vocabulary->vid);
  if ($tree && (count($tree) > 0)) {
    $options[$vocabulary->name] = array();
    foreach ($tree as $term) {
      $options[$vocabulary->name][$term->tid] = check_plain(str_repeat('-', $term->depth) . $term->name);
    }
  }
  return $options;
}

/**
 * Returns values for the language facet.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   An array of values passed as options to the form element.
 */
function facetapi_values_language(array $facet) {
  $options = array(0 => t('Language neutral'));
  foreach (language_list() as $code => $language) {
    $options[$code] = check_plain($language->name);
  }
  return $options;
}

/**
 * Returns values for the language facet.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   An array of values passed as options to the form element.
 */
function facetapi_values_type(array $facet) {
  return array_map('check_plain', node_get_types('names'));
}


