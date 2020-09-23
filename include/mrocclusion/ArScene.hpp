#ifndef MROCCLUSION_ARSCENE_HPP_INCLUDED
#define MROCCLUSION_ARSCENE_HPP_INCLUDED

#include <mrocclusion/Renderable.hpp>
#include <memory>
#include <vector>


class ArScene : public Renderable {
public:
	explicit ArScene();
	~ArScene() throw();

	virtual void render();

	void addRenderable(std::shared_ptr<Renderable>);

private:
	std::vector<std::shared_ptr<Renderable> > renderables_;
};


#endif //MROCCLUSION_ARCAMERA_HPP_INCLUDED
