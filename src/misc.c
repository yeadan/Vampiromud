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
 *	    Misc module for general commands: not skills or spells	    *
 ****************************************************************************
 * Note: Most of the stuff in here would go in act_obj.c, but act_obj was   *
 * getting big.								    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


extern int	top_exit;

/*
 * Fill a container
 * Many enhancements added by Thoric (ie: filling non-drink containers)
 */
void do_fill( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *source;
    sh_int    dest_item, src_item1, src_item2, src_item3;
    int       diff = 0;
    bool      all = FALSE;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    /* munch optional words */
    if ( (!str_cmp( arg2, "de" ) || !str_cmp( arg2, "con" ))
    &&    argument[0] != '\0' )
	argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
	send_to_char( "Llenar que?\n\r", ch );
	return;
    }

    if ( ms_find_obj(ch) )
	return;

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == NULL )
    {
	send_to_char( "No tienes eso.\n\r", ch );
	return;
    }
    else
	dest_item = obj->item_type;

    src_item1 = src_item2 = src_item3 = -1;
    switch( dest_item )
    {
	default:
	  act( AT_ACTION, "$n intenta llenar $p... (No me preguntes como)", ch, obj, NULL, TO_ROOM );
	  send_to_char( "No puedes llenar eso.\n\r", ch );
	  return;
	/* place all fillable item types here */
	case ITEM_DRINK_CON:
	  src_item1 = ITEM_FOUNTAIN;	src_item2 = ITEM_BLOOD;		break;
	case ITEM_HERB_CON:
	  src_item1 = ITEM_HERB;	src_item2 = ITEM_HERB_CON;	break;
	case ITEM_PIPE:
	  src_item1 = ITEM_HERB;	src_item2 = ITEM_HERB_CON;	break;
	case ITEM_CONTAINER:
	  src_item1 = ITEM_CONTAINER;	src_item2 = ITEM_CORPSE_NPC;
	  src_item3 = ITEM_CORPSE_PC;	break;
    }

    if ( dest_item == ITEM_CONTAINER )
    {
	if ( IS_SET(obj->value[1], CONT_CLOSED) )
	{
	    act( AT_PLAIN, "El $d esta cerrado.", ch, NULL, obj->name, TO_CHAR );
	    return;
	}
	if ( get_real_obj_weight( obj ) / obj->count
	>=   obj->value[0] )
	{
	   send_to_char( "Esta tan lleno como es posible.\n\r", ch );
	   return;
	}
    }
    else
    {
	diff = obj->value[0] - obj->value[1];
	if ( diff < 1 || obj->value[1] >= obj->value[0] )
	{

	   send_to_char( "Esta tan lleno como es posible.\n\r", ch );

	   return;

	}

    }



    if ( dest_item == ITEM_PIPE

    &&   IS_SET( obj->value[3], PIPE_FULLOFASH ) )

    {

	send_to_char( "Esta lleno de ceniza, necesitas vaciarlo primerlo.\n\r", ch );

	return;

    }



    if ( arg2[0] != '\0' )

    {

      if ( dest_item == ITEM_CONTAINER

      && (!str_cmp( arg2, "todo" ) || !str_prefix( "todo.", arg2 )) )

      {

	all = TRUE;

	source = NULL;

      }

      else

      /* This used to let you fill a pipe from an object on the ground.  Seems

         to me you should be holding whatever you want to fill a pipe with.

         It's nitpicking, but I needed to change it to get a mobprog to work

         right.  Check out Lord Fitzgibbon if you're curious.  -Narn */

      if ( dest_item == ITEM_PIPE )

      {

        if ( ( source = get_obj_carry( ch, arg2 ) ) == NULL )

	{

	   send_to_char( "No tienes eso.\n\r", ch );

	   return;

	}

	if ( source->item_type != src_item1 && source->item_type != src_item2

	&&   source->item_type != src_item3 )

	{

	   act( AT_PLAIN, "No puedes llenar $p con $P!", ch, obj, source, TO_CHAR );

	   return;

	}

      }

      else

      {

	if ( ( source =  get_obj_here( ch, arg2 ) ) == NULL )

	{

	   send_to_char( "No puedes encontrarlo.\n\r", ch );

	   return;

	}

      }

    }

    else

	source = NULL;



    if ( !source && dest_item == ITEM_PIPE )

    {

	send_to_char( "Llenarlo con que?\n\r", ch );

	return;

    }



    if ( !source )

    {

	bool      found = FALSE;

	OBJ_DATA *src_next;



	found = FALSE;

	separate_obj( obj );

	for ( source = ch->in_room->first_content;

	      source;

	      source = src_next )

	{

	    src_next = source->next_content;

	    if (dest_item == ITEM_CONTAINER)

	    {

		if ( !CAN_WEAR(source, ITEM_TAKE)

		||    IS_OBJ_STAT( source, ITEM_BURIED )

		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))

		||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)

		||   (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count)

		    > obj->value[0] )

		  continue;

		if ( all && arg2[3] == '.'

		&&  !nifty_is_name( &arg2[4], source->name ) )

		   continue;

		obj_from_room(source);

		if ( source->item_type == ITEM_MONEY )

		{

		   ch->gold += source->value[0];

		   extract_obj( source );

		}

		else

		   obj_to_obj(source, obj);

		found = TRUE;

	    }

	    else

	    if (source->item_type == src_item1

	    ||  source->item_type == src_item2

	    ||  source->item_type == src_item3)

	    {

		found = TRUE;

		break;

	    }

	}

	if ( !found )

	{

	    switch( src_item1 )

	    {

		default:

		  send_to_char( "No hay nada apropiado aqui!\n\r", ch );

		  return;

		case ITEM_FOUNTAIN:

		  send_to_char( "No hay una fuente por aqui!\n\r", ch );

		  return;

		case ITEM_BLOOD:

		  send_to_char( "No hay sangre con que llenarlo por aqui!\n\r", ch );

		  return;

		case ITEM_HERB_CON:

		  send_to_char( "No hay hierbas por aqui!\n\r", ch );

		  return;

		case ITEM_HERB:

		  send_to_char( "No puedes encontrar ninguna hierba para fumar.\n\r", ch );

		  return;

	    }

	}

	if (dest_item == ITEM_CONTAINER)

	{

	  act( AT_ACTION, "Llenas $p.", ch, obj, NULL, TO_CHAR );

	  act( AT_ACTION, "$n llena $p.", ch, obj, NULL, TO_ROOM );

	  return;

	}

    }



    if (dest_item == ITEM_CONTAINER)

    {

	OBJ_DATA *otmp, *otmp_next;

	char name[MAX_INPUT_LENGTH];

	CHAR_DATA *gch;

	char *pd;

	bool found = FALSE;



	if ( source == obj )

	{

	    send_to_char( "No puedes hacer eso.\n\r", ch );

	    return;

	}



	switch( source->item_type )

	{

	    default:	/* put something in container */

		if ( !source->in_room	/* disallow inventory items */

		||   !CAN_WEAR(source, ITEM_TAKE)

		||   (IS_OBJ_STAT( source, ITEM_PROTOTYPE) && !can_take_proto(ch))

		||    ch->carry_weight + get_obj_weight(source) > can_carry_w(ch)

		||   (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count)

		    > obj->value[0] )

		{

		    send_to_char( "No puedes hacer eso.\n\r", ch );

		    return;

		}

		separate_obj( obj );

		act( AT_ACTION, "Pones $P dentro de $p.", ch, obj, source, TO_CHAR );

		act( AT_ACTION, "$n pone $P dentro de $p.", ch, obj, source, TO_ROOM );

		obj_from_room(source);

		obj_to_obj(source, obj);

		break;

	    case ITEM_MONEY:

		send_to_char( "No puedes hacer esot... todavia.\n\r", ch );

		break;

	    case ITEM_CORPSE_PC:

		if ( IS_NPC(ch) )

		{

		    send_to_char( "No puedes hacer eso.\n\r", ch );

		    return;

		}

                if ( IS_OBJ_STAT( source, ITEM_CLANCORPSE )

                &&  !IS_IMMORTAL( ch ) )

                {

                    send_to_char( "No. Debe poderse hacer de otra forma.\n\r", ch );

                    return;

                }

		if ( !IS_OBJ_STAT( source, ITEM_CLANCORPSE )

		||   !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )

		{

		    pd = source->short_descr;

		    pd = one_argument( pd, name );

		    pd = one_argument( pd, name );

		    pd = one_argument( pd, name );

		    pd = one_argument( pd, name );



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

			    send_to_char( "Eso es el cuerpo de alguien mas.\n\r", ch );

			    return;

			}

		    }

		}

	    case ITEM_CONTAINER:

		if ( source->item_type == ITEM_CONTAINER  /* don't remove */

		&&   IS_SET(source->value[1], CONT_CLOSED) )

		{

		    act( AT_PLAIN, "El $d esta cerrado.", ch, NULL, source->name, TO_CHAR );

		    return;

		}

	    case ITEM_CORPSE_NPC:

		if ( (otmp=source->first_content) == NULL )

		{

		    send_to_char( "Esta vacio.\n\r", ch );

		    return;

		}

		separate_obj( obj );

		for ( ; otmp; otmp = otmp_next )

		{

		    otmp_next = otmp->next_content;



		    if ( !CAN_WEAR(otmp, ITEM_TAKE)

		    ||   (IS_OBJ_STAT( otmp, ITEM_PROTOTYPE) && !can_take_proto(ch))

		    ||    ch->carry_number + otmp->count > can_carry_n(ch)

		    ||    ch->carry_weight + get_obj_weight(otmp) > can_carry_w(ch)

		    ||   (get_real_obj_weight(source) + get_real_obj_weight(obj)/obj->count)

			> obj->value[0] )

			continue;

		    obj_from_obj(otmp);

		    obj_to_obj(otmp, obj);

		    found = TRUE;

		}

		if ( found )

		{

		   act( AT_ACTION, "Llenas $p con $P.", ch, obj, source, TO_CHAR );

		   act( AT_ACTION, "$n llena $p con $P.", ch, obj, source, TO_ROOM );

		}

		else

		   send_to_char( "No hay nada apropiado alli.\n\r", ch );

		break;

	}

	return;

    }



    if ( source->value[1] < 1 )

    {

	send_to_char( "No hay mas!\n\r", ch );

	return;

    }

    if ( source->count > 1 && source->item_type != ITEM_FOUNTAIN )

      separate_obj( source );

    separate_obj( obj );



    switch( source->item_type )

    {

	default:

	  bug( "do_fill: got bad item type: %d", source->item_type );

	  send_to_char( "Algo fue mal...\n\r", ch );

	  return;

	case ITEM_FOUNTAIN:

	  if ( obj->value[1] != 0 && obj->value[2] != 0 )

	  {

	     send_to_char( "Ya esta lleno e otro liquido.\n\r", ch );

	     return;

	  }

	  obj->value[2] = 0;

	  obj->value[1] = obj->value[0];

	  act( AT_ACTION, "Llenas $p con $P.", ch, obj, source, TO_CHAR );

	  act( AT_ACTION, "$n llena $p con $P.", ch, obj, source, TO_ROOM );

	  return;

	case ITEM_BLOOD:

	  if ( obj->value[1] != 0 && obj->value[2] != 13 )

	  {

	     send_to_char( "Ya esta lleno de otro liquido.\n\r", ch );

	     return;

	  }

	  obj->value[2] = 13;

	  if ( source->value[1] < diff )

	    diff = source->value[1];

	  obj->value[1] += diff;

	  act( AT_ACTION, "Llenas $p con $P.", ch, obj, source, TO_CHAR );

	  act( AT_ACTION, "$n llena $p con $P.", ch, obj, source, TO_ROOM );

	  if ( (source->value[1] -= diff) < 1 )

	  {

 	     extract_obj( source );

	     make_bloodstain( ch );

	  }

	  return;

	case ITEM_HERB:

	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )

	  {

	     send_to_char( "Hay otro tipo de hierba dentro.\n\r", ch );

	     return;

	  }

	  obj->value[2] = source->value[2];

	  if ( source->value[1] < diff )

	    diff = source->value[1];

	  obj->value[1] += diff;

	  act( AT_ACTION, "Llenas $p con $P.", ch, obj, source, TO_CHAR );

	  act( AT_ACTION, "$n llena $p con $P.", ch, obj, source, TO_ROOM );

	  if ( (source->value[1] -= diff) < 1 )

 	     extract_obj( source );

	  return;

	case ITEM_HERB_CON:

	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )

	  {

	     send_to_char( "Hay otro tipo de hirba dentro.\n\r", ch );

	     return;

	  }

	  obj->value[2] = source->value[2];

	  if ( source->value[1] < diff )

	    diff = source->value[1];

	  obj->value[1] += diff;

	  source->value[1] -= diff;

	  act( AT_ACTION, "Llenas $p con $P.", ch, obj, source, TO_CHAR );

	  act( AT_ACTION, "$n llena $p con $P.", ch, obj, source, TO_ROOM );

	  return;

	case ITEM_DRINK_CON:

	  if ( obj->value[1] != 0 && obj->value[2] != source->value[2] )

	  {

	     send_to_char( "Ya esta lleno de otro liquido.\n\r", ch );

	     return;

	  }

	  obj->value[2] = source->value[2];

	  if ( source->value[1] < diff )

	    diff = source->value[1];

	  obj->value[1] += diff;

	  source->value[1] -= diff;

	  act( AT_ACTION, "Llenas $p con $P.", ch, obj, source, TO_CHAR );

	  act( AT_ACTION, "$n llena $p con $P.", ch, obj, source, TO_ROOM );

	  return;

    }

}



