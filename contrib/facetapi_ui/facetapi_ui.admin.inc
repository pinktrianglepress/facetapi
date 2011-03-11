<?php

/**
 * @file
 *
 */

/**
 * Facet settings
 */
function facetapi_ui_realm_settings_form($form, &$form_state, $searcher, $realm_name) {

  // Gets adapter.
  $adapter = facetapi_adapter_load($searcher);

  // Gets realm repositories.
  $realm_info = facetapi_get_realm_info();
  $realm = $realm_info[$realm_name];

  $facet_info = facetapi_get_facet_info($searcher, $adapter->getType());

  $form['description'] = array(
    '#markup' => filter_xss_admin($realm['description']),
  );

  $form['table'] = array(
    '#theme' => 'facetapi_ui_realm_settings_table',
    '#facetapi' => array(
      'searcher' => $searcher,
      'realm' => $realm,
      'facet_info' => $facet_info,
    ),
  );

  // Builds "enabled_facets" options.
  $options = $default_value = array();
  foreach ($facet_info as $facet_name => $facet) {
    $settings = facetapi_get_facet_settings($searcher, $realm_name, $facet_name);

    // Gets weight, appends to facet_info array for sorting.
    $form['table']['#facetapi']['facet_info'][$facet_name]['weight'] = $settings->settings['weight'];

    $form['table']['weight:' . $facet_name] = array(
      '#type' => 'select',
      '#options' => drupal_map_assoc(range(-50, 50)),
      '#default_value' => $settings->settings['weight'],
      '#attributes' => array('class' => array('facetapi-facet-weight')),
    );

    $options[$facet_name] = '';
    $default_value[$facet_name] = $settings->settings['enabled'];
  }

  // Sorts by the weight appended above.
  uasort($form['table']['#facetapi']['facet_info'], 'drupal_sort_weight');

  $form['table']['enabled_facets'] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => $default_value,
  );

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 20,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  $form['#submit'][] = 'facetapi_ui_realm_settings_form_submit';

  return $form;
}

/**
 *
 */
function facetapi_ui_realm_settings_form_submit($form, &$form_state) {

  // Pulls variables for code readability.
  $searcher = $form['table']['#facetapi']['searcher'];
  $realm_name = $form['table']['#facetapi']['realm']['name'];
  $facet_info = $form['table']['#facetapi']['facet_info'];

  foreach ($facet_info as $facet_name => $facet) {
    $settings = facetapi_get_facet_settings($searcher, $realm_name, $facet_name);
    $settings->settings['enabled'] = $form_state['values']['enabled_facets'][$facet_name];
    $settings->settings['weight'] = $form_state['values']['weight:' . $facet_name];
    if (FALSE === ctools_export_crud_save('facetapi', $settings)) {
      drupal_set_message(t('Error saving settings.'), 'error');
    }
  }
}

/**
 * Themes a realm settings table.
 */
function theme_facetapi_ui_realm_settings_table($variables) {
  $output = '';

  // Gets variales for code readability.
  $searcher = $variables['element']['#facetapi']['searcher'];
  $realm_name = $variables['element']['#facetapi']['realm']['name'];

  $header = array(
    'enabled' => array(),
    'label' => array('data' => t('Facet')),
    'operations' => array('data' => t('Operations'), 'colspan' => 2),
    'weight' => array('data' => t('Weight')),
  );

  // Builds field options.
  $rows = array();
  $facet_info = $variables['element']['#facetapi']['facet_info'];
  foreach ($facet_info as $facet_name => $facet) {
    $rows[$facet_name] = array(
      'class' => array('draggable'),
      'data' => array(
        'indexed' => array(
          'data' => drupal_render($variables['element']['enabled_facets'][$facet_name]),
        ),
        'label' => array(
          'data' => check_plain($facet['label']) . "<div class='description'>" . filter_xss($facet['description']) . '</div>',
        ),
        'operations-1' => array(
          'data' => array(
            '#type' => 'link',
            '#title' => t('Edit'),
            '#href' => facetapi_ui_get_settings_path($searcher, $realm_name, $facet_name, 'edit'),
          ),
        ),
        'operations-2' => array(
          'data' => array(
            '#type' => 'link',
            '#title' => t('Export'),
            '#href' => facetapi_ui_get_settings_path($searcher, $realm_name, $facet_name, 'export'),
          ),
        ),
        'weight' => array(
          'data' => drupal_render($variables['element']['weight:' . $facet_name]),
        ),
      ),
    );
  }

  drupal_add_tabledrag('facetapi-ui-realm-settings', 'order', 'sibling', 'facetapi-facet-weight');
  $output .= drupal_render_children($variables['element']);
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'facetapi-ui-realm-settings')));

  return $output;
}

/**
 * Returns path to search page.
 */
function facetapi_ui_get_settings_path($searcher, $realm_name, $facet_name, $op) {
  $path_parts = array('admin', 'config', 'search', 'facetapi');
  $path_parts[] = $searcher;
  $path_parts[] = $realm_name;
  $path_parts[] = $facet_name;
  $path_parts[] = $op;
  return join('/', $path_parts);
}

/**
 *
 */
