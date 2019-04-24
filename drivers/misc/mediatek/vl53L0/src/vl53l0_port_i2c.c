/*
 * vl53l0_port_i2c.c
 *
 *  Created on: July, 2015
 *      Author:  Teresa Tao
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>

#include "stmvl53l0-i2c.h"
#include "stmvl53l0-cci.h"
#include "vl53l0_platform.h"
#include "vl53l0_i2c_platform.h"
#include "stmvl53l0.h"

#define I2C_M_WR			0x00
#define STATUS_OK			0x00
#define STATUS_FAIL			(-1)

#ifdef CONFIG_MTK_I2C_EXTENSION
#define MAX_BUFFER_SIZE	255
static char *I2CDMAWriteBuf;	/*= NULL;*//* unnecessary initialise */
static unsigned int I2CDMAWriteBuf_pa;	/* = NULL; */
static char *I2CDMAReadBuf;	/*= NULL;*//* unnecessary initialise */
static unsigned int I2CDMAReadBuf_pa;	/* = NULL; */

void stmvl53l0_dma_alloct(void)
{
	I2CDMAWriteBuf =
	    (char *)dma_alloc_coherent(NULL, MAX_BUFFER_SIZE,
				       (dma_addr_t *) &I2CDMAWriteBuf_pa,
				       GFP_KERNEL);
	if (I2CDMAWriteBuf == NULL) {
		pr_err("%s : failed to allocate dma buffer\n", __func__);
		return ;
	}
	I2CDMAReadBuf =
	    (char *)dma_alloc_coherent(NULL, MAX_BUFFER_SIZE,
				       (dma_addr_t *) &I2CDMAReadBuf_pa,
				       GFP_KERNEL);
	if (I2CDMAReadBuf == NULL) {
		pr_err("%s : failed to allocate dma buffer\n", __func__);
		return ;
	}
	pr_debug("%s :I2CDMAWriteBuf_pa %d, I2CDMAReadBuf_pa,%d\n", __func__,
		 I2CDMAWriteBuf_pa, I2CDMAReadBuf_pa);

}
void stmvl53l0_dma_release(void)
{
	if (I2CDMAWriteBuf) {
		dma_free_coherent(NULL, MAX_BUFFER_SIZE, I2CDMAWriteBuf,
				  I2CDMAWriteBuf_pa);
		I2CDMAWriteBuf = NULL;
		I2CDMAWriteBuf_pa = 0;
	}

	if (I2CDMAReadBuf) {
		dma_free_coherent(NULL, MAX_BUFFER_SIZE, I2CDMAReadBuf,
				  I2CDMAReadBuf_pa);
		I2CDMAReadBuf = NULL;
		I2CDMAReadBuf_pa = 0;
	}

}
#endif

/** int  VL53L0_I2CWrite(VL53L0_Dev_t dev, void *buff, uint8_t len);
 * @brief       Write data buffer to VL53L0 device via i2c
 * @param dev   The device to write to
 * @param buff  The data buffer
 * @param len   The length of the transaction in byte
 * @return      0 on success
 */
