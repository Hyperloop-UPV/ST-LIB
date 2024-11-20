#pragma once

#include "HALALMock/Models/PinModel/Pin.hpp"
#include "HALALMock/Models/Packets/SPIOrder.hpp"
#include "HALALMock/Services/DigitalOutputService/DigitalOutputService.hpp"
#include "HALALMock/Services/DigitalInputService/DigitalInputService.hpp"
#include "ErrorHandler/ErrorHandler.hpp"

#define MASTER_SPI_CHECK_DELAY 100000 //how often the master should check if the slave is ready, in nanoseconds

#define MASTER_MAXIMUM_QUEUE_LEN 10

/**
 * @brief SPI service class. Abstracts the use of the SPI service of the HAL library.
 * 
 */
class SPI{
public:
	/**
	 * @brief Possible states of the SPI class
	 */
    enum SPIstate{
    	IDLE = 0,
		STARTING_ORDER,
    	WAITING_ORDER,
		PROCESSING_ORDER,
		ERROR_RECOVERY,
    };

    enum class SPIMode {
        MASTER,
        SLAVE
    };

    /**
     * @brief Struct which defines all data referring to SPI peripherals. DO NOT MODIFY ON RUN TIME.
     * declared public so HAL callbacks can directly access the instance
     * NOT intended to be modified after the inscribe of the spi, configuration should be made in Runes.hpp
     *           
     */
    struct Instance{
        Pin* SCK; /**< Clock pin. */
        Pin* MOSI; /**< MOSI pin. */
        Pin* MISO; /**< MISO pin. */
        Pin* SS; /**< Slave select pin. */
        std::pair<char*, uint16_t> destination_address;
        int socket;
        Pin* RS; /**< Ready Slave pin (optional)*/
        uint8_t RShandler;

        uint32_t baud_rate_prescaler; /**< SPI baud prescaler.*/
        SPIMode mode = SPIMode::MASTER; /**< SPI mode. */

        bool initialized = false; /**< Peripheral has already been initialized */

        bool use_DMA = false;
        bool using_ready_slave = false;
        string name;
        SPIstate state = IDLE; /**< State of the spi on the Order communication*/
        uint8_t *rx_buffer;
        uint8_t *tx_buffer;
        uint16_t *available_end; /**< variable that checks for what Order id is the other end ready*/
        uint16_t *SPIOrderID; /**< SPIOrder being processed, if any*/
        RingBuffer<uint16_t, MASTER_MAXIMUM_QUEUE_LEN> SPIOrderQueue;  /**< Queue of SPIOrders to process after this one*/
        uint64_t last_end_check = 0; /**< last clock cycle where the available end was checked*/
        uint64_t Order_count = 0; /**< Order completed counter for debugging (success rate)*/
        uint64_t try_count = 0; /**< Tries from the master to communicate a packet with the slave for debugging (affected by how much the slave delays on preparing a packet)*/
        uint64_t error_count = 0; /**< Order error counter for debugging (affected by how much the bits sent are corrupted in any way)*/

        std::jthread sender_thread;
        std::queue<span<uint8_t>> transmission_queue;
        std::mutex transmission_mx;
        std::condition_variable cv_transmission;

        std::jthread receiver_thread;
        std::queue<span<uint8_t>> reception_queue;
        std::mutex reception_mx;
        std::condition_variable cv_reception;

        std::jthread sender_receiver_thread;
        std::queue<std::pair<span<uint8_t>, span<uint8_t>>> transmission_reception_queue;
        std::mutex transmission_reception_mx;
        std::condition_variable cv_transmission_reception;

        uint16_t port;
    };

    /**
     * @brief Enum that abstracts the use of the Instance struct to facilitate the mocking of the HALAL.
     *
     */
    enum Peripheral{
     	peripheral1 = 0,
     	peripheral2 = 1,
        peripheral3 = 2,
 		peripheral4 = 3,
 		peripheral5 = 4,
 		peripheral6 = 5,
     };

    static uint16_t id_counter;
    
    static map<uint8_t, SPI::Instance* > registered_spi;