void do_drink( CHAR_DATA *ch, char *argument )

{

    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    int amount;
    int liquid;

    argument = one_argument( argument, arg );

    /* munch optional words */

    if ( !str_cmp( arg, "from" ) && argument[0] != '\0' )
	argument = one_argument( argument, arg );


    if ( arg[0] == '\0' )
    {
	for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
	    if ( (obj->item_type == ITEM_FOUNTAIN)
	    ||   (obj->item_type == ITEM_BLOOD) )
		break;

	if ( !obj )
	{
	    send_to_char( "Beber que?\n\r", ch );
	    return;
	}
    }
    else
    {
	if ( ( obj = get_obj_here( ch, arg ) ) == NULL )
	{
	    send_to_char( "No puedes encontrarlo.\n\r", ch );
	    return;
	}

    }



    if ( obj->count > 1 && obj->item_type != ITEM_FOUNTAIN )

	separate_obj(obj);



    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 40 )

    {

	send_to_char( "Fayas al llevartelo a la boca. *Hic*\n\r", ch );

	return;

    }



    switch ( obj->item_type )

    {

    default:

	if ( obj->carried_by == ch )

	{

	    act( AT_ACTION, "$n se acerca $p a sus labios e intenta beber...", ch, obj, NULL, TO_ROOM );

	    act( AT_ACTION, "Te acercas $p a tus labios e intentas beber...", ch, obj, NULL, TO_CHAR );

	}

	else
	{
	    act( AT_ACTION, "$n baja hasta el suelo e intenta beber de $p... (Se encuentra bien?)", ch, obj, NULL, TO_ROOM );
	    act( AT_ACTION, "Bajas hasta el suelo e intentas beber de $p...", ch, obj, NULL, TO_CHAR );
	}
	break;



    case ITEM_POTION:
	if ( obj->carried_by == ch )
	   do_quaff( ch, obj->name );
	else
	   send_to_char( "No llevas eso.\n\r", ch );
	break;

    case ITEM_BLOOD:
    	if ( IS_VAMPIRE(ch) && !IS_NPC(ch) )
	{
	    if ( obj->timer > 0		/* if timer, must be spilled blood */
	    &&   ch->level > 5
	    &&   ch->pcdata->condition[COND_BLOODTHIRST] > (5+ch->level/10) )
	    {
		send_to_char( "Esta muy por debajo tuyo beber sangre del suelo!\n\r", ch );
		send_to_char( "Si no estas MUY necesitado, deberias beber sangre del cuello de tus victimas!\n\r", ch );
		return;
	    }
	    if ( ch->pcdata->condition[COND_BLOODTHIRST] < (10 + ch->level)  + (13 / ch->generacion)  * 40)
	    {
		if ( ch->pcdata->condition[COND_FULL] >= 48
		||   ch->pcdata->condition[COND_THIRST] >= 48 )
		{
		    send_to_char( "Estas demasiado lleno para seguir bebiendo sangre.\n\r", ch );
		    return;
		}

		if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
		{
		   act( AT_BLOOD, "$n bebe de la sangre derramada.", ch, NULL, NULL, TO_ROOM );
		   set_char_color( AT_BLOOD, ch );
		   send_to_char( "Con gran satisfaccion bebes este liquido vital...\n\r", ch );
		   if (obj->value[1] <=1)
                   {
			set_char_color( AT_BLOOD, ch );
			send_to_char( "Bebes hasta la ultima gota de la sangre derramada.\n\r", ch);
			act( AT_BLOOD, "$n bebe hasta la ultima gota de la sangre derramada.", ch, NULL, NULL, TO_ROOM );
		   }
		}
		if (ch->level >= LEVEL_AVATAR)
			gain_condition(ch, COND_BLOODTHIRST, 5);
		if (ch->level < LEVEL_AVATAR)
		{
			gain_condition(ch, COND_BLOODTHIRST, 1);
			gain_condition(ch, COND_FULL, 1);
			gain_condition(ch, COND_THIRST, 1);
		}
		if (--obj->value[1] <=0)
		{
		   if ( obj->serial == cur_obj )
		     global_objcode = rOBJ_DRUNK;
		   extract_obj( obj );
		   make_bloodstain( ch );
		}
	    }
	    else
	      send_to_char( "Ay!... no puedes beber mas sangre.\n\r", ch );
	}
	else
	  send_to_char( "No esta en tu naturaleza hacer esas cosas.\n\r", ch );
	break;
        /*Transformacion de Codigo Original SMAUG para definir tipo de objeto BLODDSTAIN*/
  case ITEM_BLOODSTAIN:
  /*
  * Si el jugador ha sido abrazado por un vampiro podra beber kualkier charco de sangre que encuentre
  * lo cual le convertira en vampiro, aqui se define la nueva clase del jugador y sus atributos asi komo disciplinas
  */
if ( xIS_SET(ch->act, PLR_ABRAZADO ) )
{
        do_decision_hereje( ch );                     /* decision hereje convierte al sujeto en cainita las variables ke usara se definen en do_abrazar en vampireroll.c */
        return;
}

if ( IS_VAMPIRE(ch) && !IS_NPC(ch) )
{
if ( ch->pcdata->condition
[COND_BLOODTHIRST] < (10 + ch->level) + (13 / ch->generacion)  * 40 )
{
if ( ch->pcdata->condition[COND_FULL]
>= 48
|| ch->pcdata->condition
[COND_THIRST] >= 48 )
{
send_to_char( "Estas demasiado lleno para beber mas sangre.\n\r", ch );
return;
}

if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )
{
act( AT_BLOOD, "$n bebe del vitae esparcido.", ch, NULL, NULL, TO_ROOM );
set_char_color( AT_BLOOD, ch );
send_to_char( "Te sientes invulnerable ante el poder de este rico vitae...\n\r", ch );
obj->value[1] =1;
set_char_color( AT_BLOOD, ch );
send_to_char( "Bebes hasta la ultima gota de sangre derramada.\n\r", ch);
act( AT_BLOOD, "$n bebe hasta la ultima gota de sangre derramada.", ch, NULL, NULL, TO_ROOM );

}


gain_condition(ch, COND_BLOODTHIRST, 5);
gain_condition(ch, COND_FULL, 5);
gain_condition(ch, COND_THIRST, 5);
if (--obj->value[1] <=0)
{
if ( obj->serial == cur_obj )
global_objcode = rOBJ_DRUNK;
extract_obj( obj );
make_bloodstain( ch );
}
}
else
send_to_char( "Oye!... no puedes consumir mas sangre.\n\r", ch );
}
else
send_to_char( "No esta en tu naturaleza hacer ese tipo de cosas.\n\r", ch );
break;

