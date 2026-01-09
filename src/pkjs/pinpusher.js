var timeline = require('./timeline');
var astronomyEvents = require('./astronomy/events');


/**
 * Get all possible pin IDs that could exist
 * @returns {Array<string>} Array of all possible pin IDs
 */
function getAllPossiblePinIds() {
  var pinIds = [];

  // Add recurring event IDs (rise/set and twilight) with sequence numbers
  var recurringBases = [
    // Rise/set events
    'sun-rise', 'sun-set', 'moon-rise', 'moon-set',
    'mercury-rise', 'mercury-set', 'venus-rise', 'venus-set',
    'mars-rise', 'mars-set', 'jupiter-rise', 'jupiter-set',
    'saturn-rise', 'saturn-set', 'uranus-rise', 'uranus-set',
    'neptune-rise', 'neptune-set', 'pluto-rise', 'pluto-set',
    // Twilight events
    'civil-dawn', 'civil-dusk', 'nautical-dawn', 'nautical-dusk',
    'astronomical-dawn', 'astronomical-dusk'
  ];

  // Add sequence numbers for recurring events
  recurringBases.forEach(function(base) {
    for (var seq = -2; seq <= 2; seq++) {
      pinIds.push(base + seq);
    }
  });

  // Add one-time event IDs
  var oneTimeIds = [
    'equinox', 'solstice',
    'planetary-transit', 'eclipse',
    'moon-apsis'
  ];

  oneTimeIds.forEach(function(id) {
    pinIds.push(id);
  });

  return pinIds;
}

// Cache for pin pushing to avoid unnecessary recalculations
var lastPinPushTime = 0;
var lastPinPushSettingsHash = null;
var PIN_PUSH_CACHE_DURATION_MS = 30 * 60 * 1000; // 30 minutes

// Preset pin IDs for different event types
const PIN_IDS = {
  // Rise/Set events
  sun_rise: "sun-rise",
  sun_set: "sun-set",
  moon_rise: "moon-rise",
  moon_set: "moon-set",
  mercury_rise: "mercury-rise",
  mercury_set: "mercury-set",
  venus_rise: "venus-rise",
  venus_set: "venus-set",
  mars_rise: "mars-rise",
  mars_set: "mars-set",
  jupiter_rise: "jupiter-rise",
  jupiter_set: "jupiter-set",
  saturn_rise: "saturn-rise",
  saturn_set: "saturn-set",
  uranus_rise: "uranus-rise",
  uranus_set: "uranus-set",
  neptune_rise: "neptune-rise",
  neptune_set: "neptune-set",
  pluto_rise: "pluto-rise",
  pluto_set: "pluto-set",

  // Twilight events
  civil_dawn: "civil-dawn",
  civil_dusk: "civil-dusk",
  nautical_dawn: "nautical-dawn",
  nautical_dusk: "nautical-dusk",
  astronomical_dawn: "astronomical-dawn",
  astronomical_dusk: "astronomical-dusk",

  // Seasonal events (simplified - events occur far enough apart)
  equinox: "equinox",
  solstice: "solstice",

  // Rare events (simplified - events occur far enough apart)
  transit: "planetary-transit",
  eclipse: "eclipse",
  moon_apsis: "moon-apsis"
};

