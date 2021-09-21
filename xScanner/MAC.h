#pragma once

#include <string>

// MAC Lookup comes from here
// https://standards.ieee.org/products-services/regauth/index.html
// http://standards-oui.ieee.org/oui36/oui36.csv

std::string LookupMacAddress(const std::string mac);
