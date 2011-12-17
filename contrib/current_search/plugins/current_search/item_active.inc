<?php

/**
 * @file
 * Displays all active items.
 */

/**
 * Extension of CurrentSearchItem that displays all active items.
 */
class CurrentSearchItemActive extends CurrentSearchItem {

  /**
   * Implements CurrentSearchItem::execute().
   */
  public function execute(FacetapiAdapter $adapter) {
    $items = array();

    // Makes sure facet builds are initialized.
    $adapter->processFacets();

    // Adds search keys.
    if ($this->settings['keys']) {
      $items[] = check_plain($adapter->getSearchKeys());
    }

    // Adds active facets to the current search block.
    foreach ($adapter->getAllActiveItems() as $item) {
      // Adds adapter to the active item for token replacement.
      $item['adapter'] = $adapter;

      // Builds variables to pass to theme function.
      $variables = array(
        'text' => token_replace($this->settings['pattern'], array('facetapi_active_item' => $item)),
        'path' => current_path(),
        'options' => array(
          'attributes' => array('class' => array()),
          'html' => TRUE,
          'query' => $this->getQueryString($item, $adapter),
        ),
      );

      // Renders the active link.
      $items[] = theme('facetapi_link_active', $variables);
    }

    // If there are items, return the render array.
    if ($items) {
      $classes = ($this->settings['css']) ? current_search_get_classes($this->settings['classes']) : array();
      return array(
        '#theme' => 'item_list',
        '#items' => $items,
        '#attributes' => array('class' => $classes),
      );
    }
  }

  /**
   * Implements CurrentSearchItem::settingsForm().
   */
  public function settingsForm(&$form, &$form_state) {

    $form['pattern'] = array(
      '#type' => 'textfield',
      '#title' => t('Pattern'),
      '#default_value' => $this->settings['pattern'],
      '#description' => t('The pattern used to render active items in the list. Token replacement patterns are allowed.'),
      '#maxlength' => 255,
    );

    $form['keys'] = array(
      '#type' => 'checkbox',
      '#title' => t('Append the keywords passed by the user to the list'),
      '#default_value' => $this->settings['keys'],
    );

    $form['css'] = array(
      '#type' => 'checkbox',
      '#title' => t('Add CSS classes to wrapper element'),
      '#default_value' => $this->settings['css'],
    );

    $form['classes'] = array(
      '#type' => 'textfield',
      '#title' => t('CSS classes'),
      '#default_value' => $this->settings['classes'],
      '#description' => t('A comma separated list of CSS classes.'),
      '#maxlength' => 128,
      '#states' => array(
        'visible' => array(
          ':input[name="plugin_settings[' . $this->name . '][css]"]' => array('checked' => TRUE),
        ),
      ),
    );

    // Adds token tree.
    $form['tokens'] = $this->getTokenTree(array('facetapi_active'));
  }

  /**
   * Implements CurrentSearchItem::getDefaultSettings().
   */
  public function getDefaultSettings() {
    return array(
      'pattern' => '[facetapi_active:active-value]',
      'keys' => FALSE,
      'css' => FALSE,
      'classes' => '',
    );
  }
}