//TODO: replace with my own icons once published media is fixed
const EVENT_STYLES = {
  sunRise: {
    foregroundColor: "#000000",
    backgroundColor: "#FFFF00",
    tinyIcon: "SUNRISE"
  },
  sunSet: {
    foregroundColor: "#000000",
    backgroundColor: "#FFAA00",
    tinyIcon: "SUNSET"
  },
  bodyRise: {
    foregroundColor: "#000000",
    backgroundColor: "#AAAA55",
    tinyIcon: "NOTIFICATION_FLAG"
  },
  bodySet: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#550000",
    tinyIcon: "NOTIFICATION_FLAG"
  },
  moonRise: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA55FF",
    tinyIcon: "NOTIFICATION_FLAG"
  },
  moonSet: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA00FF",
    tinyIcon: "NOTIFICATION_FLAG"
  },
  astronomicalDawn: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA0055",
    tinyIcon: "GENERIC_CONFIRMATION"
  },
  astronomicalDusk: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA0055",
    tinyIcon: "GENERIC_CONFIRMATION"
  },
  nauticalDawn: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#FF0055",
    tinyIcon: "NOTIFICATION_LIGHTHOUSE"
  },
  nauticalDusk: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#FF0055",
    tinyIcon: "NOTIFICATION_LIGHTHOUSE"
  },
  civilDawn: {
    foregroundColor: "#000000",
    backgroundColor: "#FF5555",
    tinyIcon: "NOTIFICATION_GENERIC"
  },
  civilDusk: {
    foregroundColor: "#000000",
    backgroundColor: "#FF5555",
    tinyIcon: "NOTIFICATION_GENERIC"
  },
  equinox: {
    foregroundColor: "#000000",
    backgroundColor: "#55FFAA",
    tinyIcon: "TIMELINE_SUN"
  },
  solstice: {
    foregroundColor: "#000000",
    backgroundColor: "#00FFFF",
    tinyIcon: "TIMELINE_SUN"
  },
  eclipse: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#5555FF",
    tinyIcon: "TIMELINE_SUN"
  },
  transit: {
    foregroundColor: "#000000",
    backgroundColor: "#FFFF00",
    tinyIcon: "TIMELINE_SUN"
  },
  lunarApsis: {
    foregroundColor: "#000000",
    backgroundColor: "#5555FF",
    tinyIcon: "NOTIFICATION_FLAG"
  }
}

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

/**
 * Generate a hash string from clay settings for cache comparison
 * @param {Object} settings - Clay settings object
 * @returns {string} Hash string representing the settings
 */
function generateSettingsHash(settings) {
  if (!settings) return '';

  // All clay settings are astronomy settings (start with CFG_), so use them all
  // Create a sorted array of key-value pairs and stringify
  var keys = Object.keys(settings).sort();
  var hashParts = keys.map(function(key) {
    var value = settings[key];
    if (Array.isArray(value)) {
      return key + ':' + value.join(',');
    }
    return key + ':' + String(value);
  });

  return hashParts.join('|');
}

/**
 * Check if pin pushing should be skipped based on cache
 * @param {Object} settings - Current clay settings
 * @returns {boolean} True if we should skip pin pushing
 */
function shouldSkipPinPush(settings) {
  var now = new Date().getTime();
  var currentSettingsHash = generateSettingsHash(settings);

  console.log('Cache check - Current hash:', currentSettingsHash);
  console.log('Cache check - Last hash:', lastPinPushSettingsHash);
  console.log('Cache check - Time since last push:', (now - lastPinPushTime) / 1000, 'seconds');
  console.log('Cache check - Cache duration:', PIN_PUSH_CACHE_DURATION_MS / 1000, 'seconds');

  // Check if settings haven't changed and cache is still valid
  if (currentSettingsHash === lastPinPushSettingsHash &&
      (now - lastPinPushTime) < PIN_PUSH_CACHE_DURATION_MS) {
    console.log('Skipping pin push - cache still valid (settings unchanged, < 30min old)');
    return true;
  }

  console.log('Proceeding with pin push - cache expired or settings changed');
  return false;
}

/**
 * Update the pin push cache after successful calculation
 * @param {Object} settings - Clay settings used for calculation
 */
function updatePinPushCache(settings) {
  lastPinPushTime = new Date().getTime();
  lastPinPushSettingsHash = generateSettingsHash(settings);
  console.log('Updated pin push cache - new hash:', lastPinPushSettingsHash);
}

/**
 * Push all astronomical events to the timeline based on observer location and settings
 * @param {Observer} observer - The observer location
 * @param {Date} date - The reference date (defaults to today)
 * @param {Object} settings - Clay settings object controlling which events to include
 */
