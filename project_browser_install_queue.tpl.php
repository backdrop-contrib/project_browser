<?php 
// $Id$

/**
 * @file
 * 
 * Default theming implementation for displaying list of queued projects
 * 
 * Variables:
 * 
 * $projects
 *   An array of the projects to list
 * 
 * @see project_browser_preprocess_project_browser_list()
 */
?>
<div id="project-browser" class="clearfix">
  
  <?php
  
  foreach ($projects as $project) {
    print "<div class='project-browser-install-queue-item' rel='" . $project['name'] . "'>";
    print "<div class='project_browser-install-queue-item-title'>" . $project['title'] . "</div>";
    print "</div>";
  }
  
  ?>
  
</div>