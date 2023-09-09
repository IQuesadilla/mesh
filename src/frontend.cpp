#include "frontend.h"

frontend::frontend()
{
    _backends.reset(new std::vector<mesh_backend*>);
    backend_timer.reset(new libQ::event_timer<mesh_backend*>(_backends));
}

void frontend::run_cycle()
{
    update();
    backend_timer->update_children();
}