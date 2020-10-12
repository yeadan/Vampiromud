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
 *         Modulo para sistema Vampire Roll                                 *
 *             No soportado por SMAUG1.4 ni superiores                      *
 *             No soportado por ningun otro tipo de codigo xD               *
 ****************************************************************************
 *   Modificado de un snippet de Erwin S. Andreasen, erwin@andreasen.org    *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*

 Note Board system, (c) 1995-96 Erwin S. Andreasen, erwin@andreasen.org
 =====================================================================

 Basically, the notes are split up into several boards. The boards do not
 exist physically, they can be read anywhere and in any position.

 Each of the note boards has its own file. Each of the boards can have its own
 "rights": who can read/write.

 Each character has an extra field added, namele the timestamp of the last note
 read by him/her on a certain board.

 The note entering system is changed too, making it more interactive. When
 entering a note, a character is put AFK and into a special CON_ state.
 Everything typed goes into the note.

 For the immortals it is possible to purge notes based on age. An Archive
 options is available which moves the notes older than X days into a special
 board. The file of this board should then be moved into some other directory
 during e.g. the startup script and perhaps renamed depending on date.

 Note that write_level MUST be >= read_level or else there will be strange
 output in certain functions.

 Board DEFAULT_BOARD must be at least readable by *everyone*.

*/

/*
 * Adaptado y mejorado por COLIKOTRON Code team (c) 199x - 200x
 * ============================================================
 */


#define L_SUP (MAX_LEVEL - 1)


NOTA_DATA boards[MAX_BOARD] =
{

{ "General",  	"Discusion general",            0,     2,     "todos", DEF_INCLUDE,21, NULL, FALSE },
{ "Ideas",	  	"Ideas para mejorar el MUD",	 0,     2,     "todos", DEF_NORMAL, 60, NULL, FALSE },
{ "Anuncios", 	"Anuncios de los Inmortales",  0,     LEVEL_IMMORTAL, "todos", DEF_NORMAL, 60, NULL, FALSE },
{ "Bugs",	  	"Bugs, fallos, errores",		     0,     1,     "inmortal", DEF_NORMAL, 60, NULL, FALSE },
{ "Personal", 	"Mensajes personales",			 0,     1,     "todos", DEF_EXCLUDE,28, NULL, FALSE },
{ "Penalizaciones", "Penalizaciones a jugadores",       0,      LEVEL_IMMORTAL, "todos", DEF_NORMAL, 60, NULL, FALSE },
{ "Heroes",     "Apartado de mensajes entre heroes",     0,      LEVEL_AVATAR, "heroes", DEF_NORMAL, 60, NULL, FALSE },
{ "Familia", "Apartado de mensajes a la familia",       0,      2, "todos", DEF_NORMAL, 60, NULL, FALSE },
};

/* The prompt that the character is given after finishing a note with ~ or END */
const char * szFinishPrompt = "&w(&gC&w)&gontinue, &w(&gV&w)&ger, &w(&gE&w)&gnviar o &w(&gA&w)&gnular?";

long last_note_stamp = 0; /* To generate unique timestamps on notes */

#define BOARD_NOACCESS -1
#define BOARD_NOTFOUND -1

bool next_board(CHAR_DATA *ch);
/*bool chequeo_apartados(CHAR_DATA *ch);*/

/* recycle a note */
void free_nota(NOTE_DATA *note)
{
    if( note->sender )
        STRFREE( note->sender );
    if( note->to_list )
        STRFREE( note->to_list );
    if( note->subject )
        STRFREE( note->subject );
    if( note->date ) /* was note->datestamp for some reason */
        STRFREE( note->date  );
    if( note->text )
        STRFREE( note->text );

	note->next = note_free;
	note_free = note;
}

/* allocate memory for a new note or recycle */
NOTE_DATA *new_note ()
{
	NOTE_DATA *note;

	if (note_free)
	{
		note = note_free;
		note_free = note_free->next;
	}
	else
		CREATE( note, NOTE_DATA, 1 );

	/* Zero all the field - Envy does not gurantee zeroed memory */
	note->next = NULL;
	note->sender = NULL;
	note->expire = 0;
	note->to_list = NULL;
	note->subject = NULL;
	note->date = NULL;
	note->date_stamp = 0;
	note->text = NULL;

	return note;
}

