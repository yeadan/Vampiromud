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
 *			      Regular update module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
 * Funciones de torneo ( arena )
 * SiGo y SaNgUi
 */
extern void start_arena();
extern void do_game();
extern int in_start_arena;
extern int ppl_in_arena;
extern int ppl_challenged;
extern int num_in_arena();

/*
 * Local functions.
 */

int	hit_gain	args( ( CHAR_DATA *ch ) );
int	mana_gain	args( ( CHAR_DATA *ch ) );
int	move_gain	args( ( CHAR_DATA *ch ) );
void	mobile_update	args( ( void ) );
void  weather_update args( ( void ) );
void	time_update	args( ( void ) );	/* FB */
void	char_update	args( ( void ) );
void	obj_update	args( ( void ) );
void    abrazo_update   args( ( void ) ); /* SiGo */
void	aggr_update	args( ( void ) );
void	room_act_update	args( ( void ) );
void	obj_act_update	args( ( void ) );
void	char_check	args( ( void ) );
void disciaffect_update args( ( CHAR_DATA *ch ) ); /*  Disciplinas */
void    quest_update    args( ( void ) ); /* Questmaster */
void    drunk_randoms	args( ( CHAR_DATA *ch ) );
void    hallucinations	args( ( CHAR_DATA *ch ) );
void	subtract_times	args( ( struct timeval *etime,
				struct timeval *stime ) );

void degradacion_generacion_pk args ( ( CHAR_DATA *victim ) );  /* SiGo y SaNgUi */
/* weather functions - FB */

void	adjust_vectors		args( ( WEATHER_DATA *weather) );

void	get_weather_echo	args( ( WEATHER_DATA *weather) );

void	get_time_echo		args( ( WEATHER_DATA *weather) );



/*

 * Global Variables

 */



CHAR_DATA *	gch_prev;
CHAR_DATA *     gch_next;

OBJ_DATA *	gobj_prev;



CHAR_DATA *	timechar;



char * corpse_descs[] =

   {

     "El cuerpo de %s esta en sus ultimos instantes de descomposicion.",

     "El cuerpo de %s esta lleno de gusanos.",

     "El cuerpo de %s inunda el aire con un hedor indescriptible.",

     "El cuerpo de %s esta lleno de moscas.",

     "El cuerpo de %s esta aqui."

   };



extern int      top_exit;



/*

 * Advancement stuff.

 */

/*
	"bajar_nivel" by KAYSER
	Esto no baja el nivel, es el resultado de bajarlo.
*/

void bajar_nivel( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];

    if ( ch->level == 1)
    return;

	/* Al cambiar de nivel se actualiza este flag */

	if ( !IS_NPC(ch) )
	xREMOVE_BIT( ch->act, PLR_BOUGHT_PET );

	/* mas */
    sprintf( buf, "&O%s &gha bajado a nivel &Y%d", ch->name, ch->level );
    mensa_todos(ch,"vampiro",buf);
    sprintf ( buf, "Has bajado a nivel %d!\n\r", ch->level);
    send_to_char ( buf, ch );

    return;
}


	/* Fin KAYSER bajar_nivel */

void advance_level( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int add_hp;
    int add_mana;
    int add_move;
    int add_prac;

/*	save_char_obj( ch );*/
    sprintf( buf, " %s",
	title_table [ch->class] [ch->level] [ch->sex == SEX_FEMALE ? 1 : 0] );
   /* set_title( ch, buf );*/

   if(ch->pcdata->max_level > ch->level)
        ch_printf(ch,"&GRecuperas alguno de tus niveles perdidos.\n\r");

    /* Ganancia de jugadores remorts */

    if (xIS_SET(ch->act, PLR_RENACIDO ) )
    {
		advanceLevelRenacidos( ch );
     }
     else
     {
    add_hp	= con_app[get_curr_con(ch)].hitp + number_range(
		    class_table[ch->class]->hp_min,
		    class_table[ch->class]->hp_max );
    add_mana	= class_table[ch->class]->fMana
		    ? number_range(6, (2*get_curr_int(ch)+get_curr_wis(ch))/2)
		    : 0;
    add_move	= number_range( 5, (get_curr_con(ch)+get_curr_dex(ch))/4 );
    add_prac	= wis_app[get_curr_wis(ch)].practice;
     }
    add_hp	= UMAX(  1, add_hp   );
    add_mana	= UMAX(  0, add_mana );
    add_move	= UMAX( 10, add_move );

    /* bonus for deadlies */
    if(IS_PKILL(ch)){
        add_mana = add_mana + add_mana*.3;
        add_move = add_move + add_move*.3;
        add_hp +=1; /* bitch at blod if you don't like this :) */
        sprintf(buf,"El favor de Kayser endurece tus musculos.\n\r");
    }
     if(ch->pcdata->max_level == ch->level)
    {
    ch->max_hit 	+= add_hp;
    ch->max_mana	+= add_mana;
    ch->max_move	+= add_move;
    ch->practice	+= add_prac;
    }

    if ( !IS_NPC(ch) )
	xREMOVE_BIT( ch->act, PLR_BOUGHT_PET );

    if ( ch->level == LEVEL_AVATAR )
    {
	sprintf( buf, "&O%s &gha llegado a &YHeroe", ch->name );
	mensa_todos(ch,"vampiro",buf);
	set_char_color( AT_WHITE, ch );
	do_help( ch, "M_ADVHERO_" );
    }

    if ( ch->level == LEVEL_SUPERAVATAR )
    {
        sprintf( buf, "&O%s &gha llegado a &YAvatar", ch->name );
	mensa_todos(ch,"vampiro",buf);
	set_char_color( AT_WHITE, ch );
   }

   if ( ch->level == LEVEL_ENGENDRO )
   {
        sprintf( buf, "&O%s &gha llegado a &YEngendro", ch->name );
	mensa_todos(ch,"vampiro",buf);
	set_char_color( AT_WHITE, ch );
   }

    sprintf ( buf, "&O%s &gha subido a nivel &Y%d", ch->name, ch->level );
    mensa_todos(ch,"vampiro",buf);
    sprintf ( buf, "Has subido a nivel %d!\n\r", ch->level);
    send_to_char ( buf, ch );
    if ( ch->level < LEVEL_IMMORTAL )
    {
    if(ch->pcdata->max_level == ch->level)
    {
      if ( IS_VAMPIRE(ch) )
        sprintf( buf,
	  "Tu ganancia es: %d/%d pv, %d/%d sangre, %d/%d mv %d/%d prac.\n\r",
	  add_hp,	ch->max_hit,
	  1,	        ch->level + 10,
	  add_move,	ch->max_move,
	  add_prac,	ch->practice
	  );
      else
        sprintf( buf,
	  "Tu ganancia es: %d/%d pv, %d/%d mana, %d/%d mv %d/%d prac.\n\r",
	  add_hp,	ch->max_hit,
 	  add_mana,	ch->max_mana,
	  add_move,	ch->max_move,
	  add_prac,	ch->practice
	  );
      set_char_color( AT_WHITE, ch );
      send_to_char( buf, ch );
      if ( !IS_NPC( ch ) )
      {
	sprintf( buf2, "&G%-13s  ->&w%-2d  &G-&w  %-5d&G   Rvnum: %-5d   %s %s",
	  ch->name,
	  ch->level,
	  get_age( ch ),
	  ch->in_room == NULL ? 0 : ch->in_room->vnum,
	  capitalize(race_table[ch->race]->race_name),
	  class_table[ch->class]->who_name );
	append_to_file( PLEVEL_FILE, buf2 );
      }
    }
    }
    if(ch->pcdata->max_level == ch->level)
    ch->pcdata->quest_accum = 0;

    return;
}


void gain_exp( CHAR_DATA *ch, int gain )
{
    int modgain;
    int modgain2;
    //CLAN_DATA *familia;
    //CLAN_DATA *fam_vict;
    char buf[MAX_STRING_LENGTH];

	if ( IS_NPC(ch) )
 		return;

        if (ch->exp < 0)
	{
        /* Bug arreglado por SiGo y SaNgUi
                   ch->exp = exp_level;           */
                   ch->exp = exp_level( ch, ch->level ) +1;
	bug("%s tenia los px en negativo...Arreglado", ch->name);
	}

    if (  ch->level < LEVEL_AVATAR )
    {
        /* Bug arreglado por SiGo
        if ( ch->exp == exp_level + 2000; */
        if (ch->exp > exp_level( ch, ch->level + 1) )
	{
        /* Bug arreglado por SiGo y SaNgUi
                   ch->exp = exp_level;           */
                ch->exp = exp_level(ch, ch->level ) + 1;
                 bug ("%s tenia los px mal... Arreglado",ch->name);
	}
    }

    /* Bonus for deadly lowbies */
    /* MUY reducido para el nuevo sistema de experiencia, KAYSER */
    modgain = gain;
    if(modgain>0 && IS_PKILL(ch) && ch->level<17){
       if(ch->level<=6){
          sprintf(buf,"El favor de los dioses incrementa tu aprendizaje.\n\r");
          modgain*=1.2;
       }
       if(ch->level<=10 && ch->level>=7){
          sprintf(buf,"La mano de los dioses acelera tu aprendizaje.\n\r");
          modgain*=1.175;
       }
       if(ch->level<=13 && ch->level>=11){
          sprintf(buf,"La bondad de los dioses auxilia tu aprendizaje.\n\r");
          modgain*=1.15;
       }
       if(ch->level<=16 && ch->level>=14){
          sprintf(buf,"El seguimiento de los dioses refuerza tu aprendizaje.\n\r");
          modgain*=1.125;
       }
	send_to_char(buf, ch);
    }

    /* per-race experience multipliers */
    modgain *= ( race_table[ch->race]->exp_multiplier/100.0);

    /* xp cap to prevent any one event from giving enuf xp to */
    /* gain more than one level - FB */

    /* Esto lo dejo, asi la gente no sube demasiado rapido, KAYSER */

    modgain = UMIN(modgain,
    	exp_level(ch, ch->level+2) - exp_level(ch, ch->level+1));

   if ( ch->level == LEVEL_AVATAR
   || ch->level == LEVEL_SUPERAVATAR
   || ch->level == LEVEL_ENGENDRO
   || ch->level == LEVEL_NEOPHYTE
   || ch->level == LEVEL_ACOLYTE 
   || ch->level > LEVEL_ACOLYTE )   
   {
   modgain2 = gain;

   modgain = 0;
   if (modgain2 > 0)
   	ch->exp_acumulada  += modgain2;
   return;
    }

    /*
     * Si el jugador esta en una familia y hay declaraciones de alianza y guerra que afecten a
     * la experiencia.
     *                                          SaNgUiNaRi & SiGo
     */

    /*if ((IS_CLANNED(ch)
        && ch->in_room->area->conquistable !=1))
    {
     if ( ch->in_room->area->propietario != "Territorio virgen" )
     {
     familia = get_clan( ch->pcdata->clan_name);
     fam_vict = get_clan( ch->in_room->area->propietario );
*/
          /* Declaracion de guerra piadosa */
     /*if ( fam_vict )
     if (familia != fam_vict )
     {
         if ( (familia->enemigos[fam_vict->numero] = fam_vict->name ) && (fam_vict->odio > 0) )
         {
         send_to_char("&wEstas en guerra contra los propietarios de este area, ganas mas puntos de experiencia.\n\r", ch);
         modgain += (modgain * 0.25);
         }
     }
     }
    } */

    ch->exp = UMAX( 0, ch->exp + modgain );

    /* No estas autorizado? No subes X) */

    if (NOT_AUTHED(ch) && ch->exp >= exp_level(ch, ch->level+1))
    {
	send_to_char("No puedes subir mas niveles sin ser autorizado.\n\r", ch);
	ch->exp = (exp_level(ch, (ch->level+1)) - 1);
	return;
    }
	/* KAYSER PARA BAJAR DE NIVEL */
    /* Bug arreglado por SiGo los niveles 1 podian bajar a nivel 0
       while ( ch->level < LEVEL_AVATAR && ch->exp < exp_level(ch, ch->level) && !IS_IMMORTAL(ch) ) */

    while ((ch->level < LEVEL_AVATAR && ch->exp < exp_level(ch, ch->level) && !IS_IMMORTAL(ch) && ch->level != 1))
    {
    set_char_color( AT_WHITE + AT_BLINK, ch );
    ch_printf( ch, "Has perdido demasiada experiencia y bajas a nivel %d!\n\r", --ch->level );
    if ( !IS_IMMORTAL( ch ) )
    bajar_nivel( ch );
    /* Bug arreglado por SiGo y SaNgUi  los inmortales podian bajar de nivel */
    break;
    }
	/* FIN KAYSER BAJAR NIVEL */

    while ( ch->level < LEVEL_AVATAR && ch->exp >= exp_level(ch, ch->level+1))
    {
	set_char_color( AT_WHITE + AT_BLINK, ch );
	ch_printf( ch, "Has obtenido la experiencia para subir a nivel %d!\n\r", ++ch->level );
        if(ch->pcdata->max_level == ch->level)
        {
        ch_printf(ch,"&GVuelves a ser el que eras!\n\r");
        sprintf ( buf, "&O%s &gha subido a nivel &Y%d", ch->name, ch->level );
        mensa_todos(ch,"vampiro",buf);
        sprintf ( buf, "Has subido a nivel %d!\n\r", ch->level);
        send_to_char ( buf, ch );
        if ( !IS_NPC(ch) )
	xREMOVE_BIT( ch->act, PLR_BOUGHT_PET );
        return;
        }

        if(ch->level > ch->pcdata->max_level)
        ch->pcdata->max_level = ch->level;

	advance_level( ch );
/*	printf (buf, "%s ha subido a nivel %d!\n\r",ch->name, ch->level);*/
    }
    return;
}

/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch )
{
    int gain;

    if ( IS_NPC(ch) )
    {
	gain = ch->level * 3 / 2;
    }
    else
    {
	gain = UMIN( 5, ch->level );

	switch ( ch->position )
	{
	case POS_DEAD:	   return 0;
	case POS_MORTAL:   return 1;
	case POS_INCAP:    return 1;
	case POS_STUNNED:  return 1;
	case POS_SLEEPING: gain += get_curr_con(ch) * 3.0;	break;
	case POS_RESTING:  gain += get_curr_con(ch) * 1.25;	break;
	}


        if ( IS_VAMPIRE(ch) ) {

            if ( ch->pcdata->condition[COND_BLOODTHIRST] <= 1 )

		gain /= 2;

	    else

	    if ( ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level) )

                gain *= 2;

            if ( IS_OUTSIDE( ch ) )

  	    {

    	       switch(time_info.sunlight)

    	       {

    	          case SUN_RISE:

    		  case SUN_SET:

      	            gain /= 2;

      	            break;

    		  case SUN_LIGHT:

      		    gain /= 4;

      		    break;

      	       }

      	    }

        }



	if ( ch->pcdata->condition[COND_FULL]   == 0 )

	    gain /= 2;



	if ( ch->pcdata->condition[COND_THIRST] == 0 )

	    gain /= 2;



    }



    if ( IS_AFFECTED(ch, AFF_POISON) )

	gain /= 4;



    return UMIN(gain, ch->max_hit - ch->hit);

}







