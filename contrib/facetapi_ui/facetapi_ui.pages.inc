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
function luceneapi_ui_export_form($form, &$form_state, $adapter, $realm, $facet) {

  // Adds link to get back to list page.
  $form['back'] = array(
    '#type' => 'link',
    '#title' => t('Go back to list page'),
    '#href' => luceneapi_ui_calculate_settings_path($settings->getTable()),
  );

  // Gets export fieldset from ctools_export_form().
  $title = t('Export configuration');
  $form = ctools_export_form($form, $form_state, $settings->getExport(), $title);

  // Adds settings and returns form.
  $form['#settings'] = $settings;
  return $form;
}