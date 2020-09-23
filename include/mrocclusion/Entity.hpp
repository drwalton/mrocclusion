#ifndef MROCCLUSION_ENTITY_HPP_INCLUDED
#define MROCCLUSION_ENTITY_HPP_INCLUDED

#include "VectorTypes.hpp"


//!\brief Class representing an object in a 3D world. Has a transform indicating
//!       its location in the world.
class Entity
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	explicit Entity(const mat4 &modelToWorld = mat4::Identity());
	virtual ~Entity() throw();

	void modelToWorld(const mat4 &m);
	mat4 modelToWorld() const;

	mat3 normToWorld() const;
private:
	mat4 modelToWorld_;
};


#endif
