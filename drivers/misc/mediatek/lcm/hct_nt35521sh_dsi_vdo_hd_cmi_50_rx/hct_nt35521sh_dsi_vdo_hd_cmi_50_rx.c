
/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include "lcm_drv.h"



// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                                          (720)
#define FRAME_HEIGHT                                         (1280)

#define REGFLAG_DELAY                                         0XFE
#define REGFLAG_END_OF_TABLE                                  0xFF   // END OF REGISTERS MARKER

#define LCM_ID_NT35521  0x5521
#define LCM_DSI_CMD_MODE                                    0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)        lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                    lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                            lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size) 

struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{

        memset(params, 0, sizeof(LCM_PARAMS));

        params->type   = LCM_TYPE_DSI;

        params->width  = FRAME_WIDTH;
        params->height = FRAME_HEIGHT;

        params->physical_width = 63;
        params->physical_height = 110;

        // enable tearing-free
        params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
        //params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

        params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE;

        // DSI
        /* Command mode setting */
        //1 Three lane or Four lane
        params->dsi.LANE_NUM				= LCM_FOUR_LANE;
        //The following defined the fomat for data coming from LCD engine.
        params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
        params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
        params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
        params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

        // Highly depends on LCD driver capability.
        // Not support in MT6573
        params->dsi.packet_size=256;

        // Video mode setting
        params->dsi.intermediat_buffer_num = 0;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

        params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
        params->dsi.word_count=720*3;
     	params->physical_width = 63;
     	params->physical_height = 110;

        params->dsi.vertical_sync_active				= 4;// 3    2
        params->dsi.vertical_backporch					= 38;// 20   1
        params->dsi.vertical_frontporch					= 40; // 1  12
        params->dsi.vertical_active_line				= FRAME_HEIGHT;

        params->dsi.horizontal_sync_active				= 10;// 50  2
        params->dsi.horizontal_backporch				= 100;//72
        params->dsi.horizontal_frontporch				= 100;//72
        params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

        params->dsi.clk_lp_per_line_enable = 1;  
        params->dsi.esd_check_enable = 1; 
        params->dsi.customization_esd_check_enable = 1;   
        params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;    
        params->dsi.lcm_esd_check_table[0].count        = 1;     
        params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;
        //params->dsi.LPX=8;
        params->dsi.noncont_clock = 1;   
        params->dsi.noncont_clock_period = 2;
        params->dsi.ssc_disable = 1;

		// Bit rate calculation
		//1 Every lane speed
        	//params->dsi.pll_select=1;
#if FRAME_WIDTH == 480	
	params->dsi.PLL_CLOCK=210;//254//247
#elif FRAME_WIDTH == 540
	params->dsi.PLL_CLOCK=230;
#elif FRAME_WIDTH == 720
	params->dsi.PLL_CLOCK=230;
#elif FRAME_WIDTH == 1080
	params->dsi.PLL_CLOCK=410;
#else
	params->dsi.PLL_CLOCK=230;
#endif //this value must be in MTK suggested table
//    params->dsi.compatibility_for_nvk = 1;  // this macro new base is unuse
}

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++) {
       
        unsigned cmd;
        cmd = table[i].cmd;
       
        switch (cmd) {

            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }

}

