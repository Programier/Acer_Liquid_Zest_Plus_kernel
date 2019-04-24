#ifdef BUILD_LK
#else
#include <linux/string.h>
#endif
#include "lcm_drv.h"
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef BUILD_LK
//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
#endif
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

#define LCM_ID       (0x8394)

static LCM_UTIL_FUNCS lcm_util;

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V3(para_tbl,size,force_update)        lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

#define   LCM_DSI_CMD_MODE							0

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFD   // END OF REGISTERS MARKER

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

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

static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

	{0xB9, 3,{0xFF,0x83,0x94}},
		   
	{0xBA, 6,{0x63,0x03,0x68,0x6B,0xB2,0xC0}},
		   
	{0xB1,11,{0x50,0x12,0x72,0x09,0x32,0x24,0x71,0x51,0x30,0x43}},
		   
	{0xB2, 6,{0x00,0x80,0x64,0x0E,0x0A,0x2F}},
		   
	{0xB4,21,{0x1C,0x78,0x1C,0x78,0x1C,0x78,0x01,0x0C,0x86,0x75,0x00,0x3F,0x1C,0x78,0x1C,0x78,0x1C,0x78,0x01,0x0C,0x86}},
		   
	{0xB6, 2,{0x6A,0x6A}},
		   
	{0xD3,33,{0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x32,0x10,0x03,0x00,0x03,0x32,0x13,0xC0,0x00,0x00,0x32,0x10,0x08,0x00,0x00,0x37,0x04,0x05,0x05,0x37,0x05,0x05,0x47,0x0E,0x40}},
		   
	{0xD5,44,{0x18,0x18,0x18,0x18,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x19,0x19,0x19,0x19,0x20,0x21,0x22,0x23}},
		   
	{0xD6,44,{0x18,0x18,0x19,0x19,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x19,0x19,0x18,0x18,0x23,0x22,0x21,0x20}},
		   
	{0xE0,58,{0x00,0x01,0x08,0x0E,0x11,0x13,0x16,0x12,0x29,0x37,0x47,0x46,0x4E,0x60,0x63,0x66,0x74,0x79,0x78,0x87,0x99,0x4E,0x4D,0x54,0x5A,0x5D,0x67,0x7E,0x7F,0x00,0x01,0x08,0x0E,0x11,0x13,0x16,0x13,0x29,0x37,0x46,0x45,0x4D,0x5F,0x63,0x67,0x75,0x79,0x78,0x89,0x9C,0x4F,0x4E,0x54,0x5B,0x5F,0x69,0x7F,0x7F}},
			      
	{0xC0, 2,{0x1F,0x31}},
		   
	{0xCC, 1,{0x0B}},//03 屏幕翻转
		   
	{0xD4, 1,{0x02}},
		   
	{0xBD, 1,{0x01}},
		   
	{0xD8,12,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}},
		   
	{0xBD, 1,{0x00}},
			      
	{0xBF, 7,{0x40,0x81,0x50,0x00,0x1A,0xFC,0x01}},

	//{0x35,1,{0x00}},	       //open EXT TE Signal

	{0x11,1,{0x00}},				 // Sleep-Out
	{REGFLAG_DELAY, 120, {}},
	{0x29,1,{0x00}},				 // Display On
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}

