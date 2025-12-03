#pragma once
/******************************************************************************/
/*                                                                            */
/*                                    TIM                                     */
/*                                                                            */
/******************************************************************************/
#define TIM_BREAK_INPUT_SUPPORT             /*!<TIM Break input feature */

/*******************  Bit definition for TIM_CR1 register  ********************/
#define TIM_CR1_CEN_Pos           (0U)
#define TIM_CR1_CEN_Msk           (0x1UL << TIM_CR1_CEN_Pos)                   /*!< 0x00000001 */
#define TIM_CR1_CEN               TIM_CR1_CEN_Msk                              /*!<Counter enable */
#define TIM_CR1_UDIS_Pos          (1U)
#define TIM_CR1_UDIS_Msk          (0x1UL << TIM_CR1_UDIS_Pos)                  /*!< 0x00000002 */
#define TIM_CR1_UDIS              TIM_CR1_UDIS_Msk                             /*!<Update disable */
#define TIM_CR1_URS_Pos           (2U)
#define TIM_CR1_URS_Msk           (0x1UL << TIM_CR1_URS_Pos)                   /*!< 0x00000004 */
#define TIM_CR1_URS               TIM_CR1_URS_Msk                              /*!<Update request source */
#define TIM_CR1_OPM_Pos           (3U)
#define TIM_CR1_OPM_Msk           (0x1UL << TIM_CR1_OPM_Pos)                   /*!< 0x00000008 */
#define TIM_CR1_OPM               TIM_CR1_OPM_Msk                              /*!<One pulse mode */
#define TIM_CR1_DIR_Pos           (4U)
#define TIM_CR1_DIR_Msk           (0x1UL << TIM_CR1_DIR_Pos)                   /*!< 0x00000010 */
#define TIM_CR1_DIR               TIM_CR1_DIR_Msk                              /*!<Direction */

#define TIM_CR1_CMS_Pos           (5U)
#define TIM_CR1_CMS_Msk           (0x3UL << TIM_CR1_CMS_Pos)                   /*!< 0x00000060 */
#define TIM_CR1_CMS               TIM_CR1_CMS_Msk                              /*!<CMS[1:0] bits (Center-aligned mode selection) */
#define TIM_CR1_CMS_0             (0x1UL << TIM_CR1_CMS_Pos)                    /*!< 0x00000020 */
#define TIM_CR1_CMS_1             (0x2UL << TIM_CR1_CMS_Pos)                    /*!< 0x00000040 */

#define TIM_CR1_ARPE_Pos          (7U)
#define TIM_CR1_ARPE_Msk          (0x1UL << TIM_CR1_ARPE_Pos)                  /*!< 0x00000080 */
#define TIM_CR1_ARPE              TIM_CR1_ARPE_Msk                             /*!<Auto-reload preload enable */

#define TIM_CR1_CKD_Pos           (8U)
#define TIM_CR1_CKD_Msk           (0x3UL << TIM_CR1_CKD_Pos)                   /*!< 0x00000300 */
#define TIM_CR1_CKD               TIM_CR1_CKD_Msk                              /*!<CKD[1:0] bits (clock division) */
#define TIM_CR1_CKD_0             (0x1UL << TIM_CR1_CKD_Pos)                    /*!< 0x00000100 */
#define TIM_CR1_CKD_1             (0x2UL << TIM_CR1_CKD_Pos)                    /*!< 0x00000200 */

#define TIM_CR1_UIFREMAP_Pos      (11U)
#define TIM_CR1_UIFREMAP_Msk      (0x1UL << TIM_CR1_UIFREMAP_Pos)              /*!< 0x00000800 */
#define TIM_CR1_UIFREMAP          TIM_CR1_UIFREMAP_Msk                         /*!<Update interrupt flag remap */

/*******************  Bit definition for TIM_CR2 register  ********************/
#define TIM_CR2_CCPC_Pos          (0U)
#define TIM_CR2_CCPC_Msk          (0x1UL << TIM_CR2_CCPC_Pos)                  /*!< 0x00000001 */
#define TIM_CR2_CCPC              TIM_CR2_CCPC_Msk                             /*!<Capture/Compare Preloaded Control */
#define TIM_CR2_CCUS_Pos          (2U)
#define TIM_CR2_CCUS_Msk          (0x1UL << TIM_CR2_CCUS_Pos)                  /*!< 0x00000004 */
#define TIM_CR2_CCUS              TIM_CR2_CCUS_Msk                             /*!<Capture/Compare Control Update Selection */
#define TIM_CR2_CCDS_Pos          (3U)
#define TIM_CR2_CCDS_Msk          (0x1UL << TIM_CR2_CCDS_Pos)                  /*!< 0x00000008 */
#define TIM_CR2_CCDS              TIM_CR2_CCDS_Msk                             /*!<Capture/Compare DMA Selection */

#define TIM_CR2_MMS_Pos           (4U)
#define TIM_CR2_MMS_Msk           (0x7UL << TIM_CR2_MMS_Pos)                   /*!< 0x00000070 */
#define TIM_CR2_MMS               TIM_CR2_MMS_Msk                              /*!<MMS[2:0] bits (Master Mode Selection) */
#define TIM_CR2_MMS_0             (0x1UL << TIM_CR2_MMS_Pos)                    /*!< 0x00000010 */
#define TIM_CR2_MMS_1             (0x2UL << TIM_CR2_MMS_Pos)                    /*!< 0x00000020 */
#define TIM_CR2_MMS_2             (0x4UL << TIM_CR2_MMS_Pos)                    /*!< 0x00000040 */

#define TIM_CR2_TI1S_Pos          (7U)
#define TIM_CR2_TI1S_Msk          (0x1UL << TIM_CR2_TI1S_Pos)                  /*!< 0x00000080 */
#define TIM_CR2_TI1S              TIM_CR2_TI1S_Msk                             /*!<TI1 Selection */
#define TIM_CR2_OIS1_Pos          (8U)
#define TIM_CR2_OIS1_Msk          (0x1UL << TIM_CR2_OIS1_Pos)                  /*!< 0x00000100 */
#define TIM_CR2_OIS1              TIM_CR2_OIS1_Msk                             /*!<Output Idle state 1 (OC1 output) */
#define TIM_CR2_OIS1N_Pos         (9U)
#define TIM_CR2_OIS1N_Msk         (0x1UL << TIM_CR2_OIS1N_Pos)                 /*!< 0x00000200 */
#define TIM_CR2_OIS1N             TIM_CR2_OIS1N_Msk                            /*!<Output Idle state 1 (OC1N output) */
#define TIM_CR2_OIS2_Pos          (10U)
#define TIM_CR2_OIS2_Msk          (0x1UL << TIM_CR2_OIS2_Pos)                  /*!< 0x00000400 */
#define TIM_CR2_OIS2              TIM_CR2_OIS2_Msk                             /*!<Output Idle state 2 (OC2 output) */
#define TIM_CR2_OIS2N_Pos         (11U)
#define TIM_CR2_OIS2N_Msk         (0x1UL << TIM_CR2_OIS2N_Pos)                 /*!< 0x00000800 */
#define TIM_CR2_OIS2N             TIM_CR2_OIS2N_Msk                            /*!<Output Idle state 2 (OC2N output) */
#define TIM_CR2_OIS3_Pos          (12U)
#define TIM_CR2_OIS3_Msk          (0x1UL << TIM_CR2_OIS3_Pos)                  /*!< 0x00001000 */
#define TIM_CR2_OIS3              TIM_CR2_OIS3_Msk                             /*!<Output Idle state 3 (OC3 output) */
#define TIM_CR2_OIS3N_Pos         (13U)
#define TIM_CR2_OIS3N_Msk         (0x1UL << TIM_CR2_OIS3N_Pos)                 /*!< 0x00002000 */
#define TIM_CR2_OIS3N             TIM_CR2_OIS3N_Msk                            /*!<Output Idle state 3 (OC3N output) */
#define TIM_CR2_OIS4_Pos          (14U)
#define TIM_CR2_OIS4_Msk          (0x1UL << TIM_CR2_OIS4_Pos)                  /*!< 0x00004000 */
#define TIM_CR2_OIS4              TIM_CR2_OIS4_Msk                             /*!<Output Idle state 4 (OC4 output) */
#define TIM_CR2_OIS5_Pos          (16U)
#define TIM_CR2_OIS5_Msk          (0x1UL << TIM_CR2_OIS5_Pos)                  /*!< 0x00010000 */
#define TIM_CR2_OIS5              TIM_CR2_OIS5_Msk                             /*!<Output Idle state 4 (OC4 output) */
#define TIM_CR2_OIS6_Pos          (18U)
#define TIM_CR2_OIS6_Msk          (0x1UL << TIM_CR2_OIS6_Pos)                  /*!< 0x00040000 */
#define TIM_CR2_OIS6              TIM_CR2_OIS6_Msk                             /*!<Output Idle state 4 (OC4 output) */

#define TIM_CR2_MMS2_Pos          (20U)
#define TIM_CR2_MMS2_Msk          (0xFUL << TIM_CR2_MMS2_Pos)                  /*!< 0x00F00000 */
#define TIM_CR2_MMS2              TIM_CR2_MMS2_Msk                             /*!<MMS[2:0] bits (Master Mode Selection) */
#define TIM_CR2_MMS2_0            (0x1UL << TIM_CR2_MMS2_Pos)                   /*!< 0x00100000 */
#define TIM_CR2_MMS2_1            (0x2UL << TIM_CR2_MMS2_Pos)                   /*!< 0x00200000 */
#define TIM_CR2_MMS2_2            (0x4UL << TIM_CR2_MMS2_Pos)                   /*!< 0x00400000 */
#define TIM_CR2_MMS2_3            (0x8UL << TIM_CR2_MMS2_Pos)                   /*!< 0x00800000 */

/*******************  Bit definition for TIM_SMCR register  *******************/
#define TIM_SMCR_SMS_Pos          (0U)
#define TIM_SMCR_SMS_Msk          (0x10007UL << TIM_SMCR_SMS_Pos)              /*!< 0x00010007 */
#define TIM_SMCR_SMS              TIM_SMCR_SMS_Msk                             /*!<SMS[2:0] bits (Slave mode selection) */
#define TIM_SMCR_SMS_0            (0x00001UL << TIM_SMCR_SMS_Pos)               /*!< 0x00000001 */
#define TIM_SMCR_SMS_1            (0x00002UL << TIM_SMCR_SMS_Pos)               /*!< 0x00000002 */
#define TIM_SMCR_SMS_2            (0x00004UL << TIM_SMCR_SMS_Pos)               /*!< 0x00000004 */
#define TIM_SMCR_SMS_3            (0x10000UL << TIM_SMCR_SMS_Pos)               /*!< 0x00010000 */

#define TIM_SMCR_TS_Pos           (4U)
#define TIM_SMCR_TS_Msk           (0x30007UL << TIM_SMCR_TS_Pos)               /*!< 0x00300070 */
#define TIM_SMCR_TS               TIM_SMCR_TS_Msk                              /*!<TS[4:0] bits (Trigger selection) */
#define TIM_SMCR_TS_0             (0x00001UL << TIM_SMCR_TS_Pos)                /*!< 0x00000010 */
#define TIM_SMCR_TS_1             (0x00002UL << TIM_SMCR_TS_Pos)                /*!< 0x00000020 */
#define TIM_SMCR_TS_2             (0x00004UL << TIM_SMCR_TS_Pos)                /*!< 0x00000040 */
#define TIM_SMCR_TS_3             (0x10000UL << TIM_SMCR_TS_Pos)                /*!< 0x00100000 */
#define TIM_SMCR_TS_4             (0x20000UL << TIM_SMCR_TS_Pos)                /*!< 0x00200000 */

#define TIM_SMCR_MSM_Pos          (7U)
#define TIM_SMCR_MSM_Msk          (0x1UL << TIM_SMCR_MSM_Pos)                  /*!< 0x00000080 */
#define TIM_SMCR_MSM              TIM_SMCR_MSM_Msk                             /*!<Master/slave mode */

