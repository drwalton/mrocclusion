#include "glue/MessageBox.hpp"
#include <SDL.h>

namespace glue {

void showMessageBoxOk(
	const std::string &title,
	const std::string &message)
{
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK" },
	};
	const SDL_MessageBoxColorScheme colorScheme = {
		{ /* .colors (.r, .g, .b) */
			/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
			{ 255, 0, 0 },
			/* [SDL_MESSAGEBOX_COLOR_TEXT] */
			{ 0, 255, 0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
			{ 255, 255, 0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
			{ 0, 0, 255 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
			{ 255, 0, 255 }
		}
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION, /* .flags */
		NULL, /* .window */
		title.c_str(),
		message.c_str(),
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&colorScheme /* .colorScheme */
	};
	int buttonid;
	SDL_ShowMessageBox(&messageboxdata, &buttonid);
}

bool showMessageBoxRetryCancel(
	const std::string &title,
	const std::string &message)
{
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Retry" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Cancel" },
	};
	const SDL_MessageBoxColorScheme colorScheme = {
		{ /* .colors (.r, .g, .b) */
			/* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
			{ 255, 0, 0 },
			/* [SDL_MESSAGEBOX_COLOR_TEXT] */
			{ 0, 255, 0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
			{ 255, 255, 0 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
			{ 0, 0, 255 },
			/* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
			{ 255, 0, 255 }
		}
	};
	const SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION, /* .flags */
		NULL, /* .window */
		title.c_str(),
		message.c_str(),
		SDL_arraysize(buttons), /* .numbuttons */
		buttons, /* .buttons */
		&colorScheme /* .colorScheme */
	};
	int buttonid;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonid) < 0 ||
		buttonid == -1 || buttonid == 1) {
		return false;
	}

	return true;
}

}
