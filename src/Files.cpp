#include "mrocclusion/Files.hpp"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <boost/filesystem.hpp>


std::string getFileContents(const std::string &filename)
{
	std::ifstream file(filename);
	if (file.fail()) throw FileException(
		"Could not open file \"" + filename + "\".");
	std::stringstream buff;
	buff << file.rdbuf();
	return buff.str();
}

std::string makeUniqueSubdirectory(const std::string & directory)
{
	boost::filesystem::path givenPath(directory);
	if (!boost::filesystem::is_directory(givenPath)) {
		throw std::runtime_error("Supplied path is not a directory!");
	}

	auto t = std::time(nullptr);
	auto tm = std::localtime(&t);
	std::stringstream dirToMake;
	dirToMake << directory << std::put_time(tm, "%d-%m-%Y_%H-%M-%S/");

	boost::filesystem::path newDir(dirToMake.str());
	boost::filesystem::create_directory(newDir);
	return dirToMake.str();
}

mat4 loadMat4FromFile(const std::string &filename)
{
	std::ifstream ifs(filename);
	if (ifs.fail()) {
		throw FileException("Could not open file \"" + filename + "\"");
	}
	mat4 m;
	for(size_t i = 0; i < 16; ++i) {
		ifs >> ((float*)(m.data()))[i];
	}
	return m;
}

void saveMat4ToFile(const std::string &filename, const mat4 &m)
{
	std::ofstream ofs(filename);
	if (ofs.fail()) {
		throw FileException("Could not write to file \"" + filename + "\"");
	}
	ofs << ((float*)(m.data()))[0];
	for(size_t i = 1; i < 16; ++i) {
		ofs << " " << ((float*)(m.data()))[i];
	}
}

//!\brief Load a mat4 from a binary file consisting of 16 32-bit floats.
RigidTransform loadRigidTransformFromFile(const std::string &filename)
{
	mat4 mat = loadMat4FromFile(filename);
	return RigidTransform(mat);
}

//!\brief Save a mat4 to a binary file consisting of 16 32-bit floats.
void saveRigidTransformToFile(const std::string &filename, const RigidTransform &m)
{
	saveMat4ToFile(filename, m);
}

bool stringEndsWith(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::string uniqueTimestamp()
{
	time_t now = time(0);
	struct tm *time = localtime(&now);
	std::stringstream timestamp;
	timestamp << 
		1900 + time->tm_year << "_" << 
		1 + time->tm_mon <<  "_" <<
		time->tm_mday <<  "_" <<
		time->tm_hour <<  "_" <<
		time->tm_min <<  "_" <<
		time->tm_sec;
	return timestamp.str();
}
