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
   * Stores all active facets.
   */
  protected $_filters;

  /**
   * Returns a boolean flagging whether $this->_searcher executed a search.
   */
  public function searchExecuted() {
    return (luceneapi_search_executed() == $this->_searcher);
  }

  /**
   * The default method used to retrieve facet data.
   */
  public function fetchDefault($realm_name, array $facet) {
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

      // Gets next "gap".
      $filter_range = explode(' TO ', trim($value, '{[]}'));
      if (2 == count($filter_range)) {
        $gap = facetapi_date_gap_get($filter_range[0], $filter_range[1]);
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

    switch ($gap) {
      case FACETAPI_DATE_MONTH:
        $format = 'Y-m-01\T00:00:00\Z';
        break;
      case FACETAPI_DATE_DAY:
        $format =  'Y-m-d\T00:00:00\Z';
        break;
      case FACETAPI_DATE_HOUR:
        $format =  'Y-m-d\TH:00:00\Z';
        break;
      case FACETAPI_DATE_MINUTE:
        $format =  'Y-m-d\TH:i:00\Z';
        break;
      case FACETAPI_DATE_SECOND:
        $format =  'Y-m-d\TH:i:s\Z';
        break;
      default:
        $format =  'Y-m-d\TH:i:s\Z';
        break;
    }

    // Converts all timestamps to dates in ISO 8601 format.
    $dates = array_combine($timestamps, $timestamps);
    array_walk($dates, 'facetapi_luceneapi_timestamp_convert', $format);

    // Treat each date facet as a range start, and use the next date facet as
    // range end.  At this point, the dates should.
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

    // Re-executes search query, gets docuent IDs.
    // NOTE: This is more efficient when caching is enabled.
    /*
    $this->_docs = array();
    $hits = luceneapi_find($this->_index, $query, array(), $positive_keys, TRUE);
    foreach ($hits as $hit) {
      $this->_docs[$hit->id] = $hit->score;
    }
    //*/

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
   * Returns query string for the facet.
   */
  public function getFacetQueryString(array $facet, array $values, $active) {
    $filters = $this->_filters;

    // Adds or removes filters from the array.
    foreach ($values as $value) {
      if (!$active) {
        $filters[$facet['field alias']][$value] = $facet['field alias'] .':'. $value;
      }
      else {
        unset($filters[$facet['field alias']][$value]);
      }
    }

    // Builds the query string.
    $active_filters = array();
    foreach ($filters as $field => $field_filters) {
      foreach ($field_filters as $value => $filter) {
        $active_filters[] = $filter;
      }
    }

    // Buids query string.
    $qstring = $_GET;
    $filter_string = join(' ', $active_filters);
    if ($filter_string) {
      $qstring['filters'] = $filter_string;
    }
    else {
      unset($qstring['filters']);
    }
    return drupal_query_string_encode($qstring, array('q', 'page'));
  }

  /**
   * Returns an array of active facet values in a field.
   */
  public function getActiveFacets($field) {
    if (NULL === $this->_filters) {
      $this->_populateActiveFilters();
    }
    return isset($this->_filters[$field]) ? $this->_filters[$field] : array();
  }

  /**
   * Returns an integer flagging whether the facet is active.
   */
  public function facetActive($field, $value) {
    if (NULL === $this->_filters) {
      $this->_populateActiveFilters();
    }
    return isset($this->_filters[$field][$value]);
  }

  /**
   * Populates filters.
   */
  protected function _populateActiveFilters() {
    $this->_filters = array();
    $string = isset($_GET['filters']) ? (string)$_GET['filters'] : '';
    foreach (facetapi_enabled_facets_get($this->getSearcher()) as $facet) {
      $field = preg_quote($facet['field alias'], '/');

      // Patterns that pull queries from filter string.
      $patterns = array(
        // Range queries.  The "TO" is case-sensitive.
        '/(^| |-)'. $field .':([\[\{](\S+) TO (\S+)[\]\}])/',
        // Match quoted values.
        '/(^| |-)'. $field .':"([^"]*)"/',
        // Match unquoted values.
        '/(^| |-)'. $field .':([^ ]*)/',
      );

      // Pulls data from the query string.
      foreach ($patterns as $pattern) {
        if (preg_match_all($pattern, $string, $matches, PREG_SET_ORDER)) {
          foreach ($matches as $match) {
            $value = trim($match[2]);
            $this->_filters[$field][$value] = trim($match[0]);

            // Update the local copy of $filters by removing the match.  If we
            // don't do this, part of the range queries will be matched as term
            // queries.
            $string = str_replace($match[0], '', $string);
          }
        }
      }
    }
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

