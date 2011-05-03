<?php

/**
 * @file
 * Abstract classes and base plugin classes.
 */

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
   * An array of active filters.
   *
   * @var array
   */
  protected $activeItems;

  /**
   * An array of parameters, such as from $_GET.
   *
   * @var array
   */
  protected $params;

  /**
   * A boolean flagging whether the breadcrumb trail has been set.
   *
   * @var array
   */
  protected $breadcrumbSet = FALSE;

  /**
   * Define the array key in the params that is used for filters..
   */
  const FILTER_KEY = 'f';

  /**
   * Constructor, sets searcher and type of content being indexed.
   *
   * @param array $searcher_info
   *   The searcher definition.
   * @param array $params
   *   An array of keyed parameters, such as $_GET.
   */
  public function __construct(array $searcher_info, array $params = array()) {
    $this->info = $searcher_info;
    $this->setParams($params);

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
   * Returns a boolean flagging whether facets in a realm shoud be displayed.
   *
   * Useful, for example, for suppressing sidebar blocks in some cases.
   *
   * @return
   *   A boolean flagging whether to display a given realm.
   *
   * @todo Generic search API should provide consistent functionality.
   */
  abstract public function suppressOutput($realm_name);

  /**
   * Processes a raw arrray of active filters.
   *
   * @param array $params
   *   An array of keyed params, such as $_GET.
   *
   * @return
   *   The called object.
   */
  public function setParams(array $params = array()) {
    $this->activeItems = array();
    // Need to rebuild any facets that used different params.
    $this->facets = array();
    // We never want to use 'q' or 'page' params.
    unset($params['q'], $params['page']);
    if (!isset($params[self::FILTER_KEY])) {
      $params[self::FILTER_KEY] = array();
    }
    $this->params = $params;
    // Gets enabled facets' field aliases and processes array of raw filters.
    $field_aliases = array();
    foreach($this->getEnabledFacets() as $facet) {
      $field_aliases[$facet['field alias']] = TRUE;
    }

    // Extracts valid filters from query string.
    foreach ($params[self::FILTER_KEY] as $pos => $filter) {
      if (!is_scalar($filter)) {
        // Remove this from consideration if it's an array or object.
        unset($params[self::FILTER_KEY][$pos]);
      }
      $parts = explode(':', $filter, 2);
      if (isset($parts[1]) && isset($field_aliases[$parts[0]])) {
        $this->activeItems[$filter] = array(
          'field alias' => $parts[0],
          'value' => $parts[1],
          'pos' => $pos,
        );
      }
    }
    return $this;
  }

  public function getParams() {
    return $this->params;
  }

  /**
   * Processes and returns all active filters.
   *
   * @return array
   *   An array of active filters.
   */
  public function getAllActiveItems() {
    // Returns array of active filters.
    return $this->activeItems;
  }

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
   * Returns the id of the adapter plugin.
   *
   * @return string
   *   The machine readable if of the adapter plugin.
   */
  public function getId() {
    return $this->info['adapter'];
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
  public function getType() {
    return $this->info['type'];
  }

  /**
   * Returns the path to the admin settings for a given realm.
   *
   * @param $realm_name
   *   The name of the realm.
   *
   * @return
   *   The path to the admin settings.
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
   * Requests facets and applies facet filters to the query.
   *
   * @param stdClass $query
   *   The query in the backend's native API.
   */
  function addFacetsFilters($query) {
    foreach ($this->getEnabledFacets() as $facet) {
      $this->addFacetQuery($facet, $query);
      $this->addActiveFilters($facet, $query);
    }
  }

  /**
   * Back-end specific function for adding filters.
   *
   * Typically will call $this->getActiveItems($facet);
   *
   * @param array $facet
   *   The facet definition.
   * @param mixed $query
   *   A mixed value containing the query in the backend's native API.
   *
   * @return FacetapiFacet
   *   The facet rendering object object.
   */
  abstract public function addActiveFilters(array $facet, $query);

  /**
   * Returns the enabled facets associated with the instance of the adapter.
   *
   * @param string $realm_name
   *   The machine readable name of the realm, pass NULL to get the enabled
   *   facets in all realms.
   *
   * @return array
   *   An array of enabled facets.
   */
  public function getEnabledFacets($realm_name = NULL) {
    return facetapi_get_enabled_facets($this->info['name'], $realm_name);
  }

  /**
   * Returns a FacetapiFacet instance for the facet being rendered.
   *
   * @param array $facet
   *   The facet definition.
   *
   * @return FacetapiFacet
   *   The facet rendering object object.
   */
  public function getFacet(array $facet) {
    if (!isset($this->facets[$facet['name']])) {
      $this->facets[$facet['name']] = new FacetapiFacet($this, $facet, $this->params, self::FILTER_KEY);
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
      $this->facetQueries[$facet['name']] = $plugin;
      $plugin->execute($query);
    }
  }

  /**
   * Returns a registered facet query
   *
   * @param array $facet
   *   The facet definition.
   *
   * @return FacetapiQueryTypeInterface
   *   The instantiated query type plugin.
   */
  public function getFacetQuery(array $facet) {
    if (isset($this->facetQueries[$facet['name']])) {
      return $this->facetQueries[$facet['name']];
    }
  }

  /**
   * Sets the breadcrumb trail on searches.
   */
  public function setBreadcrumb() {
    $breadcrumb = drupal_get_breadcrumb();
    $enabled_facets = $this->getEnabledFacets();

    $keys = $this->getSearchKeys();
    $active_items = $this->getAllActiveItems();
    // Make sure the search page itself is in the breadcrub if we are going to
    // add more crumbs. Wow this breadcrumb code sucks.
    $item = menu_get_item();
    $last_load_func = is_array($item['load_functions']) ? end($item['load_functions']) : NULL;
    if ((!$keys && $active_items) || ($keys && $last_load_func != 'menu_tail_load')) {
      $last = end($breadcrumb);
      $this_page = l($item['title'], $item['href'], $item['localized_options']);
      if ($last != $this_page) {
        $breadcrumb[] = $this_page;
      }
    }
    // Adds the current search to the query.
    // @todo Need a getBaseQuery() method or something.
    if ($keys) {
      // The last item should be text, not a link.
      $breadcrumb[] = $active_items ? l($keys, $_GET['q']) : check_plain($keys);
    }

    // Adds filters to the breadcrumb trail.
    $query = array();
    $last = end($active_items);
    foreach ($active_items as $item) {
      $query[self::FILTER_KEY][] = $item['field alias'] . ':' . $item['value'];

      // Maps the value to something human readable.
      // @todo This is wicked hacky.
      $value = $item['value'];
      foreach ($enabled_facets as $facet) {
        if ($facet['field alias'] == $item['field alias'] && $facet['map callback'] && function_exists($facet['map callback'])) {
          $mapped = $facet['map callback'](array($value));
          if (isset($mapped[$value])) {
            $value = $mapped[$value];
          }
          continue;
        }
      }
      // The last item should be text, not a link.
      if ($last == $item) {
        $breadcrumb[] = check_plain($value);
      }
      else {
        // Appends the filter to the breadcrumb trail.
        $breadcrumb[] = l($value, $_GET['q'], array('query' => $query));
      }
    }

    // Sets the breadcrumb trail with searck keys and filters.
    drupal_set_breadcrumb($breadcrumb);
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

    // Builds breadcrumb trail, only does this once.
    if (!$this->breadcrumbSet && $this->searchExecuted()) {
      $this->breadcrumbSet = TRUE;
      $this->setBreadcrumb();
    }

    // Adds JavaScript, initializes render array.
    drupal_add_js(drupal_get_path('module', 'facetapi') . '/facetapi.js');
    $build = array(
      '#adapter' => $this,
      '#realm' => $realm,
    );

    // Builds each facet in the realm, merges into realm's render array.
    foreach ($this->getEnabledFacets($realm['name']) as $facet) {
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
  protected $activeItems;

  /**
   * The build array for the facet items.
   *
   * @var array
   */
  protected $build = array();

  /**
   * The initialized query string for all facets.
   *
   * @var array
   */
  protected $params;

  /**
   * The key in $params corresponding to filters.
   *
   * @var string
   */
  protected $filter_key;

  /**
   * Constructor, sets adapter and facet definition.
   *
   * @param $adapter
   *   A FacetapiAdapter object.
   * @param $facet
   *   An array containing the facet definition.
   * @param $params
   *   An array of params such as $_GET.
   * @param $filter_key
   *   The array key in $params corresponding to filters. $params[$filter_key]
   *   must be an array.
   */
  public function __construct(FacetapiAdapter $adapter, array $facet, array $params, $filter_key) {
    $this->adapter = $adapter;
    $this->facet = $facet;
    $this->params = $params;
    $this->filter_key = $filter_key;
    // Checks which facets are "active".
    // @todo Is there a way to better lazy-load this?
    $this->processActiveItems();
  }

  /**
   * Whether a offset exists
   *
   * @param mixed offset
   *   An offset to check for.
   *
   * @return boolean
   */
  public function offsetExists($offset) {
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
  public function offsetGet($offset) {
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
  public function offsetSet($offset, $value) {
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
  public function offsetUnset($offset) {
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
    return $this->activeItems;
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
    else {
      $realm_name = $realm;
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

      if ($realm_name) {
        $settings->settings = array(
          'weight' => 0,
          'soft_limit' => 20,
          'widget' => 'facetapi_links',
          'active_sorts' => array(),
          'sort_weight' => array(),
          'sort_order' => array(),
        );

        // Apply default sort settings.
        $weight = -50;
        foreach ($this->facet['default sorts'] as $sort => $default) {
          $settings->settings['active_sorts'][$default[0]] = $default[0];
          $settings->settings['sort_weight'][$default[0]] = $weight++;
          $settings->settings['sort_order'][$default[0]] = $default[1];
        }
      }
      else {
        $settings->settings = array(
          'operator' => FACETAPI_OPERATOR_AND,
          'hard_limit' => 50,
        );
      }
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
    if (!isset($this->activeItems)) {
      $this->processActiveItems();
    }
    $values = array();
    foreach ($this->activeItems as $value => $item) {
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
    return (int) isset($this->activeItems[$value]);
  }

  /**
   * Helper function that returns the query string variables for a facet item.
   *
   * @param array $values
   *   An array containing the item's values being added to or removed from the
   *   query string dependent on whether or not the item is active.
   * @param int $active
   *   An integer flagging whether the item is active or not.
   *
   * @reutrn
   *   An array containing the query string variables.
   */
  public function getQueryString(array $values, $active) {
    $qstring = $this->params;

    // Appends to qstring if inactive, removes if inactive.
    foreach ($values as $value) {
      if ($active && isset($this->activeItems[$value])) {
        unset($qstring[$this->filter_key][$this->activeItems[$value]['pos']]);
      }
      elseif (!$active) {
        $qstring[$this->filter_key][] = $this->facet['field alias'] . ':' . $value;
      }
    }

    // Resets array keys, returns query string.
    $qstring[$this->filter_key] = array_values($qstring[$this->filter_key]);
    return array_filter($qstring);
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
      $this->initializeBuild()->processHierarchy()->processQueryStrings($this->build);
    }

    // Instantiates the widget plugin and executes.
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
    if ($this->build && $this->facet['map callback'] && function_exists($this->facet['map callback'])) {
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
   * @param array &$build
   *   The facet's render array.
   */
  protected function processQueryStrings(array &$build) {
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
  }

  /**
   * Gets active facet items from the query string, stores as a property.
   *
   * @return
   *   An instance of this class.
   */
  public function processActiveItems() {
    $this->activeItems = array();

    // Processes active items if the facet is enabled in at least one realm.
    if (facetapi_facet_enabled($this->adapter->getSearcher(), NULL, $this->facet['name'])) {

      // Gets active filters for this facet only, keys by value.
      foreach ($this->adapter->getAllActiveItems() as $item) {
        if ($item['field alias'] == $this->facet['field alias']) {
          $this->activeItems[$item['value']] = $item;
        }
      }

      // Allows the query type plugin to extract additional data, such as the
      // start and end values from range queries.
      $plugin = $this->adapter->getFacetQuery($this->facet);
      foreach ($this->activeItems as &$item) {
        $item += $plugin->extract($item);
      }
    }

    return $this;
  }
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
    $item['#markup'] = $map[$key];
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
