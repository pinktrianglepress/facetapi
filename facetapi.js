(function ($) {

Drupal.behaviors.facetapi = {
  attach: function(context, settings) {
    $('.facetapi-hidden-facet', context).hide();
    $('<a href="#" class="facetapi-showhide"></a>').text(Drupal.t('Show more')).click(function() {
      if ($(this).parent().find('.facet-hidden:visible').length == 0) {
        $(this).parent().find('.facet-hidden').show();
        $(this).text(Drupal.t('Show fewer'));
      }
      else {
        $(this).parent().find('.facet-hidden').hide();
        $(this).text(Drupal.t('Show more'));
      }
      return false;
    }).appendTo($(settings.apachesolr_show_more_blocks, context));

    // Find all checkbox facet links and give them a checkbox
    $('a.facet-checkbox.facet-click', context).each(Drupal.facetapi.addCheckbox);
    // Find all unclick links and turn them into checkboxes
    $('a.facet-checkbox.facet-unclick', context).each(Drupal.facetapi.makeCheckbox);
  }
}

Drupal.facetapi = {}

/**
 * Constructor for a class.
 */
Drupal.facetapi.Redirect = function(href) {
  this.href = href;
}

/**
 * Method to redirect to the stored href.
 */
Drupal.facetapi.Redirect.prototype.gotoHref = function() {
  window.location.href = this.href;
}

Drupal.facetapi.addCheckbox = function() {
  if (!$(this).hasClass('facet-checkbox-processed')) {
    // Create an unchecked checkbox.
    var checkbox = $('<input type="checkbox" class="facet-checkbox" />');
    // Get the href of the link that is this DOM object.
    var href = $(this).attr('href');
    redirect = new Drupal.facetapi.Redirect(href);
    checkbox.click($.proxy(redirect, 'gotoHref'));
    $(this).before(checkbox).before('&nbsp;');
    $(this).addClass('facet-checkbox-processed');
  }
}

Drupal.facetapi.makeCheckbox = function() {
  // Create a checked checkbox.
  var checkbox = $('<input type="checkbox" class="facet-checkbox" checked="true" />');
  // Get the href of the link that is this DOM object.
  var href = $(this).attr('href');
  redirect = new Drupal.facetapi.Redirect(href);
  checkbox.click($.proxy(redirect, 'gotoHref'));
  // Add the checkbox, hide the link.
  $(this).before(checkbox).hide();
}

})(jQuery);
