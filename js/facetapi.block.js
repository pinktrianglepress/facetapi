// $Id$

/**
 * Initialization method for facets in the block realm.
 * 
 * Adds the show more / less links.
 * 
 * @param facet
 *   A facetapi.facet object.
 */
facetapi.realm.init.prototype.block = function(facet) {
  if (facet.settings.limit > 0) {
    var limit = facet.settings.limit - 1;
    
    // Hides facets over the limit.
    $(facet.selector()).find('li:gt(' + limit + ')').hide();
    
    // Finds all items that have more values than our soft limit allows for,
    // adds the show more / less links.
    $(facet.selector()).filter(function() {
      return $(this).find('li').length > facet.settings.limit;
    }).each(function() {
      $('<a href="#" class="facetapi-hide-link"></a>').text(Drupal.t('Show more')).click(function() {
        if ($(this).prev().find('li:hidden').length > 0) {
          $(this).prev().find('li:gt(' + limit + ')').slideDown();
          $(this).text(Drupal.t('Show less'));
        }
        else {
          $(this).prev().find('li:gt(' + limit + ')').slideUp();
          $(this).text(Drupal.t('Show more'));
        }
        return false;
      }).insertAfter($(this));
    });
  }
}
