// $Id$

/**
 * Iterates over facets, performs facet initialization.
 */
Drupal.behaviors.facetapi = function(context) {
  for (var index in Drupal.settings.facetapi.facets) {
    var facet = new facetapi.facet(Drupal.settings.facetapi.facets[index]);
    facet.init();
  }
}

/**
 * Constructor for facet settings class.
 */
function facetapi() {}

/**
 * Constructor for the realm object.
 * 
 * @param searcher
 *   The name of the searcher module.
 * @param realmName
 *   The machine readable name of the realm.
 */
facetapi.realm = function(searcher, realmName) {
  this.searcher  = searcher;
  this.realmName = realmName;
}

/**
 * Returns a selector for a facet in the realm.
 *
 * @param facetName
 *   A string containing the machine readable name of the facet.
 * @return
 *   A string containing the selector.
 */
facetapi.realm.prototype.facetSelector = function (facetName) {
  return '#facetapi-facet-' + this.searcher + '-' + this.realmName + '-' + facetName;
}

/**
 * Returns a selector for all facets in the realm.
 * 
 * @param searcher
 *   A string containing the name of the searcher module.
 * @param facetName
 *   A string containing the machine readable name of the facet.
 * @return
 *   A string containing the selector.
 */
facetapi.realm.prototype.selector = function() {
  return '.facetapi-facet-' + this.searcher + '-' + this.realmName;
}

/**
 * Calls the facet initialization method if defined.
 * 
 * @param
 *   A facetapi.facet object.
 */
facetapi.realm.prototype.facetInit = function(facet) {
  var init = new facetapi.realm.init(this);
  if (this.realmName in init) {
    init[this.realmName](facet);
  }
}

/**
 * Constructor for class containing initialization methods for facets within
 * the realm.
 * 
 *  @param realm
 *    A facetapi.realm object.
 */
facetapi.realm.init = function(realm) {
  this.realm = realm;
}

/**
 * Facet object, handles display of facets.
 * 
 * @param settings
 *   The facet settings passed through drupal_add_js().
 */
facetapi.facet = function(settings) {
  this.realm    = new facetapi.realm(settings.searcher, settings.realmName);
  this.settings = settings;
}

/**
 * Initializes the facet.
 */
facetapi.facet.prototype.init = function() {
  this.realm.facetInit(this);
}

/**
 * Returns the jQuery selector for this facet.
 * 
 * @return
 *   A jQuery object containing the facet.
 */
facetapi.facet.prototype.selector = function() {
  return this.realm.facetSelector(this.settings.facetName);
}