int mana_gain( CHAR_DATA *ch )

{

    int gain;



    if ( IS_NPC(ch) )

    {

	gain = ch->level;

    }

    else

    {

	gain = UMIN( 5, ch->level / 2 );



	if ( ch->position < POS_SLEEPING )
	  return 0;

	switch ( ch->position )

	{

	case POS_SLEEPING: gain += get_curr_int(ch) * 3.25;	break;
	case POS_RESTING:  gain += get_curr_int(ch) * 1.75;	break;

	}



	if ( ch->pcdata->condition[COND_FULL]   == 0 )

	    gain /= 2;



	if ( ch->pcdata->condition[COND_THIRST] == 0 )

	    gain /= 2;



    }



    if ( IS_AFFECTED(ch, AFF_POISON) )

	gain /= 4;



    return UMIN(gain, ch->max_mana - ch->mana);

}







int move_gain( CHAR_DATA *ch )

{

    int gain;



    if ( IS_NPC(ch) )

    {

	gain = ch->level;

    }

    else

    {

	gain = UMAX( 15, 2 * ch->level );



	switch ( ch->position )
	{
	case POS_DEAD:	   return 0;
	case POS_MORTAL:   return -1;
	case POS_INCAP:    return -1;
	case POS_STUNNED:  return 1;
	case POS_SLEEPING: gain += get_curr_dex(ch) * 4.5;	break;
	case POS_RESTING:  gain += get_curr_dex(ch) * 2.5;	break;
	}



        if ( IS_VAMPIRE(ch) ) {

            if ( ch->pcdata->condition[COND_BLOODTHIRST] <= 1 )

		gain /= 2;

	    else

	    if ( ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level) )

                gain *= 2;

            if ( IS_OUTSIDE( ch ) )

  	    {

    	       switch(time_info.sunlight)

    	       {

    	          case SUN_RISE:

    		  case SUN_SET:

      	            gain /= 2;

      	            break;

    		  case SUN_LIGHT:

      		    gain /= 4;

      		    break;

      	       }

      	    }

        }



	if ( ch->pcdata->condition[COND_FULL]   == 0 )

	    gain /= 2;



	if ( ch->pcdata->condition[COND_THIRST] == 0 )

	    gain /= 2;

    }



    if ( IS_AFFECTED(ch, AFF_POISON) )

	gain /= 4;



    return UMIN(gain, ch->max_move - ch->move);

}





void gain_condition( CHAR_DATA *ch, int iCond, int value )

{

    int condition;

    ch_ret retcode = rNONE;



    if ( value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL || NOT_AUTHED(ch))

	return;



    condition				= ch->pcdata->condition[iCond];

    if ( iCond == COND_BLOODTHIRST )

	ch->pcdata->condition[iCond]	= URANGE( 0, condition + value,

                                                10 + ch->level + (13 / ch->generacion) * 40 );

    else

	ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );



    if ( ch->pcdata->condition[iCond] == 0 )

    {

	switch ( iCond )

	{

	case COND_FULL:

          if ( ch->level < LEVEL_AVATAR && !IS_VAMPIRE(ch))

          {

            set_char_color( AT_HUNGRY, ch );

	    send_to_char( "TE MUERES DE HAMBRE!\n\r",  ch );

            act( AT_HUNGRY, "$n se esta muriendo de hambre!", ch, NULL, NULL, TO_ROOM);

	    if ( !IS_PKILL(ch) || number_bits(1) == 0 )

		worsen_mental_state( ch, 1 );

	    retcode = damage(ch, ch, 1, TYPE_UNDEFINED);

          }

          break;



	case COND_THIRST:

          if ( ch->level < LEVEL_AVATAR && !IS_VAMPIRE(ch))

          {

            set_char_color( AT_THIRSTY, ch );

	    send_to_char( "TE MUERES DE SED!\n\r", ch );

            act( AT_THIRSTY, "$n se esta muriendo de sed!", ch, NULL, NULL, TO_ROOM);

	    worsen_mental_state( ch, IS_PKILL(ch) ? 1: 2 );

	    retcode = damage(ch, ch, 2, TYPE_UNDEFINED);

          }

          break;



        case COND_BLOODTHIRST:

          if ( ch->level < LEVEL_AVATAR )

          {

            set_char_color( AT_BLOOD, ch );

            send_to_char( "NECESITAS SANGRE!\n\r", ch );

            act( AT_BLOOD, "$n esta sufriendo las ansias de sangre!", ch,

                 NULL, NULL, TO_ROOM);

	    worsen_mental_state( ch, 2 );

	    retcode = damage(ch, ch, ch->max_hit / 20, TYPE_UNDEFINED);

          }

          break;

	case COND_DRUNK:

	    if ( condition != 0 ) {

                set_char_color( AT_SOBER, ch );

		send_to_char( "Estas sobrio.\n\r", ch );

	    }

	    retcode = rNONE;

	    break;

	default:

	    bug( "Gain_condition: invalid condition type %d", iCond );

	    retcode = rNONE;

	    break;

	}

    }



    if ( retcode != rNONE )

	return;



    if ( ch->pcdata->condition[iCond] == 1 )

    {

	switch ( iCond )

	{

	case COND_FULL:

          if ( ch->level < LEVEL_AVATAR && !IS_VAMPIRE(ch))

          {

            set_char_color( AT_HUNGRY, ch );

	    send_to_char( "Estas realmente hambriento.\n\r",  ch );

            act( AT_HUNGRY, "Puedes oir el estomago de $n rugiendo.", ch, NULL, NULL, TO_ROOM);

	    if ( number_bits(1) == 0 )

		worsen_mental_state( ch, 1 );

          }

	  break;



	case COND_THIRST:

          if ( ch->level < LEVEL_AVATAR && !IS_VAMPIRE(ch))

          {

            set_char_color( AT_THIRSTY, ch );

	    send_to_char( "Estas realmente sediento.\n\r", ch );

	    worsen_mental_state( ch, 1 );

	    act( AT_THIRSTY, "$n parece muy sediento.", ch, NULL, NULL, TO_ROOM);

          }

	  break;



        case COND_BLOODTHIRST:

          if ( ch->level < LEVEL_AVATAR )

          {

            set_char_color( AT_BLOOD, ch );

            send_to_char( "Necesitas beber sangre!\n\r", ch );

            act( AT_BLOOD, "$n tiene un extranyo brillo en $s ojos...", ch,

                 NULL, NULL, TO_ROOM);

	    worsen_mental_state( ch, 1 );

          }

          break;

	case COND_DRUNK:

	    if ( condition != 0 ) {

                set_char_color( AT_SOBER, ch );

		send_to_char( "Te sientes algo menos iluminado.\n\r", ch );

            }

	    break;

	}

    }





    if ( ch->pcdata->condition[iCond] == 2 )

    {

	switch ( iCond )

	{

	case COND_FULL:

          if ( ch->level < LEVEL_AVATAR && !IS_VAMPIRE(ch))

          {

            set_char_color( AT_HUNGRY, ch );

	    send_to_char( "Estas hambriento.\n\r",  ch );

          }

	  break;



	case COND_THIRST:

          if ( ch->level < LEVEL_AVATAR && !IS_VAMPIRE(ch))

          {

            set_char_color( AT_THIRSTY, ch );

	    send_to_char( "Estas sediento.\n\r", ch );

          }

	  break;



        case COND_BLOODTHIRST:

          if ( ch->level < LEVEL_AVATAR )

          {

            set_char_color( AT_BLOOD, ch );

            send_to_char( "Estas sediento de sangre.\n\r", ch );

          }

          break;

	}

    }



    if ( ch->pcdata->condition[iCond] == 3 )

    {

	switch ( iCond )

	{

	case COND_FULL:

          if ( ch->level < LEVEL_AVATAR && !IS_VAMPIRE(ch))

          {

            set_char_color( AT_HUNGRY, ch );

	    send_to_char( "Oyes rugir tu estomago.\n\r",  ch );

          }

	  break;



	case COND_THIRST:

          if ( ch->level < LEVEL_AVATAR && !IS_VAMPIRE(ch))

          {

            set_char_color( AT_THIRSTY, ch );

	    send_to_char( "Podrias buscar algo para beber.\n\r", ch );

          }

	  break;



        case COND_BLOODTHIRST:

          if ( ch->level < LEVEL_AVATAR )

          {

            set_char_color( AT_BLOOD, ch );

            send_to_char( "No estaria mal algo de sangre.\n\r", ch );

          }

          break;

	}

    }





    return;

}





/*

 * Put this in a seperate function so it isn't called three times per tick

 * This was added after a suggestion from Cronel	--Shaddai

 */



void check_alignment( CHAR_DATA *ch )
{

     /*

      *  Race alignment restrictions, h

      */

     if(ch->alignment<race_table[ch->race]->minalign)

     {

	set_char_color( AT_BLOOD, ch );

        send_to_char( "Tus acciones son incompatibles con los ideales de tu raza.", ch);

     }



     if(ch->alignment>race_table[ch->race]->maxalign)

     {

	set_char_color( AT_BLOOD, ch );

        send_to_char( "Tus acciones son incompatibles con los ideales de tu raza.", ch);

     }



     /*  alignment restrictions -h  */

      /* Vaya tonterias se os ocurren, ale quito esto KAYSER */
      /* (No hay que poner nada que quite o de mentalstate)*/
      /* Kayser no me tokes los kojones ademas esto es del smaug original so payaso */

	/* ke no ke?? porke no haces como ke te caes y me la xupas?*/

}



/*

 * Mob autonomous action.

 * This function takes 25% to 35% of ALL Mud cpu time.

 */

void mobile_update( void )

{

    char buf[MAX_STRING_LENGTH];

    CHAR_DATA *ch;

    EXIT_DATA *pexit;

    int door;

    ch_ret     retcode;



    retcode = rNONE;

  

    /* Examine all mobs. */

    for ( ch = last_char; ch; ch = gch_prev )

    {

	set_cur_char( ch );

	if ( ch == first_char && ch->prev )

	{

	    bug( "mobile_update: first_char->prev != NULL... fixed", 0 );

	    ch->prev = NULL;

	}



	gch_prev = ch->prev;



	if ( gch_prev && gch_prev->next != ch )

	{

	    sprintf( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",

		ch->name );

	    bug( buf, 0 );

	    bug( "Short-cutting here", 0 );

	    gch_prev = NULL;

	    ch->prev = NULL;

	    do_shout( ch, "KayserNazi dice, 'Preparate para lo peor!'" );

	}

	if ( !IS_NPC(ch) )

	{

	    drunk_randoms(ch);

	    hallucinations(ch);

	    continue;

            	}

	if ( !ch->in_room
	||   IS_AFFECTED(ch, AFF_CHARM)
	||   IS_AFFECTED(ch, AFF_PARALYSIS)
        ||   xIS_SET(ch->act, PLR_DOMINADO))
	    continue;



/* Clean up 'animated corpses' that are not charmed' - Scryn */





    if ( ( ch->pIndexData->vnum == 5 && !IS_AFFECTED(ch, AFF_CHARM)) 
	|| xIS_SET(ch->act, PLR_DOMINADO) )
	{

	  if(ch->in_room->first_person)

	    act(AT_MAGIC, "$n vuelve al polvo de donde $e vino.", ch, NULL, NULL, TO_ROOM);

   	  if(IS_NPC(ch)) /* Guard against purging switched? */

	    extract_char(ch, TRUE);

	  continue;

	}

	if ( !xIS_SET( ch->act, ACT_RUNNING )
	&&   !xIS_SET( ch->act, ACT_SENTINEL )
	&&   !ch->fighting && ch->hunting )

	{

	  WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

	  /* Commented out temporarily to avoid spam - Scryn

	  sprintf( buf, "%s esta cazando a %s desde %s.", ch->name,

	  	ch->hunting->name,

		ch->in_room->name );

	  log_string( buf ); */

	  hunt_victim( ch );

	  continue;

	}



	/* Examine call for special procedure */

	if ( !xIS_SET( ch->act, ACT_RUNNING )

	&&    ch->spec_fun )

	{

	    if ( (*ch->spec_fun) ( ch ) )

		continue;

	    if ( char_died(ch) )

		continue;

	}



	/* Check for mudprogram script on mob */

	if ( HAS_PROG( ch->pIndexData, SCRIPT_PROG ) )

	{

	    mprog_script_trigger( ch );

	    continue;

	}



	if ( ch != cur_char )

	{

	    bug( "Mobile_update: ch != cur_char after spec_fun", 0 );

	    continue;

	}



	/* That's all for sleeping / busy monster */

	if ( ch->position != POS_STANDING )
	    continue;



	if ( xIS_SET(ch->act, ACT_MOUNTED ) )

	{

	    if ( xIS_SET(ch->act, ACT_AGGRESSIVE)
	    ||   xIS_SET(ch->act, ACT_META_AGGR))
		do_emote( ch, "grunye y empieza a rugir." );
	    continue;

	}



	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE )
	&&  (xIS_SET(ch->act, ACT_AGGRESSIVE) || xIS_SET(ch->act, ACT_META_AGGR)) )
	    do_emote( ch, "mira a su alrededor y grunye." );





	/* MOBprogram random trigger */

	if ( ch->in_room->area->nplayer > 0 )

	{

	    mprog_random_trigger( ch );

	    if ( char_died(ch) )

		continue;

	    if ( ch->position < POS_STANDING )
	        continue;

	}



        /* MOBprogram hour trigger: do something for an hour */

        mprog_hour_trigger(ch);



	if ( char_died(ch) )

	  continue;



	rprog_hour_trigger(ch);

	if ( char_died(ch) )

	  continue;



	if ( ch->position < POS_STANDING )
	  continue;



	/* Scavenge */

	if ( xIS_SET(ch->act, ACT_SCAVENGER)

	&&   ch->in_room->first_content

	&&   number_bits( 2 ) == 0 )

	{

	    OBJ_DATA *obj;

	    OBJ_DATA *obj_best;

	    int max;



	    max         = 1;

	    obj_best    = NULL;

	    for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )

	    {

		if ( CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max

		&& !IS_OBJ_STAT( obj, ITEM_BURIED ) )

		{

		    obj_best    = obj;

		    max         = obj->cost;

		}

	    }



	    if ( obj_best )

	    {

		obj_from_room( obj_best );

		obj_to_char( obj_best, ch );

		act( AT_ACTION, "$n coge $p.", ch, obj_best, NULL, TO_ROOM );

	    }

	}



	/* Wander */

	if ( !xIS_SET(ch->act, ACT_RUNNING)

	&&   !xIS_SET(ch->act, ACT_SENTINEL)

	&&   !xIS_SET(ch->act, ACT_PROTOTYPE)

	&& ( door = number_bits( 5 ) ) <= 9

	&& ( pexit = get_exit(ch->in_room, door) ) != NULL

	&&   pexit->to_room

	&&   !IS_SET(pexit->exit_info, EX_CLOSED)

	&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)

	&&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)

	&& ( !xIS_SET(ch->act, ACT_STAY_AREA)

	||   pexit->to_room->area == ch->in_room->area ) )

	{



	    retcode = move_char( ch, pexit, 0 );

						/* If ch changes position due

						to it's or someother mob's

						movement via MOBProgs,

						continue - Kahn */

	    if ( char_died(ch) )

	      continue;

	    if ( retcode != rNONE || xIS_SET(ch->act, ACT_SENTINEL)

	    ||    ch->position < POS_STANDING )
	        continue;

	}



	/* Flee */

	if ( ch->hit < ch->max_hit / 2

	&& ( door = number_bits( 4 ) ) <= 9

	&& ( pexit = get_exit(ch->in_room,door) ) != NULL

	&&   pexit->to_room

	&&   !IS_SET(pexit->exit_info, EX_CLOSED)

	&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) )

	{

	    CHAR_DATA *rch;

	    bool found;



	    found = FALSE;

	    for ( rch  = ch->in_room->first_person;

		  rch;

		  rch  = rch->next_in_room )

	    {

		if ( is_fearing(ch, rch) )

		{

		    switch( number_bits(2) )

		    {

			case 0:
			  sprintf( buf, "Alejate de mi, %s!", rch->name );
			  break;
			case 1:
			  sprintf( buf, "Dejame marchar, %s!", rch->name );
			  break;
			case 2:
			  sprintf( buf, "Socorro! %s esta intentando matarme!", rch->name );
			  break;
			case 3:
			  sprintf( buf, "Que alguien me salve de %s!", rch->name );
			  break;
		    }
		    do_yell( ch, buf );
		    found = TRUE;
		    break;
		}
	    }
	    if ( found )
		retcode = move_char( ch, pexit, 0 );
	}
    }

    return;
}

