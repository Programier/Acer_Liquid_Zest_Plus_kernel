/*********************************************
HAL API
**********************************************/

void vibr_Enable_HW(void);
void vibr_Disable_HW(void);
void vibr_power_set(void);
struct vibrator_hw *mt_get_cust_vibrator_hw(void);
#if defined(CONFIG_HQ_VIBRATOR_MODE)//fmx add 20160315
void hq_vibr_power_set(int vol_mode);
#endif

