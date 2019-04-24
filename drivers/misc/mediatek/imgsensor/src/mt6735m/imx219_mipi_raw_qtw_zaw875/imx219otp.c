/*
 * Driver for CAM_CAL
 *
 *
 */
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include "kd_camera_hw.h"
#include "cam_cal.h"
#include "cam_cal_define.h"
#include "imx219otp.h"
/* #include <asm/system.h>  // for SMP */
#include <linux/dma-mapping.h>
#ifdef CONFIG_COMPAT
/* 64 bit */
#include <linux/fs.h>
#include <linux/compat.h>
#endif

//#include <mach/hardwareinfo.h>
#include "../../../misc/huaqin/hardwareinfo/hardwareinfo.h"
//#define CAM_CALGETDLT_DEBUG
#define CAM_CAL_DEBUG
#ifdef CAM_CAL_DEBUG
#define CAM_CALDB printk
#else
#define CAM_CALDB(x,...)
#endif


static DEFINE_SPINLOCK(g_CAM_CALLock); // for SMP
#define CAM_CAL_I2C_BUSNUM 0


/*******************************************************************************
*
********************************************************************************/
#define CAM_CAL_ICS_REVISION 1 //seanlin111208
/*******************************************************************************
*
********************************************************************************/
#define CAM_CAL_DRVNAME "IMX219OTP"
#define CAM_CAL_I2C_GROUP_ID 0
/*******************************************************************************
*
********************************************************************************/
extern void kdSetI2CSpeed(u16 i2cSpeed);

//81 is used for V4L driver
static dev_t g_CAM_CALdevno = MKDEV(CAM_CAL_DEV_MAJOR_NUMBER,0);
static struct cdev * g_pCAM_CAL_CharDrv = NULL;


static struct class *CAM_CAL_class = NULL;
static atomic_t g_CAM_CALatomic;


/*******************************************************************************
*
********************************************************************************/
//Address: 2Byte, Data: 1Byte

extern int iReadRegI2C(u8 * a_pSendData,u16 a_sizeSendData,u8 * a_pRecvData,u16 a_sizeRecvData,u16 i2cId);
extern int iWriteRegI2C(u8 * a_pSendData,u16 a_sizeSendData,u16 i2cId);


static u8 read_otp_reg(u32 addr)
{
	u16 get_byte=0;

	char pu_send_cmd[2] = {(char)(addr >> 8), (char)(addr & 0xFF) };
        kdSetI2CSpeed(300);
	iReadRegI2C(pu_send_cmd, 2, (u8*)&get_byte, 1, IMX219OTP_DEVICE_ID);

	return get_byte;
}

static void write_otp_reg(u32 addr, u32 para)
{
	char pu_send_cmd[3] = {(char)(addr >> 8), (char)(addr & 0xFF), (char)(para & 0xFF)};
kdSetI2CSpeed(300);
	iWriteRegI2C(pu_send_cmd, 3, IMX219OTP_DEVICE_ID);
}


