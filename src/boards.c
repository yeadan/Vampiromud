

/****************************************************************************

 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *

 * -----------------------------------------------------------|   (0...0)   *

 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *

 * -----------------------------------------------------------|    {o o}    *

 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *

 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*

 * Tricops and Fireblade                                      |             *

 * ------------------------------------------------------------------------ *

 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *

 * Chastain, Michael Quan, and Mitchell Tse.                                *

 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *

 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *

 * ------------------------------------------------------------------------ *

 *			     Special boards module			                *

 ****************************************************************************/

/****************************************************************************

 *                      VampiroMud 2000 - 2001                              *

 *               Adaptado por Implementadores de CoYoTe MuD                 *

 *                     coyote.asoc.euitt.upm.es 4004                        *

 *               Modificado por Implementadores de VampiroMud               *

 ****************************************************************************/







#include <sys/types.h>

#include <ctype.h>

#include <stdio.h>

#include <string.h>

#include <time.h>

#include <sys/stat.h>

#include "mud.h"

#ifdef USE_IMC

#include "imc-mercbase.h"

#endif





/* Defines for voting on notes. -- Narn */

#define VOTE_NONE 0

#define VOTE_OPEN 1

#define VOTE_CLOSED 2



BOARD_DATA *		first_board;

BOARD_DATA *		last_board;



bool	is_note_to	args( ( CHAR_DATA *ch, NOTE_DATA *pnote ) );

void	note_attach	args( ( CHAR_DATA *ch ) );

void	note_remove	args( ( CHAR_DATA *ch, BOARD_DATA *board,	NOTE_DATA *pnote ) );

void  do_note	args( ( CHAR_DATA *ch, char *arg_passed, bool IS_MAIL) );







bool can_remove( CHAR_DATA *ch, BOARD_DATA *board )

{

  /* If your trust is high enough, you can remove it. */

  if ( get_trust( ch ) >= board->min_remove_level )

    return TRUE;



  if ( board->extra_removers[0] != '\0' )

  {

    if ( is_name( ch->name, board->extra_removers ) )

      return TRUE;

  }

  return FALSE;

}



bool can_read( CHAR_DATA *ch, BOARD_DATA *board )

{

  /* If your trust is high enough, you can read it. */

  if ( get_trust( ch ) >= board->min_read_level )

    return TRUE;



  /* Your trust wasn't high enough, so check if a read_group or extra

     readers have been set up. */

  if ( board->read_group[0] != '\0' )

  {

    if ( ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, board->read_group ) )

      return TRUE;

    if ( ch->pcdata->council && !str_cmp( ch->pcdata->council->name, board->read_group ) )

      return TRUE;

  }

  if ( board->extra_readers[0] != '\0' )

  {

    if ( is_name( ch->name, board->extra_readers ) )

      return TRUE;

  }

  return FALSE;

}



bool can_post( CHAR_DATA *ch, BOARD_DATA *board )

{

  /* If your trust is high enough, you can post. */

  if ( get_trust( ch ) >= board->min_post_level )

    return TRUE;



  /* Your trust wasn't high enough, so check if a post_group has been set up. */

  if ( board->post_group[0] != '\0' )

  {

    if ( ch->pcdata->clan && !str_cmp( ch->pcdata->clan->name, board->post_group ) )

      return TRUE;

    if ( ch->pcdata->council && !str_cmp( ch->pcdata->council->name, board->post_group ) )

      return TRUE;

  }

  return FALSE;

}





/*

 * board commands.

 */

void write_boards_txt( )

{

    BOARD_DATA *tboard;

    FILE *fpout;

    char filename[256];



    sprintf( filename, "%s%s", BOARD_DIR, BOARD_FILE );

    fpout = fopen( filename, "w" );

    if ( !fpout )

    {

	bug( "FATAL: cannot open board.txt for writing!\n\r", 0 );

 	return;

    }

    for ( tboard = first_board; tboard; tboard = tboard->next )

    {

	fprintf( fpout, "Filename          %s~\n", tboard->note_file	    );

	fprintf( fpout, "Vnum              %d\n",  tboard->board_obj	    );

	fprintf( fpout, "Min_read_level    %d\n",  tboard->min_read_level   );

	fprintf( fpout, "Min_post_level    %d\n",  tboard->min_post_level   );

	fprintf( fpout, "Min_remove_level  %d\n",  tboard->min_remove_level );

	fprintf( fpout, "Max_posts         %d\n",  tboard->max_posts	    );

       	fprintf( fpout, "Type 	           %d\n",  tboard->type		    );

	fprintf( fpout, "Read_group        %s~\n", tboard->read_group       );

	fprintf( fpout, "Post_group        %s~\n", tboard->post_group       );

	fprintf( fpout, "Extra_readers     %s~\n", tboard->extra_readers    );

        fprintf( fpout, "Extra_removers    %s~\n", tboard->extra_removers   );

	if ( tboard->ocopymessg )

          fprintf( fpout, "OCopymessg	   %s~\n", tboard->ocopymessg       );

	if ( tboard->olistmessg )

          fprintf( fpout, "OListmessg	   %s~\n", tboard->olistmessg       );

	if ( tboard->opostmessg )

          fprintf( fpout, "OPostmessg	   %s~\n", tboard->opostmessg       );

	if ( tboard->oreadmessg )

	  fprintf( fpout, "OReadmessg	   %s~\n", tboard->oreadmessg       );

	if ( tboard->oremovemessg )

          fprintf( fpout, "ORemovemessg      %s~\n", tboard->oremovemessg     );

	if ( tboard->otakemessg )

	  fprintf( fpout, "OTakemessg	   %s~\n", tboard->otakemessg       );

	if ( tboard->postmessg )

	  fprintf( fpout, "Postmessg	   %s~\n", tboard->postmessg        );

	fprintf( fpout, "End\n" );

    }

    fclose( fpout );

}



BOARD_DATA *get_board( CHAR_DATA *ch )

{

    BOARD_DATA *board;



    for ( board = first_board; board; board = board->next )

       if ( board->board_obj==atoi(ch->board) )

         return board;

    return NULL;

}



BOARD_DATA *find_board( CHAR_DATA *ch )

{

    /*CHAR_DATA *ch;*/

    BOARD_DATA  *board;



    if ( (board = get_board(ch)) != NULL )

	    return board;

    return NULL;

}




/*
bool is_note_to( CHAR_DATA *ch, NOTE_DATA *pnote )

{

    if ( !str_cmp( ch->name, pnote->sender ) )

	return TRUE;



    if ( is_name( "Todos", pnote->to_list ) )

	return TRUE;



    if ( IS_HERO(ch) && is_name( "Inmortal", pnote->to_list ) )

	return TRUE;



    if ( is_name( ch->name, pnote->to_list ) )

	return TRUE;



    return FALSE;

}
*/






void note_attach( CHAR_DATA *ch )

{

    NOTE_DATA *pnote;



    if ( ch->pnote )

	return;



    CREATE( pnote, NOTE_DATA, 1 );

    pnote->next		= NULL;

    pnote->prev		= NULL;

    pnote->sender	= QUICKLINK( ch->name );

    pnote->date		= STRALLOC( "" );

    pnote->to_list	= STRALLOC( "" );

    pnote->subject	= STRALLOC( "" );

    pnote->text		= STRALLOC( "" );

    ch->pnote		= pnote;

    return;

}



void write_board( BOARD_DATA *board )

{

    FILE *fp;

    char filename[256];

    NOTE_DATA *pnote;



    /*

     * Rewrite entire list.

     */

    fclose( fpReserve );

    sprintf( filename, "%s%s", BOARD_DIR, board->note_file );

    if ( ( fp = fopen( filename, "w" ) ) == NULL )

    {

	perror( filename );

    }

    else

    {

	for ( pnote = board->first_note; pnote; pnote = pnote->next )

	{

	    fprintf( fp, "Sender  %s~\nDate    %s~\nTo      %s~\nSubject %s~\nVoting %d\nYesvotes %s~\nNovotes %s~\nAbstentions %s~\nText\n%s~\n\n",

		pnote->sender,

		pnote->date,

		pnote->to_list,

		pnote->subject,

                pnote->voting,

                pnote->yesvotes,

                pnote->novotes,

                pnote->abstentions,

		pnote->text

		);

	}

	fclose( fp );

    }

    fpReserve = fopen( NULL_FILE, "r" );

    return;

}





void free_note( NOTE_DATA *pnote )

