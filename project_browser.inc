<?php 
// $Id$

/**
 * @file
 * Various functions that are required by project_browser
 */

 /**
  * Returns the themed install list form
  */
function project_browser_get_install_list() {
  $queued_projects = project_browser_get_queued_projects();
  
  // Show a list of the queued projects
  return theme('project_browser_install_queue', array('projects' => $queued_projects));
}

/**
 * Show an install button for the Install Queue block
 */
function project_browser_install_button_form($form, &$form_state) {
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Install',
  );
  $form['#action'] = url('admin/modules/project_browser/install/select_versions');
  
  return $form;
}

/**
 * Shows a checkbox that will add a project to the install list
 * 
 * @param array $project
 *   A project array object
 * @param bool $queue
 *   Whether this form is used for a queued project. Defaults to false
 */
function project_browser_install_form($form, &$form_state, $project) {
  $queued_projects = project_browser_get_queued_projects();
  $form['install'] = array(
    '#type' => 'checkbox',
    '#title' => t('Install'),
    '#default_value' => isset($queued_projects[$project['name']]) ? 1 : 0,
    '#attributes' => array('class' => array('install-checkbox-' . $project['name'])),
    '#ajax' => array(
      'callback' => 'project_browser_install_form_ajax',
      'wrapper' => 'project-browser-install-queue',
      'effect' => 'fade',
      'speed' => 1000,
      'progress' => array(
        'type' => 'throbber',
        'message' => '',
      ),
    ),
  );
  $form['project'] = array(
    '#type' => 'value',
    '#value' => $project,
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
  $project = $form_state['values']['project'];
  if ($form_state['values']['install']) {
    // Add the project to the install list
    project_browser_install_queue_add($project);
  }
  else {
    // Remove the project from the install list
    project_browser_install_queue_remove($project);
  }
}

/**
 * AJAX callback for the install form
 */
function project_browser_install_form_ajax($form, &$form_state) {
  project_browser_install_form_submit($form, &$form_state);
  return project_browser_get_install_list();
}

/**
 * AJAX callback for the install queue form checkboxes
 */
function project_browser_install_queue_ajax($form, &$form_state) {
  $commands = array();
  
  foreach ($form_state['values']['projects'] as $item) {
    $project = $item['project'];
    if (!$item['c_box']) {
      // If the checkbox is unchecked, we remove it from the install queue
      project_browser_install_queue_remove($project);
      // We also want to uncheck the checkbox on the project listing if it is present on the page
      $commands[] = ajax_command_invoke('.install-checkbox-' . $project['name'], 'attr', array('checked', FALSE));
    }
  }
  
  // Replace the queue with the new queue html
  $commands[] = ajax_command_replace(NULL, project_browser_get_install_list());
  
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Shows a checkbox that will add a project to the install list
 * 
 * @param array $project
 *   A project array object
 * @param bool $queue
 *   Whether this form is used for a queued project. Defaults to false
 */
function project_browser_install_queue_form($form, &$form_state) {
  $queued_projects = project_browser_get_queued_projects();
  
  $form['#tree'] = TRUE;
  
  foreach ($queued_projects as $project) {
    $form['projects'][$project['name']]['c_box'] = array(
      '#type' => 'checkbox',
      '#title' => $project['title'],
      '#default_value' => 1,
      '#ajax' => array(
        'callback' => 'project_browser_install_queue_ajax',
        'wrapper' => 'project-browser-install-queue',
        'effect' => 'fade',
        'speed' => 1000,
        'progress' => array(
          'type' => 'throbber',
          'message' => '',
        ),
      ),
    );
      
    $form['projects'][$project['name']]['project'] = array(
      '#type' => 'value',
      '#value' => $project,
    );
  }

  return $form;
}

/**
 * Implements hook_form_submit().
 */
function project_browser_install_queue_form_submit($form, &$form_state) {
  foreach ($form_state['values']['projects'] as $item) {
    $project = $item['project'];
    if (!$item['c_box']) {
      project_browser_install_queue_remove($project);
    }
  }
}

/**
 * Form builder for the filters
 */
function project_browser_filters_form($form, &$form_state, $type) {
  $form['search_text'] = array(
    '#type' => 'textfield',
    '#size' => '30',
    '#title' => t('Search String'),
    '#default_value' => isset($_SESSION['project_browser_text_filter_' . $type]) ? $_SESSION['project_browser_text_filter_' . $type] : '',
  );
  
  // Add the categories filter if there are categories
  if ($categories = project_browser_get_categories($type, TRUE)) {
    $form['categories'] = array(
      '#type' => 'checkboxes',
      '#title' => t('Categories'),
      '#options' => $categories,
      '#default_value' => isset($_SESSION['project_browser_category_filter_' . $type]) ? $_SESSION['project_browser_category_filter_' . $type] : array(),
    );
  }
  
  $form['project_type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
  );
  
  return $form;
}

/**
 * Submit handler for filters form
 */
function project_browser_filters_form_submit($form, &$form_state) {
  $type = $form_state['values']['project_type'];
  if (isset($form_state['values']['categories'])) {
    $_SESSION['project_browser_category_filter_' . $type] = $form_state['values']['categories'];
  }
  else {
    $_SESSION['project_browser_category_filter_' . $type] = array();
  }
  $_SESSION['project_browser_text_filter_' . $type] = $form_state['values']['search_text'];
}

/**
 * Helper function to get all available categories.
 * 
 * @return array
 *    Array containing all available categories or FALSE if no categories
 */
function project_browser_get_categories($type, $condensed = FALSE) {
  $categories = array();
  
  $categories_raw = project_browser_fetch_categories($type);

  if (is_array($categories_raw) AND !empty($categories_raw)) {
    foreach ($categories_raw as $url => $modules) {
      foreach ($modules as $module => $cats) {
        foreach ($cats as $key => $value) {
          // Create a new key so that there are no duplicate categories from different sites
          $new_key = preg_replace('/[^a-z0-9_]+/', '_', strtolower($value));
          $categories[$new_key] = $value;
        }
      }
    }
  }
  
  if (is_array($categories) AND !empty($categories)) {
    ksort($categories);
    
    // Remove categories from the list if they are not on the condensed list
    if ($condensed) {
      $allowed = array_flip(variable_get('project_browser_condensed_categories_' . $type, 
        array('administration', 'media', 'content', 'developer', 'user-management')));
      $categories = array_intersect_key($categories, $allowed);
    }
    
    return $categories;
  }
  return FALSE;
}

/**
 * Helper function that prepares the categories for sending to the servers as filters
 * 
 * @param array $raw_cats
 *   The categories from $form_state['values']
 * @param string $type
 *   The type of project to prepare the categories for, eg 'module' or 'theme'
 */
function project_browser_prepare_categories($raw_cats, $type) {
  $categories = project_browser_fetch_categories($type);
  
  // Set the value of the categories to true if it is selected
  foreach ($categories as $url => $modules) {
    foreach ($modules as $module => $cats) {
      foreach ($cats as $key => $value) {
        $new_key = preg_replace('/[^a-z0-9_]+/', '_', strtolower($value));
        if (isset($raw_cats[$new_key]) AND $raw_cats[$new_key]) {
          $categories[$url][$module][$key] = TRUE;
        }
        else {
          unset($categories[$url][$module][$key]);
        }
      }
      
      // Unset the parent if there are no children
      if (empty($categories[$url][$module])) {
        unset($categories[$url][$module]);
      }
    }
    
    // Unset the parent if there are no children
    if (empty($categories[$url])) {
      unset($categories[$url]);
    }
  }
  
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
 * @param $name
 *   The short name of the project
 * @return bool
 *   TRUE if the project is enabled, FALSE otherwise
 */
function _project_browser_is_project_enabled($type, $name) {
  switch ($type) {
    case 'module':
      return module_exists($name);
      break;
    case 'theme':
      $themes = list_themes();
      return isset($themes[$name]);
      break;
  }
  return FALSE;
}

/**
 * Helper function to get the currently listed projects
 */
function project_browser_get_listed_projects() {
  if (isset($_SESSION['project_browser_listed_projects'])) {
    return $_SESSION['project_browser_listed_projects'];
  }
  
  return array();
}

/**
 * Helper function to get the currently queued projects
 */
function project_browser_get_queued_projects($type = NULL) {
  $projects = array();
  
  if (isset($_SESSION['project_browser_install_list'])) {
    foreach ($_SESSION['project_browser_install_list'] as $project) {
      if (is_array($project) AND !empty($project)) {
        if (isset($type) AND $type != $project['type']) {
          continue;
        }
        else {
          $projects[$project['name']] = $project;
        }
      }
    }
  }
  
  return $projects;
}

/**
 * Helper function to get the newly installed projects
 */
function project_browser_get_installed_projects() {
  $projects = array();
  
  if (isset($_SESSION['project_browser_installed_projects'])) {
    foreach ($_SESSION['project_browser_installed_projects'] as $project) {
      if (is_array($project) AND !empty($project)) {
        $projects[$project['name']] = $project;
      }
    }
  }
  
  return $projects;
}

/**
 * Adds a project to the install queue
 */
function project_browser_install_queue_add($project) {
  $_SESSION['project_browser_install_list'][$project['name']] = $project;
}

/**
 * Removes a project from the install queue
 */
function project_browser_install_queue_remove($project) {
  unset($_SESSION['project_browser_install_list'][$project['name']]);
}

/**
 * Helper function to get the currently queued releases
 */
function project_browser_get_queued_releases() {
  $releases = array();
  
  if (isset($_SESSION['project_browser_install_releases_list'])) {
    foreach ($_SESSION['project_browser_install_releases_list'] as $release_name => $project) {
      if (is_array($project) AND !empty($project)) {
        $releases[$release_name] = $project;
      }
    }
  }
  
  return $releases;
}

// ======================================
// Server Related Functions:
// ======================================

/**
 * Fetches results from the servers based on the parameters passed in
 * 
 * $filters should be an associative array with the following keys:
 * array(
 *   'drupal_version' => '7', // The Major Version of Drupal that is running on the Client
 *   'text' => 'views', // The text that was entered as the search query, or '' if none
 *   'categories' => array() // The categories that were selected, if any
 *   'type' => 'module', // The type of project being searched
 *   'page' => 3, // The zero-based page number
 *   'requested' => 12, // How many results are requested per page
 * )
 * 
 * The project_browser_fetch_results($filters) call returns an array like this:
 * 
 * array(
 *   'total' = 5, // The total number of results found for the filters
 *   'projects' => array( // An array of projects returned for this page request
 *     'views' => array( // A project array keyed by the machine name
 *       'type' => 'module', // The type of project this is. Can be 'module' or 'theme'
 *       'title' => 'Views', // The title of the project
 *       'name' => 'views', // The machine name of the project
 *       'author' => 'merlinofchaos', // The author's name
 *       'description' => "Long project description ...",
 *       'image' => 'http://www.example.com/image.jpg', // Absolute url to the image, if any
 *       'usage' => '542312', // How many Downloads the module has had
 *       'project url' => 'http://www.drupal.org/projects/views', // Absolute url to the project page, if any
 *       'project status url' => 'http://updates.drupal.org/release-history/views/7.x', // The absolute url of the update checker, formatted like how Drupal.org Update Status does it
 *       'last updated' => '12342523', // UNIX Timestamp of when the project was last updated
 *       'maintenance status' => 'Actively maintained', // Maintenance status
 *       'development status' => 'Under active development', // Development status
 *       'rating' => '9.6', // A rating on a scale of 1 to 10 of the project, if available
 *       'dependencies' => array( // An array of the dependencies of this module
 *         'ctools',
 *       ),
 *     ),
 *   'name_2 => array( ... ),
 *   ),
 * );
 * 
 * @param $filters
 *   An associative array of queries to use to filter results
 * @return array
 *   Returns an array of results
 */
function project_browser_fetch_results($filters) {
  $servers = project_browser_get_servers();
  
  $results = array(
    'projects' => array(),
    'total' => 0,
  );
  
  foreach ($servers as $url => $name) {
    $local_filters = $filters;
    
    // FIXME - Handle when one server runs out of results before another...
    // Only request a certain amount of results from each server to fill one page
    $local_filters['requested'] = floor($filters['requested'] / count($servers));
    
    // Send only the relevant categories to the server
    if (isset($filters['categories'])) {
      if (!isset($filters['categories'][$url])) {
        // Don't call a server for results if categories are being used, and none of them belong to the server
        continue;
      }
      $local_filters['categories'] = $filters['categories'][$url];
    }

    $results_raw = xmlrpc($url, array(
      'project_browser_server.fetch_results' => array($local_filters),
    ));
    
    // Check for errors
    if ($error = xmlrpc_error() AND $error->is_error) {
      drupal_set_message(t("Encountered an error when trying to fetch results from @name. Error @code : @message",
        array('@name' => $name, '@code' => $error->code, '@message' => $error->message)));
      continue;
    }
    
    if (isset($results_raw['total'])) {
      $results['total'] += $results_raw['total'];
    }
    
    if (isset($results_raw['projects']) AND !empty($results_raw['projects'])) {
      // Merge the results
      $results['projects'] = array_merge($results['projects'], $results_raw['projects']);
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
  // Attempt to retrieve the categories from the cache
  if ($cache = cache_get('project_browser_categories_'. $type, 'cache')) {
    $categories = $cache->data;
  }
  else {
    // Fetch the categories from the servers
    $servers = project_browser_get_servers();
    
    $categories = array();
    
    foreach ($servers as $url => $name) {
      $categories_raw = xmlrpc($url, array(
        'project_browser_server.fetch_categories' => array($type),
      ));
      
      // Check for errors
      if ($error = xmlrpc_error() AND $error->is_error) {
        drupal_set_message(t("Encountered an error when trying to fetch categories from @name. Error @code : @message",
          array('@name' => $name, '@code' => $error->code, '@message' => $error->message)));
        continue;
      }
      
      if (is_array($categories_raw) AND !empty($categories_raw)) {
        $categories[$url] = $categories_raw;
      }
    }
    
    // Cache this for 24 hours
    // FIXME - Uncomment this when we get to production
    // cache_set('project_browser_categories_'. $type, $categories, 'cache', strtotime("+24 hours"));
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
  
  // REMOVEME once drupal.org has the functionality built in
  $servers = array();
  
  if ($servers_raw = variable_get('project_browser_servers', '')) {
    // Process the variable and add the servers to the list
    $custom_servers = _project_browser_string_to_array($servers_raw);
    $servers = array_merge($servers, $custom_servers);
  }
  
  return $servers;
}

/**
 * Uses the project status url to get the available releases for a project
 * 
 * @param $project
 *   The project to get the releases for
 * @return array
 *   An array of releases for this project
 */
function project_browser_get_project_release_data($project) {
  $releases = array();
  // FIXME - Change the 'type' key to 'project_type'
  $project['project_type'] = $project['type'];
  // Build the releases cache for this project
  module_load_include('inc', 'update', 'update.fetch');
  if (_update_process_fetch_task($project)) {
    $data = _update_cache_get('available_releases::' . $project['name']);
    if (isset($data->data) AND isset($data->data['releases']) AND is_array($data->data['releases'])) {
      return $data->data;
    }
  }
  
  return FALSE;
}

// ======================================
// Batch Operations:
// ======================================

/**
 * Helper function to download a project. This code is mostly copied and pasted from
 * modules/update/update.manager.inc
 * 
 * There were no suitable functions that could be used besides 
 * drupal_form_submit('update_manager_install_form', $form_state, $project['type']);
 * and it wouldn't work because this is being run from a Batch function
 * 
 * @todo - Ideally, this should be in the update module as a standalone function, 
 * to reduce coupling and duplication
 */
function project_browser_download_project($url) {
  module_load_include('inc', 'update', 'update.manager');
  // Download the file
  $local_cache = update_manager_file_get($url);
  if (!$local_cache) {
    return array(
      'success' => FALSE,
      'message' => t('Unable to retrieve Drupal project from %url.', array('%url' => $url)),
      );
  }
  
  // Try to extract it
  $directory = _update_manager_extract_directory();
  try {
    $archive = update_manager_archive_extract($local_cache, $directory);
  }
  catch (Exception $e) {
    return array(
      'success' => FALSE,
      'message' => $e->getMessage(),
      );
  }
  $files = $archive->listContents();
  if (!$files) {
    return array(
      'success' => FALSE,
      'message' => t('Provided archive contains no files.'),
      );
  }
  
  $project = strtok($files[0], '/\\');

  $archive_errors = update_manager_archive_verify($project, $local_cache, $directory);
  if (!empty($archive_errors)) {
    if (!empty($archive_errors)) {
      foreach ($archive_errors as $error) {
        drupal_set_message($error, 'error');
      }
    }
    return array(
      'success' => FALSE,
      'message' => array_shift($archive_errors),
      );
  }

  // Make sure the Updater registry is loaded.
  drupal_get_updaters();

  $project_location = $directory . '/' . $project;
  try {
    $updater = Updater::factory($project_location);
  }
  catch (Exception $e) {
    return array(
      'success' => FALSE,
      'message' => $e->getMessage(),
      );
  }

  try {
    $project_title = Updater::getProjectTitle($project_location);
  }
  catch (Exception $e) {
    return array(
      'success' => FALSE,
      'message' => $e->getMessage(),
      );
  }

  if ($updater->isInstalled()) {
    return array(
      'success' => FALSE,
      'message' => t('%project is already installed.', array('%project' => $project_title)),
      );
  }

  $project_real_location = drupal_realpath($project_location);
  $updater_name = get_class($updater);
  
  if (fileowner($project_real_location) == fileowner(conf_path())) {
    module_load_include('inc', 'update', 'update.authorize');
    $filetransfer = new FileTransferLocal(DRUPAL_ROOT);
    
    // Initialize some variables in the Batch API $context array.
    $updater = new $updater_name($project_real_location);
  
    try {
      if ($updater->isInstalled()) {
        // This is an update.
        $tasks = $updater->update($filetransfer);
      }
      else {
        $tasks = $updater->install($filetransfer);
      }
    }
    catch (UpdaterException $e) {
      return array(
        'success' => FALSE,
        'message' => t('Error installing / updating. Error: @error', array('@error' => $e->getMessage())),
        );
    }
  }
  else {
    return array(
      'success' => FALSE,
      'message' => t('Permissions are not set up properly.'),
      );
  }
  
  return array(
    'success' => TRUE,
    );
}

/**
 * Batch function to install a single release of a project
 */
function _project_browser_batch_install_release($release_name, $project, &$context) {
  module_load_include('inc', 'project_browser', 'project_browser.pages');
  $release = project_browser_get_release($release_name, $project);
  
  $result = project_browser_download_project($release['download_link']);
  
  if ($result['success']) {
    $context['results']['successes'][] = t('Successfully installed %project.', array('%project' => $project['title']));
    $context['message'] = t('Installed %project...', array('%project' => $project['title']));
    
    // Add this to the session variable and remove it from the install_queue variable
    $_SESSION['project_browser_installed_projects'][$project['name']] = $project;
    unset($_SESSION['project_browser_install_list'][$project['name']]);
  }
  else {
    watchdog('project_browser', 'There was an error while installing %project. 
      !message',
      array('%project' => $project['title'], '!message' => $result['message']), WATCHDOG_ERROR);
    $context['results']['failures'][] = t('Error installing %project. Errors have been logged.', 
      array('%project' => $project['title']));
    $context['message'] = t('Error installing %project. !message', 
      array('%project' => $project['title'], '!message' => $result['message']));
  }
}

/**
 * Show a message and finish up the batch
 */
function _project_browser_batch_install_releases_finished($success, $results, $operations) {
  unset($_SESSION['project_browser_install_releases_list']);
  if ($success) {
    if (!empty($results)) {
      if (!empty($results['successes'])) {
        drupal_set_message(format_plural(count($results['successes']), 'One project installed.', '@count projects installed.'));
      }
      if (!empty($results['failures'])) {
        drupal_set_message(format_plural(count($results['failures']), 'Failed to install one project.', 'Failed to install @count projects.'), 'error');
      }
    }
  }
  else {
    drupal_set_message(t('Error installing projects.'), 'error');
    drupal_goto('admin/modules/project_browser/install/select_versions');
  }

  drupal_goto('admin/modules/project_browser/install/enable');
}
