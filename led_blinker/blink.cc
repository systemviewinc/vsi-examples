
#include <stdio.h>
#include <string.h>
void blink_led(char command[128]) {
	static int led = 0;
	
	switch (led) {
	case 0:
		strcpy(command,"LED_0");
		led++;
		break;
	case 1:
		strcpy(command,"LED_1");
		led++;
		break;
	case 2:
		strcpy(command,"LED_2");
		led++;
		break;
	case 3:
		strcpy(command,"LED_3");
		led++;
		break;
	case 4:
		strcpy(command,"LED_4");
		led++;
		break;
	case 5:
		strcpy(command,"LED_5");
		led++;
		break;
	case 6:
		strcpy(command,"LED_6");
		led++;
		break;
	case 7:
		strcpy(command,"LED_7");
		led = 0;
		break;
	}
}
