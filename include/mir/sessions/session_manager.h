/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#ifndef MIR_SESSIONS_APPLICATION_MANAGER_H_
#define MIR_SESSIONS_APPLICATION_MANAGER_H_

#include "mir/sessions/session_store.h"
#include "mir/thread/all.h"
#include <memory>
#include <vector>

namespace mir
{

namespace sessions
{
class SurfaceFactory;
class Session;
class SessionContainer;
class FocusSequence;
class FocusSetter;

class SessionManager : public SessionStore
{
public:
    explicit SessionManager(std::shared_ptr<SurfaceFactory> const& surface_factory,
                            std::shared_ptr<SessionContainer> const& session_container,
                            std::shared_ptr<FocusSequence> const& focus_sequence,
                            std::shared_ptr<FocusSetter> const& focus_setter);
    virtual ~SessionManager();

    virtual std::shared_ptr<Session> open_session(std::string const& name);
    virtual void close_session(std::shared_ptr<Session> const& session);
    virtual void shutdown();

    virtual void tag_session_with_lightdm_id(std::shared_ptr<Session> const& session, int id);
    virtual void focus_session_with_lightdm_id(int id);

    void focus_next();

protected:
    SessionManager(const SessionManager&) = delete;
    SessionManager& operator=(const SessionManager&) = delete;

private:
    std::shared_ptr<sessions::SurfaceFactory> const surface_factory;
    std::shared_ptr<SessionContainer> const app_container;
    std::shared_ptr<FocusSequence> const focus_sequence;
    std::shared_ptr<FocusSetter> const focus_setter;

    std::mutex mutex;
    std::weak_ptr<Session> focus_application;
    typedef std::vector<std::pair<int, std::shared_ptr<Session>>> Tags;
    Tags tags;
};

}
}

#endif // MIR_SESSIONS_APPLICATION_MANAGER_H_