/*
 * Actualizacion de Affects propios de usar disciplinas
 * VampiroMUD2.0b Email to vampiromud@hotmail.com
 */
void disciaffect_update( CHAR_DATA *ch )
{
   /* Acabemos con el npc bug */
   if( IS_NPC(ch))
   return;

    /* Forma Incorporea Taumaturgia nivel 3 */
    if( xIS_SET(ch->act, PLR_INCORPOREO))
        xREMOVE_BIT( ch->act, PLR_INCORPOREO);

    /* Corazon petreo Taumaturga nivel 4 */
    if( xIS_SET(ch->act, PLR_CORAZON_PETREO))
        xREMOVE_BIT( ch->act, PLR_CORAZON_PETREO);

    /* Dementacion Nivel 1 Pasion Update del affect */
    if( xIS_SET(ch->act, PLR_TRAUMATIZADO ))
                xREMOVE_BIT(ch->act, PLR_TRAUMATIZADO);

    /* Animalismo nivel 1 Susurros amables puedes controlar bestias y tal */
    /* O bien dominacion */
    if( xIS_SET(ch->act, PLR_DOMINADO ))
    {
        ch->pcdata->cnt_dominacion--;
        if( ch->pcdata->cnt_dominacion <= -1)
        {
                xREMOVE_BIT(ch->act, PLR_DOMINADO );
                ch->pcdata->cnt_dominacion = 0;
                act( AT_MAGIC, "Vuelves a sentirte duenyo de tus actos.\n\r", ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, "$n parece menos tonto.\n\r", ch, NULL, NULL, TO_ROOM );
        }
    }


return;
}






/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update( void )
{
    CHAR_DATA *ch;
    CHAR_DATA *ch_save;
    sh_int save_count = 0;
    ch_save	= NULL;
    for ( ch = last_char; ch; ch = gch_prev )
    {

	if ( ch == first_char && ch->prev )
	{
	    bug( "char_update: first_char->prev != NULL... fixed", 0 );
	    ch->prev = NULL;
	}

	gch_prev = ch->prev;
	set_cur_char( ch );
	if ( gch_prev && gch_prev->next != ch )
	{
	    bug( "char_update: ch->prev->next != ch", 0 );
	    return;
	}


/*
 * Chequeo de personajes con Flag Pktotal
 * Bajo pruebas
 * SiGo VampiroMud2.0 2001 email to vampiromud@hotmail.com
 */

		if( xIS_SET(ch->act, PLR_PKTOTAL))
		{
                        if (ch->cnt_pktotal > 0)
                 	       ch->cnt_pktotal--;
                        if (ch->cnt_pktotal < 1)
                        {
                                send_to_char( "&RDejas de ser PkTotal.\n\r", ch );
                                xREMOVE_BIT(ch->act,  PLR_PKTOTAL );
                                ch->cnt_pktotal = -1;

                        }
                }
/*
 * Update del estado "alterado"
 * SiGo VampiroMud2.0 2002
 */
               if( !IS_NPC(ch))
               {
                if( xIS_SET(ch->act, PLR_ALTERADO) )
                {
                        if( ch->pcdata->alterado > 0)
                                ch->pcdata->alterado--;
                        if( ch->pcdata->alterado < 1 )
                        {
                                send_to_char( "&RDejas de estar alterado.\n\r", ch );
                                xREMOVE_BIT(ch->act, PLR_ALTERADO );
                                ch->pcdata->alterado = -1;
                        }
                }
               }

/*
 * Update de los danyos agravados
 */
  if(ch->agravadas > 0)
  {
   if( ch->agravadas >= 50 )
   ch->agravadas -= number_range( 50, 500 );
   if( ch->agravadas < 50 )
   ch->agravadas = 0;
  }

    /*
     * Update de las restricciones de los puntos de quest por nivel
     */

     if( !IS_NPC(ch))
     if (( ch->pcdata->quest_accum > (50 * ch->level) ) && (ch->level < LEVEL_AVATAR))
	ch->pcdata->quest_accum = 50 * ch->level;


    /*
     * Update de diferenres affects especiales de las disciplinas
     */

    if(ES_AFECTADO(ch))
    	disciaffect_update( ch );

    /* Mensajes de advertencia de proximidad del dia a Vampiros */

    if((IS_VAMPIRE(ch)
    && !IS_SET(ch->in_room->room_flags, ROOM_DARK)))
    {
        if(time_info.hour == 5)
        send_to_char( "Ten cuidado el sol esta a punto de salir.\n\r", ch );
        if(time_info.hour == 6)
        send_to_char( "El sol acaba de salir... Te sientes mas debil.\n\r", ch );
        if(time_info.hour == 12)
        send_to_char( "El sol brilla en su maxima plenitud, busca un sitio oscuro y escondete.\n\r", ch );
        if(time_info.hour == 20)
        send_to_char( "El sol se pone... Te sientes mas poderoso!\n\r", ch );
    }

    if((IS_VAMPIRE(ch)
    && IS_SET(ch->in_room->room_flags, ROOM_DARK)))
    {
       if((time_info.hour < 6 ||
        time_info.hour > 20))
       ch->pcdata->descanso += 1;
    }

if((time_info.hour == 20 && IS_VAMPIRE(ch)))
ch->pcdata->descanso = 0;

        /*

	 *  Do a room_prog rand check right off the bat

	 *   if ch disappears (rprog might wax npc's), continue

	 */

/* Practicas negativas? */

if (ch->practice < 0)
	ch->practice = 0;
	
	
	if(!IS_NPC(ch))
	    rprog_random_trigger( ch );

	if( char_died(ch) )
	    continue;


/*
 * Para que guarde el coeficiete de cada jugador
 * SiGo Vampiromud2.0 2002 email to vampiromud@hotmail.com
 */
 if(!IS_NPC(ch))
 {
 if(!IS_VAMPIRE(ch))
 ch->pcdata->coeficiente = ((ch->max_hit + ch->max_move + ch->max_mana + ch->damroll + ch->hitroll)/ 10 );
 else
 ch->pcdata->coeficiente = ((ch->max_hit + ch->max_move + (ch->pcdata->condition[COND_BLOODTHIRST] * 100) + ch->damroll + ch->hitroll) / 10 );
 }





	if(IS_NPC(ch))
	    mprog_time_trigger(ch);


	if( char_died(ch) )
	    continue;

	rprog_time_trigger(ch);

	if( char_died(ch) )
	    continue;

        if( ch->tiempo_fv != -1 )
        {
          ch->tiempo_fv--;
          if (ch->tiempo_fv == 0 )
          {
           ch_printf(ch, "Puedes volver a entrenar la fuerza de voluntad.\n\r");
           ch->tiempo_fv = -1;
          }
        }

	/*

	 * See if player should be auto-saved.

	 */

	if ( !IS_NPC(ch)
	&& ( !ch->desc || ch->desc->connected == CON_PLAYING )
	&&    ch->level >= 2
	&&    current_time - ch->save_time > (sysdata.save_frequency*60) )
	    ch_save	= ch;
	else
	    ch_save	= NULL;

	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	}

	if ( ch->position == POS_STUNNED 
	|| ch->position == POS_PREDECAP )
	{
	    if( ch->hit < ch->max_hit )
			ch->hit += ((ch->max_hit * 10) / 100);

		if( ch->mana < ch->max_mana )
			ch->mana += ((ch->max_mana * 10) / 100);
		
		if( ch->move < ch->max_move )
			ch->move += ((ch->max_move * 10) / 100);
		
		update_pos( ch );
	}

        if ( ch->exp_acumulada < 0 )
        	ch->exp_acumulada = 0;
      /*   Morph timer expires */

      	if ( ch->morph ) {

           if ( ch->morph->timer > 0 )

	   {

            	    ch->morph->timer--;

           	    if ( ch->morph->timer == 0 )

            	   	 do_unmorph_char( ch );

	   }

  	}



        /* To make people with a nuisance's flags life difficult

         * --Shaddai

         */

        if ( !IS_NPC(ch) && ch->pcdata->nuisance )

        {

	    long int temp;



	    if ( ch->pcdata->nuisance->flags < MAX_NUISANCE_STAGE )

	    {

	     temp = ch->pcdata->nuisance->max_time-ch->pcdata->nuisance->time;

 	     temp *= ch->pcdata->nuisance->flags;

	     temp /= MAX_NUISANCE_STAGE;

	     temp += ch->pcdata->nuisance->time;

	     if ( temp < current_time )

		    ch->pcdata->nuisance->flags++;

	    }

        }



	if ( !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL )

	{

	    OBJ_DATA *obj;



	    if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL

	    &&   obj->item_type == ITEM_LIGHT

	    &&   obj->value[2] > 0 )

	    {

		if ( --obj->value[2] == 0 && ch->in_room )

		{

		    ch->in_room->light -= obj->count;

		    if (ch->in_room->light < 0 )

				ch->in_room->light=0;

		    act( AT_ACTION, "$p se apaga.", ch, obj, NULL, TO_ROOM );

		    act( AT_ACTION, "$p se apaga.", ch, obj, NULL, TO_CHAR );

		    if ( obj->serial == cur_obj )

		      global_objcode = rOBJ_EXPIRED;

		    extract_obj( obj );

		}

	    }




	    if ( ++ch->timer >= 12 )

	    {

		if ( !IS_IDLE( ch ) )

		{

		/*

		    ch->was_in_room = ch->in_room;

		    */

		    if ( ch->fighting )

			stop_fighting( ch, TRUE );

		    act( AT_ACTION, "$n desaparece en la nada.",

			ch, NULL, NULL, TO_ROOM );

		    send_to_char( "Desapareces en la nada.\n\r", ch );

		    if ( IS_SET( sysdata.save_flags, SV_IDLE ) )

			save_char_obj( ch );

		    SET_BIT(ch->pcdata->flags, PCFLAG_IDLE);

		    char_from_room( ch );

		    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );

		}

	    }



	    if ( ch->pcdata->condition[COND_FULL] > 1 )

	    {

		switch( ch->position )

		{

		    case POS_SLEEPING:  better_mental_state( ch, 4 );	break;

		    case POS_RESTING:   better_mental_state( ch, 3 );	break;

		    case POS_SITTING:

		    case POS_MOUNTED:   better_mental_state( ch, 2 );	break;

		    case POS_STANDING:  better_mental_state( ch, 1 );	break;

		    case POS_FIGHTING:

    		    case POS_EVASIVE:

    		    case POS_DEFENSIVE:

    		    case POS_AGGRESSIVE:

    		    case POS_BERSERK:

			if ( number_bits(2) == 0 )

			    better_mental_state( ch, 1 );

			break;

		}

	    }

	    if ( ch->pcdata->condition[COND_THIRST] > 1 )

	    {

		switch( ch->position )

		{

		    case POS_SLEEPING:  better_mental_state( ch, 5 );	break;

		    case POS_RESTING:   better_mental_state( ch, 3 );	break;

		    case POS_SITTING:

		    case POS_MOUNTED:   better_mental_state( ch, 2 );	break;

		    case POS_STANDING:  better_mental_state( ch, 1 );	break;

		    case POS_FIGHTING:

    		    case POS_EVASIVE:

    		    case POS_DEFENSIVE:

    		    case POS_AGGRESSIVE:

    		    case POS_BERSERK:

			if ( number_bits(2) == 0 )

			    better_mental_state( ch, 1 );

			break;

		}

	    }

	    /*

	     * Function added on suggestion from Cronel

	     */

	    check_alignment( ch );

	    gain_condition( ch, COND_DRUNK,  -1 );

	    gain_condition( ch, COND_FULL,  -1 + race_table[ch->race]->hunger_mod );



            if ( IS_VAMPIRE(ch)  && ch->level >= 10 )

	    {

		if ( time_info.hour < 21 && time_info.hour > 5 )

		  gain_condition( ch, COND_BLOODTHIRST, -1 );

	    }



	    if ( CAN_PKILL( ch ) && ch->pcdata->condition[COND_THIRST] - 9 > 10 )

	      gain_condition( ch, COND_THIRST, -9 );



	    if ( !IS_NPC( ch ) && ch->pcdata->nuisance )

	    {

		int value;



		value = ((0 - ch->pcdata->nuisance->flags)*ch->pcdata->nuisance->power);

		gain_condition ( ch, COND_THIRST, value );

		gain_condition ( ch, COND_FULL, --value );

	    }



	    if ( ch->in_room )

	      switch( ch->in_room->sector_type )

	      {

		default:

		    gain_condition( ch, COND_THIRST, -1 + race_table[ch->race]->thirst_mod);  break;

		case SECT_DESERT:

		    gain_condition( ch, COND_THIRST, -3 + race_table[ch->race]->thirst_mod);  break;

		case SECT_UNDERWATER:

		case SECT_OCEANFLOOR:

		    if ( number_bits(1) == 0 )

			gain_condition( ch, COND_THIRST, -1 + race_table[ch->race]->thirst_mod);  break;

	      }



	}

      if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->release_date > 0 &&

           ch->pcdata->release_date <= current_time )

      {

         ROOM_INDEX_DATA *location;

         if ( ch->pcdata->clan )

          location = get_room_index( ch->pcdata->clan->recall );

         else

          location = get_room_index(ROOM_VNUM_TEMPLE);

         if ( !location )

          location = ch->in_room;

         MOBtrigger = FALSE;

         char_from_room(ch);

         char_to_room( ch, location );

         send_to_char( "Los dioses te han desterrado al infierno!\n\r", ch );

         do_look(ch, "auto");

         STRFREE( ch->pcdata->helled_by );

	 ch->pcdata->helled_by = NULL;

         ch->pcdata->release_date = 0;

         save_char_obj( ch );

      }



	if ( !char_died(ch) )

	{

	    /*

	     * Careful with the damages here,

	     *   MUST NOT refer to ch after damage taken, without checking

	     *   return code and/or char_died as it may be lethal damage.

	     */

	    if ( IS_AFFECTED(ch, AFF_POISON) )

	    {

		act( AT_POISON, "$n tiembla y sufre.", ch, NULL, NULL, TO_ROOM );

		act( AT_POISON, "Tiemblas y sufres.", ch, NULL, NULL, TO_CHAR );

		damage( ch, ch, 6, gsn_poison );

	    }

	    else

	    if ( ch->position == POS_INCAP )

		damage( ch, ch, 2, TYPE_UNDEFINED );

	    else

	    if ( ch->position == POS_MORTAL )

		damage( ch, ch, 4, TYPE_UNDEFINED );

	    if ( char_died(ch) )

		continue;



	    /*
	     * Recurring spell affect
	     */

 	    if ( IS_AFFECTED(ch, AFF_RECURRINGSPELL) )
	    {
		AFFECT_DATA *paf, *paf_next;
		SKILLTYPE *skill;
		bool found = FALSE, died = FALSE;

		for ( paf = ch->first_affect; paf; paf = paf_next )
		{
		    paf_next = paf->next;
		    if ( paf->location == APPLY_RECURRINGSPELL )
		    {
			found = TRUE;
			if ( IS_VALID_SN(paf->modifier)
			&&  (skill=skill_table[paf->modifier]) != NULL
         &&   skill->type == SKILL_SPELL )
			{
			    if ( (*skill->spell_fun)(paf->modifier, ch->level, ch, ch) == rCHAR_DIED
			    ||   char_died(ch) )
			    {
				died = TRUE;
				break;
			    }
			}
		    }
		}
		if ( died )
		    continue;
		if ( !found )
		    xREMOVE_BIT(ch->affected_by, AFF_RECURRINGSPELL);
	    }


	    if ( ch->mental_state >= 30 && !IS_IMMORTAL(ch) )

		switch( (ch->mental_state+5) / 10 )

		{

		    case  3:

		    	send_to_char( "Te sientes raro.\n\r", ch );

			act( AT_ACTION, "$n parece otro.", ch, NULL, NULL, TO_ROOM );

		    	break;

		    case  4:

		    	send_to_char( "No te sientes del todo bien.\n\r", ch );

			act( AT_ACTION, "$n no parece encontrarse del todo bien.", ch, NULL, NULL, TO_ROOM );

		    	break;

		    case  5:

		    	send_to_char( "Necesitas ayuda!\n\r", ch );

			act( AT_ACTION, "$n parece que necesita ayuda.", ch, NULL, NULL, TO_ROOM );

		    	break;

		    case  6:

		    	send_to_char( "Buscate a un clerigo.\n\r", ch );

			act( AT_ACTION, "Alguien deberia ir a buscar un curandero para $n.", ch, NULL, NULL, TO_ROOM );

		    	break;

		    case  7:

		    	send_to_char( "Te sientes realmente ido...\n\r", ch );

			act( AT_ACTION, "$n no parece que sepa que esta haciendo.", ch, NULL, NULL, TO_ROOM );

		    	break;

		    case  8:

		    	send_to_char( "Lo empiezas a entender... todo.\n\r", ch );

			act( AT_ACTION, "$n empieza a comportarse como un loco!", ch, NULL, NULL, TO_ROOM );

		    	break;

		    case  9:

		    	send_to_char( "Eres UNO con el universo.\n\r", ch );

			act( AT_ACTION, "$n cree conocer 'la respuesta', esta bastante ido...", ch, NULL, NULL, TO_ROOM );

		    	break;

		    case 10:

		    	send_to_char( "Sientes que el fin se acerca.\n\r", ch );

			act( AT_ACTION, "$n esta llegando a su fin...", ch, NULL, NULL, TO_ROOM );

		    	break;

		}

	    if ( ch->mental_state <= -30 && ch->level < LEVEL_AVATAR)

		switch( (abs(ch->mental_state)+5) / 10 )

		{

		    case  10:

			if ( ch->position > POS_SLEEPING )

			{

			   if ( (ch->position == POS_STANDING

			   ||    ch->position < POS_FIGHTING)

			   &&    number_percent()+10 < abs(ch->mental_state) )

				do_sleep( ch, "" );

			   else

				send_to_char( "Apenas puedes permanecer consciente.\n\r", ch );

			}

			break;

		    case   9:

			if ( ch->position > POS_SLEEPING )

			{

			   if ( (ch->position == POS_STANDING

			   ||    ch->position < POS_FIGHTING)

			   &&   (number_percent()+20) < abs(ch->mental_state) )

				do_sleep( ch, "" );

			   else

				send_to_char( "Apenas puedes permanecer con los ojos abiertos.\n\r", ch );

			}

			break;

		    case   8:

			if ( ch->position > POS_SLEEPING )

			{

			   if ( ch->position < POS_SITTING

			   &&  (number_percent()+30) < abs(ch->mental_state) )

				do_sleep( ch, "" );

			   else

				send_to_char( "Tienes MUCHO suenyo.\n\r", ch );

			}

			break;

		    case   7:

			if ( ch->position > POS_RESTING )

			   send_to_char( "Te sientes desmotivado.\n\r", ch );

			break;

		    case   6:

			if ( ch->position > POS_RESTING )

			   send_to_char( "Te sientes sedado.\n\r", ch );

			break;

		    case   5:

			if ( ch->position > POS_RESTING )

			   send_to_char( "Te sientes muy cansado.\n\r", ch );

			break;

		    case   4:

			if ( ch->position > POS_RESTING )

			   send_to_char( "Te sientes cansado.\n\r", ch );

			break;

		    case   3:

			if ( ch->position > POS_RESTING )

			   send_to_char( "Deberias descansar.\n\r", ch );

			break;

		}

	    if ( ch->timer > 24 )

		do_quit( ch, "" );

	    else

	    if ( ch == ch_save && IS_SET( sysdata.save_flags, SV_AUTO )

	    &&   ++save_count < 10 )	/* save max of 10 per tick */

		save_char_obj( ch );

	}

    }



    return;

}







