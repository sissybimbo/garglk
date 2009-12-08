/* utils.c --- General utility functions
 * (C) 2008 Stuart Allen, distribute and use 
 * according to GNU GPL, see file COPYING for details.
 */


#include "jacl.h"
#include "language.h"
#include "types.h"
#include "prototypes.h"
#include <string.h>

extern char						function_name[];

extern struct object_type		*object[];
extern struct variable_type		*variable[];

extern int						objects;
extern int						player;

extern char						game_file[];
extern char						game_path[];
extern char						prefix[];
extern char						blorb[];
extern char   			        bookmark[];
extern char            			walkthru[];
extern char						include_directory[];
extern char            			temp_directory[];
extern char            			temp_buffer[];

void
eachturn()
{
	/* INCREMENT THE TOTAL NUMBER OF MOVES MADE AND CALL THE 'EACHTURN'
	 * FUNCTION FOR THE CURRENT LOCATION AND THE GLOBAL 'EACHTURN'
	 * FUNCTION. THESE FUNCTIONS CONTAIN ANY CODE THAT SIMULATED EVENTS
	 * OCCURING DUE TO THE PASSING OF TIME */
	TOTAL_MOVES->value++;
	execute("+eachturn");
	strcpy(function_name, "eachturn_");
	strcat(function_name, object[HERE]->label);
	execute(function_name);
	execute("+system_eachturn");
	
	/* SET TIME TO FALSE SO THAT NO MORE eachturn FUNCTIONS ARE EXECUTED
	 * UNTIL EITHER THE COMMAND PROMPT IS RETURNED TO (AND TIME IS SET
	 * TO TRUE AGAIN, OR IT IS MANUALLY SET TO TRUE BY A VERB THAT CALLS
	 * MORE THAN ONE proxy COMMAND. THIS IS BECAUSE OTHERWISE A VERB THAT 
	 * USES A proxy COMMAND TO TRANSLATE THE VERB IS RESULT IN TWO MOVES
	 * (OR MORE) HAVING PASSED FOR THE ONE ACTION. */
	TIME->value = FALSE;
}

int
get_here()
{
	/* THIS FUNCTION RETURNS THE VALUE OF 'here' IN A SAFE, ERROR CHECKED
	 * WAY */
	if (player < 1 || player > objects) {
		badplrrun(player);
		terminate(44);
	} else if (object[player]->PARENT < 1 || object[player]->PARENT> objects || object[player]->PARENT== player) {
		badparrun();
		terminate(44);
	} else {
		return (object[player]->PARENT);
	}

	/* SHOULDN'T GET HERE, JUST TRYING TO KEEP VisualC++ HAPPY */
	return 1;
}

char *
strip_return (string)
	char *string;
{
	/* STRIP ANY TRAILING RETURN OR NEWLINE OFF THE END OF A STRING */
	int index;
	int length = strlen(string);

	for (index = 0; index < length; index++) {
		switch (string[index]) {
		case '\r':
		case '\n':
			string[index] = 0;
			break;
		}
	}

	return (char *) string;
}

int
random_number()
{
	/* GENERATE A RANDOM NUMBER BETWEEN 0 AND THE CURRENT VALUE OF
	 * THE JACL VARIABLE MAX_RAND */

	return (1 + (int) ((float) MAX_RAND->value * rand() / (RAND_MAX + 1.0)));
}

