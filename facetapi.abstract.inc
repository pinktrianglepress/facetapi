<?php

/**
 * @file
 * Abstract classes and base plugin classes.
 */

/**
 * Base class for query type plugins.
 */
abstract class FacetapiQueryType {

  /**
   * The adapter object.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * The facet definition.
   *
   * @var array
   */
  protected $facet;

  /**
   *
   * @param FacetapiAdapter $adapter
   *   The adapter object.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    $this->adapter = $adapter;
    $this->facet = $facet;
  }

  /**
   * Returns the query type associated with the plugin.
   *
   * @return string
   *   The query type.
   */
  abstract static public function getType();

  /**
   * Adds the filter to the query object.
   *
   * @param stdClass $query
   *   An object containing the query in the backend's native API.
   */
  abstract public function execute($query);

  /**
   * Initializes the facet's render array.
   *
   * @return
   *   The initialized render array.
   */
  abstract public function build();
}

/**
 * Abstract class extended by search backends that retrieves facet information
 * from the database.
 */
abstract class FacetapiAdapter {

  /**
   * Searcher information retrieved by the hook.
   *
   * @var array
   */
  protected $info = array();

  /**
   * The search keys passed by the user.
   *
   * @var string
   */
  protected $keys;

  /**
   * An array of query type plugin definitions keyed by type.
   *
   * @var array
   */
  protected $queryTypes = array();

  /**
   * An array of FacetapiFacet objects.
   *
   * @var array
   */
  protected $facets = array();

  /**
   * An array of executed query tyoe plugins keyed by field name.
   *
   * @var array
   */
  protected $facetQueries = array();

  /**
   * Constructor, sets searcher and type of content being indexed.
   *
   * @param array $searcher_info
   *   The searcher definition.
   */
  public function __construct(array $searcher_info) {
    $this->info = $searcher_info;

    // Registers the query type plugins classes associated with this adapter.
    foreach (ctools_get_plugins('facetapi', 'query_types') as $plugin) {
      if ($searcher_info['adapter'] == $plugin['handler']['adapter']) {
        $type = call_user_func(array($plugin['handler']['class'], 'getType'));
        $this->queryTypes[$type] = $plugin['handler']['class'];
      }
    }
  }

  /**
   * Returns a boolean flagging whether $this->info['searcher'] executed a search.
   *
   * @return
   *   A boolean flagging whether $this->info['searcher'] executed a search.
   *
   * @todo Generic search API should provide consistent functionality.
   */
  abstract public function searchExecuted();

  /**
   * Returns a facet's active items.
   *
   * @param array $facet
   *   The facet definition.
   *
   * @return array
   *   The facet's active items.
   */
  public function getActiveItems(array $facet) {
    return $this->getFacet($facet)->getActiveItems();
  }

  /**
   * Returns The facet's active item values.
   *
   * @param array $facet
   *   The facet definition.
   *
   * @return array
   *   The facet values keyed by position.
   */
  public function getActiveValues(array $facet) {
    return $this->getFacet($facet)->getActiveValues();
  }

  /**
   * Tests whether a facet item is active by passing it's value.
   *
   * @param array $facet
   *   The facet definition.
   * @param string $value
   *   The value being checked.
   *
   * @return int
   *   Returns 1 if the facet is active, 0 if the facet is not active.
   */
  public function itemActive(array $facet, $value) {
    return $this->getFacet($facet)->itemActive($value);
  }

  /**
   * Returns the machine readable name of the searcher.
   *
   * @return string
   *   The machine readable name of the searcher.
   */
  public function getSearcher() {
    return $this->info['name'];
  }

  /**
   * Returns the type of content indexed by $this->info['searcher'].
   *
   * @return
   *   The type of content indexed by $this->info['searcher'].
   */
  public function getInstance() {
    return $this->info['instance'];
  }

  /**
   * Returns the path to the admin settings for a given realm.
   *
   * @param $realm_name
   *   The name of the realm.
   *
   * @return
   *   The type of content indexed by $this->info['searcher'].
   */
  public function getPath($realm_name) {
    return $this->info['path'] . '/facets/' . $realm_name;
  }

  /**
   * Sets the search keys.
   *
   * @param string $keys
   *   The search keys entered by the user.
   *
   * @return FacetapiAdapter
   *   An instance of this class.
   */
  public function setSearchKeys($keys) {
    $this->keys = $keys;
    return $this;
  }

  /**
   * Gets the search keys.
   *
   * @return string
   *   The search keys entered by the user.
   */
  public function getSearchKeys() {
    return $this->keys;
  }

