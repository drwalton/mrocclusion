#ifndef GLUE_MESSAGEBOX_HPP_INCLUDED
#define GLUE_MESSAGEBOX_HPP_INCLUDED

#include <string>

namespace glue {

//!\brief Shows a message box, with an OK button.
//!\note This function will not return until the button is pressed.
void showMessageBoxOk(
	const std::string &title,
	const std::string &message);

//!\brief Show a message box with retry and cancel options.
//!\note This function will not return until a button is pressed.
//!\return True if the user chooses to retry. False if any other
//!        choice is made (cancel, or hits cross to close box).
bool showMessageBoxRetryCancel(
	const std::string &title,
	const std::string &message);

}

#endif
