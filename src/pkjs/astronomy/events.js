
/**
 * Astronomy events to be shown both in the timeline and in the "upcoming" page in the app.
 *
 * These will have to be smartly displayed/added.
 * Per the documentation, timeline events times must be within 2 days in the past and a year in the future.
 */

var Astronomy = require('astronomy-engine');
var logger = require('../logger');

// Cache for events with timestamp and observer info
// Cache key format: "lat_lon_date" where lat/lon rounded to 2 decimal places, date to day
var eventsCache = {};
var CACHE_DURATION_MS = 30 * 60 * 1000; // 30 minutes
var LOCATION_THRESHOLD_DEGREES = 0.01; // ~1km threshold for significant movement

function resolveBody(body) {
  if (!body) {
    throw new Error('Body is required');
  }
  return body;
}

var phaseNames = [
  "New Moon",
  "Waxing Crescent",
  "First Quarter",
  "Waxing Gibbous",
  "Full Moon",
  "Waning Gibbous",
  "Third Quarter",
  "Waning Crescent"
];

//TODO: do all phases span the same angle range? fix if they don't
function getMoonPhase(date) {
  var when = date || new Date();
  var angle = Astronomy.MoonPhase(when); // 0=new, 180=full
  return Math.floor(((angle + 22.5) % 360) / 45); // 0-7
}

function getMoonPhaseName(date) {
  var phaseIndex = getMoonPhase(date);
  return phaseNames[phaseIndex];
}




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

/**
 * Get rise and set times for a body around a given date.
 * Returns previous 2, current day's, and next 2 rise/set times.
 * If body is 'Moon', includes moon phase information for each rise time.
 * @param {string} body - The astronomical body (e.g., 'Sun', 'Moon', 'Venus')
 * @param {Observer} observer - The observer location
 * @param {Date} date - The reference date (typically today's date)
 * @returns {Object} Object with rise and set arrays containing Date objects
 *                   Basic: {rise:[prev-1, prev, today, next, next+1], set:[prev-1, prev, today, next, next+1]}
 *                   Moon: includes {moonPhases: [{index: number, name: string}, ...]} for each rise time
 */
function getRiseSetSequence(body, observer, date) {
  var referenceDate = date || new Date();
  var resolvedBody = resolveBody(body);

  // Helper function to find the next event in a direction
  function findNextEvent(direction, startDate) {
    return Astronomy.SearchRiseSet(resolvedBody, observer, direction, startDate, 30);
  }

  // Helper function to find the previous event by searching backwards
  function findPreviousEvent(direction, startDate) {
    // Use negative limitDays to search backwards from startDate
    return Astronomy.SearchRiseSet(resolvedBody, observer, direction, startDate, -30);
  }

  // Get rise events
  var riseToday = findNextEvent(+1, referenceDate);
  var risePrev = riseToday ? findPreviousEvent(+1, riseToday.date) : null;
  var risePrevPrev = risePrev ? findPreviousEvent(+1, risePrev.date) : null;
  var riseNext = riseToday ? findNextEvent(+1, new Date(riseToday.date.getTime() + 60000)) : null;
  var riseNextNext = riseNext ? findNextEvent(+1, new Date(riseNext.date.getTime() + 60000)) : null;

  var riseSequence = [
    risePrevPrev ? risePrevPrev.date : null,
    risePrev ? risePrev.date : null,
    riseToday ? riseToday.date : null,
    riseNext ? riseNext.date : null,
    riseNextNext ? riseNextNext.date : null
  ];

  // Get set events
  var setToday = findNextEvent(-1, referenceDate);
  var setPrev = setToday ? findPreviousEvent(-1, setToday.date) : null;
  var setPrevPrev = setPrev ? findPreviousEvent(-1, setPrev.date) : null;
  var setNext = setToday ? findNextEvent(-1, new Date(setToday.date.getTime() + 60000)) : null;
  var setNextNext = setNext ? findNextEvent(-1, new Date(setNext.date.getTime() + 60000)) : null;

  var setSequence = [
    setPrevPrev ? setPrevPrev.date : null,
    setPrev ? setPrev.date : null,
    setToday ? setToday.date : null,
    setNext ? setNext.date : null,
    setNextNext ? setNextNext.date : null
  ];

  var result = {
    rise: riseSequence,
    set: setSequence
  };

  // Add moon phase information if the body is the Moon
  if (resolvedBody === 'Moon') {
    result.moonPhases = riseSequence.map(function(riseTime) {
      if (riseTime) {
        var phaseIndex = getMoonPhase(riseTime);
        return {
          index: phaseIndex,
          name: phaseNames[phaseIndex]
        };
      }
      return null;
    });
  }

  return result;
}

