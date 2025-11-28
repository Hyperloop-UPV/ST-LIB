/*
 * ST-LIB_HIGH.hpp
 *
 *  Created on: Dec 14, 2022
 *      Author: aleja
 */

#pragma once

// #include "Protections/Protection.hpp"
// #include "Protections/ProtectionManager.hpp"
#include "Control/ControlBlock.hpp"
#include "Control/FeedbackControlBlock.hpp"
#include "Control/SplitterBlock.hpp"
#include "Control/Blocks/Adder.hpp"
#include "Control/Blocks/Derivator.hpp"
#include "Control/Blocks/Integrator.hpp"
#include "Control/Blocks/MovingAverage.hpp"
#include "Control/Blocks/PI.hpp"
#include "Control/Blocks/PID.hpp"
#include "Control/Blocks/Saturator.hpp"
#include "Control/Blocks/MatrixMultiplier.hpp"
#include "Control/Blocks/MeanCalculator.hpp"
#include "Control/ControlSystem.hpp"
#ifdef SIM_ON
#else
	#include "FlashStorer/FlashStorer.hpp"
#endif
namespace STLIB_HIGH {
	void start();
}
