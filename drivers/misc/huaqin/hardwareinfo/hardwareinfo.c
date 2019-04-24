/* Huaqin  Inc. (C) 2011. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("HUAQIN SOFTWARE")
 * RECEIVED FROM HUAQIN AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. HUAQIN EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES HUAQIN PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE HUAQIN SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN HUAQIN SOFTWARE. HUAQIN SHALL ALSO NOT BE RESPONSIBLE FOR ANY HUAQIN
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND HUAQIN'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE HUAQIN SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT HUAQIN'S OPTION, TO REVISE OR REPLACE THE HUAQIN SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * HUAQIN FOR SUCH HUAQIN SOFTWARE AT ISSUE.
 *

 */


/*******************************************************************************
* Dependency
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/mtd/mtd.h>
//#include <linux/mtd/nand.h>
#include <asm/io.h>
#include <asm/uaccess.h>

//kaka_12_0224 add
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/core.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/mmc/sdio.h>
#include "hq_emmc_name.h"
//kaka_12_0224 end

#include <linux/proc_fs.h>
#include "../../mediatek/lcm/inc/lcm_drv.h"
#include "../../../input/touchscreen/mediatek/tpd.h"

//WangChao 14_0806 add+
#include "hardwareinfo.h"
//WangChao 14_0806 add-

#define HARDWARE_INFO_VERSION	0x6735

extern LCM_DRIVER  *global_lcm_drv;

//extern flashdev_info devinfo;
extern char *g_main_camera;
extern char *g_sub_camera;
extern int  g_main_camera_capture;
extern int  g_sub_camera_capture;
/****************************************************************************** 
 * Function Configuration
*************************`*****************************************************/


/****************************************************************************** 
 * Debug configuration
******************************************************************************/


//hardware info driver
static ssize_t show_hardware_info(struct device *dev,struct device_attribute *attr, char *buf)
{	
	#if 0
    int ret_value = 1;
    int count = 0;

    //show lcm driver 
    printk("show_lcm_info return lcm name\n");
    count += sprintf(buf, "LCM Name:   ");//12 bytes
	if(lcm_drv->name)	
    	count += sprintf(buf+count, "%s\n", lcm_drv->name); 
    else
    	count += sprintf(buf+count,"no lcd name found\n");


    //show nand drvier name
    printk("show nand device name\n");
    count += sprintf(buf+count, "NAND Name:  ");//12 bytes
    if(devinfo.devciename) 
        count += sprintf(buf+count, "%s\n", devinfo.devciename);
    else
        count += sprintf(buf+count,"no nand name found\n");    


    //show Main Camera drvier name
    printk("show Main Camera device name\n");
    count += sprintf(buf+count, "Main Camera:");//12 bytes
    if(g_main_camera) 
        count += sprintf(buf+count, "%s\n", g_main_camera);
    else
        count += sprintf(buf+count,"pls turn on camera once\n");

    //show Sub Camera drvier name
    printk("show Sub camera name\n");
    count += sprintf(buf+count, "Sub Camera: ");//12 bytes
    if(g_sub_camera) 
        count += sprintf(buf+count, "%s\n", g_sub_camera);
    else
        count += sprintf(buf+count,"pls turn on camera once\n");

	
    //A20 using 6620
    //show WIFI
    printk("show WIFI name\n");
    count += sprintf(buf+count, "WIFI:       ");//12 bytes
    if(1) 
        count += sprintf(buf+count, "MT6620\n");
    else
        count += sprintf(buf+count,"no WIFI name found\n");

    printk("show Bluetooth name\n");
    count += sprintf(buf+count, "Bluetooth:  ");//12 bytes
    if(1) 
        count += sprintf(buf+count, "MT6620\n");
    else
        count += sprintf(buf+count,"no Bluetooth name found\n");

    printk("show GPS name\n");
    count += sprintf(buf+count, "GPS:        ");//12 bytes
    if(1) 
        count += sprintf(buf+count, "MT6620\n");
    else
        count += sprintf(buf+count,"no GPS name found\n");

    printk("show FM name\n");
    count += sprintf(buf+count, "FM:         ");//12 bytes
    if(1) 
        count += sprintf(buf+count, "MT6620\n");
    else
        count += sprintf(buf+count,"no FM name found\n");

    
    ret_value = count;
    #endif

    return 0;
}
static ssize_t store_hardware_info(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    printk("[%s]: Not Support Write Function\n",__func__);	
    return size;
}

