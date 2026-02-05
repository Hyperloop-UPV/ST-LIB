/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : ethernetif.c
 * Description        : Ethernet interface glue code (PHY-agnostic)
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "ethernetif.h"
#include "HALAL/Services/Communication/Ethernet/PHY/phy_driver.h"
#include "lwip/ethip6.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/opt.h"
#include "lwip/timeouts.h"
#include "main.h"
#include "netif/etharp.h"
#include "netif/ethernet.h"
#include <string.h>

/* Network interface name */
#define IFNAME0 's'
#define IFNAME1 't'

#define ETH_DMA_TRANSMIT_TIMEOUT (20U)

/* RX zero-copy structures */
typedef enum { RX_ALLOC_OK = 0, RX_ALLOC_ERROR } RxAllocStatusTypeDef;

typedef struct {
  struct pbuf_custom pbuf_custom;
  uint8_t buff[(ETH_RX_BUFFER_SIZE + 31) & ~31] __ALIGNED(32);
} RxBuff_t;

#define ETH_RX_BUFFER_CNT 12U
LWIP_MEMPOOL_DECLARE(RX_POOL, ETH_RX_BUFFER_CNT, sizeof(RxBuff_t), "RX_POOL");

static uint8_t RxAllocStatus;

/* DMA descriptors (D1 / AXI SRAM) */
#if defined(__GNUC__)
ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT]
    __attribute__((section(".RxDecripSection")));
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT]
    __attribute__((section(".TxDecripSection")));
#endif

#if defined(__GNUC__)
__attribute__((
    section(".Rx_PoolSection"))) extern u8_t memp_memory_RX_POOL_base[];
#endif

ETH_HandleTypeDef heth;
ETH_TxPacketConfig TxConfig;

/* Private prototypes */
void pbuf_free_custom(struct pbuf *p);

/*******************************************************************************
 * LOW LEVEL INIT
 ******************************************************************************/
static void low_level_init(struct netif *netif) {
  heth.Instance = ETH;
  heth.Init.MACAddr = &netif->hwaddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;

  if (HAL_ETH_Init(&heth) != HAL_OK) {
    Error_Handler();
  }

  memset(&TxConfig, 0, sizeof(TxConfig));
  TxConfig.Attributes =
      ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

  LWIP_MEMPOOL_INIT(RX_POOL);

#if LWIP_ARP || LWIP_ETHERNET
  netif->hwaddr_len = ETH_HWADDR_LEN;
  memcpy(netif->hwaddr, heth.Init.MACAddr, ETH_HWADDR_LEN);
  netif->mtu = ETH_MAX_PAYLOAD;
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
#endif

  if (phy_driver && phy_driver->init) {
    phy_driver->init();
  }
}

/*******************************************************************************
 * TX
 ******************************************************************************/
static err_t low_level_output(struct netif *netif, struct pbuf *p) {
  ETH_BufferTypeDef Txbuffer[ETH_TX_DESC_CNT];
  struct pbuf *q;
  uint32_t i = 0;

  memset(Txbuffer, 0, sizeof(Txbuffer));

  for (q = p; q != NULL; q = q->next) {
    Txbuffer[i].buffer = q->payload;
    Txbuffer[i].len = q->len;
    if (i > 0)
      Txbuffer[i - 1].next = &Txbuffer[i];
    i++;
  }

  TxConfig.Length = p->tot_len;
  TxConfig.TxBuffer = Txbuffer;
  TxConfig.pData = p;

  HAL_ETH_Transmit(&heth, &TxConfig, ETH_DMA_TRANSMIT_TIMEOUT);
  return ERR_OK;
}

/*******************************************************************************
 * RX
 ******************************************************************************/
static struct pbuf *low_level_input(struct netif *netif) {
  struct pbuf *p = NULL;

  if (RxAllocStatus == RX_ALLOC_OK) {
    HAL_ETH_ReadData(&heth, (void **)&p);
  }

  return p;
}

void ethernetif_input(struct netif *netif) {
  struct pbuf *p = NULL;

  do {
    p = low_level_input(netif);
    if (p != NULL) {
      if (netif->input(p, netif) != ERR_OK) {
        pbuf_free_custom(p);
      }
    }
  } while (p != NULL);
}

/*******************************************************************************
 * NETIF INIT
 ******************************************************************************/
