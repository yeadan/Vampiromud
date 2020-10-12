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
 *			   Smaug banking support module                           *
 ****************************************************************************/
/***************************************************************************
 *                          SMAUG Banking Support Code                     *
 ***************************************************************************
 *                                                                         *
 * This code may be used freely, as long as credit is given in the help    *
 * file. Thanks.                                                           *
 *								                           *
 *                                        -= Minas Ravenblood =-           *
 *                                 Implementor of The Apocalypse Theatre   *
 *                                      (email: krisco7@hotmail.com)       *
 *									                     *
 ***************************************************************************/

/* Modifications to original source by Samson */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* You can add this or just put it in the do_bank code. I don't really know
   why I made a seperate function for this, but I did. If you do add it,
   don't forget to declare it - Minas */
/* Finds banker mobs in a room. Installed by Samson on unknown date */
/* NOTE: Smaug 1.02a Users - Your compiler probably died on this
   function - if so, remove the x in front of IS_SET and recompile */
CHAR_DATA *find_banker( CHAR_DATA *ch )
{
  CHAR_DATA *banker;

  for ( banker = ch->in_room->first_person; banker; banker = banker->next_in_room )
    if ( IS_NPC( banker ) && xIS_SET( banker->act, ACT_BANKER ) )
      break;

  return banker;
}

/* SMAUG Bank Support
 * Coded by Minas Ravenblood for The Apocalypse Theatre
 * (email: krisco7@hotmail.com)
 */
/* Installed by Samson on unknown date */
/* Deposit, withdraw, balance and transfer commands */
void do_ingreso( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *banker;
  char arg1[MAX_INPUT_LENGTH];
  char buf [MAX_STRING_LENGTH];
  int amount;

  if ( !( banker = find_banker( ch ) ) )
  {
    send_to_char( "Tu ves algun usurero aqui?\n\r", ch );
    return;
  }

  if ( IS_NPC( ch ) )
  {
    sprintf( buf, "Lo siento , %s, no hago tratos con mobs.", ch->short_descr );
    do_say( banker, buf );
    return;
  }

  if ( argument[0] == '\0' )
  {
    do_say( banker, "Si necesitas ayuda escribe AYUDA USURERO." );
    return;
  }

  argument = one_argument( argument, arg1 );

  if ( arg1 == '\0' )
  {
    sprintf( buf, "%s Cuanto oro quieres que guarde por ti?", ch->name );
    do_tell( banker, buf );
    return;
  }

  if ( str_cmp( arg1, "todo" ) && !is_number( arg1 ) )
  {
    sprintf( buf, "%s Decias algo..?", ch->name );
    do_tell( banker, buf );
    return;
  }

  if ( !str_cmp( arg1, "todo" ) )
    amount = ch->gold;
  else
    amount = atoi( arg1 );

  if ( amount > ch->gold )
  {
    sprintf( buf, "%s Igual tengo cara de tonto pero no estoy lelo no llevas tanto oro.",
    	       ch->name );
    do_tell( banker, buf );
    return;
  }

  if ( amount <= 0 )
  {
    sprintf( buf, "%s Oh, ya veo, hace un bonito dia ciertamente... ideal para morir.",
             ch->name );
    do_tell( banker, buf );
    return;
  }

  ch->gold		-= amount;
  ch->pcdata->balance	+= amount;
  sprintf( buf, "Depositas plenemente tu confianza y %d moneda%s de oro al usurero.\n\r", amount, (amount != 1) ? "s" : "" );
  set_char_color( AT_PLAIN, ch );
  send_to_char( buf, ch );
  sprintf( buf, "$n deposita plenamente su confianza y %d moneda%s de oro al usurero y se queda tan tranquilo.\n\r", amount, (amount != 1) ? "s" : "" );
  act( AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM );
  sprintf( buf, "Marcha tranquilo %s, yo gasta.... esto... cuidare bien tu dinero.",
             ch->name );
    do_tell( banker, buf );
  return;
}

void do_reintegro( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *banker;
  char arg1[MAX_INPUT_LENGTH];
  char buf [MAX_STRING_LENGTH];
  int amount;

  if ( !( banker = find_banker( ch ) ) )
  {
    send_to_char( "Yo no veo ningun usurero por aqui!\n\r", ch );
    return;
  }

  if ( IS_NPC( ch ) )
  {
    sprintf( buf, "Lo siento, %s, no hago tratos con mobs.", ch->short_descr );
    do_say( banker, buf );
    return;
  }

  if ( argument[0] == '\0' )
  {
    do_say( banker, "si necesitas ayuda teclea AYUDA USURERO." );
    return;
  }
  
  argument = one_argument( argument, arg1 );
    
  if ( arg1 == '\0' )
  {
    sprintf( buf, "%s Cuantas monedas quieres robarme... esto... que te devuelva?", ch->name );
    do_tell( banker, buf );
    return;
  }
  if ( str_cmp( arg1, "todo" ) && !is_number( arg1 ) )
  {
    sprintf( buf, "Decias algo %s?", ch->name );
    do_tell( banker, buf );
    return;
  }

  if ( !str_cmp( arg1, "todo" ) )
    amount = ch->pcdata->balance;
  else
    amount = atoi( arg1 );

  if ( amount > ch->pcdata->balance )
  {
    sprintf( buf, "%s Si hombre, yo no te he guardado tanto dinero!",
    	       ch->name );
    do_tell( banker, buf );
    return;
  }

  if ( amount <= 0 )
  {
    sprintf( buf, "%s Oh entonces %d monedas son... me debes una pasta he...",
             ch->name, amount );
    do_tell( banker, buf );
    return;
  }

  ch->pcdata->balance	-= amount;
  ch->gold		+= amount;
  sprintf( buf, "Recuperas con gran alegria %d moneda%s de oro de las garras del usurero.\n\r", amount, (amount != 1) ? "s" : "" );
  set_char_color( AT_PLAIN, ch );
  send_to_char( buf, ch );
  sprintf( buf, "$n recupera con gran alegria %d moneda%s de oro de las garras del usurero.\n\r", amount, (amount != 1) ? "s" : "" );
  act( AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM );
  sprintf( buf, "Volveras %s todos vuelven, ya gastaras esa%s %d miserable%s moneda%s",
             ch->name,  (amount != 1) ? "s" : "", amount, (amount != 1) ? "s" : "",(amount != 1) ? "s" : "" );
    do_tell( banker, buf );
  return;
}

void do_saldo( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *banker;
  char buf [MAX_STRING_LENGTH];

    if ( !( banker = find_banker( ch ) ) )
    {
      send_to_char( "Tu ves algun usurero por aqui\n\r", ch );
      return;
    }
    /*
    if ( IS_NPC( ch ) )
    {
      sprintf( buf, "Lo siento, %s, no hago tratos con mobs.", ch->short_descr );
      do_say( banker, buf );
      return;
    }   */

    set_char_color( AT_PLAIN, ch );
    sprintf( buf, "El usurero gasta... digo.... guarda %d moneda%s tuya%s debajo de su colchon.\n\r",
    ch->pcdata->balance, (ch->pcdata->balance == 1) ? "" : "s" , (ch->pcdata->balance == 1) ? "" : "s");
    send_to_char( buf, ch );
    return;
}

/* End of new bank support */

