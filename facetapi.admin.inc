<?php

/**
 * @file
 * Facet API administrative settings forms.
 */

/**
 * Realm settings form.
 *
 * @param $form
 *
 * @param &$form_state
 *
 * @param $searcher
 *   A string containing the machine readable name of the searcher.
 * @param $realm_name
 *   A string containing the machine readable name of the realm.
 *
 * @return
 *   An array containing the form.
 *
 * @ingroup forms
 */
function facetapi_realm_settings_form($form, &$form_state, $searcher, $realm_name) {

  // Instantiates adapter, loads realm.
  $adapter = facetapi_adapter_load($searcher);
  $realm = facetapi_realm_load($realm_name);
  $form['#facetapi'] = array(
    'adapter' => $adapter,
    'realm' => $realm,
    'facet_info' => facetapi_get_facet_info($searcher),
  );

  $form['description'] = array(
    '#prefix' => '<div class="facetapi-realm-description">',
    '#markup' => filter_xss_admin($realm['description']),
    '#suffix' => "</div>\n",
  );
  $form['performance'] = array(
    '#prefix' => '<div class="facetapi-performance-note">',
    '#markup' => t('For performance reasons, you should only enable facets that you intend to have available to users on the search page.'),
    '#suffix' => "</div>\n",
  );

  $form['table'] = array(
    '#theme' => 'facetapi_realm_settings_table',
    '#facetapi' => &$form['#facetapi'],
  );

  // Builds "enabled_facets" options.
  $options = $default_value = array();
  foreach ($form['#facetapi']['facet_info'] as $facet_name => $facet) {
    $settings = $adapter->getFacet($facet)->getSettings($realm);

    // Gets weight, appends to facet_info array for sorting.
    $form['#facetapi']['facet_info'][$facet_name]['weight'] = $settings->settings['weight'];

    $form['table']['weight:' . $facet_name] = array(
      '#type' => 'select',
      '#options' => drupal_map_assoc(range(-50, 50)),
      '#default_value' => $settings->settings['weight'],
      '#attributes' => array('class' => array('facetapi-facet-weight')),
    );

    $options[$facet_name] = '';
    $default_value[$facet_name] = empty($settings->enabled) ? 0: $facet_name;
  }

  // Sorts by the weight appended above.
  uasort($form['#facetapi']['facet_info'], 'drupal_sort_weight');

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

  $form['#submit'][] = 'facetapi_realm_settings_form_submit';

  return $form;
}

/**
 * Processes facetapi_realm_settings_form form submissions.
 */
function facetapi_realm_settings_form_submit($form, &$form_state) {
  $success = TRUE;

  // Pulls variables for code readability.
  $adapter = $form['#facetapi']['adapter'];
  $realm = $form['#facetapi']['realm'];
  $facet_info = $form['#facetapi']['facet_info'];

  // Builds settings object, saves to database.
  foreach ($facet_info as $facet_name => $facet) {
    $weight = $form_state['values']['weight:' . $facet_name];
    $settings = $adapter->getFacet($facet)->getSettings($realm);
    $settings->enabled = empty($form_state['values']['enabled_facets'][$facet_name]) ? 0 : 1;
    $settings->settings['weight'] = $weight;
    if (FALSE === ctools_export_crud_save('facetapi', $settings)) {
      drupal_set_message(t('Error saving settings.'), 'error');
      $success = FALSE;
    }
  }

  // Sets message if all configurations were saved.
  if ($success && !empty($realm['submit message'])) {
    drupal_set_message($realm['submit message']);
  }

  // Clears delta map cache.
  cache_clear_all('facetapi:delta_map', 'cache');
}

/**
 * Themes a realm settings table.
 */
