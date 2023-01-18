# RP2040-ENC28J60-Template

This repo is a template to get ethernet working with the integrated lwip stack that is present in the rp2040 sdk.

It includes:
 - Driver and lwip bindings for the enc28j60 from: https://git.sr.ht/~krystianch/pico-enc28j60
 - Basic arch lwip config to use without and OS

This example acts a dhcp client with an http server and iperf2 server running