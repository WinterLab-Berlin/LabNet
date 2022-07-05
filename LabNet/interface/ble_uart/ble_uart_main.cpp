#include "ble_uart_main.h"
#include "ble_uart_actor.h"
#include "../interface_messages.h"

using namespace LabNet::interface::ble_uart;

BleUartMain::BleUartMain(context_t ctx, const so_5::mbox_t self_box, const so_5::mbox_t interfaces_manager_box, const so_5::mbox_t events_box, log::Logger logger)
    : so_5::agent_t(ctx)
    , m_self_box(self_box)
    , m_interfaces_manager_box(interfaces_manager_box)
    , m_events_box(events_box)
    , m_logger(logger)
{

}

BleUartMain::~BleUartMain()
{

}

void BleUartMain::so_evt_finish()
{
    // child cooperation must be deregistered.
    for (const auto& [key, value] : m_coop) {
        so_environment().deregister_coop(value, so_5::dereg_reason::normal );    
    }
    
    so_5::send<InterfaceStopped>(m_interfaces_manager_box, Interfaces::BleUart);
    m_logger->WriteInfoEntry("ble uart finished");
}

void BleUartMain::so_evt_start()
{
}

void BleUartMain::so_define_agent()
{
    so_default_state()
        .event(m_self_box, [this](const mhood_t<InitBleUart>& mes)
        {
            std::map<std::string, so_5::mbox_t>::iterator it = m_devices.find(mes->device);
            if(it == m_devices.end())
            {
                so_5::mbox_t agent_box = so_environment().create_mbox();
                auto agent = so_environment().make_agent<BleUartActor>(agent_box, m_self_box, m_events_box, mes->device, static_cast<char>(m_devices.size()), m_logger);
                auto coop = so_environment().register_agent_as_coop(std::move(agent));
                
                m_devices[mes->device] = agent_box;
                m_coop[mes->device] = coop;
            }
        })
        .event(m_self_box, [this](mhood_t<PauseInterface>)
        {
            for (const auto& [key, value] : m_devices)
            {
                so_5::send<PauseInterface>(value);
            }
        })
        .event(m_self_box, [this](mhood_t<ContinueInterface>)
        {
            for (const auto& [key, value] : m_devices)
            {
                so_5::send<ContinueInterface>(value);
            }
        })
        .event(m_self_box, [this](const mhood_t<InterfaceInitResult>& mes)
        {
            so_5::send<InterfaceInitResult>(m_interfaces_manager_box, mes->interface, mes->is_succeed);
        });
}