static int Check_LSC_SUM(int Group)
{

	u8  reVal;
	int  address=0;// page=1,address=0; txr 
	int retry=3;
	u32 sum_value=0;

	if (Group==0) //Group 1: Page2~Page3  0x3204~0x3243  , page 4:0x3204~0x3232
	{
		write_otp_reg(0x3202,2);//page 2
		write_otp_reg(0x3200, 0x01);
			
		while(retry>0)  
		{
			reVal=read_otp_reg(0x3201);
			if((reVal &0x01)==1)
				break;
			mdelay(10);
			retry--;
		}
			
		if(retry<0)
			{
				CAM_CALDB("imx219 LSC otp page 2 Check_LSC_SUM Set value fail \n");
				return -1;
			}

			for(address=0x3204 ; address<=0x3243 ; address++)
			{
				    sum_value += read_otp_reg(address);
					//sum_value %= 255;
			}

			write_otp_reg(0x3202,3);//page 3
			write_otp_reg(0x3200, 0x01);
			
			while(retry>0)
			{	
				reVal=read_otp_reg(0x3201); 	
				if((reVal &0x01)==1)			
					break;		
				mdelay(10); 	
				retry--;	
			}
			
			if(retry<0)
				{
					CAM_CALDB("imx219 LSC otp page 3 Check_LSC_SUM Set value fail \n");
					return -1;
				}
			for(address=0x3204 ; address<=0x3243 ; address++)
				{
				    sum_value += read_otp_reg(address);
					//sum_value %= 255;
				}

			write_otp_reg(0x3202,4);//page 4
			write_otp_reg(0x3200, 0x01);

			while(retry>0)
			{	
				reVal=read_otp_reg(0x3201); 	
				if((reVal &0x01)==1)			
					break;		
				mdelay(10); 	
				retry--;	
			}
			
			if(retry<0)
				{
					CAM_CALDB("imx219 LSC otp page 4 Check_LSC_SUM Set value fail \n");
					return -1;
				}
	
			for(address=0x3204 ; address<=0x3232 ; address++)
				{
				    sum_value += read_otp_reg(address);
					//sum_value %= 255;
				}

			CAM_CALDB("imx219 LSC otp In Group 1 Check_LSC_SUM sum_value=%0x\n",sum_value);
		}
	else if (Group==2) //Group 2: page 7:0x3222~0x3243 , page 8/page 9 :0x3204~0x3243 ,page 10:0x3204~0x3210)
		{
			write_otp_reg(0x3202,7);//page 7
			write_otp_reg(0x3200, 0x01);
			
			while(retry>0)
			{
				reVal=read_otp_reg(0x3201);
				if((reVal &0x01)==1)
					break;
				mdelay(10);
				retry--;
			}
					
			if(retry<0)
				{
					CAM_CALDB("imx219 LSC otp page 7 Check_LSC_SUM Set value fail \n");
					return -1;
				}
	
			for(address=0x3222 ; address<=0x3243 ; address++)
				{
				    sum_value += read_otp_reg(address);
					//sum_value %= 255;
				}
	
			write_otp_reg(0x3202,8);//page 8
			write_otp_reg(0x3200, 0x01);
			
			while(retry>0)
			{	
				reVal=read_otp_reg(0x3201); 	
				if((reVal &0x01)==1)			
					break;		
				mdelay(10); 	
				retry--;	
			}
			
			if(retry<0)
				{
					CAM_CALDB("imx219 LSC otp page 8 Check_LSC_SUM Set value fail \n");
					return -1;
				}

			for(address=0x3204 ; address<=0x3243 ; address++)
				{
				    sum_value += read_otp_reg(address);
					//sum_value %= 255;
				}

			write_otp_reg(0x3202,9);//page 9
			write_otp_reg(0x3200, 0x01);
			for(address=0x3204 ; address<=0x3243 ; address++)
				{
				    sum_value += read_otp_reg(address);
					//sum_value %= 255;
				}

			write_otp_reg(0x3202,10);//page 10
			write_otp_reg(0x3200, 0x01);
			
			while(retry>0)
			{	
				reVal=read_otp_reg(0x3201); 	
				if((reVal &0x01)==1)			
					break;		
				mdelay(10); 	
				retry--;	
			}
			
			if(retry<0)
				{
					CAM_CALDB("imx219 LSC otp page 10 Check_LSC_SUM Set value fail \n");
					return -1;
				}

			for(address=0x3204 ; address<=0x3210 ; address++)
				{
				    sum_value += read_otp_reg(address);
					//sum_value %= 255;
				}
			
			CAM_CALDB("imx219 LSC otp In Group 2 Check_LSC_SUM sum_value=%0x\n",sum_value);
		
		}
	else
		{
			CAM_CALDB("imx219 No Vaild LSC otp \n");
			return -1;
		}
	
	CAM_CALDB("imx219 LSC otp I Check_LSC_SUM Done!! Before:sum_value=%0x\n",sum_value);
	sum_value %= 255;
	sum_value += 1;
	CAM_CALDB("imx219 LSC otp I Check_LSC_SUM Done!! After:sum_value=%0x\n",sum_value);
	
	return sum_value;	
}

extern struct global_otp_struct hw_info_otp;

