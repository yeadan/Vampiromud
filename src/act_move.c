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
 *			   Player movement module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"



const	sh_int	movement_loss	[SECT_MAX]	=
{
    1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 5, 7, 4
};

char *	const	dir_name	[]		=
{
    "norte", "este", "sur", "oeste", "arriba", "abajo",
    "noreste", "noroeste", "sureste", "suroeste", "algun sitio"
};

const	int	trap_door	[]		=
{
    TRAP_N, TRAP_E, TRAP_S, TRAP_W, TRAP_U, TRAP_D,
    TRAP_NE, TRAP_NW, TRAP_SE, TRAP_SW
};


const	sh_int	rev_dir		[]		=
{
    2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};


ROOM_INDEX_DATA * vroom_hash [64];


/*
 * Local functions.
 */
OBJ_DATA	*has_key	args( ( CHAR_DATA *ch, int key ) );


char *	const		sect_names[SECT_MAX][2] =
{
    { "En una habitacion","dentro"},	{ "En una ciudad","ciudades"	},
    { "En un campo","campos"	},	{ "En un bosque","bosques"	},
    { "colina",	"colinas"		},{ "En una montanya","montanyas"},
    { "En el agua","aguas"	},	{ "En aguas bravas","aguas"	},
    { "Bajo el aguar", "Bajo las aguas" },{ "En el aire","aire"		},
    { "In a desert","deserts"	},	{ "Algun sitio","desconocido"	},
    { "suelo oceanico", "suelo oceanico" },{ "bajo tierra", "bajo tierra"}
};


const	int		sent_total[SECT_MAX]		=
{
    3, 5, 4, 4, 1, 1, 1, 1, 1, 2, 2, 25, 1, 1
};

char *	const		room_sents[SECT_MAX][25]	=
{
    {
	"rough hewn walls of granite with the occasional spider crawling around",
	"signs of a recent battle from the bloodstains on the floor",
	"a damp musty odour not unlike rotting vegetation"
    },

    {
	"the occasional stray digging through some garbage",
	"merchants trying to lure customers to their tents",
	"some street people putting on an interesting display of talent",
	"an argument between a customer and a merchant about the price of an item",
	"several shady figures talking down a dark alleyway"
    },

    {
	"sparce patches of brush and shrubs",
	"a small cluster of trees far off in the distance",
	"grassy fields as far as the eye can see",
	"a wide variety of weeds and wildflowers"
    },

    {
	"tall, dark evergreens prevent you from seeing very far",
	"many huge oak trees that look several hundred years old",
	"a solitary lonely weeping willow",
	"a patch of bright white birch trees slender and tall"
    },

    {
	"rolling hills lightly speckled with violet wildflowers"
    },

    {
	"the rocky mountain pass offers many hiding places"
    },

    {
	"the water is smooth as glass"
    },

    {
	"rough waves splash about angrily"
    },

    {
	"a small school of fish"
    },

    {
	"the land far below",
	"a misty haze of clouds"
    },

    {
	"sand as far as the eye can see",
	"an oasis far in the distance"
    },

    {
	"nada raro",	"nada raro",	"nada raro",
	"nada raro",	"nada raro",	"nada raro",
	"nada raro",	"nada raro",	"nada raro",
	"nada raro",	"nada raro",	"nada raro",
	"nada raro",	"nada raro",	"nada raro",
	"nada raro",	"nada raro",	"nada raro",
	"nada raro",	"nada raro",	"nada raro",
	"nada raro",	"nada raro",	"nada raro",
	"nada raro",
    },

    {
        "rocks and coral which litter the ocean floor."
    },

    {
	"a lengthy tunnel of rock."
    }

};

char *grab_word( char *argument, char *arg_first )
{
    char cEnd;
    sh_int count;

    count = 0;

    while ( isspace(*argument) )
	argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	cEnd = *argument++;

    while ( *argument != '\0' || ++count >= 255 )
    {
	if ( *argument == cEnd )
	{
	    argument++;
	    break;
	}
	*arg_first++ = *argument++;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	argument++;

    return argument;
}

char *wordwrap( char *txt, sh_int wrap )
{
    static char buf[MAX_STRING_LENGTH];
    char *bufp;

    buf[0] = '\0';
    bufp = buf;
    if ( txt != NULL )
    {
        char line[MAX_STRING_LENGTH];
        char temp[MAX_STRING_LENGTH];
        char *ptr, *p;
        int ln, x;

	++bufp;
        line[0] = '\0';
        ptr = txt;
        while ( *ptr )
        {
	  ptr = grab_word( ptr, temp );
          ln = strlen( line );  x = strlen( temp );
          if ( (ln + x + 1) < wrap )
          {
	    if ( ln>0 && line[ln-1] == '.' )
              strcat( line, "  " );
	    else
              strcat( line, " " );
            strcat( line, temp );
            p = strchr( line, '\n' );
            if ( !p )
              p = strchr( line, '\r' );
            if ( p )
            {
                strcat( buf, line );
                line[0] = '\0';
            }
          }
          else
          {
            strcat( line, "\r\n" );
            strcat( buf, line );
            strcpy( line, temp );
          }
        }
        if ( line[0] != '\0' )
            strcat( buf, line );
    }
    return bufp;
}

void decorate_room( ROOM_INDEX_DATA *room )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int nRand;
    int iRand, len;
    int previous[8];
    int sector = room->sector_type;
    char *pre = "Distingues ", *post = ".";

    if ( room->name )
      STRFREE( room->name );
    if ( room->description )
      STRFREE( room->description );

    room->name	  = STRALLOC( "En una habitacion virtual" );
    room->description = STRALLOC( "Estas en un camino.\n\r" );

    return; /* Return with above till I can figure out what is
    	     * wrong with it. --Shaddai
	     */
    room->name	= STRALLOC( sect_names[sector][0] );
    buf[0] = '\0';
    nRand = number_range( 1, UMIN(8,sent_total[sector]) );

    for ( iRand = 0; iRand < nRand; iRand++ )
	previous[iRand] = -1;

    for ( iRand = 0; iRand < nRand; iRand++ )
    {
	while ( previous[iRand] == -1 )
	{
	    int x, z;

	    x = number_range( 0, sent_total[sector]-1 );

	    for ( z = 0; z < iRand; z++ )
		if ( previous[z] == x )
		  break;

	    if ( z < iRand )
		  continue;

	    previous[iRand] = x;

	    len = strlen(buf);
	    if ( len == 0 )
	    {
	      switch( number_range(1, 2 * (iRand == nRand -1) ? 1 : 2) )
	      {
		case 1:	pre = "Distingues ";	post = ".";	 break;
		case 2: pre = "Ves ";		post = ".";	 break;
		case 3: pre = "Ves ";		post = ", and "; break;
		case 4: pre = "Distingues ";	post = ", and "; break;
	      }
	      sprintf( buf2, "%s%s%s", pre, room_sents[sector][x], post );
	    }
	    else
	    if ( iRand != nRand -1 )
	    {
	      if ( buf[len-1] == '.' )
	      switch( number_range(0, 3) )
	      {
		case 0: pre = "Distingues ";	post = ".";	 break;
		case 1: pre = "Ves ";		post = ", and "; break;
		case 2: pre = "Ves ";		post = ".";	 break;
		case 3: pre = "Distingues ";	post = ", and "; break;
	      }
	      else
	      switch( number_range(0, 3) )
	      {
		case 0: pre = ""; post = ".";			break;
		case 1: pre = ""; post = " no demasiado lejos.";	break;
		case 2: pre = ""; post = ", y ";		break;
		case 3: pre = ""; post = " cercano.";		break;
	      }
	      sprintf( buf2, "%s%s%s", pre, room_sents[sector][x], post );
	    }
	    else
		sprintf( buf2, "%s.", room_sents[sector][x] );
	    if ( len > 5 && buf[len-1] == '.' )
	    {
		strcat( buf, "  " );
		buf2[0] = UPPER(buf2[0] );
	    }
	    else
	    if ( len == 0 )
	        buf2[0] = UPPER(buf2[0] );
	    strcat( buf, buf2 );
	}
    }
    /* Below is the line that causes the uninitialized memory read --Shaddai */
    sprintf( buf2, "%s\n\r", wordwrap(buf, 78) );
    room->description = STRALLOC( buf2 );
}

/*
 * Remove any unused virtual rooms				-Thoric
 */
void clear_vrooms( )
{
    int hash;
    ROOM_INDEX_DATA *room, *room_next, *prev;

    for ( hash = 0; hash < 64; hash++ )
    {
	while ( vroom_hash[hash]
	&&     !vroom_hash[hash]->first_person
	&&     !vroom_hash[hash]->first_content )
	{
	    room = vroom_hash[hash];
	    vroom_hash[hash] = room->next;
	    clean_room( room );
	    DISPOSE( room );
	    --top_vroom;
	}
	prev = NULL;
	for ( room = vroom_hash[hash]; room; room = room_next )
	{
	    room_next = room->next;
	    if ( !room->first_person && !room->first_content )
	    {
		if ( prev )
		  prev->next = room_next;
		clean_room( room );
		DISPOSE( room );
		--top_vroom;
	    }
	    if ( room )
	      prev = room;
	}
    }
}

char *rev_exit( sh_int vdir )
{
    switch( vdir )
    {
	default: return "algun sitio";
	case 0:  return "el norte";
	case 1:  return "el este";
	case 2:  return "el sur";
	case 3:  return "el oeste";
	case 4:  return "arriba";
	case 5:  return "abajo";
	case 6:  return "el noreste";
	case 7:  return "el noroeste";
	case 8:  return "el sureste";
	case 9:  return "el suroeste";
    }

    return "<???>";
}

/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
EXIT_DATA *get_exit( ROOM_INDEX_DATA *room, sh_int dir )
{
    EXIT_DATA *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir )
         return xit;
    return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