function pushAstronomyEvents(observer, date, settings) {
  console.log('pushAstronomyEvents called with settings:', JSON.stringify(settings));

  // Check cache first - skip if settings haven't changed and < 30min ago
  if (shouldSkipPinPush(settings)) {
    return 0;
  }

  var pinCount = 0;

  var referenceDate = date || new Date();
  var allEvents = astronomyEvents.getAllEvents(observer, referenceDate, settings);

  // Process rise/set events (sequence: -2, -1, 0, 1, 2)
  var processedRiseSetIds = {};
  for (var i = 0; i < allEvents.riseSetEvents.length; i++) {
    var event = allEvents.riseSetEvents[i];
    if (event.time && isDateVisibleInTimeline(event.time)) {
      var sequenceIndex = getSequenceIndexForDate(event.time);
      if (sequenceIndex >= -2 && sequenceIndex <= 2) {
        var styleKey = getRiseSetStyleKey(event);
        var style = EVENT_STYLES[styleKey];

      var title;
      if (event.body === 'Sun' && event.type === 'rise') {
        title = 'Sunrise';
      } else if (event.body === 'Sun' && event.type === 'set') {
        title = 'Sunset';
      } else if (event.body === 'Moon' && event.type === 'rise') {
        title = 'Moonrise';
      } else if (event.body === 'Moon' && event.type === 'set') {
        title = 'Moonset';
      } else {
        title = capitalizeFirst(event.body) + ' ' + (event.type === 'rise' ? 'Rises' : 'Sets');
      }



      var pinId = generateEventPinId(event, 'riseset', sequenceIndex);

      // Skip if we've already processed this pin ID
      if (processedRiseSetIds[pinId]) {
        continue;
      }
      processedRiseSetIds[pinId] = true;

      pinCount++;

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

      timeline.insertUserPin(pin, function(responseText) {
        console.log('Pushed ' + title + ' pin: ' + responseText);
      });
      }
    }
  }

  // Process twilight events (sequence: -2, -1, 0, 1, 2)
  var processedTwilightIds = {};
  for (var i = 0; i < allEvents.twilightEvents.length; i++) {
    var event = allEvents.twilightEvents[i];
    if (event.time && isDateVisibleInTimeline(event.time)) {
      var sequenceIndex = getSequenceIndexForDate(event.time);
      if (sequenceIndex >= -2 && sequenceIndex <= 2) {
        var styleKey = event.subtype + capitalizeFirst(event.type);
        var style = EVENT_STYLES[styleKey];

        var title = capitalizeFirst(event.subtype) + ' ' + event.type;

        var pinId = generateEventPinId(event, 'twilight', sequenceIndex);

        // Skip if we've already processed this pin ID
        if (processedTwilightIds[pinId]) {
          continue;
        }
        processedTwilightIds[pinId] = true;

        pinCount++;

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
          actions: generatePinActions(event, 'twilight')
        };

        timeline.insertUserPin(pin, function(responseText) {
          console.log('Pushed ' + title + ' pin: ' + responseText);
        });
      }
    }
  }

  // Process seasonal events
  allEvents.seasonalEvents.forEach(function(event) {
    if (event.date && isDateInTimelineRange(event.date)) {
      var isEquinox = event.type.includes('Equinox');
      var styleKey = isEquinox ? 'equinox' : 'solstice';
      var style = EVENT_STYLES[styleKey];

      var title = formatSeasonalTitle(event.type);
      var subtitle = event.year.toString();

      var pinId = generateEventPinId(event, 'seasonal');

      pinCount++;

      var pin = {
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

      timeline.insertUserPin(pin, function(responseText) {
        console.log('Pushed ' + title + ' pin: ' + responseText);
      });
    }
  });

  // Process transit events
  allEvents.transitEvents.forEach(function(event) {
    if (event.start && isDateInTimelineRange(event.start)) {
      var style = EVENT_STYLES.transit;

      var title = event.body + ' Transit';
      var subtitle = 'Begins';

      var pinId = generateEventPinId(event, 'transit');

      pinCount++;

      var pin = {
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

      timeline.insertUserPin(pin, function(responseText) {
        console.log('Pushed ' + title + ' pin: ' + responseText);
      });
    }
  });

  // Process eclipse events
  allEvents.eclipseEvents.forEach(function(event) {
    if (event.peak && isDateInTimelineRange(event.peak)) {
      var style = EVENT_STYLES.eclipse;

      var title = capitalizeFirst(event.kind) + ' ' + event.type + ' Eclipse';

      var pinId = generateEventPinId(event, 'eclipse');

      pinCount++;

      var pin = {
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

      timeline.insertUserPin(pin, function(responseText) {
        console.log('Pushed ' + title + ' pin: ' + responseText);
      });
    }
  });

  // Process lunar apsis events
  allEvents.lunarApsisEvents.forEach(function(event) {
    if (event.time && isDateInTimelineRange(event.time)) {
      var style = EVENT_STYLES.lunarApsis;

      var title = 'Moon ' + capitalizeFirst(event.kind);
      var subtitle = event.distance.toFixed(2) + ' AU';

      var pinId = generateEventPinId(event, 'apsis');

      pinCount++;

      var pin = {
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

      timeline.insertUserPin(pin, function(responseText) {
        console.log('Pushed ' + title + ' pin: ' + responseText);
      });
    }
  });

    // Update cache after successful pin pushing
    updatePinPushCache(settings);

    console.log('Total pins pushed: ' + pinCount);
    return pinCount;
}

/**
 * Get the appropriate style key for a rise/set event
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
 */
function generateEventPinId(event, category, sequenceIndex) {
  // Map event properties to preset pin IDs
  if (category === 'riseset') {
    var body = event.body.toLowerCase();
    var type = event.type; // 'rise' or 'set'
    var key = body + '_' + type;
    var baseId = PIN_IDS[key] || key;
    // Add sequence index for recurring events (-2, -1, 0, 1, 2)
    return sequenceIndex !== undefined ? baseId + sequenceIndex : baseId;
  } else if (category === 'twilight') {
    var subtype = event.subtype; // 'civil', 'nautical', 'astronomical'
    var type = event.type; // 'dawn' or 'dusk'
    var key = subtype + '_' + type;
    var baseId = PIN_IDS[key] || key;
    // Add sequence index for recurring events (-2, -1, 0, 1, 2)
    return sequenceIndex !== undefined ? baseId + sequenceIndex : baseId;
  } else if (category === 'seasonal') {
    // Use simplified IDs since seasonal events occur far enough apart
    return event.type.includes('Equinox') ? PIN_IDS.equinox : PIN_IDS.solstice;
  } else if (category === 'transit') {
    return PIN_IDS.transit;
  } else if (category === 'eclipse') {
    // Use simplified ID since eclipses occur far enough apart
    return PIN_IDS.eclipse;
  } else if (category === 'apsis') {
    // Use simplified ID since apsis events occur far enough apart
    return PIN_IDS.moon_apsis;
  }

  // Fallback
  return category;
}

/**
 * Format a Date object to a readable time string
 */
function formatTime(date) {
  return date.toLocaleTimeString([], {hour: '2-digit', minute: '2-digit'});
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
  } else if (category === 'twilight' || category === 'seasonal') {
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
 * Get the display name for a body
 * @param {Object} event - The event object
 * @param {string} category - The event category
 * @returns {string} Display name for the body
 */
function getBodyName(event, category) {
  var body;
  if (category === 'riseset' || category === 'transit') {
    body = event.body;
  } else if (category === 'twilight' || category === 'seasonal') {
    body = 'Sun';
  } else if (category === 'eclipse') {
    body = event.type === 'solar' ? 'Sun' : 'Moon';
  } else if (category === 'apsis') {
    body = 'Moon';
  }

  return body || 'Unknown';
}

/**
 * Capitalize the first letter of a string
 */
function capitalizeFirst(str) {
  return str.charAt(0).toUpperCase() + str.slice(1);
}

/**
 * Generate a properly formatted timestamp for Pebble timeline pins
 * Removes milliseconds to avoid display issues
 */
function generateTimestamp() {
  return new Date().toISOString().replace(/\.\d{3}Z$/, 'Z');
}

// Push a pin when the app starts
function pushTestPin() {
    // An hour ahead
    var date = new Date();
    date.setHours(date.getHours() + 1);
  
    // Create the pin
    var pin = {
      "id": "00-00-test",
      "time": date.toISOString(),
    //   "duration": 32,
     "layout": {
        "type": "genericPin",
        "backgroundColor": "#000000",
        "title": "Test Pin",
        "subtitle": "For Testing",
      "body": "Here is a test pin with random numbers: " + Math.random().toString(),
      "tinyIcon": "system://images/NOTIFICATION_FLAG",
      "lastUpdated": Date.now()
    },
    "actions": [
      {
        "title": "Open App",
        "type": "openWatchApp",
        "launchCode": 10
      }
    ]
    };
  
    console.log("ISO: " + date.toISOString());
    console.log("timestamp: " + generateTimestamp());

    // Push the pin
    timeline.insertUserPin(pin, function(responseText) {
      console.log('Test pin pushed: ' + responseText);
    });
  }

function deleteTestPin() {
  timeline.deleteUserPin({
    "id": "00-00-test"
  }, function(responseText) {
    console.log('Test pin deleted: ' + responseText);
  });
}

module.exports = {
  pushTestPin: pushTestPin,
  deleteTestPin: deleteTestPin,
  pushAstronomyEvents: pushAstronomyEvents,
};