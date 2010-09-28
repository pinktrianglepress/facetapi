<?php
// $Id$

/**
 * @file
 * Various callbacks referenced in facet definitions.
 */

/**
 * Converts UIDs to username.
 *
 * @param $uids
 *   An array containing the user IDs.
 *
 * @return
 *   An array mapping user IDs to usernames.
 */
function facetapi_callback_uid_map(array $uids) {
  $sql = '
    SELECT uid AS id, name
    FROM {users}
    WHERE uid IN (!placeholders)
  ';

  $map = facetapi_map_query($sql, $uids);
  if (isset($map[0])) {
    $map[0] = variable_get('anonymous', t('Anonymous'));
  }
  return $map;
}

/**
 * Converts machine readable content types to display names.
 *
 * @param $types
 *   An array containing the machine readable content types.
 *
 * @return
 *   An array mapping the machine readable content types to the display names.
 */
function facetapi_callback_type_map(array $types) {
  $sql = '
    SELECT type AS id, name
    FROM {node_type}
    WHERE type IN (!placeholders)
  ';
  return facetapi_map_query($sql, $types, 'varchar');
}

/**
 * Maps a taxonomy ID to a term name.
 *
 * @param $tids
 *   An array containing the term IDs.
 *
 * @return
 *   An array mapping the term IDs to the display name.
 */
function facetapi_callback_taxonomy_map(array $tids) {
  $sql = '
    SELECT tid AS id, name
    FROM {term_data}
    WHERE tid IN (!placeholders)
  ';
  return facetapi_map_query($sql, $tids);
}

/**
 * Converts machine readable languages to
 *
 * @param $languages
 *   An array containing the machine readable content types.
 *
 * @return
 *   An array mapping the machine readable content types to the display names.
 */
function facetapi_callback_language_map(array $languages) {
  $language_list = language_list();
  $map = array();
  foreach ($languages as $language) {
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
 * Converts date ranges to human readable dates.
 *
 * @param $ranges
 *   An array containing the date ranges.
 *
 * @return
 *   An array mapping the ranges to nice display dates.
 */
function facetapi_callback_date_map(array $ranges) {
  $map = array();
  foreach ($ranges as $value) {
    $range = explode(' TO ', trim($value, '{[]}'));
    if (2 == count($range)) {
      $gap = facetapi_date_gap_get($range[0], $range[1]);
      $map[$value] = facetapi_date_format($range[0], $gap);
    }
  }
  return $map;
}

/**
 * Gets parent information for taxonomy terms.
 *
 * @param &$tids
 *   An array containing the term ids.
 *
 * @return
 *   An associative array keyed by term ID to parent ID.
 */
function facetapi_callback_taxonomy_hierarchy(array $tids) {
  $placeholders = db_placeholders($tids);

  $sql = "
    SELECT tid, parent
    FROM {term_hierarchy}
    WHERE parent > 0 AND (tid IN ($placeholders) OR parent IN ($placeholders))
  ";

  // Executes query to get parents, relates the terms to one another.
  $parents = array();
  if ($result = db_query($sql, array_merge($tids, $tids))) {
    while ($record = db_fetch_object($result)) {
      $parents[$record->tid][] = $record->parent;
    }
  }
  return $parents;
}

/**
 * Returns values for the language facet.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   An array of values passed as options to the form element.
 */
function facetapi_callback_language_values(array $facet) {
  $options = array('und' => t('Language neutral'));
  foreach (language_list() as $code => $language) {
    $options[$code] = check_plain($language->name);
  }
  return $options;
}

/**
 * Returns values for the language facet.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   An array of values passed as options to the form element.
 */
function facetapi_callback_type_values(array $facet) {
  return array_map('check_plain', node_get_types('names'));
}

/**
 * Returns all user names.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   An array of values passed as options to the form element.
 */
function facetapi_callback_user_values(array $facet) {
  $users = array();
  $sql = "SELECT uid, name FROM {users} WHERE status = 1";
  if ($result = db_query($sql)) {
    while ($record = db_fetch_object($result)) {
      $users[$record->uid] = check_plain($record->name);
    }
  }
  return $users;
}

/**
 * Returns values for vocabulary facets.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   An array of values passed as options to the form element.
 */
function facetapi_callback_taxonomy_values(array $facet) {
  // Extracts vid, loads vocabulary object, returns empty array on any error.
  if (!preg_match('/^vocabulary_(\d+)$/', $facet['name'], $match)) {
    return array();
  }
  if (!$vocabulary = taxonomy_vocabulary_load($match[1])) {
    return array();
  }

  // Builds options from taxonomy tree.
  $options = array();
  $tree = taxonomy_get_tree($vocabulary->vid);
  if ($tree && (count($tree) > 0)) {
    $options[$vocabulary->name] = array();
    foreach ($tree as $term) {
      $options[$vocabulary->name][$term->tid] = check_plain(str_repeat('-', $term->depth) . $term->name);
    }
  }
  return $options;
}

/**
 * Callback that returns the minimum date in the node table.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   The minimum time in the node table.
 */
function facetapi_callback_min_date(array $facet) {
  // @todo is this the appropriate escaping function??
  $table = db_escape_table($facet['field']);
  $sql = "SELECT MIN($table) FROM {node} WHERE status = 1";
  if ($result = db_query($sql)) {
    if ($date = db_result($result)) {
      return $date;
    }
  }
  return FALSE;
}

/**
 * Callback that returns the minimum value in the node table.
 *
 * @param $facet
 *   An array containing the facet definition.
 *
 * @return
 *   The minimum time in the node table.
 */
function facetapi_callback_max_date(array $facet) {
  // @todo is this the appropriate escaping function??
  $table = db_escape_table($facet['field']);
  $sql = "SELECT MAX($table) FROM {node} WHERE status = 1";
  if ($result = db_query($sql)) {
    if ($date = db_result($result)) {
      return $date;
    }
  }
  return FALSE;
}
