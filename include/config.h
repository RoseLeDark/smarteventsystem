#pragma once

#ifdef SES_BUILD
#define SES_API __declspec(dllexport)
#else
#define SES_API __declspec(dllimport)
#endif

#define TIMEDLOCK_INFINITY_WAIT 0