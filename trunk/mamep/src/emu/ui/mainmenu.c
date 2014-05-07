/*********************************************************************

    ui/mainmenu.c

    Internal MAME menus for the user interface.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "emu.h"
#include "osdnet.h"
#include "emuopts.h"
#include "ui/ui.h"
#include "rendutil.h"
#include "cheat.h"
#include "uiinput.h"
#include "ui/filemngr.h"
#include "ui/filesel.h"
#include "ui/barcode.h"
#include "ui/tapectrl.h"
#include "ui/mainmenu.h"
#include "ui/miscmenu.h"
#include "ui/imginfo.h"
#include "ui/selgame.h"
#include "audit.h"
#include "crsshair.h"
#include <ctype.h>
#include "imagedev/cassette.h"
#include "imagedev/bitbngr.h"
#include "machine/bcreader.h"


/***************************************************************************
    MENU HANDLERS
***************************************************************************/

/*-------------------------------------------------
    ui_menu_main constructor - populate the main menu
-------------------------------------------------*/

ui_menu_main::ui_menu_main(running_machine &machine, render_container *container) : ui_menu(machine, container)
{
}

void ui_menu_main::populate()
{
	astring menu_text;

	/* add input menu items */
	item_append(_("Input (general)"), NULL, 0, (void *)INPUT_GROUPS);

	menu_text.printf(_("Input (this %s)"),emulator_info::get_capstartgamenoun());
	item_append(menu_text.cstr(), NULL, 0, (void *)INPUT_SPECIFIC);
#ifdef USE_AUTOFIRE
	item_append(_("Autofire Setting"), NULL, 0, (void *)AUTOFIRE);
#endif /* USE_AUTOFIRE */
#ifdef USE_CUSTOM_BUTTON
	item_append(_("Custom Buttons"), NULL, 0, (void *)CUSTOM_BUTTON);
#endif /* USE_CUSTOM_BUTTON */

	/* add optional input-related menus */
	if (machine().ioport().has_analog())
		item_append(_("Analog Controls"), NULL, 0, (void *)ANALOG);
	if (machine().ioport().has_dips())
		item_append(_("Dip Switches"), NULL, 0, (void *)SETTINGS_DIP_SWITCHES);
	if (machine().ioport().has_configs())
	{
		menu_text.printf(_("%s Configuration"),emulator_info::get_capstartgamenoun());
		item_append(menu_text.cstr(), NULL, 0, (void *)SETTINGS_DRIVER_CONFIG);
	}

	/* add bookkeeping menu */
	item_append(_("Bookkeeping Info"), NULL, 0, (void *)BOOKKEEPING);

	/* add game info menu */
	menu_text.printf(_("%s Information"),emulator_info::get_capstartgamenoun());
	item_append(menu_text.cstr(), NULL, 0, (void *)GAME_INFO);

	image_interface_iterator imgiter(machine().root_device());
	if (imgiter.first() != NULL)
	{
		/* add image info menu */
		item_append(_("Image Information"), NULL, 0, (void *)IMAGE_MENU_IMAGE_INFO);

		/* add file manager menu */
		item_append(_("File Manager"), NULL, 0, (void *)IMAGE_MENU_FILE_MANAGER);

		/* add tape control menu */
		cassette_device_iterator cassiter(machine().root_device());
		if (cassiter.first() != NULL)
			item_append(_("Tape Control"), NULL, 0, (void *)MESS_MENU_TAPE_CONTROL);
	}

	if (machine().ioport().has_bioses())
		item_append(_("Bios Selection"), NULL, 0, (void *)BIOS_SELECTION);

	slot_interface_iterator slotiter(machine().root_device());
	if (slotiter.first() != NULL)
	{
		/* add slot info menu */
		item_append(_("Slot Devices"), NULL, 0, (void *)SLOT_DEVICES);
	}

	barcode_reader_device_iterator bcriter(machine().root_device());
	if (bcriter.first() != NULL)
	{
		/* add slot info menu */
		item_append(_("Barcode Reader"), NULL, 0, (void *)BARCODE_READ);
	}

	network_interface_iterator netiter(machine().root_device());
	if (netiter.first() != NULL)
	{
		/* add image info menu */
		item_append(_("Network Devices"), NULL, 0, (void*)NETWORK_DEVICES);
	}

	/* add keyboard mode menu */
	if (machine().ioport().has_keyboard() && machine().ioport().natkeyboard().can_post())
		item_append(_("Keyboard Mode"), NULL, 0, (void *)KEYBOARD_MODE);

	/* add sliders menu */
	item_append(_("Slider Controls"), NULL, 0, (void *)SLIDERS);

	/* add video options menu */
	item_append(_("Video Options"), NULL, 0, (machine().render().target_by_index(1) != NULL) ? (void *)VIDEO_TARGETS : (void *)VIDEO_OPTIONS);

#ifdef USE_SCALE_EFFECTS
	/* add image enhancement menu */
	item_append(_("Image Enhancement"), NULL, 0, (void *)SCALE_EFFECT);
#endif /* USE_SCALE_EFFECTS */

	/* add crosshair options menu */
	if (crosshair_get_usage(machine()))
		item_append(_("Crosshair Options"), NULL, 0, (void *)CROSSHAIR);

	/* add cheat menu */
	if (machine().options().cheat() && machine().cheat().first() != NULL)
		item_append(_("Cheat"), NULL, 0, (void *)CHEAT);

#ifdef CMD_LIST
	/* add command list menu */
		item_append(_("Show Command List"), NULL, 0, (void *)COMMAND);
#endif /* CMD_LIST */

	/* add reset and exit menus */
	menu_text.printf(_("Select New %s"),emulator_info::get_capstartgamenoun());
	item_append(menu_text.cstr(), NULL, 0, (void *)SELECT_GAME);
}

