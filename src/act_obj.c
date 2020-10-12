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
 *			   Object manipulation module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "bet.h"


/*double sqrt( double x );*/

/*
 * External functions
 */

void    show_list_to_char  args( ( OBJ_DATA *list, CHAR_DATA *ch,
				bool fShort, bool fShowNothing ) );
void    write_corpses   args( ( CHAR_DATA *ch, char *name, OBJ_DATA *objrem ) );

/*
 * Local functions.
 */
void	get_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj,
			    OBJ_DATA *container ) );
bool	remove_obj	args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, sh_int wear_bit ) );

char* 	get_chance_verb args( ( OBJ_DATA *obj ) );
char*  	get_ed_number	args( ( OBJ_DATA *obj, int number ) );

OBJ_DATA* recursive_note_find	args( ( OBJ_DATA *obj, char* argument) );

/*
 * how resistant an object is to damage				-Thoric
 */
sh_int get_obj_resistance( OBJ_DATA *obj )
{
    sh_int resist;

    resist = number_fuzzy(MAX_ITEM_IMPACT);

    /* magical items are more resistant */
    if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      resist += number_fuzzy(12);
    /* metal objects are definately stronger */
    if ( IS_OBJ_STAT( obj, ITEM_METAL ) )
      resist += number_fuzzy(5);
    /* quest flags para objetos */
    if ( IS_OBJ_STAT( obj, ITEM_QUEST ) )
      resist += 5000000;
    /* organic objects are most likely weaker */
    if ( IS_OBJ_STAT( obj, ITEM_ORGANIC ) )
      resist -= number_fuzzy(5);
    /* blessed objects should have a little bonus */
    if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
      resist += number_fuzzy(5);
    /* lets make store inventory pretty tough */
    if ( IS_OBJ_STAT( obj, ITEM_INVENTORY ) )
      resist += 20000;

    /* okay... let's add some bonus/penalty for item level... */
    resist += (obj->level / 10) - 2;

    /* and lasty... take armor or weapon's condition into consideration */
    if (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON)
      resist += (obj->value[0] / 2) - 2;

    return URANGE(10, resist, 99);
}


void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container )
{
    CLAN_DATA *clan;
    int weight;
    int amt; /* gold per-race multipliers */
    sh_int os_type;

    if ( !CAN_WEAR(obj, ITEM_TAKE)
       && (ch->level < sysdata.level_getobjnotake )  )
    {
	send_to_char( "No puedes hacer eso.\n\r", ch );
	return;
    }

    if( !IS_IMMORTAL(ch))
            if( IS_OBJ_STAT( obj, ITEM_QUEST ) && str_cmp( obj->propietario, ch->name) )
            {
              if( obj->propietario != NULL )
                ch_printf( ch, "%s lleva la marca de %s no puedes cogerlo.\n\r", obj->short_descr, obj->propietario );
              else
                ch_printf( ch, "%s es un objeto Quest no puedes cogerlo.\n\r", obj->short_descr );
                return;
            }

    if( !IS_IMMORTAL(ch))
            if( IS_OBJ_STAT( obj, ITEM_LOYAL ) && !IS_OBJ_STAT( obj, ITEM_QUEST  ) && str_cmp( obj->propietario, ch->name) )
            {
              if( obj->propietario != NULL )
                ch_printf( ch, "No puedes coger %s es propiedad de %s.\n\r", obj->short_descr, obj->propietario );
                else
                ch_printf( ch, "%s es un objeto marcado no puedes cogerlo.\n\r", obj->short_descr );
                return;
            }

    if ( IS_SET( obj->magic_flags, ITEM_PKDISARMED )
    &&  !IS_NPC( ch ) )
    {
	if ( CAN_PKILL( ch ) && !get_timer( ch, TIMER_PKILLED ) )
	{
	    if ( ch->level - obj->value[5] > 5
	    ||   obj->value[5] - ch->level > 5 )
	    {
		send_to_char_color( "\n\r&bNo puedes cogerlo,  no es tuyo.\n\r", ch );
		return;
	    }
	    else
	    {
		REMOVE_BIT( obj->magic_flags, ITEM_PKDISARMED );
		obj->value[5] = 0;
	    }
	}
	else
	{
	    send_to_char_color( "\n\r&BNo puedes cogerlo, no es tuyo.\n\r", ch );
 	    return;
	}
    }

    if ( IS_OBJ_STAT( obj, ITEM_PROTOTYPE )
    &&  !can_take_proto( ch ) )
    {
	send_to_char( "Una fuerza divina te impide hacer eso.\n\r", ch );
	return;
    }

    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( AT_PLAIN, "$d: no puedes llevar mas objetos.",
		ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ( IS_OBJ_STAT(obj, ITEM_COVERING) )
	weight = obj->weight;
    else
	weight = get_obj_weight( obj );

    if ( ch->carry_weight + weight > can_carry_w( ch ) )
    {
	act( AT_PLAIN, "$d: no puedes llevar tanto peso",
	    ch, NULL, obj->name, TO_CHAR );
	return;
    }

    if ( container )
    {

      if( !IS_IMMORTAL(ch))
            if( IS_OBJ_STAT( obj, ITEM_QUEST ) && str_cmp( obj->propietario, ch->name) )
            {
              if( obj->propietario != NULL )
                ch_printf( ch, "%s lleva la marca de %s no puedes cogerlo.\n\r", obj->short_descr, obj->propietario );
              else
                ch_printf( ch, "%s es un objeto Quest no puedes cogerlo.\n\r", obj->short_descr );
                return;
            }

      if( !IS_IMMORTAL(ch))
            if( IS_OBJ_STAT( obj, ITEM_LOYAL ) && !IS_OBJ_STAT( obj, ITEM_QUEST  ) && str_cmp( obj->propietario, ch->name) )
            {
              if( obj->propietario != NULL )
                ch_printf( ch, "No puedes coger %s es propiedad de %s.\n\r", obj->short_descr, obj->propietario );
                else
                ch_printf( ch, "%s es un objeto marcado no puedes cogerlo.\n\r", obj->short_descr );
                return;
            }

	if ( container->item_type == ITEM_KEYRING && !IS_OBJ_STAT(container, ITEM_COVERING) )
	{
	    act( AT_ACTION, "Coges $p de $P", ch, obj, container, TO_CHAR );
	    act( AT_ACTION, "$n coge $p de $P", ch, obj, container, TO_ROOM );
	}
	else
	{
	    act( AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ?
		"Coges $p de $P." : "Coges $p de $P",
		ch, obj, container, TO_CHAR );
	    act( AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ?
		"$n coge $p de $P." : "$n coge $p de $P",
		ch, obj, container, TO_ROOM );
	}
	if ( IS_OBJ_STAT( container, ITEM_CLANCORPSE )
	&&  !IS_NPC( ch ) && str_cmp( container->name+7, ch->name ) )
                container->value[5]++;
	obj_from_obj( obj );
     }
    else
    {
	act( AT_ACTION, "Coges $p.", ch, obj, container, TO_CHAR );
	act( AT_ACTION, "$n coge $p.", ch, obj, container, TO_ROOM );
	obj_from_room( obj );
    }

    /* Clan storeroom checks */
    if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM)
    && (!container || container->carried_by == NULL) )
    {
/*	if (!char_died) save_char_obj(ch); */
	for ( clan = first_clan; clan; clan = clan->next )
	  if ( clan->storeroom == ch->in_room->vnum )
	    save_clan_storeroom(ch, clan);
    }

    if ( obj->item_type != ITEM_CONTAINER )
	check_for_trap( ch, obj, TRAP_GET );
    if ( char_died(ch) )
	return;

    if ( obj->item_type == ITEM_MONEY )
    {

        amt = obj->value[0];

/*
 *  The idea was to make some races more adroit at money handling,
 *  however, this resulted in elves dropping 1M gps and picking
 *  up 1.1M, repeating, and getting rich.  The only solution would
 *  be to fuzzify the "drop coins" code, but that seems like it'd
 *  lead to more confusion than it warrants.  -h
 *
 *  When you work on this again, make it so that amt is NEVER multiplied
 *  by more than 1.0.  Use less than 1.0 for ogre, orc, troll, etc.
 *  (Ie: a penalty rather than a bonus)
 */
#ifdef GOLD_MULT
        switch(ch->race){
          case(1):  amt *=1.1; break;/* elf */
          case(2):  amt *=0.97; break; /* dwarf */
          case(3):  amt *=1.02; break;/* halfling */
          case(4):  amt *=1.08; break;/* pixie */
          case(6):  amt *=0.92; break;/* half-ogre */
          case(7):  amt *=0.94; break;/* half-orc */
          case(8):  amt *=0.90; break;/* half-troll */
          case(9):  amt *=1.04; break;/* half-elf */
          case(10): amt *=1.06; break;/* gith */
        }
#endif

	ch->gold += amt;
	extract_obj( obj );
    }
    else
    {
	obj = obj_to_char( obj, ch );

        if( container )
    if (container->name == ch->name)
    write_taquillas(ch, container->name, NULL);
    }

    if ( char_died(ch) || obj_extracted(obj) )
      return;
    oprog_get_trigger(ch, obj);
    return;
}