/*

 * Update all objs.

 * This function is performance sensitive.

 */

void obj_update( void )
{
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    sh_int AT_TEMP;

    for ( obj = last_object; obj; obj = gobj_prev )

    {

	CHAR_DATA *rch;
    //CHAR_DATA *pj;

	char *message;



	if ( obj == first_object && obj->prev )

	{

	    bug( "obj_update: first_object->prev != NULL... fixed", 0 );

	    obj->prev = NULL;

	}

	gobj_prev = obj->prev;

	if ( gobj_prev && gobj_prev->next != obj )

	{

	    bug( "obj_update: obj->prev->next != obj", 0 );

	    return;

	}

	set_cur_obj( obj );

	if ( obj->carried_by )
	  oprog_random_trigger( obj );
	else
	if( obj->in_room && obj->in_room->area->nplayer > 0 )
	  oprog_random_trigger( obj );

        if( obj_extracted(obj) )
	  continue;

       if( IS_OBJ_STAT( obj, ITEM_QUEST ))
       {
          for( victim = first_char; victim; victim = victim->next )
          if(!IS_NPC(victim))
          {
          if( !str_cmp( victim->name, obj->propietario ))
          {
          if( obj->carried_by && str_cmp( obj->propietario, obj->carried_by->name ) )
          quest_reclamar( victim, obj );

          if( obj->in_obj && xIS_SET( obj->in_obj->extra_flags, ITEM_TAQUILLA ) && str_cmp( obj->in_obj->name, obj->propietario ))
          quest_reclamar( victim, obj );

          if( obj->in_room )
          quest_reclamar( victim, obj );
          }
          }
       }



	if ( obj->item_type == ITEM_PIPE )

	{

	    if ( IS_SET( obj->value[3], PIPE_LIT ) )

	    {

		if ( --obj->value[1] <= 0 )

		{

		  obj->value[1] = 0;

		  REMOVE_BIT( obj->value[3], PIPE_LIT );

		}

		else

		if ( IS_SET( obj->value[3], PIPE_HOT ) )

		  REMOVE_BIT( obj->value[3], PIPE_HOT );

		else

		{

		  if ( IS_SET( obj->value[3], PIPE_GOINGOUT ) )

		  {

		    REMOVE_BIT( obj->value[3], PIPE_LIT );

		    REMOVE_BIT( obj->value[3], PIPE_GOINGOUT );

		  }

		  else

		    SET_BIT( obj->value[3], PIPE_GOINGOUT );

		}

		if ( !IS_SET( obj->value[3], PIPE_LIT ) )

		  SET_BIT( obj->value[3], PIPE_FULLOFASH );

	    }

	    else

	      REMOVE_BIT( obj->value[3], PIPE_HOT );

	}





/* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */



        if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )

        {

          sh_int timerfrac = UMAX(1, obj->timer - 1);

          if ( obj->item_type == ITEM_CORPSE_PC )

	    timerfrac = (int)(obj->timer / 8 + 1);



	  if ( obj->timer > 0 && obj->value[2] > timerfrac )

	  {

            char buf[MAX_STRING_LENGTH];

            char name[MAX_STRING_LENGTH];

            char *bufptr;

            bufptr = one_argument( obj->short_descr, name );

            bufptr = one_argument( bufptr, name );

            bufptr = one_argument( bufptr, name );



	    separate_obj(obj);

            obj->value[2] = timerfrac;

            sprintf( buf, corpse_descs[ UMIN( timerfrac - 1, 4 ) ],

                          bufptr );



            STRFREE( obj->description );

            obj->description = STRALLOC( buf );

          }

        }



	/* don't let inventory decay */

	if ( IS_OBJ_STAT(obj, ITEM_INVENTORY) )
	  continue;



	/* groundrot items only decay on the ground */

	if ( IS_OBJ_STAT(obj, ITEM_GROUNDROT) && !obj->in_room )

	  continue;



	if ( ( obj->timer <= 0 || --obj->timer > 0 ) )

	  continue;



	/* if we get this far, object's timer has expired. */



         AT_TEMP = AT_PLAIN;

	 switch ( obj->item_type )

	 {

	 default:

	   message = "$p misteriosamente desaparece.";

           AT_TEMP = AT_PLAIN;

	   break;

	 case ITEM_CONTAINER:

	   message = "$p se cae al suelo y se deshace...era demasiado viejo.";

	   AT_TEMP = AT_OBJECT;

	   break;

	 case ITEM_PORTAL:

	   message = "$p desaparece misteriosamente.";

           remove_portal(obj);

	   obj->item_type = ITEM_TRASH;		/* so extract_obj	 */

           AT_TEMP = AT_MAGIC;			/* doesn't remove_portal */

	   break;

	 case ITEM_FOUNTAIN:

	   message = "$p se seca.";

           AT_TEMP = AT_BLUE;

	   break;

	 case ITEM_CORPSE_NPC:

	   message = "$p se convierte en polvo.";

           AT_TEMP = AT_OBJECT;

	   break;

	 case ITEM_CORPSE_PC:

	   message = "$p se convierte en polvo.";

           AT_TEMP = AT_MAGIC;

	   break;

	 case ITEM_COOK:

	 case ITEM_FOOD:

	   message = "$p se descompone en el suelo.";

           AT_TEMP = AT_HUNGRY;

	   break;

         case ITEM_BLOOD:

           message = "$p se seca lentamente en el suelo.";

           AT_TEMP = AT_BLOOD;

           break;

         case ITEM_BLOODSTAIN:

           message = "$p se seca lentamente en el suelo.";

           AT_TEMP = AT_BLOOD;

	   break;

         case ITEM_SCRAPS:

           message = "$p se deshace en la nada.";

           AT_TEMP = AT_OBJECT;

	   break;

	 case ITEM_FIRE:

	   /* This is removed because it is done in obj_from_room

	    * Thanks to gfinello@mail.karmanet.it for pointing this out.

	    * --Shaddai

	   if (obj->in_room)

	   {

	     --obj->in_room->light;

	     if ( obj->in_room->light < 0 )

			obj->in_room->light = 0;

	   }

	   */

	   message = "$p se quema.";

	   AT_TEMP = AT_FIRE;

	 }



	if ( obj->carried_by )

	{

	    act( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );

	}

	else if ( obj->in_room
	&&      ( rch = obj->in_room->first_person ) != NULL
	&&	!IS_OBJ_STAT( obj, ITEM_BURIED ) )
	{
	    act( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
            act( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
	}

	if ( obj->serial == cur_obj )
	  global_objcode = rOBJ_EXPIRED;
	extract_obj( obj );
    }
    return;

}





/*
 * Chequeo de personajes abrazados e instruidos
 * Bajo pruebas
 * SiGo VampiroMud2.0 2001 email to vampiromud@hotmail.com
 */

void abrazo_update( void )
{
/* Funcion llamada por update_handler() por pulse_area */

        CHAR_DATA *ch, *ch_next;

        for ( ch = first_char; ch != NULL; ch = ch_next )
        {
                ch_next = ch->next;

                        if (IS_NPC(ch) )
                        continue;

                        if (ch->cnt_abrazo >=1)
                        {
                                ch->cnt_abrazo --;
                                if (ch->cnt_abrazo == 0)
                                {
                                send_to_char( "No has aceptado beber la sangre ofrecida y mueres agonicamente.\n\r", ch );
                                xREMOVE_BIT(ch->act,  PLR_ABRAZADO );
                                raw_kill( ch, ch );
                                ch->cnt_abrazo = -1;
                                return;
                                }
                        }
        }
        return;
 }

/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check( void )
{
    CHAR_DATA *ch, *ch_next;
    OBJ_DATA *obj;
    EXIT_DATA *pexit;
    static int cnt = 0;
    int door, retcode;

    /* This little counter can be used to handle periodic events */
    cnt = (cnt+1) % SECONDS_PER_TICK;

    for ( ch = first_char; ch; ch = ch_next )
    {
	set_cur_char(ch);
	ch_next = ch->next;
	will_fall(ch, 0);

	if ( char_died( ch ) )
	  continue;

	if ( IS_NPC( ch ) )
	{

	    if ( (cnt & 1) )

		continue;



	    /* running mobs	-Thoric */

	    if ( xIS_SET(ch->act, ACT_RUNNING) )

	    {

		if ( !xIS_SET(ch->act, ACT_SENTINEL)

		&&   !ch->fighting && ch->hunting )

		{

		    WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

		    hunt_victim( ch );

		    continue;

		}



		if ( ch->spec_fun )

		{

		    if ( (*ch->spec_fun) ( ch ) )

			continue;

		    if ( char_died(ch) )

			continue;

		}



		if ( !xIS_SET(ch->act, ACT_SENTINEL)

		&&   !xIS_SET(ch->act, ACT_PROTOTYPE)

		&& ( door = number_bits( 4 ) ) <= 9

		&& ( pexit = get_exit(ch->in_room, door) ) != NULL

		&&   pexit->to_room

		&&   !IS_SET(pexit->exit_info, EX_CLOSED)

		&&   !IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)

		&&   !IS_SET(pexit->to_room->room_flags, ROOM_DEATH)

		&& ( !xIS_SET(ch->act, ACT_STAY_AREA)

		||   pexit->to_room->area == ch->in_room->area ) )

		{

		    retcode = move_char( ch, pexit, 0 );

		    if ( char_died(ch) )

			continue;

		    if ( retcode != rNONE || xIS_SET(ch->act, ACT_SENTINEL)

		    ||    ch->position < POS_STANDING )

			continue;

		}

	    }

	    continue;

	}

	else

	{

	    if ( ch->mount

	    &&   ch->in_room != ch->mount->in_room )

	    {

		xREMOVE_BIT( ch->mount->act, ACT_MOUNTED );

		ch->mount = NULL;

		ch->position = POS_STANDING;

		send_to_char( "Al no estar encima de tu montura... caes al suelo.\n\rOUCH!\n\r", ch );

	    }



	    if ( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER )

	    || ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )

	    {

		if ( !IS_AFFECTED( ch, AFF_AQUA_BREATH ) )

		{

		    if ( ch->level < LEVEL_IMMORTAL )

		    {

			long dam;

        		/* Changed level of damage at Brittany's request. -- Narn */

			dam = number_range( ch->max_hit / 100, ch->max_hit / 50 );

			dam = UMAX( 1, dam );

			if ( number_bits(3) == 0 )

			  send_to_char( "Toses y te atragantas intentando respirar agua!\n\r", ch );

			damage( ch, ch, dam, TYPE_UNDEFINED );

		    }

		}

	    }



	    if ( char_died( ch ) )

		continue;



	    if ( ch->in_room

	    && (( ch->in_room->sector_type == SECT_WATER_NOSWIM )

	    ||  ( ch->in_room->sector_type == SECT_WATER_SWIM ) ) )

	    {

		if ( !IS_AFFECTED( ch, AFF_FLYING )

		&& !IS_AFFECTED( ch, AFF_FLOATING )

		&& !IS_AFFECTED( ch, AFF_AQUA_BREATH )

		&& !ch->mount )

		{

		    for ( obj = ch->first_carrying; obj; obj = obj->next_content )

		      if ( obj->item_type == ITEM_BOAT ) break;



		    if ( !obj )

		    {

			if ( ch->level < LEVEL_IMMORTAL )

			{

			    int mov;

			    long dam;



			    if ( ch->move > 0 )

			    {

				mov = number_range( ch->max_move / 20, ch->max_move / 5 );

				mov = UMAX( 1, mov );



				if ( ch->move - mov < 0 )

				    ch->move = 0;

				else

				    ch->move -= mov;

			    }

			    else

			    {

				dam = number_range( ch->max_hit / 20, ch->max_hit / 5 );

				dam = UMAX( 1, dam );



				if ( number_bits(3) == 0 )

				   send_to_char( "Victima del cansancio, tragas un poco de agua....\n\r", ch );

				damage( ch, ch, dam, TYPE_UNDEFINED );

			    }

			}

		    }

		}

	    }



	    /* beat up on link dead players */

	    if ( !ch->desc )

	    {

		CHAR_DATA *wch, *wch_next;



		for ( wch = ch->in_room->first_person; wch; wch = wch_next )

		{

		    wch_next	= wch->next_in_room;



		    if (!IS_NPC(wch)
		    ||   wch->fighting
		    ||   IS_AFFECTED(wch, AFF_CHARM)
                    ||   xIS_SET(ch->act, PLR_DOMINADO)
		    ||   !IS_AWAKE(wch)
		    || (xIS_SET(wch->act, ACT_WIMPY) && IS_AWAKE(ch) )
		    ||   !can_see( wch, ch ) )
			continue;



		    if ( is_hating( wch, ch ) )

		    {

			found_prey( wch, ch );

			continue;

		    }



		    if ( (!xIS_SET(wch->act, ACT_AGGRESSIVE)
		    &&    !xIS_SET(wch->act, ACT_META_AGGR))
		    ||    xIS_SET(wch->act, ACT_MOUNTED)
		    ||    IS_SET(wch->in_room->room_flags, ROOM_SAFE )
                    ||    xIS_SET(wch->afectado_por, DAF_SILENCIO_MORTAL) )
			continue;

		    global_retcode = multi_hit( wch, ch, TYPE_UNDEFINED );

		}

	    }

	}

    }

}





