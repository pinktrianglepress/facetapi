<?php

/**
 * @file
 * Classes used by the Facet API module.
 */

/**
 * Facet API adapter for the Apache Solr Search Integration module.
 */
class FacetapiApachesolrFacetapiAdapter extends FacetapiAdapter {

  /**
   * Returns a boolean flagging whether $this->_searcher executed a search.
   */
  public function searchExecuted() {
    return apachesolr_has_searched();
  }

  /**
   * Returns the search keys.
   */
  public function getSearchKeys() {
    if (NULL === $this->keys) {
      if ($query = apachesolr_current_query()) {
        return $query->get_query_basic();
      }
    }
    else {
      return $this->keys;
    }
  }

  /**
   *
   */
  public function addFacet(array $facet, $query) {
    parent::addFacet($facet, $query);

    $queries = array();
    foreach ($this->getActiveItems($facet) as $value => $item) {
      $queries[] = $facet['field alias'] . ':' . $value;
    }
    if (!empty($queries)) {
      $query->params['fq'][$facet['field alias']] = $queries;
    }
  }
}
