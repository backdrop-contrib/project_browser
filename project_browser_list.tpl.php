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
<div id="dashboard" class="clearfix">
  <div class="project-browser dashboard-region">
    <div class='block'>
      <h2><?php print $title ?></h2>
      <div class='project-browser-main-content'>
        <?php print $main_content; ?>
      </div>
    </div>
    <?php //print $pager; ?>
  </div>

  <div class="project-browser-sidebar-right dashboard-region">
    <div class="project-browser-filters-block block">
      <h2>Filters</h2>
      <?php print $filters; ?>
    </div>
    <div class="project-browser-install-queue-block block">
      <h2>Install list</h2>
      <?php print $install_list; ?>
    </div>
  </div>
</div>