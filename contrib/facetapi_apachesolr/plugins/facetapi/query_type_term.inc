<?php

/**
 * @file
 * Term query type plugin for the Apache Solr adapter.
 */

/**
 * Plugin for "term" query types.
 */
class FacetapiApachesolrTerm extends FacetapiQueryType {

  /**
   * Returns the query type associated with the plugin.
   *
   * @return string
   *   The query type.
   */
  static public function getType() {
    return 'term';
  }

  /**
   * Adds the filter to the query object.
   *
   * @param stdClass $query
   *   An object containing the query in the backend's native API.
   */
  public function execute($query) {
    $searcher = $this->adapter->getSearcher();

    // Adds the operator parameter.
    //$operator = facetapi_get_setting('operator', $searcher, '', $this->facet['name']);
    $operator = 'and';
    $ex = (FACETAPI_OPERATOR_OR != $operator) ? '' : "{!ex={$this->facet['field']}}";
    $query->params['facet.field'][] = $ex . $this->facet['field'];

    // Adds "hard limit" parameter to prevent too many return values.
    //$limit = facetapi_get_setting('hard_limit', $searcher, '', $this->facet['name']);
    $limit = 50;
    $query->params['f.' . $this->facet['field'] . '.facet.limit'] = ($limit !== NULL) ? (int) $limit : 20;
  }

  /**
   * Initializes the facet's build array.
   *
   * @return array
   *   The initialized render array.
   */
  public function build() {
    $build = array();
    if ($response = apachesolr_static_response_cache()) {
      $values = (array) $response->facet_counts->facet_fields->{$this->facet['field']};
      foreach ($values as $value => $count) {
        $build[$value] = array('#count' => $count);
      }
    }
    return $build;
  }
}