static int Get_AWB_OTP(unsigned int addr,int length,u8 * buf)
{
	u8  reVal;
	//int page=1; txr
	int retry=3;
	write_otp_reg(0x3300, 0x08); //ECC  oFF
	write_otp_reg(0x3200, 0x01);

	while(retry>0)
	{
		reVal=read_otp_reg(0x3201);
		if((reVal &0x01)==1)
			break;
		mdelay(10);
		retry--;
	
	}
	if(retry<0)
		return -1;
	write_otp_reg(0x3202, 1);//AWB OTP In Page1
	
	for(retry=0;retry<length;retry++)
	{
		
		buf[retry]=read_otp_reg(addr+retry);
		CAM_CALDB("address=%0x,value=%0x\n",addr+retry,buf[retry]); 		               
	}
	hw_info_otp.vendor_id=  read_otp_reg(addr+1);
	hw_info_otp.year = read_otp_reg(addr+2);
	hw_info_otp.month = read_otp_reg(addr+3);
	hw_info_otp.day = read_otp_reg(addr+4);
	hw_info_otp.lens_id= read_otp_reg(addr+5);
	hw_info_otp.vcm_id= read_otp_reg(addr+6);  
	printk("lili>imx219otp otp hardinfo: year=%0x month=%0x day=%0x vendor_id=%0x vcm_id=%0x\n",hw_info_otp.year,hw_info_otp.month,hw_info_otp.day,hw_info_otp.vendor_id,hw_info_otp.vcm_id);
	
return 0;
}

static int Find_LSC_Group(u8 module_types)
{
	u8  reVal=1;
	//int page=1; txr
	int retry=3;
	
	if(module_types == 1){ 
		printk("LSC This is Sunny module\n");
		write_otp_reg(0x3300, 0x08); //ECC  ON
	}else{
		printk("LSC This is Qtech module\n");
		write_otp_reg(0x3300, 0x00); //ECC  ON	
	}
	write_otp_reg(0x3200, 0x01 );
	
	while(retry>0)
	{
		reVal=read_otp_reg(0x3201);
		if((reVal &0x01)==1)
			break;
		mdelay(10);
		retry--;
	}
	
	if(retry<0)
		{
			CAM_CALDB("imx219 Find_LSC_Group LSC otp Page0 No Data \n");
		return -1;
		}
	write_otp_reg(0x3202,0);// o page read lsc shading group

	reVal=read_otp_reg(0x3243);//raad 0x3243 for lsc  group1 ? or  Group 2?
	CAM_CALDB("imx219 otp 0x3243=%0x\n",reVal);
	
	if((((reVal)&0xF0)==0x40))
		{
			CAM_CALDB("imx219 LSC otp in Group 1 \n");
		return 0;
		}
	else if((((reVal)&0xF0)==0xD0))
		{
			CAM_CALDB("imx219 LSC otp in Group 2 \n");
			return 2;
		}
	else
		{
			CAM_CALDB("imx219 LSC otp No Data \n");
			return -1;
		}	
	
}
int IMX219_LSC_Vaild_Group=-1;

int read_otp_module_id(void)
	{
	   u8 reVal = 0;
	   u8 module_id = 0;
	   //int cnt = 0;
	   write_otp_reg(0x0100,0x00);
	
	   //12MHZ target 25us
	   write_otp_reg(0x3302,0x01);
	   write_otp_reg(0x3303,0x2c);
	
	   write_otp_reg(0x012A,0x0c);
	   write_otp_reg(0x012B,0x00);
	   /*
	   //24MHZ,target 25us
	   write_otp_reg(0x3302,0x02);
	   write_otp_reg(0x3303,0x58);
	
	   write_otp_reg(0x012A,0x18);
	   write_otp_reg(0x012B,0x00);
	   */
	   //set ECC here OFF
	  // if(page == 1||page == 0)
	   write_otp_reg(0x3300,0x08);
	   
	   //write_otp_reg(0x3300,0x00);
	   //set Read mode
	   write_otp_reg(0x3200,0x01);
	   //check read status OK?
	/*
	   while((read_cmos_sensor(0x3201)&0x01)==0)
		{
		cnt++;
	msleep(10);//mDelay(10)?;
	if(cnt==100) 
	break;
		}
		*/
		
	   
	   //set page
	   write_otp_reg(0x3202,0);
	   
	   reVal = read_otp_reg(0x3204);
	   
	if((((reVal)&0xF0)==0x40))
	{ 
	module_id = read_otp_reg(0x3205);
	
	printk("lili >>>> imx219 info OTP read module_id = %x\n",module_id);
	hw_info_otp.otp_valid = 1;
	
	hw_info_otp.vendor_id=  read_otp_reg(0x3205);
	hw_info_otp.year = read_otp_reg(0x3206);
	hw_info_otp.month = read_otp_reg(0x3207);
	hw_info_otp.day = read_otp_reg(0x3208);
	hw_info_otp.lens_id= read_otp_reg(0x3209);
	hw_info_otp.vcm_id= read_otp_reg(0x3210);  
	printk("lili>imx219otp otp hardinfo: year=%0x month=%0x day=%0x vendor_id=%0x vcm_id=%0x\n",hw_info_otp.year,hw_info_otp.month,hw_info_otp.day,hw_info_otp.vendor_id,hw_info_otp.vcm_id);
	
	}
	else if((((reVal)&0xF0)==0xD0))
	{
	module_id = read_otp_reg(0x3211);
	hw_info_otp.otp_valid = 1;
	hw_info_otp.vendor_id=  read_otp_reg(0x3211);
	hw_info_otp.year = read_otp_reg(0x3212);
	hw_info_otp.month = read_otp_reg(0x3213);
	hw_info_otp.day = read_otp_reg(0x3214);
	hw_info_otp.lens_id= read_otp_reg(0x3215);
	hw_info_otp.vcm_id= read_otp_reg(0x3216);  
	printk("lili>imx219otp otp hardinfo: year=%0x month=%0x day=%0x vendor_id=%0x vcm_id=%0x\n",hw_info_otp.year,hw_info_otp.month,hw_info_otp.day,hw_info_otp.vendor_id,hw_info_otp.vcm_id);
	printk("lili >>>> imx219 info OTP read module_id = %x\n",module_id);
	
	}
	else
	{
	printk("Basic Info is Invalid or empty");
	hw_info_otp.otp_valid = 0;
	
	}
	
	 printk("imx219_qtw otp hardinfo: %0x\n",hw_info_otp.otp_valid);   
	 
	   return module_id;
	}

