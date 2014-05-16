The finna-pomodoro uses a stable API to guide the communication between the
server and the group of clients. When you open a GUI application, it can detect
that no server is running and start one, completely hiding this user-unfriendly
implementation detail.

The cool thing about the API is that you can use several clients to control your
pomodoro. You could schedule a gently voice to remind you about work and
procrastination time. You could control your pomodoro through configured
keyboard shortcuts that trigger cli commands, a nice GUI interface or even
through your systray.

The protocol has been specified in such a way that the server is stateful, but
clients can start and connect at anytime (they begin stateless) and will store
as little state as possible. Another concern kept in mind while the protocol
specifying the protocol was to make harder to develop clients that would show
wrong information about the pomodoro session.

# D-Bus

The pomodoro API is built on top of the cool D-Bus concepts.

## Service name

The finna pomodoro project's service name on D-Bus is
`io.github.finna_pomodoro_project`. This service should use the session bus, not
the system bus.

## Objects

Current specification of the finna pomodoro project only defines one object, the
`/io/github/finna_pomodoro_project/Pomodoro` object.

The `/io/github/finna_pomodoro_project/Pomodoro` object implements the
`io.github.finna_pomodoro_project.Pomodoro` interface.

### `io.github.finna_pomodoro_project.Pomodoro` interface

This interfaces exposes the following methods:

- `start()`: It will reset the pomodoro and resume it.
- `stop()`: It will pause the pomodoro and reset afterwards.
- `pause()`: It will pause the pomodoro.
- `resume()`: It will resume the pomodoro.
- `toggle()`: Think about `toggle` as an atomic implementation of the following
  procedure:

  ```cpp
  if (pomodoro.is_running())
      pomodoro.pause();
  else
      pomodoro.resume();
  ```

  This verion is provided, because you cannot implement an atomic version on
  your own in client mode. It's guaranteed that the pomodoro will retain the
  state for the `is_running()` method if two implementations call the method,
  because it won't suffer interference dependant upon delivery order.
- `bool is_running()`: Return `true` if pomodoro is running or `false`
  otherwise.
- `bool is_working_time()`: Return `true` if it is working time or `false`
  otherwise.
- `uint32_t current_countdown()`: Return (as an `uint32_t`) the time (in
  seconds) remaining until the pomodoro changes its state (time to
  work/procrastinate). `uint32_t` type was chosen instead `uint64_t` to
  discourage users to adopt very large periods. NOTE: API to configure the
  pomodoro is not specified.

This interface exposes the following signals:

- `pomodoro_resumed()`: Emitted each time the pomodoro starts (`is_running()`
  will return `true`).
- `pomodoro_paused()`: Emitted each time the pomodoro stops (`is_running()` will
  return `false`).
- `work_session_started()`: Emitted each time the time to work start.
- `work_session_stopped()`: Emitted each time the time to procrastinate start.

When the pomodoro is paused, the implementation **MUST NOT** send signals to
keep client's info about pomodoro's state updated. The only exception is the
`pomodoro_paused` signal, which **MUST** be emitted. Clients should use hints
to keep the user aware (such as a question mark next to the current remaining
time). Also, the pomodoro **MUST** send a `work_session_started()` signal or a
`work_session_stopped()` signal after the `pomodoro_resumed()` is sent.

### `io.github.finna_pomodoro_project.AdvancedPomodoro` interface

This is a debug interface that only makes sense to use combined with the
`io.github.finna_pomodoro_project.Pomodoro` interface. Not all pomodoros are
required to implement this interface.

This interfaces exposes the following methods:

- `uint32_t current_session()`: Return the current session number (starts from
  `0`).
- `uint32_t max_sessions()`: Return the max number of sessions.
  `current_session()` always returns a number smaller than this. The
  implementation **MUST** not change its value and it's safe for clients to
  cache this value.

This interface exposes the following signals:

- `uint32_t current_session_changed()`: Emitted each time the pomodoro changes.
  Returns the new session number;