function facetapi_ui_facet_settings_form($form, &$form_state, $adapter, array $realm, array $facet) {

  $searcher = $adapter->getSearcher();
  if (!$settings = facetapi_get_facet_settings($searcher, $realm['name'], $facet['name'])) {
    $settings = ctools_export_crud_new('facetapi');
    $settings->name = $searcher . ':' . $realm['name'] . ':' . $facet['name'];
    $settings->settings = array();
  }

  $form['global'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global settings'),
    '#description' => t(
      'Configuration settings for the %facet facet across <em>all</em> realms that apply to %searcher searches.',
      array(
        '%facet' => $facet['label'],
        '%searcher' => $searcher,
      )
    ),
    '#collapsible' => TRUE,
  );

  $form['global']['operator'] = array(
    '#type' => 'radios',
    '#title' => t('Operator'),
    '#default_value' => $settings->settings['operator'],
    '#options' => array(
      FACETAPI_OPERATOR_AND => t('AND'),
      FACETAPI_OPERATOR_OR => t('OR'),
    ),
    '#description' => t('AND filters are exclusive. OR filters are inclusive. Selecting more AND filters narrows the result set. Selecting more OR filters widens the result set.')
  );

  $form['global']['hard_limit'] = array(
    '#type' => 'select',
    '#title' => t('Hard limit'),
    '#default_value' => $settings->settings['hard_limit'],
    '#options' => drupal_map_assoc(array(100, 75, 50, 40, 30, 20, 15, 10, 5, 3)),
    '#description' => t('Display no more than this number of facet items.')
  );

  $form['realm'] = array(
    '#type' => 'fieldset',
    '#title' => t('@realm realm settings', array('@realm' => $realm['label'])),
    '#description' => t(
      'Configuration settings for the %facet facet in the %realm realm that apply to %searcher searches.',
      array(
        '%facet' => $facet['label'],
        '%realm' => $realm['label'],
        '%searcher' => $adapter->getSearcher(),
      )
    ),
    '#collapsible' => TRUE,
  );

  // Gets widgets, sorts.
  $widget_info = facetapi_get_widget_info(array(
    'realm' => $realm,
    'facet' => $facet,
  ));
  uasort($widget_info, 'drupal_sort_weight');

  // Builds select options for widgets.
  $widget_options = array();
  foreach ($widget_info as $widget_name => $widget) {
    $widget_options[$widget_name] = $widget['label'];
  }

  $form['realm']['widget'] = array(
    '#type' => 'select',
    '#title' => t('Display widget'),
    '#default_value' => $settings->settings['widget'],
    '#options' => $widget_options,
  );

  $form['realm']['soft_limit'] = array(
    '#type' => 'select',
    '#title' => t('Soft limit'),
    '#default_value' => $settings->settings['soft_limit'],
    '#options' => array(0 => t('No limit')) + drupal_map_assoc(array(50, 40, 30, 20, 15, 10, 5, 3)),
    '#description' => t('Limits the number of displayed facets via JavaScript.'),
  );


  // Gets all sort definitions, sorts active for this facet.
  $sort_info = facetapi_get_sort_info();
  //$active_sorts = facetapi_facet_sorts_get($adapter, $realm, $facet);
  $active_sorts = array();

  // Initializes the sorting table.
  $form['realm']['sort'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sorting options'),
    '#description' => t('Determine how facet items are sorted. Selecting the checkbox activates the associated sort, and the sorts are applied from the top down. Click and drag the rows to change the order the sorts are applied.'),
  );

  $form['realm']['sort']['table'] = array(
    '#theme' => 'facetapi_ui_sort_settings_table',
    '#facetapi' => array(
      'adapter' => $adapter,
      'realm' => $realm,
      'facet' => $facet,
      'sort_info' => $sort_info,
    ),
  );

  // Builds checkbox options for sorts.
  $sort_options = array();
  foreach ($sort_info as $sort_name => $sort) {
    $sort_options[$sort_name] = '';

    $form['realm']['sort']['table']['sort_order:' . $sort_name] = array(
      '#type' => 'select',
      '#title' => '',
      '#options' => array(
        SORT_DESC => t('Descending'),
        SORT_ASC => t('Ascending'),
      ),
      '#default_value' => SORT_ASC,
    );

    $form['realm']['sort']['table']['sort_weight:'. $sort_name] = array(
      '#type' => 'weight',
      '#title' => '',
      '#delta' => 50,
      '#default_value' => 0,
      '#attributes' => array('class' => array('facetapi-sort-weight')),
    );
  }

  $form['realm']['sort']['table']['sort'] = array(
    '#type' => 'checkboxes',
    '#options' => $sort_options,
    '#default_value' => array(),
  );

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 20,
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  $adapter_info = facetapi_get_adapter_info();
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => $adapter_info[$searcher]['path'] . '/facets/' . $realm['name'],
  );

  return $form;
}

/**
 * Themes a sort settings table.
 */
function theme_facetapi_ui_sort_settings_table($variables) {
  $output = '';

  // Gets variales for code readability.
  $searcher = $variables['element']['#facetapi']['adapter']->getSearcher();

  $header = array(
    'enabled' => array(),
    'label' => array('data' => t('Sort')),
    'order' => array('data' => t('Order')),
    'weight' => array('data' => t('Weight')),
  );

  // Builds field options.
  $rows = array();
  $sort_info = $variables['element']['#facetapi']['sort_info'];
  foreach ($sort_info as $sort_name => $sort) {
    $rows[$sort_name] = array(
      'class' => array('draggable'),
      'data' => array(
        'enabled' => array(
          'data' => drupal_render($variables['element']['sort'][$sort_name]),
        ),
        'label' => array(
          'data' => check_plain($sort['label']) . "<div class='description'>" . filter_xss($sort['description']) . '</div>',
        ),
        'order' => array(
          'data' => drupal_render($variables['element']['sort_order:' . $sort_name]),
        ),
        'weight' => array(
          'data' => drupal_render($variables['element']['sort_weight:' . $sort_name]),
        ),
      ),
    );
  }

  drupal_add_tabledrag('facetapi-ui-sort-settings', 'order', 'sibling', 'facetapi-sort-weight');
  $output .= drupal_render_children($variables['element']);
  $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'facetapi-ui-sort-settings')));

  return $output;
}