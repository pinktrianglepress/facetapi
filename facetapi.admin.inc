<?php
// $Id$

/**
 * @file
 * Administrative settings for Facet API modules.
 */

/**
 * Administrative settings for Search Lucene Facets.
 *
 * @param &$form_state
 *   A keyed array containing the current state of the form.
 * @param $searcher
 *   A string containing the machine readable name of the searcher module.
 * @return
 *   A FAPI array passed through system_settings_form().
 */
function facetapi_admin_settings_form(&$form_state, $searcher) {
  $type = facetapi_adapter_load($searcher)->getType();

  // Common url() function options that will redirect form submissions in linked
  // pages back to this form.
  $url_options = array('query' => array('destination' => $_GET['q']));

  // Values that must persist.
  $form['storage'] = array(
    '#type' => 'value',
    '#value' => array(
      'searcher' => $searcher,
      'realms' => facetapi_realms_get(),
      'facets' => facetapi_facets_get($searcher, $type),
    ),
  );

  // Adds description, submit handler if there are facets.
  if (!empty($form['storage']['#value']['facets'])) {
    $form['description']['#value'] = t(
      'This page provides a drag-and-drop interface to enable certian facets in each realm and control the order they are shown on the page.  <em>Realms</em> are groups of facets that are displayed in a similar fashion on the search page. Since rendering is handled by the relam, a single facet may be displayed in different ways, for exmaple a form element or a list of clickable links. Like the core search, users need the <em>use advanced search</em> <a href="@permissions-page">permissions</a> to be able to use the facets.',
      array('@permissions-page' => url('admin/user/permissions', $url_options))
    );
  }

  // Iterates over realms, builds form for each realm.
  foreach ($form['storage']['#value']['realms'] as $realm_name => $realm) {
    if (!empty($form['storage']['#value']['facets'])) {

      // Gets facets, formats options array.
      $options = array();
      $sorted_facets = $form['storage']['#value']['facets'];
      facetapi_facets_sort($sorted_facets, $searcher, $realm_name);
      foreach ($sorted_facets as $facet_name => $facet) {
        $options[$facet_name] = '';
      }

      // Index settings fieldset.
      $form['facets'][$realm_name] = array(
        '#type' => 'fieldset',
        '#title' => t('Realm: @name', array('@name' => $realm['title'])),
        '#collapsible' => TRUE,
      );

      // Adds description of relam if one was provided.
      if (isset($realm['description'])) {
        $form['facets'][$realm_name]['#description'] = filter_xss($realm['description']);
      }

      // Adds facet checkboxes.
      $checkbox_var = sprintf('facetapi:facet_status:%s:%s', $searcher, $realm_name);
      $form['facets'][$realm_name]['table'][$checkbox_var] = array(
        '#type' => 'checkboxes',
        '#options' => $options,
        '#default_value' => variable_get($checkbox_var, array()),
      );

      // Adds facet weight dropboxes if relam is "sortable".
      if ($realm['sortable']) {
        foreach ($options as $facet_name => $title) {
          $weight_var = sprintf('facetapi:facet_weight:%s:%s:%s', $searcher, $realm_name, $facet_name);
          $form['weight'][$realm_name][$weight_var] = array(
            '#type' => 'weight',
            '#title' => '',
            '#delta' => 50,
            '#default_value' => variable_get($weight_var, 0),
            '#attributes' => array('class' => 'facetapi-facet-weight'),
          );
        }
      }

      // Adds "fieldset" specific option to expand fieldset if facets selected.
      if ('fieldset' == $realm_name) {
        $variable = sprintf('facetapi:expand_fieldset:%s:fieldset', $searcher);
        $form['facets'][$realm_name][$variable] = array(
          '#type' => 'checkbox',
          '#title' => t('Expand fieldset on faceted search'),
          '#default_value' => variable_get($variable, 1),
          '#description' => t('When facets are selected, the fieldset will remain expanded so users can more easily refine their search.'),
        );
      }
    }
    else {
      $form['facets'] = array(
        '#value' => t('No facets are available to this module.'),
      );
      return $form;
    }
  }

  // Finalizes the form and returns.
  $form = system_settings_form($form);
  $form['#theme'] = 'facetapi_admin_settings_form';
  $form['#submit'][] = 'facetapi_admin_settings_form_submit';
  return $form;
}

