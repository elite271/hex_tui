#pragma once
#include "buffer.h"
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>


static constexpr size_t BYTES_PER_ROW = 16;

struct EditorState 
{
    size_t cursor   = 0;   
    size_t scroll   = 0;   
    bool   high_nib = true; 
    bool   modified  = false;
    std::string status_msg;
};



class HexViewComponent : public ftxui::ComponentBase
{
public:
    HexViewComponent(Buffer& buf, EditorState& state, int rows);
    ftxui::Element Render() override;
    bool OnEvent(ftxui::Event event) override;

private:
    void clamp_scroll();

    Buffer& buf_;
    EditorState& state_;
    int visible_rows_;
};


ftxui::Component MakeHexView(Buffer& buf, EditorState& state, int rows);