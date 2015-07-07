all:
	gcc fan_pi.c -o fan_pi -lwiringPi

install:
	cp ./fan_pi /usr/local/bin
