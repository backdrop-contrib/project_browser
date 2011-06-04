<?php 
// $Id$

/**
 * @file
 * 
 * Default theming implementation for displaying list of projects.
 * 
 * 
 * 
 * @see project_browser_preprocess_project_browser_list()
 */
?>
<div id="project-browser" class="clearfix">
  <div id="project-browser-main" class="project-browser-region">
    <?php print $main_content; ?>
    <?php //print $pager; ?>
  </div>

  <div id="project-browser-sidebar-right" class="project-browser-region">
    <?php print $filters; ?>
    <?php print $install_list; ?>
  </div>
  
</div>