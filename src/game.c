#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"
#include "utils.h"

/*
 * Returns the player that is playing
 */
Player * currentPlayer ()
{
	if (game.current == PLAYER1)
		return game.player1;
	else
		return game.player2;
}

/*
 * Returns the player that is not playing
 */
Player * otherPlayer ()
{
	if (game.current == PLAYER1)
		return game.player2;
	else
		return game.player1;
}

/*
 * Initialises the game
 */
void initGame ()
{
	int i, j;

	game.round = 0;

	game.lastHit.letter = 0;
	game.lastHit.number = 0;

	game.player1 = malloc (sizeof(Player));
	game.player2 = malloc (sizeof(Player));
	game.current = PLAYER1;
	game.winner	 = NONE;

	game.player1->board	 = malloc (sizeof(Board));
	game.player1->id	 = PLAYER1;
	game.player1->played = 0;

	game.player2->board	 = malloc (sizeof(Board));
	game.player2->id	 = PLAYER2;
	game.player2->played = 0;

	initPlaceableShips (game.player1);
	initPlaceableShips (game.player2);

	for (i = 0; i < BOARD_SIZE; i++)
	{
		for (j = 0; j < BOARD_SIZE; j++)
		{
			game.player1->board->ship[i][j] = NO_SHIP;
			game.player2->board->ship[i][j] = NO_SHIP;
			game.player1->board->flag[i][j] = NO_FLAG;
			game.player2->board->flag[i][j] = NO_FLAG;
			game.player1->board->shot[i][j] = NO_SHOT;
			game.player2->board->shot[i][j] = NO_SHOT;
		}
	}

	srand (time (NULL));

	placeShipsRandomly (game.player1);
	placeShipsRandomly (game.player2);
}	/* initGame */

/*
 * Quits the game and frees memory
 */
void quitGame ()
{
	free (game.player1->board);
	free (game.player2->board);
	free (game.player1);
	free (game.player2);
}

/*
 * Returns a value according to if ship is placeable
 *
 * 0 : Ship is placeable
 * 1 : Ship blocked
 * 2 : Parameters error
 *
 */
int shipIsNotPlaceable (Player * player, Coordinates pos, Direction direction, ShipType shipType)
{
	int length = getShipLength (shipType);
	int i;

	if (player == NULL)
	{
		printf ("[DEBUG] Player is NULL\n");
		return 2;
	}

	if (!positionIsInBound (pos))
	{
		printf ("[DEBUG] position is out of bounds\n");
		return 2;
	}

	if (!directionIsInBound (direction))
	{
		printf ("[DEBUG] direction is out of bounds\n");
		return 2;
	}

	if (!shipTypeIsInBound (shipType))
	{
		printf ("[DEBUG] shipType is out of bounds\n");
		return 2;
	}

	if (player->placeableShips[shipType - 1] <= 0)
	{
		printf ("[DEBUG] shipType %d is not availailable (%d)\n", shipType, player->placeableShips[shipType - 1]);
		return 2;
	}

	if (direction == BAS && pos.number + length > BOARD_SIZE + 1)
	{
		return 1;
	}

	if (direction == GAUCHE && pos.letter - length < 0)
	{
		return 1;
	}

	if (direction == HAUT && pos.number - length < 0)
	{
		return 1;
	}

	if (direction == DROITE && pos.letter + length > BOARD_SIZE + 1)
	{
		return 1;
	}

	for (i = 0; i < length; i++)
	{
		if (direction == BAS && player->board->ship[pos.number - 1 + i][pos.letter - 1] != 0)
			return 1;

		if (direction == GAUCHE && player->board->ship[pos.number - 1][pos.letter - 1 - i] != 0)
			return 1;

		if (direction == HAUT && player->board->ship[pos.number - 1 - i][pos.letter - 1] != 0)
			return 1;

		if (direction == DROITE && player->board->ship[pos.number - 1][pos.letter - 1 + i] != 0)
			return 1;
	}

	return 0;
}	/* shipIsNotPlaceable */

/* Init all ships placeability to 1
 *
 * TODO : Make it configurable
 */
void initPlaceableShips (Player * player)
{
	int i;

	for (i = 0; i < N_SHIPS; i++)
		player->placeableShips[i] = 1;
}

