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
 * ------------------------------------------------------------------------ *                                                                                                                                                                                                                                               * Agradecimientos  especiales a: Reena ke no entra xD, Vidal por el apoyo  *
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
 *         Modulo de conjuros pertenecientes a las disciplinas       *
 ****************************************************************************/

 #include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef sun
  #include <strings.h>
#endif
#include <time.h>
#include "mud.h"


/* nivel 3 comunion espiritual */
ch_ret spell_comunion_espiritual ( int sn, int level, CHAR_DATA *ch, void *vo )
{
  MORPH_DATA *morph;
  SKILLTYPE *skill = get_skilltype(sn);

  morph = find_morph( ch, target_name, TRUE );
  if ( !morph )
  {
    send_to_char("No te puedes fusionar en nada parecido a eso!\n\r", ch );
    return rSPELL_FAILED;
  }
  if ( !do_morph_char(ch, morph) )
  {
  	failed_casting( skill, ch, NULL, NULL );
	return rSPELL_FAILED;
  }
  return rNONE;
}

/* Nivel tres Toque Espiritual */


ch_ret spell_toque_espiritual( int sn, int level, CHAR_DATA *ch, void *vo )
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    AFFECT_DATA *paf;
    SKILLTYPE *sktmp;
    SKILLTYPE *skill = get_skilltype(sn);
    char *name;

    if ( target_name[0] == '\0' )
    {
      send_to_char( "Ver el espiritu de quien o de que?\n\r", ch );
      return rSPELL_FAILED;
    }

    if ( ( obj = get_obj_carry( ch, target_name ) ) != NULL )
    {
	set_char_color( AT_LBLUE, ch );
	ch_printf( ch,
	"\n\r%s es %s",
/*		obj->name,*/
		obj->short_descr,
		aoran( item_type_name( obj ) ) );
	if ( obj->item_type != ITEM_LIGHT && obj->wear_flags-1 > 0 )
  	  ch_printf( ch, ", se lleva en:  %s\n\r",
		flag_string(obj->wear_flags-1, w_flags) );
	else
	  send_to_char( ".\n\r", ch );
	ch_printf( ch,

	"Propiedades especiales:  %s\n\rPesa %d, su valor es %d, y es de nivel %d.\n\r",

		extra_bit_name( &obj->extra_flags ),

	/*	magic_bit_name( obj->magic_flags ), -- unused for now */

		obj->weight,

		obj->cost,

		obj->level );

	set_char_color( AT_MAGIC, ch );



    switch ( obj->item_type )

    {

    case ITEM_CONTAINER:

	ch_printf( ch, "%s parece ser %s.\n\r", capitalize(obj->short_descr),

		obj->value[0] < 76  ? "de muy poca capacidad"           :

		obj->value[0] < 150 ? "de poca capacidad" :

		obj->value[0] < 300 ? "de capacidad media"          :

		obj->value[0] < 550 ? "de bastante capacidad" :

		obj->value[0] < 751 ? "de mucha capacidad"           :

				      "de una capacidad asombrosa" );

	break;



    case ITEM_PILL:

    case ITEM_SCROLL:

    case ITEM_POTION:

	ch_printf( ch, "Nivel %d, hechizos de:", obj->value[0] );



	if ( obj->value[1] >= 0 && (sktmp=get_skilltype(obj->value[1])) != NULL )

	{

	    send_to_char( " '", ch );

	    send_to_char( sktmp->name, ch );

	    send_to_char( "'", ch );

	}



	if ( obj->value[2] >= 0 && (sktmp=get_skilltype(obj->value[2])) != NULL )

	{

	    send_to_char( " '", ch );

	    send_to_char( sktmp->name, ch );

	    send_to_char( "'", ch );

	}



	if ( obj->value[3] >= 0 && (sktmp=get_skilltype(obj->value[3])) != NULL )

	{

	    send_to_char( " '", ch );

	    send_to_char( sktmp->name, ch );

	    send_to_char( "'", ch );

	}



	send_to_char( ".\n\r", ch );

	break;



    case ITEM_SALVE:

	ch_printf( ch, "Tiene %d(%d) hechizos de nivel %d",

	  obj->value[1], obj->value[2], obj->value[0] );

	if ( obj->value[4] >= 0 && (sktmp=get_skilltype(obj->value[4])) != NULL )

        {

            send_to_char( " '", ch );

            send_to_char( sktmp->name, ch );

            send_to_char( "'", ch );

        }

	if ( obj->value[5] >= 0 && (sktmp=get_skilltype(obj->value[5])) != NULL )

        {

            send_to_char( " '", ch );

            send_to_char( sktmp->name, ch );

            send_to_char( "'", ch );

        }

	send_to_char( ".\n\r", ch );

	break;



    case ITEM_WAND:

    case ITEM_STAFF:

	ch_printf( ch, "Tiene %d(%d) cargas de nivel %d",

	    obj->value[1], obj->value[2], obj->value[0] );



	if ( obj->value[3] >= 0 && (sktmp=get_skilltype(obj->value[3])) != NULL )

	{

	    send_to_char( " '", ch );

	    send_to_char( sktmp->name, ch );

	    send_to_char( "'", ch );

	}



	send_to_char( ".\n\r", ch );

	break;



    case ITEM_WEAPON:

   if (obj->value[3] == 0)

        ch_printf( ch, "Tipo de arma: Pugilismo\n\r" );

        else

        if (obj->value[3] == 1)

        ch_printf( ch, "Tipo de arma: Long Blades\n\r" );

        else

        if (obj->value[3] == 2)

        ch_printf( ch, "Tipo de arma Short Blade\n\r" );

        else

        if (obj->value[3] == 3)

        ch_printf( ch, "Tipo de arma: Sword\n\r" );

        else

        if (obj->value[3] == 4)

        ch_printf( ch, "Tipo de arma: Flexible Arm\n\r" );

        else

        if (obj->value[3] == 5)

        ch_printf( ch, "Tipo de arma: Talonous Arm\n\r" );

        else

        if (obj->value[3] == 6)

        ch_printf( ch, "Tipo de arma: Pugilism\n\r" );

        else

        if (obj->value[3] == 7)

        ch_printf( ch, "Tipo de arma: Bludgeon\n\r" );

        else

        if (obj->value[3] == 8)

        ch_printf( ch, "Tipo de arma: Bludgeon\n\r" );

        else

        if (obj->value[3] == 9)

        ch_printf( ch, "Tipo de arma: Unknown\n\r" ); /* What type would a v3 of 9 make it? Not sure-Joe*/

        else

        if (obj->value[3] == 10)

        ch_printf( ch, "Tipo de arma: Pugilism\n\r" );

        else

        if (obj->value[3] == 11)

        ch_printf( ch, "Tipo de arma: Short Blade\n\r" );

        else

        if (obj->value[3] == 12)

        ch_printf( ch, "Tipo de arma: Pugilism\n\r" );

        else

        if (obj->value[3] >= 13)

        ch_printf( ch, "Tipo de arma: Missile Weapon\n\r" );

/* modificado por SiGo el /30/11/2000 */

   ch_printf( ch, "Su poder destructor es %d a %d (media %d)%s\n\r",

	    obj->value[1], obj->value[2],

	    ( obj->value[1] + obj->value[2] ) / 2,

	    IS_OBJ_STAT( obj, ITEM_POISONED) ? ", y esta totalmente envenenado." :

	    "." );





   break;





    case ITEM_ARMOR:

	ch_printf( ch, "Armadura: %d.\n\r", obj->value[0] );

	break;

    }



    for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )

	showaffect( ch, paf );



    for ( paf = obj->first_affect; paf; paf = paf->next )

	showaffect( ch, paf );



    return rNONE;

    }



    else if ( ( victim = get_char_room( ch, target_name ) ) != NULL )

    {



    /* comentado por el momento

    if ( IS_SET( victim->immune, RIS_MAGIC ) )

    {

	immune_casting( skill, ch, victim, NULL );

	return rSPELL_FAILED;

    }  */



    /* If they are morphed or a NPC use the appropriate short_desc otherwise

     * use their name -- Shaddai

     */



    if ( victim->morph && victim->morph->morph )

	name = capitalize(victim->morph->morph->short_desc);

    else if ( IS_NPC(victim) )

    	name = capitalize(victim->short_descr);

    else

    	name = victim->name;



    ch_printf(ch, "%s parece estar entre nivel %d y %d.\n\r",

	name,

	victim->level - (victim->level % 5),

        victim->level - (victim->level % 5) + 5);



    if ( IS_NPC(victim) && victim->morph )

	ch_printf(ch,"%s parece ser de %s.\n\r",

	name,

	(ch->level > victim->level + 10)

	 ? victim->name : "alguien mas");



    ch_printf(ch,"%s parece %s, y sigue el camino de %s.\n\r",

     	name, aoran(get_race(victim)), get_class(victim));



    if ( (chance(ch, 50) && ch->level >= victim->level + 10 )

    ||    IS_IMMORTAL(ch) )

    {

    ch_printf(ch, "%s parece estar afectado por: ", name);



    if (!victim->first_affect)

    {

      send_to_char( "nada.\n\r", ch );

      return rNONE;

    }



      for ( paf = victim->first_affect; paf; paf = paf->next )

      {

        if (victim->first_affect != victim->last_affect)

        {

          if( paf != victim->last_affect && (sktmp=get_skilltype(paf->type)) != NULL )

            ch_printf( ch, "%s, ", sktmp->name );



	  if( paf == victim->last_affect && (sktmp=get_skilltype(paf->type)) != NULL )

	  {

	    ch_printf( ch, "y %s.\n\r", sktmp->name );

	    return rNONE;

	  }

        }

        else

	{

	  if ( (sktmp=get_skilltype(paf->type)) != NULL )

	    ch_printf( ch, "%s.\n\r", sktmp->name );

	  else

	    send_to_char( "\n\r", ch );

	  return rNONE;

	}

      }

    }

  }



  else

  {

    ch_printf(ch, "No puedes encontrar %s!\n\r", target_name );

    return rSPELL_FAILED;

  }

  return rNONE;

}


/* CONJURO HABILIDAD KAYSER */

ch_ret spell_habilidad( int sn, int level, CHAR_DATA *ch, void *vo )
{

	SKILLTYPE *skill = get_skilltype(sn);

	AFFECT_DATA af;

 	if ( IS_AFFECTED(ch, AFF_HABILIDAD) )
 		{
 			send_to_char("Ya estas afectado por este conjuro.\n\r", ch);
       			return rSPELL_FAILED;
		}
	send_to_char("Sientes la habilidad en tu cuerpo.\n\r", ch);

 	af.type      = sn;
     	af.duration  = level * DUR_CONV;
	af.bitvector = meb(AFF_HABILIDAD);
        af.location  = NULL;
     	xSET_BIT(ch->affected_by, AFF_HABILIDAD );
 	affect_to_char( ch, &af );

     	return rNONE;

}




