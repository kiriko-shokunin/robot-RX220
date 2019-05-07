#include"RX-I2C.h"
#define CLK400
//#define CLK100
void init_riic0(void){
	SYSTEM.PRCR.WORD = 0xA50a;		// 消費電力低減機能設定レジスタ書き込み許可
	MSTP(RIIC0)=0;
	SYSTEM.PRCR.WORD = 0xA500;		// 消費電力低減機能設定レジスタ書き込み禁止	
	RIIC0.ICCR1.BIT.ICE	= 0;
	while( RIIC0.ICCR1.BIT.ICE != 0 );
	RIIC0.ICCR1.BIT.IICRST 	= 1;
	RIIC0.ICCR1.BIT.IICRST 	= 0;
	
#ifdef CLK100
	//100kbps 
	RIIC0.ICMR1.BIT.CKS 		= 2;	//2  CKS = 1
	RIIC0.ICBRH.BYTE		= 10;	//2  ICBRH=8
	RIIC0.ICBRL.BYTE		= 12;	//3  ICBRL=19
#endif

#ifdef CLK400
	//400kbps 	
	RIIC0.ICMR1.BIT.CKS	= 1;
	RIIC0.ICBRH.BYTE		= 9;	
	RIIC0.ICBRL.BYTE		= 9;
#endif	
	RIIC0.ICIER.BIT.ALIE	= 0;
	RIIC0.ICIER.BYTE 		= 0x00;
	IEN(RIIC0,EEI0)			= 1;
	IEN(RIIC0,RXI0)		= 1;
	IEN(RIIC0,TXI0)			= 1;
	IEN(RIIC0,TEI0)			= 1;
	IPR(RIIC0,EEI0)			= 14;
	IPR(RIIC0,RXI0)			= 14;
	IPR(RIIC0,TXI0)			= 14;
	IPR(RIIC0,TEI0)			= 14;	
	RIIC0.ICSER.BYTE = 0x00;
	RIIC0.ICMR2.BIT.TMOL=1;
	RIIC0.ICMR3.BIT.NF=2;
	RIIC0.ICMR3.BIT.ACKWP = 0;        // disable protect for ACKBT 
	PORT1.PMR.BIT.B6 = 1;
	PORT1.PMR.BIT.B7 = 1;
	RIIC0.ICCR1.BIT.ICE =1;
	RIIC0.ICFER.BIT.TMOE =1;
	RIIC0.ICFER.BIT.MALE =0;
	RIIC0.ICFER.BIT.NALE =0;
	RIIC0.ICFER.BIT.SALE =1;
	RIIC0.ICFER.BIT.NACKE =0;
	RIIC0.ICFER.BIT.NFE =1;
	RIIC0.ICFER.BIT.SCLE =1;
	
	MPC.PWPR.BIT.B0WI = 0;		//PFSWEへの書き込み許可	
	MPC.PWPR.BIT.PFSWE = 1;		//PFCレジスタへ書き込み許可
	MPC.P16PFS.BIT.PSEL = 15;	// SCL
	MPC.P17PFS.BIT.PSEL = 15;	// SDA
	MPC.PWPR.BIT.PFSWE = 0;		//PFCレジスタへ書き込み禁止	
	MPC.PWPR.BIT.B0WI = 1;		//PFSWEへの書き込み禁止	
}
void int_iic0_eei(void){
	if( RIIC0.ICSR2.BIT.TMOF==1 ){
		RIIC0.ICCR2.BIT.SP			= 1;	//ストップコンディションの要求をする
	}
	if(RIIC0.ICSR2.BIT.NACKF==1){
		RIIC0.ICCR2.BIT.SP			= 1;	//ストップコンディションの要求をする
	}
	if(RIIC0.ICSR2.BIT.AL==1){
		RIIC0.ICCR2.BIT.SP			= 1;	//ストップコンディションの要求をする
	}
	RIIC0.ICCR1.BIT.IICRST=1;
	RIIC0.ICCR1.BIT.ICE=0;
	RIIC0.ICSR2.BIT.NACKF=0;
	RIIC0.ICSR2.BIT.TMOF=0;
	RIIC0.ICSR2.BIT.AL=0;	
}
void int_iic0_rxi(void){
	static int num;
	int i;
	//RIIC0.ICMR3.BIT.RDRFS=1;
	receve_data[num++]=RIIC0.ICDRR;
	if(num>=50){
		num=0;	
	}
	if(receve_num==num){
			RIIC0.ICMR3.BIT.ACKWP=1;
			RIIC0.ICMR3.BIT.ACKBT=1;	
	}else if(receve_num<num){
			num=0;
			for(i=0;i<=receve_num;i++){
				receve_data_out[i]=receve_data[i+1];
			}
			
			for(i=0;i<sizeof(receve_data);i++){
				receve_data[i]=0;
			}
			receve_fin_flag=1;
			stop_disen_flag=0;
			receve_flag=0;
			RIIC0.ICIER.BIT.TIE 			= 0;	//　送信データエンプティ割込み
			IEN(RIIC0,TXI0)				= 1;
			RIIC0.ICIER.BIT.TEIE 		= 1;	//　送信終了割込み　
			IEN(RIIC0,TEI0)				= 1;
			RIIC0.ICIER.BIT.RIE 			= 0;	//　割込み
			RIIC0.ICSR2.BIT.STOP 		= 0;
			RIIC0.ICCR2.BIT.SP			= 1;	//ストップコンディションの要求をする
			RIIC0.ICMR3.BIT.ACKWP=1;
			RIIC0.ICMR3.BIT.ACKBT=1;	
	}else{
		RIIC0.ICMR3.BIT.ACKWP=1;
		RIIC0.ICMR3.BIT.ACKBT=0;	
	}
			
}
void int_iic0_txi(void){
	static int n=0 ;
	
	if(receve_flag==1){
		RIIC0.ICDRT	=  (send_data[0] | 0x01);//R/W#ビットを１にして突っ込む
		
		
	}else{
		if( n < send_data_num ){
				RIIC0.ICDRT = send_data[n++];
		}else{
				n=0;
				RIIC0.ICIER.BIT.TIE 			= 0;	//　送信データエンプティ割込み
				IEN(RIIC0,TXI0)				= 0;
				RIIC0.ICIER.BIT.TEIE 		= 1;	//　送信終了割込み　
				IEN(RIIC0,TEI0)				= 1;	
		}
	}
}
void int_iic0_tei(void){
		RIIC0.ICSR2.BIT.AL			= 0;	//アービトレーションロストフラグをクリア
		RIIC0.ICCR2.BIT.SP			= 0;	//ストップコンディションの発行を要求しない
		RIIC0.ICSR2.BIT.NACKF		= 0;	//NACKフラグをクリア	
		RIIC0.ICSR2.BIT.STOP 		= 0;
		RIIC0.ICCR2.BIT.SP			= 1;	//ストップコンディションの要求をする
		RIIC0.ICIER.BIT.TEIE 		= 1;	//　送信終了割込み　
		IEN(RIIC0,TEI0)				= 1;
			
		while(RIIC0.ICSR2.BIT.STOP != 1);			
		RIIC0.ICMR2.BIT.TMWE		= 1;	//タイマ嘔吐内部カウンタ書込み許可
		RIIC0.TMOCNTL.BYTE 		= 0;	//タイムアウト
		RIIC0.TMOCNTU.BYTE		= 0;	//
		RIIC0.ICSR2.BIT.TMOF		= 0;	//タイムアウト検出フラグクリア
		RIIC0.ICSR2.BIT.AL			= 0;	//
		RIIC0.ICCR2.BIT.SP			= 0;	//
		RIIC0.ICSR2.BIT.NACKF		= 0;	//
		RIIC0.ICSR2.BIT.STOP		= 0;	//
		RIIC0.ICIER.BIT.TIE 			= 0;	//　送信データエンプティ割込み
		RIIC0.ICIER.BIT.SPIE 		= 1;	//　送信データエンプティ割込み
		IEN(RIIC0,TXI0)				= 0;
		RIIC0.ICIER.BIT.TEIE 		= 0;	//　送信終了割込み　
		IEN(RIIC0,TEI0)				= 0;
		RIIC0.ICIER.BIT.SPIE 		= 0;	//　送信データエンプティ割込み
		if(stop_disen_flag==1){
			RIIC0.ICCR1.BIT.IICRST=1;
			RIIC0.ICCR1.BIT.ICE=1;
			init_riic0();			
			RIIC0.ICIER.BIT.TIE 			= 1;	//　送信データエンプティ割込み
			RIIC0.ICIER.BIT.TEIE 		= 0;	//　送信終了割込み　
			IEN(RIIC0,TXI0)				= 1;
			IEN(RIIC0,TEI0)				= 0;
			IEN(RIIC0,RXI0)			= 1;
			RIIC0.ICIER.BIT.RIE 			= 1;	//　割込み
			RIIC0.ICCR2.BIT.ST = 1;
			receve_flag=1;	
		}
}
int iic0_send(char* string , int data_num,int address){
	int i,r_data;
		 
	if( RIIC0.ICSR2.BIT.TEND==1 || RIIC0.ICCR2.BIT.BBSY==1){
		r_data=-1;
	}else{
		RIIC0.ICCR1.BIT.IICRST=1;
		RIIC0.ICCR1.BIT.ICE=0;
		init_riic0();
		for(i=0;i<data_num;i++){
			send_data[i+1]= *(string + i);
		}
		receve_flag=0;
		stop_disen_flag=0;
		send_data_num	 			= data_num;	
		RIIC0.ICMR2.BIT.TMWE		= 1;
		RIIC0.TMOCNTL.BYTE		= 0x0f;
		RIIC0.TMOCNTU.BYTE		= 0x00;
		RIIC0.ICMR2.BIT.TMWE		= 0;
		RIIC0.ICIER.BIT.TMOIE 		= 1;	//タイムアウト割込み
		RIIC0.ICIER.BIT.NAKIE		= 1;
		RIIC0.ICIER.BIT.TIE 			= 1;	//　送信データエンプティ割込み
		RIIC0.ICIER.BIT.TEIE 		= 0;	//　送信終了割込み　
		IEN(RIIC0,TXI0)				= 1;
		IEN(RIIC0,TEI0)				= 0;
		RIIC0.ICCR2.BIT.ST = 1;	
		send_data[0]				= (address<<1);
	}
	return r_data;
}
char* iic0_gets(){ 
	 receve_fin_flag=0;
	return receve_data_out;
}

