
#include <linux/types.h>
#include <mt-plat/charging.h>
#include <mt-plat/upmu_common.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <mt-plat/mt_boot.h>
#include <mt-plat/battery_common.h>
#include <mach/mt_charging.h>
#include <mach/mt_pmic.h>
#include "bq25890.h"
#include <linux/gpio.h>
#include <mach/gpio_const.h>


/* ============================================================ // */
/* Define */
/* ============================================================ // */
#define STATUS_OK    0
#define STATUS_UNSUPPORTED    -1
#define GETARRAYNUM(array) (sizeof(array)/sizeof(array[0]))

/* ============================================================ // */
/* Global variable */
/* ============================================================ // */

#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
#define WIRELESS_CHARGER_EXIST_STATE 0

#if defined(GPIO_PWR_AVAIL_WLC)
/*K.S.?*/
unsigned int wireless_charger_gpio_number = GPIO_PWR_AVAIL_WLC;
#else
unsigned int wireless_charger_gpio_number = 0;
#endif

#endif

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
static CHARGER_TYPE g_charger_type = CHARGER_UNKNOWN;
#endif

kal_bool charging_type_det_done = KAL_TRUE;





/*BQ25890 REG06 VREG[5:0]*/
const unsigned int VBAT_CV_VTH[] = {
	3840000, 3856000, 3872000, 3888000,
	3904000, 3920000, 3936000, 3952000,
	3968000, 3984000, 4000000, 4016000,
	4032000, 4048000, 4064000, 4080000,
	4096000, 4112000, 4128000, 4144000,
	4160000, 4176000, 4192000, 4208000,
	4224000, 4240000, 4256000, 4272000,
	4288000, 4304000, 4320000, 4336000,
	4352000, 4368000, 4384000, 4400000,
	4416000, 4432000, 4448000, 4464000,
	4480000, 4496000, 4512000, 4528000,
	4544000, 4560000, 4576000, 4592000,
	4608000
};

/*BQ25890 REG04 ICHG[6:0]*/
const unsigned int CS_VTH[] = {
	0, 6400, 12800, 19200,
	25600, 32000, 38400, 44800,
	51200, 57600, 64000, 70400,
	76800, 83200, 89600, 96000,
	102400, 108800, 115200, 121600,
	128000, 134400, 140800, 147200,
	153600, 160000, 166400, 172800,
	179200, 185600, 192000, 198400,
	204800, 211200, 217600, 224000,
	230400, 236800, 243200, 249600,
	256000, 262400, 268800, 275200,
	281600, 288000, 294400, 300800,
	307200, 313600, 320000, 326400,
	332800, 339200, 345600, 352000,
	358400, 364800, 371200, 377600,
	384000, 390400, 396800, 403200,
	409600, 416000, 422400, 428800,
	435200, 441600, 448000, 454400,
	460800, 467200, 473600, 480000,
	486400, 492800, 499200, 505600
};

/*BQ25890 REG00 IINLIM[5:0]*/
const unsigned int INPUT_CS_VTH[] = {
	10000, 15000, 20000, 25000,
	30000, 35000, 40000, 45000,
	50000, 55000, 60000, 65000,
	70000, 75000, 80000, 85000,
	90000, 95000, 100000, 105000,
	110000, 115000, 120000, 125000,
	130000, 135000, 140000, 145000,
	150000, 155000, 160000, 165000,
	170000, 175000, 180000, 185000,
	190000, 195000, 200000, 200500,
	210000, 215000, 220000, 225000,
	230000, 235000, 240000, 245000,
	250000, 255000, 260000, 265000,
	270000, 275000, 280000, 285000,
	290000, 295000, 300000, 305000,
	310000, 315000, 320000, 325000
};

const unsigned int VCDT_HV_VTH[] = {
	BATTERY_VOLT_04_200000_V, BATTERY_VOLT_04_250000_V, BATTERY_VOLT_04_300000_V,
	BATTERY_VOLT_04_350000_V,
	BATTERY_VOLT_04_400000_V, BATTERY_VOLT_04_450000_V, BATTERY_VOLT_04_500000_V,
	BATTERY_VOLT_04_550000_V,
	BATTERY_VOLT_04_600000_V, BATTERY_VOLT_06_000000_V, BATTERY_VOLT_06_500000_V,
	BATTERY_VOLT_07_000000_V,
	BATTERY_VOLT_07_500000_V, BATTERY_VOLT_08_500000_V, BATTERY_VOLT_09_500000_V,
	BATTERY_VOLT_10_500000_V
};