/*Esto sirve para que al hacer bloodlet no pete la maquina
 * entre otras cosas */

    case ITEM_FOUNTAIN:

	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )

	{

	   act( AT_ACTION, "$n bebe de la fuente.", ch, NULL, NULL, TO_ROOM );

	   send_to_char( "Bebes de la fuente.\n\r", ch );

	}



	if ( !IS_NPC(ch) )

	    ch->pcdata->condition[COND_THIRST] = 40;

	break;



    case ITEM_DRINK_CON:

	if ( obj->value[1] <= 0 )

	{

	    send_to_char( "Ya esta vacio.\n\r", ch );

	    return;

	}



	if ( ( liquid = obj->value[2] ) >= LIQ_MAX )

	{

	    bug( "Do_drink: bad liquid number %d.", liquid );

	    liquid = obj->value[2] = 0;

	}



	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )

	{

	   act( AT_ACTION, "$n bebe $T de $p.",

		ch, obj, liq_table[liquid].liq_name, TO_ROOM );

	   act( AT_ACTION, "Bebes $T de $p.",

		ch, obj, liq_table[liquid].liq_name, TO_CHAR );

	}



	amount = 1; /* UMIN(amount, obj->value[1]); */

	/* what was this? concentrated drinks?  concentrated water

	   too I suppose... sheesh! */



	gain_condition( ch, COND_DRUNK,

	    amount * liq_table[liquid].liq_affect[COND_DRUNK  ] );

	gain_condition( ch, COND_FULL,

	    amount * liq_table[liquid].liq_affect[COND_FULL   ] );

	gain_condition( ch, COND_THIRST,

	    amount * liq_table[liquid].liq_affect[COND_THIRST ] );



	if ( !IS_NPC(ch) )

	{

	    if ( ch->pcdata->condition[COND_DRUNK]  > 24 )

		send_to_char( "Estas como una cuba!.\n\r", ch );

	    else

	    if ( ch->pcdata->condition[COND_DRUNK]  > 18 )

		send_to_char( "Estas muy borracho.\n\r", ch );

	    else

	    if ( ch->pcdata->condition[COND_DRUNK]  > 12 )

		send_to_char( "Estas borracho.\n\r", ch );

	    else

	    if ( ch->pcdata->condition[COND_DRUNK]  > 8 )

		send_to_char( "Te encuentras algo borracho.\n\r", ch );

	    else

	    if ( ch->pcdata->condition[COND_DRUNK]  > 5 )

		send_to_char( "Estas con el puntillo.\n\r", ch );



	    if ( ch->pcdata->condition[COND_FULL]   > 40 )

		send_to_char( "Estas lleno.\n\r", ch );



	    if ( ch->pcdata->condition[COND_THIRST] > 40 )

		send_to_char( "Estas hinchado de liquido.\n\r", ch );

	    else

	    if ( ch->pcdata->condition[COND_THIRST] > 36 )

		send_to_char( "Estas algo lleno de liquido.\n\r", ch );

	    else

	    if ( ch->pcdata->condition[COND_THIRST] > 30 )

		send_to_char( "No sientes sed.\n\r", ch );

	}



	if ( obj->value[3] )
	{
	    /* The drink was poisoned! */
	    AFFECT_DATA af;

	    act( AT_POISON, "$n tiembla y sufre.", ch, NULL, NULL, TO_ROOM );
	    act( AT_POISON, "Tiemblas y sufres.", ch, NULL, NULL, TO_CHAR );
	    ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );
	    af.type      = gsn_poison;
	    af.duration  = 3 * obj->value[3];
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = meb(AFF_POISON);
	    affect_join( ch, &af );

	}



	obj->value[1] -= amount;

	if ( obj->value[1] <= 0 )

	{

	    send_to_char( "El contenedor vacio se desvanece.\n\r", ch );

	    if ( cur_obj == obj->serial )

	      global_objcode = rOBJ_DRUNK;

	    extract_obj( obj );

	}

	break;

    }

    if ( who_fighting( ch ) && IS_PKILL( ch ) )

      WAIT_STATE( ch, PULSE_PER_SECOND/3 );

    else

      WAIT_STATE( ch, PULSE_PER_SECOND );

    return;

}



void do_eat( CHAR_DATA *ch, char *argument )

{

    char buf[MAX_STRING_LENGTH];

    OBJ_DATA *obj;

    ch_ret retcode;

    int foodcond;

    bool hgflag = TRUE;



    if ( argument[0] == '\0' )

    {

	send_to_char( "Comer que?\n\r", ch );

	return;

    }



    if ( IS_NPC(ch) || ch->pcdata->condition[COND_FULL] > 5 )

	if ( ms_find_obj(ch) )

	    return;



    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )

	return;



    if ( !IS_IMMORTAL(ch) )

    {

	if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL &&

	     obj->item_type != ITEM_COOK)

	{

	    act( AT_ACTION, "$n empieza a mordisquear $p... ($e debe tener MUCHA hambre)",  ch, obj, NULL, TO_ROOM );

	    act( AT_ACTION, "Empiezas a mordisquear $p...", ch, obj, NULL, TO_CHAR );

	    return;

	}



	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 )

	{

	    send_to_char( "Estas demasiado lleno para poder comer mas.\n\r", ch );

	    return;

	}

    }



    if ( !IS_NPC(ch) && (!IS_PKILL(ch) || (IS_PKILL(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_HIGHGAG) ) ) )

	hgflag = FALSE;



    /* required due to object grouping */

    separate_obj( obj );

    if ( obj->in_obj )

    {

	if ( !hgflag )

	  act( AT_PLAIN, "Coges $p de $P.", ch, obj, obj->in_obj, TO_CHAR );

	act( AT_PLAIN, "$n coge $p de $P.", ch, obj, obj->in_obj, TO_ROOM );

    }

    if ( ch->fighting && number_percent( ) > (get_curr_dex(ch) * 2 + 47) )

    {

	sprintf( buf, "%s",

	( ch->in_room->sector_type == SECT_UNDERWATER ||

	  ch->in_room->sector_type == SECT_WATER_SWIM ||

	  ch->in_room->sector_type == SECT_WATER_NOSWIM )   ? "se disuelve en el agua" :

	( ch->in_room->sector_type == SECT_AIR        ||

	  IS_SET( ch->in_room->room_flags, ROOM_NOFLOOR ) ) ? "cae al suelo" :

							      "es pisoteado" );

        act( AT_MAGIC, "$n deja caer $p, y $T.", ch, obj, buf, TO_ROOM );

	if ( !hgflag )

	  act( AT_MAGIC, "Oops, $p resbala de tus manos y $T!", ch, obj, buf, TO_CHAR );

    }

    else

    {

        if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )

        {

          if ( !obj->action_desc || obj->action_desc[0]=='\0' )

          {

            act( AT_ACTION, "$n come $p.",  ch, obj, NULL, TO_ROOM );

	    if ( !hgflag )

		act( AT_ACTION, "Comes $p.", ch, obj, NULL, TO_CHAR );

          }

          else

            actiondesc( ch, obj, NULL );

        }



    switch ( obj->item_type )

    {



    case ITEM_COOK:

    case ITEM_FOOD:

	WAIT_STATE( ch, PULSE_PER_SECOND/3 );

	if ( obj->timer > 0 && obj->value[1] > 0 )

	   foodcond = (obj->timer * 10) / obj->value[1];

	else

	   foodcond = 10;



	if ( !IS_NPC(ch) )

	{

	    int condition;



	    condition = ch->pcdata->condition[COND_FULL];

	    gain_condition( ch, COND_FULL, (obj->value[0] * foodcond) / 10 );

	    if ( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )

		send_to_char( "You are no longer hungry.\n\r", ch );

	    else if ( ch->pcdata->condition[COND_FULL] > 40 )

		send_to_char( "You are full.\n\r", ch );

	}



	if (  obj->value[3] != 0

	||   (foodcond < 4 && number_range( 0, foodcond + 1 ) == 0)

	|| (obj->item_type == ITEM_COOK && obj->value[2] == 0 ) )

	{

	    /* The food was poisoned! */

	    AFFECT_DATA af;



	    if ( obj->value[3] != 0 )

	    {

		act( AT_POISON, "$n tiembla y sufre.", ch, NULL, NULL, TO_ROOM );

		act( AT_POISON, "Tiemblas y sufres.", ch, NULL, NULL, TO_CHAR );

		ch->mental_state = URANGE( 20, ch->mental_state + 5, 100 );

	    }

	    else

	    {

		act( AT_POISON, "$n escupe $p.", ch, obj, NULL, TO_ROOM );

		act( AT_POISON, "Escupes $p.", ch, obj, NULL, TO_CHAR );

		ch->mental_state = URANGE( 15, ch->mental_state + 5, 100 );

              }

	    af.type      = gsn_poison;
	    af.duration  = 2 * obj->value[0]
	    		 * (obj->value[3] > 0 ? obj->value[3] : 1);
	    af.location  = APPLY_NONE;
	    af.modifier  = 0;
	    af.bitvector = meb(AFF_POISON);
	    affect_join( ch, &af );
	}

	break;



    case ITEM_PILL:

	sysdata.upill_val += obj->cost/100;

	if ( who_fighting( ch ) && IS_PKILL( ch ) )

          WAIT_STATE( ch, PULSE_PER_SECOND/4 );

        else

          WAIT_STATE( ch, PULSE_PER_SECOND/3 );

	/* allow pills to fill you, if so desired */

	if ( !IS_NPC(ch) && obj->value[4] )

	{

	    int condition;



	    condition = ch->pcdata->condition[COND_FULL];

	    gain_condition( ch, COND_FULL, obj->value[4] );

	    if ( condition <= 1 && ch->pcdata->condition[COND_FULL] > 1 )

		send_to_char( "Ya no tienes hambre.\n\r", ch );

	    else if ( ch->pcdata->condition[COND_FULL] > 40 )

		send_to_char( "Estas lleno.\n\r", ch );

	}

	retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );

	if ( retcode == rNONE )

	  retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );

	if ( retcode == rNONE )

	  retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

	break;

    }



    }

    if ( obj->serial == cur_obj )

      global_objcode = rOBJ_EATEN;

    extract_obj( obj );

    return;

}



