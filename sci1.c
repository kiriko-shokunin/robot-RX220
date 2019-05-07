/*sci1.c*/

#include "sci1.h"
void INIT_SCI1(void){
	SYSTEM.PRCR.WORD = 0xA502;		// 消費電力低減機能設定レジスタ書き込み許可
	MSTP(SCI1)=0;
	SYSTEM.PRCR.WORD = 0xA500;		// 消費電力低減機能設定レジスタ書き込み禁止
	SCI1.SCR.BIT.TE =0;			//送信停止
	SCI1.SCR.BIT.RE =0;
	SCI1.SMR.BIT.CKS=0; 			// PCLK/1
	SCI1.SMR.BIT.STOP =0;			// 1stop bit
	SCI1.SCR.BIT.TEIE =0;			//トランスミットエンドインタラプトイネーブル
	SCI1.SCR.BIT.TIE =1;	
	SCI1.SCR.BIT.RIE =1;
	ICU.IER[27].BIT.IEN3		= 1;	//	
	ICU.IER[27].BIT.IEN4		= 1;	//
	ICU.IER[27].BIT.IEN5		= 1;	//
	ICU.IPR[218].BIT.IPR		= 15;	//004
	SCI1.SEMR.BIT.ABCS = 1;			//1syc8bit
	SCI1.SEMR.BIT.NFEN = 1;			//ノイズフィルタ
	BPS(115200);
	MPC.PWPR.BIT.B0WI = 0;			//PFSWEへの書き込み許可	
	MPC.PWPR.BIT.PFSWE = 1;			//PFCレジスタへ書き込み許可
	PORT2.PMR.BIT.B6 = 1;
	PORT3.PMR.BIT.B0 = 1;
	MPC.P30PFS.BIT.PSEL = 10;		// RXD1
	MPC.P26PFS.BIT.PSEL = 10;		// TXD1
	MPC.PWPR.BIT.PFSWE = 0;			//PFCレジスタへ書き込み禁止	
	MPC.PWPR.BIT.B0WI = 1;			//PFSWEへの書き込み禁止
	SCI1.SCR.BIT.TE =0;			//送信開始	
	SCI1.SCR.BIT.RE =1;			//受信開始
	SCI1.TDR = 0xff;
}
void int_scr1_txi1(void){
	static int i=0;
	 setpsw_i();
	if(trans_b[i]=='\0'){
		SCI.SCR.BIT.TIE =0;
		SCI.SCR.BIT.TEIE=1;
		SCI.TDR='\0';
		i=0;
	}else{
		SCI.TDR=trans_b[i++];

	}
	trans_b[(i-1)]=0x00;
}//intprg.cで呼び出す
void int_scr1_tei1(void){
	SCI1.SCR.BIT.TIE =0;
	SCI1.SCR.BIT.TE =0;	//
	SCI1.SCR.BIT.TEIE=0;
	PORT2.PMR.BIT.B6 = 0;
	PORT2.PDR.BIT.B6 = 1;
	PORT2.PODR.BIT.B6=1;
}//intprg.cで呼び出す
void int_scr1_rxi1(void){
	int n;
	setpsw_i();
	get_buff_m[receve_data_num++]=RDR;
	if(receve_data_num >= sizeof(get_buff_m)-1 ){
		receve_data_num=0;
	}
	
	
	switch(RDR){
		case ESC_KEY:
			receve_data_num=0;
			for( n=0; n<=sizeof(get_buff_m); n++ ){
				get_buff_m[n]='\0';
			}for( n=0; n<=sizeof(get_buff_2); n++ ){
				get_buff_2[n]='\0';
			}
			break;
		case ENTER_KEY: 
			string_num = receve_data_num;
			receve_data_num=0;
			for( n=0; n<=sizeof(get_buff_2); n++ ){
				get_buff_2[n]='\0';
			}
			for( n=0; n<string_num-1 ; n++ ){
				get_buff_2[n]=get_buff_m[n];
				get_buff_m[n]='\0';
			}
			get_buff_2[string_num]='\0';
			for(n=0 ;n<sizeof(get_buff_m) ;n++){
				get_buff_m[n]='\0';
			}
			break;
	}
}//intprg.cで呼び出す
void int_scr1_eri1(void){
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
	sci1_error =er;	
}//intprg.cで呼び出す
void sci1_put_start(void){
	PORT2.PMR.BIT.B6 = 1;
	SCI1.SCR.BIT.TIE =1;
	SCI1.SCR.BIT.TE =1;	//送信開始
}
void sci1_puts(unsigned char* c){//文字列送信関数
	int i;
	if(TEND != 1)return;
	for(i=0; (*(c+i)!='\0')&&(sizeof(trans_b)>=i);i++) {
		trans_b[i]=(*(c+i));
	}
	sci1_put_start();
}
void sci1_put_data(unsigned char* c,int n){//データ送信用関数
	int i;
	if(TEND != 1)return;
	for(i=0; i<=n-1;i++){
		trans_b[i]=(*(c+i));
	}
	sci1_put_start();
}
int sci1_gets( volatile unsigned char* p){//受信文字列読み出し関数
	int n ;
	for( n=0 ;  (get_buff_2[n]!='\0')&&(n<=sizeof(get_buff_2)); n++){
		*(p+n) = get_buff_2[n];
		get_buff_2[n]='\0';
	}
	return 0;
}