/* append this note to the given file */
void append_note (FILE *fp, NOTE_DATA *note)
{
	fprintf (fp, "Enviar  %s~\n", note->sender);
	fprintf (fp, "Fecha    %s~\n", note->date);
	fprintf (fp, "Stamp   %ld\n", note->date_stamp);
	fprintf (fp, "Caduca  %ld\n", note->expire);
	fprintf (fp, "Para      %s~\n", note->to_list);
	fprintf (fp, "Motivo %s~\n", note->subject);
	fprintf (fp, "Texto\n%s~\n\n", note->text);
}

/* Save a note in a given board */
void finish_note (NOTA_DATA *board, NOTE_DATA *note)
{
	FILE *fp;
	NOTE_DATA *p;

	char filename[200];

 /* The following is done in order to generate unique date_stamps */

	if (last_note_stamp >= current_time)
		note->date_stamp = ++last_note_stamp;
	else
	{
	    note->date_stamp = current_time;
	    last_note_stamp = current_time;
	}

	if (board->note_first) /* are there any notes in there now? */
	{
		for (p = board->note_first; p->next; p = p->next )
			; /* empty */

		p->next = note;
	}
	else /* nope. empty list. */
		board->note_first = note;

	/* append note to note file */

	sprintf (filename, "%s%s", NOTE_DIR, board->short_name);

	fp = fopen (filename, "a");
	if (!fp)
	{
		bug ("Could not open one of the note files in append mode",0);
		board->changed = TRUE; /* set it to TRUE hope it will be OK later? */
		return;
	}

	append_note (fp, note);
	fclose (fp);
}

/* Find the number of a board */
int board_number (const NOTA_DATA *board)
{
	int i;

	for (i = 0; i < MAX_BOARD; i++)
		if (board == &boards[i])
			return i;

	return -1;
}

/* Find a board number based on  a string */
int board_lookup (const char *name)
{
	int i;

	for (i = 0; i < MAX_BOARD; i++)
		if (!str_cmp (boards[i].short_name, name))
			return i;

	return -1;
}

/* Remove list from the list. Do not free note */
void unlink_note (NOTA_DATA *board, NOTE_DATA *note)
{
	NOTE_DATA *p;

	if (board->note_first == note)
		board->note_first = note->next;
	else
	{
		for (p = board->note_first; p && p->next != note; p = p->next);
		if (!p)
			bug ("unlink_note: could not find note.",0);
		else
			p->next = note->next;
	}
}

/* Find the nth note on a board. Return NULL if ch has no access to that note */
NOTE_DATA* find_note (CHAR_DATA *ch, NOTA_DATA *board, int num)
{
	int count = 0;
	NOTE_DATA *p;

	for (p = board->note_first; p ; p = p->next)
			if (++count == num)
				break;

	if ( (count == num) && is_note_to (ch, p))
		return p;
	else
		return NULL;

}

/* save a single board */
void save_board (NOTA_DATA *board)
{
	FILE *fp;
	char filename[200];
	char buf[MAX_STRING_LENGTH];
	NOTE_DATA *note;

	sprintf (filename, "%s%s", NOTE_DIR, board->short_name);

	fp = fopen (filename, "w");
	if (!fp)
	{
		sprintf (buf, "Error writing to: %s", filename);
		bug (buf, 0);
	}
	else
	{
		for (note = board->note_first; note ; note = note->next)
			append_note (fp, note);

		fclose (fp);
	}
}

/* Show one not to a character */
void show_note_to_char (CHAR_DATA *ch, NOTE_DATA *note, int num)
{
	char buf[4*MAX_STRING_LENGTH];

	/* Ugly colors ? */
	sprintf (buf, "&g[&w%4d&g] %s:&w%s\n\r&gFecha:  &w%s\n\r&gPara &w%s\n\r"
	         "&g---------------------------------------------------------------------------\n\r"
	         "&w%s\n\r",
	         num, note->sender, note->subject,
	         note->date,
	         note->to_list,
	         note->text);

	send_to_char (buf,ch);
}

/* Save changed boards */
void save_notes ()
{
	int i;

	for (i = 0; i < MAX_BOARD; i++)
		if (boards[i].changed) /* only save changed boards */
			save_board (&boards[i]);
}

