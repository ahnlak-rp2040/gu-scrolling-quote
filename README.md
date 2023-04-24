[![CMake](https://github.com/ahnlak-rp2040/gu-scolling-quote/actions/workflows/cmake.yml/badge.svg)](https://github.com/ahnlak-rp2040/gu-scolling-quote/actions/workflows/cmake.yml)

# Galactic Unicorn Scrolling Quote Demo

This is small demonstration of pulling a quote from a remote web server, and
scrolling it on the Unicorn.

This is provided as an example of the [PicoW Boilerplate](https://github.com/ahnlak-rp2040/picow-boilerplate)
in action; it uses both the `config` and `httpclient` libraries from there
to manage the fetching of data.

Once you copy the `uf2` onto your [Galactic Unicorn](https://shop.pimoroni.com/products/galactic-unicorn)
and the device has rebooted, it should mount a 'PicoW' drive on which you will
find the `CONFIG.TXT` file. Adding your WiFi credentials to this file should
prompt the device to fetch a suitable quote and start scrolling it.

The quote is pulled from [Quotable](https://github.com/lukePeavey/quotable),
largely because it (a) doesn't require authentication and (b) doesn't require
TLS1.3!

## Controls

The LUX buttons adjust the brightness of the message.

Pressing the 'A' button will request a fresh quote from the server.


Share & Enjoy.