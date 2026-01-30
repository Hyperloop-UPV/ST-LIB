/*
 * FMAC.hpp
 *
 *  Created on: 30 nov. 2022
 *      Author: Ricardo
 */

#pragma once

#include "HALAL/Models/DMA/DMA.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#ifndef FMAC_ERROR_CHECK
#define FMAC_ERROR_CHECK 0
#endif


#ifdef HAL_FMAC_MODULE_ENABLED

class MultiplierAccelerator{
public:
	enum FMACmodes{
		None,
		FIR,
		IIR,
	};

	enum FMACstates{
		NOT_RUNNING,
		WAITING_DATA,
		RUNNING,
	};


	struct FMACMemoryLayout{
		int16_t *FeedbackFilterCoeffs;
		uint16_t FeedbackFilterSize;
		int16_t *FFilterCoeffs;
		uint16_t FFiltersSize;
		int16_t *FeedbackInValues;
		uint16_t FeedbackInSize;
		int16_t *FInValues;
		uint16_t FInSize;
		int16_t *OutValues;
		int16_t *AuxiliarOutValues;
		uint16_t OutSize;
	};

	struct FMACProcessInstance{
		FMACstates state = NOT_RUNNING;
		int16_t* input_data;
		uint16_t input_data_size;
		int16_t* output_data;
		int16_t *running_output_data;
		uint16_t output_data_size;
	};

	struct FMACInstance{
		FMACmodes mode;
		FMAC_HandleTypeDef *hfmac;
	};

	static FMACInstance Instance;
	static FMACMemoryLayout MemoryLayout;
	static FMACProcessInstance Process;

	/**
	 * @brief Inscribe to make the FMAC run in IIR mode with software inputs and software outputs.
	 */
	static void IIR_software_in_software_out_inscribe(uint16_t input_coefficient_array_size, int16_t* input_coefficient_array, uint16_t feedback_coefficient_array_size, int16_t* feedback_coefficient_array);

	/**
	 * @brief used in the HALAL::start() to end the configuration of the FMAC.
	 */
	static void start();

	/**
	 * @brief preloads data to be run into the FMAC. Recommended to preload at least as much zeroes as Feedback Coefficients in the feedback data if no data is planned to be preloaded.
	 */
	static void software_preload(int16_t* preload_data, uint8_t amount_to_preload, int16_t* preload_feedback_data, uint8_t amount_to_feedback_preload);

	/**
	 * @brief orders the FMAC to start processing, and will run calculations with the preloaded data.
	 */
	static void software_run(int16_t* calculated_data, uint16_t* output_size);

	/**
	 * @brief loads input and only starts a run if the output was loaded with a software function
	 */
	static void software_load_input(int16_t* input_data, uint16_t* input_size);

	/**
	 * @brief loads input and starts a run using the same output pointer as the last run
	 */
	static void software_load_repeat(int16_t* input_data, uint16_t* input_size);

	/**
	 * @brief loads both input and output pointers and starts a run
	 */
	static void software_load_full(int16_t* input_data, uint16_t* input_size, int16_t* output_data, uint16_t* output_size);

	/**
	 * @brief makes the FMAC stop processing. Needed to change the Coefficients.
	 */
	static void software_stop();

	/**
	 * @brief wether or not the FMAC is ready to receive more data. Should be called before using any load function to check if the peripheral is ready.
	 */
	static bool is_ready();
private:

	/**
	 * @brief generic process used on all inscribe type functions
	*/
	static void inscribe();

	/**
	 * @brief method that configurates the FMAC, used in all init methods
	 * 
	 * @param FeedbackFiltersCoeffs pointer to the Coefficients used in the Feedback mode. Declare them as nullptr on FIR as they will not be used. Known as A coefficients in FMAC HAL peripheral
	 * @param FeedbackFilterSize number of FeedbackCoefficients, must be one less than FFiltersSize in normal Feedback use, and 0 on any FIR use
	 * @param FFiltersCoeffs pointer to the Coefficients used in all modes as the external input. 
	 * @param FFiltersSize number of FIRCoefficients, equal to the inputs computated in one calculation
	 * @param 
	*/
	static void configure(int16_t *FeedbackFiltersCoeffs, uint8_t FeedbackFilterSize, int16_t *FFiltersCoeffs, uint8_t FFiltersSize,
	int16_t *FeedbackInValues, uint8_t FeedbackInSize, int16_t *FInValues, uint8_t FInSize, int16_t *OutValues, uint8_t OutSize, FMACmodes mode);
};

#endif
