var Astronomy = require('astronomy-engine');
var Observer = require('./astronomy/observer');

var activeObserver = null;

function logNextMoonPhase() {
  var now = new Date();
  var quarter = Astronomy.SearchMoonQuarter(now);
  var phaseNames = ["New Moon", "First Quarter", "Full Moon", "Third Quarter"];
  var phaseText = "Current/Next Moon Phase: " + phaseNames[quarter.quarter] +
    " at " + quarter.time.date.toUTCString();
  console.log(phaseText);
}

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');

  Observer.initObserver().then(function(observer) {
    activeObserver = observer;
    console.log('Observer ready (lat=' + observer.latitude +
      ', lon=' + observer.longitude + ', h=' + observer.height + ')');
    logNextMoonPhase();
  }).catch(function(err) {
    console.log('Proceeding without observer: ' + err.message);
    logNextMoonPhase();
  });
});