/**
 * Get dawn and dusk times for a body around a given date.
 * Returns previous 2, current day's, and next 2 dawn/dusk times.
 * @param {string} body - The astronomical body (typically 'Sun')
 * @param {Observer} observer - The observer location
 * @param {Date} date - The reference date (typically today's date)
 * @param {string} twilightType - Type of twilight: 'civil' (-6°), 'nautical' (-12°), 'astronomical' (-18°)
 * @returns {Object} Object with dawn and dusk arrays containing Date objects
 *                   {dawn:[prev-1, prev, today, next, next+1], dusk:[prev-1, prev, today, next, next+1]}
 */
function getTwilightSequence(body, observer, date, twilightType) {
  var referenceDate = date || new Date();
  var resolvedBody = resolveBody(body);

  // Define altitude based on twilight type
  var altitude;
  switch (twilightType) {
    case 'civil':
      altitude = -6;
      break;
    case 'nautical':
      altitude = -12;
      break;
    case 'astronomical':
      altitude = -18;
      break;
    default:
      throw new Error('Invalid twilight type. Use: civil, nautical, or astronomical');
  }

  // Helper function to find the next altitude event
  function findNextAltitudeEvent(direction, startDate) {
    return Astronomy.SearchAltitude(resolvedBody, observer, direction, startDate, 30, altitude);
  }

  // Helper function to find the previous altitude event by searching backwards
  function findPreviousAltitudeEvent(direction, startDate) {
    // Use negative limitDays to search backwards from startDate
    return Astronomy.SearchAltitude(resolvedBody, observer, direction, startDate, -30, altitude);
  }

  // Get dawn events (body ascending through altitude, so direction +1)
  var dawnToday = findNextAltitudeEvent(+1, referenceDate);
  var dawnPrev = dawnToday ? findPreviousAltitudeEvent(+1, dawnToday.date) : null;
  var dawnPrevPrev = dawnPrev ? findPreviousAltitudeEvent(+1, dawnPrev.date) : null;
  var dawnNext = dawnToday ? findNextAltitudeEvent(+1, new Date(dawnToday.date.getTime() + 60000)) : null;
  var dawnNextNext = dawnNext ? findNextAltitudeEvent(+1, new Date(dawnNext.date.getTime() + 60000)) : null;

  var dawnSequence = [
    dawnPrevPrev ? dawnPrevPrev.date : null,
    dawnPrev ? dawnPrev.date : null,
    dawnToday ? dawnToday.date : null,
    dawnNext ? dawnNext.date : null,
    dawnNextNext ? dawnNextNext.date : null
  ];

  // Get dusk events (body descending through altitude, so direction -1)
  var duskToday = findNextAltitudeEvent(-1, referenceDate);
  var duskPrev = duskToday ? findPreviousAltitudeEvent(-1, duskToday.date) : null;
  var duskPrevPrev = duskPrev ? findPreviousAltitudeEvent(-1, duskPrev.date) : null;
  var duskNext = duskToday ? findNextAltitudeEvent(-1, new Date(duskToday.date.getTime() + 60000)) : null;
  var duskNextNext = duskNext ? findNextAltitudeEvent(-1, new Date(duskNext.date.getTime() + 60000)) : null;

  var duskSequence = [
    duskPrevPrev ? duskPrevPrev.date : null,
    duskPrev ? duskPrev.date : null,
    duskToday ? duskToday.date : null,
    duskNext ? duskNext.date : null,
    duskNextNext ? duskNextNext.date : null
  ];

  return {
    dawn: dawnSequence,
    dusk: duskSequence
  };
}