EXIT_DATA *get_exit_to( ROOM_INDEX_DATA *room, sh_int dir, int vnum )
{
    EXIT_DATA *xit;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (xit = room->first_exit; xit; xit = xit->next )
       if ( xit->vdir == dir && xit->vnum == vnum )
         return xit;
    return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
EXIT_DATA *get_exit_num( ROOM_INDEX_DATA *room, sh_int count )
{
    EXIT_DATA *xit;
    int cnt;

    if ( !room )
    {
	bug( "Get_exit: NULL room", 0 );
	return NULL;
    }

    for (cnt = 0, xit = room->first_exit; xit; xit = xit->next )
       if ( ++cnt == count )
         return xit;
    return NULL;
}


/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance( CHAR_DATA *ch, sh_int move )
{
    int cur, max;

    max = can_carry_w(ch);
    cur = ch->carry_weight;
    if ( cur >= max )
      return move * 4;
    else
    if ( cur >= max * 0.95 )
      return move * 3.5;
    else
    if ( cur >= max * 0.90 )
      return move * 3;
    else
    if ( cur >= max * 0.85 )
      return move * 2.5;
    else
    if ( cur >= max * 0.80 )
      return move * 2;
    else
    if ( cur >= max * 0.75 )
      return move * 1.5;
    else
      return move;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall( CHAR_DATA *ch, int fall )
{
    if ( ch->level < LEVEL_AVATAR && IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR )
    &&   CAN_GO(ch, DIR_DOWN)
    && (!IS_AFFECTED( ch, AFF_FLYING )
    || ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) ) ) )
    {
	if ( fall > 80 )
	{
	   bug( "Falling (in a loop?) more than 80 rooms: vnum %d", ch->in_room->vnum );
	   char_from_room( ch );
	   char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	   fall = 0;
	   return TRUE;
	}
	set_char_color( AT_FALLING, ch );
	send_to_char( "Estas cayendo...\n\r", ch );
	move_char( ch, get_exit(ch->in_room, DIR_DOWN), ++fall );
	return TRUE;
    }
    return FALSE;
}


/*
 * create a 'virtual' room					-Thoric
 */
ROOM_INDEX_DATA *generate_exit( ROOM_INDEX_DATA *in_room, EXIT_DATA **pexit )
{
    EXIT_DATA *xit, *bxit;
    EXIT_DATA *orig_exit = (EXIT_DATA *) *pexit;
    ROOM_INDEX_DATA *room, *backroom;
    int brvnum;
    int serial;
    int roomnum;
    int distance = -1;
    int vdir = orig_exit->vdir;
    sh_int hash;
    bool found = FALSE;

    if ( in_room->vnum > 32767 )	/* room is virtual */
    {
	serial = in_room->vnum;
	roomnum = in_room->tele_vnum;
	if ( (serial & 65535) == orig_exit->vnum )
	{
	  brvnum = serial >> 16;
	  --roomnum;
	  distance = roomnum;
	}
	else
	{
	  brvnum = serial & 65535;
	  ++roomnum;
	  distance = orig_exit->distance - 1;
	}
	backroom = get_room_index( brvnum );
    }
    else
    {
	int r1 = in_room->vnum;
	int r2 = orig_exit->vnum;

	brvnum = r1;
	backroom = in_room;
	serial = (UMAX( r1, r2 ) << 16) | UMIN( r1, r2 );
	distance = orig_exit->distance - 1;
	roomnum = r1 < r2 ? 1 : distance;
    }
    hash = serial % 64;

    for ( room = vroom_hash[hash]; room; room = room->next )
	if ( room->vnum == serial && room->tele_vnum == roomnum )
	{
	    found = TRUE;
	    break;
	}
    if ( !found )
    {
	CREATE( room, ROOM_INDEX_DATA, 1 );
	room->area	  = in_room->area;
	room->vnum	  = serial;
	room->tele_vnum	  = roomnum;
	room->sector_type = in_room->sector_type;
	room->room_flags  = in_room->room_flags;
	decorate_room( room );
	room->next	  = vroom_hash[hash];
	vroom_hash[hash]  = room;
	++top_vroom;
    }
    if ( !found || (xit=get_exit(room, vdir))==NULL )
    {
	xit = make_exit(room, orig_exit->to_room, vdir);
	xit->keyword		= STRALLOC( "" );
	xit->description	= STRALLOC( "" );
	xit->key		= -1;
	xit->distance = distance;
    }
    if ( !found )
    {
	bxit = make_exit(room, backroom, rev_dir[vdir]);
	bxit->keyword		= STRALLOC( "" );
	bxit->description	= STRALLOC( "" );
	bxit->key		= -1;
	if ( (serial & 65535) != orig_exit->vnum )
	  bxit->distance = roomnum;
	else
	{
	  EXIT_DATA *tmp = get_exit( backroom, vdir );
	  int fulldist = tmp->distance;

	  bxit->distance = fulldist - distance;
	}
    }
    (EXIT_DATA *) pexit = xit;
    return room;
}

