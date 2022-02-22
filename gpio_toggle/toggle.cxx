#include <string.h>

static int toggled = 0;

void toggle_this(char out1[256/sizeof(char)]) {
	if(toggled) {
		strcpy(out1, "SET_HIGH");
		toggled = 0;
	} else {
		strcpy(out1, "SET_LOW");
		toggled = 1;
	}
}
