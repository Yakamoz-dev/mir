/*
 * Copyright © 2014-2017 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mir_test_framework/async_server_runner.h"
#include "mir/test/popen.h"
#include <mir_test_framework/executable_path.h>
#include <miral/x11_support.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <string>

namespace mo = mir::options;
namespace mtf = mir_test_framework;
using namespace testing;

namespace
{
struct AbstractGLMark2Test : testing::Test, mtf::AsyncServerRunner {
    void SetUp() override {
        start_server();
    }

    void TearDown() override {
        stop_server();
    }

    enum ResultFileType {
        raw, json
    };

    virtual char const *command() =0;

    int run_glmark2(char const *args) {
        ResultFileType file_type = raw; // Should this still be selectable?

        auto const cmd = std::string{} + command() + " -b build " + args;
        mir::test::Popen p(cmd);

        const ::testing::TestInfo *const test_info =
                ::testing::UnitTest::GetInstance()->current_test_info();

        char output_filename[256];
        snprintf(output_filename, sizeof(output_filename) - 1,
                 "/tmp/%s_%s.log",
                 test_info->test_case_name(), test_info->name());

        printf("Saving GLMark2 detailed results to: %s\n", output_filename);
        // ^ Although I would vote to just print them to stdout instead

        std::string line;
        std::ofstream glmark2_output;
        int score = -1;
        glmark2_output.open(output_filename);
        while (p.get_line(line)) {
            int match;
            if (sscanf(line.c_str(), " glmark2 Score: %d", &match) == 1) {
                score = match;
            }

            if (file_type == raw) {
                glmark2_output << line << std::endl;
            }
        }

        if (file_type == json) {
            std::string json = "{";
            json += "\"benchmark_name\":\"glmark2-es2-mir\"";
            json += ",";
            json += "\"score\":\"" + std::to_string(score) + "\"";
            json += "}";
            glmark2_output << json;
        }

        return score;
    }

    static const int minimal_framerate = 100;
};

struct GLMark2Xwayland : AbstractGLMark2Test
{
    GLMark2Xwayland()
    {
        // This is a slightly awkward method of enabling X11 support,
        // but refactoring these tests to use MirAL can wait.
        miral::X11Support{}(server);
        add_to_environment("MIR_SERVER_ENABLE_X11", "1");
    }

    char const* command() override
    {
        add_to_environment("DISPLAY", server.x11_display().value().c_str());
        static char const* const command = "glmark2-es2";
        return command;
    }
};

struct GLMark2Wayland : AbstractGLMark2Test
{
    GLMark2Wayland()
    {
        add_to_environment("WAYLAND_DISPLAY",                "GLMark2Wayland");
    }

    char const* command() override
    {
        static char const* const command = "glmark2-es2-wayland";
        return command;
    }
};

TEST_F(GLMark2Wayland, fullscreen)
{
    EXPECT_THAT(run_glmark2("--fullscreen"), Ge(minimal_framerate));
}

TEST_F(GLMark2Wayland, windowed)
{
    EXPECT_THAT(run_glmark2(""), Ge(minimal_framerate));
}

TEST_F(GLMark2Xwayland, fullscreen)
{
    EXPECT_THAT(run_glmark2("--fullscreen"), Ge(minimal_framerate));
}

TEST_F(GLMark2Xwayland, windowed)
{
    EXPECT_THAT(run_glmark2(""), Ge(minimal_framerate));
}
}