#ifdef CONFIG_MTK_DUAL_INPUT_CHARGER_SUPPORT
#ifndef CUST_GPIO_VIN_SEL
#define CUST_GPIO_VIN_SEL 18
#endif
DISO_IRQ_Data DISO_IRQ;
int g_diso_state = 0;
int vin_sel_gpio_number = (CUST_GPIO_VIN_SEL | 0x80000000);
static char *DISO_state_s[8] = {
	"IDLE",
	"OTG_ONLY",
	"USB_ONLY",
	"USB_WITH_OTG",
	"DC_ONLY",
	"DC_WITH_OTG",
	"DC_WITH_USB",
	"DC_USB_OTG",
};
#endif

/* ============================================================ // */
/* function prototype */
/* ============================================================ // */


/* ============================================================ // */
/* extern variable */
/* ============================================================ // */

/* ============================================================ // */
/* extern function */
/* ============================================================ // */
/* extern unsigned int upmu_get_reg_value(unsigned int reg); upmu_common.h, _not_ used */
/* extern bool mt_usb_is_device(void); _not_ used */
/* extern void Charger_Detect_Init(void); _not_ used */
/* extern void Charger_Detect_Release(void); _not_ used */
/* extern int hw_charging_get_charger_type(void);  included in charging.h*/
/* extern void mt_power_off(void); _not_ used */
/* extern unsigned int mt6311_get_chip_id(void); _not_ used*/
/* extern int is_mt6311_exist(void); _not_ used */
/* extern int is_mt6311_sw_ready(void); _not_ used */

static unsigned int charging_error;
static unsigned int charging_set_error_state(void *data);
/* ============================================================ // */
unsigned int charging_value_to_parameter(const unsigned int *parameter, const unsigned int array_size,
				       const unsigned int val)
{
	if (val < array_size) {
		return parameter[val];
	} else {
		pr_notice("Can't find the parameter \r\n");
		return parameter[0];
	}
}

unsigned int charging_parameter_to_value(const unsigned int *parameter, const unsigned int array_size,
				       const unsigned int val)
{
	unsigned int i;

	pr_debug("array_size = %d \r\n", array_size);

	for (i = 0; i < array_size; i++) {
		if (val == *(parameter + i))
			return i;
	}

	battery_log(BAT_LOG_CRTI, "NO register value match \r\n");
	/* TODO: ASSERT(0);    // not find the value */
	return 0;
}

static unsigned int bmt_find_closest_level(const unsigned int *pList, unsigned int number,
					 unsigned int level)
{
	unsigned int i;
	unsigned int max_value_in_last_element;

	if (pList[0] < pList[1])
		max_value_in_last_element = KAL_TRUE;
	else
		max_value_in_last_element = KAL_FALSE;

	if (max_value_in_last_element == KAL_TRUE) {
		for (i = (number - 1); i != 0; i--) {	/* max value in the last element */
			if (pList[i] <= level) {
				battery_log(2, "zzf_%d<=%d     i=%d\n", pList[i], level, i);
				return pList[i];
			}
		}

		battery_log(BAT_LOG_CRTI, "Can't find closest level \r\n");
		return pList[0];
		/* return CHARGE_CURRENT_0_00_MA; */
	} else {
		for (i = 0; i < number; i++) {	/* max value in the first element */
			if (pList[i] <= level)
				return pList[i];
		}

		battery_log(BAT_LOG_CRTI, "Can't find closest level \r\n");
		return pList[number - 1];
		/* return CHARGE_CURRENT_0_00_MA; */
	}
}

static unsigned int is_chr_det(void)
{
	unsigned int val = 0;

	val = pmic_get_register_value(PMIC_RGS_CHRDET);
	pr_debug("[is_chr_det] %d\n", val);

	return val;
}

