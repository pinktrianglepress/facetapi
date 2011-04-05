<?php

/**
 * @file
 *
 */

/**
 * Export form, wrapper around ctools_export_form().
 *
 * @param $form
 *
 * @param &$form_state
 *
 * @param $settings
 *
 * @see ctools_export_form()
 */
function facetapi_ui_export_form($form, &$form_state, $adapter, $realm, $facet) {

  // Adds link to get back to list page.
  $form['back'] = array(
    '#type' => 'link',
    '#title' => t('Go back to realm settings page'),
    '#href' => $adapter->getPath($realm['name']),
  );

  // Gets export fieldset from ctools_export_form().
  $title = t('Export configuration');
  $settings = $adapter->getFacet($facet)->getSettings($realm);
  $export = ctools_export_crud_export('facetapi', $settings);
  $form = ctools_export_form($form, $form_state, $export, $title);

  // Adds settings and returns form.
  $form['#settings'] = $settings;
  return $form;
}