int VL53L0_I2CWrite(VL53L0_DEV dev, uint8_t *buff, uint8_t len)
{


	int err = 0;
	if (dev->bus_type == CCI_BUS) {
#ifdef CAMERA_CCI
		uint16_t index;
		struct cci_data *cci_client_obj =
			(struct cci_data *)dev->client_object;
		struct msm_camera_i2c_client *client = cci_client_obj->client;

		index = buff[0];
		/*pr_err("%s: index: %d len: %d\n", __func__, index, len); */

		if (len == 2) {
			uint8_t data;

			data = buff[1];
			/* for byte access */
			err = client->i2c_func_tbl->i2c_write(client, index,
					data, MSM_CAMERA_I2C_BYTE_DATA);
			if (err < 0) {
				pr_err("%s:%d failed status=%d\n",
					__func__, __LINE__, err);
				return err;
			}
		} else if (len == 3) {
			uint16_t data;

			data = ((uint16_t)buff[1] << 8) | (uint16_t)buff[2];
			err = client->i2c_func_tbl->i2c_write(client, index,
					data, MSM_CAMERA_I2C_WORD_DATA);
			if (err < 0) {
				pr_err("%s:%d failed status=%d\n",
					__func__, __LINE__, err);
				return err;
			}
		} else if (len >= 5) {
			err = client->i2c_func_tbl->i2c_write_seq(client,
					index, &buff[1], (len-1));
			if (err < 0) {
				pr_err("%s:%d failed status=%d\n",
					__func__, __LINE__, err);
				return err;
			}

		}
#endif
#ifndef CAMERA_CCI
	} else {
		struct i2c_data *i2c_client_obj =
					(struct i2c_data *)dev->client_object;
		struct i2c_client *client = (struct i2c_client*)i2c_client_obj->client;
#ifndef CONFIG_MTK_I2C_EXTENSION		
		struct i2c_msg msg[] = {
			{
			 .addr = client->addr,
			 .flags = 0,
			 .len = len,
			 .buf = buff,
			 },
		};

		err = i2c_transfer(client->adapter, msg, 1);
		/* return the actual messages transfer */
		if (err != 1) {
			pr_err("%s: i2c_transfer err:%d, addr:0x%x, reg:0x%x\n",
				__func__, err, client->addr,
				(buff[0] << 8 | buff[1]));
			return STATUS_FAIL;
		}
#else
	if((NULL!=client) && (len>0) && (len<=128))
	{
		// DMA Write
		memcpy(I2CDMAWriteBuf, buff, len);
		client->addr = (client->addr & I2C_MASK_FLAG);
		client->ext_flag = client->ext_flag | I2C_DMA_FLAG;
		if((err=i2c_master_send(client, (unsigned char *)I2CDMAWriteBuf_pa, len))!=len);
		if((err < 0)||(err !=len)){
			vl53l0_dbgmsg("zqq i2c read func writeF ret %d\n",err);
			return err;
		}
	}
#endif
#endif
	}

	return 0;
}


/** int VL53L0_I2CRead(VL53L0_Dev_t dev, void *buff, uint8_t len);
 * @brief       Read data buffer from VL53L0 device via i2c
 * @param dev   The device to read from
 * @param buff  The data buffer to fill
 * @param len   The length of the transaction in byte
 * @return      transaction status
 */
int VL53L0_I2CRead(VL53L0_DEV dev, uint8_t *buff, uint8_t len)
{

	int err = 0;

	if (dev->bus_type == CCI_BUS) {
#ifdef CAMERA_CCI
		uint16_t index;
		struct cci_data *cci_client_obj =
				(struct cci_data *)dev->client_object;
		struct msm_camera_i2c_client *client = cci_client_obj->client;

		index = buff[0];
		/* pr_err("%s: index: %d\n", __func__, index); */
		err = client->i2c_func_tbl->i2c_read_seq(client,
							index, buff, len);
		if (err < 0) {
			pr_err("%s:%d failed status=%d\n",
					__func__, __LINE__, err);
			return err;
		}
#endif
	} else {
#ifndef CAMERA_CCI
		struct i2c_data *i2c_client_obj =
					(struct i2c_data *)dev->client_object;
		struct i2c_client *client = (struct i2c_client*) i2c_client_obj->client;
#ifndef CONFIG_MTK_I2C_EXTENSION
		struct i2c_msg msg[] = {
			{
			 .addr = client->addr,
			 .flags = I2C_M_RD,
			 .len = len,
			 .buf = buff,
			 },
		};

		err = i2c_transfer(client->adapter, &msg[0], 1);
		/* return the actual mesage transfer */
		if (err != 1) {
			pr_err("%s: Read i2c_transfer err:%d, addr:0x%x reg:0x%x\n",
				__func__, err, client->addr,buff[0]);
			return STATUS_FAIL;
		}
		
#else
	// DMA Read
	if((NULL!=client) && (len>0) && (len<=128))

	{
		client->addr = (client->addr & I2C_MASK_FLAG);
		client->ext_flag = client->ext_flag | I2C_DMA_FLAG;
		err = i2c_master_recv(client, (unsigned char *)I2CDMAReadBuf_pa, len);
		//vl53l0_dbgmsg("zqq i2c DMA Read ret %d\n",err);
		memcpy(buff, I2CDMAReadBuf, len);
		if(err != len){
			vl53l0_dbgmsg("zqq i2c DMA Read fail ret %d len %d\n",err,len);
			return err;
		}
	}
#endif
#endif
	}

	return 0;
}

