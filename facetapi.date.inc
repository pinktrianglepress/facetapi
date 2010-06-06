<?php
// $Id$

/**
 * @file
 * Date API functions for facets.
 */

/**
 * String that represents a time gap of a year between two dates.
 */
define('FACETAPI_DATE_YEAR', 'YEAR');

/**
 * String that represents a time gap of a month between two dates.
 */
define('FACETAPI_DATE_MONTH', 'MONTH');

/**
 * String that represents a time gap of a day between two dates.
 */
define('FACETAPI_DATE_DAY', 'DAY');

/**
 * String that represents a time gap of an hour between two dates.
 */
define('FACETAPI_DATE_HOUR', 'HOUR');

/**
 * String that represents a time gap of a minute between two dates.
 */
define('FACETAPI_DATE_MINUTE', 'MINUTE');

/**
 * String that represents a time gap of a second between two dates.
 */
define('FACETAPI_DATE_SECOND', 'SECOND');

/**
 * Date string for ISO 8601 date formats.
 */
define('FACETAPI_DATE_ISO8601', 'Y-m-d\TH:i:s\Z');

/**
 * Helper function to convert dates from Unix timestamps into ISO 8601 format.
 *
 * @param $timestamp
 *   An integer containing the Unix timestamp being converted.
 * @return
 *   A string containing the date in ISO 8601 format,
 *
 * @see http://lucene.apache.org/solr/api/org/apache/solr/schema/DateField.html
 */
function facetapi_isodate($timestamp) {
  return gmdate(FACETAPI_DATE_ISO8601, $timestamp);
}

/**
 * Return the next smaller date gap.
 *
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values.
 * @return
 *   A string containing the smaller date gap, NULL if there is no smaller gap.
 *   See FACETAPI_DATE_* constants for valid values.
 */
function facetapi_gap_drilldown($gap) {
  $drilldown = array(
    FACETAPI_DATE_YEAR => FACETAPI_DATE_MONTH,
    FACETAPI_DATE_MONTH => FACETAPI_DATE_DAY,
    FACETAPI_DATE_DAY => FACETAPI_DATE_HOUR,
    FACETAPI_DATE_HOUR => FACETAPI_DATE_MINUTE,
  );
  return isset($drilldown[$gap]) ? $drilldown[$gap] : NULL;
}

/**
 * Determine the gap in a date range query filter that we generated.
 *
 * This function assumes that the start and end dates are the beginning and end
 * of a single period: 1 year, month, day, hour, minute, or second (all date
 * range query filters we generate meet this criteria).  So, if the seconds are
 * different, it is a second gap. If the seconds are the same (incidentally,
 * they will also be 0) but the minutes are different, it is a minute gap. If
 * the minutes are the same but hours are different, it's an hour gap. etc.
 *
 * @param $start_date
 *   A string containing the start date as an ISO date string.
 * @param $end_date
 *   A string containing the end date as an ISO date string.
 * @return
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values. Returns FALSE of either of the dates cannot be converted to a
 *   timestamp.
 */
function facetapi_query_gap_get($start_date, $end_date) {
  $pattern = '@(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})@';

  // An array keyed by date gap constants to index position in $pattern.
  // NOTE: Order is important so we can get the smallest matched gap first.
  $gap_idx = array(
    FACETAPI_DATE_SECOND => 6,
    FACETAPI_DATE_MINUTE => 5,
    FACETAPI_DATE_HOUR => 4,
    FACETAPI_DATE_DAY => 3,
    FACETAPI_DATE_MONTH => 2,
    FACETAPI_DATE_YEAR => 1,
  );

  // Executes the preg_match, extracts pieces from the date string.
  if (preg_match($pattern, $start_date, $start) && preg_match($pattern, $end_date, $end)) {
    // Returns the gap constant on first match.
    foreach ($gap_idx as $gap => $idx) {
      if ($start[$idx] != $end[$idx]) {
        return $gap;
      }
    }
  }

  // If we can't tell, defaults to year.
  return FACETAPI_DATE_YEAR;
}

/**
 * Determines the best search gap to use for an arbitrary date range.
 *
 * Generally, we use the maximum gap that fits between the start and end date.
 * If they are more than a year apart, 1 year; if they are more than a month
 * apart, 1 month; etc.
 *
 * This function uses Unix timestamps for its computation and so is not useful
 * for dates outside that range.
 *
 * @param $start_date
 *   A string containing the start date as an ISO date string.
 * @param $end_date
 *   A string containing the end date as an ISO date string.
 * @return
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values. Returns FALSE of either of the dates cannot be converted to a
 *   timestamp.
 */
