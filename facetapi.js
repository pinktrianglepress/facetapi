// $Id$

Drupal.behaviors.facetapi = function(context) {
  $.each(Drupal.settings.facetapi, function (searcher, realms) {
    $.each(Drupal.settings.facetapi[searcher], function (realm_name, facets) {
      $.each(Drupal.settings.facetapi[searcher][realm_name].$, function (facet_name, settings) {
        if (settings.limit > 0) {
          limit=settings.limit - 1;
          $('div#block-facetapi-'+searcher+'-'+realm_name+'-'+facet_name+' ul').find('li:gt('+limit+')').hide();
          $('div#block-facetapi-'+searcher+'-'+realm_name+'-'+facet_name+' ul').filter(function() {
            return $(this).find('li').length > settings.limit;
          }).each(function() {
            $('<a href="#" class="facetapi-hide-link'+searcher+'"></a>').text(Drupal.t('Show more')).click(function() {
              cur_limit=Drupal.settings.facetapi[$(this).attr('class').substring(21)]['limit'] - 1;
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
