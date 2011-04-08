<?php

/**
 * @file
 */

/**
 * Base class for widget plugins.
 */
abstract class FacetapiWidget {

  /**
   * The facet object.
   *
   * @var FacetapiFacet
   */
  protected $facet;

  /**
   * The normalized render array.
   *
   * @var array
   */
  protected $build;

  /**
   * The key of the facet's render array added to the realm's render array.
   *
   * @var string
   */
  protected $key;

  /**
   * JavaScript settings.
   *
   * @var array
   */
  protected $jsSettings;

  /**
   *
   * @param array $realm
   *   The realm being rendered.
   * @param array $settings
   *   The realm settings.
   * @param FacetapiFacet $facet
   *   The facet object.
   */
  public function __construct(array $realm, stdClass $settings, FacetapiFacet $facet) {
    $this->facet = $facet;

    // Captures searcher for code readability.
    $searcher = $facet->getAdapter()->getSearcher();

    // Initializes render array.
    $this->build = array(
      '#title' => $facet['label'],
      '#description' => $facet['description'],
      '#weight' => $facet['weight'],
      '#adapter' => $facet->getAdapter(),
      '#realm_name' => $realm['name'],
      '#facet' => $facet->getFacet(),
      '#settings' => $settings,
      $facet['field alias'] => $facet->getBuild(),
      '#attributes' => array(
        'class' => "facetapi-facet-{$facet['name']}",
        'id' => "facetapi-facet-$searcher-{$realm['name']}-{$facet['name']}",
      ),
    );

    // Sets the key of the element as appended to the realm's render array.
    $this->key = $facet['field alias'];

    // Applies sorting algorithms.
    // @todo Need to pass build to facet sorting stuff.
    //$this->sorts = $this->sortFacet($realm);

    // Gets widget from settings, finds default if necessary.
    // @todo Implement default widgets.
    $widget_name = $this->build['#settings']->settings['widget'];

    // Initializes JavaScript settings.
    $this->jsSettings = array(
      'searcher' => $searcher,
      'instance' => $facet->getAdapter()->getInstance(),
      'realmName' => $realm['name'],
      'facetName' => $facet['name'],
      'widget' => $settings->settings['widget'],
      'queryType' => $facet['query type'],
    );
  }

  /**
   * Abstract public function.
   */
  abstract public function execute();

  /**
   * Returns the render array.
   *
   * @return array
   *   The render array.
   */
  public function getBuild() {
    return $this->build;
  }

  /**
   * Returns the element's key value.
   *
   * @return string
   *   The element's key.
   */
  public function getKey() {
    return $this->key;
  }


  /**
   * Returns the JavaScript settings.
   *
   * @return array
   *   The JavaScript settings.
   */
  public function getJavaScriptSettings() {
    return $this->jsSettings;
  }
}
