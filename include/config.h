/**
* SES Smart Event System
* (c) 2025 Amber-Sophia Schröck
* This file is copyright under the latest version of the EUPL.
* Please see LICENSE file for your rights under this license.
**/
#pragma once

#ifdef SES_BUILD
#define SES_API __declspec(dllexport)
#else
#define SES_API __declspec(dllimport)
#endif

#define TIMEDLOCK_INFINITY_WAIT 0