void
create_paths(full_path)
	char		*full_path;
{
	int				index;
	char           *last_slash;

	/* SAVE A COPY OF THE SUPPLIED GAMEFILE NAME */
	strcpy(game_file, full_path);

	/* FIND THE LAST SLASH IN THE SPECIFIED GAME PATH AND REMOVE THE GAME
	 * FILE SUFFIX IF ANY EXISTS */
	last_slash = (char *) NULL;
	
	/* GET A POINTER TO THE LAST SLASH IN THE FULL PATH */
	last_slash = strrchr(full_path, DIR_SEPARATOR);

	for (index = strlen(full_path); index >= 0; index--) {
		if (full_path[index] == DIR_SEPARATOR){
			/* NO '.' WAS FOUND BEFORE THE LAST SLASH WAS REACHED,
			 * THERE IS NO FILE EXTENSION */
			break;
		} else if (full_path[index] == '.') {
			full_path[index] = 0;
			break;
		}
	}

	/* STORE THE GAME PATH AND THE GAME FILENAME PARTS SEPARATELY */
	if (last_slash == (char *) NULL) {
		/* GAME MUST BE IN CURRENT DIRECTORY SO THERE WILL BE NO GAME PATH */
		strcpy(prefix, full_path);
		game_path[0] = 0;

		/* THIS ADDITION OF ./ TO THE FRONT OF THE GAMEFILE IF IT IS IN THE
		 * CURRENT DIRECTORY IS REQUIRED TO KEEP Gargoyle HAPPY. */
		sprintf (temp_buffer, ".%c%s", DIR_SEPARATOR, game_file);
		strcpy (game_file, temp_buffer);
	} else {
		/* STORE THE DIRECTORY THE GAME FILE IS IN WITH THE TRAILING
		 * SLASH IF THERE IS ONE */
		last_slash++;
		strcpy(prefix, last_slash);
		*last_slash = '\0';
		strcpy(game_path, full_path);
	}

	/* SET DEFAULT WALKTHRU FILE NAME */
	sprintf(walkthru, "%s.walkthru", prefix);

	/* SET DEFAULT SAVED GAME FILE NAME */
	sprintf(bookmark, "%s.bookmark", prefix);

	/* SET DEFAULT BLORB FILE NAME */
	sprintf(blorb, "%s.blorb", prefix);

	/* SET DEFAULT FILE LOCATIONS IF NOT SET BY THE USER IN CONFIG */
	if (include_directory[0] == 0) {
		strcpy(include_directory, game_path);
		strcat(include_directory, INCLUDE_DIR);
	}

	if (temp_directory[0] == 0) {
		strcpy(temp_directory, game_path);
		strcat(temp_directory, TEMP_DIR);
	}
}

// THIS FUNCTION EXISTS PURELY FOR BACKWARD COMPATIBILITY
void
move_player()
{
	/* THIS FUNCTION IS USED TO ATTEMPT TO MOVE THE PLAYER IN ONE OF
	 * THE DIRECTIONS. AS WELL AS CHECKING THE VALUE OF THE DESTINATION
	 * IT CALLS THE 'MOVEMENT' FUNCTION FOR THE CURRENT LOCATION AND 
	 * THE GLOBAL 'MOVEMENT' FUNCTION TO SEE IF THE MOVE SHOULD BE 
	 * PREVENTED FROM OCCURING. THIS CODE IS CALLED AS A RESULT OF
	 * THE JACL COMMAND 'TRAVEL' */

	/* BUILD THE FUNCTION NAME OF THE ASSOCIATED MOVEMENT FUNCTION*/
	strcpy(function_name, "movement_");
	strcat(function_name, object[HERE]->label);

	/* CALL IT */
	if (execute(function_name) == TRUE) {
		/* IF THIS RETURNS TRUE, THE MOVE SHOULDN'T HAPPEN */
		return;
	}

	/* CALL THE GLOBAL MOVEMENT FUNCTION*/
	if (execute("+movement") == TRUE) {
		/* IF THIS RETURNS TRUE, THE MOVE SHOULDN'T HAPPEN */
		return;
	}

	if (DESTINATION->value) {
		/* THE DESINTATION IS VALID, SO MOVE THE PLAYER */
		object[player]->PARENT = DESTINATION->value;

		/* PRINT THE NEW LOCATION DECRIPTION */
		look_around();
	} else {
		/* THE MOVE CANT HAPPEN AS THERE IS NO EXIT IN THE
		 * SPECIFIED DIRECTION */
		TIME->value = FALSE;
		write_text(CANT_GO);
	}
}
