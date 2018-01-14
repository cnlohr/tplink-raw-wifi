//Copyright 2015 <>< Charles Lohr, see LICENSE file.

#include <stdlib.h>
#include "mem.h"
#include "c_types.h"
#include "user_interface.h"
#include "ets_sys.h"
#include "uart.h"
#include "osapi.h"
#include "espconn.h"
#include "esp82xxutil.h"
#include "commonservices.h"
#include "vars.h"
#include <mdns.h>
#include <espnow.h>

#define procTaskPrio        0
#define procTaskQueueLen    1

#define CHANNEL 6

static os_timer_t some_timer;
static struct espconn *pUdpServer;
usr_conf_t * UsrCfg = (usr_conf_t*)(SETTINGS.UserData);

//int ICACHE_FLASH_ATTR StartMDNS();


void user_rf_pre_init(void) { /*nothing*/ }

//Tasks that happen all the time.

os_event_t    procTaskQueue[procTaskQueueLen];

static void ICACHE_FLASH_ATTR procTask(os_event_t *events)
{
	CSTick( 0 );
	system_os_post(procTaskPrio, 0, 0 );
}

void espnowcb(u8 *mac_addr, u8 *data, u8 len)
{
	printf( "++%02x:%02x:%02x:%02x:%02x:%02x / %d:", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], len );
	int i;
	for( i = 0; i < len; i++ )
	{
		printf( "%02x ", data[i] );
	}
	printf( "\n" );
}


void espnowcbtx(u8 *mac_addr, u8 status)
{
	printf( ">>%02x:%02x:%02x:%02x:%02x:%02x / %d\n", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5], status );
}


//Timer event.
static void ICACHE_FLASH_ATTR myTimer(void *arg)
{
	static int doneinit;
	if( !doneinit )
	{
		esp_now_init();
	//	esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
	//	uint8_t routermac[] = {0x30, 0xb5, 0xc2, 0x5d, 0x1b, 0xc6,}; //Src Address of my router.
		esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
		esp_now_register_recv_cb(espnowcb);
		esp_now_register_send_cb(espnowcbtx);
		uint8_t routermac[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff,}; //Src Address of my router.
		printf( "PEERINFO: %d\n", esp_now_add_peer(routermac, ESP_NOW_ROLE_SLAVE, CHANNEL, 0, 0) );
		doneinit = 1;
	}
	
	uint8_t da[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	esp_now_send(da, "ESP8266", 8 );
	CSTick( 1 ); // Send a one to uart
}


//Called when new packet comes in.
static void ICACHE_FLASH_ATTR
udpserver_recv(void *arg, char *pusrdata, unsigned short len)
{
	struct espconn *pespconn = (struct espconn *)arg;

	uart0_sendStr("X");
}

void ICACHE_FLASH_ATTR charrx( uint8_t c )
{
	//Called from UART.
}


void user_init(void)
{
	uart_init(BIT_RATE_115200, BIT_RATE_115200);

	uart0_sendStr("\r\nesp82XX Web-GUI\r\n" VERSSTR "\b");

//Uncomment this to force a system restore.
//	system_restore();

	CSSettingsLoad( 0 );
	CSPreInit();

    pUdpServer = (struct espconn *)os_zalloc(sizeof(struct espconn));
	ets_memset( pUdpServer, 0, sizeof( struct espconn ) );
	espconn_create( pUdpServer );
	pUdpServer->type = ESPCONN_UDP;
	pUdpServer->proto.udp = (esp_udp *)os_zalloc(sizeof(esp_udp));
	pUdpServer->proto.udp->local_port = COM_PORT;
	espconn_regist_recvcb(pUdpServer, udpserver_recv);

	if( espconn_create( pUdpServer ) )
	{
		while(1) { uart0_sendStr( "\r\nFAULT\r\n" ); }
	}

	CSInit();

	SetServiceName( "espcom" );
	AddMDNSName(    "esp82xx" );
	AddMDNSName(    "espcom" );
	AddMDNSService( "_http._tcp",    "An ESP82XX Webserver", WEB_PORT );
	AddMDNSService( "_espcom._udp",  "ESP82XX Comunication", COM_PORT );
	AddMDNSService( "_esp82xx._udp", "ESP82XX Backend",      BACKEND_PORT );

	//Add a process
	system_os_task(procTask, procTaskPrio, procTaskQueue, procTaskQueueLen);

	//Timer example
	os_timer_disarm(&some_timer);
	os_timer_setfn(&some_timer, (os_timer_func_t *)myTimer, NULL);
	os_timer_arm(&some_timer, 100, 1);

	printf( "Boot Ok.\n" );

//	wifi_set_sleep_type(LIGHT_SLEEP_T);
//	wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);


	struct softap_config cfg;
	wifi_softap_get_config(&cfg);
	cfg.channel = CHANNEL;
	wifi_softap_set_config(&cfg);
	wifi_set_opmode(SOFTAP_MODE);


	wifi_set_user_fixed_rate(FIXED_RATE_MASK_ALL, PHY_RATE_48);

	system_os_post(procTaskPrio, 0, 0 );
}


//There is no code in this project that will cause reboots if interrupts are disabled.
void EnterCritical() { }

void ExitCritical() { }


