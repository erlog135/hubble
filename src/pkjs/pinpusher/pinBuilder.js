/**
 * Pin construction functions for all event types
 */

var constants = require('./constants');
var PIN_IDS = constants.PIN_IDS;
var EVENT_STYLES = constants.EVENT_STYLES;

/**
 * Capitalize the first letter of a string
 * @param {string} str - The string to capitalize
 * @returns {string} String with first letter capitalized
 */
function capitalizeFirst(str) {
  return str.charAt(0).toUpperCase() + str.slice(1);
}

/**
 * Generate a properly formatted timestamp for Pebble timeline pins
 * Removes milliseconds to avoid display issues
 * @returns {string} ISO timestamp without milliseconds
 */
function generateTimestamp() {
  return new Date().toISOString().replace(/\.\d{3}Z$/, 'Z');
}

/**
 * Get the appropriate style key for a rise/set event
 * @param {Object} event - The event object
 * @returns {string} Style key for EVENT_STYLES lookup
 */
function getRiseSetStyleKey(event) {
  if (event.body === 'Sun') {
    return event.type === 'rise' ? 'sunRise' : 'sunSet';
  } else if (event.body === 'Moon') {
    return event.type === 'rise' ? 'moonRise' : 'moonSet';
  } else {
    return event.type === 'rise' ? 'bodyRise' : 'bodySet';
  }
}

/**
 * Format a seasonal event type into a readable title
 * @param {string} type - The seasonal event type
 * @returns {string} Formatted title
 */
function formatSeasonalTitle(type) {
  switch (type) {
    case 'marchEquinox': return 'March Equinox';
    case 'juneSolstice': return 'June Solstice';
    case 'septemberEquinox': return 'September Equinox';
    case 'decemberSolstice': return 'December Solstice';
    default: return type;
  }
}

/**
 * Generate a preset pin ID for an event
 * @param {Object} event - The event object
 * @param {string} category - The event category
 * @param {number} sequenceIndex - For recurring events, the sequence index (-2, -1, 0, 1, 2)
 * @returns {string} Pin ID
 */
function generateEventPinId(event, category, sequenceIndex) {
  if (category === 'riseset') {
    var body = event.body.toLowerCase();
    var type = event.type;
    var key = body + '_' + type;
    var baseId = PIN_IDS[key] || key;
    return sequenceIndex !== undefined ? baseId + sequenceIndex : baseId;
  } else if (category === 'twilight') {
    var subtype = event.subtype;
    var twilightType = event.type;
    var twilightKey = subtype + '_' + twilightType;
    var twilightBaseId = PIN_IDS[twilightKey] || twilightKey;
    return sequenceIndex !== undefined ? twilightBaseId + sequenceIndex : twilightBaseId;
  } else if (category === 'solarNoonMidnight') {
    var solarKey = 'solar_' + event.type;
    var solarBaseId = PIN_IDS[solarKey] || solarKey;
    return sequenceIndex !== undefined ? solarBaseId + sequenceIndex : solarBaseId;
  } else if (category === 'seasonal') {
    return event.type.includes('Equinox') ? PIN_IDS.equinox : PIN_IDS.solstice;
  } else if (category === 'transit') {
    return PIN_IDS.transit;
  } else if (category === 'eclipse') {
    return PIN_IDS.eclipse;
  } else if (category === 'apsis') {
    return PIN_IDS.moon_apsis;
  }
  return category;
}

/**
 * Get the body ID for timeline pin actions
 * @param {Object} event - The event object
 * @param {string} category - The event category
 * @returns {number} Body ID for the event
 */
function getBodyId(event, category) {
  var body;
  if (category === 'riseset' || category === 'transit') {
    body = event.body;
  } else if (category === 'twilight' || category === 'seasonal' || category === 'solarNoonMidnight') {
    body = 'Sun';
  } else if (category === 'eclipse') {
    body = event.type === 'solar' ? 'Sun' : 'Moon';
  } else if (category === 'apsis') {
    body = 'Moon';
  }

  // Map body names to IDs (must match BODY_NAMES in msgproc.js)
  var bodyIds = {
    'Moon': 0,
    'Mercury': 1,
    'Venus': 2,
    'Mars': 3,
    'Jupiter': 4,
    'Saturn': 5,
    'Uranus': 6,
    'Neptune': 7,
    'Pluto': 8,
    'Sun': 9
  };

  return bodyIds[body] || 0;
}