static unsigned int charging_hw_init(void *data)
{
	unsigned int status = STATUS_OK;

	bq25890_config_interface(bq25890_CON2, 0x1, 0x1, 4);	/* disable ico Algorithm -->bear:en */
	bq25890_config_interface(bq25890_CON2, 0x0, 0x1, 3);	/* disable HV DCP for gq25897 */
	bq25890_config_interface(bq25890_CON2, 0x0, 0x1, 2);	/* disbale MaxCharge for gq25897 */
	bq25890_config_interface(bq25890_CON2, 0x0, 0x1, 1);	/* disable DPDM detection */

	bq25890_config_interface(bq25890_CON7, 0x1, 0x3, 4);	/* enable  watch dog 40 secs 0x1 */
	bq25890_config_interface(bq25890_CON7, 0x1, 0x1, 3);	/* enable charging timer safty timer */
	bq25890_config_interface(bq25890_CON7, 0x2, 0x3, 1);	/* charging timer 12h */

	bq25890_config_interface(bq25890_CON2, 0x0, 0x1, 5);	/* boost freq 1.5MHz when OTG_CONFIG=1 */
	bq25890_config_interface(bq25890_CONA, 0x7, 0xF, 4);	/* boost voltagte 4.998V default */
	bq25890_config_interface(bq25890_CONA, 0x3, 0x7, 0);	/* boost current limit 1.3A */

	bq25890_config_interface(bq25890_CON8, 0x0, 0x7, 5);	/* disable ir_comp_resistance */
	bq25890_config_interface(bq25890_CON8, 0x0, 0x7, 2);	/* enable ir_comp_vdamp */
	bq25890_config_interface(bq25890_CON8, 0x3, 0x3, 0);	/* thermal 120 default */

	bq25890_config_interface(bq25890_CON9, 0x0, 0x1, 4);	/* JEITA_VSET: VREG-200mV */
	bq25890_config_interface(bq25890_CON7, 0x1, 0x1, 0);	/* JEITA_ISet : 20% x ICHG */

	bq25890_config_interface(bq25890_CON3, 0x5, 0x7, 1);	/* System min voltage default 3.5V */

	/*PreCC mode */
	bq25890_config_interface(bq25890_CON5, 0x1, 0xF, 4);	/* precharge current default 128mA */
	bq25890_config_interface(bq25890_CON6, 0x1, 0x1, 1);	/* precharge2cc voltage,BATLOWV, 3.0V */
	/*CC mode */
	bq25890_config_interface(bq25890_CON4, 0x08, 0x7F, 0);	/* ICHG (0x08)512mA --> (0x20)2.048mA */
	/*CV mode */
	bq25890_config_interface(bq25890_CON6, 0x20, 0x3F, 2);	/* VREG=CV 4.352V (default 4.208V) */
	bq25890_config_interface(bq25890_CON6, 0x0, 0x1, 0);	/* recharge voltage@VRECHG=CV-100MV */
	bq25890_config_interface(bq25890_CON7, 0x1, 0x1, 7);	/* disable ICHG termination detect */
	bq25890_config_interface(bq25890_CON5, 0x3, 0x7, 0);	/* termianation current default 192mA */
	/*Vbus current limit */
	bq25890_config_interface(bq25890_CON0, 0x3F, 0x3F, 0);	/* input current limit, IINLIM, 3.25A */
	bq25890_config_interface(bq25890_CON0, 0x01, 0x01, 6);	/* enable ilimit Pin */
	 /*DPM*/ bq25890_config_interface(bq25890_CON1, 0x6, 0xF, 0);	/* Vindpm offset  600MV */
	bq25890_config_interface(bq25890_COND, 0x1, 0x1, 7);	/* vindpm vth 0:relative 1:absolute */
	bq25890_config_interface(bq25890_COND, 0x14, 0x7F, 0);	/* absolute VINDPM = 2.6 + code x 0.1 =4.6V */





/*	upmu_set_rg_vcdt_hv_en(0);*/

#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
	if (wireless_charger_gpio_number != 0) {
#ifdef CONFIG_MTK_LEGACY
		mt_set_gpio_mode(wireless_charger_gpio_number, 0);	/* 0:GPIO mode */
		mt_set_gpio_dir(wireless_charger_gpio_number, 0);	/* 0: input, 1: output */
#else
/*K.S. way here*/
#endif
	}
#endif

#ifdef CONFIG_MTK_DUAL_INPUT_CHARGER_SUPPORT
#ifdef CONFIG_MTK_LEGACY
	mt_set_gpio_mode(vin_sel_gpio_number, 0);	/* 0:GPIO mode */
	mt_set_gpio_dir(vin_sel_gpio_number, 0);	/* 0: input, 1: output */
#else
/*K.S. way here*/
#endif
#endif
	return status;
}

static unsigned int charging_dump_register(void *data)
{
	unsigned int status = STATUS_OK;

	pr_debug("charging_dump_register\r\n");
	bq25890_dump_register();

	return status;
}

