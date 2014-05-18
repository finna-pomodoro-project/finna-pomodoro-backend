#include "server.hpp"

#include <iostream>
#include <cstdint>
#include <cassert>
#include <array>

#include <giomm/dbusownname.h>

using std::array;
using std::chrono::seconds;
using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::cout;
using std::endl;
using Glib::RefPtr;
using Glib::ustring;
using Glib::VariantContainerBase;
using Glib::Variant;
using Glib::signal_timeout;
using Gio::DBus::NodeInfo;
using Gio::DBus::Connection;
using Gio::DBus::own_name;
using Gio::DBus::Message;
using Gio::DBus::BusType;
using Gio::DBus::MethodInvocation;

static array<guint32, 8> periods_in_sec = {
    1500, // WORK
    300, // PROCRASTINATE,
    1500, // WORK
    300, // PROCRASTINATE,
    1500, // WORK
    300, // PROCRASTINATE,
    1500, // WORK
    900, // PROCRASTINATE
};

RefPtr<NodeInfo> Server::introspection_data = NodeInfo::create_for_xml([](){
return R"xml(
<node name="/io/github/finna_pomodoro_project/Pomodoro">
  <interface name="io.github.finna_pomodoro_project.Pomodoro">
    <method name="start" />
    <method name="stop" />
    <method name="pause" />
    <method name="resume" />
    <method name="toggle" />

    <method name="current_countdown">
      <arg type="u" name="seconds" direction="out" />
    </method>

    <method name="is_working_time">
      <arg type="b" direction="out" />
    </method>

    <method name="is_running">
      <arg type="b" direction="out" />
    </method>

    <signal name="pomodoro_resumed" />
    <signal name="pomodoro_paused" />
    <signal name="work_session_started" />
    <signal name="work_session_stopped" />
  </interface>
</node>
)xml";
}());

Server::Server() :
    interface_vtable(sigc::mem_fun(*this, &Server::on_method_call))
{
    auto bus_acquired = [this](const RefPtr<Connection> &conn, ustring name) {
        bus_connection = conn;
        cout << "Bus \"" << name << "\" acquired" << endl;

        conn->register_object("/io/github/finna_pomodoro_project/Pomodoro",
                              introspection_data->lookup_interface(),
                              interface_vtable);
    };

    auto name_acquired = [](const RefPtr<Connection>&, ustring name) {
        cout << "Name \"" << name << "\" acquired" << endl;
    };

    auto name_lost = [](const RefPtr<Connection>&, ustring name) {
        cout << "Name \"" << name << "\" lost" << endl;
    };

    id = own_name(BusType::BUS_TYPE_SESSION, "io.github.finna_pomodoro_project",
                  bus_acquired, name_acquired, name_lost);
}

void Server::start()
{
    /* We try to update the clock as soon as possible, to avoid measurements
       errors that can add up.

       We don't care about passing a precise duration to the timeout signal
       emitter, because our algorithm will compensate later on the callback
       itself. */
    last_resumed = steady_clock::now();

    if (is_running_)
        on_timeout_connection.disconnect();

    remaining_sec = periods_in_sec[0];
    on_timeout_connection = signal_timeout()
        .connect_seconds(sigc::mem_fun(*this, &Server::on_timeout),
                         remaining_sec);
    next_period = 1;
    is_running_ = true;

    // asynchronous operations are done by last
    send_signal("pomodoro_resumed");
    if (next_period % 2)
        send_signal("work_session_started");
    else
        send_signal("work_session_stopped");
}

void Server::stop()
{
    if (is_running_)
        on_timeout_connection.disconnect();

    remaining_sec = periods_in_sec[0];
    next_period = 1;
    is_running_ = false;

    // asynchronous operations are done by last
    send_signal("pomodoro_paused");
}

void Server::pause()
{
    if (!is_running_)
        return;

    {
        auto elapsed = steady_clock::now() - last_resumed;

        while (elapsed > seconds{remaining_sec}) {
            iterate_pomodoro();
            elapsed = steady_clock::now() - last_resumed;
        }

        remaining_sec -= duration_cast<seconds>(elapsed).count();
    }

    on_timeout_connection.disconnect();
    is_running_ = false;

    // asynchronous operations are done by last
    send_signal("pomodoro_paused");
}

void Server::resume()
{
    if (is_running_)
        return;

    /* We try to update the clock as soon as possible, to avoid measurements
       errors that can add up.

       We don't care about passing a precise duration to the timeout signal
       emitter, because our algorithm will compensate later on the callback
       itself. */
    last_resumed = steady_clock::now();
    on_timeout_connection = signal_timeout()
        .connect_seconds(sigc::mem_fun(*this, &Server::on_timeout),
                         remaining_sec);
    is_running_ = true;

    // asynchronous operations are done by last
    send_signal("pomodoro_resumed");
    if (next_period % 2)
        send_signal("work_session_started");
    else
        send_signal("work_session_stopped");
}

