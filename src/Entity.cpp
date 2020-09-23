#include "mrocclusion/Entity.hpp"
#include <Eigen/Dense>


Entity::Entity(const mat4 &modelToWorld)
:modelToWorld_(modelToWorld)
{}

Entity::~Entity() throw()
{}

void Entity::modelToWorld(const mat4 &m)
{
	modelToWorld_ = m;
}

mat4 Entity::modelToWorld() const
{
	return modelToWorld_;
}

mat3 Entity::normToWorld() const
{
	return modelToWorld_.block<3,3>(0,0).inverse().transpose();
}

