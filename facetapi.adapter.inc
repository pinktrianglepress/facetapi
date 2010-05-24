<?php
// $Id$

/**
 * @file
 * Defines classes used by the FacetAPI module.
 */

/**
 * Abstract class extended by search backends that retrieves facet information
 * from the database.
 */
abstract class FacetapiAdapter {

  /**
   * The machine readable name of the searcher module.
   */
  protected $_searcher;

  /**
   * The type of content indexed by $this->_searcher.
   */
  protected $_type;

  /**
   * The module that defines the adapter.
   */
  protected $_module;

  /**
   * An array of facet item defaults.
   */
  protected $_item_defaults = array(
    'function' => 'facetapi_link',
    'text' => '',
    'options' => array(),
    'count' => 0,
    'active' => 0,
    'parents' => array(),
    'children' => array(),
  );

  /**
   * Constructor, sets searcher and type of content being indexed.
   *
   * @param $searcher
   *   A string containing the machine readable name of the searcher module.
   * @param $type
   *   A string containing the type of content indexed by $searcher.
   * @return
   *   NULL
   */
  public function __construct($searcher, $type, $module) {
    $this->setSearcher($searcher)
      ->setType($type)
      ->setModule($module);
  }

  /**
   * Returns a boolean flagging whether $this->_searcher executed a search.
   *
   * @return
   *   A boolean flagging whether $this->_searcher executed a search.
   * @todo Generic search API should provide consistent functionality.
   */
  abstract public function searchExecuted();

  /**
   * Returns a normalized array with facet counts included.
   *
   * @param $facets
   *   An array of enabled facets.
   * @return
   *   An array with facet count values appended.
   * @todo Define how the array is structured in this docblock.
   */
  abstract public function getFacetData(array $facets);

  /**
   * Returns a facet link's query string.
   *
   * @param $facet
   *   An array containing the full facet definition.
   * @param $values
   *   An array containig the values to add or remove from the query string.
   * @param $active
   *   An integer, 1 if the facet is active, 0 if the facet is not active.
   * @reutrn
   *   An array containing the query strng variables.
   */
  abstract public function getFacetQueryString(array $facet, array $values, $active);

  /**
   * Returns an array of active facet values in a field.
   *
   * @param $field
   *   A string containing the field name.
   * @return
   *   An array of values.
   */
  abstract public function getActiveFacets($field);

  /**
   * Tests whether a facet is active.
   *
   * NOTE: This method returns an integer instead of a boolean because the value
   * is used by the Facet API's custom sorting functions. It ends up being less
   * code to compare integers than booleans.
   *
   * @param $field
   *   A string containing the facet's field.
   * @param $value
   *   A string containing the facet value.
   * @return
   *   An integer, 1 if the facet is active, 0 if the facet is not active.
   */
  abstract public function facetActive($field, $value);

  /**
   * The default method used to retrieve facet data.
   *
   * @param $facet
   *   An array containing the full facet definition.
   * @return
   *   An array of facet data.
   */
  abstract public function fetchDefault(array $facet);

  /**
   * Fetches facet data via the appropriate fetch*() method.
   *
   * @param $facet
   *   An array containing the full facet definition.
   * @return
   *   An array containing the facet data.
   */
  public function fetch(array $facet) {
    if (!empty($facet['fetch type'])) {
      $method = 'fetch'. ucfirst($facet['fetch type']);
      if (method_exists($this, $method)) {
        return $this->$method($facet);
      }
    }
    return $this->fetchDefault($facet);
  }

  /**
   * Applies the default sort to the facets. If facets are already sorted by
   * getFacetCounts(), the adapter doesn't have to override this method.
   *
   * @param &$facet_items
   *   An array of facet items.
   * @return
   *   NULL
   */
  public function sortDefault(array &$facet_items) {
  }

  /**
   * Sets the searcher module.
   *
   * @param $searcher
   *   A string containing the machine readable name of the searcher module.
   * @return
   *   An instance of this class.
   */
  public function setSearcher($searcher) {
    $this->_searcher = $searcher;
    return $this;
  }

  /**
   * Returns the searcher module.
   *
   * @return
   *   A string containing the machine readable name of the searcher module.
   */
  public function getSearcher() {
    return $this->_searcher;
  }

  /**
   * Sets the type of content indexed by $this->_searcher.
   *
   * @param $type
   *   A string containing the type of content indexed by $this->_searcher.
   * @return
   *   An instance of this class.
   */
  public function setType($type) {
    $this->_type = $type;
    return $this;
  }

  /**
   * Returns the type of content indexed by $this->_searcher.
   *
   * @return
   *   A string containing the type of content indexed by $this->_searcher.
   */
  public function getType() {
    return $this->_type;
  }

  /**
   * Sets the module that defines the adapter.
   *
   * @param $module
   *   A string containing the module that defines the adapter.
   * @return
   *   An instance of this class.
   */
  public function setModule($module) {
    $this->_module = $module;
    return $module;
  }

  /**
   * Returns the module that defines the adapter.
   *
   * @return
   *   A string containing the module that defines the adapter.
   */
  public function getModule() {
    return $this->_module;
  }
}
