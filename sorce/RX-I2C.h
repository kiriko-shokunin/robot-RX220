#ifndef __RX_I2C
#define __RX_I2C
#include "iodefine.h"
#include "machine.h"

 char send_data[50];
 char receve_data[50];
 char receve_data_out[50];
 int send_data_num;
 int receve_num;
 int stop_disen_flag;
 int receve_flag,receve_fin_flag;
extern void init_riic0(void);
extern void int_iic0_eei(void);
extern void int_iic0_txi(void);
extern void int_iic0_rxi(void);
extern void int_iic0_tei(void);
extern int iic0_send(char*,int,int);
extern char* iic0_gets();
#endif
 