  /**
   * Returns a FacetapiFacet instnace for the facet being rendered.
   *
   * @param array $facet
   *   The facet definition.
   *
   * @return FacetapiFacet
   *   The facet rendering object object.
   */
  public function getFacet(array $facet) {
    if (!isset($this->facets[$facet['name']])) {
      $this->facets[$facet['name']] = new FacetapiFacet($this, $facet);
    }
    return $this->facets[$facet['name']];
  }

  /**
   * Adds a facet query to the backend.
   *
   * @param array $facet
   *   The facet definition.
   * @param mixed $query
   *   A mixed value containing the query in the backend's native API.
   */
  public function addFacetQuery(array $facet, $query) {
    if (isset($this->queryTypes[$facet['query type']])) {
      $plugin = new $this->queryTypes[$facet['query type']]($this, $facet);
      $plugin->execute($query);
      $this->facetQueries[$facet['name']] = $plugin;
    }
  }

  /**
   * Returns a registered facet query
   *
   * @param array $facet
   *   The facet definition.
   *
   * @return FacetapiQueryType
   *   The instantiated query type plugin.
   */
  public function getFacetQuery(array $facet) {
    if (isset($this->facetQueries[$facet['name']])) {
      return $this->facetQueries[$facet['name']];
    }
  }

  /**
   * Builds the render array for facets in a realm.
   *
   * @param string $realm_name
   *   The machine readable name of the realm.
   *
   * @return array
   *   The render array.
   */
  public function buildRealm($realm_name) {
    // Bails if realm isn't valid.
    // @todo Call watchdog()?
    if (!$realm = facetapi_realm_load($realm_name)) {
      return array();
    }

    // Adds JavaScript, initializes render array.
    drupal_add_js(drupal_get_path('module', 'facetapi') . '/js/facetapi.js');
    $build = array(
      '#adapter' => $this,
      '#realm' => $realm,
    );

    // Builds each facet in the realm, merges into realm's render array.
    foreach (facetapi_get_enabled_facets($this->info['name'], $realm['name']) as $facet) {
      $field_alias = $facet['field alias'];
      $facet_build = $this->getFacet($facet)->build($realm);

      // Tries to be smart when merging the render arrays. Crazy things happen
      // when merging facets with the same field alias such as taxonomy terms in
      // the fieldset realm. We want to merge only the values.
      foreach (element_children($facet_build) as $child) {
        if (!isset($build[$child])) {
          $build = array_merge_recursive($build, $facet_build);
        }
        else {
          if (isset($build[$child][$field_alias]) && isset($facet_build[$child][$field_alias])) {
            $build[$child][$field_alias] = array_merge_recursive(
              $build[$child][$field_alias],
              $facet_build[$child][$field_alias]
            );
          }
          elseif (isset($build[$child]['#options']) && isset($facet_build[$child]['#options'])) {
            $build[$child]['#options'] = array_merge_recursive(
              $build[$child]['#options'],
              $facet_build[$child]['#options']
            );
          }
          else {
            $build = array_merge_recursive($build, $facet_build);
          }
        }
      }
    }

    // Allows modules to alter the render array before returning.
    drupal_alter('facetapi_facets', $build, $this, $realm);
    return $build;
  }
}

/**
 * Stores facet data, provides methods that build the facet's render array.
 */
class FacetapiFacet implements ArrayAccess {

  /**
   * The FacetapiAdapter object.
   *
   * @var FacetapiAdapter
   */
  protected $adapter;

  /**
   * The facet definition.
   *
   * @var array
   */
  protected $facet;

  /**
   * An array of active facets items.
   *
   * @var array
   */
  protected $active = array();

  /**
   * The build array for the facet items.
   *
   * @var array
   */
  protected $build = array();

  /**
   * Constructor, sets adapter and facet definition.
   *
   * @param $adapter
   *   A FacetapiAdapter object.
   * @param $facet
   *   An array containing the facet definition.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet) {
    $this->adapter = $adapter;
    $this->facet = $facet;
  }

  /**
   * Whether a offset exists
   *
   * @param mixed offset
   *   An offset to check for.
   *
   * @return boolean
   */
  public function offsetExists ($offset) {
    return isset($this->facet[$offset]);
  }

  /**
   * Returns the value at specified offset.
   *
   * @param mixed offset
   *   The offset to retrieve.
   *
   * @return mixed
   */
  public function offsetGet ($offset) {
    return isset($this->facet[$offset]) ? $this->facet[$offset] : NULL;
  }