/*

 * Aggress.

 *

 * for each descriptor

 *     for each mob in room

 *         aggress on some random PC

 *

 * This function should take 5% to 10% of ALL mud cpu time.

 * Unfortunately, checking on each PC move is too tricky,

 *   because we don't the mob to just attack the first PC

 *   who leads the party into the room.

 *

 */

void aggr_update( void )

{

    DESCRIPTOR_DATA *d, *dnext;

    CHAR_DATA *wch;

    CHAR_DATA *ch;

    CHAR_DATA *ch_next;

    CHAR_DATA *vch;

    CHAR_DATA *vch_next;

    CHAR_DATA *victim;

    struct act_prog_data *apdtmp;



#ifdef UNDEFD

  /*

   *  GRUNT!  To do

   *

   */

        if ( IS_NPC( wch ) && wch->mpactnum > 0

	    && wch->in_room->area->nplayer > 0 )

        {

            MPROG_ACT_LIST * tmp_act, *tmp2_act;

	    for ( tmp_act = wch->mpact; tmp_act;

		 tmp_act = tmp_act->next )

	    {

                 oprog_wordlist_check( tmp_act->buf,wch, tmp_act->ch,

				      tmp_act->obj, tmp_act->vo, ACT_PROG );

                 DISPOSE( tmp_act->buf );

            }

	    for ( tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act )

	    {

                 tmp2_act = tmp_act->next;

                 DISPOSE( tmp_act );

            }

            wch->mpactnum = 0;

            wch->mpact    = NULL;

        }

#endif



    /* check mobprog act queue */

    while ( (apdtmp = mob_act_list) != NULL )

    {

	wch = mob_act_list->vo;

	if ( !char_died(wch) && wch->mpactnum > 0 )

	{

	    MPROG_ACT_LIST * tmp_act;



	    while ( (tmp_act = wch->mpact) != NULL )

	    {

		if ( tmp_act->obj && obj_extracted(tmp_act->obj) )

		  tmp_act->obj = NULL;

		if ( tmp_act->ch && !char_died(tmp_act->ch) )

		  mprog_wordlist_check( tmp_act->buf, wch, tmp_act->ch,

					tmp_act->obj, tmp_act->vo, ACT_PROG );

		wch->mpact = tmp_act->next;

		DISPOSE(tmp_act->buf);

		DISPOSE(tmp_act);

	    }

	    wch->mpactnum = 0;

	    wch->mpact    = NULL;

        }

	mob_act_list = apdtmp->next;

	DISPOSE( apdtmp );

    }





    /*

     * Just check descriptors here for victims to aggressive mobs

     * We can check for linkdead victims in char_check	-Thoric

     */

    for ( d = first_descriptor; d; d = dnext )

    {

	dnext = d->next;

	if ( d->connected != CON_PLAYING || (wch=d->character) == NULL )

	   continue;



	if ( char_died(wch)

	||   IS_NPC(wch)

	||   wch->level >= LEVEL_IMMORTAL

	||  !wch->in_room )

	    continue;



	for ( ch = wch->in_room->first_person; ch; ch = ch_next )

	{

	    int count;



	    ch_next	= ch->next_in_room;



	    if ( !IS_NPC(ch)
	    ||   ch->fighting
	    ||   IS_AFFECTED(ch, AFF_CHARM)
            ||   xIS_SET(ch->act, PLR_DOMINADO)
	    ||   !IS_AWAKE(ch)
	    ||   (xIS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	    ||   !can_see( ch, wch ) )
		continue;



	    if ( is_hating(ch, wch) )

	    {

		found_prey(ch, wch);

		continue;

	    }



	    if ( (!xIS_SET(ch->act, ACT_AGGRESSIVE)
	    &&    !xIS_SET(ch->act, ACT_META_AGGR))
	    ||     xIS_SET(ch->act, ACT_MOUNTED)
	    ||     IS_SET(ch->in_room->room_flags, ROOM_SAFE )
            ||     xIS_SET(ch->afectado_por, DAF_SILENCIO_MORTAL) )
		continue;



	    /*
	     * Ok we have a 'wch' player character and a 'ch' npc aggressor.
	     * Now make the aggressor fight a RANDOM pc victim in the room,
	     *   giving each 'vch' an equal chance of selection.
	     *
	     * Depending on flags set, the mob may attack another mob
	     */
	    count	= 0;
	    victim	= NULL;
	    for ( vch = wch->in_room->first_person; vch; vch = vch_next )
	    {
  vch_next = vch->next_in_room;

		if ( (!IS_NPC(vch) || xIS_SET(ch->act, ACT_META_AGGR)
		||    xIS_SET(vch->act, ACT_ANNOYING))
		&&   vch->level < LEVEL_IMMORTAL
		&&   (!xIS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
		&&   can_see( ch, vch ) )
		{
		    if ( number_range( 0, count ) == 0 )
			victim = vch;
		    count++;
		}
	    }

	    if ( !victim )
	    {
		bug( "Aggr_update: null victim.", count );
		continue;
	    }

	    /* backstabbing mobs (Thoric) */
	    if ( IS_NPC(ch) && xIS_SET(ch->attacks, ATCK_BACKSTAB ) )
	    {

		OBJ_DATA *obj;



		if ( !ch->mount

    		&& (obj = get_eq_char( ch, WEAR_WIELD )) != NULL

    		&& (obj->value[3] == 11 || obj->value[3] == 2)

		&& !victim->fighting

		&& victim->hit >= victim->max_hit )

		{

		    check_attacker( ch, victim );

		    WAIT_STATE( ch, skill_table[gsn_backstab]->beats );

		    if ( !IS_AWAKE(victim)

		    ||   number_percent( )+5 < ch->level )

		    {

			global_retcode = multi_hit( ch, victim, gsn_backstab );

			continue;

		    }

		    else

		    {

			global_retcode = damage( ch, victim, 0, gsn_backstab );

			continue;

		    }

		}

	    }

	    global_retcode = multi_hit( ch, victim, TYPE_UNDEFINED );

	}

    }



    return;

}



/* From interp.c */

bool check_social  args( ( CHAR_DATA *ch, char *command, char *argument ) );



/*

 * drunk randoms	- Tricops

 * (Made part of mobile_update	-Thoric)

 */

void drunk_randoms( CHAR_DATA *ch )

{

    CHAR_DATA *rvch = NULL;

    CHAR_DATA *vch;

    sh_int drunk;

    sh_int position;



    if ( IS_NPC( ch ) || ch->pcdata->condition[COND_DRUNK] <= 0 )

	return;



    if ( number_percent() < 30 )

	return;



    drunk = ch->pcdata->condition[COND_DRUNK];

    position = ch->position;

    ch->position = POS_STANDING;



    if ( number_percent() < (2*drunk / 20) )

	check_social( ch, "eruptar", "" );

    else

    if ( number_percent() < (2*drunk / 20) )

	check_social( ch, "bailar", "" );

    else

    if ( number_percent() < (2*drunk / 20) )

	check_social( ch, "desternillarse", "" );

    else

    if ( number_percent() < (2*drunk / 20) )

	check_social( ch, "cantar", "" );

    else

    if ( drunk > (10+(get_curr_con(ch)/5))

    &&   number_percent() < ( 2 * drunk / 18 ) )

    {

	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )

	    if ( number_percent() < 10 )

		rvch = vch;

	check_social( ch, "vomitar", (rvch ? rvch->name : "") );

    }



    ch->position = position;

    return;

}



/*

 * Random hallucinations for those suffering from an overly high mentalstate

 * (Hats off to Albert Hoffman's "problem child")	-Thoric

 */

void hallucinations( CHAR_DATA *ch )

{

    if ( ch->mental_state >= 30 && number_bits(5 - (ch->mental_state >= 50) - (ch->mental_state >= 75)) == 0 )
    {
	char *t;

 switch( number_range( 1, UMIN(21, (ch->mental_state+5) / 5)) )
	{
	    default:
	    case  1: t = "Te sientes impaciente... no puedes estarte quieto.\n\r";		break;
	    case  2: t = "Te estremece el entorno.\n\r";				break;
	    case  3: t = "Tu piel se esta cayendo.\n\r";					break;
	    case  4: t = "Te repente sientes que algo no va bien.\n\r";	break;
	    case  5: t = "Esos malditos duendes se estan riendo de ti!\n\r";		break;
	    case  6: t = "Puedes oir a tu madre llorar...\n\r";			break;
	    case  7: t =  "&R[  10000  ]&w El backstab de &WAlguien te &R<&Y<&G< &PDeJa &GEn EsTaDo &WCriTiCo &G>&Y>&R>&w !  \n\r&R[  10000  ]&w El backstab de &WAlguien te &R<&Y<&G< &PDeJa &GEn EsTaDo &WCriTiCo &G>&Y>&R>&w !  \n\r&R[  10000  ]&w El backstab de &WAlguien te &R<&Y<&G< &PDeJa &GEn EsTaDo &WCriTiCo &G>&Y>&R>&w !  \n\r&REstas MUERTO!!!\n\r";	break;
	    case  8: t = "Dolorosos recuerdos de la infancia invaden tu mente.\n\r";	break;
	    case  9: t = "Oyes como alguien pronuncia tu nombre en la distancia...\n\r";	break;
            case 10: t = "Tu cabeza esta dando vueltas... no puedes pensar claro.\n\r";	break;
            case 11: t = "El suelo... parece que se mueve...\n\r";			break;
          case 12: t = "No estas seguro de lo que es real y de lo que no lo es.\n\r";		break;
	    case 13: t = "Es todo un suenyo... o es real?\n\r";				break;
	    case 14: t = "Oyes a tus nietos llamandote para que los cuides.\n\r";	break;
       case 15: t = "Estas viniendo a por ti... quieren llevarte contigo...\n\r";   break;
	    case 16: t = "Te sientes poderoso!\n\r";				break;
	    case 17: t = "Te sientes ligero... sientes que puedes tocar el cielo.\n\r";	break;
	    case 18: t = "&R[  10000  ]&w El backstab de &WAlguien te &R<&Y<&G< &PDeJa &GEn EsTaDo &WCriTiCo &G>&Y>&R>&w !  \n\r&R[  10000  ]&w El backstab de &WAlguien te &R<&Y<&G< &PDeJa &GEn EsTaDo &WCriTiCo &G>&Y>&R>&w !  \n\r&R[  10000  ]&w El backstab de &WAlguien te &R<&Y<&G< &PDeJa &GEn EsTaDo &WCriTiCo &G>&Y>&R>&w !  \n\r&REstas MUERTO!!!\n\r";	break;
	    case 19: t = "Al mirar hacia abajo ves tu propio cuerpo... eres un angel!\n\r";	break;
	    case 20: t = "Te sientes inmortal!\n\r";					break;
	    case 21: t = "Ahh... el poder de un dios... que hacer ahora...\n\r";	break;
	}
	send_to_char( t, ch );

    }

    return;

}



void tele_update( void )

{

    TELEPORT_DATA *tele, *tele_next;



    if ( !first_teleport )

      return;



    for ( tele = first_teleport; tele; tele = tele_next )

    {

	tele_next = tele->next;

	if ( --tele->timer <= 0 )

	{

	    if ( tele->room->first_person )

	    {

		if ( IS_SET( tele->room->room_flags, ROOM_TELESHOWDESC ) )

		  teleport( tele->room->first_person, tele->room->tele_vnum,

			TELE_SHOWDESC | TELE_TRANSALL );

		else

		  teleport( tele->room->first_person, tele->room->tele_vnum,

			TELE_TRANSALL );

	    }

	    UNLINK( tele, first_teleport, last_teleport, next, prev );

	    DISPOSE( tele );

	}

    }

}



#if FALSE

/*

 * Write all outstanding authorization requests to Log channel - Gorog

 */

void auth_update( void )

{

  CHAR_DATA *victim;

  DESCRIPTOR_DATA *d;

  char log_buf [MAX_INPUT_LENGTH];

  bool first_time = TRUE;         /* so titles are only done once */



  for ( d = first_descriptor; d; d = d->next )

      {

      victim = d->character;

      if ( victim && IS_WAITING_FOR_AUTH(victim) )

         {

         if ( first_time )

            {

            first_time = FALSE;

            strcpy (log_buf, "Autorizaciones pendientes:" );

            /*log_string( log_buf ); */

            to_channel( log_buf, CHANNEL_AUTH, "Auth", 1);

            }

         sprintf( log_buf, " %s@%s nuevo %s %s", victim->name,

            victim->desc->host, race_table[victim->race]->race_name,

            class_table[victim->class]->who_name );

/*         log_string( log_buf ); */

         to_channel( log_buf, CHANNEL_AUTH, "Auth", 1);

         }

      }

}

#endif



void auth_update( void )

{

    CHAR_DATA *victim;

    DESCRIPTOR_DATA *d;

    char buf [MAX_INPUT_LENGTH], log_buf [MAX_INPUT_LENGTH];

    bool found_hit = FALSE;         /* was at least one found? */



    strcpy (log_buf, "Pending authorizations:\n\r" );

    for ( d = first_descriptor; d; d = d->next )

    {

	if ( (victim = d->character) && IS_WAITING_FOR_AUTH(victim) )

	{

	    found_hit = TRUE;

	    sprintf( buf, " %s@%s new %s %s\n\r", victim->name,

		victim->desc->host, race_table[victim->race]->race_name,

		class_table[victim->class]->who_name );

	    strcat (log_buf, buf);

	}

    }

    if (found_hit)

    {

/*	log_string( log_buf ); */

	to_channel( log_buf, CHANNEL_AUTH, "Auth", 1);

    }

}



/*

 * Handle all kinds of updates.

 * Called once per pulse from game loop.

 * Random times to defeat tick-timing clients and players.

 */

void update_handler( void )
{
    static  int     pulse_start_arena = PULSE_ARENA;  /* pulsaciones de lucha en arena */
    static  int     pulse_arena = PULSE_ARENA;
    static  int     pulse_area;
    static  int     pulse_mobile;
    static  int     pulse_violence;
    static  int     pulse_point;
    static  int	    pulse_second;
    struct timeval stime;
    struct timeval etime;

    if(in_start_arena || ppl_challenged)
    if( --pulse_start_arena <= 0)
    {
      pulse_start_arena = PULSE_ARENA;
      start_arena();
    }

    if(ppl_in_arena)
    if(( --pulse_arena <= 0) || (num_in_arena()==1))
    {
      pulse_arena = PULSE_ARENA;
      do_game();
    }

    if ( timechar )
    {
      set_char_color(AT_PLAIN, timechar);
      send_to_char( "Empezando a actualizar.\n\r", timechar );
      gettimeofday(&stime, NULL);
    }

    if ( --pulse_area     <= 0 )
    {
	pulse_area	= number_range( PULSE_AREA / 2, 3 * PULSE_AREA / 2 );
        area_update	( );
        quest_update ( );
        abrazo_update( );
    }

    if ( --pulse_mobile   <= 0 )
    {
                  pulse_mobile	= PULSE_MOBILE;
	mobile_update  ( );
    }

    if ( --pulse_violence <= 0 )
    {
	pulse_violence	= PULSE_VIOLENCE;
	violence_update	( );
    }

    if ( --pulse_point    <= 0 )
    {
	pulse_point     = number_range( PULSE_TICK * 0.75, PULSE_TICK * 1.25 );

        auth_update     ( );			/* Gorog */
        time_update	( );
	weather_update	( );
	char_update	( );
	obj_update	( );
	clear_vrooms	( );			/* remove virtual rooms */
    }

    if ( --pulse_second   <= 0 )
    {
	pulse_second	= PULSE_PER_SECOND;
	char_check( );
 	reboot_check(0);
    }

    if ( --auction->pulse <= 0 )
    {
	auction->pulse = PULSE_AUCTION;
	auction_update( );
    }

    tele_update( );
    aggr_update( );
    obj_act_update ( );
    room_act_update( );
    clean_obj_queue();		/* dispose of extracted objects */
    clean_char_queue();		/* dispose of dead mobs/quitting chars */
    if ( timechar )
    {
      gettimeofday(&etime, NULL);
      set_char_color(AT_PLAIN, timechar);
      send_to_char( "Actualizacion completa.\n\r", timechar );
      subtract_times(&etime, &stime);
      ch_printf( timechar, "Ha tardado %d.%06d segundos.\n\r",
          etime.tv_sec, etime.tv_usec );
      timechar = NULL;
    }
    tail_chain( );
    return;
}


void remove_portal( OBJ_DATA *portal )
{
    ROOM_INDEX_DATA *fromRoom, *toRoom;
    EXIT_DATA *pexit;
    bool found;

    if ( !portal )
    {
	bug( "remove_portal: portal is NULL", 0 );
	return;
    }

    fromRoom = portal->in_room;
    found = FALSE;
    if ( !fromRoom )
    {
	bug( "remove_portal: portal->in_room is NULL", 0 );
	return;
    }



    for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )

	if ( IS_SET( pexit->exit_info, EX_PORTAL ) )

	{

	    found = TRUE;

	    break;

	}



    if ( !found )

    {

	bug( "remove_portal: portal not found in room %d!", fromRoom->vnum );

	return;

    }



    if ( pexit->vdir != DIR_PORTAL )

	bug( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );



    if ( ( toRoom = pexit->to_room ) == NULL )

      bug( "remove_portal: toRoom is NULL", 0 );



    extract_exit( fromRoom, pexit );



    return;

}