void do_get( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    OBJ_DATA *container;
    sh_int number;
    sh_int os_type;
    bool found;

    argument = one_argument( argument, arg1 );
    if ( is_number(arg1) )
    {
	number = atoi(arg1);
	if ( number < 1 )
	{
	    send_to_char( "Era facil...\n\r", ch );
	    return;
	}
	if ( (ch->carry_number + number) > can_carry_n(ch) )
	{
	    send_to_char( "No puedes llevar mas.\n\r", ch );
	    return;
	}
	argument = one_argument( argument, arg1 );
    }
    else
	number = 0;
    argument = one_argument( argument, arg2 );
    /* munch optional words */
    if ( !str_cmp( arg2, "from" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    /* Get type. */
    if ( arg1[0] == '\0' )
    {
	send_to_char( "Coger que?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( arg2[0] == '\0' )
    {
	if ( number <= 1 && str_cmp( arg1, "todo" ) && str_prefix( "todo.", arg1 ) )
	{
	    /* 'get obj' */
	    obj = get_obj_list( ch, arg1, ch->in_room->first_content );
	    if ( !obj )
	    {
		act( AT_PLAIN, "No veo $T aqui.", ch, NULL, arg1, TO_CHAR );
		return;
	    }

            if( !IS_IMMORTAL(ch))
            if( IS_OBJ_STAT( obj, ITEM_QUEST ) && str_cmp( obj->propietario, ch->name) )
            {
              if( obj->propietario != NULL )
                ch_printf( ch, "%s lleva la marca de %s no puedes cogerlo.\n\r", obj->short_descr, obj->propietario );
              else
                ch_printf( ch, "%s es un objeto Quest no puedes cogerlo.\n\r", obj->short_descr );
                return;
            }

            if( !IS_IMMORTAL(ch))
            if( IS_OBJ_STAT( obj, ITEM_LOYAL ) && !IS_OBJ_STAT( obj, ITEM_QUEST  ) && str_cmp( obj->propietario, ch->name) )
            {
              if( obj->propietario != NULL )
                ch_printf( ch, "No puedes coger %s es propiedad de %s.\n\r", obj->short_descr, obj->propietario );
                else
                ch_printf( ch, "%s es un objeto marcado no puedes cogerlo.\n\r", obj->short_descr );
                return;
            }

	    separate_obj(obj);
	    get_obj( ch, obj, NULL );
	    if ( char_died(ch) )
		return;
	    if ( IS_SET( sysdata.save_flags, SV_GET ) )
		save_char_obj( ch );
	}
	else
	{
	    sh_int cnt = 0;
	    bool fAll;
	    char *chk;

/*	    if ( IS_SET( ch->in_room->room_flags, ROOM_DONATION ) )
	    {
		send_to_char( "The gods frown upon such a display of greed!\n\r", ch );
		return;
	    }
*/
	    if ( !str_cmp(arg1, "todo") )
	    	fAll = TRUE;
	    else
		fAll = FALSE;
	    if ( number > 1 )
		chk = arg1;
	    else
		chk = &arg1[4];
	    /* 'get all' or 'get all.obj' */
	    found = FALSE;
	    for ( obj = ch->in_room->last_content; obj; obj = obj_next )
	    {
		obj_next = obj->prev_content;
		if ( ( fAll || nifty_is_name( chk, obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if ( number && (cnt + obj->count) > number )
			split_obj( obj, number - cnt );
		    cnt += obj->count;
		    get_obj( ch, obj, NULL );
		    if ( char_died(ch)
		    ||   ch->carry_number >= can_carry_n( ch )
		    ||   ch->carry_weight >= can_carry_w( ch )
		    ||   (number && cnt >= number) )
		    {
			if ( IS_SET(sysdata.save_flags, SV_GET)
			&&  !char_died(ch) )
			    save_char_obj(ch);
			return;
		    }
		}
	    }

	    if ( !found )
	    {
		if ( fAll )
		  send_to_char( "No veo nada aqui.\n\r", ch );
		else
		  act( AT_PLAIN, "No veo $T aqui.", ch, NULL, chk, TO_CHAR );
	    }
	    else
	    if ( IS_SET( sysdata.save_flags, SV_GET ) )
		save_char_obj( ch );
	}
    }
    else
    {
	/* 'get ... container' */
	if ( !str_cmp( arg2, "todo" ) || !str_prefix( "todo.", arg2 ) )
	{
	    send_to_char( "No puedes hacer eso.\n\r", ch );
	    return;
	}

	if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )
	{
	    act( AT_PLAIN, "No veo $T aqui.", ch, NULL, arg2, TO_CHAR );
	    return;
	}

	switch ( container->item_type )
	{
	default:
	    if ( !IS_OBJ_STAT( container, ITEM_COVERING ) )
	    {
		send_to_char( "Eso no es un container.\n\r", ch );
		return;
	    }
	    if ( ch->carry_weight + container->weight > can_carry_w( ch ) )
	    {
		send_to_char( "Es demasiado pesado para que lo puedas llevar.\n\r", ch );
		return;
	    }
	    break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_KEYRING:
	case ITEM_QUIVER:
	    break;

	case ITEM_CORPSE_PC:
	    {
                                   char name[MAX_INPUT_LENGTH];
		CHAR_DATA *gch;
		char *pd;


		if ( IS_NPC(ch) )
		{
		    send_to_char( "No puedes hacer eso.\n\r", ch );
		    return;
		}

		pd = container->short_descr;
		pd = one_argument( pd, name );
		pd = one_argument( pd, name );
		pd = one_argument( pd, name );
		pd = one_argument( pd, name );

		if ( IS_OBJ_STAT( container, ITEM_CLANCORPSE )
		&&  !IS_NPC(ch) && (get_timer( ch, TIMER_PKILLED ) > 0 )
		&& str_cmp( name, ch->name ) )
		{
		     send_to_char( "No puedes saquear ese cuerpo... aun.\n\r", ch );
		     return;
		}

		/* Killer/owner loot only if die to pkill blow --Blod */
		/* Added check for immortal so IMMS can get things out of
		 * corpses --Shaddai */

		if ( IS_OBJ_STAT( container, ITEM_CLANCORPSE )
                &&  !IS_NPC( ch ) && !IS_IMMORTAL(ch)
		&&   container->action_desc[0] !='\0'
		&&   str_cmp( name, ch->name )
		&&   str_cmp( container->action_desc, ch->name ) )
		{
		    send_to_char( "No es tu victima.\n\r", ch );
		    return;
		}

		if ( IS_OBJ_STAT( container, ITEM_CLANCORPSE )
		&&  !IS_NPC(ch) && str_cmp( name, ch->name )
		&&   container->value[5] >= 3 )
		{
		     send_to_char( "El excesivo saqueo ha dejado este cuerpo protegido por los dioses.\n\r", ch);
		     return;
		}

		if ( IS_OBJ_STAT( container, ITEM_CLANCORPSE )
		&&  !IS_NPC(ch) && IS_SET( ch->pcdata->flags, PCFLAG_DEADLY )
		&&   container->value[4] - ch->level < 6
		&&   container->value[4] - ch->level > -6 )
		     break;

		if ( str_cmp( name, ch->name ) && !IS_IMMORTAL(ch) )
		{
		    bool fGroup;

		    fGroup = FALSE;
		    for ( gch = first_char; gch; gch = gch->next )
		    {
			if ( !IS_NPC(gch)
			&&   is_same_group( ch, gch )
			&&   !str_cmp( name, gch->name ) )
			{
			    fGroup = TRUE;
			    break;
			}
		    }

		    if ( !fGroup )
		    {
			send_to_char( "Hay otro cuerpo mas.\n\r", ch );
			return;
		    }
		}
	    }
	}

	if ( !IS_OBJ_STAT(container, ITEM_COVERING )
	&&    IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "Esta cerrado.", ch, NULL, container->name, TO_CHAR );
	    return;
	}

	if ( number <= 1 && str_cmp( arg1, "todo" ) && str_prefix( "todo.", arg1 ) )
	{
	    /* 'get obj container' */
	    obj = get_obj_list( ch, arg1, container->first_content );
	    if ( !obj )
	    {
		act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
			"No veo nada parecido a eso en $T." :
			"No veo nada parecido a eso en $T.",
			ch, NULL, arg2, TO_CHAR );
		return;
	    }
	    separate_obj(obj);
	    get_obj( ch, obj, container );
	    /* Oops no wonder corpses were duping oopsie did I do that
	     * --Shaddai
             */
	    if ( container->item_type == ITEM_CORPSE_PC )
		write_corpses( NULL, container->short_descr+14, NULL );
	    check_for_trap( ch, container, TRAP_GET );
	    if ( char_died(ch) )
		return;
	    if ( IS_SET( sysdata.save_flags, SV_GET ) )
		save_char_obj( ch );
	}
	else
	{
	    int cnt = 0;
	    bool fAll;
	    char *chk;

	    /* 'get all container' or 'get all.obj container' */
/*
	    if ( IS_OBJ_STAT( container, ITEM_DONATION ) )
	    {
		send_to_char( "The gods frown upon such an act of greed!\n\r", ch );
		return;
	    }

            if ( IS_OBJ_STAT( container, ITEM_CLANCORPSE )
            &&  !IS_IMMORTAL(ch)
            &&  !IS_NPC(ch)
            &&  str_cmp( ch->name, container->name+7 ) )
            {
                send_to_char( "The gods frown upon such wanton greed!\n\r", ch );
                return;
            }
*/
	    if ( !str_cmp(arg1, "todo") )
		fAll = TRUE;
	    else
		fAll = FALSE;
	    if ( number > 1 )
		chk = arg1;
	    else
		chk = &arg1[4];
	    found = FALSE;
	    for ( obj = container->first_content; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;
		if ( ( fAll || nifty_is_name( chk, obj->name ) )
		&&   can_see_obj( ch, obj ) )
		{
		    found = TRUE;
		    if ( number && (cnt + obj->count) > number )
			split_obj( obj, number - cnt );
		    cnt += obj->count;
		    get_obj( ch, obj, container );
		    if ( char_died(ch)
		    ||   ch->carry_number >= can_carry_n( ch )
		    ||   ch->carry_weight >= can_carry_w( ch )
		    ||   (number && cnt >= number) )
		      return;
		}
	    }

	    if ( !found )
	    {
		if ( fAll )
		{
		    if ( container->item_type == ITEM_KEYRING && !IS_OBJ_STAT(container, ITEM_COVERING) )
			act( AT_PLAIN, "$T no tiene llave.",
			    ch, NULL, arg2, TO_CHAR );
		    else
			act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
			    "No veo nada dentro de $T." :
			    "No veo nada en $T.",
			    ch, NULL, arg2, TO_CHAR );
		}
		else
		{
		    if ( container->item_type == ITEM_KEYRING && !IS_OBJ_STAT(container, ITEM_COVERING) )
			act( AT_PLAIN, "$T no se abre con esa llave (?).",
			    ch, NULL, arg2, TO_CHAR );
		    else
			act( AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
			    "No veo nada como eso dentro de $T." :
			    "No veo nada como eso en $T.",
			    ch, NULL, arg2, TO_CHAR );
		}
	    }
	    else
		check_for_trap( ch, container, TRAP_GET );
	    if ( char_died(ch) )
		return;
	    /* Oops no wonder corpses were duping oopsie did I do that
	     * --Shaddai
             */
	    if ( container->item_type == ITEM_CORPSE_PC )
		write_corpses( NULL, container->short_descr+14, NULL );
	    if ( found && IS_SET( sysdata.save_flags, SV_GET ) )
		save_char_obj( ch );
	}
    }
    return;
}



void do_put( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *container;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    CLAN_DATA *clan;
    sh_int	count;
    int		number;
    bool	save_char = FALSE;

    argument = one_argument( argument, arg1 );
    if ( is_number(arg1) )
    {
	number = atoi(arg1);
	if ( number < 1 )
	{
	    send_to_char( "Era facil...\n\r", ch );
	    return;
	}
	argument = one_argument( argument, arg1 );
    }
    else
	number = 0;
    argument = one_argument( argument, arg2 );
    /* munch optional words */
    if ( (!str_cmp(arg2, "dentro") || !str_cmp(arg2, "desde")
    ||    !str_cmp(arg2, "en") || !str_cmp(arg2, "bajo")
    ||    !str_cmp(arg2, "dentro de") || !str_cmp(arg2, "sobre"))
    &&     argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Poner que donde?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( !str_cmp(arg2, "todo") || !str_prefix("todo.", arg2) )
    {
	send_to_char( "No puedes hacer eso.\n\r", ch );
	return;
    }

    if ( (container=get_obj_here(ch, arg2)) == NULL )
    {
	act( AT_PLAIN, "No veo $T aqui.", ch, NULL, arg2, TO_CHAR );
	return;
    }

    if ( !container->carried_by && IS_SET(sysdata.save_flags, SV_PUT) )
	save_char = TRUE;

    if ( IS_OBJ_STAT(container, ITEM_COVERING) )
    {
	if ( ch->carry_weight + container->weight > can_carry_w(ch) )
	{
	    send_to_char( "Es demasiado pesado para que puedas cargarlo.\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( container->item_type != ITEM_CONTAINER
	&&   container->item_type != ITEM_KEYRING
	&&   container->item_type != ITEM_QUIVER )
	{
	    send_to_char( "Eso no es un container.\n\r", ch );
	    return;
	}

	if ( IS_SET(container->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "El $d esta cerrado.", ch, NULL, container->name, TO_CHAR );
	    return;
	}
    }

    if ( number <= 1 && str_cmp(arg1, "todo") && str_prefix("todo.", arg1) )
    {
	/* 'put obj container' */
	if ( (obj=get_obj_carry(ch, arg1)) == NULL )
	{
	    send_to_char( "No tienes ese objeto.\n\r", ch );
	    return;
	}

	if ( obj == container )
	{
	    send_to_char( "No puedes meterlo dentro de si mismo.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj(ch, obj) && (!IS_IMMORTAL(ch) ))
	{
	    send_to_char( "No puedes deshacerte de eso.\n\r", ch );
	    return;
	}

	if ( container->item_type == ITEM_KEYRING && obj->item_type != ITEM_KEY )
	{
	    send_to_char( "No es una llave.\n\r", ch );
	    return;
	}

	if ( container->item_type == ITEM_QUIVER && obj->item_type != ITEM_PROJECTILE )
	{
	    send_to_char( "No es un proyectil.\n\r", ch );
	    return;
	}

	if ( (IS_OBJ_STAT(container, ITEM_COVERING)
	&&   (get_obj_weight(obj) / obj->count)
	  > ((get_obj_weight(container) / container->count)
	  -   container->weight)) )
	{
	    send_to_char( "No cabe.\n\r", ch );
	    return;
	}

	/* note use of get_real_obj_weight */
	if ( (get_real_obj_weight(obj) / obj->count)
	   + (get_real_obj_weight(container) / container->count)
	   >  container->value[0] )
	{
	    send_to_char( "No cabe.\n\r", ch );
	    return;
	}

	separate_obj(obj);
	separate_obj(container);
	obj_from_char( obj );
	obj = obj_to_obj( obj, container );
                  if (container->name == ch->name)
                  write_taquillas(ch, container->name, NULL);
	check_for_trap ( ch, container, TRAP_PUT );
	if ( char_died(ch) )
	    return;
	count = obj->count;
	obj->count = 1;
	if ( container->item_type == ITEM_KEYRING && !IS_OBJ_STAT(container, ITEM_COVERING) )
	{
	    act( AT_ACTION, "$n pone $p en $P.", ch, obj, container, TO_ROOM );
	    act( AT_ACTION, "Pones $p en $P.", ch, obj, container, TO_CHAR );
	}
	else
	{
	    act( AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING)
		? "$n esconde $p dentro de $P." : "$n pone $p en $P.",
		ch, obj, container, TO_ROOM );
	    act( AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING)
		? "Esconder $p dentro de $P." : "Pones $p en $P.",
		ch, obj, container, TO_CHAR );
	}
	obj->count = count;

       /* Oops no wonder corpses were duping oopsie did I do that
        * --Shaddai
        */
        if ( container->item_type == ITEM_CORPSE_PC )
		write_corpses( NULL, container->short_descr+14, NULL );

	if ( save_char )
	  save_char_obj(ch);
	/* Clan storeroom check */
	if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM)
	&&   container->carried_by == NULL)
	{
/*	   if (!char_died && !save_char ) save_char_obj(ch); */
	   for ( clan = first_clan; clan; clan = clan->next )
	      if ( clan->storeroom == ch->in_room->vnum )
		save_clan_storeroom(ch, clan);
	}
    }
    else
    {
	bool found = FALSE;
	int cnt = 0;
	bool fAll;
	char *chk;

	if ( !str_cmp(arg1, "todo") )
	    fAll = TRUE;
	else
	    fAll = FALSE;
	if ( number > 1 )
	    chk = arg1;
	else
	    chk = &arg1[4];

	separate_obj(container);
	/* 'put all container' or 'put all.obj container' */
	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( ( fAll || nifty_is_name( chk, obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   obj != container
	    &&   can_drop_obj( ch, obj )
	    &&  (container->item_type != ITEM_KEYRING || obj->item_type == ITEM_KEY)
	    &&  (container->item_type != ITEM_QUIVER  || obj->item_type == ITEM_PROJECTILE)
	    &&   get_obj_weight( obj ) + get_obj_weight( container )
		 <= container->value[0] )
	    {
		if ( number && (cnt + obj->count) > number )
		    split_obj(obj, number - cnt);
		cnt += obj->count;
		obj_from_char(obj);
		if ( container->item_type == ITEM_KEYRING )
		{
		    act( AT_ACTION, "$n pone $p en $P.", ch, obj, container, TO_ROOM );
		    act( AT_ACTION, "Pones $p en $P.", ch, obj, container, TO_CHAR );
		}
		else
		{
		    act( AT_ACTION, "$n pone $p en $P.", ch, obj, container, TO_ROOM );
		    act( AT_ACTION, "Pones $p en $P.", ch, obj, container, TO_CHAR );
		}
		obj = obj_to_obj( obj, container );
		found = TRUE;

		check_for_trap( ch, container, TRAP_PUT );
		if ( char_died(ch) )
		    return;
		if ( number && cnt >= number )
		    break;
	    }
	}

	/*
	 * Don't bother to save anything if nothing was dropped   -Thoric
	 */
	if ( !found )
	{
	    if ( fAll )
	      act( AT_PLAIN, "No estas cargando nada.",
		    ch, NULL, NULL, TO_CHAR );
	    else
	      act( AT_PLAIN, "No estas cargando ningun $T.",
		    ch, NULL, chk, TO_CHAR );
	    return;
	}

	if ( save_char )
	    save_char_obj(ch);
       /* Oops no wonder corpses were duping oopsie did I do that
        * --Shaddai
        */
        if ( container->item_type == ITEM_CORPSE_PC )
		write_corpses( NULL, container->short_descr+14, NULL );

	/* Clan storeroom check */
        if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM)
	&& container->carried_by == NULL )
	{
/*	  if (!char_died && !save_char) save_char_obj(ch); */
	  for ( clan = first_clan; clan; clan = clan->next )
	     if ( clan->storeroom == ch->in_room->vnum )
        	save_clan_storeroom(ch, clan);
	}
    }

    return;
}


void do_donate( CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *obj;
   char arg[MAX_INPUT_LENGTH];
   int room_chance;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0' )
   {
     send_to_char("Donar que?\n\r",ch);
     return;
   }

   if (ch->position == POS_FIGHTING)
   {
     send_to_char("No puedes donarlo mientras luchas!\n\r",ch);
     return;
   }

   if ( (obj = get_obj_carry (ch, arg)) == NULL)
   {
     send_to_char("No tienes eso!\n\r",ch);
     return;
   }
   else
   {
     if (!can_drop_obj(ch, obj) && ch->level < LEVEL_IMMORTAL )
     {
        send_to_char("No puedes donarlo, esta maldito!\n\r",ch);
        return;
     }

     if ((obj->item_type == ITEM_CORPSE_NPC) || (obj->item_type == ITEM_CORPSE_PC))
     {
        send_to_char("No puedes donar cuerpos!\n\r",ch);
        return;
     }

     if (obj->timer > 0)
     {
        send_to_char("No puedes donar eso.\n\r",ch);
        return;
     }

     if( IS_OBJ_STAT( obj, ITEM_QUEST) || IS_OBJ_STAT( obj, ITEM_LOYAL ) )
     {
        send_to_char( "No puedes donar ese objeto.\n\r", ch );
        return;
     }

     room_chance = number_range( 1,9 ); /* Smaug random number generator */

     if ( room_chance >= 5 )
     {
     act( AT_ACTION, "Donas $p, muy generoso de tu parte!", ch, obj, NULL, TO_CHAR );
	  separate_obj(obj);
	  obj_from_char(obj);
	  obj_to_room(obj, get_room_index(ROOM_VNUM_DONATION));
	  return;
     }
     else
     {
     act( AT_ACTION, "Donas $p, muy generoso de tu parte!", ch, obj, NULL, TO_CHAR );
	  separate_obj(obj);
	  obj_from_char(obj);
	  obj_to_room(obj, get_room_index(ROOM_VNUM_DONATIONTWO));
	  return;
     }
   }
}

void do_drop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    bool found;
    CLAN_DATA *clan;
    int number;

    argument = one_argument( argument, arg );
    if ( is_number(arg) )
    {
	number = atoi(arg);
	if ( number < 1 )
	{
	    send_to_char( "Era facil...\n\r", ch );
	    return;
	}
	argument = one_argument( argument, arg );
    }
    else
	number = 0;

    if ( arg[0] == '\0' )
    {
	send_to_char( "Dejar que?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( !IS_NPC(ch) && xIS_SET( ch->act, PLR_LITTERBUG ) )
    {
       set_char_color( AT_YELLOW, ch );
       send_to_char( "Una fuerza divina te impide dejarlo...\n\r", ch );
       return;
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_NODROP )
    &&   ch != supermob )
    {
       set_char_color( AT_MAGIC, ch );
       send_to_char( "Una fuerza magica te detiene!\n\r", ch );
       set_char_color( AT_TELL, ch );
       send_to_char( "alguien te dice telepaticamente: 'No dejes basura aqui!'\n\r", ch );
       return;
    }

    if ( number > 0 )
    {
	/* 'drop NNNN coins' */

	if ( !str_cmp( arg, "monedas" ) || !str_cmp( arg, "moneda" ) )
	{
	    if ( ch->gold < number )
	    {
		send_to_char( "No tienes tanto dinero.\n\r", ch );
		return;
	    }

	    ch->gold -= number;

	    for ( obj = ch->in_room->first_content; obj; obj = obj_next )
	    {
		obj_next = obj->next_content;

		switch ( obj->pIndexData->vnum )
		{
		case OBJ_VNUM_MONEY_ONE:
		   number += 1;
		   extract_obj( obj );
		   break;

		case OBJ_VNUM_MONEY_SOME:
		   number += obj->value[0];
		   extract_obj( obj );
		   break;
		}
	    }

	    act( AT_ACTION, "$n deja algo de oro.", ch, NULL, NULL, TO_ROOM );
	    obj_to_room( create_money( number ), ch->in_room );
	    send_to_char( "Dejas el oro en el suelo.\n\r", ch );
	    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
		save_char_obj( ch );
	    return;
	}
    }

    if ( number <= 1 && str_cmp( arg, "todo" ) && str_prefix( "todo.", arg ) )
    {
	/* 'drop obj' */
	if ( ( obj = get_obj_carry( ch, arg ) ) == NULL )
	{
	    send_to_char( "No tienes eso.\n\r", ch );
	    return;
	}

	if ( !can_drop_obj( ch, obj ) )
	{
	    send_to_char( "No puedes.\n\r", ch );
	    return;
	}

	separate_obj( obj );
	act( AT_ACTION, "$n deja $p.", ch, obj, NULL, TO_ROOM );
	act( AT_ACTION, "Dejas $p.", ch, obj, NULL, TO_CHAR );

	obj_from_char( obj );
	obj = obj_to_room( obj, ch->in_room );
	oprog_drop_trigger ( ch, obj );   /* mudprogs */

        if( char_died(ch) || obj_extracted(obj) )
          return;

	/* Clan storeroom saving */
	if ( IS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) )
	{
/*	   if (!char_died) save_char_obj(ch); */
	   for ( clan = first_clan; clan; clan = clan->next )
 	      if ( clan->storeroom == ch->in_room->vnum )
		save_clan_storeroom(ch, clan);
	}
    }
    else
    {
	int cnt = 0;
	char *chk;
	bool fAll;

	if ( !str_cmp(arg, "todo") )
	    fAll = TRUE;
	else
	    fAll = FALSE;
	if ( number > 1 )
	    chk = arg;
	else
	    chk = &arg[4];
	/* 'drop all' or 'drop all.obj' */
	if ( IS_SET( ch->in_room->room_flags, ROOM_NODROPALL )
        ||   IS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )
	{
	    send_to_char( "Parece que no puedes hacer eso aqui...\n\r", ch );
	    return;
	}
	found = FALSE;
	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;

	    if ( (fAll || nifty_is_name( chk, obj->name ) )
	    &&   can_see_obj( ch, obj )
	    &&   obj->wear_loc == WEAR_NONE
	    &&   can_drop_obj( ch, obj ) )
	    {
		found = TRUE;
		if ( HAS_PROG(obj->pIndexData, DROP_PROG) && obj->count > 1 )
		{
		   ++cnt;
		   separate_obj( obj );
		   obj_from_char( obj );
		   if ( !obj_next )
		     obj_next = ch->first_carrying;
		}
		else
		{
		   if ( number && (cnt + obj->count) > number )
		     split_obj( obj, number - cnt );
		   cnt += obj->count;
		   obj_from_char( obj );
		}
		act( AT_ACTION, "$n deja $p.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Dejas $p.", ch, obj, NULL, TO_CHAR );
		obj = obj_to_room( obj, ch->in_room );
		oprog_drop_trigger( ch, obj );		/* mudprogs */
                if ( char_died(ch) )
                    return;
		if ( number && cnt >= number )
		    break;
	    }
	}

	if ( !found )
	{
	    if ( fAll )
	      act( AT_PLAIN, "No estas llevando nada.",
		    ch, NULL, NULL, TO_CHAR );
	    else
	      act( AT_PLAIN, "No estas llevando ningun $T.",
		    ch, NULL, chk, TO_CHAR );
	}
    }
    if ( IS_SET( sysdata.save_flags, SV_DROP ) )
	save_char_obj( ch );	/* duping protector */
    return;
}