#if 0
static void lcm_initialization(void)
{
    unsigned int data_array[16];


    data_array[0] = 0x00063902;
    data_array[1] = 0x52AA55F0;
    data_array[2] = 0x00000008;                 
    dsi_set_cmdq(data_array, 3, 1);

    MDELAY(2);
    data_array[0] = 0x00033902;
    data_array[1] = 0x002168B1;                
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(2);
    data_array[0] = 0x00023902;
    data_array[1] = 0x0000C8B5;                
    dsi_set_cmdq(data_array, 2, 1);
    MDELAY(2);
    data_array[0] = 0x00023902;
    data_array[1] = 0x00000FB6;                
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00053902;                                    
    data_array[1] = 0x0A0000B8;
    data_array[2] = 0x00000000;                
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00023902;                                  
    data_array[1] = 0x000000B9;            
    dsi_set_cmdq(data_array, 2, 1);    

    data_array[0] = 0x00023902;              
    data_array[1] = 0x000002BA;                
    dsi_set_cmdq(data_array, 2, 1);      

    data_array[0] = 0x00033902;
    data_array[1] = 0x006363BB;                
    dsi_set_cmdq(data_array, 2, 1);  

    data_array[0] = 0x00033902;
    data_array[1] = 0x000000BC;                
    dsi_set_cmdq(data_array, 2, 1); 

    data_array[0] = 0x00063902;
    data_array[1] = 0x0D7F02BD;
    data_array[2] = 0x0000000B;                 
    dsi_set_cmdq(data_array, 3, 1); 

    data_array[0] = 0x00113902;
    data_array[1] = 0x873641CC;
    data_array[2] = 0x10654654;
    data_array[3] = 0x12101412;
    data_array[4] = 0x15084014;
    data_array[5] = 0x00000005;                 
    dsi_set_cmdq(data_array, 6, 1);

    data_array[0] = 0x00023902;
    data_array[1] = 0x000000D0;                  
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00113902;
    data_array[1] = 0x080400D1;
    data_array[2] = 0x1814100C;
    data_array[3] = 0x2824201C;
    data_array[4] = 0x3834302C;
    data_array[5] = 0x0000003C;                 
    dsi_set_cmdq(data_array, 6, 1); 

    data_array[0] = 0x00023902;
    data_array[1] = 0x000000D3;                
    dsi_set_cmdq(data_array, 2, 1); 

    data_array[0] = 0x00033902;
    data_array[1] = 0x004444D6;                
    dsi_set_cmdq(data_array, 2, 1); 


    data_array[0] = 0x000D3902;
    data_array[1] = 0x000000D7;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;                 
    dsi_set_cmdq(data_array, 5, 1);   

    data_array[0] = 0x000E3902;
    data_array[1] = 0x000000D8;
    data_array[2] = 0x00000000;
    data_array[3] = 0x00000000;
    data_array[4] = 0x00000000;                  
    dsi_set_cmdq(data_array, 5, 1);   

    data_array[0] = 0x00033902;
    data_array[1] = 0x000603D9; //               
    dsi_set_cmdq(data_array, 2, 1); 

    data_array[0] = 0x00033902;
    data_array[1] = 0x00FF00E5;                  
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00053902;
    data_array[1] = 0xE7ECF3E6;
    data_array[2] = 0x000000DF;                
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x000B3902; 
    data_array[1] = 0xCCD9F3E7; 
    data_array[2] = 0x99A6B3CD; 
    data_array[3] = 0x00959999;                
    dsi_set_cmdq(data_array, 4, 1);

    data_array[0] = 0x000B3902; 
    data_array[1] = 0xCCD9F3E8; 
    data_array[2] = 0x99A6B3CD; 
    data_array[3] = 0x00959999;                
    dsi_set_cmdq(data_array, 4, 1);

    data_array[0] = 0x00033902; 
    data_array[1] = 0x000400E9;                
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902; 
    data_array[1] = 0x000000EA;                
    dsi_set_cmdq(data_array, 2, 1);


    data_array[0] = 0x00053902;     
    data_array[1] = 0x007887EE; 
    data_array[2] = 0x00000000;                
    dsi_set_cmdq(data_array, 3, 1); 

    data_array[0] = 0x00033902;
    data_array[1] = 0x00FF07EF;        
    dsi_set_cmdq(data_array, 2, 1);  

    data_array[0] = 0x00063902;
    data_array[1] = 0x52AA55F0; 
    data_array[2] = 0x00000108;                  
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00033902;                  
    data_array[1] = 0x000D0DB0;                  
    dsi_set_cmdq(data_array, 2, 1);              
    data_array[0] = 0x00033902;                  
    data_array[1] = 0x000D0DB1;                  
    dsi_set_cmdq(data_array, 2, 1);              
    data_array[0] = 0x00033902;                  
    data_array[1] = 0x002D2DB3;    
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00033902;    
    data_array[1] = 0x001919B4;    
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00033902;       
    data_array[1] = 0x000404B5; //           
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00033902;            
    data_array[1] = 0x000505B6;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;            
    data_array[1] = 0x000505B7;            
    dsi_set_cmdq(data_array, 2, 1);       
    data_array[0] = 0x00033902;            
    data_array[1] = 0x000505B8;       
    dsi_set_cmdq(data_array, 2, 1);  
    data_array[0] = 0x00033902;       
    data_array[1] = 0x003333B9; //      
    dsi_set_cmdq(data_array, 2, 1);  
    data_array[0] = 0x00033902;       
    data_array[1] = 0x001616BA; //      
    dsi_set_cmdq(data_array, 2, 1);  
    data_array[0] = 0x00033902;       
    data_array[1] = 0x000086BC;    //6e
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00033902;
    data_array[1] = 0x000086BD;   //6e 
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;
    data_array[1] = 0x00002aBE; //    vcom 20  26
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;    
    data_array[1] = 0x000021BF; //

    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;    
    data_array[1] = 0x000004C0; //   
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00023902;    
    data_array[1] = 0x000000C1;    
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00033902;    
    data_array[1] = 0x001919C2;    
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00033902;
    data_array[1] = 0x000A0AC3;    
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00033902;           
    data_array[1] = 0x002323C4;           
    dsi_set_cmdq(data_array, 2, 1);      
    data_array[0] = 0x00043902;           
    data_array[1] = 0x008000C7;           
    dsi_set_cmdq(data_array, 2, 1);      
    data_array[0] = 0x00073902;           
    data_array[1] = 0x000000C9;           
    data_array[2] = 0x00000000;           
    dsi_set_cmdq(data_array, 3, 1);      
    data_array[0] = 0x00023902;           
    data_array[1] = 0x000001CA;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00530BCB;            
    dsi_set_cmdq(data_array, 2, 1);    
    data_array[0] = 0x00023902;
    data_array[1] = 0x000000CC;    
    dsi_set_cmdq(data_array, 2, 1);
    data_array[0] = 0x00043902;           
    data_array[1] = 0x53520BCD;           
    dsi_set_cmdq(data_array, 2, 1);      
    data_array[0] = 0x00023902;           
    data_array[1] = 0x000044CE;           
    dsi_set_cmdq(data_array, 2, 1);      
    data_array[0] = 0x00043902;           
    data_array[1] = 0x505000CF;           
    dsi_set_cmdq(data_array, 2, 1);      
    data_array[0] = 0x00033902;           
    data_array[1] = 0x005050D0;           
    dsi_set_cmdq(data_array, 2, 1);      
    data_array[0] = 0x00033902;           
    data_array[1] = 0x005050D1;         
    dsi_set_cmdq(data_array, 2, 1);    
    data_array[0] = 0x00023902;
    data_array[1] = 0x000037D2; //         
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;
    data_array[1] = 0x000039D3;          
    dsi_set_cmdq(data_array, 2, 1);    

    data_array[0] = 0x00063902;          
    data_array[1] = 0x52AA55F0;           
    data_array[2] = 0x00000208;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00023902;                            
    data_array[1] = 0x000001EE;          
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00113902;          
    data_array[1] = 0x00c700B0; //          
    data_array[2] = 0x01f600d9;           
    data_array[3] = 0x011d010b;           
    data_array[4] = 0x0153013a;           
    data_array[5] = 0x0000007b;        
    dsi_set_cmdq(data_array, 6, 1);

    data_array[0] = 0x00113902;          
    data_array[1] = 0x019c01B1;//           
    data_array[2] = 0x02f701d0;           
    data_array[3] = 0x026a0236;           
    data_array[4] = 0x029a026b;           
    data_array[5] = 0x000000cb;        
    dsi_set_cmdq(data_array, 6, 1); 

    data_array[0] = 0x00113902;          
    data_array[1] = 0x03e902B2; //          
    data_array[2] = 0x03310313;           
    data_array[3] = 0x03700358;           
    data_array[4] = 0x03a60391;           
    data_array[5] = 0x000000c2;        
    dsi_set_cmdq(data_array, 6, 1);   

    data_array[0] = 0x00053902;          
    data_array[1] = 0x03ee03B3; //         
    data_array[2] = 0x000000ff;           
    dsi_set_cmdq(data_array, 3, 1);


    data_array[0] = 0x00063902;          
    data_array[1] = 0x52AA55F0;           
    data_array[2] = 0x00000308;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x000000B0;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;//06
    data_array[1] = 0x000000B1;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x000003B2;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x000003B3;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x000003B4;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);   

    data_array[0] = 0x00063902;          
    data_array[1] = 0x000003B5;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x000003B6;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x000003B7;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x000003B8;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x000003B9;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x001035BA;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x001035BB;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x001035BC;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x001035BD;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00053902;          
    data_array[1] = 0x003400C0;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00053902;          
    data_array[1] = 0x003400C1;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00053902;          
    data_array[1] = 0x003400C2;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00053902;          
    data_array[1] = 0x003400C3;           
    data_array[2] = 0x00000000;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000040C4;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000040C5;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000040C6;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000040C7;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000000EF;        
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x52AA55F0;           
    data_array[2] = 0x00000508;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00101BB0;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00101BB1;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00101BB2;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00101BB3;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00101BB4;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00101BB5;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00101BB6;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;           
    data_array[1] = 0x00101BB7;           
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;            
    data_array[1] = 0x000000B8;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;            
    data_array[1] = 0x000000B9;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;            
    data_array[1] = 0x000000BA;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;            
    data_array[1] = 0x000000BB;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;            
    data_array[1] = 0x000000BC;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00063902;            
    data_array[1] = 0x030303BD;            
    data_array[2] = 0x00000100;            
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00023902;//0C        
    data_array[1] = 0x000003C0;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902; //0D       
    data_array[1] = 0x000005C1;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;            
    data_array[1] = 0x000003C2;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;            
    data_array[1] = 0x000005C3;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;            
    data_array[1] = 0x000080C4;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;               
    data_array[1] = 0x0000A2C5;               
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;//12           
    data_array[1] = 0x000080C6;               
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;                             
    data_array[1] = 0x0000A2C7;            

    dsi_set_cmdq(data_array, 2, 1);                    
    data_array[0] = 0x00033902;                             
    data_array[1] = 0x002001C8;            

    dsi_set_cmdq(data_array, 2, 1);                           
    data_array[0] = 0x00033902;                            
    data_array[1] = 0x002000C9;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;                            
    data_array[1] = 0x000001CA;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;                            
    data_array[1] = 0x000000CB;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00043902;//18                        
    data_array[1] = 0x010000CC;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00043902;//18                        
    data_array[1] = 0x010000CD;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00043902;//18                        
    data_array[1] = 0x010000CE;            
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00043902;//18                        
    data_array[1] = 0x010000CF;            
    dsi_set_cmdq(data_array, 2, 1);        

    data_array[0] = 0x00023902; ////
    data_array[1] = 0x000000D0;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00063902;    
    data_array[1] = 0x000003D1;    
    data_array[2] = 0x00001007;    
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;    
    data_array[1] = 0x000013D2;    
    data_array[2] = 0x00001107;    
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;    
    data_array[1] = 0x000023D3;    
    data_array[2] = 0x00001007;    
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;    
    data_array[1] = 0x000033D4;    
    data_array[2] = 0x00001107;    
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000006E5;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000006E6;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000006E7;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000006E8;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000006E9;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000006EA;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000006EB;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000006EC;    
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;    
    data_array[1] = 0x000031ED;    
    dsi_set_cmdq(data_array, 2, 1);  

    data_array[0] = 0x00063902;          
    data_array[1] = 0x52AA55F0;           
    data_array[2] = 0x00000608;          
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x001110B0;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x001312B1;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000008B2;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D2DB3;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x00342DB4;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D34B5;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x00342DB6;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x003434B7;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000A02B8;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000800B9;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000109BA;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x00030BBB;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x003434BC;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D34BD;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x00342DBE;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D34BF;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D2DC0;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000901C1;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x001819C2;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x001617C3;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x001819C4;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x001617C5;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000901C6;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D2DC7;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x00342DC8;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D34C9;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x00342DCA;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x003434CB;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x00030BCC;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000109CD;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000800CE;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000A02CF;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x003434D0;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D34D1;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x00342DD2;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D34D3;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x002D2DD4;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x000008D5;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x001110D6;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x001312D7;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x555555D8;          
    data_array[2] = 0x00005555;                      
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00063902;          
    data_array[1] = 0x555555D9;          
    data_array[2] = 0x00005555;                      
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x003434E5;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00033902;          
    data_array[1] = 0x003434E6;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;          
    data_array[1] = 0x000005E7;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;          
    data_array[1] = 0x00000035;                      
    dsi_set_cmdq(data_array, 2, 1);


    data_array[0] = 0x00110500;               
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(120);
    data_array[0] = 0x00290500;               
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(60);

    data_array[0] = 0x00023902;          
    data_array[1] = 0x00002c53;                      
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0] = 0x00023902;          
    data_array[1] = 0x0000ff51;                      
    dsi_set_cmdq(data_array, 2, 1);
}
#endif
static struct LCM_setting_table lcm_initialization_setting[] = {