ch_ret move_char( CHAR_DATA *ch, EXIT_DATA *pexit, int fall )
{
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    ROOM_INDEX_DATA *from_room;
    OBJ_DATA *boat;
    char buf[MAX_STRING_LENGTH];
    char *txt;
    char *dtxt;
    ch_ret retcode;
    sh_int door, distance;
    bool drunk = FALSE;
    bool nuisance = FALSE;
    bool brief = FALSE;

    if ( !IS_NPC( ch ) )
    {
      if ( IS_DRUNK( ch, 2 ) && ( ch->position != POS_SHOVE )
	&& ( ch->position != POS_DRAG ) && (ch->level < LEVEL_AVATAR ) )
	drunk = TRUE;

      /* Nuisance flag, makes them walk in random directions 50% of the time
       * -Shaddai
       */

      if ( ch->level < LEVEL_AVATAR && ch->pcdata->nuisance && ch->pcdata->nuisance->flags > 8 &&
	 ( ch->position != POS_SHOVE ) && ( ch->position != POS_DRAG ) &&
	 number_percent() > (ch->pcdata->nuisance->flags*ch->pcdata->nuisance->power))
	nuisance = TRUE;
    }

    if ( (nuisance || drunk) && !fall )
    {
      door = number_door();
      pexit = get_exit( ch->in_room, door );
    }

#ifdef DEBUG
    if ( pexit )
    {
	sprintf( buf, "move_char: %s to door %d", ch->name, pexit->vdir );
	log_string( buf );
    }
#endif

    retcode = rNONE;
    txt = NULL;

    if ( IS_NPC(ch) && xIS_SET(ch->act, ACT_MOUNTED) )
      return retcode;

    in_room = ch->in_room;
    from_room = in_room;
    if ( !pexit || (to_room = pexit->to_room) == NULL )
    {
	if ( drunk && ch->position != POS_MOUNTED
	&&   ch->in_room->sector_type != SECT_WATER_SWIM
	&&   ch->in_room->sector_type != SECT_WATER_NOSWIM
	&&   ch->in_room->sector_type != SECT_UNDERWATER
	&&   ch->in_room->sector_type != SECT_OCEANFLOOR
	&&   ch->level < LEVEL_AVATAR )
	{
	  switch ( number_bits( 4 ) )
	  {
	    default:
		act( AT_ACTION, "Te tropiezas con algo iendo todo borracho.", ch, NULL, NULL, TO_CHAR );
		act( AT_ACTION, "$n se tropieza de la mierda que lleva.", ch, NULL, NULL, TO_ROOM );
		break;
	    case 3:
		act( AT_ACTION, "De la borrachera que llevas te haces a ti mismo la zancadilla y te caes al suelo.", ch, NULL, NULL, TO_CHAR );
		act( AT_ACTION, "$n se hace la zancadilla y cae de la borrachera que lleva.", ch, NULL, NULL, TO_ROOM );
		ch->position = POS_RESTING;
	        break;
	    case 4:
		act( AT_SOCIAL, "You utter a string of slurred obscenities.", ch, NULL, NULL, TO_CHAR) ;
		act( AT_ACTION, "Something blurry and immovable has intercepted you as you stagger along.", ch, NULL, NULL, TO_CHAR );
		act( AT_HURT, "Oh geez... THAT really hurt.  Everything slowly goes dark and numb...", ch, NULL, NULL, TO_CHAR );
		act( AT_ACTION, "$n drunkenly staggers into something.", ch, NULL, NULL, TO_ROOM );
		act( AT_SOCIAL, "$n utters a string of slurred obscenities: @*&^%@*&!", ch, NULL, NULL, TO_ROOM );
		act( AT_ACTION, "$n topples to the ground with a thud.", ch, NULL, NULL, TO_ROOM );
		ch->position = POS_INCAP;
		break;
	  }
	}
	else if ( nuisance )
	  act( AT_ACTION, "Empiezas a dar vueltas intentando recordar a donde ibas.", ch, NULL, NULL, TO_CHAR );
	else if ( drunk )
	  act( AT_ACTION, "Dios mio que borracho que vas... donde conyo vas?.", ch, NULL, NULL, TO_CHAR );
        else
	  send_to_char( "No puedes ir por alli.\n\r", ch );
	return rNONE;
    }

    door = pexit->vdir;
    distance = pexit->distance;

    /*
     * Exit is only a "window", there is no way to travel in that direction
     * unless it's a door with a window in it		-Thoric
     */
    if ( IS_SET( pexit->exit_info, EX_WINDOW )
    &&  !IS_SET( pexit->exit_info, EX_ISDOOR ) )
    {
	send_to_char( "No puedes ir por alli.\n\r", ch );
	return rNONE;
    }

    if (  IS_SET(pexit->exit_info, EX_PORTAL)
       && IS_NPC(ch) )
    {
        act( AT_PLAIN, "Los mobs no pueden usar portales.", ch, NULL, NULL, TO_CHAR );
	return rNONE;
    }

    if ( IS_SET(pexit->exit_info, EX_NOMOB)
	&& IS_NPC(ch) )
    {
	act( AT_PLAIN, "Los mobs no pueden entrar alli.", ch, NULL, NULL, TO_CHAR );
	return rNONE;
    }

    if ( IS_SET(pexit->exit_info, EX_CLOSED)
    && (!IS_AFFECTED(ch, AFF_PASS_DOOR || 
	!xIS_SET(ch->afectado_por, DAF_INCORPOREO))
    ||   IS_SET(pexit->exit_info, EX_NOPASSDOOR)) )
    {
	if ( !IS_SET( pexit->exit_info, EX_SECRET )
	&&   !IS_SET( pexit->exit_info, EX_DIG ) )
	{
	  if ( drunk )
	  {
	    act( AT_PLAIN, "$n corre hacia el $d en $s estado de embriaguez.", ch,
		NULL, pexit->keyword, TO_ROOM );
	    act( AT_PLAIN, "Corres hacia el $d iendo borracho.", ch,
		NULL, pexit->keyword, TO_CHAR );
	  }
	 else
	  act( AT_PLAIN, "El $d esta cerrado.", ch, NULL, pexit->keyword, TO_CHAR );
	}
       else
	{
	  if ( drunk )
	    send_to_char( "Te tambaleas de lado a lado *HIC*.\n\r", ch );
	   else
	    send_to_char( "No puedes ir por alli.\n\r", ch );
	}

        if (!xIS_SET(ch->afectado_por, DAF_INCORPOREO))
	return rNONE;
    }

    /*
     * Crazy virtual room idea, created upon demand.		-Thoric
     */
    if ( distance > 1 )
	if ( (to_room=generate_exit(in_room, &pexit)) == NULL )
	    send_to_char( "No puedes ir por alli.\n\r", ch );

    if ( !fall
    &&   IS_AFFECTED(ch, AFF_CHARM)
    &&   ch->master
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "Que? Y abandonar a tu querido amo?\n\r", ch );
	return rNONE;
    }

    if ( !fall
    && xIS_SET(ch->act, PLR_DOMINADO)
    &&   ch->master
    &&   in_room == ch->master->in_room )
    {
	send_to_char( "Que?? Y abandonar a tu querido amo?\n\r", ch );
	return rNONE;
    }

    if ( room_is_private( to_room ) )
    {
	send_to_char( "Esa habitacion es privada ahora.\n\r", ch );
	return rNONE;
    }

    if ( room_is_dnd(ch, to_room) )
    {
	send_to_char( "Esta habitacion es \"No molestar\" ahora.\n\r", ch );
	return rNONE;
    }

    if ( !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  ch->in_room->area != to_room->area )
    {
	if ( ch->level < to_room->area->low_hard_range )
	{
	    set_char_color( AT_TELL, ch );
	    switch( to_room->area->low_hard_range - ch->level )
	    {
		case 1:
		  send_to_char( "Una voz en tu mente dice, 'Pronto estaras preparado para seguir ese camino...'", ch );
		  break;
		case 2:
		  send_to_char( "Una voz en tu mente dice, 'Pronto deberias estar preparado para seguir ese camino...'", ch );
		  break;
		case 3:
		  send_to_char( "Una voz en tu mente dice, 'No estas preparado para seguir ese camino... todavia.'.\n\r", ch);
		  break;
		default:
		  send_to_char( "Una voz en tu mente dice, 'No estas preparado para seguir ese camino.'\n\r", ch);
	    }
	    return rNONE;
	}
	else
	if ( ch->level > to_room->area->hi_hard_range )
	{
	    set_char_color( AT_TELL, ch );
	    send_to_char( "Una voz en tu mente dice, 'No hay nada para ti hacia ese camino.'", ch );
	    return rNONE;
	}
    }

    if ( !fall && !IS_NPC(ch) )
    {
	/*int iClass;*/
	int move;
/* Pretty sure we don't need to check for guilds anymore now that we have
   the new dh. -- Narn
*/
/*
	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
	{
	    if ( iClass != ch->class
	    &&   to_room->vnum == class_table[iClass]->guild )
	    {
		send_to_char( "No se te esta permitido entrar alli.\n\r", ch );
		return rNONE;
	    }
	}
*/

/* Prevent deadlies from entering a nopkill-flagged area from a
   non-flagged area, but allow them to move around if already
   inside a nopkill area. - Blodkai
*/

        if (  IS_SET( to_room->area->flags, AFLAG_NOPKILL )
        &&   !IS_SET( ch->in_room->area->flags, AFLAG_NOPKILL )
        && (  IS_PKILL (ch) && !IS_IMMORTAL (ch) ) )
        {
	    set_char_color( AT_MAGIC, ch );
            send_to_char( "\n\rUna fuerza divina prohibe entrar a jugadores PK en esta area...\n\r", ch);
            return rNONE;
        }

	if ( in_room->sector_type == SECT_AIR
	||   to_room->sector_type == SECT_AIR
	||   IS_SET( pexit->exit_info, EX_FLY ) )
	{
	    if ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLYING ) )
	    {
		send_to_char( "Tu montura no puede volar.\n\r", ch );
		return rNONE;
	    }
	    if ( !ch->mount && !IS_AFFECTED(ch, AFF_FLYING) )
	    {
		send_to_char( "Necesitas volar para ir alli.\n\r", ch );
		return rNONE;
	    }
	}

	if ( in_room->sector_type == SECT_WATER_NOSWIM
	||   to_room->sector_type == SECT_WATER_NOSWIM )
	{
	    if ( (ch->mount && !IS_FLOATING(ch->mount)) || !IS_FLOATING(ch) )
	    {
		/*
		 * Look for a boat.
		 * We can use the boat obj for a more detailed description.
		 */
		if ( ch->level > LEVEL_AVATAR || (boat=get_objtype(ch, ITEM_BOAT)) != NULL )
		{
		    if ( drunk )
			txt = "rema en circulo";
		    else
			txt = "rema";
		}
		else
		{
		    if ( ch->mount )
			send_to_char( "Tu montura podria ahogarse!\n\r", ch );
		    else
			send_to_char( "Necesitas un barco para poder ir.\n\r", ch );
		    return rNONE;
		}
	    }
	}

	if ( IS_SET( pexit->exit_info, EX_CLIMB ) )
	{
	    bool found;

	    found = FALSE;
	    if ( ch->mount && IS_AFFECTED( ch->mount, AFF_FLYING ) )
		found = TRUE;
	    else
	    if ( ch->level > LEVEL_AVATAR || IS_AFFECTED(ch, AFF_FLYING) )
		found = TRUE;

	    if ( !found && !ch->mount )
	    {
		if ( ch->level < LEVEL_AVATAR && (( !IS_NPC(ch) && number_percent( ) > LEARNED(ch, gsn_climb) )
		||      drunk || ch->mental_state < -90 ))
		{
		   send_to_char( "Empiezas a trepar... pero pierdes el equilibrio y caes!\n\r", ch);
		   learn_from_failure( ch, gsn_climb );
		   if ( pexit->vdir == DIR_DOWN )
		   {
			retcode = move_char( ch, pexit, 1 );
			return retcode;
		   }
		   set_char_color( AT_HURT, ch );
		   send_to_char( "OUCH! Caes al suelo!\n\r", ch );
		   WAIT_STATE( ch, 20 );
		   retcode = damage( ch, ch, (pexit->vdir == DIR_UP ? 10 : 5),
					TYPE_UNDEFINED );
		   return retcode;
		}
		found = TRUE;
		learn_from_success( ch, gsn_climb );
      WAIT_STATE( ch, skill_table[gsn_climb]->beats * ( ch->generacion / 13 ) );
		txt = "climbs";
	    }

	    if ( !found )
	    {
		send_to_char( "No puedes trepar.\n\r", ch );
		return rNONE;
	    }
	}

	if ( ch->mount )
	{
	  switch (ch->mount->position)
	  {
	    case POS_DEAD:
            send_to_char( "Tu montura esta muerta!\n\r", ch );
	    return rNONE;
            break;

            case POS_MORTAL:
            case POS_INCAP:
            send_to_char( "Tu montura esta demasiado herida para moverla.\n\r", ch );
	    return rNONE;
            break;

            case POS_STUNNED:
            send_to_char( "Tu montura esta inmovilizada.\n\r", ch );
     	    return rNONE;
            break;

            case POS_SLEEPING:
            send_to_char( "Tu montura esta durmiendo.\n\r", ch );
	    return rNONE;
            break;

            case POS_RESTING:
            send_to_char( "Tu montura esta descansando.\n\r", ch);
	    return rNONE;
            break;

            case POS_SITTING:
            send_to_char( "Tu montura esta sentada.\n\r", ch);
	    return rNONE;
            break;

	    default:
	    break;
  	  }

	  if ( !IS_FLOATING(ch->mount) )
	    move = movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)];
	  else
	    move = 1;
	  if ( ch->mount->move < move )
	  {
	    send_to_char( "Tu montura esta demasiado cansada.\n\r", ch );
	    return rNONE;
	  }
	}
	else
	{
	  if ( !IS_FLOATING(ch) )
	    move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, in_room->sector_type)] );
	  else
	    move = 1;
	  if ( ch->level > LEVEL_AVATAR )
	    move = 0;
	  if ( ch->move < move )
	  {
	    send_to_char( "Estas demasiado cansado.\n\r", ch );
	    return rNONE;
	  }
	}
