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


module.exports = {
  getHorizontal: getHorizontal,
  getIllumination: getIllumination,
  getRiseSet: getRiseSet,
  getMoonPhase: getMoonPhase
};