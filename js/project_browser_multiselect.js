(function ($) {
  $(document).ready(function() {
    $('#edit-categories').multiselect({
      noneSelectedText: Backdrop.t("Choose"),
      selectedList: 99,
      minWidth: 500,
      position: {
        my: 'right top',
        at: 'right bottom'
      }
    });
  });
})(jQuery);
