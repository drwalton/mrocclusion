#ifndef MROCCLUSION_ANTTWEAKBAR_HPP_INCLUDED
#define MROCCLUSION_ANTTWEAKBAR_HPP_INCLUDED

#include <mrocclusion/GLWindow.hpp>
#include <mrocclusion/VectorTypes.hpp>

class AntTweakBar final {
public:
	AntTweakBar(GLWindow *window, const std::string &name = "Variables");
	~AntTweakBar() throw();

	bool processEvent(const SDL_Event &e);

	void draw();

	//Ints
	void addVarRW(const std::string &name, int *val, 
		const std::string &label, int min, int max, const std::string &help = "");
	void addVarRO(const std::string &name, const int *val, 
		const std::string &label, const std::string &help = "");

	//Floats
	void addVarRW(const std::string &name, float *val,
		const std::string &label, float min, float max, float step, const std::string &help = "");
	void addVarRO(const std::string &name, const float *val, 
		const std::string &label, const std::string &help = "");

	//Boolean
	void addVarRW(const std::string &name, bool *val,
		const std::string &label, const std::string &help = "");
	void addVarRO(const std::string &name, const bool *val,
		const std::string &label, const std::string &help = "");

	//Colours (RGB)
	void addColorRW(const std::string &name, vec3 *val,
		const std::string &label, const std::string &help = "");
	void addColorRO(const std::string &name, const vec3 *val,
		const std::string &label, const std::string &help = "");

	//3D Directions
	void addDirRW(const std::string &name, vec3 *val,
		const std::string &label, const std::string &help = "");
	void addDirRO(const std::string &name, const vec3 *val,
		const std::string &label, const std::string &help = "");

	//Strings
	void addVarRO(const std::string &name, const std::string *val,
		const std::string &label, const std::string &help = "");

	//Button
	void addButton(const std::string & name, void(*callback)(void*), 
		void *data = nullptr, const std::string &label = "");

	void addSeparator(const std::string &name, const std::string &help = "");

	struct EnumVal {
		int value;
		std::string name;
	};
	typedef int EnumType;

	EnumType defineEnum(const std::string &name, const std::vector<EnumVal> &vals);
	void addEnumVarRO(const std::string &name, EnumType type, const int *val);
	void addEnumVarRW(const std::string &name, EnumType type, int *val);

private:
	struct Impl;
	std::unique_ptr<Impl> pimpl_;
};

#endif
