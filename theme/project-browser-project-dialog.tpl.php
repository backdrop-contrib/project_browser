<?php 
/**
 * @file
 * 
 * Default theme implementation for displaying a project in a dialog.
 * 
 * Available variables:
 * - $image: The project image, if any
 * - $title: The title of the project
 * - $author: Ther author of the project
 * - $last_updated: A string representing when the project was last updated
 * - $description: The project description. This could be long.
 * - $extras: A string of any extra info that should go at the bottom
 * - $status: Whether the project is already installed, enabled, or not
 * - $install: The add to install queue button/link
 * These are defined in project_browser_preprocess_project_browser_project()
 * 
 * @see project_browser_preprocess_project_browser_project()
 */ 
?>

<div class="project-item">
  <?php if ($image) { ?>
    <div class='project-image'>
      <img src='<?php print $image; ?>' />
    </div>
  <?php } ?>
  
  <div class="project-information" >
    <div class="project-title">
      <?php print $title; ?>
    </div>
    
    <div class="project-author">
      <?php print $author; ?>
    </div>
    
    <div class="project-updated">
      <?php print $last_updated; ?>
    </div>
    
    <div class="project-description">
      <?php print $description; ?>
    </div>
  </div>
  
  <div class="project-extra">
    <?php print $extras; ?>
  </div>
  
  <div class="project-status">
      <?php print $status; ?>
  </div>
  <div class='clearfix'></div>
</div>