/**
 * Get the next equinox or solstice after the current date.
 * @param {Date} date - The reference date (defaults to today)
 * @returns {Object} Object containing the next seasonal event
 *                   {type: 'marchEquinox'|'juneSolstice'|'septemberEquinox'|'decemberSolstice', date: Date, year: number}
 */
function getNextSeasonalEvent(date) {
  var referenceDate = date || new Date();
  var currentYear = referenceDate.getFullYear();

  // Get seasons for current year and next year (in case we're late in December)
  var currentYearSeasons = Astronomy.Seasons(currentYear);
  var nextYearSeasons = Astronomy.Seasons(currentYear + 1);

  // Create array of all seasonal events for current and next year
  var seasonalEvents = [
    { type: 'marchEquinox', date: currentYearSeasons.mar_equinox.date, year: currentYear },
    { type: 'juneSolstice', date: currentYearSeasons.jun_solstice.date, year: currentYear },
    { type: 'septemberEquinox', date: currentYearSeasons.sep_equinox.date, year: currentYear },
    { type: 'decemberSolstice', date: currentYearSeasons.dec_solstice.date, year: currentYear },
    { type: 'marchEquinox', date: nextYearSeasons.mar_equinox.date, year: currentYear + 1 }
  ];

  // Find the next event after the reference date
  var referenceTime = referenceDate.getTime();

  for (var i = 0; i < seasonalEvents.length; i++) {
    if (seasonalEvents[i].date.getTime() > referenceTime) {
      return seasonalEvents[i];
    }
  }

  // If no event found in current/next year, return null (shouldn't happen)
  return null;
}

/**
 * Get the next transit of Mercury or Venus after the current date (whichever occurs sooner).
 * Technically both can happen at the same time but that is gonna be in like 60,000 years so I'll deal with it later.
 * @param {Date} date - The reference date (defaults to today)
 * @returns {Object|null} Object containing transit information, or null if no transit found
 *                       {body: 'Mercury'|'Venus', start: Date, peak: Date, finish: Date}
 */
function getNextTransit(date) {
  var referenceDate = date || new Date();

  // Search for the next transit of Mercury
  var mercuryTransit = Astronomy.SearchTransit('Mercury', referenceDate);

  // Search for the next transit of Venus
  var venusTransit = Astronomy.SearchTransit('Venus', referenceDate);

  // Determine which transit occurs first (or if any exist)
  var nextTransit = null;
  var nextBody = null;

  if (mercuryTransit && venusTransit) {
    // Both transits exist, find which occurs first
    if (mercuryTransit.start.date < venusTransit.start.date) {
      nextTransit = mercuryTransit;
      nextBody = 'Mercury';
    } else {
      nextTransit = venusTransit;
      nextBody = 'Venus';
    }
  } else if (mercuryTransit) {
    nextTransit = mercuryTransit;
    nextBody = 'Mercury';
  } else if (venusTransit) {
    nextTransit = venusTransit;
    nextBody = 'Venus';
  }

  if (!nextTransit) {
    return null;
  }

  // Return transit information
  return {
    body: nextBody,
    start: nextTransit.start ? nextTransit.start.date : null,
    peak: nextTransit.peak ? nextTransit.peak.date : null,
    finish: nextTransit.finish ? nextTransit.finish.date : null
  };
}

/**
 * Get the next eclipse (lunar or solar) after the current date (whichever occurs sooner).
 * @param {Date} date - The reference date (defaults to today)
 * @returns {Object|null} Object containing eclipse information, or null if no eclipse found
 *                       Lunar: {type: 'lunar', peak: Date, partialBegin: Date, totalBegin: Date, totalEnd: Date, partialEnd: Date, kind: string}
 *                       Solar: {type: 'solar', peak: Date, kind: string, distance: number, latitude?: number, longitude?: number, obscuration?: number}
 */
