/**
 * Date and timeline utility functions for Pebble timeline pins
 */

/**
 * Checks if a date is within the valid Pebble timeline range:
 *   - No more than 2 days in the past
 *   - No more than 1 year in the future
 * @param {Date} date - The date to check
 * @returns {boolean} True if date is within the valid timeline window
 */
function isDateInTimelineRange(date) {
  var now = new Date();
  var twoDaysAgo = new Date(now.getTime() - 2 * 24 * 60 * 60 * 1000);
  var oneYearLater = new Date(now.getTime() + 365 * 24 * 60 * 60 * 1000);
  return date >= twoDaysAgo && date <= oneYearLater;
}

/**
 * Checks if a date is within the visible timeline window:
 *   - No more than 2 days in the past
 *   - No more than 2 days in the future
 * @param {Date} date - The date to check
 * @returns {boolean} True if date is within the visible timeline window
 */
function isDateVisibleInTimeline(date) {
  var now = new Date();
  var twoDaysAgo = new Date(now.getTime() - 2 * 24 * 60 * 60 * 1000);
  var twoDaysLater = new Date(now.getTime() + 2 * 24 * 60 * 60 * 1000);
  return date >= twoDaysAgo && date <= twoDaysLater;
}

/**
 * Calculate the sequence index (-2 to 2) for a date relative to today
 * @param {Date} eventDate - The event date
 * @returns {number} Sequence index (-2, -1, 0, 1, 2) or null if outside range
 */
function getSequenceIndexForDate(eventDate) {
  var now = new Date();
  var eventDay = new Date(eventDate.getFullYear(), eventDate.getMonth(), eventDate.getDate());
  var today = new Date(now.getFullYear(), now.getMonth(), now.getDate());

  // Calculate difference in days
  var diffTime = eventDay.getTime() - today.getTime();
  var diffDays = Math.round(diffTime / (1000 * 60 * 60 * 24));

  // Return sequence index if within -2 to 2 range
  if (diffDays >= -2 && diffDays <= 2) {
    return diffDays;
  }
  return null; // Outside visible range
}

module.exports = {
  isDateInTimelineRange: isDateInTimelineRange,
  isDateVisibleInTimeline: isDateVisibleInTimeline,
  getSequenceIndexForDate: getSequenceIndexForDate
};
