var Keys = require('message_keys');
var geomagnetism = require('geomagnetism');

function getMagneticDeclination(observer, date) {
  if (!observer || !observer.latitude || !observer.longitude) {
    console.log('Cannot calculate magnetic declination: invalid observer');
    return null;
  }

  try {
    // Create geomagnetism model for the given date (or current date if none provided)
    var when = date || new Date();
    var model = geomagnetism.model(when);

    // Get magnetic declination at the observer's location
    // Note: geomagnetism expects [lat, lon] array, and optionally altitude in km
    var location = [observer.latitude, observer.longitude];

    // If observer has height, convert from meters to kilometers for geomagnetism
    if (observer.height !== undefined && observer.height !== null) {
      location.push(observer.height / 1000);
    }

    var magneticInfo = model.point(location);

    console.log('Magnetic declination at (' + observer.latitude + ', ' + observer.longitude +
      '): ' + magneticInfo.decl + ' degrees');

    return magneticInfo.decl;
  } catch (err) {
    console.log('Error calculating magnetic declination: ' + err.message);
    return null;
  }
}

function sendMagneticDeclination(declination) {
  if (declination === null) {
    console.log('Not sending magnetic declination: value is null');
    return;
  }

  // Round declination to nearest integer degree
  var declinationRounded = Math.round(declination);

  Pebble.sendAppMessage(
    (function() {
      var dict = {};
      dict[Keys.DECLINATION] = declinationRounded;
      return dict;
    })(),
    function() {
      console.log('Sent magnetic declination: ' + declinationRounded + ' degrees');
    },
    function(err) {
      console.log('Failed to send magnetic declination: ' + JSON.stringify(err));
    }
  );
}

module.exports = {
  getMagneticDeclination: getMagneticDeclination,
  sendMagneticDeclination: sendMagneticDeclination
};
