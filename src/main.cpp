#include "ppx-app/app.hpp"
#include "lynx/rendering/buffer.hpp"
#include "ppx-app/menu_layer.hpp"
#include "ppx/joints/revolute_joint2D.hpp"
#include "kit/profile/clock.hpp"

int main()
{
    KIT_SET_LEVEL(debug)

    ppx::app app;
    ppx::entity2D::specs specs1, specs2;
    specs2.pos = {20.f, 0.65f};
    specs2.angpos = 0.23f;
    specs2.angvel = 5.f;
    specs2.vel = {5.f, 1.f};
    specs1.shape = ppx::entity2D::shape_type::CIRCLE;
    specs2.vertices = geo::polygon::ngon(5.f, 6);
    auto e1 = app.engine().add_entity(specs1);
    auto e2 = app.engine().add_entity(specs2);

    ppx::revolute_joint2D::specs spspecs;
    spspecs.e1 = e1;
    spspecs.e2 = e2;
    spspecs.has_anchors = true;
    spspecs.anchor2 = {3.f, 0.f};

    app.engine().add_constraint<ppx::revolute_joint2D>(spspecs);

    // e1->mass(30.f);
    app.run();
}