/*
 * Sets the other player as the one playing
 *
 * Increase the round when both players have played
 */
void nextPlayer ()
{
	currentPlayer()->played = 1;

	if (otherPlayer()->played)
	{
		currentPlayer()->played = 0;
		otherPlayer()->played	= 0;
		game.round++;
	}

	if (game.current == PLAYER1)
		game.current = PLAYER2;
	else
		game.current = PLAYER1;
}

/* Returns if all the ships of a player are placed
 *
 * Return values :
 * 0: Ships are remaining
 * 1: All ships have been placed
 */
int allShipsPlaced (Player * player)
{
	int i;

	for (i = 0; i < N_SHIPS; i++)
		if (player->placeableShips[i] > 0)
			return 0;

	return 1;
}

/* Shoot the selected player
 *
 * Return values :
 * 0: Ship successfully placed
 * 1: Ship blocked
 * 2: Parameters error
 *
 */
int placeShip (Player * player, Coordinates pos, Direction direction, ShipType shipType)
{
	int notPlaceable = shipIsNotPlaceable (player, pos, direction, shipType);

	if (notPlaceable)
		return notPlaceable;

	int i, j, k;
	int length = getShipLength (shipType);

	i = pos.number - 1;
	j = pos.letter - 1;

	for (k = 0; k < length; k++)
	{
		player->board->ship[i][j] = shipType;

		switch (direction)
		{
			case BAS:
				i++;
				break;
			case GAUCHE:
				j--;
				break;
			case HAUT:
				i--;
				break;
			case DROITE:
				j++;
				break;
		}
	}

	player->placeableShips[shipType - 1]--;

	return 0;
}	/* placeShip */

/*
 * Places all remaining ship of a player randomly
 */
void placeShipsRandomly (Player * player)
{
	int i;
	Coordinates pos;

	while (!allShipsPlaced (player))
		for (i = 1; i <= N_SHIPS;)
		{
			pos.number = randInBounds (1, BOARD_SIZE);
			pos.letter = randInBounds (1, BOARD_SIZE);

			if (placeShip (player, pos, randInBounds (0, 3), i) != 1)
				i++;
		}
}

/*
 * Returns the length of the ship from its type
 */
int getShipLength (ShipType shipType)
{
	switch (shipType)
	{
		case CARRIER:
			return 5;

		case BATTLESHIP:
			return 4;

		case CRUISER:
		case SUBMARINE:
			return 3;

		case DESTROYER:
			return 2;

		default:
			return 0;
	}
}

/*
 * Returns if the player is the one playing
 *
 * Return values :
 * 0: Not playing
 * 1: Playing
 */
int isPlaying (Player * player)
{
	if (game.current == player->id)
		return 1;

	return 0;
}

/* Shoot the selected player
 *
 * Return values :
 * NO_SHOT: Hit an already hit location or another error
 * MISSED:  Missed
 * HIT:     Hit
 * FLOWED:  Flowed
 */
ShotType shootPlayer (Player * player, Coordinates pos)
{
	if (player == NULL)
	{
		printf ("[DEBUG] Player is NULL\n");
		return NO_SHOT;
	}

	if (!positionIsInBound (pos))
	{
		printf ("[DEBUG] position is out of bounds\n");
		return NO_SHOT;
	}

	if (player->board->shot[pos.number - 1][pos.letter - 1] != NO_SHOT)
		return NO_SHOT;

	game.lastHit = pos;

	if (player->board->ship[pos.number - 1][pos.letter - 1] != NO_SHIP)
	{
		player->board->shot[pos.number - 1][pos.letter - 1] = HIT;
		return HIT;
	}

	player->board->shot[pos.number - 1][pos.letter - 1] = MISSED;

	return MISSED;
}

int gameIsOver ()
{
	int i, j;

	for (i = 0; i < BOARD_SIZE; i++)
		for (j = 0; j < BOARD_SIZE; j++)
			if (currentPlayer()->board->ship[i][j] != 0 && currentPlayer()->board->shot[i][j] != HIT && currentPlayer()->board->shot[i][j] != FLOWED)
				return 0;

	game.winner = otherPlayer()->id;

	return 1;
}