static ssize_t show_version(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
	
    ret_value = sprintf(buf, "Version:    :0x%x\n", HARDWARE_INFO_VERSION); 	

    return ret_value;
}
static ssize_t show_lcm(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
	
    ret_value = sprintf(buf, "lcd name    :%s\n", global_lcm_drv->name); 	

    return ret_value;
}

extern struct tpd_device  *tpd;
extern struct tpd_driver_t * g_tpd_drv;
char * g_tpd_module_name=" ";
u16 g_hq_ctp_module_fw_version; 
static ssize_t show_ctp(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
    int count = 0;
	printk("[%s]: \n",__func__);
	if((g_tpd_drv != NULL) &&(g_tpd_drv->tpd_device_name !=NULL))
    count += sprintf(buf + count, "ctp driver  :%s_%s\n", g_tpd_drv->tpd_device_name,g_tpd_module_name); 
	else
		count += sprintf(buf + count, "ctp driver NULL"); 
    if((g_tpd_drv != NULL) &&(g_hq_ctp_module_fw_version != 0))
    count += sprintf(buf + count, "ctp version :%x\n", g_hq_ctp_module_fw_version); 

    ret_value = count;
	printk("[%s]: end \n",__func__);
    return ret_value;
}
static ssize_t show_module(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
    int count = 0;

    //count += sprintf(buf + count, "ctp driver  :%s\n", g_tpd_drv->tpd_device_name); 	
  if(tpd->hq_ctp_module_name)
	  count += sprintf(buf + count, "ctp module  :%s\n", tpd->hq_ctp_module_name);
  else
	  count += sprintf(buf + count , "ctp module  :NULL\n");

    if(tpd->hq_ctp_module_id)
    	count += sprintf(buf + count, "ctp module id  %s\n", tpd->hq_ctp_module_id);

	if(tpd->touch_num)
    	count += sprintf(buf + count, "ctp touch_num  %s\n", tpd->touch_num);
	
	ret_value = count;
	
    return ret_value;
}

// Modify start by lijingwei
// FactoryMode : diff between main cam and sub cam when capture
static int g_maincam_capture_4factory = 0;
static int g_subcam_capture_4factory = 0;

void set_maincam_capture_4factory(int flag) {
	g_maincam_capture_4factory = flag;
}

void set_subcam_capture_4factory(int flag) {
	g_subcam_capture_4factory = flag;
}
// Modify end by lijingwei

static ssize_t show_main_camera(struct device *dev,struct device_attribute *attr, char *buf)
{

    int ret_value = 1;

   
    if(g_main_camera)
    	ret_value = sprintf(buf , "main camera :%s capture:%d\n", g_main_camera,g_maincam_capture_4factory);
    else
        ret_value = sprintf(buf , "main camera :PLS TURN ON CAMERA FIRST\n");
	
	
    return ret_value;
    
}

