#pragma once

//I don't know the consequences of this but for some reason anything below NT 4.0 isn't defined.
#ifndef _WIN32_WINNT_NT35
#define _WIN32_WINNT_NT35 0x0350
#endif