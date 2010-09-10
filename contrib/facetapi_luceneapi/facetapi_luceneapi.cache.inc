<?php
// $Id$

/**
 * @file
 * Term frequency cache functions.
 */

/**
 * Executes a match query, returns array keyed by document IDs.  This method
 * skips sorting, so it more efficient than calling luceneapi_find.
 *
 * @param $index
 *   A Zend_Search_Lucene_Interface object.
 * @param $query
 *  A Zend_Search_Lucene_Search_Query object.
 * @return
 *   An array keyed by document IDs.
 */
function facetapi_luceneapi_match_query(Zend_Search_Lucene_Interface $index, Zend_Search_Lucene_Search_Query $query) {
  $docs = array();
  try {
    $query = $query->rewrite($index)->optimize($index);
    $query->execute($index);
    $docs = $query->matchedDocs();
  }
  catch (Exception $e) {
    luceneapi_throw_error($e, WATCHDOG_ERROR, 'facetapi_luceneapi');
  }
  return $docs;
}

/**
 * Helper function to get the wildcard prefix.
 *
 * @param $text
 *   A string containing the text prefix is being extracted from.
 * @return
 *   A string containing the prefix.
 */
function facetapi_luceneapi_prefix_get($text) {
  $q_pos = strpos($text, '?');
  $a_pos = strpos($text, '*');
  if ($q_pos !== false) {
    if ($a_pos !== false) {
      return substr($text, 0, min($q_pos, $a_pos));
    }
    return substr($text, 0, $q_pos);
  }
  elseif ($a_pos !== false) {
    return substr($text, 0, $a_pos);
  }
  return $text;
}

/**
 * Reads termfrequencies from the lookup table to avoid having to calculate it
 * on the fly, which is very performance intensive.
 *
 * @param $searcher
 *   A string containing the machine readable name of the searcher module.
 * @param $term
 *   A Zend_Search_Lucene_Index_Term object.
 * @return
 *   An array keyed by Lucene document ID to term frequency.
 */
function facetapi_luceneapi_termfreqs_get($searcher, Zend_Search_Lucene_Index_Term $term) {
  static $cache = array();

  // If cache is not populated for the field, reads the field's cached data.
  if (!isset($cache[$searcher][$term->field])) {

    $variable = 'facetapi:termfreqs_cached:'. $searcher .':::'. $term->field;
    if (!variable_get($variable, FALSE)) {
      facetapi_luceneapi_termfreqs_populate($searcher, $term->field);
      variable_set($variable, TRUE);
    }

    // Reads term frequencies from the database.
    $cache[$searcher][$term->field] = array();
    $table = '{'. $searcher .'_termfreqs}';
    $sql = "SELECT term, termfreqs FROM $table WHERE field = '%s'";
    if ($result = db_query($sql, array($term->field))) {
      while ($record = db_fetch_object($result)) {
        $cache[$searcher][$term->field][$record->term] = array(
          'termfreqs' => $record->termfreqs,
          'unserialized' => FALSE,
        );
      }
    }
  }

  // Returns array of termfreqs, unserializes if necessary.
  if (isset($cache[$searcher][$term->field][$term->text])) {
    $data = &$cache[$searcher][$term->field][$term->text];
    if (empty($data['unserialized'])) {
      $data['termfreqs'] = unserialize($data['termfreqs']);
      $data['unserialized'] = TRUE;
    }
    return $data['termfreqs'];
  }
  else {
    return array();
  }
}

/**
 * Gets the filter counts for an array of terms.
 *
 * @param $index
 *   A LuceneAPI_Search_Lucene_Proxy object, such as the one returned by the
 *   luceneapi_index_open() function.
 * @param $docs
 *   An array of documents keyed by docuent ID to score.
 * @param $terms
 *   An array of Zend_Search_Lucene_Index_Term objects.
 */
function facetapi_luceneapi_terms_count(Zend_Search_Lucene_Proxy $index, array $docs, array $terms) {
  $freqs = array();

  // Reading from a termFreqs cache table is MUCH more efficient than
  // calculating them on the fly for every search.
  foreach ($terms as $term) {
    $freqs += facetapi_luceneapi_termfreqs_get('luceneapi_node', $term);
  }

  // NOTE: This is surprisingly efficient.
  return count(array_intersect_key($docs, $freqs));
}

/**
 * Rewrites a wildcard query into primitive terms.
 *
 * @param $index
 *   A Zend_Search_Lucene_Interface object, such as the one returned by the
 *   luceneapi_index_open() function.
 * @param $text
 *   A string containing the text being parsed.
 * @param $fields
 *   An array of fields the terms are matched against.
 * @return
 *   An array of Zend_Search_Lucene_Index_Term objects.
 */
