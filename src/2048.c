/* Copyright (C) 2014 James Smith <james@theta.pw>
 * See the license.txt file
 */

#include <ctype.h>
#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

#include "tile.h"

static struct tile_game game;

int
ask_yn (const char *prompt)
{
	printf ("\n%s ", prompt);
	fflush (NULL);

	getchar ();
	switch (toupper(getchar())) {
	case 'N':
		return 0;
	}

	return 1;
}

void
draw_game (void)
{
	char *lines = "\n\t____ ____ ____ ____\n";

	pthread_mutex_lock (&game.lock);
	printf ("\n\nScore: %d\tMax tile: %d\tCreated: %d%s",
			game.score, game.max_tile, game.moves, lines);

	for (int i = 0; i < TILES; i++)
		printf ("%s%4d", (i && i % 4)?" ":"\n\t", game.tiles[i]);

	puts(lines);
	pthread_mutex_unlock (&game.lock);
}

int
main (void)
{
	bool win_condition = false;
	int ret, ch;

	puts ("Playing 2048 " VERSION " in ASCII mode");

	init_tiles (&game);

	ch = ret = 0;
	do {
		draw_game ();
		printf ("move [wasd]: ");
		fflush (NULL);

		if (ch == 0)
			continue;

		enum tile_dir dir = -1;

		switch (ch) {
		case 'W': dir = UP;	break;
		case 'A': dir = LEFT;	break;
		case 'S': dir = DOWN;	break;
		case 'D': dir = RIGHT;	break;
		default: continue;
		}

		if (move_tiles (&game, dir) == -1)
			break;

		if (game.max_tile == 2048 && !win_condition) {
			win_condition = 1;
			if (!ask_yn ("You won! Do you wish to continue? [Y/n]"))
				break;
		}

	} while ((ch = toupper(getchar())) != EOF);

	destroy_tiles (&game);

	printf ("\nScore: %d\n", game.score);
	if (game.max_tile < 2048) {
		puts ("You lost. Better luck next time\n");
		return 1;
	}

	return 0;
}
