
#include <stdio.h>
#include <string.h>
void blink_led(char command[128]) {
	static int led = 0;

	switch (led) {
	case 0:
		strcpy(command,"LED 1");
		led++;
		break;
	case 1:
		strcpy(command,"LED 2");
		led++;
		break;
	case 2:
		strcpy(command,"LED 4");
		led++;
		break;
	case 3:
		strcpy(command,"LED 8");
		led++;
		break;
	case 4:
		strcpy(command,"LED 16");
		led++;
		break;
	case 5:
		strcpy(command,"LED 32");
		led++;
		break;
	case 6:
		strcpy(command,"LED 64");
		led++;
		break;
	case 7:
		strcpy(command,"LED 128");
		led = 0;
		break;
	}
}
