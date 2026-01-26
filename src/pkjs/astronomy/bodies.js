var Astronomy = require('astronomy-engine');
var Constellations = require('./constellations');

// Constellation names in order (matching BODY_NAMES indices 10-28)
var CONSTELLATION_NAMES = [
  'Aries', 'Taurus', 'Gemini', 'Cancer', 'Leo', 'Virgo',
  'Libra', 'Scorpius', 'Sagittarius', 'Capricornus', 'Aquarius', 'Pisces',
  'Orion', 'Ursa Major', 'Ursa Minor', 'Cassiopeia', 'Cygnus', 'Crux', 'Lyra'
];

function resolveBody(body) {
  if (!body) {
    throw new Error('Body is required');
  }
  return body;
}

function getHorizontal(body, observer, date) {
  var when = date || new Date();
  
  // Check if this is a constellation (index >= 10)
  var constellationIndex = CONSTELLATION_NAMES.indexOf(body);
  
  if (constellationIndex !== -1) {
    // Use fixed RA/Dec from constellations.js
    var coords = Constellations.CONSTELLATION_COORDS[constellationIndex];
    var hor = Astronomy.Horizon(when, observer, coords.ra / 15, coords.dec, Astronomy.Refraction.Normal);
    return {
      azimuth: hor.azimuth,
      altitude: hor.altitude
    };
  } else {
    // Use astronomy engine for planets, moon, sun
    var equ = Astronomy.Equator(resolveBody(body), when, observer, true, true);
    var hor = Astronomy.Horizon(when, observer, equ.ra, equ.dec, Astronomy.Refraction.Normal);
    return {
      azimuth: hor.azimuth,
      altitude: hor.altitude
    };
  }
}

function getIllumination(body, date) {
  var when = date || new Date();
  return Astronomy.Illumination(resolveBody(body), when);
}

function getRiseSet(body, observer, date) {
  var when = date || new Date();
  var rise = Astronomy.SearchRiseSet(resolveBody(body), observer, +1, when, 99);
  var set = Astronomy.SearchRiseSet(resolveBody(body), observer, -1, when, 99);
  return {
    rise: rise ? rise.date : null,
    set: set ? set.date : null
  };
}


module.exports = {
  getHorizontal: getHorizontal,
  getIllumination: getIllumination,
  getRiseSet: getRiseSet
};