#pragma once


#if defined(_WIN32)
#define CVOX_API __declspec(dllexport)
#else
#define CVOX_API __attribute__((visibility("default")))
#endif