//{REGFLAG_END_OF_TABLE, 0x00, {}} 	  

	/* FIXME */
	/*
		params->dsi.horizontal_sync_active				= 0x16;// 50  2
		params->dsi.horizontal_backporch				= 0x38;
		params->dsi.horizontal_frontporch				= 0x18;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		params->dsi.horizontal_blanking_pixel =0;    //lenovo:fix flicker issue
	    //params->dsi.LPX=8; 
	*/

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

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = BURST_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_FOUR_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.vertical_sync_active				= 0x04;
		params->dsi.vertical_backporch					= 0x0C;
		params->dsi.vertical_frontporch					= 0x0A; 
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 0x12;
		params->dsi.horizontal_backporch				= 0x30;
		params->dsi.horizontal_frontporch				= 0x30;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	    //params->dsi.LPX=8; 

		// Bit rate calculation
		params->dsi.PLL_CLOCK = 210;
		//1 Every lane speed
		//params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps
		//params->dsi.pll_div2=0;		// div2=0,1,2,3;div1_real=1,2,4,4	
		//params->dsi.fbk_div =9;    // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
		
		// Non-continuous clock 
	params->dsi.noncont_clock = TRUE; 
		//params->dsi.noncont_clock_period = 2; // Unit : frames		
		
	params->dsi.clk_lp_per_line_enable 		= 1;	
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;

	params->dsi.lcm_esd_check_table[0].cmd          = 0x09;
    	params->dsi.lcm_esd_check_table[0].count        = 3;
    	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x80;
	params->dsi.lcm_esd_check_table[0].para_list[1] = 0x73;
	params->dsi.lcm_esd_check_table[0].para_list[2] = 0x04;
	
	params->dsi.lcm_esd_check_table[1].cmd          = 0xd9;
	params->dsi.lcm_esd_check_table[1].count        = 1;
	params->dsi.lcm_esd_check_table[1].para_list[0] = 0x80;
		
	params->dsi.lcm_esd_check_table[2].cmd          = 0x45;
	params->dsi.lcm_esd_check_table[2].count        = 2;
	params->dsi.lcm_esd_check_table[2].para_list[0] = 0x5;
	params->dsi.lcm_esd_check_table[2].para_list[1] = 0x18;	

}

static void lcm_init(void)
{
		SET_RESET_PIN(1);
		MDELAY(10); 
		SET_RESET_PIN(0);
		MDELAY(10); 
		SET_RESET_PIN(1);
		MDELAY(120); 

		push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	
	MDELAY(20); 
	
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	
	MDELAY(120); 

	
	/*SET_RESET_PIN(1);	
	SET_RESET_PIN(0);
	MDELAY(1); // 1ms
	
	SET_RESET_PIN(1);
	MDELAY(20);*/

}


static void lcm_resume(void)
{
	lcm_init();
}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{

    unsigned int id = 0;
    unsigned char buffer[3];

    unsigned int data_array[16];
        
        
    SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
    MDELAY(10);
	
    SET_RESET_PIN(0);
    MDELAY(10);
	
    SET_RESET_PIN(1);
    MDELAY(10);	

    data_array[0]=0x00043902; 
    data_array[1]=0x9483FFB9;
    dsi_set_cmdq(data_array, 2, 1); 
    MDELAY(10);
	
    data_array[0] = 0x00033700;
    dsi_set_cmdq(data_array, 1, 1); 

    read_reg_v2(0x04, buffer, 3); 
    id = (buffer[0] << 8) | buffer[1]; //we only need ID
    
    #ifdef BUILD_LK
		printf("%s, LK debug: read id, buf:0x%02x ,0x%02x,0x%02x, id=0X%X", __func__, buffer[0], buffer[1], buffer[2], id);
    #else
		printk("%s, LK debug: read id, buf:0x%02x ,0x%02x,0x%02x, id=0X%X", __func__, buffer[0], buffer[1], buffer[2], id);
    #endif

    return (0x94 == buffer[1])?1:0;


}

#if 0
static unsigned int lcm_esd_check(void)
{
  #ifndef BUILD_LK
	char  buffer[3];
	int   array[4];

	if(lcm_esd_test)
	{
		lcm_esd_test = FALSE;
		return TRUE;
	}

	array[0] = 0x00013700;
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x36, buffer, 1);
	if(buffer[0]==0x90)
	{
		return FALSE;
	}
	else
	{			 
		return TRUE;
	}
#else
	return FALSE;
#endif

}

static unsigned int lcm_esd_recover(void)
{
	lcm_init();
	lcm_resume();

	return TRUE;
}
#endif


LCM_DRIVER hx8394f_dsi_vdo_hd720_by_zal1506_lcm_drv = 
{
    .name			= "hx8394f_dsi_vdo_hd720_by_zal1506",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
	//.esd_check = lcm_esd_check,
	//.esd_recover = lcm_esd_recover,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