{

    STRFREE( pnote->text    );

    STRFREE( pnote->subject );

    STRFREE( pnote->to_list );

    STRFREE( pnote->date    );

    STRFREE( pnote->sender  );

    if ( pnote->yesvotes )

	DISPOSE( pnote->yesvotes );

    if ( pnote->novotes )

	DISPOSE( pnote->novotes );

    if ( pnote->abstentions )

	DISPOSE( pnote->abstentions );

    DISPOSE( pnote );

}



void note_remove( CHAR_DATA *ch, BOARD_DATA *board, NOTE_DATA *pnote )

{



    if ( !board )

    {

      bug( "note remove: null board", 0 );

      return;

    }



    if ( !pnote )

    {

      bug( "note remove: null pnote", 0 );

      return;

    }



    /*

     * Remove note from linked list.

     */

    UNLINK( pnote, board->first_note, board->last_note, next, prev );



    --board->num_posts;

    free_note( pnote );

    write_board( board );

}





OBJ_DATA *find_quill( CHAR_DATA *ch )

{

    OBJ_DATA *quill;



    for ( quill = ch->last_carrying; quill; quill = quill->prev_content )

 	if ( quill->item_type == ITEM_PEN

        &&   can_see_obj( ch, quill ) )

	  return quill;

    return NULL;

}



void do_noteroom( CHAR_DATA *ch, char *argument )

{

    BOARD_DATA *board;

    char arg[MAX_STRING_LENGTH];

    char arg_passed[MAX_STRING_LENGTH];



    if ( ch->board == NULL )

        ch->board = "1300";



    strcpy(arg_passed, argument);



    switch( ch->substate )

    {

	case SUB_WRITING_NOTE:

	do_note(ch, arg_passed, FALSE);

 	break;



	default:



    argument = one_argument(argument, arg);

    smash_tilde( argument );

    if (!str_cmp(arg, "crear") || !str_cmp(arg, "para")

    ||  !str_cmp(arg, "objeto") || !str_cmp(arg, "mostrar"))

    {

        do_note(ch, arg_passed, FALSE);

        return;

    }



    board = find_board( ch );

    if ( !board )

    {

        send_to_char( "&GTablero actual (&WGeneral&G), si quieres cambiar de tablero\n\r", ch );

        send_to_char( "deberas de usar el comando tablero\n\r", ch );

        return;

    }



    if (board->type != BOARD_NOTE)

    {

      send_to_char("Solo puedes usar el comando nota en un board.\n\r", ch);

      return;

    }

    else

    {

      do_note(ch, arg_passed, FALSE);

      return;

    }

  }

}



void do_mailroom(CHAR_DATA *ch, char *argument)

{

    BOARD_DATA *board;

    char arg[MAX_STRING_LENGTH];

    char arg_passed[MAX_STRING_LENGTH];



    if ( ch->board == NULL )

        ch->board = "42";



    strcpy(arg_passed, argument);



    switch( ch->substate )

    {

	case SUB_WRITING_NOTE:

	do_note(ch, arg_passed, TRUE);

 	break;



	default:



    argument = one_argument(argument, arg);

    smash_tilde( argument );

    if (!str_cmp(arg, "write") || !str_cmp(arg, "to")

    ||  !str_cmp(arg, "subject") || !str_cmp(arg, "show"))

    {

        do_note(ch, arg_passed, TRUE);

	return;

    }



    board = find_board( ch );

    if ( !board )

    {

        send_to_char( "There is no mail facility here.\n\r", ch );

        return;

    }



    if (board->type != BOARD_MAIL)

    {

	send_to_char("Usa mail unicamente en el foro mails.\n\r", ch);

	return;

    }

    else

    {

	do_note(ch, arg_passed, TRUE);

	return;

    }

  }

}



void do_note( CHAR_DATA *ch, char *arg_passed, bool IS_MAIL )

