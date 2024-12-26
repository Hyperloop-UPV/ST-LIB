#include "ST-LIB.hpp"

//Data packets for LCU
class DataPacket{
    
enum class general_state{ 
 DEFINING,
CONNECTING,
OPERATIONAL,
FAULT 
};
enum class slave_state{ 
 DEFINING,
CONNECTING,
OPERATIONAL,
FAULT 
};

    private:
        constexpr static size_t size =2;
        uint32_t id{0};
    public:
        std::array<StackPacket*,size> packets; 
        StackPacket* levitation_control_data;
        StackPacket* lcu_data;

        
    DataPacket(uint16_t idpacket316,uint32_t current_control_frequency,uint32_t levitation_control_frequency,float32 lcu_coil_current_ref_1,float32 lcu_coil_current_ref_2,float32 lcu_coil_current_ref_3,float32 lcu_coil_current_ref_4,float32 lcu_coil_current_ref_5,float32 lcu_coil_current_ref_6,float32 lcu_coil_current_ref_7,float32 lcu_coil_current_ref_8,float32 lcu_coil_current_ref_9,float32 lcu_coil_current_ref_10,float32 pos_y,float32 pos_y_d,float32 pos_y_i,float32 pos_z,float32 pos_z_d,float32 pos_z_i,float32 pos_rot_x,float32 pos_rot_x_d,float32 pos_rot_x_i,float32 pos_rot_y,float32 pos_rot_y_d,float32 pos_rot_y_i,float32 pos_rot_z,float32 pos_rot_z_d,float32 pos_rot_z_i,uint16_t idpacket315,general_state general_state,slave_state slave_state,float32 lcu_coil_temp_1,float32 lcu_coil_temp_2,float32 lcu_coil_temp_3,float32 lcu_coil_temp_4,float32 lcu_coil_temp_5,float32 lcu_coil_temp_6,float32 lcu_coil_temp_7,float32 lcu_coil_temp_8,float32 lcu_coil_temp_9,float32 lcu_coil_temp_10,float32 lcu_driver_temp_1,float32 lcu_driver_temp_2,float32 lcu_driver_temp_3,float32 lcu_driver_temp_4,float32 lcu_driver_temp_5,float32 lcu_driver_temp_6,float32 lcu_driver_temp_7,float32 lcu_driver_temp_8,float32 lcu_driver_temp_9,float32 lcu_driver_temp_10,float32 lcu_coil_current_1,float32 lcu_coil_current_2,float32 lcu_coil_current_3,float32 lcu_coil_current_4,float32 lcu_coil_current_5,float32 lcu_coil_current_6,float32 lcu_coil_current_7,float32 lcu_coil_current_8,float32 lcu_coil_current_9,float32 lcu_coil_current_10,float32 lcu_vbat_1,float32 lcu_vbat_2,float32 lcu_vbat_3,float32 lcu_vbat_4,float32 lcu_vbat_5,float32 lcu_vbat_6,float32 lcu_vbat_7,float32 lcu_vbat_8,float32 lcu_vbat_9,float32 lcu_vbat_10,float32 lcu_airgap_1,float32 lcu_airgap_2,float32 lcu_airgap_3,float32 lcu_airgap_4,float32 lcu_airgap_5,float32 lcu_airgap_6,float32 lcu_airgap_7,float32 lcu_airgap_8)
{

   levitation_control_data = new StackPacket(&idpacket316,&current_control_frequency,&levitation_control_frequency,&lcu_coil_current_ref_1,&lcu_coil_current_ref_2,&lcu_coil_current_ref_3,&lcu_coil_current_ref_4,&lcu_coil_current_ref_5,&lcu_coil_current_ref_6,&lcu_coil_current_ref_7,&lcu_coil_current_ref_8,&lcu_coil_current_ref_9,&lcu_coil_current_ref_10,&pos_y,&pos_y_d,&pos_y_i,&pos_z,&pos_z_d,&pos_z_i,&pos_rot_x,&pos_rot_x_d,&pos_rot_x_i,&pos_rot_y,&pos_rot_y_d,&pos_rot_y_i,&pos_rot_z,&pos_rot_z_d,&pos_rot_z_i);
   packets[id] = levitation_control_data;
   id++;
   lcu_data = new StackPacket(&idpacket315,&general_state,&slave_state,&lcu_coil_temp_1,&lcu_coil_temp_2,&lcu_coil_temp_3,&lcu_coil_temp_4,&lcu_coil_temp_5,&lcu_coil_temp_6,&lcu_coil_temp_7,&lcu_coil_temp_8,&lcu_coil_temp_9,&lcu_coil_temp_10,&lcu_driver_temp_1,&lcu_driver_temp_2,&lcu_driver_temp_3,&lcu_driver_temp_4,&lcu_driver_temp_5,&lcu_driver_temp_6,&lcu_driver_temp_7,&lcu_driver_temp_8,&lcu_driver_temp_9,&lcu_driver_temp_10,&lcu_coil_current_1,&lcu_coil_current_2,&lcu_coil_current_3,&lcu_coil_current_4,&lcu_coil_current_5,&lcu_coil_current_6,&lcu_coil_current_7,&lcu_coil_current_8,&lcu_coil_current_9,&lcu_coil_current_10,&lcu_vbat_1,&lcu_vbat_2,&lcu_vbat_3,&lcu_vbat_4,&lcu_vbat_5,&lcu_vbat_6,&lcu_vbat_7,&lcu_vbat_8,&lcu_vbat_9,&lcu_vbat_10,&lcu_airgap_1,&lcu_airgap_2,&lcu_airgap_3,&lcu_airgap_4,&lcu_airgap_5,&lcu_airgap_6,&lcu_airgap_7,&lcu_airgap_8);
   packets[id] = lcu_data;
   id++;

}
};