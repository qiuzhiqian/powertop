
#include "device.h"
#include <vector>
#include <algorithm>
#include <stdio.h>
using namespace std;

#include "backlight.h"
#include "usb.h"
#include "ahci.h"
#include "alsa.h"
#include "rfkill.h"
#include "i915-gpu.h"
#include "thinkpad-fan.h"

#include "../parameters/parameters.h"
#include "../display.h"

void device::start_measurement(void)
{
}

void device::end_measurement(void)
{
}

double	device::utilization(void)
{
	return 0.0;
}



vector<class device *> all_devices;


void devices_start_measurement(void)
{
	unsigned int i;
	for (i = 0; i < all_devices.size(); i++)
		all_devices[i]->start_measurement();
}

void devices_end_measurement(void)
{
	unsigned int i;
	for (i = 0; i < all_devices.size(); i++)
		all_devices[i]->end_measurement();
}

static bool power_device_sort(class device * i, class device * j)
{
	double pI, pJ;
	pI = i->power_usage(&all_results, &all_parameters);
	pJ = j->power_usage(&all_results, &all_parameters);

	if (pI == pJ) {
		int vI, vJ;
		vI = i->power_valid();
		vJ = j->power_valid();

		if (vI != vJ)
			return vI > vJ;

		return i->utilization() > j->utilization();
	}
        return pI > pJ;
}


void report_devices(void)
{
	WINDOW *win;
	unsigned int i;

	char util[128];
	char power[128];

	win = tab_windows["Device stats"];
        if (!win)
                return;

        wclear(win);
        wmove(win, 2,0);

	sort(all_devices.begin(), all_devices.end(), power_device_sort);

	wprintw(win, "Power est.  Usage     Device\n");
	for (i = 0; i < all_devices.size(); i++) {
		double P;

		util[0] = 0;

		if (all_devices[i]->util_units()) {
			if (all_devices[i]->utilization() < 1000)
				sprintf(util, "%5.1f%s",  all_devices[i]->utilization(),  all_devices[i]->util_units());
			else
				sprintf(util, "%5i%s",  (int)all_devices[i]->utilization(),  all_devices[i]->util_units());
		}
		while (strlen(util) < 9) strcat(util, " ");

		P = all_devices[i]->power_usage(&all_results, &all_parameters);

		if (P > 1.5) 
			sprintf(power, "%7.2fW   ", P);
		else
			sprintf(power, "%6.1f mW  ", P*1000);

		if (!all_devices[i]->power_valid())
			strcpy(power, "           ");


		wprintw(win, "%s %s %s\n", 
			power,
			util,
			all_devices[i]->human_name()
			);
	}
}


void create_all_devices(void)
{
	create_all_backlights();
	create_all_usb_devices();
	create_all_ahcis();
	create_all_alsa();
	create_all_rfkills();
	create_i915_gpu();
	create_thinkpad_fan();
}
