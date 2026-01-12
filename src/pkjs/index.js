var Observer = require('./astronomy/observer');
var Bodies = require('./astronomy/bodies');
var Events = require('./astronomy/events');
var MsgProc = require('./msgproc');
var Declination = require('./declination');
var PinPusher = require('./pinpusher');
var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var Keys = require('message_keys');
var clay = new Clay(clayConfig);

var activeObserver = null;

// Handle messages from the watch - single listener for all message types
Pebble.addEventListener('appmessage', function(e) {
  var payload = e && e.payload ? e.payload : {};
  console.log('Received payload: ' + JSON.stringify(payload));

  // Try body request handler first (if observer is available)
  if (activeObserver) {
    var handled = MsgProc.registerBodyRequestHandler(function() { return activeObserver; })(payload);
    if (handled) {
      return; // Body request was handled
    }
  }

  // Handle declination request
  if (payload.hasOwnProperty("REQUEST_DECLINATION")) {
    console.log('Received REQUEST_DECLINATION');
    
    if (!activeObserver) {
      console.log('No active observer, cannot calculate declination');
      return;
    }
    
    try {
      var declination = Declination.getMagneticDeclination(activeObserver);
      if (declination !== null) {
        Declination.sendMagneticDeclination(declination);
      } else {
        console.log('Failed to calculate magnetic declination');
      }
    } catch (error) {
      console.log('Error handling declination request:', error);
    }
    return;
  }

  // Handle events refresh
  if (payload.hasOwnProperty("REQUEST_EVENTS_REFRESH")) {
    console.log('Received REQUEST_EVENTS_REFRESH');

    if (!activeObserver) {
      console.log('No active observer, sending error');
      Pebble.sendAppMessage(
        { 'EVENTS_REFRESHED': -1 },
        function() {
          console.log('Sent events refresh error response');
        },
        function(err) {
          console.log('Failed to send events refresh error: ' + JSON.stringify(err));
        }
      );
      return;
    }

    // Get current clay settings
    var claySettingsString = localStorage.getItem('clay-settings');
    var claySettings = {};
    if (claySettingsString) {
      try {
        claySettings = JSON.parse(claySettingsString);
      } catch (e) {
        console.log('Error parsing clay settings:', e);
        Pebble.sendAppMessage(
          { 'EVENTS_REFRESHED': -1 },
          function() {
            console.log('Sent events refresh error response');
          },
          function(err) {
            console.log('Failed to send events refresh error: ' + JSON.stringify(err));
          }
        );
        return;
      }
    }

    try {
      // Push astronomy events and get count
      var eventCount = PinPusher.pushAstronomyEvents(activeObserver, new Date(), claySettings);
      console.log('Pushed ' + eventCount + ' events to timeline');
      Pebble.sendAppMessage(
        { 'EVENTS_REFRESHED': eventCount },
        function() {
          console.log('Sent events refresh success response with count: ' + eventCount);
        },
        function(err) {
          console.log('Failed to send events refresh success: ' + JSON.stringify(err));
        }
      );
    } catch (error) {
      console.log('Error pushing events:', error);
      Pebble.sendAppMessage(
        { 'EVENTS_REFRESHED': -1 },
        function() {
          console.log('Sent events refresh error response');
        },
        function(err) {
          console.log('Failed to send events refresh error: ' + JSON.stringify(err));
        }
      );
    }
  }
});

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');

  // Get current clay settings
  var claySettingsString = localStorage.getItem('clay-settings');
  var claySettings = {};
  if (claySettingsString) {
    try {
      claySettings = JSON.parse(claySettingsString);
    } catch (e) {
      console.log('Error parsing clay settings:', e);
    }
  }
  console.log('Current clay settings:', JSON.stringify(claySettings));

  PinPusher.pushTestPin();
  // PinPusher.deleteTestPin();

  Observer.initObserver().then(function(observer) {
    activeObserver = observer;
    console.log('Observer ready (lat=' + observer.latitude +
      ', lon=' + observer.longitude + ', h=' + observer.height + ')');

      // Body request handler is now registered dynamically in the appmessage listener
      console.log('Body request handler ready');

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