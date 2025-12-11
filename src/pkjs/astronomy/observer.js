var Astronomy = require('astronomy-engine');

var DEFAULT_ALTITUDE_METERS = 0;

function coordsToObserver(position) {
  var coords = position.coords || {};
  var altitude = typeof coords.altitude === 'number' ? coords.altitude : DEFAULT_ALTITUDE_METERS;
  return new Astronomy.Observer(coords.latitude, coords.longitude, altitude);
}

function requestLocation() {
  return new Promise(function(resolve, reject) {
    if (typeof navigator === 'undefined' || !navigator.geolocation) {
      reject(new Error('Geolocation unavailable'));
      return;
    }

    navigator.geolocation.getCurrentPosition(resolve, reject, {
      enableHighAccuracy: true,
      maximumAge: 10000,
      timeout: 10000
    });
  });
}

function initObserver() {
  return requestLocation().then(function(position) {
    var observer = coordsToObserver(position);
    console.log('Observer initialized from phone location: lat=' +
      observer.latitude + ', lon=' + observer.longitude +
      ', h=' + observer.height);
    return observer;
  }).catch(function(err) {
    console.log('Unable to initialize observer from location: ' + err.message);
    throw err;
  });
}

module.exports = {
  initObserver: initObserver,
  coordsToObserver: coordsToObserver,
  requestLocation: requestLocation
};