/*	WAIT_STATE( ch, move ); */ /* Quito el delay al moverse */
	if ( ch->mount )
	  ch->mount->move -= move;
	else
	  ch->move -= move;
    }

    /*
     * Check if player can fit in the room
     */
    if ( to_room->tunnel > 0 )
    {
	CHAR_DATA *ctmp;
	int count = ch->mount ? 1 : 0;

	for ( ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room )
	  if ( ++count >= to_room->tunnel )
	  {
		if ( ch->mount && count == to_room->tunnel )
		  send_to_char( "No hay suficiente habitacion para ti y tu montura.\n\r", ch );
		else
		  send_to_char( "No hay suficiente habitacion para ti alli.\n\r", ch );
		return rNONE;
	  }
    }

    /* check for traps on exit - later */

    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( !xIS_SET(ch->afectado_por, DAF_SILENCIO_MORTAL)
    && ( IS_NPC(ch) || !xIS_SET(ch->act, PLR_WIZINVIS) ) ) )
    {
      if ( fall )
        txt = "cae hacia";
      else
      if ( !txt )
      {
        if ( ch->mount )
        {
	  if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
	    txt = "flota";
 	  else
	  if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
	    txt = "vuela";
	  else
	    txt = "cabalga";
        }
        else
        {
	  if ( IS_AFFECTED( ch, AFF_FLOATING ) )
	  {
	    if ( drunk )
	      txt = "flota en circulo";
	     else
	      txt = "flota";
	  }
	  else
	  if ( IS_AFFECTED( ch, AFF_FLYING ) )
	  {
	    if ( drunk )
	      txt = "vuela dando tumbos";
	     else
	      txt = "vuela hacia";
	  }
	  else
          if ( ch->position == POS_SHOVE )
            txt = "es empujado";
 	  else
	  if ( ch->position == POS_DRAG )
            txt = "es arrastrado";
  	  else
	  {
	    if ( drunk )
	      txt = "se tambalea borracho";
	     else
	      txt = "se va hacia";
	  }
        }
      }
      if ( ch->mount )
      {
	sprintf( buf, "$n %s %s sobre $N.", txt, rev_exit(door) );
	act( AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT );
      }
      else
      {
	sprintf( buf, "$n %s $T.", txt );
	act( AT_ACTION, buf, ch, NULL, rev_exit(door), TO_ROOM );
      }
    }

    rprog_leave_trigger( ch );
    if( char_died(ch) )
      return global_retcode;

    char_from_room( ch );
    if ( ch->mount )
    {
      rprog_leave_trigger( ch->mount );
      if( char_died(ch) )
        return global_retcode;
      if( ch->mount )
      {
        char_from_room( ch->mount );
        char_to_room( ch->mount, to_room );
      }
    }


    char_to_room( ch, to_room );
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
    && ( !xIS_SET(ch->afectado_por, DAF_SILENCIO_MORTAL))
    && ( IS_NPC(ch) || !xIS_SET(ch->act, PLR_WIZINVIS) ) )
    {
      if ( fall )
        txt = "cae";
      else
      if ( ch->mount )
      {
	if ( IS_AFFECTED( ch->mount, AFF_FLOATING ) )
	  txt = "flota hacia";
	else
	if ( IS_AFFECTED( ch->mount, AFF_FLYING ) )
	  txt = "vuela hacia";
	else
	  txt = "cabalga hacia";
      }
      else
      {
	if ( IS_AFFECTED( ch, AFF_FLOATING ) )
	{
	  if ( drunk )
	    txt = "flota dando tumbos";
	   else
	    txt = "llega flotando";
	}
	else
	if ( IS_AFFECTED( ch, AFF_FLYING ) )
	{
	  if ( drunk )
	    txt = "vuela en circulo";
	   else
	    txt = "llega volando";
	}
	else
	if ( ch->position == POS_SHOVE )
          txt = "es empujado hacia";
	else
	if ( ch->position == POS_DRAG )
	  txt = "es arrastrado hacia";
  	else
	{
	  if ( drunk )
	    txt = "se tambaleo borracho hacia";
	   else
	    txt = "ha llegado";
	}
      }
      dtxt = rev_exit(door);
      if ( ch->mount )
      {
	sprintf( buf, "$n %s con $N.", txt, dtxt );
	act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
      }
      else
      {
	sprintf( buf, "$n %s.", txt, dtxt );
	act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
      }
    }

    if ( !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  ch->in_room->area != to_room->area )
    {
	if ( ch->level < to_room->area->low_soft_range )
	{
	   set_char_color( AT_MAGIC, ch );
	   send_to_char("Te sientes incomodo en este extranyo lugar...\n\r", ch);
	}
	else
	if ( ch->level > to_room->area->hi_soft_range )
	{
	   set_char_color( AT_MAGIC, ch );
	   send_to_char("Te sientes sin ganas de visitar este lugar...\n\r", ch);
	}
    }

    /* Make sure everyone sees the room description of death traps. */
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
	if ( xIS_SET(ch->act, PLR_BRIEF) )
	    brief = TRUE;
	xREMOVE_BIT(ch->act, PLR_BRIEF);
    }

    do_look( ch, "auto" );
    if ( brief )
 	xSET_BIT(ch->act, PLR_BRIEF);

    /*
     * Put good-old EQ-munching death traps back in!		-Thoric
     */
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
       act( AT_DEAD, "$n se acerca hacia una terrible muerte!", ch, NULL, NULL, TO_ROOM );
       set_char_color( AT_DEAD, ch );
       send_to_char( "Oops... estas muerto!\n\r", ch );
       sprintf(buf, "%s cayo en una TRAMPA MORTAL en la habitacion %d!",
		     ch->name, ch->in_room->vnum );
       log_string( buf );
       to_channel( buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
       extract_char( ch, FALSE );
       return rCHAR_DIED;
    }

    /* BIG ugly looping problem here when the character is mptransed back
       to the starting room.  To avoid this, check how many chars are in
       the room at the start and stop processing followers after doing
       the right number of them.  -- Narn
    */
    if ( !fall )
    {
      CHAR_DATA *fch;
      CHAR_DATA *nextinroom;
      int chars = 0, count = 0;

      for ( fch = from_room->first_person; fch; fch = fch->next_in_room )
        chars++;

      for ( fch = from_room->first_person; fch && ( count < chars ); fch = nextinroom )
      {
	nextinroom = fch->next_in_room;
        count++;
	if ( fch != ch		/* loop room bug fix here by Thoric */
	&& fch->master == ch
	&& fch->position == POS_STANDING )
	{
	act( AT_ACTION, "Sigues a $N.", fch, NULL, ch, TO_CHAR );
	    move_char( fch, pexit, 0 );
	}
      }
    }

    if ( ch->in_room->first_content )
      retcode = check_room_for_traps( ch, TRAP_ENTER_ROOM );
    if ( retcode != rNONE )
      return retcode;

    if ( char_died(ch) )
      return retcode;

    mprog_entry_trigger( ch );
    if ( char_died(ch) )
      return retcode;

    rprog_enter_trigger( ch );
    if ( char_died(ch) )
       return retcode;

    mprog_greet_trigger( ch );
    if ( char_died(ch) )
       return retcode;

    oprog_greet_trigger( ch );
    if ( char_died(ch) )
       return retcode;

    if (!will_fall( ch, fall )
    &&   fall > 0 )
    {
	if (!IS_AFFECTED( ch, AFF_FLOATING )
	|| ( ch->mount && !IS_AFFECTED( ch->mount, AFF_FLOATING ) ) )
	{
	  set_char_color( AT_HURT, ch );
	  send_to_char( "OUCH! Caes al suelo!\n\r", ch );
	  WAIT_STATE( ch, 20 );
	  retcode = damage( ch, ch, 20 * fall, TYPE_UNDEFINED );
	}
	else
	{
	  set_char_color( AT_MAGIC, ch );
	  send_to_char( "Descientes suavemente hasta el suelo.\n\r", ch );
	}
    }
    return retcode;
}


void do_north( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTH), 0 );
    return;
}


void do_east( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_EAST), 0 );
    return;
}


void do_south( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTH), 0 );
    return;
}


void do_west( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_WEST), 0 );
    return;
}


void do_up( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_UP), 0 );
    return;
}


void do_down( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_DOWN), 0 );
    return;
}

void do_northeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHEAST), 0 );
    return;
}

void do_northwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_NORTHWEST), 0 );
    return;
}

void do_southeast( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHEAST), 0 );
    return;
}

void do_southwest( CHAR_DATA *ch, char *argument )
{
    move_char( ch, get_exit(ch->in_room, DIR_SOUTHWEST), 0 );
    return;
}



EXIT_DATA *find_door( CHAR_DATA *ch, char *arg, bool quiet )
{
    EXIT_DATA *pexit;
    int door;

    if (arg == NULL || !str_cmp(arg,""))
      return NULL;

    pexit = NULL;
	 if ( !str_cmp( arg, "n"  ) || !str_cmp( arg, "norte"	  ) ) door = 0;
    else if ( !str_cmp( arg, "e"  ) || !str_cmp( arg, "este"	  ) ) door = 1;
    else if ( !str_cmp( arg, "s"  ) || !str_cmp( arg, "sur"	  ) ) door = 2;
    else if ( !str_cmp( arg, "o"  ) || !str_cmp( arg, "oeste"	  ) ) door = 3;
    else if ( !str_cmp( arg, "ar"  ) || !str_cmp( arg, "arriba"	  ) ) door = 4;
    else if ( !str_cmp( arg, "ab"  ) || !str_cmp( arg, "abajo"	  ) ) door = 5;
    else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "noreste" ) ) door = 6;
    else if ( !str_cmp( arg, "no" ) || !str_cmp( arg, "noroeste" ) ) door = 7;
    else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "sureste" ) ) door = 8;
    else if ( !str_cmp( arg, "so" ) || !str_cmp( arg, "suroeste" ) ) door = 9;
    else
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	{
	    if ( (quiet || IS_SET(pexit->exit_info, EX_ISDOOR))
	    &&    pexit->keyword
	    &&    nifty_is_name( arg, pexit->keyword ) )
		return pexit;
	}
	if ( !quiet )
	  act( AT_PLAIN, "No ves $T aqui.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( (pexit = get_exit( ch->in_room, door )) == NULL )
    {
	if ( !quiet)
	  act( AT_PLAIN, "No ves $T aqui.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( quiet )
	return pexit;

    if ( IS_SET(pexit->exit_info, EX_SECRET) )
    {
	act( AT_PLAIN, "No ves $T aqui.", ch, NULL, arg, TO_CHAR );
	return NULL;
    }

    if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
    {
	send_to_char( "No puedes hacer eso.\n\r", ch );
	return NULL;
    }

    return pexit;
}


void set_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    SET_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	SET_BIT( pexit_rev->exit_info, flag );
}

void remove_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    REMOVE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	REMOVE_BIT( pexit_rev->exit_info, flag );
}

void toggle_bexit_flag( EXIT_DATA *pexit, int flag )
{
    EXIT_DATA *pexit_rev;

    TOGGLE_BIT(pexit->exit_info, flag);
    if ( (pexit_rev = pexit->rexit) != NULL
    &&   pexit_rev != pexit )
	TOGGLE_BIT( pexit_rev->exit_info, flag );
}

