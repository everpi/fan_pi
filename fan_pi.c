/*  
 *  fan_pi Fan controller for Raspberry Pi
 *
 *  Copyright (C) 2015 EverPi - everpi[at]tsar[dot]in
 *  blog.everpi.net 
 * 
 *  This file is part of fan_pi.
 *
 *  fan_pi is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published b
y
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  fan_pi is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with fan_pi.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <wiringPi.h>

#define HELP \
        "\n  \033[1mFan Controller by blog.everpi.net\n\n" \
        "\tUsage: [TEMP_MIN] [TEMP_RANGE] [option]\n" \
        "\t         -d\tdaemon\033[0m\n\n"

#define SYSFS_THERMAL "/sys/class/thermal/thermal_zone0/temp"

#define PWM_MIN 500 // max 1024

int little_atoi(char *par){
	
	if(par[1]){	
		return ( (par[0] - 48)*10 + (par[1] - 48) );
	}else   return (par[0] - 48);
}

int get_soc_temp_sysfs();

int main(int argc, char *argv[]){
	
	int temp = 0;
	int pwm = PWM_MIN;	
	int celsius_step = 0;
	int TEMP_MIN = 0;
	int TEMP_RANGE = 0;

        if(argc < 3){
    
            printf("%s",HELP);
            return 0;
    	}

	if(argc == 4 && argv[3][1] == 'd') daemon(0,0);

	errno = 0;	

	TEMP_MIN = little_atoi(argv[1]);
	TEMP_RANGE = little_atoi(argv[2]);

	printf("TEMP_MIN:%d - TEMP_RANGE:%d\n",TEMP_MIN,TEMP_RANGE);

	if(TEMP_MIN >= 85 || 
	   TEMP_RANGE > (85-TEMP_MIN)){
		fprintf(stdout,"Out of range\n");
		return -1;
	} 
	
  	wiringPiSetup();
  	pinMode(1,PWM_OUTPUT);	
	
	setuid(65534);
	setgid(65534);
	
	pwmWrite(1,pwm);	

	celsius_step = ((1024-PWM_MIN)/(TEMP_RANGE));
	
	while(1){
		
		temp = get_soc_temp_sysfs();
		
		if(temp >= TEMP_MIN){
		
			pwm = PWM_MIN+(((temp)-TEMP_MIN)*celsius_step);
			
			if(pwm > 1024) pwm = 1024;	

		}else pwm = 0;
                
		pwmWrite(1,pwm);
			
		
                if(argc < 4){
                        printf("Temp: %dC\nPwm: %d\n\n",temp,pwm);
                }
		
		sleep(1);
	
	}

}

int get_soc_temp_sysfs(){

	static int fd = 0;
	int er = 0;
	char temp[8] = {0};

	if(!fd){

		fd = open(SYSFS_THERMAL, O_RDONLY);
		
		if(fd == -1){
			fprintf(stderr,"%s\n",strerror(errno));
			return -1;
		}
	}
	
	er = read(fd, temp, 7);
	
	if(er == -1){
		fprintf(stderr,"%s\n",strerror(errno));	
		return -1;
	}
	lseek(fd,0,SEEK_SET);	

	return strtol(temp,NULL,10)/1000;
}
