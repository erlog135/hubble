var Observer = require('./astronomy/observer');
var Bodies = require('./astronomy/bodies');
var Events = require('./astronomy/events');
var MsgProc = require('./msgproc');
var Declination = require('./declination');
var PinPusher = require('./pinpusher');
var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

var activeObserver = null;

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');

  //PinPusher.pushTestPin();
  
  
  Observer.initObserver().then(function(observer) {
    activeObserver = observer;
    console.log('Observer ready (lat=' + observer.latitude +
      ', lon=' + observer.longitude + ', h=' + observer.height + ')');
      
      // Register handler for body data requests from watch
      MsgProc.registerBodyRequestHandler(function() { return activeObserver; });
      console.log('Body request handler registered');
      
      console.log('Settings: ' + localStorage.getItem('clay-settings'));
     console.log('All events: ' + JSON.stringify(Events.getAllEvents(activeObserver, new Date())));
      
      // Calculate and send magnetic declination
      var declination = Declination.getMagneticDeclination(activeObserver);
      //Declination.sendMagneticDeclination(declination);
    }).catch(function(err) {
      console.log('Proceeding without observer: ' + err.message);
      var phaseIndex = Events.getMoonPhase(new Date());
      var phaseName = Events.getMoonPhaseName(new Date());
      console.log('Current Moon Phase: ' + phaseName +
        ' (index ' + phaseIndex + ')');
      });
      
      


});