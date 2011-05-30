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

/**
 * Page to install a module
 * 
 * @param string $op
 *    Operation to preform.
 * @param integer $pid
 *    Project ID
 */
function project_browser_installation_page($op) {
  switch ($op) {
    case 'install':
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
      
      drupal_goto('admin/project_browser');
      break;
  }
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
}

/**
 * Theme the list of projects.
 * 
 * @param $variables
 *    An associative array containing:
 *     - projects_list : array of all projects
 *     
 * @return
 *    A string containing the listing output.
 */
function template_preprocess_project_browser_list(&$variables) {
  include_once(drupal_get_path('module', 'project_browser') . '/project_browser.inc');
  
  // Add the themed list
  $variables['list'] = '';
  
  // Theme each individual project and add to the list
  foreach ($variables['projects_list'] as $project) {
    $variables['list'] .= theme('project_browser_project', array('project' => $project));
  }
  
  // Add the pager
  $variables['pager'] = theme('pager', array('tags' => NULL));
  
  // Add the filters
  $version_form = drupal_get_form('project_browser_version_filter_form');
  $filters = drupal_render($version_form);
  
  // Add the available category to filter on
  $category_form = drupal_get_form('project_browser_category_filter_form');
  $filters .= drupal_render($category_form); 
  
  // Add the sorting of the list
  $filters .= project_browser_sort_list();
  $variables['filters'] = $filters;
  
  // Add the install list
  $variables['install_list'] = project_browser_get_install_list();
}

/**
 * Theme one project.
 * 
 * @param $variables
 *   An associative array containing:
 *     - project : associative array of project variables
 *     
 * @return
 *   A string containing the listing output.
 */
function template_preprocess_project_browser_project(&$variables) {
  include_once(drupal_get_path('module', 'project_browser') . '/project_browser.inc');
  
  $project = $variables['project'];
  
  $variables['title'] = l(check_plain($project['title']), check_url($project['project_url']));
  $variables['description'] = $project['description'];
  
  // Check if the project is installed
  if (module_exists($project->short_name)) {
    $variables['status'] = '<div class="install-disabled">Already installed</div>'; 
    $variables['install'] = '';
  }
  elseif (FALSE) { // TODO - Add a check to see if the project is downloaded or not
    $variables['status'] = '<div class="install-disabled">Already downloaded</div>';
    $variables['install'] = '';
  }
  else {
    $variables['status'] = '<div class="install-enabled">Install</div>';
    $install_form = drupal_get_form('project_browser_' . $project->short_name . '_install_form', $project->pid);
    $variables['install'] = drupal_render($install_form);
  }
}