    static unordered_map<SPI::Peripheral, SPI::Instance*> available_spi;

    /**
     * @brief SPI 3 wrapper enum of the STM32H723.
     *
     */
    static SPI::Peripheral spi1;
    static SPI::Peripheral spi2;
    static SPI::Peripheral spi3;
    static SPI::Peripheral spi4;
    static SPI::Peripheral spi5;
    static SPI::Peripheral spi6;

    static SPI::Instance instance1;
	static SPI::Instance instance2;
	static SPI::Instance instance3;
	static SPI::Instance instance4;
	static SPI::Instance instance5;
	static SPI::Instance instance6;

    static std::string ip;

    /*=========================================
     * User functions for configuration of the SPI
     ==========================================*/

    /**
     * @brief Registers a new SPI.
     * 
     * @param spi SPI peripheral to register.
     * @return uint8_t Id of the service.
     */
    static uint8_t inscribe(SPI::Peripheral& spi);


    /**
     * @brief assigns the RS pin to the SPI.
     */
    static void assign_RS(uint8_t id, Pin& RSPin);

    /**
     * @brief Method that initializes all enrolled SPI peripherals
     *        service. The peripherals that you want to use
     *  	  must be enrolled before initializing.
     */
    static void start();



    /*=========================================
     * User functions for dummy mode
     ==========================================*/

    /**@brief	Transmits 1 byte by SPI.
     * 
     * @param id Id of the SPI
     * @param data Data to be send
     * @return bool Returns true if the data has been send successfully.
     * 			    Returns false if a problem has occurred.
     */
    static bool transmit(uint8_t id, uint8_t data);

    /**@brief	Transmits size bytes by SPI.
	 *
	 * @param id Id of the SPI
	 * @param data Data to be send
	 * @param size Size in bytes to be send
	 * @return bool Returns true if the data has been send successfully.
	 * 			    Returns false if a problem has occurred.
	 */
    static bool transmit(uint8_t id, span<uint8_t> data);

    /**@brief	Transmits size bytes by SPI via DMA
	 *
	 * @param id Id of the SPI
	 * @param data Data to be send
	 * @param size Size in bytes to be send
	 * @return bool Returns true if the data has been send successfully.
	 * 			    Returns false if a problem has occurred.
	 */
    static bool transmit_DMA(uint8_t id, span<uint8_t> data);
    /**						
     * @brief This method requests an array of data. The data
     * 		  will be stored in data parameter. You must make sure you have
     * 		  enough space
     * 
     * @param id Id of the SPI
     * @param data Pointer where data will be stored
     * @param size Size in bytes to receive.
     * @return bool Returns true if the data have been read successfully.
     * 			    Returns false if a problem has occurred.
     */
    static bool receive(uint8_t id, span<uint8_t> data);
    /**						
     * @brief This method requests an array of data. The data
     * 		  will be stored in data parameter. You must make sure you have
     * 		  enough space, this function uses DMA
     * 
     * @param id Id of the SPI
     * @param data Pointer where data will be stored
     * @param size Size in bytes to receive.
     * @return bool Returns true if the data have been read successfully.
     * 			    Returns false if a problem has occurred.
     */
    static bool receive_DMA(uint8_t id, span<uint8_t> data);

    /**
	 * @brief This method transmits one order of command_size bytes and
	 * 		  then stores the data received after that order.
	 *
	 * @param id Id of the SPI
	 * @param command_data Command
	 * @param command_size Command size in bytes to receive
	 * @param receive_data Pointer where data will be stored
	 * @param receive_size Number of bytes to read
	 * @return bool Returns true if the data have been read successfully.
	 * 			    Returns false if a problem has occurred.
	 */
    static bool transmit_and_receive(uint8_t id, span<uint8_t> command_data, span<uint8_t> receive_data);



    /*=============================================
     * User functions for Order mode
     ==============================================*/

    /**
     * @brief master send Order method, which tries to send a single Order
     */
    static bool master_transmit_Order(uint8_t id, SPIBaseOrder &Order);
    static bool master_transmit_Order(uint8_t id, SPIBaseOrder *Order);

