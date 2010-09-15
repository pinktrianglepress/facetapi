<?php
// $Id$

/**
 * @file
 * Classes used by the Facet API module.
 */

/**
 * Facet API adapter for Search Lucene API modules.
 */
class FacetapiLuceneapiAdapter extends FacetapiAdapter {

  /**
   * Returns a boolean flagging whether $this->_searcher executed a search.
   */
  public function searchExecuted() {
    return (luceneapi_search_executed() == $this->_searcher);
  }

  /**
   * The default method used to retrieve facet data.
   */
  public function fetchTerm($realm_name, array $facet) {
    $facet_items = array();

    $sql = "
      SELECT term
      FROM {". $this->getSearcher() ."_termfreqs}
      WHERE field = '%s'
    ";

    // Gets the cached term frequencies.
    if ($result = db_query($sql, array($facet['field']))) {
      while ($record = db_fetch_object($result)) {
        $term = new Zend_Search_Lucene_Index_Term($record->term, $facet['field']);
        if ($count = facetapi_luceneapi_terms_count($this->_index, $this->_docs, array($term))) {
          $facet_items[$record->term] = array('count' => $count);
        }
      }
    }

    return $facet_items;
  }

  /**
   * Fetches data from facets that filter results by date ranges.
   */
  public function fetchDate($realm_name, array $facet) {

    $sql = "
      SELECT MIN(term) as minimum, MAX(term) as maximum
      FROM {". $this->getSearcher() ."_termfreqs}
      WHERE field = '%s'
    ";

    // Calculates the minimum and maximum values.
    $range = array();
    if ($result = db_query($sql, array($facet['field']))) {
      if ($record = db_fetch_object($result)) {
        $range = array($record->minimum, $record->maximum);
      }
    }

    // Gets all values in the resultset.
    $raw_values = array();
    foreach (facetapi_luceneapi_range_matches_get($this->_index, $range[0], $range[1], TRUE, array($facet['field'])) as $key => $term) {
      if ($count = facetapi_luceneapi_terms_count($this->_index, $this->_docs, array($term))) {
        $raw_values[$term->text] = $count;
      }
    }

    // Sorts by timestamp if there are values, otherwise return an empty array.
    if (!empty($raw_values)) {
      $facet_items = array();
      ksort($raw_values);
    }
    else {
      return array();
    }

    // Gets the active date facets, starts to builds the "parent - child"
    // relationships.
    $parent = NULL;
    $gap = NULL;

    $pattern = '/^'. preg_quote($facet['field alias'], '/') .':/';
    foreach ($this->getActiveFacets($facet['field']) as $filter) {

      // Strips field from value, adds to $facet_items array.
      $value = preg_replace($pattern, '', $filter);
      $facet_items[$value] = array('count' => count($this->_docs));

      // Gets next "gap" increment, mintue being the lowest be can go.
      $filter_range = explode(' TO ', trim($value, '{[]}'));
      if (2 == count($filter_range)) {
        $gap = facetapi_next_date_gap_get(
          facetapi_date_gap_get($filter_range[0], $filter_range[1]),
          FACETAPI_DATE_MINUTE
        );
      }

      // If there is a previous item, there is a parent, uses a reference so the
      // arrays are populated when they are updated.
      if (NULL !== $parent) {
        $facet_items[$parent]['children'][$value] = &$facet_items[$value];
        $facet_items[$value]['parents'][$parent] = $parent;
      }

      // Stores the last value iterated over.
      $parent = $value;
    }

    // Mind the gap! Calculates gap from min and max timestamps.
    $timestamps = array_keys($raw_values);
    if (NULL === $parent) {
      if (count($raw_values) > 1) {
        $gap = facetapi_timestamp_gap_get(min($timestamps), max($timestamps));
      }
      else {
        $gap = FACETAPI_DATE_HOUR;
      }
    }

    // Converts all timestamps to dates in ISO 8601 format.
    $dates = array_combine($timestamps, $timestamps);
    array_walk($dates, 'facetapi_luceneapi_timestamp_convert', $gap);

    // Treat each date as the range start and next data as the range end.
    $range_end = array();
    $previous = NULL;
    foreach (array_unique($dates) as $date) {
      if (NULL !== $previous) {
        $range_end[$previous] = facetapi_next_date_increment_get($previous, $gap);
      }
      $previous = $date;
    }
    $range_end[$previous] = facetapi_next_date_increment_get($previous, $gap);

    // Groups dates by the range they belong to, builds the $facet_items array
    // with the facet counts and formatted range values.
    foreach ($raw_values as $value => $count) {
      $new_value = '['. $dates[$value] .' TO '. $range_end[$dates[$value]] .']';
      if (!isset($facet_items[$new_value])) {
        $facet_items[$new_value] = array('count' => $count);
      }
      else {
        $facet_items[$new_value]['count'] += $count;
      }

      // Adds parent information if not already set.
      if (NULL !== $parent && !isset($facet_items[$new_value]['parents'])) {
        $facet_items[$parent]['children'][$new_value] = &$facet_items[$new_value];
        $facet_items[$new_value]['parents'][$parent] = $parent;
      }
    }

    // Returns the dates.
    return $facet_items;
  }

  /**
   * Overrides the fetch() function, prepopulates the termFreqs cache by calling
   * the facetapi_luceneapi_termfreqs_get() function.
   */
  public function fetch($realm_name, array $facet) {
    $term = new Zend_Search_Lucene_Index_Term(NULL, $facet['field']);
    facetapi_luceneapi_termfreqs_get($this->getSearcher(), $term);
    return parent::fetch($realm_name, $facet);
  }

  /**
   * Returns a normalized array with facet counts included.
   */
  public function getFacetData($realm_name, array $facets) {

    // Opens the index, returns an empty index in it cannot be opened.
    if (!$this->_index = luceneapi_index_open($this->getSearcher(), $errstr)) {
      luceneapi_throw_error($errstr);
      return array();
    }

    // Ugly. We have to re-build the query.
    // @todo Add an API function similar to apachesolr_current_query().

    // Gets query container, appends user query and facet queries.
    if (!$query = luceneapi_query_get('boolean')) {
      throw new LuceneAPI_Exception(t('Error instantiating boolean query.'));
    }

    // Parses user query if a string was passed.
    $user_query = luceneapi_query_parse(search_get_keys(), $this->getSearcher(), 'keys');
    if (!$user_query || form_get_errors()) {
      return array();
    }

    // Adds user query to query container, allows modules to alter the query.
    luceneapi_subquery_add($query, $user_query, 'required', TRUE);
    module_invoke_all('luceneapi_query_alter', $query, $this->getSearcher(), $this->getType());

    // Gets documents matched in the query.
    $this->_docs = facetapi_luceneapi_match_query($this->_index, $query);
    return parent::getFacetData($realm_name, $facets);
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
      return search_get_keys();
    }
    return $this->_keys;
  }
}