static unsigned int charging_enable(void *data)
	{
		unsigned int status = STATUS_OK;
		unsigned int enable = *(unsigned int *) (data);
		kal_bool static hiz_flag = 0;
		
		if (KAL_TRUE == enable) {
			bq25890_set_en_hiz(0);
			bq25890_chg_en(enable);
			hiz_flag = 0;

			printk("FMX_CHARGING_ok= %d\n",enable);
			
		}else if(2==enable) {
		
			hiz_flag = 1;
			printk("FMX_CHARGING_runing= %d\n",enable);
			bq25890_chg_en(KAL_FALSE);
			bq25890_set_en_hiz(1);
			
		}else {
			printk("FMX_CHARGING_no= %d\n",enable);

			if(hiz_flag == 1){
				bq25890_set_en_hiz(1);
			}
			bq25890_chg_en(0x0);
		}
		
		return status;
	}


static unsigned int charging_set_cv_voltage(void *data)
{
	unsigned int status;
	unsigned short array_size;
	unsigned int set_cv_voltage;
	unsigned short register_value;
	/*static kal_int16 pre_register_value; */

	array_size = GETARRAYNUM(VBAT_CV_VTH);
	status = STATUS_OK;
	/*pre_register_value = -1; */
	battery_log(BAT_LOG_CRTI, "charging_set_cv_voltage set_cv_voltage=%d\n",
		    *(unsigned int *) data);
	set_cv_voltage = bmt_find_closest_level(VBAT_CV_VTH, array_size, *(unsigned int *) data);
	register_value =
	    charging_parameter_to_value(VBAT_CV_VTH, GETARRAYNUM(VBAT_CV_VTH), set_cv_voltage);
	battery_log(BAT_LOG_CRTI, "charging_set_cv_voltage register_value=0x%x\n", register_value);
	printk("charging_set_cv_voltage=%d\n",register_value);
	bq25890_set_vreg(register_value);

	return status;
}

//phb add for ata test start at 2016.4.19
static unsigned int charging_get_vbus(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int val;

	val = bq25890_get_vbus();
	printk("PHB: charging_get_vbus = %d mV\n", (val * 100+2600));
	
	*(unsigned int *) data = (val * 100+2600);

	return status;
}
//phb add for ata test end at 2016.4.19

static unsigned int charging_get_current(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int array_size;
	/*unsigned short reg_value; */
	unsigned int val;

	/*Get current level */
	array_size = GETARRAYNUM(CS_VTH);
	val = bq25890_get_ichg();
	//printk("[@@Vinton]charging_get_current val = %d\n", val * 50);
	*(unsigned int *) data = 50 * val;
	/* *(unsigned int *)data = charging_value_to_parameter(CS_VTH,array_size,val); */

	return status;
}

extern unsigned int mt_get_bl_brightness(void);

static unsigned int charging_set_current(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int set_chr_current;
	unsigned int array_size;
	unsigned int register_value;
	unsigned int current_value = *(unsigned int *) data;

	array_size = GETARRAYNUM(CS_VTH);
	set_chr_current = bmt_find_closest_level(CS_VTH, array_size, current_value);
	register_value = charging_parameter_to_value(CS_VTH, array_size, set_chr_current);
	/* bq25890_config_interface(bq25890_CON4, register_value, 0x7F, 0); */
	//fanmingxing add for acer battery temperature 2016/4/22


	if(current_value == 650000){
		printk("[@@Vinton] brightness = %d\n",mt_get_bl_brightness());
		bq25890_set_ichg(0x19);
	}
	else if(current_value>250000){
		    //0x38->3.504A,0x30->3.072A_AC_normal_temperature
           printk("fmx_charging_set_current_1=%d,register_0X37\n",current_value/100);
	       bq25890_set_ichg(0x38);
	}else if(current_value>65000){
	        //0x20->2.048A AC_no_normal_temperature
	       printk("fmx_charging_set_current_2=%d,register_0X20\n",current_value/100);
	       bq25890_set_ichg(0x20);
	}else{
	        //0x08->0.512A_USB
	        bq25890_set_ichg(0x08); 
	        printk("fmx_charging_set_current_3=%d,register_0X08\n",current_value/100);
	    
	 }
	printk("fmx_charging_set_current:register_38=3.5A,register_20=2A,register_08=0.5A");
	return status;
}