void do_open( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    int door;
    sh_int os_type;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Abrir que?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'open door' */
	EXIT_DATA *pexit_rev;

        if (  IS_SET(pexit->exit_info, EX_SECRET)
        &&    pexit->keyword && !nifty_is_name( arg, pexit->keyword ) )
            { ch_printf( ch, "No ves %s aqui.\n\r", arg ); return;   }
	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "No puedes hacer eso.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "Ya esta abierto.\n\r",      ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED)
	     &&IS_SET(pexit->exit_info, EX_BOLTED))
	    { send_to_char( "La cerradura esta bloqueada.\n\r",	   ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_BOLTED) )
	    { send_to_char( "Esta atascada.\n\r",		   ch ); return; }
	if (  IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "Esta cerrado.\n\r",            ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    act( AT_ACTION, "$n abre el $d.", ch, NULL, pexit->keyword, TO_ROOM );
	    act( AT_ACTION, "Abres el $d.", ch, NULL, pexit->keyword, TO_CHAR );
	    if ( (pexit_rev = pexit->rexit) != NULL
	    &&   pexit_rev->to_room == ch->in_room )
	    {
		CHAR_DATA *rch;

		for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		    act( AT_ACTION, "El $d se abre.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	    }
	    remove_bexit_flag( pexit, EX_CLOSED );
	    if ( (door=pexit->vdir) >= 0 && door < 10 )
		check_room_for_traps( ch, trap_door[door]);
            return;
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
      if ( os_type == OS_TAQUILLA || xIS_SET(obj->extra_flags, ITEM_TAQUILLA) )
	{
	  if (!IS_IMMORTAL(ch) && ( obj->name != ch->name ) )
	    {
              ch_printf( ch, "Esa es la taquilla de %s, eres un mangui.\n\r", arg );
              return;
            }
	  ch_printf( ch, "Inspeccionas tu %s.\n\r", capitalize( obj->short_descr ) );
	}

	/* 'open object' */
        if ( obj->item_type != ITEM_CONTAINER )
	{
          ch_printf( ch, "%s no es un contenedor.\n\r", capitalize( obj->short_descr ) );
          return;
        }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	{
          ch_printf( ch, "%s ya esta abierto.\n\r", capitalize( obj->short_descr ) );
          return;
        }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
	{
          ch_printf( ch, "%s no se puede abrir ni cerrar.\n\r", capitalize( obj->short_descr ) );
          return;
        }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	{
          ch_printf( ch, "%s esta cerrado con llave.\n\r", capitalize( obj->short_descr ) );
          return;
        }

             

	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "Abres $p.", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "$n abre $p.", ch, obj, NULL, TO_ROOM );
	check_for_trap( ch, obj, TRAP_OPEN );
	return;
    }

    ch_printf( ch, "No ves %s aqui.\n\r", arg );
    return;
}


void do_close( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    int door;
    sh_int os_type;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Cerrar que?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'close door' */
	EXIT_DATA *pexit_rev;

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "No puedes hacer eso.\n\r",      ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "Ya esta cerrado.\n\r",    ch ); return; }

	act( AT_ACTION, "$n cierra el $d.", ch, NULL, pexit->keyword, TO_ROOM );
	act( AT_ACTION, "Cierras el $d.", ch, NULL, pexit->keyword, TO_CHAR );

	/* close the other side */
	if ( ( pexit_rev = pexit->rexit ) != NULL
	&&   pexit_rev->to_room == ch->in_room )
	{
	    CHAR_DATA *rch;

	    SET_BIT( pexit_rev->exit_info, EX_CLOSED );
	    for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )
		act( AT_ACTION, "El $d se cierra.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	}
	set_bexit_flag( pexit, EX_CLOSED );
	if ( (door=pexit->vdir) >= 0 && door < 10 )
	  check_room_for_traps( ch, trap_door[door]);
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
      /* Cerrar taquillas codigo original de COLIKOTRON code team
         Programado y compilado por SiGo y SaNgUi 16/09/2001 */
         if ( os_type == OS_TAQUILLA || xIS_SET( obj->extra_flags, ITEM_TAQUILLA) )
	{
	  if ( obj->name != ch->name )
	    {
              ch_printf( ch, "Esa es la taquilla de %s, que la cierre el.\n\r", arg );
              return;
            }
	  ch_printf( ch, "Ya no te interesa lo que haya en tu %s.\n\r", capitalize( obj->short_descr ) );
          write_taquillas( ch, obj->name, obj ); 
	}

        /* 'close object' */
	if ( obj->item_type != ITEM_CONTAINER )
	{
          ch_printf( ch, "%s no es un contenedor.\n\r", capitalize( obj->short_descr ) );
          return;
        }
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{
          ch_printf( ch, "%s ya esta cerrado.\n\r", capitalize( obj->short_descr ) );
          return;
        }
	if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
        {
          ch_printf( ch, "%s no se puede abrir ni cerrar.\n\r", capitalize( obj->short_descr ) );
          return;
        }

	SET_BIT(obj->value[1], CONT_CLOSED);
	act( AT_ACTION, "Cierras $p.", ch, obj, NULL, TO_CHAR );
	act( AT_ACTION, "$n cierra $p.", ch, obj, NULL, TO_ROOM );
	check_for_trap( ch, obj, TRAP_CLOSE );
	return;
    }

    ch_printf( ch, "No ves %s aqui.\n\r", arg );
    return;
}


/*
 * Keyring support added by Thoric
 * Idea suggested by Onyx <MtRicmer@worldnet.att.net> of Eldarion
 *
 * New: returns pointer to key/NULL instead of TRUE/FALSE
 *
 * If you want a feature like having immortals always have a key... you'll
 * need to code in a generic key, and make sure extract_obj doesn't extract it
 */
OBJ_DATA *has_key( CHAR_DATA *ch, int key )
{
    OBJ_DATA *obj, *obj2;

    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
    {
	if ( obj->pIndexData->vnum == key || (obj->item_type == ITEM_KEY && obj->value[0] == key) )
	    return obj;
	else
	if ( obj->item_type == ITEM_KEYRING )
	    for ( obj2 = obj->first_content; obj2; obj2 = obj2->next_content )
		if ( obj2->pIndexData->vnum == key || obj2->value[0] == key )
		    return obj2;
    }

    return NULL;
}


void do_lock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *key;
    EXIT_DATA *pexit;
    int count;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Bloquear quet?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'lock door' */

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "No puedes hacer eso.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "No esta cerrado.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "No puede ser bloqueado.\n\r",     ch ); return; }
	if ( (key=has_key(ch, pexit->key)) == NULL )
	    { send_to_char( "Te falta la llave.\n\r",       ch ); return; }
	if ( IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "Ya esta bloqueado.\n\r",    ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    send_to_char( "*Click*\n\r", ch );
	    count = key->count;
	    key->count = 1;
	    act( AT_ACTION, "$n bloquea el $d con $p.", ch, key, pexit->keyword, TO_ROOM );
	    key->count = count;
	    set_bexit_flag( pexit, EX_LOCKED );
            return;
        }
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'lock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "Eso no es un contenedor.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "No esta cerrado.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "No puede ser bloqueado.\n\r",     ch ); return; }
	if ( (key=has_key(ch, obj->value[2])) == NULL )
	    { send_to_char( "Te falta la llave.\n\r",       ch ); return; }
	if ( IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "Ya esta bloqueado.\n\r",    ch ); return; }

	SET_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	count = key->count;
	key->count = 1;
	act( AT_ACTION, "$n bloquea $p con $P.", ch, obj, key, TO_ROOM );
	key->count = count;
	return;
    }

    ch_printf( ch, "No ves %s aqui.\n\r", arg );
    return;
}