err_t ethernetif_init(struct netif *netif) {
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
#if LWIP_IPV4
  netif->output = etharp_output;
#endif
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif
  netif->linkoutput = low_level_output;
  low_level_init(netif);
  return ERR_OK;
}

/*******************************************************************************
 * PHY-AGNOSTIC LINK CHECK
 ******************************************************************************/
static uint8_t eth_link_up = 0;
void ethernet_link_check_state(struct netif *netif) {
  phy_link_state_t state = phy_driver->get_link_state();

  if (state == PHY_LINK_DOWN) {
    if (eth_link_up) {
      eth_link_up = 0;
      HAL_ETH_Stop(&heth);
      netif_set_link_down(netif);
      netif_set_down(netif);
    }
    return;
  }

  /* Link UP */
  if (!eth_link_up) {
    ETH_MACConfigTypeDef macconf;
    HAL_ETH_GetMACConfig(&heth, &macconf);

    switch (state) {
    case PHY_100_FULL:
      macconf.Speed = ETH_SPEED_100M;
      macconf.DuplexMode = ETH_FULLDUPLEX_MODE;
      break;
    case PHY_100_HALF:
      macconf.Speed = ETH_SPEED_100M;
      macconf.DuplexMode = ETH_HALFDUPLEX_MODE;
      break;
    case PHY_10_FULL:
      macconf.Speed = ETH_SPEED_10M;
      macconf.DuplexMode = ETH_FULLDUPLEX_MODE;
      break;
    case PHY_10_HALF:
      macconf.Speed = ETH_SPEED_10M;
      macconf.DuplexMode = ETH_HALFDUPLEX_MODE;
      break;
    default:
      return;
    }

    HAL_ETH_SetMACConfig(&heth, &macconf);
    HAL_ETH_Start(&heth);

    netif_set_link_up(netif);
    netif_set_up(netif);

    eth_link_up = 1;
  }
}

/*******************************************************************************
 * RX/TX CALLBACKS
 ******************************************************************************/
void HAL_ETH_RxAllocateCallback(uint8_t **buff) {
  struct pbuf_custom *p = LWIP_MEMPOOL_ALLOC(RX_POOL);
  if (p) {
    *buff = (uint8_t *)p + offsetof(RxBuff_t, buff);
    p->custom_free_function = pbuf_free_custom;
    pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, *buff, ETH_RX_BUFFER_SIZE);
  } else {
    RxAllocStatus = RX_ALLOC_ERROR;
    *buff = NULL;
  }
}

void HAL_ETH_RxLinkCallback(void **pStart, void **pEnd, uint8_t *buff,
                            uint16_t len) {
  struct pbuf *p = (struct pbuf *)(buff - offsetof(RxBuff_t, buff));
  p->next = NULL;
  p->len = len;
  p->tot_len = len;

  if (!*pStart)
    *pStart = p;
  else
    ((struct pbuf *)*pEnd)->next = p;

  *pEnd = p;
  uint32_t addr = (uint32_t)buff & ~0x1F;
  uint32_t size = len + ((uint32_t)buff - addr);
  size = (size + 31) & ~0x1F;

  SCB_InvalidateDCache_by_Addr((uint32_t *)addr, size);
}

void HAL_ETH_TxFreeCallback(uint32_t *buff) { pbuf_free((struct pbuf *)buff); }

void pbuf_free_custom(struct pbuf *p) {
  LWIP_MEMPOOL_FREE(RX_POOL, p);
  if (RxAllocStatus == RX_ALLOC_ERROR)
    RxAllocStatus = RX_ALLOC_OK;
}

u32_t sys_now(void) { return HAL_GetTick(); }

/*******************************************************************************
 * PHY IO (LAN8742)
 ******************************************************************************/
int32_t ETH_PHY_IO_Init(void) {
  HAL_ETH_SetMDIOClockRange(&heth);
  return 0;
}

int32_t ETH_PHY_IO_DeInit(void) { return 0; }

int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr,
                           uint32_t *pRegVal) {
  return (HAL_ETH_ReadPHYRegister(&heth, DevAddr, RegAddr, pRegVal) == HAL_OK)
             ? 0
             : -1;
}

int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr,
                            uint32_t RegVal) {
  return (HAL_ETH_WritePHYRegister(&heth, DevAddr, RegAddr, RegVal) == HAL_OK)
             ? 0
             : -1;
}

int32_t ETH_PHY_IO_GetTick(void) { return HAL_GetTick(); }
