/*
 * Copyright © 2019 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: William Wold <william.wold@canonical.com>
 */

#ifndef MIR_FRONTEND_FOREIGN_TOPLEVEL_MANAGER_V1_H
#define MIR_FRONTEND_FOREIGN_TOPLEVEL_MANAGER_V1_H

#include "wlr-foreign-toplevel-management-unstable-v1_wrapper.h"
#include "mir_toolkit/common.h"
#include "mir/frontend/surface_id.h"

namespace mir
{
class Executor;
namespace scene
{
class Surface;
class Session;
}
namespace shell
{
class Shell;
class SurfaceSpecification;
}
namespace frontend
{
class WlSeat;
class OutputManager;
class SurfaceStack;

/// Informs a client about toplevels from itself and other clients
/// The Wayland objects it creates for each toplevel can be used to aquire information and control that toplevel
/// Useful for task bars and app switchers
class ForeignToplevelManagerV1Global
    : public wayland::ForeignToplevelManagerV1::Global
{
public:
    ForeignToplevelManagerV1Global(
        wl_display* display,
        std::shared_ptr<shell::Shell> shell,
        WlSeat& seat,
        OutputManager* output_manager,
        std::shared_ptr<SurfaceStack> const& surface_stack);

    std::shared_ptr<shell::Shell> const shell;
    WlSeat& seat;
    OutputManager* const output_manager;
    std::shared_ptr<SurfaceStack> const surface_stack;

private:
    void bind(wl_resource* new_resource) override;
};
}
}

#endif // MIR_FRONTEND_LAYER_SHELL_V1_H
