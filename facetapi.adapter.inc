<?php

/**
 * @file
 * Defines classes used by the FacetAPI module.
 */

/**
 * Stores facet data, provides methods that build the facet's render array.
 */
class FacetapiFacet {

  /**
   * A FacetapiAdapter object.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * An array containing the facet definition.
   *
   * @var array
   */
  protected $facet;

  /**
   * An array of active facets items.
   *
   * @var array
   */
  protected $active;

  /**
   * The build array for the facet items.
   *
   * @var array
   */
  protected $build;

  /**
   * Constructor, sets adapter and facet definition.
   *
   * @param $adapter
   *   A FacetapiAdapter object.
   * @param $facet
   *   An array containing the facet definition.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    $this->setAdapter($adapter)->setFacet($facet);
  }

  /**
   * Sets adapter object.
   *
   * @param FacetapiAdapter $adapter
   *   The Facet API adapter object.
   *
   * @return FacetapiFacet
   *   An instance of this class.
   */
  public function setAdapter(FacetapiAdapter $adapter) {
    $this->adapter = $adapter;
    return $this;
  }

  /**
   * Returns the adapter object.
   *
   * @return FacetapiAdapter
   *   The adapter object.
   */
  public function getAdapter() {
    return $this->adapter;
  }

  /**
   * Sets the facet definition.
   *
   * @return FacetapiFacet
   *   An instance of this class.
   */
  public function setFacet(array $facet) {
    $this->facet = $facet;
    return $this;
  }

  /**
   * Returns the facet definition.
   *
   * @return array
   *   An array containing the facet definition.
   */
  public function getFacet() {
    return $this->facet;
  }

  /**
   * Returns the facet's active items.
   *
   * @return array
   *   The facet's active items.
   */
  public function getActiveItems() {
    if (!isset($this->active)) {
      $this->processActiveItems();
    }
    return $this->active;
  }

  /**
   * Returns an array of the facet's active item values.
   *
   * This value is useful as a form element's default value.
   *
   * @return array
   *   The facet values keyed by position.
   */
  public function getActiveValues() {
    if (!isset($this->active)) {
      $this->processActiveItems();
    }
    $values = array();
    foreach ($this->active as $value => $item) {
      $values[$item['pos']] = $value;
    }
    if (!empty($values)) {
      $values = array_combine($values, $values);
    }
    return $values;
  }

  /**
   * Tests whether a facet item is active by passing it's value.
   *
   * NOTE: This method returns an integer instead of a boolean because the value
   * is used by the Facet API's custom sorting functions. It ends up being less
   * code to compare integers than booleans.
   *
   * @param string $value
   *   The facet item's value.
   *
   * @return
   *   Returns 1 if the item is active, 0 if it is inactive.
   */
  public function itemActive($value) {
    return (int) isset($this->active[$value]);
  }

  /**
   * Helper function that returns the query string variables for a facet item.
   *
   * @param $values
   *   An array containing the item's values being added to or removed from the
   *   query string dependent on whether or not the item is active.
   * @param $active
   *   An integer flagging whether the item is active or not.
   *
   * @reutrn
   *   An array containing the query string variables.
   */
  public function getQueryString(array $values, $active) {

    // Gets field alias for readability.
    $field_alias = $this->facet['field alias'];

    // Builds array of query string variables.
    $qstring = $_GET;
    foreach ($values as $value) {
      if ($active && isset($this->active[$value])) {
        unset($qstring[$field_alias][$this->active[$value]['pos']]);
      }
      elseif (!$active) {
        if (!isset($qstring[$field_alias])) {
          $qstring[$field_alias] = array();
        }
        elseif (!is_array($qstring[$field_alias])) {
          $qstring[$field_alias] = array((string) $qstring[$field_alias]);
        }
        $qstring[$field_alias][] = $value;
      }
    }

    return $qstring;
  }

  /**
   * Returns the facet's render array.
   *
   * @param $realm
   *   An array containing the realm definition.
   *
   * @return
   *   The facet's build array.
   */
  public function build(array $realm) {

    // Builds render array for facet items if necessary.
    if (!isset($this->build)) {
      $this->build = $this->buildItems();
      $this->processHierarchy($this->build)->processQueryStrings($this->build);
    }

    // Gets searcher since we use it a lot, gets field alias for readability.
    $searcher = $this->adapter->getSearcher();
    $field_alias = $this->facet['field alias'];

    // Initializes render array.
    $build = array(
      '#title' => $this->facet['title'],
      '#description' => $this->facet['description'],
      '#weight' => $this->facet['weight'],
      '#adapter' => $this->adapter,
      '#realm_name' => $realm['name'],
      '#facet' => $this->facet,
      $field_alias => $this->build,
    );

    // Adds identifiers to facet.
    $build['#attributes'] = array(
      'class' => "facetapi-facet-{$this->facet['name']}",
      'id' => "facetapi-facet-$searcher-{$realm['name']}-{$this->facet['name']}",
    );

    // Applies sorting algorithms.
    $this->sorts = facetapi_get_facet_sorts($this->adapter, $realm, $this->facet);
    $this->sort($build[$field_alias]);
    unset($this->sorts);

    // Gets available widgets.
    $widgets = facetapi_get_widget_info(array(
      'realm' => $realm,
      'facet' => $this->facet,
    ));

    // Gets widget from settings, finds default if necessary.
    $widget_name = facetapi_get_facet_widget($widgets, $searcher, $realm, $this->facet);

    // Initializes JavaScript settings.
    $facet_settings = array(
      'searcher' => $searcher,
      'type' => $this->adapter->getType(),
      'realmName' => $realm['name'],
      'facetName' => $this->facet['name'],
      'widget' => $widget_name,
      'queryType' => $this->facet['query type'],
    );

    // Passes render array, JavaScript settings to widget.
    $key = $this->facet['field alias'];
    if (NULL !== $widget_name && isset($widgets[$widget_name])) {
      $build['#widget'] = $widgets[$widget_name];
      if (facetapi_file_include($build['#widget'])) {
        $build['#widget']['callback']($build, $key, $facet_settings);
      }
    }

    // Adds JavaScript settings.
    $settings['facetapi']['facets'][] = $facet_settings;
    drupal_add_js($settings, 'setting');

    return array($key => $build);
  }

