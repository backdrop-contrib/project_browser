<?php 
// $Id$

/**
 * @file
 * 
 * Default theme implementation for displaying a project.
 * 
 * 
 * @see template_preprocess_project_browser_project
 */
?>
<div class="project-item">
  <div class="project-information" >
    <div class="project-title">
      <?php print $title; ?>
    </div>
    <div class="project-description">
      <?php print $description; ?>
    </div>
  </div>
  <div class="project-status">
      <?php print $status; ?>
     	<?php print $install; ?>
  </div>
</div>