void reboot_check( time_t reset )

{

  static char *tmsg[] =

  { "Sientes como el suelo tiembla y el fin se acerca!",

    "Relampagos iluminan el cielo!",

    "El sonido de truenos avecina una gran tormenta!",

    "El cielo oscurece de repente.",

    "De pronto las criaturas a tu alrededor van haciendose mas y mas pequenyas.",

    "El mar que atraviesa el reino se ha vuelto frigido.",

    "El aura magica que envolvia al reino parece muy inestable.",

    "Sientes un cambio en las fuerzas magicas que te envuelven."

  };

  static const int times[] = { 60, 120, 180, 240, 300, 600, 900, 1800 };

  static const int timesize =

      UMIN(sizeof(times)/sizeof(*times), sizeof(tmsg)/sizeof(*tmsg));

  char buf[MAX_STRING_LENGTH];

  static int trun;

  static bool init = FALSE;



  if ( !init || reset >= current_time )

  {

    for ( trun = timesize-1; trun >= 0; trun-- )

      if ( reset >= current_time+times[trun] )

        break;

    init = TRUE;

    return;

  }



  if ( (current_time % 1800) == 0 )

  {

    sprintf(buf, "%.24s: %d jugadores", ctime(&current_time), num_descriptors);

    append_to_file(USAGE_FILE, buf);

    sprintf(buf, "%.24s:  %dptn  %dpll  %dsc %dbr  %d global loot",

	ctime(&current_time),

	sysdata.upotion_val,

	sysdata.upill_val,

	sysdata.scribed_used,

	sysdata.brewed_used,

	sysdata.global_looted );

    append_to_file( ECONOMY_FILE, buf );

  }



  if ( new_boot_time_t - boot_time < 60*60*18 &&

      !set_boot_time->manual )

    return;



  if ( new_boot_time_t <= current_time )

  {

    CHAR_DATA *vch;

    extern bool mud_down;



    if ( auction->item )

    {

      sprintf(buf, "La venta de %s ha sido interrumpida por el mud.",

          auction->item->short_descr);

      talk_auction(buf);

      obj_to_char(auction->item, auction->seller);

      auction->item = NULL;

      if ( auction->buyer && auction->buyer != auction->seller )

      {

        auction->buyer->gold += auction->bet;

        send_to_char("Se te ha devuelto el dinero.\n\r", auction->buyer);

      }

    }

    echo_to_all(AT_YELLOW, "Una fuerza magica te fuerza a abandonar este reino "

        "\n\rmientras la vida aqui es reconstruida.", ECHOTAR_ALL);

    log_string( "Reboot automatico" );

    for ( vch = first_char; vch; vch = vch->next )

      if ( !IS_NPC(vch) )

        save_char_obj(vch);

    mud_down = TRUE;

    return;

  }



  if ( trun != -1 && new_boot_time_t - current_time <= times[trun] )

  {

    echo_to_all(AT_YELLOW, tmsg[trun], ECHOTAR_ALL);

    if ( trun <= 5 )

      sysdata.DENY_NEW_PLAYERS = TRUE;

    --trun;

    return;

  }

  return;

}



/* the auction update*/



void auction_update (void)

{

    int tax, pay;

    char buf[MAX_STRING_LENGTH];



    if(!auction->item)

    {

    	if(AUCTION_MEM > 0 && auction->history[0] &&

    			++auction->hist_timer == 6*AUCTION_MEM)

    	{

    		int i;



    		for(i = AUCTION_MEM - 1; i >= 0; i--)

    		{

    			if(auction->history[i])

    			{

    				auction->history[i] = NULL;

    				auction->hist_timer = 0;

    				break;

    			}

    		}

    	}

    	return;

    }



    switch (++auction->going) /* increase the going state */

    {

	case 1 : /* going once */

	case 2 : /* going twice */

	    if (auction->bet > auction->starting)

		sprintf (buf, "%s: va %s por %s.", auction->item->short_descr,

			((auction->going == 1) ? "a la de una" : "a la de dos"), num_punct( auction->bet ) );

	    else

		sprintf (buf, "%s: va %s (ofertas no recibidas todavia).",  auction->item->short_descr,

			((auction->going == 1) ? "a la de una" : "a la de dos"));



	    talk_auction (buf);

	    break;



	case 3 : /* SOLD! */

	    if (!auction->buyer && auction->bet)

	    {

		bug( "Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet );

		auction->bet = 0;

	    }

	    if (auction->bet > 0 && auction->buyer != auction->seller)

	    {

		sprintf (buf, "%s vendido a %s por %s.",

			auction->item->short_descr,

			IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name,

			num_punct(auction->bet) );

		talk_auction(buf);



		act(AT_ACTION, "El subastador se materializa ante ti y te da $p.",

			auction->buyer, auction->item, NULL, TO_CHAR);

		act(AT_ACTION, "El subastador se materializa ante $n, y le da $m $p.",

			auction->buyer, auction->item, NULL, TO_ROOM);



		if ( (auction->buyer->carry_weight

		+     get_obj_weight( auction->item ))

		>     can_carry_w( auction->buyer ) )

		{

		    act( AT_PLAIN, "$p es demasiado pesado para llevarlo en tu inventario.", auction->buyer, auction->item, NULL, TO_CHAR );

    		    act( AT_PLAIN, "$n lleva demasiado peso para que pueda llevar $p, asi que $e lo deja.", auction->buyer, auction->item, NULL, TO_ROOM );

		    obj_to_room( auction->item, auction->buyer->in_room );

		}

		else

		    obj_to_char( auction->item, auction->buyer );

	        pay = (int)auction->bet * 0.9;

		tax = (int)auction->bet * 0.1;

		boost_economy( auction->seller->in_room->area, tax );

                auction->seller->gold += pay; /* give him the money, tax 10 % */

		sprintf(buf, "El subastador te da %s monedas de oro, cobrandote un impuesto de %s.\n\r",

		  num_punct(pay),

		  num_punct(tax) );

		send_to_char(buf, auction->seller);

                auction->item = NULL; /* reset item */

		if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )

		{

		    save_char_obj( auction->buyer );

		    save_char_obj( auction->seller );

		}

            }

            else /* not sold */

            {

                sprintf (buf, "No hay ofertas para %s - objeto extraido de la subasta.\n\r",auction->item->short_descr);

                talk_auction(buf);

                act (AT_ACTION, "El subastador aparece ante ti y te devuelve $p.",

                      auction->seller,auction->item,NULL,TO_CHAR);

                act (AT_ACTION, "El subastador aparece ante $n y le devuelve $p a $m.",

                      auction->seller,auction->item,NULL,TO_ROOM);

		if ( (auction->seller->carry_weight

		+     get_obj_weight( auction->item ))

		>     can_carry_w( auction->seller ) )

		{

		    act( AT_PLAIN, "Dejas $p en el suelo al no poder cargarlo.", auction->seller,

			auction->item, NULL, TO_CHAR );

		    act( AT_PLAIN, "$n deja $p en el suelo al no poder cargarlo.", auction->seller,

			auction->item, NULL, TO_ROOM );

		    obj_to_room( auction->item, auction->seller->in_room );

		}

		else

		    obj_to_char (auction->item,auction->seller);

		tax = (int)auction->item->cost * 0.05;

		boost_economy( auction->seller->in_room->area, tax );

		sprintf(buf, "El subastador te cobra un impuesto de %s.\n\r", num_punct(tax) );

		send_to_char(buf, auction->seller);

		if ((auction->seller->gold - tax) < 0)

		  auction->seller->gold = 0;

		else

		  auction->seller->gold -= tax;

		if ( IS_SET( sysdata.save_flags, SV_AUCTION ) )

		    save_char_obj( auction->seller );

	    } /* else */

	    auction->item = NULL; /* clear auction */

    } /* switch */

} /* func */



void subtract_times(struct timeval *etime, struct timeval *stime)

{

  etime->tv_sec -= stime->tv_sec;

  etime->tv_usec -= stime->tv_usec;

  while ( etime->tv_usec < 0 )

  {

    etime->tv_usec += 1000000;

    etime->tv_sec--;

  }

  return;

}



/*

 * Function to update weather vectors according to climate

 * settings, random effects, and neighboring areas.

 * Last modified: July 18, 1997

 * - Fireblade

 */

void adjust_vectors(WEATHER_DATA *weather)