/* Load a single board */
void load_board (NOTA_DATA *board)
{
	FILE *fp, *fp_archive;
	NOTE_DATA *last_note;
	char filename[200];

	sprintf (filename, "%s%s", NOTE_DIR, board->short_name);

	fp = fopen (filename, "r");

	/* Silently return */
	if (!fp)
		return;

	/* Start note fetching. copy of db.c:load_notes() */

    last_note = NULL;

    for ( ; ; )
    {
        NOTE_DATA *pnote;
        char letter;

        do
        {
            letter = getc( fp );
            if ( feof(fp) )
            {
                fclose( fp );
                return;
            }
        }
        while ( isspace(letter) );
        ungetc( letter, fp );

        CREATE( pnote, NOTE_DATA, sizeof( *pnote ) );

        if ( str_cmp( fread_word( fp ), "enviar" ) )
            break;
        pnote->sender     = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "fecha" ) )
            break;
        pnote->date       = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "stamp" ) )
            break;
        pnote->date_stamp = fread_number( fp );

        if ( str_cmp( fread_word( fp ), "caduca" ) )
            break;
        pnote->expire = fread_number( fp );

        if ( str_cmp( fread_word( fp ), "para" ) )
            break;
        pnote->to_list    = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "motivo" ) )
            break;
        pnote->subject    = fread_string( fp );

        if ( str_cmp( fread_word( fp ), "texto" ) )
            break;
        pnote->text       = fread_string( fp );

        pnote->next = NULL; /* jic */

        /* Should this note be archived right now ? */

        if (pnote->expire < current_time)
        {
			char archive_name[200];

			sprintf (archive_name, "%s%s.old", NOTE_DIR, board->short_name);
			fp_archive = fopen (archive_name, "a");
			if (!fp_archive)
				bug ("Could not open archive boards for writing",0);
			else
			{
				append_note (fp_archive, pnote);
				fclose (fp_archive); /* it might be more efficient to close this later */
			}

			free_nota (pnote);
			board->changed = TRUE;
			continue;

        }


        if ( board->note_first == NULL )
            board->note_first = pnote;
        else
            last_note->next     = pnote;

        last_note         = pnote;
    }

    bug( "Load_notes: bad key word.", 0 );
    return; /* just return */
}

/* Initialize structures. Load all boards. */
void load_todas ()
{
	int i;

	for (i = 0; i < MAX_BOARD; i++)
		load_board (&boards[i]);
}

/* Returns TRUE if the specified note is address to ch */
bool is_note_to (CHAR_DATA *ch, NOTE_DATA *note)
{
	if (!str_cmp (ch->name, note->sender))
		return TRUE;

	if (is_full_name ("todos", note->to_list))
		return TRUE;

	if (IS_IMMORTAL(ch) && (
		is_full_name ("imm", note->to_list) ||
		is_full_name ("imms", note->to_list) ||
		is_full_name ("immortal", note->to_list) ||
		is_full_name ("inmortal", note->to_list) ||
		is_full_name ("dios", note->to_list) ||
		is_full_name ("dioses", note->to_list) ||
                is_full_name ("inmortales", note->to_list) ||
		is_full_name ("immortals", note->to_list)))
		return TRUE;

	if ((get_trust(ch) == MAX_LEVEL) && (
		is_full_name ("imp", note->to_list) ||
		is_full_name ("imps", note->to_list) ||
		is_full_name ("implementor", note->to_list) ||
		is_full_name ("implementors", note->to_list)))
		return TRUE;

	if (is_full_name (ch->name, note->to_list))
		return TRUE;

	/* Allow a note to e.g. 40 to send to characters level 40 and above */
	if (is_number(note->to_list) && get_trust(ch) >= atoi(note->to_list))
		return TRUE;

	return FALSE;
}

/* Return the number of unread notes 'ch' has in 'board' */
/* Returns BOARD_NOACCESS if ch has no access to board */
int unread_notes (CHAR_DATA *ch, NOTA_DATA *board)
{
	NOTE_DATA *note;
	time_t last_read;
	int count = 0;

	if (board->read_level > get_trust(ch))
		return BOARD_NOACCESS;

	last_read = ch->pcdata->last_note[board_number(board)];

	for (note = board->note_first; note; note = note->next)
		if (is_note_to(ch, note) && ((long)last_read < (long)note->date_stamp))
			count++;

	return count;
}

/*
 * COMMANDS
 */

