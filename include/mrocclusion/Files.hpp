#ifndef MROCCLUSION_FILES_HPP_INCLUDED
#define MROCCLUSION_FILES_HPP_INCLUDED

#include <string>
#include "Exception.hpp"
#include "VectorTypes.hpp"

//!\brief Retrieve the contents of a file and return as a std::string.
//!\throw FileException if the file is not found.
std::string getFileContents(const std::string &filename);

//!\brief Given a directory, makes a subdirectory with a unique name
//! determined by the current date and time.
//!\return The created subdirectory path (terminated with a forward slash).
std::string makeUniqueSubdirectory(const std::string &directory);

//!\brief Load a mat4 from a binary file consisting of 16 32-bit floats.
mat4 loadMat4FromFile(const std::string &filename);

//!\brief Save a mat4 to a binary file consisting of 16 32-bit floats.
void saveMat4ToFile(const std::string &filename, const mat4 &m);

//!\brief Load a mat4 from a binary file consisting of 16 32-bit floats.
RigidTransform loadRigidTransformFromFile(const std::string &filename);

//!\brief Save a mat4 to a binary file consisting of 16 32-bit floats.
void saveRigidTransformToFile(const std::string &filename, const RigidTransform &m);

//!\brief Check if the last few characters of a string exactly match the supplied 
//!       ending string.
//!\note Case-sensitive
//!\note Returns false if ending is longer than value.
bool stringEndsWith(std::string const & value, std::string const & ending);

//!\brief Generate a unique timestamp based on the current wall time.
//!\note This will be unique, provided it is not used twice within a second.
std::string uniqueTimestamp();

#endif
