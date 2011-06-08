<?php 
// $Id$

/**
 * @file
 * 
 * Default theme implementation for displaying a project.
 * 
 * 
 * @see project_browser_preprocess_project_browser_project
 */

$class = ($first) ? 'project-item-first' : 'project-item';
 
?>
<div class="<?php print $class; ?>">
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
  <div class='clearfix'></div>
</div>
