/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


/****************************************************************************
* This is a demo for bluetooth config wifi connection to ap. You can config ESP32 to connect a softap
* or config ESP32 as a softap to be connected by other device. APP can be downloaded from github 
* android source code: https://github.com/EspressifApp/EspBlufi
* iOS source code: https://github.com/EspressifApp/EspBlufiForiOS
****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "mqtt_client.h"


#include "igs_blufi.h"
#include "igs_main.h"
#include "igs_mqtt.h"


#ifdef   PRODUCT_IGS 
#include "igs_util.h"

extern  IxeMqttParam       xm_params;
extern esp_mqtt_client_handle_t	 xm_client;


uint8_t heart_buf[7] =  {0x55,0xaa,0x00,0x00,0x00,0x00,0xff};
uint8_t pid_buf[7] =    {0x55,0xaa,0x00,0x01,0x00,0x00,0x00};
uint8_t work_mode[7] =  {0x55,0xaa,0x00,0x02,0x00,0x00,0x01};
uint8_t net_config[7] = {0x55,0xaa,0x00,0x05,0x00,0x00,0x04};

uint8_t get_status[7] = {0x55,0xaa,0x00,0x08,0x00,0x00,0x07};

uint8_t all_on[12] =  {0x55,0xaa,0x00,0x06,0x00,0x05,0x01,0x01,0x00,0x01,0x01,0x0e};
uint8_t all_off[12] = {0x55,0xaa,0x00,0x06,0x00,0x05,0x01,0x01,0x00,0x01,0x00,0x0d};

uint8_t pump_on[12] =  {0x55,0xaa,0x00,0x06,0x00,0x05,0x02,0x01,0x00,0x01,0x01,0x0f};
uint8_t pump_off[12] = {0x55,0xaa,0x00,0x06,0x00,0x05,0x02,0x01,0x00,0x01,0x00,0x0e};

uint8_t light_mode1[12] =  {0x55,0xaa,0x00,0x06,0x00,0x05,0x03,0x01,0x00,0x01,0x01,0x10};
uint8_t light_mode0[12] = {0x55,0xaa,0x00,0x06,0x00,0x05,0x03,0x01,0x00,0x01,0x00,0x0f};

uint8_t grow_day[15] =  {0x55,0xaa,0x00,0x06,0x00,0x08,0x05,0x02,0x00,0x04,0x00,0x00,0x01,0x00,0x19};
uint8_t grow_day0[15] = {0x55,0xaa,0x00,0x06,0x00,0x08,0x05,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x18};

uint8_t light_time[15] =   {0x55,0xaa,0x00,0x06,0x00,0x08,0x65,0x00,0x00,0x04,0x08,0x00,0x10,0x00,0x8e};

uint8_t sync_time[15] = {0x55,0xaa,0x00,0x1c,0x00,0x08,0x01,0x10,0x04,0x13,0x05,0x06,0x07,0x02,0x5f};

uint8_t sync_net[8] = {0x55,0xaa,0x00,0x03,0x00,0x01,0x04,0x07};



QueueHandle_t blufi_uart_queue = NULL;
IgsData     igs_data;
extern  volatile IxeData		  x_datas;

static uint8_t igs_reset_factory(void)
{
   nvs_handle_t    my_handle;
   esp_err_t       err;
  
   err = nvs_open("storage", NVS_READWRITE, &my_handle);
   if (err != ESP_OK) return 2;
   nvs_erase_key(my_handle,"ixe_params");
   x_datas.restart = 0x01; 
   esp_wifi_stop();
   esp_wifi_restore();
    
   return 1;
}


uint8_t igs_mqtt_update_datas(void)
{
  char	    mqtt_pub_buf[64] = {0};
  uint8_t   temp_buf[20] = {0};
  uint8_t   temp_len = 17;
  int i;
  
  temp_buf[0] = FRAME_TYPE_DEFAULT;
  temp_buf[1] = FRAME_CONTROL_DEFAULT;
  temp_buf[2] = FRAME_SNUMBER_DEFAULT;
  temp_buf[3] = temp_len;
  temp_buf[4] = IGS_CMD_RESP;
  temp_buf[5] = IGS_QUERY_PAMS;
  temp_buf[6] = x_datas.status;
  temp_buf[7] = igs_data.water_lack;
  temp_buf[8] = igs_data.all_onoff;
  temp_buf[9] = igs_data.pump_onoff;
  temp_buf[10] = igs_data.light_mode;
  temp_buf[11] = igs_data.grow_days[2];
  temp_buf[12] = igs_data.grow_days[3];
  temp_buf[13] = igs_data.light_time[0];
  temp_buf[14] = igs_data.light_time[1];
  temp_buf[15] = igs_data.light_time[2];
  temp_buf[16] = igs_data.light_time[3];

  for(i=0;i<temp_len;i++)
  {    
    sprintf(&mqtt_pub_buf[i*2],"%02x",temp_buf[i]);   
  }   
   mqtt_pub_buf[temp_len*2] = '\0';

  esp_mqtt_client_publish(xm_client, xm_params.mqtt_pub_topic, mqtt_pub_buf, 0, 0, 0);
  printf("igs_mqtt_update_datas len = %d: %s\n",temp_len*2,mqtt_pub_buf);
  return 1;  
}

uint8_t igs_sync_mcu_time()
{
   struct tm timeinfo = { 0 };
   time_t now = 0;
   int i;
   int ret = 0;
   
   time(&now);
   localtime_r(&now, &timeinfo);
   if(timeinfo.tm_year < (2021 - 1900))
   	 sync_time[6] = 0x00;
   else
   	 sync_time[6] = 0x01;
	 
    sync_time[7] = timeinfo.tm_year - 100;
	sync_time[8] = timeinfo.tm_mon+1;
	sync_time[9] = timeinfo.tm_mday;
	sync_time[10] = timeinfo.tm_hour;
	sync_time[11] = timeinfo.tm_min;
	sync_time[12] = timeinfo.tm_sec;
	sync_time[13] = timeinfo.tm_wday;

	//printf("app set sync time: ");
	 for(i=0;i<14;i++)
	 {
	    //printf(" %02x",sync_time[i]);
		ret += sync_time[i];
	 }
	 sync_time[14] = ret%256;
	// printf(" %02x\n",sync_time[14]);
	 ret = uart_write_bytes(UART_NUM_2,(char*)&sync_time[0],sizeof(sync_time));
	 if(ret != sizeof(sync_time))
	 	return 2;
	 
	return 1;
}

static void igs_app_query_resp(uint8_t *send_buf,uint8_t *send_len)
{
  send_buf[0] = IGS_CMD_RESP;
  send_buf[1] = IGS_QUERY_PAMS;
  send_buf[2] = x_datas.status;
  send_buf[3] = igs_data.water_lack;
  send_buf[4] = igs_data.all_onoff;
  send_buf[5] = igs_data.pump_onoff;
  send_buf[6] = igs_data.light_mode; 
  send_buf[7] = igs_data.grow_days[2];
  send_buf[8] = igs_data.grow_days[3];
  send_buf[9] = igs_data.light_time[0];
  send_buf[10] = igs_data.light_time[1];
  send_buf[11] = igs_data.light_time[2];
  send_buf[12] = igs_data.light_time[3];
  
  *send_len = 13;
  
  return;
}

static uint8_t igs_app_set_params(uint8_t *data)
{
  int ret;
  int i;

  
  if(igs_data.all_onoff != data[0])
   {
	  printf("app set all onoff = %d\n",data[0]);
	  if(data[0])
		ret = uart_write_bytes(UART_NUM_2,(char*)&all_on[0],sizeof(all_on));
	  else
		ret = uart_write_bytes(UART_NUM_2,(char*)&all_off[0],sizeof(all_off));
	  if(ret != sizeof(all_on))
		 return 2;
	  else
		 return 1;
   }
  
  if(igs_data.pump_onoff != data[1])
  {
     printf("app set pump onoff = %d\n",data[1]);
     if(data[1])
	   ret = uart_write_bytes(UART_NUM_2,(char*)&pump_on[0],sizeof(pump_on));
	 else
	   ret = uart_write_bytes(UART_NUM_2,(char*)&pump_off[0],sizeof(pump_off));
	 if(ret != sizeof(pump_on))
	 	return 2;
	 else
	 	return 1;
  }

  if(igs_data.light_mode!= data[2])
  {
     printf("app set light mode = %d\n",data[2]);
     if(data[2])
	   ret = uart_write_bytes(UART_NUM_2,(char*)&light_mode1[0],sizeof(light_mode1));
	 else
	   ret = uart_write_bytes(UART_NUM_2,(char*)&light_mode0[0],sizeof(light_mode0));
	 if(ret != sizeof(light_mode1))
	 	return 2;
	 else
	 	return 1;
  }
 
  if(igs_data.grow_days[2]!= data[3] || igs_data.grow_days[3]!= data[4]) 
  {
     printf("app set grow day = %d\n",data[3]*256+data[4]);
     grow_day[12] = data[3];
	 grow_day[13] = data[4];
	 ret = 0;
	 printf("app set grow day: ");
	 for(i=0;i<14;i++)
	 {
	    printf(" %02x",grow_day[i]);
		ret += grow_day[i];
	 }
	 grow_day[14] = ret%256;
	 printf(" %02x\n",grow_day[14]);
	 ret = uart_write_bytes(UART_NUM_2,(char*)&grow_day[0],sizeof(grow_day));
	 if(ret != sizeof(grow_day))
	 	return 2;
	 else
	 	return 1;
  }

  if(igs_data.light_time[0]!= data[5] || igs_data.light_time[1]!= data[6] || igs_data.light_time[2]!= data[7] || igs_data.light_time[3]!= data[8]) 
  {
     printf("app set light time = %d:%d ~ %d:%d\n",data[5],data[6],data[7],data[8]);
     light_time[10] = data[5];
	 light_time[11] = data[6];
	 light_time[12] = data[7];
	 light_time[13] = data[8];
	 ret = 0;
	 printf("app set light time: ");
	 for(i=0;i<14;i++)
	 {
       printf(" %02x",light_time[i]);
	   ret += light_time[i];
	 }
	 light_time[14] = ret%256;
	 printf(" %02x\n",light_time[14]);
	 ret = uart_write_bytes(UART_NUM_2,(char*)&light_time[0],sizeof(light_time));
	 if(ret != sizeof(light_time))
	 	return 2;
  }
  
  return 1;
}

void igs_app_set_resp(uint8_t set_type,uint8_t set_result,uint8_t *send_buf,uint8_t *send_len)
{
  send_buf[0] = IGS_CMD_RESP;
  send_buf[1] = set_type;
  send_buf[2] = set_result;
  *send_len = 3;
  BLUFI_INFO("[%s:%d]set_type:%d ,result = %d\n",__FILE__,__LINE__,set_type,set_result);
  
  return;
}


void ixe_product_comd_handler(uint8_t *recv_data,uint32_t recv_len,uint8_t *send_buf,uint8_t *send_len)
{
  uint8_t  main_type;
  uint8_t  sub_type;
  uint8_t  *data;
  uint8_t  ret = 0;

  
  if(NULL == recv_data || recv_len<2)
  {
    return;
  }
  main_type = recv_data[0];
  sub_type  = recv_data[1];
  data = &recv_data[2];
  
  //query command
  if(main_type == IGS_CMD_RECV)
  {
    switch(sub_type)
    {
      case IGS_QUERY_PAMS:
	  	     igs_app_query_resp(send_buf,send_len);
	  	     break;
	  case IGS_SET_PAMS:
	  	     ret = igs_app_set_params(data);
	  	     igs_app_set_resp(sub_type,ret,send_buf,send_len);
	  	     break;
	  default:
	  	     break;
	}
  }else{
      igs_app_set_resp(sub_type,2,send_buf,send_len);
  }
  return;
}


void igs_data_change_handle(uint8_t *recv_buf,uint8_t length)
{
  uint8_t  data_type;

  data_type = recv_buf[6];
  switch(data_type)
  {
     case 0x01:
	 	       igs_data.all_onoff = recv_buf[10];
	 	       BLUFI_INFO("In %s, all_on = %d\n",__func__,igs_data.all_onoff);
			   break;
	 case 0x02:
	 	       igs_data.pump_onoff = recv_buf[10];
	 	       BLUFI_INFO("In %s, pump_on = %d\n",__func__,igs_data.pump_onoff);
			   break;
	 case 0x03:
	 	       igs_data.light_mode = recv_buf[10];
	 	       BLUFI_INFO("In %s, light_mode = %d\n",__func__,igs_data.light_mode);
			   break;
	 case 0x05:
	 	       igs_data.grow_days[0] = recv_buf[10];
			   igs_data.grow_days[1] = recv_buf[11];
			   igs_data.grow_days[2] = recv_buf[12];
			   igs_data.grow_days[3] = recv_buf[13];
	 	       BLUFI_INFO("In %s, grow_days = %d\n",__func__,igs_data.grow_days[2]*256 + igs_data.grow_days[3]);
			   break;
	 case 0x0a:
	 	       igs_data.water_lack = recv_buf[10];
	 	       BLUFI_INFO("In %s, water_lack = %d\n",__func__,igs_data.water_lack);
			   break;
	 case 0x65:
	 	       igs_data.light_time[0] = recv_buf[10];
			   igs_data.light_time[1] = recv_buf[11];
			   igs_data.light_time[2] = recv_buf[12];
			   igs_data.light_time[3] = recv_buf[13];
	 	       BLUFI_INFO("In %s, light on at %d:%d, off at %d:%d\n",__func__,recv_buf[10],recv_buf[11],recv_buf[12],recv_buf[13]);
			   break;
	   default:
	   	       break;
  }
  igs_mqtt_update_datas();
}

void igs_uart_recv_handle(uint8_t *recv_buf,uint8_t length)
{
  int i;
  uint8_t type;

  if(recv_buf[0] != 0x55 || recv_buf[1] != 0xaa || recv_buf[2] != 0x03)
  {
    BLUFI_INFO("In %s, esp recv wrong cmd from igs\n",__func__);
	return;
  }

  type = recv_buf[3];
  if(type != 0x1c && type != 0x00)
  {
	printf( "%s recv at cmd len = %d: ", __func__,length);
	for(i=0;i<length;i++)
	  printf("%02x ",recv_buf[i]);
	printf("\n");
  }
  switch(type)
  {
     case 0x00:
	 	       //BLUFI_INFO("In %s, esp recv heart buf from igs\n",__func__);
			   break;
	 case 0x01:
	 	       BLUFI_INFO("In %s, esp recv pid igs\n",__func__);
			   break;
	 case 0x02:
	 	       BLUFI_INFO("In %s, esp recv work mode from igs\n",__func__);
			   break;
	 case 0x05:
	 	       BLUFI_INFO("In %s, esp recv net config from igs\n",__func__);
			   igs_reset_factory();
			   break;
	 case 0x07:
	 	       //BLUFI_INFO("In %s, esp recv data change from igs\n",__func__);
			   igs_data_change_handle(recv_buf,length);
			   break;
	 case 0x1c:
	 	       //BLUFI_INFO("In %s, esp recv get time from igs\n",__func__);
			   igs_sync_mcu_time();
			   break;
	   default:
	   	       break;

  }
  
}

void blufi_uart_task(void *pvParameters)
{
    uart_event_t event;
    int i,j;
	uint8_t  recv_buf[64] = {0};
	
    for (;;) {
        //Waiting for UART event.
        if (xQueueReceive(blufi_uart_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            switch (event.type) {
            //Event of UART receving data
            case UART_DATA:
				if(event.size)
                {
                    uint8_t *temp = NULL;
					temp = (uint8_t *)malloc(sizeof(uint8_t)*event.size);
                    if(temp == NULL){
                        BLUFI_INFO("%s malloc.1 failed\n", __func__);
                        break;
                    }
                    memset(temp,0x0,event.size);
                    uart_read_bytes(UART_NUM_2,temp,event.size,portMAX_DELAY);
                    //igs cmd: 0x55 0xaa 0x03... 
					for(i=0,j=0;i<event.size;i++)
					{
                      if(temp[i] == 0x55 && temp[i+1] == 0xaa)
                      {
						if(j > 0)
						{
                          igs_uart_recv_handle(&recv_buf[0],j);
						  memset(&recv_buf[0],0x00,64);
						  j = 0;
						}
						recv_buf[j++] = temp[i];
					  }else if(j > 0){
                        recv_buf[j++] = temp[i];
					  }
					  
					}
					if(j > 4)
					{
					  igs_uart_recv_handle(&recv_buf[0],j);
					}
				    free(temp);
				 }
                break;
            default:
                break;
            }
        }
    }
    vTaskDelete(NULL);
}


static void blufi_uart_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_RTS,
        .rx_flow_ctrl_thresh = 122,
        .source_clk = UART_SCLK_APB,
    };

    //Install UART driver, and get the queue.
    uart_driver_install(UART_NUM_2, 4096, 8192, 10,&blufi_uart_queue,0);
	//uart_driver_install(UART_NUM_2, 4096, 8192, 10,NULL,0);
    //Set UART parameters
    uart_param_config(UART_NUM_2, &uart_config);
    //Set UART pins
    //uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
	uart_set_pin(UART_NUM_2, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    xTaskCreate(blufi_uart_task, "blufi_uart_Task", 2048, (void*)UART_NUM_2, 8, NULL);
}



void igs_util_task(void* arg)
{
  int ret;
  blufi_uart_init();

    sleep(5);
    ret = uart_write_bytes(UART_NUM_2,(char*)&heart_buf[0],7);
 	printf("Uart write %d bytes ok, heart buf!\n",ret);
 	sleep(5);
    ret = uart_write_bytes(UART_NUM_2,(char*)&pid_buf[0],7);
	printf("Uart write %d bytes ok, pid buf!\n",ret);
	sleep(5);
    ret = uart_write_bytes(UART_NUM_2,(char*)&work_mode[0],7);
	printf("Uart write %d bytes ok, work mode!\n",ret);
  
  while(1)
  {
    sleep(15);
    ret = uart_write_bytes(UART_NUM_2,(char*)&heart_buf[0],7);
 	//printf("Uart write %d bytes ok, heart buf!\n",ret);

	#if 0
	sleep(10);
    ret = uart_write_bytes(UART_NUM_2,(char*)&all_on[0],12);
 	printf("Uart write %d bytes ok, all on!\n",ret);
	
	sleep(10);
    ret = uart_write_bytes(UART_NUM_2,(char*)&light_mode[0],12);
	printf("Uart write %d bytes ok,light on!\n",ret);
	sleep(10);
	ret = uart_write_bytes(UART_NUM_2,(char*)&light_mode0[0],12);
	printf("Uart write %d bytes ok,light off!\n",ret);
	
	sleep(10);
    ret = uart_write_bytes(UART_NUM_2,(char*)&pump_on[0],12);
	printf("Uart write %d bytes ok,pump on!\n",ret);
	sleep(10);
	ret = uart_write_bytes(UART_NUM_2,(char*)&pump_off[0],12);
	printf("Uart write %d bytes ok,pump off!\n",ret);
	
	sleep(10);
    ret = uart_write_bytes(UART_NUM_2,(char*)&grow_day[0],15);
	printf("Uart write %d bytes ok,grow day!\n",ret);
	sleep(10);
	ret = uart_write_bytes(UART_NUM_2,(char*)&grow_day0[0],15);
	printf("Uart write %d bytes ok,grow day 0!\n",ret);

	sleep(10);
	ret = uart_write_bytes(UART_NUM_2,(char*)&light_time[0],sizeof(light_time));
	printf("Uart write %d bytes ok,light time!\n",ret);

	sleep(30);
    ret = uart_write_bytes(UART_NUM_2,(char*)&all_off[0],12);
 	printf("Uart write %d bytes ok, all off!\n",ret);
	sleep(30);
	#endif
  }

}

#endif