/**
 * Get the display name for a body
 * @param {Object} event - The event object
 * @param {string} category - The event category
 * @returns {string} Display name for the body
 */
function getBodyName(event, category) {
  var body;
  if (category === 'riseset' || category === 'transit') {
    body = event.body;
  } else if (category === 'twilight' || category === 'seasonal' || category === 'solarNoonMidnight') {
    body = 'Sun';
  } else if (category === 'eclipse') {
    body = event.type === 'solar' ? 'Sun' : 'Moon';
  } else if (category === 'apsis') {
    body = 'Moon';
  }
  return body || 'Unknown';
}

/**
 * Generate actions array for a timeline pin
 * @param {Object} event - The event object
 * @param {string} category - The event category
 * @returns {Array} Array of action objects
 */
function generatePinActions(event, category) {
  var bodyId = getBodyId(event, category);
  var bodyName = getBodyName(event, category);

  return [
    {
      "title": bodyName + " Details",
      "type": "openWatchApp",
      "launchCode": 200 + bodyId
    },
    {
      "title": "Open Hubble",
      "type": "openWatchApp",
      "launchCode": 100
    },
    {
      "title": "Refresh events",
      "type": "openWatchApp",
      "launchCode": 101
    }
  ];
}

/**
 * Build a rise/set event pin
 * @param {Object} event - The rise/set event
 * @param {number} sequenceIndex - Sequence index (-2 to 2)
 * @returns {Object} Complete pin object
 */
function buildRiseSetPin(event, sequenceIndex) {
  var styleKey = getRiseSetStyleKey(event);
  var style = EVENT_STYLES[styleKey];

  var title;

  if ((event.body === 'Sun' || event.body === 'Moon') && (event.type === 'rise' || event.type === 'set')) {
    title = event.body + event.type;
  } else {
    title = capitalizeFirst(event.body) + ' ' + (event.type === 'rise' ? 'Rises' : 'Sets');
  }

  var pinId = generateEventPinId(event, 'riseset', sequenceIndex);

  var pin = {
    id: pinId,
    time: event.time.toISOString(),
    layout: {
      type: "genericPin",
      primaryColor: style.foregroundColor,
      secondaryColor: style.foregroundColor,
      backgroundColor: style.backgroundColor,
      title: title,
      tinyIcon: "system://images/" + style.tinyIcon,
      lastUpdated: generateTimestamp()
    },
    actions: generatePinActions(event, 'riseset')
  };

  // Add moon phase info to subtitle if available
  if (event.moonPhase) {
    pin.layout.subtitle = event.moonPhase.name;
  }

  return pin;
}

/**
 * Build a twilight event pin
 * @param {Object} event - The twilight event
 * @param {number} sequenceIndex - Sequence index (-2 to 2)
 * @returns {Object} Complete pin object
 */
function buildTwilightPin(event, sequenceIndex) {
  var styleKey = event.subtype + capitalizeFirst(event.type);
  var style = EVENT_STYLES[styleKey];

  var title = capitalizeFirst(event.subtype) + ' ' + event.type;
  var pinId = generateEventPinId(event, 'twilight', sequenceIndex);

  return {
    id: pinId,
    time: event.time.toISOString(),
    layout: {
      type: "genericPin",
      primaryColor: style.foregroundColor,
      secondaryColor: style.foregroundColor,
      backgroundColor: style.backgroundColor,
      title: title,
      tinyIcon: "system://images/" + style.tinyIcon,
      lastUpdated: generateTimestamp()
    },
    actions: generatePinActions(event, 'twilight')
  };
}

/**
 * Build a seasonal event pin (equinox/solstice)
 * @param {Object} event - The seasonal event
 * @returns {Object} Complete pin object
 */
function buildSeasonalPin(event) {
  var isEquinox = event.type.includes('Equinox');
  var styleKey = isEquinox ? 'equinox' : 'solstice';
  var style = EVENT_STYLES[styleKey];

  var title = formatSeasonalTitle(event.type);
  var subtitle = event.year.toString();
  var pinId = generateEventPinId(event, 'seasonal');

  return {
    id: pinId,
    time: event.date.toISOString(),
    layout: {
      type: "genericPin",
      primaryColor: style.foregroundColor,
      secondaryColor: style.foregroundColor,
      backgroundColor: style.backgroundColor,
      title: title,
      subtitle: subtitle,
      tinyIcon: "system://images/" + style.tinyIcon,
      lastUpdated: generateTimestamp()
    },
    actions: generatePinActions(event, 'seasonal')
  };
}