static unsigned int charging_set_input_current(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int current_value = *(unsigned int *) data;
	unsigned int set_chr_current;
	unsigned int array_size;
	unsigned int register_value;

	/*if(current_value >= CHARGE_CURRENT_2500_00_MA)
	   {
	   register_value = 0x6;
	   }
	   else if(current_value == CHARGE_CURRENT_1000_00_MA)
	   {
	   register_value = 0x4;
	   }
	   else
	   { */
	array_size = GETARRAYNUM(INPUT_CS_VTH);
	set_chr_current = bmt_find_closest_level(INPUT_CS_VTH, array_size, current_value);
	register_value = charging_parameter_to_value(INPUT_CS_VTH, array_size, set_chr_current);
	/*} */

	/* bq25890_config_interface(bq25890_CON0, register_value, 0x3F, 0);//input  current */
	//bq25890_set_iinlim(register_value);
	//fanmingxing add for acer battery temperature 2016/4/22
	if(current_value == 650000){
		bq25890_set_iinlim(0x10); 
	}else if(current_value>250000){
		         //0x20->1.6AC_normal_temperature
	            bq25890_set_iinlim(0x20); 
	            printk("fmx_charging_set_input_current_1= %d,register_0X20\n",current_value/100);
	 }else if(current_value>65000){
	             //0x10->0.8a AC_no_normal_temperature
	            bq25890_set_iinlim(0x10);
	            printk("fmx_charging_set_input_current_2= %d,register_0X10\n",current_value/100);
	 }else{
	              //0x0A->0.5A_USB
	             bq25890_set_iinlim(0x10);
	             printk("fmx_charging_set_input_current_3= %d,register_0X0A\n",current_value/100);
	 	}
	 printk("fmx_charging_set_input_current:register_20=1.6A,register_10=0.8A,register_0A=0.5A");
	return status;
}

static unsigned int charging_get_charging_status(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned char reg_value;

	bq25890_read_interface(bq25890_CONB, &reg_value, 0x3, 3);	/* ICHG to BAT */

	if (reg_value == 0x3)	/* check if chrg done */
		*(unsigned int *) data = KAL_TRUE;
	else
		*(unsigned int *) data = KAL_FALSE;

	return status;
}

static unsigned int charging_reset_watch_dog_timer(void *data)
{
	unsigned int status = STATUS_OK;

	pr_info("charging_reset_watch_dog_timer\r\n");

	bq25890_config_interface(bq25890_CON3, 0x1, 0x1, 6);	/* reset watchdog timer */

	return status;
}

static unsigned int charging_set_hv_threshold(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int set_hv_voltage;
	unsigned int array_size;
	unsigned short register_value;
	unsigned int voltage = *(unsigned int *) (data);

	array_size = GETARRAYNUM(VCDT_HV_VTH);
	set_hv_voltage = bmt_find_closest_level(VCDT_HV_VTH, array_size, voltage);
	register_value = charging_parameter_to_value(VCDT_HV_VTH, array_size, set_hv_voltage);
	pmic_set_register_value(PMIC_RG_VCDT_HV_VTH, register_value);

	return status;
}


static unsigned int charging_get_hv_status(void *data)
{
	unsigned int status = STATUS_OK;

	*(kal_bool *) (data) = pmic_get_register_value(PMIC_RGS_VCDT_HV_DET);
	return status;
}


static unsigned int charging_get_battery_status(void *data)
{
	unsigned int status = STATUS_OK;
#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(kal_bool *) (data) = 0;
	battery_log(BAT_LOG_CRTI, "bat exist for evb\n");
#else
	unsigned int val = 0;

	val = pmic_get_register_value(PMIC_BATON_TDET_EN);
	battery_log(BAT_LOG_FULL, "[charging_get_battery_status] BATON_TDET_EN = %d\n", val);
	if (val) {
		pmic_set_register_value(PMIC_BATON_TDET_EN, 1);
		pmic_set_register_value(PMIC_RG_BATON_EN, 1);
		*(kal_bool *) (data) = pmic_get_register_value(PMIC_RGS_BATON_UNDET);
	} else {
		*(kal_bool *) (data) = KAL_FALSE;
	}
#endif
	return status;
}


static unsigned int charging_get_charger_det_status(void *data)
{
	unsigned int status = STATUS_OK;
	int value;

#if defined(CONFIG_MTK_FPGA)
	*(kal_bool *) (data) = 1;
	battery_log(BAT_LOG_CRTI, "chr exist for fpga\n");
#else
	value = pmic_get_register_value(PMIC_RGS_CHRDET);

	*(kal_bool *) (data) = value;

	//printk("[@@Vinton] charging_get_charger_det_status data = %d\n", value);
#endif
	return status;
}


kal_bool charging_type_detection_done(void)
{
	return charging_type_det_done;
}


