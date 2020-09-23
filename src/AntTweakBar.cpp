#include "mrocclusion/AntTweakBar.hpp"
#include <AntTweakBar.h>
#include <sstream>

struct AntTweakBar::Impl {
	Impl(GLWindow *window, const std::string &name)
	{
		TwInit(TW_OPENGL_CORE, nullptr);
		int w, h;
		window->size(&w, &h);
		TwWindowSize(w, h);
		bar = TwNewBar(name.c_str());
	}
	~Impl() throw()
	{
		TwTerminate();
	}

	void draw()
	{
		TwDraw();
	}

	bool processEvent(const SDL_Event &e)
	{
		return TwEventSDL(&e, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);
	}

	void addVarRW(const std::string &name, int *val,
		const std::string &label, 
		int min, int max,
		const std::string &help)
	{
		std::string def = makeDef(label, help, min, max);
		TwAddVarRW(bar, name.c_str(), TW_TYPE_INT32, val, def.c_str());
	}

	void addVarRO(const std::string &name, const int *val,
		const std::string &label, const std::string &help = "")
	{
		TwAddVarRO(bar, name.c_str(), TW_TYPE_INT32, val, makeDef(label, help).c_str());
	}

	void addVarRW(const std::string &name, float *val,
		const std::string &label, float min, float max, float step, const std::string &help = "")
	{
		std::string def = makeDef(label, help, min, max, step);
		TwAddVarRW(bar, name.c_str(), TW_TYPE_FLOAT, val, def.c_str());
	}

	void addVarRO(const std::string &name, const float *val,
		const std::string &label, const std::string &help = "")
	{
		TwAddVarRO(bar, name.c_str(), TW_TYPE_FLOAT, val, makeDef(label, help).c_str());
	}

	void addVarRW(const std::string &name, bool *val,
		const std::string &label, const std::string &help)
	{
		std::string def = makeDef(label, help);
		def += " true='true' false='false'";
		TwAddVarRW(bar, name.c_str(), TW_TYPE_BOOLCPP, val, def.c_str());
	}

	void addVarRO(const std::string &name, const bool *val,
		const std::string &label, const std::string &help)
	{
		std::string def = makeDef(label, help);
		def += " true='true' false='false'";
		TwAddVarRO(bar, name.c_str(), TW_TYPE_BOOLCPP, val, def.c_str());
	}

	void addColorRW(const std::string &name, vec3 *val,
		const std::string &label, const std::string &help)
	{
		std::string def = makeDef(label, help);
		TwAddVarRW(bar, name.c_str(), TW_TYPE_COLOR3F, val, def.c_str());
	}

	void addColorRO(const std::string &name, const vec3 *val,
		const std::string &label, const std::string &help)
	{
		std::string def = makeDef(label, help);
		TwAddVarRO(bar, name.c_str(), TW_TYPE_COLOR3F, val, def.c_str());
	}

	void addDirRW(const std::string &name, vec3 *val,
		const std::string &label, const std::string &help)
	{
		std::string def = makeDef(label, help);
		TwAddVarRW(bar, name.c_str(), TW_TYPE_DIR3F, val, def.c_str());
	}

	void addDirRO(const std::string &name, const vec3 *val,
		const std::string &label, const std::string &help)
	{
		std::string def = makeDef(label, help);
		TwAddVarRO(bar, name.c_str(), TW_TYPE_DIR3F, val, def.c_str());
	}

	void addVarRO(const std::string &name, const std::string *val,
		const std::string &label, const std::string &help)
	{
		std::string def = makeDef(label, help);
		TwAddVarRO(bar, name.c_str(), TW_TYPE_STDSTRING, val, def.c_str());
	}


	std::string makeDef(const std::string &label, const std::string &help)
	{
		std::stringstream ss;
		ss << " label='" << label << "' ";
		if (help != "") {
			ss << " help='" << help << "'";
		}
		return ss.str();
	}
	template<typename T>
	std::string makeDef(const std::string &label, const std::string &help, T min, T max)
	{
		std::stringstream ss;
		ss << makeDef(label, help);
		ss << " min=" << min << " max=" << max;
		return ss.str();
	}

