<?php
// $Id$

/**
 * @file
 * Classes used by the Facet API module.
 */

/**
 * Facet API adapter for Apache Solr Search Integration.
 */
class FacetapiApachesolrAdapter extends FacetapiAdapter {

  /**
   * The statically cached Solr response object.
   */
  protected $_response;

  /**
   * Constructor, sets the response object.
   *
   * @param $searcher
   *   A string containing the machine readable name of the searcher module.
   * @param $type
   *   A string containing the type of content indexed by $searcher.
   * @param $module
   *   A string containing the module that defined the adapter.
   */
  public function __construct($searcher, $type, $module) {
    $this->_response = apachesolr_static_response_cache();
    parent::__construct($searcher, $type, $module);
  }

  /**
   * Returns a boolean flagging whether $this->_searcher executed a search.
   */
  public function searchExecuted() {
    return apachesolr_has_searched();
  }

  /**
   * The default "fetch" method for facet data.
   */
  public function fetchTerm(array $facet) {
    $facet_items = array();
    if (NULL !== $this->_response) {
      $raw_values = (array)$this->_response->facet_counts->facet_fields->{$facet['field']};
      foreach ($raw_values as $value => $count) {
        $facet_items[$value] = array('#count' => $count);
      }
    }
    return $facet_items;
  }

  /**
   * Fetches data from facets that filter results by date ranges.
   */
  public function fetchDate(array $facet) {
    $facet_items = array();
    if (NULL === $this->_response) {
      return array();
    }

    // Gets total number of documents matched in search.
    // NOTE: We need a Solr_Base_Query::get_solr() method.
    static $total;
    if (NULL === $total) {
      if ($raw_response = json_decode($this->_response->getRawResponse())) {
        $total = $raw_response->response->numFound;
      }
      else {
        $total = 0;
      }
    }

    // Gets the active date facets, starts to builds the "parent - child"
    // relationships.
    $parent = NULL;
    foreach ($this->getActiveItems($facet) as $value => $item) {

      // Builds the raw facet "value", the count for selected items will be the
      // total number of rows returned in the query.
      $facet_items[$value] = array('#count' => $total);

      // If there is a previous item, there is a parent, uses a reference so the
      // arrays are populated when they are updated.
      if (NULL !== $parent) {
        $facet_items[$parent]['#item_children'][$value] = &$facet_items[$value];
        $facet_items[$value]['#item_parents'][$parent] = $parent;
      }

      // Stores the last value iterated over.
      $parent = $value;
    }

    // Gets raw facet data from the Solr server.
    if (isset($this->_response->facet_counts->facet_dates)) {
      $raw_data = (array)$this->_response->facet_counts->facet_dates->{$facet['field']};
    }
    else {
      $raw_data = array();
    }

    $end = (!empty($raw_data['end'])) ? $raw_data['end'] : '';
    $gap = (!empty($raw_data['gap'])) ? $raw_data['gap'] : '';

    unset($raw_data['end']);
    unset($raw_data['gap']);

    // Treat each date facet as a range start, and use the next date facet
    // as range end.  Use 'end' for the final end.
    $range_end = array();
    $previous = NULL;
    foreach ($raw_data as $value => $count) {
      if (isset($previous)) {
        $range_end[$previous] = $value;
      }
      $previous = $value;
    }
    $range_end[$previous] = $end;

    // Builds facet counts object used by the server.
    foreach ($raw_data as $value => $count) {
      if ($count) {
        $new_value = '['. $value .' TO '. $range_end[$value] .']';
        $facet_items[$new_value] = array('#count' => $count, '#active' => 0);
        if (NULL !== $parent) {
          $facet_items[$parent]['#item_children'][$new_value] = &$facet_items[$new_value];
          $facet_items[$new_value]['#item_parents'][$parent] = $parent;
        }
      }
    }

    return $facet_items;
  }

  /**
   *
   */
  public function fetch(array $facet) {
    if (NULL === $this->_response) {
      $this->_response = apachesolr_static_response_cache();
    }
    return parent::fetch($facet);
  }

  /**
   * Applies a default sort to the facet items.
   */
  public function sortDefault(array &$facet_items) {
    uasort($facet_items, 'facetapi_sort_count');
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