static unsigned int charging_get_charger_type(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(CHARGER_TYPE *) (data) = STANDARD_HOST;
#else
#if defined(MTK_WIRELESS_CHARGER_SUPPORT)
	int wireless_state = 0;

	if (wireless_charger_gpio_number != 0) {
#ifdef CONFIG_MTK_LEGACY
		wireless_state = mt_get_gpio_in(wireless_charger_gpio_number);
#else
/*K.S. way here*/
#endif
		if (wireless_state == WIRELESS_CHARGER_EXIST_STATE) {
			*(CHARGER_TYPE *) (data) = WIRELESS_CHARGER;
			pr_notice("WIRELESS_CHARGER!\n");
			return status;
		}
	} else {
		pr_notice("wireless_charger_gpio_number=%d\n", wireless_charger_gpio_number);
	}

	if (g_charger_type != CHARGER_UNKNOWN && g_charger_type != WIRELESS_CHARGER) {
		*(CHARGER_TYPE *) (data) = g_charger_type;
		pr_notice("return %d!\n", g_charger_type);
		return status;
	}
#endif

	if (is_chr_det() == 0) {
		g_charger_type = CHARGER_UNKNOWN;
		*(CHARGER_TYPE *) (data) = CHARGER_UNKNOWN;
		pr_notice("[charging_get_charger_type] return CHARGER_UNKNOWN\n");
		return status;
	}

	charging_type_det_done = KAL_FALSE;
	*(CHARGER_TYPE *) (data) = hw_charging_get_charger_type();
	charging_type_det_done = KAL_TRUE;
	g_charger_type = *(CHARGER_TYPE *) (data);

#endif

	return status;
}

static unsigned int charging_get_is_pcm_timer_trigger(void *data)
{
	unsigned int status = STATUS_OK;

#if 0

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
	*(kal_bool *) (data) = KAL_FALSE;
#else
	if (slp_get_wake_reason() == WR_PCM_TIMER)
		*(kal_bool *) (data) = KAL_TRUE;
	else
		*(kal_bool *) (data) = KAL_FALSE;

	battery_log(BAT_LOG_CRTI, "slp_get_wake_reason=%d\n", slp_get_wake_reason());
#endif

#endif

	return status;
}

static unsigned int charging_set_platform_reset(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	battery_log(BAT_LOG_CRTI, "charging_set_platform_reset\n");
	kernel_restart("battery service reboot system");
#endif

	return status;
}

static unsigned int charging_get_platfrom_boot_mode(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	*(unsigned int *) (data) = get_boot_mode();

	battery_log(BAT_LOG_CRTI, "get_boot_mode=%d\n", get_boot_mode());
#endif

	return status;
}

static unsigned int charging_set_power_off(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(CONFIG_POWER_EXT) || defined(CONFIG_MTK_FPGA)
#else
	battery_log(BAT_LOG_CRTI, "charging_set_power_off\n");
	kernel_power_off();
#endif

	return status;
}

static unsigned int charging_get_power_source(void *data)
{
	unsigned int status = STATUS_OK;

#if 0				/* #if defined(MTK_POWER_EXT_DETECT) */
	if (MT_BOARD_PHONE == mt_get_board_type())
		*(kal_bool *) data = KAL_FALSE;
	else
		*(kal_bool *) data = KAL_TRUE;
#else
	*(kal_bool *) data = KAL_FALSE;
#endif

	return status;
}

static unsigned int charging_get_csdac_full_flag(void *data)
{
	return STATUS_UNSUPPORTED;
}

static unsigned int charging_set_ta_current_pattern(void *data)
{
	kal_bool pumpup;
	unsigned int increase;

	pumpup = *(kal_bool *) (data);
	if (pumpup == KAL_TRUE)
		increase = 1;
	else
		increase = 0;
	/*unsigned int charging_status = KAL_FALSE; */
#if 1
	bq25890_pumpx_up(increase);
	battery_log(BAT_LOG_CRTI, "Pumping up adaptor...");

#else
	if (increase == KAL_TRUE) {
		bq25890_set_ichg(0x0);	/* 64mA */
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() on 1");
		msleep(85);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() off 1");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() on 2");
		msleep(85);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() off 2");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() on 3");
		msleep(281);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() off 3");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() on 4");
		msleep(281);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() off 4");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() on 5");
		msleep(281);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() off 5");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() on 6");
		msleep(485);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_increase() off 6");
		msleep(50);

		battery_log(BAT_LOG_CRTI, "mtk_ta_increase() end\n");

		bq25890_set_ichg(0x8);	/* 512mA */
		msleep(200);
	} else {
		bq25890_set_ichg(0x0);	/* 64mA */
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() on 1");
		msleep(281);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() off 1");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() on 2");
		msleep(281);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() off 2");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() on 3");
		msleep(281);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() off 3");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() on 4");
		msleep(85);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() off 4");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() on 5");
		msleep(85);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() off 5");
		msleep(85);

		bq25890_set_ichg(0x8);	/* 512mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() on 6");
		msleep(485);

		bq25890_set_ichg(0x0);	/* 64mA */
		battery_log(BAT_LOG_FULL, "mtk_ta_decrease() off 6");
		msleep(50);

		battery_log(BAT_LOG_CRTI, "mtk_ta_decrease() end\n");

		bq25890_set_ichg(0x8);	/* 512mA */
	}