		{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
    {0xC8, 1, {0x80}},//80
     
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
    {0xBC, 2, {0x7A,0x00}},//90     
    {0xBD, 2, {0x7A,0x00}},//90     
    {0xCA, 1, {0x01}},     
		{0xC0,1,{0x0C}},
    {0xBE, 1, {0x60}},//VCOM 55 5A
    //{0xBE, 1, {0x4E}},  
    {0xB3, 2, {0x38,0x38}}, 
    {0xB4, 2, {0x11,0x11}},    
    {0xB6, 2, {0x05,0x05}},     
    {0xB9, 2, {0x45,0x45}},     
    {0xBA, 2, {0x25,0x25}},     
    {0xC4, 2, {0x11,0x11}},     
    {0xCE, 1, {0x66}},     

		{0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
        //3Gamma 2.2
    {0xB0, 16, {0x00,0x17,0x00,0x53,0x00,0x82,0x00,0x9F,0x00,0xB4,0x00,0xD8,0x00,0xF3,0x01,0x1D}},
    {0xB1, 16, {0x01,0x40,0x01,0x78,0x01,0xA1,0x01,0xE5,0x02,0x1C,0x02,0x1D,0x02,0x4F,0x02,0x86}},
    {0xB2, 16, {0x02,0xAA,0x02,0xDA,0x02,0xF9,0x03,0x26,0x03,0x3E,0x03,0x5E,0x03,0x6D,0x03,0x6E}},
    {0xB3, 4, {0x03,0xD5,0x03,0xE7}},
    {0xB4, 16, {0x00,0x1D,0x00,0x4F,0x00,0x7E,0x00,0x99,0x00,0xB1,0x00,0xD4,0x00,0xF1,0x01,0x1C}},
    {0xB5, 16, {0x01,0x3F,0x01,0x75,0x01,0xA0,0x01,0xE3,0x02,0x1A,0x02,0x1C,0x02,0x4F,0x02,0x86}},
    {0xB6, 16, {0x02,0xAA,0x02,0xDB,0x02,0xFD,0x03,0x28,0x03,0x45,0x03,0x6A,0x03,0x7B,0x03,0xC2}},
    {0xB7, 4, {0x03,0xD3,0x03,0xED}},

    {0xB8, 16, {0x00,0x04,0x00,0x41,0x00,0x6D,0x00,0x8A,0x00,0xA1,0x00,0xC7,0x00,0xE5,0x01,0x12}},
    {0xB9, 16, {0x01,0x36,0x01,0x6E,0x01,0x9A,0x01,0xE0,0x02,0x18,0x02,0x19,0x02,0x4D,0x02,0x85}},
    {0xBA, 16, {0x02,0xA9,0x02,0xDC,0x02,0xFF,0x03,0x2F,0x03,0x52,0x03,0x95,0x03,0xD5,0x03,0xF1}},
    {0xBB, 4, {0x03,0xF8,0x03,0xFB}},
        //R(-) MCR cmd
    {0xBC, 16, {0x00,0x17,0x00,0x53,0x00,0x82,0x00,0x9F,0x00,0xB4,0x00,0xD8,0x00,0xF3,0x01,0x1D}},
    {0xBD, 16, {0x01,0x40,0x01,0x78,0x01,0xA1,0x01,0xE5,0x02,0x1C,0x02,0x1D,0x02,0x4F,0x02,0x86}},
    {0xBE, 16, {0x02,0xAA,0x02,0xDA,0x02,0xF9,0x03,0x26,0x03,0x3E,0x03,0x5E,0x03,0x6D,0x03,0x6E}},
    {0xBF, 4, {0x03,0xD5,0x03,0xE7}},
    {0xC0, 16, {0x00,0x1D,0x00,0x4F,0x00,0x7E,0x00,0x99,0x00,0xB1,0x00,0xD4,0x00,0xF1,0x01,0x1C}},
    {0xC1, 16, {0x01,0x3F,0x01,0x75,0x01,0xA0,0x01,0xE3,0x02,0x1A,0x02,0x1C,0x02,0x4F,0x02,0x86}},
    {0xC2, 16, {0x02,0xAA,0x02,0xDB,0x02,0xFD,0x03,0x28,0x03,0x45,0x03,0x6A,0x03,0x7B,0x03,0xC2}},
    {0xC3, 4, {0x03,0xD3,0x03,0xED}},
        //B(-) MCR cmd
    {0xC4, 16, {0x00,0x04,0x00,0x41,0x00,0x6D,0x00,0x8A,0x00,0xA1,0x00,0xC7,0x00,0xE5,0x01,0x12}},
    {0xC5, 16, {0x01,0x36,0x01,0x6E,0x01,0x9A,0x01,0xE0,0x02,0x18,0x02,0x19,0x02,0x4D,0x02,0x85}},
    {0xC6, 16, {0x02,0xA9,0x02,0xDC,0x02,0xFF,0x03,0x2F,0x03,0x52,0x03,0x95,0x03,0xD5,0x03,0xF1}},
    {0xC7, 4, {0x03,0xF8,0x03,0xFB}},


		{0xF0,5,{0x55,0xAA,0x52,0x08,0x06}},
    {0xB0, 2, {0x29,0x2A}},     
    {0xB1, 2, {0x10,0x12}},     
    {0xB2, 2, {0x14,0x16}},     
    {0xB3, 2, {0x18,0x1A}},     
    {0xB4, 2, {0x02,0x04}},     
    {0xB5, 2, {0x34,0x34}},     
    {0xB6, 2, {0x34,0x2E}},     
    {0xB7, 2, {0x2E,0x2E}},     
    {0xB8, 2, {0x34,0x00}},     
    {0xB9, 2, {0x34,0x34}},     

    {0xBA, 2, {0x34,0x34}}, 
    {0xBB, 2, {0x01,0x34}},     
    {0xBC, 2, {0x2E,0x2E}},     
    {0xBD, 2, {0x2E,0x34}},     
    {0xBE, 2, {0x34,0x34}},     
    {0xBF, 2, {0x05,0x03}},     
    {0xC0, 2, {0x1B,0x19}},     
    {0xC1, 2, {0x17,0x15}},     
    {0xC2, 2, {0x13,0x11}},     
    {0xC3, 2, {0x2A,0x29}},    
    {0xE5, 2, {0x2E,0x2E}},     

    {0xC4, 2, {0x29,0x2A}},     
    {0xC5, 2, {0x1B,0x19}},     
    {0xC6, 2, {0x17,0x15}},     
    {0xC7, 2, {0x13,0x11}},     
    {0xC8, 2, {0x01,0x05}},     
    {0xC9, 2, {0x34,0x34}},     
    {0xCA, 2, {0x34,0x2E}},     
    {0xCB, 2, {0x2E,0x2E}},     
    {0xCC, 2, {0x34,0x03}},     
    {0xCD, 2, {0x34,0x34}},     

    {0xCE, 2, {0x34,0x34}},     
    {0xCF, 2, {0x02,0x34}},     
    {0xD0, 2, {0x2E,0x2E}},     
    {0xD1, 2, {0x2E,0x34}},     
    {0xD2, 2, {0x34,0x34}},     
    {0xD3, 2, {0x04,0x00}},     
    {0xD4, 2, {0x10,0x12}},     
    {0xD5, 2, {0x14,0x16}},     
    {0xD6, 2, {0x18,0x1A}},     
    {0xD7, 2, {0x2A,0x29}},     
    {0xE6, 2, {0x2E,0x2E}},     

    {0xD8, 5, {0x00,0x00,0x00,0x54,0x00}},     
    {0xD9, 5, {0x00,0x15,0x00,0x00,0x00}},     
		{0xE7,1,{0x00}},
         
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
    {0xB1, 2, {0x00,0x00}},     
    {0xB0, 2, {0x00,0x00}},     
    {0xB2, 5, {0x05,0x00,0x00,0x00,0x00}},     
    {0xB3, 5, {0x05,0x00,0x00,0x00,0x00}},     
    {0xB4, 5, {0x05,0x00,0x00,0x00,0x00}},     
    {0xB5, 5, {0x05,0x00,0x17,0x00,0x00}},     
    {0xB6, 5, {0x12,0x00,0x19,0x00,0x00}},     
    {0xB7, 5, {0x12,0x00,0x19,0x00,0x00}},     
    {0xB8, 5, {0x12,0x00,0x19,0x00,0x00}},     
    {0xB9, 5, {0x12,0x00,0x19,0x00,0x00}},     
    {0xBA, 5, {0x57,0x00,0x00,0x00,0x00}},     
    {0xBB, 5, {0x57,0x00,0x00,0x00,0x00}},     
    {0xBC, 5, {0x75,0x00,0x1A,0x00,0x00}},     
    {0xBD, 5, {0x53,0x00,0x1A,0x00,0x00}},     

    {0xC0, 4, {0x00,0x34,0x00,0x00}},     
    {0xC1, 4, {0x00,0x34,0x00,0x00}},  
    {0xC2, 4, {0x00,0x34,0x00,0x00}},     
    {0xC3, 4, {0x00,0x34,0x00,0x00}},     
    {0xC4, 1, {0x20}},     
    {0xC5, 1, {0x00}},     
    {0xC6, 1, {0x00}},     
    {0xC7, 1, {0x00}},     
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xED, 1, {0x30}},     
		{0xB0,2,{0x17,0x06}},
    {0xB8, 1, {0x08}},     
    {0xBD, 5, {0x03,0x07,0x00,0x03,0x00}},     
		{0xB1,2,{0x17,0x06}},
    {0xB9, 1, {0x00}},     
    {0xB2, 2, {0x00,0x00}},     
    {0xBA, 1, {0x00}},     
		{0xB3,2,{0x17,0x06}},
    {0xBB, 1, {0x0A}},     
		{0xB4,2,{0x17,0x06}},
		{0xB5,2,{0x17,0x06}},
    {0xB6, 2, {0x14,0x03}},     
    {0xB7, 2, {0x00,0x00}},     
    {0xBC, 1, {0x02}},     
		{0xE5,1,{0x06}},
		{0xE6,1,{0x06}},
    {0xE7, 1, {0x00}},     
		{0xE8,1,{0x06}},
    {0xE9, 1, {0x06}},     
		{0xEA,1,{0x06}},
    {0xEB, 1, {0x00}},     
    {0xEC, 1, {0x00}},     

