#pragma once

#include "prerequisites.hpp"

namespace time_kill::core {
    struct WindowConfig {
        int width = 800;
        int height = 600;
        String title = "Time Kill Window";
        bool fullscreen = false;
        bool resizable = true;
    };
}
