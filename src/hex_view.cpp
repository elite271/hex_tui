#include "hex_view.h"
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace ftxui;

static std::string byte_to_hex(uint8_t b)
{
	std::ostringstream ss;

	ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
		<< static_cast<int>(b);

	return ss.str();
}

static char byte_to_ascii(uint8_t b)
{
	return (b >= 0x20 && b < 0x7f) ? static_cast<char>(b) : '.';
}

HexViewComponent::HexViewComponent(Buffer& buf, EditorState& state, int rows)
	: buf_(buf), state_(state), visible_rows_(rows)
{
}


Element HexViewComponent::Render()
{
	Elements rows;

	size_t first_byte = state_.scroll * BYTES_PER_ROW;

	for (int r = 0; r < visible_rows_; ++r)
	{
		size_t row_offset = first_byte + r * BYTES_PER_ROW;

		if (row_offset >= buf_.size())
		{
			break;
		}

		auto chunk = buf_.read(row_offset, BYTES_PER_ROW);

		std::ostringstream oss;
		oss << std::uppercase << std::hex << std::setw(8)
			<< std::setfill('0') << row_offset;

		Element offset_el = text(oss.str()) | color(Color::GrayDark);

		Elements hex_cells;
		for (size_t i = 0; i < BYTES_PER_ROW; ++i)
		{
			if (i > 0 && i % 8 == 0)
			{
				hex_cells.push_back(text(" "));
			}

			if (i < chunk.size())
			{
				size_t abs = row_offset + i;
				std::string h = byte_to_hex(chunk[i]);
				Element cell = text(h + " ");

				if (abs == state_.cursor)
				{
					cell = cell | bgcolor(Color::Blue) | color(Color::White);
				}
				else if (chunk[i] == 0x00)
				{
					cell = cell | color(Color::GrayDark);
				}
				else
				{
					cell = cell | color(Color::White);
				}

				hex_cells.push_back(cell);
			}
			else
			{
				hex_cells.push_back(text("   "));
			}
		}
		Element hex_el = hbox(hex_cells);

		Elements ascii_cells;
		ascii_cells.push_back(text("│") | color(Color::GrayDark));

		for (size_t i = 0; i < chunk.size(); ++i)
		{
			size_t abs = row_offset + i;
			std::string ch(1, byte_to_ascii(chunk[i]));
			Element ac = text(ch);

			if (abs == state_.cursor)
			{
				ac = ac | bgcolor(Color::Blue) | color(Color::White);
			}
			else if (chunk[i] == 0x00)
			{
				ac = ac | color(Color::GrayDark);
			}

			ascii_cells.push_back(ac);
		}
		Element ascii_el = hbox(ascii_cells);

		rows.push_back(hbox({
			offset_el,
			text("  "),
			hex_el,
			text(" "),
			ascii_el,
			}));
	}

	std::ostringstream status;
	status << " " << buf_.path()
		<< (buf_.dirty() ? " [+]" : "")
		<< "  offset: 0x" << std::uppercase << std::hex << state_.cursor
		<< "  nibble: " << (state_.high_nib ? "HI" : "LO")
		<< "  [q]uit [s]ave [←→↑↓] navigate";

	Element status_bar = text(status.str())
		| bgcolor(Color::Blue) | color(Color::White) | flex;

	return vbox({
		vbox(rows) | flex,
		separator(),
		status_bar,
		});
}

bool HexViewComponent::OnEvent(Event event)
{
	size_t file_size = buf_.size();
	size_t& cur = state_.cursor;

	if (event == Event::ArrowRight)
	{
		if (cur + 1 < file_size)
		{
			++cur; state_.high_nib = true;
		}

		clamp_scroll();

		return true;
	}
	if (event == Event::ArrowLeft)
	{
		if (cur > 0)
		{
			--cur;
			state_.high_nib = true;
		}

		clamp_scroll();

		return true;
	}
	if (event == Event::ArrowDown)
	{
		if (cur + BYTES_PER_ROW < file_size)
		{
			cur += BYTES_PER_ROW;
		}

		clamp_scroll();

		return true;
	}
	if (event == Event::ArrowUp)
	{
		if (cur >= BYTES_PER_ROW)
		{
			cur -= BYTES_PER_ROW;
		}

		clamp_scroll();

		return true;
	}

	if (event == Event::PageDown)
	{
		size_t step = visible_rows_ * BYTES_PER_ROW;
		cur = std::min(cur + step, file_size - 1);
		clamp_scroll();

		return true;
	}
	if (event == Event::PageUp)
	{
		size_t step = visible_rows_ * BYTES_PER_ROW;
		cur = (cur >= step) ? cur - step : 0;
		clamp_scroll();

		return true;
	}

	if (event.is_character())
	{
		char c = event.character()[0];
		int  nibble = -1;

		if (c >= '0' && c <= '9') nibble = c - '0';
		else if (c >= 'a' && c <= 'f') nibble = c - 'a' + 10;
		else if (c >= 'A' && c <= 'F') nibble = c - 'A' + 10;

		if (nibble >= 0 && cur < file_size)
		{
			auto chunk = buf_.read(cur, 1);
			uint8_t byte = chunk.empty() ? 0 : chunk[0];

			if (state_.high_nib)
			{
				byte = (byte & 0x0F) | (nibble << 4);
			}
			else
			{
				byte = (byte & 0xF0) | nibble;
			}

			buf_.write_byte(cur, byte);

			if (!state_.high_nib && cur + 1 < file_size) ++cur;

			state_.high_nib = !state_.high_nib;
			clamp_scroll();

			return true;
		}
	}

	return false;
}


void HexViewComponent::clamp_scroll()
{
	size_t cursor_row = state_.cursor / BYTES_PER_ROW;

	if (cursor_row >= state_.scroll + visible_rows_)
	{
		state_.scroll = cursor_row - visible_rows_ + 1;
	}

	if (cursor_row < state_.scroll)
	{
		state_.scroll = cursor_row;
	}
}


ftxui::Component MakeHexView(Buffer& buf, EditorState& state, int rows)
{
	return Make<HexViewComponent>(buf, state, rows);
}
