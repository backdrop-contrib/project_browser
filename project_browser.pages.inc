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
  $drupal_version = explode('.', DRUPAL_CORE_COMPATIBILITY);
  $filters = array(
    'drupal_version' => $drupal_version[0],
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
  
  // Save the listed projects in the session so it can be used
  $_SESSION['project_browser_listed_projects'] = $projects;
  
  $list = array();
  foreach ($projects as $project) {
    $list[] = $project;
  }
  
  return theme('project_browser_list', array('projects_list' => $list, 'type' => $type));
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
  drupal_add_css(drupal_get_path('module', 'project_browser') . '/project_browser.css', array('preprocess' => FALSE));
  
  switch ($op) {
    case 'select_versions':
      $content = project_browser_installation_select_versions_page();
      break;
    case 'backup':
      $content = project_browser_installation_backup_page();
      break;
    case 'install':
      $content = project_browser_installation_install_page();
      break;
    case 'finished':
      $content = project_browser_installation_enable_page();
      break;
  }
  return theme('project_browser_install', array('current_task' => $op, 'main_content' => drupal_render($content)));
}

/**
 * Task page for the Select versions installation task
 * 
 * Shows a form where the user can select which versions to install for each 
 * project
 */
function project_browser_installation_select_versions_page() {
  // Show a form that lets the user select which version of the projects to install
  $queued_projects = project_browser_get_queued_projects();
  
  return drupal_get_form('project_browser_installation_select_versions_form', $queued_projects);
}

/**
 * Form builder for the select versions form
 * 
 * @param array $projects
 *   An array of projects to get the releases for
 */
function project_browser_installation_select_versions_form($form, &$form_state, $projects) {
  $form = array();
  
  // First unset any old data
  unset($_SESSION['project_browser_install_releases_list']);
  unset($_SESSION['project_browser_installed_projects']);
  
  $form['releases'] = array(
    '#type' => 'fieldset',
    '#title' => 'Select which version to install',
    '#tree' => TRUE,
    '#collapsible' => FALSE,
  );

  foreach ($projects as $project) {
    // Get the available releases for this project
    $releases = project_browser_get_project_releases($project);

    $releases_list = array();
    
    foreach ($releases as $version => $release) {
      $release_title = t("@version - @date", array(
        '@version' => $release['version'],
        '@date' => format_date($release['date'], 'custom', 'M j, Y'),
        ));
      if (isset($release['terms']['Release type']) AND !empty($release['terms']['Release type'])) {
        $release_title .= " (" . implode(', ', $release['terms']['Release type']) . ")";
      }
      $releases_list[$version] = l($release_title, $release['release_link']);
    }
    
    $form['releases']['project-' . $project['name']] = array(
      '#type' => 'fieldset',
      '#title' => $project['title'],
      '#collapsible' => FALSE,
    );
    $form['releases']['project-' . $project['name']]['project'] = array(
      '#type' => 'value',
      '#value' => $project,
    );
    $form['releases']['project-' . $project['name']]['release_name'] = array(
      '#type' => 'radios',
      '#title' => t('Select release'),
      '#options' => $releases_list,
      '#required' => TRUE,
    );
  }
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
  
  return $form;
}

/**
 * Submit handler for the select versions form
 */
function project_browser_installation_select_versions_form_submit($form, &$form_state) {
  foreach ($form_state['values']['releases'] as $item) {
    // Load the selected release
    if ($release = project_browser_get_release($item['release_name'], $item['project'])) {
      // Add the release to a session variable
      $_SESSION['project_browser_install_releases_list'][$item['release_name']] = $item['project'];
    }
  }
  
  drupal_goto('admin/modules/project_browser/install/install');
}

/**
 * Get a release from a project and a release_name
 */
function project_browser_get_release($release_name, $project) {
  $releases = project_browser_get_project_releases($project);
  
  return isset($releases[$release_name]) ? $releases[$release_name] : FALSE;
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
  // Show a Install button
  $queued_releases = project_browser_get_queued_releases();
  
  return drupal_get_form('project_browser_installation_install_form', $queued_releases);
}

/**
 * Form builder for the select versions form
 * 
 * @param array $projects
 *   An array of projects to get the releases for
 */
function project_browser_installation_install_form($form, &$form_state, $releases) {
  $form = array();
  
  $form['releases'] = array(
    '#type' => 'fieldset',
    '#title' => t('Install'),
    '#description' => t('You are about to install the following projects.'),
    '#tree' => TRUE,
    '#collapsible' => FALSE,
  );

  foreach ($releases as $release_name => $project) {
    $form['releases'][$release_name] = array(
      '#type' => 'item',
      '#markup' => $project['title'] . ' ' . $release_name,
    );
  }
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Install'),
  );
  
  return $form;
}

/**
 * Submit handler for the select versions form
 */
function project_browser_installation_install_form_submit($form, &$form_state) {
  module_load_include('inc', 'update', 'update.manager');
  
  $queued_releases = project_browser_get_queued_releases();
  
  $operations = array();
  foreach ($queued_releases as $release_name => $project) {
    $operations[] = array('_project_browser_batch_install_release', array($release_name, $project));
  }
  $batch = array(
    'operations' => $operations,
    'finished' => '_project_browser_batch_install_releases_finished',
    'title' => t('Installing projects'),
    'init_message' => t('Installing modules...'),
    'progress_message' => t('Installed @current out of @total.'),
    'error_message' => t('Installation has encountered an error.'),
    'file' => drupal_get_path('module', 'project_browser') . '/project_browser.inc',
  );
  batch_set($batch);
}

/**
 * Task page for the Enable projects installation task
 * 
 * Show a form which lets the user enable the newly installed projects
 */
function project_browser_installation_enable_page() {
  // TODO - Use http://api.drupal.org/api/drupal/developer--topics--forms_api_reference.html/7#tableselect
  $installed_projects = project_browser_get_installed_projects();
  dpm($installed_projects);
  return 'Placeholder';
}

/**
 * Get a task list to the sidebar area when installing projects
 * 
 * This will need to be called from every page of the install process
 * 
 * @param $active
 *   (Optional) Set the active task by key
 */
function project_browser_installation_task_list($active = NULL) {
  // Default list of tasks.
  $tasks = array(
    'select_versions' => l(t('Select versions'), 'admin/modules/project_browser/install/select_versions'),
    'backup' => t('Backup site'),
    'install' => t('Install projects'),
    'enable' => t('Enable projects'),
  );
  
  require_once DRUPAL_ROOT . '/includes/theme.maintenance.inc';
  
  return theme_task_list(array('items' => $tasks, 'active' => $active));
}