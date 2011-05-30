<?php 
// $Id$

/**
 * @file
 * 
 */

/**
 * Page to display the projects.
 * 
 * @param string $type
 *   Type of project to view, ie 'module' or 'theme'
 * @return 
 *   Themed list of the projects
 */
function project_browser_page($type) {
  drupal_add_css(drupal_get_path('module', 'project_browser') . '/project_browser.css', array('preprocess' => FALSE));
  
  // Build the filters
  $filters = array(
    'drupal_version' => DRUPAL_CORE_COMPATIBILITY,
    'type' => $type,
  );
  
  // Add filters
  if (isset($_SESSION['project_browser_category_filter'])) {
    $filters['categories'] = $_SESSION['project_browser_category_filter'];
  }
  if (isset($_SESSION['project_browser_text_filter'])) {
    $filters['text'] = $_SESSION['project_browser_text_filter'];
  }
  
  // Get the projects to display here based on the filters
  $projects = project_browser_fetch_results($filters);
  
  $list = array();
  foreach ($projects as $project) {
    $list[] = $project;
  }
  
  return theme('project_browser_list', array('projects_list' => $list));
}

// ======================================
// Installation Page:
// ======================================

/**
 * Page to install projects
 * 
 * @param string $op
 *    Operation to preform.
 */
function project_browser_installation_page($op) {
  project_browser_installation_task_list($op);
  switch ($op) {
    case 'select_versions':
      project_browser_installation_select_versions_page();
      break;
    case 'backup':
      project_browser_installation_backup_page();
      break;
    case 'install':
      project_browser_installation_install_page();
      break;
    case 'finished':
      project_browser_installation_enable_page();
      break;
  }
}

/**
 * Task page for the Select versions installation task
 * 
 * Shows a form where the user can select which versions to install for each 
 * project
 */
function project_browser_installation_select_versions_page() {
  // TODO - Show a form that lets the user select which version of the projects to install
}

/**
 * Task page for the Backup site installation task
 * 
 * Creates a backup of the database and lets the user download the file
 */
function project_browser_installation_backup_page() {
  // TODO - Implement this
}

/**
 * Task page for the Install Projects installation task
 * 
 * Starts a batch install process of the projects
 */
function project_browser_installation_install_page() {
  include_once (drupal_get_path('module', 'update') . '/update.manager.inc' );
  
  $queued_releases = project_browser_get_install_list();
  
  $operations = array();
  foreach ($queued_releases as $release) {
    $operations[] = array('_project_browser_batch_install_release', array($release));
  }
  $batch = array(
    'operations' => $operations,
    'finished' => '_project_browser_batch_install_releases_finished',
    'title' => t('Installing projects'),
    'init_message' => t('Beginning the install process.'),
    'progress_message' => t('Installed @current out of @total.'),
    'error_message' => t('Installation has encountered an error.'),
  );
  batch_set($batch);
}

/**
 * Task page for the Enable projects installation task
 * 
 * Show a form which lets the user enable the newly installed projects
 */
function project_browser_installation_enable_page() {
  // TODO - Implement this
}

/**
 * Add a task list to the sidebar area when installing projects
 * 
 * This will need to be called from every page of the install process
 * 
 * @param $active
 *   (Optional) Set the active task by key
 */
function project_browser_installation_task_list($active = NULL) {
  // Default list of tasks.
  $tasks = array(
    'select_versions' => t('Select versions'),
    'backup' => t('Backup site'),
    'install' => t('Install projects'),
    'enable' => t('Enable projects'),
  );

  drupal_add_region_content('sidebar_first', theme('task_list', array('items' => $tasks, 'active' => $active)));
}

/**
 * Batch function to install a single release of a project
 */
function _project_browser_batch_install_release($release, &$context) {
  // Fill in values.
  $form_state = array();
  $form_state['values']['project_url'] = $release['download_url'];
  $form_state['values']['op'] = t('Install');
  
  drupal_form_submit('update_manager_install_form', $form_state, $release['type']);
  
  // Check for errors
  $errors = form_get_errors();
  
  if (is_array($errors) AND !empty($errors)) {
    watchdog('project_browser', 'There was an error while installing %project. 
      The errors are as follows: <br />@errors',
      array('@errors' => implode('<br />', $errors)), WATCHDOG_ERROR);
    $context['results'][] = t('Error installing %project. Errors have been logged.', array('%project' => $release['title']));
    $context['message'] = t('Error installing %project.', array('%project' => $release['title']));
  }
  else {
    $context['results'][] = t('Successfully installed %project.', array('%project' => $release['title']));
    $context['message'] = t('Installing projects: %project...', array('%project' => $release['title']));
  }
}

/**
 * Show a message and finish up the batch
 */
function _project_browser_batch_install_releases_finished($success, $results, $operations) {
  foreach ($results as $result) {
    drupal_set_message($result);
  }
  
  if ($success) {
    $message = format_plural(count($results), 'One project installed', '@count projects installed.');
  }
  else {
    $message = t('Finished with an error.');
  }
  drupal_set_message($message);
  
  drupal_goto('admin/project_browser/install/enable');
}