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
   * Returns a boolean flagging whether $this->_searcher executed a search.
   */
  public function searchExecuted() {
    return apachesolr_has_searched();
  }

  /**
   * The default "fetch" method for facet data.
   */
  public function fetchDefault(array $facet) {
    $facet_items = array();
    $raw_values = (array)$this->_response->facet_counts->facet_fields->{$facet['field']};
    foreach ($raw_values as $value => $count) {
      $facet_items[$value]= array('count' => $count);
    }
    return $facet_items;
  }

  /**
   * Fetches data from facets that filter results by date ranges.
   */
  public function fetchDate(array $facet) {
    $facet_items = array();

    // Makes sure we have access to the query object.
    if (!$query = apachesolr_current_query()) {
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
    foreach ($this->getActiveFacets($facet['field alias']) as $filter) {

      // Builds the raw facet "value", the count for selected items will be the
      // total number of rows returned in the query.
      $value = '['. $filter['#start'] .' TO '. $filter['#end'] .']';
      $facet_items[$value] = array('count' => $total);

      // If there is a previous item, there is a parent, uses a reference so the
      // arrays are populated when they are updated.
      if (NULL !== $parent) {
        $facet_items[$parent]['children'][$value] = &$facet_items[$value];
        $facet_items[$value]['parents'][$parent] = $parent;
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
        $facet_items[$new_value] = array('count' => $count);
        if (NULL !== $parent) {
          $facet_items[$parent]['children'][$new_value] = &$facet_items[$new_value];
          $facet_items[$new_value]['parents'][$parent] = $parent;
        }
      }
    }

    return $facet_items;
  }

  /**
   * Returns an array of facet counts.
   */
  public function getFacetData(array $facets) {
    if ($this->_response = apachesolr_static_response_cache()) {
      return parent::getFacetData($facets);
    }
    else {
      return array();
    }
  }

  /**
   * Applies a default sort to the facet items.
   */
  public function sortDefault(array &$facet_items) {
    uasort($facet_items, 'facetapi_sort_count');
  }

  /**
   * Returns query string variables for a facet link.
   */
  public function getFacetQueryString(array $facet, array $values, $active) {
    $new_query = clone apachesolr_current_query();
    $method = ($active) ? 'remove_filter' : 'add_filter';
    foreach ($values as $value) {
      // @todo Ask Robert or Peter about the $exclude variable.
      // NOTE: The remove_filter method doesn't have a third param, so we can
      // pass FALSE with no problems.
      $new_query->$method($facet['field alias'], $value, FALSE);
    }

    // Builds and returns query string.
    $qstring = $_GET;
    unset($qstring['filters']);
    return drupal_query_string_encode(
      array_merge($qstring, $new_query->get_url_queryvalues()), array('q', 'page')
    );
  }

  /**
   * Returns an array of active facet values in a field.
   *
   * @param $field
   *   A string containing the field name.
   * @return
   *   An array of values.
   */
  public function getActiveFacets($field) {
    if ($query = apachesolr_current_query()) {
      $active = array();
      foreach ($query->get_filters($field) as $filter) {
        $active[$filter['#value']] = $filter['#query'];
      }
      return $active;
    }
    return array();
  }

  /**
   * Returns an integer flagging whether the facet is active.
   */
  public function facetActive($field, $value) {
    if (!$query = apachesolr_current_query()) {
      $query = FALSE;
    }
    return ($query) ? (int)$query->has_filter($field, $value) : 0;
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
