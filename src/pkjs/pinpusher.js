var timeline = require('./timeline');


//TODO: replace with my own icons once published media is fixed
const EVENT_STYLES = {
  sunRise: {
    foregroundColor: "#000000",
    backgroundColor: "#FFFF00",
    tinyIcon: "SUNRISE"
  },
  sunSet: {
    foregroundColor: "#000000",
    backgroundColor: "#FFAA00",
    tinyIcon: "SUNSET"
  },
  bodyRise: {
    foregroundColor: "#000000",
    backgroundColor: "#AAAA55",
    tinyIcon: "NOTIFICATION_FLAG"
  },
  bodySet: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#550000",
    tinyIcon: "NOTIFICATION_FLAG"
  },
  moonRise: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA55FF",
    tinyIcon: "NOTIFICATION_FLAG"
  },
  moonSet: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA00FF",
    tinyIcon: "NOTIFICATION_FLAG"
  },
  astronomicalDawn: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA0055",
    tinyIcon: "GENERIC_CONFIRMATION"
  },
  astronomicalDusk: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#AA0055",
    tinyIcon: "GENERIC_CONFIRMATION"
  },
  nauticalDawn: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#FF0055",
    tinyIcon: "NOTIFICATION_LIGHTHOUSE"
  },
  nauticalDusk: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#FF0055",
    tinyIcon: "NOTIFICATION_LIGHTHOUSE"
  },
  civilDawn: {
    foregroundColor: "#000000",
    backgroundColor: "#FF5555",
    tinyIcon: "NOTIFICATION_GENERIC"
  },
  civilDusk: {
    foregroundColor: "#000000",
    backgroundColor: "#FF5555",
    tinyIcon: "NOTIFICATION_GENERIC"
  },
  equinox: {
    foregroundColor: "#000000",
    backgroundColor: "#55FFAA",
    tinyIcon: "TIMELINE_SUN"
  },
  solstice: {
    foregroundColor: "#000000",
    backgroundColor: "#00FFFF",
    tinyIcon: "TIMELINE_SUN"
  },
  eclipse: {
    foregroundColor: "#FFFFFF",
    backgroundColor: "#5555FF",
    tinyIcon: "TIMELINE_SUN"
  },
  transit: {
    foregroundColor: "#000000",
    backgroundColor: "#FFFF00",
    tinyIcon: "TIMELINE_SUN"
  },
  lunarApsis: {
    foregroundColor: "#000000",
    backgroundColor: "#5555FF",
    tinyIcon: "NOTIFICATION_FLAG"
  }
}

// Push a pin when the app starts
function pushTestPin() {
    // An hour ahead
    var date = new Date();
    date.setHours(date.getHours() + 1);
  
    // Create the pin
    var pin = {
      "id": "00-00-test",
      "time": date.toISOString(),
    //   "duration": 32,
     "layout": {
        "type": "genericPin",
        "backgroundColor": "#000000",
        "title": "Test Pin",
        "subtitle": "For Testing",
        "body": "Here is a test pin",
        "tinyIcon": "system://images/NOTIFICATION_FLAG",
        "lastUpdated": new Date().toISOString()
      }
    };
  
    // Push the pin
    timeline.insertUserPin(pin, function(responseText) {
      console.log('Test pin pushed: ' + responseText);
    });
  }

  module.exports.pushTestPin = pushTestPin;