{

    char buf[MAX_STRING_LENGTH];

    char arg[MAX_INPUT_LENGTH];

    NOTE_DATA  *pnote;

    BOARD_DATA *board;

    int vnum;

    int anum;

    int first_list;

    OBJ_DATA *quill = NULL, *paper = NULL, *tmpobj = NULL;

    EXTRA_DESCR_DATA *ed = NULL;

    char notebuf[MAX_STRING_LENGTH];

    char short_desc_buf[MAX_STRING_LENGTH];

    char long_desc_buf[MAX_STRING_LENGTH];

    char keyword_buf[MAX_STRING_LENGTH];

    bool mfound = FALSE;



    if ( IS_NPC(ch) )

	return;



    if ( !ch->desc )

    {

	bug( "do_note: no descriptor", 0 );

	return;

    }



    switch( ch->substate )

    {

	default:

	  break;

	case SUB_WRITING_NOTE:

	  if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL

	  ||     paper->item_type != ITEM_PAPER )

	  {

	     bug("do_note: player not holding paper", 0);

	     stop_editing( ch );

	     return;

          }

	  ed = ch->dest_buf;

	  STRFREE( ed->description );

	  ed->description = copy_buffer( ch );

	  stop_editing( ch );

	  return;

    }



    set_char_color( AT_NOTE, ch );

    arg_passed = one_argument( arg_passed, arg );

    smash_tilde( arg_passed );



    /* Reusing alot of code but this will have to do till I can think

     * of a better way to do it. --Shaddai

     */



    if ( !str_cmp( arg, "fecha" ) )

    {

	board = find_board( ch );

	if ( !board )

	{

	    send_to_char( "Aqui no hay ningun board donde mirar.\n\r", ch );

	    return;

	}

	if ( !can_read( ch, board ) )

	{

	    send_to_char( "No puedes leer en ese board...\n\r", ch );

	    return;

	}



        first_list = atoi(arg_passed);

        if (first_list)

        {

	    if (IS_MAIL)

            {

		send_to_char( "No puedes utilizar un numero para lista (por el momento) con el mail.\n\r", ch);

		return;

 	    }



	    if (first_list < 1)

	    {

		send_to_char( "La primera nota es la 1 melon!!\n\r", ch);

		return;

	    }

	}





        if (!IS_MAIL)

        {

	    set_pager_color( AT_NOTE, ch );

	    vnum = 0;

	    for ( pnote = board->first_note; pnote; pnote = pnote->next )

            {

		vnum++;

		if (is_note_to(ch,pnote))

		if ( (first_list && vnum >= first_list) || !first_list )

		   pager_printf_color( ch, "&G[%2d&G%c &GDe:&g%-12s%c &GFecha:&w%-24s &GObj:&w%-35s\n\r",

			vnum,

			is_note_to( ch, pnote ) ? ']' : ']',

			pnote->sender,

			(pnote->voting != VOTE_NONE) ? (pnote->voting == VOTE_OPEN ? 'V' : 'C') : ' ',

			pnote->date,

			pnote->subject );

	    }

	    if ( board->olistmessg )

	        act( AT_ACTION, board->olistmessg, ch, NULL, NULL, TO_CANSEE );

	    else

	        act( AT_ACTION, "$n echa un vistazo a las notas.", ch, NULL, NULL, TO_CANSEE );

	    return;

	}

	else

	{

      	    vnum = 0;





            if (IS_MAIL) /* SB Mail check for Brit */

             {

              for ( pnote = board->first_note; pnote; pnote = pnote->next )

                if (is_note_to( ch, pnote )) mfound = TRUE;



              if ( !mfound && get_trust(ch) < sysdata.read_all_mail )

               {

                ch_printf( ch, "No tienes ningun mail.\n\r");

                return;

               }

             }



            for ( pnote = board->first_note; pnote; pnote = pnote->next )

		if (is_note_to( ch, pnote ) || get_trust(ch) >= sysdata.read_all_mail)

          	      sprintf( buf, "&G[&w%2d&G%c &GDe: &w%-12s %-13s &GObj: &w %s\n\r",

			++vnum,

			is_note_to( ch, pnote ) ? ']' : ']',

			pnote->sender,

                  pnote->date,

			pnote->subject );

                  send_to_char_color (buf,ch);

	    return;

	}

    }



    if ( !str_cmp( arg, "lista" ) )

    {

	board = find_board( ch );

	if ( !board )

	{

	    send_to_char( "Aqui no hay ningun board donde mirar.\n\r", ch );

	    return;

	}

	if ( !can_read( ch, board ) )

	{

	    send_to_char( "No puedes leer en ese board...\n\r", ch );

	    return;

	}



        first_list = atoi(arg_passed);

        if (first_list)

        {

	    if (IS_MAIL)

            {

		send_to_char( "No puedes usar un numero de lista (por el momento) con el mail.\n\r", ch);

		return;

 	    }



	    if (first_list < 1)

	    {

		send_to_char( "La primera nota es la 1 melon!\n\r", ch);

		return;

	    }

	}





        if (!IS_MAIL)

        {

	    set_pager_color( AT_NOTE, ch );

	    vnum = 0;

          for ( pnote = board->first_note; pnote; pnote = pnote->next )

            {

		vnum++;

                if (is_note_to(ch,pnote))

		if ( (first_list && vnum >= first_list) || !first_list )

         pager_printf_color( ch, "&R[&w%3d&R%c De: &w%-10s%c &RPara:&w%-12.12s &RObj:&w %s\n\r",

			vnum,

			is_note_to( ch, pnote ) ? ']' : ']',

			pnote->sender,

			(pnote->voting != VOTE_NONE) ? (pnote->voting == VOTE_OPEN ? 'V' : 'C') : ' ',

			pnote->to_list,

			pnote->subject );

	    }

	    if ( board->olistmessg )

		act( AT_ACTION, board->olistmessg, ch, NULL, NULL, TO_CANSEE );

	    else

		act( AT_ACTION, "$n echa un vistazo a las notas.", ch, NULL, NULL, TO_CANSEE );

	    return;

	}

	else

	{

      	    vnum = 0;





            if (IS_MAIL) /* SB Mail check for Brit */

             {

              for ( pnote = board->first_note; pnote; pnote = pnote->next )

                if (is_note_to( ch, pnote )) mfound = TRUE;



              if ( !mfound && get_trust(ch) < sysdata.read_all_mail )

               {

                ch_printf( ch, "No tienes ningun mail.\n\r");

                return;

               }

             }



            for ( pnote = board->first_note; pnote; pnote = pnote->next )

		if (is_note_to( ch, pnote ) || get_trust(ch) >= sysdata.read_all_mail)

                sprintf( buf, "&R[&w%2d&R%c &RDe: &w%s &RObj: %s\n\r",

			++vnum,

			is_note_to( ch, pnote ) ? ']' : ']',

			pnote->sender,

			pnote->subject );

                  send_to_char_color (buf,ch);

	    return;

	}

    }



    if ( !str_cmp( arg, "leer" ) )

    {

	bool fAll;



	board = find_board( ch );

	if ( !board )

	{

	    send_to_char( "Aqui no hay ningun board donde mirar.\n\r", ch );

	    return;

	}

	if ( !can_read( ch, board ) )

	{

	    send_to_char( "No puedes leer en ese board...\n\r", ch );

	    return;

	}

/* KAYSER */



/* FIN KAYSER */

	if ( !str_cmp( arg_passed, "todas" ) )

	{

	    fAll = TRUE;

	    anum = 0;

	}

	else

	if ( is_number( arg_passed ) )

	{

	    fAll = FALSE;

	    anum = atoi( arg_passed );

	}

	else

	{

	    send_to_char( "Leer que nota?\n\r", ch );

	    return;

	}



	set_pager_color( AT_NOTE, ch );

	if (!IS_MAIL)

 	{

	    vnum = 0;

	    for ( pnote = board->first_note; pnote; pnote = pnote->next )

            {

		vnum++;

		if (is_note_to(ch,pnote))

		if ( vnum == anum || fAll )

		{

		    pager_printf_color( ch, "&G[&w%3d&G] De &w %s &GObjeto:&w %s\n\r&GFecha: &w%s\n\r&GPara:&w %s\n\r\n\r&Y%s",

			vnum,

			pnote->sender,

			pnote->subject,

			pnote->date,

			pnote->to_list,

			pnote->text );



		    if ( pnote->yesvotes[0] != '\0' || pnote->novotes[0] != '\0'

                    || pnote->abstentions[0] != '\0' )

		    {

			send_to_pager( "------------------------------------------------------------\n\r", ch );

			pager_printf( ch, "Votos:\n\rSi:     %s\n\rNo:      %s\n\rAbstenciones: %s\n\r",

				pnote->yesvotes, pnote->novotes, pnote->abstentions );

		    }

		    if ( board->oreadmessg )

			act( AT_ACTION, board->oreadmessg, ch, NULL, NULL, TO_CANSEE );

		    else

			act( AT_ACTION, "$n lee una nota.", ch, NULL, NULL, TO_CANSEE );

		    return;

		}

	    }

	    send_to_char( "No existe esa nota.\n\r", ch );

	    return;

	}

	else

	{

	    vnum = 0;

	    for ( pnote = board->first_note; pnote; pnote = pnote->next )

	    {

		if (is_note_to(ch, pnote) || get_trust(ch) >= sysdata.read_all_mail)

		{

		    vnum++;

		    if ( vnum == anum || fAll )

		    {

			/*if ( ch->gold < 10

			&&   get_trust(ch) < sysdata.read_mail_free )

			{

			    send_to_char("Cuesta 10 monedas leer un mensaje.\n\r", ch);

			    return;

			}

			if (get_trust(ch) < sysdata.read_mail_free)

			   ch->gold -= 10;*/

         pager_printf_color( ch, "&R[&w%3d&R] De: &w%s &RObj: %s\n\r&RFecha: &w%s\n\r&RPara: &w%s\n\r\n\r&Y%s",

			    vnum,

			    pnote->sender,

			    pnote->subject,

			    pnote->date,

			    pnote->to_list,

			    pnote->text );

			return;

		    }

		}

	    }

	    send_to_char( "No existe ese mensaje.\n\r", ch );

	    return;

	}

    }



    /* Voting added by Narn, June '96 */

    if ( !str_cmp( arg, "votar" ) )

    {

	char arg2[MAX_INPUT_LENGTH];

	arg_passed = one_argument( arg_passed, arg2 );



	board = find_board( ch );

	if ( !board )

	{

	    send_to_char( "No hay ningun board de votaciones aqui.\n\r", ch );

	    return;

	}

	if ( !can_read( ch, board ) )

	{

            send_to_char( "No puedes votar en este board.\n\r", ch );

            return;

	}



	if ( is_number( arg2 ) )

	    anum = atoi( arg2 );

	else

	{

            send_to_char( "Votar que numero de nota?\n\r", ch );

            return;

	}



	vnum = 1;

	for ( pnote = board->first_note; pnote && vnum < anum; pnote = pnote->next )

	    vnum++;

	if ( !pnote )

	{

	    send_to_char( "No existe esa nota.\n\r", ch );

	    return;

	}



	/* Options: open close yes no abstain */

	/* If you're the author of the note and can read the board you can open

	   and close voting, if you can read it and voting is open you can vote.

	*/

	if ( !str_cmp( arg_passed, "abrir" ) )

	{

	    if ( str_cmp( ch->name, pnote->sender ) )

	    {

		send_to_char( "No eres el autor de esa nota.\n\r", ch );

		return;

	    }

	    pnote->voting = VOTE_OPEN;

	    act( AT_ACTION, "$n abre votaciones en una nota.", ch, NULL, NULL, TO_ROOM );

            send_to_char( "Votaciones Abiertas.\n\r", ch );

            write_board( board );

            return;

	}

	if ( !str_cmp( arg_passed, "cerrar" ) )

	{

	    if ( str_cmp( ch->name, pnote->sender ) )

	    {

		send_to_char( "No eres el autor de esa nota.\n\r", ch );

		return;

	    }

	    pnote->voting = VOTE_CLOSED;

	    act( AT_ACTION, "$n cierra votaciones en una nota.", ch, NULL, NULL, TO_ROOM );

	    send_to_char( "Votaciones Cerradas.\n\r", ch );

	    write_board( board );

	    return;

	}



	/* Make sure the note is open for voting before going on. */

	if ( pnote->voting != VOTE_OPEN )

	{

	    send_to_char( "Estan cerradas las votaciones en esa nota.\n\r", ch );

	    return;

	}



	/* Can only vote once on a note. */

	sprintf( buf, "%s %s %s",

		pnote->yesvotes, pnote->novotes, pnote->abstentions );

	if ( is_name( ch->name, buf ) )

	{

	    send_to_char( "Ya has votado en esa nota.\n\r", ch );

	    return;

	}

	if ( !str_cmp( arg_passed, "si" ) )

	{

	    sprintf( buf, "%s %s", pnote->yesvotes, ch->name );

	    DISPOSE( pnote->yesvotes );

	    pnote->yesvotes = str_dup( buf );

	    act( AT_ACTION, "$n vota en una nota.", ch, NULL, NULL, TO_ROOM );

	    send_to_char( "Ok.\n\r", ch );

	    write_board( board );

	    return;

	}

	if ( !str_cmp( arg_passed, "no" ) )

	{

	    sprintf( buf, "%s %s", pnote->novotes, ch->name );

	    DISPOSE( pnote->novotes );

	    pnote->novotes = str_dup( buf );

	    act( AT_ACTION, "$n vota en una nota.", ch, NULL, NULL, TO_ROOM );

	    send_to_char( "Ok.\n\r", ch );

	    write_board( board );

	    return;

	}

	if ( !str_cmp( arg_passed, "abstencion" ) )

	{

	    sprintf( buf, "%s %s", pnote->abstentions, ch->name );

	    DISPOSE( pnote->abstentions );

	    pnote->abstentions = str_dup( buf );

	    act( AT_ACTION, "$n vota en una nota.", ch, NULL, NULL, TO_ROOM );

	    send_to_char( "Ok.\n\r", ch );

	    write_board( board );

	    return;

	}

	do_note( ch, "", FALSE );

    }

    if ( !str_cmp( arg, "crear" ) )

    {

	if ( ch->substate == SUB_RESTRICTED )

	{

	    send_to_char( "No puedes escribir una nota con otro comando.\n\r", ch );

	    return;

	}



       /*Esto lo quito para que no haga falta pluma ni papel

	if (get_trust (ch) < sysdata.write_mail_free)

	{

	    quill = find_quill( ch );

            if (!quill)

	    {

		send_to_char("Necesitas una pluma para escribir una nota.\n\r", ch);

		return;

	    }

	    if ( quill->value[0] < 1 )

	    {

		send_to_char("Tu pluma no tiene tinta.\n\r", ch);

		return;

	    }

	}*/

	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL

	||     paper->item_type != ITEM_PAPER )

	{

/*Y esto para que el papel se cree solo

	    if (get_trust(ch) < sysdata.write_mail_free )

	    {

		send_to_char("Necesitas sostener papel para escribir una nota.\n\r", ch);

		return;

	    }  */

	    paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );

	    if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)

	      unequip_char(ch, tmpobj);

	    paper = obj_to_char(paper, ch);

	    equip_char(ch, paper, WEAR_HOLD);

	    /*act(AT_MAGIC, "Un papel aparece magicamente en las manos de $n!",

      	        ch, NULL, NULL, TO_ROOM);

  	    act(AT_MAGIC, "Un papel aparece en tus manos.",

    	        ch, NULL, NULL, TO_CHAR);*/

	}

	if (paper->value[0] < 2 )

	{

	    paper->value[0] = 1;

	    ed = SetOExtra(paper, "_text_");

	    ch->substate = SUB_WRITING_NOTE;

	    ch->dest_buf = ed;

	    /*if ( get_trust(ch) < sysdata.write_mail_free )

		--quill->value[0];*/

	    start_editing( ch, ed->description );

	    return;

	}

	else

	{

	    send_to_char("No puedes modificar esta nota.\n\r", ch);

	    return;

	}

    }



    if ( !str_cmp( arg, "objeto" ) )

    {

	/*if(get_trust(ch) < sysdata.write_mail_free)

	{

	    quill = find_quill( ch );

	    if ( !quill )

	    {

		send_to_char("Necesitas una pluma para escribir una nota.\n\r", ch);

		return;

	    }

	    if ( quill->value[0] < 1 )

	    {

		send_to_char("Tu pluma esta seca.\n\r", ch);

		return;

	    }

	}*/

	if (!arg_passed || arg_passed[0] == '\0')

	{

	    send_to_char("Cual quieres que sea el objeto de la nota?\n\r", ch);

	    return;

	}

	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL

	||     paper->item_type != ITEM_PAPER )

	{

	  /*  if(get_trust(ch) < sysdata.write_mail_free )

	    {

		send_to_char("Necesitas sostener un papel para escribir una nota.\n\r", ch);

		return;

	    }*/

	    paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );

    	    if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)

		unequip_char(ch, tmpobj);

	    paper = obj_to_char(paper, ch);

	    equip_char(ch, paper, WEAR_HOLD);

