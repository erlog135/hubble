/**
 * Cache management for pin pushing to avoid unnecessary recalculations
 */

// Persistent storage keys (separate from Clay's 'clay-settings')
var STORAGE_KEY_LAST_SETTINGS = 'pinpusher-last-pin-push-settings';
var STORAGE_KEY_LAST_TIME = 'pinpusher-last-pin-push-time';

// Cache state
var lastPinPushTime = 0;
var lastPinPushSettings = null;
var PIN_PUSH_CACHE_DURATION_MS = 30 * 60 * 1000; // 30 minutes

function safeLocalStorageGet(key) {
  try {
    if (typeof localStorage === 'undefined' || !localStorage) return null;
    return localStorage.getItem(key);
  } catch (e) {
    console.log('Pinpusher cache: localStorage.getItem failed for', key, e);
    return null;
  }
}

function safeLocalStorageSet(key, value) {
  try {
    if (typeof localStorage === 'undefined' || !localStorage) return false;
    localStorage.setItem(key, value);
    return true;
  } catch (e) {
    console.log('Pinpusher cache: localStorage.setItem failed for', key, e);
    return false;
  }
}

function safeLocalStorageRemove(key) {
  try {
    if (typeof localStorage === 'undefined' || !localStorage) return false;
    localStorage.removeItem(key);
    return true;
  } catch (e) {
    console.log('Pinpusher cache: localStorage.removeItem failed for', key, e);
    return false;
  }
}

/**
 * Hydrate in-memory cache from localStorage (best effort).
 */
function hydrateCacheFromStorage() {
  var storedTime = safeLocalStorageGet(STORAGE_KEY_LAST_TIME);
  if (storedTime) {
    var parsedTime = parseInt(storedTime, 10);
    if (!isNaN(parsedTime) && parsedTime > 0) {
      lastPinPushTime = parsedTime;
    }
  }

  var storedSettings = safeLocalStorageGet(STORAGE_KEY_LAST_SETTINGS);
  if (storedSettings) {
    try {
      var parsedSettings = JSON.parse(storedSettings);
      if (parsedSettings && typeof parsedSettings === 'object') {
        lastPinPushSettings = parsedSettings;
      }
    } catch (e) {
      console.log('Pinpusher cache: failed to parse stored settings', e);
    }
  }

  if (lastPinPushTime || lastPinPushSettings) {
    console.log('Pinpusher cache hydrated from localStorage (time=' + lastPinPushTime + ')');
  }
}

/**
 * Compare two settings objects for equality
 * @param {Object} settings1 - First settings object
 * @param {Object} settings2 - Second settings object
 * @returns {boolean} True if settings are equal
 */
function areSettingsEqual(settings1, settings2) {
  if (!settings1 && !settings2) return true;
  if (!settings1 || !settings2) return false;

  var keys1 = Object.keys(settings1).sort();
  var keys2 = Object.keys(settings2).sort();

  // Check if same number of keys
  if (keys1.length !== keys2.length) return false;

  // Check if all keys match
  for (var i = 0; i < keys1.length; i++) {
    if (keys1[i] !== keys2[i]) return false;
  }

  // Check if all values match
  for (var j = 0; j < keys1.length; j++) {
    var key = keys1[j];
    var value1 = settings1[key];
    var value2 = settings2[key];

    if (Array.isArray(value1) && Array.isArray(value2)) {
      if (value1.length !== value2.length) return false;
      for (var k = 0; k < value1.length; k++) {
        if (value1[k] !== value2[k]) return false;
      }
    } else if (value1 !== value2) {
      return false;
    }
  }

  return true;
}

/**
 * Check if pin pushing should be skipped based on cache
 * @param {Object} settings - Current clay settings
 * @returns {Object} Object with skip flag: {shouldSkip: boolean}
 */
function shouldSkipPinPush(settings) {
  var now = new Date().getTime();
  var settingsUnchanged = areSettingsEqual(lastPinPushSettings, settings);

  console.log('Cache check - Settings unchanged:', settingsUnchanged);
  console.log('Cache check - Time since last push:', (now - lastPinPushTime) / 1000, 'seconds');
  console.log('Cache check - Cache duration:', PIN_PUSH_CACHE_DURATION_MS / 1000, 'seconds');

  // Check if settings haven't changed and cache is still valid
  if (settingsUnchanged && (now - lastPinPushTime) < PIN_PUSH_CACHE_DURATION_MS) {
    console.log('Skipping pin push - cache still valid (settings unchanged, < 30min old)');
    return { shouldSkip: true };
  }

  console.log('Proceeding with pin push - cache expired or settings changed');
  return { shouldSkip: false };
}

/**
 * Update the pin push cache after successful calculation
 * @param {Object} settings - Clay settings used for calculation
 */
function updatePinPushCache(settings) {
  lastPinPushTime = new Date().getTime();
  lastPinPushSettings = JSON.parse(JSON.stringify(settings)); // Deep copy
  console.log('Updated pin push cache at:', new Date(lastPinPushTime).toISOString());

  // Persist for comparisons across PKJS restarts
  safeLocalStorageSet(STORAGE_KEY_LAST_TIME, String(lastPinPushTime));
  try {
    safeLocalStorageSet(STORAGE_KEY_LAST_SETTINGS, JSON.stringify(lastPinPushSettings));
  } catch (e) {
    console.log('Pinpusher cache: failed to stringify settings for storage', e);
  }
}

/**
 * Get the last cached settings (for comparison)
 * @returns {Object|null} Last cached settings or null
 */
function getLastSettings() {
  return lastPinPushSettings;
}

/**
 * Reset the cache (useful for testing or forced refresh)
 */
function resetCache() {
  lastPinPushTime = 0;
  lastPinPushSettings = null;
  console.log('Pin push cache reset');

  safeLocalStorageRemove(STORAGE_KEY_LAST_TIME);
  safeLocalStorageRemove(STORAGE_KEY_LAST_SETTINGS);
}

// Best-effort hydration at module load time
hydrateCacheFromStorage();

module.exports = {
  areSettingsEqual: areSettingsEqual,
  shouldSkipPinPush: shouldSkipPinPush,
  updatePinPushCache: updatePinPushCache,
  getLastSettings: getLastSettings,
  resetCache: resetCache
};
