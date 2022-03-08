#ifndef  __MY_OTA_TEST__
#define  __MY_OTA_TEST__

enum ixe_update_status{
	NONE_UPDATE = 0,
	UPDATE_START,
    DOWNLOAD_OVER,
    DOWNLOAD_FAIL,
    VERSION_SAME,
    UPDATE_FAIL,
    END_UPDATE
};

void xota_set_status(uint8_t value);
uint8_t xota_get_status(void);
esp_err_t ixe_ble_stop(void);
void ixe_ota_task(void *pvParameter);


#endif