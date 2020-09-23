#include "mrocclusion/ArScene.hpp"


ArScene::ArScene()
{}

ArScene::~ArScene() throw()
{}

void ArScene::render()
{
	for (auto &e : renderables_) {
		mat4 m = e->modelToWorld();
		//Transform renderable to position of scene.
		e->modelToWorld(this->modelToWorld() * m);
		e->render();
		//Transform back.
		e->modelToWorld(m);
	}
}

void ArScene::addRenderable(std::shared_ptr<Renderable> entity)
{
	renderables_.push_back(entity);
}


