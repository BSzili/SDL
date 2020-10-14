/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>
  Copyright (C) 2020 Jacek Piszczek

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include "SDL_sysjoystick_c.h"

#ifdef SDL_JOYSTICK_MORPHOS

#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"
#include "SDL_endian.h"
#include "SDL_joystick.h"
#include "SDL_events.h"
#include "gamepadlib.h"

// SDL2 deadzone is around 409, we need 1638
#define DEADZONE_MIN (-0.05)
#define DEADZONE_MAX (0.05)

#define JOYSTICK_MIN -1.0
#define JOYSTICK_MAX 1.0

#define CLAMP(val) \
			(((val) <= (DEADZONE_MAX) && (val) >= (DEADZONE_MIN)) ? (0) : \
			((val) > (JOYSTICK_MAX)) ? (JOYSTICK_MAX) : (((val) < (JOYSTICK_MIN)) ? (JOYSTICK_MIN) : (val)))

static const SDL_JoystickGUID gamepadlibGUID = { { 3, 150, 102 } };
static gmlibHandle *handle;
static BOOL updateNeeded = TRUE;
static BOOL hadJoysticks[gmlibSlotMax];

struct joystick_hwdata { ULONG dummy; };

static int SDL_MorphOS_JoystickGetCount(void)
{
	int count = 0;
	for (int i = 0; i < gmlibSlotMax; i++)
	{
		count += hadJoysticks[i];
	}
	return count;
}

static void SDL_MorphOS_JoystickDetect(void)
{
	// this seems to be called on every frame *after* polling joystick states
	// mark updateNeeded since we'll have to call the gmlib update on the next
	// joystick update - UNLESS updateNeeded is still TRUE

	if (updateNeeded)
	{
		// if it remains true, it means SDL isn't polling any joysticks!
		gmlibUpdate(handle);

		for (ULONG slot = gmlibSlotMin; slot <= gmlibSlotMax; slot++)
		{
			ULONG device_index = slot - 1;

			if (gmlibGetGamepad(handle, slot, NULL))
			{
				if (!hadJoysticks[device_index])
				{
					hadJoysticks[device_index] = TRUE;
					SDL_PrivateJoystickAdded(device_index);
					return;
				}
			}
			else if (hadJoysticks[device_index])
			{
				hadJoysticks[device_index] = FALSE;
				SDL_PrivateJoystickRemoved(device_index);
			}
		}
	}

	updateNeeded = TRUE;
}

/* Function to scan the system for joysticks.
 * It should return 0, or -1 on an unrecoverable fatal error.
 */
static int SDL_MorphOS_JoystickInit(void)
{
	handle = gmlibInitialize("SDL", 0);
	if (handle)
		SDL_MorphOS_JoystickDetect();
	return handle ? 0 : -1;
}

/* Function to get the device-dependent name of a joystick */
static const char *SDL_MorphOS_JoystickGetDeviceName(int device_index)
{
	static gmlibGamepad info;

	if (gmlibGetGamepad(handle, device_index + 1, &info))
	{
		return info._name;
	}

	return NULL;
}

/* Function to perform the mapping from device index to the instance id for this index */
static SDL_JoystickID SDL_MorphOS_JoystickGetDeviceInstanceID(int device_index)
{
	return device_index;
}

/* Function to open a joystick for use.
   The joystick to open is specified by the index field of the joystick.
   This should fill the nbuttons and naxes fields of the joystick structure.
   It returns 0, or -1 if there is an error.
 */
static int SDL_MorphOS_JoystickOpen(SDL_Joystick * joystick, int device_index)
{
	if (gmlibGetGamepad(handle, device_index + 1, NULL))
	{
		// gmlib has fixed counts there
		joystick->nhats = 0;
		joystick->naxes = 6;
		joystick->nbuttons = 14;
		joystick->hwdata = (struct joystick_hwdata *)(device_index + 1);
		return 0;
	}
	
	return -1;
}

