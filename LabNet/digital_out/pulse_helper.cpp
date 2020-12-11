#include "pulse_helper.h"
#include "../interface/digital_messages.h"

namespace LabNet::digital_out
{

    struct TurnOn final : public so_5::signal_t
    {
    };
    struct TurnOff final : public so_5::signal_t
    {
    };

    PulseHelper::PulseHelper(context_t ctx,
        log::Logger logger,
        so_5::mbox_t dig_out_box,
        so_5::mbox_t report_box,
        so_5::mbox_t interface_box,
        interface::Interfaces interface,
        uint8_t pin)
        : so_5::agent_t(ctx)
        , logger_(logger)
        , dig_out_box_(dig_out_box)
        , report_box_(report_box)
        , interface_box_(interface_box)
        , interface_(interface)
        , pin_(pin)
    {
    }

    void PulseHelper::so_define_agent()
    {
        this >>= wait_state_;

        wait_state_
            .event([this](mhood_t<StartPulse> mes) {
                high_duration_ = mes->high_duration;
                low_duration_ = mes->low_duration;
                pulses_ = mes->pulses;

                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, true, so_direct_mbox());
                this >>= starting_state_;
            })
            .event([this](mhood_t<StopHelper> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, false, report_box_);
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<JustSwitch> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, mes->state, report_box_);
            });

        starting_state_
            .event([this](mhood_t<interface::digital_messages::ReturnDigitalOutState> mes) {
                this >>= running_state_;

                so_5::send<interface::digital_messages::ReturnDigitalOutState>(report_box_, interface_, mes->pin, mes->state, std::chrono::high_resolution_clock::now());

                turn_off_timer_ = so_5::send_periodic<TurnOff>(
                    so_direct_mbox(),
                    std::chrono::milliseconds(high_duration_),
                    std::chrono::milliseconds(high_duration_ + low_duration_));

                turn_on_timer_ = so_5::send_periodic<TurnOn>(
                    so_direct_mbox(),
                    std::chrono::milliseconds(high_duration_ + low_duration_),
                    std::chrono::milliseconds(high_duration_ + low_duration_));
            })
            .event([this](mhood_t<interface::digital_messages::InvalidDigitalOutPin> mes) {
                so_5::send<interface::digital_messages::ReturnDigitalOutState>(report_box_, interface_, mes->pin, false, std::chrono::high_resolution_clock::now());
            })
            .event([this](mhood_t<StopHelper> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, false, report_box_);
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<JustSwitch> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, mes->state, report_box_);
                this >>= wait_state_;
            });

        running_state_
            .event([this](mhood_t<TurnOff> mes) {
                if (pulses_ < 255)
                    pulses_--;

                if (pulses_ <= 0)
                {
                    so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, false, report_box_);

                    turn_off_timer_.release();
                    turn_on_timer_.release();
                }
                else
                {
                    so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, false, so_direct_mbox());
                }
            })
            .event([this](mhood_t<TurnOn> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, true, so_direct_mbox());
            })
            .event([this](mhood_t<StopHelper> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, false, report_box_);
                turn_off_timer_.release();
                turn_on_timer_.release();
                so_deregister_agent_coop_normally();
            })
            .event([this](mhood_t<StartPulse> mes) {
                turn_off_timer_.release();
                turn_on_timer_.release();

                high_duration_ = mes->high_duration;
                low_duration_ = mes->low_duration;
                pulses_ = mes->pulses;

                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, true, so_direct_mbox());
                this >>= starting_state_;
            })
            .event([this](mhood_t<JustSwitch> mes) {
                so_5::send<interface::digital_messages::SetDigitalOut>(interface_box_, interface_, pin_, mes->state, report_box_);
                turn_off_timer_.release();
                turn_on_timer_.release();

                this >>= wait_state_;
            });
        ;
    }
}