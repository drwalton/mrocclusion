add_library(poly2tri
	poly2tri.h
	common/shapes.cc
	common/shapes.h
	common/utils.h
	sweep/advancing_front.cc
	sweep/advancing_front.h
	sweep/cdt.cc
	sweep/cdt.h
	sweep/sweep.cc
	sweep/sweep.h
	sweep/sweep_context.cc
	sweep/sweep_context.h
)

set_target_properties(poly2tri PROPERTIES FOLDER 3rdParty)
