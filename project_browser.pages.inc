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
  drupal_add_js(drupal_get_path('module', 'project_browser') . '/project_browser_more_link.js', 
   array('scope' => 'footer'));
  
  // Build the filters
  $drupal_version = explode('.', DRUPAL_CORE_COMPATIBILITY);
  $filters = array(
    'drupal_version' => $drupal_version[0],
    'type' => $type,
  );
  
  // Add filters
  if (isset($_SESSION['project_browser_category_filter_' . $type])) {
    $categories = array_filter($_SESSION['project_browser_category_filter_' . $type]);
    if (!empty($categories)) {
      $filters['categories'] = project_browser_prepare_categories($categories, $type);
    }
  }
  if (isset($_SESSION['project_browser_text_filter_' . $type])) {
    $filters['text'] = $_SESSION['project_browser_text_filter_' . $type];
  }
  $filters['requested'] = 12;
  $filters['page'] = isset($_GET['page']) ? $_GET['page'] : 0;
  
  // Get the projects to display here based on the filters
  $results = project_browser_fetch_results($filters);
  
  // Save the listed projects in the session so it can be used
  $_SESSION['project_browser_listed_projects'] = $results['projects'];
  
  $list = array();
  foreach ($results['projects'] as $project) {
    $list[] = $project;
  }
  
  // Add the pager
  $total = $results['total'];
  $num_per_page = 12;
  $page = pager_default_initialize($total, $num_per_page);
  $offset = $num_per_page * $page;
  $start = ($total) ? $offset + 1 : 0;
  $finish = $offset + $num_per_page;
  if ($finish > $total) {
    $finish = $total;
  }
  
  $build = array();
  $build['content'] = array(
    'project_browser_header' => array(
      '#markup' => t('Showing @start to @finish of @total.', array(
        '@start' => $start, '@finish' => $finish, '@total' => $total)),
      '#weight' => 1,
    ),
    'project_browser_list' => array(
      '#markup' => theme('project_browser_list', array('projects_list' => $list, 'type' => $type)),
      '#weight' => 2,
    ),
    'pager' => array(
      '#theme' => 'pager',
      '#weight' => 5,
    ),
  );
  
  return $build;
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
      drupal_set_title(t("Select versions"));
      $content = project_browser_installation_select_versions_page();
      break;
    case 'backup':
      drupal_set_title(t("Backup"));
      $content = project_browser_installation_backup_page();
      break;
    case 'install':
      drupal_set_title(t("Install"));
      $content = project_browser_installation_install_page();
      break;
    case 'enable':
      drupal_set_title(t("Enable modules"));
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
  
  $form['#tree'] = TRUE;
  
  $form['releases'] = array();
  
  foreach ($projects as $project) {
    // Get the available releases for this project
    $release_data = project_browser_get_project_release_data($project);
    
    // We use the update module to calculate the recommended version
    $project_data = array(
      'existing_major' => 0,
      'existing_version' => 0,
      'install_type' => '',
    );
    module_load_include('inc', 'update', 'update.compare');
    update_calculate_project_update_status($project, $project_data, $release_data);
    // dpm($project_data);

    $releases_list = array();
    
    foreach ($release_data['releases'] as $version => $release) {
      $release_title = t("@version - @date", array(
        '@version' => $release['version'],
        '@date' => format_date($release['date'], 'custom', 'M j, Y'),
        ));
      if (isset($release['terms']['Release type']) AND !empty($release['terms']['Release type'])) {
        $release_title .= " (" . implode(', ', $release['terms']['Release type']) . ")";
      }
      $releases_list[$version] = l($release_title, $release['release_link']);
    }
    
    $form['releases'][$project['name']] = array(
      '#type' => 'fieldset',
      '#title' => $project['title'],
      '#collapsible' => FALSE,
      '#prefix' => '<div id="project-browser-select-versions-'. $project['name'] .'">',
      '#suffix' => '</div>',
    );
    
    $form['releases'][$project['name']]['project'] = array(
      '#type' => 'value',
      '#value' => $project,
    );
    $recommended_releases = array();
    $recommended_releases[$project_data['recommended']] = $releases_list[$project_data['recommended']];
    
    $form['releases'][$project['name']]['release_name'] = array(
      '#type' => 'radios',
      '#title' => t('Recommended Release'),
      '#options' => $recommended_releases,
      '#default_value' => $project_data['recommended'],
      '#required' => TRUE,
    );
    if (isset($form_state['values']['releases'][$project['name']]['more']) AND $form_state['values']['releases'][$project['name']]['more']) {
      $form['releases'][$project['name']]['release_name']['#title'] = t('Select release');
      $form['releases'][$project['name']]['release_name']['#options'] = $releases_list;
    }
    else {
      $form['releases'][$project['name']]['more'] = array(
        '#type' => 'checkbox',
        '#title' => t('Show more releases'),
        '#default_value' => isset($form_state['values']['releases'][$project['name']]['more']) ? $form_state['values']['releases'][$project['name']]['more'] : FALSE,
        '#ajax' => array(
            'callback' => 'project_browser_select_releases_ajax_callback',
            'wrapper' => 'project-browser-select-versions-'. $project['name'],
            'method' => 'replace',
            'effect' => 'fade',
        ),
      );
    }
  }

  $form['backup_warning'] = array(
    '#type' => 'markup',
    '#markup' => t('Back up your database and site before you continue. !link.',
      array('!link' => l(t('Learn how'), 'http://drupal.org/node/22281'))),
  );
  $form['maintenance_mode'] = array(
    '#type' => 'checkbox', 
    '#title' => t('Perform updates with site in maintenance mode (strongly recommended)'), 
    '#default_value' => TRUE, 
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Next'),
  );
  
  return $form;
}

