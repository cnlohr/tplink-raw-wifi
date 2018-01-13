#include "librawp.h"
#include <stdio.h>
#include <netinet/if_ether.h>  //For ETH_P_ALL
#include <linux/if_packet.h> //For PACKET_ADD_MEMBERSHIP, etc.
#include <linux/wireless.h>
#include <linux/if.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
//#include <net/if.h>


int librawp_setup( const char * interface, int channel )
{
	int sock_raw;
	struct ifreq ifr;
	struct sockaddr_ll sa;
	size_t if_name_len=strlen(interface);
	int ss = 0;

	sock_raw = socket( PF_PACKET, SOCK_RAW, htons(ETH_P_ALL) );
	if( sock_raw < 0 )
	{
		fprintf( stderr, "Error: Can't open socket.\n" );
		return -1;
	}
	if (if_name_len<sizeof(ifr.ifr_name)) {
		memcpy(ifr.ifr_name,interface,if_name_len);
		ifr.ifr_name[if_name_len]=0;
	
	}
	else
	{
		close( sock_raw );
		return -1;
	}

	ioctl( sock_raw, SIOCGIFINDEX, &ifr );

//#define PROMISC
#ifdef PROMISC
	ifr.ifr_flags |= IFF_PROMISC;
	ioctl(sock_raw, SIOCSIFFLAGS, &ifr);
#endif

	memset( &sa, 0, sizeof( sa ) );
	sa.sll_family=PF_PACKET;
	sa.sll_protocol = 0x0000;
	fprintf( stderr, "IFRI: %d\n", ifr.ifr_ifindex );
	sa.sll_ifindex = ifr.ifr_ifindex;
	sa.sll_hatype = 0;
	sa.sll_pkttype = PACKET_HOST;

	bind( sock_raw,(const struct sockaddr *)&sa, sizeof( sa ) );

	//This one is really useful on some OLD systems.  On newer systems, it breaks things.
	ss = setsockopt( sock_raw, SOL_SOCKET, SO_BINDTODEVICE, interface, strlen(interface) );

	if( ss < 0 )
	{
		close( sock_raw );
		return -1;
	}

	return sock_raw;
}

int librawp_receive( int rawp, librawp_cb_t cb, void * id, int is_wifi )
{
	int data_size;
	uint8_t buffer[65536];

	while(1)
	{
		data_size = recv( rawp, buffer, sizeof( buffer ), 0 );
		if( data_size < 0 )
		{
			fprintf( stderr, "Recvfrom error (%d), failed to get packets\n", data_size);
			return -2;
		}


		cb( id, 0, buffer, data_size );
	}

	return 0;
}



