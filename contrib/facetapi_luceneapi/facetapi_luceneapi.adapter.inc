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
   * Returns an array of facet counts.
   */
  public function getFacetData(array $facets) {

    // @todo Error handling.
    if (!$index = luceneapi_index_open($this->getSearcher(), $errstr)) {
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

    // Adds user query to query container.
    luceneapi_subquery_add($query, $user_query, 'required', TRUE);

    // Gets the sort array from the query string, sanitized fields.
    if (isset($_GET['lucenesort'])) {
      $sort = luceneapi_sort_arguments_get($index, $_GET['lucenesort']);
    }
    drupal_alter('luceneapi_sort_arguments', $sort, $this->getSearcher(), $this->getType());

    // Invokes hooks in the appropriate order and executes the query.
    module_invoke_all('luceneapi_query_alter', $query, $this->getSearcher(), $this->getType());

    // initializes array of items
    $data = $facets;
    foreach ($data as $facet_name => &$facet_data) {
      $facet_data['items'] = array();

      // Gets list of "active" values from ones passed through the query string.
      // Does NOT get items for markup facets.
      if ('markup' != $facet_data['type']) {
        if (isset($_GET[$facet_data['element']])) {
          $active_values = (array)$_GET[$facet_data['element']];
        }
        else {
          $active_values = array();
        }
        foreach ($active_values as $active_value) {
          // @todo Port $value === '0' to Search Lucene API
          if ($active_value || $active_value === '0') {
            $facet_data['items'][$active_value]['active'] = 1;
          }
        }
      }
    }
    unset($facet_data);

    // Re-executes search query, we have to execute counts via PHP.
    $hits = luceneapi_find($index, $query, array(), $positive_keys, TRUE);

    // determines counts
    foreach ($hits as $hit) {
      try {
        $document = $index->getDocument($hit->id);
      }
      catch (Exception $e) {
        luceneapi_throw_error($e, WATCHDOG_ERROR, $index->getModule());
        continue;
      }
      foreach ($data as $facet_name => &$facet_data) {
        try {
          if (NULL !== $facet_data['field']) {
            $value = $document->getFieldValue($facet_data['field']);
          }
          else {
            continue;
          }
        }
        catch (Exception $e) {
          continue;
        }
        // @todo Port $value === '0' to Search Lucene API
        if ($value || $value === '0') {

          // Normalizes values to an array.  Items that have multiple values in a
          // field have a 'delimiter' in the facet definition.
          if (isset($facet_data['delimiter'])) {
            $values = explode($facet_data['delimiter'], $value);
          }
          else {
            $values = array($value);
          }

          // increments count for values
          foreach ($values as $value) {
            if (isset($facet_data['items'][$value]['count'])) {
              $facet_data['items'][$value]['count']++;
            }
            else {
              $facet_data['items'][$value]['count'] = 1;
            }
          }
        }
      }
      unset($facet_data);
    }

    foreach ($data as $facet_name => &$facet_data) {
      $facet_data['items'] = array();

      // Hides facet if it is not markup and there are no items, otherwise
      // builds array so we can access it in hook_facetapi_postrender_alter().
      if (empty($facet_data['count'])) {
        if ('markup' != $facet_data['type']) {
          unset($data[$facet_name]);
        }
        else {
          $facet_data['items'][0] = array(
            'theme hook' => 'facetapi_link',
            'text' => '',
            'options' => array(),
            'active' => 0,
            'count' => 0,
            'parents' => array(),
            'children' => array(),
          );
        }
        continue;
      }

      // Adds facet counts, determines which theme function to use.
      foreach ($facet_data['count'] as $value => $count) {
        $options = array();
        if (!isset($facet_data['active'][$value])) {
          $theme_hook = 'facetapi_link';
        }
        else {
          $options['attributes']['class'] = 'active';
          $theme_hook = 'facetapi_link_active';
        }

        // Adds item to array with theme function, counts.
        $facet_data['items'][$value] = array(
          'theme hook' => $theme_hook,
          'text' => $value,
          'options' => $options,
          'count' => $count,
          'parents' => array(),
          'children' => array(),
        );
      }
    }
    unset($facet_data);

    // Returns the "rendered" items.
    return $data;
  }

  /**
   * Applies a default sort to the facet items.
   */
  public function sortDefault(array &$facet_items) {
    uasort($facet_items, 'facetapi_sort_count');
  }

  /**
   * Returns query string variables for a facet link.
   *
   * @todo Make the code nicer.
   */
  public function getFacetQueryString(array $facet, array $values, $active) {
    $qstring = $_GET;
    if (!$active) {
      foreach ($values as $value) {
        if (empty($qstring[$facet['element']]) || is_array($qstring[$facet['element']])) {
          $qstring[$facet['element']][$value] = $value;
        }
        else {
          $qstring[$facet['element']] = array($value => $value);
        }
      }
    }
    else {
      if (is_array($qstring[$facet['element']])) {
        foreach ($values as $value) {
          unset($qstring[$facet['element']][$value]);
        }
      }
      else {
        unset($qstring[$facet['element']]);
      }
    }
    return $qstring;
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

