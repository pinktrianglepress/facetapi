<?php

/**
 * @file
 * Performs a dependency check against the passed content type.
 */

/**
 *
 */
class FacetapiDependencyBundle extends FacetapiDependency {

  /**
   * Executes the dependency check.
   */
  public function execute() {

  }

  /**
   * Adds dependency settings to the form.
   */
  public function settingsForm(&$form, &$form_state) {

    $form[$this->id]['bundle'] = array(
      '#title' => t('Dependency settings'),
      '#type' => 'radios',
      '#options' => array(
        'none' => t('No dependencies.'),
        'referenced' => t('A content type this field is referenced by must be active.'),
        'selected' => t('At least one of the selected content types must be active.'),
      ),
      '#default_value' => 'none',
    );

    $form[$this->id]['bundle_selected'] = array(
      '#title' => t('Content types'),
      '#type' => 'checkboxes',
      '#options' => array_map('check_plain', node_type_get_names()),
      '#default_value' => array(),
      '#states' => array(
        'visible' => array(
          'input[name="bundle"]' => array('value' => 'selected'),
        ),
      ),
    );
  }
}
