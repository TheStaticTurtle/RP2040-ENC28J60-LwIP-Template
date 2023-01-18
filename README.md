# RP2040-ENC28J60-Template

This repo is a template to get ethernet working with the integrated lwip stack that is present in the rp2040 sdk.

It includes:
 - Driver and lwip bindings for the enc28j60 from: https://git.sr.ht/~krystianch/pico-enc28j60
 - Basic arch lwip config to use without and OS

This example acts a dhcp client with an http server and iperf2 server running

By default it's using SPI0 at 200kHz with these pins
| ENC28J60 | RP2040 |
|----------|--------|
| SCK      | GP2    |
| SI       | GP3    |
| SO       | GP4    |
| CS       | GP5    |
| INT      | GP6    |
| RESET    | -      |
| CLK      | -      |
| WOL      | -      |
