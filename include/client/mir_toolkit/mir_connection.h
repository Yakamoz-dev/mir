/*
 * Copyright © 2012-2014 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 2 or 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MIR_TOOLKIT_MIR_CONNECTION_H_
#define MIR_TOOLKIT_MIR_CONNECTION_H_

#include <mir_toolkit/client_types.h>
#include <mir_toolkit/common.h>
#include <mir_toolkit/deprecations.h>

#include <stdbool.h>

#ifdef __cplusplus
/**
 * \addtogroup mir_toolkit
 * @{
 */
extern "C" {
#endif

/**
 * Request a connection to the Mir server. The supplied callback is called when
 * the connection is established, or fails. The returned wait handle remains
 * valid until the connection has been released.
 *   \warning callback could be called from another thread. You must do any
 *            locking appropriate to protect your data accessed in the
 *            callback.
 *   \param [in] server       File path of the server socket to connect to, or
 *                            NULL to choose the default server (can be set by
 *                            the $MIR_SOCKET environment variable)
 *   \param [in] app_name     A name referring to the application
 *   \param [in] callback     Callback function to be invoked when request
 *                            completes
 *   \param [in,out] context  User data passed to the callback function
 *   \return                  A handle that can be passed to mir_wait_for
 */
MirWaitHandle *mir_connect(
    char const *server,
    char const *app_name,
    MirConnectedCallback callback,
    void *context);

/**
 * Perform a mir_connect() but also wait for and return the result.
 *   \param [in] server    File path of the server socket to connect to, or
 *                         NULL to choose the default server
 *   \param [in] app_name  A name referring to the application
 *   \return               The resulting MirConnection
 */
MirConnection *mir_connect_sync(char const *server, char const *app_name);

/**
 * Test for a valid connection
 * \param [in] connection  The connection
 * \return                 True if the supplied connection is valid, or
 *                         false otherwise.
 */
bool mir_connection_is_valid(MirConnection *connection);

/**
 * Retrieve a text description of the last error. The returned string is owned
 * by the library and remains valid until the connection has been released.
 *   \param [in] connection  The connection
 *   \return                 A text description of any error resulting in an
 *                           invalid connection, or the empty string "" if the
 *                           connection is valid.
 */
char const *mir_connection_get_error_message(MirConnection *connection);

/**
 * Release a connection to the Mir server
 *   \param [in] connection  The connection
 */
void mir_connection_release(MirConnection *connection);

/**
 * Register a callback to be called when a Lifecycle state change occurs.
 *   \param [in] connection     The connection
 *   \param [in] callback       The function to be called when the state change occurs
 *   \param [in,out] context    User data passed to the callback function
 */
void mir_connection_set_lifecycle_event_callback(MirConnection* connection,
    MirLifecycleEventCallback callback, void* context);


/**
 * Register a callback for server ping events.
 *
 * The server may send ping requests to detect unresponsive applications. Clients should
 * process this with their regular event handling, and call mir_connection_pong() in response.
 *
 * The shell may treat a client which fails to pong in a timely fashion differently; a common
 * response is to overlay the surface with an unresponsive application message.
 *
 * A default implementation that immediately calls pong is provided; toolkits SHOULD override
 * this default implementation to more accurately reflect the state of their event processing
 * loop.
 *
 * \param [in] connection       The connection
 * \param [in] callback         The function to be called on ping events.
 * \param [in] context          User data passed to the callback function
 */
void mir_connection_set_ping_event_callback(MirConnection* connection,
    MirPingEventCallback callback, void* context);


/**
 * Respond to a ping event
 * \param [in] connection       The connection
 * \param [in] serial           Serial from the ping event
 */
void mir_connection_pong(MirConnection* connection, int32_t serial);

/**
 * Query the display
 *
 * \pre mir_connection_is_valid(connection) == true
 * \warning return value must be destroyed via mir_display_config_release()
 *
 * \param [in]  connection        The connection
 * \return                        structure that describes the display configuration
 */
MirDisplayConfig* mir_connection_create_display_configuration(MirConnection* connection);

/**
 * Register a callback to be called when the hardware display configuration changes
 *
 * Once a change has occurred, you can use mir_connection_create_display_configuration to see
 * the new configuration.
 *
 *   \param [in] connection  The connection
 *   \param [in] callback     The function to be called when a display change occurs
 *   \param [in,out] context  User data passed to the callback function
 */
void mir_connection_set_display_config_change_callback(
    MirConnection* connection,
    MirDisplayConfigCallback callback, void* context);

/**
 * Apply the display config for the connection
 *
 * The display config is applied to this connection only (per-connection
 * config) and is invalidated when a hardware change occurs. Clients should
 * register a callback with mir_connection_set_display_config_change_callback()
 * to get notified about hardware changes, so that they can apply a new config.
 *
 *   \param [in] connection             The connection
 *   \param [in] display_config         The display_config to apply
 */
void mir_connection_apply_session_display_config(MirConnection* connection, MirDisplayConfig const* display_config);

/**
 * Remove the display configuration for the connection
 *
 * If a session display config is applied to the connection it is removed, and
 * the base display config is used. If there was no previous call to
 * mir_connection_apply_session_display_config this will do nothing.
 *
 *   \param [in] connection             The connection
 */
void mir_connection_remove_session_display_config(MirConnection* connection);

/**
 * Preview a new base display configuration
 *
 * The base display configuration is the configuration the server applies when
 * there is no active per-connection configuration.
 *
 * The display configuration will automatically revert to the previous
 * settings after timeout_seconds unless confirmed by a call to
 * mir_connection_confirm_base_display_configuration(), or is reverted
 * immediately after a call to mir_connection_cancel_display_configuration_preview().
 *
 * If this request succeeds a configuration change event is sent to the
 * client. Clients should register a callback with
 * mir_connection_set_display_config_change_callback() in order to determine
 * when this call succeeds.
 *
 * If the configuration is not confirmed before timeout_seconds have elapsed
 * a second configuration change event is sent, with the old config.
 *
 * \param [in] connection       The connection
 * \param [in] configuration    The MirDisplayConfig to set as base
 * \param [in] timeout_seconds  The time the server should wait, in seconds,
 *                                  for the change to be confirmed before
 *                                  reverting to the previous configuration.
 */
void mir_connection_preview_base_display_configuration(
    MirConnection* connection,
    MirDisplayConfig const* configuration,
    int timeout_seconds);

/**
 * Confirm a base configuration change initiated by mir_connection_preview_base_display_configuration()
 *
 * The base display configuration is the configuration the server applies when
 * there is no active per-connection configuration.
 *
 * If this request succeeds a second configuration change event is sent to the
 * client, identical to the one sent after
 * mir_connection_preview_base_display_configuration(). Clients should
 * register a callback with mir_connection_set_display_config_change_callback()
 * in order to determine when this call succeeds.
 *
 * The MirDisplayConfig must be the same as the one passed to
 * mir_connection_preview_base_display_configuration().
 *
 * \param [in] connection       The connection
 * \param [in] configuration    The MirDisplayConfig to confirm as base
 *                                  configuration.
 */
void mir_connection_confirm_base_display_configuration(
    MirConnection* connection,
    MirDisplayConfig const* configuration);

/**
 * Cancel a pending base display configuration preview.
 *
 * If this request succeeds a configuration change event is sent to the client,
 * with the now-current base display configuration.
 *
 * This call will fail if there is no display configuration preview current.
 * A client can detect this by registering a callback with
 * mir_connection_set_error_callback() and checking for
 * mir_display_configuration_error_no_preview_in_progress.
 *
 * \param [in] connection   The connection
 */
void mir_connection_cancel_base_display_configuration_preview(
    MirConnection* connection);

/**
 * Get a display type that can be used with EGL.
 *   \deprecated Use MirConnection * as the native display instead
 *   \param [in] connection  The connection
 *   \return                 An EGLNativeDisplayType that the client can use
 */
MirEGLNativeDisplayType mir_connection_get_egl_native_display(MirConnection *connection)
MIR_FOR_REMOVAL_IN_VERSION_1("Use MirConnection * as the native display instead");

/**
 * Get the list of possible formats that a surface can be created with.
 *   \param [in] connection         The connection
 *   \param [out] formats           List of valid formats to create surfaces with
 *   \param [in]  formats_size      size of formats list
 *   \param [out] num_valid_formats number of valid formats returned in formats
 *
 * \note Users of EGL should call mir_connection_get_egl_pixel_format instead,
 *       as it will take the guesswork out of choosing between similar pixel
 *       formats. At the moment, this function returns a compatible list of
 *       formats likely to work for either software or hardware rendering.
 *       However it is not the full or accurate list and will be replaced in
 *       future by a function that takes the intended MirBufferUsage into
 *       account.
 */
void mir_connection_get_available_surface_formats(
    MirConnection* connection, MirPixelFormat* formats,
    unsigned const int formats_size, unsigned int *num_valid_formats);

/**
 * Create a snapshot of the attached input devices and device configurations.
 * \warning return value must be destroyed via mir_input_config_release()
 * \warning may return null if connection is invalid
 * \param [in]  connection        The connection
 * \return      structure that describes the input configuration
 */
MirInputConfig* mir_connection_create_input_config(MirConnection *connection);

/**
 * Apply the input configuration for the connection
 *
 * Configure the behavior of input device attached to a server when the session
 * this connection represents is the focused. If the session is not focused
 * the configuration will be stored for later use.
 *
 * The call returns after sending the configuration to the server.
 *
 * Errors during application of the configuration will be indicated through
 * the error callback.
 *
 *   \param [in] connection             The connection
 *   \param [in] config                 The input config
 */
void mir_connection_apply_session_input_config(
    MirConnection* connection, MirInputConfig const* config);

/**
 * Set the input configuration as base configuration.
 *
 * Configure the behavior of input device attached to a server. When
 * allowed by the shell the configuration will be used as base configuration.
 *
 * So whenever the active session has no session specific configuration this
 * input configuration will be used.
 *
 * The call returns after sending the configuration to the server.
 *
 * Errors during application of the configuration will be indicated through
 * the error callback.
 *
 *   \param [in] connection             The connection
 *   \param [in] config                 The input config
 */
void mir_connection_set_base_input_config(
    MirConnection* connection, MirInputConfig const* config);

/**
 * Release this snapshot of the input configuration.
 * This invalidates any pointers retrieved from this structure.
 *
 * \param [in] config  The input configuration
 */
void mir_input_config_release(MirInputConfig const* config);

/**
 * Register a callback to be called when the input devices change.
 *
 * Once a change has occurred, you can use mir_connection_create_input_config
 * to get an updated snapshot of the input device configuration.
 *
 * \param [in] connection  The connection
 * \param [in] callback    The function to be called when a change occurs
 * \param [in,out] context User data passed to the callback function
 */
void mir_connection_set_input_config_change_callback(
    MirConnection* connection,
    MirInputConfigCallback callback, void* context);

/**
 * Register a callback to be called on non-fatal errors
 *
 * \param [in] connection   The connection
 * \param [in] callback     The function to be called when an error occurs
 * \param [in,out] context  User data passed to the callback function
 */
void mir_connection_set_error_callback(
    MirConnection* connection,
    MirErrorCallback callback,
    void* context);

/**
 * Returns client API version
 *
 * Result of the function should be compared to result of MIR_VERSION_NUMBER
 *
 * \return  The client API version
 */
unsigned mir_get_client_api_version();

/**
 * Enumerates the supported extensions
 *
 * \param [in]  connection  The connection
 * \param [out] context     User data passed to the enumerator function
 * \param [in]  enumerator  The function to be called for each extension
 *
 */
void mir_connection_enumerate_extensions(
    MirConnection* connection,
    void* context,
    void (*enumerator)(void* context, char const* extension, int version));

#ifdef __cplusplus
}
/**@}*/
#endif

#endif /* MIR_TOOLKIT_MIR_CONNECTION_H_ */
