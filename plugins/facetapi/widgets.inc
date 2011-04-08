<?php

/**
 * @file
 */

/**
 * Base class for widget plugins.
 */
abstract class FacetapiWidget {

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
   * Sets key to facet name since link facets shouldn't be grouped by alias.
   */
  public function __construct($build, $key, $js_settings) {
    parent::__construct($build, $key, $js_settings);
    $this->key = $this->build['#facet']['name'];
  }

  /**
   * Renders the links.
   */
  public function execute() {
    $this->key = $this->build['#facet']['name'];

    dsm($this->build);

    /*
    drupal_add_js(drupal_get_path('module', 'facetapi') .'/js/facetapi.block.js');

    // Captures facet information for code readability,
    $facet_name  = $build['#facet']['name'];
    $field_alias = $build['#facet']['field alias'];



    // Adds theme functions to each item dependent on whether it is active or not.
    $this->setThemeHooks($build[$field_alias], 'facetapi_link_active', 'facetapi_link_inactive');

    // Converts the array to something that can be read by theme_item_list().
    $build['#theme'] = 'facetapi_item_list';
    $build[$field_alias] = $this->buildItemList($build[$field_alias]);

    // Adds "soft limit" to the javascript settings.
    $js_settings['limit'] = $build['#settings']->settings['soft_limit'];
    */

    $this->build = array(
      '#theme' => 'item_list',
      '#title' => 'Test',
      '#items' => array('One', 'Two'),
    );



  }

  /**
   * Recursive function that sets theme hooks.
   *
   * @param &$build
   *   A render array containing the facet items.
   * @param $active_hook
   *   A string containing the theme hook to use when the facet is active.
   * @param $inactive_hook
   *   A string containing the theme hook to use when the facet is inactive.
   */
  protected function setThemeHooks(array &$build, $active_hook = NULL, $inactive_hook = NULL) {
    foreach ($build as $value => &$item) {
      if (empty($item['#active']) && NULL !== $inactive_hook) {
        $item['#theme'] = $inactive_hook;
      }
      if (!empty($item['#active']) && NULL !== $active_hook) {
        $item['#theme'] = $active_hook;
      }
      if (!empty($item['#item_children'])) {
        $this->setThemeHooks($item['#item_children'], $active_hook, $inactive_hook);
      }
    }
  }

  /**
   * Recursive function that converts the render array into an array that can be
   * passed to theme_item_list().
   *
   * @param $build
   *   The render array for the facet's items.
   *
   * @return
   *   An array that can be passed the theme_item_list().
   *
   * @see theme_item_list()
   * @see theme_facetapi_item_list()
   */
  function buildItemList($build) {
    $items = array();
    foreach ($build as $value => $item) {
      $row = array();

      $options = array(
        'attributes' => array(),
        //'query' => drupal_http_build_query($item['#query'], array('q', 'page')),
      );

      // We don't display children unless the parent is clicked.
      if (!empty($item['#item_children'])) {
        if ($item['#active']) {
          $row['class'] = 'expanded';
          $row['children'] = $this->buildItemList($item['#item_children']);
        }
        else {
          $row['class'] = 'collapsed';
        }
      }

      // Gets theme hook, adds last minute classes.
      if ($item['#active']) {
        $options['attributes']['class'] = 'active';
      }

      // Themes the link.
      $row['data'] = theme(
        $item['#theme'], $item['#markup'], $_GET['q'], $options, $item['#count']
      );

      // Adds links to array.
      $items[] = $row;
    }

    return $items;
  }
}