int IMX219_UpdateLSC(u8 update_sensor_otp_awb, u8 update_sensor_otp_lsc,u8 module_types)
{
	// int page=0;
	int Vaild_Group=0,Checksum_Value=0,Group1_Checksum_Value=0,Group2_Checksum_Value=0;
	int retry=3;
	u8  reVal=-1;

#if 1
	if(update_sensor_otp_lsc)
	{	
		Vaild_Group=Find_LSC_Group(module_types);
		
		if(Vaild_Group==-1)
			{
			CAM_CALDB("imx219 LSC otp Find_LSC_Group fail Vaild_Group=%d \n",Vaild_Group);
				return -1;
			}
		
		Checksum_Value=Check_LSC_SUM(Vaild_Group);

		if(Checksum_Value == -1)
			{
				CAM_CALDB("imx219 LSC otp Check_LSC_SUM fail Checksum_Value=%d \n",Checksum_Value);
				CAM_CALDB("imx219 No Vaild LSC otp \n");
				return -1;
			}
		else{
		    if (Vaild_Group==0)
		    	{
					if(module_types == 1){
						printk("LSC This is Sunny module\n");
						write_otp_reg(0x3300, 0x00); //ECC  ON
					}
					write_otp_reg(0x3202,0);
					write_otp_reg(0x3200, 0x01);
					
					while(retry>0)
					{	
						reVal=read_otp_reg(0x3201); 	
						if((reVal &0x01)==1)			
							break;		
						mdelay(10); 	
						retry--;	
					}
					
					if(retry<0)
						{
							CAM_CALDB("imx219 LSC otp Group0 IMX219_UpdateLSC page 0  Set value fail \n");
							return -1;
						}
					
					Group1_Checksum_Value = read_otp_reg(0x3241);
					
					CAM_CALDB("imx219 LSC otp Group 1 0x3241 Group1_Checksum_Value=%d \n",Group1_Checksum_Value);
					if(1)//(Checksum_Value == Group1_Checksum_Value)
						{
							CAM_CALDB("imx219 LSC otp In Group 1 Check_LSC_SUM OK!!! ");
							IMX219_LSC_Vaild_Group = Vaild_Group;
							write_otp_reg(0x0190, 0x01);
							write_otp_reg(0x0191,0x00);
							write_otp_reg(0x0192, Vaild_Group);
							write_otp_reg(0x0193, 0x00);
							write_otp_reg(0x01A4, 0x03);
							
							CAM_CALDB("IMX219_Update Group 1 LSC OK \n");
						}
					else	
						{
							CAM_CALDB("IMX219_UpdateLSC Group1 Checksum fail , UpdataLSC Fail !!!!\n");
						}
		    	}
			else if (Vaild_Group==2)
		    	{
		    		if(module_types == 1){
						printk("LSC This is Sunny module\n");
						write_otp_reg(0x3300, 0x00); //ECC  ON
					}
					write_otp_reg(0x3202,0);
					write_otp_reg(0x3200, 0x01);
					
					while(retry>0)
					{	
						reVal=read_otp_reg(0x3201); 	
						if((reVal &0x01)==1)			
							break;		
						mdelay(10); 	
						retry--;	
					}
					
					if(retry<0)
						{
							CAM_CALDB("imx219 LSC otp Group2 IMX219_UpdateLSC page 0  Set value fail \n");
							return -1;
						}
					
					Group2_Checksum_Value = read_otp_reg(0x3242);
					
					CAM_CALDB("imx219 LSC otp Group 2 0x3242 Group1_Checksum_Value=%d \n",Group1_Checksum_Value);
					
					if(1)//(Checksum_Value == Group2_Checksum_Value)
						{
							CAM_CALDB("imx219 LSC otp In Group 2 Check_LSC_SUM OK!!! ");
							IMX219_LSC_Vaild_Group = Vaild_Group;
							write_otp_reg(0x0190, 0x01);
							write_otp_reg(0x0191,0x00);
							write_otp_reg(0x0192, Vaild_Group);
							write_otp_reg(0x0193, 0x00);
							write_otp_reg(0x01A4, 0x03);
							
							CAM_CALDB("IMX219_Update Group 2 LSC OK \n");
						}
					else	
						{
							CAM_CALDB("IMX219_UpdateLSC Group2 Checksum fail , UpdataLSC Fail !!!!\n");
						}
		    	}
			else
				{
					CAM_CALDB("imx219  No Vaild LSC OTP Data\n");
					CAM_CALDB("IMX219_UpdateLSC Fail !!!!\n");
                                 
				}
		}
	}

#endif
   return 0;
}


