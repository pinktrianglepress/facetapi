// $Id$

Drupal.behaviors.facetapi = function(context) {
  
  // Iterates over the settings array for each facet in each realm for each
  // searcher module.
  $.each(Drupal.settings.facetapi, function (searcher, realms) {
    $.each(Drupal.settings.facetapi[searcher], function (realm_name, facets) {
      $.each(Drupal.settings.facetapi[searcher][realm_name], function (facet_name, settings) {
        
        // A limit of 0 means no limit.
        if (settings.limit > 0) {
          
          // Hides all items that are past the "soft limit".
          limit=settings.limit - 1;
          $('div#block-facetapi-'+searcher+'-'+realm_name+'-'+facet_name+' ul:first').find('li:gt('+limit+')').hide();
          
          // Finds all items that have more values than our soft limit allows
          // for, adds the show more/less links as appropriate.
          $('div#block-facetapi-'+searcher+'-'+realm_name+'-'+facet_name+' ul:first').filter(function() {
            return $(this).find('li').length > settings.limit;
          }).each(function() {
            $('<a href="#" class="facetapi-hide-link"></a>').text(Drupal.t('Show more')).click(function() {
              
              // Gets the limit for the facet, adds show more/less link.
              cur_limit = Drupal.settings.facetapi[searcher][realm_name][facet_name].limit - 1;
              if ($(this).prev().find('li:hidden').length > 0) {
                $(this).prev().find('li:gt('+cur_limit+')').slideDown();
                $(this).text(Drupal.t('Show less'));
              }
              else {
                $(this).prev().find('li:gt('+cur_limit+')').slideUp();
                $(this).text(Drupal.t('Show more'));
              }
              return false;
            }).insertAfter($(this));
          });
        }
      });
    });
  });
};