  /**
   * Finds and stores the facet's active items.
   *
   * @return
   *   An instance of this class.
   */
  public function processActiveItems() {
    $this->active = array();

    // Bails if the facet isn't enabled in any realm.
    if (!facetapi_facet_enabled($this->adapter->getSearcher(), NULL, $this->facet['name'])) {
      return $this;
    }

    // Gets active items from query string, normalizes to an array.
    $field_alias = $this->facet['field alias'];
    if (isset($_GET[$field_alias])) {
      if (is_array($_GET[$field_alias])) {
        $data = $_GET[$field_alias];
      }
      else {
        $data = array((string) $_GET[$field_alias]);
      }
    }
    else {
      $data = array();
    }

    // Allows hooks to add additional information to the active item. For
    // example, range queries extract the start and end values from the item.
    $hook = 'facetapi_value_'. $this->facet['query type'];
    foreach ($data as $key => $value) {
      $this->active[$value] = array(
        'pos' => $key,
        'value' => $value,
      );
      drupal_alter($hook, $this->active[$value], $this->adapter);
    }

    return $this;
  }

  /**
   * Builds the render array for the facet's items.
   *
   * @return
   *   A render array for the facet's items.
   */
  public function buildItems() {
    $build = array();

    // Build array defaults.
    $defaults = array(
      '#markup' => '',
      '#indexed_value' => '',
      '#count' => 0,
      '#active' => 0,
      '#item_parents' => array(),
      '#item_children' => array(),
    );

    // Builds render arrays for each item.
    if (NULL !== $this->facet['field']) {
      $build = $this->adapter->getFacetQuery($this->facet)->build();
    }
    else {
      $items = array();
    }
    foreach (element_children($items) as $value) {
      $item_defaults = array(
        '#markup' => $value,
        '#indexed_value' => $value,
        '#active' => $this->itemActive($value),
      );

      // This seems silly, but it maintains the references to the child items
      // stored in the #item_children property.
      $items[$value] = array_merge($defaults, $item_defaults, $items[$value]);
      $build[$value] = &$items[$value];
    }

    // Maps the IDs to human readable values via the mapping callback.
    if (!empty($this->facet['map callback']) && function_exists($this->facet['map callback'])) {
      $map = call_user_func($this->facet['map callback'], array_keys($build));
      array_walk($build, 'facetapi_ids_replace', $map);
    }

    return $build;
  }

  /**
   * Processes hierarchical relationships between the facet items.
   *
   * @param &$build
   *   The facet's render array.
   *
   * @return
   *   An instance of this class.
   */
  public function processHierarchy(&$build) {

    // Builds the hierarchy information if the hierarchy callback is defined.
    if (!empty($this->facet['hierarchy callback']) && !empty($build)) {
      $parents = $this->facet['hierarchy callback'](array_keys($build));
      foreach ($parents as $value => $parents) {
        foreach ($parents as $parent) {
          if (isset($build[$parent]) && isset($build[$value])) {
            // Use a reference so we see the updated data.
            $build[$parent]['#item_children'][$value] = &$build[$value];
            $build[$value]['#item_parents'][$parent] = $parent;
          }
        }
      }
    }

    // Tests whether parents have an active child.
    // @todo: Can we make this more efficient?
    do {
      $active = 0;
      foreach ($build as $value => $item) {
        if ($item['#active'] && !empty($item['#item_parents'])) {
          // @todo Can we build facets with multiple parents? Core taxonomy
          // form cannot, so we will need a check here.
          foreach ($item['#item_parents'] as $parent) {
            if (!$build[$parent]['#active']) {
              $active = $build[$parent]['#active'] = 1;
            }
          }
        }
      }
    } while ($active);

    // Strips children whose parents are inactive.
    $build = array_filter($build, 'facetapi_filter_inactive_parent');

    // Returns instance of this class.
    return $this;
  }

