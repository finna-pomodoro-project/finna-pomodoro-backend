#include "server.hpp"

#include <glibmm/main.h>

int main(int, char *[])
{
    Gio::init();

    Glib::RefPtr<Glib::MainLoop> main_loop = Glib::MainLoop::create();

    Server server;

    main_loop->run();

    return EXIT_SUCCESS;
}
