/*

mashctld

a web-controllable two-level temperature and mash process
controler for various sensors and actuators

(c) 2011-2013 Sven Geggus <sven-web20mash@geggus.net>
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

gpio plugin for actuator control using libgpiod v2

*/
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <gpiod.h>
#include "minIni.h"
#include "errorcodes.h"

static char plugin_name[] = "actuator_gpio";

// default is no error
static int plugin_error[2] = {0, 0};

// defaults is the raspberry Pi default gpio chip
// this
#define GPIO_CHIP		"/dev/gpiochip0"
#define DEFAULT_ACT_LINE	25
#define DEFAULT_STIR_LINE	26
#define CONSUMER		"mashctld"

#define PREFIX "[gpio actuator plugin] "

struct s_gpio_act_cfg {
  unsigned int              line_offset[2];
  char                      devname[2][64];
  struct gpiod_chip         *chip;
  struct gpiod_line_request *request[2];
};

static struct s_gpio_act_cfg gpio_act_cfg;

extern bool actuator_simul[2];
extern void debug(char *fmt, ...);
extern void errorlog(char *fmt, ...);

// Open chip on first call
static struct gpiod_chip *get_chip(void) {
  if (!gpio_act_cfg.chip) {
    gpio_act_cfg.chip = gpiod_chip_open(GPIO_CHIP);
    if (!gpio_act_cfg.chip)
      errorlog(PREFIX "unable to open GPIO chip " GPIO_CHIP "\n");
  }
  return gpio_act_cfg.chip;
}

static struct gpiod_line_request *request_output_line(struct gpiod_chip *chip,
                                                      unsigned int offset) {
  struct gpiod_line_settings  *settings = NULL;
  struct gpiod_line_config    *line_cfg = NULL;
  struct gpiod_request_config *req_cfg  = NULL;
  struct gpiod_line_request   *request  = NULL;
  unsigned int offsets[1] = { offset };

  settings = gpiod_line_settings_new();
  if (!settings)
    goto out;
  gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
  gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

  line_cfg = gpiod_line_config_new();
  if (!line_cfg)
    goto out;
  if (gpiod_line_config_add_line_settings(line_cfg, offsets, 1, settings) < 0)
    goto out;

  req_cfg = gpiod_request_config_new();
  if (!req_cfg)
    goto out;
  gpiod_request_config_set_consumer(req_cfg, CONSUMER);

  request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

out:
  gpiod_request_config_free(req_cfg);
  gpiod_line_config_free(line_cfg);
  gpiod_line_settings_free(settings);
  return request;
}

void actuator_initfunc(char *cfgfile, int devno) {
  struct gpiod_chip *chip;

  debug(PREFIX "actuator_initfunc device %d\n", devno);

  // GPIO line Numbers to use for heating and stirring device
  if (devno == 0) {
    gpio_act_cfg.line_offset[0] = (unsigned int)ini_getl(
        "actuator_plugin_gpio", "actuator_line",
        DEFAULT_ACT_LINE, cfgfile);
  } else {
    gpio_act_cfg.line_offset[1] = (unsigned int)ini_getl(
        "actuator_plugin_gpio", "stirring_line",
        DEFAULT_STIR_LINE, cfgfile);
  }

  snprintf(gpio_act_cfg.devname[devno], sizeof(gpio_act_cfg.devname[devno]),
           GPIO_CHIP ":%u", gpio_act_cfg.line_offset[devno]);

  // open chip
  chip = get_chip();
  if (!chip) {
    errorlog(PREFIX "falling back to simulation mode\n");
    actuator_simul[devno] = true;
    plugin_error[devno]   = IOERROR;
    return;
  }

  // request gpio line as output
  gpio_act_cfg.request[devno] =
      request_output_line(chip, gpio_act_cfg.line_offset[devno]);

  if (!gpio_act_cfg.request[devno]) {
    errorlog(PREFIX "unable to request line %u on " GPIO_CHIP "\n",
             gpio_act_cfg.line_offset[devno]);
    errorlog(PREFIX "falling back to simulation mode\n");
    actuator_simul[devno] = true;
    plugin_error[devno]   = IOERROR;
    return;
  }
}

void actuator_setstate(int devno, int state) {
  debug(PREFIX "setting device %d to %d\n", devno, state);
  gpiod_line_request_set_value(
      gpio_act_cfg.request[devno],
      gpio_act_cfg.line_offset[devno],
      state ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

void fill_dirlist(size_t max, char *dirlist) {
  struct gpiod_chip      *chip;
  struct gpiod_chip_info *chip_info;
  struct gpiod_line_info *line_info;
  unsigned int num_lines, i;
  size_t pos = 0, space = max, num;
  bool overflow = false;

  chip = gpiod_chip_open(GPIO_CHIP);
  if (!chip) {
    dirlist[0] = '\0';
    return;
  }

  chip_info = gpiod_chip_get_info(chip);
  if (!chip_info) {
    gpiod_chip_close(chip);
    dirlist[0] = '\0';
    return;
  }

  num_lines = gpiod_chip_info_get_num_lines(chip_info);

  for (i = 0; i < num_lines; i++) {
    line_info = gpiod_chip_get_line_info(chip, i);
    if (!line_info)
      continue;

    if (gpiod_line_info_get_direction(line_info)
            != GPIOD_LINE_DIRECTION_OUTPUT) {
      gpiod_line_info_free(line_info);
      continue;
    }

    if (!overflow) {
      num = snprintf(dirlist + pos, space,
                     "\"" GPIO_CHIP ":%u\",", i);
      if (num >= space)
        overflow = true;
      pos   += num;
      space -= num;
    }
    gpiod_line_info_free(line_info);
  }

  gpiod_chip_info_free(chip_info);
  gpiod_chip_close(chip);

  if (pos > 0 && !overflow)
    dirlist[pos - 1] = '\0';   // remove trailing comma
  else
    dirlist[0] = '\0';
}

size_t actuator_getInfo(int devno, size_t max, char *buf) {
  size_t pos, rest;
  char dirlist[1024];

  debug(PREFIX "actuator_getInfo called\n");

  fill_dirlist(1024, dirlist);

  pos = snprintf(buf, max,
    "  {\n"
    "    \"type\": \"actuator\",\n"
    "    \"name\": \"%s\",\n"
    "    \"device\": \"%s\",\n"
    "    \"error\": \"%d\",\n",
    plugin_name, gpio_act_cfg.devname[devno], plugin_error[devno]);
  if (pos >= max) { buf[0] = '\0'; return 0; }

  rest = max - pos;
  pos += snprintf(buf + pos, rest,
    "    \"devlist\": [%s],\n"
    "    \"options\": []\n",
    dirlist);
  if (pos >= max) { buf[0] = '\0'; return 0; }

  rest = max - pos;
  pos += snprintf(buf + pos, rest, "  }\n");
  if (pos >= max) buf[0] = '\0';

  return strlen(buf);
}
