
/**
 * Astronomy events to be shown both in the timeline and in the "upcoming" page in the app.
 * 
 * These will have to be smartly displayed/added.
 * Per the documentation, timeline events times must be within 2 days in the past and a year in the future.
 */

/**
 * Checks if a date is within the valid Pebble timeline range:
 *   - No more than 2 days in the past
 *   - No more than 1 year (366 days for leap year) in the future
 * @param {Date} date - The date to check
 * @returns {boolean} True if date is within the valid timeline window
 */
function isDateInTimelineRange(date) {
  var now = new Date();
  var twoDaysAgo = new Date(now.getTime() - 2 * 24 * 60 * 60 * 1000);
  var oneYearLater = new Date(now.getTime() + 366 * 24 * 60 * 60 * 1000);
  return date >= twoDaysAgo && date <= oneYearLater;
}

module.exports.isDateInTimelineRange = isDateInTimelineRange;
