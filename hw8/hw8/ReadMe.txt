usage: ./pr01 'filename' 'filename' 'method'
Usable Commands:
./pr01 02.PPM ct.PPM subtraction
./pr01 02.PPM ct.PPM min
./pr01 02.PPM ct.PPM max
./pr01 02.PPM ct.PPM multiply
./pr01 02.PPM ct.PPM over

Green Screen:
usage: ./pr01 'filename' 'filename' 'greenscreen' 'parameter1' 'parameter2'

parameter1 is for half kernel size, kernel size= 2 * parameter1 + 1

parameter2 is for the distance threshold when using average foreground color. When it gets bigger, more foreground average will be used. When it is zero, all uncertain pixels will use its original color.

Usable Command:
./pr01 chromokey.ppm explosions.ppm greenscreen 11 150