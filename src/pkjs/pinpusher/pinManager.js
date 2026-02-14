/**
 * Pin management - settings comparison and pin deletion
 */

var timeline = require('../timeline');
var constants = require('./constants');
var logger = require('../logger');

/**
 * Compare old and new settings to identify which features have been disabled
 * @param {Object} oldSettings - Previous clay settings
 * @param {Object} newSettings - Current clay settings
 * @returns {Array<string>} Array of pin ID patterns that should be deleted
 */
function getDisabledPinIdPatterns(oldSettings, newSettings) {
  var disabledPatterns = [];

  // Helper to check if a setting was disabled
  function wasDisabled(settingKey) {
    var oldValue = oldSettings ? oldSettings[settingKey] : true;
    var newValue = newSettings ? newSettings[settingKey] : true;
    return oldValue === true && newValue === false;
  }

  // Helper to check if a planet setting was disabled
  function wasPlanetDisabled(index) {
    var oldPlanets = oldSettings && oldSettings.CFG_PLANET_EVENTS ? oldSettings.CFG_PLANET_EVENTS : [false, false, false, false, false, false, false, false];
    var newPlanets = newSettings && newSettings.CFG_PLANET_EVENTS ? newSettings.CFG_PLANET_EVENTS : [false, false, false, false, false, false, false, false];
    return oldPlanets[index] === true && newPlanets[index] === false;
  }

  var planetNames = ['mercury', 'venus', 'mars', 'jupiter', 'saturn', 'uranus', 'neptune', 'pluto'];

  // Check each setting and map to pin ID patterns
  if (wasDisabled('CFG_SUN_RISE_SET')) {
    disabledPatterns.push('sun-rise', 'sun-set');
  }
  if (wasDisabled('CFG_SUN_CIVIL_DAWN_DUSK')) {
    disabledPatterns.push('civil-dawn', 'civil-dusk');
  }
  if (wasDisabled('CFG_SUN_NAUTICAL_DAWN_DUSK')) {
    disabledPatterns.push('nautical-dawn', 'nautical-dusk');
  }
  if (wasDisabled('CFG_SUN_ASTRONOMICAL_DAWN_DUSK')) {
    disabledPatterns.push('astronomical-dawn', 'astronomical-dusk');
  }
  if (wasDisabled('CFG_SUN_SOLAR_NOON_MIDNIGHT')) {
    disabledPatterns.push('solar-noon', 'solar-midnight');
  }
  if (wasDisabled('CFG_SUN_SOLSTICES')) {
    disabledPatterns.push('solstice');
  }
  if (wasDisabled('CFG_SUN_EQUINOXES')) {
    disabledPatterns.push('equinox');
  }
  if (wasDisabled('CFG_SUN_ECLIPSES')) {
    disabledPatterns.push('eclipse');
  }
  if (wasDisabled('CFG_SUN_SOLAR_TRANSITS')) {
    disabledPatterns.push('planetary-transit');
  }
  if (wasDisabled('CFG_MOON_RISE_SET')) {
    disabledPatterns.push('moon-rise', 'moon-set');
  }
  if (wasDisabled('CFG_MOON_APOGEE_PERIGEE')) {
    disabledPatterns.push('moon-apsis');
  }

  // Check planet settings
  for (var i = 0; i < planetNames.length; i++) {
    if (wasPlanetDisabled(i)) {
      disabledPatterns.push(planetNames[i] + '-rise', planetNames[i] + '-set');
    }
  }

  return disabledPatterns;
}

/**
 * Delete pins that match the given ID patterns
 * @param {Array<string>} patterns - Array of pin ID patterns to delete
 * @returns {number} Number of deletion operations initiated
 */
function deletePinsByPatterns(patterns) {
  if (!patterns || patterns.length === 0) {
    return 0;
  }

  logger.log('Deleting pins for disabled patterns:', patterns);

  var deletions = 0;

  // Get all possible pin IDs and filter by patterns
  var allPinIds = constants.getAllPossiblePinIds();
  var pinsToDelete = [];

  patterns.forEach(function(pattern) {
    allPinIds.forEach(function(pinId) {
      if (pinId.indexOf(pattern) === 0) {
        pinsToDelete.push(pinId);
      }
    });
  });

  // Remove duplicates
  pinsToDelete = pinsToDelete.filter(function(item, pos) {
    return pinsToDelete.indexOf(item) === pos;
  });

  logger.log('Found', pinsToDelete.length, 'pins to delete');

  // Delete each pin
  pinsToDelete.forEach(function(pinId) {
    timeline.deleteUserPin({ id: pinId }, function(responseText) {
      logger.log('Deleted pin:', pinId, responseText);
    });
    deletions++;
  });

  return deletions;
}

module.exports = {
  getDisabledPinIdPatterns: getDisabledPinIdPatterns,
  deletePinsByPatterns: deletePinsByPatterns
};
