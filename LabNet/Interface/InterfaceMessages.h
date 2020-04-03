#pragma once

#include <so_5/all.hpp>
#include <string>

namespace Interface
{
	struct reset_all_interface final : public so_5::signal_t {};
	struct pause_all_interface final : public so_5::signal_t {};
	struct continue_all_interface final : public so_5::signal_t {};
}