#define TIM_SMCR_ETF_Pos          (8U)
#define TIM_SMCR_ETF_Msk          (0xFUL << TIM_SMCR_ETF_Pos)                  /*!< 0x00000F00 */
#define TIM_SMCR_ETF              TIM_SMCR_ETF_Msk                             /*!<ETF[3:0] bits (External trigger filter) */
#define TIM_SMCR_ETF_0            (0x1UL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000100 */
#define TIM_SMCR_ETF_1            (0x2UL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000200 */
#define TIM_SMCR_ETF_2            (0x4UL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000400 */
#define TIM_SMCR_ETF_3            (0x8UL << TIM_SMCR_ETF_Pos)                   /*!< 0x00000800 */

#define TIM_SMCR_ETPS_Pos         (12U)
#define TIM_SMCR_ETPS_Msk         (0x3UL << TIM_SMCR_ETPS_Pos)                 /*!< 0x00003000 */
#define TIM_SMCR_ETPS             TIM_SMCR_ETPS_Msk                            /*!<ETPS[1:0] bits (External trigger prescaler) */
#define TIM_SMCR_ETPS_0           (0x1UL << TIM_SMCR_ETPS_Pos)                  /*!< 0x00001000 */
#define TIM_SMCR_ETPS_1           (0x2UL << TIM_SMCR_ETPS_Pos)                  /*!< 0x00002000 */

#define TIM_SMCR_ECE_Pos          (14U)
#define TIM_SMCR_ECE_Msk          (0x1UL << TIM_SMCR_ECE_Pos)                  /*!< 0x00004000 */
#define TIM_SMCR_ECE              TIM_SMCR_ECE_Msk                             /*!<External clock enable */
#define TIM_SMCR_ETP_Pos          (15U)
#define TIM_SMCR_ETP_Msk          (0x1UL << TIM_SMCR_ETP_Pos)                  /*!< 0x00008000 */
#define TIM_SMCR_ETP              TIM_SMCR_ETP_Msk                             /*!<External trigger polarity */

/*******************  Bit definition for TIM_DIER register  *******************/
#define TIM_DIER_UIE_Pos          (0U)
#define TIM_DIER_UIE_Msk          (0x1UL << TIM_DIER_UIE_Pos)                  /*!< 0x00000001 */
#define TIM_DIER_UIE              TIM_DIER_UIE_Msk                             /*!<Update interrupt enable */
#define TIM_DIER_CC1IE_Pos        (1U)
#define TIM_DIER_CC1IE_Msk        (0x1UL << TIM_DIER_CC1IE_Pos)                /*!< 0x00000002 */
#define TIM_DIER_CC1IE            TIM_DIER_CC1IE_Msk                           /*!<Capture/Compare 1 interrupt enable */
#define TIM_DIER_CC2IE_Pos        (2U)
#define TIM_DIER_CC2IE_Msk        (0x1UL << TIM_DIER_CC2IE_Pos)                /*!< 0x00000004 */
#define TIM_DIER_CC2IE            TIM_DIER_CC2IE_Msk                           /*!<Capture/Compare 2 interrupt enable */
#define TIM_DIER_CC3IE_Pos        (3U)
#define TIM_DIER_CC3IE_Msk        (0x1UL << TIM_DIER_CC3IE_Pos)                /*!< 0x00000008 */
#define TIM_DIER_CC3IE            TIM_DIER_CC3IE_Msk                           /*!<Capture/Compare 3 interrupt enable */
#define TIM_DIER_CC4IE_Pos        (4U)
#define TIM_DIER_CC4IE_Msk        (0x1UL << TIM_DIER_CC4IE_Pos)                /*!< 0x00000010 */
#define TIM_DIER_CC4IE            TIM_DIER_CC4IE_Msk                           /*!<Capture/Compare 4 interrupt enable */
#define TIM_DIER_COMIE_Pos        (5U)
#define TIM_DIER_COMIE_Msk        (0x1UL << TIM_DIER_COMIE_Pos)                /*!< 0x00000020 */
#define TIM_DIER_COMIE            TIM_DIER_COMIE_Msk                           /*!<COM interrupt enable */
#define TIM_DIER_TIE_Pos          (6U)
#define TIM_DIER_TIE_Msk          (0x1UL << TIM_DIER_TIE_Pos)                  /*!< 0x00000040 */
#define TIM_DIER_TIE              TIM_DIER_TIE_Msk                             /*!<Trigger interrupt enable */
#define TIM_DIER_BIE_Pos          (7U)
#define TIM_DIER_BIE_Msk          (0x1UL << TIM_DIER_BIE_Pos)                  /*!< 0x00000080 */
#define TIM_DIER_BIE              TIM_DIER_BIE_Msk                             /*!<Break interrupt enable */
#define TIM_DIER_UDE_Pos          (8U)
#define TIM_DIER_UDE_Msk          (0x1UL << TIM_DIER_UDE_Pos)                  /*!< 0x00000100 */
#define TIM_DIER_UDE              TIM_DIER_UDE_Msk                             /*!<Update DMA request enable */
#define TIM_DIER_CC1DE_Pos        (9U)
#define TIM_DIER_CC1DE_Msk        (0x1UL << TIM_DIER_CC1DE_Pos)                /*!< 0x00000200 */
#define TIM_DIER_CC1DE            TIM_DIER_CC1DE_Msk                           /*!<Capture/Compare 1 DMA request enable */
#define TIM_DIER_CC2DE_Pos        (10U)
#define TIM_DIER_CC2DE_Msk        (0x1UL << TIM_DIER_CC2DE_Pos)                /*!< 0x00000400 */
#define TIM_DIER_CC2DE            TIM_DIER_CC2DE_Msk                           /*!<Capture/Compare 2 DMA request enable */
#define TIM_DIER_CC3DE_Pos        (11U)
#define TIM_DIER_CC3DE_Msk        (0x1UL << TIM_DIER_CC3DE_Pos)                /*!< 0x00000800 */
#define TIM_DIER_CC3DE            TIM_DIER_CC3DE_Msk                           /*!<Capture/Compare 3 DMA request enable */
#define TIM_DIER_CC4DE_Pos        (12U)
#define TIM_DIER_CC4DE_Msk        (0x1UL << TIM_DIER_CC4DE_Pos)                /*!< 0x00001000 */
#define TIM_DIER_CC4DE            TIM_DIER_CC4DE_Msk                           /*!<Capture/Compare 4 DMA request enable */
#define TIM_DIER_COMDE_Pos        (13U)
#define TIM_DIER_COMDE_Msk        (0x1UL << TIM_DIER_COMDE_Pos)                /*!< 0x00002000 */
#define TIM_DIER_COMDE            TIM_DIER_COMDE_Msk                           /*!<COM DMA request enable */
#define TIM_DIER_TDE_Pos          (14U)
#define TIM_DIER_TDE_Msk          (0x1UL << TIM_DIER_TDE_Pos)                  /*!< 0x00004000 */
#define TIM_DIER_TDE              TIM_DIER_TDE_Msk                             /*!<Trigger DMA request enable */

/********************  Bit definition for TIM_SR register  ********************/
#define TIM_SR_UIF_Pos            (0U)
#define TIM_SR_UIF_Msk            (0x1UL << TIM_SR_UIF_Pos)                    /*!< 0x00000001 */
#define TIM_SR_UIF                TIM_SR_UIF_Msk                               /*!<Update interrupt Flag */
#define TIM_SR_CC1IF_Pos          (1U)
#define TIM_SR_CC1IF_Msk          (0x1UL << TIM_SR_CC1IF_Pos)                  /*!< 0x00000002 */
#define TIM_SR_CC1IF              TIM_SR_CC1IF_Msk                             /*!<Capture/Compare 1 interrupt Flag */
#define TIM_SR_CC2IF_Pos          (2U)
#define TIM_SR_CC2IF_Msk          (0x1UL << TIM_SR_CC2IF_Pos)                  /*!< 0x00000004 */
#define TIM_SR_CC2IF              TIM_SR_CC2IF_Msk                             /*!<Capture/Compare 2 interrupt Flag */
#define TIM_SR_CC3IF_Pos          (3U)
#define TIM_SR_CC3IF_Msk          (0x1UL << TIM_SR_CC3IF_Pos)                  /*!< 0x00000008 */
#define TIM_SR_CC3IF              TIM_SR_CC3IF_Msk                             /*!<Capture/Compare 3 interrupt Flag */
#define TIM_SR_CC4IF_Pos          (4U)
#define TIM_SR_CC4IF_Msk          (0x1UL << TIM_SR_CC4IF_Pos)                  /*!< 0x00000010 */
#define TIM_SR_CC4IF              TIM_SR_CC4IF_Msk                             /*!<Capture/Compare 4 interrupt Flag */
#define TIM_SR_COMIF_Pos          (5U)
#define TIM_SR_COMIF_Msk          (0x1UL << TIM_SR_COMIF_Pos)                  /*!< 0x00000020 */
#define TIM_SR_COMIF              TIM_SR_COMIF_Msk                             /*!<COM interrupt Flag */
#define TIM_SR_TIF_Pos            (6U)
#define TIM_SR_TIF_Msk            (0x1UL << TIM_SR_TIF_Pos)                    /*!< 0x00000040 */
#define TIM_SR_TIF                TIM_SR_TIF_Msk                               /*!<Trigger interrupt Flag */
#define TIM_SR_BIF_Pos            (7U)
#define TIM_SR_BIF_Msk            (0x1UL << TIM_SR_BIF_Pos)                    /*!< 0x00000080 */
#define TIM_SR_BIF                TIM_SR_BIF_Msk                               /*!<Break interrupt Flag */
#define TIM_SR_B2IF_Pos           (8U)
#define TIM_SR_B2IF_Msk           (0x1UL << TIM_SR_B2IF_Pos)                   /*!< 0x00000100 */
#define TIM_SR_B2IF               TIM_SR_B2IF_Msk                              /*!<Break2 interrupt Flag */
#define TIM_SR_CC1OF_Pos          (9U)
#define TIM_SR_CC1OF_Msk          (0x1UL << TIM_SR_CC1OF_Pos)                  /*!< 0x00000200 */
#define TIM_SR_CC1OF              TIM_SR_CC1OF_Msk                             /*!<Capture/Compare 1 Overcapture Flag */
#define TIM_SR_CC2OF_Pos          (10U)
#define TIM_SR_CC2OF_Msk          (0x1UL << TIM_SR_CC2OF_Pos)                  /*!< 0x00000400 */
#define TIM_SR_CC2OF              TIM_SR_CC2OF_Msk                             /*!<Capture/Compare 2 Overcapture Flag */
#define TIM_SR_CC3OF_Pos          (11U)
#define TIM_SR_CC3OF_Msk          (0x1UL << TIM_SR_CC3OF_Pos)                  /*!< 0x00000800 */
#define TIM_SR_CC3OF              TIM_SR_CC3OF_Msk                             /*!<Capture/Compare 3 Overcapture Flag */
#define TIM_SR_CC4OF_Pos          (12U)
#define TIM_SR_CC4OF_Msk          (0x1UL << TIM_SR_CC4OF_Pos)                  /*!< 0x00001000 */
#define TIM_SR_CC4OF              TIM_SR_CC4OF_Msk                             /*!<Capture/Compare 4 Overcapture Flag */
#define TIM_SR_CC5IF_Pos          (16U)
#define TIM_SR_CC5IF_Msk          (0x1UL << TIM_SR_CC5IF_Pos)                  /*!< 0x00010000 */
#define TIM_SR_CC5IF              TIM_SR_CC5IF_Msk                             /*!<Capture/Compare 5 interrupt Flag */
#define TIM_SR_CC6IF_Pos          (17U)
#define TIM_SR_CC6IF_Msk          (0x1UL << TIM_SR_CC6IF_Pos)                  /*!< 0x00020000 */
#define TIM_SR_CC6IF              TIM_SR_CC6IF_Msk                             /*!<Capture/Compare 6 interrupt Flag */
#define TIM_SR_SBIF_Pos           (13U)
#define TIM_SR_SBIF_Msk           (0x1UL << TIM_SR_SBIF_Pos)                   /*!< 0x00002000 */
#define TIM_SR_SBIF               TIM_SR_SBIF_Msk                              /*!< System Break Flag */