int iic0_reqest_send( char* data , int r_num,int addr){
	int out=1;
	int i;
	receve_fin_flag=0;
		
	if( RIIC0.ICSR2.BIT.TEND==1 || RIIC0.ICCR2.BIT.BBSY==1){
		out=-1;
	}else{
		receve_flag=0;
		stop_disen_flag=1;
		///// 内部リセット  ///
		RIIC0.ICCR1.BIT.ICE=0;
		RIIC0.ICCR1.BIT.IICRST=1;
		RIIC0.ICCR1.BIT.ICE=0;
		init_riic0();
		RIIC0.ICSR2.BIT.NACKF=0;
		RIIC0.ICSR2.BIT.TMOF=0;
		RIIC0.ICSR2.BIT.AL=0;	
		///////////////////
		for(i=0;i<sizeof(send_data);i++ ){
			send_data[i]=0x00;
		}
		for(i=0;i<2;i++){
			send_data[i+1]= *(data + i);
		}
		send_data_num				=2;	//送信データは1バイト	
		send_data[send_data_num]	=  ((addr<<1) | 0x01);//R/W#ビットを１にして突っ込む
		for(i=3;i<9;i++){
			send_data[i]= 0x00;
		}
		receve_num=r_num;
		RIIC0.ICMR2.BIT.TMWE		= 1;
		RIIC0.TMOCNTL.BYTE		= 0x0f;
		RIIC0.TMOCNTU.BYTE		= 0x00;
		RIIC0.ICMR2.BIT.TMWE		= 0;
		
		RIIC0.ICIER.BIT.STIE			= 0;
		RIIC0.ICIER.BIT.ALIE 		= 1;	
		RIIC0.ICIER.BIT.TMOIE 		=1 ;	//タイムアウト割込み
		RIIC0.ICIER.BIT.NAKIE		= 1;
		RIIC0.ICIER.BIT.TIE 			= 1;	//　送信データエンプティ割込み
		IEN(RIIC0,TXI0)				= 1;
		RIIC0.ICIER.BIT.TEIE 		= 0;	//　送信終了割込み　
		IEN(RIIC0,TEI0)				= 0;
		RIIC0.ICCR2.BIT.ST = 1;	
		receve_flag=0;
		RIIC0.ICDRT=send_data[0]				= (addr<<1);
	} 
	return out;
}

int iic0_set_addr(int addr, int num){
	int out=1;
	switch(num){
		case 0:
			RIIC0.SARL0.BIT.SVA = addr;
			break;
		case 1:
			RIIC0.SARL1.BIT.SVA = addr;
			break;
		case 2:
			RIIC0.SARL2.BIT.SVA = addr;
			break;
		default:
			out = -1;
			break;
	}
	return out;
}
int  iic0_receve(char* p_data,int n){ 
	int i,out;
	if(receve_fin_flag==1){
		receve_fin_flag=0;
		for(i=0; i<=n ;i++){
			*(p_data+i) = receve_data_out[i] ;	
		}
		for(i=0;i<sizeof(receve_data_out);i++){
			receve_data_out[i]=0;
		}
		out=1;
	}else {
		out=-1;	
	}
	return out;
}
