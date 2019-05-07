/*_SCI1.h_*/

#ifndef __RXSCI1
#define __RXSCI1

#include"iodefine.h"
#include<machine.h>
#define SCI		SCI1
#define RDR		SCI.RDR
#define TEND		SCI.SSR.BIT.TEND

#define	BPS(a)		BPS_##a
#define BPS_19200	SCI.SEMR.BIT.ABCS=1;SCI.BRR =104
#define BPS_38400	SCI.SEMR.BIT.ABCS=1;SCI.BRR =51
#define BPS_115200	SCI.SEMR.BIT.ABCS=1;SCI.BRR =17
#define BPS_230400	SCI.SEMR.BIT.ABCS=0;SCI.BRR =5
#define BPS_460800	SCI.SEMR.BIT.ABCS=0;SCI.BRR =2
#define BPS_921600	SCI.SEMR.BIT.ABCS=0;SCI.BRR =1

#define ESC_KEY		(0x1b)//ASCII
#define ENTER_KEY	'\r'
static unsigned char get_buff;
static unsigned char get_buff_m[40];
static unsigned char trans_b[100];
static short sci1_error=0;
static int receve_data_num , string_num;
static char get_buff_2[40];
 
extern void INIT_SCI1(void);
extern void int_scr1_txi1(void);
extern void int_scr1_tei1(void);
extern void int_scr1_rxi1(void);
extern void int_scr1_eri1(void);
static  void sci1_put_start(void);
extern  void sci1_puts(unsigned char* c);
extern  void sci1_put_data(unsigned char* c,int n);
extern  int sci1_gets( volatile unsigned char* p);

#endif