void do_quaff( CHAR_DATA *ch, char *argument )

{

    OBJ_DATA *obj;

    ch_ret retcode;

    bool hgflag = TRUE;



    if ( argument[0] == '\0' || !str_cmp(argument, "") )

    {

	send_to_char( "Tomar que?\n\r", ch );

	return;

    }



    if ( (obj = find_obj(ch, argument, TRUE)) == NULL )

	return;



    if ( !IS_NPC(ch) && IS_AFFECTED( ch, AFF_CHARM) || xIS_SET(ch->act, PLR_DOMINADO))
    	return;



    if ( obj->item_type != ITEM_POTION )

    {

	if ( obj->item_type == ITEM_DRINK_CON )

	   do_drink( ch, obj->name );

	else

	{

	   act( AT_ACTION, "$n se lleva $p a sus labios e intenta beberselo...", ch, obj, NULL, TO_ROOM );

	   act( AT_ACTION, "Te llevas $p a tus labios e intentas bebertelo...", ch, obj, NULL, TO_CHAR );

	}

	return;

    }



    /*

     * Empty container check 					-Shaddai

     */

     if ( obj->value[1] == -1 && obj->value[2] == -1 && obj->value[3] == -1 )

     {

       send_to_char( "Intentas beber pero esta vacio.\n\r", ch );

       return;

     }

    /*

     * Fullness checking					-Thoric

     */

    if ( !IS_NPC(ch)

    && ( ch->pcdata->condition[COND_FULL] >= 48

    ||   ch->pcdata->condition[COND_THIRST] >= 48 ) )

    {

	send_to_char( "Tu estomago no puede contener mas.\n\r", ch );

	return;

    }



    /* People with nuisance flag feels up quicker. -- Shaddai */

    /* Yeah so I can't spell I'm a coder :P --Shaddai */

    /* You are now adept at feeling up quickly! -- Blod */

    if ( !IS_NPC(ch) && ch->pcdata->nuisance &&

		ch->pcdata->nuisance->flags > 3

     &&(ch->pcdata->condition[COND_FULL]>=(48-(3*ch->pcdata->nuisance->flags)+

	ch->pcdata->nuisance->power)

     ||ch->pcdata->condition[COND_THIRST]>=(48-(ch->pcdata->nuisance->flags)+

	ch->pcdata->nuisance->power)))

    {

        send_to_char( "Tu estomago no puede contener mas.\n\r", ch );

        return;

    }



    if ( !IS_NPC(ch) && (!IS_PKILL(ch) || (IS_PKILL(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_HIGHGAG) ) ) )

	hgflag = FALSE;



    separate_obj( obj );

    if ( obj->in_obj )

    {

      if ( !CAN_PKILL( ch ) )

      {

        act( AT_PLAIN, "Coges $p de $P.", ch, obj, obj->in_obj, TO_CHAR );

	act( AT_PLAIN, "$n coge $p de $P.", ch, obj, obj->in_obj, TO_ROOM );

      }

    }



    /*

     * If fighting, chance of dropping potion			-Thoric

     */

    if ( ch->fighting && number_percent( ) > (get_curr_dex(ch) * 2 + 48) )

    {

	act( AT_MAGIC, "A $n se le cae $p de las manos y se rompe en mil pedazos.", ch, obj, NULL, TO_ROOM );

	if ( !hgflag )

	  act( AT_MAGIC, "Oops... $p se te ha caido de las manos y se ha roto!", ch, obj, NULL ,TO_CHAR );

    }

    else

    {

	if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )

	{

          if ( !CAN_PKILL( ch ) || !obj->in_obj )

          {

	    act( AT_ACTION, "$n toma $p.",  ch, obj, NULL, TO_ROOM );

	    if ( !hgflag )

	      act( AT_ACTION, "Tomas $p.", ch, obj, NULL, TO_CHAR );

          }

          else if ( obj->in_obj )

          {

            act( AT_ACTION, "$n toma $p de $P.", ch, obj, obj->in_obj, TO_ROOM );

	    if ( !hgflag )

	      act( AT_ACTION, "You tomas $p de $P.", ch, obj, obj->in_obj, TO_CHAR );

          }

	}



        if ( who_fighting( ch ) && IS_PKILL( ch ) )

          WAIT_STATE( ch, PULSE_PER_SECOND/5 );

        else

          WAIT_STATE( ch, PULSE_PER_SECOND/3 );



	gain_condition( ch, COND_THIRST, 1 );

	if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 43 )

	  act( AT_ACTION, "Tu estomago se esta acercando al limite de su capacidad.", ch, NULL, NULL, TO_CHAR );

	retcode = obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );

	if ( retcode == rNONE )

	  retcode = obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );

	if ( retcode == rNONE )

	  retcode = obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );

    }

    if ( obj->pIndexData->vnum == OBJ_VNUM_FLASK_BREWING )

	sysdata.brewed_used++;

    else

	sysdata.upotion_val += obj->cost/100;

    if ( cur_obj == obj->serial )

      global_objcode = rOBJ_QUAFFED;

    extract_obj( obj );

    return;

}





void do_recite( CHAR_DATA *ch, char *argument )

{

    char arg1[MAX_INPUT_LENGTH];

    char arg2[MAX_INPUT_LENGTH];

    CHAR_DATA *victim;

    OBJ_DATA *scroll;

    OBJ_DATA *obj;

    ch_ret    retcode;



    argument = one_argument( argument, arg1 );

    argument = one_argument( argument, arg2 );



    if ( arg1[0] == '\0' )

    {

	send_to_char( "Recitar que?\n\r", ch );

	return;

    }



    if ( ms_find_obj(ch) )

	return;



    if ( ( scroll = get_obj_carry( ch, arg1 ) ) == NULL )

    {

	send_to_char( "No tienes ese pergamino.\n\r", ch );

	return;

    }



    if ( scroll->item_type != ITEM_SCROLL )

    {

	act( AT_ACTION, "$n sostiene $p como si fuese a recitar algo de eso...",  ch, scroll, NULL, TO_ROOM );

	act( AT_ACTION, "Sostienes $p y te quedas parado con la boca abierta. (Y ahora que?)", ch, scroll, NULL, TO_CHAR );

	return;

    }



    if ( IS_NPC(ch)

    && (scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING) )

    {

	send_to_char( "No entiendes este dialecto.\n\r", ch );

	return;

    }



    if( ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING)

      &&(ch->level + 10 < scroll->value[0]))

    {

        send_to_char( "Este pergamino es demasiado complicado para que lo puedas entender.\n\r", ch);

        return;

    }



    obj = NULL;

    if ( arg2[0] == '\0' )

	victim = ch;

    else

    {

	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL

	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )

	{

	    send_to_char( "No puedes encontrar eso.\n\r", ch );

	    return;

	}

    }



    if ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING )

	sysdata.scribed_used++;

    separate_obj( scroll );

    act( AT_MAGIC, "$n recita $p.", ch, scroll, NULL, TO_ROOM );

    act( AT_MAGIC, "Recitas $p.", ch, scroll, NULL, TO_CHAR );



    if ( victim != ch )

	WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

    else

        WAIT_STATE( ch, PULSE_PER_SECOND/2 );



    retcode = obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );

    if ( retcode == rNONE )

      retcode = obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );

    if ( retcode == rNONE )

      retcode = obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );



    if ( scroll->serial == cur_obj )

      global_objcode = rOBJ_USED;

    extract_obj( scroll );

    return;

}