function getNextEclipse(date) {
  var referenceDate = date || new Date();

  // Search for the next lunar eclipse
  var lunarEclipse = Astronomy.SearchLunarEclipse(referenceDate);

  // Search for the next global solar eclipse
  var solarEclipse = Astronomy.SearchGlobalSolarEclipse(referenceDate);

  // Determine which eclipse occurs first (or if any exist)
  var nextEclipse = null;
  var nextType = null;

  if (lunarEclipse && solarEclipse) {
    // Both eclipses exist, find which occurs first
    if (lunarEclipse.peak.date < solarEclipse.peak.date) {
      nextEclipse = lunarEclipse;
      nextType = 'lunar';
    } else {
      nextEclipse = solarEclipse;
      nextType = 'solar';
    }
  } else if (lunarEclipse) {
    nextEclipse = lunarEclipse;
    nextType = 'lunar';
  } else if (solarEclipse) {
    nextEclipse = solarEclipse;
    nextType = 'solar';
  }

  if (!nextEclipse) {
    return null;
  }

  // Return eclipse information with common properties
  var result = {
    type: nextType,
    peak: nextEclipse.peak ? nextEclipse.peak.date : null
  };

  // Add type-specific properties
  if (nextType === 'lunar') {
    // Lunar eclipse properties
    result.partialBegin = nextEclipse.partial_begin ? nextEclipse.partial_begin.date : null;
    result.totalBegin = nextEclipse.total_begin ? nextEclipse.total_begin.date : null;
    result.totalEnd = nextEclipse.total_end ? nextEclipse.total_end.date : null;
    result.partialEnd = nextEclipse.partial_end ? nextEclipse.partial_end.date : null;
    result.kind = nextEclipse.kind; // 'total', 'penumbral', 'partial'
  } else {
    // Solar eclipse properties - handle all GlobalSolarEclipseInfo properties
    result.kind = nextEclipse.kind; // 'partial', 'annular', 'total'
    result.distance = nextEclipse.distance; // Distance in km between shadow axis and Earth center

    // Latitude and longitude are only defined for total or annular eclipses
    if (nextEclipse.kind === 'total' || nextEclipse.kind === 'annular') {
      result.latitude = nextEclipse.latitude; // Geographic latitude where eclipse is centered
      result.longitude = nextEclipse.longitude; // Geographic longitude where eclipse is centered
      result.obscuration = nextEclipse.obscuration; // Fraction of Sun's disc obscured (0, 1]
    } else {
      // For partial eclipses, these properties are undefined
      result.latitude = undefined;
      result.longitude = undefined;
      result.obscuration = undefined;
    }
  }

  return result;
}

/**
 * Get the next lunar apsis (perigee or apogee) after the current date.
 * @param {Date} date - The reference date (defaults to today)
 * @returns {Object|null} Object containing apsis information, or null if none found
 *                       {time: Date, kind: 'perigee'|'apogee', distance: number}
 */
function getNextLunarApsis(date) {
  var referenceDate = date || new Date();

  // Search for the next lunar apsis (perigee or apogee)
  var apsis = Astronomy.SearchLunarApsis(referenceDate);

  if (!apsis) {
    return null;
  }

  // Return apsis information
  return {
    time: apsis.time ? apsis.time.date : null,
    kind: apsis.kind === 0 ? 'perigee' : 'apogee', // 0 = perigee, 1 = apogee based on typical astronomy enums
    distance: apsis.dist_km // Distance in kilometers
  };
}

/**
 * Get solar noon and midnight times for around a given date.
 * Solar noon is when the Sun reaches its highest point (hour angle 0).
 * Solar midnight is when the Sun reaches its lowest point (hour angle 12).
 * Returns previous 2, current day's, and next 2 solar noon/midnight times.
 * @param {Observer} observer - The observer location
 * @param {Date} date - The reference date (typically today's date)
 * @returns {Object} Object with noon and midnight arrays containing Date objects
 *                   {noon:[prev-1, prev, today, next, next+1], midnight:[prev-1, prev, today, next, next+1]}
 */