/*******************  Bit definition for TIM_EGR register  ********************/
#define TIM_EGR_UG_Pos            (0U)
#define TIM_EGR_UG_Msk            (0x1UL << TIM_EGR_UG_Pos)                    /*!< 0x00000001 */
#define TIM_EGR_UG                TIM_EGR_UG_Msk                               /*!<Update Generation */
#define TIM_EGR_CC1G_Pos          (1U)
#define TIM_EGR_CC1G_Msk          (0x1UL << TIM_EGR_CC1G_Pos)                  /*!< 0x00000002 */
#define TIM_EGR_CC1G              TIM_EGR_CC1G_Msk                             /*!<Capture/Compare 1 Generation */
#define TIM_EGR_CC2G_Pos          (2U)
#define TIM_EGR_CC2G_Msk          (0x1UL << TIM_EGR_CC2G_Pos)                  /*!< 0x00000004 */
#define TIM_EGR_CC2G              TIM_EGR_CC2G_Msk                             /*!<Capture/Compare 2 Generation */
#define TIM_EGR_CC3G_Pos          (3U)
#define TIM_EGR_CC3G_Msk          (0x1UL << TIM_EGR_CC3G_Pos)                  /*!< 0x00000008 */
#define TIM_EGR_CC3G              TIM_EGR_CC3G_Msk                             /*!<Capture/Compare 3 Generation */
#define TIM_EGR_CC4G_Pos          (4U)
#define TIM_EGR_CC4G_Msk          (0x1UL << TIM_EGR_CC4G_Pos)                  /*!< 0x00000010 */
#define TIM_EGR_CC4G              TIM_EGR_CC4G_Msk                             /*!<Capture/Compare 4 Generation */
#define TIM_EGR_COMG_Pos          (5U)
#define TIM_EGR_COMG_Msk          (0x1UL << TIM_EGR_COMG_Pos)                  /*!< 0x00000020 */
#define TIM_EGR_COMG              TIM_EGR_COMG_Msk                             /*!<Capture/Compare Control Update Generation */
#define TIM_EGR_TG_Pos            (6U)
#define TIM_EGR_TG_Msk            (0x1UL << TIM_EGR_TG_Pos)                    /*!< 0x00000040 */
#define TIM_EGR_TG                TIM_EGR_TG_Msk                               /*!<Trigger Generation */
#define TIM_EGR_BG_Pos            (7U)
#define TIM_EGR_BG_Msk            (0x1UL << TIM_EGR_BG_Pos)                    /*!< 0x00000080 */
#define TIM_EGR_BG                TIM_EGR_BG_Msk                               /*!<Break Generation */
#define TIM_EGR_B2G_Pos           (8U)
#define TIM_EGR_B2G_Msk           (0x1UL << TIM_EGR_B2G_Pos)                   /*!< 0x00000100 */
#define TIM_EGR_B2G               TIM_EGR_B2G_Msk                              /*!<Break Generation */


/******************  Bit definition for TIM_CCMR1 register  *******************/
#define TIM_CCMR1_CC1S_Pos        (0U)
#define TIM_CCMR1_CC1S_Msk        (0x3UL << TIM_CCMR1_CC1S_Pos)                /*!< 0x00000003 */
#define TIM_CCMR1_CC1S            TIM_CCMR1_CC1S_Msk                           /*!<CC1S[1:0] bits (Capture/Compare 1 Selection) */
#define TIM_CCMR1_CC1S_0          (0x1UL << TIM_CCMR1_CC1S_Pos)                 /*!< 0x00000001 */
#define TIM_CCMR1_CC1S_1          (0x2UL << TIM_CCMR1_CC1S_Pos)                 /*!< 0x00000002 */

#define TIM_CCMR1_OC1FE_Pos       (2U)
#define TIM_CCMR1_OC1FE_Msk       (0x1UL << TIM_CCMR1_OC1FE_Pos)               /*!< 0x00000004 */
#define TIM_CCMR1_OC1FE           TIM_CCMR1_OC1FE_Msk                          /*!<Output Compare 1 Fast enable */
#define TIM_CCMR1_OC1PE_Pos       (3U)
#define TIM_CCMR1_OC1PE_Msk       (0x1UL << TIM_CCMR1_OC1PE_Pos)               /*!< 0x00000008 */
#define TIM_CCMR1_OC1PE           TIM_CCMR1_OC1PE_Msk                          /*!<Output Compare 1 Preload enable */

#define TIM_CCMR1_OC1M_Pos        (4U)
#define TIM_CCMR1_OC1M_Msk        (0x1007UL << TIM_CCMR1_OC1M_Pos)             /*!< 0x00010070 */
#define TIM_CCMR1_OC1M            TIM_CCMR1_OC1M_Msk                           /*!<OC1M[2:0] bits (Output Compare 1 Mode) */
#define TIM_CCMR1_OC1M_0          (0x0001UL << TIM_CCMR1_OC1M_Pos)              /*!< 0x00000010 */
#define TIM_CCMR1_OC1M_1          (0x0002UL << TIM_CCMR1_OC1M_Pos)              /*!< 0x00000020 */
#define TIM_CCMR1_OC1M_2          (0x0004UL << TIM_CCMR1_OC1M_Pos)              /*!< 0x00000040 */
#define TIM_CCMR1_OC1M_3          (0x1000UL << TIM_CCMR1_OC1M_Pos)              /*!< 0x00010000 */

#define TIM_CCMR1_OC1CE_Pos       (7U)
#define TIM_CCMR1_OC1CE_Msk       (0x1UL << TIM_CCMR1_OC1CE_Pos)               /*!< 0x00000080 */
#define TIM_CCMR1_OC1CE           TIM_CCMR1_OC1CE_Msk                          /*!<Output Compare 1Clear Enable */

#define TIM_CCMR1_CC2S_Pos        (8U)
#define TIM_CCMR1_CC2S_Msk        (0x3UL << TIM_CCMR1_CC2S_Pos)                /*!< 0x00000300 */
#define TIM_CCMR1_CC2S            TIM_CCMR1_CC2S_Msk                           /*!<CC2S[1:0] bits (Capture/Compare 2 Selection) */
#define TIM_CCMR1_CC2S_0          (0x1UL << TIM_CCMR1_CC2S_Pos)                 /*!< 0x00000100 */
#define TIM_CCMR1_CC2S_1          (0x2UL << TIM_CCMR1_CC2S_Pos)                 /*!< 0x00000200 */

#define TIM_CCMR1_OC2FE_Pos       (10U)
#define TIM_CCMR1_OC2FE_Msk       (0x1UL << TIM_CCMR1_OC2FE_Pos)               /*!< 0x00000400 */
#define TIM_CCMR1_OC2FE           TIM_CCMR1_OC2FE_Msk                          /*!<Output Compare 2 Fast enable */
#define TIM_CCMR1_OC2PE_Pos       (11U)
#define TIM_CCMR1_OC2PE_Msk       (0x1UL << TIM_CCMR1_OC2PE_Pos)               /*!< 0x00000800 */
#define TIM_CCMR1_OC2PE           TIM_CCMR1_OC2PE_Msk                          /*!<Output Compare 2 Preload enable */

#define TIM_CCMR1_OC2M_Pos        (12U)
#define TIM_CCMR1_OC2M_Msk        (0x1007UL << TIM_CCMR1_OC2M_Pos)             /*!< 0x01007000 */
#define TIM_CCMR1_OC2M            TIM_CCMR1_OC2M_Msk                           /*!<OC2M[2:0] bits (Output Compare 2 Mode) */
#define TIM_CCMR1_OC2M_0          (0x0001UL << TIM_CCMR1_OC2M_Pos)              /*!< 0x00001000 */
#define TIM_CCMR1_OC2M_1          (0x0002UL << TIM_CCMR1_OC2M_Pos)              /*!< 0x00002000 */
#define TIM_CCMR1_OC2M_2          (0x0004UL << TIM_CCMR1_OC2M_Pos)              /*!< 0x00004000 */
#define TIM_CCMR1_OC2M_3          (0x1000UL << TIM_CCMR1_OC2M_Pos)              /*!< 0x01000000 */

#define TIM_CCMR1_OC2CE_Pos       (15U)
#define TIM_CCMR1_OC2CE_Msk       (0x1UL << TIM_CCMR1_OC2CE_Pos)               /*!< 0x00008000 */
#define TIM_CCMR1_OC2CE           TIM_CCMR1_OC2CE_Msk                          /*!<Output Compare 2 Clear Enable */

/*----------------------------------------------------------------------------*/

#define TIM_CCMR1_IC1PSC_Pos      (2U)
#define TIM_CCMR1_IC1PSC_Msk      (0x3UL << TIM_CCMR1_IC1PSC_Pos)              /*!< 0x0000000C */
#define TIM_CCMR1_IC1PSC          TIM_CCMR1_IC1PSC_Msk                         /*!<IC1PSC[1:0] bits (Input Capture 1 Prescaler) */
#define TIM_CCMR1_IC1PSC_0        (0x1UL << TIM_CCMR1_IC1PSC_Pos)               /*!< 0x00000004 */
#define TIM_CCMR1_IC1PSC_1        (0x2UL << TIM_CCMR1_IC1PSC_Pos)               /*!< 0x00000008 */

#define TIM_CCMR1_IC1F_Pos        (4U)
#define TIM_CCMR1_IC1F_Msk        (0xFUL << TIM_CCMR1_IC1F_Pos)                /*!< 0x000000F0 */
#define TIM_CCMR1_IC1F            TIM_CCMR1_IC1F_Msk                           /*!<IC1F[3:0] bits (Input Capture 1 Filter) */
#define TIM_CCMR1_IC1F_0          (0x1UL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x00000010 */
#define TIM_CCMR1_IC1F_1          (0x2UL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x00000020 */
#define TIM_CCMR1_IC1F_2          (0x4UL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x00000040 */
#define TIM_CCMR1_IC1F_3          (0x8UL << TIM_CCMR1_IC1F_Pos)                 /*!< 0x00000080 */

#define TIM_CCMR1_IC2PSC_Pos      (10U)
#define TIM_CCMR1_IC2PSC_Msk      (0x3UL << TIM_CCMR1_IC2PSC_Pos)              /*!< 0x00000C00 */
#define TIM_CCMR1_IC2PSC          TIM_CCMR1_IC2PSC_Msk                         /*!<IC2PSC[1:0] bits (Input Capture 2 Prescaler) */
#define TIM_CCMR1_IC2PSC_0        (0x1UL << TIM_CCMR1_IC2PSC_Pos)               /*!< 0x00000400 */
#define TIM_CCMR1_IC2PSC_1        (0x2UL << TIM_CCMR1_IC2PSC_Pos)               /*!< 0x00000800 */

#define TIM_CCMR1_IC2F_Pos        (12U)
#define TIM_CCMR1_IC2F_Msk        (0xFUL << TIM_CCMR1_IC2F_Pos)                /*!< 0x0000F000 */
#define TIM_CCMR1_IC2F            TIM_CCMR1_IC2F_Msk                           /*!<IC2F[3:0] bits (Input Capture 2 Filter) */
#define TIM_CCMR1_IC2F_0          (0x1UL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x00001000 */
#define TIM_CCMR1_IC2F_1          (0x2UL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x00002000 */
#define TIM_CCMR1_IC2F_2          (0x4UL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x00004000 */
#define TIM_CCMR1_IC2F_3          (0x8UL << TIM_CCMR1_IC2F_Pos)                 /*!< 0x00008000 */

/******************  Bit definition for TIM_CCMR2 register  *******************/
#define TIM_CCMR2_CC3S_Pos        (0U)
#define TIM_CCMR2_CC3S_Msk        (0x3UL << TIM_CCMR2_CC3S_Pos)                /*!< 0x00000003 */
#define TIM_CCMR2_CC3S            TIM_CCMR2_CC3S_Msk                           /*!<CC3S[1:0] bits (Capture/Compare 3 Selection) */
#define TIM_CCMR2_CC3S_0          (0x1UL << TIM_CCMR2_CC3S_Pos)                 /*!< 0x00000001 */
#define TIM_CCMR2_CC3S_1          (0x2UL << TIM_CCMR2_CC3S_Pos)                 /*!< 0x00000002 */

