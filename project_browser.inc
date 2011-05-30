<?php 
// $Id$

/**
 * @file
 * 
 */

/**
 * Implements hook_form().
 * TODO: Form to add/remove module from install list.
 * 
 * @param array $release
 *   Associative array like so:
 *     array(
 *       'title' => 'Views',
 *       'short_name' => 'views',
 *       'version_string' => '7.x-3.3',
 *       'download_url' => 'http://www.drupal.org/views-7.x-3.3.tar.gz',
 *     )
 */
function project_browser_install_form($form, &$form_state, $release) {
  $form['install'] = array(
    '#type' => 'checkbox',
    '#prefix' => '<div class="install-item-prefix">'.t('Add to list').'</div>',
    '#default_value' => 0,
    '#attributes' => array('onchange' => 'this.form.submit();'),
  );
  $form['release'] = array(
    '#type' => 'value',
    '#value' => $release,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Install',
    '#submit' => array('project_browser_install_form_submit'),
    '#attributes' => array('style' => 'display: none;')
  );

  return $form;
}

/**
 * Implements hook_form_submit().
 */
function project_browser_install_form_submit($form, &$form_state) {
  // Add the project to the install list
  $_SESSION['project_browser_install_list'][] = $form_state['values']['release'];
}

/**
 * Form to get all available categories. 
 */
function project_browser_category_filter_form($form, &$form_state) {
  $categories = project_browser_get_categories();
  $form['category'] = array(
    '#type' => 'fieldset',
    '#title' => t('Category'),
    '#tree' => TRUE,
    '#collapsible' => FALSE,
  );
  
  foreach ($categories as $category) {
    $form_category = str_replace(' ', '_', $category);
    $form['category'][$form_category] = array(
      '#type' => 'checkbox',
      '#title' => $category,
      '#default_value' => isset($_SESSION['project_browser_category'][$category]) ? $_SESSION['project_browser_category'][$category] : 0,
      '#attributes' => array('onchange' => 'this.form.submit()'),
    );
  }
  
  $form['category']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Filter',
    '#attributes' => array('style' => 'display: none;')
  );
  
  return $form;
}

/**
 * Submit handler for setting filter
 */
function project_browser_category_filter_form_submit($form, &$form_state) {
  $_SESSION['project_browser_category'][$session_key] = $form_state['values']['category'];
}

/**
 * Form to let user search for text
 */
function project_browser_text_filter_form($form, &$form_state) {
  $categories = project_browser_get_categories();
  $form['search'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search'),
    '#collapsible' => FALSE,
  );
  $form['search']['search_text'] = array(
    '#type' => 'textbox',
    '#title' => t('Search String'),
    '#default_value' => isset($_SESSION['project_browser_text_filter']) ? $_SESSION['project_browser_text_filter'] : '',
  );
  $form['search']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
  );
  
  return $form;
}

/**
 * Submit handler for setting filter
 */
function project_browser_text_filter_form_submit($form, &$form_state) {
  $_SESSION['project_browser_category'][$session_key] = $value;
}

/**
 * Helper function to get install list.
 * 
 * Install list is all projects that are selected to install. 
 */
function project_browser_get_install_list() {
  return $_SESSION['project_browser_install_list'];
}

/**
 * Helper function to get available sortings.
 */
function project_browser_sort_list() {
  // TODO - Finish this
}

/**
 * Helper function to get all available categories.
 * 
 * @return array
 *    Array containing all available categories
 */
function project_browser_get_categories() {
  $categories = project_browser_fetch_categories($type);
  
  ksort($categories);
  return $categories;
}

/**
 * Helper function to parse the servers string into an array
 */
function _project_browser_string_to_array($string) {
  $values = array();

  $list = explode("\n", $string);
  $list = array_map('trim', $list);
  $list = array_filter($list, 'strlen');
  
  foreach ($list as $position => $text) {
    $value = $key = FALSE;
    
    $matches = array();
    if (preg_match('/(.*)\|(.*)/', $text, $matches)) {
      $key = $matches[1];
      $value = $matches[2];
      $values[$key] = $value;
    }
  }
  
  return $values;
}

/**
 * Helper function to check if a project is enabled
 * 
 * @param $type
 *   The type of project. Could be 'theme' or 'module'
 * @param $short_name
 *   The short name of the project
 * @return bool
 *   TRUE if the project is enabled, FALSE otherwise
 */