function getSolarNoonMidnightSequence(observer, date) {
  var referenceDate = date || new Date();

  // Helper function to find the next hour angle event
  function findNextHourAngle(hourAngle, startDate) {
    return Astronomy.SearchHourAngle('Sun', observer, hourAngle, startDate, +1);
  }

  // Helper function to find the previous hour angle event
  function findPreviousHourAngle(hourAngle, startDate) {
    return Astronomy.SearchHourAngle('Sun', observer, hourAngle, startDate, -1);
  }

  // Get solar noon events (hour angle 0)
  var noonToday = findNextHourAngle(0, referenceDate);
  var noonPrev = noonToday ? findPreviousHourAngle(0, noonToday.time.date) : null;
  var noonPrevPrev = noonPrev ? findPreviousHourAngle(0, noonPrev.time.date) : null;
  var noonNext = noonToday ? findNextHourAngle(0, new Date(noonToday.time.date.getTime() + 60000)) : null;
  var noonNextNext = noonNext ? findNextHourAngle(0, new Date(noonNext.time.date.getTime() + 60000)) : null;

  var noonSequence = [
    noonPrevPrev ? noonPrevPrev.time.date : null,
    noonPrev ? noonPrev.time.date : null,
    noonToday ? noonToday.time.date : null,
    noonNext ? noonNext.time.date : null,
    noonNextNext ? noonNextNext.time.date : null
  ];

  // Get solar midnight events (hour angle 12)
  var midnightToday = findNextHourAngle(12, referenceDate);
  var midnightPrev = midnightToday ? findPreviousHourAngle(12, midnightToday.time.date) : null;
  var midnightPrevPrev = midnightPrev ? findPreviousHourAngle(12, midnightPrev.time.date) : null;
  var midnightNext = midnightToday ? findNextHourAngle(12, new Date(midnightToday.time.date.getTime() + 60000)) : null;
  var midnightNextNext = midnightNext ? findNextHourAngle(12, new Date(midnightNext.time.date.getTime() + 60000)) : null;

  var midnightSequence = [
    midnightPrevPrev ? midnightPrevPrev.time.date : null,
    midnightPrev ? midnightPrev.time.date : null,
    midnightToday ? midnightToday.time.date : null,
    midnightNext ? midnightNext.time.date : null,
    midnightNextNext ? midnightNextNext.time.date : null
  ];

  return {
    noon: noonSequence,
    midnight: midnightSequence
  };
}

/**
 * Check if cached events are still valid (within time limit and observer hasn't moved significantly)
 * @param {Object} cacheEntry - The cached entry with timestamp and observer
 * @param {Observer} observer - Current observer location
 * @returns {boolean} True if cache is valid
 */
function isCacheValid(cacheEntry, observer) {
  var now = new Date().getTime();
  var timeDiff = now - cacheEntry.timestamp;

  // Check if cache is expired
  if (timeDiff > CACHE_DURATION_MS) {
    return false;
  }

  // Check if observer has moved significantly
  var latDiff = Math.abs(cacheEntry.observer.latitude - observer.latitude);
  var lonDiff = Math.abs(cacheEntry.observer.longitude - observer.longitude);

  return latDiff <= LOCATION_THRESHOLD_DEGREES && lonDiff <= LOCATION_THRESHOLD_DEGREES;
}

/**
 * Generate cache key based on observer location, date, and settings
 * @param {Observer} observer - Observer location
 * @param {Date} date - Reference date
 * @param {Object} settings - Clay settings object
 * @returns {string} Cache key
 */
function generateCacheKey(observer, date, settings) {
  var lat = Math.round(observer.latitude * 100) / 100; // Round to 2 decimal places
  var lon = Math.round(observer.longitude * 100) / 100; // Round to 2 decimal places
  var dateStr = date.toISOString().split('T')[0]; // YYYY-MM-DD format

  // Create a hash of the settings to include in cache key
  var settingsHash = '';
  if (settings) {
    var keys = Object.keys(settings).sort();
    settingsHash = keys.map(function(key) {
      var value = settings[key];
      if (Array.isArray(value)) {
        return value.join(',');
      }
      return String(value);
    }).join('|');
  }

  return lat + '_' + lon + '_' + dateStr + '_' + settingsHash;
}

/**
 * Get all available astronomical events for the given observer and date.
 * Results are cached for 30 minutes if the observer hasn't moved significantly.
 * @param {Observer} observer - The observer location
 * @param {Date} date - The reference date (defaults to today)
 * @param {Object} settings - Clay settings object controlling which events to include
 * @returns {Object} Object containing all available astronomical events based on settings
 */