/*	    act(AT_MAGIC, "Un papel aparece magicamente en las manos de $n!",

		ch, NULL, NULL, TO_ROOM);

	    act(AT_MAGIC, "Un papel aparece en tus manos.",

		ch, NULL, NULL, TO_CHAR);*/

	}

	if (paper->value[1] > 1 )

        {

	    send_to_char("No puedes modificar esta nota.\n\r", ch);

	    return;

        }

        else

	{

	    paper->value[1] = 1;

	    ed = SetOExtra(paper, "_subject_");

	    STRFREE( ed->description );

	    ed->description = STRALLOC( arg_passed );

	    send_to_char("Ok.\n\r", ch);

	    return;

	}

    }



    if ( !str_cmp( arg, "para" ) )

    {

	struct stat fst;

/*	char *pn;*/

	char fname[1024];

#ifdef USE_IMC

	bool imc = FALSE;

#endif



/*	if(get_trust(ch) < sysdata.write_mail_free )

	{

	    quill = find_quill( ch );

	    if ( !quill )

	    {

		send_to_char("Necesitas una pluma para escribir una nota.\n\r", ch);

		return;

	    }

	    if ( quill->value[0] < 1 )

	    {

		send_to_char("Tu pluma esta seca.\n\r", ch);

		return;

	    }

	}*/

	if (!arg_passed || arg_passed[0] == '\0')

	{

	    send_to_char("Por favor, especifica una direccion.\n\r", ch);

	    return;

	}

	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL

	||     paper->item_type != ITEM_PAPER )

	{

	   /* if(get_trust(ch) < sysdata.write_mail_free )

	    {

		send_to_char("Necesitas sostener un papel para escribir una nota.\n\r", ch);

		return;

	    }*/

	    paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );

    	    if ((tmpobj = get_eq_char(ch, WEAR_HOLD)) != NULL)

		unequip_char(ch, tmpobj);

	    paper = obj_to_char(paper, ch);

	    equip_char(ch, paper, WEAR_HOLD);

	    /*act(AT_MAGIC, "Un papel aparece magicamente en las manos de $n!",

	      ch, NULL, NULL, TO_ROOM);

	    act(AT_MAGIC, "Un papel aparece en tus manos.",

      	      ch, NULL, NULL, TO_CHAR);*/

	}



	if (paper->value[2] > 1)

	{

	    send_to_char("No puedes modificar esta nota.\n\r",ch);

	    return;

	}



        arg_passed[0] = UPPER(arg_passed[0]);



#ifdef USE_IMC

	if (strchr(arg_passed, '@')!=NULL)

	{

	    if (get_trust(ch) < sysdata.imc_mail_level)

	    {

		ch_printf(ch, "Necesitas tener nivel %d para mandar notas"

		    "notas a otros muds.\n\r", sysdata.imc_mail_level);

		return;

	    }

	    imc = TRUE;

	}

#endif



        sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower(arg_passed[0]),

                 capitalize( arg_passed ) );



#ifdef USE_IMC

	if ( !IS_MAIL || imc || stat( fname, &fst ) != -1 || !str_cmp(arg_passed, "all") )

#else

	if ( !IS_MAIL || stat( fname, &fst ) != -1 || !str_cmp(arg_passed, "all") )

