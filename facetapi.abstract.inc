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
   *   The initialized render array,
   */
  abstract public function build();
}

/**
 * Abstract class extended by search backends that retrieves facet information
 * from the database.
 */
abstract class FacetapiAdapter {

  /**
   * The machine readable name of the searcher.
   *
   * @var string
   */
  protected $searcher;

  /**
   * The machine readable name of the instance the searcher is connected to.
   *
   * @var string
   */
  protected $instance;

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
    $this->setSearcher($searcher_info['name'])->setInstance($searcher_info['instance']);

    // Registers the query type plugins classes associated with this adapter.
    foreach (ctools_get_plugins('facetapi', 'query_types') as $plugin) {
      if ($searcher_info['adapter'] == $plugin['handler']['adapter']) {
        $type = call_user_func(array($plugin['handler']['class'], 'getType'));
        $this->queryTypes[$type] = $plugin['handler']['class'];
      }
    }
  }

  /**
   * Returns a boolean flagging whether $this->searcher executed a search.
   *
   * @return
   *   A boolean flagging whether $this->searcher executed a search.
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
   * Sets the machine readable name of the searcher.
   *
   * @param string $searcher
   *   The machine readable name of the searcher.
   *
   * @return FacetapiAdapter
   *   An instance of this class.
   */
  public function setSearcher($searcher) {
    $this->searcher = $searcher;
    return $this;
  }

  /**
   * Returns the machine readable name of the searcher.
   *
   * @return string
   *   The machine readable name of the searcher.
   */
  public function getSearcher() {
    return $this->searcher;
  }

  /**
   * Sets the type of content indexed by $this->searcher.
   *
   * @param string $type
   *   The type of content indexed by $this->searcher.
   *
   * @return FacetapiAdapter
   *   An instance of this class.
   */
  public function setInstance($instance) {
    $this->instance = $instance;
    return $this;
  }

  /**
   * Returns the type of content indexed by $this->searcher.
   *
   * @return
   *   The type of content indexed by $this->searcher.
   */
  public function getInstance() {
    return $this->instance;
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
   *
   */
  public function addFacetQuery(array $facet, $query) {
    if (isset($this->queryTypes[$facet['query type']])) {
      $plugin = new $this->queryTypes[$facet['query type']]($this, $facet);
      $plugin->execute($query);
      $this->facetQueries[$facet['name']] = $plugin;
    }
  }

  /**
   *
   */
  public function getFacetQuery(array $facet) {
    if (isset($this->facetQueries[$facet['name']])) {
      return $this->facetQueries[$facet['name']];
    }
  }
}