    /**
     * @brief slave listen Orders method. When called, the slave will start to listen Orders until the state is set again to IDLE
     */
    static void slave_listen_Orders(uint8_t id);

    /**
	 * @brief This method transmits one order of command_size bytes and
	 * 		  then stores the data received after that order using DMA
	 *
	 * @param id Id of the SPI
	 * @param command_data Command
	 * @param command_size Command size in bytes to receive
	 * @param receive_data Pointer where data will be stored
	 * @param receive_size Number of bytes to read
	 * @return bool Returns true if the data have been read successfully.
	 * 			    Returns false if a problem has occurred.
	 */
    static bool transmit_and_receive_DMA(uint8_t id, span<uint8_t> command_data, span<uint8_t> receive_data);
    /**
     * @brief method that needs to be called periodically by the master code for proper communication. Can be called at any speed, but the faster it is called, the faster the Orders will resolve.
     *
     * this update has to be called by the code in order for master to check if the slave is ready to send the Order.
     * If it is not called periodically, the master_transmit_Order will not work. Not needed for dummy communication (not using Orders)
     */
    static void Order_update();



    /*=============================================
     * SPI Module Functions for HAL (cannot be private)
     ==============================================*/

    /**
     * @brief master Order that checks the state of the slave, used on the callback of the SPI module and handled automatically by Order_update method (as long as it is called).
     */
    static void master_check_available_end(SPI::Instance* spi);

    /**
     * @brief slave updates the information on its buffer to check what ID is master asking on next query
     */
    static void slave_check_packet_ID(SPI::Instance* spi);

    /**
     * @brief This method sets chip select to high level, and is used automatically by the SPI module
     *
     * The high level signal on SPI on chip select marks a slave to not process anything.
     * For more information read the chip_select_off method.
     *
     * @param spi Id of the SPI
     */
    static void chip_select_on(uint8_t id);

    /**
	 * @brief This method sets chip select to low level, and is used automatically by the SPI module.
	 *
	 * The low level signal on SPI on chip select marks the slave connected to it to start processing data, and is marked by the master
	 * The slave on the ST-LIB doesn t use this configuration yet and is instead active all the time, so for now it can only function as a single point to point configuration
	 * to have multiple ST-LIB SPI slaves on a single SPI interface you may need to implement an EXTI on the slave that deactivates it while its on.
	 * This method is still used to communicate to any slave that is not coded with the ST-LIB, as those follow the common rules of the SPI
	 *
	 * @param spi Id of the SPI
	 */
    static void chip_select_off(uint8_t id);

    static inline void spi_communicate_order_data(SPI::Instance* spi, uint8_t* value_to_send, uint8_t* value_to_receive, uint16_t size_to_send);


    static void turn_on_chip_select(SPI::Instance* spi);
    static void turn_off_chip_select(SPI::Instance* spi);

    /*
     * @brief returns true if its KNOWN that the slave has the packet ready, and false if it is NOT KNOWN
     *
     * This function uses the optional RS to know if the slave is ready, and always returns false if the RS is not used.
     */
    static bool known_slave_ready(SPI::Instance* spi);


    /**
     * @brief function used by slave to signal using RS that it has the message ready
     */
    static void mark_slave_ready(SPI::Instance* spi);


    /**
     * @brief function used by slave to signal using RS that it is waiting a new message
     */
    static void mark_slave_waiting(SPI::Instance* spi);

    /**
     * @brief Recovers SPI from any error so it starts working again, and counts the error
     */
    static void spi_recover(uint8_t id);
    static void spi_recover(SPI::Instance* spi);

    /**
     * @brief Check if an SPI bus collision occurred during recover
     */
    static void spi_check_bus_collision(SPI::Instance* spi);


private:
    /**
     * @brief This method initializes the SPI peripheral that is passed to it as a parameter.
     * 
     * @param spi Peripheral instance to be initialized.
     */
    static void init(SPI::Instance* spi);

    static unordered_map<uint8_t, std::pair<int, int>> spi_master_sockets; /** Map that associates registered SPI master with its server socket and its port */

    static void TxRxCpltCallback(uint8_t id);

};