#endif

	{

	    paper->value[2] = 1;

	    ed = SetOExtra(paper, "_to_");

	    STRFREE( ed->description );

	    ed->description = STRALLOC( arg_passed );

	    send_to_char("Ok.\n\r",ch);

	    return;

        }

	else

	{

	    send_to_char("Ningun jugador es llamado asi.\n\r",ch);

	    return;

	}



    }



    if ( !str_cmp( arg, "mostrar" ) )

    {

	char *subject, *to_list, *text;



	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL

	||     paper->item_type != ITEM_PAPER )

	{

	    send_to_char("No editas ninguna nota.\n\r", ch);

	    return;

	}



	if ( (subject = get_extra_descr( "_subject_", paper->first_extradesc )) == NULL )

	  subject = "(no subject)";

	if ( (to_list = get_extra_descr( "_to_", paper->first_extradesc )) == NULL )

	  to_list = "(nobody)";

	sprintf( buf, "%s: %s\n\rPara: %s\n\r",

	    ch->name,

	    subject,

	    to_list );

	send_to_char( buf, ch );

	if ( (text = get_extra_descr( "_text_", paper->first_extradesc )) == NULL )

	  text = "The note is blank.\n\r";

	send_to_char( text, ch );

	return;

    }



    if ( !str_cmp( arg, "enviar" ) )

    {

	char *strtime, *to, *subj, *text/*, *np = NULL*/;

#ifdef USE_IMC

	bool imc = FALSE;

#endif



	if ( ( paper = get_eq_char(ch, WEAR_HOLD) ) == NULL

	||     paper->item_type != ITEM_PAPER )

	{

	    send_to_char("No editas ninguna nota.\n\r", ch);

	    return;

	}



	if ( paper->value[0] == 0 )

	{

	    send_to_char("No hay nada escrito en la nota.\n\r", ch);

	    return;

	}



	if ( paper->value[1] == 0 )

	{

	    send_to_char("La nota no tiene ningun objeto.\n\r", ch);

	    return;

	}



	if (paper->value[2] == 0)

	{

	    send_to_char("La nota no esta dirigida a nadie!\n\r", ch);

	    return;

	}



	strtime				= ctime( &current_time );

	strtime[strlen(strtime)-1]	= '\0';



	/* handle IMC notes */

	to = get_extra_descr( "_to_", paper->first_extradesc );

	subj = get_extra_descr( "_subject_", paper->first_extradesc );

	text = get_extra_descr( "_text_", paper->first_extradesc );



#ifdef USE_IMC

	if (to && strchr(to, '@')!=NULL)

	{

	    if ( !subj || !*subj )

	    {

		send_to_char( "Necesitas especificar un objeto con IMC mail.\n\r", ch );

		return;

	    }

	    if ( !text || !*text )

	    {

		send_to_char( "Tiene que haber texto en un IMC mail.\n\r", ch );

		return;

	    }



	    imc = TRUE;

	}

#endif

	board = find_board( ch );



	if ( !board )

	{

	    send_to_char( "No hay ningun board aqui donde mandar tu nota.\n\r", ch );

	    return;

	}

#ifdef USE_IMC

	if ( (imc && board->board_obj != sysdata.imc_mail_vnum) ||

	    (!imc && board->board_obj == sysdata.imc_mail_vnum) )

	{

	    send_to_char( "Puedes mandar IMC mail solo en la IMC board.\n\r", ch );

	    return;

	}

#endif

	if ( !can_post( ch, board ) )

	{

	    send_to_char( "Una fuerza magica te previene de mandar notas aqui...\n\r", ch );

	    return;

	}



	if ( board->num_posts >= board->max_posts )

	{

	    send_to_char( "Tu nota no cabe en este board.\n\r", ch );

	    return;

	}



	if ( board->opostmessg )

		act ( AT_ACTION, board->opostmessg, ch, NULL, NULL, TO_ROOM );

	else

		act( AT_ACTION, "$n manda una nota.", ch, NULL, NULL, TO_ROOM );



	CREATE( pnote, NOTE_DATA, 1 );

	pnote->date			= STRALLOC( strtime );



	pnote->to_list = to ? STRALLOC( to ) : STRALLOC( "todos" );

	pnote->text = text ? STRALLOC( text ) : STRALLOC( "" );

	pnote->subject = subj ? STRALLOC( subj ) : STRALLOC( "" );

	pnote->sender  = QUICKLINK( ch->name );

        pnote->voting      = 0;

        pnote->yesvotes    = str_dup( "" );

        pnote->novotes     = str_dup( "" );

        pnote->abstentions = str_dup( "" );



#ifdef USE_IMC

	if ( imc )

	    imc_post_mail(ch, pnote->sender, pnote->to_list, pnote->date,

	        pnote->subject, pnote->text);

#endif



	LINK( pnote, board->first_note, board->last_note, next, prev );

	board->num_posts++;

        write_board( board );

	if ( board->postmessg )

		act( AT_ACTION, board->postmessg, ch, NULL, NULL, TO_CHAR );

	else

        if ( !xIS_SET( ch->act, PLR_WIZINVIS ) )

        {

        sprintf( buf, "&z[Tienes una nueva nota de %s en el tablero (%10.10s)]\n\r", ch->name,

        ch->board == "1300"  ? "General"       :

        ch->board == "1302"  ? "Ideas"         :

        ch->board == "1301"  ? "Inmortal"      :

        ch->board == "1302"  ? "Correo (Mail)" :

        ch->board == "1303"  ? "Rol"           :

                               "General"      );

       echo_to_all( AT_GREEN, buf, ECHOTAR_ALL );

       }

       else

       {

        sprintf( buf, "&z[Tienes una nueva nota de Algun Inmortal en el tablero (%10.10s)]\n\r" ,

        ch->board == "1300"  ? "General"       :

        ch->board == "1302"  ? "Ideas"         :

        ch->board == "1301"  ? "Inmortal"      :

        ch->board == "1302"  ? "Correo (Mail)" :

        ch->board == "1303"  ? "Rol"           :

                               "General"      );

       echo_to_all( AT_GREEN, buf, ECHOTAR_ALL );

       }

	   send_to_char( "Colocas (mandas) tu nota en el board.\n\r", ch );

                	extract_obj( paper );

	return;

    }



    if ( !str_cmp( arg, "eliminar" )

    ||   !str_cmp( arg, "coger" )

    ||   !str_cmp( arg, "copiar" ) )

    {

	char take;



	board = find_board( ch );

	if ( !board )

	{

	    send_to_char( "No hay ningun board aqui!\n\r", ch );

	    return;

	}

	if ( !str_cmp( arg, "coger" ) )

	  take = 1;

	else if ( !str_cmp( arg, "copiar" ) )

	{

	  if ( !IS_IMMORTAL(ch) )

	  {

	    send_to_char( "Como?  Escribe 'help nota' y enterate.\n\r", ch );

	    return;

	  }

	  take = 2;

	}

	else

	  take = 0;



	if ( !is_number( arg_passed ) )

	{

	    send_to_char( "Eliminar que nota?\n\r", ch );

	    return;

	}



   if ( !IS_IMMORTAL(ch) )

   {

      send_to_char( "No puedes eliminar notas de este board!\n\r", ch );

      return;

   }



	if ( !can_read( ch, board ) )

	{

	    send_to_char( "No puedes eliminar notas de este board!\n\r", ch );

	    return;

	}



	anum = atoi( arg_passed );

	vnum = 0;

	for ( pnote = board->first_note; pnote; pnote = pnote->next )

	{

	    if (IS_MAIL && ((is_note_to(ch, pnote))

	    ||  get_trust(ch) >= sysdata.take_others_mail))

	       vnum++;

	    else if (!IS_MAIL)

               vnum++;

	    if ( ( is_note_to( ch, pnote )

	    ||	    can_remove (ch, board))

	    &&   ( vnum == anum ) )

	    {

		if ( (is_name("Todos", pnote->to_list))

		&&   (get_trust( ch ) < sysdata.take_others_mail)

		&&   (take == 1) )

		{

		    send_to_char("Notas para 'todos' no pueden ser cogidas.\n\r", ch);

		    return;

		}



 		if ( take != 0 )

		{

		    if ( ch->gold < 50 && get_trust(ch) < sysdata.read_mail_free )

		    {

			if ( take == 1 )

			  send_to_char("Cuesta 50 monedas coger tu .\n\r", ch);

			else

			  send_to_char("Cuesta 50 monedas copiar tu.\n\r", ch);

			return;

		    }

		    if ( get_trust(ch) < sysdata.read_mail_free )

		      ch->gold -= 50;

		    paper = create_object( get_obj_index(OBJ_VNUM_NOTE), 0 );

		    ed = SetOExtra( paper, "_sender_" );

		    STRFREE( ed->description );

		    ed->description = QUICKLINK(pnote->sender);

		    ed = SetOExtra( paper, "_text_" );

		    STRFREE( ed->description );

		    ed->description = QUICKLINK(pnote->text);

		    ed = SetOExtra( paper, "_to_" );

		    STRFREE( ed->description );

		    ed->description = QUICKLINK( pnote->to_list );

		    ed = SetOExtra( paper, "_subject_" );

		    STRFREE( ed->description );

		    ed->description = QUICKLINK( pnote->subject );

		    ed = SetOExtra( paper, "_date_" );

		    STRFREE( ed->description );

		    ed->description = QUICKLINK( pnote->date );

		    ed = SetOExtra( paper, "note" );

		    STRFREE( ed->description );

		    sprintf(notebuf, "De: ");

		    strcat(notebuf, pnote->sender);

		    strcat(notebuf, "\n\rPara: ");

		    strcat(notebuf, pnote->to_list);

		    strcat(notebuf, "\n\rObjeto: ");

		    strcat(notebuf, pnote->subject);

		    strcat(notebuf, "\n\r\n\r");

		    strcat(notebuf, pnote->text);

		    strcat(notebuf, "\n\r");

		    ed->description = STRALLOC(notebuf);

		    paper->value[0] = 2;

		    paper->value[1] = 2;

	 	    paper->value[2] = 2;

		    sprintf(short_desc_buf, "una nota de %s para %s",

			pnote->sender, pnote->to_list);

		    STRFREE(paper->short_descr);

		    paper->short_descr = STRALLOC(short_desc_buf);

		    sprintf(long_desc_buf, "Una nota de %s para %s esta aqui tirada.",

			pnote->sender, pnote->to_list);

		    STRFREE(paper->description);

		    paper->description = STRALLOC(long_desc_buf);

		    sprintf(keyword_buf, "note parchment paper %s",

			pnote->to_list);

		    STRFREE(paper->name);

		    paper->name = STRALLOC(keyword_buf);

		}

		if ( take != 2 )

		    note_remove( ch, board, pnote );

		send_to_char( "Ok.\n\r", ch );

		if ( take == 1 )

		{

		    if ( board->otakemessg )

		    	act( AT_ACTION, board->otakemessg, ch, NULL, NULL, TO_ROOM );

		    else

		    	act( AT_ACTION, "$n coge una nota.", ch, NULL, NULL, TO_ROOM );

		    obj_to_char(paper, ch);

		}

		else if ( take == 2 )

		{

		    if ( board->ocopymessg )

		       act( AT_ACTION, board->ocopymessg, ch, NULL, NULL, TO_ROOM );

		    else

		       act( AT_ACTION, "$n copia una nota.", ch, NULL, NULL, TO_ROOM );

		    obj_to_char(paper, ch);

		}

		else

		{

		   if ( board->oremovemessg )

		      act( AT_ACTION, board->oremovemessg, ch, NULL, NULL, TO_ROOM );

		   else

		      act( AT_ACTION, "$n elimina una nota.", ch, NULL, NULL, TO_ROOM );

	 	}

		return;

	    }

	}



	send_to_char( "Esa nota no existe.\n\r", ch );

	return;

    }



    send_to_char( "Como? Escribe 'help nota' y enterate.\n\r", ch );

    return;

}