		{0xC0,1,{0x07}},
    {0xC1, 1, {0x80}},     


    {0xC2, 1, {0xA4}},     
    {0xC3, 1, {0x05}},     


    {0xC4, 1, {0x00}},     

    {0xC5, 1, {0x02}},     
    {0xC6, 1, {0x22}},     
    {0xC7, 1, {0x03}},     

    {0xC8, 2, {0x05,0x30}},     
    {0xC9, 2, {0x01,0x31}},     
    {0xCA, 2, {0x03,0x21}},     
    {0xCB, 2, {0x01,0x20}},     
    {0xD1, 5, {0x00,0x05,0x09,0x07,0x10}},     
    {0xD2, 5, {0x10,0x05,0x0E,0x03,0x10}},     
    {0xD3, 5, {0x20,0x00,0x48,0x07,0x10}},     
    {0xD4, 5, {0x30,0x00,0x43,0x07,0x10}},     
    {0xD0, 1, {0x00}},     
    {0xCC, 3, {0x00,0x00,0x3E}},  
    {0xCD, 3, {0x00,0x00,0x3E}},     

    {0xCE, 3, {0x00,0x00,0x02}},       
    {0xCF, 3, {0x00,0x00,0x02}},      
    {0x6F, 1, {0x11}},     
    {0xF3, 1, {0x01}},    

