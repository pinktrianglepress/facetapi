<?php

/**
 * @file
 * Base dependency plugin class.
 */

/**
 * Abstract class extended by dependency plugins.
 */
abstract class FacetapiDependency {

  /**
   * The adapter object.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * The facet definition.
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
   *
   *
   * @param FacetapiAdapter $adapter
   *
   * @param array $facet
   *
   * @param array $active_items
   *
   */
  public function __construct(FacetapiAdapter $adapter, array $facet, array $active_items) {
    $this->adapter = $adapter;
    $this->facet = $facet;
    $this->activeItems = $active_items;
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
}
