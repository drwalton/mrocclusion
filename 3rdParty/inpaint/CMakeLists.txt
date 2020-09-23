add_library(inpaint
	gradientcalculator.h
	gradientcalculator.cpp

	inpainter.h
	inpainter.cpp
)

set_target_properties(inpaint PROPERTIES FOLDER 3rdParty)

