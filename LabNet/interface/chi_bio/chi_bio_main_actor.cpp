#include "chi_bio_main_actor.h"
#include "../../network/LabNet.pb.h"
#include "../../network/LabNetClient.pb.h"
#include "../../network/LabNetServer.pb.h"
#include "../interface_messages.h"
#include "../resources/resources_actor.h"
#include <cmath>
#include <wiringPiI2C.h>

namespace LabNet::interface::chi_bio
{
    struct TurnPumpOff
    {
        uint8_t pump_id;
        uint32_t msg_id;
    };

    ChiBioMainActor::ChiBioMainActor(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger)
        : so_5::agent_t(ctx)
        , self_box_(self_box)
        , interfaces_manager_box_(interfaces_manager_box)
        , events_box_(events_box)
        , logger_(logger)
        , res_box_(ctx.env().create_mbox("res_man"))
    {
        resources_.push_back(resources::WiringToResource(8));
        resources_.push_back(resources::WiringToResource(9));

        in1_ = std::string("In1");
        in2_ = std::string("In2");
        pumps_ = { "Pump1", "Pump2", "Pump3", "Pump4" };

        registers_[pumps_[0]][in1_]["ONL"] = 0x06;
        registers_[pumps_[0]][in1_]["ONH"] = 0x07;
        registers_[pumps_[0]][in1_]["OFFL"] = 0x08;
        registers_[pumps_[0]][in1_]["OFFH"] = 0x09;
        registers_[pumps_[0]][in2_]["ONL"] = 0x0A;
        registers_[pumps_[0]][in2_]["ONH"] = 0x0B;
        registers_[pumps_[0]][in2_]["OFFL"] = 0x0C;
        registers_[pumps_[0]][in2_]["OFFH"] = 0x0D;

        registers_[pumps_[1]][in1_]["ONL"] = 0x0E;
        registers_[pumps_[1]][in1_]["ONH"] = 0x0F;
        registers_[pumps_[1]][in1_]["OFFL"] = 0x10;
        registers_[pumps_[1]][in1_]["OFFH"] = 0x11;
        registers_[pumps_[1]][in2_]["ONL"] = 0x12;
        registers_[pumps_[1]][in2_]["ONH"] = 0x13;
        registers_[pumps_[1]][in2_]["OFFL"] = 0x14;
        registers_[pumps_[1]][in2_]["OFFH"] = 0x15;

        registers_[pumps_[2]][in1_]["ONL"] = 0x16;
        registers_[pumps_[2]][in1_]["ONH"] = 0x17;
        registers_[pumps_[2]][in1_]["OFFL"] = 0x18;
        registers_[pumps_[2]][in1_]["OFFH"] = 0x19;
        registers_[pumps_[2]][in2_]["ONL"] = 0x1A;
        registers_[pumps_[2]][in2_]["ONH"] = 0x1B;
        registers_[pumps_[2]][in2_]["OFFL"] = 0x1C;
        registers_[pumps_[2]][in2_]["OFFH"] = 0x1D;

        registers_[pumps_[3]][in1_]["ONL"] = 0x1E;
        registers_[pumps_[3]][in1_]["ONH"] = 0x1F;
        registers_[pumps_[3]][in1_]["OFFL"] = 0x20;
        registers_[pumps_[3]][in1_]["OFFH"] = 0x21;
        registers_[pumps_[3]][in2_]["ONL"] = 0x22;
        registers_[pumps_[3]][in2_]["ONH"] = 0x23;
        registers_[pumps_[3]][in2_]["OFFL"] = 0x24;
        registers_[pumps_[3]][in2_]["OFFH"] = 0x25;

        for (uint8_t i = 0; i < 16; i++)
        {
            pump_stat_[i] = std::pair<uint8_t, uint32_t> { 0, 0 };
        }
    }

    ChiBioMainActor::~ChiBioMainActor()
    {
    }

