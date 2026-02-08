/**
 * this file is used to be included inside LWIP, so it cannot have any C++ dependencies
 * also declared global and extern to indicate external linkage, then lwip.c includes 
 * this file and has the symbol.
 * 
 *
*/
#pragma once
#include <stdbool.h>
extern bool ETH_is_cable_connected;
