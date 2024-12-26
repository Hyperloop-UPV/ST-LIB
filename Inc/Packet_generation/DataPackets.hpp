#include "ST-LIB.hpp"

//Data packets for OBCCU
class DataPacket{
    


    private:
        constexpr static size_t size =19;
        uint32_t id{0};
    public:
        std::array<StackPacket*,size> packets; 
        StackPacket* battery_1;
        StackPacket* battery_2;
        StackPacket* battery_3;
        StackPacket* battery_4;
        StackPacket* battery_5;
        StackPacket* battery_6;
        StackPacket* battery_7;
        StackPacket* battery_8;
        StackPacket* battery_9;
        StackPacket* battery_10;
        StackPacket* Fault;
        StackPacket* Total_Voltage;
        StackPacket* IMD;
        StackPacket* Measurements;
        StackPacket* OBCPU_Measurements;
        StackPacket* Contactors_State;
        StackPacket* Output_Voltage;

        
    DataPacket(uint16_t idpacket910,float32 battery1_SOC,float32 battery1_cell1,float32 battery1_cell2,float32 battery1_cell3,float32 battery1_cell4,float32 battery1_cell5,float32 battery1_cell6,float32 battery1_minimum_cell,float32 battery1_maximum_cell,float32 battery1_temperature1,float32 battery1_temperature2,bool battery1_is_balancing,float32 battery1_total_voltage,uint16_t idpacket911,float32 battery2_SOC,float32 battery2_cell1,float32 battery2_cell2,float32 battery2_cell3,float32 battery2_cell4,float32 battery2_cell5,float32 battery2_cell6,float32 battery2_minimum_cell,float32 battery2_maximum_cell,float32 battery2_temperature1,float32 battery2_temperature2,bool battery2_is_balancing,float32 battery2_total_voltage,uint16_t idpacket912,float32 battery3_SOC,float32 battery3_cell1,float32 battery3_cell2,float32 battery3_cell3,float32 battery3_cell4,float32 battery3_cell5,float32 battery3_cell6,float32 battery3_minimum_cell,float32 battery3_maximum_cell,float32 battery3_temperature1,float32 battery3_temperature2,bool battery3_is_balancing,float32 battery3_total_voltage,uint16_t idpacket913,float32 battery4_SOC,float32 battery4_cell1,float32 battery4_cell2,float32 battery4_cell3,float32 battery4_cell4,float32 battery4_cell5,float32 battery4_cell6,float32 battery4_minimum_cell,float32 battery4_maximum_cell,float32 battery4_temperature1,float32 battery4_temperature2,bool battery4_is_balancing,float32 battery4_total_voltage,uint16_t idpacket914,float32 battery5_SOC,float32 battery5_cell1,float32 battery5_cell2,float32 battery5_cell3,float32 battery5_cell4,float32 battery5_cell5,float32 battery5_cell6,float32 battery5_minimum_cell,float32 battery5_maximum_cell,float32 battery5_temperature1,float32 battery5_temperature2,bool battery5_is_balancing,float32 battery5_total_voltage,uint16_t idpacket915,float32 battery6_SOC,float32 battery6_cell1,float32 battery6_cell2,float32 battery6_cell3,float32 battery6_cell4,float32 battery6_cell5,float32 battery6_cell6,float32 battery6_minimum_cell,float32 battery6_maximum_cell,float32 battery6_temperature1,float32 battery6_temperature2,bool battery6_is_balancing,float32 battery6_total_voltage,uint16_t idpacket916,float32 battery7_SOC,float32 battery7_cell1,float32 battery7_cell2,float32 battery7_cell3,float32 battery7_cell4,float32 battery7_cell5,float32 battery7_cell6,float32 battery7_minimum_cell,float32 battery7_maximum_cell,float32 battery7_temperature1,float32 battery7_temperature2,bool battery7_is_balancing,float32 battery7_total_voltage,uint16_t idpacket917,float32 battery8_SOC,float32 battery8_cell1,float32 battery8_cell2,float32 battery8_cell3,float32 battery8_cell4,float32 battery8_cell5,float32 battery8_cell6,float32 battery8_minimum_cell,float32 battery8_maximum_cell,float32 battery8_temperature1,float32 battery8_temperature2,bool battery8_is_balancing,float32 battery8_total_voltage,uint16_t idpacket918,float32 battery9_SOC,float32 battery9_cell1,float32 battery9_cell2,float32 battery9_cell3,float32 battery9_cell4,float32 battery9_cell5,float32 battery9_cell6,float32 battery9_minimum_cell,float32 battery9_maximum_cell,float32 battery9_temperature1,float32 battery9_temperature2,bool battery9_is_balancing,float32 battery9_total_voltage,uint16_t idpacket919,float32 battery10_SOC,float32 battery10_cell1,float32 battery10_cell2,float32 battery10_cell3,float32 battery10_cell4,float32 battery10_cell5,float32 battery10_cell6,float32 battery10_minimum_cell,float32 battery10_maximum_cell,float32 battery10_temperature1,float32 battery10_temperature2,bool battery10_is_balancing,float32 battery10_total_voltage,uint16_t idpacket0,uint16_t idpacket921,float32 total_voltage_high,uint16_t idpacket922,uint8_t imd_status,float32 freq_imd,float32 duty_imd,uint16_t idpacket924,float32 battery_current,uint16_t idpacket925,float32 obcpu_input_current,float32 obcpu_input_voltage,float32 obcpu_output_current,float32 obcpu_output_voltage,uint16_t idpacket926,uint8_t contactors_state,uint8_t plug_state,uint8_t board_state,uint16_t idpacket981,float32 output_voltage_lpus,bool driving_mosfets)
{

   battery_1 = new StackPacket(&idpacket910,&battery1_SOC,&battery1_cell1,&battery1_cell2,&battery1_cell3,&battery1_cell4,&battery1_cell5,&battery1_cell6,&battery1_minimum_cell,&battery1_maximum_cell,&battery1_temperature1,&battery1_temperature2,&battery1_is_balancing,&battery1_total_voltage);
   packets[id] = battery_1;
   id++;
   battery_2 = new StackPacket(&idpacket911,&battery2_SOC,&battery2_cell1,&battery2_cell2,&battery2_cell3,&battery2_cell4,&battery2_cell5,&battery2_cell6,&battery2_minimum_cell,&battery2_maximum_cell,&battery2_temperature1,&battery2_temperature2,&battery2_is_balancing,&battery2_total_voltage);
   packets[id] = battery_2;
   id++;
   battery_3 = new StackPacket(&idpacket912,&battery3_SOC,&battery3_cell1,&battery3_cell2,&battery3_cell3,&battery3_cell4,&battery3_cell5,&battery3_cell6,&battery3_minimum_cell,&battery3_maximum_cell,&battery3_temperature1,&battery3_temperature2,&battery3_is_balancing,&battery3_total_voltage);
   packets[id] = battery_3;
   id++;
   battery_4 = new StackPacket(&idpacket913,&battery4_SOC,&battery4_cell1,&battery4_cell2,&battery4_cell3,&battery4_cell4,&battery4_cell5,&battery4_cell6,&battery4_minimum_cell,&battery4_maximum_cell,&battery4_temperature1,&battery4_temperature2,&battery4_is_balancing,&battery4_total_voltage);
   packets[id] = battery_4;
   id++;
   battery_5 = new StackPacket(&idpacket914,&battery5_SOC,&battery5_cell1,&battery5_cell2,&battery5_cell3,&battery5_cell4,&battery5_cell5,&battery5_cell6,&battery5_minimum_cell,&battery5_maximum_cell,&battery5_temperature1,&battery5_temperature2,&battery5_is_balancing,&battery5_total_voltage);
   packets[id] = battery_5;
   id++;
   battery_6 = new StackPacket(&idpacket915,&battery6_SOC,&battery6_cell1,&battery6_cell2,&battery6_cell3,&battery6_cell4,&battery6_cell5,&battery6_cell6,&battery6_minimum_cell,&battery6_maximum_cell,&battery6_temperature1,&battery6_temperature2,&battery6_is_balancing,&battery6_total_voltage);
   packets[id] = battery_6;
   id++;
   battery_7 = new StackPacket(&idpacket916,&battery7_SOC,&battery7_cell1,&battery7_cell2,&battery7_cell3,&battery7_cell4,&battery7_cell5,&battery7_cell6,&battery7_minimum_cell,&battery7_maximum_cell,&battery7_temperature1,&battery7_temperature2,&battery7_is_balancing,&battery7_total_voltage);
   packets[id] = battery_7;
   id++;
   battery_8 = new StackPacket(&idpacket917,&battery8_SOC,&battery8_cell1,&battery8_cell2,&battery8_cell3,&battery8_cell4,&battery8_cell5,&battery8_cell6,&battery8_minimum_cell,&battery8_maximum_cell,&battery8_temperature1,&battery8_temperature2,&battery8_is_balancing,&battery8_total_voltage);
   packets[id] = battery_8;
   id++;
   battery_9 = new StackPacket(&idpacket918,&battery9_SOC,&battery9_cell1,&battery9_cell2,&battery9_cell3,&battery9_cell4,&battery9_cell5,&battery9_cell6,&battery9_minimum_cell,&battery9_maximum_cell,&battery9_temperature1,&battery9_temperature2,&battery9_is_balancing,&battery9_total_voltage);
   packets[id] = battery_9;
   id++;
   battery_10 = new StackPacket(&idpacket919,&battery10_SOC,&battery10_cell1,&battery10_cell2,&battery10_cell3,&battery10_cell4,&battery10_cell5,&battery10_cell6,&battery10_minimum_cell,&battery10_maximum_cell,&battery10_temperature1,&battery10_temperature2,&battery10_is_balancing,&battery10_total_voltage);
   packets[id] = battery_10;
   id++;
   Fault = new StackPacket(&idpacket0);
   packets[id] = Fault;
   id++;
   Total_Voltage = new StackPacket(&idpacket921,&total_voltage_high);
   packets[id] = Total_Voltage;
   id++;
   IMD = new StackPacket(&idpacket922,&imd_status,&freq_imd,&duty_imd);
   packets[id] = IMD;
   id++;
   Measurements = new StackPacket(&idpacket924,&battery_current);
   packets[id] = Measurements;
   id++;
   OBCPU_Measurements = new StackPacket(&idpacket925,&obcpu_input_current,&obcpu_input_voltage,&obcpu_output_current,&obcpu_output_voltage);
   packets[id] = OBCPU_Measurements;
   id++;
   Contactors_State = new StackPacket(&idpacket926,&contactors_state,&plug_state,&board_state);
   packets[id] = Contactors_State;
   id++;
   Output_Voltage = new StackPacket(&idpacket981,&output_voltage_lpus,&driving_mosfets);
   packets[id] = Output_Voltage;
   id++;

}
};