#ifdef CONFIG_COMPAT
static int compat_put_cal_info_struct(
	COMPAT_stCAM_CAL_INFO_STRUCT __user *data32,
	stCAM_CAL_INFO_STRUCT __user *data)
{
	u8 *p;
	compat_uint_t i;
	int err = 0;

	err = get_user(i, &data->u4Offset);
	err |= put_user(i, &data32->u4Offset);
	err |= get_user(i, &data->u4Length);
	err |= put_user(i, &data32->u4Length);

	err |= get_user(p, &data->pu1Params);
	err |= put_user(ptr_to_compat(p), &data32->pu1Params);

	return err;
}
static int compat_get_cal_info_struct(
	COMPAT_stCAM_CAL_INFO_STRUCT __user *data32,
	stCAM_CAL_INFO_STRUCT __user *data)
{

	compat_uptr_t p;
	compat_uint_t i;
	int err = 0;

	err = get_user(i, &data32->u4Offset);
	err |= put_user(i, &data->u4Offset);
	err |= get_user(i, &data32->u4Length);
	err |= put_user(i, &data->u4Length);

	err |= get_user(p, &data32->pu1Params);
	err |= put_user(compat_ptr(p), &data->pu1Params);

	return err;
}

static long imx219_Ioctl_Compat(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret;
	int err;

	COMPAT_stCAM_CAL_INFO_STRUCT __user *data32;
	stCAM_CAL_INFO_STRUCT __user *data;

	CAM_CALDB("[CAMERA SENSOR] imx219_Ioctl_Compat,%p %p %x ioc size %d\n",
	filp->f_op , filp->f_op->unlocked_ioctl, cmd, _IOC_SIZE(cmd));

	if (!filp->f_op || !filp->f_op->unlocked_ioctl)
		return -ENOTTY;

	switch (cmd) {

	case COMPAT_CAM_CALIOC_G_READ: {
		data32 = compat_ptr(arg);
		data = compat_alloc_user_space(sizeof(*data));
		if (data == NULL)
			return -EFAULT;

		err = compat_get_cal_info_struct(data32, data);
		if (err)
			return err;

		ret = filp->f_op->unlocked_ioctl(filp, CAM_CALIOC_G_READ, (unsigned long)data);
		err = compat_put_cal_info_struct(data32, data);


		if (err != 0)
			CAM_CALDB("[CAMERA SENSOR] compat_put_acdk_sensor_getinfo_struct failed\n");
		return ret;
	}
	default:
		return -ENOIOCTLCMD;
	}
}


