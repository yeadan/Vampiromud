/****************************************************************************
 *                       [M]ulti [U]ser [D]ungeon                           *
 ****************************************************************************
 * ------------------------------------------------------------------------ *
 * [C]achondeo  [O]rganizado    [L]ibre [I]ndependiente [K][O]jonudamente   *
 * [T]aleguero  [R]eivindicativo [O]scuramente [N]ocivo ***DANGER***        *
 * ------------------------------------------------------------------------ *
 * COLIKOTRON 1.0 No(C) 199x, 200x   por: La "gresca"                       *
 * ------------------------------------------------------------------------ *
 * Colikotron Code Team: SaNgUiNaRi BaSTaRDo, Kayser NaZi, SiGo RaLLaoR     *
 * Lume PuTa, AlDuS CaBRoN. 199x, 200x                                      *
 * ------------------------------------------------------------------------ *
 * Colikotron 's   Builders   Team :  Gnorc  Melon,     Thesap  Sapo Angel  *
 * Tirkov "Mama un bocata Nocilla", Rei Gei, MiDo FuMeTa, WaDe "Enviame al  *
 * guardian de humo", Somm gei de entre los geis,  Heinrich  "enviame  el   *
 * help.are", Sirius margaritas, Lume Puta, SaNgUi "No estoy",   Kayser     *
 * SiGo "Reclamaciones dirijirse a mi escroto", y seguro que algun pillao + *
 * que se nos olvida ( secuelas del hatxis ) No(C) 199x, 200x   Colikotron  *
 * ------------------------------------------------------------------------ *
 * Agradecimientos  especiales a: Reena ke no entra xD, Vidal por el apoyo  *
 * logistico, Satai por sus inestimables consejos, Txako por ser el primer  *
 * jugador que llego a Avatar, A Estel por ser apa apa, a FRENX por darnos  *
 * alguna que otra calada de su Marihuana xD, al  Consejo  de Mortales por  *
 * dejarse rallar  en nuestro nombre, a Cervezas  Aguila por que si  no la  *
 * cagariamos.... o no XD                                                   *
 ****************************************************************************
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *         [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame            *
 *         SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider            *
 ****************************************************************************
 *         Modulo para sistema de notas                                     *
 *             No soportado por SMAUG1.4 ni superiores                      *
 *             No soportado por ningun otro tipo de codigo xD               *
 ****************************************************************************
 *   Modificado de un snippet de Erwin S. Andreasen, erwin@andreasen.org    *
 ***************************************************************************/
/* Includes for board system */
/* This is version 2 of the board system, (c) 1995-96 erwin@andreasen.org */


/*#define CON_NOTE_TO					10
#define CON_NOTE_SUBJECT				11
#define CON_NOTE_EXPIRE					12
#define CON_NOTE_TEXT					13
#define CON_NOTE_FINISH					14*/

#define NOTE_DIR  				"../notas/" /* set it to something you like */

#define DEF_NORMAL  0 /* No forced change, but default (any string)   */
#define DEF_INCLUDE 1 /* 'names' MUST be included (only ONE name!)    */
#define DEF_EXCLUDE 2 /* 'names' must NOT be included (one name only) */
#define MAX_BOARD 	  8

#define DEFAULT_BOARD 0 /* default board is board #0 in the boards      */
                        /* It should be readable by everyone!           */

#define MAX_LINE_LENGTH 80 /* enforce a max length of 80 on text lines, reject longer lines */
						   /* This only applies in the Body of the note */

#define MAX_NOTE_TEXT (4*MAX_STRING_LENGTH - 1000)

#define BOARD_NOTFOUND -1 /* Error code from board_lookup() and board_number */

/* Data about a board */
struct nota_data
{
	char *short_name; /* Max 8 chars */
	char *long_name;  /* Explanatory text, should be no more than 40 ? chars */

	int read_level; /* minimum level to see board */
	int write_level;/* minimum level to post notes */

	char *names;       /* Default recipient */
	int force_type; /* Default action (DEF_XXX) */

	int purge_days; /* Default expiration */

	/* Non-constant data */

	NOTE_DATA *note_first; /* pointer to board's first note */
	bool changed; /* currently unused */

};

typedef struct nota_data NOTA_DATA;


/* External variables */

extern NOTA_DATA boards[MAX_BOARD]; /* Declare */


/* Prototypes */

void finish_note (NOTA_DATA *board, NOTE_DATA *note); /* attach a note to a board */
void free_note   (NOTE_DATA *note); /* deallocate memory used by a note */
void load_todas (void); /* load all boards */
int board_lookup (const char *name); /* Find a board with that name */
bool is_note_to (CHAR_DATA *ch, NOTE_DATA *note); /* is tha note to ch? */
void personal_message (const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void make_note (const char* board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void save_notes ();

/* for nanny */
void handle_con_note_to 		(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_subject 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_expire 	(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_text 		(DESCRIPTOR_DATA *d, char * argument);
void handle_con_note_finish 	(DESCRIPTOR_DATA *d, char * argument);


/* Commands */

DECLARE_DO_FUN (do_nota		);
DECLARE_DO_FUN (do_apartados	);