#define TIM_CCMR2_OC3FE_Pos       (2U)
#define TIM_CCMR2_OC3FE_Msk       (0x1UL << TIM_CCMR2_OC3FE_Pos)               /*!< 0x00000004 */
#define TIM_CCMR2_OC3FE           TIM_CCMR2_OC3FE_Msk                          /*!<Output Compare 3 Fast enable */
#define TIM_CCMR2_OC3PE_Pos       (3U)
#define TIM_CCMR2_OC3PE_Msk       (0x1UL << TIM_CCMR2_OC3PE_Pos)               /*!< 0x00000008 */
#define TIM_CCMR2_OC3PE           TIM_CCMR2_OC3PE_Msk                          /*!<Output Compare 3 Preload enable */

#define TIM_CCMR2_OC3M_Pos        (4U)
#define TIM_CCMR2_OC3M_Msk        (0x1007UL << TIM_CCMR2_OC3M_Pos)                /*!< 0x00010070 */
#define TIM_CCMR2_OC3M            TIM_CCMR2_OC3M_Msk                           /*!<OC3M[3:0] bits (Output Compare 3 Mode) */
#define TIM_CCMR2_OC3M_0          (0x1UL << TIM_CCMR2_OC3M_Pos)                 /*!< 0x00000010 */
#define TIM_CCMR2_OC3M_1          (0x2UL << TIM_CCMR2_OC3M_Pos)                 /*!< 0x00000020 */
#define TIM_CCMR2_OC3M_2          (0x4UL << TIM_CCMR2_OC3M_Pos)                 /*!< 0x00000040 */
#define TIM_CCMR2_OC3M_3          (0x1000UL << TIM_CCMR2_OC3M_Pos)              /*!< 0x00010000 */

#define TIM_CCMR2_OC3CE_Pos       (7U)
#define TIM_CCMR2_OC3CE_Msk       (0x1UL << TIM_CCMR2_OC3CE_Pos)               /*!< 0x00000080 */
#define TIM_CCMR2_OC3CE           TIM_CCMR2_OC3CE_Msk                          /*!<Output Compare 3 Clear Enable */

#define TIM_CCMR2_CC4S_Pos        (8U)
#define TIM_CCMR2_CC4S_Msk        (0x3UL << TIM_CCMR2_CC4S_Pos)                /*!< 0x00000300 */
#define TIM_CCMR2_CC4S            TIM_CCMR2_CC4S_Msk                           /*!<CC4S[1:0] bits (Capture/Compare 4 Selection) */
#define TIM_CCMR2_CC4S_0          (0x1UL << TIM_CCMR2_CC4S_Pos)                 /*!< 0x00000100 */
#define TIM_CCMR2_CC4S_1          (0x2UL << TIM_CCMR2_CC4S_Pos)                 /*!< 0x00000200 */

#define TIM_CCMR2_OC4FE_Pos       (10U)
#define TIM_CCMR2_OC4FE_Msk       (0x1UL << TIM_CCMR2_OC4FE_Pos)               /*!< 0x00000400 */
#define TIM_CCMR2_OC4FE           TIM_CCMR2_OC4FE_Msk                          /*!<Output Compare 4 Fast enable */
#define TIM_CCMR2_OC4PE_Pos       (11U)
#define TIM_CCMR2_OC4PE_Msk       (0x1UL << TIM_CCMR2_OC4PE_Pos)               /*!< 0x00000800 */
#define TIM_CCMR2_OC4PE           TIM_CCMR2_OC4PE_Msk                          /*!<Output Compare 4 Preload enable */

#define TIM_CCMR2_OC4M_Pos        (12U)
#define TIM_CCMR2_OC4M_Msk        (0x1007UL << TIM_CCMR2_OC4M_Pos)                /*!< 0x01007000 */
#define TIM_CCMR2_OC4M            TIM_CCMR2_OC4M_Msk                           /*!<OC4M[3:0] bits (Output Compare 4 Mode) */
#define TIM_CCMR2_OC4M_0          (0x1UL << TIM_CCMR2_OC4M_Pos)                 /*!< 0x00001000 */
#define TIM_CCMR2_OC4M_1          (0x2UL << TIM_CCMR2_OC4M_Pos)                 /*!< 0x00002000 */
#define TIM_CCMR2_OC4M_2          (0x4UL << TIM_CCMR2_OC4M_Pos)                 /*!< 0x00004000 */
#define TIM_CCMR2_OC4M_3          (0x1000UL << TIM_CCMR2_OC4M_Pos)              /*!< 0x01000000 */

#define TIM_CCMR2_OC4CE_Pos       (15U)
#define TIM_CCMR2_OC4CE_Msk       (0x1UL << TIM_CCMR2_OC4CE_Pos)               /*!< 0x00008000 */
#define TIM_CCMR2_OC4CE           TIM_CCMR2_OC4CE_Msk                          /*!<Output Compare 4 Clear Enable */

/*----------------------------------------------------------------------------*/

#define TIM_CCMR2_IC3PSC_Pos      (2U)
#define TIM_CCMR2_IC3PSC_Msk      (0x3UL << TIM_CCMR2_IC3PSC_Pos)              /*!< 0x0000000C */
#define TIM_CCMR2_IC3PSC          TIM_CCMR2_IC3PSC_Msk                         /*!<IC3PSC[1:0] bits (Input Capture 3 Prescaler) */
#define TIM_CCMR2_IC3PSC_0        (0x1UL << TIM_CCMR2_IC3PSC_Pos)               /*!< 0x00000004 */
#define TIM_CCMR2_IC3PSC_1        (0x2UL << TIM_CCMR2_IC3PSC_Pos)               /*!< 0x00000008 */

#define TIM_CCMR2_IC3F_Pos        (4U)
#define TIM_CCMR2_IC3F_Msk        (0xFUL << TIM_CCMR2_IC3F_Pos)                /*!< 0x000000F0 */
#define TIM_CCMR2_IC3F            TIM_CCMR2_IC3F_Msk                           /*!<IC3F[3:0] bits (Input Capture 3 Filter) */
#define TIM_CCMR2_IC3F_0          (0x1UL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x00000010 */
#define TIM_CCMR2_IC3F_1          (0x2UL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x00000020 */
#define TIM_CCMR2_IC3F_2          (0x4UL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x00000040 */
#define TIM_CCMR2_IC3F_3          (0x8UL << TIM_CCMR2_IC3F_Pos)                 /*!< 0x00000080 */

#define TIM_CCMR2_IC4PSC_Pos      (10U)
#define TIM_CCMR2_IC4PSC_Msk      (0x3UL << TIM_CCMR2_IC4PSC_Pos)              /*!< 0x00000C00 */
#define TIM_CCMR2_IC4PSC          TIM_CCMR2_IC4PSC_Msk                         /*!<IC4PSC[1:0] bits (Input Capture 4 Prescaler) */
#define TIM_CCMR2_IC4PSC_0        (0x1UL << TIM_CCMR2_IC4PSC_Pos)               /*!< 0x00000400 */
#define TIM_CCMR2_IC4PSC_1        (0x2UL << TIM_CCMR2_IC4PSC_Pos)               /*!< 0x00000800 */

#define TIM_CCMR2_IC4F_Pos        (12U)
#define TIM_CCMR2_IC4F_Msk        (0xFUL << TIM_CCMR2_IC4F_Pos)                /*!< 0x0000F000 */
#define TIM_CCMR2_IC4F            TIM_CCMR2_IC4F_Msk                           /*!<IC4F[3:0] bits (Input Capture 4 Filter) */
#define TIM_CCMR2_IC4F_0          (0x1UL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x00001000 */
#define TIM_CCMR2_IC4F_1          (0x2UL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x00002000 */
#define TIM_CCMR2_IC4F_2          (0x4UL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x00004000 */
#define TIM_CCMR2_IC4F_3          (0x8UL << TIM_CCMR2_IC4F_Pos)                 /*!< 0x00008000 */

/*******************  Bit definition for TIM_CCER register  *******************/
#define TIM_CCER_CC1E_Pos         (0U)
#define TIM_CCER_CC1E_Msk         (0x1UL << TIM_CCER_CC1E_Pos)                 /*!< 0x00000001 */
#define TIM_CCER_CC1E             TIM_CCER_CC1E_Msk                            /*!<Capture/Compare 1 output enable */
#define TIM_CCER_CC1P_Pos         (1U)
#define TIM_CCER_CC1P_Msk         (0x1UL << TIM_CCER_CC1P_Pos)                 /*!< 0x00000002 */
#define TIM_CCER_CC1P             TIM_CCER_CC1P_Msk                            /*!<Capture/Compare 1 output Polarity */
#define TIM_CCER_CC1NE_Pos        (2U)
#define TIM_CCER_CC1NE_Msk        (0x1UL << TIM_CCER_CC1NE_Pos)                /*!< 0x00000004 */
#define TIM_CCER_CC1NE            TIM_CCER_CC1NE_Msk                           /*!<Capture/Compare 1 Complementary output enable */
#define TIM_CCER_CC1NP_Pos        (3U)
#define TIM_CCER_CC1NP_Msk        (0x1UL << TIM_CCER_CC1NP_Pos)                /*!< 0x00000008 */
#define TIM_CCER_CC1NP            TIM_CCER_CC1NP_Msk                           /*!<Capture/Compare 1 Complementary output Polarity */
#define TIM_CCER_CC2E_Pos         (4U)
#define TIM_CCER_CC2E_Msk         (0x1UL << TIM_CCER_CC2E_Pos)                 /*!< 0x00000010 */
#define TIM_CCER_CC2E             TIM_CCER_CC2E_Msk                            /*!<Capture/Compare 2 output enable */
#define TIM_CCER_CC2P_Pos         (5U)
#define TIM_CCER_CC2P_Msk         (0x1UL << TIM_CCER_CC2P_Pos)                 /*!< 0x00000020 */
#define TIM_CCER_CC2P             TIM_CCER_CC2P_Msk                            /*!<Capture/Compare 2 output Polarity */
#define TIM_CCER_CC2NE_Pos        (6U)
#define TIM_CCER_CC2NE_Msk        (0x1UL << TIM_CCER_CC2NE_Pos)                /*!< 0x00000040 */
#define TIM_CCER_CC2NE            TIM_CCER_CC2NE_Msk                           /*!<Capture/Compare 2 Complementary output enable */
#define TIM_CCER_CC2NP_Pos        (7U)
#define TIM_CCER_CC2NP_Msk        (0x1UL << TIM_CCER_CC2NP_Pos)                /*!< 0x00000080 */
#define TIM_CCER_CC2NP            TIM_CCER_CC2NP_Msk                           /*!<Capture/Compare 2 Complementary output Polarity */
#define TIM_CCER_CC3E_Pos         (8U)
#define TIM_CCER_CC3E_Msk         (0x1UL << TIM_CCER_CC3E_Pos)                 /*!< 0x00000100 */
#define TIM_CCER_CC3E             TIM_CCER_CC3E_Msk                            /*!<Capture/Compare 3 output enable */
#define TIM_CCER_CC3P_Pos         (9U)
#define TIM_CCER_CC3P_Msk         (0x1UL << TIM_CCER_CC3P_Pos)                 /*!< 0x00000200 */
#define TIM_CCER_CC3P             TIM_CCER_CC3P_Msk                            /*!<Capture/Compare 3 output Polarity */
#define TIM_CCER_CC3NE_Pos        (10U)
#define TIM_CCER_CC3NE_Msk        (0x1UL << TIM_CCER_CC3NE_Pos)                /*!< 0x00000400 */
#define TIM_CCER_CC3NE            TIM_CCER_CC3NE_Msk                           /*!<Capture/Compare 3 Complementary output enable */
#define TIM_CCER_CC3NP_Pos        (11U)
#define TIM_CCER_CC3NP_Msk        (0x1UL << TIM_CCER_CC3NP_Pos)                /*!< 0x00000800 */
#define TIM_CCER_CC3NP            TIM_CCER_CC3NP_Msk                           /*!<Capture/Compare 3 Complementary output Polarity */
#define TIM_CCER_CC4E_Pos         (12U)
#define TIM_CCER_CC4E_Msk         (0x1UL << TIM_CCER_CC4E_Pos)                 /*!< 0x00001000 */
#define TIM_CCER_CC4E             TIM_CCER_CC4E_Msk                            /*!<Capture/Compare 4 output enable */
#define TIM_CCER_CC4P_Pos         (13U)
#define TIM_CCER_CC4P_Msk         (0x1UL << TIM_CCER_CC4P_Pos)                 /*!< 0x00002000 */
#define TIM_CCER_CC4P             TIM_CCER_CC4P_Msk                            /*!<Capture/Compare 4 output Polarity */
#define TIM_CCER_CC4NP_Pos        (15U)
#define TIM_CCER_CC4NP_Msk        (0x1UL << TIM_CCER_CC4NP_Pos)                /*!< 0x00008000 */
#define TIM_CCER_CC4NP            TIM_CCER_CC4NP_Msk                           /*!<Capture/Compare 4 Complementary output Polarity */
#define TIM_CCER_CC5E_Pos         (16U)
#define TIM_CCER_CC5E_Msk         (0x1UL << TIM_CCER_CC5E_Pos)                 /*!< 0x00010000 */
#define TIM_CCER_CC5E             TIM_CCER_CC5E_Msk                            /*!<Capture/Compare 5 output enable */
#define TIM_CCER_CC5P_Pos         (17U)
#define TIM_CCER_CC5P_Msk         (0x1UL << TIM_CCER_CC5P_Pos)                 /*!< 0x00020000 */
#define TIM_CCER_CC5P             TIM_CCER_CC5P_Msk                            /*!<Capture/Compare 5 output Polarity */
#define TIM_CCER_CC6E_Pos         (20U)
#define TIM_CCER_CC6E_Msk         (0x1UL << TIM_CCER_CC6E_Pos)                 /*!< 0x00100000 */
#define TIM_CCER_CC6E             TIM_CCER_CC6E_Msk                            /*!<Capture/Compare 6 output enable */
#define TIM_CCER_CC6P_Pos         (21U)
#define TIM_CCER_CC6P_Msk         (0x1UL << TIM_CCER_CC6P_Pos)                 /*!< 0x00200000 */
#define TIM_CCER_CC6P             TIM_CCER_CC6P_Msk                            /*!<Capture/Compare 6 output Polarity */
/*******************  Bit definition for TIM_CNT register  ********************/
#define TIM_CNT_CNT_Pos           (0U)
#define TIM_CNT_CNT_Msk           (0xFFFFFFFFUL << TIM_CNT_CNT_Pos)            /*!< 0xFFFFFFFF */
#define TIM_CNT_CNT               TIM_CNT_CNT_Msk                              /*!<Counter Value */
#define TIM_CNT_UIFCPY_Pos        (31U)
#define TIM_CNT_UIFCPY_Msk        (0x1UL << TIM_CNT_UIFCPY_Pos)                /*!< 0x80000000 */
#define TIM_CNT_UIFCPY            TIM_CNT_UIFCPY_Msk                           /*!<Update interrupt flag copy */
/*******************  Bit definition for TIM_PSC register  ********************/
#define TIM_PSC_PSC_Pos           (0U)
#define TIM_PSC_PSC_Msk           (0xFFFFUL << TIM_PSC_PSC_Pos)                /*!< 0x0000FFFF */
#define TIM_PSC_PSC               TIM_PSC_PSC_Msk                              /*!<Prescaler Value */

