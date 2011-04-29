<?php

/**
 * @file
 * Abstract widget plugin class.
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
        'class' => drupal_html_class("facetapi-facet-{$facet['name']}"),
        'id' => drupal_html_id("facetapi-facet-$searcher-{$realm['name']}-{$facet['name']}"),
      ),
    );

    // Applies sorting algorithms to the render array.
    $this->sortFacet($this->build);

    // Sets the key of the element as appended to the realm's render array.
    $this->key = $facet['field alias'];

    // Gets widget from settings, finds default if necessary.
    // @todo Implement default widgets.
    $widget_name = $this->build['#settings']->settings['widget'];

    // Initializes JavaScript settings.
    $this->jsSettings = array(
      'id' => $this->build['#attributes']['id'],
      'searcher' => $searcher,
      'type' => $facet->getAdapter()->getType(),
      'realmName' => $realm['name'],
      'facetName' => $facet['name'],
      'queryType' => $facet['query type'],
      'widget' => $settings->settings['widget'],
      'limit' => $settings->settings['soft_limit'],
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

  /**
   * Applies selected sorting algorithms to the render array.
   *
   * @param array &$build
   *   The facet's render array.
   */
  function sortFacet(array &$build) {
    $settings = $build['#settings']->settings;

    // Gets active sort definitions.
    $this->sorts = array_intersect_key(
      facetapi_get_sort_info(),
      array_filter($settings['active_sorts'])
    );

    // Finalizes sort definitions with settings or defaults.
    foreach ($this->sorts as $name => &$info) {
      $info['weight'] = $settings['sort_weight'][$name];
      $info['order'] = $settings['sort_order'][$name];
    }

    // Orders the sorts, applies sorting algorithms in that order.
    uasort($this->sorts, 'drupal_sort_weight');
    $this->applySorts($build[$this->facet['field alias']]);
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
  $a_count = (isset($a['#markup'])) ? $a['#markup'] : '';
  $b_count = (isset($b['#markup'])) ? $b['#markup'] : '';
  return strcasecmp($a['#markup'], $b['#markup']);
}