#endif
	return STATUS_OK;
}

static unsigned int charging_set_vindpm(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int v = *(unsigned int *) data;

	bq25890_set_VINDPM(v);

	return status;
}

static unsigned int charging_set_vbus_ovp_en(void *data)
{
	unsigned int status = STATUS_OK;
	unsigned int e = *(unsigned int *) data;

	pmic_set_register_value(PMIC_RG_VCDT_HV_EN, e);

	return status;
}



static unsigned int charging_diso_init(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(MTK_DUAL_INPUT_CHARGER_SUPPORT)
	struct device_node *node;
	DISO_ChargerStruct *pDISO_data = (DISO_ChargerStruct *) data;

	int ret;
	/* Initialization DISO Struct */
	pDISO_data->diso_state.cur_otg_state = DISO_OFFLINE;
	pDISO_data->diso_state.cur_vusb_state = DISO_OFFLINE;
	pDISO_data->diso_state.cur_vdc_state = DISO_OFFLINE;

	pDISO_data->diso_state.pre_otg_state = DISO_OFFLINE;
	pDISO_data->diso_state.pre_vusb_state = DISO_OFFLINE;
	pDISO_data->diso_state.pre_vdc_state = DISO_OFFLINE;

	pDISO_data->chr_get_diso_state = KAL_FALSE;

	pDISO_data->hv_voltage = VBUS_MAX_VOLTAGE;

	/* Initial AuxADC IRQ */
	DISO_IRQ.vdc_measure_channel.number = AP_AUXADC_DISO_VDC_CHANNEL;
	DISO_IRQ.vusb_measure_channel.number = AP_AUXADC_DISO_VUSB_CHANNEL;
	DISO_IRQ.vdc_measure_channel.period = AUXADC_CHANNEL_DELAY_PERIOD;
	DISO_IRQ.vusb_measure_channel.period = AUXADC_CHANNEL_DELAY_PERIOD;
	DISO_IRQ.vdc_measure_channel.debounce = AUXADC_CHANNEL_DEBOUNCE;
	DISO_IRQ.vusb_measure_channel.debounce = AUXADC_CHANNEL_DEBOUNCE;

	/* use default threshold voltage, if use high voltage,maybe refine */
	DISO_IRQ.vusb_measure_channel.falling_threshold = VBUS_MIN_VOLTAGE / 1000;
	DISO_IRQ.vdc_measure_channel.falling_threshold = VDC_MIN_VOLTAGE / 1000;
	DISO_IRQ.vusb_measure_channel.rising_threshold = VBUS_MIN_VOLTAGE / 1000;
	DISO_IRQ.vdc_measure_channel.rising_threshold = VDC_MIN_VOLTAGE / 1000;

	node = of_find_compatible_node(NULL, NULL, "mediatek,AUXADC");
	if (!node) {
		battery_log(BAT_LOG_CRTI, "[diso_adc]: of_find_compatible_node failed!!\n");
	} else {
		pDISO_data->irq_line_number = irq_of_parse_and_map(node, 0);
		battery_log(BAT_LOG_CRTI, "[diso_adc]: IRQ Number: 0x%x\n",
			    pDISO_data->irq_line_number);
	}

	mt_irq_set_sens(pDISO_data->irq_line_number, MT_EDGE_SENSITIVE);
	mt_irq_set_polarity(pDISO_data->irq_line_number, MT_POLARITY_LOW);

	ret = request_threaded_irq(pDISO_data->irq_line_number, diso_auxadc_irq_handler,
				   pDISO_data->irq_callback_func, IRQF_ONESHOT, "DISO_ADC_IRQ",
				   NULL);

	if (ret) {
		battery_log(BAT_LOG_CRTI, "[diso_adc]: request_irq failed.\n");
	} else {
		set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
		set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
		battery_log(BAT_LOG_CRTI, "[diso_adc]: diso_init success.\n");
	}

#if defined(MTK_DISCRETE_SWITCH) && defined(MTK_DSC_USE_EINT)
	battery_log(BAT_LOG_CRTI, "[diso_eint]vdc eint irq registitation\n");
	mt_eint_set_hw_debounce(CUST_EINT_VDC_NUM, CUST_EINT_VDC_DEBOUNCE_CN);
	mt_eint_registration(CUST_EINT_VDC_NUM, CUST_EINTF_TRIGGER_LOW, vdc_eint_handler, 0);
	mt_eint_mask(CUST_EINT_VDC_NUM);
#endif
#endif

	return status;
}