/*******************  Bit definition for TIM_ARR register  ********************/
#define TIM_ARR_ARR_Pos           (0U)
#define TIM_ARR_ARR_Msk           (0xFFFFFFFFUL << TIM_ARR_ARR_Pos)            /*!< 0xFFFFFFFF */
#define TIM_ARR_ARR               TIM_ARR_ARR_Msk                              /*!<actual auto-reload Value */

/*******************  Bit definition for TIM_RCR register  ********************/
#define TIM_RCR_REP_Pos           (0U)
#define TIM_RCR_REP_Msk           (0xFFUL << TIM_RCR_REP_Pos)                  /*!< 0x000000FF */
#define TIM_RCR_REP               TIM_RCR_REP_Msk                              /*!<Repetition Counter Value */

/*******************  Bit definition for TIM_CCR1 register  *******************/
#define TIM_CCR1_CCR1_Pos         (0U)
#define TIM_CCR1_CCR1_Msk         (0xFFFFUL << TIM_CCR1_CCR1_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR1_CCR1             TIM_CCR1_CCR1_Msk                            /*!<Capture/Compare 1 Value */

/*******************  Bit definition for TIM_CCR2 register  *******************/
#define TIM_CCR2_CCR2_Pos         (0U)
#define TIM_CCR2_CCR2_Msk         (0xFFFFUL << TIM_CCR2_CCR2_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR2_CCR2             TIM_CCR2_CCR2_Msk                            /*!<Capture/Compare 2 Value */

/*******************  Bit definition for TIM_CCR3 register  *******************/
#define TIM_CCR3_CCR3_Pos         (0U)
#define TIM_CCR3_CCR3_Msk         (0xFFFFUL << TIM_CCR3_CCR3_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR3_CCR3             TIM_CCR3_CCR3_Msk                            /*!<Capture/Compare 3 Value */

/*******************  Bit definition for TIM_CCR4 register  *******************/
#define TIM_CCR4_CCR4_Pos         (0U)
#define TIM_CCR4_CCR4_Msk         (0xFFFFUL << TIM_CCR4_CCR4_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR4_CCR4             TIM_CCR4_CCR4_Msk                            /*!<Capture/Compare 4 Value */

/*******************  Bit definition for TIM_CCR5 register  *******************/
#define TIM_CCR5_CCR5_Pos         (0U)
#define TIM_CCR5_CCR5_Msk         (0xFFFFFFFFUL << TIM_CCR5_CCR5_Pos)          /*!< 0xFFFFFFFF */
#define TIM_CCR5_CCR5             TIM_CCR5_CCR5_Msk                            /*!<Capture/Compare 5 Value */
#define TIM_CCR5_GC5C1_Pos        (29U)
#define TIM_CCR5_GC5C1_Msk        (0x1UL << TIM_CCR5_GC5C1_Pos)                /*!< 0x20000000 */
#define TIM_CCR5_GC5C1            TIM_CCR5_GC5C1_Msk                           /*!<Group Channel 5 and Channel 1 */
#define TIM_CCR5_GC5C2_Pos        (30U)
#define TIM_CCR5_GC5C2_Msk        (0x1UL << TIM_CCR5_GC5C2_Pos)                /*!< 0x40000000 */
#define TIM_CCR5_GC5C2            TIM_CCR5_GC5C2_Msk                           /*!<Group Channel 5 and Channel 2 */
#define TIM_CCR5_GC5C3_Pos        (31U)
#define TIM_CCR5_GC5C3_Msk        (0x1UL << TIM_CCR5_GC5C3_Pos)                /*!< 0x80000000 */
#define TIM_CCR5_GC5C3            TIM_CCR5_GC5C3_Msk                           /*!<Group Channel 5 and Channel 3 */

/*******************  Bit definition for TIM_CCR6 register  *******************/
#define TIM_CCR6_CCR6_Pos         (0U)
#define TIM_CCR6_CCR6_Msk         (0xFFFFUL << TIM_CCR6_CCR6_Pos)              /*!< 0x0000FFFF */
#define TIM_CCR6_CCR6             TIM_CCR6_CCR6_Msk                            /*!<Capture/Compare 6 Value */

/*******************  Bit definition for TIM_BDTR register  *******************/
#define TIM_BDTR_DTG_Pos          (0U)
#define TIM_BDTR_DTG_Msk          (0xFFUL << TIM_BDTR_DTG_Pos)                 /*!< 0x000000FF */
#define TIM_BDTR_DTG              TIM_BDTR_DTG_Msk                             /*!<DTG[0:7] bits (Dead-Time Generator set-up) */
#define TIM_BDTR_DTG_0            (0x01UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000001 */
#define TIM_BDTR_DTG_1            (0x02UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000002 */
#define TIM_BDTR_DTG_2            (0x04UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000004 */
#define TIM_BDTR_DTG_3            (0x08UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000008 */
#define TIM_BDTR_DTG_4            (0x10UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000010 */
#define TIM_BDTR_DTG_5            (0x20UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000020 */
#define TIM_BDTR_DTG_6            (0x40UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000040 */
#define TIM_BDTR_DTG_7            (0x80UL << TIM_BDTR_DTG_Pos)                  /*!< 0x00000080 */

#define TIM_BDTR_LOCK_Pos         (8U)
#define TIM_BDTR_LOCK_Msk         (0x3UL << TIM_BDTR_LOCK_Pos)                 /*!< 0x00000300 */
#define TIM_BDTR_LOCK             TIM_BDTR_LOCK_Msk                            /*!<LOCK[1:0] bits (Lock Configuration) */
#define TIM_BDTR_LOCK_0           (0x1UL << TIM_BDTR_LOCK_Pos)                  /*!< 0x00000100 */
#define TIM_BDTR_LOCK_1           (0x2UL << TIM_BDTR_LOCK_Pos)                  /*!< 0x00000200 */

#define TIM_BDTR_OSSI_Pos         (10U)
#define TIM_BDTR_OSSI_Msk         (0x1UL << TIM_BDTR_OSSI_Pos)                 /*!< 0x00000400 */
#define TIM_BDTR_OSSI             TIM_BDTR_OSSI_Msk                            /*!<Off-State Selection for Idle mode */
#define TIM_BDTR_OSSR_Pos         (11U)
#define TIM_BDTR_OSSR_Msk         (0x1UL << TIM_BDTR_OSSR_Pos)                 /*!< 0x00000800 */
#define TIM_BDTR_OSSR             TIM_BDTR_OSSR_Msk                            /*!<Off-State Selection for Run mode */
#define TIM_BDTR_BKE_Pos          (12U)
#define TIM_BDTR_BKE_Msk          (0x1UL << TIM_BDTR_BKE_Pos)                  /*!< 0x00001000 */
#define TIM_BDTR_BKE              TIM_BDTR_BKE_Msk                             /*!<Break enable for Break1 */
#define TIM_BDTR_BKP_Pos          (13U)
#define TIM_BDTR_BKP_Msk          (0x1UL << TIM_BDTR_BKP_Pos)                  /*!< 0x00002000 */
#define TIM_BDTR_BKP              TIM_BDTR_BKP_Msk                             /*!<Break Polarity for Break1 */
#define TIM_BDTR_AOE_Pos          (14U)
#define TIM_BDTR_AOE_Msk          (0x1UL << TIM_BDTR_AOE_Pos)                  /*!< 0x00004000 */
#define TIM_BDTR_AOE              TIM_BDTR_AOE_Msk                             /*!<Automatic Output enable */
#define TIM_BDTR_MOE_Pos          (15U)
#define TIM_BDTR_MOE_Msk          (0x1UL << TIM_BDTR_MOE_Pos)                  /*!< 0x00008000 */
#define TIM_BDTR_MOE              TIM_BDTR_MOE_Msk                             /*!<Main Output enable */

#define TIM_BDTR_BKF_Pos          (16U)
#define TIM_BDTR_BKF_Msk          (0xFUL << TIM_BDTR_BKF_Pos)                  /*!< 0x000F0000 */
#define TIM_BDTR_BKF              TIM_BDTR_BKF_Msk                             /*!<Break Filter for Break1 */
#define TIM_BDTR_BK2F_Pos         (20U)
#define TIM_BDTR_BK2F_Msk         (0xFUL << TIM_BDTR_BK2F_Pos)                 /*!< 0x00F00000 */
#define TIM_BDTR_BK2F             TIM_BDTR_BK2F_Msk                            /*!<Break Filter for Break2 */