#endif
/*******************************************************************************
*
********************************************************************************/
#define NEW_UNLOCK_IOCTL
#ifndef NEW_UNLOCK_IOCTL
static int CAM_CAL_Ioctl(struct inode * a_pstInode,
struct file * a_pstFile,
unsigned int a_u4Command,
unsigned long a_u4Param)
#else 
static long CAM_CAL_Ioctl(
    struct file *file, 
    unsigned int a_u4Command, 
    unsigned long a_u4Param
)
#endif
{
    int i4RetValue = 0;
    u8 * pBuff = NULL;
    u8 * pWorkingBuff = NULL;
    stCAM_CAL_INFO_STRUCT *ptempbuf;


#ifdef CAM_CALGETDLT_DEBUG
    struct timeval ktv1, ktv2;
    unsigned long TimeIntervalUS;
#endif
    
    if(_IOC_NONE == _IOC_DIR(a_u4Command))
    {
    }
    else
    {
        pBuff = (u8 *)kmalloc(sizeof(stCAM_CAL_INFO_STRUCT),GFP_KERNEL);

        if(NULL == pBuff)
        {
            CAM_CALDB("[S24CAM_CAL] ioctl allocate mem failed\n");
            return -ENOMEM;
        }

        if(_IOC_WRITE & _IOC_DIR(a_u4Command))
        {
            if(copy_from_user((u8 *) pBuff , (u8 *) a_u4Param, sizeof(stCAM_CAL_INFO_STRUCT)))
            {    //get input structure address
                kfree(pBuff);
                CAM_CALDB("[S24CAM_CAL] ioctl copy from user failed\n");
                return -EFAULT;
            }
        }   
//printk("[S24CAM_CAL] LONG TEST SIZE=%ld.\n",sizeof( stCAM_CAL_INFO_STRUCT));
    }

    ptempbuf = (stCAM_CAL_INFO_STRUCT *)pBuff;

	 printk("[S24CAM_CAL] pBuff u4Length=%d.\n",ptempbuf->u4Length);
    pWorkingBuff = (u8*)kmalloc(ptempbuf->u4Length,GFP_KERNEL); 
	
	
	//CAM_CALDB("[S24CAM_CAL]WorkingBuff Start Address is =%x, Buff length=%d, WorkingBuff End Address is=%x\n", pWorkingBuff, ptempbuf->u4Length, pWorkingBuff+ptempbuf->u4Length);
	    
    if(NULL == pWorkingBuff)
    {  
        kfree(pBuff);
        CAM_CALDB("[S24CAM_CAL] ioctl allocate mem failed\n");
        return -ENOMEM;
    }
     //CAM_CALDB("[S24CAM_CAL] init Working buffer address 0x%8x  command is 0x%8x\n", (u32)pWorkingBuff, (u32)a_u4Command);

 
    if(copy_from_user((u8*)pWorkingBuff ,  (u8*)ptempbuf->pu1Params, 2))
    {
        kfree(pBuff);
        kfree(pWorkingBuff);
        CAM_CALDB("[S24CAM_CAL] ioctl copy from user failed\n");
        return -EFAULT;
    } 
    
    switch(a_u4Command)
    {  
        case CAM_CALIOC_S_WRITE:    
            CAM_CALDB("[S24CAM_CAL] Write CMD \n");
#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv1);
#endif            
			CAM_CALDB("[S24CAM_CAL] ptempbuf->u4Offset=%0x.\n",ptempbuf->u4Offset);
			CAM_CALDB("[S24CAM_CAL] ptempbuf->u4Length=%0x.\n",ptempbuf->u4Length);

            //i4RetValue = iWriteData((u16)ptempbuf->u4Offset, ptempbuf->u4Length, pWorkingBuff);
#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv2);
            if(ktv2.tv_sec > ktv1.tv_sec)
            {
                TimeIntervalUS = ktv1.tv_usec + 1000000 - ktv2.tv_usec;
            }
            else
            {
                TimeIntervalUS = ktv2.tv_usec - ktv1.tv_usec;
            }
            printk("Write data %d bytes take %lu us\n",ptempbuf->u4Length, TimeIntervalUS);
#endif            
            break;
        case CAM_CALIOC_G_READ:
            CAM_CALDB("[S24CAM_CAL] Read CMD \n");
#ifdef CAM_CALGETDLT_DEBUG            
            do_gettimeofday(&ktv1);
#endif 
            CAM_CALDB("[CAM_CAL] offset %x \n", ptempbuf->u4Offset);
            CAM_CALDB("[CAM_CAL] length %d \n", ptempbuf->u4Length);
           // CAM_CALDB("[CAM_CAL] Before read Working buffer address 0x%8x,value 0x%8x \n", (u32)pWorkingBuff,(u8 *)pWorkingBuff);

			printk("[CAM_CAL] offset %x \n", ptempbuf->u4Offset);
			printk("[CAM_CAL] length %d \n", ptempbuf->u4Length);
			 //printk("[CAM_CAL] Before read Working buffer address 0x%8x \n", (u32)pWorkingBuff);		

			i4RetValue = Get_AWB_OTP((u16)ptempbuf->u4Offset, ptempbuf->u4Length, pWorkingBuff);
                   // i4RetValue = 222;//
			//i4RetValue = iReadData(0x350a, ptempbuf->u4Length, pWorkingBuff);
			CAM_CALDB("[imx135otp_CAL] After read Working buffer data  0x%4x \n", pWorkingBuff[0]); 		
		

#ifdef CAM_CALGETDLT_DEBUG
            do_gettimeofday(&ktv2);
            if(ktv2.tv_sec > ktv1.tv_sec)
            {
                TimeIntervalUS = ktv1.tv_usec + 1000000 - ktv2.tv_usec;
            }
            else
            {
                TimeIntervalUS = ktv2.tv_usec - ktv1.tv_usec;
            }
            printk("Read data %d bytes take %lu us\n",ptempbuf->u4Length, TimeIntervalUS);
#endif            

            break;
        default :
      	     CAM_CALDB("[S24CAM_CAL] No CMD \n");
            i4RetValue = -EPERM;
        break;
    }

    if(_IOC_READ & _IOC_DIR(a_u4Command))
    {
        //copy data to user space buffer, keep other input paremeter unchange.
        CAM_CALDB("[S24CAM_CAL] to user length %d \n", ptempbuf->u4Length);
       // CAM_CALDB("[S24CAM_CAL] to user  Working buffer address 0x%8x \n", (u32)pWorkingBuff);
        if(copy_to_user((u8 __user *) ptempbuf->pu1Params , (u8 *)pWorkingBuff , ptempbuf->u4Length))//ptempbuf->u4Length
        {
            kfree(pBuff);
            kfree(pWorkingBuff);
            CAM_CALDB("[S24CAM_CAL] ioctl copy to user failed\n");
            return -EFAULT;
        }
    }

    kfree(pBuff);
    kfree(pWorkingBuff);
    return i4RetValue;
}


