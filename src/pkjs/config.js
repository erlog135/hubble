module.exports = [
  {"type":"heading", "defaultValue": "Timeline Events"},
  {
    
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Sun Events"
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_ASTRONOMICAL_DAWN_DUSK",
        "label": "Astronomical Dawn & Dusk",
        "defaultValue": false,
        "description": "Sun is 18° below horizon."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_NAUTICAL_DAWN_DUSK",
        "label": "Nautical Dawn & Dusk",
        "defaultValue": false,
        "description": "Sun is 12° below horizon."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_CIVIL_DAWN_DUSK",
        "label": "Civil Dawn & Dusk",
        "defaultValue": false,
        "description": "Sun is 6° below horizon."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_RISE_SET",
        "label": "Sunrise & Sunset",
        "defaultValue": false,
        "description": "Sun crosses the horizon."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_SOLAR_NOON_MIDNIGHT",
        "label": "Solar Noon & Midnight",
        "defaultValue": false,
        "description": "Sun at highest point (noon) and lowest point (midnight) in the sky."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_SOLSTICES",
        "label": "Solstices",
        "defaultValue": false,
        "description": "Days with the longest and shortest daylight of the year."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_EQUINOXES",
        "label": "Equinoxes",
        "defaultValue": true,
        "description": "Days with equal daylight and darkness."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_ECLIPSES",
        "label": "Eclipses",
        "defaultValue": true,
        "description": "Moon passes between Earth and Sun, or Earth passes between Moon and Sun. Location, type, and peak of eclipse will be provided."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_SUN_SOLAR_TRANSITS",
        "label": "Solar Transits",
        "defaultValue": true,
        "description": "Mercury or Venus passes in front of the Sun."
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Moon Events"
      },
      {
        "type": "toggle",
        "messageKey": "CFG_MOON_RISE_SET",
        "label": "Moonrise & Moonset",
        "defaultValue": true,
        "description": "Moon crosses the horizon. Moon phase will also be provided."
      },
      {
        "type": "toggle",
        "messageKey": "CFG_MOON_APOGEE_PERIGEE",
        "label": "Lunar Apogee & Perigee",
        "defaultValue": true,
        "description": "Moon at farthest (apogee) and closest (perigee) points in its orbit around Earth."
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Planet Events"
      },
      {
        "type": "checkboxgroup",
        "messageKey": "CFG_PLANET_EVENTS",
        "label": "Rise & Set Events",
        "defaultValue": [false, false, false, false, false, false, false, false],
        "options": [
          "Mercury",
          "Venus",
          "Mars",
          "Jupiter",
          "Saturn",
          "Uranus",
          "Neptune",
          "Pluto"
        ]
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];