        {0x35,1,{0x00}},
    {0x62, 1, {0x01}},//ESD
        //{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
       // {0xEF,2,{0x02,0xFF}},


{0x11,1,{0x00}},
{REGFLAG_DELAY, 120, {}},

{0x29,1,{0x00}},
{REGFLAG_DELAY, 50, {}},


{REGFLAG_END_OF_TABLE, 0x00, {}}
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
};

static void lcm_init(void)
{
    
    SET_RESET_PIN(1);
	MDELAY(5);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);
   // lcm_initialization();
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
    unsigned int data_array[16];
    data_array[0]=0x00280500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(10);
    data_array[0]=0x00100500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(100);

    SET_RESET_PIN(0);
    MDELAY(20);
}


static void lcm_resume(void)
{
     lcm_init();
/*
    unsigned int data_array[16];
    data_array[0]=0x00110500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(100);
    data_array[0]=0x00290500;
    dsi_set_cmdq(data_array,1,1);
    MDELAY(10);
*/
}


static unsigned int lcm_compare_id(void)
{
    unsigned int id=0;
    unsigned char buffer[3];
    unsigned int array[16]; 
    unsigned int data_array[16];

    SET_RESET_PIN(1);
        MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
   
    SET_RESET_PIN(1);
    MDELAY(120);

    data_array[0] = 0x00063902;
    data_array[1] = 0x52AA55F0; 
    data_array[2] = 0x00000108;               
    dsi_set_cmdq(data_array, 3, 1);

    array[0] = 0x00033700;// read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
   
    read_reg_v2(0xC5, buffer, 3);
    id = buffer[1]; //we only need ID
    #ifdef BUILD_LK
        printf("%s, LK nt35590 debug: nt35590 id = 0x%08x buffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n", __func__, id,buffer[0],buffer[1],buffer[2]);
    #else
        printk("%s, LK nt35590 debug: nt35590 id = 0x%08x buffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n", __func__, id,buffer[0],buffer[1],buffer[2]);
    #endif

   // if(id == LCM_ID_NT35521)
    if(buffer[0]==0x55 && buffer[1]==0x21)
        return 1;
    else
        return 0;


}









// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_nt35521sh_dsi_vdo_hd_cmi_50_rx= 
{
    .name			= "hct_nt35521sh_dsi_vdo_hd_cmi_50_rx",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .compare_id    = lcm_compare_id,
#if 0//defined(LCM_DSI_CMD_MODE)
//    .set_backlight	= lcm_setbacklight,
    //.set_pwm        = lcm_setpwm,
    //.get_pwm        = lcm_getpwm,
    .update         = lcm_update
#endif
};

