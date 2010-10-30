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
 *
 * @return
 *   A FAPI array passed through system_settings_form().
 */
function facetapi_admin_settings_form(&$form_state, $searcher) {
  $form = array();

  // Stores necessary information.
  $form['#searcher'] = $searcher;
  $form['#type']     = facetapi_adapter_load($searcher)->getType();
  $form['#realms']   = facetapi_realms_get();
  $form['#facets']   = facetapi_facets_get($searcher, $form['#type']);

  // Common url() function options that will redirect form submissions in linked
  // pages back to this form.
  $url_options = array('query' => array('destination' => $_GET['q']));

  // Adds description, submit handler if there are facets.
  if (!empty($form['#facets'])) {
    $form['description']['#value'] = t(
      'This page provides a drag-and-drop interface to enable certian facets in each realm and control the order they are shown on the page.  <em>Realms</em> are groups of facets that are displayed in a similar fashion on the search page. Since rendering is handled by the relam, a single facet may be displayed in different ways, for exmaple a form element or a list of clickable links. Like the core search, users need the <em>use advanced search</em> <a href="@permissions-page">permissions</a> to be able to use the facets.',
      array('@permissions-page' => url('admin/user/permissions', $url_options))
    );
  }

  // Iterates over realms, builds form for each realm.
  foreach ($form['#realms'] as $realm_name => $realm) {
    if (!empty($form['#facets'])) {

      // Sorts facets, builds options for checkbox.
      $options = array();
      $sorted_facets = $form['#facets'];
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
        '#value' => t('No facets are available for this searcher.'),
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
 *
 * @return
 *   A themed form element.
 */
function theme_facetapi_admin_settings_form($form) {

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
        $facet = $form['#facets'][$facet_name];

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
              $form['#searcher'], $realm_name, $facet_name
            )),
          ),
        );

        // Adds weight column if realm is "sortable".
        if ($form['#realms'][$realm_name]['sortable']) {
          $weight_var = sprintf('facetapi:facet_weight:%s:%s:%s', $form['#searcher'], $realm_name, $facet_name);
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
    $table_id = sprintf('facetapi-%s-%s-table', $form['#searcher'], $realm_name);

    // Adds tabledrag if sortable, otherwise removes weight header.
    $row_headers = $headers;
    if ($form['#realms'][$realm_name]['sortable']) {
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
 *
 * @return
 *   A FAPI array containing the form.
 */
function facetapi_facet_settings_form(&$form_state, FacetapiAdapter $adapter, array $realm, array $facet) {
  $form = array();

  // Captures Identifier for facet specific settings.
  $identifier = "{$adapter->getSearcher()}:{$realm['name']}:{$facet['name']}";

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
    '#description' => t('Display no more than this number of facet items.')
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

  // Variables passed to facetapi_widgets_get().
  $varialbes = array(
    'adapter' => $adapter,
    'realm'   => $realm,
    'facet'   => $facet,
  );

  // Gets widgets, sorts.
  $widgets = facetapi_widgets_get($varialbes);
  uasort($widgets, 'facetapi_sort_weight');

  // Builds select options for widgets.
  $options = array();
  foreach ($widgets as $widget_name => $definition) {
    $options[$widget_name] = $definition['title'];
  }

  $widget_var = "facetapi:widget:$identifier";
  $form['realm'][$widget_var] = array(
    '#type' => 'select',
    '#title' => t('Display widget'),
    '#default_value' => variable_get($widget_var, 'facetapi_links'),
    '#options' => $options,
  );

  $slimit_var = "facetapi:soft_limit:$identifier";
  $form['realm'][$slimit_var] = array(
    '#type' => 'select',
    '#title' => t('Soft limit'),
    '#default_value' => variable_get($slimit_var, 10),
    '#options' => array(0 => t('No limit')) + drupal_map_assoc(array(50, 40, 30, 20, 15, 10, 5, 3)),
    '#description' => t('Limits the number of displayed facets via JavaScript.'),
  );


  // Gets all sort definitions, sorts active for this facet.
  $sorts        = facetapi_sorts_get();
  $active_sorts = facetapi_facet_sorts_get($adapter, $realm, $facet);

  // Initializes the sorting table.
  $form['realm']['sort'] = array(
    '#type' => 'fieldset',
    '#title' => t('Sorting options'),
    '#description' => t('Determine how facet items are sorted. Selecting the checkbox activates the associated sort, and the sorts are applied from the top down. Click and drag the rows to change the order the sorts are applied.'),
    'table' => array('#theme' => 'facetapi_facet_settings_form'),
  );

  // Builds checkbox options for sorts.
  $options = array();
  foreach ($sorts as $sort_name => $sort_info) {
    $options[$sort_name] = $sort_info['title'];
  }

  $sort_var = "facetapi:sort:$identifier";
  $form['realm']['sort']['table']['sort'][$sort_var] = array(
    '#type' => 'checkboxes',
    '#options' => $options,
    '#default_value' => variable_get($sort_var, drupal_map_assoc(array_keys($active_sorts))),
  );

  foreach ($options as $sort_name => $sort_title) {
    $default = isset($active_sorts[$sort_name]) ? $active_sorts[$sort_name]['order'] : SORT_DESC;
    $order_var = "facetapi:sort_order:$identifier:$sort_name";
    $form['realm']['sort']['table']['sort_order'][$order_var] = array(
      '#type' => 'select',
      '#title' => '',
      '#options' => array(
        SORT_DESC => t('Descending'),
        SORT_ASC => t('Ascending'),
      ),
      '#default_value' => variable_get($order_var, $default),
    );
  }

  foreach ($options as $sort_name => $sort_title) {
    if (isset($active_sorts[$sort_name])) {
      $default = $active_sorts[$sort_name]['weight'];
    }
    else {
      $default = $sorts[$sort_name]['weight'];
    }
    $weight_var = "facetapi:sort_weight:$identifier:$sort_name";
    $form['realm']['sort']['table']['sort_weight'][$weight_var] = array(
      '#type' => 'weight',
      '#title' => '',
      '#delta' => 50,
      '#default_value' => variable_get($weight_var, $default),
      '#attributes' => array('class' => 'facetapi-sort-weight'),
    );
  }

  return system_settings_form($form);
}

/**
 * Themes the facet sort form into a draggable table.
 *
 * @param $form
 *   A FAPI array containing a fieldset.
 * @return
 *   A themed form element.
 */
function theme_facetapi_facet_settings_form($form) {
  $output = '';

  // Gets all sort definitions.
  $sorts = facetapi_sorts_get();

  // Initializes table headers.
  $headers = array();
  $headers[] = array('data' => '', 'class' => 'checkbox');
  $headers[] = t('Type');
  $headers[] = t('Order');
  $headers[] = t('Description');
  $headers[] = t('Weight');

  // Builds table rows.
  $rows = array();
  foreach (element_children($form['sort']) as $sort_var) {

    // Pulls information from variable, iterates over available sorts.
    list($module, $variable, $searcher, $realm_name, $facet_name) = explode(':', $sort_var);
    foreach (element_children($form['sort'][$sort_var]) as $option) {

      // Captures longer variable names for code readability.
      $weight_var = "facetapi:sort_weight:$searcher:$realm_name:$facet_name:$option";
      $order_var  = "facetapi:sort_order:$searcher:$realm_name:$facet_name:$option";

      // Extracts the titile form the checkbox, hides title.
      $title = $form['sort'][$sort_var][$option]['#title'];
      $form['sort'][$sort_var][$option]['#title'] = '';

      // Builds the table rows.
      $rows[] = array(
        'weight' => $form['sort_weight'][$weight_var]['#default_value'],
        'class' => 'draggable',
        'data' => array(
          array(
            'data' => drupal_render($form['sort'][$sort_var][$option]),
            'class' => 'checkbox',
          ),
          array(
            'data' => check_plain($title),
          ),
          array(
            'data' => drupal_render($form['sort_order'][$order_var]),
          ),
          array(
            'data' => filter_xss($sorts[$option]['description']),
          ),
          array(
            'data' => drupal_render($form['sort_weight'][$weight_var]),
            'class' => 'dropbox',
          ),
        ),
      );
    }
  }

  // Sorts rows by weight.
  // @todo Replace with drupal_sort_weight() in D7.
  usort($rows, 'facetapi_sort_weight');

  // Builds the sort options table.
  $table_id = "facetapi-sort-$searcher-$realm_name-$facet_name";
  drupal_add_tabledrag($table_id, 'order', 'sibling', 'facetapi-sort-weight');
  $form['#value'] = theme('table', $headers, $rows, array('id' => $table_id));

  // Renders and returns table.
  $output .= drupal_render($form);
  return $output;
}

