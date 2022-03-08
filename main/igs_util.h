#ifndef _IGS_UTIL
#define _IGS_UTIL


enum igs_app_cmd_main{
	IGS_CMD_RECV = 0x0b,
	IGS_CMD_RESP = 0x0c,
	IGS_CMD_TYPE__END
};

enum igs_app_cmd_type{
	IGS_CMD_START = 0,
	IGS_QUERY_PAMS,
	IGS_SET_PAMS,
	IGS_QUERY_END
};


typedef struct IgsParam
{ 
	uint16_t   cycle_hours;
	uint16_t   pump_go_times;
	uint16_t   pump_go_minutes;
	uint16_t   pump_delay_minutes;
	uint8_t    run_type;
	
}IgsParam; 


typedef struct _Igs_datas_{
	uint8_t    water_lack;   
	uint8_t    all_onoff; 
	uint8_t    pump_onoff;
	uint8_t    light_mode;
	uint8_t    grow_days[4];
	uint8_t    light_time[4];
}IgsData;

void ixe_product_comd_handler(uint8_t *recv_data,uint32_t recv_len,uint8_t *send_buf,uint8_t *send_len);

void igs_util_task(void* arg);

#endif
