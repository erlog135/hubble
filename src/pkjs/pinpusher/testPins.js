/**
 * Test pin utilities for development
 */

var timeline = require('../timeline');
var pinBuilder = require('./pinBuilder');

/**
 * Push a test pin to the timeline (for development testing)
 */
function pushTestPin() {
  // An hour ahead
  var date = new Date();
  date.setHours(date.getHours() + 1);

  // Create the pin
  var pin = {
    "id": "00-00-test",
    "time": date.toISOString(),
    "layout": {
      "type": "genericPin",
      "backgroundColor": "#000000",
      "title": "Test Pin",
      "subtitle": "For Testing",
      "body": "Here is a test pin with random numbers: " + Math.random().toString(),
      "tinyIcon": "system://images/NOTIFICATION_FLAG",
      "lastUpdated": pinBuilder.generateTimestamp()
    },
    "actions": [
      {
        "title": "Open App",
        "type": "openWatchApp",
        "launchCode": 10
      }
    ]
  };

  console.log("ISO: " + date.toISOString());
  console.log("timestamp: " + pinBuilder.generateTimestamp());

  // Push the pin
  timeline.insertUserPin(pin, function(responseText) {
    console.log('Test pin pushed: ' + responseText);
  });
}

/**
 * Delete the test pin from the timeline
 */
function deleteTestPin() {
  timeline.deleteUserPin({
    "id": "00-00-test"
  }, function(responseText) {
    console.log('Test pin deleted: ' + responseText);
  });
}

module.exports = {
  pushTestPin: pushTestPin,
  deleteTestPin: deleteTestPin
};
