#include"iodefine.h"
#include "machine.h"
/*半二重通信*/
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
	SYSTEM.PRCR.WORD = 0xA502;		// 消費電力低減機能設定レジスタ書き込み許可
	MSTP(SCI6)=0;
	SYSTEM.PRCR.WORD = 0xA500;		// 消費電力低減機能設定レジスタ書き込み禁止
	SCI.SCR.BIT.TE =0;	//送信停止
	SCI.SCR.BIT.RE =0;
	SCI.SMR.BIT.CKS=0; 	// PCLK/1
	SCI.SMR.BIT.STOP =1;	// stop bit
	SCI.SMR.BIT.PE =1;	// 2パリティ有効
	SCI.SMR.BIT.PM =0;	// 偶数パリティ
	SCI.SCR.BIT.TEIE =0;	//トランスミットエンドインタラプトイネーブル
	SCI.SCR.BIT.TIE =1;	
	SCI.SCR.BIT.RIE =1;
	ICU.IER[28].BIT.IEN2		= 1;	//	
	ICU.IER[28].BIT.IEN3		= 1;	//	
	ICU.IER[28].BIT.IEN4		= 1;	//
	ICU.IER[28].BIT.IEN5		= 1;	//
	ICU.IPR[226].BIT.IPR		= 15;	//004
	SCI.SEMR.BIT.ABCS = 1;//1syc8bit
	SCI.SEMR.BIT.NFEN = 1;//ノイズフィルタ
	SCI.BRR =17;		//115200 bps
	
	PMR(RXD_PIN)=1;
	PMR(TXD_PIN)=0;
	SCI.SCR.BIT.TE =0;	//送信開始	
	SCI.SCR.BIT.RE =1;	//受信開始
	SCI.TDR = 0xff;
}
/*
送信バッファエンプティ割り込み
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
/*受信割り込み　intprg.c　で呼び出す
受信バッファ内部からバッファに順次読み出す
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
		SCI.SSR.BIT.PER=0;///パリティエラーフラグ
		er= 2;
	}
	if(SCI.SSR.BIT.FER == 1){
		SCI.SSR.BIT.FER=0;///フレームエラーフラグ
		er= 3;
	}
	if(SCI.SSR.BIT.ORER == 1){
		SCI.SSR.BIT.ORER=0;//オーバランエラーフラグ
		er = 4;
	}

	sci6_error =er;
}
void sci6_putc(unsigned char c){
	PORTB.PMR.BIT.B0 = 1;		//	0:GPIO 1:周辺機能
	PORTB.PMR.BIT.B1 = 1;
	SCI.SCR.BIT.TIE =1;
	SCI.SCR.BIT.TE =1;	//送信開始
	SCI.TDR=c;
	while(TEND == 0);
	SCI.SCR.BIT.TIE =0;
	SCI.SCR.BIT.TEIE=1;
}
/*文字列を送信する関数
・文字列の最後にヌル文字がないとダメ
・引数に文字列の先頭アドレスを入れる
*/
void sci6_puts(unsigned char* c){
	int i;
	if(TEND != 1)return;
	R_STOP;
	for(i=0; *(c+i)!='\0';i++) {
		 sci6_trans_b[i]=(*(c+i));
		 
	}
	MPC.PWPR.BIT.B0WI = 0;		//PFSWEへの書き込み許可	
	MPC.PWPR.BIT.PFSWE = 1;		//PFCレジスタへ書き込み許可
	PORTB.PMR.BIT.B0 = 1;		//	0:GPIO 1:周辺機能
	PORTB.PMR.BIT.B1 = 1;
	MPC.PB0PFS.BIT.PSEL = 11;	// RXD6
	MPC.PB1PFS.BIT.PSEL = 11;	// TXD6
	MPC.PWPR.BIT.PFSWE = 0;		//PFCレジスタへ書き込み禁止	
	MPC.PWPR.BIT.B0WI = 1;		//PFSWEへの書き込み禁止

	SCI.SCR.BIT.TIE =1;
	SCI.SCR.BIT.TE =1;	//送信開始
}
/*//////////////////\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
データを送る関数
文字列以外も送信できるように送信するデータ数を指定できる
c :文字列の先頭アドレス
n :送信するデータ個数
\\\\\\\\\\\\\\\\\\\\///////////////////////////////////*/
void sci6_put(unsigned char* c,int n){
	int i;
	if(TEND != 1)return;
	R_STOP;
	for(i=0; i<=n-1;i++) {
		 sci6_trans_b[i]=(*(c+i));
	}
	sci6_trance_bit_num=n;
	MPC.PWPR.BIT.B0WI = 0;		//PFSWEへの書き込み許可	
	MPC.PWPR.BIT.PFSWE = 1;		//PFCレジスタへ書き込み許可
	PORTB.PMR.BIT.B0 = 1;		//	0:GPIO 1:周辺機能
	PORTB.PMR.BIT.B1 = 1;
	MPC.PB0PFS.BIT.PSEL = 11;	// RXD6
	MPC.PB1PFS.BIT.PSEL = 11;	// TXD6
	MPC.PWPR.BIT.PFSWE = 0;		//PFCレジスタへ書き込み禁止	
	MPC.PWPR.BIT.B0WI = 1;		//PFSWEへの書き込み禁止
	SCI.SCR.BIT.TIE =1;
	SCI.SCR.BIT.TE =1;	//送信開始
}
/*///////////////////////
受信したデータを与えた配列等に代入する関数
///////////////////////*/
int sci6_get(char* p_data){
	int out , n ;
	 for(n=0;n<sizeof(sci6_get_buff_2);n++){
		 *(p_data+n)=sci6_get_buff_2[n];	
	 }
	

	return out;
}
/*///////////////////////
受信したデータの先頭アドレスを返す関数
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