/**
 * Build a transit event pin
 * @param {Object} event - The transit event
 * @returns {Object} Complete pin object
 */
function buildTransitPin(event) {
  var style = EVENT_STYLES.transit;

  var title = event.body + ' Transit';
  var subtitle = 'Begins';
  var pinId = generateEventPinId(event, 'transit');

  return {
    id: pinId,
    time: event.start.toISOString(),
    layout: {
      type: "genericPin",
      primaryColor: style.foregroundColor,
      secondaryColor: style.foregroundColor,
      backgroundColor: style.backgroundColor,
      title: title,
      subtitle: subtitle,
      tinyIcon: "system://images/" + style.tinyIcon,
      lastUpdated: generateTimestamp()
    },
    actions: generatePinActions(event, 'transit')
  };
}

/**
 * Build an eclipse event pin
 * @param {Object} event - The eclipse event
 * @returns {Object} Complete pin object
 */
function buildEclipsePin(event) {
  var style = EVENT_STYLES.eclipse;

  var title = capitalizeFirst(event.kind) + ' ' + event.type + ' Eclipse';
  var pinId = generateEventPinId(event, 'eclipse');

  return {
    id: pinId,
    time: event.peak.toISOString(),
    layout: {
      type: "genericPin",
      primaryColor: style.foregroundColor,
      secondaryColor: style.foregroundColor,
      backgroundColor: style.backgroundColor,
      title: title,
      tinyIcon: "system://images/" + style.tinyIcon,
      lastUpdated: generateTimestamp()
    },
    actions: generatePinActions(event, 'eclipse')
  };
}

/**
 * Build a lunar apsis event pin
 * @param {Object} event - The lunar apsis event
 * @returns {Object} Complete pin object
 */
function buildLunarApsisPin(event) {
  var style = EVENT_STYLES.lunarApsis;

  var title = 'Lunar ' + capitalizeFirst(event.kind);
  var subtitle = event.distance.toLocaleString('en-US', { maximumFractionDigits: 0 }) + ' km';
  var pinId = generateEventPinId(event, 'apsis');

  return {
    id: pinId,
    time: event.time.toISOString(),
    layout: {
      type: "genericPin",
      primaryColor: style.foregroundColor,
      secondaryColor: style.foregroundColor,
      backgroundColor: style.backgroundColor,
      title: title,
      subtitle: subtitle,
      tinyIcon: "system://images/" + style.tinyIcon,
      lastUpdated: generateTimestamp()
    },
    actions: generatePinActions(event, 'apsis')
  };
}

/**
 * Build a solar noon/midnight event pin
 * @param {Object} event - The solar noon/midnight event
 * @param {number} sequenceIndex - Sequence index (-2 to 2)
 * @returns {Object} Complete pin object
 */
function buildSolarNoonMidnightPin(event, sequenceIndex) {
  var styleKey = event.type === 'noon' ? 'solarNoon' : 'solarMidnight';
  var style = EVENT_STYLES[styleKey];

  var title = event.type === 'noon' ? 'Solar Noon' : 'Solar Midnight';
  var pinId = generateEventPinId(event, 'solarNoonMidnight', sequenceIndex);

  return {
    id: pinId,
    time: event.time.toISOString(),
    layout: {
      type: "genericPin",
      primaryColor: style.foregroundColor,
      secondaryColor: style.foregroundColor,
      backgroundColor: style.backgroundColor,
      title: title,
      tinyIcon: "system://images/" + style.tinyIcon,
      lastUpdated: generateTimestamp()
    },
    actions: generatePinActions(event, 'solarNoonMidnight')
  };
}

module.exports = {
  buildRiseSetPin: buildRiseSetPin,
  buildTwilightPin: buildTwilightPin,
  buildSolarNoonMidnightPin: buildSolarNoonMidnightPin,
  buildSeasonalPin: buildSeasonalPin,
  buildTransitPin: buildTransitPin,
  buildEclipsePin: buildEclipsePin,
  buildLunarApsisPin: buildLunarApsisPin,
  generateEventPinId: generateEventPinId,
  capitalizeFirst: capitalizeFirst,
  generateTimestamp: generateTimestamp
};
