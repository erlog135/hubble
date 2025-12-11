// Message processing utilities for body/details window
/**
 * BodyPackage bit layout (8 bytes):
 * body id (8 bit uint)
 * azimuth (9 bit uint) 0-360 degrees
 * altitude (8 bit signed) -90 to 90 degrees
 * rise hour (5 bit uint) 0-23
 * rise minute (6 bit uint) 0-59
 * set hour (5 bit uint) 0-23
 * set minute (6 bit uint) 0-59
 * luminance * 10 (9 bit signed) -256 to 255
 * phase (3 bit uint) 0-7 (ignored unless moon)
 * +5 bits padding
 */

var Bodies = require('./astronomy/bodies');
var Keys = require('message_keys');

// Body names keyed by body id (must stay in sync with watch side)
var BODY_NAMES = [
  "Mercury",
  "Venus",
  "Mars",
  "Jupiter",
  "Saturn",
  "Uranus",
  "Neptune",
  "Pluto",
  "Moon",
  "Io",
  "Europa",
  "Ganymede",
  "Callisto"
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
  var horizontal = Bodies.getHorizontal(bodyName, observer, when);
  var riseSet = Bodies.getRiseSet(bodyName, observer, when);
  var illum = Bodies.getIllumination(bodyName, when);
  var phase = (bodyName === 'Moon') ? Bodies.getMoonPhase(when) : 0;

  var az = encodeUnsigned(horizontal.azimuth, 9, 0, 360);
  var alt = encodeSigned(horizontal.altitude, 8, -90, 90);

  var riseHour = riseSet.rise ? riseSet.rise.getUTCHours() : SENTINEL_HOUR;
  var riseMin = riseSet.rise ? riseSet.rise.getUTCMinutes() : SENTINEL_MIN;
  var setHour = riseSet.set ? riseSet.set.getUTCHours() : SENTINEL_HOUR;
  var setMin = riseSet.set ? riseSet.set.getUTCMinutes() : SENTINEL_MIN;

  var lumTimes10 = encodeSigned((illum && illum.mag != null) ? illum.mag * 10 : 0, 9, -256, 255);
  var phaseIndex = encodeUnsigned(phase, 3, 0, 7);

  var buffer = new Uint8Array(8);
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

  write(encodeUnsigned(bodyId, 8, 0, 255), 8);
  write(az, 9);
  write(alt, 8);
  write(encodeUnsigned(riseHour, 5, 0, 31), 5);
  write(encodeUnsigned(riseMin, 6, 0, 63), 6);
  write(encodeUnsigned(setHour, 5, 0, 31), 5);
  write(encodeUnsigned(setMin, 6, 0, 63), 6);
  write(lumTimes10, 9);
  write(phaseIndex, 3);
  write(0, 5); // padding

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
    var bodyId = payload[Keys.REQUEST_BODY];
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