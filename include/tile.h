/* Copyright (C) 2014 James Smith <james@theta.pw>
 * See the license.txt file
 */

#ifndef TILE_H_
#define TILE_H_

#include <pthread.h>

#define TILES 16

enum tile_dir {
	UP,
	DOWN,
	LEFT,
	RIGHT,
};

struct tile_game {
	pthread_mutex_t lock;
	unsigned int seedp;
	unsigned int max_tile;
	unsigned int moves;	/* tiles created */
	unsigned int score;

	unsigned int *tiles;	/* array of tiles, TILES long */
};

int init_tiles (struct tile_game *);
int move_tiles (struct tile_game *, enum tile_dir);
void destroy_tiles (struct tile_game *);

#endif /* 2048_TILE_H_ */
