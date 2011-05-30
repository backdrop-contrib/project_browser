<?php 
// $Id$

/**
 * @file
 * 
 * Default theming implementation for displaying the install page
 * 
 * 
 * 
 * @see project_preprocess_project_browser_install()
 */
?>
<?php if ($projects_list) :?>
  <div class="project-browser">
  <!-- 
    <div class="project-browser-header">
      <?php //print $header; ?>
    </div>  -->
    <?php print $list; ?>
    <?php //print $pager; ?>
  </div>
<?php else :?><!-- ENDIF / START ELSE -->
  <div class="project-browser">
    <h2><?php print t('No results found.')?></h2>
  </div>
<?php endif; ?>
<div class="project-browser-sidebar-right">
  <?php print $filter; ?>
</div>