static void SDL_MorphOS_JoystickUpdate(SDL_Joystick * joystick)
{
	ULONG slot = (ULONG)joystick->hwdata;

	if (slot < gmlibSlotMin || slot > gmlibSlotMax)
		return;
	
	ULONG device_index = slot - 1;
	
	if (updateNeeded)
		gmlibUpdate(handle);

	if (gmlibGetGamepad(handle, slot, NULL))
	{
		if (!hadJoysticks[device_index])
		{
			hadJoysticks[device_index] = TRUE;
			SDL_PrivateJoystickAdded(device_index);
			return;
		}
		
		gmlibGamepadData data;
		gmlibGetData(handle, slot, &data);

		#define PRESSED(x) (x ? SDL_PRESSED : SDL_RELEASED)
		
		SDL_PrivateJoystickButton(joystick, 0, PRESSED(data._buttons._bits._dpadLeft));
		SDL_PrivateJoystickButton(joystick, 1, PRESSED(data._buttons._bits._dpadRight));
		SDL_PrivateJoystickButton(joystick, 2, PRESSED(data._buttons._bits._dpadUp));
		SDL_PrivateJoystickButton(joystick, 3, PRESSED(data._buttons._bits._dpadDown));
		SDL_PrivateJoystickButton(joystick, 4, PRESSED(data._buttons._bits._back));
		SDL_PrivateJoystickButton(joystick, 5, PRESSED(data._buttons._bits._start));
		SDL_PrivateJoystickButton(joystick, 6, PRESSED(data._buttons._bits._leftStickButton));
		SDL_PrivateJoystickButton(joystick, 7, PRESSED(data._buttons._bits._rightStickButton));
		SDL_PrivateJoystickButton(joystick, 8, PRESSED(data._buttons._bits._xLeft));
		SDL_PrivateJoystickButton(joystick, 9, PRESSED(data._buttons._bits._yTop));
		SDL_PrivateJoystickButton(joystick,10, PRESSED(data._buttons._bits._aBottom));
		SDL_PrivateJoystickButton(joystick,11, PRESSED(data._buttons._bits._bRight));
		SDL_PrivateJoystickButton(joystick,12, PRESSED(data._buttons._bits._shoulderLeft));
		SDL_PrivateJoystickButton(joystick,13, PRESSED(data._buttons._bits._shoulderRight));

		SDL_PrivateJoystickAxis(joystick, 0, (Sint16)(CLAMP(data._leftStick._eastWest) * SDL_JOYSTICK_AXIS_MAX));
		SDL_PrivateJoystickAxis(joystick, 1, (Sint16)(CLAMP(data._leftStick._northSouth) * SDL_JOYSTICK_AXIS_MAX));

		SDL_PrivateJoystickAxis(joystick, 2, (Sint16)(CLAMP(data._rightStick._eastWest) * SDL_JOYSTICK_AXIS_MAX));
		SDL_PrivateJoystickAxis(joystick, 3, (Sint16)(CLAMP(data._rightStick._northSouth) * SDL_JOYSTICK_AXIS_MAX));

		SDL_PrivateJoystickAxis(joystick, 4, (Sint16)(CLAMP(data._leftTrigger) * SDL_JOYSTICK_AXIS_MAX));
		SDL_PrivateJoystickAxis(joystick, 5, (Sint16)(CLAMP(data._rightTrigger) * SDL_JOYSTICK_AXIS_MAX));
		
		SDL_JoystickPowerLevel ePowerLevel = SDL_JOYSTICK_POWER_UNKNOWN;
		ULONG level = data._battery * 100;

		switch (level)
		{
		case 1 ... 5:
			ePowerLevel = SDL_JOYSTICK_POWER_EMPTY;
			break;
		case 6 ... 20:
			ePowerLevel = SDL_JOYSTICK_POWER_LOW;
			break;
		case 21 ... 70:
			ePowerLevel = SDL_JOYSTICK_POWER_MEDIUM;
			break;
		case 71 ... 100:
			ePowerLevel = SDL_JOYSTICK_POWER_FULL;
			break;
		}
		if (level != SDL_JOYSTICK_POWER_UNKNOWN)
			SDL_PrivateJoystickBatteryLevel(joystick, ePowerLevel);
	}
	else if (hadJoysticks[device_index])
	{
		hadJoysticks[device_index] = FALSE;
		SDL_PrivateJoystickRemoved(device_index);
		return;
	}
}

/* Function to close a joystick after use */
void SDL_MorphOS_JoystickClose(SDL_Joystick * joystick)
{
	D("[%s]\n", __FUNCTION__);
	joystick->hwdata = NULL;
}

/* Function to perform any system-specific joystick related cleanup */
static void SDL_MorphOS_JoystickQuit(void)
{
	D("[%s]\n", __FUNCTION__);
	gmlibShutdown(handle);
	handle = NULL;
}

static SDL_JoystickGUID SDL_MorphOS_JoystickGetDeviceGUID( int device_index )
{
	return gamepadlibGUID;
}

/*
 Rumble experimental
 Add duration in function, impossible to stop rumble in progress, so SDL2 can't stop it
*/
static int SDL_MorphOS_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble, Uint32 duration_ms)
{
	ULONG slot = (ULONG)joystick->hwdata;
	DOUBLE lpower=(DOUBLE)(low_frequency_rumble/65535), hpower=(DOUBLE)(high_frequency_rumble/65535);
	ULONG duration = duration_ms;
	gmlibSetRumble(handle, slot, lpower, hpower, duration);
    return 0;
}

static int SDL_MorphOS_JoystickGetDevicePlayerIndex(int device_index)
{
    return device_index;
}

static void SDL_MorphOS_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

SDL_JoystickDriver SDL_MORPHOS_JoystickDriver =
{
    SDL_MorphOS_JoystickInit,
    SDL_MorphOS_JoystickGetCount,
    SDL_MorphOS_JoystickDetect,
    SDL_MorphOS_JoystickGetDeviceName,
    SDL_MorphOS_JoystickGetDevicePlayerIndex,
    SDL_MorphOS_JoystickSetDevicePlayerIndex,
    SDL_MorphOS_JoystickGetDeviceGUID,
    SDL_MorphOS_JoystickGetDeviceInstanceID,
    SDL_MorphOS_JoystickOpen,
    SDL_MorphOS_JoystickRumble,
    SDL_MorphOS_JoystickUpdate,
    SDL_MorphOS_JoystickClose,
    SDL_MorphOS_JoystickQuit,
};

#endif
