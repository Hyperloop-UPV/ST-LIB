/*
 * UART.hpp
 *
 *  Created on: 28 nov. 2022
 *      Author: Pablo
 */
#pragma once

#include "C++Utilities/CppUtils.hpp"
#include "ErrorHandler/ErrorHandler.hpp"


#define TXBUSYMASK 0b1
#define RXBUSYMASK 0b10

#define endl "\n\r"
/**
 * @brief UART service class. Abstracts the use of the UART service of the HAL library.
 * 
 */
class UART{

public:

    /**
     * @brief Enum which abstracts the use of the Instance struct to facilitate the mocking of the HALAL.Struct
     *
     */
    enum Peripheral{
    	peripheral1 = 0,
    	peripheral2 = 1,
        peripheral3 = 2,
		peripheral4 = 3,
		peripheral5 = 4,
		peripheral6 = 5,
		peripheral7 = 6,
		peripheral8 = 7,
		peripheral9 = 8,
		peripheral10 = 9
    };

    static uint16_t id_counter;
    

    static uint8_t printf_uart;
    static bool printf_ready;

    /**
	* @brief UART  wrapper enum of the STM32H723.
	*
	*/
    static UART::Peripheral uart1;
    static UART::Peripheral uart2;
    static UART::Peripheral uart3;
    static UART::Peripheral uart4;
    static UART::Peripheral uart5;
    static UART::Peripheral uart6;
    static UART::Peripheral uart7;
    static UART::Peripheral uart8;
    static UART::Peripheral uart9;
    static UART::Peripheral uart10;


    /**
     * @brief Registers a new UART.
     * 
     * @param uart UART peripheral to register.
     * @return uint8_t Id of the service.
     */
    static uint8_t inscribe(UART::Peripheral& uart){return 0;};

    /**
     * @brief This method initializes all registered UARTs. The peripherals
     * 		  must be enrolled before calling this method.
     * 
     */
    static void start(){ return;}

    /**@brief	Transmits 1 byte by DMA and interrupts.
     *          To send various packets in a row you must check if the UART is busy
     *          using is_busy(). All calls to this functions previous
     *          the bus is ready be ignored.
     * 
     * @param id Id of the UART
     * @param data data to be send
     * @return bool Returns true if the request to send the packet has been done
     *            successfully. Returns false if the UART is busy or a problem
     *            has occurred.
     */
    static bool transmit(uint8_t id, uint8_t data){return true;};

    /**@brief	Transmits size number of bytes by DMA and interrupts.
	 *          To send various packets in a row you must check if the UART is busy
	 *          using is_busy(). All calls to this functions previous
     *          the bus is ready be ignored.
	 *
	 * @param id Id of the UART
	 * @param data Data to be sent.
	 * @return bool Returns true if the request to send the packet has been done
	 *            successfully. Returns false if the UART is busy or a problem
	 *            has occurred.
	 */

    static bool transmit(uint8_t id, span<uint8_t> data){return true;};

    /**@brief	Transmits 1 byte by polling.
	 *
	 * @param id Id of the UART
	 * @param data Data to be sent.
	 * @return bool Returns true if the request to send the packet has been done
	 *            successfully. Returns false if the UART is busy or a problem
	 *            has occurred.
	 */
    static bool transmit_polling(uint8_t id, uint8_t data){return true;};

    /**@brief	Transmits size bytes by polling.
	 *
	 * @param id Id of the UART
	 * @param data Data to be sent.
	 * @return bool Returns true if the packet has been send successfully.
	 * 			    Returns false if the UART is busy or a problem has occurred.
	 */
    static bool transmit_polling(uint8_t id, span<uint8_t> data){return true;};

    /**						
     * @brief This method request the receive of size bytes
     *        by DMA and interrupts. Thus the data should not be used until
     *        you have checked that the value is already available using the 
     *        method has_next_paclet(). All calls to this functions previous
     *        the last packet is ready will be ignored.
     *
     * @see   UART::has_next_packet()
     * 
     * @param id Id of the UART
     * @param data Where data will be stored
     * @return bool Return true if the order to receive a new packet has been
     *            processed correctly. Return false if the UART is busy or a
     *            problem has occurred.
     */
    static bool receive(uint8_t id, span<uint8_t> data){return true;};

    /**
	* @brief This method receive size number of bytes by polling.
	*
	* @see   UART::has_next_packet()
	*
	* @param id Id of the UART
	* @param data Where data will be stored
	* @return bool Return true if the data has been read successfully.
	* 			   Return false if the UART is busy or a problem has occurred.
	*/
    static bool receive_polling(uint8_t id, span<uint8_t> data){return true;};

    /**
     * @brief This method is used to check if the UART receive operation has finished and data is ready.
     * 
     * @param id Id of the UART
     * @return bool Return true if the packet is ready to use and false if not.
     */
    static bool has_next_packet(uint8_t id){return true;};

    /**
     * @brief This method is used to check if the UART transmit operations busy.
     * 
     * @param id Id of the UART
     * @return bool Return true if the UART transmit operation is busy and false if not.
     */
    static bool is_busy(uint8_t id){return false;};

    /**
	 * @brief This method is used to set up the printf. It's inscribe and configure the selected UART to work
	 * 		  as standard and error output.
	 *
	 * @param uart Uart peripheral to be configured.
	 * @return bool True if everything went well. False if something has gone wrong.
	 */
    static bool set_up_printf(UART::Peripheral& uart){return true;};

    /**
  	 * @brief This method is used to print a message through the uart configured for printf.
  	 * 		  It only works if it has been configured correctly.
  	 *
  	 * @param ptr Pointer to the character string.
  	 * @return bool True if everything went well. False if something has gone wrong.
  	 */
    static void print_by_uart(char* ptr, int len){return;};

};