	template<typename T>
	std::string makeDef(const std::string &label, const std::string &help, T min, T max, T step)
	{
		std::stringstream ss;
		ss << makeDef(label, help, min, max);
		ss << " step=" << step;
		return ss.str();
	}

	void addSeparator(const std::string & name, const std::string & help)
	{
		std::string def = makeDef(name, help);
		TwAddSeparator(bar, name.c_str(), def.c_str());
	}

	TwBar *bar;
};

AntTweakBar::AntTweakBar(GLWindow * window, const std::string &name)
	:pimpl_(new Impl(window, name))
{}

AntTweakBar::~AntTweakBar() throw()
{}

bool AntTweakBar::processEvent(const SDL_Event & e)
{
	return pimpl_->processEvent(e);
}

void AntTweakBar::draw()
{
	pimpl_->draw();
}

void AntTweakBar::addVarRW(const std::string & name, int * val, 
	const std::string & label,
	int min, int max, 
	const std::string & help)
{
	pimpl_->addVarRW(name, val, label, min, max, help);
}

void AntTweakBar::addVarRO(const std::string & name, const int * val, const std::string & label, const std::string & help)
{
	pimpl_->addVarRO(name, val, label, help);
}

void AntTweakBar::addVarRW(const std::string &name, float *val,
	const std::string &label, float min, float max, float step, const std::string &help)
{
	pimpl_->addVarRW(name, val, label, min, max, step, help);
}

void AntTweakBar::addVarRO(const std::string & name, const float * val, const std::string & label, const std::string & help)
{
	pimpl_->addVarRO(name, val, label, help);
}

void AntTweakBar::addVarRW(const std::string &name, bool *val,
	const std::string &label, const std::string &help)
{
	pimpl_->addVarRW(name, val, label, help);
}

void AntTweakBar::addVarRO(const std::string &name, const bool *val,
	const std::string &label, const std::string &help)
{
	pimpl_->addVarRO(name, val, label, help);
}

void AntTweakBar::addColorRW(const std::string & name, vec3 * val, const std::string & label, const std::string & help)
{
	pimpl_->addColorRW(name, val, label, help);
}

void AntTweakBar::addColorRO(const std::string & name, const vec3 * val, const std::string & label, const std::string & help)
{
	pimpl_->addColorRO(name, val, label, help);
}

void AntTweakBar::addDirRW(const std::string & name, vec3 * val, const std::string & label, const std::string & help)
{
	pimpl_->addDirRW(name, val, label, help);
}

void AntTweakBar::addDirRO(const std::string & name, const vec3 * val, const std::string & label, const std::string & help)
{
	pimpl_->addDirRO(name, val, label, help);
}

void AntTweakBar::addVarRO(const std::string & name, const std::string * val, const std::string & label, const std::string & help)
{
	pimpl_->addVarRO(name, val, label, help);
}

void AntTweakBar::addButton(const std::string & name, void(*callback)(void*), void *data, const std::string &label)
{
	std::string def;
	if (label == "") def = "";
	else def = " label='" + label + "'";
	TwAddButton(pimpl_->bar, name.c_str(), callback, data, def.c_str());
}

void AntTweakBar::addSeparator(const std::string & name, const std::string & help)
{
	pimpl_->addSeparator(name, help);
}

AntTweakBar::EnumType AntTweakBar::defineEnum(const std::string & name, const std::vector<EnumVal>& vals)
{
	std::vector<TwEnumVal> twVals;
	for (auto &v : vals) {
		TwEnumVal  twV;
		twV.Label = v.name.c_str();
		twV.Value = v.value;
		twVals.push_back(twV);
	}
	return TwDefineEnum(name.c_str(), &twVals[0], twVals.size());
}

void AntTweakBar::addEnumVarRO(const std::string & name, EnumType type, const int * val)
{
	TwAddVarRO(pimpl_->bar, name.c_str(), (TwType)type, val, nullptr);
}

void AntTweakBar::addEnumVarRW(const std::string & name, EnumType type, int * val)
{
	TwAddVarRW(pimpl_->bar, name.c_str(), (TwType)type, val, nullptr);
}