void do_give( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char buf  [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "a" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	send_to_char( "Darle que a quien?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( is_number( arg1 ) )
    {
	/* 'give NNNN coins victim' */
	int amount;

	amount   = atoi(arg1);
	if ( amount <= 0
	|| ( str_cmp( arg2, "monedas" ) && str_cmp( arg2, "moneda" ) ) )
	{
	    send_to_char( "No puedes hacer eso.\n\r", ch );
	    return;
	}

	argument = one_argument( argument, arg2 );
	if ( !str_cmp( arg2, "a" ) && argument[0] != '\0' )
	    argument = one_argument( argument, arg2 );
	if ( arg2[0] == '\0' )
	{
	    send_to_char( "Darle que a quien?\n\r", ch );
	    return;
	}

	if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
	{
	    send_to_char( "No esta aqui.\n\r", ch );
	    return;
	}

	if ( ch->gold < amount )
	{
	    send_to_char( "No tienes tanto oro.\n\r", ch );
	    return;
	}

	ch->gold     -= amount;
	victim->gold += amount;
        strcpy(buf, "$n te da ");
        strcat(buf, arg1 );
        strcat(buf, (amount > 1) ? " monedas de oro." : " moneda de oro.");

	act( AT_ACTION, buf, ch, NULL, victim, TO_VICT    );
	act( AT_ACTION, "$n le da a $N algo de oro.",  ch, NULL, victim, TO_NOTVICT );
	act( AT_ACTION, "Le das a $N algo de oro.",  ch, NULL, victim, TO_CHAR    );
	mprog_bribe_trigger( victim, ch, amount );
	if ( IS_SET( sysdata.save_flags, SV_GIVE ) && !char_died(ch) )
	    save_char_obj(ch);
	if ( IS_SET( sysdata.save_flags, SV_RECEIVE ) && !char_died(victim) )
	    save_char_obj(victim);
	return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No tienes eso.\n\r", ch );
	return;
    }

    if ( obj->wear_loc != WEAR_NONE )
    {
	send_to_char( "Debes desvestirlo primero.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( !can_drop_obj( ch, obj ) )
    {
	send_to_char( "No puedes.\n\r", ch );
	return;
    }

    if ( victim->carry_number + (get_obj_number(obj)/obj->count) > can_carry_n( victim ) )
    {
	act( AT_PLAIN, "$N tiene las manos llenas.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( victim->carry_weight + (get_obj_weight(obj)/obj->count) > can_carry_w( victim ) )
    {
	act( AT_PLAIN, "$N no puede llevar mas peso.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
	act( AT_PLAIN, "$N no puede verlo.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if (IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) && !can_take_proto( victim ) )
    {
	act( AT_PLAIN, "No puedes dar eso a $N!", ch, NULL, victim, TO_CHAR );
	return;
    }

    separate_obj(obj);
    obj_from_char(obj);
    act(AT_ACTION, "$n da $p a $N.", ch, obj, victim, TO_NOTVICT);
    act(AT_ACTION, "$n te da $p.",   ch, obj, victim, TO_VICT   );
    act(AT_ACTION, "Das $p a $N.", ch, obj, victim, TO_CHAR   );
    obj = obj_to_char(obj, victim);
    mprog_give_trigger(victim, ch, obj);
    if ( IS_SET(sysdata.save_flags, SV_GIVE) && !char_died(ch) )
	save_char_obj(ch);
    if ( IS_SET(sysdata.save_flags, SV_RECEIVE) && !char_died(victim) )
	save_char_obj(victim);
    return;
}

/*
 * Damage an object.						-Thoric
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 */
obj_ret damage_obj( OBJ_DATA *obj )
{
    CHAR_DATA *ch;
    obj_ret objcode;

    ch = obj->carried_by;
    objcode = rNONE;
    
    if ( IS_OBJ_STAT( obj, ITEM_QUEST ) ) 
    return;
    
    if (obj->item_type == ITEM_LIGHT)
    	return;

    separate_obj( obj );
    if ( !IS_NPC(ch) && (!IS_PKILL(ch) || (IS_PKILL(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_GAG))) )
	act( AT_OBJECT, "($p se deteriora)", ch, obj, NULL, TO_CHAR );
    else
    if ( obj->in_room && ( ch = obj->in_room->first_person ) != NULL )
    {
	act( AT_OBJECT, "($p se deteriora)", ch, obj, NULL, TO_ROOM );
	act( AT_OBJECT, "($p se deteriora)", ch, obj, NULL, TO_CHAR );
	ch = NULL;
    }
    if ( obj->item_type != ITEM_LIGHT )
    	oprog_damage_trigger(ch, obj);
    else if ( ( !in_arena( ch ) ) || ( !IS_OBJ_STAT( obj, ITEM_QUEST ) ) ) 
    	oprog_damage_trigger(ch, obj);

    if ( obj_extracted(obj) )
	return global_objcode;

    switch( obj->item_type )
    {
	default:
	  make_scraps( obj );
	  objcode = rOBJ_SCRAPPED;
	  break;
	case ITEM_CONTAINER:
	case ITEM_KEYRING:
	case ITEM_QUIVER:
	  if (--obj->value[3] <= 0)
	  {
	    if ( !in_arena( ch ) )
	    {
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
	    }
	    else
		obj->value[3] = 1;
	  }
	  break;
	case ITEM_LIGHT:
	  if (--obj->value[0] <= 0)
	  {
	    if ( !in_arena( ch ) )
	    {
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
	    }
	    else
		obj->value[0] = 1;
	  }
	  break;
	case ITEM_ARMOR:
	  if ( ch && obj->value[0] >= 1 )
	    ch->armor += apply_ac( obj, obj->wear_loc );
	  if (--obj->value[0] <= 0)
	  {
	    if ( !IS_PKILL( ch ) && !in_arena( ch ) )
	    {
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
	    }
	    else
	    {
		obj->value[0] = 1;
		ch->armor -= apply_ac( obj, obj->wear_loc );
	    }
	  }
	  else
	  if ( ch && obj->value[0] >= 1 )
	    ch->armor -= apply_ac( obj, obj->wear_loc );
	  break;
	case ITEM_WEAPON:
	  if (--obj->value[0] <= 0)
	  {
	     if ( !IS_PKILL( ch ) && !in_arena( ch ) )
	     {
		make_scraps( obj );
		objcode = rOBJ_SCRAPPED;
	     }
	     else
		obj->value[0] = 1;
	  }
	  break;
    }
    return objcode;
}


/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
    OBJ_DATA *obj, *tmpobj;

    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
	return TRUE;

    if ( !fReplace
    &&   ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
    {
	act( AT_PLAIN, "$d: no puedes llevar mas objetos.",
	    ch, NULL, obj->name, TO_CHAR );
	return FALSE;
    }

    if ( !fReplace )
	return FALSE;

    if ( IS_OBJ_STAT(obj, ITEM_NOREMOVE) )
    {
	act( AT_PLAIN, "No puedes deshacerte de $p.", ch, obj, NULL, TO_CHAR );
	return FALSE;
    }

    if ( obj == get_eq_char( ch, WEAR_WIELD )
    && ( tmpobj = get_eq_char( ch, WEAR_DUAL_WIELD)) != NULL )
       tmpobj->wear_loc = WEAR_WIELD;

    unequip_char( ch, obj );

    act( AT_ACTION, "$n deja de usar $p.", ch, obj, NULL, TO_ROOM );
    act( AT_ACTION, "Dejas de usar $p.", ch, obj, NULL, TO_CHAR );
    oprog_remove_trigger( ch, obj );
    return TRUE;
}

/*
 * See if char could be capable of dual-wielding		-Thoric
 */
bool could_dual( CHAR_DATA *ch )
{
    if ( IS_NPC(ch) || ch->pcdata->learned[gsn_dual_wield] )
	return TRUE;

    return FALSE;
}

/*
 * See if char can dual wield at this time			-Thoric
 */
bool can_dual( CHAR_DATA *ch )
{
    if ( !could_dual(ch) )
	return FALSE;

    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
	send_to_char( "Ya estas llevando dos armas!\n\r", ch );
	return FALSE;
    }
    if ( get_eq_char( ch, WEAR_SHIELD ) )
    {
	send_to_char( "Tienes que tener las DOS manos libres para llevar dos armas.\n\r", ch );
	return FALSE;
    }
    if ( get_eq_char( ch, WEAR_HOLD ) )
    {
	send_to_char( "Tienes que tener las DOS manos libres para llevar dos armas.\n\r", ch );
	return FALSE;
    }
    if( xIS_SET(ch->afectado_por, DAF_GARRAS_BESTIA))
    {
        send_to_char( "Tus manos tienen un aspecto algo extranyo y no puedes sujetar las armas.\n\r", ch );
        return FALSE;
    }
    return TRUE;
}


/*
 * Check to see if there is room to wear another object on this location
 * (Layered clothing support)
 */
bool can_layer( CHAR_DATA *ch, OBJ_DATA *obj, sh_int wear_loc )
{
    OBJ_DATA   *otmp;
    sh_int	bitlayers = 0;
    sh_int	objlayers = obj->pIndexData->layers;

    for ( otmp = ch->first_carrying; otmp; otmp = otmp->next_content )
	if ( otmp->wear_loc == wear_loc )
	    if ( !otmp->pIndexData->layers )
		return FALSE;
	    else
		bitlayers |= otmp->pIndexData->layers;

    if ( (bitlayers && !objlayers) || bitlayers > objlayers )
	return FALSE;
    if ( !bitlayers || ((bitlayers & ~objlayers) == bitlayers) )
	return TRUE;
    return FALSE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 *
 * Restructured a bit to allow for specifying body location	-Thoric
 * & Added support for layering on certain body locations
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, sh_int wear_bit )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *tmpobj = NULL;
    sh_int bit, tmp;

    separate_obj( obj );
    if ( get_trust( ch ) < obj->level )
    {
	sprintf( buf, "Has de ser nivel %d para usar este objeto.\n\r",
	    obj->level );
	send_to_char( buf, ch );
	act( AT_ACTION, "$n intenta usar $p, pero es demasiado novato.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( !IS_IMMORTAL(ch)
    &&  (( IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR)
        && ch->class == CLASS_WARRIOR					)
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR)
        && ch->class == CLASS_PALADIN					        )
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_MAGE)
	&& ch->class == CLASS_MAGE					)
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_THIEF)
	&& ch->class == CLASS_THIEF					)
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_VAMPIRE)
	&& ch->class == CLASS_VAMPIRE					)
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_DRUID)
	&& ch->class == CLASS_DRUID					)
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_WARRIOR)
	&& ch->class == CLASS_RANGER					)
    ||	 ( IS_OBJ_STAT(obj, ITEM_ANTI_MAGE)
	&& ch->class == CLASS_AUGURER					)
    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_CLERIC)
	&& ch->class == CLASS_CLERIC					)) )
    {
	act( AT_MAGIC, "Tienes prohibido usar este objeto.", ch, NULL, NULL, TO_CHAR );
	act( AT_ACTION, "$n intenta usar $p, pero tiene prohibido hacerlo.",
	    ch, obj, NULL, TO_ROOM );
	return;
    }

    if ( wear_bit > -1 )
    {
	bit = wear_bit;
	if ( !CAN_WEAR(obj, 1 << bit) )
	{
	    if ( fReplace )
	    {
		switch( 1 << bit )
		{
		    case ITEM_HOLD:
			send_to_char( "No puedes llevar eso.\n\r", ch );
			break;
		    case ITEM_WIELD:
		    case ITEM_MISSILE_WIELD:
			send_to_char( "No puedes llevar eso.\n\r", ch );
			break;
		    default:
			sprintf( buf, "No puedes llevar eso en tu %s.\n\r",
				w_flags[bit] );
			send_to_char( buf, ch );
		}
	    }
	    return;
	}
    }
    else
    {
	for ( bit = -1, tmp = 1; tmp < 31; tmp++ )
	{
	    if ( CAN_WEAR(obj, 1 << tmp) )
	    {
		bit = tmp;
		break;
	    }
	}
    }

    /* currently cannot have a light in non-light position */
    if ( obj->item_type == ITEM_LIGHT )
    {
	if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
	    return;
        if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
        {
	    act( AT_ACTION, "$n sostiene $p como luz.", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "Sostienes $p como luz.",  ch, obj, NULL, TO_CHAR );
        }
	equip_char( ch, obj, WEAR_LIGHT );
        oprog_wear_trigger( ch, obj );
	return;
    }

    if ( bit == -1 )
    {
	if ( fReplace )
	    send_to_char( "No puedes llevar, blandir ni sostener eso.\n\r", ch );
	return;
    }

    switch ( 1 << bit )
    {
	default:
	    bug( "wear_obj: uknown/unused item_wear bit %d", bit );
	    if ( fReplace )
		send_to_char( "No puedes llevar, blandir ni sostener eso.\n\r", ch );
	    return;

	case ITEM_WEAR_FINGER:
	    if ( get_eq_char( ch, WEAR_FINGER_L )
	    &&   get_eq_char( ch, WEAR_FINGER_R )
	    &&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	    &&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
		return;

	    if ( !get_eq_char( ch, WEAR_FINGER_L ) )
	    {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
		    act( AT_ACTION, "$n se pone $p en su dedo izquierdo.",    ch, obj, NULL, TO_ROOM );
		    act( AT_ACTION, "Te pones $p en tu dedo izquierdo.",  ch, obj, NULL, TO_CHAR );
                }
		equip_char( ch, obj, WEAR_FINGER_L );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    if ( !get_eq_char( ch, WEAR_FINGER_R ) )
	    {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
		    act( AT_ACTION, "$n se pone $p en su dedo derecho.",   ch, obj, NULL, TO_ROOM );
		    act( AT_ACTION, "Te pones $p en tu dedo derecho.", ch, obj, NULL, TO_CHAR );
                }
		equip_char( ch, obj, WEAR_FINGER_R );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    bug( "Wear_obj: No hay dedos libres (?).", 0 );
	    send_to_char( "Ya llevas algo en los dos dedos.\n\r", ch );
	    return;

	case ITEM_WEAR_NECK:
	    if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	    &&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	    &&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	    &&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
		return;

	    if ( !get_eq_char( ch, WEAR_NECK_1 ) )
	    {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
		  act( AT_ACTION, "$n se pone $p alrededor del cuello.",   ch, obj, NULL, TO_ROOM );
		  act( AT_ACTION, "Te pones $p alrededor del cuello.", ch, obj, NULL, TO_CHAR );
                }
		equip_char( ch, obj, WEAR_NECK_1 );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    if ( !get_eq_char( ch, WEAR_NECK_2 ) )
	    {
                if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                {
		  act( AT_ACTION, "$n se pone $p alrededor del cuello.",   ch, obj, NULL, TO_ROOM );
		  act( AT_ACTION, "Te pones $p alrededor del cuello.", ch, obj, NULL, TO_CHAR );
                }
		equip_char( ch, obj, WEAR_NECK_2 );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    bug( "Wear_obj: el cuello no esta libre.", 0 );
	    send_to_char( "Ya llevas dos objetos alrededor del cuello.\n\r", ch );
	    return;

	case ITEM_WEAR_BODY:

	    if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
		return;

	    if ( !can_layer( ch, obj, WEAR_BODY ) )
	    {
		send_to_char( "No te puedes poner nada en el cuerpo sin quitarte lo que ya llevas.\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n se pone $p en $s cuerpo.",   ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Te pones $p en el cuerpo.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_BODY );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_HEAD:
	    if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
		return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n se pone $p en la cabeza.",   ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Te pones $p en la cabeza.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_HEAD );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_EYES:
	    if ( !remove_obj( ch, WEAR_EYES, fReplace ) )
		return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n se pone $p en los $s ojos.",   ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Te pones $p en los ojos.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_EYES );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_FACE:
	    if ( !remove_obj( ch, WEAR_FACE, fReplace ) )
		return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n se pone $p sobre la cara.",   ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Te pones $p sobre la cara.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_FACE );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_EARS:
	    if ( !remove_obj( ch, WEAR_EARS, fReplace ) )
		return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n se pone $p en las orejas.",   ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Te pones $p en las orejas.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_EARS );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_LEGS:

	    if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
		return;

	    if ( !can_layer( ch, obj, WEAR_LEGS ) )
	    {
		send_to_char( "No puedes ponerte nada en las piernas sin quitarte lo que ya llevas puesto.\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n se pone $p en las piernas.",   ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Te pones $p en las piernas.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_LEGS );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_FEET:

	    if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
	      return;

	    if ( !can_layer( ch, obj, WEAR_FEET ) )
	    {
		send_to_char( "No puedes ponerte nada en los pies sin quitarte lo que ya llevas puesto.\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
	      act( AT_ACTION, "$n se pone $p en los pies.",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "Te pones $p en los pies.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_FEET );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_HANDS:

	    if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
	      return;

	    if ( !can_layer( ch, obj, WEAR_HANDS ) )
	    {
		send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
	      act( AT_ACTION, "$n se pone $p en las manos.",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "Te pones $p en las manos.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_HANDS );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_ARMS:

	    if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
	      return;

	    if ( !can_layer( ch, obj, WEAR_ARMS ) )
	    {
		send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
		return;
	    }
	    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
	      act( AT_ACTION, "$n se pone $p en los brazos.",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "Te pones $p en los brazos.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_ARMS );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_ABOUT:

	    if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
	      return;

	    if ( !can_layer( ch, obj, WEAR_ABOUT ) )
	    {
		send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
	      act( AT_ACTION, "$n se pone $p sobre el cuerpo.",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "Te pones $p sobre el cuerpo.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_ABOUT );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_BACK:
	    if ( !remove_obj( ch, WEAR_BACK, fReplace ) )
		return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n se pone $p en la espalda.",   ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Te pones $p en la espalda.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_BACK );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_WAIST:

	    if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	      return;

	    if ( !can_layer( ch, obj, WEAR_WAIST ) )
	    {
		send_to_char( "It won't fit overtop of what you're already wearing.\n\r", ch );
		return;
	    }
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
	      act( AT_ACTION, "$n se pone $p alrededor de la cintura.",   ch, obj, NULL, TO_ROOM );
	      act( AT_ACTION, "Te pones $p alrededor de la cintura.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_WAIST );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_WEAR_WRIST:
	    if ( get_eq_char( ch, WEAR_WRIST_L )
	    &&   get_eq_char( ch, WEAR_WRIST_R )
	    &&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	    &&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
	       return;

	    if ( !get_eq_char( ch, WEAR_WRIST_L ) )
	    {
		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
		   act( AT_ACTION, "$n se pone $p en $s munyeca izquierdo.",
			ch, obj, NULL, TO_ROOM );
		   act( AT_ACTION, "Te pones $p en tu munyeca izquierdo.",
			ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEAR_WRIST_L );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    if ( !get_eq_char( ch, WEAR_WRIST_R ) )
	    {
		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
		    act( AT_ACTION, "$n se pone $p en $s munyeca derecho.",
			ch, obj, NULL, TO_ROOM );
		    act( AT_ACTION, "Te pones $p en tu munyeca derecho.",
			ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEAR_WRIST_R );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    bug( "Wear_obj: No hay tobillos libres.", 0 );
	    send_to_char( "Ya tienes las dos munyecas ocupadas.\n\r", ch );
	    return;

	case ITEM_WEAR_ANKLE:
	    if ( get_eq_char( ch, WEAR_ANKLE_L )
	    &&   get_eq_char( ch, WEAR_ANKLE_R )
	    &&   !remove_obj( ch, WEAR_ANKLE_L, fReplace )
	    &&   !remove_obj( ch, WEAR_ANKLE_R, fReplace ) )
	       return;

	    if ( !get_eq_char( ch, WEAR_ANKLE_L ) )
	    {
		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
		   act( AT_ACTION, "$n se pone $p en $s tobillo izquierdo.",
			ch, obj, NULL, TO_ROOM );
		   act( AT_ACTION, "Te pones $p en tu tobillo izquierdo.",
			ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEAR_ANKLE_L );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    if ( !get_eq_char( ch, WEAR_ANKLE_R ) )
	    {
		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
		    act( AT_ACTION, "$n se pone $p en $s tobillo derecho.",
			ch, obj, NULL, TO_ROOM );
		    act( AT_ACTION, "Te pones $p en tu tobillo derecho.",
			ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEAR_ANKLE_R );
		oprog_wear_trigger( ch, obj );
		return;
	    }

	    bug( "Wear_obj: no free ankle.", 0 );
	    send_to_char( "Ya tienes los tobillos ocupados.\n\r", ch );
	    return;

	case ITEM_WEAR_SHIELD:
	    if ( get_eq_char(ch, WEAR_DUAL_WIELD)
	    ||  (get_eq_char(ch, WEAR_WIELD) && get_eq_char(ch, WEAR_MISSILE_WIELD)) )
	    {
		send_to_char( "No puedes usar un escudo y DOS armas!\n\r", ch );
		return;
	    }
	    if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
		return;
            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n se pone $p como escudo.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Te pones $p como escudo.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_SHIELD );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_MISSILE_WIELD:
	case ITEM_WIELD:
	    if ( !could_dual(ch) )
	    {
		if ( !remove_obj( ch, WEAR_MISSILE_WIELD, fReplace ) )
		    return;
		if ( !remove_obj(ch, WEAR_WIELD, fReplace) )
		    return;
		tmpobj = NULL;
	    }
	    else
	    {
		OBJ_DATA *mw, *dw, *hd;

		tmpobj = get_eq_char(ch, WEAR_WIELD);
		mw = get_eq_char(ch, WEAR_MISSILE_WIELD);
		dw = get_eq_char(ch, WEAR_DUAL_WIELD);
		if ( tmpobj && (mw || dw) )
		{
		    send_to_char( "Ya estas llevando dos armas.\n\r", ch );
		    return;
		}
                /* Para que los que usan el garras de la bestia no pueda blandir armas */

                if(!IS_IMMORTAL(ch) && xIS_SET(ch->afectado_por, DAF_GARRAS_BESTIA))
                {
                        send_to_char( "Tus manos tienen un aspecto animal y no puedes blandir armas con ellas.\n\r", ch );
                        return;
                }
		hd = get_eq_char(ch, WEAR_HOLD);
		if ( (mw && hd) || (tmpobj && hd) )
		{
		    send_to_char( "Ya estas llevando un arma y sosteniendo algo.\n\r", ch );
		    return;
		}
	    }

	    if ( tmpobj )
	    {
		if ( can_dual(ch) )
		{
		    if ( get_obj_weight(obj) + get_obj_weight(tmpobj) > str_app[get_curr_str(ch)].wield )
		    {
			send_to_char( "Esta arma pesa demasiado para blandirla.\n\r", ch );
			return;
	      	    }
                    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
                    {
			act( AT_ACTION, "$n blande $p como arma secundaria.", ch, obj, NULL, TO_ROOM );
			act( AT_ACTION, "Blandes $p como arma secundaria.", ch, obj, NULL, TO_CHAR );
                    }
		    if ( 1 << bit == ITEM_MISSILE_WIELD )
			equip_char( ch, obj, WEAR_MISSILE_WIELD );
		    else
			equip_char( ch, obj, WEAR_DUAL_WIELD );
		    oprog_wear_trigger( ch, obj );
		}
	        return;
	    }

	    if ( get_obj_weight(obj) > str_app[get_curr_str(ch)].wield )
	    {
		send_to_char( "Esta arma pesa demasiado para blandirla.\n\r", ch );
		return;
	    }

            if(!IS_IMMORTAL(ch) && xIS_SET(ch->afectado_por, DAF_GARRAS_BESTIA))
                {
                        send_to_char( "Tus manos tienen un aspecto animal y no puedes blandir armas con ellas.\n\r", ch );
                        return;
                }

            if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n blande $p.", ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Blandes $p.", ch, obj, NULL, TO_CHAR );
            }
	    if ( 1 << bit == ITEM_MISSILE_WIELD )
		equip_char( ch, obj, WEAR_MISSILE_WIELD );
	    else
		equip_char( ch, obj, WEAR_WIELD );
	    oprog_wear_trigger( ch, obj );
	    return;

	case ITEM_HOLD:
	    if ( get_eq_char(ch, WEAR_DUAL_WIELD)
	    ||  (get_eq_char(ch, WEAR_WIELD) && get_eq_char(ch, WEAR_MISSILE_WIELD)) )
	    {
		send_to_char( "No puedes sostener algo y llevar DOS armas!\n\r", ch );
		return;
	    }
	    if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
		return;
            if ( obj->item_type == ITEM_WAND
               || obj->item_type == ITEM_STAFF
               || obj->item_type == ITEM_FOOD
	       || obj->item_type == ITEM_COOK
               || obj->item_type == ITEM_PILL
               || obj->item_type == ITEM_POTION
               || obj->item_type == ITEM_SCROLL
               || obj->item_type == ITEM_DRINK_CON
               || obj->item_type == ITEM_BLOOD
               || obj->item_type == ITEM_PIPE
               || obj->item_type == ITEM_HERB
               || obj->item_type == ITEM_KEY
               || !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
            {
		act( AT_ACTION, "$n sostiene $p en $s manos.",   ch, obj, NULL, TO_ROOM );
		act( AT_ACTION, "Sostienes $p en tus manos.", ch, obj, NULL, TO_CHAR );
            }
	    equip_char( ch, obj, WEAR_HOLD );
	    oprog_wear_trigger( ch, obj );
	    return;

         /* Nuevo wear satelite SiGo */
       		case ITEM_WEAR_SATELITE:
                if ( !remove_obj( ch, WEAR_SATELITE, fReplace ) )
                        return;
                if ( !get_eq_char( ch, WEAR_SATELITE ) )
	        {
		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
		   act( AT_ACTION, "$n suelta $p que comienza a flotar a su alrededor.",
			ch, obj, NULL, TO_ROOM );
		   act( AT_ACTION, "Sueltas $p al aire y comienza a flotar a tu arededor.",
			ch, obj, NULL, TO_CHAR );
		}
		equip_char( ch, obj, WEAR_SATELITE );
		oprog_wear_trigger( ch, obj );
		return;
                }
    }
}


void do_wear( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    sh_int wear_bit;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( (!str_cmp(arg2, "sobre")  || !str_cmp(arg2, "por encima de") || !str_cmp(arg2, "alrededor"))
    &&   argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Llevar, sostener o blandir que?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( !str_cmp( arg1, "todo" ) )
    {
	OBJ_DATA *obj_next;

	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	    {
		wear_obj( ch, obj, FALSE, -1 );
		if ( char_died(ch) )
		    return;
	    }
	}
	return;
    }
    else
    {
	if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
	{
	    send_to_char( "No tienes eso.\n\r", ch );
	    return;
	}
	if ( arg2[0] != '\0' )
	  wear_bit = get_wflag(arg2);
	else
	  wear_bit = -1;
	wear_obj( ch, obj, TRUE, wear_bit );
    }

    return;
}



void do_remove( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj, *obj_next;


    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Desvestirte que?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

   if ( !str_cmp( arg, "todo" ) )  /* SB Remove all */
    {
      for ( obj = ch->first_carrying; obj != NULL ; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( obj->wear_loc != WEAR_NONE && can_see_obj ( ch, obj ) )
          remove_obj ( ch, obj->wear_loc, TRUE );
      }
      return;
    }

    if ( ( obj = get_obj_wear( ch, arg ) ) == NULL )
    {
	send_to_char( "No estas usando eso.\n\r", ch );
	return;
    }
    if ( (obj_next=get_eq_char(ch, obj->wear_loc)) != obj )
    {
	act( AT_PLAIN, "Debes quitarte $p primero.", ch, obj_next, NULL, TO_CHAR );
	return;
    }

    remove_obj( ch, obj->wear_loc, TRUE );
    return;
}


void do_bury( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    bool shovel;
    sh_int move;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Que quieres enterrar?\n\r", ch );
        return;
    }

    if ( ms_find_obj(ch) )
        return;

    shovel = FALSE;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
      if ( obj->item_type == ITEM_SHOVEL )
      {
	  shovel = TRUE;
	  break;
      }

    obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );
    if ( !obj )
    {
        send_to_char( "No puedes encontrarlo.\n\r", ch );
        return;
    }

    separate_obj(obj);
    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	if ( !IS_OBJ_STAT( obj, ITEM_CLANCORPSE )
	|| IS_NPC(ch) || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )
	   {
		act( AT_PLAIN, "No puedes enterrar $p.", ch, obj, 0, TO_CHAR );
        	return;
           }
    }

    switch( ch->in_room->sector_type )
    {
	case SECT_CITY:
	case SECT_INSIDE:
	    send_to_char( "El suelo esta demasiado duro para poder cavar.\n\r", ch );
	    return;
	case SECT_WATER_SWIM:
	case SECT_WATER_NOSWIM:
	case SECT_UNDERWATER:
	    send_to_char( "No puedes enterrar nada aqui.\n\r", ch );
	    return;
	case SECT_AIR:
	    send_to_char( "Que? En el aire?!\n\r", ch );
	    return;
    }

    if ( obj->weight > (UMAX(5, (can_carry_w(ch) / 10)))
    &&  !shovel )
    {
	send_to_char( "Necesitarias una escavadora para enterrar algo tan grande.\n\r", ch );
	return;
    }

    move = (obj->weight * 50 * (shovel ? 1 : 5)) / UMAX(1, can_carry_w(ch));
    move = URANGE( 2, move, 1000 );
    if ( move > ch->move )
    {
	send_to_char( "No podrias enterrar algo de este tamanyo.\n\r", ch );
	return;
    }
    ch->move -= move;
    if ( obj->item_type == ITEM_CORPSE_NPC
    ||   obj->item_type == ITEM_CORPSE_PC )
	adjust_favor( ch, 6, 1 );

    act( AT_ACTION, "Entierras $p...", ch, obj, NULL, TO_CHAR );
    act( AT_ACTION, "$n entierra $p...", ch, obj, NULL, TO_ROOM );
    xSET_BIT( obj->extra_flags, ITEM_BURIED );
    WAIT_STATE( ch, URANGE( 10, move / 2, 100 ) );
    return;
}

void do_sacrifice( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char name[50];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )
    {
	act( AT_ACTION, "$n se ofrece como sacrificio a los dioses.",
	    ch, NULL, NULL, TO_ROOM );
	send_to_char( "Los dioses aprecian tu oferta, puede que acepten mas tarde.\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    obj = get_obj_list_rev( ch, arg, ch->in_room->last_content );
    if ( !obj )
    {
	send_to_char( "No puedes hacer eso.\n\r", ch );
	return;
    }

    separate_obj(obj);
    if ( !CAN_WEAR(obj, ITEM_TAKE) )
    {
	act( AT_PLAIN, "$p no es un sacrificio aceptable.", ch, obj, 0, TO_CHAR );
	return;
    }

    if( IS_OBJ_STAT( obj, ITEM_QUEST)  || IS_OBJ_STAT( obj, ITEM_LOYAL ) )
    {
       if( str_cmp( obj->propietario, ch->name ) )
       {
        act( AT_PLAIN, "No puedes sacrificar $p, no es tuyo.\n\r", ch, obj, 0, TO_CHAR );
        act( AT_PLAIN, "$n ha intentado sacrificar $p que no es suyo.\n\r", ch, obj, 0, TO_ROOM );
       }
       else
        act( AT_PLAIN, "Realmente no quieres sacrificar $p verdad?.\n\r", ch, obj, 0, TO_CHAR );

        return;
    }

    if ( IS_SET( obj->magic_flags, ITEM_PKDISARMED )
    &&  !IS_NPC( ch ) )
    {
        if ( CAN_PKILL( ch ) && !get_timer( ch, TIMER_PKILLED ) )
        {
            if ( ch->level - obj->value[5] > 5
            ||   obj->value[5] - ch->level > 5 )
            {
                send_to_char_color( "\n\r&bUna fuerza divina frena tu mano.\n\r", ch );
                return;
            }
	}
    }
    if ( !IS_NPC( ch ) && ch->pcdata->deity && ch->pcdata->deity->name[0] != '\0' )
    {
	strcpy( name, ch->pcdata->deity->name );
    }
    else if ( !IS_NPC( ch ) && IS_GUILDED(ch) && sysdata.guild_overseer[0] != '\0' )
    {
	strcpy( name, sysdata.guild_overseer );
    }
    else if ( !IS_NPC( ch ) && ch->pcdata->clan && ch->pcdata->clan->deity[0] != '\0' )
    {
	strcpy( name, ch->pcdata->clan->deity );
    }
    else
    {
	strcpy( name, "Enoch" );
    }
    ch->gold += 1;
    if ( obj->item_type == ITEM_CORPSE_NPC
    ||   obj->item_type == ITEM_CORPSE_PC )
       adjust_favor( ch, 5, 1 );
    sprintf( buf, "%s te da una moneda de oro por tu sacrificio.\n\r", name );
    send_to_char( buf, ch );
    sprintf( buf, "$n sacrifica $p a %s.", name );
    act( AT_ACTION, buf, ch, obj, NULL, TO_ROOM );
    oprog_sac_trigger( ch, obj );
    if ( obj_extracted(obj) )
      return;
    if ( cur_obj == obj->serial )
      global_objcode = rOBJ_SACCED;
    extract_obj( obj );
    return;
}

void do_brandish( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    OBJ_DATA *staff;
    ch_ret retcode;
    int sn;

    if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "No sostienes nada en tus manos.\n\r", ch );
	return;
    }

    if ( staff->item_type != ITEM_STAFF )
    {
	send_to_char( "Solo puede empunyar una vara.\n\r", ch );
	return;
    }

    if ( ( sn = staff->value[3] ) < 0
    ||   sn >= top_sn
    ||   skill_table[sn]->spell_fun == NULL )
    {
	bug( "Do_brandish: bad sn %d.", sn );
	return;
    }

    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    if ( staff->value[2] > 0 )
    {
      if ( !oprog_use_trigger( ch, staff, NULL, NULL, NULL ) )
      {
        act( AT_MAGIC, "$n empunya $p.", ch, staff, NULL, TO_ROOM );
        act( AT_MAGIC, "Empunyas $p.",  ch, staff, NULL, TO_CHAR );
      }
	for ( vch = ch->in_room->first_person; vch; vch = vch_next )
	{
	    vch_next	= vch->next_in_room;
            if ( !IS_NPC( vch ) && xIS_SET( vch->act, PLR_WIZINVIS )
                  && vch->pcdata->wizinvis >= LEVEL_IMMORTAL )
                continue;
            else
	    switch ( skill_table[sn]->target )
	    {
	    default:
		bug( "Do_brandish: bad target for sn %d.", sn );
		return;

	    case TAR_IGNORE:
		if ( vch != ch )
		    continue;
		break;

	    case TAR_CHAR_OFFENSIVE:
		if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_DEFENSIVE:
		if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
		    continue;
		break;

	    case TAR_CHAR_SELF:
		if ( vch != ch )
		    continue;
		break;
	    }

	    retcode = obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
	    if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
	    {
		bug( "do_brandish: char died", 0 );
		return;
	    }
	}
    }

    if ( --staff->value[2] <= 0 )
    {
	act( AT_MAGIC, "$p brilla y desaparece de las manos de $n!", ch, staff, NULL, TO_ROOM );
	act( AT_MAGIC, "$p brilla y desaparece!", ch, staff, NULL, TO_CHAR );
	if ( staff->serial == cur_obj )
	  global_objcode = rOBJ_USED;
	extract_obj( staff );
    }

    return;
}



void do_zap( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *wand;
    OBJ_DATA *obj;
    ch_ret retcode;

    one_argument( argument, arg );
    if ( arg[0] == '\0' && !ch->fighting )
    {
	send_to_char( "Agitar que o a quien?\n\r", ch );
	return;
    }

    if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
    {
	send_to_char( "No sostienes nada en tus manos.\n\r", ch );
	return;
    }

    if ( wand->item_type != ITEM_WAND )
    {
	send_to_char( "Solo puedes agitar una varita.\n\r", ch );
	return;
    }

    obj = NULL;
    if ( arg[0] == '\0' )
    {
	if ( ch->fighting )
	{
	    victim = who_fighting( ch );
	}
	else
	{
	    send_to_char( "Agitar que o a quien?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	&&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL )
	{
	    send_to_char( "No puedes encontrarlo.\n\r", ch );
	    return;
	}
    }

    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );

    if ( wand->value[2] > 0 )
    {
	if ( victim )
	{
          if ( !oprog_use_trigger( ch, wand, victim, NULL, NULL ) )
          {
	    act( AT_MAGIC, "$n agita $p sobre $N.", ch, wand, victim, TO_ROOM );
	    act( AT_MAGIC, "Agitas $p sobre $N.", ch, wand, victim, TO_CHAR );
          }
	}
	else
	{
          if ( !oprog_use_trigger( ch, wand, NULL, obj, NULL ) )
          {
	    act( AT_MAGIC, "$n agita $p sobre $P.", ch, wand, obj, TO_ROOM );
	    act( AT_MAGIC, "Agitas $p sobre $P.", ch, wand, obj, TO_CHAR );
          }
	}

	retcode = obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
	if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )
	{
	   bug( "do_zap: char died", 0 );
	   return;
	}
    }

    if ( --wand->value[2] <= 0 )
    {
      act( AT_MAGIC, "$p explota en mil pedazos!", ch, wand, NULL, TO_ROOM );
      act( AT_MAGIC, "$p explota en mil pedazos!.", ch, wand, NULL, TO_CHAR );
      if ( wand->serial == cur_obj )
        global_objcode = rOBJ_USED;
      extract_obj( wand );
    }

    return;
}

/*
 * Save items in a clan storage room			-Scryn & Thoric
 */
void save_clan_storeroom( CHAR_DATA *ch, CLAN_DATA *clan )
{
    FILE *fp;
    char filename[256];
    sh_int templvl;
    OBJ_DATA *contents;

    if ( !clan )
    {
	bug( "save_clan_storeroom: Null clan pointer!", 0 );
	return;
    }

    if ( !ch )
    {
	bug ("save_clan_storeroom: Null ch pointer!", 0);
	return;
    }

    sprintf( filename, "%s%s.vault", CLAN_DIR, clan->filename );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
	bug( "save_clan_storeroom: fopen", 0 );
	perror( filename );
    }
    else
    {
	templvl = ch->level;
	ch->level = LEVEL_HERO;		/* make sure EQ doesn't get lost */
        contents = ch->in_room->last_content;
        if (contents)
	  fwrite_obj(ch, contents, fp, 0, OS_CARRY );
	fprintf( fp, "#END\n" );
	ch->level = templvl;
	fclose( fp );
	return;
    }
    return;
}

/* put an item on auction, or see the stats on the current item or bet */
void do_auction (CHAR_DATA *ch, char *argument)
{
    OBJ_DATA *obj;
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int i;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    argument = one_argument (argument, arg3);

    set_char_color( AT_LBLUE, ch );

    if (IS_NPC(ch)) /* NPC can be extracted at any time and thus can't auction! */
	return;

    if ( ch->level < 3 )
    {
	send_to_char( "Debes ser al menos nivel 3 para poder subastar...\n\r", ch );
	return;
    }

    if ( ( time_info.hour > 20 || time_info.hour < 8 )
    && auction->item == NULL
    && !IS_IMMORTAL( ch ) )
    {
	send_to_char ("\n\rEl subastador trabaja desde las 8 AM a las 8 PM.\n\r", ch );
        return;
    }

    if (arg1[0] == '\0')
    {
        if (auction->item != NULL)
        {
	    AFFECT_DATA *paf;
  	    obj = auction->item;

            /* show item data here */
            if (auction->bet > 0)
                sprintf (buf, "\n\rLa oferta actual es de %s monedas de oro.\n\r", num_punct( auction->bet ) );
            else
                sprintf (buf, "\n\rAun no se han recibido ofertas.\n\r");
	    set_char_color ( AT_BLUE, ch );
            send_to_char (buf,ch);
/*          spell_identify (0, LEVEL_HERO - 1, ch, auction->item); */

	    sprintf( buf,
		"Objeto '%s' es %s, Habilidades especiales: %s\n\rPeso: %d, Valor: %d, y su nivel es %d.\n\r",
		obj->name,
		aoran( item_type_name( obj ) ),
		extra_bit_name( &obj->extra_flags ),
/*		magic_bit_name( obj->magic_flags ), -- currently unused */
		obj->weight,
		obj->cost,
		obj->level );
	    set_char_color( AT_LBLUE, ch );
	    send_to_char( buf, ch );
	    if ( obj->item_type != ITEM_LIGHT && obj->wear_flags-1 > 0 )
	      ch_printf( ch, "Vestido en: %s\n\r",
		flag_string(obj->wear_flags -1, w_flags ) );

	    set_char_color( AT_BLUE, ch );

	    switch ( obj->item_type )
	    {
		case ITEM_CONTAINER:
		case ITEM_KEYRING:
		case ITEM_QUIVER:
                  ch_printf( ch, "%s parece que %s.\n\r", capitalize(obj->short_descr),
   	                obj->value[0] < 76  ? "tiene muy poca capacidad"           :
	                obj->value[0] < 150 ? "tiene poca capacidad" :
			obj->value[0] < 300 ? "tiene una capacidad media"          :
	                obj->value[0] < 500 ? "tiene bastante capacidad" :
	                obj->value[0] < 751 ? "tiene mucha capacidad"           :
	                                      "tiene una capacidad exagerada!" );
	          break;

		case ITEM_PILL:
		case ITEM_SCROLL:
		case ITEM_POTION:
		  sprintf( buf, "Nivel %d hechizos de:", obj->value[0] );
		  send_to_char( buf, ch );

		  if ( obj->value[1] >= 0 && obj->value[1] < top_sn )
		  {
		     send_to_char( " '", ch );
		     send_to_char( skill_table[obj->value[1]]->name, ch );
		     send_to_char( "'", ch );
		  }

		  if ( obj->value[2] >= 0 && obj->value[2] < top_sn )
		  {
		     send_to_char( " '", ch );
		     send_to_char( skill_table[obj->value[2]]->name, ch );
		     send_to_char( "'", ch );
		  }

		  if ( obj->value[3] >= 0 && obj->value[3] < top_sn )
		  {
		     send_to_char( " '", ch );
		     send_to_char( skill_table[obj->value[3]]->name, ch );
		     send_to_char( "'", ch );
		  }

		  send_to_char( ".\n\r", ch );
		  break;

		case ITEM_WAND:
		case ITEM_STAFF:
		  sprintf( buf, "Tiene %d(%d) cargas de nivel %d",
			obj->value[1], obj->value[2], obj->value[0] );
		  send_to_char( buf, ch );

		  if ( obj->value[3] >= 0 && obj->value[3] < top_sn )
		  {
		     send_to_char( " '", ch );
		     send_to_char( skill_table[obj->value[3]]->name, ch );
		     send_to_char( "'", ch );
		  }

		  send_to_char( ".\n\r", ch );
		  break;

		case ITEM_MISSILE_WEAPON:
		case ITEM_WEAPON:
		  sprintf( buf, "El danyo es de %d a %d (media %d).%s\n\r",
			obj->value[1], obj->value[2],
			( obj->value[1] + obj->value[2] ) / 2,
			IS_OBJ_STAT( obj, ITEM_POISONED) ?
			"\n\rEste arma esta envenenada." : "" );
		  send_to_char( buf, ch );
		  break;

		case ITEM_ARMOR:
		  sprintf( buf, "Armadura: %d.\n\r", obj->value[0] );
		  send_to_char( buf, ch );
		  break;
	    }

	    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		showaffect( ch, paf );

	    for ( paf = obj->first_affect; paf; paf = paf->next )
		showaffect( ch, paf );
	    if ( ( obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_KEYRING
	    ||     obj->item_type == ITEM_QUIVER)   && obj->first_content )
	    {
		set_char_color( AT_OBJECT, ch );
		send_to_char( "Contiene:\n\r", ch );
		show_list_to_char( obj->first_content, ch, TRUE, FALSE );
	    }

	    if (IS_IMMORTAL(ch))
	    {
		sprintf(buf, "Vendedor: %s.  Comprador: %s.  Round: %d.\n\r",
                        auction->seller->name, auction->buyer->name,
                        (auction->going + 1));
		send_to_char(buf, ch);
		sprintf(buf, "Tiempo restante en el round: %d.\n\r", auction->pulse);
		send_to_char(buf, ch);
	    }
            return;
	}
	else
	{
	    set_char_color ( AT_LBLUE, ch );
	    send_to_char ( "\n\rNo se esta subastando nada ahora. Que quieres subastar?\n\r", ch );
	    return;
	}
    }

    if ( IS_IMMORTAL(ch) && !str_cmp(arg1,"stop"))
    if (auction->item == NULL)
    {
        send_to_char ("No hay subastar que parar.\n\r",ch);
        return;
    }
    else /* stop the auction */
    {
	set_char_color ( AT_LBLUE, ch );
        sprintf (buf,"La subasta de %s ha sido detenida por un Inmortal.",
                        auction->item->short_descr);
        talk_auction (buf);
        obj_to_char (auction->item, auction->seller);
	if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
	    save_char_obj(auction->seller);
        auction->item = NULL;
        if (auction->buyer != NULL && auction->buyer != auction->seller) /* return money to the buyer */
        {
            auction->buyer->gold += auction->bet;
            send_to_char ("Tu dinero ha sido devuelto.\n\r",auction->buyer);
        }
        return;
    }

    if (!str_cmp(arg1,"pujar") )
        if (auction->item != NULL)
        {
            int newbet;

	    if ( ch->level < auction->item->level )
	    {
		send_to_char("No tienes suficiente experiencia para usar este objeto.\n\r", ch );
		return;
	    }

	    if ( ch == auction->seller)
	    {
		send_to_char("Lo estas subastando tu!\n\r", ch);
		return;
	    }

            /* make - perhaps - a bet now */
            if (arg2[0] == '\0')
            {
                send_to_char ("Cuanto quieres ofrecer?\n\r",ch);
                return;
            }

            newbet = parsebet (auction->bet, arg2);
/*	    ch_printf( ch, "Se ha ofrecido: %d\n\r",newbet);	*/

	    if (newbet < auction->starting)
	    {
		send_to_char("Debes aportar algo mas que la oferta inicial!\n\r", ch);
		return;
	    }

	    /* to avoid slow auction, use a bigger amount than 100 if the bet
 	       is higher up - changed to 10000 for our high economy
            */

            if (newbet < (auction->bet + 1000))
            {
                send_to_char ("Debes ofrecer al menos 1000 monedas mas sobre la actual oferta.\n\r",ch);
                return;
            }

            if (newbet > ch->gold)
            {
                send_to_char ("No tienes tanto dinero!\n\r",ch);
                return;
            }

	    if (newbet > 2000000000)
	    {
		send_to_char("No puedes ofrecer mas de 2 mil millones!\n\r", ch);
		return;
	    }

	    /* Is it the item they really want to bid on? --Shaddai */
	    if ( arg3[0] != '\0' &&
	 	 !nifty_is_name( arg3, auction->item->name ) )
	    {
	     send_to_char("Ese objeto no se esta subastando ahora.\n\r",ch);
	     return;
	    }
            /* the actual bet is OK! */

            /* return the gold to the last buyer, if one exists */
            if (auction->buyer != NULL && auction->buyer != auction->seller)
                auction->buyer->gold += auction->bet;

            ch->gold -= newbet; /* substract the gold - important :) */
	    if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
		save_char_obj(ch);
            auction->buyer = ch;
            auction->bet   = newbet;
            auction->going = 0;
            auction->pulse = PULSE_AUCTION; /* start the auction over again */

            sprintf (buf,"Una oferta de %s monedas de oro ha sido recibida por parte de %s.\n\r",num_punct(newbet),auction->item->short_descr);
            talk_auction (buf);
            return;


        }
        else
        {
            send_to_char ("Nada esta siendo subastado ahora mismo.\n\r",ch);
            return;
        }

/* finally... */
    if ( ms_find_obj(ch) )
	return;

    obj = get_obj_carry (ch, arg1); /* does char have the item ? */

    if (obj == NULL)
    {
        send_to_char ("No tienes eso.\n\r",ch);
        return;
    }

    if (obj->timer > 0)
    {
	send_to_char ("No puedes subastar objetos de ese tipo.\n\r", ch);
	return;
    }

    /* prevent repeat auction items */
    for(i = 0; i < AUCTION_MEM && auction->history[i]; i++)
    {
    	if(auction->history[i] == obj->pIndexData)
    	{
	    send_to_char("Ese objeto ha sido subastado "
	    	"recientemente, prueba otra vez mas tarde.\n\r", ch);
	    return;
    	}
    }


    if (arg2[0] == '\0')
    {
      auction->starting = 0;
      strcpy(arg2, "0");
    }

    if ( !is_number(arg2) )
    {
	send_to_char("Debes poner un numero de monedas para empezar la subasta.\n\r", ch);
	return;
    }

    if ( atoi(arg2) < 0 )
    {
	send_to_char("No puedes subastar algo por menos de 0 monedas!\n\r", ch);
 	return;
    }

    if (auction->item == NULL)
    switch (obj->item_type)
    {

    default:
        act (AT_TELL, "No puedes subastar $T.",ch, NULL, item_type_name (obj), TO_CHAR);
        return;

/* insert any more item types here... items with a timer MAY NOT BE
   AUCTIONED!
*/
    case ITEM_LIGHT:
    case ITEM_TREASURE:
    case ITEM_POTION:
    case ITEM_CONTAINER:
    case ITEM_KEYRING:
    case ITEM_QUIVER:
    case ITEM_DRINK_CON:
    case ITEM_FOOD:
    case ITEM_COOK:
    case ITEM_PEN:
    case ITEM_BOAT:
    case ITEM_PILL:
    case ITEM_PIPE:
    case ITEM_HERB_CON:
    case ITEM_INCENSE:
    case ITEM_FIRE:
    case ITEM_RUNEPOUCH:
    case ITEM_MAP:
    case ITEM_BOOK:
    case ITEM_RUNE:
    case ITEM_MATCH:
    case ITEM_HERB:
    case ITEM_WEAPON:
    case ITEM_MISSILE_WEAPON:
    case ITEM_ARMOR:
    case ITEM_STAFF:
    case ITEM_WAND:
    case ITEM_SCROLL:
	separate_obj(obj);
	obj_from_char (obj);
	if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )
	    save_char_obj(ch);
	auction->item = obj;
	auction->bet = 0;
	auction->buyer = ch;
	auction->seller = ch;
	auction->pulse = PULSE_AUCTION;
	auction->going = 0;
	auction->starting = atoi(arg2);

    	/* add the new item to the history */
    	if(AUCTION_MEM > 0)
        {
     		memmove((char *) auction->history+sizeof(OBJ_INDEX_DATA *),
                      	auction->history, (AUCTION_MEM - 1)*sizeof(OBJ_INDEX_DATA *));
                auction->history[0] = obj->pIndexData;
	}

	/* reset the history timer */
	auction->hist_timer = 0;


	if (auction->starting > 0)
	  auction->bet = auction->starting;

	sprintf (buf, "Un nuevo objeto esta siendo subastado: %s a %d monedas de oro.", obj->short_descr, auction->starting);
	talk_auction (buf);

	return;

    } /* switch */
    else
    {
        act (AT_TELL, "$p se esta subastando ahora!",ch,auction->item,NULL,TO_CHAR);
	if ( !IS_IMMORTAL(ch) )
          WAIT_STATE( ch, PULSE_VIOLENCE );
        return;
    }
}



/* Make objects in rooms that are nofloor fall - Scryn 1/23/96 */

void obj_fall( OBJ_DATA *obj, bool through )
{
    EXIT_DATA *pexit;
    ROOM_INDEX_DATA *to_room;
    static int fall_count;
    char buf[MAX_STRING_LENGTH];
    static bool is_falling; /* Stop loops from the call to obj_to_room()  -- Altrag */

    if ( !obj->in_room || is_falling )
    	return;

    if (fall_count > 30)
    {
    	bug( "object falling in loop more than 30 times", 0 );
	extract_obj(obj);
    	fall_count = 0;
	return;
     }

     if ( IS_SET( obj->in_room->room_flags, ROOM_NOFLOOR )
     &&   CAN_GO( obj, DIR_DOWN )
     &&   !IS_OBJ_STAT( obj, ITEM_MAGIC ) )
     {

	pexit = get_exit( obj->in_room, DIR_DOWN );
    	to_room = pexit->to_room;

    	if (through)
	  fall_count++;
	else
	  fall_count = 0;

	if (obj->in_room == to_room)
	{
	    sprintf(buf, "Objeto cayendo en la misma habitacion, habitacion %d",
		to_room->vnum);
	    bug( buf, 0 );
	    extract_obj( obj );
            return;
	}

	if (obj->in_room->first_person)
	{
	  	act( AT_PLAIN, "$p cae a lo lejos...",
			obj->in_room->first_person, obj, NULL, TO_ROOM );
		act( AT_PLAIN, "$p cae a lo lejos...",
			obj->in_room->first_person, obj, NULL, TO_CHAR );
	}
	obj_from_room( obj );
	is_falling = TRUE;
	obj = obj_to_room( obj, to_room );
	is_falling = FALSE;

	if (obj->in_room->first_person)
	{
	  	act( AT_PLAIN, "$p cae a lo lejos...",
			obj->in_room->first_person, obj, NULL, TO_ROOM );
		act( AT_PLAIN, "$p cae a lo lejos...",
			obj->in_room->first_person, obj, NULL, TO_CHAR );
	}

 	if (!IS_SET( obj->in_room->room_flags, ROOM_NOFLOOR ) && through )
	{
/*		int dam = (int)9.81*sqrt(fall_count*2/9.81)*obj->weight/2;
*/		int dam = fall_count*obj->weight/2;
		/* Damage players */
		if ( obj->in_room->first_person && number_percent() > 15 )
		{
			CHAR_DATA *rch;
			CHAR_DATA *vch = NULL;
			int chcnt = 0;

			for ( rch = obj->in_room->first_person; rch;
				rch = rch->next_in_room, chcnt++ )
				if ( number_range( 0, chcnt ) == 0 )
					vch = rch;
			act( AT_WHITE, "$p cae sobre $n!", vch, obj, NULL, TO_ROOM );
			act( AT_WHITE, "$p cae sobre ti!", vch, obj, NULL, TO_CHAR );

			if ( IS_NPC( vch ) && xIS_SET( vch->act, ACT_HARDHAT ) )
			  act( AT_WHITE, "$p te cae en la cabeza!", vch, obj, NULL, TO_CHAR );
			else
			  damage( vch, vch, dam*vch->level, TYPE_UNDEFINED );
		}
    	/* Damage objects */
	    switch( obj->item_type )
     	    {
	     	case ITEM_WEAPON:
		case ITEM_ARMOR:
		    if ( (obj->value[0] - dam) <= 0 )
 		    {
   			if (obj->in_room->first_person)
			{
			act( AT_PLAIN, "$p se destruye con la caida!",
				obj->in_room->first_person, obj, NULL, TO_ROOM );
			act( AT_PLAIN, "$p se destruye con la caida!",
				obj->in_room->first_person, obj, NULL, TO_CHAR );
			}
			make_scraps(obj);
	 	    }
		    else
	           	obj->value[0] -= dam;
		    break;
		default:
		    if ( (dam*15) > get_obj_resistance(obj) )
		    {
	              if (obj->in_room->first_person)
		      {
 			    act( AT_PLAIN, "$p se destruye con la caida!",
			    	obj->in_room->first_person, obj, NULL, TO_ROOM );
			    act( AT_PLAIN, "$p se destruye con la caida!",
		    		obj->in_room->first_person, obj, NULL, TO_CHAR );
		      }
		      make_scraps(obj);
		    }
		    break;
	    }
     	}
     	obj_fall( obj, TRUE );
    }
    return;
}

/* Scryn, by request of Darkur, 12/04/98 */
/* Reworked recursive_note_find to fix crash bug when the note was left
 * blank.  7/6/98 -- Shaddai
 */

void do_findnote( CHAR_DATA *ch, char *argument )
{
  OBJ_DATA* obj;

  if ( IS_NPC(ch) )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  if( argument[0] == '\0' )
  {
    send_to_char( "Debes poner algo mas.\n\r", ch);
    return;
  }

  obj = recursive_note_find(ch->first_carrying, argument);

  if(obj)
  {
    if(obj->in_obj)
    {
      obj_from_obj(obj);
      obj = obj_to_char(obj, ch);
    }
    wear_obj(ch, obj, TRUE, -1);
  }
  else
    send_to_char("Nota no encontrada.\n\r", ch);
  return;
}

OBJ_DATA* recursive_note_find( OBJ_DATA *obj, char *argument )
{
  OBJ_DATA* returned_obj;
  bool match = TRUE;
  char* argcopy;
  char* subject;

  char arg[MAX_INPUT_LENGTH];
  char subj[MAX_STRING_LENGTH];

  if(!obj)
    return NULL;

  switch(obj->item_type)
  {
    case ITEM_PAPER:

      if ((subject = get_extra_descr( "_subject_", obj->first_extradesc))==NULL)
        break;
      sprintf(subj, "%s", strlower(subject) );
      subject = strlower(subj);

      argcopy = argument;

      while(match)
      {
	argcopy = one_argument(argcopy, arg);

        if(arg[0] == '\0')
          break;

	if(!strstr(subject, arg))
          match = FALSE;
      }


      if(match)
        return obj;
      break;

    case ITEM_CONTAINER:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
      if(obj->first_content)
      {
      	returned_obj = recursive_note_find(obj->first_content, argument);
        if(returned_obj)
          return returned_obj;
      }
      break;

    default:
      break;
  }

  return recursive_note_find(obj->next_content, argument);
}

void do_rolldie(CHAR_DATA *ch, char *argument)
{
  OBJ_DATA* die;

  char output_string[MAX_STRING_LENGTH];
  char roll_string[MAX_STRING_LENGTH];
  char total_string[MAX_STRING_LENGTH];

  char* verb;

/*  char* face_string = NULL;
  char** face_table = NULL;*/
  int rollsum = 0;
  int roll_count = 0;

  int numsides;
  int numrolls;

  bool* face_seen_table = NULL;

  if ( IS_NPC(ch) )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  if ( ( die = get_eq_char(ch, WEAR_HOLD) ) == NULL
  ||     die->item_type != ITEM_CHANCE )
  {
    ch_printf(ch, "Debes sostener un objeto para cambiar!\n\r");
    return;
  }

  numrolls = (is_number(argument)) ? atoi(argument) : 1;
  verb = get_chance_verb(die);

  if(numrolls > 100)
  {
    ch_printf(ch, "No puedes %s mas de 100 veces!\n\r", verb);
    return;
  }

  numsides = die->value[0];

  if(numsides <= 1)
  {
    ch_printf(ch, "No hay elementos de cambio en este juego!\n\r");
    return;
  }

  if(die->value[3] == 1)
  {
    if(numrolls > numsides)
    {
      ch_printf(ch, "Buen intento, pero solo puedes %s %d veces.\n\r", verb, numsides);
      return;
    }
    face_seen_table = calloc(numsides, sizeof(bool));
    if(!face_seen_table)
    {
      bug("do_rolldie: cannot allocate memory for face_seen_table array, terminating.\n\r",0);
      return;
    }
  }

  sprintf(roll_string, " ");

  while(roll_count++ < numrolls)
  {
    int current_roll;
    char current_roll_string[MAX_STRING_LENGTH];

    do
    {
      current_roll = number_range(1, numsides);
    } while(die->value[3] == 1 && face_seen_table[current_roll - 1] == TRUE);

    if(die->value[3] == 1)
      face_seen_table[current_roll - 1] = TRUE;

    rollsum += current_roll;

    if(roll_count > 1)
       strcat(roll_string, ", ");
    if(numrolls > 1 && roll_count == numrolls)
         strcat(roll_string, "and ");

    if(die->value[1] == 1)
    {
      char* face_name = get_ed_number(die, current_roll);
      if(face_name)
      {
        char* face_name_copy = strdup(face_name); /* Since I want to tokenize without modifying the original string */
        sprintf(current_roll_string, "%s", strtok(face_name_copy, "\n"));
        free(face_name_copy);
      }
      else
        sprintf(current_roll_string, "%d",  current_roll);
    }
    else
      sprintf(current_roll_string, "%d",  current_roll);
    strcat(roll_string, current_roll_string);
  }

  if(numrolls > 1 && die->value[2] == 1)
  {
    sprintf(total_string, ", de un total de %d", rollsum);
    strcat(roll_string, total_string);
  }

  strcat(roll_string, ".\n\r");

  sprintf(output_string, "Tu %s%s", verb, roll_string);
  act( AT_GREEN, output_string, ch, NULL, NULL, TO_CHAR);

  sprintf(output_string, "$n %s%s", verb, roll_string);
  act( AT_GREEN, output_string, ch, NULL, NULL, TO_ROOM);

  if(face_seen_table)
    free(face_seen_table);
  return;
}

char* get_ed_number(OBJ_DATA* obj, int number)
{
  EXTRA_DESCR_DATA* ed;
  int count;

  for(ed = obj->first_extradesc, count = 1; ed; ed = ed->next, count++)
  {
    if(count == number)
      return ed->description;
  }

  return NULL;
}

char* get_chance_verb(OBJ_DATA* obj)
{
  return (obj->action_desc[0] != '\0') ? obj->action_desc : "roll$q";
}

/*dice chance deal throw*/
