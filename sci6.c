#include"iodefine.h"
#include "machine.h"
/*����d�ʐM*/
//////////////////////////////////
#define	RXD_PIN	B,0
#define	TXD_PIN	B,1
///////////////////////////////////
#define SCI		SCI6
#define RDR		SCI.RDR
#define TEND		SCI.SSR.BIT.TEND
#define R_STOP		SCI.SCR.BIT.RE =0
#define R_START		SCI.SCR.BIT.RE =1
#define _PMR(x,y)	PORT##x.PMR.BIT.B##y
#define PMR(x)		_PMR(x)
 #define _PDR(x,y)	( PORT##x.PDR.BIT.B##y  )
#define PDR(x)		_PDR( x  )

 unsigned char sci6_get_buff[20];
 char sci6_get_buff_2[20];
 unsigned char  sci6_get_b;
 unsigned char  sci6_trans_buff ;
 unsigned char  sci6_trans_buff_m[];
 unsigned char  sci6_trans_b[100];
 short sci6_error=0;
 int sci6_trance_bit_num,get_buff_num;
int r_data_num;

void INIT_SCI6(void){
	SYSTEM.PRCR.WORD = 0xA502;		// ����d�͒ጸ�@�\�ݒ背�W�X�^�������݋���
	MSTP(SCI6)=0;
	SYSTEM.PRCR.WORD = 0xA500;		// ����d�͒ጸ�@�\�ݒ背�W�X�^�������݋֎~
	SCI.SCR.BIT.TE =0;	//���M��~
	SCI.SCR.BIT.RE =0;
	SCI.SMR.BIT.CKS=0; 	// PCLK/1
	SCI.SMR.BIT.STOP =1;	// stop bit
	SCI.SMR.BIT.PE =1;	// 2�p���e�B�L��
	SCI.SMR.BIT.PM =0;	// �����p���e�B
	SCI.SCR.BIT.TEIE =0;	//�g�����X�~�b�g�G���h�C���^���v�g�C�l�[�u��
	SCI.SCR.BIT.TIE =1;	
	SCI.SCR.BIT.RIE =1;
	ICU.IER[28].BIT.IEN2		= 1;	//	
	ICU.IER[28].BIT.IEN3		= 1;	//	
	ICU.IER[28].BIT.IEN4		= 1;	//
	ICU.IER[28].BIT.IEN5		= 1;	//
	ICU.IPR[226].BIT.IPR		= 15;	//004
	SCI.SEMR.BIT.ABCS = 1;//1syc8bit
	SCI.SEMR.BIT.NFEN = 1;//�m�C�Y�t�B���^
	SCI.BRR =17;		//115200 bps
	
	PMR(RXD_PIN)=1;
	PMR(TXD_PIN)=0;
	SCI.SCR.BIT.TE =0;	//���M�J�n	
	SCI.SCR.BIT.RE =1;	//��M�J�n
	SCI.TDR = 0xff;
}
/*
���M�o�b�t�@�G���v�e�B���荞��
*/
void int_scr6_txi6(void){
	static int i=0;
	setpsw_i();
	if( i>= sci6_trance_bit_num){
		SCI.SCR.BIT.TIE =0;
		SCI.SCR.BIT.TEIE=1;
		//SCI.TDR='\0';
		i=0;
	}else{
		SCI.TDR= sci6_trans_b[i];
		 sci6_trans_b[i++] = '\0';
	}
}
unsigned char reverse_bit8(unsigned char x)
{
	x = ((x & 0x55) << 1) | ((x & 0xAA) >> 1);
	x = ((x & 0x33) << 2) | ((x & 0xCC) >> 2);
	return (x << 4) | (x >> 4);
}
void int_scr6_tei6(void){
	setpsw_i();
	SCI.SCR.BIT.TIE =0;
	SCI.SCR.BIT.TE =0;	
	SCI.SCR.BIT.TEIE=0;
	R_START;
	get_buff_num=0;
}
/*��M���荞�݁@intprg.c�@�ŌĂяo��
��M�o�b�t�@��������o�b�t�@�ɏ����ǂݏo��
*/
void int_scr6_rxi6(void){
	int n;
	setpsw_i();
	 sci6_get_buff[get_buff_num++]=RDR;
	
	 if(get_buff_num>=r_data_num){
		 get_buff_num=0;
		 
		 for(n=0;n<sizeof(sci6_get_buff_2);n++){
			 sci6_get_buff_2[n]=sci6_get_buff[n];
		 }
	 }
}
void int_scr6_eri6(void){
		int er=1;
	if(SCI.SSR.BIT.PER == 1){
		SCI.SSR.BIT.PER=0;///�p���e�B�G���[�t���O
		er= 2;
	}
	if(SCI.SSR.BIT.FER == 1){
		SCI.SSR.BIT.FER=0;///�t���[���G���[�t���O
		er= 3;
	}
	if(SCI.SSR.BIT.ORER == 1){
		SCI.SSR.BIT.ORER=0;//�I�[�o�����G���[�t���O
		er = 4;
	}

	sci6_error =er;
}
void sci6_putc(unsigned char c){
	PORTB.PMR.BIT.B0 = 1;		//	0:GPIO 1:���Ӌ@�\
	PORTB.PMR.BIT.B1 = 1;
	SCI.SCR.BIT.TIE =1;
	SCI.SCR.BIT.TE =1;	//���M�J�n
	SCI.TDR=c;
	while(TEND == 0);
	SCI.SCR.BIT.TIE =0;
	SCI.SCR.BIT.TEIE=1;
}
/*������𑗐M����֐�
�E������̍Ō�Ƀk���������Ȃ��ƃ_��
�E�����ɕ�����̐擪�A�h���X������
*/
void sci6_puts(unsigned char* c){
	int i;
	if(TEND != 1)return;
	R_STOP;
	for(i=0; *(c+i)!='\0';i++) {
		 sci6_trans_b[i]=(*(c+i));
		 
	}
	MPC.PWPR.BIT.B0WI = 0;		//PFSWE�ւ̏������݋���	
	MPC.PWPR.BIT.PFSWE = 1;		//PFC���W�X�^�֏������݋���
	PORTB.PMR.BIT.B0 = 1;		//	0:GPIO 1:���Ӌ@�\
	PORTB.PMR.BIT.B1 = 1;
	MPC.PB0PFS.BIT.PSEL = 11;	// RXD6
	MPC.PB1PFS.BIT.PSEL = 11;	// TXD6
	MPC.PWPR.BIT.PFSWE = 0;		//PFC���W�X�^�֏������݋֎~	
	MPC.PWPR.BIT.B0WI = 1;		//PFSWE�ւ̏������݋֎~

	SCI.SCR.BIT.TIE =1;
	SCI.SCR.BIT.TE =1;	//���M�J�n
}
/*//////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
�f�[�^�𑗂�֐�
������ȊO�����M�ł���悤�ɑ��M����f�[�^�����w��ł���
c :������̐擪�A�h���X
n :���M����f�[�^��
\\\\\\\\\\\\\\\\\\\\///////////////////////////////////*/
void sci6_put(unsigned char* c,int n){
	int i;
	if(TEND != 1)return;
	R_STOP;
	for(i=0; i<=n-1;i++) {
		 sci6_trans_b[i]=(*(c+i));
	}
	sci6_trance_bit_num=n;
	MPC.PWPR.BIT.B0WI = 0;		//PFSWE�ւ̏������݋���	
	MPC.PWPR.BIT.PFSWE = 1;		//PFC���W�X�^�֏������݋���
	PORTB.PMR.BIT.B0 = 1;		//	0:GPIO 1:���Ӌ@�\
	PORTB.PMR.BIT.B1 = 1;
	MPC.PB0PFS.BIT.PSEL = 11;	// RXD6
	MPC.PB1PFS.BIT.PSEL = 11;	// TXD6
	MPC.PWPR.BIT.PFSWE = 0;		//PFC���W�X�^�֏������݋֎~	
	MPC.PWPR.BIT.B0WI = 1;		//PFSWE�ւ̏������݋֎~
	SCI.SCR.BIT.TIE =1;
	SCI.SCR.BIT.TE =1;	//���M�J�n
}
/*///////////////////////
��M�����f�[�^��^�����z�񓙂ɑ������֐�
///////////////////////*/
int sci6_get(char* p_data){
	int out , n ;
	 for(n=0;n<sizeof(sci6_get_buff_2);n++){
		 *(p_data+n)=sci6_get_buff_2[n];	
	 }
	

	return out;
}
/*///////////////////////
��M�����f�[�^�̐擪�A�h���X��Ԃ��֐�
///////////////////////*/
char* sci6_get_p(void){
	return sci6_get_buff_2;
}
int set_rdata_num(int num){
	int out;
	if(num<=0 && num >sizeof(sci6_get_buff)){
		out=1;
		r_data_num=num;
	}else{
		out = -1;
		r_data_num=sizeof(sci6_get_buff);
	}
	return out;
}