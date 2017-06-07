
#include <stdio.h>
#include <string.h>
#include <vsi_device.h>
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

void drive_led(char command[128], vsi::device &led)
{
	unsigned int led_val = 0;
	if (strcmp(command,"LED_0") == 0) {
		led_val = 1;
	} else 	if (strcmp(command,"LED_1") == 0) {
		led_val = 2;
	} else 	if (strcmp(command,"LED_2") == 0) {
		led_val = 4;
	} else 	if (strcmp(command,"LED_3") == 0) {
		led_val = 8;
	} else 	if (strcmp(command,"LED_4") == 0) {
		led_val = 16;
	} else	if (strcmp(command,"LED_5") == 0) {
		led_val = 32;
	} else 	if (strcmp(command,"LED_6") == 0) {
		led_val = 64;
	} else 	if (strcmp(command,"LED_7") == 0) {
		led_val = 128;
	}
	led.pwrite(&led_val,sizeof(led_val),0);
}