  /**
   * Assigns a value to the specified offset.
   *
   * @param mixed offset
   *   The offset to assign the value to.
   * @param mixed value
   *   The value to set.
   */
  public function offsetSet ($offset, $value) {
    if (NULL === $offset) {
      $this->facet[] = $value;
    }
    else {
      $this->facet[$offset] = $value;
    }
  }

  /**
   * Unsets an offset.
   *
   * @param mixed offset
   *   The offset to unset.
   */
  public function offsetUnset ($offset) {
    unset($this->facet[$offset]);
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
   * Returns the facet definition.
   *
   * @return array
   *   An array containing the facet definition.
   */
  public function getFacet() {
    return $this->facet;
  }

  /**
   * Returns the facet definition.
   *
   * @return array
   *   An array containing the facet definition.
   */
  public function getBuild() {
    return $this->build;
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
   * Returns the facet's active sorts in the order specified via the GUI.
   *
   * @return array
   *   The facet's active sorts.
   */
  public function getActiveSorts() {
    return array();
  }

  /**
   * Gets facet setting for the passed realm.
   *
   * @param string|array $realm
   *   The machine readable name of the realm or realm definition. Pass null to
   *   get global settings.
   *
   * @return
   *   An object containing the settings.
   */
  public function getSettings($realm = NULL) {
    // Normalizes the realm name.
    if (!$realm) {
      $realm_name = '';
    }
    elseif (is_array($realm)) {
      $realm_name = $realm['name'];
    }

    // Loads the settings via the CTools API.
    ctools_include('export');
    $name = $this->adapter->getSearcher() . ':' . $realm_name . ':' . $this->facet['name'];
    if (!$settings = ctools_export_crud_load('facetapi', $name)) {
      $settings = ctools_export_crud_new('facetapi');
      $settings->name = $name;
      $settings->searcher = $this->adapter->getSearcher();
      $settings->realm = $realm_name;
      $settings->facet = $this->facet['name'];
      $settings->enabled = 0;
      $settings->settings = array(
        'weight' => 0,
        'operator' => FACETAPI_OPERATOR_AND,
        'hard_limit' => 50,
        'soft_limit' => 20,
        'widget' => 'facetapi_links',
        'active_sorts' => array(),
        'sort_weight' => array(),
        'sort_order' => array(),
      );
    }
    return $settings;
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
    $settings = $this->getSettings($realm);

    // Builds the base render array for this facet across all realms. This
    // ensures we are only pulling the data from the server once per facet.
    if (!$this->build) {
      $this->initializeBuild()->processHierarchy()->processQueryStrings();
    }

    // Instantiates the widget plugins and executes.
    $widget_name = $settings->settings['widget'];
    $class = ctools_plugin_load_class('facetapi', 'widgets', $widget_name, 'handler');
    $plugin = new $class($realm, $settings, $this);
    $plugin->execute();

    // Adds JavaScript settings in a way that merges with others already set.
    $merge_settings['facetapi']['facets'][] = $plugin->getJavaScriptSettings();
    drupal_add_js($merge_settings, 'setting');

    // Returns array keyed by the FacetapiWidget::$key property.
    return array($plugin->getKey() => $plugin->getBuild());
  }

  /**
   * Builds the render arrays for the items in the facet.
   *
   * @return FacetapiFacet
   *   An instance of this class.
   */
  protected function initializeBuild() {

    // Build array defaults.
    $defaults = array(
      '#markup' => '',
      '#indexed_value' => '',
      '#count' => 0,
      '#active' => 0,
      '#item_parents' => array(),
      '#item_children' => array(),
    );

    // Bails if there is no field attached to the facet, in other words if the
    // facet is simply rendering markup.
    if (!$this->facet['field']) {
      $this->build = array();
      return $this;
    }

    // Builds render arrays for each item.
    $this->build = $this->adapter->getFacetQuery($this->facet)->build();
    foreach (element_children($this->build) as $value) {
      $item_defaults = array(
        '#markup' => $value,
        '#indexed_value' => $value,
        '#active' => $this->itemActive($value),
      );
      $this->build[$value] = array_merge($defaults, $item_defaults, $this->build[$value]);
    }

    // Maps the IDs to human readable values via the mapping callback.
    if ($this->facet['map callback'] && function_exists($this->facet['map callback'])) {
      $map = call_user_func($this->facet['map callback'], array_keys($this->build));
      array_walk($this->build, 'facetapi_replace_ids', $map);
    }

    return $this;
  }

  /**
   * Processes hierarchical relationships between the facet items.
   *
   * @return FacetapiFacet
   *   An instance of this class.
   */
  protected function processHierarchy() {

    // Builds the hierarchy information if the hierarchy callback is defined.
    if ($this->facet['hierarchy callback'] && $this->build) {
      $parents = $this->facet['hierarchy callback'](array_keys($this->build));
      foreach ($parents as $value => $parents) {
        foreach ($parents as $parent) {
          if (isset($this->build[$parent]) && isset($this->build[$value])) {
            // Use a reference so we see the updated data.
            $this->build[$parent]['#item_children'][$value] = &$this->build[$value];
            $this->build[$value]['#item_parents'][$parent] = $parent;
          }
        }
      }
    }

    // Tests whether parents have an active child.
    // @todo: Can we make this more efficient?
    do {
      $active = 0;
      foreach ($this->build as $value => $item) {
        if ($item['#active'] && !empty($item['#item_parents'])) {
          // @todo Can we build facets with multiple parents? Core taxonomy
          // form cannot, so we will need a check here.
          foreach ($item['#item_parents'] as $parent) {
            if (!$this->build[$parent]['#active']) {
              $active = $this->build[$parent]['#active'] = 1;
            }
          }
        }
      }
    } while ($active);

    // Strips children whose parents are inactive
    $this->build = array_filter($this->build, 'facetapi_filter_inactive_parent');
    return $this;
  }

  /**
   * Initializes the render array's query string variables.
   *
   * @return FacetapiFacet
   *   An instance of this class.
   */
  protected function processQueryStrings() {
    foreach ($this->build as $value => &$item) {
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
   * Returns sorts enabled for a facet in a realm.
   *
   * Sort definitions are returned in the order specified in the configurations
   * settings.
   *
   * @param $realm
   *   An array containing the full realm definition.
   *
   * @return
   *   An array of enabled sort definitions.
   *
   * @todo Move to FacetapiFacet object.
   */
  function sortFacet(array $realm) {

    // Gets active sorts.
    /*
    $active_sorts = (array) facetapi_setting_get(
      'sort', $searcher, $realm['name'], $facet['name']
    );
    */
    $active_sorts = array();

    // Applies defaults if nothing is set.
    $defaults = array();
    if (empty($active_sorts)) {
      $weight = -50;
      foreach ($this->facet['default sorts'] as $sort) {
        $active_sorts[$sort[0]] = $sort[0];
        $defaults[$sort[0]] = array(
          'weight' => $weight++,
          'order'  => $sort[1],
        );
      }
    }

    // Finalizes sort definitions with settings or defaults.
    $this->sorts = array_intersect_key(facetapi_get_sort_info(), array_filter($active_sorts));
    foreach ($this->sorts as $sort_name => &$sort_info) {
      if (empty($defaults)) {
        // @todo Settings aren't found here anymore.
        $sort_info['weight'] = facetapi_setting_get(
          'sort_weight', $this->info['searcher'], $realm['name'], $this->facet['name'], $sort_name
        );
        $sort_info['order'] = facetapi_setting_get(
          'sort_order', $this->info['searcher'], $realm['name'], $this->facet['name'], $sort_name
        );
      }
      else {
        $sort_info['weight'] = $defaults[$sort_name]['weight'];
        $sort_info['order']  = $defaults[$sort_name]['order'];
      }
    }
    unset($sort_info);
    uasort($this->sorts, 'drupal_sort_weight');

    // Applies sorting algorithms.
    $this->applySorts($this->build);
    unset($this->sorts);
  }

  /**
   * Sorts the facet's build array.
   *
   * @param array &$build
   *   Reference to the render array, allows us to sort one hierarchical level
   *   at a time.
   */
  protected function applySorts(&$build) {
    foreach (element_children($build) as $value) {
      if (!empty($build[$value]['#item_children'])) {
        $this->applySorts($build[$value]['#item_children']);
      }
    }
    uasort($build, array($this, 'sortCallback'));
  }

  /**
   * Generic sort callback, useful as a callback to uasort().
   *
   * Applies sorts in the order they are specified in the settings.
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

  /**
   * Gets active facet items from the query string, stores as a property.
   *
   * @return
   *   An instance of this class.
   */
  public function processActiveItems() {
    $this->active = array();

    // Bails if the facet isn't enabled in any realm.
    // @todo Is this check superfluous?
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
    // @todo This hook is not longer valid.
    $hook = 'facetapi_value_' . $this->facet['query type'];
    foreach ($data as $key => $value) {
      $this->active[$value] = array(
        'pos' => $key,
        'value' => $value,
      );
      drupal_alter($hook, $this->active[$value], $this->adapter);
    }

    return $this;
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