static unsigned int charging_get_diso_state(void *data)
{
	unsigned int status = STATUS_OK;

#if defined(MTK_DUAL_INPUT_CHARGER_SUPPORT)
	int diso_state = 0x0;
	DISO_ChargerStruct *pDISO_data = (DISO_ChargerStruct *) data;

	_get_diso_interrupt_state();
	diso_state = g_diso_state;
	battery_log(BAT_LOG_CRTI, "[do_chrdet_int_task] current diso state is %s!\n",
		    DISO_state_s[diso_state]);
	if (((diso_state >> 1) & 0x3) != 0x0) {
		switch (diso_state) {
		case USB_ONLY:
			set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
#ifdef MTK_DISCRETE_SWITCH
#ifdef MTK_DSC_USE_EINT
			mt_eint_unmask(CUST_EINT_VDC_NUM);
#else
			set_vdc_auxadc_irq(DISO_IRQ_ENABLE, 1);
#endif
#endif
			pDISO_data->diso_state.cur_vusb_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_OFFLINE;
			break;
		case DC_ONLY:
			set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_ENABLE, DISO_IRQ_RISING);
			pDISO_data->diso_state.cur_vusb_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_OFFLINE;
			break;
		case DC_WITH_USB:
			set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_ENABLE, DISO_IRQ_FALLING);
			pDISO_data->diso_state.cur_vusb_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_OFFLINE;
			break;
		case DC_WITH_OTG:
			set_vdc_auxadc_irq(DISO_IRQ_DISABLE, 0);
			set_vusb_auxadc_irq(DISO_IRQ_DISABLE, 0);
			pDISO_data->diso_state.cur_vusb_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_ONLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_ONLINE;
			break;
		default:	/* OTG only also can trigger vcdt IRQ */
			pDISO_data->diso_state.cur_vusb_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_vdc_state = DISO_OFFLINE;
			pDISO_data->diso_state.cur_otg_state = DISO_ONLINE;
			battery_log(BAT_LOG_CRTI, " switch load vcdt irq triggerd by OTG Boost!\n");
			break;	/* OTG plugin no need battery sync action */
		}
	}

	if (DISO_ONLINE == pDISO_data->diso_state.cur_vdc_state)
		pDISO_data->hv_voltage = VDC_MAX_VOLTAGE;
	else
		pDISO_data->hv_voltage = VBUS_MAX_VOLTAGE;
#endif

	return status;
}


static unsigned int charging_set_error_state(void *data)
{
	unsigned int status = STATUS_OK;

	charging_error = *(unsigned int *) (data);

	return status;
}

static unsigned int(*const charging_func[CHARGING_CMD_NUMBER]) (void *data) = {
charging_hw_init, charging_dump_register, charging_enable, charging_set_cv_voltage,
	    charging_get_vbus,charging_get_current, charging_set_current, charging_set_input_current,
	    charging_get_charging_status, charging_reset_watch_dog_timer,
	    charging_set_hv_threshold, charging_get_hv_status, charging_get_battery_status,
	    charging_get_charger_det_status, charging_get_charger_type,
	    charging_get_is_pcm_timer_trigger, charging_set_platform_reset,
	    charging_get_platfrom_boot_mode, charging_set_power_off,
	    charging_get_power_source, charging_get_csdac_full_flag,
	    charging_set_ta_current_pattern, charging_diso_init, charging_get_diso_state,
	    charging_set_error_state, charging_set_vindpm, charging_set_vbus_ovp_en};

/*
* FUNCTION
*        Internal_chr_control_handler
*
* DESCRIPTION
*         This function is called to set the charger hw
*
* CALLS
*
* PARAMETERS
*        None
*
* RETURNS
*
*
* GLOBALS AFFECTED
*       None
*/
int chr_control_interface(CHARGING_CTRL_CMD cmd, void *data)
{
	int status;

	if (cmd < CHARGING_CMD_NUMBER)
		status = charging_func[cmd] (data);
	else
		return STATUS_UNSUPPORTED;

	return status;
}
