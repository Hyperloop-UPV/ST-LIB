/*
 * Ethernet.cpp
 *
 *  Created on: Nov 23, 2022
 *      Author: stefa
 */

#include "HALAL/Services/Communication/Ethernet/Ethernet.hpp"
#include "ErrorHandler/ErrorHandler.hpp"
#include "HALAL/Models/MPUManager/MPUManager.hpp"

#ifdef HAL_ETH_MODULE_ENABLED
extern uint32_t EthernetLinkTimer;
extern struct netif gnetif;
extern ip4_addr_t ipaddr, netmask, gw;
extern uint8_t IP_ADDRESS[4], NETMASK_ADDRESS[4], GATEWAY_ADDRESS[4];

bool Ethernet::is_ready = false;
bool Ethernet::is_running = false;

void Ethernet::mpu_start() {
  MPU_Region_InitTypeDef MPU_InitStruct = {0};
  /* Disables the MPU */
  HAL_MPU_Disable();
  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.BaseAddress = 0x30000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512B;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /** Initializes and configures the Region and the memory to be protected
   */
  MPU_InitStruct.Number = MPU_REGION_NUMBER3;
  MPU_InitStruct.BaseAddress = 0x08000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_1MB;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

// void Ethernet::start(string local_mac, string local_ip, string subnet_mask,
//                      string gateway) {
//   start(MAC(local_mac), IPV4(local_ip), IPV4(subnet_mask), IPV4(gateway));
// }

void Ethernet::start(MAC local_mac, IPV4 local_ip, IPV4 subnet_mask,
                     IPV4 gateway) {
  if (!is_running && is_ready) {
    ipaddr = local_ip.address;
    netmask = subnet_mask.address;
    gw = gateway.address;
    IP_ADDRESS[0] = ipaddr.addr & 0xFF;
    IP_ADDRESS[1] = (ipaddr.addr >> 8) & 0xFF;
    IP_ADDRESS[2] = (ipaddr.addr >> 16) & 0xFF;
    IP_ADDRESS[3] = (ipaddr.addr >> 24) & 0xFF;
    NETMASK_ADDRESS[0] = netmask.addr & 0xFF;
    NETMASK_ADDRESS[1] = (netmask.addr >> 8) & 0xFF;
    NETMASK_ADDRESS[2] = (netmask.addr >> 16) & 0xFF;
    NETMASK_ADDRESS[3] = (netmask.addr >> 24) & 0xFF;
    GATEWAY_ADDRESS[0] = gw.addr & 0xFF;
    GATEWAY_ADDRESS[1] = (gw.addr >> 8) & 0xFF;
    GATEWAY_ADDRESS[2] = (gw.addr >> 16) & 0xFF;
    GATEWAY_ADDRESS[3] = (gw.addr >> 24) & 0xFF;
    gnetif.hwaddr[0] = local_mac.address[0];
    gnetif.hwaddr[1] = local_mac.address[1];
    gnetif.hwaddr[2] = local_mac.address[2];
    gnetif.hwaddr[3] = local_mac.address[3];
    gnetif.hwaddr[4] = local_mac.address[4];
    gnetif.hwaddr[5] = local_mac.address[5];
    MX_LWIP_Init();
    is_running = true;
  } else {
    ErrorHandler("Unable to start Ethernet!");
  }

  if (not is_ready) {
    ErrorHandler("Ethernet is not ready");
    return;
  }
}

void Ethernet::inscribe() {
  if (!is_ready) {
    Pin::inscribe(PA1, ALTERNATIVE);
    Pin::inscribe(PA2, ALTERNATIVE);
    Pin::inscribe(PA7, ALTERNATIVE);
    Pin::inscribe(PB13, ALTERNATIVE);
    Pin::inscribe(PC1, ALTERNATIVE);
    Pin::inscribe(PC4, ALTERNATIVE);
    Pin::inscribe(PC5, ALTERNATIVE);
    Pin::inscribe(PG11, ALTERNATIVE);
    Pin::inscribe(PG0, ALTERNATIVE);
    Pin::inscribe(PG13, ALTERNATIVE);
    is_ready = true;
  } else {
    ErrorHandler("Unable to inscribe Ethernet because is already ready!");
  }
}

void Ethernet::update() {
  if (not is_running) {
    ErrorHandler("Ethernet is not running, check if its been inscribed");
    return;
  }

  ethernetif_input(&gnetif);
  sys_check_timeouts();

  if (HAL_GetTick() - EthernetLinkTimer >= 100) {
    EthernetLinkTimer = HAL_GetTick();
    ethernet_link_check_state(&gnetif);

    if (gnetif.flags == 15) {
      netif_set_up(&gnetif);
    }
  }
}

#endif
