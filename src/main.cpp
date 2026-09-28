#include "buffer.h"
#include "hex_view.h"
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/component/event.hpp>
#include <iostream>

int main(int argc, char* argv[]) 
{
    if (argc < 2) 
    {
        std::cerr << "Usage: hexed <file>\n";
        return 1;
    }

    Buffer buf(argv[1]);
    EditorState state;

    auto screen = ftxui::ScreenInteractive::Fullscreen();

    int rows = screen.dimy() - 2;
    if (rows < 1) rows = 20;

    auto view = MakeHexView(buf, state, rows);

    auto root = ftxui::CatchEvent(view, [&](ftxui::Event ev) -> bool 
    {

        if (ev == ftxui::Event::Character('q') ||
            ev == ftxui::Event::Character('Q')) 
        {
            screen.ExitLoopClosure()();
            return true;
        }

        if (ev == ftxui::Event::Character('s') ||
            ev == ftxui::Event::Character('S')) 
        {
            buf.save();
            return true;
        }
        
        return false;
    });

    screen.Loop(root);
    return 0;
}