#define TIM_BDTR_BK2E_Pos         (24U)
#define TIM_BDTR_BK2E_Msk         (0x1UL << TIM_BDTR_BK2E_Pos)                 /*!< 0x01000000 */
#define TIM_BDTR_BK2E             TIM_BDTR_BK2E_Msk                            /*!<Break enable for Break2 */
#define TIM_BDTR_BK2P_Pos         (25U)
#define TIM_BDTR_BK2P_Msk         (0x1UL << TIM_BDTR_BK2P_Pos)                 /*!< 0x02000000 */
#define TIM_BDTR_BK2P             TIM_BDTR_BK2P_Msk                            /*!<Break Polarity for Break2 */
#define TIM_BDTR_BKDSRM_Pos       (26U)
#define TIM_BDTR_BKDSRM_Msk       (0x1UL << TIM_BDTR_BKDSRM_Pos)               /*!< 0x04000000 */
#define TIM_BDTR_BKDSRM           TIM_BDTR_BKDSRM_Msk                          /*!<Break Disarm */
#define TIM_BDTR_BK2DSRM_Pos      (27U)
#define TIM_BDTR_BK2DSRM_Msk      (0x1UL << TIM_BDTR_BK2DSRM_Pos)              /*!< 0x08000000 */
#define TIM_BDTR_BK2DSRM          TIM_BDTR_BK2DSRM_Msk                         /*!<Break2 Disarm */
#define TIM_BDTR_BKBID_Pos        (28U)
#define TIM_BDTR_BKBID_Msk        (0x1UL << TIM_BDTR_BKBID_Pos)                /*!< 0x10000000 */
#define TIM_BDTR_BKBID            TIM_BDTR_BKBID_Msk                           /*!<Break Bidirectional */
#define TIM_BDTR_BK2BID_Pos       (29U)
#define TIM_BDTR_BK2BID_Msk       (0x1UL << TIM_BDTR_BK2BID_Pos)               /*!< 0x20000000 */
#define TIM_BDTR_BK2BID           TIM_BDTR_BK2BID_Msk                          /*!<Break2 Bidirectional */

/*******************  Bit definition for TIM_DCR register  ********************/
#define TIM_DCR_DBA_Pos           (0U)
#define TIM_DCR_DBA_Msk           (0x1FUL << TIM_DCR_DBA_Pos)                  /*!< 0x0000001F */
#define TIM_DCR_DBA               TIM_DCR_DBA_Msk                              /*!<DBA[4:0] bits (DMA Base Address) */
#define TIM_DCR_DBA_0             (0x01UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000001 */
#define TIM_DCR_DBA_1             (0x02UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000002 */
#define TIM_DCR_DBA_2             (0x04UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000004 */
#define TIM_DCR_DBA_3             (0x08UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000008 */
#define TIM_DCR_DBA_4             (0x10UL << TIM_DCR_DBA_Pos)                   /*!< 0x00000010 */

#define TIM_DCR_DBL_Pos           (8U)
#define TIM_DCR_DBL_Msk           (0x1FUL << TIM_DCR_DBL_Pos)                  /*!< 0x00001F00 */
#define TIM_DCR_DBL               TIM_DCR_DBL_Msk                              /*!<DBL[4:0] bits (DMA Burst Length) */
#define TIM_DCR_DBL_0             (0x01UL << TIM_DCR_DBL_Pos)                   /*!< 0x00000100 */
#define TIM_DCR_DBL_1             (0x02UL << TIM_DCR_DBL_Pos)                   /*!< 0x00000200 */
#define TIM_DCR_DBL_2             (0x04UL << TIM_DCR_DBL_Pos)                   /*!< 0x00000400 */
#define TIM_DCR_DBL_3             (0x08UL << TIM_DCR_DBL_Pos)                   /*!< 0x00000800 */
#define TIM_DCR_DBL_4             (0x10UL << TIM_DCR_DBL_Pos)                   /*!< 0x00001000 */

/*******************  Bit definition for TIM_DMAR register  *******************/
#define TIM_DMAR_DMAB_Pos         (0U)
#define TIM_DMAR_DMAB_Msk         (0xFFFFUL << TIM_DMAR_DMAB_Pos)              /*!< 0x0000FFFF */
#define TIM_DMAR_DMAB             TIM_DMAR_DMAB_Msk                            /*!<DMA register for burst accesses */

/******************  Bit definition for TIM_CCMR3 register  *******************/
#define TIM_CCMR3_OC5FE_Pos       (2U)
#define TIM_CCMR3_OC5FE_Msk       (0x1UL << TIM_CCMR3_OC5FE_Pos)               /*!< 0x00000004 */
#define TIM_CCMR3_OC5FE           TIM_CCMR3_OC5FE_Msk                          /*!<Output Compare 5 Fast enable */
#define TIM_CCMR3_OC5PE_Pos       (3U)
#define TIM_CCMR3_OC5PE_Msk       (0x1UL << TIM_CCMR3_OC5PE_Pos)               /*!< 0x00000008 */
#define TIM_CCMR3_OC5PE           TIM_CCMR3_OC5PE_Msk                          /*!<Output Compare 5 Preload enable */

#define TIM_CCMR3_OC5M_Pos        (4U)
#define TIM_CCMR3_OC5M_Msk        (0x1007UL << TIM_CCMR3_OC5M_Pos)                /*!< 0x00010070 */
#define TIM_CCMR3_OC5M            TIM_CCMR3_OC5M_Msk                           /*!<OC5M[3:0] bits (Output Compare 5 Mode) */
#define TIM_CCMR3_OC5M_0          (0x1UL << TIM_CCMR3_OC5M_Pos)                 /*!< 0x00000010 */
#define TIM_CCMR3_OC5M_1          (0x2UL << TIM_CCMR3_OC5M_Pos)                 /*!< 0x00000020 */
#define TIM_CCMR3_OC5M_2          (0x4UL << TIM_CCMR3_OC5M_Pos)                 /*!< 0x00000040 */
#define TIM_CCMR3_OC5M_3          (0x1000UL << TIM_CCMR3_OC5M_Pos)              /*!< 0x00010000 */

#define TIM_CCMR3_OC5CE_Pos       (7U)
#define TIM_CCMR3_OC5CE_Msk       (0x1UL << TIM_CCMR3_OC5CE_Pos)               /*!< 0x00000080 */
#define TIM_CCMR3_OC5CE           TIM_CCMR3_OC5CE_Msk                          /*!<Output Compare 5 Clear Enable */

#define TIM_CCMR3_OC6FE_Pos       (10U)
#define TIM_CCMR3_OC6FE_Msk       (0x1UL << TIM_CCMR3_OC6FE_Pos)               /*!< 0x00000400 */
#define TIM_CCMR3_OC6FE           TIM_CCMR3_OC6FE_Msk                          /*!<Output Compare 4 Fast enable */
#define TIM_CCMR3_OC6PE_Pos       (11U)
#define TIM_CCMR3_OC6PE_Msk       (0x1UL << TIM_CCMR3_OC6PE_Pos)               /*!< 0x00000800 */
#define TIM_CCMR3_OC6PE           TIM_CCMR3_OC6PE_Msk                          /*!<Output Compare 4 Preload enable */

#define TIM_CCMR3_OC6M_Pos        (12U)
#define TIM_CCMR3_OC6M_Msk        (0x1007UL << TIM_CCMR3_OC6M_Pos)                /*!< 0x01007000 */
#define TIM_CCMR3_OC6M            TIM_CCMR3_OC6M_Msk                           /*!<OC4M[3:0] bits (Output Compare 4 Mode) */
#define TIM_CCMR3_OC6M_0          (0x1UL << TIM_CCMR3_OC6M_Pos)                 /*!< 0x00001000 */
#define TIM_CCMR3_OC6M_1          (0x2UL << TIM_CCMR3_OC6M_Pos)                 /*!< 0x00002000 */
#define TIM_CCMR3_OC6M_2          (0x4UL << TIM_CCMR3_OC6M_Pos)                 /*!< 0x00004000 */
#define TIM_CCMR3_OC6M_3          (0x1000UL << TIM_CCMR3_OC6M_Pos)              /*!< 0x01000000 */

#define TIM_CCMR3_OC6CE_Pos       (15U)
#define TIM_CCMR3_OC6CE_Msk       (0x1UL << TIM_CCMR3_OC6CE_Pos)               /*!< 0x00008000 */
#define TIM_CCMR3_OC6CE           TIM_CCMR3_OC6CE_Msk                          /*!<Output Compare 4 Clear Enable */
/*******************  Bit definition for TIM1_AF1 register  *********************/
#define TIM1_AF1_BKINE_Pos        (0U)
#define TIM1_AF1_BKINE_Msk        (0x1UL << TIM1_AF1_BKINE_Pos)                /*!< 0x00000001 */
#define TIM1_AF1_BKINE            TIM1_AF1_BKINE_Msk                           /*!<BKINE Break input enable bit */
#define TIM1_AF1_BKCMP1E_Pos      (1U)
#define TIM1_AF1_BKCMP1E_Msk      (0x1UL << TIM1_AF1_BKCMP1E_Pos)              /*!< 0x00000002 */
#define TIM1_AF1_BKCMP1E          TIM1_AF1_BKCMP1E_Msk                         /*!<BKCMP1E Break Compare1 Enable bit */
#define TIM1_AF1_BKCMP2E_Pos      (2U)
#define TIM1_AF1_BKCMP2E_Msk      (0x1UL << TIM1_AF1_BKCMP2E_Pos)              /*!< 0x00000004 */
#define TIM1_AF1_BKCMP2E          TIM1_AF1_BKCMP2E_Msk                         /*!<BKCMP1E Break Compare2 Enable bit */
#define TIM1_AF1_BKDF1BK0E_Pos    (8U)
#define TIM1_AF1_BKDF1BK0E_Msk    (0x1UL << TIM1_AF1_BKDF1BK0E_Pos)            /*!< 0x00000100 */
#define TIM1_AF1_BKDF1BK0E        TIM1_AF1_BKDF1BK0E_Msk                       /*!<BKDF1BK0E Break input DFSDM Break 0 */
#define TIM1_AF1_BKINP_Pos        (9U)
#define TIM1_AF1_BKINP_Msk        (0x1UL << TIM1_AF1_BKINP_Pos)                /*!< 0x00000200 */
#define TIM1_AF1_BKINP            TIM1_AF1_BKINP_Msk                           /*!<BRKINP Break input polarity */
#define TIM1_AF1_BKCMP1P_Pos      (10U)
#define TIM1_AF1_BKCMP1P_Msk      (0x1UL << TIM1_AF1_BKCMP1P_Pos)              /*!< 0x00000400 */
#define TIM1_AF1_BKCMP1P          TIM1_AF1_BKCMP1P_Msk                         /*!<BKCMP1P Break COMP1 input polarity */
#define TIM1_AF1_BKCMP2P_Pos      (11U)
#define TIM1_AF1_BKCMP2P_Msk      (0x1UL << TIM1_AF1_BKCMP2P_Pos)              /*!< 0x00000800 */
#define TIM1_AF1_BKCMP2P          TIM1_AF1_BKCMP2P_Msk                         /*!<BKCMP2P Break COMP2 input polarity */

#define TIM1_AF1_ETRSEL_Pos       (14U)
#define TIM1_AF1_ETRSEL_Msk       (0xFUL << TIM1_AF1_ETRSEL_Pos)               /*!< 0x0003C000 */
#define TIM1_AF1_ETRSEL           TIM1_AF1_ETRSEL_Msk                          /*!<ETRSEL[3:0] bits (TIM1 ETRSEL) */
#define TIM1_AF1_ETRSEL_0         (0x1UL << TIM1_AF1_ETRSEL_Pos)                /*!< 0x00004000 */
#define TIM1_AF1_ETRSEL_1         (0x2UL << TIM1_AF1_ETRSEL_Pos)                /*!< 0x00008000 */
#define TIM1_AF1_ETRSEL_2         (0x4UL << TIM1_AF1_ETRSEL_Pos)                /*!< 0x00010000 */
#define TIM1_AF1_ETRSEL_3         (0x8UL << TIM1_AF1_ETRSEL_Pos)                /*!< 0x00020000 */