/*

 * Function to handle the state changing of a triggerobject (lever)  -Thoric

 */

void pullorpush( CHAR_DATA *ch, OBJ_DATA *obj, bool pull )

{

    char buf[MAX_STRING_LENGTH];

    CHAR_DATA		*rch;

    bool		 isup;

    ROOM_INDEX_DATA	*room,  *to_room;

    EXIT_DATA		*pexit, *pexit_rev;

    int			 edir;

    char		*txt;



    if ( IS_SET( obj->value[0], TRIG_UP ) )

      isup = TRUE;

    else

      isup = FALSE;

    switch( obj->item_type )

    {

	default:

	  sprintf( buf, "No puedes %s eso!\n\r", pull ? "estirar" : "empujar" );

	  send_to_char( buf, ch );

	  return;

	  break;

	case ITEM_SWITCH:

	case ITEM_LEVER:

	case ITEM_PULLCHAIN:

	  if ( (!pull && isup) || (pull && !isup) )

	  {

		sprintf( buf, "Ya esta %s.\n\r", isup ? "arriba" : "abajo" );

		send_to_char( buf, ch );

		return;

 	  }

	case ITEM_BUTTON:

	  if ( (!pull && isup) || (pull && !isup) )

	  {

		sprintf( buf, "Ya esta %s.\n\r", isup ? "pulsado" : "sin pulsar" );

		send_to_char( buf, ch );

		return;

	  }

	  break;

    }

    if( (pull) && HAS_PROG(obj->pIndexData,PULL_PROG) )

    {

	if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )

 	  REMOVE_BIT( obj->value[0], TRIG_UP );

 	oprog_pull_trigger( ch, obj );

        return;

    }

    if( (!pull) && HAS_PROG(obj->pIndexData,PUSH_PROG) )

    {

	if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )

	  SET_BIT( obj->value[0], TRIG_UP );

	oprog_push_trigger( ch, obj );

        return;

    }



    if ( !oprog_use_trigger( ch, obj, NULL, NULL, NULL ) )

    {

      sprintf( buf, "$n %s $p.", pull ? "estira" : "empuja" );

      act( AT_ACTION, buf,  ch, obj, NULL, TO_ROOM );

      sprintf( buf, "%s $p.", pull ? "Estiras" : "Empujas" );

      act( AT_ACTION, buf, ch, obj, NULL, TO_CHAR );

    }



    if ( !IS_SET(obj->value[0], TRIG_AUTORETURN ) )

    {

	if ( pull )

	  REMOVE_BIT( obj->value[0], TRIG_UP );

	else

	  SET_BIT( obj->value[0], TRIG_UP );

    }

    if ( IS_SET( obj->value[0], TRIG_TELEPORT )

    ||   IS_SET( obj->value[0], TRIG_TELEPORTALL )

    ||   IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )

    {

	int flags;



	if ( ( room = get_room_index( obj->value[1] ) ) == NULL )

	{

	    bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );

	    return;

	}

	flags = 0;

	if ( IS_SET( obj->value[0], TRIG_SHOWROOMDESC ) )

	  SET_BIT( flags, TELE_SHOWDESC );

	if ( IS_SET( obj->value[0], TRIG_TELEPORTALL ) )

	  SET_BIT( flags, TELE_TRANSALL );

	if ( IS_SET( obj->value[0], TRIG_TELEPORTPLUS ) )

	  SET_BIT( flags, TELE_TRANSALLPLUS );



	teleport( ch, obj->value[1], flags );

	return;

    }



    if ( IS_SET( obj->value[0], TRIG_RAND4 )

    ||	 IS_SET( obj->value[0], TRIG_RAND6 ) )

    {

	int maxd;



	if ( ( room = get_room_index( obj->value[1] ) ) == NULL )

	{

	    bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );

	    return;

	}



	if ( IS_SET( obj->value[0], TRIG_RAND4 ) )

	  maxd = 3;

	else

	  maxd = 5;



	randomize_exits( room, maxd );

	for ( rch = room->first_person; rch; rch = rch->next_in_room )

	{

	   send_to_char( "Oyes un extranyo sonido.\n\r", rch );

	   send_to_char( "Parece que algo ha cambiado...\n\r", rch );

	}

    }

    if ( IS_SET( obj->value[0], TRIG_DOOR ) )

    {

	room = get_room_index( obj->value[1] );

	if ( !room )

	  room = obj->in_room;

	if ( !room )

	{

	  bug( "PullOrPush: obj points to invalid room %d", obj->value[1] );

	  return;

	}

	if ( IS_SET( obj->value[0], TRIG_D_NORTH ) )

	{

	  edir = DIR_NORTH;

	  txt = "al norte";

	}

	else

	if ( IS_SET( obj->value[0], TRIG_D_SOUTH ) )

	{

	  edir = DIR_SOUTH;

	  txt = "al sur";

	}

	else

	if ( IS_SET( obj->value[0], TRIG_D_EAST ) )

	{

	  edir = DIR_EAST;

	  txt = "al este";

	}

	else

	if ( IS_SET( obj->value[0], TRIG_D_WEST ) )

	{

	  edir = DIR_WEST;

	  txt = "al oeste";

	}

	else

	if ( IS_SET( obj->value[0], TRIG_D_UP ) )

	{

	  edir = DIR_UP;

	  txt = "desde arriba";

	}

	else

	if ( IS_SET( obj->value[0], TRIG_D_DOWN ) )

	{

	  edir = DIR_DOWN;

	  txt = "desde abajo";

	}

	else

	{

	  bug( "PullOrPush: door: no direction flag set.", 0 );

	  return;

	}

	pexit = get_exit( room, edir );

	if ( !pexit )

	{

	    if ( !IS_SET( obj->value[0], TRIG_PASSAGE ) )

	    {

		bug( "PullOrPush: obj points to non-exit %d", obj->value[1] );

		return;

	    }

	    to_room = get_room_index( obj->value[2] );

	    if ( !to_room )

	    {

		bug( "PullOrPush: dest points to invalid room %d", obj->value[2] );

		return;

	    }

	    pexit = make_exit( room, to_room, edir );

	    pexit->keyword	= STRALLOC( "" );

	    pexit->description	= STRALLOC( "" );

	    pexit->key		= -1;

	    pexit->exit_info	= 0;

	    top_exit++;

	    act( AT_PLAIN, "Se abre una puerta secreta!", ch, NULL, NULL, TO_CHAR );

	    act( AT_PLAIN, "Se abre una puerta secreta!", ch, NULL, NULL, TO_ROOM );

	    return;

	}

	if ( IS_SET( obj->value[0], TRIG_UNLOCK )

	&&   IS_SET( pexit->exit_info, EX_LOCKED) )

	{

	    REMOVE_BIT(pexit->exit_info, EX_LOCKED);

	    act( AT_PLAIN, "Oyes un click $T.", ch, NULL, txt, TO_CHAR );

	    act( AT_PLAIN, "Oyes un click $T.", ch, NULL, txt, TO_ROOM );

	    if ( ( pexit_rev = pexit->rexit ) != NULL

	    &&   pexit_rev->to_room == ch->in_room )

		REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );

	    return;

	}

	if ( IS_SET( obj->value[0], TRIG_LOCK   )

	&&  !IS_SET( pexit->exit_info, EX_LOCKED) )

	{

	    SET_BIT(pexit->exit_info, EX_LOCKED);

	    act( AT_PLAIN, "Oyes un click $T.", ch, NULL, txt, TO_CHAR );

	    act( AT_PLAIN, "Oyes un click $T.", ch, NULL, txt, TO_ROOM );

	    if ( ( pexit_rev = pexit->rexit ) != NULL

	    &&   pexit_rev->to_room == ch->in_room )

		SET_BIT( pexit_rev->exit_info, EX_LOCKED );

	    return;

	}

	if ( IS_SET( obj->value[0], TRIG_OPEN   )

	&&   IS_SET( pexit->exit_info, EX_CLOSED) )

	{

	    REMOVE_BIT(pexit->exit_info, EX_CLOSED);

	    for ( rch = room->first_person; rch; rch = rch->next_in_room )

		act( AT_ACTION, "El $d se abre.", rch, NULL, pexit->keyword, TO_CHAR );

	    if ( ( pexit_rev = pexit->rexit ) != NULL

	    &&   pexit_rev->to_room == ch->in_room )

	    {

		REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );

		/* bug here pointed out by Nick Gammon */

		for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )

		    act( AT_ACTION, "El $d se abre.", rch, NULL, pexit_rev->keyword, TO_CHAR );

	    }

	    check_room_for_traps( ch, trap_door[edir]);

	    return;

	}

	if ( IS_SET( obj->value[0], TRIG_CLOSE   )

	&&  !IS_SET( pexit->exit_info, EX_CLOSED) )

	{

	    SET_BIT(pexit->exit_info, EX_CLOSED);

	    for ( rch = room->first_person; rch; rch = rch->next_in_room )

		act( AT_ACTION, "El $d se cierra.", rch, NULL, pexit->keyword, TO_CHAR );

	    if ( ( pexit_rev = pexit->rexit ) != NULL

	    &&   pexit_rev->to_room == ch->in_room )

	    {

		SET_BIT( pexit_rev->exit_info, EX_CLOSED );

		/* bug here pointed out by Nick Gammon */

		for ( rch = pexit->to_room->first_person; rch; rch = rch->next_in_room )

		    act( AT_ACTION, "El $d se cierra.", rch, NULL, pexit_rev->keyword, TO_CHAR );

	    }

	    check_room_for_traps( ch, trap_door[edir]);

	    return;

	}

    }

}





