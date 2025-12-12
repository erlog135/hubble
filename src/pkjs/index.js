var Observer = require('./astronomy/observer');
var Bodies = require('./astronomy/bodies');
var MsgProc = require('./msgproc');

var activeObserver = null;
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

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');

  Observer.initObserver().then(function(observer) {
    activeObserver = observer;
    console.log('Observer ready (lat=' + observer.latitude +
      ', lon=' + observer.longitude + ', h=' + observer.height + ')');

    // Register handler for body data requests from watch
    MsgProc.registerBodyRequestHandler(function() { return activeObserver; });
    console.log('Body request handler registered');

    var phaseIndex = Bodies.getMoonPhase(new Date());
    console.log('Current Moon Phase: ' + phaseNames[phaseIndex] +
      ' (index ' + phaseIndex + ')');
  }).catch(function(err) {
    console.log('Proceeding without observer: ' + err.message);
    var phaseIndex = Bodies.getMoonPhase(new Date());
    console.log('Current Moon Phase: ' + phaseNames[phaseIndex] +
      ' (index ' + phaseIndex + ')');
  });
});