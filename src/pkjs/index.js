var Observer = require('./astronomy/observer');
var Bodies = require('./astronomy/bodies');
var MsgProc = require('./msgproc');
var Declination = require('./declination');

var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

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

  console.log('Settings: ' + localStorage.getItem('clay-settings'));

  Observer.initObserver().then(function(observer) {
    activeObserver = observer;
    console.log('Observer ready (lat=' + observer.latitude +
      ', lon=' + observer.longitude + ', h=' + observer.height + ')');

    // Register handler for body data requests from watch
    MsgProc.registerBodyRequestHandler(function() { return activeObserver; });
    console.log('Body request handler registered');

    // Calculate and send magnetic declination
    var declination = Declination.getMagneticDeclination(activeObserver);
    //TODO: Uncomment this when we have a way to store the declination
    //Declination.sendMagneticDeclination(declination);

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