BOARD_DATA *read_board( char *boardfile, FILE *fp )

{

    BOARD_DATA *board;

    char *word;

    char  buf[MAX_STRING_LENGTH];

    bool fMatch;

    char letter;



	do

	{

	    letter = getc( fp );

	    if ( feof(fp) )

	    {

		fclose( fp );

		return NULL;

	    }

	}

	while ( isspace(letter) );

	ungetc( letter, fp );



	CREATE( board, BOARD_DATA, 1 );

	/* Setup pointers --Shaddai */

	board->otakemessg = NULL;

	board->opostmessg = NULL;

	board->oremovemessg = NULL;

	board->olistmessg = NULL;

	board->ocopymessg = NULL;

	board->oreadmessg = NULL;

	board->postmessg = NULL;



#ifdef KEY
#undef KEY
#endif
#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}



    for ( ; ; )

    {

	word   = feof( fp ) ? "End" : fread_word( fp );

	fMatch = FALSE;



	switch ( UPPER(word[0]) )

	{

	case '*':

	    fMatch = TRUE;

	    fread_to_eol( fp );

	    break;

	case 'E':

	    KEY( "Extra_readers",	board->extra_readers,	fread_string_nohash( fp ) );

            KEY( "Extra_removers",       board->extra_removers,   fread_string_nohash( fp ) );

            if ( !str_cmp( word, "End" ) )

            {

 	      board->num_posts	= 0;

	      board->first_note	= NULL;

	      board->last_note	= NULL;

	      board->next	= NULL;

	      board->prev	= NULL;

              if ( !board->read_group )

                board->read_group    = str_dup( "" );

              if ( !board->post_group )

                board->post_group    = str_dup( "" );

              if ( !board->extra_readers )

                board->extra_readers = str_dup( "" );

              if ( !board->extra_removers )

                board->extra_removers = str_dup( "" );

              return board;

            }

	case 'F':

	    KEY( "Filename",	board->note_file,	fread_string_nohash( fp ) );

	case 'M':

	    KEY( "Min_read_level",	board->min_read_level,	fread_number( fp ) );

	    KEY( "Min_post_level",	board->min_post_level,	fread_number( fp ) );

	    KEY( "Min_remove_level",	board->min_remove_level,fread_number( fp ) );

	    KEY( "Max_posts",		board->max_posts,	fread_number( fp ) );

	case 'O':

	    KEY( "OTakemessg",  board->otakemessg,	fread_string_nohash(fp));

	    KEY( "OCopymessg",  board->ocopymessg,	fread_string_nohash(fp));

	    KEY( "OReadmessg",  board->oreadmessg,	fread_string_nohash(fp));

	    KEY( "ORemovemessg",  board->oremovemessg,	fread_string_nohash(fp));

	    KEY( "OListmessg",  board->olistmessg,	fread_string_nohash(fp));

	    KEY( "OPostmessg",  board->opostmessg,	fread_string_nohash(fp));

	case 'P':

	    KEY( "Post_group",	board->post_group,	fread_string_nohash( fp ) );

	    KEY( "Postmessg",  board->postmessg,	fread_string_nohash(fp));

	case 'R':

	    KEY( "Read_group",	board->read_group,	fread_string_nohash( fp ) );

	case 'T':

	    KEY( "Type",	board->type,		fread_number( fp ) );

	case 'V':

	    KEY( "Vnum",	board->board_obj,	fread_number( fp ) );

        }

	if ( !fMatch )

	{

	    sprintf( buf, "read_board: no match: %s", word );

	    bug( buf, 0 );

	}

    }



  return board;

}



NOTE_DATA *read_note( char *notefile, FILE *fp )

{

    NOTE_DATA *pnote;

    char *word;



    for ( ; ; )

    {

	char letter;



	do

	{

	    letter = getc( fp );

	    if ( feof(fp) )

	    {

		fclose( fp );

		return NULL;

	    }

	}

	while ( isspace(letter) );

	ungetc( letter, fp );



	CREATE( pnote, NOTE_DATA, 1 );



	if ( str_cmp( fread_word( fp ), "sender" ) )

	    break;

	pnote->sender	= fread_string( fp );



	if ( str_cmp( fread_word( fp ), "date" ) )

	    break;

	pnote->date	= fread_string( fp );



	if ( str_cmp( fread_word( fp ), "to" ) )

	    break;

	pnote->to_list	= fread_string( fp );



	if ( str_cmp( fread_word( fp ), "subject" ) )

	    break;

	pnote->subject	= fread_string( fp );



        word = fread_word( fp );

        if ( !str_cmp( word, "voting" ) )

        {

          pnote->voting = fread_number( fp );



	  if ( str_cmp( fread_word( fp ), "yesvotes" ) )

	    break;

	  pnote->yesvotes	= fread_string_nohash( fp );



	  if ( str_cmp( fread_word( fp ), "novotes" ) )

	    break;

	  pnote->novotes	= fread_string_nohash( fp );



	  if ( str_cmp( fread_word( fp ), "abstentions" ) )

	    break;

	  pnote->abstentions	= fread_string_nohash( fp );



          word = fread_word( fp );

        }



	if ( str_cmp( word, "text" ) )

	    break;

	pnote->text	= fread_string( fp );



        if ( !pnote->yesvotes )    pnote->yesvotes	= str_dup( "" );

        if ( !pnote->novotes )     pnote->novotes	= str_dup( "" );

        if ( !pnote->abstentions ) pnote->abstentions	= str_dup( "" );

	pnote->next		= NULL;

	pnote->prev		= NULL;

	return pnote;

    }

    bug ( "read_note: bad key word." );

    exit( 1 );

}



/*

 * Load boards file.

 */

void load_boards( void )

