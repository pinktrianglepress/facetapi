<?php

/**
 * @file
 *
 */

/**
 * Widget that renders facets as a list of clickable checkboxes.
 */
class FacetapiWidgetCheckboxLinks extends FacetapiWidgetLinks {

  /**
   * Adds JavaScript settings.
   */
  public function init() {
    parent::init();
    $this->jsSettings['makeCheckboxes'] = 1;
  }

  /**
   * Sets the base class for checkbox facet items.
   *
   * @return array
   *   An array of classes.
   */
  public function getItemClasses() {
    return array('facetapi-checkbox');
  }
}
