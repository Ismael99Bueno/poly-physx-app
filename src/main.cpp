#include "ppx-app/app.hpp"
#include "lynx/rendering/buffer.hpp"
#include "ppx-app/menu_layer.hpp"

int main()
{
    ppx::app app;
    ppx::entity2D::specs specs1, specs2;
    specs2.pos = {20.f, 0.65f};
    specs2.angpos = 0.23f;
    specs2.vel = {-15.f, 0.f};
    app.engine().add_entity(specs1);
    app.engine().add_entity(specs2);
    app.run();
}