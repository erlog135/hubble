/**
 * Centralized logging for pkjs. Set ENABLED to true to see debug output.
 * Usage: var logger = require('./logger'); logger.ENABLED = true;
 */
var logger = {
  ENABLED: false,
  log: function () {
    if (logger.ENABLED && console && console.log) {
      console.log.apply(console, arguments);
    }
  },
  warn: function () {
    if (logger.ENABLED && console && console.warn) {
      console.warn.apply(console, arguments);
    }
  },
  error: function () {
    if (logger.ENABLED && console && console.error) {
      console.error.apply(console, arguments);
    }
  }
};

module.exports = logger;