/**
 * Themes the facet form into a draggable table.
 *
 * @param $form
 *   A FAPI array containing a fieldset.
 * @return
 *   A themed form element.
 */
function theme_facetapi_admin_settings_form($form) {
  // Gets storage items for code readability.
  $storage = $form['storage']['#value'];

  // Initializes table headers.
  $headers = array();
  $headers[] = array('data' => t('Enabled'), 'class' => 'checkbox');
  $headers[] = t('Facet');
  $headers[] = t('Description');
  $headers[] = t('');
  $headers[] = t('Weight');

  // If there are facets defined for this module, adds them.
  foreach (element_children($form['facets']) as $realm_name) {
    $form['facets'][$realm_name]['table']['#value'] = '';
    $rows = array();

    // Iterates over facets, builds table rows.
    foreach (element_children($form['facets'][$realm_name]['table']) as $checkbox_var) {

      // Gets sorted facet names, iterates over facets to build rows.
      $facet_names = array_keys($form['facets'][$realm_name]['table'][$checkbox_var]['#options']);
      foreach ($facet_names as $facet_name) {
        $row = array();

        // Gets full facet definition from storage.
        $facet = $storage['facets'][$facet_name];

        // Builds out all rows except for the weight column.
        $row['data'] = array(
          array(
            'data' => drupal_render($form['facets'][$realm_name]['table'][$checkbox_var][$facet_name]),
            'class' => 'checkbox'
          ),
          array(
            'data' => check_plain($facet['title']),
          ),
          array(
            'data' => isset($facet['description']) ? filter_xss($facet['description']) : '',
          ),
          array(
            'data' => l(t('Configure'), sprintf(
              'admin/settings/%s/facetapi/%s/%s',
              $storage['searcher'], $realm_name, $facet_name
            )),
          ),
        );

        // Adds weight column if realm is "sortable".
        if ($storage['realms'][$realm_name]['sortable']) {
          $weight_var = sprintf('facetapi:facet_weight:%s:%s:%s', $storage['searcher'], $realm_name, $facet_name);
          $row['class'] = 'draggable';
          $row['data'][] = array(
            'data' => drupal_render($form['weight'][$realm_name][$weight_var]),
            'class' => 'dropbox'
          );
        }

        // Appends the row.
        $rows[] = $row;
      }
    }

    // Builds the CSS ID for the table.
    $table_id = sprintf('facetapi-%s-%s-table', $storage['searcher'], $realm_name);

    // Adds tabledrag if sortable, otherwise removes weight header.
    $row_headers = $headers;
    if ($storage['realms'][$realm_name]['sortable']) {
      drupal_add_tabledrag($table_id, 'order', 'sibling', 'facetapi-facet-weight');
    }
    else {
      array_pop($row_headers);
    }

    // Themes the facet table.
    $form['facets'][$realm_name]['table']['#value'] .= theme(
      'table', $row_headers, $rows, array('id' => $table_id)
    );
  }

  // Returns the form rendered as a table.
  return drupal_render($form);
}

/**
 * Processes facetapi_admin_settings_form for submissions.
 */
function facetapi_admin_settings_form_submit(&$form, &$form_state) {
  cache_clear_all('facetapi:', 'cache', TRUE);
}

/**
 * Returns settings for an individual facet that apply to a realm.
 *
 * @param &$form_state
 *   A keyed array containing the current state of the form.
 * @param $adapter
 *   A FacetapiAdapter object.
 * @param $realm
 *   An array containing the full realm definition.
 * @param $facet
 *   An array containing the fill realm definition.
 * @return
 *   A FAPI array containing the form.
 */