function facetapi_luceneapi_wildcard_matches_get(Zend_Search_Lucene_Interface $index, $text, array $fields = array('contents')) {

  // Prevents too many terms from being extracted.
  $max_terms = Zend_Search_Lucene::getTermsPerQueryLimit();

  // Gets the word prefix.
  $prefix = facetapi_luceneapi_prefix_get($text);
  $prefix_len = strlen($prefix);
  $pattern = '/^'. str_replace(array('\\?', '\\*'), array('.', '.*'), preg_quote($text, '/')) .'$/';

  // Caclulates matches.
  $matches = array();
  foreach ($fields as $field) {
    $index->resetTermsStream();
    $index->skipTo(new Zend_Search_Lucene_Index_Term($prefix, $field));
    while ($index->currentTerm() !== NULL && $index->currentTerm()->field == $field) {
      if ($prefix != '' && substr($index->currentTerm()->text, 0, $prefix_len) != $prefix) {
        break;
      }
      if (preg_match($pattern, $index->currentTerm()->text) === 1) {
        $matches[] = $index->currentTerm();
        // Terms per query limit reached.
        if ($max_terms != 0 && count($matches) > $max_terms) {
          throw new Zend_Search_Lucene_Exception('Terms per query limit is reached.');
        }
      }
      $index->nextTerm();
    }
    $index->closeTermsStream();
  }

  return $matches;
}

/**
 * Rewrites a range query into primitive terms.
 *
 * @param $index
 *   A Zend_Search_Lucene_Interface object, such as the one returned by the
 *   luceneapi_index_open() function.
 * @param $lower
 *   A string containing the lower boundary.
 * @param $upper
 *   A string containing the upper boundary.
 * @param $inclusive
 *   A boolean flagging whether to include the upper term in the result set.
 * @param $fields
 *   An array of fields the terms are matched against.
 * @return
 *   An array of Zend_Search_Lucene_Index_Term objects.
 */
function facetapi_luceneapi_range_matches_get(Zend_Search_Lucene_Interface $index, $lower, $upper, $inclusive = FALSE, array $fields = array('contents')) {

  // Prevents too many terms from being extracted.
  $max_terms = Zend_Search_Lucene::getTermsPerQueryLimit();

  $matches = array();
  foreach ($fields as $field) {
    $index->resetTermsStream();

    if ($lower !== NULL) {
      $lower_term = new Zend_Search_Lucene_Index_Term($lower, $field);
      $index->skipTo($lower_term);

      if (!$inclusive && $index->currentTerm() == $lower_term) {
        $index->nextTerm();
      }
    }
    else {
      $index->skipTo(new Zend_Search_Lucene_Index_Term('', $field));
    }

    if ($upper !== NULL) {
      // Walk up to the upper term
      $upper_term = new Zend_Search_Lucene_Index_Term($upper, $field);

      while (
        $index->currentTerm() !== NULL &&
        $index->currentTerm()->field == $field &&
        $index->currentTerm()->text < $upper_term->text
      ) {

        $matches[] = $index->currentTerm();
        if ($max_terms != 0 && count($matches) > $max_terms) {
          throw new Zend_Search_Lucene_Exception('Terms per query limit is reached.');
        }
        $index->nextTerm();
      }

      if ($inclusive && $index->currentTerm() == $upper_term) {
        // Include upper term into result
        $matches[] = $upper_term;
      }
    }
    else {
      // Walk up to the end of field data
      while ($index->currentTerm() !== NULL && $index->currentTerm()->field == $field) {
        $matches[] = $index->currentTerm();
        if ($max_terms != 0 && count($matches) > $max_terms) {
          throw new Zend_Search_Lucene_Exception('Terms per query limit is reached.');
        }
        $index->nextTerm();
      }
    }

    $index->closeTermsStream();
  }

  return $matches;
}

/**
 * Populates the termfreqs() database for the specified fields.
 *
 * @param $searcher
 *   A string containing the machine readable name of the searcher module.
 * @param $fields
 *   An array containing the fields the termFreqs cache is being populated for.
 * @param &$context
 *   An optional array containing the batch context when using this function as
 *   a batch operation.
 * @return
 *   NULL
 */
function facetapi_luceneapi_termfreqs_populate($searcher, $fields = NULL, &$context = NULL) {
  if (!$index = luceneapi_index_open($searcher)) {
    return;
  }

  // Gets fields, defaults to all enabled facets.
  if (NULL === $fields) {
    $fields = array();
    foreach (facetapi_enabled_facets_get('luceneapi_node') as $facet) {
      $fields[$facet['field']] = $facet['field'];
    }
  }
  else {
    $fields = drupal_map_assoc((array)$fields);
  }

  // Sets message if this is a batch process.
  if (NULL !== $context) {
    $context['message'] = format_plural(
      count($fields),
      'Populating termFreqs cache for the %fields field',
      'Populating termFreqs cache for the %fields fields',
      array('%fields' => join(', ', $fields))
    );
  }

  // Gets terms from index.
  $terms = array();
  foreach ($index->terms() as $term) {
    if (isset($fields[$term->field])) {
      $terms[$term->field][$term->text] = $index->termFreqs($term);
    }
  }

  // Populates termfreqs cache for all passed fields.
  $table = '{'. $searcher .'_termfreqs}';
  foreach ($terms as $field => $field_terms) {

    // Builds placeholders and values.
    $values = $placeholders = array();
    foreach ($field_terms as $term => $termfreqs) {
      $placeholders[] = "('%s', '%s', '%s')";
      $values[] = $term;
      $values[] = $field;
      $values[] = serialize($termfreqs);
    }

    // If there are values, inserts data.
    if (!empty($values)) {
      $sql = "DELETE FROM $table WHERE field = '%s'";
      db_query($sql, array($field));

      // @todo Batch in groups of 100? 1000?  OK as is?
      $sql = "INSERT INTO $table (term, field, termfreqs) VALUES ". join(',', $placeholders);
      db_query($sql, $values);
    }
  }
}
