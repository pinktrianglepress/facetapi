<?php
// $Id$

/**
 * @file
 * Classes used by the Facet API module.
 */

/**
 * Facet API adapter for the core Search module.
 */
class FacetapiSearchAdapter extends FacetapiAdapter {

  /**
   * Returns a boolean flagging whether $this->_searcher executed a search.
   *
   * @return
   *   A booean flagging whether $this->_searcher executed a search.
   */
  public function searchExecuted() {
    return (search_get_keys() && arg(1) == 'node');
  }

  /**
   * Returns an array of facet counts.
   */
  public function getFacetData(array $facets) {
  }

  /**
   * Applies a default sort to the facet items.
   */
  public function sortDefault(array &$items) {
  }

  /**
   * Returns query string variables for a facet link.
   */
  public function getQueryString(array $facet, array $values, $active) {
  }

  /**
   * Returns an array of active facet values in a field.
   */
  public function getActiveFacets($field) {

  }

  /**
   * Returns an integer flagging whether the facet is active.
   */
  public function facetActive($field, $value) {

  }
}

