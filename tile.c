/* Copyright (C) 2014 James Smith <james@theta.pw>
 * See the license.txt file
 */

#include <err.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include "tile.h"

/* mod is an array of pointers that is modified to suit the direction
 * the player wants to move in.
 *
 * This allows us to design the algorithm for only one direction, not 4.
 *
 * All player movements are treated as 'left', so we assign the values in
 * the array to make that possible. (Explained better below)
 */

/* XXX Because of this variable, the 'library' is NOT reentrant.
 * Move mod to the tile_game struct if you want that.
 */
static unsigned int *mod[TILES];

/*
 * Removes zero spaces between tiles on a single row.
 */
static void
compress_row (size_t row)
{
	short first_zero = -1, first_non_zero = -1;

	if (row > 3)
		return;

	for (int i = 0; i < 4; i++) {

		/* Find first zero */
		if (*mod[row*4 +i] == 0 && first_zero < 0)
			first_zero = i;

		/* Find first non-zero after the first zero */
		if (*mod[row*4 +i] == 0 || first_zero < 0)
			continue;

		first_non_zero = i;
		break;
	}

	/* Either:
	 * 1) All zeros, i.e. ( 0, 0, 0, 0)
	 * 2) All non-zeros, e.g. ( 2, 2, 2, 8)
	 * 3) No non-zeros after a zero, e.g. ( 8, 0, 0, 0)
	 */
	if (first_non_zero < 0 || first_zero < 0)
		return;

	for (int i = 0; i+first_non_zero < 4; i++) {
		int tmp;
		size_t a, b;

		a = row*4 +first_non_zero +i;
		b = row*4 +first_zero +i;

		tmp = *mod[a];
		*mod[a] = *mod[b];
		*mod[b] = tmp;
	}

	/* Repeat until finished */
	compress_row (row);
}

/*
 * Create a new tile in a random empty location
 * with a starting value of { 2, 4 }
 */
static int
new_tile (struct tile_game *pgame)
{
	unsigned char empty[16];
	size_t i, j, loc;

	pthread_mutex_lock (&pgame->lock);

	for (i = 0, j = 0; i < TILES; i++) {
		if (pgame->tiles[i])
			continue;
		empty[j++] = i;
	}

	if (j == 0)
		return -1;

	loc = empty[rand_r(&pgame->seedp) % j];

	/* Even distribution of { 2, 4 } */
	pgame->tiles[loc] = ((rand_r(&pgame->seedp) & 1) +1) * 2;

	pgame->score += pgame->tiles[loc];
	pgame->moves++;

	pthread_mutex_unlock (&pgame->lock);

	return pgame->tiles[loc];
}

/*
 * Creates two initital game tiles.
 */
int
init_tiles (struct tile_game *pgame)
{
	pthread_mutex_init (&pgame->lock, NULL);
	pthread_mutex_lock (&pgame->lock);

	pgame->seedp = time(NULL);
	pgame->max_tile = 0;
	pgame->moves = 0;
	pgame->score = 0;

	pgame->tiles = calloc (TILES, sizeof(*pgame->tiles));

	if (!pgame->tiles) {
		destroy_tiles (pgame);
		err (2, "Unable to allocate %lu bytes of memory: %s %d\n",
				TILES * sizeof(*pgame->tiles),
				__FILE__, __LINE__);
	}

	pthread_mutex_unlock (&pgame->lock);

	new_tile (pgame);
	new_tile (pgame);

	return 1;
}

void
destroy_tiles (struct tile_game *pgame)
{
	pthread_mutex_destroy (&pgame->lock);
	free (pgame->tiles);
}

/*
 * Move all tiles in a direction, combining any tiles of the same number.
 * Then create a new tile in an empty place.
 * If no pieces can be moved, and creating a new tile fails,
 * then the game is over.
 */
int
move_tiles (struct tile_game *pgame, enum tile_dir dir)
{
	if (!pgame || !pgame->tiles) {
		errx (2, "Must initialize the game first");
	}

	pthread_mutex_lock (&pgame->lock);

	/* Re-align tiles so all operations are 'left' operations
	 *
	 * Essentially, we're rotating the matrix by 90^ when moving down,
	 * and -90^ when moving up. Moving right we flip the matrix over the y
	 * axis.
	 * Left is the normal, so we keep the tile positions the same
	 */
	switch (dir) {
	case UP:
		for (int i = 0; i < 4; i++) {
			mod[i*4 +0] = &pgame->tiles[0*4 +i];
			mod[i*4 +1] = &pgame->tiles[1*4 +i];
			mod[i*4 +2] = &pgame->tiles[2*4 +i];
			mod[i*4 +3] = &pgame->tiles[3*4 +i];
		}
		break;
	case DOWN:
		for (int i = 0; i < 4; i++) {
			mod[i*4 +0] = &pgame->tiles[3*4 +i];
			mod[i*4 +1] = &pgame->tiles[2*4 +i];
			mod[i*4 +2] = &pgame->tiles[1*4 +i];
			mod[i*4 +3] = &pgame->tiles[0*4 +i];
		}
		break;
	case LEFT:
		for (int i = 0; i < TILES; i++) {
			mod[i] = &pgame->tiles[i];
		}
		break;
	case RIGHT:
		for (int i = 0; i < 4; i++) {
			mod[i*4 +0] = &pgame->tiles[i*4 +3];
			mod[i*4 +1] = &pgame->tiles[i*4 +2];
			mod[i*4 +2] = &pgame->tiles[i*4 +1];
			mod[i*4 +3] = &pgame->tiles[i*4 +0];
		}
		break;
	}

	/* We only check each tile once. E.g.
	 * ( 2, 2, 2, 2 ) -> ( 4, 0, 2, 2 ) ->
	 * ( 4, 2, 2, 0 ) -> ( 4, 4, 0, 0 ).
	 */
	for (int i = 0; i < TILES; i++) {
		compress_row (i/4);

		if ((i % 4 == 3) || (*mod[i] != *mod[i+1]))
			continue;

		*mod[i] *= 2;
		*mod[i+1] = 0;

		pgame->score += *mod[i];

		if (*mod[i] > pgame->max_tile)
			pgame->max_tile = *mod[i];
	}

	pthread_mutex_unlock (&pgame->lock);

	return new_tile (pgame);
}
