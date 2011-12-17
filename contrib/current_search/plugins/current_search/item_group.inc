<?php

/**
 * @file
 * Displays field groups.
 */

/**
 * Extension of CurrentSearchItem that displays field groups.
 */
class CurrentSearchGroup extends CurrentSearchItem {

  /**
   * Implements CurrentSearchItem::execute().
   */
  public function execute(FacetapiAdapter $adapter) {
    $groups = array();

    // Makes sure facet builds are initialized.
    $adapter->processFacets();

    // Adds other current search module's CSS.
    $path = drupal_get_path('module', 'current_search');
    drupal_add_css($path . '/current_search.css');

    // Adds active facets to the current search block.
    $searcher = $adapter->getSearcher();
    foreach ($adapter->getAllActiveItems() as $item) {
      $facet_name = $item['facets'][0];
      $facet_value = $item['value'];
      $groups[$facet_name][$facet_value] = $item;
    }

    // Iterates over groups, builds list.
    $build = array();
    foreach ($groups as $facet_name => $group) {
      $items = array();

      // Builds list items.
      foreach ($group as $item) {
        $markup = $adapter->getMappedValue($item['facets'][0], $item['value']);
        $text = ($markup['#html']) ? $markup['#markup'] : check_plain($markup['#markup']);
        $variables = array(
          'text' => $text,
          'path' => current_path(),
          'options' => array(
            'attributes' => array('class' => array()),
            'html' => TRUE,
            'query' => $this->getQueryString($item, $adapter),
          ),
        );
        $items[] = theme('current_search_link_active', $variables);
      }

      // If there are items, add the render array.
      if ($items) {
        $build[$facet_name]['#theme_wrappers'] = array('current_search_group_wrapper');

        // Performs token replacemenets and themes the group title.
        $facet = facetapi_facet_load($facet_name, $searcher);
        $title = filter_xss(token_replace($this->settings['field_pattern'], array('facetapi_facet' => $facet)));
        $build[$facet_name]['title']['#markup'] = theme('current_search_group_title', array('title' => $title));

        // Builds the list.
        $build[$facet_name]['list'] = array(
          '#theme' => 'item_list',
          '#items' => $items,
          '#attributes' => array('class' => array('inline')),
        );
      }
    }

    return $build;
  }

  /**
   * Implements CurrentSearchItem::settingsForm().
   */
  public function settingsForm(&$form, &$form_state) {

    $form['field_pattern'] = array(
      '#type' => 'textfield',
      '#title' => t('Field pattern'),
      '#default_value' => $this->settings['field_pattern'],
      '#maxlength' => 255,
      '#description' => t('The pattern of the field label preceeding the links. Token replacement patterns are allowed.'),
    );

    // Adds token tree.
    $form['tokens'] = $this->getTokenTree(array('facetapi_facet'));
  }

  /**
   * Implements CurrentSearchItem::getDefaultSettings().
   */
  public function getDefaultSettings() {
    return array(
      'field_pattern' => '[facetapi_facet:facet-label]:',
    );
  }
}
