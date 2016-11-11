#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "espconn.h"
#include "string.h"
#include <ip_addr.h>
#include <c_types.h>
#include "rom.h"
#include "mem.h"
#include "utils/utils.h"
#include "websockets.h"


struct espconn espconnection;
esp_tcp tcpconn;
struct espconn *connection_collection [5];
uint8 connection_counter = 0;
static void ICACHE_FLASH_ATTR dataRecived (void *args, char *pusrdata, unsigned short length) {
    struct espconn *recivedConnection = args;
	char *token;
	bool flag;
	char *header;
	char *response;
	header =(char *) os_zalloc(strlen(pusrdata)+1);
	strcpy(header, pusrdata);


	if (isUpgradeable(header)) {
		os_printf("upgradeable \n");
		strcpy(header, pusrdata);
		if(handshake(header, recivedConnection)) {
			os_printf("connection ready \n state: %d \n",recivedConnection->state);
		}

	} else {
		os_printf("not upgrade request \n \n");

		os_printf("connection state: %d", recivedConnection->state);
	}



	os_free(header);
    // espconn_send(recivedConnection, string, os_strlen(string));

    // espconn_disconnect(recivedConnection);
}

static void ICACHE_FLASH_ATTR dataSent (void *args) {
	os_printf("data sent \n");
}

static void ICACHE_FLASH_ATTR connectCB (void *arg) {
	//called once when connected.
    struct espconn *newConnection = arg;

    espconn_regist_recvcb(newConnection, dataRecived);
    espconn_regist_sentcb(newConnection, dataSent);

    // espconn_send(newConnection, string, os_strlen(string));
}

void tcp_server_start () {
    tcpconn.local_port = 80;
    espconnection.type = ESPCONN_TCP;
    espconnection.state = ESPCONN_NONE;
    espconnection.proto.tcp = &tcpconn;

    espconn_regist_connectcb(&espconnection, connectCB);
    espconn_accept(&espconnection);
}




void ready () {
    char ssid[32] = WIFINAME;
    char password[64] = PASSWORD;
    uint8 connection_status;
    struct station_config stationConfiguration;
    os_printf("System Ready \n");
    spi_flash_erase_sector(0x7E);

    os_memcpy(&stationConfiguration.ssid, ssid, 32);
    os_memcpy(&stationConfiguration.password, password, 64);

    wifi_set_opmode(0x01);
    wifi_station_set_auto_connect(FALSE);

    if(wifi_station_set_config(&stationConfiguration)) {
        os_printf("Config set \n");
        wifi_station_connect();

        tcp_server_start();

    } else {
        os_printf("Error setting configurations");
    }


};


uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
}


void ICACHE_FLASH_ATTR
user_init() {
    uart_div_modify(0, UART_CLK_FREQ / 115200);
    system_init_done_cb(&ready);
}
