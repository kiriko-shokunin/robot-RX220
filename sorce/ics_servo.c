const unsigned int put_position = 0x80;
 int servo_angle_buff[];
 
int set_position(int sci_no,unsigned int id ,int set_position){
	int error=0;
	static char trance_data[3] ;
	switch(sci_no){
	/*case 9:
		trance_data[0] = ( char) (put_position | (id) );
		trance_data[1] = ( char) ((set_position >> 7) & 0x7f);
		trance_data[2] = ( char) (set_position & 0x7f);
		sci9_put(&trance_data[0],3);
		break;
	*/case 6:
		trance_data[0] = (char) (put_position | (id) );
		trance_data[1] = (char) ((set_position >> 7) & 0x7f);
		trance_data[2] = (char) (set_position & 0x7f);
		sci6_put(&trance_data[0],3);
			break;
/* 	   case 5:
		trance_data[0] = (char) (put_position | (id) );
		trance_data[1] = ( char) ((set_position >> 7) & 0x7f);
		trance_data[2] = ( char) (set_position & 0x7f);
		sci5_put(&trance_data[0],3);
		break;
		case 12:
		trance_data[0] = ( char) (put_position | (id) );
		trance_data[1] = ( char) ((set_position >> 7) & 0x7f);
		trance_data[2] = ( char) (set_position & 0x7f);
		sci12_puts(&trance_data[0],3);
		break;*/		
	default:
			error =1;
			break;
	}
	if(error == 0){ }
	return error ;
}
int play_ics_motion(int f){
	return 0;
}
int set_angle(int id,int angle){
	if(id == 3 &&id == 5&&id == 7&&id == 9&&id == 11){
		angle*=-1;	
	}
		servo_angle_buff[id]=(int)(11500-3500)/270*angle+7500;
		return 0;
}
int servo_free(int id){
		servo_angle_buff[id]=0;
		return 0;
}