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
     	<?php print $install; ?>
  </div>
  <div class='clearfix'></div>
</div>