function facetapi_timestamp_gap_get($start_time, $end_time) {
  switch (TRUE) {
    // NOTE: 86400365 == 60 * 60 * 24 * 365
    case ($end_time - $start_time >= 86400365):
      return FACETAPI_DATE_YEAR;

    case (date('Ym', $start_time) != date('Ym', $end_time)):
      return FACETAPI_DATE_MONTH;

    case ($end_time - $start_time > 86400):
      return FACETAPI_DATE_DAY;

    case ($end_time - $start_time > 3600):
      return FACETAPI_DATE_HOUR;

    default:
      return FACETAPI_DATE_MINUTE;
  }
}

/**
 * Converts ISO date strings to Unix timestamps, passes values to the
 * facetapi_timestamp_gap_get() function to calculate the gap.
 *
 * @param $start_date
 *   A string containing the start date as an ISO date string.
 * @param $end_date
 *   A string containing the end date as an ISO date string.
 * @return
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values. Returns FALSE of either of the dates cannot be converted to a
 *   timestamp.
 */
function facetapi_date_gap_get($start_date, $end_date) {

  // Converts dates to Unix timestamps.
  $start_time = strtotime($start_date);
  $end_time = strtotime($end_date);

  // NOTE: Previous to PHP 5.1.0, this strtotime returns -1 on failure.
  if (FALSE !== $start_time && -1 != $start_time && FALSE !== $end_time && -1 != $end_time) {
    return facetapi_timestamp_gap_get($start_time, $end_time);
  }
  return FALSE;
}

/**
 * Returns a formatted date based on the passed timestamp and gap.
 *
 * This function assumes that gaps less than one day will be displayed in a
 * search context in which a larger containing gap including a day is already
 * displayed.  So, HOUR, MINUTE, and SECOND gaps only display time information,
 * without date.
 *
 * @param $timestamp
 *   An integer containing the Unix timestamp.
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values, defaults to YEAR.
 * @return
 *   A gap-appropriate display date used in the facet link.
 */
function facetapi_timestamp_format($timestamp, $gap = FACETAPI_DATE_YEAR) {
  // TODO: If we assume that multiple search queries are formatted in order, we
  // could store a static list of all gaps we've formatted. Then, if we format
  // an HOUR, MINUTE, or SECOND without previously having formatted a DAY or
  // later, we could include date information.  However, we'd need to do that
  // per-field and I'm not our callers always have field information handy.
  switch ($gap) {
    case FACETAPI_DATE_MONTH:
      return format_date($timestamp, 'custom', 'F Y', 0);

    case FACETAPI_DATE_DAY:
      return format_date($timestamp, 'custom', 'F j, Y', 0);

    case FACETAPI_DATE_HOUR:
      return format_date($timestamp, 'custom', 'g A', 0);

    case FACETAPI_DATE_MINUTE:
      return format_date($timestamp, 'custom', 'g:i A', 0);

    case FACETAPI_DATE_SECOND:
      return format_date($timestamp, 'custom', 'g:i:s A', 0);

    default:
      return format_date($timestamp, 'custom', 'Y', 0);
  }
}

/**
 * Returns a formatted date based on the passed ISO date string and gap.
 *
 * @param $date
 *   A string containing the date as an ISO date string.
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values, defaults to YEAR.
 * @return
 *   A gap-appropriate display date used in the facet link.
 */
function facetapi_date_format($date, $gap = FACETAPI_DATE_YEAR) {
  $timestamp = strtotime($date);
  return facetapi_timestamp_format($timestamp, $gap);
}

/**
 * Format the beginning of a date range query filter that we generated.
 *
 * @param $start_date
 *   A string containing the start date as an ISO date string.
 * @param $end_date
 *   A string containing the end date as an ISO date string.
 * @return
 *   A gap-appropriate display date used in the facet link.
 */
function facetapi_format_range($start_date, $end_date) {
  $gap = facetapi_query_gap_get($start_date, $end_date);
  return facetapi_date_format($start_date, $gap);
}

/**
 * Returns the next increment from the given ISO date and gap.
 *
 * @param $date
 *   A string containing the date as an ISO date string.
 * @param $gap
 *   A string containing the gap, see FACETAPI_DATE_* constants for valid
 *   values, defaults to YEAR.
 * @return
 *   A string containing the date, FALSE if the passed date could not be parsed.
 */
function facetapi_next_date_increment_get($date, $gap) {
  $pattern = '@(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})@';
  if (preg_match($pattern, $date, $match)) {

    // Increments the timestamp.
    switch ($gap) {
      case FACETAPI_DATE_MONTH:
        $match[2] += 1;
        break;
      case FACETAPI_DATE_DAY:
        $match[3] += 1;
        break;
      case FACETAPI_DATE_HOUR:
        $match[4] += 1;
        break;
      case FACETAPI_DATE_MINUTE:
        $match[5] += 1;
        break;
      case FACETAPI_DATE_SECOND:
        $match[6] += 1;
        break;
      default:
        $match[1] += 1;
        break;
    }

    // Gets the next incremenet.
    return facetapi_isodate(
      gmmktime($match[4], $match[5], $match[6], $match[2], $match[3], $match[1])
    );
  }
  return FALSE;
}

