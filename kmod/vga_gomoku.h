#ifndef _vga_gomoku_H
#define _vga_gomoku_H

#include <linux/ioctl.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint16_t params[8];
} vga_gomoku_arg_t;


#define VGA_GOMOKU_MAGIC 'q'
#define VGA_DRIVER_FILENAME "/dev/vga_ball"

/* ioctls and their arguments */
#define VGA_GOMOKU_WRITE _IOW(VGA_GOMOKU_MAGIC, 1, vga_gomoku_arg_t *)
#define VGA_GOMOKU_READ  _IOR(VGA_GOMOKU_MAGIC, 2, vga_gomoku_arg_t *)

#endif
