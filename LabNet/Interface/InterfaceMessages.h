#pragma once

#include <so_5/all.hpp>
#include <string>

namespace Interface
{
	struct reset_interface final : public so_5::signal_t {};
	struct pause_interface final : public so_5::signal_t {};
	struct continue_interface final : public so_5::signal_t {};
}