function getAllEvents(observer, date, settings) {
  var referenceDate = date || new Date();
  var cacheKey = generateCacheKey(observer, referenceDate, settings);

  // Parse settings (default to enabled if not provided)
  var cfg = settings || {};
  var sunRiseSet = cfg.CFG_SUN_RISE_SET !== false;
  var sunCivilTwilight = cfg.CFG_SUN_CIVIL_DAWN_DUSK !== false;
  var sunNauticalTwilight = cfg.CFG_SUN_NAUTICAL_DAWN_DUSK !== false;
  var sunAstronomicalTwilight = cfg.CFG_SUN_ASTRONOMICAL_DAWN_DUSK !== false;
  var sunSolarNoonMidnight = cfg.CFG_SUN_SOLAR_NOON_MIDNIGHT !== false;
  var sunSolstices = cfg.CFG_SUN_SOLSTICES !== false;
  var sunEquinoxes = cfg.CFG_SUN_EQUINOXES !== false;
  var sunEclipses = cfg.CFG_SUN_ECLIPSES !== false;
  var sunSolarTransits = cfg.CFG_SUN_SOLAR_TRANSITS !== false;
  var moonRiseSet = cfg.CFG_MOON_RISE_SET !== false;
  var moonApogeePerigee = cfg.CFG_MOON_APOGEE_PERIGEE !== false;
  var planetEvents = cfg.CFG_PLANET_EVENTS || [false, false, false, false, false, false, false, false];

  // Check cache first
  if (eventsCache[cacheKey] && isCacheValid(eventsCache[cacheKey], observer)) {
    return eventsCache[cacheKey].events;
  }

  // Calculate all events
  var events = {
    riseSetEvents: [],
    twilightEvents: [],
    solarNoonMidnightEvents: [],
    seasonalEvents: [],
    transitEvents: [],
    eclipseEvents: [],
    lunarApsisEvents: []
  };

  // Get bodies to check for rise/set events based on settings
  var bodies = [];
  if (sunRiseSet) bodies.push('Sun');
  if (moonRiseSet) bodies.push('Moon');
  // Planet events array: [Mercury, Venus, Mars, Jupiter, Saturn, Uranus, Neptune, Pluto]
  if (planetEvents[1]) bodies.push('Venus');  // Venus
  if (planetEvents[2]) bodies.push('Mars');   // Mars
  if (planetEvents[3]) bodies.push('Jupiter'); // Jupiter
  if (planetEvents[4]) bodies.push('Saturn');  // Saturn
  if (planetEvents[0]) bodies.push('Mercury'); // Mercury
  if (planetEvents[5]) bodies.push('Uranus');  // Uranus
  if (planetEvents[6]) bodies.push('Neptune'); // Neptune
  if (planetEvents[7]) bodies.push('Pluto');   // Pluto

  // Get rise/set events for all bodies
  bodies.forEach(function(body) {
    try {
      var riseSet = getRiseSetSequence(body, observer, referenceDate);
      riseSet.rise.forEach(function(riseTime, index) {
        if (riseTime) {
          var event = {
            type: 'rise',
            body: body,
            time: riseTime
          };
          if (body === 'Moon' && riseSet.moonPhases && riseSet.moonPhases[index]) {
            event.moonPhase = riseSet.moonPhases[index];
          }
          events.riseSetEvents.push(event);
        }
      });
      riseSet.set.forEach(function(setTime) {
        if (setTime) {
          events.riseSetEvents.push({
            type: 'set',
            body: body,
            time: setTime
          });
        }
      });
    } catch (e) {
      // Skip bodies that might not be visible or calculable
      logger.log('Error getting rise/set for ' + body + ':', e.message);
    }
  });

  // Get twilight events for Sun
  try {
    if (sunCivilTwilight) {
      var civilTwilight = getTwilightSequence('Sun', observer, referenceDate, 'civil');
      civilTwilight.dawn.forEach(function(dawnTime) {
        if (dawnTime) {
          events.twilightEvents.push({
            type: 'dawn',
            subtype: 'civil',
            time: dawnTime
          });
        }
      });
      civilTwilight.dusk.forEach(function(duskTime) {
        if (duskTime) {
          events.twilightEvents.push({
            type: 'dusk',
            subtype: 'civil',
            time: duskTime
          });
        }
      });
    }

    if (sunNauticalTwilight) {
      var nauticalTwilight = getTwilightSequence('Sun', observer, referenceDate, 'nautical');
      nauticalTwilight.dawn.forEach(function(dawnTime) {
        if (dawnTime) {
          events.twilightEvents.push({
            type: 'dawn',
            subtype: 'nautical',
            time: dawnTime
          });
        }
      });
      nauticalTwilight.dusk.forEach(function(duskTime) {
        if (duskTime) {
          events.twilightEvents.push({
            type: 'dusk',
            subtype: 'nautical',
            time: duskTime
          });
        }
      });
    }

    if (sunAstronomicalTwilight) {
      var astronomicalTwilight = getTwilightSequence('Sun', observer, referenceDate, 'astronomical');
      astronomicalTwilight.dawn.forEach(function(dawnTime) {
        if (dawnTime) {
          events.twilightEvents.push({
            type: 'dawn',
            subtype: 'astronomical',
            time: dawnTime
          });
        }
      });
      astronomicalTwilight.dusk.forEach(function(duskTime) {
        if (duskTime) {
          events.twilightEvents.push({
            type: 'dusk',
            subtype: 'astronomical',
            time: duskTime
          });
        }
      });
    }
  } catch (e) {
    logger.log('Error getting twilight events:', e.message);
  }

  // Get solar noon/midnight events
  if (sunSolarNoonMidnight) {
    try {
      var solarNoonMidnight = getSolarNoonMidnightSequence(observer, referenceDate);
      solarNoonMidnight.noon.forEach(function(noonTime) {
        if (noonTime) {
          events.solarNoonMidnightEvents.push({
            type: 'noon',
            time: noonTime
          });
        }
      });
      solarNoonMidnight.midnight.forEach(function(midnightTime) {
        if (midnightTime) {
          events.solarNoonMidnightEvents.push({
            type: 'midnight',
            time: midnightTime
          });
        }
      });
    } catch (e) {
      logger.log('Error getting solar noon/midnight events:', e.message);
    }
  }

  // Get seasonal events (equinoxes/solstices)
  if (sunSolstices || sunEquinoxes) {
    try {
      var seasonalEvent = getNextSeasonalEvent(referenceDate);
      if (seasonalEvent) {
        events.seasonalEvents.push(seasonalEvent);
      }
    } catch (e) {
      logger.log('Error getting seasonal events:', e.message);
    }
  }

  // Get transit events
  if (sunSolarTransits) {
    try {
      var transitEvent = getNextTransit(referenceDate);
      if (transitEvent && transitEvent.start) {
        events.transitEvents.push(transitEvent);
      }
    } catch (e) {
      logger.log('Error getting transit events:', e.message);
    }
  }

  // Get eclipse events
  if (sunEclipses) {
    try {
      var eclipseEvent = getNextEclipse(referenceDate);
      if (eclipseEvent && eclipseEvent.peak) {
        events.eclipseEvents.push(eclipseEvent);
      }
    } catch (e) {
      logger.log('Error getting eclipse events:', e.message);
    }
  }

  // Get lunar apsis events
  if (moonApogeePerigee) {
    try {
      var apsisEvent = getNextLunarApsis(referenceDate);
      if (apsisEvent && apsisEvent.time) {
        events.lunarApsisEvents.push(apsisEvent);
      }
    } catch (e) {
      logger.log('Error getting lunar apsis events:', e.message);
    }
  }

  // Sort events by time within each category
  Object.keys(events).forEach(function(category) {
    events[category].sort(function(a, b) {
      var timeA = a.time || a.start || a.peak || a.date;
      var timeB = b.time || b.start || b.peak || b.date;
      return timeA - timeB;
    });
  });

  // Cache the results
  eventsCache[cacheKey] = {
    timestamp: new Date().getTime(),
    observer: {
      latitude: observer.latitude,
      longitude: observer.longitude
    },
    events: events
  };

  return events;
}

module.exports = {
  getRiseSetSequence,
  getTwilightSequence,
  getSolarNoonMidnightSequence,
  getNextSeasonalEvent,
  getNextTransit,
  getNextEclipse,
  getNextLunarApsis,
  getMoonPhase,
  getMoonPhaseName,
  getAllEvents
};