/**
 * Serve the extra releases via AJAX
 */
function project_browser_select_releases_ajax_callback($form, &$form_state) {
  $project = $form_state['triggering_element']['#parents'][1];
  return $form['releases'][$project];
}

/**
 * Submit handler for the select versions form
 */
function project_browser_installation_select_versions_form_submit($form, &$form_state) {
  // Store maintenance_mode setting so we can restore it when done.
  $_SESSION['maintenance_mode'] = variable_get('maintenance_mode', FALSE);
  if ($form_state['values']['maintenance_mode'] == TRUE) {
    variable_set('maintenance_mode', TRUE);
  }
  
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
  $release_data = project_browser_get_project_release_data($project);
  
  return isset($release_data['releases'][$release_name]) ? $release_data['releases'][$release_name] : FALSE;
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
    '#description' => t('You are about to install the following projects:'),
    '#tree' => TRUE,
    '#collapsible' => FALSE,
  );

  foreach ($releases as $release_name => $project) {
    $items[] = $project['title'] . ' ' . $release_name;
  }
  
  $form['releases']['items'] = array(
    '#type' => 'item',
    '#markup' => theme('item_list', array('items' => $items)),
  );
  
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
  $installed_projects = project_browser_get_installed_projects();
  
  return drupal_get_form('project_browser_installation_enable_form', $installed_projects);
}

/**
 * Form builder for the select versions form
 * 
 * @param array $projects
 *   An array of projects to get the releases for
 */
function project_browser_installation_enable_form($form, &$form_state, $projects) {
  // Restore the maintenance mode to what it was at the end
  variable_set('maintenance_mode', $_SESSION['maintenance_mode']);
  
  $modules = system_rebuild_module_data();
  
  $form['instructions'] = array(
    '#type' => 'item',
    '#markup' => t('The projects you selected have been successfully installed. 
      If you installed any new modules, you may enable them using the form below 
      or on the main !link page.', array('!link' => l(t('Modules'), 'admin/modules'))),
  );
  
  $options = array();
  $missing = array();
  
  foreach ($projects as $project) {
    if ($project['type'] == 'module') {
      $dependency_check = TRUE;
      $dependencies = array();
      if (isset($modules[$project['name']])) {
        foreach ($modules[$project['name']]->info['dependencies'] as $dependency) {
          if (isset($modules[$dependency])) {
            $dependencies[] = $modules[$dependency]->info['name'] .' ('. t('Installed') .')';
          }
          else {
            $dependency_check = FALSE;
            $dependencies[] = $dependency .' ('. t('Missing') .')';
          }
        }
        if ($dependency_check) {
          $options[$project['name']] = array(
            array('data' => $modules[$project['name']]->info['name']),
            array('data' => $modules[$project['name']]->info['version']),
            array('data' => implode(', ', $dependencies)),
          );
        }
        else {
          $missing[$project['name']] = array(
            array('data' => $modules[$project['name']]->info['name']),
            array('data' => $modules[$project['name']]->info['version']),
            array('data' => implode(', ', $dependencies)),
          );
        }
      }
      else {
        drupal_set_message(t('There was an error getting information for @module',
          array('@module' => $project['name'])), 'error');
      }
    }
  }

  $headers = array(
    array('data' => t('Title')),
    array('data' => t('Version')),
    array('data' => t('Dependencies')),
  );
  
  if (!empty($options)) {
    $form['modules'] = array(
      '#type' => 'tableselect', 
      '#title' => t('Enable modules'),
      '#description' => t('Select which modules you would like to enable.'),
      '#header' => $headers, 
      '#options' => $options, 
      '#empty' => t('No new modules installed.'), 
      '#multiple' => TRUE,
      '#js_select' => TRUE,
      '#weight' => 1,
    );
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#submit' => array('project_browser_installation_enable_form_submit'),
      '#value' => t('Enable modules'),
      '#weight' => 99,
    );
  }
  
  if (!empty($missing)) {
    $form['missing'] = array(
      '#type' => 'item', 
      '#title' => t('Missing Dependencies'),
      '#description' => t('These modules are missing one or more dependencies, 
        and so cannot be enabled.'),
      '#markup' => theme('table', array('header' => $headers, 'rows' => $missing)), 
      '#weight' => 2,
    );
  }
  
  return $form;
}

/**
 * Form submit handler
 */
function project_browser_installation_enable_form_submit($form, &$form_state) {
  $enable_queue = array_filter($form_state['values']['modules']);
  // TODO - Enable these all at once?
  // module_enable($enable_queue);
  
  // Enable the modules one by one
  foreach ($enable_queue as $project_name) {
    if (!module_enable(array($project_name))) {
      drupal_set_message(t('Error installing a module: @module', array('@module' => $project_name)), 'error');
    }
  }
  
  drupal_goto('admin/modules');
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