/*
 * FMAC.hpp
 *
 *  Created on: 30 nov. 2022
 *      Author: Ricardo
 */

#include "HALAL/Services/FMAC/FMAC.hpp"

MultiplierAccelerator::FMACMemoryLayout MultiplierAccelerator::MemoryLayout;
MultiplierAccelerator::FMACProcessInstance MultiplierAccelerator::Process;

void MultiplierAccelerator::IIR_software_in_software_out_inscribe(uint16_t input_coefficient_array_size, int16_t* input_coefficient_array, uint16_t feedback_coefficient_array_size, int16_t* feedback_coefficient_array){
	if(input_coefficient_array_size > 63 || feedback_coefficient_array_size > 63){
		ErrorHandler("Error while configurating IIR FMAC, no coefficient can be greater than 63");
		return;
	}

	MemoryLayout.FFiltersSize = input_coefficient_array_size;
	MemoryLayout.FFilterCoeffs = input_coefficient_array;
	MemoryLayout.FeedbackFilterSize = feedback_coefficient_array_size;
	MemoryLayout.FeedbackFilterCoeffs = feedback_coefficient_array;
	MemoryLayout.FInSize = 127 - MemoryLayout.FFiltersSize;
	MemoryLayout.FInValues = new int16_t[MemoryLayout.FInSize];
	MemoryLayout.FeedbackInSize = 127 - MemoryLayout.FeedbackFilterSize;
	MemoryLayout.FeedbackInValues = new int16_t[MemoryLayout.FeedbackInSize];
	MemoryLayout.OutSize = MemoryLayout.FeedbackInSize;
	MemoryLayout.OutValues = new int16_t[MemoryLayout.OutSize];
	Instance.mode = IIR;

	MultiplierAccelerator::inscribe();
}

void MultiplierAccelerator::inscribe(){
    constexpr auto fmac_dma_config {DMA::inscribe_stream<FMAC_BASE, DMA2_Stream0_BASE, DMA2_Stream1_BASE, DMA2_Stream2_BASE>()};
}

void MultiplierAccelerator::start(){
	if(Instance.mode != MultiplierAccelerator::None){
		Instance.hfmac->Instance = FMAC;
		if(HAL_FMAC_Init(Instance.hfmac)!= HAL_OK){
			ErrorHandler("Error while initialising the FMAC");
		}

		FMAC_FilterConfigTypeDef sFmacConfig;

		if(Instance.mode == IIR){
			sFmacConfig.CoeffBaseAddress  = 0;
			sFmacConfig.CoeffBufferSize   = MemoryLayout.FFiltersSize + MemoryLayout.FeedbackFilterSize;
			sFmacConfig.InputBaseAddress  = MemoryLayout.FFiltersSize + MemoryLayout.FeedbackFilterSize;
			sFmacConfig.InputBufferSize   = MemoryLayout.FInSize;
			sFmacConfig.InputThreshold    = FMAC_THRESHOLD_1;
			sFmacConfig.OutputBaseAddress = MemoryLayout.FFiltersSize + MemoryLayout.FeedbackFilterSize + MemoryLayout.FInSize;
			sFmacConfig.OutputBufferSize  = MemoryLayout.FeedbackInSize;
			sFmacConfig.OutputThreshold   = FMAC_THRESHOLD_1;
			sFmacConfig.pCoeffA           = MemoryLayout.FeedbackFilterCoeffs;
			sFmacConfig.CoeffASize        = MemoryLayout.FeedbackFilterSize;
			sFmacConfig.pCoeffB           = MemoryLayout.FFilterCoeffs;
			sFmacConfig.CoeffBSize        = MemoryLayout.FFiltersSize;
			sFmacConfig.Filter            = FMAC_FUNC_IIR_DIRECT_FORM_1;
			sFmacConfig.InputAccess       = FMAC_BUFFER_ACCESS_DMA;
			sFmacConfig.OutputAccess      = FMAC_BUFFER_ACCESS_DMA;
			sFmacConfig.Clip              = FMAC_CLIP_DISABLED;
			sFmacConfig.P                 = MemoryLayout.FFiltersSize;
			sFmacConfig.Q                 = MemoryLayout.FeedbackFilterSize;
			sFmacConfig.R                 = 0;

			if (HAL_FMAC_FilterConfig(Instance.hfmac, &sFmacConfig) != HAL_OK)
			{ErrorHandler("Error while configurating the FMAC");}

		}else{

		}

	}
}

