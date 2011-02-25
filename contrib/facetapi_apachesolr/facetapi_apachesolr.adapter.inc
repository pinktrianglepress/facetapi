<?php

/**
 * @file
 * Classes used by the Facet API module.
 */

/**
 * Facet API adapter for the Apache Solr Search Integration module.
 */
class FacetapiApachesolrAdapter extends FacetapiAdapter {

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
    if (NULL === $this->_keys) {
      if ($query = apachesolr_current_query()) {
        return $query->get_query_basic();
      }
    }
    else {
      return $this->_keys;
    }
    return NULL;
  }
}
