<?php

/**
 * @file
 * Base dependency plugin class.
 */

/**
 * Abstract class extended by dependency plugins.
 *
 * Dependencies are conditions that must be met for facets to be processed and
 * displayed. The system allows site builders to implement the "progressive
 * disclosure" pattern, where facets are exposed only after some action has
 * been taken. For example, a common use case is to only expose taxonomy facets
 * when the bundle they are attached to is selected by the user as an active
 * facet.
 */
abstract class FacetapiDependency {

  /**
   * The adapter object.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * The facet definition as re
   *
   * @var array
   */
  protected $facet;

  /**
   * An array of active items.
   *
   * @var array
   */
  protected $activeItems;

  /**
   * An array of facet settings.
   *
   * @var array
   */
  protected $settings;

  /**
   *
   *
   * @param FacetapiAdapter $adapter
   *
   * @param array $facet
   *
   * @param array $active_items
   *
   * @param stdClass $settings
   *
   * @param array $id
   *
   * @param array $label
   *
   */
  public function __construct($id, FacetapiAdapter $adapter, array $facet, stdClass $settings, array $active_items = array()) {
    $this->id = $id;
    $this->adapter = $adapter;
    $this->facet = $facet;
    $this->activeItems = $active_items;

    // Captures dependency settings only, makes sure defaults are set.
    if (empty($settings->settings['dependencies'])) {
      $settings->settings['dependencies'] = array();
    }
    $this->settings = $settings->settings['dependencies'];
    $this->settings += $this->getDefaultSettings();
  }

  /**
   * Gets the id of the plugin.
   *
   * @return string
   *   The machine readable name of the plugin.
   */
  public function getId() {
    return $this->id;
  }

  /**
   * Gets the facet definition.
   *
   * @return array
   *   The facet definition.
   */
  public function getFacet() {
    return $this->facet;
  }

  /**
   * Performs the dependency check.
   *
   * @return boolean|NULL
   *   Return NULL to pass through to other dependency plugins, or return a
   *   boolean to explicitly set the result.
   */
  abstract public function execute();

  /**
   *
   */
  public function settingsForm(&$form, &$form_state) {
    // Nothing to do.
  }

  /**
   *
   */
  public function getDefaultSettings() {
    return array();
  }
}