/* Start writing a note */
void do_nwrite (CHAR_DATA *ch, char *argument)
{
	char *strtime;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) /* NPC cannot post notes */
		return;

	if (get_trust(ch) < ch->pcdata->board->write_level)
	{
		send_to_char ("&wNo puedes escribir notas en este apartado.\n\r",ch);
		return;
	}

	/* continue previous note, if any text was written*/
	if (ch->pcdata->in_progress && (!ch->pcdata->in_progress->text))
	{
		send_to_char ("&wEstabas &gEDITANDO &wuna &GNOTA&w escribe &gNOTA ESCRIBIR&w\n\r"
		              "para continuar editandola.\n\r\n\r",ch);
		free_nota (ch->pcdata->in_progress);
		ch->pcdata->in_progress = NULL;
	}


	if (!ch->pcdata->in_progress)
	{
		ch->pcdata->in_progress = new_note();
		ch->pcdata->in_progress->sender = str_dup (ch->name);

		/* convert to ascii. ctime returns a string which last character is \n, so remove that */
		strtime = ctime (&current_time);
		strtime[strlen(strtime)-1] = '\0';

		ch->pcdata->in_progress->date = str_dup (strtime);
	}

	act ( AT_PLAIN, "&g$n comienza a escribir una nota.", ch, NULL, NULL, TO_ROOM);

	/* Begin writing the note ! */
	sprintf (buf, "&wEstas &g%s &wuna nueva nota en el apartado &w%s&g.\n\r"
	              "Si usas tintin escribe #ver para anular los alias, si usas otro cliente, desconectalos!\n\r",
	               ch->pcdata->in_progress->text ? "continuando" : "escribiendo",
	               ch->pcdata->board->short_name);
	send_to_char (buf,ch);

	sprintf (buf, "&gDe: &w%s\n\r", ch->name);
	send_to_char (buf,ch);

	if (!ch->pcdata->in_progress->text) /* Are we continuing an old note or not? */
	{
		switch (ch->pcdata->board->force_type)
		{
		case DEF_NORMAL:
			sprintf (buf, "&gPresiona &wRETURN &gpara destinatario por defecto \"&w%s&g\" .\n\r",
					  ch->pcdata->board->names);
			break;
		case DEF_INCLUDE:
			sprintf (buf, "&gEl destinatario debe &wINCLUIR&g \"&w%s&g\". Si no sera añadido automaticamente.\n\r",
						   ch->pcdata->board->names);
			break;

		case DEF_EXCLUDE:
			sprintf (buf, "&x&gEl destinatario &wNO PUEDE SER&g: \"&w%s&g\".\n\r",
						   ch->pcdata->board->names);

			break;
		}

		send_to_char (buf,ch);
		send_to_char ("&gPara:&w ",ch);
                                                  
		ch->desc->connected = CON_NOTE_TO;
		/* nanny takes over from here */

	}
	else /* we are continuing, print out all the fields and the note so far*/
	{
		sprintf (buf, "&gPara: &w%s\n\r"
		              "&gCaduca: &w%s\n\r"
		              "&gMotivo: &w%s\n\r",
		               ch->pcdata->in_progress->to_list,
			       ctime(&ch->pcdata->in_progress->expire),
		               ch->pcdata->in_progress->subject);
		send_to_char (buf,ch);
		send_to_char ( "&gNota demasiado larga:&w\n\r" ,ch);
		send_to_char (ch->pcdata->in_progress->text,ch);




		send_to_char ("\n\r&gEscribe '&w~&g' o '&wEND&g' en una linea en blanco para finalizar la nota.\n\r"

"================================================================================&w\n\r",ch);


		ch->desc->connected = CON_NOTE_TEXT;

	}

}


/* Read next note in current group. If no more notes, go to next board */
void do_nread (CHAR_DATA *ch, char *argument)
{
	NOTE_DATA *p;
	int count = 0, number;
	time_t *last_note = &ch->pcdata->last_note[board_number(ch->pcdata->board)];

	if (!str_cmp(argument, "releer"))
	{ /* read last note again */

	}
	else if (is_number (argument))
	{
		number = atoi(argument);

		for (p = ch->pcdata->board->note_first; p; p = p->next)
			if (++count == number)
				break;

		if (!p || !is_note_to(ch, p))
			send_to_char ("No existe.\n\r",ch);
		else
		{
			show_note_to_char (ch,p,count);
			*last_note =  UMAX (*last_note, p->date_stamp);
		}
	}
	else /* just next one */
	{
		char buf[MAX_STRING_LENGTH];

		count = 1;
		for (p = ch->pcdata->board->note_first; p ; p = p->next, count++)
		{	
			if ((p->date_stamp > *last_note) && is_note_to(ch,p))
			{
				show_note_to_char (ch,p,count);
				/* Advance if new note is newer than the currently newest for that char */
				*last_note =  UMAX (*last_note, p->date_stamp);
				return;
			}
		}
                if (next_board (ch))
                {
                send_to_char( "&wNo hay nuevas notas en este apartado.\n\r", ch );
                sprintf (buf, "&wBuscando en el siguiente apartado, &g%s&w.\n\r", ch->pcdata->board->short_name);
           	do_nread(ch,null);
		}
      		else
                {
                /*chequeo_apartados(ch);*/
		sprintf (buf, "&wNo hay ninguna nota nueva.\n\r");
		}
      		send_to_char (buf,ch);		
	}
}