/*******************  Bit definition for TIM1_AF2 register  *********************/
#define TIM1_AF2_BK2INE_Pos       (0U)
#define TIM1_AF2_BK2INE_Msk       (0x1UL << TIM1_AF2_BK2INE_Pos)               /*!< 0x00000001 */
#define TIM1_AF2_BK2INE           TIM1_AF2_BK2INE_Msk                          /*!<BK2INE Break input 2 enable bit */
#define TIM1_AF2_BK2CMP1E_Pos     (1U)
#define TIM1_AF2_BK2CMP1E_Msk     (0x1UL << TIM1_AF2_BK2CMP1E_Pos)             /*!< 0x00000002 */
#define TIM1_AF2_BK2CMP1E         TIM1_AF2_BK2CMP1E_Msk                        /*!<BK2CMP1E Break2 Compare1 Enable bit */
#define TIM1_AF2_BK2CMP2E_Pos     (2U)
#define TIM1_AF2_BK2CMP2E_Msk     (0x1UL << TIM1_AF2_BK2CMP2E_Pos)             /*!< 0x00000004 */
#define TIM1_AF2_BK2CMP2E         TIM1_AF2_BK2CMP2E_Msk                        /*!<BK2CMP1E Break2 Compare2 Enable bit  */
#define TIM1_AF2_BK2DFBK1E_Pos    (8U)
#define TIM1_AF2_BK2DFBK1E_Msk    (0x1UL << TIM1_AF2_BK2DFBK1E_Pos)            /*!< 0x00000100 */
#define TIM1_AF2_BK2DFBK1E        TIM1_AF2_BK2DFBK1E_Msk                       /*!<BK2DFBK1E Break input2 DFSDM Break 1 */
#define TIM1_AF2_BK2INP_Pos       (9U)
#define TIM1_AF2_BK2INP_Msk       (0x1UL << TIM1_AF2_BK2INP_Pos)               /*!< 0x00000200 */
#define TIM1_AF2_BK2INP           TIM1_AF2_BK2INP_Msk                          /*!<BRKINP Break2 input polarity */
#define TIM1_AF2_BK2CMP1P_Pos     (10U)
#define TIM1_AF2_BK2CMP1P_Msk     (0x1UL << TIM1_AF2_BK2CMP1P_Pos)             /*!< 0x00000400 */
#define TIM1_AF2_BK2CMP1P         TIM1_AF2_BK2CMP1P_Msk                        /*!<BKCMP1P Break2 COMP1 input polarity */
#define TIM1_AF2_BK2CMP2P_Pos     (11U)
#define TIM1_AF2_BK2CMP2P_Msk     (0x1UL << TIM1_AF2_BK2CMP2P_Pos)             /*!< 0x00000800 */
#define TIM1_AF2_BK2CMP2P         TIM1_AF2_BK2CMP2P_Msk                        /*!<BKCMP2P Break2 COMP2 input polarity */

/*******************  Bit definition for TIM_TISEL register  *********************/
#define TIM_TISEL_TI1SEL_Pos      (0U)
#define TIM_TISEL_TI1SEL_Msk      (0xFUL << TIM_TISEL_TI1SEL_Pos)              /*!< 0x0000000F */
#define TIM_TISEL_TI1SEL          TIM_TISEL_TI1SEL_Msk                         /*!<TI1SEL[3:0] bits (TIM TI1 SEL)*/
#define TIM_TISEL_TI1SEL_0        (0x1UL << TIM_TISEL_TI1SEL_Pos)               /*!< 0x00000001 */
#define TIM_TISEL_TI1SEL_1        (0x2UL << TIM_TISEL_TI1SEL_Pos)               /*!< 0x00000002 */
#define TIM_TISEL_TI1SEL_2        (0x4UL << TIM_TISEL_TI1SEL_Pos)               /*!< 0x00000004 */
#define TIM_TISEL_TI1SEL_3        (0x8UL << TIM_TISEL_TI1SEL_Pos)               /*!< 0x00000008 */

#define TIM_TISEL_TI2SEL_Pos      (8U)
#define TIM_TISEL_TI2SEL_Msk      (0xFUL << TIM_TISEL_TI2SEL_Pos)              /*!< 0x00000F00 */
#define TIM_TISEL_TI2SEL          TIM_TISEL_TI2SEL_Msk                         /*!<TI2SEL[3:0] bits (TIM TI2 SEL)*/
#define TIM_TISEL_TI2SEL_0        (0x1UL << TIM_TISEL_TI2SEL_Pos)               /*!< 0x00000100 */
#define TIM_TISEL_TI2SEL_1        (0x2UL << TIM_TISEL_TI2SEL_Pos)               /*!< 0x00000200 */
#define TIM_TISEL_TI2SEL_2        (0x4UL << TIM_TISEL_TI2SEL_Pos)               /*!< 0x00000400 */
#define TIM_TISEL_TI2SEL_3        (0x8UL << TIM_TISEL_TI2SEL_Pos)               /*!< 0x00000800 */

#define TIM_TISEL_TI3SEL_Pos      (16U)
#define TIM_TISEL_TI3SEL_Msk      (0xFUL << TIM_TISEL_TI3SEL_Pos)              /*!< 0x000F0000 */
#define TIM_TISEL_TI3SEL          TIM_TISEL_TI3SEL_Msk                         /*!<TI3SEL[3:0] bits (TIM TI3 SEL)*/
#define TIM_TISEL_TI3SEL_0        (0x1UL << TIM_TISEL_TI3SEL_Pos)               /*!< 0x00010000 */
#define TIM_TISEL_TI3SEL_1        (0x2UL << TIM_TISEL_TI3SEL_Pos)               /*!< 0x00020000 */
#define TIM_TISEL_TI3SEL_2        (0x4UL << TIM_TISEL_TI3SEL_Pos)               /*!< 0x00040000 */
#define TIM_TISEL_TI3SEL_3        (0x8UL << TIM_TISEL_TI3SEL_Pos)               /*!< 0x00080000 */

#define TIM_TISEL_TI4SEL_Pos      (24U)
#define TIM_TISEL_TI4SEL_Msk      (0xFUL << TIM_TISEL_TI4SEL_Pos)              /*!< 0x0F000000 */
#define TIM_TISEL_TI4SEL          TIM_TISEL_TI4SEL_Msk                         /*!<TI4SEL[3:0] bits (TIM TI4 SEL)*/
#define TIM_TISEL_TI4SEL_0        (0x1UL << TIM_TISEL_TI4SEL_Pos)               /*!< 0x01000000 */
#define TIM_TISEL_TI4SEL_1        (0x2UL << TIM_TISEL_TI4SEL_Pos)               /*!< 0x02000000 */
#define TIM_TISEL_TI4SEL_2        (0x4UL << TIM_TISEL_TI4SEL_Pos)               /*!< 0x04000000 */
#define TIM_TISEL_TI4SEL_3        (0x8UL << TIM_TISEL_TI4SEL_Pos)               /*!< 0x08000000 */

/*******************  Bit definition for TIM8_AF1 register  *********************/
#define TIM8_AF1_BKINE_Pos        (0U)
#define TIM8_AF1_BKINE_Msk        (0x1UL << TIM8_AF1_BKINE_Pos)                /*!< 0x00000001 */
#define TIM8_AF1_BKINE            TIM8_AF1_BKINE_Msk                           /*!<BKINE Break input enable bit */
#define TIM8_AF1_BKCMP1E_Pos      (1U)
#define TIM8_AF1_BKCMP1E_Msk      (0x1UL << TIM8_AF1_BKCMP1E_Pos)              /*!< 0x00000002 */
#define TIM8_AF1_BKCMP1E          TIM8_AF1_BKCMP1E_Msk                         /*!<BKCMP1E Break Compare1 Enable bit */
#define TIM8_AF1_BKCMP2E_Pos      (2U)
#define TIM8_AF1_BKCMP2E_Msk      (0x1UL << TIM8_AF1_BKCMP2E_Pos)              /*!< 0x00000004 */
#define TIM8_AF1_BKCMP2E          TIM8_AF1_BKCMP2E_Msk                         /*!<BKCMP1E Break Compare2 Enable bit  */
#define TIM8_AF1_BKDFBK2E_Pos     (8U)
#define TIM8_AF1_BKDFBK2E_Msk     (0x1UL << TIM8_AF1_BKDFBK2E_Pos)             /*!< 0x00000100 */
#define TIM8_AF1_BKDFBK2E         TIM8_AF1_BKDFBK2E_Msk                        /*!<BKDFBK2E Break input DFSDM Break 2 */
#define TIM8_AF1_BKINP_Pos        (9U)
#define TIM8_AF1_BKINP_Msk        (0x1UL << TIM8_AF1_BKINP_Pos)                /*!< 0x00000200 */
#define TIM8_AF1_BKINP            TIM8_AF1_BKINP_Msk                           /*!<BRKINP Break input polarity */
#define TIM8_AF1_BKCMP1P_Pos      (10U)
#define TIM8_AF1_BKCMP1P_Msk      (0x1UL << TIM8_AF1_BKCMP1P_Pos)              /*!< 0x00000400 */
#define TIM8_AF1_BKCMP1P          TIM8_AF1_BKCMP1P_Msk                         /*!<BKCMP1P Break COMP1 input polarity */
#define TIM8_AF1_BKCMP2P_Pos      (11U)
#define TIM8_AF1_BKCMP2P_Msk      (0x1UL << TIM8_AF1_BKCMP2P_Pos)              /*!< 0x00000800 */
#define TIM8_AF1_BKCMP2P          TIM8_AF1_BKCMP2P_Msk                         /*!<BKCMP2P Break COMP2 input polarity */

#define TIM8_AF1_ETRSEL_Pos       (14U)
#define TIM8_AF1_ETRSEL_Msk       (0xFUL << TIM8_AF1_ETRSEL_Pos)               /*!< 0x0003C000 */
#define TIM8_AF1_ETRSEL           TIM8_AF1_ETRSEL_Msk                          /*!<ETRSEL[3:0] bits (TIM8 ETRSEL) */
#define TIM8_AF1_ETRSEL_0         (0x1UL << TIM8_AF1_ETRSEL_Pos)                /*!< 0x00004000 */
#define TIM8_AF1_ETRSEL_1         (0x2UL << TIM8_AF1_ETRSEL_Pos)                /*!< 0x00008000 */
#define TIM8_AF1_ETRSEL_2         (0x4UL << TIM8_AF1_ETRSEL_Pos)                /*!< 0x00010000 */
#define TIM8_AF1_ETRSEL_3         (0x8UL << TIM8_AF1_ETRSEL_Pos)                /*!< 0x00020000 */
/*******************  Bit definition for TIM8_AF2 register  *********************/
#define TIM8_AF2_BK2INE_Pos       (0U)
#define TIM8_AF2_BK2INE_Msk       (0x1UL << TIM8_AF2_BK2INE_Pos)               /*!< 0x00000001 */
#define TIM8_AF2_BK2INE           TIM8_AF2_BK2INE_Msk                          /*!<BK2INE Break input 2 enable bit */
#define TIM8_AF2_BK2CMP1E_Pos     (1U)
#define TIM8_AF2_BK2CMP1E_Msk     (0x1UL << TIM8_AF2_BK2CMP1E_Pos)             /*!< 0x00000002 */
#define TIM8_AF2_BK2CMP1E         TIM8_AF2_BK2CMP1E_Msk                        /*!<BK2CMP1E Break2 Compare1 Enable bit */
#define TIM8_AF2_BK2CMP2E_Pos     (2U)
#define TIM8_AF2_BK2CMP2E_Msk     (0x1UL << TIM8_AF2_BK2CMP2E_Pos)             /*!< 0x00000004 */
#define TIM8_AF2_BK2CMP2E         TIM8_AF2_BK2CMP2E_Msk                        /*!<BK2CMP1E Break2 Compare2 Enable bit  */
#define TIM8_AF2_BK2DFBK3E_Pos    (8U)
#define TIM8_AF2_BK2DFBK3E_Msk    (0x1UL << TIM8_AF2_BK2DFBK3E_Pos)            /*!< 0x00000100 */
#define TIM8_AF2_BK2DFBK3E        TIM8_AF2_BK2DFBK3E_Msk                       /*!<BK2DFBK1E Break input2 DFSDM Break 3 */
#define TIM8_AF2_BK2INP_Pos       (9U)
#define TIM8_AF2_BK2INP_Msk       (0x1UL << TIM8_AF2_BK2INP_Pos)               /*!< 0x00000200 */
#define TIM8_AF2_BK2INP           TIM8_AF2_BK2INP_Msk                          /*!<BRKINP Break2 input polarity */
#define TIM8_AF2_BK2CMP1P_Pos     (10U)
#define TIM8_AF2_BK2CMP1P_Msk     (0x1UL << TIM8_AF2_BK2CMP1P_Pos)             /*!< 0x00000400 */
#define TIM8_AF2_BK2CMP1P         TIM8_AF2_BK2CMP1P_Msk                        /*!<BKCMP1P Break2 COMP1 input polarity */
#define TIM8_AF2_BK2CMP2P_Pos     (11U)
#define TIM8_AF2_BK2CMP2P_Msk     (0x1UL << TIM8_AF2_BK2CMP2P_Pos)             /*!< 0x00000800 */
#define TIM8_AF2_BK2CMP2P         TIM8_AF2_BK2CMP2P_Msk                        /*!<BKCMP2P Break2 COMP2 input polarity */

