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
  var today = new Date(now.getFullYear(), now.getMonth(), now.getDate());
  var eventDay = new Date(date.getFullYear(), date.getMonth(), date.getDate());
  var msPerDay = 1000 * 60 * 60 * 24;
  var dayDifference = Math.floor((eventDay - today) / msPerDay);
  return dayDifference >= -2 && dayDifference <= 2;
}

/**
 * Calculate the day difference between today and the event date.
 * @param {Date} eventDate - The event date
 * @returns {number} Difference in days (positive if in future, negative if in past)
 */
function getSequenceIndexForDate(eventDate) {
  var now = new Date();
  var today = new Date(now.getFullYear(), now.getMonth(), now.getDate());
  var eventDay = new Date(eventDate.getFullYear(), eventDate.getMonth(), eventDate.getDate());
  var msPerDay = 1000 * 60 * 60 * 24;
  return Math.floor((eventDay - today) / msPerDay);
}

module.exports = {
  isDateInTimelineRange: isDateInTimelineRange,
  isDateVisibleInTimeline: isDateVisibleInTimeline,
  getSequenceIndexForDate: getSequenceIndexForDate
};