{

	NEIGHBOR_DATA *neigh;

	double dT, dP, dW;



	if(!weather)

	{

		bug("adjust_vectors: NULL weather data.", 0);

		return;

	}



	dT = 0;

	dP = 0;

	dW = 0;



	/* Add in random effects */

	dT += number_range(-rand_factor, rand_factor);

	dP += number_range(-rand_factor, rand_factor);

	dW += number_range(-rand_factor, rand_factor);



	/* Add in climate effects*/

	dT += climate_factor *

		(((weather->climate_temp - 2)*weath_unit) -

		(weather->temp))/weath_unit;

	dP += climate_factor *

		(((weather->climate_precip - 2)*weath_unit) -

		(weather->precip))/weath_unit;

	dW += climate_factor *

		(((weather->climate_wind - 2)*weath_unit) -

		(weather->wind))/weath_unit;





	/* Add in effects from neighboring areas */

	for(neigh = weather->first_neighbor; neigh;

			neigh = neigh->next)

	{

		/* see if we have the area cache'd already */

		if(!neigh->address)

		{

			/* try and find address for area */

			neigh->address = get_area(neigh->name);



			/* if couldn't find area ditch the neigh */

			if(!neigh->address)

			{

				NEIGHBOR_DATA *temp;

				bug("adjust_weather: "

					"invalid area name.", 0);

				temp = neigh->prev;

				UNLINK(neigh,

				       weather->first_neighbor,

				       weather->last_neighbor,

				       next,

				       prev);

				STRFREE(neigh->name);

				DISPOSE(neigh);

				neigh = temp;

				continue;

			}

		}



		dT +=(neigh->address->weather->temp -

		      weather->temp) / neigh_factor;

		dP +=(neigh->address->weather->precip -

		      weather->precip) / neigh_factor;

		dW +=(neigh->address->weather->wind -

		      weather->wind) / neigh_factor;

	}



	/* now apply the effects to the vectors */

	weather->temp_vector += (int)dT;

	weather->precip_vector += (int)dP;

	weather->wind_vector += (int)dW;



	/* Make sure they are within the right range */

	weather->temp_vector = URANGE(-max_vector,

		weather->temp_vector, max_vector);

	weather->precip_vector = URANGE(-max_vector,

		weather->precip_vector, max_vector);

	weather->wind_vector = URANGE(-max_vector,

		weather->wind_vector, max_vector);



	return;

}



/*

 * function updates weather for each area

 * Last Modified: July 31, 1997

 * Fireblade

 */

void weather_update()

{

	AREA_DATA *pArea;

	DESCRIPTOR_DATA *d;

	int limit;



	limit = 3 * weath_unit;



	for(pArea = first_area; pArea;

		pArea = (pArea == last_area) ? first_build : pArea->next)

	{

		/* Apply vectors to fields */

		pArea->weather->temp +=

			pArea->weather->temp_vector;

		pArea->weather->precip +=

			pArea->weather->precip_vector;

		pArea->weather->wind +=

			pArea->weather->wind_vector;



		/* Make sure they are within the proper range */

		pArea->weather->temp = URANGE(-limit,

			pArea->weather->temp, limit);

		pArea->weather->precip = URANGE(-limit,

			pArea->weather->precip, limit);

		pArea->weather->wind = URANGE(-limit,

			pArea->weather->wind, limit);



		/* get an appropriate echo for the area */

		get_weather_echo(pArea->weather);

	}



	for(pArea = first_area; pArea;

		pArea = (pArea == last_area) ? first_build : pArea->next)

	{

		adjust_vectors(pArea->weather);

	}



	/* display the echo strings to the appropriate players */

	for(d = first_descriptor; d; d = d->next)

	{

		WEATHER_DATA *weath;



		if(d->connected == CON_PLAYING &&

			IS_OUTSIDE(d->character) &&

			!NO_WEATHER_SECT(d->character->in_room->sector_type) &&

			IS_AWAKE(d->character))

		{

			weath = d->character->in_room->area->weather;

			if(!weath->echo)

				continue;

			set_char_color(weath->echo_color, d->character);

			ch_printf(d->character, weath->echo);

		}

	}



	return;

}



/*

 * get weather echo messages according to area weather...

 * stores echo message in weath_data.... must be called before

 * the vectors are adjusted

 * Last Modified: August 10, 1997

 * Fireblade

 */

void get_weather_echo(WEATHER_DATA *weath)

{

	int n;

	int temp, precip, wind;

	int dT, dP, dW;

	int tindex, pindex, windex;



	/* set echo to be nothing */

	weath->echo = NULL;

	weath->echo_color = AT_GREY;



	/* get the random number */

	n = number_bits(2);



	/* variables for convenience */

	temp = weath->temp;

	precip = weath->precip;

	wind = weath->wind;



	dT = weath->temp_vector;

	dP = weath->precip_vector;

	dW = weath->wind_vector;



	tindex = (temp + 3*weath_unit - 1)/weath_unit;

	pindex = (precip + 3*weath_unit - 1)/weath_unit;

	windex = (wind + 3*weath_unit - 1)/weath_unit;



	/* get the echo string... mainly based on precip */

	switch(pindex)

	{

		case 0:

			if(precip - dP > -2*weath_unit)

			{

				char *echo_strings[4] =

				{

					"Las nubes desaparecen.\n\r",

					"Las nubes desaparecen.\n\r",

					"El cielo empieza a "

						"despejarse de nubes.\n\r",

					"Las nuves lentamente van "

						"desapareciendo del cielo.\n\r"

				};



				weath->echo = echo_strings[n];

				weath->echo_color = AT_WHITE;

			}

			break;



		case 1:

			if(precip - dP <= -2*weath_unit)

			{

				char *echo_strings[4] =

				{

					"El cielo se esta nublando.\n\r",

					"El cielo se esta nublando.\n\r",

					"El cielo se esta cubriendo "

						"de nubes.\n\r",

					"El cielo se esta cubriendo de "

						"nubes.\n\r"

				};

				weath->echo = echo_strings[n];

				weath->echo_color = AT_GREY;

			}

			break;



		case 2:

			if(precip - dP > 0)

			{

				if(tindex > 1)

				{

					char *echo_strings[4] =

					{

						"La lluvia ha cesado.\n\r",

						"La lluvia ha cesado.\n\r",

						"Deja de llover.\n\r",

						"Deja de llover.\n\r"

					};

					weath->echo = echo_strings[n];

					weath->echo_color = AT_CYAN;

				}

				else

				{

					char *echo_strings[4] =

					{

						"Deja de nevar.\n\r",

						"Deja de nevar.\n\r",

						"Esta dejando de nevar.\n\r",

						"Esta dejando de nevar.\n\r"

					};

					weath->echo = echo_strings[n];

					weath->echo_color = AT_WHITE;

				}

			}

			break;



		case 3:

			if(precip - dP <= 0)

			{

				if(tindex > 1)

				{

					char *echo_strings[4] =

					{

						"Empieza a llover.\n\r",

						"Empieza a llover.\n\r",

						"Ha empezado a llover "

							"intensamente.\n\r",

						"Ha empezado a llover "

							"intensamente.\n\r"

					};

					weath->echo = echo_strings[n];

					weath->echo_color = AT_CYAN;

				}

				else

				{

					char *echo_strings[4] =

					{

						"Empieza a nevar.\n\r",

						"Empieza a nevar.\n\r",

						"Copos de nieve empiezan a caer "

							"del cielo.\n\r",

						"Oooooh... esta empezando a nevar :)\n\r"

					};

					weath->echo = echo_strings[n];

					weath->echo_color = AT_WHITE;

				}

			}

			else if(tindex < 2 && temp - dT > -weath_unit)

			{

				char *echo_strings[4] =

				{

					"La temperatuta baja y la lluvia "

						"empieza a convertirse granizo.\n\r",

					"Ves como la lluvia empieza  "

						"a transformarse en granizo.\n\r",

					"Empieza a granizar intensamente!\n\r",

					"Ves como empiezan a caer copos de nieve "

						"con la lluvia.\n\r"

				};

				weath->echo = echo_strings[n];

				weath->echo_color = AT_WHITE;

			}

			else if(tindex > 1 && temp - dT <= -weath_unit)

			{

				char *echo_strings[4] =

				{

					"Los copos de nieve empiezan a ser "

						"remplazados por gotas de lluvia.\n\r",

					"Los copos de nieve empiezan a ser "

						"remplazados por gotas de lluvia.\n\r",

					"La nieve empieza a convertirse en lluvia.\n\r",

					"La nieve empieza a convertirse en lluvia.\n\r"

				};

				weath->echo = echo_strings[n];

				weath->echo_color = AT_CYAN;

			}

			break;



		case 4:

			if(precip - dP > 2*weath_unit)

			{

				if(tindex > 1)

				{

					char *echo_strings[4] =

					{

						"Deja de relampaguear.\n\r",

						"Deja de relampaguear.\n\r",

						"Deja de tronar y el cielo se tranquiliza.\n\r",

						"La tormenta amaina.\n\r"

					};

					weath->echo = echo_strings[n];

					weath->echo_color = AT_GREY;

				}

			}

			else if(tindex < 2 && temp - dT > -weath_unit)

			{

				char *echo_strings[4] =

				{

					"La fria lluvia se transforma en nieve.\n\r",

					"La fria lluvia se transforma en nieve.\n\r",

					"Ves como la lluvia se empieza a transformar "

						"en nieve.\n\r",

					"La fria lluvia empieza a congelarse y transformarse en nieve.\n\r"

				};

				weath->echo = echo_strings[n];

				weath->echo_color = AT_WHITE;

			}

			else if(tindex > 1 && temp - dT <= -weath_unit)

			{

				char *echo_strings[4] =

				{

					"La nieve se transforma en una fria lluvia.\n\r",

					"La nieve se transforma en una fria lluvia.\n\r",

					"Ves como se empieza a transformar  "

						"la nieve en lluvia.\n\r",

					"La nieve es lentamente replazada por una fuerte "

						"lluvia.\n\r"

				};

				weath->echo = echo_strings[n];

				weath->echo_color = AT_CYAN;

			}

			break;



		case 5:

			if(precip - dP <= 2*weath_unit)

			{

				if(tindex > 1)

				{

					char *echo_strings[4] =

					{

						"Relampagos iluminan el "

							"cielo.\n\r",

						"Relampagos iluminan el "

							"cielo.\n\r",

						"Rayos y truenos inundan "

							"el cielo.\n\r",

						"Relampagos iluminan el cielo.\n\r"

					};

					weath->echo = echo_strings[n];

					weath->echo_color = AT_YELLOW;

				}

			}

			else if(tindex > 1 && temp - dT <= -weath_unit)

			{

				char *echo_strings[4] =

				{

					"El cielo ruge con los truenos mientras "

						"la nieve se transforma en lluvia.\n\r",

					"El cielo ruge con los truenos mientras "

						"la nieve se transforma en lluvia.\n\r",

					"El cielo ruge con los truenos mientras "

						"la nieve se transforma en lluvia.\n\r",

					"El cielo ruge con los truenos mientras "

						"la nieve se transforma en lluvia.\n\r",

				};

				weath->echo = echo_strings[n];

				weath->echo_color = AT_WHITE;

			}

			else if(tindex < 2 && temp - dT > -weath_unit)

			{

				char *echo_strings[4] =

				{

					"Los rayos cesan a la vez que la lluvia "

						"se transforma en una fuerte ventisca de "

						"nieve.\n\r",

					"Los rayos cesan a la vez que la lluvia "

						"se transforma en una fuerte ventisca de "

						"nieve.\n\r",

					"Los rayos cesan a la vez que la lluvia "

						"se transforma en una fuerte ventisca de "

						"nieve.\n\r",

					"Los rayos cesan a la vez que la lluvia "

						"se transforma en una fuerte ventisca de "

						"nieve.\n\r",

				};

				weath->echo = echo_strings[n];

				weath->echo_color = AT_CYAN;

			}

			break;



		default:

			bug("echo_weather: invalid precip index");

			weath->precip = 0;

			break;

	}



	return;

}



/*

 * get echo messages according to time changes...

 * some echoes depend upon the weather so an echo must be

 * found for each area

 * Last Modified: August 10, 1997

 * Fireblade

 */

void get_time_echo(WEATHER_DATA *weath)
{

	int n;
	int pindex;

	n = number_bits(2);
	pindex = (weath->precip + 3*weath_unit - 1)/weath_unit;
	weath->echo = NULL;
	weath->echo_color = AT_GREY;

	switch(time_info.hour)

	{

		case 5:
		{
			char *echo_strings[4] =
			{
				"Empieza un nuevo dia.\n\r",
				"Empieza un nuevo dia.\n\r",
				"Empieza un nuevo dia.\n\r",
				"Empieza un nuevo dia.\n\r"
			};


			time_info.sunlight = SUN_RISE;
			weath->echo = echo_strings[n];
			weath->echo_color = AT_YELLOW;
			break;
		}
		case 6:
		{
			char *echo_strings[4] =
			{
				"El sol sale por el horizonte.\n\r",
				"El sol sale por el horizonte.\n\r",
				"El sol sale por el horizonte.\n\r",
				"El sol sale por el horizonte.\n\r",
			};

			time_info.sunlight = SUN_LIGHT;

			weath->echo = echo_strings[n];

			weath->echo_color = AT_ORANGE;

			break;

		}

		case 12:

		{

			if(pindex > 0)

			{

				weath->echo = "Es mediodia.\n\r";

			}

			else

			{

				char *echo_strings[2] =

				{

					"La intensidad del sol "

						"anuncia la llegada del mediodia.\n\r",

					"La intensidad del sol anuncia "

						"la llegada del mediodia.\n\r"

				};

				weath->echo = echo_strings[n%2];

			}

			time_info.sunlight = SUN_LIGHT;

			weath->echo_color = AT_WHITE;

			break;

		}

		case 19:

		{

			char *echo_strings[4] =

			{

				"El sol desaparece lentamente por el horizonte.\n\r",

				"El sol desaparece lentamente por el horizonte.\n\r",

				"El sol desaparece lentamente por el horizonte.\n\r",

				"El sol desaparece lentamente por el horizonte.\n\r",

			};

			time_info.sunlight = SUN_SET;

			weath->echo = echo_strings[n];

			weath->echo_color = AT_RED;

			break;

		}

		case 20:

		{

			if(pindex > 0)

			{

				char *echo_strings[2] =
				{
					"La noche empieza.\n\r",
					"El crepusculo desciende a tu alrededor.\n\r"
				};
				weath->echo = echo_strings[n%2];
			}

			else

			{

				char *echo_strings[2] =

				{

					"La luna brilla en la oscuridad "

						"de la noche.\n\r",

					"LA luna brilla en la oscuridad "

						"de la noche.\n\r"

				};

				weath->echo = echo_strings[n%2];

			}

			time_info.sunlight = SUN_DARK;

			weath->echo_color = AT_DBLUE;

			break;

		}

	}



	return;

}



/*
 * update the time
 */
void time_update()
{
	AREA_DATA *pArea;
	DESCRIPTOR_DATA *d;
	WEATHER_DATA *weath;

 switch(++time_info.hour)
	{
		case 5:
		case 6:
                case 10:
		case 12:
		case 19:
		case 20:
			for(pArea = first_area; pArea;
				pArea = (pArea == last_area) ? first_build : pArea->next)
			{
				get_time_echo(pArea->weather);
			}

			for(d = first_descriptor; d; d = d->next)
			{
				if(d->connected == CON_PLAYING &&
					IS_OUTSIDE(d->character) &&
					IS_AWAKE(d->character))
				{
					weath = d->character->in_room->area->weather;
					if(!weath->echo)
						continue;
					set_char_color(weath->echo_color,
						d->character);
					ch_printf(d->character, weath->echo);
				}

			}

			break;

		case 24:
			time_info.hour = 0;
                        time_info.day++;
			break;

     }

	if(time_info.day >= 30)
	{
		time_info.day = 0;
		time_info.month++;
	}

	if(time_info.month >= 17)
	{
		time_info.month = 0;
		time_info.year++;
	}
	return;
}

