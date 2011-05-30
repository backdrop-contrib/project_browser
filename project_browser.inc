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
  if ($form_state['submitted']) {
    // TODO - Use tree to constrain this to a set variable parent
    $protected = array('submit', 'form_build_id', 'form_token', 'form_id', 'op');
    foreach ($form_state['values'] as $key => $value) {
      if (!in_array($key, $protected)) {
        $session_key = str_replace('_', ' ', $key);
        $_SESSION['project_browser_category'][$session_key] = $value;
      }
    }
  }
}

/**
 * Helper function to get install list.
 * 
 * Install list is all projects that are selected to install. 
 */
function project_browser_get_install_list() {
  // TODO - Finish this
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
  $categories = array();
  $protected = array('Drupal project', 'Modules', 'Themes');
  
  // TODO - Add a new function to get the categories via XMLRPC
  
  ksort($categories);
  return $categories;
}