void MultiplierAccelerator::software_preload(int16_t* preload_data, uint8_t amount_to_preload, int16_t* preload_feedback_data, uint8_t amount_to_feedback_preload){
#if FMAC_ERROR_CHECK != 0
	if(amount_to_preload > MemoryLayout.FInSize || amount_to_preload > MemoryLayout.FeedbackInSize){
		ErrorHandler("Error while preloading data, cannot preload more data than the structure can hold");
	}

	while(HAL_FMAC_GetState(Instance.hfmac) != HAL_FMAC_STATE_READY){

	}
#endif

	if (HAL_FMAC_FilterPreload(Instance.hfmac, preload_data, amount_to_preload, preload_feedback_data, amount_to_feedback_preload) != HAL_OK){
		ErrorHandler("Unexpected error on preload of data");
	}

}

void MultiplierAccelerator::software_run(int16_t* calculated_data, uint16_t* output_size){
	Process.running_output_data = calculated_data;
	Process.output_data_size = *output_size;
	Process.state = MultiplierAccelerator::WAITING_DATA;
	if(HAL_FMAC_FilterStart(Instance.hfmac, calculated_data, output_size) != HAL_OK){
		ErrorHandler("Error while starting FMAC");
	}
}

void MultiplierAccelerator::software_load_input(int16_t* input_data, uint16_t* input_size){
	Process.input_data = input_data;
	Process.input_data_size = *input_size;
	if(HAL_FMAC_AppendFilterData(Instance.hfmac, input_data, input_size)  != HAL_OK){
		ErrorHandler("Error while loading data into the FMAC");
	}
	Process.state = MultiplierAccelerator::RUNNING;
}


void MultiplierAccelerator::software_load_repeat(int16_t* input_data, uint16_t* input_size){
	Process.state = MultiplierAccelerator::RUNNING;

	Process.running_output_data = Process.output_data;
	if(HAL_FMAC_ConfigFilterOutputBuffer(Instance.hfmac, Process.running_output_data, &Process.output_data_size) != HAL_OK){
		ErrorHandler("Error while preparing buffer for the FMAC");
	}

	Process.input_data = input_data;
	Process.input_data_size = *input_size;
	if(HAL_FMAC_AppendFilterData(Instance.hfmac, input_data, input_size)  != HAL_OK){
		ErrorHandler("Error while loading data into the FMAC");
	}
}

void MultiplierAccelerator::software_load_full(int16_t* input_data, uint16_t* input_size, int16_t* output_data, uint16_t* output_size){
	Process.state = MultiplierAccelerator::RUNNING;

	Process.running_output_data = output_data;
	Process.output_data_size = *output_size;
	if(HAL_FMAC_ConfigFilterOutputBuffer(Instance.hfmac, output_data, output_size) != HAL_OK){
		ErrorHandler("Error while preparing buffer for the FMAC");
	}

	Process.input_data = input_data;
	Process.input_data_size = *input_size;
	if(HAL_FMAC_AppendFilterData(Instance.hfmac, input_data, input_size)  != HAL_OK){
		ErrorHandler("Error while loading data into the FMAC");
	}
}

void MultiplierAccelerator::software_stop(){
	if(HAL_FMAC_FilterStop(Instance.hfmac) != HAL_OK){
		ErrorHandler("Error while stopping FMAC");
	}
}

bool MultiplierAccelerator::is_ready(){
	return Instance.hfmac->State == HAL_FMAC_STATE_READY && Process.state != MultiplierAccelerator::RUNNING;
}

void HAL_FMAC_HalfOutputDataReadyCallback(FMAC_HandleTypeDef *hfmac){}


void HAL_FMAC_OutputDataReadyCallback(FMAC_HandleTypeDef *hfmac){
	SCB_CleanInvalidateDCache_by_Addr( (uint32_t*)MultiplierAccelerator::Process.running_output_data, sizeof(MultiplierAccelerator::Process.output_data));
	MultiplierAccelerator::Process.output_data = MultiplierAccelerator::Process.running_output_data;
	MultiplierAccelerator::Process.state = MultiplierAccelerator::WAITING_DATA;
}


void HAL_FMAC_ErrorCallback(FMAC_HandleTypeDef *hfmac){
	ErrorHandler("Error while running FMAC");
}