void do_pull( CHAR_DATA *ch, char *argument )

{

    char arg[MAX_INPUT_LENGTH];

    OBJ_DATA *obj;



    one_argument( argument, arg );

    if ( arg[0] == '\0' )

    {

	send_to_char( "Estirar que?\n\r", ch );

	return;

    }



    if ( ms_find_obj(ch) )

	return;



    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )

    {

	act( AT_PLAIN, "No veo $T aqui.", ch, NULL, arg, TO_CHAR );

	return;

    }



    pullorpush( ch, obj, TRUE );

}



void do_push( CHAR_DATA *ch, char *argument )

{

    char arg[MAX_INPUT_LENGTH];

    OBJ_DATA *obj;



    one_argument( argument, arg );

    if ( arg[0] == '\0' )

    {

	send_to_char( "Empujar que?\n\r", ch );

	return;

    }



    if ( ms_find_obj(ch) )

	return;



    if ( ( obj = get_obj_here( ch, arg ) ) == NULL )

    {

	act( AT_PLAIN, "No veo $T aqui.", ch, NULL, arg, TO_CHAR );

	return;

    }



    pullorpush( ch, obj, FALSE );

}



void do_rap( CHAR_DATA *ch, char *argument )

{

        EXIT_DATA *pexit;

        char       arg [ MAX_INPUT_LENGTH ];



        one_argument( argument, arg );



        if ( arg[0] == '\0' )

        {

          send_to_char( "Golpear que?\n\r", ch );

          return;

        }

        if ( ch->fighting )

        {

          send_to_char( "Tienes mejores cosas que hacer con las manos ahora mismo.\n\r", ch );

          return;

        }

        if ( ( pexit = find_door( ch, arg, FALSE ) ) != NULL )

        {

          ROOM_INDEX_DATA *to_room;

          EXIT_DATA       *pexit_rev;

          char            *keyword;

          if ( !IS_SET( pexit->exit_info, EX_CLOSED ) )

          {

            send_to_char( "Poruqe llamar? Esta abierta.\n\r", ch );

            return;

          }

          if ( IS_SET( pexit->exit_info, EX_SECRET ) )

            keyword = "muro";

          else

            keyword = pexit->keyword;

          act( AT_ACTION, "Golpeas el $d.", ch, NULL, keyword, TO_CHAR );

          act( AT_ACTION, "$n golpea el $d.", ch, NULL, keyword, TO_ROOM );

          if ( (to_room = pexit->to_room) != NULL

          &&   (pexit_rev = pexit->rexit) != NULL

          &&    pexit_rev->to_room        == ch->in_room )

          {

            CHAR_DATA *rch;

            for ( rch = to_room->first_person; rch; rch = rch->next_in_room )

            {

              act( AT_ACTION, "Alguien golpea sonoramente desde el otro lado del muro $d.",

                rch, NULL, pexit_rev->keyword, TO_CHAR );

            }

          }

        }

        else

        {

          act( AT_ACTION, "Haces como si estuvieras llamando a traves del aire.",

	    ch, NULL, NULL, TO_CHAR );

          act( AT_ACTION, "$n hace como si llamara a traves del aire.",

	    ch, NULL, NULL, TO_ROOM );

        }

        return;

}



/* pipe commands (light, tamp, smoke) by Thoric */

void do_tamp( CHAR_DATA *ch, char *argument )

{

    OBJ_DATA *pipe;

    char arg[MAX_INPUT_LENGTH];



    one_argument( argument, arg );

    if ( arg[0] == '\0' )

    {

	send_to_char( "Colocar el que?\n\r", ch );

	return;

    }



    if ( ms_find_obj(ch) )

	return;



    if ( (pipe = get_obj_carry( ch, arg )) == NULL )

    {

	send_to_char( "No estas llevando eso.\n\r", ch );

	return;

    }

    if ( pipe->item_type != ITEM_PIPE )

    {

	send_to_char( "No puedes colocar eso en una pipa.\n\r", ch );

	return;

    }

    if ( !IS_SET( pipe->value[3], PIPE_TAMPED ) )

    {

	act( AT_ACTION, "Colocas $p cuidadosamente en una pipa.", ch, pipe, NULL, TO_CHAR );

	act( AT_ACTION, "$n coloca cuidadosamente $p en una pipa.", ch, pipe, NULL, TO_ROOM );

	SET_BIT( pipe->value[3], PIPE_TAMPED );

	return;

    }

    send_to_char( "No necesita ser colocado.\n\r", ch );

}



void do_smoke( CHAR_DATA *ch, char *argument )

{

    OBJ_DATA *pipe;

    char arg[MAX_INPUT_LENGTH];



    one_argument( argument, arg );

    if ( arg[0] == '\0' )

    {

	send_to_char( "Fumar que?\n\r", ch );

	return;

    }



    if ( ms_find_obj(ch) )

	return;



    if ( (pipe = get_obj_carry( ch, arg )) == NULL )

    {

	send_to_char( "No estas llevando eso.\n\r", ch );

	return;

    }

    if ( pipe->item_type != ITEM_PIPE )

    {

	act( AT_ACTION, "Intentas fumarte $p... pero parece que no funciona.", ch, pipe, NULL, TO_CHAR );

	act( AT_ACTION, "$n intenta fumarse $p...", ch, pipe, NULL, TO_ROOM );

	return;

    }

    if ( !IS_SET( pipe->value[3], PIPE_LIT ) )

    {

	act( AT_ACTION, "Intentas fumarte $p, pero no esta encendido.", ch, pipe, NULL, TO_CHAR );

	act( AT_ACTION, "$n intenta fumar $p, pero no esta encendido.", ch, pipe, NULL, TO_ROOM );

	return;

    }

    if ( pipe->value[1] > 0 )

    {

	if ( !oprog_use_trigger( ch, pipe, NULL, NULL, NULL ) )

	{

	   act( AT_ACTION, "LE das una calada a $p.", ch, pipe, NULL, TO_CHAR );

	   act( AT_ACTION, "$n le da una calada a $p.", ch, pipe, NULL, TO_ROOM );

	}



	if ( IS_VALID_HERB( pipe->value[2] ) && pipe->value[2] < top_herb )

	{

	    int sn		= pipe->value[2] + TYPE_HERB;

	    SKILLTYPE *skill	= get_skilltype( sn );



       WAIT_STATE( ch, skill->beats * ( ch->generacion / 13 ) );

	    if ( skill->spell_fun )

		obj_cast_spell( sn, UMIN(skill->min_level, ch->level),

			ch, ch, NULL );

	    if ( obj_extracted( pipe ) )

		return;

	}

	else

	    bug( "do_smoke: bad herb type %d", pipe->value[2] );



	SET_BIT( pipe->value[3], PIPE_HOT );

	if ( --pipe->value[1] < 1 )

	{

	   REMOVE_BIT( pipe->value[3], PIPE_LIT );

	   SET_BIT( pipe->value[3], PIPE_DIRTY );

	   SET_BIT( pipe->value[3], PIPE_FULLOFASH );

	}

    }

}



void do_light( CHAR_DATA *ch, char *argument )

{

    OBJ_DATA *pipe;

    char arg[MAX_INPUT_LENGTH];



    one_argument( argument, arg );

    if ( arg[0] == '\0' )

    {

	send_to_char( "Encender que?\n\r", ch );

	return;

    }



    if ( ms_find_obj(ch) )

	return;



    if ( (pipe = get_obj_carry( ch, arg )) == NULL )

    {

	send_to_char( "No tienes eso.\n\r", ch );

	return;

    }

    if ( pipe->item_type != ITEM_PIPE )

    {

	send_to_char( "No puedes encender eso.\n\r", ch );

	return;

    }

    if ( !IS_SET( pipe->value[3], PIPE_LIT ) )

    {

	if ( pipe->value[1] < 1 )

	{

	  act( AT_ACTION, "Intentas encender $p, pero esta vacio.", ch, pipe, NULL, TO_CHAR );

	  act( AT_ACTION, "$n intenta encender $p, pero esta vacio", ch, pipe, NULL, TO_ROOM );

	  return;

	}

	act( AT_ACTION, "Enciendes cuidadosamente $p.", ch, pipe, NULL, TO_CHAR );

	act( AT_ACTION, "$n enciende cuidadosamente $p.", ch, pipe, NULL, TO_ROOM );

	SET_BIT( pipe->value[3], PIPE_LIT );

	return;

    }

    send_to_char( "Ya esta encendido.\n\r", ch );

}



