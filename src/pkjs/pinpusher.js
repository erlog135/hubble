var timeline = require('./timeline');

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
        "backgroundColor": "#00AAFF",
        "title": "Test Pin",
        "subtitle": "For Testing",
        "body": "Here is a test pin",
        "tinyIcon": "system://images/SCHEDULED_FLIGHT",
        "lastUpdated": new Date().toISOString()
      }
    };
  
    // Push the pin
    timeline.insertUserPin(pin, function(responseText) {
      console.log('Test pin pushed: ' + responseText);
    });
  }

  module.exports.pushTestPin = pushTestPin;