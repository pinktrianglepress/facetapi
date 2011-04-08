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

    // Sets the key of the elemen.
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

/**
 * Widget that renders facets as a list of clickable links.
 */
class FacetapiWidgetLinks extends FacetapiWidget {

  /**
   * Renders the links.
   */
  public function execute() {

    // Sets the key, captures the facet's render array.
    $this->key = $this->build['#facet']['name'];
    $element = &$this->build[$this->facet['field alias']];

    // Sets each item's theme hook, builds item list.
    $this->setThemeHooks($element);
    $element = array(
      '#theme' => 'item_list',
      '#title' => theme('facetapi_title', array('title' => $this->build['#title'])),
      '#items' => $this->buildListItems($element),
    );

    /*
    drupal_add_js(drupal_get_path('module', 'facetapi') .'/js/facetapi.block.js');
    // Adds "soft limit" to the javascript settings.
    $js_settings['limit'] = $build['#settings']->settings['soft_limit'];
    */
  }

  /**
   * Recursive function that sets each item's theme hook.
   *
   * @param array &$build
   *   A render array containing the facet items.
   */
  protected function setThemeHooks(array &$build) {
    foreach ($build as $value => &$item) {
      $item['#theme'] = ($item['#active']) ? 'facetapi_link_active' : 'facetapi_link_inactive';
      if (!empty($item['#item_children'])) {
        $this->setThemeHooks($item['#item_children']);
      }
    }
    return $this;
  }

  /**
   * Recursive function that converts the render array into an array that can be
   * passed to theme_item_list().
   *
   * @param array $build
   *   The facet's render array.
   *
   * @return array
   *   The "items" parameter for theme_item_list().
   */
  function buildListItems($build) {
    $items = array();
    foreach ($build as $value => $item) {
      $row = array();

      // Initializes variables passed to theme hook.
      $variables = array(
        'text' => $item['#markup'],
        'path' => $_GET['q'],
        'count' => $item['#count'],
        'options' => array(
          'attributes' => array(),
          'html' => FALSE,
          'query' => $item['#query'],
        ),
      );

      // We don't display children unless the parent is clicked.
      if (!empty($item['#item_children'])) {
        if ($item['#active']) {
          $row['class'] = 'expanded';
          $row['children'] = $this->buildListItems($item['#item_children']);
        }
        else {
          $row['class'] = 'collapsed';
        }
      }

      // Gets theme hook, adds last minute classes.
      if ($item['#active']) {
        $variables['options']['attributes']['class'] = 'active';
      }

      // Themes the link, adds row to items.
      $row['data'] = theme($item['#theme'], $variables);
      $items[] = $row;
    }

    return $items;
  }
}
