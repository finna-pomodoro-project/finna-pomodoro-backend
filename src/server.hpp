#include <chrono>

#include <sigc++/connection.h>

#include <giomm.h>
#include <glibmm.h>

class Server
{
public:
    Server();

    void start();
    void stop();
    void pause();
    void resume();
    void toggle();
    void current_countdown(const Glib::RefPtr<Gio::DBus::MethodInvocation> &invocation) const;
    void is_working_time(const Glib::RefPtr<Gio::DBus::MethodInvocation> &invocation) const;
    void is_running(const Glib::RefPtr<Gio::DBus::MethodInvocation> &invocation) const;

private:
    // # METHODS:

    /** Because there is only one method call and only one object, it would be
        redundant to call the write boilerplate code every time. */
    void send_signal(const Glib::ustring& signal);
    void iterate_pomodoro();

    // # SLOTS:
    void on_method_call(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                        const Glib::ustring &sender,
                        const Glib::ustring &object_path,
                        const Glib::ustring &interface_name,
                        const Glib::ustring &method_name,
                        const Glib::VariantContainerBase &parameters,
                        const Glib::RefPtr<Gio::DBus::MethodInvocation> &invocation);
    bool on_timeout();

    // # Gio::DBus related ATTRIBUTES:

    const Gio::DBus::InterfaceVTable interface_vtable;

    // # Gio::Dbus::Connection related ATTRIBUTES

    guint id = 0;
    Glib::RefPtr<Gio::DBus::Connection> bus_connection;

    // # Pomodoro state related ATTRIBUTES

    int remaining_sec = 25 * 60;
    int next_period = 1; // index for periods_in_sec
    bool is_running_ = false;
    std::chrono::steady_clock clock;
    std::chrono::steady_clock::time_point last_resumed;
    sigc::connection on_timeout_connection;

    // # STATIC ATTRIBUTES

    static Glib::RefPtr<Gio::DBus::NodeInfo> introspection_data;
};
