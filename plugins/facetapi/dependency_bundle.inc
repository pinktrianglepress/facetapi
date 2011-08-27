<?php

/**
 * @file
 * Performs a dependency check against the passed content type.
 */

/**
 * Adds a dependency on content type.
 */
class FacetapiDependencyBundle extends FacetapiDependency {

  /**
   * Executes the dependency check.
   */
  public function execute() {
    switch ($this->settings['bundle']) {
      case 'referenced':
        // @todo Do we need defensive coding here?
        $field_info = field_info_field($this->facet['field api name']);

        // Iterate over field info, collect bundles this field is attached to.
        $field_bundles = array();
        foreach ($field_info['bundles'] as $entity => $bundles) {
          $field_bundles[$entity] = array_flip($bundles);
        }

        // Find active facets that contain bundle data.
        $enabled = array_filter($this->adapter->getEnabledFacets(), array($this, 'filterBundleFacets'));
        $active = array_filter($this->activeItems);
        $active_bundles = array_intersect_key($enabled, $active);

        // Check if there is any match.
        foreach ($active_bundles as $facet) {
          foreach ($facet['field api bundles'] as $entity) {
            $bundles = (isset($field_bundles[$entity])) ? $field_bundles[$entity] : array();
            if (array_intersect_key($active[$facet['name']], $bundles)) {
              return NULL;
            }
          }
        }
        return FALSE;

      case 'selected':
        $matches = array_intersect_key(
          array_filter($this->settings['bundle_selected']),
          $this->activeItems['bundle']
        );
        return ($matches) ? NULL : FALSE;
    }
  }

  /**
   * Returns TRUE if the facet contains bundle information.
   *
   * @param array $facet
   *   The facet definition beinf filtered.
   *
   * @return
   *   A boolean flagging whether the item should remain in the array.
   */
  public function filterBundleFacets($facet) {
    return !empty($facet['field api bundles']);
  }

  /**
   * Adds dependency settings to the form.
   */
  public function settingsForm(&$form, &$form_state) {

    // Builds array of options.
    $options = array();
    $options['none'] = t('No dependencies.');
    if ($this->facet['field api name']) {
      $options['referenced'] = t('A content type this field is attached to must be active.');
    }
    $options['selected'] = t('At least one of the selected content types must be active.');

    $form[$this->id]['bundle'] = array(
      '#title' => t('Dependency settings'),
      '#type' => 'radios',
      '#options' => $options,
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