void do_empty( CHAR_DATA *ch, char *argument )

{

    OBJ_DATA *obj;

    char arg1[MAX_INPUT_LENGTH];

    char arg2[MAX_INPUT_LENGTH];



    argument = one_argument( argument, arg1 );

    argument = one_argument( argument, arg2 );

    if ( !str_cmp( arg2, "into" ) && argument[0] != '\0' )

	argument = one_argument( argument, arg2 );



    if ( arg1[0] == '\0' )

    {

	send_to_char( "Vaciar que?\n\r", ch );

	return;

    }

    if ( ms_find_obj(ch) )

	return;



    if ( (obj = get_obj_carry( ch, arg1 )) == NULL )

    {

	send_to_char( "No llevas eso.\n\r", ch );

	return;

    }

    if ( obj->count > 1 )

      separate_obj(obj);



    switch( obj->item_type )

    {

	default:

	  act( AT_ACTION, "Sacudes $p en un intento por vaciarlo...", ch, obj, NULL, TO_CHAR );

	  act( AT_ACTION, "$n empieza a sacudir $p en un intento por vaciarlo...", ch, obj, NULL, TO_ROOM );

	  return;

	case ITEM_PIPE:

	  act( AT_ACTION, "Cuidadosamente giras $p y empiezas a vaciarlo.", ch, obj, NULL, TO_CHAR );

	  act( AT_ACTION, "$n cuidadosamente gira $p y empieza a vaciarlo.", ch, obj, NULL, TO_ROOM );

	  REMOVE_BIT( obj->value[3], PIPE_FULLOFASH );

	  REMOVE_BIT( obj->value[3], PIPE_LIT );

	  obj->value[1] = 0;

	  return;

	case ITEM_DRINK_CON:

	  if ( obj->value[1] < 1 )

	  {

		send_to_char( "Ya esta vacio.\n\r", ch );

		return;

	  }

	  act( AT_ACTION, "Vacias $p.", ch, obj, NULL, TO_CHAR );

	  act( AT_ACTION, "$n vacia $p.", ch, obj, NULL, TO_ROOM );

	  obj->value[1] = 0;

	  return;

	case ITEM_CONTAINER:

	case ITEM_QUIVER:

	  if ( IS_SET(obj->value[1], CONT_CLOSED) )

	  {

		act( AT_PLAIN, "El $d esta cerrado.", ch, NULL, obj->name, TO_CHAR );

		return;

	  }

	case ITEM_KEYRING:

	  if ( !obj->first_content )

	  {

		send_to_char( "Ya esta vacio.\n\r", ch );

		return;

	  }

	  if ( arg2[0] == '\0' )

	  {

		if ( IS_SET( ch->in_room->room_flags, ROOM_NODROP )

		||  xIS_SET( ch->act, PLR_LITTERBUG ) )

		{

		       set_char_color( AT_MAGIC, ch );

		       send_to_char( "Una fuerza magica te detiene!\n\r", ch );

		       set_char_color( AT_TELL, ch );

		       send_to_char( "Alguien te dice telepaticamente, 'No tires basuras aqui!'\n\r", ch );

		       return;

		}

		if ( IS_SET( ch->in_room->room_flags, ROOM_NODROPALL )

		||   IS_SET( ch->in_room->room_flags, ROOM_CLANSTOREROOM ) )

		{

		   send_to_char( "No parece que puedas hacerlo...\n\r", ch );

		   return;

		}

		if ( empty_obj( obj, NULL, ch->in_room ) )

		{

		    act( AT_ACTION, "Vacias $p.", ch, obj, NULL, TO_CHAR );

		    act( AT_ACTION, "$n vacia $p.", ch, obj, NULL, TO_ROOM );

		    if ( IS_SET( sysdata.save_flags, SV_EMPTY ) )

			save_char_obj( ch );

		}

		else

		    send_to_char( "Mmmm... no funciona.\n\r", ch );

	  }

	  else

	  {

		OBJ_DATA *dest = get_obj_here( ch, arg2 );



		if ( !dest )

		{

		    send_to_char( "No puedes encontrar eso.\n\r", ch );

		    return;

		}

		if ( dest == obj )

		{

		    send_to_char( "No puedes vaciar algo en si mismo!\n\r", ch );

		    return;

		}

		if ( dest->item_type != ITEM_CONTAINER && dest->item_type != ITEM_KEYRING

		&&   dest->item_type != ITEM_QUIVER )

		{

		    send_to_char( "Eso no es un contenedor!\n\r", ch );

		    return;

		}

		if ( IS_SET(dest->value[1], CONT_CLOSED) )

		{

		    act( AT_PLAIN, "El $d esta cerrado.", ch, NULL, dest->name, TO_CHAR );

		    return;

		}

		separate_obj( dest );

		if ( empty_obj( obj, dest, NULL ) )

		{

		    act( AT_ACTION, "Vacias $p en $P.", ch, obj, dest, TO_CHAR );

		    act( AT_ACTION, "$n vacia $p en $P.", ch, obj, dest, TO_ROOM );

		    if ( !dest->carried_by

		    &&    IS_SET( sysdata.save_flags, SV_EMPTY ) )

			save_char_obj( ch );

		}

		else

		    act( AT_ACTION, "$P esta demasiado lleno.", ch, obj, dest, TO_CHAR );

	  }

	  return;

    }

}



/*

 * Apply a salve/ointment					-Thoric

 * Support for applying to others.  Pkill concerns dealt with elsewhere.

 */

void do_apply( CHAR_DATA *ch, char *argument )

{



    char arg1[MAX_INPUT_LENGTH];

    char arg2[MAX_INPUT_LENGTH];

    CHAR_DATA *victim;

    OBJ_DATA *salve;

    OBJ_DATA *obj;

    ch_ret    retcode;



    argument = one_argument( argument, arg1 );

    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )

    {

	send_to_char( "Aplicar que?\n\r", ch );

	return;

    }

    if ( ch->fighting )

    {

	send_to_char( "Estas demasiado ocupado luchando...\n\r", ch );

	return;

    }

    if ( ms_find_obj(ch) )

	return;

    if ( ( salve = get_obj_carry( ch, arg1 ) ) == NULL )

    {

	send_to_char( "No tienes eso.\n\r", ch );

	return;

    }



    obj = NULL;

    if ( arg2[0] == '\0' )

	victim = ch;

    else

    {

	if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL

	&&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL )

	{

	    send_to_char( "Aplicarlo a que o a quien\n\r", ch );

	    return;

	}

   }



   /* apply salve to another object */

   if ( obj )

   {

	send_to_char( "No puedes hacer eso... todavia.\n\r", ch );

	return;

   }



   if ( victim->fighting )

   {

	send_to_char( "Funcionaria mejor si no estuviera luchando ...\n\r", ch );

	return;

   }



   if ( salve->item_type != ITEM_SALVE )

   {

	if ( victim == ch )

	{

	    act( AT_ACTION, "$n empieza a frotar $p con $m mismo...", ch, salve, NULL, TO_ROOM );

            act( AT_ACTION, "Empiezas a frotar $p contigo mismo...", ch, salve, NULL, TO_CHAR );

	}

	else

	{

	    act( AT_ACTION, "$n empieza a frotar $p sobre $N...", ch, salve, victim, TO_NOTVICT );

	    act( AT_ACTION, "$n empieza a frotar $p sobre ti...", ch, salve, victim, TO_VICT );

	    act( AT_ACTION, "Empiezas a frotar $p sobre $N...", ch, salve, victim, TO_CHAR );

	}

        return;

    }

    separate_obj( salve );

    --salve->value[1];



    if ( !oprog_use_trigger( ch, salve, NULL, NULL, NULL ) )

    {

	if ( !salve->action_desc || salve->action_desc[0]=='\0' )

	{

	    if ( salve->value[1] < 1 )

	    {

		if ( victim != ch )

		{

		    act( AT_ACTION, "$n frota el ultimo $p sobre $N.",

			ch, salve, victim, TO_NOTVICT );

		    act( AT_ACTION, "$n frota el ultimo $p sobre ti.",

			ch, salve, victim, TO_VICT );

		    act( AT_ACTION, "Frotas el ultimo$p sobre $N.",

			ch, salve, victim, TO_CHAR );

		}

		else

		{

		    act( AT_ACTION, "Frotas el ultimo $p sobre ti mismo.",

			ch, salve, NULL, TO_CHAR );

		    act( AT_ACTION, "$n frota el ultimo $p sobre $m mismo.",

			ch, salve, NULL, TO_ROOM );

		}

	    }

	    else

	    {

		if ( victim != ch )

        	{

		    act( AT_ACTION, "$n frota $p sobre $N.",

			ch, salve, victim, TO_NOTVICT );

		    act( AT_ACTION, "$n frota $p sobre ti.",

			ch, salve, victim, TO_VICT );

		    act( AT_ACTION, "Frotas $p sobre $N.",

			ch, salve, victim, TO_CHAR );

		}

		else

		{

		    act( AT_ACTION, "Frotas $p sobre ti mismo.",

			ch, salve, NULL, TO_CHAR );

		    act( AT_ACTION, "$n frota $p sobre $m mismo.",

			ch, salve, NULL, TO_ROOM );

		}

	    }

	}

	else

	    actiondesc( ch, salve, NULL );

    }



    WAIT_STATE( ch, salve->value[3] );

    retcode = obj_cast_spell( salve->value[4], salve->value[0], ch, victim, NULL );

    if ( retcode == rNONE )

	retcode = obj_cast_spell( salve->value[5], salve->value[0], ch, victim, NULL );

    if ( retcode == rCHAR_DIED || retcode == rBOTH_DIED )

    {

	bug( "do_apply:  char died", 0 );

        return;

    }



    if ( !obj_extracted(salve) && salve->value[1] <= 0 )

	extract_obj( salve );

    return;

}



