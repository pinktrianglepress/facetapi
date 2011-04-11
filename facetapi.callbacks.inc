<?php

/**
 * @file
 * Callbacks defined in hook_facetapi_facet_info() implementations.
 */



/**
 * Map callback for node types.
 */
function facetapi_map_bundle(array $values) {
  $names = node_type_get_names();
  return array_intersect_key($names, array_flip($values));
}

/**
 * Map callback for node authors.
 */
function facetapi_map_author(array $values) {
  $result = db_select('users', 'u')
    ->fields('u', array('uid', 'name'))
    ->condition('uid', $values, 'IN')
    ->execute();

  $map = array();
  while ($record = $result->fetchAssoc()) {
    $map[$record['uid']] = $record['name'];
  }
  if (isset($map[0])) {
    $map[0] = variable_get('anonymous', t('Anonymous'));
  }
  return $map;
}

/**
 * Map callback for languages.
 */
function facetapi_map_language(array $values) {
  $map = array();
  $language_list = language_list();
  foreach ($values as $language) {
    if (isset($language_list[$language])) {
      $map[$language] = $language_list[$language]->name;
    }
    else {
      $map[$language] = t('Language neutral');
    }
  }
  return $map;
}

/**
 * Callback that returns the minimum date in the node table.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   The minimum time in the node table.
 *
 * @todo Cache this value.
 */
function facetapi_get_min_date(array $facet) {
  $query = db_select('node', 'n')->condition('status', 1);
  $query->addExpression('MIN(' . $facet['name'] . ')', 'max');
  return $query->execute()->fetch()->max;
}

/**
 * Callback that returns the minimum value in the node table.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   The minimum time in the node table.
 *
 * @todo Cache this value.
 */
function facetapi_get_max_date(array $facet) {
  $query = db_select('node', 'n')->condition('status', 1);
  $query->addExpression('MAX(' . $facet['name'] . ')', 'max');
  return $query->execute()->fetch()->max;
}
