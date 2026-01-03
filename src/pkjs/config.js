module.exports = [
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Favorites"
      },
      {
        "type": "text",
        "defaultValue": "Check items to be displayed in a favorites list in the app."
      },
      {
        "type": "checkboxgroup",
        "messageKey": "CFG_FAVORITES",
        "label": "Favorites",
        "defaultValue": [false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false],
        "options": [
          "Moon",
          "Mercury",
          "Venus",
          "Mars",
          "Jupiter",
          "Saturn",
          "Uranus",
          "Neptune",
          "Pluto",
          "Sun",
          "Aries",
          "Taurus",
          "Gemini",
          "Cancer",
          "Leo",
          "Virgo",
          "Libra",
          "Scorpius",
          "Sagittarius",
          "Capricornus",
          "Aquarius",
          "Pisces",
          "Orion",
          "Ursa Major",
          "Ursa Minor",
          "Cassiopeia",
          "Cygnus",
          "Crux",
          "Lyra"
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Timeline Events"
      },
      {
        "type": "text",
        "defaultValue": "If items are favorited, checked events will be shown in the timeline."
      },
      {
        "type": "checkboxgroup",
        "messageKey": "CFG_SUN_EVENTS",
        "label": "Sun Events",
        "defaultValue": [true, false, false, false, true, true, true, true],
        "options": [
          "Astronomical Dawn & Dusk",
          "Nautical Dawn & Dusk",
          "Civil Dawn & Dusk",
          "Rise & Set",
          "Solstices",
          "Equinoxes",
          "Eclipses",
          "Solar Transits"
        ]
      },
      {
        "type": "checkboxgroup",
        "messageKey": "CFG_MOON_EVENTS",
        "label": "Moon Events",
        "defaultValue": [true, true],
        "options": [
          "Rise & Set",
          "Lunar Apogee & Perigee"
        ]
      },
      {
        "type": "checkboxgroup",
        "messageKey": "CFG_PLANET_EVENTS",
        "label": "Planet Rise & Set Events",
        "defaultValue": [true, true, true, true, true, true, true, true],
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