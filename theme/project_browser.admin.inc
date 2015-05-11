<?php
/**
 * @file
 * Project Browser module admin pages.
 */

// ======================================
// Administration Page:
// ======================================

/**
 * Admin Settings Form
 */
function project_browser_admin() {
  $form['main'] = array(
    '#type' => 'fieldset',
    '#title' => t('Main settings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['main']['project_browser_servers'] = array(
    '#type' => 'textarea',
    '#title' => t('Repositories'),
    '#default_value' => config_get('project_browser.settings', 'project_browser_servers'),
    '#description' => t("Add new repositories to use for the Project Browser, one per line, in 
      the 'url|method|Site Name' format. <MAINSITE_URL> is added by default, and doesn't need to be 
      set here."),
    '#required' => FALSE,
  );


  // Add a submit button
  $form['actions']['#type'] = 'actions';
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  // Return the form!
  return $form;
}

// Add a submit function to process config
function project_browser_admin_submit($form, &$form_state) {
  $values = $form_state['values'];
  $config = config('project_browser.settings');
  $config->set('project_browser_servers', $form_state['values']['project_browser_servers']);
  $config->save();
  backdrop_set_message(t('Configuration options saved.'));
}