function _project_browser_is_project_enabled($type, $short_name) {
  switch ($type) {
    case 'module':
      return module_exists($short_name);
      break;
    case 'theme':
      $themes = list_themes();
      return isset($themes[$short_name]);
      break;
  }
  return FALSE;
}

// ======================================
// Server Related Functions:
// ======================================

/**
 * Fetches results from the servers based on the parameters passed in
 * 
 * $filters should be an associative array with the following keys:
 * array(
 *   'drupal_version' => '6.x', // The Major Version of Drupal that is running on the Client
 *   'text' => 'Views', // The text that was entered as the search query, or 'all'
 *   'type' => 'module', // The type of project being searched
 * )
 * 
 * The project_browser_fetch_results($filters) call returns an array like this:
 * 
 * array(
 *   'views' => array( // The key is the project (unique) shortname
 *     'type' => 'module', // The type of project this is. Can be 'module' or 'theme'
 *     'title' => 'Views',
 *     'short_name' => 'views',
 *     'author' => 'merlinofchaos',
 *     'description' => "Long project description ...",
 *     'image' => 'http://www.example.com/image.jpg', // Absolute url to the image, if any
 *     'usage' => '542312', // How many Downloads the module has had
 *     'project_url' => 'http://www.drupal.org/projects/views', // Absolute url to the project page, if any
 *     'update_url' => 'http://updates.drupal.org/release-history/views/7.x', // The absolute url of the update checker, formatted like how Drupal.org Update Status does it
 *     'last_updated' => '12342523', // UNIX Timestamp of when the project was last updated
 *     'rating' => '9.6', // A rating on a scale of 1 to 10 of the project, if available
 *     'dependencies' => array( // An array of the dependencies of this module
 *         'ctools',
 *       ),
 *   ),
 *   'short_name_2 => array( ... ),
 * );
 * 
 * @param $filters
 *   An associative array of queries to use to filter results
 * @return array
 *   Returns an array of results
 */
function project_browser_fetch_results($filters) {
  $servers = project_browser_get_servers();
  
  $results = array();
  
  foreach ($servers as $url => $name) {
    $results_raw = xmlrpc($url, array(
      'project_browser_server.fetch_results' => array($filters),
    ));
    
    // Check for errors
    $error = xmlrpc_error();
    if ($error->is_error) {
      drupal_set_message(t("Encountered an error when trying to fetch results from @name. Error @code : @message",
        array('@name' => $name, '@code' => $error->code, '@message' => $error->message)));
      continue;
    }
    
    if (is_array($results_raw) AND !empty($results_raw)) {
      // Merge the results
      $results = array_merge($results, $results_raw);
    }
  }
  
  return $results;
}

/**
 * Fetches categories from the servers based on the type of project
 * 
 * @param $type
 *   The type of project we are getting categories for
 * @return array
 *   Returns an array of the categories
 */
function project_browser_fetch_categories($type) {
  $servers = project_browser_get_servers();
  
  $categories = array();
  
  foreach ($servers as $url => $name) {
    $categories_raw = xmlrpc($url, array(
      'project_browser_server.fetch_categories' => array($type),
    ));
    
    // Check for errors
    $error = xmlrpc_error();
    if ($error->is_error) {
      drupal_set_message(t("Encountered an error when trying to fetch categories from @name. Error @code : @message",
        array('@name' => $name, '@code' => $error->code, '@message' => $error->message)));
      continue;
    }
    
    if (is_array($categories_raw) AND !empty($categories_raw)) {
      // Merge the results
      $categories = array_merge($categories, $categories_raw);
    }
  }
  
  return $categories;
}

/**
 * Gets the servers to use for fetching results
 * 
 * @return array
 *   Returns an associative array of servers, populated from the project_browser_servers variable, 
 *   in 'url => name' format
 */
function project_browser_get_servers() {
  $servers = array(
    'http://www.drupal.org/xmlrpc.php' => 'Drupal.org',
  );
  if ($servers_raw = variable_get('project_browser_servers', '')) {
    // Process the variable and add the servers to the list
    $custom_servers = _project_browser_string_to_array($servers_raw);
    $servers = array_merge($servers, $custom_servers);
  }
  
  return $servers;
}