  /**
   * Initializes the render array's query string variables.
   *
   * @param &$build
   *   The facet's render array.
   *
   * @return
   *   An instance of this class.
   */
  function processQueryStrings(array &$build) {
    foreach ($build as $value => &$item) {
      $values = array($value);

      // If the item is active an has children, gets the paths for the children.
      // Merges child values with this facet item's value so that unclicking the
      // parent deactivated the children as well.
      if (!empty($item['#active']) && !empty($item['#item_children'])) {
        $this->processQueryStrings($item['#item_children']);
        $values = array_merge(facetapi_get_child_values($item['#item_children']), $values);
      }

      // Formats query string for facet item, sets theme function.
      $item['#query'] = $this->getQueryString($values, $item['#active']);
    }

    // Returns instance of this calss.
    return $this;
  }

  /**
   * Sorts the facet's build array.
   *
   * @param &$build
   *   An array containing the render array.
   */
  protected function sort(&$build) {
    foreach (element_children($build) as $value) {
      if (!empty($build[$value]['#item_children'])) {
        $this->sort($build[$value]['#item_children']);
      }
    }
    uasort($build, array($this, 'sortCallback'));
  }

  /**
   * Callback for uasort() that applies each sort in the order specified in the
   * admin interface.
   */
  protected function sortCallback(array $a, array $b) {
    $return = 0;
    foreach ($this->sorts as $sort) {
      if ($return = $sort['callback']($a, $b)) {
        if (SORT_DESC == $sort['order']) {
          $return *= -1;
        }
        break;
      }
    }
    return $return;
  }
}

/**
 * Helper function to execute a map query. A map query is useful for converting
 * unique identifiers to human readable values, for example a uid to username.
 *
 * @param $sql
 *   A string containing the SQL query mapping the ID to another value. The
 *   query must select the "id" and "name" fields.
 * @param $ids
 *   An array containing the IDs being mapped.
 * @param $type
 *   The Schema API type of the ID field (e.g. 'int', 'text', or 'varchar').
 *
 * @return
 *   An array of mapped IDs.
 */
function facetapi_map_query($sql, array $ids, $type = 'int') {
  $map = array();
  if (!empty($ids)) {
    $sql = str_replace('!placeholders', db_placeholders($ids, $type), $sql);
    if ($result = db_query($sql, $ids)) {
      while ($record = db_fetch_object($result)) {
        $map[$record->id] = $record->name;
      }
    }
  }
  return $map;
}


/**
 * Replaces ID's with a mapped value, useful as a callback for array_walk().
 *
 * @param &$item
 *   An array containing the facet item.
 * @param $key
 *   An integer containing the array key, or the ID being mapped to a value.
 * @param $map
 *   An array containing the mapped values.
 */
function facetapi_replace_ids(array &$item, $key, array $map) {
  if (isset($map[$key])) {
    $item['#value'] = $map[$key];
  }
}

/**
 * Recursive function that returns an array of values for all descendants of a
 * facet item.
 *
 * @param $build
 *   A render array containing the facet item's children.
 *
 * @return
 *   An array containing the values of all descendants.
 */
function facetapi_get_child_values(array $build) {
  $values = array_keys($build);
  foreach ($build as $item) {
    if (!empty($item['#item_children'])) {
      $values = array_merge(facetapi_get_child_values($item['#item_children']), $values);
    }
  }
  return $values;
}

/**
 * Callback for array_filter() that strips out all children whose parents are
 * inactive.
 *
 * @param $build
 *   The facet item's render array.
 *
 * @return
 *   A boolean flagging whether the value should remain in the array.
 */
function facetapi_filter_inactive_parent(array $build) {
  return empty($build['#item_parents']);
}

/**
 * Sorts by whether or not a facet is active.
 */
function facetapi_sort_active(array $a, array $b) {
  $a_active = (isset($a['#active'])) ? $a['#active'] : 0;
  $b_active = (isset($b['#active'])) ? $b['#active'] : 0;
  if ($a_active == $b_active) {
    return 0;
  }
  return ($a_active < $b_active) ? -1 : 1;
}

/**
 * Sorts by facet count.
 */
function facetapi_sort_count(array $a, array $b) {
  $a_count = (isset($a['#count'])) ? $a['#count'] : 0;
  $b_count = (isset($b['#count'])) ? $b['#count'] : 0;
  if ($a_count == $b_count) {
    return 0;
  }
  return ($a_count < $b_count) ? -1 : 1;
}

/**
 * Sorts by raw indexed value.
 */
function facetapi_sort_indexed(array $a, array $b) {
  $a_value = (isset($a['#indexed_value'])) ? $a['#indexed_value'] : '';
  $b_value = (isset($b['#indexed_value'])) ? $b['#indexed_value'] : '';
  if ($a_value == $b_value) {
    return 0;
  }
  return ($a_value < $b_value) ? -1 : 1;
}

/**
 * Sorts by display value.
 */
function facetapi_sort_display(array $a, array $b) {
  $a_count = (isset($a['#value'])) ? $a['#value'] : '';
  $b_count = (isset($b['#value'])) ? $b['#value'] : '';
  return strcasecmp($a['#value'], $b['#value']);
}