/* Remove a note */
void do_nremove (CHAR_DATA *ch, char *argument)
{
	NOTE_DATA *p;

	if (!is_number(argument))
	{
		send_to_char ("&wBorrar que nota?\n\r",ch);
		return;
	}

	p = find_note (ch, ch->pcdata->board, atoi(argument));
	if (!p)
	{
		send_to_char ("&wNo existe.\n\r",ch);
		return;
	}

	if (str_cmp(ch->name,p->sender) && (get_trust(ch) < MAX_LEVEL))
	{
		send_to_char ("&wNo estas autorizado para borrar esa nota.\n\r",ch);
		return;
	}

	unlink_note (ch->pcdata->board,p);
	free_nota (p);
	send_to_char ("Nota borrada!\n\r",ch);

	save_board(ch->pcdata->board); /* save the board */
}


/* List all notes or if argument given, list N of the last notes */
/* Shows REAL note numbers! */
void do_nlist(CHAR_DATA *ch, char *argument)
{
	int count= 0, show = 0, num = 0, has_shown = 0;
	time_t last_note;
	NOTE_DATA *p;
	char buf[MAX_STRING_LENGTH];


	if (is_number(argument))	 /* first, count the number of notes */
	{
		show = atoi(argument);

		for (p = ch->pcdata->board->note_first; p; p = p->next)
			if (is_note_to(ch,p))
				count++;
	}

	send_to_char ( "&gNotas en este apartado:\n\r"
	              "&g&g[&wNum&g]  &wAutor
Motivo\n\r&g==============================================================================&w\n\r",ch);

	last_note = ch->pcdata->last_note[board_number (ch->pcdata->board)];

	for (p = ch->pcdata->board->note_first; p; p = p->next)
	{
		num++;
		if (is_note_to(ch,p))
		{
			has_shown++; /* note that we want to see X VISIBLE note, not just last X */
			if (!show || ((count-show) < has_shown))
			{
				sprintf (buf, "&g[&w%2d&g] &w%c%-13s  %s\n\r",
				               num,
				               last_note < p->date_stamp ? '*' : ' ',
				               p->sender, p->subject);
				send_to_char (buf,ch);
			}
		}

	}
}

/* catch up with some notes */
void do_ncatchup (CHAR_DATA *ch, char *argument)
{
	NOTE_DATA *p;

	/* Find last note */
	for (p = ch->pcdata->board->note_first; p && p->next; p = p->next);

	if (!p)
		send_to_char ("&wNo hay notas en este apartado.\n\r",ch);
	else
	{
		ch->pcdata->last_note[board_number(ch->pcdata->board)] = p->date_stamp;
		send_to_char ("&WMarcando todas las notas como leidas.\n\r",ch);
	}
}

/* Dispatch function for backwards compatibility */
void do_nota (CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	argument = one_argument (argument, arg);

	if ((!arg[0]) || (!str_cmp(arg, "leer"))) /* 'note' or 'note read X' */
		do_nread (ch, argument);

	else if (!str_cmp (arg, "lista"))
		do_nlist (ch, argument);

	else if (!str_cmp (arg, "escribir"))
		do_nwrite (ch, argument);

	else if (!str_cmp (arg, "borrar"))
		do_nremove (ch, argument);

	else if (!str_cmp (arg, "purgar"))
		send_to_char ("Obsoleto.\n\r",ch);

	else if (!str_cmp (arg, "archivar"))
		send_to_char ("Obsoleto.\n\r",ch);

	else if (!str_cmp (arg, "catchup"))
		do_ncatchup (ch, argument);
	else
		do_help (ch, "nota");
}

/* Show all accessible boards with their numbers of unread messages OR
   change board. New board name can be given as a number or as a name (e.g.
    board personal or board 4 */
void do_apartados (CHAR_DATA *ch, char *argument)
{
	int i, count, number;
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;

	if (!argument[0]) /* show boards */
	{
		int unread;

		count = 1;
		send_to_char ( "&gNum     Nombre       Estado Descripcion\n\r"
		               "&g==== =============== ====== =====================\n\r",ch);
		for (i = 0; i < MAX_BOARD; i++)
		{
			unread = unread_notes (ch,&boards[i]); /* how many unread notes? */
			if (unread != BOARD_NOACCESS)
			{
				sprintf (buf,"&c[&w%2d&c] &w%15s &c[&w%s%4d&c] &w%s\n\r",
				                   count, boards[i].short_name, unread ? "&r" : "&g",
				                    unread, boards[i].long_name);
				send_to_char (buf,ch);
				count++;
			} /* if has access */

		} /* for each board */

		sprintf (buf, "\n\r&gTu apartado actual es &w%s.\n\r", ch->pcdata->board->short_name);
		send_to_char (buf,ch);

		/* Inform of rights */
		if (ch->pcdata->board->read_level > get_trust(ch))
			send_to_char ("&gNo puedes leer ni escribir notas en este apartado.\n\r",ch);
		else if (ch->pcdata->board->write_level > get_trust(ch))
			send_to_char ("&gPuedes leer notas en este apartado.\n\r",ch);
		else
			send_to_char ("&gPuedes leer y escribir notas en este apartado.\n\r",ch);

		return;
	} /* if empty argument */

    if (ch->pcdata->in_progress)
    {
    	send_to_char ("&gMejor acaba de editar la nota que tienes pendiente.\n\r",ch);
        return;
    }

	/* Change board based on its number */
	if (is_number(argument))
	{
		count = 0;
		number = atoi(argument);
		for (i = 0; i < MAX_BOARD; i++)
			if (unread_notes(ch,&boards[i]) != BOARD_NOACCESS)
				if (++count == number)
					break;

		if (count == number) /* found the board.. change to it */
		{
			ch->pcdata->board = &boards[i];
			sprintf (buf, "&gPasando al apartado &w%s. &g%s.\n\r",boards[i].short_name,
			              (get_trust(ch) < boards[i].write_level)
			              ? "Solo puedes leer en el."
			              : "Puedes leer y escribir en el.");
			send_to_char (buf,ch);
		}
		else /* so such board */
			send_to_char ("No existe.\n\r",ch);

		return;
	}

	/* Non-number given, find board with that name */

	for (i = 0; i < MAX_BOARD; i++)
		if (!str_cmp(boards[i].short_name, argument))
			break;

	if (i == MAX_BOARD)
	{
		send_to_char ("No existe.\n\r",ch);
		return;
	}

	/* Does ch have access to this board? */
	if (unread_notes(ch,&boards[i]) == BOARD_NOACCESS)
	{
		send_to_char ("No existe.\n\r",ch);
		return;
	}

	ch->pcdata->board = &boards[i];
	sprintf (buf, "&gPasando al apartado &w%s. &g%s.\n\r",boards[i].short_name,
			              (get_trust(ch) < boards[i].write_level)
			              ? "Solo puedes leer en el."
			              : "Puedes leer y escribir en el.");
	send_to_char (buf,ch);
}

/* Send a note to someone on the personal board */
void personal_message (const char *sender, const char *to, const char *subject, const int expire_days, const char *text)
{
	make_note ("Personal", sender, to, subject, expire_days, text);
}

void make_note (const char* board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text)
{
	int board_index = board_lookup (board_name);
	NOTA_DATA *board;
	NOTE_DATA *note;
	char *strtime;

	if (board_index == BOARD_NOTFOUND)
	{
		bug ("make_note: board not found",0);
		return;
	}

	if (strlen(text) > MAX_NOTE_TEXT)
	{
		bug ("make_note: text too long (%d bytes)", strlen(text));
		return;
	}


	board = &boards [board_index];

	note = new_note(); /* allocate new note */

	note->sender = str_dup (sender);
	note->to_list = str_dup(to);
	note->subject = str_dup (subject);
	note->expire = current_time + expire_days * 60 * 60 * 24;
	note->text = str_dup (text);

	/* convert to ascii. ctime returns a string which last character is \n, so remove that */
	strtime = ctime (&current_time);
	strtime[strlen(strtime)-1] = '\0';

	note->date = str_dup (strtime);

	finish_note (board, note);

}

/* tries to change to the next accessible board */
bool next_board (CHAR_DATA *ch)
{
	int i = board_number(ch->pcdata->board) + 1;

	while ((i < MAX_BOARD) && (unread_notes(ch,&boards[i]) == BOARD_NOACCESS))
		i++;

	if (i == MAX_BOARD)
		return FALSE;
	else
	{
		ch->pcdata->board = &boards[i];
		return TRUE;
	}
}

void handle_con_note_to (DESCRIPTOR_DATA *d, char * argument)
{
	char buf [MAX_INPUT_LENGTH];
	CHAR_DATA *ch = d->character;

	if (!ch->pcdata->in_progress)
	{
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_TO, but no note in progress",0);
		return;
	}

	strcpy (buf, argument);
	smash_tilde (buf); /* change ~ to - as we save this field as a string later */

	switch (ch->pcdata->board->force_type)
	{
		case DEF_NORMAL: /* default field */
			if (!buf[0]) /* empty string? */
			{
				ch->pcdata->in_progress->to_list = str_dup (ch->pcdata->board->names);
				sprintf (buf, "&gFijando destino por defecto: &w%s\n\r", ch->pcdata->board->names);
				send_to_char( buf, ch);
			}
			else
				ch->pcdata->in_progress->to_list = str_dup (buf);

			break;

		case DEF_INCLUDE: /* forced default */
			if (!is_full_name (ch->pcdata->board->names, buf))
			{
				strcat (buf, " ");
				strcat (buf, ch->pcdata->board->names);
				ch->pcdata->in_progress->to_list = str_dup(buf);

				sprintf (buf, "\n\r&gNota para: &w%s %s\n\r",
						 ch->pcdata->board->names, ch->pcdata->in_progress->to_list);
				send_to_char( buf, ch);
			}
			else
				ch->pcdata->in_progress->to_list = str_dup (buf);
			break;

		case DEF_EXCLUDE: /* forced exclude */
			if (!buf[0])
			{
				ch_printf(ch, "&gDebes de especificar el destinatario.\n\r"
									"&gPara: " );
				return;
			}

			if (is_full_name (ch->pcdata->board->names, buf))
			{
				sprintf (buf, "No puedes enviar una nota a %s en este tablero. Prueba otra vez.\n\r"
				         "&gPara:      ", ch->pcdata->board->names);
				send_to_char(buf, ch);
				return; /* return from nanny, not changing to the next state! */
			}
			else
				ch->pcdata->in_progress->to_list = str_dup (buf);
			break;

	}

	send_to_char( "&gMotivo: &w",ch );
	d->connected = CON_NOTE_SUBJECT;
}

void handle_con_note_subject (DESCRIPTOR_DATA *d, char * argument)
{
	char buf [MAX_INPUT_LENGTH];
	CHAR_DATA *ch = d->character;

	if (!ch->pcdata->in_progress)
	{
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_SUBJECT, but no note in progress",0);
		return;
	}

	strcpy (buf, argument);
	smash_tilde (buf); /* change ~ to - as we save this field as a string later */

	/* Do not allow empty subjects */

	if (!buf[0])
	{
		ch_printf(ch, "&wIntroduce el motivo de la nota!\n\r",0);
		ch_printf(ch, "&gMotivo: &w", 0);
	}
	else  if (strlen(buf)>60)
	{
		send_to_char ( "&wNo, no. Esto es el motivo, ya escribiras la nota mas tarde.\n\r",ch );
	}
	else
	/* advance to next stage */
	{
		ch->pcdata->in_progress->subject = str_dup(buf);
		if (IS_IMMORTAL(ch)) /* immortals get to choose number of expire days */
		{
			sprintf (buf,"\n\r&gEn cuantos dias caducara?\n\r"
			             "&gPresiona &wENTER &gpara fijarlo por defecto del apartado,&w%d &gdias.\n\r"
           				 "&gCaduca:&w  ",
		                 ch->pcdata->board->purge_days);
			send_to_char(buf, ch);
			d->connected = CON_NOTE_EXPIRE;
		}
		else
		{
			ch->pcdata->in_progress->expire =
				current_time + ch->pcdata->board->purge_days * 24L * 3600L;
			sprintf (buf, "&gEsta nota caducara &w%s\r",ctime(&ch->pcdata->in_progress->expire));
			send_to_char( buf, ch);
			ch_printf(ch, "&gEscribe '&w~&g' o '&wEND&g' en una linea en blanco para finalizar la nota.\n\r"

"&g==============================================================================&w\n\r");
			d->connected = CON_NOTE_TEXT;
		}
	}
}

void handle_con_note_expire(DESCRIPTOR_DATA *d, char * argument)
{
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];
	time_t expire;
	int days;

	if (!ch->pcdata->in_progress)
	{
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_EXPIRE, but no note in progress",0);
		return;
	}

	/* Numeric argument. no tilde smashing */
	strcpy (buf, argument);
	if (!buf[0]) /* assume default expire */
		days = 	ch->pcdata->board->purge_days;
	else /* use this expire */
		if (!is_number(buf))
		{
			send_to_char("&wEscribe el numero de dias!\n\r",ch);
			send_to_char("&gCaduca:&w  ",ch);
			return;
		}
		else
		{
			days = atoi (buf);
			if (days <= 0)
			{
				send_to_char( "&wLos dias tienen que ser en positivo enterao! :)\n\r",ch);
				send_to_char( "&wCaduca:&w  ",ch);
				return;
			}
		}

	expire = current_time + (days*24L*3600L); /* 24 hours, 3600 seconds */

	ch->pcdata->in_progress->expire = expire;

	/* note that ctime returns XXX\n so we only need to add an \r */

	ch_printf(ch, "\n\r&gEscribe '&w~&g' o '&wEND&g' en una linea en blanco para finalizar la nota.\n\r"

"&g==============================================================================&w\n\r" 
);

	d->connected = CON_NOTE_TEXT;
}



