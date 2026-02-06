/**
 * Pin ID and style constants for Pebble timeline pins
 */

/**
 * Get all possible pin IDs that could exist
 * @returns {Array<string>} Array of all possible pin IDs
 */
function getAllPossiblePinIds() {
  var pinIds = [];

  // Add recurring event IDs (rise/set, twilight, and solar noon/midnight) with sequence numbers
  var recurringBases = [
    // Rise/set events
    'sun-rise', 'sun-set', 'moon-rise', 'moon-set',
    'mercury-rise', 'mercury-set', 'venus-rise', 'venus-set',
    'mars-rise', 'mars-set', 'jupiter-rise', 'jupiter-set',
    'saturn-rise', 'saturn-set', 'uranus-rise', 'uranus-set',
    'neptune-rise', 'neptune-set', 'pluto-rise', 'pluto-set',
    // Twilight events
    'civil-dawn', 'civil-dusk', 'nautical-dawn', 'nautical-dusk',
    'astronomical-dawn', 'astronomical-dusk',
    // Solar noon/midnight events
    'solar-noon', 'solar-midnight'
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

// Preset pin IDs for different event types
var PIN_IDS = {
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

  // Solar noon/midnight events
  solar_noon: "solar-noon",
  solar_midnight: "solar-midnight",

  // Seasonal events (simplified - events occur far enough apart)
  equinox: "equinox",
  solstice: "solstice",

  // Rare events (simplified - events occur far enough apart)
  transit: "planetary-transit",
  eclipse: "eclipse",
  moon_apsis: "moon-apsis"
};

//TODO: replace with my own icons once published media is fixed
var EVENT_STYLES = {
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
    backgroundColor: "#555500",
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
    backgroundColor: "#AA00AA",
    tinyIcon: "GENERIC_CONFIRMATION"
  },
  astronomicalDusk: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA00AA",
    tinyIcon: "GENERIC_CONFIRMATION"
  },
  nauticalDawn: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA00FF",
    tinyIcon: "NOTIFICATION_LIGHTHOUSE"
  },
  nauticalDusk: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA00FF",
    tinyIcon: "NOTIFICATION_LIGHTHOUSE"
  },
  civilDawn: {
    foregroundColor: "#000000",
    backgroundColor: "#AA55FF",
    tinyIcon: "NOTIFICATION_GENERIC"
  },
  civilDusk: {
    foregroundColor: "#000000",
    backgroundColor: "#AA55FF",
    tinyIcon: "NOTIFICATION_GENERIC"
  },
  solarNoon: {
    foregroundColor: "#000000",
    backgroundColor: "#FFFF55",
    tinyIcon: "TIMELINE_SUN"
  },
  solarMidnight: {
    foregroundColor: "#000000",
    backgroundColor: "#AAAA55",
    tinyIcon: "TIMELINE_SUN"
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
};

module.exports = {
  PIN_IDS: PIN_IDS,
  EVENT_STYLES: EVENT_STYLES,
  getAllPossiblePinIds: getAllPossiblePinIds
};
