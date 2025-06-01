// SPDX-License-Identifier: EUPL-1.2

#pragma once
#include <cstdint>
#include <chrono>

#include "config.h"

namespace ses {

    class SES_API tool {
    public:
        /// <summary>
        /// Gibt die aktuelle Zeit seit dem Start des Programms in Millisekunden zurück.
        /// </summary>
        /// <returns>Die Anzahl der Millisekunden seit dem Start des Programms als uint64_t.</returns>
        static uint64_t now() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
        }
    };
}