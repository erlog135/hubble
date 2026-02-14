/**
 * Pin pusher - main orchestrator for pushing astronomical events to Pebble timeline
 */

var timeline = require('../timeline');
var astronomyEvents = require('../astronomy/events');
var logger = require('../logger');

var cache = require('./cache');
var dateUtils = require('./dateUtils');
var pinBuilder = require('./pinBuilder');
var pinManager = require('./pinManager');
var testPins = require('./testPins');

/**
 * Push all astronomical events to the timeline based on observer location and settings
 * @param {Observer} observer - The observer location
 * @param {Date} date - The reference date (defaults to today)
 * @param {Object} settings - Clay settings object controlling which events to include
 * @returns {number} Number of pins pushed
 */
function pushAstronomyEvents(observer, date, settings) {
  logger.log('pushAstronomyEvents called with settings:', JSON.stringify(settings));

  // Check cache
  var cacheCheck = cache.shouldSkipPinPush(settings);
  if (cacheCheck.shouldSkip) {
    return 0;
  }

  // Check for disabled features and delete their pins
  var lastSettings = cache.getLastSettings();
  if (lastSettings) {
    var disabledPatterns = pinManager.getDisabledPinIdPatterns(lastSettings, settings);
    if (disabledPatterns.length > 0) {
      logger.log('Disabled patterns found:', disabledPatterns);
      var deletions = pinManager.deletePinsByPatterns(disabledPatterns);
      logger.log('Initiated deletion of', deletions, 'pins for disabled features');
    }
  }

  var pinCount = 0;
  var referenceDate = date || new Date();
  var allEvents = astronomyEvents.getAllEvents(observer, referenceDate, settings);

  // Process rise/set events
  pinCount += processRiseSetEvents(allEvents.riseSetEvents);

  // Process twilight events
  pinCount += processTwilightEvents(allEvents.twilightEvents);

  // Process solar noon/midnight events
  pinCount += processSolarNoonMidnightEvents(allEvents.solarNoonMidnightEvents);

  // Process seasonal events
  pinCount += processSeasonalEvents(allEvents.seasonalEvents);

  // Process transit events
  pinCount += processTransitEvents(allEvents.transitEvents);

  // Process eclipse events
  pinCount += processEclipseEvents(allEvents.eclipseEvents);

  // Process lunar apsis events
  pinCount += processLunarApsisEvents(allEvents.lunarApsisEvents);

  // Update cache after successful pin pushing
  cache.updatePinPushCache(settings);

  logger.log('Total pins pushed: ' + pinCount);
  return pinCount;
}

/**
 * Process and push rise/set events
 * @param {Array} events - Array of rise/set events
 * @returns {number} Number of pins pushed
 */
function processRiseSetEvents(events) {
  var pinCount = 0;
  var processedIds = {};

  for (var i = 0; i < events.length; i++) {
    var event = events[i];
    if (!event.time || !dateUtils.isDateVisibleInTimeline(event.time)) {
      continue;
    }

    var sequenceIndex = dateUtils.getSequenceIndexForDate(event.time);
    if (sequenceIndex === null || sequenceIndex < -2 || sequenceIndex > 2) {
      continue;
    }

    var pin = pinBuilder.buildRiseSetPin(event, sequenceIndex);

    // Skip if we've already processed this pin ID
    if (processedIds[pin.id]) {
      continue;
    }
    processedIds[pin.id] = true;

    pinCount++;
    timeline.insertUserPin(pin, function(responseText) {
      logger.log('Pushed ' + pin.layout.title + ' pin: ' + responseText);
    });
  }

  return pinCount;
}

/**
 * Process and push twilight events
 * @param {Array} events - Array of twilight events
 * @returns {number} Number of pins pushed
 */
function processTwilightEvents(events) {
  var pinCount = 0;
  var processedIds = {};

  for (var i = 0; i < events.length; i++) {
    var event = events[i];
    if (!event.time || !dateUtils.isDateVisibleInTimeline(event.time)) {
      continue;
    }

    var sequenceIndex = dateUtils.getSequenceIndexForDate(event.time);
    if (sequenceIndex === null || sequenceIndex < -2 || sequenceIndex > 2) {
      continue;
    }

    var pin = pinBuilder.buildTwilightPin(event, sequenceIndex);

    // Skip if we've already processed this pin ID
    if (processedIds[pin.id]) {
      continue;
    }
    processedIds[pin.id] = true;

    pinCount++;
    timeline.insertUserPin(pin, function(responseText) {
      logger.log('Pushed ' + pin.layout.title + ' pin: ' + responseText);
    });
  }

  return pinCount;
}

