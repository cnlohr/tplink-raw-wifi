#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <librawp.h>
#include <unistd.h>
#include <pthread.h>

#define endianflip(x) ( ((x&0xff)<<24) | ((x&0xff00)<<8) | ((x&0xff0000)>>8) | ((x&0xff0000)>>24) )

int rawp;


void mycb( void * id, void * rr, uint8_t * data, int dlen )
{
	int i;

#ifndef MAKE_PCAP_STREAM
	printf( "%d: ", dlen );
	for( i = 0; i < dlen; i++ )
		//printf( "%02x (%c) ", data[i], (data[i]>32)?data[i]:' '  );
		printf( "0x%02x, ", data[i], (data[i]>32)?data[i]:' '  );
	printf( "\n" );
#else
	struct timeval tv;
	gettimeofday (&tv, 0 );
	uint32_t out[4];
	out[0] = endianflip(tv.tv_sec);
	out[1] = endianflip(tv.tv_usec);
	out[2] = endianflip(dlen);
	out[3] = endianflip(dlen);
	fwrite( out, 16, 1, stdout );
	fwrite( data, dlen, 1, stdout );
#endif
}


void * TXThread( void * v )
{
	//Send rawp packets?
	int ct = 0;
	while(1)
	{
		uint8_t pack[] = { 
			0x00, 0x00, 0x26, 0x00, 0x2f, 0x40, 0x00, 0xa0,	//Radiotap header
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//TX Time (or something?
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //TX Time?
			0x00, //Flags: No FCS at end
			0x60, //Data rate.
			0x85, 0x09, 0xc0, 0x00, 0x14, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x01, //More flags and stuff (mostly ignored)

#if 0
/* Looks like an ESP-Now Packet */
			0xd0, 0x00, 0x00, 0x00, //IEEE Header
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //Destination Address
			//0x1a, 0xfe, 0x34, 0xe1, 0x48, 0x05, //Src Address  (ESP Mac)
			0x30, 0xb5, 0xc2, 0x5d, 0x1b, 0xc6, //Src Address  (TP Link MAC)
			//0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	//Src address			
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	//
			0x00, 0x00,	//Sequence number
			0x7f, 0x18, 0xfe, 0x34, // "vendor specific" information (matches.
			0x4b, 0xfc, 0xec, 0xb6,	// 0x67? ?!?!?! session or something?
			0xdd, 0x0d, 0x18, 0xfe, 0x34, 0x04, 0x01,	//ESPNOW protocol
			0x45, 0x53, 0x50, 0x38, 0x32, 0x36, 0x36, 0x00,  //Payload
		};
		pack[60] = ct<<5;
		pack[61] = ct>>3;
		ct++;

		pack[66] = rand();
		pack[67] = rand();
		pack[68] = rand();
		pack[69] = rand();
#else
			0x08, 0x00, 0x00, 0x00, //IEEE Header NOTE: 0x08 seems to immediately get through to the ESP8266.
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //Destination Address
			//0x1a, 0xfe, 0x34, 0xe1, 0x48, 0x05, //Src Address  (ESP Mac)
			0x30, 0xb5, 0xc2, 0x5d, 0x1b, 0xc6, //Src Address  (TP Link MAC)
			//0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	//Src address			
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff,	//
			0x00, 0x00,	//Sequence number
			0x7f, 0x18, 0xfe, 0x34, // "vendor specific" information (matches.
			0x4b, 0xfc, 0xec, 0xb6,	// 0x67? ?!?!?! session or something?
			0xdd, 0x0d, 0x18, 0xfe, 0x34, 0x04, 0x01,	//ESPNOW protocol
			0x45, 0x53, 0x50, 0x38, 0x32, 0x36, 0x36, 0x00,  //Payload
		};

#endif

		int r = send( rawp, pack, sizeof(pack), 0 );
		/*data_size = recv( rawp, buffer, sizeof( buffer ), 0 );
		if( data_size < 0 )
		{
			fprintf( stderr, "Recvfrom error, failed to get packets\n");
			return -2;
		}
*/

		usleep(1000);
	}
}


int main()
{

#if 1
	system( "ifconfig mon0 down > /dev/null" );
	system( "iw phy phy0 interface add mon0 type monitor > /dev/null");
	//system( "iw dev wlan0 del > /dev/null" );
	system( "ifconfig mon0 up > /dev/null" );
	//system( "iw dev mon0 set channel 1" );
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
	pthread_t t;
	pthread_create( &t, 0, TXThread, 0 );
	int r = librawp_receive( rawp, mycb, id, 1 );
	return 0;
}

