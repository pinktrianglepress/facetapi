// $Id$

Drupal.behaviors.facetapi = function(context) {

  // Iterates over the settings array for each facet in each realm for each
  // searcher module.
  $.each(Drupal.settings.facetapi, function (searcher, realms) {
    $.each(Drupal.settings.facetapi[searcher], function (realmName, facets) {
      $.each(Drupal.settings.facetapi[searcher][realmName], function (facetName, settings) {
        
        // Converts to checkboxes.
        if (Drupal.settings.facetapi[searcher][realmName][facetName].widget == 'facetapi_checkboxes') {
          $('div#block-facetapi-' + searcher + '-' + realmName + '-' + facetName + ' ul li a:not([class=active])')
            .each(facetapi.addCheckbox)
          $('div#block-facetapi-' + searcher + '-' + realmName + '-' + facetName + ' ul li a.active')
            .each(facetapi.addActiveCheckbox)
        }
        
        // A limit of 0 means no limit.
        if (settings.limit > 0) {
          
          // Hides all items that are past the "soft limit".
          limit=settings.limit - 1;
          $('div#block-facetapi-' + searcher + '-' + realmName + '-' + facetName + ' ul:first')
            .find('li:gt('+limit+')')
            .hide();
          
          // Finds all items that have more values than our soft limit allows
          // for, adds the show more/less links as appropriate.
          $('div#block-facetapi-' + searcher + '-' + realmName + '-' + facetName + ' ul:first').filter(function() {
            return $(this).find('li').length > settings.limit;
          }).each(function() {
            $('<a href="#" class="facetapi-hide-link"></a>').text(Drupal.t('Show more')).click(function() {
              
              // Gets the limit for the facet, adds show more/less link.
              var softLimit = Drupal.settings.facetapi[searcher][realmName][facetName].limit - 1;
              if ($(this).prev().find('li:hidden').length > 0) {
                $(this).prev().find('li:gt(' + softLimit + ')').slideDown();
                $(this).text(Drupal.t('Show less'));
              }
              else {
                $(this).prev().find('li:gt(' + softLimit + ')').slideUp();
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

// Checkbox building methods.
function facetapi() {
  // Nothing to be done
}

facetapi.addCheckbox = function() {
  // Put href in context scope to be visible in the anonymous function.
  var href = $(this).attr('href');
  $(this).before($('<input type="checkbox" />')
    .attr('class', 'facetapi-checkbox')
    .click(function(){
      window.location.href = href;
    })
  );
}

facetapi.addActiveCheckbox = function() {
  // Create a checked checkbox.
  var checkbox = $('<input type="checkbox" />')
    .attr('class', 'facetapi-checkbox')
    .attr('checked', true);
  // Put href in context scope to be visible in the anonymous function.
  var href = $(this).attr('href');
  checkbox.click(function(){
    window.location.href = href;
  });
  // Add the checkbox.
  $(this).before(checkbox);
}