void do_unlock( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *key;
    EXIT_DATA *pexit;
    int count;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Desbloquear que?\n\r", ch );
	return;
    }

    if ( ( pexit = find_door( ch, arg, TRUE ) ) != NULL )
    {
	/* 'unlock door' */

	if ( !IS_SET(pexit->exit_info, EX_ISDOOR) )
	    { send_to_char( "No puedes hacer eso.\n\r",      ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
	    { send_to_char( "No esta cerrado.\n\r",        ch ); return; }
	if ( pexit->key < 0 )
	    { send_to_char( "No puede ser desbloqueado.\n\r",   ch ); return; }
	if ( (key=has_key(ch, pexit->key)) == NULL )
	    { send_to_char( "Te falta la llave.\n\r",       ch ); return; }
	if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
	    { send_to_char( "Ya esta desbloqueado.\n\r",  ch ); return; }

	if ( !IS_SET(pexit->exit_info, EX_SECRET)
	||   (pexit->keyword && nifty_is_name( arg, pexit->keyword )) )
	{
	    send_to_char( "*Click*\n\r", ch );
	    count = key->count;
	    key->count = 1;
	    act( AT_ACTION, "$n desbloquea el $d con $p.", ch, key, pexit->keyword, TO_ROOM );
	    key->count = count;
	    if ( IS_SET(pexit->exit_info, EX_EATKEY) )
	    {
		separate_obj(key);
		extract_obj(key);
	    }
	    remove_bexit_flag( pexit, EX_LOCKED );
            return;
	}
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
    {
	/* 'unlock object' */
	if ( obj->item_type != ITEM_CONTAINER )
	    { send_to_char( "Eso no es un contenedor.\n\r", ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_CLOSED) )
	    { send_to_char( "No esta cerrado.\n\r",        ch ); return; }
	if ( obj->value[2] < 0 )
	    { send_to_char( "No puede ser desbloqueado.\n\r",   ch ); return; }
	if ( (key=has_key(ch, obj->value[2])) == NULL )
	    { send_to_char( "Te falta la llave.\n\r",       ch ); return; }
	if ( !IS_SET(obj->value[1], CONT_LOCKED) )
	    { send_to_char( "Ya esta desbloqueado.\n\r",  ch ); return; }

	REMOVE_BIT(obj->value[1], CONT_LOCKED);
	send_to_char( "*Click*\n\r", ch );
	count = key->count;
	key->count = 1;
	act( AT_ACTION, "$n desbloquea $p con $P.", ch, obj, key, TO_ROOM );
	key->count = count;
	if ( IS_SET(obj->value[1], CONT_EATKEY) )
	{
	    separate_obj(key);
	    extract_obj(key);
	}
	return;
    }

    ch_printf( ch, "No ves %s aqui.\n\r", arg );
    return;
}

void do_bashdoor( CHAR_DATA *ch, char *argument )
{
	CHAR_DATA *gch;
	EXIT_DATA *pexit;
	char       arg [ MAX_INPUT_LENGTH ];

	if ( !IS_NPC( ch )
	&& ch->level < skill_table[gsn_bashdoor]->skill_level[ch->class] )
	{
	    send_to_char( "No estas preparado para derrumbar puertas!\n\r", ch );
	    return;
	}

	one_argument( argument, arg );

	if ( arg[0] == '\0' )
	{
	    send_to_char( "Derrumbar que?\n\r", ch );
	    return;
	}

	if ( ch->fighting )
	{
	    send_to_char( "No puedes parar de luchar.\n\r", ch );
	    return;
	}

	if ( ( pexit = find_door( ch, arg, FALSE ) ) != NULL )
	{
	    ROOM_INDEX_DATA *to_room;
	    EXIT_DATA       *pexit_rev;
	    int              chance;
	    char	    *keyword;

	    if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )
	    {
		send_to_char( "Calmate. Ya estaba abierto.\n\r", ch );
		return;
	    }

       WAIT_STATE( ch, skill_table[gsn_bashdoor]->beats * ( ch->generacion / 13 ) );

	    if ( IS_SET( pexit->exit_info, EX_SECRET ) )
		keyword = "wall";
	    else
		keyword = pexit->keyword;
	    if ( !IS_NPC(ch) )
		chance = LEARNED(ch, gsn_bashdoor) / 2;
	    else
		chance = 90;
	    if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
		chance /= 3;

	    if ( !IS_SET( pexit->exit_info, EX_BASHPROOF )
	    &&   ch->move >= 15
	    &&   number_percent( ) < ( chance + 4 * ( get_curr_str( ch ) - 19 ) ) )
	    {
		REMOVE_BIT( pexit->exit_info, EX_CLOSED );
		if ( IS_SET( pexit->exit_info, EX_LOCKED ) )
		REMOVE_BIT( pexit->exit_info, EX_LOCKED );
		SET_BIT( pexit->exit_info, EX_BASHED );

		act(AT_SKILL, "Crash! Derrumbas el $d!", ch, NULL, keyword, TO_CHAR );
		act(AT_SKILL, "$n derrumba el $d!",          ch, NULL, keyword, TO_ROOM );
		learn_from_success(ch, gsn_bashdoor);

		if ( (to_room = pexit->to_room) != NULL
		&&   (pexit_rev = pexit->rexit) != NULL
		&&    pexit_rev->to_room	== ch->in_room )
		{
			CHAR_DATA *rch;

			REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
			if ( IS_SET( pexit_rev->exit_info, EX_LOCKED ) )
			  REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
			SET_BIT( pexit_rev->exit_info, EX_BASHED );

			for ( rch = to_room->first_person; rch; rch = rch->next_in_room )
			{
			    act(AT_SKILL, "The $d crashes open!",
				rch, NULL, pexit_rev->keyword, TO_CHAR );
			}
		}
		damage( ch, ch, ( ch->max_hit / 20 ), gsn_bashdoor );

	    }
	    else
	    {
		act(AT_SKILL, "WHAAAAM!!!  Te tiras contra el $d, pero no se mueve.",
			ch, NULL, keyword, TO_CHAR );
		act(AT_SKILL, "WHAAAAM!!!  $n se tira contra el $d, pero no se mueve.",
			ch, NULL, keyword, TO_ROOM );
		damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor );
		learn_from_failure(ch, gsn_bashdoor);
	    }
	}
	else
	{
	    act(AT_SKILL, "WHAAAAM!!!  Te tiars contra la pared, pero no cede.",
		ch, NULL, NULL, TO_CHAR );
	    act(AT_SKILL, "WHAAAAM!!!  $n se tira contra la pared, pero no cede.",
		ch, NULL, NULL, TO_ROOM );
	    damage( ch, ch, ( ch->max_hit / 20 ) + 10, gsn_bashdoor );
	    learn_from_failure(ch, gsn_bashdoor);
	}
	if ( !char_died( ch ) )
	   for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	   {
		 if ( IS_AWAKE( gch )
		 && !gch->fighting
		 && ( IS_NPC( gch ) && !IS_AFFECTED( gch, AFF_CHARM ) )
		 && ( ch->level - gch->level <= 4 )
		 && number_bits( 2 ) == 0 )
		 multi_hit( gch, ch, TYPE_UNDEFINED );

                 if ( IS_AWAKE( gch )
		 && !gch->fighting
		 && ( IS_NPC( gch ) && xIS_SET(gch->act, PLR_DOMINADO ) )
		 && ( ch->level - gch->level <= 4 )
		 && number_bits( 2 ) == 0 )
		 multi_hit( gch, ch, TYPE_UNDEFINED );
	   }

        return;
}