/* generate an action description message */

void actiondesc( CHAR_DATA *ch, OBJ_DATA *obj, void *vo )

{

    char charbuf[MAX_STRING_LENGTH];

    char roombuf[MAX_STRING_LENGTH];

    char *srcptr = obj->action_desc;

    char *charptr = charbuf;

    char *roomptr = roombuf;

    const char *ichar = "Tu";

    const char *iroom = "Alguien";



while ( *srcptr != '\0' )

{

  if ( *srcptr == '$' )

  {

    srcptr++;

    switch ( *srcptr )

    {

      case 'e':

        ichar = "tu";

        iroom = "$e";

        break;



      case 'm':

        ichar = "tu";

        iroom = "$m";

        break;



      case 'n':

        ichar = "tu";

        iroom = "$n";

        break;



      case 's':

        ichar = "tu";

        iroom = "$s";

        break;



      /*case 'q':

        iroom = "s";

        break;*/



      default:

        srcptr--;

        *charptr++ = *srcptr;

        *roomptr++ = *srcptr;

        break;

    }

  }

  else if ( *srcptr == '%' && *++srcptr == 's' )

  {

    ichar = "Tu";

    iroom = IS_NPC( ch ) ? ch->short_descr : ch->name;

  }

  else

  {

    *charptr++ = *srcptr;

    *roomptr++ = *srcptr;

    srcptr++;

    continue;

  }



  while ( ( *charptr = *ichar ) != '\0' )

  {

    charptr++;

    ichar++;

  }



  while ( ( *roomptr = *iroom ) != '\0' )

  {

    roomptr++;

    iroom++;

  }

  srcptr++;

}



*charptr = '\0';

*roomptr = '\0';



/*

sprintf( buf, "Charbuf: %s", charbuf );

log_string_plus( buf, LOG_HIGH, LEVEL_LESSER );

sprintf( buf, "Roombuf: %s", roombuf );

log_string_plus( buf, LOG_HIGH, LEVEL_LESSER );

*/



switch( obj->item_type )

{

  case ITEM_BLOOD:

  case ITEM_FOUNTAIN:

    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );

    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );

    return;



  case ITEM_DRINK_CON:

    act( AT_ACTION, charbuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_CHAR );

    act( AT_ACTION, roombuf, ch, obj, liq_table[obj->value[2]].liq_name, TO_ROOM );

    return;



  case ITEM_PIPE:

    return;



  case ITEM_ARMOR:

  case ITEM_WEAPON:

  case ITEM_LIGHT:

    return;



  case ITEM_COOK:

  case ITEM_FOOD:

  case ITEM_PILL:

    act( AT_ACTION, charbuf, ch, obj, ch, TO_CHAR );

    act( AT_ACTION, roombuf, ch, obj, ch, TO_ROOM );

    return;



  default:

    return;

}

return;

}



/*

 * Extended Bitvector Routines					-Thoric

 */



/* check to see if the extended bitvector is completely empty */

bool ext_is_empty( EXT_BV *bits )

{

    int x;



    for ( x = 0; x < XBI; x++ )

	if ( bits->bits[x] != 0 )

	    return FALSE;



    return TRUE;

}



void ext_clear_bits( EXT_BV *bits )

{

    int x;



    for ( x = 0; x < XBI; x++ )

	bits->bits[x] = 0;

}



/* for use by xHAS_BITS() -- works like IS_SET() */

int ext_has_bits( EXT_BV *var, EXT_BV *bits )

{

    int x, bit;



    for ( x = 0; x < XBI; x++ )

	if ( (bit=(var->bits[x] & bits->bits[x])) != 0 )

	    return bit;



    return 0;

}



/* for use by xSAME_BITS() -- works like == */

bool ext_same_bits( EXT_BV *var, EXT_BV *bits )

{

    int x;



    for ( x = 0; x < XBI; x++ )

	if ( var->bits[x] != bits->bits[x] )

	    return FALSE;



    return TRUE;

}



/* for use by xSET_BITS() -- works like SET_BIT() */

void ext_set_bits( EXT_BV *var, EXT_BV *bits )

{

    int x;



    for ( x = 0; x < XBI; x++ )

	var->bits[x] |= bits->bits[x];

}



/* for use by xREMOVE_BITS() -- works like REMOVE_BIT() */

void ext_remove_bits( EXT_BV *var, EXT_BV *bits )

{

    int x;



    for ( x = 0; x < XBI; x++ )

	var->bits[x] &= ~(bits->bits[x]);

}



/* for use by xTOGGLE_BITS() -- works like TOGGLE_BIT() */

void ext_toggle_bits( EXT_BV *var, EXT_BV *bits )

{

    int x;



    for ( x = 0; x < XBI; x++ )

	var->bits[x] ^= bits->bits[x];

}



/*

 * Read an extended bitvector from a file.			-Thoric

 */

EXT_BV fread_bitvector( FILE *fp )

{

    EXT_BV ret;

    int c, x = 0;

    int num = 0;



    memset( &ret, '\0', sizeof(ret) );

    for ( ;; )

    {

	num = fread_number(fp);

	if ( x < XBI )

	    ret.bits[x] = num;

	++x;

	if ( (c=getc(fp)) != '&' )

	{

	    ungetc(c, fp);

	    break;

	}

    }



    return ret;

}



/* return a string for writing a bitvector to a file */

char *print_bitvector( EXT_BV *bits )

{

    static char buf[XBI * 12];

    char *p = buf;

    int x, cnt = 0;



    for ( cnt = XBI-1; cnt > 0; cnt-- )

	if ( bits->bits[cnt] )

	    break;

    for ( x = 0; x <= cnt; x++ )

    {

	sprintf(p, "%d", bits->bits[x]);

	p += strlen(p);

	if ( x < cnt )

	    *p++ = '&';

    }

    *p = '\0';



    return buf;

}



/*

 * Write an extended bitvector to a file			-Thoric

 */

void fwrite_bitvector( EXT_BV *bits, FILE *fp )

{

    fputs( print_bitvector(bits), fp );

}





EXT_BV meb( int bit )

{

    EXT_BV bits;



    xCLEAR_BITS(bits);

    if ( bit >= 0 )

	xSET_BIT(bits, bit);



    return bits;

}





EXT_BV multimeb( int bit, ... )

{

    EXT_BV bits;

    va_list param;

    int b;



    xCLEAR_BITS(bits);

    if ( bit < 0 )

	return bits;



    xSET_BIT(bits, bit);



    va_start(param, bit);



    while ((b = va_arg(param, int)) != -1)

	xSET_BIT(bits, b);



    va_end(param);



    return bits;

}





#ifdef WIN32



/* routines not in Windows runtime libraries */



void gettimeofday(struct timeval *tv, struct timezone *tz)

{

    tv->tv_sec = time (0);

    tv->tv_usec = 0;

}



/* directory parsing stuff */



DIR * opendir (char * sDirName)

{

    DIR * dp = malloc (sizeof (DIR));



    dp->hDirectory = 0;      /* if zero, we must do a FindFirstFile */

    strcpy (dp->sDirName, sDirName);  /* remember for FindFirstFile */

    return dp;

}



struct dirent * readdir (DIR * dp)

{



    /* either read the first entry, or the next entry */

    do

    {

	if (dp->hDirectory == 0)

	{

	    dp->hDirectory = FindFirstFile (dp->sDirName, &dp->Win32FindData);

	    if (dp->hDirectory == INVALID_HANDLE_VALUE)

		return NULL;

	}

	else

	if (!FindNextFile (dp->hDirectory, &dp->Win32FindData))

	    return NULL;



	/* skip directories */



    } while (dp->Win32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);



    /* make a copy of the name string */

    dp->dirinfo.d_name = dp->Win32FindData.cFileName;



/* return a pointer to the DIR structure */



    return &dp->dirinfo;

}



void closedir(DIR * dp)

{

    if (dp->hDirectory)

	FindClose (dp->hDirectory);

    free (dp);

}



#endif