function facetapi_facet_settings_form(&$form_state, FacetapiAdapter $adapter, array $realm, array $facet) {
  $form = array();

  $form['global'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global settings'),
    '#description' => t(
      'Configuration settings for the %facet facet across <em>all</em> realms that apply to %searcher searches.',
      array(
        '%facet' => $facet['title'],
        '%searcher' => $adapter->getSearcher(),
      )
    ),
    '#collapsible' => TRUE,
  );

  $operator_var = sprintf(
    'facetapi:operator:%s::%s',
    $adapter->getSearcher(), $facet['name']
  );
  $form['global'][$operator_var] = array(
    '#type' => 'radios',
    '#title' => t('Operator'),
    '#default_value' => variable_get($operator_var, FACETAPI_OPERATOR_AND),
    '#options' => array(
      FACETAPI_OPERATOR_AND => t('AND'),
      FACETAPI_OPERATOR_OR => t('OR'),
    ),
    '#description' => t('AND filters are exclusive. OR filters are inclusive. Selecting more AND filters narrows the result set. Selecting more OR filters widens the result set.')
  );

  $hlimit_var = sprintf(
    'facetapi:hard_limit:%s::%s',
    $adapter->getSearcher(), $facet['name']
  );
  $form['global'][$hlimit_var] = array(
    '#type' => 'select',
    '#title' => t('Hard limit'),
    '#default_value' => variable_get($hlimit_var, 20),
    '#options' => drupal_map_assoc(array(100, 75, 50, 40, 30, 20, 15, 10, 5, 3)),
    '#description' => t('Display no more than this number of facets.')
  );

  $form['realm'] = array(
    '#type' => 'fieldset',
    '#title' => t('@realm realm settings', array('@realm' => $realm['title'])),
    '#description' => t(
      'Configuration settings for the %facet facet in the %realm realm that apply to %searcher searches.',
      array(
        '%facet' => $facet['title'],
        '%realm' => $realm['title'],
        '%searcher' => $adapter->getSearcher(),
      )
    ),
    '#collapsible' => TRUE,
  );


  // Gets widgets, sorts.
  $widgets = module_invoke_all('facetapi_widget');
  uasort($widgets, 'facetapi_sort_weight');

  // Builds options.
  $options = array();
  foreach ($widgets as $widget_name => $definition) {
    $options[$widget_name] = $definition['title'];
  }

  $widget_var = sprintf(
    'facetapi:widget:%s:%s:%s',
    $adapter->getSearcher(), $realm['name'], $facet['name']
  );
  $form['realm'][$widget_var] = array(
    '#type' => 'radios',
    '#title' => t('Display widget'),
    '#default_value' => variable_get($widget_var, 'facetapi_links'),
    '#options' => $options,
  );

  $sort_var = sprintf(
    'facetapi:sort:%s:%s:%s',
    $adapter->getSearcher(), $realm['name'], $facet['name']
  );
  $form['realm'][$sort_var] = array(
    '#type' => 'radios',
    '#title' => t('Sort order'),
    '#default_value' => variable_get($sort_var, 0),
    '#options' => array(
      0 => t('Count'),
      SORT_STRING .':'. SORT_ASC => t('Alphanumeric, ascending'),
      SORT_STRING .':'. SORT_DESC => t('Alphanumeric, descending'),
      SORT_NUMERIC .':'. SORT_ASC => t('Key sort, ascending'),
      SORT_NUMERIC .':'. SORT_DESC => t('Key sort, descending'),
    ),
    '#description' => t('The sort order of facet links in this block. <em>Count</em>, which is the default, will show facets with the most results first. <em>Alphanumeric</em> will sort alphabetically, either ascending or descending. <em>Key</em> sorts by the raw value indexed in the search engine, usually a machine readable ID.')
  );

  $slimit_var = sprintf(
    'facetapi:soft_limit:%s:%s:%s',
    $adapter->getSearcher(), $realm['name'], $facet['name']
  );
  $form['realm'][$slimit_var] = array(
    '#type' => 'select',
    '#title' => t('Soft limit'),
    '#default_value' => variable_get($slimit_var, 10),
    '#options' => drupal_map_assoc(array(50, 40, 30, 20, 15, 10, 5, 3)),
    '#description' => t('Limits the number of displayed facets via jQuery.')
  );

  return system_settings_form($form);
}