static u32 g_u4Opened = 0;  //txr
//#define
//Main jobs:
// 1.check for device-specified errors, device not ready.
// 2.Initialize the device if it is opened for the first time.
static int CAM_CAL_Open(struct inode * a_pstInode, struct file * a_pstFile)
{
    CAM_CALDB("[S24CAM_CAL] CAM_CAL_Open\n");
    spin_lock(&g_CAM_CALLock);
    if(g_u4Opened)
    {
        spin_unlock(&g_CAM_CALLock);
		CAM_CALDB("[S24CAM_CAL] Opened, return -EBUSY\n");
        return -EBUSY;
    }
    else
    {
        g_u4Opened = 1;
        atomic_set(&g_CAM_CALatomic,0);
    }
    spin_unlock(&g_CAM_CALLock);


#if 0
    if(TRUE != hwPowerOn(MT65XX_POWER_LDO_VCAMA, VOL_2800, CAM_CAL_DRVNAME))
    {
        CAM_CALDB("[S24CAM_CAL] Fail to enable analog gain\n");
        return -EIO;
    }
#endif
	

    return 0;
}

//Main jobs:
// 1.Deallocate anything that "open" allocated in private_data.
// 2.Shut down the device on last close.
// 3.Only called once on last time.
// Q1 : Try release multiple times.
static int CAM_CAL_Release(struct inode * a_pstInode, struct file * a_pstFile)
{
    spin_lock(&g_CAM_CALLock);

    g_u4Opened = 0;

    atomic_set(&g_CAM_CALatomic,0);

    spin_unlock(&g_CAM_CALLock);

    return 0;
}

static const struct file_operations g_stCAM_CAL_fops =
{
    .owner = THIS_MODULE,
    .open = CAM_CAL_Open,
    .release = CAM_CAL_Release,
    //.ioctl = CAM_CAL_Ioctl
#ifdef CONFIG_COMPAT
	.compat_ioctl = imx219_Ioctl_Compat,
#endif
    .unlocked_ioctl = CAM_CAL_Ioctl
};

