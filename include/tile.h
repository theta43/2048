#ifndef TILE_H
#define TILE_H_

#include <pthread.h>

#define TILES 16

enum tile_dir {
	UP,
	DOWN,
	LEFT,
	RIGHT,
};

struct game {
	pthread_mutex_t lock;
	unsigned int seedp;
	unsigned int max_tile;
	unsigned int moves;
	unsigned int score;

	unsigned int *tiles;
};

int init_tiles (struct game *);
void destroy_tiles (struct game *);
int move_tiles (struct game *, enum tile_dir);

#endif