/*******************  Bit definition for TIM2_AF1 register  *********************/
#define TIM2_AF1_ETRSEL_Pos      (14U)
#define TIM2_AF1_ETRSEL_Msk      (0xFUL << TIM2_AF1_ETRSEL_Pos)                /*!< 0x0003C000 */
#define TIM2_AF1_ETRSEL          TIM2_AF1_ETRSEL_Msk                           /*!<ETRSEL[3:0] bits (TIM2 ETRSEL) */
#define TIM2_AF1_ETRSEL_0        (0x1UL << TIM2_AF1_ETRSEL_Pos)                 /*!< 0x00004000 */
#define TIM2_AF1_ETRSEL_1        (0x2UL << TIM2_AF1_ETRSEL_Pos)                 /*!< 0x00008000 */
#define TIM2_AF1_ETRSEL_2        (0x4UL << TIM2_AF1_ETRSEL_Pos)                 /*!< 0x00010000 */
#define TIM2_AF1_ETRSEL_3        (0x8UL << TIM2_AF1_ETRSEL_Pos)                 /*!< 0x00020000 */

/*******************  Bit definition for TIM3_AF1 register  *********************/
#define TIM3_AF1_ETRSEL_Pos      (14U)
#define TIM3_AF1_ETRSEL_Msk      (0xFUL << TIM3_AF1_ETRSEL_Pos)                /*!< 0x0003C000 */
#define TIM3_AF1_ETRSEL          TIM3_AF1_ETRSEL_Msk                           /*!<ETRSEL[3:0] bits (TIM3 ETRSEL) */
#define TIM3_AF1_ETRSEL_0        (0x1UL << TIM3_AF1_ETRSEL_Pos)                 /*!< 0x00004000 */
#define TIM3_AF1_ETRSEL_1        (0x2UL << TIM3_AF1_ETRSEL_Pos)                 /*!< 0x00008000 */
#define TIM3_AF1_ETRSEL_2        (0x4UL << TIM3_AF1_ETRSEL_Pos)                 /*!< 0x00010000 */
#define TIM3_AF1_ETRSEL_3        (0x8UL << TIM3_AF1_ETRSEL_Pos)                 /*!< 0x00020000 */

/*******************  Bit definition for TIM5_AF1 register  *********************/
#define TIM5_AF1_ETRSEL_Pos      (14U)
#define TIM5_AF1_ETRSEL_Msk      (0xFUL << TIM5_AF1_ETRSEL_Pos)                /*!< 0x0003C000 */
#define TIM5_AF1_ETRSEL          TIM5_AF1_ETRSEL_Msk                           /*!<ETRSEL[3:0] bits (TIM5 ETRSEL) */
#define TIM5_AF1_ETRSEL_0        (0x1UL << TIM5_AF1_ETRSEL_Pos)                 /*!< 0x00004000 */
#define TIM5_AF1_ETRSEL_1        (0x2UL << TIM5_AF1_ETRSEL_Pos)                 /*!< 0x00008000 */
#define TIM5_AF1_ETRSEL_2        (0x4UL << TIM5_AF1_ETRSEL_Pos)                 /*!< 0x00010000 */
#define TIM5_AF1_ETRSEL_3        (0x8UL << TIM5_AF1_ETRSEL_Pos)                 /*!< 0x00020000 */

/*******************  Bit definition for TIM15_AF1 register  *********************/
#define TIM15_AF1_BKINE_Pos        (0U)
#define TIM15_AF1_BKINE_Msk        (0x1UL << TIM15_AF1_BKINE_Pos)              /*!< 0x00000001 */
#define TIM15_AF1_BKINE            TIM15_AF1_BKINE_Msk                         /*!<BKINE Break input enable bit */
#define TIM15_AF1_BKCMP1E_Pos      (1U)
#define TIM15_AF1_BKCMP1E_Msk      (0x1UL << TIM15_AF1_BKCMP1E_Pos)            /*!< 0x00000002 */
#define TIM15_AF1_BKCMP1E          TIM15_AF1_BKCMP1E_Msk                       /*!<BKCMP1E Break Compare1 Enable bit */
#define TIM15_AF1_BKCMP2E_Pos      (2U)
#define TIM15_AF1_BKCMP2E_Msk      (0x1UL << TIM15_AF1_BKCMP2E_Pos)            /*!< 0x00000004 */
#define TIM15_AF1_BKCMP2E          TIM15_AF1_BKCMP2E_Msk                       /*!<BKCMP1E Break Compare2 Enable bit  */
#define TIM15_AF1_BKDF1BK2E_Pos    (8U)
#define TIM15_AF1_BKDF1BK2E_Msk    (0x1UL << TIM15_AF1_BKDF1BK2E_Pos)          /*!< 0x00000100 */
#define TIM15_AF1_BKDF1BK2E        TIM15_AF1_BKDF1BK2E_Msk                     /*!<BRK dfsdm1_break[0] enable */
#define TIM15_AF1_BKINP_Pos        (9U)
#define TIM15_AF1_BKINP_Msk        (0x1UL << TIM15_AF1_BKINP_Pos)              /*!< 0x00000200 */
#define TIM15_AF1_BKINP            TIM15_AF1_BKINP_Msk                         /*!<BRKINP Break input polarity */
#define TIM15_AF1_BKCMP1P_Pos      (10U)
#define TIM15_AF1_BKCMP1P_Msk      (0x1UL << TIM15_AF1_BKCMP1P_Pos)            /*!< 0x00000400 */
#define TIM15_AF1_BKCMP1P          TIM15_AF1_BKCMP1P_Msk                       /*!<BKCMP1P Break COMP1 input polarity */
#define TIM15_AF1_BKCMP2P_Pos      (11U)
#define TIM15_AF1_BKCMP2P_Msk      (0x1UL << TIM15_AF1_BKCMP2P_Pos)            /*!< 0x00000800 */
#define TIM15_AF1_BKCMP2P          TIM15_AF1_BKCMP2P_Msk                       /*!<BKCMP2P Break COMP2 input polarity */

/*******************  Bit definition for TIM16_ register  *********************/
#define TIM16_AF1_BKINE_Pos        (0U)
#define TIM16_AF1_BKINE_Msk        (0x1UL << TIM16_AF1_BKINE_Pos)              /*!< 0x00000001 */
#define TIM16_AF1_BKINE            TIM16_AF1_BKINE_Msk                         /*!<BKINE Break input enable bit */
#define TIM16_AF1_BKCMP1E_Pos      (1U)
#define TIM16_AF1_BKCMP1E_Msk      (0x1UL << TIM16_AF1_BKCMP1E_Pos)            /*!< 0x00000002 */
#define TIM16_AF1_BKCMP1E          TIM16_AF1_BKCMP1E_Msk                       /*!<BKCMP1E Break Compare1 Enable bit */
#define TIM16_AF1_BKCMP2E_Pos      (2U)
#define TIM16_AF1_BKCMP2E_Msk      (0x1UL << TIM16_AF1_BKCMP2E_Pos)            /*!< 0x00000004 */
#define TIM16_AF1_BKCMP2E          TIM16_AF1_BKCMP2E_Msk                       /*!<BKCMP1E Break Compare2 Enable bit  */
#define TIM16_AF1_BKDF1BK2E_Pos    (8U)
#define TIM16_AF1_BKDF1BK2E_Msk    (0x1UL << TIM16_AF1_BKDF1BK2E_Pos)          /*!< 0x00000100 */
#define TIM16_AF1_BKDF1BK2E        TIM16_AF1_BKDF1BK2E_Msk                     /*!<BRK dfsdm1_break[1] enable */
#define TIM16_AF1_BKINP_Pos        (9U)
#define TIM16_AF1_BKINP_Msk        (0x1UL << TIM16_AF1_BKINP_Pos)              /*!< 0x00000200 */
#define TIM16_AF1_BKINP            TIM16_AF1_BKINP_Msk                         /*!<BRKINP Break input polarity */
#define TIM16_AF1_BKCMP1P_Pos      (10U)
#define TIM16_AF1_BKCMP1P_Msk      (0x1UL << TIM16_AF1_BKCMP1P_Pos)            /*!< 0x00000400 */
#define TIM16_AF1_BKCMP1P          TIM16_AF1_BKCMP1P_Msk                       /*!<BKCMP1P Break COMP1 input polarity */
#define TIM16_AF1_BKCMP2P_Pos      (11U)
#define TIM16_AF1_BKCMP2P_Msk      (0x1UL << TIM16_AF1_BKCMP2P_Pos)            /*!< 0x00000800 */
#define TIM16_AF1_BKCMP2P          TIM16_AF1_BKCMP2P_Msk                       /*!<BKCMP2P Break COMP2 input polarity */

/*******************  Bit definition for TIM17_AF1 register  *********************/
#define TIM17_AF1_BKINE_Pos        (0U)
#define TIM17_AF1_BKINE_Msk        (0x1UL << TIM17_AF1_BKINE_Pos)              /*!< 0x00000001 */
#define TIM17_AF1_BKINE            TIM17_AF1_BKINE_Msk                         /*!<BKINE Break input enable bit */
#define TIM17_AF1_BKCMP1E_Pos      (1U)
#define TIM17_AF1_BKCMP1E_Msk      (0x1UL << TIM17_AF1_BKCMP1E_Pos)            /*!< 0x00000002 */
#define TIM17_AF1_BKCMP1E          TIM17_AF1_BKCMP1E_Msk                       /*!<BKCMP1E Break Compare1 Enable bit */
#define TIM17_AF1_BKCMP2E_Pos      (2U)
#define TIM17_AF1_BKCMP2E_Msk      (0x1UL << TIM17_AF1_BKCMP2E_Pos)            /*!< 0x00000004 */
#define TIM17_AF1_BKCMP2E          TIM17_AF1_BKCMP2E_Msk                       /*!<BKCMP1E Break Compare2 Enable bit  */
#define TIM17_AF1_BKDF1BK2E_Pos    (8U)
#define TIM17_AF1_BKDF1BK2E_Msk    (0x1UL << TIM17_AF1_BKDF1BK2E_Pos)          /*!< 0x00000100 */
#define TIM17_AF1_BKDF1BK2E        TIM17_AF1_BKDF1BK2E_Msk                     /*!<BRK dfsdm1_break[2] enable */
#define TIM17_AF1_BKINP_Pos        (9U)
#define TIM17_AF1_BKINP_Msk        (0x1UL << TIM17_AF1_BKINP_Pos)              /*!< 0x00000200 */
#define TIM17_AF1_BKINP            TIM17_AF1_BKINP_Msk                         /*!<BRKINP Break input polarity */
#define TIM17_AF1_BKCMP1P_Pos      (10U)
#define TIM17_AF1_BKCMP1P_Msk      (0x1UL << TIM17_AF1_BKCMP1P_Pos)            /*!< 0x00000400 */
#define TIM17_AF1_BKCMP1P          TIM17_AF1_BKCMP1P_Msk                       /*!<BKCMP1P Break COMP1 input polarity */
#define TIM17_AF1_BKCMP2P_Pos      (11U)
#define TIM17_AF1_BKCMP2P_Msk      (0x1UL << TIM17_AF1_BKCMP2P_Pos)            /*!< 0x00000800 */
#define TIM17_AF1_BKCMP2P          TIM17_AF1_BKCMP2P_Msk                       /*!<BKCMP2P Break COMP2 input polarity */
