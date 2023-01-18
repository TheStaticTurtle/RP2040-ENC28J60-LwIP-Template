#include <hardware/gpio.h>

#include <hardware/spi.h>
#include <pico/critical_section.h>
#include <pico/stdio.h>
#include <pico/util/queue.h>

#include "lwip/lwipopts.h"
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/dhcp.h>
#include <lwip/timeouts.h>
#include <lwip/apps/httpd.h>
#include <lwip/apps/lwiperf.h>

#include "driver/enc28j60/enc28j60.h"
#include "lwip/enc28j60-ethernetif.h"

//#include "tcpecho_raw.h"

/* Configuration */
#define SPI spi0
#define SPI_BAUD 2000000
#define SCK_PIN 2
#define SI_PIN 3
#define SO_PIN 4
#define CS_PIN 5
#define INT_PIN 6
#define RX_QUEUE_SIZE 10
#define MAC_ADDRESS { 0x62, 0x5E, 0x22, 0x07, 0xDE, 0x92 }
/*
#define IP_ADDRESS IPADDR4_INIT_BYTES(10, 10, 10, 98)
#define NETWORK_MASK IPADDR4_INIT_BYTES(255, 255, 255, 0)
#define GATEWAY_ADDRESS IPADDR4_INIT_BYTES(10, 10, 10, 254)
*/
#define IP_ADDRESS IPADDR4_INIT_BYTES(0, 0, 0, 0)
#define NETWORK_MASK IPADDR4_INIT_BYTES(0, 0, 0, 0)
#define GATEWAY_ADDRESS IPADDR4_INIT_BYTES(0, 0, 0, 0)

queue_t rx_queue;
critical_section_t spi_cs;
struct netif netif;
struct enc28j60 enc28j60 = {
	.spi = SPI,
	.cs_pin = CS_PIN,
	.mac_address = MAC_ADDRESS,
	.next_packet = 0,
	.critical_section = &spi_cs,
};

void
eth_irq(uint gpio, uint32_t events)
{
	enc28j60_isr_begin(&enc28j60);
	uint8_t flags = enc28j60_interrupt_flags(&enc28j60);

	if (flags & ENC28J60_PKTIF) {
		struct pbuf *packet = low_level_input(&netif);
		if (packet != NULL) {
			if (!queue_try_add(&rx_queue, &packet)) {
				pbuf_free(packet);
			}
		}
	}

	if (flags & ENC28J60_TXERIF) {
		LWIP_DEBUGF(NETIF_DEBUG, ("eth_irq: transmit error\n"));
	}

	if (flags & ENC28J60_RXERIF) {
		LWIP_DEBUGF(NETIF_DEBUG, ("eth_irq: receive error\n"));
	}

	enc28j60_interrupt_clear(&enc28j60, flags);
	enc28j60_isr_end(&enc28j60);
}

void lwiperf_report(void *arg, enum lwiperf_report_type report_type, const ip_addr_t *local_addr, u16_t local_port, const ip_addr_t *remote_addr, u16_t remote_port, u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec) {
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(local_addr);
  LWIP_UNUSED_ARG(local_port);

  printf("IPERF report: type=%d, remote: %s:%d, total bytes: %"U32_F", duration in ms: %"U32_F", kbits/s: %"U32_F"\n",
    (int)report_type, ipaddr_ntoa(remote_addr), (int)remote_port, bytes_transferred, ms_duration, bandwidth_kbitpsec);
}

int main() {
	gpio_init_mask((1 << CS_PIN) | (1 << PICO_DEFAULT_LED_PIN));
	gpio_set_dir_out_masked((1 << CS_PIN) | (1 << PICO_DEFAULT_LED_PIN));
	gpio_put(PICO_DEFAULT_LED_PIN, true);

	stdio_init_all();

	gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SI_PIN, GPIO_FUNC_SPI);
	gpio_set_function(SO_PIN, GPIO_FUNC_SPI);
	spi_init(spi0, SPI_BAUD);

	queue_init(&rx_queue, sizeof(struct pbuf *), RX_QUEUE_SIZE);
	critical_section_init(&spi_cs);

	const struct ip4_addr ipaddr = IP_ADDRESS;
	const struct ip4_addr netmask = NETWORK_MASK;
	const struct ip4_addr gw = GATEWAY_ADDRESS;
	lwip_init();
	netif_add(&netif, &ipaddr, &netmask, &gw, &enc28j60, ethernetif_init, netif_input);
	
	netif_set_up(&netif);
	netif_set_link_up(&netif);
	netif_set_default(&netif);
	
	dhcp_start(&netif);

	gpio_set_irq_enabled_with_callback(INT_PIN, GPIO_IRQ_EDGE_FALL, true, eth_irq);
	enc28j60_interrupts(&enc28j60, ENC28J60_PKTIE | ENC28J60_TXERIE | ENC28J60_RXERIE);

    httpd_init();
  lwiperf_start_tcp_server_default(lwiperf_report, NULL);
	//tcpecho_raw_init();

	while (true) {
		struct pbuf* p = NULL;
		queue_try_remove(&rx_queue, &p);
		if (p != NULL) {
			if(netif.input(p, &netif) != ERR_OK) {
				LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
				pbuf_free(p);
			}
		}

		sys_check_timeouts();
		gpio_put(PICO_DEFAULT_LED_PIN, false);
		best_effort_wfe_or_timeout(make_timeout_time_ms(sys_timeouts_sleeptime()));
		gpio_put(PICO_DEFAULT_LED_PIN, true);
	}
}
