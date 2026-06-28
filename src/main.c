#include "panel.h"
#include <zephyr/kernel.h>

int main(void)
{
	k_msleep(100);
	return panel_run();
}