#define CAM_CAL_DYNAMIC_ALLOCATE_DEVNO 1
inline static int RegisterCAM_CALCharDrv(void)
{
    struct device* CAM_CAL_device = NULL;
   CAM_CALDB("RegisterCAM_CALCharDrv\n");
#if CAM_CAL_DYNAMIC_ALLOCATE_DEVNO
    if( alloc_chrdev_region(&g_CAM_CALdevno, 0, 1,CAM_CAL_DRVNAME) )
    {
        CAM_CALDB("[S24CAM_CAL] Allocate device no failed\n");

        return -EAGAIN;
    }
#else
    if( register_chrdev_region(  g_CAM_CALdevno , 1 , CAM_CAL_DRVNAME) )
    {
        CAM_CALDB("[S24CAM_CAL] Register device no failed\n");

        return -EAGAIN;
    }
#endif

    //Allocate driver
    g_pCAM_CAL_CharDrv = cdev_alloc();

    if(NULL == g_pCAM_CAL_CharDrv)
    {
        unregister_chrdev_region(g_CAM_CALdevno, 1);

        CAM_CALDB("[S24CAM_CAL] Allocate mem for kobject failed\n");

        return -ENOMEM;
    }

    //Attatch file operation.
    cdev_init(g_pCAM_CAL_CharDrv, &g_stCAM_CAL_fops);

    g_pCAM_CAL_CharDrv->owner = THIS_MODULE;

    //Add to system
    if(cdev_add(g_pCAM_CAL_CharDrv, g_CAM_CALdevno, 1))
    {
        CAM_CALDB("[S24CAM_CAL] Attatch file operation failed\n");

        unregister_chrdev_region(g_CAM_CALdevno, 1);

        return -EAGAIN;
    }

    CAM_CAL_class = class_create(THIS_MODULE, "CAM_CALdrv_IMX219OTP");
    if (IS_ERR(CAM_CAL_class)) {
        int ret = PTR_ERR(CAM_CAL_class);
        CAM_CALDB("Unable to create class, err = %d\n", ret);
        return ret;
    }
    CAM_CAL_device = device_create(CAM_CAL_class, NULL, g_CAM_CALdevno, NULL, CAM_CAL_DRVNAME);

    return 0;
}

inline static void UnregisterCAM_CALCharDrv(void)
{
    //Release char driver
    cdev_del(g_pCAM_CAL_CharDrv);

    unregister_chrdev_region(g_CAM_CALdevno, 1);

    device_destroy(CAM_CAL_class, g_CAM_CALdevno);
    class_destroy(CAM_CAL_class);
}


static int CAM_CAL_probe(struct platform_device *pdev)
{
	
printk("[LONG]CAM_CAL_probe\n");
return 0;
    //return i2c_add_driver(&CAM_CAL_i2c_driver);
}

static int CAM_CAL_remove(struct platform_device *pdev)
{
	
    //i2c_del_driver(&CAM_CAL_i2c_driver);
    return 0;
}

// platform structure
static struct platform_driver g_stCAM_CAL_Driver = {
    .probe		= CAM_CAL_probe,
    .remove	= CAM_CAL_remove,
    .driver		= {
        .name	= CAM_CAL_DRVNAME,
        .owner	= THIS_MODULE,
    }
};


static struct platform_device g_stCAM_CAL_Device = {
    .name = CAM_CAL_DRVNAME,
    .id = 0,
    .dev = {
    }
};

static int __init CAM_CAL_i2C_init(void)
{
CAM_CALDB("CAM_CAL_i2C_init\n");

	
	RegisterCAM_CALCharDrv();
	
    if(platform_driver_register(&g_stCAM_CAL_Driver)){
        CAM_CALDB("failed to register S24CAM_CAL driver\n");
        return -ENODEV;
    }

    if (platform_device_register(&g_stCAM_CAL_Device))
    {
        CAM_CALDB("failed to register S24CAM_CAL driver, 2nd time\n");
        return -ENODEV;
    }	

    return 0;
}

static void __exit CAM_CAL_i2C_exit(void)
{
	
	platform_driver_unregister(&g_stCAM_CAL_Driver);
}

module_init(CAM_CAL_i2C_init);
module_exit(CAM_CAL_i2C_exit);

//MODULE_DESCRIPTION("CAM_CAL driver");
//MODULE_AUTHOR("Sean Lin <Sean.Lin@Mediatek.com>");
//MODULE_LICENSE("GPL");


