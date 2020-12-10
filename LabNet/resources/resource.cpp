#include "resource.h"
#include <map>

namespace LabNet
{
    namespace resources
    {
        std::map<resource, int32_t> resource_to_wiring_map = {
            { resource::GPIO0, 30 },
            { resource::GPIO1, 31 },
            { resource::GPIO2, 8 },
            { resource::GPIO3, 9 },
            { resource::GPIO4, 7 },
            { resource::GPIO5, 21 },
            { resource::GPIO6, 22 },
            { resource::GPIO7, 11 },
            { resource::GPIO8, 10 },
            { resource::GPIO9, 13 },
            { resource::GPIO10, 12 },
            { resource::GPIO11, 14 },
            { resource::GPIO12, 26 },
            { resource::GPIO13, 23 },
            { resource::GPIO14, 15 },
            { resource::GPIO15, 16 },
            { resource::GPIO16, 27 },
            { resource::GPIO17, 0 },
            { resource::GPIO18, 1 },
            { resource::GPIO19, 24 },
            { resource::GPIO20, 28 },
            { resource::GPIO21, 29 },
            { resource::GPIO22, 3 },
            { resource::GPIO23, 4 },
            { resource::GPIO24, 5 },
            { resource::GPIO25, 6 },
            { resource::GPIO26, 25 },
            { resource::GPIO27, 2 }
        };

        std::map<int32_t, resource> wiring_to_resource_map = {
            { 0, resource::GPIO17 },
            { 1, resource::GPIO18 },
            { 2, resource::GPIO27 },
            { 3, resource::GPIO22 },
            { 4, resource::GPIO23 },
            { 5, resource::GPIO24 },
            { 6, resource::GPIO25 },
            { 7, resource::GPIO4 },
            { 8, resource::GPIO2 },
            { 9, resource::GPIO3 },
            { 10, resource::GPIO8 },
            { 11, resource::GPIO7 },
            { 12, resource::GPIO10 },
            { 13, resource::GPIO9 },
            { 14, resource::GPIO11 },
            { 15, resource::GPIO14 },
            { 16, resource::GPIO15 },
            { 21, resource::GPIO5 },
            { 22, resource::GPIO6 },
            { 23, resource::GPIO13 },
            { 24, resource::GPIO19 },
            { 25, resource::GPIO26 },
            { 26, resource::GPIO12 },
            { 27, resource::GPIO16 },
            { 28, resource::GPIO20 },
            { 29, resource::GPIO21 },
            { 30, resource::GPIO0 },
            { 31, resource::GPIO1 }
        };

        int32_t resource_to_wiring(resource bcm)
        {
            auto it = resource_to_wiring_map.find(bcm);
            if (it != resource_to_wiring_map.end())
                return it->second;
            else
            {
                return -1;
            }
        }

        int32_t resource_to_wiring(int32_t bcm)
        {
            if (bcm < 0 || bcm > 27)
                return -1;
            else
                resource_to_wiring(static_cast<resource>(bcm));
        }

        resource wiring_to_resource(int32_t wiring)
        {
            auto it = wiring_to_resource_map.find(wiring);
            if (it != wiring_to_resource_map.end())
                return it->second;
            else
            {
                return resource::None;
            }
        }
    }
}