void Server::toggle()
{
    is_running_ ? pause() : resume();
}

void Server::current_countdown(const RefPtr<MethodInvocation> &invocation) const
{
    /* current_countdown won't change the pomodoro state to decrease the number
       of errors that can add up very easily. */
    int remaining_sec = this->remaining_sec;
    if (is_running_) {
        auto elapsed = steady_clock::now() - last_resumed;

        if (elapsed > seconds{remaining_sec})
            remaining_sec = 0;
        else
            remaining_sec -= duration_cast<seconds>(elapsed).count();
    }
    auto countdown = Variant<uint32_t>::create(remaining_sec);
    invocation->return_value(VariantContainerBase::create_tuple(countdown));
}

void Server::is_working_time(const RefPtr<MethodInvocation> &invocation)
{
    auto is_running = Variant<bool>::create(next_period % 2);
    invocation->return_value(VariantContainerBase::create_tuple(is_running));
}

void Server::is_running(const RefPtr<MethodInvocation> &invocation)
{
    auto is_running = Variant<bool>::create(is_running_);
    invocation->return_value(VariantContainerBase::create_tuple(is_running));
}

void Server::send_signal(const ustring& signal)
{
    if (bus_connection) {
        RefPtr<Message> message
            = Message::create_signal(/*path = */"/io/github"
                                     "/finna_pomodoro_project/Pomodoro",
                                     /*interface = */"io.github"
                                     ".finna_pomodoro_project.Pomodoro",
                                     signal);
        bus_connection->send_message(message);
    }
}

void Server::iterate_pomodoro()
{
    last_resumed += seconds{remaining_sec};
    remaining_sec = periods_in_sec[next_period++];
    next_period %= periods_in_sec.size();
}

void Server::on_method_call(const RefPtr<Connection> &/*connection*/,
                            const ustring &/*sender*/,
                            const ustring &/*object_path*/,
                            const ustring &interface_name,
                            const ustring &method_name,
                            const VariantContainerBase &/*parameters*/,
                            const RefPtr<MethodInvocation> &invocation)
{
    // Get rid of unused warning on release mode
    (void)(interface_name);
    assert (interface_name == "io.github.finna_pomodoro_project.Pomodoro");

    if (method_name == "start") {
        start();
    } else if (method_name == "stop") {
        stop();
    } else if (method_name == "pause") {
        pause();
    } else if (method_name == "resume") {
        resume();
    } else if (method_name == "toggle") {
        toggle();
    } else if (method_name == "current_countdown") {
        current_countdown(invocation);
    } else if (method_name == "is_working_time") {
        is_working_time(invocation);
    } else if (method_name == "is_running") {
        is_running(invocation);
    } else {
        auto error = Gio::DBus::Error(Gio::DBus::Error::UNKNOWN_METHOD,
                                      "Method does not exist.");
        invocation->return_error(error);
    }
}

bool Server::on_timeout()
{
    // The definition of this function looks overly complicated, but its complex
    // behaviour comes from the fact that we try to deal with the fact that this
    // function might be called much later than we scheduled. This situation can
    // happen when the system is under high load or when the pomodoro process is
    // suspended.

    // Just a little paranoid, because we don't go much further (eg. integer
    // overflows).

    {
        /* target_time_point is a cache of the desired value and isn't correct.

           We proceed with the incorrect value anyway to avoid possible slow
           implementations and repeat the procedure for the real target
           afterwards. */
        auto cached_time_point = steady_clock::now();

        while (last_resumed + seconds{remaining_sec} < cached_time_point)
            iterate_pomodoro();

        while (last_resumed + seconds{remaining_sec} < steady_clock::now())
            iterate_pomodoro();
    }

    {
        auto elapsed = steady_clock::now() - last_resumed;

        while (elapsed > seconds{remaining_sec}) {
            iterate_pomodoro();
            elapsed = steady_clock::now() - last_resumed;
        }

        remaining_sec -= duration_cast<seconds>(elapsed).count();

        last_resumed = steady_clock::now();
        on_timeout_connection = signal_timeout()
            .connect_seconds(sigc::mem_fun(*this, &Server::on_timeout),
                             remaining_sec);
    }

    // asynchronous operations are done by last
    if (next_period % 2)
        send_signal("work_session_started");
    else
        send_signal("work_session_stopped");

    /* The return value is equivalent to the call
       `on_timeout_connection.disconnect()` with the old `on_timeout_connection`
       object/value. */
    return false;
}