static ssize_t show_sub_camera(struct device *dev,struct device_attribute *attr, char *buf)
{

    int ret_value = 1;
    int count = 0;

    if(g_sub_camera)
    	ret_value = sprintf(buf , "sub camera  :%s capture:%d\n", g_sub_camera,g_subcam_capture_4factory);
    else
        ret_value = sprintf(buf , "sub camera  :PLS TURN ON CAMERA FIRST\n");

	
    return ret_value;
    
}
//MT6620/6628 names
static ssize_t show_wifi(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;

       ret_value = sprintf(buf, "wifi name   :MT6735\n"); 	

    return ret_value;
}
static ssize_t show_bt(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;

        ret_value = sprintf(buf, "bt name   :MT6735\n"); 	

    return ret_value;
}
static ssize_t show_gps(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;

        ret_value = sprintf(buf, "GPS name   :MT6735\n"); 	

    return ret_value;
}
static ssize_t show_fm(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;

        ret_value = sprintf(buf, "FM name   :MT6735\n"); 	

    return ret_value;
}

char *g_alsps_name  = NULL;
static ssize_t show_alsps(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
    #if defined(CONFIG_CUSTOM_KERNEL_ALSPS)
    if(g_alsps_name)
        ret_value = sprintf(buf, "AlSPS name  :%s\n",g_alsps_name); 	
    else
        ret_value = sprintf(buf, "AlSPS name  :Not found\n"); 
    #else
    ret_value = sprintf(buf, "AlSPS name  :Not support ALSPS\n"); 	
    #endif
    return ret_value;
}

char *g_gsensor_name  = NULL;
static ssize_t show_gsensor(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
    #if defined(CONFIG_CUSTOM_KERNEL_ACCELEROMETER)
    if(g_gsensor_name)
        ret_value = sprintf(buf, "GSensor name:%s\n",g_gsensor_name); 	
    else
        ret_value = sprintf(buf, "GSensor name:Not found\n"); 
    #else
    ret_value = sprintf(buf, "GSensor name:Not support GSensor\n"); 	
    #endif
    return ret_value;
}

char *g_msensor_name  = NULL;
static ssize_t show_msensor(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
    #if defined(CONFIG_CUSTOM_KERNEL_MAGNETOMETER)
    if(g_msensor_name)
        ret_value = sprintf(buf, "MSensor name:%s\n",g_msensor_name); 	
    else
        ret_value = sprintf(buf, "MSensor name:Not found\n"); 
    #else
    ret_value = sprintf(buf, "MSensor name:Not support MSensor\n"); 	
    #endif
    return ret_value;
}

char *g_Gyro_name  = NULL;
static ssize_t show_gyro(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
    #if defined(CONFIG_CUSTOM_KERNEL_GYROSCOPE)
    if(g_Gyro_name)
        ret_value = sprintf(buf, "Gyro  name  :%s\n",g_Gyro_name); 	
    else
        ret_value = sprintf(buf, "Gyro  name  :Not found\n"); 
    #else
    ret_value = sprintf(buf, "Gyro  name  :Not support Gyro\n"); 	
    #endif
    return ret_value;
}

char *g_LaserFocus_name  = NULL;
static ssize_t show_laserFocus(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
    if(g_LaserFocus_name)
        ret_value = sprintf(buf, "LaserFocus:%s\n",g_LaserFocus_name); 	
    else
        ret_value = sprintf(buf, "LaserFocus:NULL\n"); 

    return ret_value;
}

//kaka_12_0224 add
extern struct mmc_host *g_emmc_host;
extern EMMC_NAME_STRUCT emmc_names_table[];
extern int num_of_emmc_records;

