// Constellation RA/Dec coordinates (decimal degrees)
// RA: 0-360 degrees, Dec: -90 to 90 degrees
// Order matches BODY_NAMES in msgproc.js

var CONSTELLATION_COORDS = [
  // Zodiac constellations
  { ra: 39.75, dec: 20.8 },      // Aries
  { ra: 70.5, dec: 15.8 },       // Taurus
  { ra: 105.0, dec: 22.5 },      // Gemini
  { ra: 129.75, dec: 20.0 },     // Cancer
  { ra: 160.05, dec: 15.0 },     // Leo
  { ra: 201.3, dec: -3.0 },      // Virgo
  { ra: 228.0, dec: -15.5 },     // Libra
  { ra: 253.05, dec: -26.5 },    // Scorpius
  { ra: 285.0, dec: -25.0 },     // Sagittarius
  { ra: 315.0, dec: -18.0 },     // Capricornus
  { ra: 334.5, dec: -10.5 },     // Aquarius
  { ra: 7.5, dec: 10.0 },        // Pisces
  
  // Other notable constellations
  { ra: 83.7, dec: 0.0 },        // Orion
  { ra: 165.0, dec: 50.0 },      // Ursa Major
  { ra: 225.0, dec: 75.0 },      // Ursa Minor
  { ra: 15.0, dec: 60.0 },       // Cassiopeia
  { ra: 309.0, dec: 42.0 },      // Cygnus
  { ra: 187.5, dec: -60.0 },     // Crux
  { ra: 283.5, dec: 38.5 }       // Lyra
];

module.exports = {
  CONSTELLATION_COORDS: CONSTELLATION_COORDS
};
