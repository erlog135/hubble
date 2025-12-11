//message processing functions for body/details window
/**
 * BodyPackage:
 * a small 7-byte, 56-bit package, requested on demand per-body:
 * 
 * azimuth (9 bit uint) 0-360 degrees
 * altitude (8 bit int) -90 to 90 degrees
 * 
 * rise hour (5 bit uint) 0-23
 * rise minute (6 bit uint) 0-59
 * 
 * set hour (5 bit uint) 0-23
 * set minute (6 bit uint) 0-59
 * 
 * luminance * 10 (9 bit int) -256 to 255
 * phase (3 bit uint) 0-7 (ignored for anything other than the moon)
 *  
 * 
 * +5 bits free
 */