void do_stand( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) && ch->level < LEVEL_AVATAR )
	    { send_to_char( "No puedes despertarte!\n\r", ch ); return; }

        if( xIS_SET(ch->afectado_por, DAF_KOMA) && ch->level < LEVEL_ENGENDRO )
        {       send_to_char( "Estas en koma no puedes despertar!\n\r", ch ); return; }

	send_to_char( "Te despiertas.\n\r", ch );
	act( AT_ACTION, "$n se despierta de $s suenyo.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_RESTING:
        send_to_char( "Te levantas.\n\r", ch );
	act( AT_ACTION, "$n se levanta de $s descanso.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_STANDING;
        break;

    case POS_SITTING:
	send_to_char( "Te pones de pie.\n\r", ch );
	act( AT_ACTION, "$n se pone de pie.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_STANDING;
	break;

    case POS_STANDING:
	send_to_char( "Ya estas de pie.\n\r", ch );
	break;

    case POS_FIGHTING:
    case POS_EVASIVE:
    case POS_DEFENSIVE:
    case POS_AGGRESSIVE:
    case POS_BERSERK:
	send_to_char( "Ya estas luchando!\n\r", ch );
	break;
    }

    return;
}


void do_sit( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
            { send_to_char( "No puedes despertarte!\n\r", ch ); return; }

        if( xIS_SET(ch->afectado_por, DAF_KOMA) && ch->level < LEVEL_ENGENDRO )
        {       send_to_char( "Estas en koma no puedes despertar!\n\r", ch ); return; }

	send_to_char( "Te despiertas y te sientas.\n\r", ch );
	act( AT_ACTION, "$n se despierta y se sienta.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SITTING;
	break;

    case POS_RESTING:
        send_to_char( "Paras de descansar y te sientas.\n\r", ch );
	act( AT_ACTION, "$n para de descansar y se sienta.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_SITTING;
	break;

    case POS_STANDING:
        send_to_char( "Te sientas.\n\r", ch );
	act( AT_ACTION, "$n se sienta.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_SITTING;
        break;
    case POS_SITTING:
	send_to_char( "Ya estas sentado.\n\r", ch );
	return;

    case POS_FIGHTING:
    case POS_EVASIVE:
    case POS_DEFENSIVE:
    case POS_AGGRESSIVE:
    case POS_BERSERK:
	send_to_char( "Estas demasiado ocupado luchando!\n\r", ch );
	return;
    case POS_MOUNTED:
        send_to_char( "Ya estas sentado... en tu montura.\n\r", ch );
        return;
    }

    return;
}


void do_rest( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
	if ( IS_AFFECTED(ch, AFF_SLEEP) )
            { send_to_char( "No puedes despertarte!\n\r", ch ); return; }

        if( xIS_SET(ch->afectado_por, DAF_KOMA) && ch->level < LEVEL_ENGENDRO )
        {       send_to_char( "Estas en koma no puedes despertar!\n\r", ch ); return; }

	send_to_char( "Te pones a descansar despues de despertarte.\n\r", ch );
	act( AT_ACTION, "$n se pone a descansar al salir de $s suenyo.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
	break;

    case POS_RESTING:
	send_to_char( "Ya estas descansando.\n\r", ch );
	return;

    case POS_STANDING:
        send_to_char( "Te estiras perezosamente a descansar.\n\r", ch );
	act( AT_ACTION, "$n se estira perezosamente a descansar.", ch, NULL, NULL, TO_ROOM );
        ch->position = POS_RESTING;
        break;

    case POS_SITTING:
        send_to_char( "Te tumbas y te pones a descansar.\n\r", ch );
	act( AT_ACTION, "$n se tumba y se pone a descansar.", ch, NULL, NULL, TO_ROOM );
	ch->position = POS_RESTING;
	break;

    case POS_FIGHTING:
    case POS_EVASIVE:
    case POS_DEFENSIVE:
    case POS_AGGRESSIVE:
    case POS_BERSERK:
	send_to_char( "Estas demasiado ocupado luchando!\n\r", ch );
	return;
    case POS_MOUNTED:
        send_to_char( "Mejor desmontate primero.\n\r", ch );
        return;
    }

    rprog_rest_trigger( ch );
    return;
}


void do_sleep( CHAR_DATA *ch, char *argument )
{
    switch ( ch->position )
    {
    case POS_SLEEPING:
		send_to_char( "Ya estas durmiendo.\n\r", ch );
		return;

    case POS_RESTING:
	        send_to_char( "Te acomodas en el suelo y te pones a dormir.\n\r", ch );
		act( AT_ACTION, "$n cierra $s ojos y se pone a dormir.", ch, NULL, NULL, TO_ROOM );
	        ch->position = POS_SLEEPING;
	        break;


    case POS_SITTING:
	        send_to_char( "Te tumbas y te pones a dormir.\n\r", ch );
		act( AT_ACTION, "$n se tumba y se pone a dormir.", ch, NULL, NULL, TO_ROOM );
	        ch->position = POS_SLEEPING;
	        break;

    case POS_STANDING:
	        send_to_char( "Te pones a dormir.\n\r", ch );
		act( AT_ACTION, "$n se pone a dormir.", ch, NULL, NULL, TO_ROOM );
	        ch->position = POS_SLEEPING;
	        break;

    case POS_FIGHTING:
    case POS_EVASIVE:
    case POS_DEFENSIVE:
    case POS_AGGRESSIVE:
    case POS_BERSERK:
		send_to_char( "Estas demasiado ocupado luchando!\n\r", ch );
		return;
    case POS_MOUNTED:
	        send_to_char( "Deberias desmontar primero.\n\r", ch );
    }

    rprog_sleep_trigger( ch );
    return;
}


void do_wake( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
	{ do_stand( ch, argument ); return; }

    if ( !IS_AWAKE(ch) )
	{ send_to_char( "Despiertate primero!\n\r", ch ); return; }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
	{ send_to_char( "No esta aqui.\n\r", ch ); return; }

    if ( IS_AWAKE(victim) )
    { act( AT_PLAIN, "$N ya esta despierto.", ch, NULL, victim, TO_CHAR ); return; }

    if ( IS_AFFECTED(victim, AFF_SLEEP) || victim->position < POS_SLEEPING )
    { act( AT_PLAIN, "No parece que puedas despertar a $M!",  ch, NULL, victim, TO_CHAR );  return; }

    if( xIS_SET(victim->afectado_por, DAF_KOMA) && ch->level < LEVEL_ENGENDRO )
        {       send_to_char( "Esta en koma no puedes despertarle!\n\r", ch ); return; }

    act( AT_ACTION, "Despiertas $M.", ch, NULL, victim, TO_CHAR );
    victim->position = POS_STANDING;
    act( AT_ACTION, "$n te despierta.", ch, NULL, victim, TO_VICT );
    return;
}


/*
 * teleport a character to another room
 */
void teleportch( CHAR_DATA *ch, ROOM_INDEX_DATA *room, bool show )
{
    char buf[MAX_STRING_LENGTH];

    if ( room_is_private( room ) )
      return;
    act( AT_ACTION, "$n desaparece en la nada!", ch, NULL, NULL, TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, room );
    act( AT_ACTION, "$n aparece de la nada!", ch, NULL, NULL, TO_ROOM );
    if ( show )
      do_look( ch, "auto" );
    if ( IS_SET( ch->in_room->room_flags, ROOM_DEATH ) && !IS_IMMORTAL( ch ) )
    {
       act( AT_DEAD, "$n cae hacia una terrible muerte!", ch, NULL, NULL, TO_ROOM );
       set_char_color( AT_DEAD, ch );
       send_to_char( "Oops... estas muerto!\n\r", ch );
       sprintf(buf, "%s cae en una TRAMPA MORTAL en la room %d!",
		     ch->name, ch->in_room->vnum );
       log_string( buf );
       to_channel( buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
       extract_char( ch, FALSE );
    }
}

void teleport( CHAR_DATA *ch, sh_int room, int flags )
{
    CHAR_DATA *nch, *nch_next;
    ROOM_INDEX_DATA *start = ch->in_room, *dest;
    bool show;

    dest = get_room_index( room );
    if ( !dest )
    {
	bug( "teleport: bad room vnum %d", room );
	return;
    }

    if ( IS_SET( flags, TELE_SHOWDESC ) )
	show = TRUE;
    else
	show = FALSE;
    if ( !IS_SET( flags, TELE_TRANSALL ) )
    {
	teleportch( ch, dest, show );
	return;
    }

    /* teleport everybody in the room */
    for ( nch = start->first_person; nch; nch = nch_next )
    {
	nch_next = nch->next_in_room;
	teleportch( nch, dest, show );
    }

    /* teleport the objects on the ground too */
    if ( IS_SET( flags, TELE_TRANSALLPLUS ) )
    {
	OBJ_DATA *obj, *obj_next;

	for ( obj = start->first_content; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    obj_from_room(obj);
	    obj_to_room(obj, dest);
	}
    }
}

/*
 * "Climb" in a certain direction.				-Thoric
 */
void do_climb( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xCLIMB ) )
	    {
		move_char( ch, pexit, 0 );
		return;
	    }
	send_to_char( "No puedes trepar aqui.\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xCLIMB ))
    {
	move_char( ch, pexit, 0 );
	return;
    }
    send_to_char( "No puedes trepar aqui.\n\r", ch );
    return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 */
void do_enter( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xENTER ) )
	    {
		move_char( ch, pexit, 0 );
		return;
	    }
	if ( ch->in_room->sector_type != SECT_INSIDE && IS_OUTSIDE(ch) )
	    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
		if ( pexit->to_room && (pexit->to_room->sector_type == SECT_INSIDE
		||  IS_SET(pexit->to_room->room_flags, ROOM_INDOORS)) )
		{
		    move_char( ch, pexit, 0 );
		    return;
		}
	send_to_char( "No puedes encontrar una entrada aqui.\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xENTER ))
    {
	move_char( ch, pexit, 0 );
	return;
    }
    send_to_char( "No puedes entrar por aqui.\n\r", ch );
    return;
}

/*
 * Leave through an exit.					-Thoric
 */
void do_leave( CHAR_DATA *ch, char *argument )
{
    EXIT_DATA *pexit;
    bool found;

    found = FALSE;
    if ( argument[0] == '\0' )
    {
	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	    if ( IS_SET( pexit->exit_info, EX_xLEAVE ) )
	    {
		move_char( ch, pexit, 0 );
		return;
	    }
	if ( ch->in_room->sector_type == SECT_INSIDE || !IS_OUTSIDE(ch) )
	    for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
		if ( pexit->to_room && pexit->to_room->sector_type != SECT_INSIDE
		&&  !IS_SET(pexit->to_room->room_flags, ROOM_INDOORS) )
		{
		    move_char( ch, pexit, 0 );
		    return;
		}
	send_to_char( "No puedes encontrar ninguna salida.\n\r", ch );
	return;
    }

    if ( (pexit = find_door( ch, argument, TRUE )) != NULL
    &&   IS_SET( pexit->exit_info, EX_xLEAVE ))
    {
	move_char( ch, pexit, 0 );
	return;
    }
    send_to_char( "No puedes ir por alli.\n\r", ch );
    return;
}

/*
 * Check to see if an exit in the room is pulling (or pushing) players around.
 * Some types may cause damage.					-Thoric
 *
 * People kept requesting currents (like SillyMUD has), so I went all out
 * and added the ability for an exit to have a "pull" or a "push" force
 * and to handle different types much beyond a simple water current.
 *
 * This check is called by violence_update().  I'm not sure if this is the
 * best way to do it, or if it should be handled by a special queue.
 *
 * Future additions to this code may include equipment being blown away in
 * the wind (mostly headwear), and people being hit by flying objects
 *
 * TODO:
 *	handle more pulltypes
 *	give "entrance" messages for players and objects
 *	proper handling of player resistance to push/pulling
 */
ch_ret pullcheck( CHAR_DATA *ch, int pulse )
{
    ROOM_INDEX_DATA *room;
    EXIT_DATA *xtmp, *xit = NULL;
    OBJ_DATA *obj, *obj_next;
    bool move = FALSE, moveobj = TRUE, showroom = TRUE;
    int pullfact, pull;
    int resistance;
    char *tochar = NULL, *toroom = NULL, *objmsg = NULL;
    char *destrm = NULL, *destob = NULL, *dtxt = "somewhere";

    if ( (room=ch->in_room) == NULL )
    {
	bug( "pullcheck: %s not in a room?!?", ch->name );
	return rNONE;
    }

    /* Find the exit with the strongest force (if any) */
    for ( xtmp = room->first_exit; xtmp; xtmp = xtmp->next )
	if ( xtmp->pull && xtmp->to_room && (!xit || abs(xtmp->pull) > abs(xit->pull)) )
	    xit = xtmp;

    if ( !xit )
	return rNONE;

    pull = xit->pull;

    /* strength also determines frequency */
    pullfact = URANGE(1, 20-(abs(pull)/5), 20);

    /* strongest pull not ready yet... check for one that is */
    if ( (pulse % pullfact) != 0 )
    {
	for ( xit = room->first_exit; xit; xit = xit->next )
	    if ( xit->pull && xit->to_room )
	    {
		pull = xit->pull;
		pullfact = URANGE(1, 20-(abs(pull)/5), 20);
		if ( (pulse % pullfact) != 0 )
		    break;
	    }

	if ( !xit )
	    return rNONE;
    }

    /* negative pull = push... get the reverse exit if any */
    if ( pull < 0 )
	if ( (xit=get_exit(room, rev_dir[xit->vdir])) == NULL )
	    return rNONE;

    dtxt = rev_exit(xit->vdir);

    /*
     * First determine if the player should be moved or not
     * Check various flags, spells, the players position and strength vs.
     * the pull, etc... any kind of checks you like.
     */
    switch(xit->pulltype)
    {
	case PULL_CURRENT:
	case PULL_WHIRLPOOL:
	    switch(room->sector_type)
	    {
		/* allow whirlpool to be in any sector type */
		default:
		    if ( xit->pulltype == PULL_CURRENT )
			break;
		case SECT_WATER_SWIM:
		case SECT_WATER_NOSWIM:
		    if ( (ch->mount && !IS_FLOATING(ch->mount))
		    ||  (!ch->mount && !IS_FLOATING(ch)) )
			move = TRUE;
		    break;

		case SECT_UNDERWATER:
		case SECT_OCEANFLOOR:
		    move = TRUE;
		    break;
	    }
	    break;
	case PULL_GEYSER:
	case PULL_WAVE:
	    move = TRUE;
	    break;

	case PULL_WIND:
	case PULL_STORM:
	    /* if not flying... check weight, position & strength */
	    move = TRUE;
	    break;

	case PULL_COLDWIND:
	    /* if not flying... check weight, position & strength */
	    /* also check for damage due to bitter cold */
	    move = TRUE;
	    break;

	case PULL_HOTAIR:
	    /* if not flying... check weight, position & strength */
	    /* also check for damage due to heat */
	    move = TRUE;
	    break;

	/* light breeze -- very limited moving power */
	case PULL_BREEZE:
	    move = FALSE;
	    break;

	/*
	 * exits with these pulltypes should also be blocked from movement
	 * ie: a secret locked pickproof door with the name "_sinkhole_", etc
	 */
	case PULL_EARTHQUAKE:
	case PULL_SINKHOLE:
	case PULL_QUICKSAND:
	case PULL_LANDSLIDE:
	case PULL_SLIP:
	case PULL_LAVA:
	    if ( (ch->mount && !IS_FLOATING(ch->mount))
	    ||  (!ch->mount && !IS_FLOATING(ch)) )
		move = TRUE;
	    break;

	/* as if player moved in that direction him/herself */
	case PULL_UNDEFINED:
	    return move_char(ch, xit, 0);

	/* all other cases ALWAYS move */
	default:
	    move = TRUE;
	    break;
    }

    /* assign some nice text messages */
    switch(xit->pulltype)
    {
	case PULL_MYSTERIOUS:
	    /* no messages to anyone */
	    showroom = FALSE;
	    break;
	case PULL_WHIRLPOOL:
	case PULL_VACUUM:
	    tochar = "You are sucked $T!";
	    toroom = "$n is sucked $T!";
	    destrm = "$n is sucked in from $T!";
	    objmsg = "$p is sucked $T.";
	    destob = "$p is sucked in from $T!";
	    break;
	case PULL_CURRENT:
	case PULL_LAVA:
	    tochar = "You drift $T.";
	    toroom = "$n drifts $T.";
	    destrm = "$n drifts in from $T.";
	    objmsg = "$p drifts $T.";
	    destob = "$p drifts in from $T.";
	    break;
	case PULL_BREEZE:
	    tochar = "You drift $T.";
	    toroom = "$n drifts $T.";
	    destrm = "$n drifts in from $T.";
	    objmsg = "$p drifts $T in the breeze.";
	    destob = "$p drifts in from $T.";
	    break;
	case PULL_GEYSER:
	case PULL_WAVE:
	    tochar = "You are pushed $T!";
	    toroom = "$n is pushed $T!";
	    destrm = "$n is pushed in from $T!";
	    destob = "$p floats in from $T.";
	    break;
	case PULL_EARTHQUAKE:
	    tochar = "The earth opens up and you fall $T!";
	    toroom = "The earth opens up and $n falls $T!";
	    destrm = "$n falls from $T!";
	    objmsg = "$p falls $T.";
	    destob = "$p falls from $T.";
	    break;
	case PULL_SINKHOLE:
	    tochar = "The ground suddenly gives way and you fall $T!";
	    toroom = "The ground suddenly gives way beneath $n!";
	    destrm = "$n falls from $T!";
	    objmsg = "$p falls $T.";
	    destob = "$p falls from $T.";
	    break;
	case PULL_QUICKSAND:
	    tochar = "You begin to sink $T into the quicksand!";
	    toroom = "$n begins to sink $T into the quicksand!";
	    destrm = "$n sinks in from $T.";
	    objmsg = "$p begins to sink $T into the quicksand.";
	    destob = "$p sinks in from $T.";
	    break;
	case PULL_LANDSLIDE:
	    tochar = "The ground starts to slide $T, taking you with it!";
	    toroom = "The ground starts to slide $T, taking $n with it!";
	    destrm = "$n slides in from $T.";
	    objmsg = "$p slides $T.";
	    destob = "$p slides in from $T.";
	    break;
	case PULL_SLIP:
	    tochar = "You lose your footing!";
	    toroom = "$n loses $s footing!";
	    destrm = "$n slides in from $T.";
	    objmsg = "$p slides $T.";
	    destob = "$p slides in from $T.";
	    break;
	case PULL_VORTEX:
	    tochar = "You are sucked into a swirling vortex of colors!";
	    toroom = "$n is sucked into a swirling vortex of colors!";
	    toroom = "$n appears from a swirling vortex of colors!";
	    objmsg = "$p is sucked into a swirling vortex of colors!";
	    objmsg = "$p appears from a swirling vortex of colors!";
	    break;
	case PULL_HOTAIR:
	    tochar = "A blast of hot air blows you $T!";
	    toroom = "$n is blown $T by a blast of hot air!";
	    destrm = "$n is blown in from $T by a blast of hot air!";
	    objmsg = "$p is blown $T.";
	    destob = "$p is blown in from $T.";
	    break;
	case PULL_COLDWIND:
	    tochar = "A bitter cold wind forces you $T!";
	    toroom = "$n is forced $T by a bitter cold wind!";
	    destrm = "$n is forced in from $T by a bitter cold wind!";
	    objmsg = "$p is blown $T.";
	    destob = "$p is blown in from $T.";
	    break;
	case PULL_WIND:
	    tochar = "A strong wind pushes you $T!";
	    toroom = "$n is blown $T by a strong wind!";
	    destrm = "$n is blown in from $T by a strong wind!";
	    objmsg = "$p is blown $T.";
	    destob = "$p is blown in from $T.";
	    break;
	case PULL_STORM:
	    tochar = "The raging storm drives you $T!";
	    toroom = "$n is driven $T by the raging storm!";
	    destrm = "$n is driven in from $T by a raging storm!";
	    objmsg = "$p is blown $T.";
	    destob = "$p is blown in from $T.";
	    break;
	default:
	    if ( pull > 0 )
	    {
		tochar = "You are pulled $T!";
		toroom = "$n is pulled $T.";
		destrm = "$n is pulled in from $T.";
		objmsg = "$p is pulled $T.";
		objmsg = "$p is pulled in from $T.";
	    }
	    else
	    {
		tochar = "You are pushed $T!";
		toroom = "$n is pushed $T.";
		destrm = "$n is pushed in from $T.";
		objmsg = "$p is pushed $T.";
		objmsg = "$p is pushed in from $T.";
	    }
	    break;
    }


    /* Do the moving */
    if ( move )
    {
	/* display an appropriate exit message */
	if ( tochar )
	{
	    act(AT_PLAIN, tochar, ch, NULL, dir_name[xit->vdir], TO_CHAR );
	    send_to_char("\n\r", ch);
	}
	if ( toroom )
	    act(AT_PLAIN, toroom, ch, NULL, dir_name[xit->vdir], TO_ROOM );

	/* display an appropriate entrance message */
	if ( destrm && xit->to_room->first_person )
	{
	    act(AT_PLAIN, destrm, xit->to_room->first_person, NULL, dtxt, TO_CHAR );
	    act(AT_PLAIN, destrm, xit->to_room->first_person, NULL, dtxt, TO_ROOM );
	}


	/* move the char */
	if ( xit->pulltype == PULL_SLIP )
	    return move_char(ch, xit, 1);
	char_from_room(ch);
	char_to_room(ch, xit->to_room);

	if ( showroom )
	    do_look(ch, "auto");

	/* move the mount too */
	if ( ch->mount )
	{
	    char_from_room(ch->mount);
	    char_to_room(ch->mount, xit->to_room);
	    if ( showroom )
		do_look(ch->mount, "auto");
	}
    }

    /* move objects in the room */
    if ( moveobj )
    {
	for ( obj = room->first_content; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( IS_OBJ_STAT(obj, ITEM_BURIED) || !CAN_WEAR(obj, ITEM_TAKE) )
		continue;

	    resistance = get_obj_weight(obj);
	    if ( IS_OBJ_STAT(obj, ITEM_METAL) )
		resistance = (resistance * 6) / 5;
	    switch(obj->item_type)
	    {
		case ITEM_SCROLL:
		case ITEM_NOTE:
		case ITEM_TRASH:
		    resistance >>= 2;	break;
		case ITEM_SCRAPS:
		case ITEM_CONTAINER:
		    resistance >>= 1;	break;
		case ITEM_PEN:
		case ITEM_WAND:
		    resistance = (resistance * 5) / 6;
		    break;

		case ITEM_CORPSE_PC:
		case ITEM_CORPSE_NPC:
		case ITEM_FOUNTAIN:
		    resistance <<= 2;	break;
	    }

	    /* is the pull greater than the resistance of the object? */
	    if ( (abs(pull) * 10) > resistance )
	    {
		if ( objmsg && room->first_person )
		{
		    act(AT_PLAIN, objmsg, room->first_person, obj, dir_name[xit->vdir], TO_CHAR);
		    act(AT_PLAIN, objmsg, room->first_person, obj, dir_name[xit->vdir], TO_ROOM);
		}
		if ( destob && xit->to_room->first_person )
		{
		    act(AT_PLAIN, destob, xit->to_room->first_person, obj, dtxt, TO_CHAR);
		    act(AT_PLAIN, destob, xit->to_room->first_person, obj, dtxt, TO_ROOM);
		}
		obj_from_room(obj);
		obj_to_room(obj, xit->to_room);
	    }
	}
    }

    return rNONE;
}

/*
 * This function bolts a door. Written by Blackmane
 */

void do_bolt (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Bolt what?\n\r", ch);
      return;
    }

  if ((pexit = find_door (ch, arg, TRUE)) != NULL)
  {

      	if (!IS_SET (pexit->exit_info, EX_ISDOOR))
	{
   	  send_to_char ("No puedes hacer eso.\n\r", ch);
   	  return;
 	}
        if (!IS_SET (pexit->exit_info, EX_CLOSED))
 	{
   	  send_to_char ("It's not closed.\n\r", ch);
   	  return;
 	}
        if (!IS_SET (pexit->exit_info, EX_ISBOLT))
 	{
   	  send_to_char ("You don't see a bolt.\n\r", ch);
   	  return;
 	}
 	if (IS_SET (pexit->exit_info, EX_BOLTED))
 	{
   	  send_to_char ("It's already bolted.\n\r", ch);
   	  return;
 	}

        if (!IS_SET (pexit->exit_info, EX_SECRET)
   	|| (pexit->keyword && nifty_is_name (arg, pexit->keyword)))
 	{
   	  send_to_char ("*Clunk*\n\r", ch);
   	  act (AT_ACTION,"$n bolts the $d.", ch, NULL, pexit->keyword, TO_ROOM);
   	  set_bexit_flag (pexit, EX_BOLTED);
   	  return;
 	}
    }

    ch_printf (ch, "You see no %s here.\n\r", arg);
    return;
}



/*
 * This function unbolts a door.  Written by Blackmane
 */

void do_unbolt (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;

  one_argument (argument, arg);

  if (arg[0] == '\0')
  {
      send_to_char ("Unbolt what?\n\r", ch);
      return;
  }

  if ((pexit = find_door (ch, arg, TRUE)) != NULL)
  {

      	if (!IS_SET (pexit->exit_info, EX_ISDOOR))
  	{
   	  send_to_char ("No puedes hacer eso.\n\r", ch);
   	  return;
 	}
      	if (!IS_SET (pexit->exit_info, EX_CLOSED))
 	{
   	  send_to_char ("No esta cerrado.\n\r", ch);
   	  return;
 	}
      	if (!IS_SET (pexit->exit_info, EX_ISBOLT))
 	{
   	  send_to_char ("You don't see a bolt.\n\r", ch);
   	  return;
 	}
      	if (!IS_SET (pexit->exit_info, EX_BOLTED))
 	{
   	  send_to_char ("It's already unbolted.\n\r", ch);
   	  return;
 	}

      	if (!IS_SET (pexit->exit_info, EX_SECRET)
   	|| (pexit->keyword && nifty_is_name (arg, pexit->keyword)))
 	{
   	  send_to_char ("*Clunk*\n\r", ch);
   	  act (AT_ACTION,"$n unbolts the $d.",ch,NULL,pexit->keyword,TO_ROOM);
   	  remove_bexit_flag (pexit, EX_BOLTED);
   	  return;
 	}
    }

    ch_printf (ch, "No ves eso aqui.\n\r", arg);
    return;
}

