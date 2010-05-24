<?php
// $Id$

/**
 * @file
 * Export / import functions for Facet API.
 */

/**
 * Displays a form that exports something.
 *
 * @param &$form_state
 *   A keyed array containing the current state of the form.
 * @param $adapter
 *   A FacetapiAdapter object.
 * @param $realm
 *   An array containing the full realm definition.
 * @param $facet
 *   An array containing the fill realm definition.
 * @return
 *   A FAPI array containing the form.
 */
function facetapi_import_form(&$form_state, FacetapiAdapter $adapter, array $realm, array $facet) {
  $form = array();

  $form['storage'] = array(
    '#type' => 'value',
    '#value' => array(
      'searcher' => $adapter->getSearcher(),
      'realm_name' => $realm['name'],
      'facet_name' => $facet['name'],
    ),
  );

  $form['import'] = array(
    '#title' => 'Import data',
    '#type' => 'textarea',
    '#rows' => 15,
    '#required' => TRUE,
  );

  $form['export'] = array(
    '#type' => 'value',
    '#value' => array(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  return $form;
}

/**
 * Validates facetapi_import_form form submissions.
 */
function facetapi_import_form_validate(&$form, &$form_state) {
  $storage = $form_state['values']['storage'];
  if ($facets = facetapi_export_extract($form_state['values']['import'], 'import')) {
    if (isset($facets[$storage['facet_name']])) {
      $export = array($storage['facet_name'] => $facets[$storage['facet_name']]);
      form_set_value($form['export'], $export, $form_state);
    }
    else {
      form_set_error('import', t(
        'Expecting import data for the %facet-name facet',
        array('%facet-name' => $storage['facet_name'])
      ));
    }
  }
}

/**
 * Processes facetapi_import_form form submissions.
 */
function facetapi_import_form_submit(&$form, &$form_state) {
  $storage = $form_state['values']['storage'];
  $export = $form_state['values']['export'];

  // Imports the configurations, clears the cache.
  facetapi_facets_import($export, $storage['searcher'], $storage['realm_name']);
  cache_clear_all('facetapi:', 'cache', TRUE);

  // Sets success message.
  drupal_set_message(t(
    'Successfully imported configurations for the %facet-name facet .',
    array('%facet-name' => $storage['facet_name'])
  ));
}

/**
 * Imports facets.
 *
 * @param $export
 *   An array containing the export array.
 * @param $searcher
 *   A string containing the machine readable name of the searcher module.
 * @param $realm_name
 *   A string containing the machine readable name of the realm.
 * @return
 *   NULL
 * @todo Check that facets are defined?
 */
function facetapi_facets_import(array $export, $searcher, $realm_name) {

  // Defines setting defaults.
  $defaults = array(
    'facet_enabled' => FALSE,
    'sort' => 0,
    'operator' => FACETAPI_OPERATOR_AND,
    'soft_limit' => 10,
    'hard_limit' => 20,
  );

  // Allows modules to alter the export array being imported.
  drupal_alter('facetapi_facets_import', $export, $searcher, $realm_name);

  // Iterates over exports, applies defaults and imports settings.
  foreach ($export as $facet_name => $settings) {
    $settings = array_merge($defaults, $settings);

    // The "facet_enabled" setting is a special case, use the API function to
    // perform the action. Build the function name based on the setting.
    $function  = 'facetapi_facet_';
    $function .= ($settings['facet_enabled']) ? 'enable' : 'disable';
    $function($searcher, $realm_name, $facet_name);
    unset($settings['facet_enabled']);

    // Sets all of the settings.
    foreach ($settings as $setting => $value) {
      facetapi_setting_set($setting, $value, $searcher, $realm_name, $facet_name);
    }
  }
}

/**
 * Extracts export settings from the passed import string.
 *
 * @param $import
 *   A string containing the import data passed through the form.
 * @param $element
 *   A string containing the form element them import data was passed through.
 *   Form errors will be thrown if $element is not NULL.
 * @return
 *   An array containing he import data, FALSE on errors.
 */
function facetapi_export_extract($import, $element = NULL) {
  try {
    // Builds code to pass to drupal_eval().
    $code = "<?php

      ob_start();
      $import
      ob_end_clean();

      if (isset(\$facets)) {
        echo serialize(\$facets);
      }
    ";

    // Executes the import code, makes sure we get what we expect.
    if (!$export = unserialize(drupal_eval($code))) {
      throw new Exception(t('Import data malformed.'));
    }
    elseif (!isset($export)) {
      throw new Exception(t('Import data not found.'));
    }
    elseif (!is_array($export)) {
      throw new Exception(t('Expecting import data to be an array.'));
    }

    // Returns the "export" data.
    return $export;
  }
  catch (Exception $e) {
    if (NULL !== $element) {
      form_set_error($element, $e->getMessage());
    }
  }
  return FALSE;
}

/**
 * Displays a form that exports something.
 *
 * @param &$form_state
 *   A keyed array containing the current state of the form.
 * @param $adapter
 *   A FacetapiAdapter object.
 * @param $realm
 *   An array containing the full realm definition.
 * @param $facet
 *   An array containing the fill realm definition.
 * @return
 *   A FAPI array containing the form.
 */
function facetapi_export_form(&$form_state, FacetapiAdapter $adapter, array $realm, array $facet) {
  $form = array();

  $form['export'] = array(
    '#title' => 'Export data',
    '#type' => 'textarea',
    '#default_value' => facetapi_facet_export_get($adapter, $realm, $facet),
    '#rows' => 15,
  );

  return $form;
}

/**
 * Returns a facet export string.
 *
 * @param $adapter
 *   A FacetapiAdapter object.
 * @param $realm
 *   An array contaiing the full realm definition.
 * @param $facet
 *   An array containing the full facet definition.
 * @return
 *   A string containing PHP code with the export array.
 */
function facetapi_facet_export_get(FacetapiAdapter $adapter, array $realm, array $facet) {
  $export = facetapi_facet_settings_get($adapter, $realm, $facet);
  return sprintf("\$facets['%s'] = %s;", $facet['name'], var_export($export, TRUE));
}

/**
 * Returns an array containing a facet's settings.
 *
 * @param $adapter
 *   A FacetapiAdapter object.
 * @param $realm
 *   An array contaiing the full realm definition.
 * @param $facet
 *   An array containing the full facet definition.
 * @return
 *   An array containing the facet's settings.
 */
function facetapi_facet_settings_get(FacetapiAdapter $adapter, array $realm, array $facet) {
  $export = array();

  // Gets machine readable name of the searcher module.
  $searcher = $adapter->getSearcher();

  // An array keyed by setting name to default value.
  $defaults = array(
    'sort' => 0,
    'operator' => FACETAPI_OPERATOR_AND,
    'soft_limit' => 10,
    'hard_limit' => 20,
  );

  // Builds the export variable.
  foreach ($defaults as $setting => $default) {
    // Gets setting value, uses default if varialbe is not set.
    $value = facetapi_setting_get($setting, $searcher, $realm['name'], $facet['name']);
    $export[$setting] = (NULL !== $value) ? $value : $default;
  }

  // Adds whether the facet is enabled in the given realm.
  // NOTE: Using ternary operator was simply too long to fit on one line IMHO.
  if (facetapi_facet_enabled($searcher, $realm['name'], $facet['name'])) {
    $export['facet_status'] = TRUE;
  }
  else {
    $export['facet_status'] = FALSE;
  }

  // Allows modules to alter the export data prior to being returned.
  drupal_alter('facetapi_facets_export', $export, $searcher, $realm['name']);
  return $export;
}