{

    FILE	*board_fp;

    FILE	*note_fp;

    BOARD_DATA	*board;

    NOTE_DATA	*pnote;

    char	boardfile[256];

    char	notefile[256];



    first_board	= NULL;

    last_board	= NULL;



    sprintf( boardfile, "%s%s", BOARD_DIR, BOARD_FILE );

    if ( ( board_fp = fopen( boardfile, "r" ) ) == NULL )

	return;



    while ( (board = read_board( boardfile, board_fp )) != NULL )

    {

	LINK( board, first_board, last_board, next, prev );

	sprintf( notefile, "%s%s", BOARD_DIR, board->note_file );

	log_string( notefile );

	if ( ( note_fp = fopen( notefile, "r" ) ) != NULL )

	{

	    while ( (pnote = read_note( notefile, note_fp )) != NULL )

	    {

		LINK( pnote, board->first_note, board->last_note, next, prev );

		board->num_posts++;

	    }

	}

    }

    return;

}





void do_makeboard( CHAR_DATA *ch, char *argument )

{

    BOARD_DATA *board;



    if ( !argument || argument[0] == '\0' )

    {

	send_to_char( "Usage: makeboard <filename>\n\r", ch );

	return;

    }



    smash_tilde( argument );



    CREATE( board, BOARD_DATA, 1 );



    LINK( board, first_board, last_board, next, prev );

    board->note_file	   = str_dup( strlower( argument ) );

    board->read_group      = str_dup( "" );

    board->post_group      = str_dup( "" );

    board->extra_readers   = str_dup( "" );

    board->extra_removers  = str_dup( "" );

}



void do_bset( CHAR_DATA *ch, char *argument )

{

    BOARD_DATA *board;

    bool found;

    char arg1[MAX_INPUT_LENGTH];

    char arg2[MAX_INPUT_LENGTH];

    char buf[MAX_STRING_LENGTH];

    int value;



    argument = one_argument( argument, arg1 );

    argument = one_argument( argument, arg2 );



    set_char_color( AT_NOTE, ch );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )

    {

	send_to_char( "Usage: bset <board filename> <field> value\n\r", ch );

	send_to_char( "\n\rField being one of:\n\r", ch );

	send_to_char( "  ovnum read post remove maxpost filename type\n\r", ch );

	send_to_char( "  read_group post_group extra_readers extra_removers\n\r", ch );

	send_to_char ("The following will affect how an action is sent:\n\r", ch );

	send_to_char ("  oremove otake olist oread ocopy opost postmessg\n\r", ch );

	return;

    }



    value = atoi( argument );

    found = FALSE;

    for ( board = first_board; board; board = board->next )

	if ( !str_cmp( arg1, board->note_file ) )

	{

	   found = TRUE;

	   break;

	}

    if ( !found )

    {

	send_to_char( "Board not found.\n\r", ch );

	return;

    }



    if ( !str_cmp( arg2, "ovnum" ) )

    {

	if ( !get_obj_index(value) )

	{

	    send_to_char( "No such object.\n\r", ch );

	    return;

	}

	board->board_obj = value;

	write_boards_txt( );

	send_to_char( "Done.  (board's object vnum set)\n\r", ch );

	return;

    }



    if ( !str_cmp( arg2, "read" ) )

    {

	if ( value < 0 || value > MAX_LEVEL )

	{

	    send_to_char( "Value outside valid character level range.\n\r", ch );

	    return;

	}

	board->min_read_level = value;

	write_boards_txt( );

	send_to_char( "Done.  (minimum reading level set)\n\r", ch );

	return;

    }



    if ( !str_cmp( arg2, "read_group" ) )

    {

	if ( !argument || argument[0] == '\0' )

	{

	    send_to_char( "No reading group specified.\n\r", ch );

	    return;

	}

	DISPOSE( board->read_group );

        if ( !str_cmp( argument, "none" ) )

	  board->read_group = str_dup( "" );

        else

	  board->read_group = str_dup( argument );

	write_boards_txt( );

	send_to_char( "Done.  (reading group set)\n\r", ch );

	return;

    }



    if ( !str_cmp( arg2, "post_group" ) )

    {

	if ( !argument || argument[0] == '\0' )

	{

	    send_to_char( "No posting group specified.\n\r", ch );

	    return;

	}

	DISPOSE( board->post_group );

        if ( !str_cmp( argument, "none" ) )

	  board->post_group = str_dup( "" );

        else

	  board->post_group = str_dup( argument );

	write_boards_txt( );

	send_to_char( "Done.  (posting group set)\n\r", ch );

	return;

    }

   if ( !str_cmp( arg2, "postmessg" ) )

   {

        if ( !argument || argument[0] == '\0' )

        {

            send_to_char( "No message specified.\n\r", ch );

            return;

        }

	if ( board->postmessg )

          	DISPOSE( board->postmessg );

        if ( !str_cmp( argument, "none" ) )

		board->postmessg = NULL;

        else

	{

            sprintf( buf, "%s", argument );

            board->postmessg = str_dup( buf );

	}

        write_boards_txt( );

	send_to_char("Done.\n\r", ch );

	return;

   }

   if ( !str_cmp( arg2, "opost" ) )

   {

        if ( !argument || argument[0] == '\0' )

        {

            send_to_char( "No message specified.\n\r", ch );

            return;

        }

	if ( board->opostmessg )

           DISPOSE( board->opostmessg );

        if ( !str_cmp( argument, "none" ) )

		board->opostmessg = NULL;

        else

	{

            sprintf( buf, "%s", argument );

            board->opostmessg = str_dup( buf );

	}

        write_boards_txt( );

	send_to_char("Done.\n\r", ch );

	return;

   }

   if ( !str_cmp( arg2, "oremove" ) )

   {

        if ( !argument || argument[0] == '\0' )

        {

            send_to_char( "No message specified.\n\r", ch );

            return;

        }

	if ( board->oremovemessg )

           DISPOSE( board->oremovemessg );

        if ( !str_cmp( argument, "none" ) )

		board->oremovemessg = NULL;

        else

	{

            sprintf( buf, "%s", argument );

            board->oremovemessg = str_dup( buf );

	}

        write_boards_txt( );

	send_to_char("Done.\n\r", ch );

	return;

   }

   if ( !str_cmp( arg2, "otake" ) )

   {

        if ( !argument || argument[0] == '\0' )

        {

            send_to_char( "No message specified.\n\r", ch );

            return;

        }

	if ( board->otakemessg )

           DISPOSE( board->otakemessg );

        if ( !str_cmp( argument, "none" ) )

		board->otakemessg = NULL;

        else

	{

            sprintf( buf, "%s", argument );

            board->otakemessg = str_dup( buf );

	}

        write_boards_txt( );

	send_to_char("Done.\n\r", ch );

	return;

   }

   if ( !str_cmp( arg2, "ocopy" ) )

   {

        if ( !argument || argument[0] == '\0' )

        {

            send_to_char( "No message specified.\n\r", ch );

            return;

        }

	if ( board->ocopymessg )

           DISPOSE( board->ocopymessg );

        if ( !str_cmp( argument, "none" ) )

		board->ocopymessg = NULL;

        else

	{

            sprintf( buf, "%s", argument );

            board->ocopymessg = str_dup( buf );

	}

        write_boards_txt( );

	send_to_char("Done.\n\r", ch );

	return;

   }

   if ( !str_cmp( arg2, "oread" ) )

   {

	if ( !argument || argument[0] == '\0' )

	{

	    send_to_char( "No message sepcified.\n\r", ch );

	    return;

	}

	if ( board->oreadmessg )

	   DISPOSE( board->oreadmessg );

        if ( !str_cmp( argument, "none" ) )

                board->oreadmessg = NULL;

        else

        {

            sprintf( buf, "%s", argument );

            board->oreadmessg = str_dup( buf );

        }

        write_boards_txt( );

        send_to_char("Done.\n\r", ch );

        return;

   }

   if ( !str_cmp( arg2, "olist" ) )

   {

        if ( !argument || argument[0] == '\0' )

        {

            send_to_char( "No message specified.\n\r", ch );

            return;

        }

	if ( board->olistmessg )

           DISPOSE( board->olistmessg );

        if ( !str_cmp( argument, "none" ) )

		board->olistmessg = NULL;

        else

	{

            sprintf( buf, "%s", argument );

            board->olistmessg = str_dup( buf );

	}

        write_boards_txt( );

	send_to_char("Done.\n\r", ch );

	return;

   }

   if ( !str_cmp( arg2, "extra_removers" ) )

    {

        if ( !argument || argument[0] == '\0' )

        {

            send_to_char( "No names specified.\n\r", ch );

            return;

        }

        if ( !str_cmp( argument, "none" ) )

            buf[0] = '\0';

        else

            sprintf( buf, "%s %s", board->extra_removers, argument );

        DISPOSE( board->extra_removers );

        board->extra_removers = str_dup( buf );

        write_boards_txt( );

        send_to_char( "Done.  (extra removers set)\n\r", ch );

        return;

    }



    if ( !str_cmp( arg2, "extra_readers" ) )

    {

	if ( !argument || argument[0] == '\0' )

	{

	    send_to_char( "No names specified.\n\r", ch );

	    return;

	}

	if ( !str_cmp( argument, "none" ) )

	    buf[0] = '\0';

	else

	    sprintf( buf, "%s %s", board->extra_readers, argument );

	DISPOSE( board->extra_readers );

	board->extra_readers = str_dup( buf );

	write_boards_txt( );

	send_to_char( "Done.  (extra readers set)\n\r", ch );

	return;

    }



    if ( !str_cmp( arg2, "filename" ) )

    {

	if ( !argument || argument[0] == '\0' )

	{

	    send_to_char( "No filename specified.\n\r", ch );

	    return;

	}

	DISPOSE( board->note_file );

	board->note_file = str_dup( argument );

	write_boards_txt( );

	send_to_char( "Done.  (board's filename set)\n\r", ch );

	return;

    }



    if ( !str_cmp( arg2, "post" ) )

    {

	if ( value < 0 || value > MAX_LEVEL )

	{

	  send_to_char( "Value outside valid character level range.\n\r", ch );

	  return;

	}

	board->min_post_level = value;

	write_boards_txt( );

	send_to_char( "Done.  (minimum posting level set)\n\r", ch );

	return;

    }



    if ( !str_cmp( arg2, "remove" ) )

    {

	if ( value < 0 || value > MAX_LEVEL )

	{

	  send_to_char( "Value outside valid character level range.\n\r", ch );

	  return;

	}

	board->min_remove_level = value;

	write_boards_txt( );

	send_to_char( "Done.  (minimum remove level set)\n\r", ch );

	return;

    }



    if ( !str_cmp( arg2, "maxpost" ) )

    {

	if ( value < 1 || value > 999 )

	{

	  send_to_char( "Value out of range.\n\r", ch );

	  return;

	}

	board->max_posts = value;

	write_boards_txt( );

	send_to_char( "Done.  (maximum number of posts set)\n\r", ch );

	return;

    }

    if ( !str_cmp( arg2, "type" ) )

    {

	if ( value < 0 || value > 1 )

	{

	  send_to_char( "Value out of range.\n\r", ch );

	  return;

	}

	board->type = value;

	write_boards_txt( );

	send_to_char( "Done.  (board's type set)\n\r", ch );

	return;

    }



    do_bset( ch, "" );

    return;

}





