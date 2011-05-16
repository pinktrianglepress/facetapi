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
    switch ($this->settings['bundle']) {
      case 'referenced':

        break;

      case 'selected':
        $matches = array_intersect_key(
          array_filter($this->settings['bundle_selected']),
          $this->activeItems['bundle']
        );
        return ($matches) ? NULL : FALSE;
    }
  }

  /**
   * Adds dependency settings to the form.
   */
  public function settingsForm(&$form, &$form_state) {

    // @todo Implement "referenced".
    $form[$this->id]['bundle'] = array(
      '#title' => t('Dependency settings'),
      '#type' => 'radios',
      '#options' => array(
        'none' => t('No dependencies.'),
        //'referenced' => t('A content type this field is referenced by must be active.'),
        'selected' => t('At least one of the selected content types must be active.'),
      ),
      '#default_value' => $this->settings['bundle'],
    );

    $form[$this->id]['bundle_selected'] = array(
      '#title' => t('Content types'),
      '#type' => 'checkboxes',
      '#options' => array_map('check_plain', node_type_get_names()),
      '#default_value' => $this->settings['bundle_selected'],
      '#states' => array(
        'visible' => array(
          'input[name="bundle"]' => array('value' => 'selected'),
        ),
      ),
    );
  }

  /**
   * Returns defaults for settings.
   */
  public function getDefaultSettings() {
    return array(
      'bundle' => 'none',
      'bundle_selected' => array(),
    );
  }
}
