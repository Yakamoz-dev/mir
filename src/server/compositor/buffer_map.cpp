/*
 * Copyright © 2015 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir/graphics/graphic_buffer_allocator.h"
#include "mir/frontend/event_sink.h"
#include "buffer_map.h"
#include <boost/throw_exception.hpp>

namespace mc = mir::compositor;
namespace mf = mir::frontend;
namespace mg = mir::graphics;

namespace mir { namespace compositor {
enum class BufferMap::Owner
{
    server,
    client
};
}
}

mc::BufferMap::BufferMap(
    mf::BufferStreamId id,
    std::shared_ptr<mf::EventSink> const& sink,
    std::shared_ptr<mg::GraphicBufferAllocator> const& allocator) :
    stream_id(id),
    sink(sink),
    allocator(allocator)
{
}

mg::BufferID mc::BufferMap::add_buffer(mg::BufferProperties const& properties)
{
    std::unique_lock<decltype(mutex)> lk(mutex);
    auto buffer = allocator->alloc_buffer(properties);
    buffers[buffer->id()] = {buffer, Owner::client};
    sink->send_buffer(stream_id, *buffer, mg::BufferIpcMsgType::full_msg);
    return buffer->id();
}

void mc::BufferMap::remove_buffer(mg::BufferID id)
{
    std::unique_lock<decltype(mutex)> lk(mutex);
    buffers.erase(checked_buffers_find(id, lk));
}

void mc::BufferMap::send_buffer(mg::BufferID id)
{
    std::unique_lock<decltype(mutex)> lk(mutex);
    auto it = buffers.find(id);
    if (it != buffers.end())
    {
        sink->send_buffer(stream_id, *it->second.buffer, mg::BufferIpcMsgType::update_msg);
        it->second.owner = Owner::client;
    }
}

void mc::BufferMap::receive_buffer(graphics::BufferID id)
{
    std::unique_lock<decltype(mutex)> lk(mutex);
    auto it = buffers.find(id);
    if (it != buffers.end())
        it->second.owner = Owner::server;
}

std::shared_ptr<mg::Buffer>& mc::BufferMap::operator[](mg::BufferID id)
{
    std::unique_lock<decltype(mutex)> lk(mutex);
    return checked_buffers_find(id, lk)->second.buffer;
}

mc::BufferMap::Map::iterator mc::BufferMap::checked_buffers_find(
    mg::BufferID id, std::unique_lock<std::mutex> const&)
{
    auto it = buffers.find(id);
    if (it == buffers.end())
        BOOST_THROW_EXCEPTION(std::logic_error("cannot find buffer by id"));
    return it;
}

size_t mc::BufferMap::client_owned_buffer_count()
{
    return 0;
}
