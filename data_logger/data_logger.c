#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <librawp.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define endianflip(x) ( ((x&0xff)<<24) | ((x&0xff00)<<8) | ((x&0xff0000)>>8) | ((x&0xff0000)>>24) )

int rawp;


void mycb( void * id, void * rr, uint8_t * data, int dlen )
{
	int i;
#if 0
	printf( "%d: ", dlen );
	for( i = 0; i < dlen; i++ )
		printf( "[%d %02x (%c) ", i, data[i], (data[i]>32)?data[i]:' '  );
		//printf( "0x%02x, ", data[i], (data[i]>32)?data[i]:' '  );
	printf( "\n" );
#endif
	//We're specifically looking for messages with "VOLTEE" as the name.

	if( memcmp( data + 68, "VOLTTE", 6) == 0 )
	{
		printf( "VOLTEE MATCH\n" );
		uint32_t packid = data[74]<<24 | data[75] << 16 | data[76] << 8 | data[77];
		float voltage = ((float)(data[78]<<8 | data[79]))/1000.0;
		int8_t rssi0 = data[0x22];
		int8_t rssi1 = data[0x24];
		char mac[128];
		char run[128];
		sprintf( mac, "%02x-%02x-%02x-%02x-%02x-%02x", data[48], data[49], data[50], data[51], data[52], data[53] );
		sprintf( run, "sh submit_value.sh %d %f %s %d %d %d", packid, voltage, mac, rssi0, rssi1 );
		printf( "%s\n", run );
		system( run );
	}
}


int main()
{
#if 1
	system( "ifconfig mon0 down > /dev/null" );
	system( "iw phy phy0 interface add mon0 type monitor > /dev/null");
	//system( "iw dev wlan0 del > /dev/null" );
	system( "ifconfig mon0 up > /dev/null" );
	system( "iw dev mon0 set channel 7" );
#endif

	sleep(1);
#ifdef MAKE_PCAP_STREAM
	uint8_t pcap_header[] = {
		0xd4, 0xc3, 0xb2, 0xa1, 0x02, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x04, 0x00, 0x7f, 0x00, 0x00, 0x00, 
	};
	fwrite( pcap_header, sizeof(pcap_header), 1, stdout );
#endif

	rawp = librawp_setup( "mon0", 1 );
	fprintf( stderr, "RAWP: %d\n", rawp );
	if( rawp <= 0 ) return -1;
	void * id = 0;
//	pthread_t t;
//	pthread_create( &t, 0, TXThread, 0 );
	int r = librawp_receive( rawp, mycb, id, 1 );
	return 0;
}

