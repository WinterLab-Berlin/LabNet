#include "resource.h"
#include <map>

namespace LabNet
{
    namespace resources
    {
        std::map<Resource, int32_t> resource_to_wiring_map = {
            { Resource::Gpio0, 30 },
            { Resource::Gpio1, 31 },
            { Resource::Gpio2, 8 },
            { Resource::Gpio3, 9 },
            { Resource::Gpio4, 7 },
            { Resource::Gpio5, 21 },
            { Resource::Gpio6, 22 },
            { Resource::Gpio7, 11 },
            { Resource::Gpio8, 10 },
            { Resource::Gpio9, 13 },
            { Resource::Gpio10, 12 },
            { Resource::Gpio11, 14 },
            { Resource::Gpio12, 26 },
            { Resource::Gpio13, 23 },
            { Resource::Gpio14, 15 },
            { Resource::Gpio15, 16 },
            { Resource::Gpio16, 27 },
            { Resource::Gpio17, 0 },
            { Resource::Gpio18, 1 },
            { Resource::Gpio19, 24 },
            { Resource::Gpio20, 28 },
            { Resource::Gpio21, 29 },
            { Resource::Gpio22, 3 },
            { Resource::Gpio23, 4 },
            { Resource::Gpio24, 5 },
            { Resource::Gpio25, 6 },
            { Resource::Gpio26, 25 },
            { Resource::Gpio27, 2 }
        };

        std::map<int32_t, Resource> wiring_to_resource_map = {
            { 0, Resource::Gpio17 },
            { 1, Resource::Gpio18 },
            { 2, Resource::Gpio27 },
            { 3, Resource::Gpio22 },
            { 4, Resource::Gpio23 },
            { 5, Resource::Gpio24 },
            { 6, Resource::Gpio25 },
            { 7, Resource::Gpio4 },
            { 8, Resource::Gpio2 },
            { 9, Resource::Gpio3 },
            { 10, Resource::Gpio8 },
            { 11, Resource::Gpio7 },
            { 12, Resource::Gpio10 },
            { 13, Resource::Gpio9 },
            { 14, Resource::Gpio11 },
            { 15, Resource::Gpio14 },
            { 16, Resource::Gpio15 },
            { 21, Resource::Gpio5 },
            { 22, Resource::Gpio6 },
            { 23, Resource::Gpio13 },
            { 24, Resource::Gpio19 },
            { 25, Resource::Gpio26 },
            { 26, Resource::Gpio12 },
            { 27, Resource::Gpio16 },
            { 28, Resource::Gpio20 },
            { 29, Resource::Gpio21 },
            { 30, Resource::Gpio0 },
            { 31, Resource::Gpio1 }
        };

        int32_t ResourceToWiring(Resource bcm)
        {
            auto it = resource_to_wiring_map.find(bcm);
            if (it != resource_to_wiring_map.end())
                return it->second;
            else
            {
                return -1;
            }
        }

        int32_t ResourceToWiring(int32_t bcm)
        {
            if (bcm < 0 || bcm > 27)
                return -1;
            else
                return ResourceToWiring(static_cast<Resource>(bcm));
        }

        Resource WiringToResource(int32_t wiring)
        {
            auto it = wiring_to_resource_map.find(wiring);
            if (it != wiring_to_resource_map.end())
                return it->second;
            else
            {
                return Resource::None;
            }
        }
    }
}