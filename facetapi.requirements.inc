<?php

/**
 * @file
 * Contains requirements callbacks.
 */

/**
 * Checks equality of a realm property.
 */
function facetapi_requirement_realm_property(array $realm, array $facet, array $options) {
  return facetapi_requirement_property($realm, $options);
}

/**
 * Checks equality of a facet property.
 */
function facetapi_requirement_facet_property(array $realm, array $facet, array $options) {
  return facetapi_requirement_property($facet, $options);
}

/**
 * Checks the equality of a property.
 */
function facetapi_requirement_property(array $definition, array $options) {
  $passed = TRUE;
  foreach ($options as $key => $requirement) {
    $condition = $definition[$key];
    if (is_array($requirement)) {
      if (array_intersect_key($condition, $requirement) != $requirement) {
        $passed = FALSE;
        break;
      }
    }
    else {
      if ($requirement != $condition) {
        $passed = FALSE;
        break;
      }
    }
  }
  return $passed;
}

/**
 * Checks whether a realm property is set.
 */
function facetapi_requirement_realm_property_set(array $realm, array $facet, array $options) {
  return facetapi_requirement_property_set($realm, $options);
}

/**
 * Checks whether a facet property is set.
 */
function facetapi_requirement_facet_property_set(array $realm, array $facet, array $options) {
  return facetapi_requirement_property_set($facet, $options);
}

/**
 * Checks whether a property is set.
 */
function facetapi_requirement_property_set(array $definition, array $options) {
  $passed = TRUE;
  foreach ($options as $key => $requirement) {
    if (!($requirement ? !empty($definition[$key]) : empty($definition[$key]))) {
      $passed = FALSE;
      break;
    }
  }
  return $passed;
}

/**
 * Checks whether the facet is hierarchical.
 */
function facetapi_requirement_facet_hierarchical(array $realm, array $facet, $option) {
  $options = array('hierarchy callback' => (bool) $option);
  return facetapi_requirement_facet_property_set($realm, $facet, $options);
}