ui_menu_main::~ui_menu_main()
{
}

/*-------------------------------------------------
    menu_main - handle the main menu
-------------------------------------------------*/

void ui_menu_main::handle()
{
	/* process the menu */
	const ui_menu_event *menu_event = process(0);
	if (menu_event != NULL && menu_event->iptkey == IPT_UI_SELECT) {
		switch((long long)(menu_event->itemref)) {
		case INPUT_GROUPS:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_input_groups(machine(), container)));
			break;

		case INPUT_SPECIFIC:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_input_specific(machine(), container)));
			break;

#ifdef USE_AUTOFIRE
		case AUTOFIRE:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_autofire(machine(), container)));
			break;

#endif /* USE_AUTOFIRE */
#ifdef USE_CUSTOM_BUTTON
		case CUSTOM_BUTTON:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_custom_button(machine(), container)));
			break;

#endif /* USE_CUSTOM_BUTTON */
		case SETTINGS_DIP_SWITCHES:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_settings_dip_switches(machine(), container)));
			break;

		case SETTINGS_DRIVER_CONFIG:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_settings_driver_config(machine(), container)));
			break;

		case ANALOG:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_analog(machine(), container)));
			break;

		case BOOKKEEPING:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_bookkeeping(machine(), container)));
			break;

		case GAME_INFO:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_game_info(machine(), container)));
			break;

		case IMAGE_MENU_IMAGE_INFO:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_image_info(machine(), container)));
			break;

		case IMAGE_MENU_FILE_MANAGER:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_file_manager(machine(), container)));
			break;

		case MESS_MENU_TAPE_CONTROL:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_mess_tape_control(machine(), container, NULL)));
			break;

		case SLOT_DEVICES:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_slot_devices(machine(), container)));
			break;

		case NETWORK_DEVICES:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_network_devices(machine(), container)));
			break;

		case KEYBOARD_MODE:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_keyboard_mode(machine(), container)));
			break;

		case SLIDERS:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_sliders(machine(), container, false)));
			break;

		case VIDEO_TARGETS:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_video_targets(machine(), container)));
			break;

		case VIDEO_OPTIONS:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_video_options(machine(), container, machine().render().first_target())));
			break;

#ifdef USE_SCALE_EFFECTS
		case SCALE_EFFECT:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_scale_effect(machine(), container)));
			break;
#endif /* USE_SCALE_EFFECTS */

		case CROSSHAIR:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_crosshair(machine(), container)));
			break;

		case CHEAT:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_cheat(machine(), container)));
			break;

#ifdef CMD_LIST
		case COMMAND:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_command(machine(), container)));
			break;
#endif /* CMD_LIST */

		case SELECT_GAME:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_select_game(machine(), container, 0)));
			break;

		case BIOS_SELECTION:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_bios_selection(machine(), container)));
			break;

		case BARCODE_READ:
			ui_menu::stack_push(auto_alloc_clear(machine(), ui_menu_barcode_reader(machine(), container)));
			break;

		default:
			fatalerror("ui_menu_main::handle - unknown reference\n");
		}
	}
}