    void ChiBioMainActor::so_evt_finish()
    {
        if (res_reserved_)
        {
            so_5::send<LabNet::resources::ReleaseResourcesRequest>(res_box_, self_box_, self_box_, resources_, static_cast<uint16_t>(0));
        }

        // turn all off
        if (mux_handle_ && pump_handle_)
        {
            for (int i = 0; i < 4; i++)
            {
                using namespace std::chrono_literals;
                SelectChiBioBoard(i);

                // software chip reset
                wiringPiI2CWriteReg8(pump_handle_, 0x00, 0x06);
                std::this_thread::sleep_for(1ms);

                int led = wiringPiI2CReadReg8(pump_handle_, 0xFD); // turn all off
                led |= 0x10;
                wiringPiI2CWriteReg8(pump_handle_, 0xFD, led);
            }
        }

        so_5::send<InterfaceStopped>(interfaces_manager_box_, Interfaces::ChiBio);
        logger_->WriteInfoEntry("chi bio board finished");
    }

    void ChiBioMainActor::so_evt_start()
    {
        logger_->WriteInfoEntry("chi bio board started");
    }

    void ChiBioMainActor::so_define_agent()
    {
        this >>= init_state_;

        init_state_
            .event(self_box_,
                [this](const mhood_t<InitChiBio>& mes) {
                    so_5::send<resources::ReserveResourcesRequest>(res_box_, self_box_, self_box_, resources_, 1);
                })
            .event(self_box_,
                [this](const mhood_t<resources::ReserveResourcesReply> msg) {
                    if (msg->result)
                    {
                        res_reserved_ = true;

                        mux_handle_ = wiringPiI2CSetup(0x74);
                        if (mux_handle_ < 0)
                        {
                            logger_->WriteInfoEntry("chi bio mux open error: ");
                            so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::ChiBio, false);
                        }
                        else
                        {
                            pump_handle_ = wiringPiI2CSetup(0x61);
                            if (pump_handle_ < 0)
                            {
                                logger_->WriteInfoEntry("chi bio pump open error: ");
                                so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::ChiBio, false);
                            }
                            else
                            {
                                // boards setup
                                for (int i = 0; i < 4; i++)
                                {
                                    using namespace std::chrono_literals;
                                    SelectChiBioBoard(i);

                                    // software chip reset
                                    wiringPiI2CWriteReg8(pump_handle_, 0x00, 0x06);
                                    std::this_thread::sleep_for(1ms);

                                    int led = wiringPiI2CReadReg8(pump_handle_, 0xFD); // turn all off
                                    led |= 0x10;
                                    wiringPiI2CWriteReg8(pump_handle_, 0xFD, led);

                                    // set frequency - start
                                    wiringPiI2CWriteReg8(pump_handle_, 0x00, 0x10); // sleep on
                                    std::this_thread::sleep_for(1ms);

                                    SetPumpFreq(100); // after spleep, we can set new freq

                                    wiringPiI2CWriteReg8(pump_handle_, 0x00, 0x00); // sleep off
                                    std::this_thread::sleep_for(1ms);
                                    // set frequency - end
                                }

                                so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::ChiBio, true);
                                this >>= running_state_;
                            }
                        }
                    }
                    else
                    {
                        so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::ChiBio, false);
                    }
                });

        running_state_
            .event(self_box_,
                [this](const mhood_t<InitChiBio>& msg) {
                    so_5::send<InterfaceInitResult>(interfaces_manager_box_, Interfaces::ChiBio, true);
                })
            .event(self_box_,
                [this](std::shared_ptr<LabNetProt::Client::MoveChiBioPump> msg) {
                    if (msg->pump_id() < 16)
                    {
                        int board = msg->pump_id() / 4;
                        int pump = msg->pump_id() % 4;
                        int32_t move_time = abs(msg->move());

                        SelectChiBioBoard(board);
                        if (msg->move() < 0)
                        {
                            SetPumpDuty(pumps_[pump], in1_, 0.0f);
                            SetPumpDuty(pumps_[pump], in2_, 1.0f);
                            pump_stat_[msg->pump_id()].first = 1;
                        }
                        else if (msg->move() > 0)
                        {
                            SetPumpDuty(pumps_[pump], in1_, 1.0f);
                            SetPumpDuty(pumps_[pump], in2_, 0.0f);
                            pump_stat_[msg->pump_id()].first = 2;
                        }
                        else
                        {
                            SetPumpDuty(pumps_[pump], in1_, 0.0f);
                            SetPumpDuty(pumps_[pump], in2_, 0.0f);
                            pump_stat_[msg->pump_id()].first = 0;
                        }

                        pump_stat_[msg->pump_id()].second += 1;
                        if (move_time > 1)
                        {
                            so_5::send_delayed<TurnPumpOff>(self_box_, std::chrono::milliseconds(move_time), msg->pump_id(), pump_stat_[msg->pump_id()].second);
                        }

                        std::shared_ptr<LabNetProt::Server::ChiBioPumpMoveResult> state = std::make_shared<LabNetProt::Server::ChiBioPumpMoveResult>();
                        state->set_pump_id(msg->pump_id());
                        state->set_result(static_cast<LabNetProt::Server::ChiBioPumpMoveResult_MoveResult>(pump_stat_[msg->pump_id()].first));
                        so_5::send<std::shared_ptr<LabNetProt::Server::ChiBioPumpMoveResult>>(events_box_, state);
                    }
                })
            .event(self_box_,
                [this](const mhood_t<TurnPumpOff>& msg) {
                    if (msg->pump_id < 16 && pump_stat_[msg->pump_id].second == msg->msg_id)
                    {
                        int board = msg->pump_id / 4;
                        int pump = msg->pump_id % 4;

                        SetPumpDuty(pumps_[pump], in1_, 0.0f);
                        SetPumpDuty(pumps_[pump], in2_, 0.0f);
                        pump_stat_[msg->pump_id].first = 0;

                        std::shared_ptr<LabNetProt::Server::ChiBioPumpMoveResult> state = std::make_shared<LabNetProt::Server::ChiBioPumpMoveResult>();
                        state->set_pump_id(msg->pump_id);
                        state->set_result(static_cast<LabNetProt::Server::ChiBioPumpMoveResult_MoveResult>(pump_stat_[msg->pump_id].first));
                        so_5::send<std::shared_ptr<LabNetProt::Server::ChiBioPumpMoveResult>>(events_box_, state);
                    }
                })
            .event(self_box_,
                [this](mhood_t<PauseInterface>) {
                    // turn all off
                    for (int i = 0; i < 4; i++)
                    {
                        using namespace std::chrono_literals;
                        SelectChiBioBoard(i);

                        // software chip reset
                        wiringPiI2CWriteReg8(pump_handle_, 0x00, 0x06);
                        std::this_thread::sleep_for(1ms);

                        int led = wiringPiI2CReadReg8(pump_handle_, 0xFD); // turn all off
                        led |= 0x10;
                        wiringPiI2CWriteReg8(pump_handle_, 0xFD, led);
                    }

                    this >>= paused_state_;
                });

        paused_state_
            .event(self_box_,
                [this](mhood_t<ContinueInterface>) {
                    this >>= running_state_;
                });
    }

    int ChiBioMainActor::SetPumpFreq(int freq)
    {
        float prescaleValue = 25000000.0; // 25MHz
        prescaleValue /= 4096.0f; // 12-bit
        prescaleValue /= float(freq);
        prescaleValue -= 1.0;

        float prescale = static_cast<int>(floor(prescaleValue + 0.5));

        return wiringPiI2CWriteReg8(pump_handle_, 0xFE, (int)prescale);
    }

    void ChiBioMainActor::SetPumpDuty(std::string& pumpId, std::string& dir, float duty)
    {
        int data = (int)(duty * (4096 - 1));
        wiringPiI2CWriteReg8(pump_handle_, registers_.at(pumpId)[dir]["ONL"], 0);
        wiringPiI2CWriteReg8(pump_handle_, registers_.at(pumpId)[dir]["ONH"], 0);
        wiringPiI2CWriteReg8(pump_handle_, registers_.at(pumpId)[dir]["OFFL"], data & 0xFF);
        wiringPiI2CWriteReg8(pump_handle_, registers_.at(pumpId)[dir]["OFFH"], (data >> 8) & 0xFF);
    }

    int ChiBioMainActor::SelectChiBioBoard(int boardNbr)
    {
        return wiringPiI2CWriteReg8(mux_handle_, 0, 1 << boardNbr);
    }
};