static ssize_t show_flash(struct device *dev,struct device_attribute *attr, char *buf)
{
    int ret_value = 1;
	char temp_buf[10];

	int i = 0;
	if(!g_emmc_host) {
		ret_value = sprintf(buf, "EMMC name   : CANT DETECT mmc_host\n");
		goto ERROR_R;
	}
	if(!g_emmc_host->card) {
		ret_value = sprintf(buf, "EMMC name   : CANT DETECT emmc\n");
		goto ERROR_R;
	}

	printk("num_of_emmc_records :%d\n",num_of_emmc_records);
			
	temp_buf[0] = (g_emmc_host->card->raw_cid[0] >> 24) & 0xFF; /* Manufacturer ID */
	temp_buf[1] = (g_emmc_host->card->raw_cid[0] >> 16) & 0xFF; /* Reserved(6)+Card/BGA(2) */
	temp_buf[2] = (g_emmc_host->card->raw_cid[0] >> 8 ) & 0xFF; /* OEM/Application ID */
	temp_buf[3] = (g_emmc_host->card->raw_cid[0] >> 0 ) & 0xFF; /* Product name [0] */
	temp_buf[4] = (g_emmc_host->card->raw_cid[1] >> 24) & 0xFF; /* Product name [1] */
	temp_buf[5] = (g_emmc_host->card->raw_cid[1] >> 16) & 0xFF; /* Product name [2] */
	temp_buf[6] = (g_emmc_host->card->raw_cid[1] >> 8 ) & 0xFF; /* Product name [3] */
	temp_buf[7] = (g_emmc_host->card->raw_cid[1] >> 0 ) & 0xFF; /* Product name [4] */
	temp_buf[8] = (g_emmc_host->card->raw_cid[2] >> 24) & 0xFF; /* Product name [5] */
	temp_buf[9] = (g_emmc_host->card->raw_cid[2] >> 16) & 0xFF; /* Product revision */

	for(i = 0;i < num_of_emmc_records;i++){
		if (memcmp(temp_buf, emmc_names_table[i].ID, 9) == 0){
			ret_value = sprintf(buf, "EMMC name   :%s\n",emmc_names_table[i].emmc_name); 
			break;
		}	
	}

	if(i == num_of_emmc_records){
		ret_value = sprintf(buf, "EMMC name   : CANT DETECT please call emigen\n");
	}

	printk("\n\ng_emmc_host->card->raw_cid[0]:0x%x\n",g_emmc_host->card->raw_cid[0]);
	printk("g_emmc_host->card->raw_cid[1]:0x%x\n",g_emmc_host->card->raw_cid[1]);
	printk("g_emmc_host->card->raw_cid[2]:0x%x\n",g_emmc_host->card->raw_cid[2]);
	printk("g_emmc_host->card->raw_cid[3]:0x%x\n\n",g_emmc_host->card->raw_cid[3]);
	
    
ERROR_R:
    return ret_value;
}

//kaka_12_0224 end

struct global_otp_struct hw_info_otp;
static ssize_t show_otp(struct device *dev,struct device_attribute *attr, char *buf)
{

    int ret_value = 1;

    if(!g_main_camera)
        ret_value = sprintf(buf , "OTP :Please turn on main camera first\n");
    else
    {
        if(hw_info_otp.otp_valid)
            ret_value = sprintf(buf, "OTP:Vendor 0x%0x, Version %d-%d-%d, Lens 0x%0x, Vcm 0x%0x\n",
            	hw_info_otp.vendor_id,hw_info_otp.year,hw_info_otp.month,hw_info_otp.day,hw_info_otp.lens_id,hw_info_otp.vcm_id); 	
        else
            ret_value = sprintf(buf, "OTP:no valid otp\n");
    }

    return ret_value;
    
}

static DEVICE_ATTR(99_version, 0444, show_version, NULL);
static DEVICE_ATTR(00_lcm, 0444, show_lcm, NULL);
static DEVICE_ATTR(01_ctp, 0444, show_ctp, NULL);
static DEVICE_ATTR(02_main_camera, 0444, show_main_camera, NULL);
static DEVICE_ATTR(03_sub_camera, 0444, show_sub_camera, NULL);
static DEVICE_ATTR(04_flash, 0444, show_flash, NULL);
static DEVICE_ATTR(05_gsensor, 0444, show_gsensor, NULL);
static DEVICE_ATTR(06_msensor, 0444, show_msensor, NULL);
static DEVICE_ATTR(07_alsps, 0444, show_alsps, NULL);
static DEVICE_ATTR(08_gyro, 0444, show_gyro, NULL);
static DEVICE_ATTR(09_wifi, 0444, show_wifi, NULL);
static DEVICE_ATTR(10_bt, 0444, show_bt, NULL);
static DEVICE_ATTR(11_gps, 0444, show_gps, NULL);
static DEVICE_ATTR(12_fm, 0444, show_fm, NULL);
static DEVICE_ATTR(13_module, 0444, show_module, NULL);
static DEVICE_ATTR(14_main_otp, 0444, show_otp, NULL);
static DEVICE_ATTR(15_laserFocus, 0444, show_laserFocus, NULL);