void handle_con_note_text (DESCRIPTOR_DATA *d, char * argument)
{
	CHAR_DATA *ch = d->character;
	char buf[MAX_STRING_LENGTH];
	char letter[4*MAX_STRING_LENGTH];

	if (!ch->pcdata->in_progress)
	{
		d->connected = CON_PLAYING;
		bug ("nanny: In CON_NOTE_TEXT, but no note in progress",0);
		return;
	}

	/* First, check for EndOfNote marker */

	strcpy (buf, argument);
	if ((!str_cmp(buf, "~")) || (!str_cmp(buf, "END")))
	{
		ch_printf(ch, "\n\r\n\r");
		send_to_char(szFinishPrompt,ch);
		(send_to_char("\n\r", ch));
		d->connected = CON_NOTE_FINISH;
		return;
	}

	smash_tilde (buf); /* smash it now */

	/* Check for too long lines. Do not allow lines longer than 80 chars */

	if (strlen (buf) > MAX_LINE_LENGTH)
	{
		send_to_char("&gLinea demasiado larga, no mas de &w80 &gcaracteres!\n\r",ch);
		return;
	}

	/* Not end of note. Copy current text into temp buffer, add new line, and copy back */

	/* How would the system react to strcpy( , NULL) ? */
	if (ch->pcdata->in_progress->text)
	{
		strcpy (letter, ch->pcdata->in_progress->text);
		STRFREE( ch->pcdata->in_progress->text );
		ch->pcdata->in_progress->text = NULL; /* be sure we don't free it twice */
	}
	else
		strcpy (letter, "");

	/* Check for overflow */

	if ((strlen(letter) + strlen (buf)) > MAX_NOTE_TEXT)
	{ /* Note too long, take appropriate steps */
		send_to_char("&gNota demasiado larga!\n\r",ch);
		free_nota(ch->pcdata->in_progress);
		ch->pcdata->in_progress = NULL;			/* important */
		d->connected = CON_PLAYING;
		return;
	}

	/* Add new line to the buffer */

	strcat (letter, buf);
	strcat (letter, "\r\n"); /* new line. \r first to make note files better readable */

	/* allocate dynamically */
	ch->pcdata->in_progress->text = str_dup (letter);
}