void do_bstat( CHAR_DATA *ch, char *argument )

{

    BOARD_DATA *board;

    bool found;

    char arg[MAX_INPUT_LENGTH];



    argument = one_argument( argument, arg );

    found = FALSE;

    for ( board = first_board; board; board = board->next )

      if ( !str_cmp( arg, board->note_file ) )

      {

	found = TRUE;

	break;

      }



    if ( !found )

    {

      if ( argument && argument[0] != '\0' )

      {

        send_to_char_color( "&GBoard not found.  Usage: bstat <board filename>\n\r", ch );

        return;

      }

      else

      {

        board = find_board( ch );

	if ( !board )

	{

	  send_to_char_color( "&GNo board present.  Usage: bstat <board filename>\n\r", ch );

	  return;

	}

      }

    }



    ch_printf_color( ch, "\n\r&GFilename: &W%-15.15s &GOVnum: &W%-5d  &GRead: &W%-2d  &GPost: &W%-2d  &GRemove: &W%-2d\n\r&GMaxpost:  &W%-3d              &GType: &W%d\n\r&GPosts:    %d\n\r",

		board->note_file,	 board->board_obj,

		board->min_read_level,	 board->min_post_level,

		board->min_remove_level, board->max_posts,

                board->type,		 board->num_posts );



    ch_printf_color( ch, "&GRead_group:     &W%s\n\r&GPost_group:     &W%s\n\r&GExtra_readers:  &W%s\n\r&GExtra_removers: &W%s\n\r",

		board->read_group, board->post_group, board->extra_readers, board->extra_removers );

    ch_printf_color( ch, "&GPost Message:    %s\n\r", board->postmessg?board->postmessg:"Default Message" );

    ch_printf_color( ch, "&GOPost Message:   %s\n\r", board->opostmessg?board->opostmessg:"Default Message" );

    ch_printf_color( ch, "&GORead Message:   %s\n\r", board->oreadmessg?board->oreadmessg:"Default Message" );

    ch_printf_color( ch, "&GORemove Message: %s\n\r", board->oremovemessg?board->oremovemessg:"Default Message" );

    ch_printf_color( ch, "&GOTake Message:   %s\n\r", board->otakemessg?board->otakemessg:"Default Message" );

    ch_printf_color( ch, "&GOList Message:   %s\n\r", board->olistmessg?board->olistmessg:"Default Message" );

    ch_printf_color( ch, "&GOCopy Message:   %s\n\r", board->ocopymessg?board->ocopymessg:"Default Message" );

    return;

}





void do_boards( CHAR_DATA *ch, char *argument )

{

    BOARD_DATA *board;



    if ( !first_board )

    {

      send_to_char_color( "T&Ghere are no boards yet.\n\r", ch );

      return;

    }

    for ( board = first_board; board; board = board->next )

      pager_printf_color( ch, "&G%-15.15s #: %5d Read: %2d Post: %2d Rmv: %2d Max: %3d Posts: &g%3d &GType: %d\n\r",

	board->note_file,	 board->board_obj,

	board->min_read_level,	 board->min_post_level,

	board->min_remove_level, board->max_posts, board->num_posts,

	board->type);

}



void mail_count(CHAR_DATA *ch)

{

  BOARD_DATA *board;

  NOTE_DATA *note;

  int cnt_to = 0, cnt_from=0;



  for ( board = first_board; board; board = board->next )

    if ( board->type == BOARD_MAIL && can_read(ch, board) )

      for ( note = board->first_note; note; note = note->next )

      {

        if ( is_name(ch->name, note->to_list) )

          ++cnt_to;

	else if ( !str_cmp(ch->name, note->sender ) )

	  ++cnt_from;

      }

  if ( cnt_to )

    ch_printf(ch, "Tienes %d mensaje%s nuevos en tu mail.\n\r", cnt_to,

	(cnt_to > 1) ? "s " : " ");



  if ( cnt_from )

    ch_printf(ch, "Has escrito %d mensaje%s por tu mail.\n\r", cnt_from,

	(cnt_from > 1) ? "s " : " ");

  return;

}



void do_board ( CHAR_DATA *ch, char *argument)

{

  char arg[MAX_INPUT_LENGTH];

  char buf[MAX_STRING_LENGTH];



  if ( IS_NPC(ch) )

     return;



  if (ch->board == NULL)

     ch->board = "1300";



  one_argument (argument, arg);

  if (arg[0] == '\0' )

  {

     sprintf (buf, "\n\r&GCambiar a que tablero? (Actual: &w%s&G)\n\r",

        ch->board == "1300"  ? "General"       :

        ch->board == "1302"  ? "Ideas"         :

        ch->board == "1301"  ? "Inmortal"      :

        ch->board == "1302"  ? "Correo (Mail)" :

        ch->board == "1303"  ? "Rol"           :

                               "General"      );

     send_to_char_color (buf,ch);

     return;

  }



  if ( !str_prefix ( arg, "general") )

  {

    send_to_char_color ("&GCambias al tablero general.\n\r", ch);

    ch->board = "1300";

    return;

  }

  if ( !str_prefix ( arg, "mail") )

  {

    send_to_char_color ("&GPasas a leer tus mails.\n\r", ch);

    ch->board = "1302";

    return;

  }



  if ( !str_prefix (arg, "ideas") )

  {

    send_to_char_color ("&GCambias al tablero ideas.\n\r", ch);

    ch->board = "1302";

    return;

  }



  if ( !str_prefix (arg, "rol") )

  {

    send_to_char_color ("&GCambias al tablero rol.\n\r", ch);

    ch->board = "1303";

    return;

  }



  if ( !str_prefix (arg, "inmortal") )

  {

    /* if ( !IS_IMMORTAL(ch) )

     {*/

        send_to_char_color ("&GCambias al tablero inmortal", ch);

        ch->board = "1301";

        return;

   /*  }

     send_to_char_color ("&GNo puedes leer en este foro.", ch);

     return;*/

   }



   send_to_char_color ("&GCambiar a que tablero?.\n\r", ch);

   return;

}



