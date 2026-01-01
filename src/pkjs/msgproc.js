// Message processing utilities for body/details window
/**
 * BodyPackage bit layout (7 bytes):
 * body id (5 bit uint) + phase (3 bit uint)
 * azimuth (9 bit uint) 0-360 degrees
 * altitude (8 bit signed) -90 to 90 degrees
 * rise hour (5 bit uint) 0-23
 * rise minute (6 bit uint) 0-59
 * set hour (5 bit uint) 0-23
 * set minute (6 bit uint) 0-59
 * luminance * 10 (9 bit signed) -256 to 255
 */

var Keys = require('message_keys');
var Bodies = require('./astronomy/bodies');

// Body names keyed by body id (must stay in sync with watch side)
var BODY_NAMES = [
  "Moon",
  "Mercury",
  "Venus",
  "Mars",
  "Jupiter",
  "Saturn",
  "Uranus",
  "Neptune",
  "Pluto",
  "Sun",
  //+12 traditional zodiac constellations
  "Aries",
  "Taurus",
  "Gemini",
  "Cancer",
  "Leo",
  "Virgo",
  "Libra",
  "Scorpius",
  "Sagittarius",
  "Capricornus",
  "Aquarius",
  "Pisces",
  //+7 other notable ones
  "Orion",
  "Ursa Major",
  "Ursa Minor",
  "Cassiopeia",
  "Cygnus",
  "Crux",
  "Lyra"
];

var SENTINEL_HOUR = 31;   // fits in 5 bits
var SENTINEL_MIN = 63;    // fits in 6 bits

function clamp(value, min, max) {
  return Math.min(max, Math.max(min, value));
}

function encodeSigned(value, bits, min, max) {
  var clamped = clamp(Math.round(value), min, max);
  if (clamped < 0) {
    return (1 << bits) + clamped;
  }
  return clamped;
}

function encodeUnsigned(value, bits, min, max) {
  return clamp(Math.round(value), min, max);
}

function packBodyPackage(bodyId, observer, date) {
  var bodyName = BODY_NAMES[bodyId];
  if (!bodyName) {
    throw new Error('Unknown body id: ' + bodyId);
  }
  if (!observer) {
    throw new Error('Observer required to compute body package');
  }

  var when = date || new Date();

  // Determine body type
  var canHaveRiseSet = (bodyId <= 9);  // Moon (0), planets (1-8), and Sun (9)

  var horizontal = null;
  try {
    // For planets and Moon, use standard horizontal calculation
    horizontal = Bodies.getHorizontal(bodyName, observer, when);
  } catch (err) {
    console.log('Warning: Could not calculate horizontal position for ' + bodyName + ': ' + err.message);
    horizontal = { azimuth: 0, altitude: 0 };
  }

  var riseSet = null;
  if (canHaveRiseSet) {
    try {
      riseSet = Bodies.getRiseSet(bodyName, observer, when);
      console.log('Rise/set: ' + riseSet.rise + ' - ' + riseSet.set);
    } catch (err) {
      console.log('Warning: Could not calculate rise/set for ' + bodyName + ': ' + err.message);
      riseSet = { rise: null, set: null };
    }
  } else {
    // Bodies that can't have rise/set get null values
    riseSet = { rise: null, set: null };
  }

  var illum = null;
  try {
    illum = Bodies.getIllumination(bodyName, when);
  } catch (err) {
    console.log('Warning: Could not calculate illumination for ' + bodyName + ': ' + err.message);
    illum = { mag: 0 };
  }

  var phase = 0;
  if (bodyName === 'Moon') {
    try {
      phase = Bodies.getMoonPhase(when);
    } catch (err) {
      console.log('Warning: Could not calculate moon phase: ' + err.message);
      phase = 0;
    }
  }

  var az = horizontal ? encodeUnsigned(horizontal.azimuth || 0, 9, 0, 360) : 0;
  var alt = horizontal ? encodeSigned(horizontal.altitude || 0, 8, -90, 90) : 0;

  var riseHour = riseSet.rise ? riseSet.rise.getHours() : SENTINEL_HOUR;
  var riseMin = riseSet.rise ? riseSet.rise.getMinutes() : SENTINEL_MIN;
  var setHour = riseSet.set ? riseSet.set.getHours() : SENTINEL_HOUR;
  var setMin = riseSet.set ? riseSet.set.getMinutes() : SENTINEL_MIN;


  console.log('Rise/set: ' + riseHour + ':' + riseMin + ' - ' + setHour + ':' + setMin);
  console.log('Illum: ' + illum.mag);
  console.log('Phase: ' + phase);
  
  var lumTimes10 = encodeSigned((illum && illum.mag != null) ? illum.mag * 10 : 0, 9, -256, 255);
  var phaseIndex = encodeUnsigned(phase, 3, 0, 7);

  var buffer = new Uint8Array(7);
  var bitPos = 0;
  function write(value, width) {
    for (var i = 0; i < width; i++) {
      var byteIndex = bitPos >> 3;
      var bitIndex = bitPos & 7;
      if (value & 1) {
        buffer[byteIndex] |= (1 << bitIndex);
      }
      value >>= 1;
      bitPos++;
    }
  }

  write(encodeUnsigned(bodyId, 5, 0, 31), 5);
  write(phaseIndex, 3);
  write(az, 9);
  write(alt, 8);
  write(encodeUnsigned(riseHour, 5, 0, 31), 5);
  write(encodeUnsigned(riseMin, 6, 0, 63), 6);
  write(encodeUnsigned(setHour, 5, 0, 31), 5);
  write(encodeUnsigned(setMin, 6, 0, 63), 6);
  write(lumTimes10, 9);

  return buffer;
}

function sendBodyPackage(bodyId, observer, date) {
  var payload = packBodyPackage(bodyId, observer, date);
  Pebble.sendAppMessage(
    (function() {
      var dict = {};
      dict[Keys.BODY_PACKAGE] = Array.from(payload);
      return dict;
    })(),
    function() {
      console.log('Sent body package for body ' + bodyId);
    },
    function(err) {
      console.log('Failed to send body package: ' + JSON.stringify(err));
    }
  );
}

function registerBodyRequestHandler(observerProvider) {
  Pebble.addEventListener('appmessage', function(e) {
    var payload = e && e.payload ? e.payload : {};
    console.log('Received payload: ' + JSON.stringify(payload));

    //this is okay in emulator but not on device
    // var bodyId = payload[Keys.REQUEST_BODY];

    var bodyId = null;

    if (payload.hasOwnProperty("REQUEST_BODY")) {
      bodyId = payload["REQUEST_BODY"];
    }
    
    console.log('Received body request for body ' + bodyId);
    if (bodyId === undefined || bodyId === null) {
      return;
    }

    var observer = (typeof observerProvider === 'function') ? observerProvider() : observerProvider;
    if (!observer) {
      console.log('Cannot process body request: missing observer');
      return;
    }

    try {
      sendBodyPackage(bodyId, observer, new Date());
    } catch (err) {
      console.log('Error handling body request: ' + err.message);
    }
  });
}

module.exports = {
  packBodyPackage: packBodyPackage,
  sendBodyPackage: sendBodyPackage,
  registerBodyRequestHandler: registerBodyRequestHandler
};