static struct attribute *hdinfo_attributes[] = {
	&dev_attr_00_lcm.attr,
	&dev_attr_01_ctp.attr,
	&dev_attr_02_main_camera.attr,
	&dev_attr_03_sub_camera.attr,
	&dev_attr_04_flash.attr,
	&dev_attr_05_gsensor.attr,
	&dev_attr_06_msensor.attr,
	&dev_attr_07_alsps.attr,
	&dev_attr_08_gyro.attr,
	&dev_attr_09_wifi.attr,
	&dev_attr_10_bt.attr,
	&dev_attr_11_gps.attr,
	&dev_attr_12_fm.attr,
	&dev_attr_13_module.attr,
	&dev_attr_14_main_otp.attr,
	&dev_attr_15_laserFocus.attr,
	&dev_attr_99_version.attr,
	NULL
};
static struct attribute_group hdinfo_attribute_group = {
	.attrs = hdinfo_attributes
};

///////////////////////////////////////////////////////////////////////////////////////////
//// platform_driver API 
///////////////////////////////////////////////////////////////////////////////////////////
static int HardwareInfo_driver_probe(struct platform_device *dev)	
{	
    int err = 0;

    printk("** HardwareInfo_driver_probe!! **\n" );
 	err = sysfs_create_group(&(dev->dev.kobj), &hdinfo_attribute_group);
	if(err < 0)
	{ 
        printk("HardwareInfo error\n");
        goto exit_error;
    }
	
    memset(&hw_info_otp, 0, sizeof(hw_info_otp));

exit_error:	
    return err;
}

static int HardwareInfo_driver_remove(struct platform_device *dev)
{
    printk("** HardwareInfo_drvier_remove!! **");

	sysfs_remove_group(&(dev->dev.kobj), &hdinfo_attribute_group);

    return 0;
}

static struct platform_driver HardwareInfo_driver = {
    .probe		= HardwareInfo_driver_probe,
    .remove     = HardwareInfo_driver_remove,
    .driver     = {
        .name = "HardwareInfo",
    },
};

static struct platform_device HardwareInfo_device = {
    .name   = "HardwareInfo",
    .id	    = -1,
};

static int __init HardwareInfo_mod_init(void)
{
    int ret = 0;

    ret = platform_device_register(&HardwareInfo_device);
    if (ret) {
        printk("**HardwareInfo_mod_init  Unable to driver register(%d)\n", ret);
        goto  fail_2;
    }
    
    ret = platform_driver_register(&HardwareInfo_driver);
    if (ret) {
        printk("**HardwareInfo_mod_init  Unable to driver register(%d)\n", ret);
        goto  fail_1;
    }

    goto ok_result;
    
fail_1:
	platform_driver_unregister(&HardwareInfo_driver);
fail_2:
	platform_device_unregister(&HardwareInfo_device);
ok_result:

    return ret;
}


/*****************************************************************************/
static void __exit HardwareInfo_mod_exit(void)
{
	platform_driver_unregister(&HardwareInfo_driver);
	platform_device_unregister(&HardwareInfo_device);
}
/*****************************************************************************/
module_init(HardwareInfo_mod_init);
module_exit(HardwareInfo_mod_exit);
/*****************************************************************************/
MODULE_AUTHOR("Kaka Ni <nigang@huaqin.com>");
MODULE_DESCRIPTION("MT6577 Hareware Info driver");
MODULE_LICENSE("GPL");