void advance_generacion( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    char outbuf[MAX_STRING_LENGTH];

if ( IS_VAMPIRE (ch))
  {
     ch->generacion--;

    switch ( ch->generacion)
    {
      case MAX_GENERACION :
               sprintf( outbuf, "NeoNaTo" );
               break;
               case GENERACION_DECIMOSEGUNDA :
               sprintf( outbuf, "DoCeaVa GeNeRaCioN" );
               break;
               case GENERACION_DECIMOPRIMERA :
               sprintf( outbuf, "DeCiMoPRiMeRa GeNeRaCioN" );
               break;
               case GENERACION_DECIMA :
               sprintf( outbuf, "DeCiMa GeNeRaCioN" );
               break;
               case GENERACION_NOVENA :
               sprintf( outbuf, "NoVeNa GeNeRaCioN" );
               break;
               case GENERACION_OCTAVA :
               sprintf( outbuf, "OcTaVa GeNeRaCioN" );
               break;
               case GENERACION_SEPTIMA :
               sprintf( outbuf, "SepTiMa GeNeRaCioN" );
               break;
               case GENERACION_SEXTA :
               sprintf( outbuf, "SexTa GeNeRaCioN" );
               break;
               case GENERACION_QUINTA :
               sprintf( outbuf, "QuiNTa GeNeRaCioN" );
               break;
               case GENERACION_MATUSALEN :
               sprintf( outbuf, "MaTuSaLeN" );
               break;
               case GENERACION_ANTEDILUVIANO :
               sprintf( outbuf, "aNTeDiLuViano" );
               break;
               case GENERACION_SEGUNDA :
               sprintf( outbuf, "SeGuNDa GeNeRaCioN" );
               break;
               case GENERACION_CAIN :
               sprintf( outbuf, "PRiMeRa GeNeRaCioN(Cain)" );
               break;
               }

              sprintf( buf, "%s ha subido a %s", ch->name, outbuf );
              mensa_todos( ch, "generacion", buf);
              ch_printf( ch, "&gOle tus huevos!!! Has subido a &R%s&G!!!\n\r", outbuf);
              return;

 return;
 }

if ( !IS_VAMPIRE (ch) )
 {

    ch->generacion--;

           send_to_char("Los Dioses han ascendido tu rango!!!\n\r", ch );

           switch ( ch->generacion )
            {
               case MAX_GENERACION :
               sprintf( outbuf, "Novato" );
               break;
               case GENERACION_DECIMOSEGUNDA :
               sprintf( outbuf, "Aprendiz" );
               break;
               case GENERACION_DECIMOPRIMERA :
               sprintf( outbuf, "Estudiante" );
               break;
               case GENERACION_DECIMA :
               sprintf( outbuf, "Iluminado" );
               break;
               case GENERACION_NOVENA :
               sprintf( outbuf, "Sabio" );
               break;
               case GENERACION_OCTAVA :
               sprintf( outbuf, "Maestro" );
               break;
               case GENERACION_SEPTIMA :
               sprintf( outbuf, "Experto" );
               break;
               case GENERACION_SEXTA :
               sprintf( outbuf, "Inhumano" );
               break;
               case GENERACION_QUINTA :
               sprintf( outbuf, "Celestial" );
               break;
               case GENERACION_MATUSALEN :
               sprintf( outbuf, "Destructor" );
               break;
               case GENERACION_ANTEDILUVIANO :
               sprintf( outbuf, "Divinidad" );
               break;
               case GENERACION_SEGUNDA :
               sprintf( outbuf, "Legendario" );
               break;
               case GENERACION_CAIN :
               sprintf( outbuf, "HeRaLDo" );
               break;
            }
     sprintf( buf, "%s ha subido al rango de %s", ch->name, outbuf );
     mensa_todos( ch, "rango", buf);

     return;
   }

/* Diferentes casos segun la generacion del personaje  Los
 * antediluvianos segunda generacion y primera generacion
 * no ganan stats.*//*SiGo*/

return;
}
void do_generacionup( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;

    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    set_char_color( AT_IMMORT, ch );

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' || arg2[0] == '\0' || is_number( arg2 ) ) {
        send_to_char( "Sintaxis:  geneup <jugador> <mejorar>/<empeorar>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Ese jugador no esta en la habitacion.\n\r", ch);
        return;
    }

    if ( IS_NPC( victim ) ) {
        send_to_char( "No puedes mejorar la generacion de un Mob.\n\r", ch );
        return;
    }

    if ( get_trust( ch ) <= get_trust( victim ) && ch != victim ) {
        send_to_char( "No puedes hacer eso.\n\r", ch );
        return;
    }

    /* Empeoramiento de la Generacion */

   if ( !str_prefix( arg2, "empeorar"  ) )
    {

    if ( victim->generacion == MAX_GENERACION )
    {
      send_to_char("No puedes empeorar mas su generacion, para eso matalo.\n\r", ch );
      return;
    }

        send_to_char( "Los Dioses han empeorado tu Generacion!!!\n\r", victim );
        degradacion_generacion_pk( victim );
  return;
 }
    /* Mejorando la generacion del personaje */
    if ( !str_prefix( arg2, "mejorar"  ) )
    {

    if ( victim->generacion == GENERACION_CAIN )
    {
      send_to_char("No puede mejorar mas su Generacion, el ya es tan poderoso como Cain.\n\r", ch );
     return;
    }

   if ( !IS_VAMPIRE ( victim ) )
    {
      send_to_char("Los Dioses han mejorado tu Rango!!!\n\r", victim );
      advance_generacion ( victim );
         return;
    }

   if ( IS_VAMPIRE (victim) )
   {
      send_to_char("Los Dioses han mejorado tu Generacion!!!\n\r", victim );
      advance_generacion ( victim );
   return;
   }

}
return;
}



void degradacion_generacion_pk(CHAR_DATA *victim)
{
   char buf[MAX_STRING_LENGTH];
   char outbuf[MAX_STRING_LENGTH];

    victim->generacion++;

    if ( !IS_VAMPIRE(victim) )
    {
       switch ( victim->generacion )
       {
               case MAX_GENERACION :
               sprintf( outbuf, "Novato" );
               break;
               case GENERACION_DECIMOSEGUNDA :
               sprintf( outbuf, "Aprendiz" );
               break;
               case GENERACION_DECIMOPRIMERA :
               sprintf( outbuf, "Estudiante" );
               break;
               case GENERACION_DECIMA :
               sprintf( outbuf, "Iluminado" );
               break;
               case GENERACION_NOVENA :
               sprintf( outbuf, "Sabio" );
               break;
               case GENERACION_OCTAVA :
               sprintf( outbuf, "Maestro" );
               break;
               case GENERACION_SEPTIMA :
               sprintf( outbuf, "Experto" );
               break;
               case GENERACION_SEXTA :
               sprintf( outbuf, "Inhumano" );
               break;
               case GENERACION_QUINTA :
               sprintf( outbuf, "Celestial" );
               break;
               case GENERACION_MATUSALEN :
               sprintf( outbuf, "Destructor" );
               break;
               case GENERACION_ANTEDILUVIANO :
               sprintf( outbuf, "Divinidad" );
               break;
               case GENERACION_SEGUNDA :
               sprintf( outbuf, "Legendario" );
               break;
               case GENERACION_CAIN :
               sprintf( outbuf, "HeRaLDo" );
               break;
            }
              sprintf( buf, "%s ha bajado al rango de %s", victim->name, outbuf );
              mensa_todos( victim, "rango", buf);
              ch_printf( victim, "&gMierda!!! Has bajado a Rango de &R%s&g!!!\n\r", outbuf);
              return;
      }

              if ( IS_VAMPIRE(victim) )
              {

               switch ( victim->generacion )
               {
               case MAX_GENERACION :
               sprintf( outbuf, "NeoNaTo" );
               break;
               case GENERACION_DECIMOSEGUNDA :
               sprintf( outbuf, "DoCeaVa GeNeRaCioN" );
               break;
               case GENERACION_DECIMOPRIMERA :
               sprintf( outbuf, "DeCiMoPRiMeRa GeNeRaCioN" );
               break;
               case GENERACION_DECIMA :
               sprintf( outbuf, "DeCiMa GeNeRaCioN" );
               break;
               case GENERACION_NOVENA :
               sprintf( outbuf, "NoVeNa GeNeRaCioN" );
               break;
               case GENERACION_OCTAVA :
               sprintf( outbuf, "OcTaVa GeNeRaCioN" );
               break;
               case GENERACION_SEPTIMA :
               sprintf( outbuf, "SepTiMa GeNeRaCioN" );
               break;
               case GENERACION_SEXTA :
               sprintf( outbuf, "SexTa GeNeRaCioN" );
               break;
               case GENERACION_QUINTA :
               sprintf( outbuf, "QuiNTa GeNeRaCioN" );
               break;
               case GENERACION_MATUSALEN :
               sprintf( outbuf, "MaTuSaLeN" );
               break;
               case GENERACION_ANTEDILUVIANO :
               sprintf( outbuf, "AnTeDiLuViaNo" );
               break;
               case GENERACION_SEGUNDA :
               sprintf( outbuf, "SeGuNDa GeNeRaCioN" );
               break;
               case GENERACION_CAIN :
               sprintf( outbuf, "ser el mismisimo CaIn" );
               break;
            }
              sprintf( buf, "%s ha bajado a %s", victim->name, outbuf );
              mensa_todos( victim, "generacion", buf);
              ch_printf( victim, "&gMierda!!! Has bajado a &R%s&G!!!\n\r", outbuf);
              return;
       }
    return;
}
/*
 * Nuevo sistema de Mensajes internos de VampiroMud
 * este sistema sustituye el antiguo send_to_char a base
 * de codigo por una nueva funcion que envia mensajes personales
 * a quien se requiera.
 * COLIKOTRON Code Team(QPR). SiGo Y SaNgUi 11/11/2001
 * Hoy hace dos meses que Bin Laden se hizo Archifamoso XD
 */
void mensa( CHAR_DATA *ch, char *envio, char *mensaje)
{
   if(!envio)
   {
   bug( "Funcion Mensaje: No hay tipo de envio o envio nulo", 0);
   return;
   }

   if(!mensaje)
   {
   bug( "Funcion Mensaje: No hay Mensaje o Mensaje nulo", 0);
   return;
   }

   if(!ch)
   {
   bug( "Funcion Mensaje: No hay Jugador o jugador nulo", 0);
   return;
   }

   ch_printf(ch,"&z[&wInfo%s&z]", capitalize(envio));
   ch_printf(ch,"[&g %s &z]\n\r", mensaje );
   return;
}

void mensa_nota( CHAR_DATA *ch, char *envio, char *mensaje)
{
	CHAR_DATA *p, *p_prev;
	/* Si no hay envio */
   if(!envio)
   {
   bug( "Funcion Mensaje_Nota: No hay tipo de envio o envio nulo", 0);
   return;
   }
   /* Si no hay mensaje */

   if(!mensaje)
   {
   bug( "Funcion Mensaje_Nota: No hay Mensaje o Mensaje nulo", 0);
   return;
   }
   for (p = last_char; p ; p = p_prev )
   {
   p_prev = p->prev;


   if(!p)
   bug( "Funcion Mensaje_Nota: No hay Jugador o jugador nulo", 0);
   else
   {
		if (ch->pcdata->board->read_level < get_trust(ch))
		{
			ch_printf(p,"&z[&wInfo%s&z]", capitalize(envio));
			ch_printf(p,"[&g %s &z]\n\r", mensaje );
		}
   }
  }
  return;
}


void mensa_todos( CHAR_DATA *ch, char *envio, char *mensaje)
{
   CHAR_DATA *p, *p_prev;
  /* Si no hay envio */
   if(!envio)
   {
   bug( "Funcion Mensaje: No hay tipo de envio o envio nulo", 0);
   return;
   }
   /* Si no hay mensaje */

   if(!mensaje)
   {
   bug( "Funcion Mensaje: No hay Mensaje o Mensaje nulo", 0);
   return;
   }
   for (p = last_char; p ; p = p_prev )
   {
   p_prev = p->prev;


   if(!p)
   bug( "Funcion Mensaje: No hay Jugador o jugador nulo", 0);
   else
   {
   ch_printf(p,"&z[&wInfo%s&z]", capitalize(envio));
   ch_printf(p,"[&g %s &z]\n\r", mensaje );
   }
  }
  return;
}

void mensa_familia( CHAR_DATA *ch, char *mensaje)
{
   CHAR_DATA *p, *p_prev;

   /* Si no hay mensaje */

   if(!mensaje)
   {
   bug( "Funcion Mensaje: No hay Mensaje o Mensaje nulo", 0);
   return;
   }
   for (p = last_char; p ; p = p_prev )
   {
   p_prev = p->prev;


   if(!p)
   bug( "Funcion Mensaje: No hay Jugador o jugador nulo", 0);
   else
   {
   if(IS_NPC(p))
   continue;
   if(p->pcdata->clan != ch->pcdata->clan)
   continue;
   else
   ch_printf(p,"&z[&wInfoFamilia&z]" ) ; ch_printf(p,"[&g %s &z]\n\r", mensaje );
   }
  }
  return;
}

void espera_menos( CHAR_DATA *ch, int nivel, int espera )
{
        int devuelve = 0;

        /* Si no hay jugador */
        if(!ch)
        {
                bug( "Jugador nulo en funcion espera_menos.", 0 );
                return;
        }

        /* Si no hay nivel */
        if(!nivel)
        {
                bug( "Nivel nulo en funcion espera_menos.", 0 );
                return;
        }

        /* Si no hay espera */
        if( !espera)
        {
                bug( "Espera de wait nula en funcion espera_menos.",  0 );
                return;
        }

        if( nivel > 5 )
        nivel = 5;

        if(IS_VAMPIRE(ch))
        {
                if(!xIS_SET(ch->afectado_por, DAF_CELERIDAD))
                {
                bug( "%s ha llegado a la funcion espera_menos sin tener Celeridad activada. (????)", ch->name );
                WAIT_STATE(ch, espera);
                return;
                }
                else
                devuelve = (espera -= espera * (nivel * 0.1));

                if( devuelve < 1 )
                devuelve = 1;
        }
        else
        {
                if(!xIS_SET(ch->act, PLR_TEMPORIS))
                {
                bug( "%s ha llegado a la funcion espera_menos sin tener Temporis activada. (????)", ch->name );
                WAIT_STATE(ch, espera);
                return;
                }
                else
                devuelve = (espera -= espera * (nivel * 0.1));

                if( devuelve < 1 )
                devuelve = 1;
        }


WAIT_STATE(ch, devuelve);
return;
}







