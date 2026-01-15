/**
 * Pin pusher module - pushes astronomical events to Pebble timeline
 * 
 * This file re-exports from the modular pinpusher/ directory for backward compatibility.
 * 
 * Module structure:
 *   pinpusher/
 *   ├── index.js      - Main orchestrator
 *   ├── constants.js  - PIN_IDS, EVENT_STYLES, getAllPossiblePinIds()
 *   ├── cache.js      - Cache management
 *   ├── dateUtils.js  - Timeline date utilities
 *   ├── pinBuilder.js - Pin construction for all event types
 *   ├── pinManager.js - Settings comparison, pin deletion
 *   └── testPins.js   - Development test utilities
 */

var pinpusher = require('./pinpusher/index');

module.exports = pinpusher;
