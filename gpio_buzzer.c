/*

control buzzer connected to gpio of raspberry pi

(c) 2013 Sven Geggus <sven-web20mash@geggus.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.

*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <gpiod.h>

#define GPIO_CHIP    "/dev/gpiochip0"
#define BUZZER_LINE  18
#define CONSUMER     "gpio_buzzer"

static volatile int run = 1;

static void signalHandler(int sig) {
  (void)sig;
  run = 0;
}

static void die_usage(const char *prog) {
  fprintf(stderr, "usage: %s [duration] [interval(seconds)]\n", prog);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

  struct gpiod_chip           *chip     = NULL;
  struct gpiod_line_settings  *settings = NULL;
  struct gpiod_line_config    *line_cfg = NULL;
  struct gpiod_request_config *req_cfg  = NULL;
  struct gpiod_line_request   *request  = NULL;
  unsigned int offsets[1] = { BUZZER_LINE };
  int duration;
  float interval;
  int ret = EXIT_FAILURE;

  if (argc > 3 || argc < 1)
    die_usage(argv[0]);

  signal(SIGINT,  signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGALRM, signalHandler);

  // open chip
  chip = gpiod_chip_open(GPIO_CHIP);
  if (!chip) {
    fprintf(stderr, "unable to open GPIO chip " GPIO_CHIP "\n");
    goto out;
  }

  // set line to output
  settings = gpiod_line_settings_new();
  if (!settings)
    goto out;
  gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
  gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

  // line-Config
  line_cfg = gpiod_line_config_new();
  if (!line_cfg)
    goto out;
  if (gpiod_line_config_add_line_settings(line_cfg, offsets, 1, settings) < 0)
    goto out;

  // request config
  req_cfg = gpiod_request_config_new();
  if (!req_cfg)
    goto out;
  gpiod_request_config_set_consumer(req_cfg, CONSUMER);

  // req_cfg line
  request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
  if (!request) {
    fprintf(stderr,
            "unable to request line %d on " GPIO_CHIP
            " (already in use?)\n", BUZZER_LINE);
    goto out;
  }

  gpiod_request_config_free(req_cfg);  req_cfg  = NULL;
  gpiod_line_config_free(line_cfg);    line_cfg = NULL;
  gpiod_line_settings_free(settings);  settings = NULL;

  // set alarm timer
  if (argc > 1) {
    if (1 != sscanf(argv[1], "%d", &duration))
      die_usage(argv[0]);
    if (duration < 0)
      die_usage(argv[0]);
    alarm(duration);
  }

  // interval from command line
  if (argc == 3) {
    if (1 != sscanf(argv[2], "%f", &interval))
      die_usage(argv[0]);
    if (interval < 0)
      die_usage(argv[0]);
  } else {
    interval = 100;
  }

  // buzzer loop
  while (run) {
    gpiod_line_request_set_value(request, BUZZER_LINE,
                                 GPIOD_LINE_VALUE_ACTIVE);
    usleep((useconds_t)(1000 * interval));
    gpiod_line_request_set_value(request, BUZZER_LINE,
                                 GPIOD_LINE_VALUE_INACTIVE);
    usleep((useconds_t)(1000 * interval));
  }

  // disable buzzer
  gpiod_line_request_set_value(request, BUZZER_LINE,
                               GPIOD_LINE_VALUE_INACTIVE);
  ret = EXIT_SUCCESS;

out:
  // cleanup
  if (request)  gpiod_line_request_release(request);
  if (req_cfg)  gpiod_request_config_free(req_cfg);
  if (line_cfg) gpiod_line_config_free(line_cfg);
  if (settings) gpiod_line_settings_free(settings);
  if (chip)     gpiod_chip_close(chip);

  return ret;
}