/**
 * Process and push solar noon/midnight events
 * @param {Array} events - Array of solar noon/midnight events
 * @returns {number} Number of pins pushed
 */
function processSolarNoonMidnightEvents(events) {
  var pinCount = 0;
  var processedIds = {};

  for (var i = 0; i < events.length; i++) {
    var event = events[i];
    if (!event.time || !dateUtils.isDateVisibleInTimeline(event.time)) {
      continue;
    }

    var sequenceIndex = dateUtils.getSequenceIndexForDate(event.time);
    if (sequenceIndex === null || sequenceIndex < -2 || sequenceIndex > 2) {
      continue;
    }

    var pin = pinBuilder.buildSolarNoonMidnightPin(event, sequenceIndex);

    // Skip if we've already processed this pin ID
    if (processedIds[pin.id]) {
      continue;
    }
    processedIds[pin.id] = true;

    pinCount++;
    timeline.insertUserPin(pin, function(responseText) {
      logger.log('Pushed ' + pin.layout.title + ' pin: ' + responseText);
    });
  }

  return pinCount;
}

/**
 * Process and push seasonal events
 * @param {Array} events - Array of seasonal events
 * @returns {number} Number of pins pushed
 */
function processSeasonalEvents(events) {
  var pinCount = 0;

  events.forEach(function(event) {
    if (!event.date || !dateUtils.isDateInTimelineRange(event.date)) {
      return;
    }

    var pin = pinBuilder.buildSeasonalPin(event);
    pinCount++;

    timeline.insertUserPin(pin, function(responseText) {
      logger.log('Pushed ' + pin.layout.title + ' pin: ' + responseText);
    });
  });

  return pinCount;
}

/**
 * Process and push transit events
 * @param {Array} events - Array of transit events
 * @returns {number} Number of pins pushed
 */
function processTransitEvents(events) {
  var pinCount = 0;

  events.forEach(function(event) {
    if (!event.start || !dateUtils.isDateInTimelineRange(event.start)) {
      return;
    }

    var pin = pinBuilder.buildTransitPin(event);
    pinCount++;

    timeline.insertUserPin(pin, function(responseText) {
      logger.log('Pushed ' + pin.layout.title + ' pin: ' + responseText);
    });
  });

  return pinCount;
}

/**
 * Process and push eclipse events
 * @param {Array} events - Array of eclipse events
 * @returns {number} Number of pins pushed
 */
function processEclipseEvents(events) {
  var pinCount = 0;

  events.forEach(function(event) {
    if (!event.peak || !dateUtils.isDateInTimelineRange(event.peak)) {
      return;
    }

    var pin = pinBuilder.buildEclipsePin(event);
    pinCount++;

    timeline.insertUserPin(pin, function(responseText) {
      logger.log('Pushed ' + pin.layout.title + ' pin: ' + responseText);
    });
  });

  return pinCount;
}

/**
 * Process and push lunar apsis events
 * @param {Array} events - Array of lunar apsis events
 * @returns {number} Number of pins pushed
 */
function processLunarApsisEvents(events) {
  var pinCount = 0;

  events.forEach(function(event) {
    if (!event.time || !dateUtils.isDateInTimelineRange(event.time)) {
      return;
    }

    var pin = pinBuilder.buildLunarApsisPin(event);
    pinCount++;

    timeline.insertUserPin(pin, function(responseText) {
      logger.log('Pushed ' + pin.layout.title + ' pin: ' + responseText);
    });
  });

  return pinCount;
}

// Export public API
module.exports = {
  pushAstronomyEvents: pushAstronomyEvents,
  pushTestPin: testPins.pushTestPin,
  deleteTestPin: testPins.deleteTestPin,
  // Expose sub-modules for advanced usage
  cache: cache,
  dateUtils: dateUtils,
  pinBuilder: pinBuilder,
  pinManager: pinManager,
  constants: require('./constants')
};
