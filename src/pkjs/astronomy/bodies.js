var Astronomy = require('astronomy-engine');


function resolveBody(body) {
  if (!body) {
    throw new Error('Body is required');
  }
  return body;
}

function getHorizontal(body, observer, date) {
  var when = date || new Date();
  var equ = Astronomy.Equator(resolveBody(body), when, observer, true, true);
  var hor = Astronomy.Horizon(when, observer, equ.ra, equ.dec, Astronomy.Refraction.Normal);
  return {
    azimuth: hor.azimuth,
    altitude: hor.altitude
  };
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

//TODO: do all phases span the same angle range? fix if they don't
function getMoonPhase(date) {
  var when = date || new Date();
  var angle = Astronomy.MoonPhase(when); // 0=new, 180=full
  return Math.floor(((angle + 22.5) % 360) / 45); // 0-7
}

// Jupiter's major moons (lowercase names as used in JupiterMoons object keys)
// Not exported since the function now accepts names directly

function getJupiterMoonHorizontal(moonName, observer, date) {
  var when = date || new Date();

  // Get Jupiter's equatorial coordinates
  var jupiterEqu = Astronomy.Equator('Jupiter', when, observer, true, true);

  // Get Jupiter's moons relative positions
  var moons = Astronomy.JupiterMoons(when);

  //console.log('Jupiter moons:', JSON.stringify(moons));

  // Get the specific moon's position relative to Jupiter
  var moon = moons[moonName];
  if (!moon) {
    throw new Error('Unknown Jupiter moon: ' + moonName);
  }

  // Convert to absolute equatorial coordinates
  var moonEqu = {
    ra: jupiterEqu.ra + moon.x / 3600,  // Convert arcseconds to degrees
    dec: jupiterEqu.dec + moon.y / 3600
  };

  // Convert to horizontal coordinates
  var hor = Astronomy.Horizon(when, observer, moonEqu.ra, moonEqu.dec, Astronomy.Refraction.Normal);

  return {
    azimuth: hor.azimuth,
    altitude: hor.altitude
  };
}

module.exports = {
  getHorizontal: getHorizontal,
  getIllumination: getIllumination,
  getRiseSet: getRiseSet,
  getMoonPhase: getMoonPhase,
  getJupiterMoonHorizontal: getJupiterMoonHorizontal
};