void handle_con_note_finish (DESCRIPTOR_DATA *d, char * argument)
{

	CHAR_DATA *ch = d->character;
        char buf[MAX_STRING_LENGTH];

		if (!ch->pcdata->in_progress)
		{
			d->connected = CON_PLAYING;
			bug ("nanny: In CON_NOTE_FINISH, but no note in progress",0);
			return;
		}

		switch (tolower(argument[0]))
		{
			case 'c': /* keep writing */
				send_to_char("&wContinuando la nota...\n\r",ch);
				d->connected = CON_NOTE_TEXT;
				break;

			case 'v': /* view note so far */
				if (ch->pcdata->in_progress->text)
				{
					send_to_char("&wCuerpo de la nota:\n\r",ch);
					send_to_char( ch->pcdata->in_progress->text, ch);
				}
				else
				send_to_char("&wNo has escrito nada!\n\r\n\r",ch);
				send_to_char( szFinishPrompt, ch);
				send_to_char("\n\r",ch);
				break;

			case 'e': /* post note */
				finish_note (ch->pcdata->board, ch->pcdata->in_progress);
				send_to_char("&wNota enviada.\n\r",ch);
				d->connected = CON_PLAYING;
				/* remove AFK status */
				ch->pcdata->in_progress = NULL;
				act ( AT_DGREEN, "$n acaba de redactar su nota.", ch, NULL, NULL, TO_ROOM);
				break;

			case 'a':
				send_to_char("&wNota cancelada!\n\r",ch);
				free_nota (ch->pcdata->in_progress);
				ch->pcdata->in_progress = NULL;
				d->connected = CON_PLAYING;
				/* remove afk status */
				break;

			default: /* invalid response */
				send_to_char( "La respuestas no es correcta:\n\r\n\r",ch);
				send_to_char( szFinishPrompt, ch);
				send_to_char("\n\r", ch );

		}
}