function theme_facetapi_realm_settings_table($variables) {
  $output = '';

  // Adds CSS.
  drupal_add_css(drupal_get_path('module', 'facetapi') . '/facetapi.css');

  // Gets variales for code readability.
  $searcher = $variables['element']['#facetapi']['adapter']->getSearcher();
  $realm_name = $variables['element']['#facetapi']['realm']['name'];

  $header = array(
    'enabled' => array(),
    'label' => array('data' => t('Facet')),
    'operations' => array('data' => t('Operations')),
    'weight' => array('data' => t('Weight')),
  );

  // Builds field options.
  $rows = array();
  $facet_info = $variables['element']['#facetapi']['facet_info'];
  foreach ($facet_info as $facet_name => $facet) {

    // Builds array of operations to use in the dropbutton.
    $operations = array();
    $operations[] = array(
      'title' => t('Edit'),
      'href' => facetapi_get_settings_path($searcher, $realm_name, $facet_name, 'edit')
    );
    if ($facet['dependency plugins']) {
      $operations[] = array(
        'title' => t('Dependencies'),
        'href' => facetapi_get_settings_path($searcher, $realm_name, $facet_name, 'dependencies')
      );
    }
    $operations[] = array(
      'title' => t('Export'),
      'href' => facetapi_get_settings_path($searcher, $realm_name, $facet_name, 'export')
    );

    // Builds rows.
    $rows[$facet_name] = array(
      'class' => array('draggable'),
      'data' => array(
        'indexed' => array(
          'data' => drupal_render($variables['element']['enabled_facets'][$facet_name]),
        ),
        'label' => array(
          'data' => check_plain($facet['label']) . "<div class='description'>" . filter_xss($facet['description']) . '</div>',
        ),
        'operations' => array(
          'class' => 'facetapi-operations',
          'data' => theme('links__ctools_dropbutton', array(
            'links' => $operations,
            'attributes' => array(
              'class' => array('inline', 'links', 'actions', 'horizontal', 'right')
            ),
          )),
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
function facetapi_get_settings_path($searcher, $realm_name, $facet_name, $op) {
  $path_parts = array('admin', 'config', 'search', 'facetapi');
  $path_parts[] = $searcher;
  $path_parts[] = $realm_name;
  $path_parts[] = $facet_name;
  $path_parts[] = $op;
  return join('/', $path_parts);
}

/**
 * Facet display settings form.
 *
 * @param $form
 *
 * @param &$form_state
 *
 * @param FacetapiAdapter $adapter
 *   The adapter the settings apply to.
 * @param array $realm
 *   The realm definition.
 * @param array $facet
 *   The facet definition.
 *
 * @return
 *   An array containing the form.
 *
 * @ingroup forms
 */
function facetapi_facet_settings_form($form, &$form_state, FacetapiAdapter $adapter, array $realm, array $facet) {

  $searcher = $adapter->getSearcher();
  $facet_settings = $adapter->getFacet($facet)->getSettings($realm);
  $global_settings = $adapter->getFacet($facet)->getSettings();

  // Adds Facet API settings, excluded values aren't saved.
  $form['#facetapi'] = array(
    'adapter' => $adapter,
    'realm' => $realm,
    'facet' => $facet,
    'sort_info' => array(),
    'excluded_values' => array(
      'form_build_id', 'form_token', 'form_id', 'op', 'submit', 'submit_list',
      'settings__active_tab',
    ),
  );

  // Orders the sorts by weight.
  foreach (facetapi_get_sort_info() as $sort_name => $sort_info) {
    // @todo Figure out a better default system.
    if (isset($facet_settings->settings['sort_weight'][$sort_name])) {
      $weight = $facet_settings->settings['sort_weight'][$sort_name];
    }
    else {
      $weight = 0;
    }
    $form['#facetapi']['sort_info'][$sort_name] = $sort_info;
    $form['#facetapi']['sort_info'][$sort_name]['weight'] = $weight;
  }
  uasort($form['#facetapi']['sort_info'], 'drupal_sort_weight');

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

  $all_options = array(
    FACETAPI_OPERATOR_AND => t('AND'),
    FACETAPI_OPERATOR_OR => t('OR'),
  );
  $allowed = array_filter($facet['allowed operators']);
  $options = array_intersect_key($all_options, $allowed);
  $form['global']['operator'] = array(
    '#type' => 'radios',
    '#access' => count($options) > 1,
    '#title' => t('Operator'),
    '#default_value' => $global_settings->settings['operator'],
    '#options' => $options,
    '#description' => t('AND filters are exclusive. OR filters are inclusive. Selecting more AND filters narrows the result set. Selecting more OR filters widens the result set.')
  );

  $form['global']['hard_limit'] = array(
    '#type' => 'select',
    '#title' => t('Hard limit'),
    '#default_value' => $global_settings->settings['hard_limit'],
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
  /*
  $widget_info = facetapi_get_widget_info(array(
    'realm' => $realm,
    'facet' => $facet,
  ));
  uasort($widget_info, 'drupal_sort_weight');
  */

  // Builds select options for widgets.
  ctools_include('plugins');
  $widget_options = array();
  foreach (ctools_get_plugins('facetapi', 'widgets') as $id => $plugin) {
    $widget_options[$id] = $plugin['handler']['label'];
  }

  $form['realm']['widget'] = array(
    '#type' => 'select',
    '#title' => t('Display widget'),
    '#default_value' => $facet_settings->settings['widget'],
    '#options' => $widget_options,
  );

  $form['realm']['soft_limit'] = array(
    '#type' => 'select',
    '#title' => t('Soft limit'),
    '#default_value' => $facet_settings->settings['soft_limit'],
    '#options' => array(0 => t('No limit')) + drupal_map_assoc(array(50, 40, 30, 20, 15, 10, 5, 3)),
    '#description' => t('Limits the number of displayed facets via JavaScript.'),
  );

  // Initializes the sorting table.
  $form['realm']['sort'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sorting options'),
    '#description' => t('Determine how facet items are sorted. Selecting the checkbox activates the associated sort, and the sorts are applied from the top down. Click and drag the rows to change the order the sorts are applied.'),
  );

  $form['realm']['sort']['table'] = array(
    '#theme' => 'facetapi_sort_settings_table',
    '#facetapi' => &$form['#facetapi'],
  );

  // Builds checkbox options for sorts.
  $sort_options = array();
  foreach ($form['#facetapi']['sort_info'] as $sort_name => $sort) {
    $sort_options[$sort_name] = '';

    // @todo Figure out a better default system.
    if (isset($facet_settings->settings['sort_order'][$sort_name])) {
      $default_value = $facet_settings->settings['sort_order'][$sort_name];
    }
    else {
      $default_value = SORT_ASC;
    }
    $form['realm']['sort']['table']['sort_order:' . $sort_name] = array(
      '#type' => 'select',
      '#title' => '',
      '#options' => array(
        SORT_DESC => t('Descending'),
        SORT_ASC => t('Ascending'),
      ),
      '#default_value' => $default_value,
    );

    // @todo Figure out a better default system.
    if (isset($facet_settings->settings['sort_weight'][$sort_name])) {
      $default_value = $facet_settings->settings['sort_weight'][$sort_name];
    }
    else {
      $default_value = 0;
    }
    $form['realm']['sort']['table']['sort_weight:'. $sort_name] = array(
      '#type' => 'weight',
      '#title' => '',
      '#delta' => 50,
      '#default_value' => $default_value,
      '#attributes' => array('class' => array('facetapi-sort-weight')),
    );
  }

  $form['realm']['sort']['table']['active_sorts'] = array(
    '#type' => 'checkboxes',
    '#options' => $sort_options,
    '#default_value' => $facet_settings->settings['active_sorts'],
  );

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 20,
  );

  // Gets destination from query string which is set when the page is navigated
  // to via a contextual link.
  if (isset($_GET['destination']) && !url_is_external($_GET['destination'])) {
    $destination = $_GET['destination'];
  }
  else {
    $destination = FALSE;
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  // Do not show the button if the page was navigated to via a contextual link
  // because it redirects back to the page the user came from.
  if (!$destination) {
    $form['actions']['submit_realm'] = array(
      '#type' => 'submit',
      '#value' => t('Save and go back to realm settings'),
    );
  }

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => (!$destination) ? $adapter->getPath($realm['name']) : $destination,
  );

  $form['#submit'][] = 'facetapi_facet_settings_form_submit';

  return $form;
}

/**
 * Themes a sort settings table.
 */
function theme_facetapi_sort_settings_table($variables) {
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
          'data' => drupal_render($variables['element']['active_sorts'][$sort_name]),
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

/**
 * Processes facetapi_facet_settings_form submissions.
 */
function facetapi_facet_settings_form_submit($form, &$form_state) {

  // Pulls variables for code readability.
  $adapter = $form['#facetapi']['adapter'];
  $realm = $form['#facetapi']['realm'];
  $facet = $form['#facetapi']['facet'];

  // Loads settings, saves all form values as settings other than excluded.
  $facet_settings = $adapter->getFacet($facet)->getSettings($realm);
  $facet_settings->settings = array_merge($facet_settings->settings, array_diff_key(
    $form_state['values'],
    array_flip($form['#facetapi']['excluded_values'])
  ));
  unset($facet_settings->settings['operator']);
  unset($facet_settings->settings['hard_limit']);

  // Converts sort_weight:* and sort_order:* settings to associative arrays.
  facetapi_combine_settings('sort_weight', $facet_settings);
  facetapi_combine_settings('sort_order', $facet_settings);

  $global_settings = $adapter->getFacet($facet)->getSettings();
  $global_settings->settings['operator'] = $form_state['values']['operator'];
  $global_settings->settings['hard_limit'] = $form_state['values']['hard_limit'];

  $success = TRUE;
  if (FALSE === ctools_export_crud_save('facetapi', $facet_settings)) {
    drupal_set_message(t('Error saving configuration options.'), 'error');
    $success = FALSE;
  }
  if (FALSE === ctools_export_crud_save('facetapi', $global_settings)) {
    drupal_set_message(t('Error saving configuration options.'), 'error');
    $success = FALSE;
  }

  // Sets message if both sets of configurations were saved.
  if ($success) {
    drupal_set_message(t('The configuration options have been saved.'));
  }

  // Redirects back to the realm settings page if necessary.
  $clicked = $form_state['clicked_button']['#value'];
  if (t('Save and go back to realm settings') == $clicked) {
    $form_state['redirect'] = $adapter->getPath($realm['name']);
  }
}

/**
 * Combines settings named "setting:*" into a single array of settings.
 *
 * This makes it easier to process settings at runtime.
 *
 * @param $setting_name
 *   A string containing the name of the setting.
 * @param $settings
 *   An object containing the settings loaded from CTools.
 */
function facetapi_combine_settings($setting_name, $settings) {
  $start = strlen($setting_name) + 1;
  $settings->settings[$setting_name] = array();
  foreach ($settings->settings as $name => $value) {
    if (0 === strpos($name, $setting_name . ':')) {
      $key = substr($name, $start);
      unset($settings->settings[$name]);
      $settings->settings[$setting_name][$key] = $value;
    }
  }
}

/**
 * Facet dependency settings form.
 *
 * @param $form
 *
 * @param &$form_state
 *
 * @param FacetapiAdapter $adapter
 *   The adapter the settings apply to.
 * @param array $realm
 *   The realm definition.
 * @param array $dependencies
 *   An array of dependencies.
 *
 * @return
 *   An array containing the form.
 *
 * @ingroup forms
 */
function facetapi_facet_dependencies_form($form, &$form_state, FacetapiAdapter $adapter, array $realm, array $dependencies) {

  // Adds Facet API settings, excluded values aren't saved.
  $form['#facetapi'] = array(
    'adapter' => $adapter,
    'realm' => $realm,
    'excluded_values' => array(
      'form_build_id', 'form_token', 'form_id', 'op', 'submit', 'submit_list',
      'settings__active_tab',
    ),
  );

  $form['plugins'] = array(
    '#type' => 'vertical_tabs',
    '#weight' => 10,
  );

  // Iterates over plugins, adds settings as vertical tabs.
  $plugins = ctools_get_plugins('facetapi', 'dependencies');
  foreach ($dependencies as $plugin) {

    // Initializes vertical tab.
    $id = $plugin->getId();
    $form[$id] = array(
      '#type' => 'fieldset',
      '#title' => check_plain($plugins[$id]['handler']['label']),
      '#group' => 'plugins',
    );

    // Allows the plugin to add settings.
    $plugin->settingsForm($form, $form_state);

    // Removes vertical tab if nothing was added.
    if (!element_children($form[$id])) {
      unset($form[$id]);
    }
  }

  $form['actions'] = array(
    '#type' => 'actions',
    '#weight' => 20,
  );

  // Gets destination from query string which is set when the page is navigated
  // to via a contextual link.
  if (isset($_GET['destination']) && !url_is_external($_GET['destination'])) {
    $destination = $_GET['destination'];
  }
  else {
    $destination = FALSE;
  }

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  // Do not show the button if the page was navigated to via a contextual link
  // because it redirects back to the page the user came from.
  if (!$destination) {
    $form['actions']['submit_realm'] = array(
      '#type' => 'submit',
      '#value' => t('Save and go back to realm settings'),
    );
  }

  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => (!$destination) ? $adapter->getPath($realm['name']) : $destination,
  );

  $form['#submit'][] = 'facetapi_facet_dependencies_form_submit';

  return $form;
}

/**
 * Processes facetapi_facet_settings_form submissions.
 */
function facetapi_facet_dependencies_form_submit($form, &$form_state) {
  $success = TRUE;

  // Pulls variables for code readability.
  $adapter = $form['#facetapi']['adapter'];
  $realm = $form['#facetapi']['realm'];

  // Sets message if both sets of configurations were saved.
  if ($success) {
    drupal_set_message(t('The configuration options have been saved.'));
  }

  // Redirects back to the realm settings page if necessary.
  $clicked = $form_state['clicked_button']['#value'];
  if (t('Save and go back to realm settings') == $clicked) {
    $form_state['redirect'] = $adapter->getPath($realm['name']);
  }
}

/**
 * Export form, wrapper around ctools_export_form().
 *
 * @param $form
 *
 * @param &$form_state
 *
 * @param $settings
 *
 * @see ctools_export_form()
 */
function facetapi_export_form($form, &$form_state, $adapter, $realm, $facet) {

  // Adds link to get back to list page.
  $form['back'] = array(
    '#type' => 'link',
    '#title' => t('Go back to realm settings page'),
    '#href' => $adapter->getPath($realm['name']),
  );

  // Gets export fieldset from ctools_export_form().
  $title = t('Export configuration');
  $settings = $adapter->getFacet($facet)->getSettings($realm);
  $export = ctools_export_crud_export('facetapi', $settings);
  $form = ctools_export_form($form, $form_state, $export, $title);

  // Adds settings and returns form.
  $form['#settings'] = $settings;
  return $form;
}
