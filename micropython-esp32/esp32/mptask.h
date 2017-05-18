#ifndef MPTASK_H_
#define MPTASK_H_


enum
{
	CTRL_MODE_UART = 0,
	CTRL_MODE_BLE,
};

//extern FATFS sflash_fatfs;
extern void pyexec_pure_cmd_repl(void);
void pyexec_set_ctrl_mode( uint8_t mode );
uint8_t pyexec_get_ctrl_mode( void );

#endif
