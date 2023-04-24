/*
 * main.cpp - part of the Galactic Unicorn Scrolling Quote demo.
 *
 * This file defines the main() function, the entrypoint for your program.
 *
 * This program fetches a quote from https://quotes.rest/
 *
 * Copyright (C) 2023 Pete Favelle <ahnlak@ahnlak.com>
 * This file is released under the BSD 3-Clause License; see LICENSE for details.
 */

/* Standard header files. */

#include <stdio.h>
#include <stdlib.h>


/* SDK header files. */

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "pico_graphics.hpp"
#include "galactic_unicorn.hpp"
#include "font8_data.hpp"


/* Local header files. */

#include "usbfs.h"
#include "opt/config.h"
#include "opt/httpclient.h"
#include "okcolor.hpp"


/* Functions. */

#define MESSAGE_LENGTH    130

/*
 * extract_message - pulls the content out of the returned JSON; we are only
 *                   interested in the 'content' block.
 */

bool extract_message( const char *p_source, char *p_target, int p_targetlen )
{
  const char *l_startptr;
  const char *l_endptr;
  int         l_message_length;

  /* Try and find the content element. */
  l_startptr = strstr( p_source, "\"content\"" );
  if ( l_startptr == NULL )
  {
    return false;
  }

  /* Move onto the body of that element. */
  l_startptr += 9;
  while ( ( *l_startptr == ' ' ) || ( *l_startptr == ':' ) || ( *l_startptr == '\"' ) )
  {
    l_startptr++;
  }

  /* Now look for the closing quote on that body. */
  l_endptr = strchr( l_startptr, '\"' );
  if ( l_endptr == NULL )
  {
    return false;
  }

  /* Then simply copy the body into the target, paying attention to the length. */
  l_message_length = l_endptr - l_startptr;
  if ( l_message_length >= p_targetlen )
  {
    l_message_length = p_targetlen-1;
  }
  strncpy( p_target, l_startptr, l_message_length );
  p_target[l_message_length] = '\0';

  /* All done then! */
  return true;
}


/*
 * main() - the entrypoint of the application; this is what runs when the PicoW
 *          starts up, and would never normally exit.
 */

int main()
{
  float                             l_scroll_offset;
  int                               l_message_width;
  char                              l_scroll_message[MESSAGE_LENGTH+1];
  httpclient_request_t             *l_http_request;
  pimoroni::PicoGraphics_PenRGB888  l_graphics( 53, 11, NULL );
  pimoroni::GalacticUnicorn         l_unicorn;

  /* Initialise stdio handling. */
  stdio_init_all();

  /* Initialise the WiFi chipset. */
  if ( cyw43_arch_init() )
  {
    printf( "Failed to initialise the WiFI chipset (cyw43)\n" );
    return 1;
  }

  /* And the USB handling. */
  usbfs_init();

  /* Declare some default configuration details. */
  config_t default_config[] = 
  {
    { "WIFI_SSID", "my_network" },
    { "WIFI_PASSWORD", "my_password" },
    { "", "" }
  };

  /* Set up the initial load of the configuration file. */
  config_load( "config.txt", default_config, 10 );

  /* Save it straight out, to preserve any defaults we put there. */
  config_save();

  /* Initialise the Galactic Unicorn and associated graphics. */
  l_unicorn.init();
  l_graphics.set_font( &font8 );
  l_scroll_offset = -53.0f;
  strcpy( l_scroll_message, "Loading..." );

  /* Set up a simple web request. */
  httpclient_set_credentials( config_get( "WIFI_SSID" ), config_get( "WIFI_PASSWORD" ) );
  l_http_request = httpclient_open( "https://api.quotable.io/random", NULL, 1024 );

  /* Enter the main program loop now. */
  while( true )
  {
    /* Monitor the configuration file. */
    if ( config_check() )
    {
      /* Switch to potentially new WiFi credentials. */
      httpclient_set_credentials( config_get( "WIFI_SSID" ), config_get( "WIFI_PASSWORD" ) );
    }

    /* Service the http request, if still active. */
    if ( l_http_request != NULL )
    {
      httpclient_status_t l_status = httpclient_check( l_http_request );
      if ( ( l_status == HTTPCLIENT_COMPLETE ) || ( l_status == HTTPCLIENT_TRUNCATED ) )
      {
        /* Try and find the quote  in the response. */
        if ( extract_message( httpclient_get_response( l_http_request ), l_scroll_message, MESSAGE_LENGTH ) )
        {
          /* Successfully found it, so reset the scroll offset. */
          l_scroll_offset = -53.0f;
        }

        /* And close down the request. */
        httpclient_close( l_http_request );
        l_http_request = NULL;
      }
    }

    /* Handle user input to adjust the brightness of the Unicorn. */
    if( l_unicorn.is_pressed( pimoroni::GalacticUnicorn::SWITCH_BRIGHTNESS_UP ) )
    {
      l_unicorn.adjust_brightness( +0.01 );
    }
    if( l_unicorn.is_pressed( pimoroni::GalacticUnicorn::SWITCH_BRIGHTNESS_DOWN ) )
    {
      l_unicorn.adjust_brightness( -0.01 );
    }

    /* And allow button A to request a fresh quote. */
    if ( l_unicorn.is_pressed( pimoroni::GalacticUnicorn::SWITCH_A ) )
    {
      /* Only try if we're not already trying. */
      if ( l_http_request == NULL )
      {
        l_http_request = httpclient_open( "https://api.quotable.io/random", NULL, 1024 );
      }
    }

    /* Render the text, scrolled appropriately. */
    l_message_width = l_graphics.measure_text( l_scroll_message, 1 );
    l_scroll_offset += 0.25f;

    /* Wrap the message. */
    if ( l_scroll_offset > l_message_width )
    {
      l_scroll_offset = -53.0f;
    }

    /* Clear the display. */
    l_graphics.set_pen( 0, 0, 0 );
    l_graphics.clear();

    /*
     * Work out a suitable colour to use - this pretty much lifted straight 
     * from Pimoroni's original scrolltext demo
     */
    ok_color::HSL l_hsl{ l_scroll_offset/100.0f, 1.0f, 0.5f };
    ok_color::RGB l_rgb = ok_color::okhsl_to_srgb( l_hsl );

    /* Set the pen to that colour, and render the text. */
    l_graphics.set_pen( l_rgb.r*255, l_rgb.g*255, l_rgb.b*255 );
    l_graphics.text( l_scroll_message, pimoroni::Point( 0-l_scroll_offset, 2 ), -1, 0.55 );

    /* Finally, render all that onto the Unicorn. */
    l_unicorn.update( &l_graphics );

    /* And wait a brief moment. */
    usbfs_sleep_ms( 10 );
  }

  /* We would never expect to reach an end....! */
  return 0;
}

/* End of file main.cpp */
