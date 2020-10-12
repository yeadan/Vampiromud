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
 *			    Battle & death module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


extern char		lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *	gch_prev;

OBJ_DATA *used_weapon;   /* Used to figure out which weapon later */

/*
 * Local functions.
 */
void	new_dam_message	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int  dam,
			    int dt, OBJ_DATA *obj ) );
void	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
int	align_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim ) );
ch_ret	one_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
int	obj_hitroll	args( ( OBJ_DATA *obj ) );
void    show_condition  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    crear_automob   args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/*
 * Check to see if player's attacks are (still?) suppressed
 * #ifdef TRI
 */
bool is_attack_supressed( CHAR_DATA *ch )
{
  TIMER *timer;

  if (IS_NPC(ch))
    return FALSE;

  timer = get_timerptr( ch, TIMER_ASUPRESSED );

  if ( !timer )
    return FALSE;

  /* perma-supression -- bard? (can be reset at end of fight, or spell, etc) */
  if ( timer->value == -1 )
    return TRUE;

  /* this is for timed supressions */
  if ( timer->count >= 1 )
    return TRUE;

  return FALSE;
}

/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned( CHAR_DATA *ch )
{
    OBJ_DATA *obj;

    if ( !used_weapon )
    	return FALSE;

    if ( (obj=get_eq_char(ch, WEAR_WIELD)) != NULL
    &&    used_weapon == obj
    &&    IS_OBJ_STAT(obj, ITEM_POISONED) )
	return TRUE;
    if ( (obj=get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL
    &&    used_weapon == obj
    &&	  IS_OBJ_STAT(obj, ITEM_POISONED) )
    	return TRUE;

    return FALSE;
}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hunting || ch->hunting->who != victim )
      return FALSE;

    return TRUE;
}

bool is_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->hating || ch->hating->who != victim )
      return FALSE;

    return TRUE;
}

bool is_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !ch->fearing || ch->fearing->who != victim )
      return FALSE;

    return TRUE;
}

void stop_hunting( CHAR_DATA *ch )
{
    if ( ch->hunting )
    {
	STRFREE( ch->hunting->name );
	DISPOSE( ch->hunting );
	ch->hunting = NULL;
    }
    return;
}

void stop_hating( CHAR_DATA *ch )
{
    if ( ch->hating )
    {
	STRFREE( ch->hating->name );
	DISPOSE( ch->hating );
	ch->hating = NULL;
    }
    return;
}

void stop_fearing( CHAR_DATA *ch )
{
    if ( ch->fearing )
    {
	STRFREE( ch->fearing->name );
	DISPOSE( ch->fearing );
	ch->fearing = NULL;
    }
    return;
}

void start_hunting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hunting )
      stop_hunting( ch );

    CREATE( ch->hunting, HHF_DATA, 1 );
    ch->hunting->name = QUICKLINK( victim->name );
    ch->hunting->who  = victim;
    return;
}

void start_hating( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->hating )
      stop_hating( ch );

    CREATE( ch->hating, HHF_DATA, 1 );
    ch->hating->name = QUICKLINK( victim->name );
    ch->hating->who  = victim;
    return;
}

void start_fearing( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( ch->fearing )
      stop_fearing( ch );

    CREATE( ch->fearing, HHF_DATA, 1 );
    ch->fearing->name = QUICKLINK( victim->name );
    ch->fearing->who  = victim;
    return;
}

/*
 * Get the current armor class for a vampire based on time of day
 */
sh_int VAMP_AC( CHAR_DATA * ch )
{
  if  ( IS_VAMPIRE( ch ) && IS_OUTSIDE( ch ) )
  {
    switch(time_info.sunlight)
    {
    case SUN_DARK:
      return -10;
    case SUN_RISE:
      return 5;
    case SUN_LIGHT:
      return 10;
    case SUN_SET:
      return 2;
    default:
      return 0;
    }
  }
  else
    return 0;
}

int max_fight( CHAR_DATA *ch )
{
    return 8;
}



/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 *
 * Note:  This function also handles some non-violence updates.
 */
void violence_update( void )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *ch;
    CHAR_DATA *lst_ch;
    CHAR_DATA *victim;
    CHAR_DATA *rch, *rch_next;
    AFFECT_DATA *paf, *paf_next;
    DISCI_AFFECT *daf, *daf_next;
    TIMER	*timer, *timer_next;
    ch_ret	retcode;
    int		attacktype, cnt;
    SKILLTYPE	*skill;
    NIVEL       *nivel;
    static int	pulse = 0;

    lst_ch = NULL;
    pulse = (pulse+1) % 100;

    for ( ch = last_char; ch; lst_ch = ch, ch = gch_prev )
    {
	set_cur_char( ch );

	if ( ch == first_char && ch->prev )
	{
	   bug( "ERROR: first_char->prev != NULL, fixing...", 0 );
	   ch->prev = NULL;
	}

	gch_prev	= ch->prev;

	if ( gch_prev && gch_prev->next != ch )
	{
	    sprintf( buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.",
		ch->name );
	    bug( buf, 0 );
	    bug( "Short-cutting here", 0 );
	    ch->prev = NULL;
	    gch_prev = NULL;
	    do_shout( ch, "Thoric says, 'Prepare for the worst!'" );
	}

	/*
	 * See if we got a pointer to someone who recently died...
	 * if so, either the pointer is bad... or it's a player who
	 * "died", and is back at the healer...
	 * Since he/she's in the char_list, it's likely to be the later...
	 * and should not already be in another fight already
	 */
	if ( char_died(ch) )
	    continue;

	/*
	 * See if we got a pointer to some bad looking data...
	 */
	if ( !ch->in_room || !ch->name )
	{
	    log_string( "violence_update: bad ch record!  (Shortcutting.)" );
	    sprintf( buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
	    	(int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next );
	    log_string( buf );
	    log_string( lastplayercmd );
	    if ( lst_ch )
	      sprintf( buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
	      		(int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next );
	    else
	      strcpy( buf, "lst_ch: NULL" );
	    log_string( buf );
	    gch_prev = NULL;
	    continue;
	}

        /*
         * Experience gained during battle deceases as battle drags on
         */
	if ( ch->fighting )
	  if ( (++ch->fighting->duration % 24) == 0 )
	    ch->fighting->xp = ((ch->fighting->xp * 9) / 10);




	for ( timer = ch->first_timer; timer; timer = timer_next )
	{
	    timer_next = timer->next;
	    if ( --timer->count <= 0 )
	    {
		if ( timer->type == TIMER_ASUPRESSED )
		{
		    if ( timer->value == -1 )
		    {
			timer->count = 1000;
			continue;
		    }
		}
		if ( timer->type == TIMER_NUISANCE )
		{
		    DISPOSE( ch->pcdata->nuisance );
		}

		if ( timer->type == TIMER_DO_FUN )
		{
		    int tempsub;

		    tempsub = ch->substate;
		    ch->substate = timer->value;
		    (timer->do_fun)( ch, "" );
		    if ( char_died(ch) )
			break;
		    ch->substate = tempsub;
		}
		extract_timer( ch, timer );
	    }
	}

	if ( char_died(ch) )
	    continue;

	/*
	 * We need spells that have shorter durations than an hour.
	 * So a melee round sounds good to me... -Thoric
	 */
	for ( paf = ch->first_affect; paf; paf = paf_next )
	{
	    paf_next	= paf->next;
	    if ( paf->duration > 0 )
		paf->duration--;
	    else
	    if ( paf->duration < 0 )
		;
	    else
	    {
		if ( !paf_next
		||    paf_next->type != paf->type
		||    paf_next->duration > 0 )
		{
		    skill = get_skilltype(paf->type);
		    if ( paf->type > 0 && skill && skill->msg_off )
		    {
			set_char_color( AT_WEAROFF, ch );
			send_to_char( skill->msg_off, ch );
			send_to_char( "\n\r", ch );
		    }
		}
		if (paf->type == gsn_possess)
	        {
		    ch->desc->character = ch->desc->original;
		    ch->desc->original  = NULL;
		    ch->desc->character->desc = ch->desc;
		    ch->desc->character->switched = NULL;
		    ch->desc            = NULL;
		}
		affect_remove( ch, paf );
	    }
	}

        for( daf = ch->primer_affect; daf; daf = daf_next )
        {
                daf_next    = daf->next;
                if( daf->duracion > 0 )
                    daf->duracion--;
                else
                if( daf->duracion < 0 )
                    ;
                else
                {
                        if( !daf_next
                        ||   daf_next->nivel != daf->nivel
                        || daf_next->duracion > 0 )
                        {
                                nivel = get_dnivel(daf->nivel);
                                bug( "nivel = %s tiempo %d", nivel->nombre, daf->duracion );
                                if( daf->nivel != NULL && nivel && nivel->sevames )
                                {
                                        set_char_color( AT_WEAROFF, ch );
                                        send_to_char( nivel->sevames, ch );
                                        send_to_char( "\n\r", ch );
                                }
                         }
                         disciplina_remove( ch, daf );
                 }
          }

	if ( char_died(ch) )
	    continue;

	/* check for exits moving players around */
	if ( (retcode=pullcheck(ch, pulse)) == rCHAR_DIED || char_died(ch) )
	    continue;

	/* Let the battle begin! */

	if ( ( victim = who_fighting( ch ) ) == NULL
	||   IS_AFFECTED( ch, AFF_PARALYSIS ) )
	    continue;

        retcode = rNONE;

        if( IS_VAMPIRE(ch) && ch->pcdata->habilidades[DISCIPLINA_EXTINCION] >= 3 )
        {
        if( !IS_NPC(victim))
        {
        int probabilidad = 0;
        int potencia = 0;
        int fallo = 0;

                  probabilidad = ch->intimidacion - victim->intimidacion;
                    if ( probabilidad <= 1 )
                          probabilidad = 1;

                       fallo = number_range( 1, 30 );
                    if( fallo <= probabilidad )
                     {

                                potencia = victim->max_hit - ( victim->max_hit *0.90 ) + (probabilidad * 100);
                                apunyalamiento( ch, victim, potencia );
                                if( ch->class == CLASS_VAMPIRE )
                                {
                                potencia = victim->max_hit - ( victim->max_hit *0.90 ) + (probabilidad * 100);
                                apunyalamiento( ch, victim, potencia + number_range(-10, 10 ));
                                }
                     }
    }
    if( IS_NPC(victim))
    {
        int probabilidad = 0;
        int potencia = 0;
        int fallo = 0;

                  probabilidad = ch->intimidacion - victim->intimidacion;
                    if ( probabilidad <= 1 )
                          probabilidad = 1;

                       fallo = number_range( 1, 30 );
                    if( fallo <= probabilidad )
                     {
                                potencia = victim->max_hit - ( victim->max_hit *0.90 );
                                apunyalamiento( ch, victim, potencia );
                                if( ch->class == CLASS_VAMPIRE )
                                {
                                probabilidad = number_range( 1, 5 );
                                if( probabilidad >= 4)
                                {
                                potencia = victim->max_hit - ( victim->max_hit *0.90 );
                                apunyalamiento( ch, victim, potencia + number_range(-10, 10 ));
                                }
                                else
                                        send_to_char( "&BTe resbalas y no consigues asestar tu apunyalamiento trapero.\n\r", ch );
                                }
                     }
   }
   }
	if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE ) && !IS_IMMORTAL( ch ) && !IS_IMMORTAL( victim ) )
	{
	   sprintf( buf, "violence_update: %s fighting %s in a SAFE room.",
	   	ch->name, victim->name );
	   log_string( buf );
	   stop_fighting( ch, TRUE );
	}
	else
	if ( IS_AWAKE(ch) && ch->in_room == victim->in_room )
	    retcode = multi_hit( ch, victim, TYPE_UNDEFINED );
	else
	    stop_fighting( ch, FALSE );

	if ( char_died(ch) )
	    continue;

	if ( retcode == rCHAR_DIED
	|| ( victim = who_fighting( ch ) ) == NULL )
	    continue;

	/*
	 *  Mob triggers
	 *  -- Added some victim death checks, because it IS possible.. -- Alty
	 */
	rprog_rfight_trigger( ch );
	if ( char_died(ch) || char_died(victim) )
	    continue;
	mprog_hitprcnt_trigger( ch, victim );
	if ( char_died(ch) || char_died(victim) )
	    continue;
	mprog_fight_trigger( ch, victim );
	if ( char_died(ch) || char_died(victim) )
	    continue;

	/*
	 * NPC special attack flags				-Thoric
	 */
	if ( IS_NPC(ch) )
	{
	    if ( !xIS_EMPTY(ch->attacks) )
	    {
		attacktype = -1;
		if ( 30 + (ch->level/4) >= number_percent( ) )
		{
		    cnt = 0;
		    for ( ;; )
		    {
			if ( cnt++ > 10 )
			{
			    attacktype = -1;
			    break;
			}
			attacktype = number_range( 7, MAX_ATTACK_TYPE-1 );
			if ( xIS_SET( ch->attacks, attacktype ) )
			    break;
		    }
		    switch( attacktype )
		    {
			case ATCK_BASH:
			    do_bash( ch, "" );
			    retcode = global_retcode;
			    break;
			case ATCK_STUN:
			    do_stun( ch, "" );
			    retcode = global_retcode;
			    break;
			case ATCK_GOUGE:
			    do_gouge( ch, "" );
			    retcode = global_retcode;
			    break;
			case ATCK_FEED:
			    do_gouge( ch, "" );
			    retcode = global_retcode;
			    break;
			case ATCK_DRAIN:
			    retcode = spell_energy_drain( skill_lookup( "energy drain" ), ch->level, ch, victim );
			    break;
  		       	case ATCK_FIREBREATH:
			    retcode = spell_fire_breath( skill_lookup( "fire breath" ), ch->level, ch, victim );
             		    break;
             	 	case ATCK_FROSTBREATH:
			    retcode = spell_frost_breath( skill_lookup( "frost breath" ), ch->level, ch, victim );
			    break;
			case ATCK_ACIDBREATH:
			    retcode = spell_acid_breath( skill_lookup( "acid breath" ), ch->level, ch, victim );
			    break;
			case ATCK_LIGHTNBREATH:
			    retcode = spell_lightning_breath( skill_lookup( "lightning breath" ), ch->level, ch, victim );
			    break;
			case ATCK_DRAINBREATH:
			    retcode = spell_draining_breath( skill_lookup( "draining breath" ), ch->level, ch, victim );
			    break;	
			case ATCK_GASBREATH:
			    retcode = spell_gas_breath( skill_lookup( "gas breath" ), ch->level, ch, victim );
			    break;
			case ATCK_SPIRALBLAST:
			    retcode = spell_spiral_blast( skill_lookup( "spiral blast" ),
				ch->level, ch, victim );
			    break;
			case ATCK_POISON:
			    retcode = spell_poison( gsn_poison, ch->level, ch, victim );
			    break;
			case ATCK_NASTYPOISON:
			    /*
			    retcode = spell_nasty_poison( skill_lookup( "nasty poison" ), ch->level, ch, victim );
			    */
			    break;
			case ATCK_GAZE:
			    /*
			    retcode = spell_gaze( skill_lookup( "gaze" ), ch->level, ch, victim );
			    */
			    break;
			case ATCK_BLINDNESS:
			    retcode = spell_blindness( gsn_blindness, ch->level, ch, victim );
			    break;
			case ATCK_CAUSESERIOUS:
			    retcode = spell_cause_serious( skill_lookup( "cause serious" ), ch->level, ch, victim );
			    break;
			case ATCK_EARTHQUAKE:
			    retcode = spell_earthquake( skill_lookup( "earthquake" ), ch->level, ch, victim );
			    break;
			case ATCK_CAUSECRITICAL:
			    retcode = spell_cause_critical( skill_lookup( "cause critical" ), ch->level, ch, victim );
			    break;
			case ATCK_CURSE:
			    retcode = spell_curse( skill_lookup( "curse" ), ch->level, ch, victim );
			    break;
			case ATCK_FLAMESTRIKE:
			    retcode = spell_flamestrike( skill_lookup( "flamestrike" ), ch->level, ch, victim );
			    break;
			case ATCK_HARM:
			    retcode = spell_harm( skill_lookup( "harm" ), ch->level, ch, victim );
			    break;
			case ATCK_FIREBALL:
			    retcode = spell_fireball( skill_lookup( "fireball" ), ch->level, ch, victim );
			    break;
			case ATCK_COLORSPRAY:
			    retcode = spell_colour_spray( skill_lookup( "colour spray" ), ch->level, ch, victim );
			    break;
			case ATCK_WEAKEN:
			    retcode = spell_weaken( skill_lookup( "weaken" ), ch->level, ch, victim );
			    break;
		    }
		    if ( attacktype != -1 && (retcode == rCHAR_DIED || char_died(ch)) )
			continue;
		}
	    }
	    /*
	     * NPC special defense flags				-Thoric
	     */
	    if ( !xIS_EMPTY(ch->defenses) )
	    {
		attacktype = -1;
		if ( 50 + (ch->level/4) > number_percent( ) )
		{
		    cnt = 0;
		    for ( ;; )
		    {
			if ( cnt++ > 10 )
			{
			    attacktype = -1;
			    break;
			}
			attacktype = number_range( 2, MAX_DEFENSE_TYPE-1 );
			if ( xIS_SET( ch->defenses, attacktype ) )
			    break;
		    }

		    switch( attacktype )
		    {
			case DFND_CURELIGHT:
			    /* A few quick checks in the cure ones so that a) less spam and
			       b) we don't have mobs looking stupider than normal by healing
			       themselves when they aren't even being hit (although that
			       doesn't happen TOO often */
			    if ( ch->hit < ch->max_hit )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'cure light'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "cure light" ), ch->level, ch, ch );
			    }
			    break;
			case DFND_CURESERIOUS:
			    if ( ch->hit < ch->max_hit )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'cure serious'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "cure serious" ), ch->level, ch, ch );
			    }
			    break;
			case DFND_CURECRITICAL:
			    if ( ch->hit < ch->max_hit )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'cure critical'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "cure critical" ), ch->level, ch, ch );
			    }
			    break;
			case DFND_HEAL:
			    if ( ch->hit < ch->max_hit )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'curacion'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "heal" ), ch->level, ch, ch );
			    }
			    break;
			case DFND_DISPELMAGIC:
			    if ( ch->first_affect )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'disipar magia'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_dispel_magic( skill_lookup( "dispel magic" ), ch->level, ch, victim );
			    }
			    break;
			case DFND_DISPELEVIL:
			    act( AT_MAGIC, "$n pronuncia las palabras 'disipar mal.", ch, NULL, NULL, TO_ROOM );
			    retcode = spell_dispel_evil( skill_lookup( "dispel evil" ), ch->level, ch, victim );
			    break;
			case DFND_TELEPORT:
			    retcode = spell_teleport( skill_lookup( "teleport"), ch->level, ch, ch );
			    break;
			case DFND_SHOCKSHIELD:
			    if ( !IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'shockshield.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "shockshield" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
                       case DFND_VENOMSHIELD:
                            if ( !IS_AFFECTED( ch, AFF_VENOMSHIELD ) )
                            {
                                act( AT_MAGIC, "$n pronuncia las palabras 'venomshield'.", ch, NULL, NULL, TO_ROOM );
                                retcode = spell_smaug( skill_lookup( "venomshield" ), ch->level, ch, ch );
                            }
                            else
                                retcode = rNONE;
                            break;
			case DFND_ACIDMIST:
			    if ( !IS_AFFECTED( ch, AFF_ACIDMIST ) )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'acidmist'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "acidmist" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
			case DFND_FIRESHIELD:
			    if ( !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'fireshield'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "fireshield" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
			case DFND_ICESHIELD:
			    if ( !IS_AFFECTED( ch, AFF_ICESHIELD ) )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'iceshield'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "iceshield" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
			case DFND_TRUESIGHT:
			    if ( !IS_AFFECTED( ch, AFF_TRUESIGHT ) )
				retcode = spell_smaug( skill_lookup( "true" ), ch->level, ch, ch );
			    else
				retcode = rNONE;
			    break;
			case DFND_SANCTUARY:
			    if ( !IS_AFFECTED( ch, AFF_SANCTUARY ) )
			    {
				act( AT_MAGIC, "$n pronuncia las palabras 'santuario'.", ch, NULL, NULL, TO_ROOM );
				retcode = spell_smaug( skill_lookup( "sanctuary" ), ch->level, ch, ch );
			    }
			    else
				retcode = rNONE;
			    break;
		    }
		    if ( attacktype != -1 && (retcode == rCHAR_DIED || char_died(ch)) )
			continue;
		}
	    }
	}

	/*
	 * Fun for the whole family!
	 */
	for ( rch = ch->in_room->first_person; rch; rch = rch_next )
	{
	    rch_next = rch->next_in_room;

            /*
             *   Group Fighting Styles Support:
             *   If ch is tanking
             *   If rch is using a more aggressive style than ch
             *   Then rch is the new tank   -h
             */
            /* &&( is_same_group(ch, rch)      ) */

            if( ( !IS_NPC(ch) && !IS_NPC(rch) )
              &&( rch!=ch                     )
              &&( rch->fighting               )
              &&( who_fighting(rch->fighting->who) == ch    )
              &&( !xIS_SET( rch->fighting->who->act, ACT_AUTONOMOUS ) )
              &&( rch->style < ch->style      )
              )
            {
                 rch->fighting->who->fighting->who = rch;

            }

	    if ( IS_AWAKE(rch) && !rch->fighting )
	    {
		/*
		 * PC's auto-assist others in their group.
		 */
		if ( !IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM) || xIS_SET(ch->act, PLR_DOMINADO ))
		{
		    if ( ( ( !IS_NPC(rch) && rch->desc )
		    ||        IS_AFFECTED(rch, AFF_CHARM) || xIS_SET(rch->act, PLR_DOMINADO ))
		    && is_same_group(ch, rch)
		    && !is_safe( rch, victim, TRUE) )
			multi_hit( rch, victim, TYPE_UNDEFINED );
		    continue;
		}

		/*
		 * NPC's assist NPC's of same type or 12.5% chance regardless.
		 */
		if ( IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM)
		&&  !xIS_SET(rch->act, ACT_NOASSIST) )
		{
		    if ( char_died(ch) )
			break;
		    if ( rch->pIndexData == ch->pIndexData
		    ||   number_bits( 3 ) == 0 )
		    {
			CHAR_DATA *vch;
			CHAR_DATA *target;
			int number;

			target = NULL;
			number = 0;
                        for ( vch = ch->in_room->first_person; vch; vch = vch->next )
			{
			    if ( can_see( rch, vch )
			    &&   is_same_group( vch, victim )
			    &&   number_range( 0, number ) == 0 )
			    {
				if ( vch->mount && vch->mount == rch )
				  target = NULL;
				else
				{
				  target = vch;
				  number++;
				}
			    }
			}

			if ( target )
			    multi_hit( rch, target, TYPE_UNDEFINED );
		    }
		}
	    }
	}
    }
    return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    int     chance;
    int	    dual_bonus;
    ch_ret  retcode;

    if( IS_NPC(victim))
    {
    if( xIS_SET(victim->act, ACT_AUTOMOB) || IS_SET(ch->in_room->area->flags, AFLAG_AUTOAREA) && !xIS_SET(victim->act, ACT_UALIZADO ) )
    crear_automob( ch, victim );
    }

    /* add timer to pkillers */
    if ( !IS_NPC(ch) && !IS_NPC(victim) )
    {
        if ( xIS_SET(ch->act, PLR_NICE) ) return rNONE;
        add_timer( ch,     TIMER_RECENTFIGHT, 11, NULL, 0 );
        add_timer( victim, TIMER_RECENTFIGHT, 11, NULL, 0 );
    }

    if( !IS_NPC(ch) && !IS_NPC(victim) )
    {
        if( !xIS_SET(ch->act, PLR_ALTERADO) )
        {
        xSET_BIT(ch->act, PLR_ALTERADO);
        ch->pcdata->alterado = 5;
        }
        if( !xIS_SET(victim->act, PLR_ALTERADO) )
        {
        xSET_BIT(victim->act, PLR_ALTERADO);
        ch->pcdata->alterado = 5;
        }
    }

    if ( is_attack_supressed( ch ) )
      return rNONE;

    if ( IS_NPC(ch) && xIS_SET(ch->act, ACT_NOATTACK) )
      return rNONE;

    if ( (retcode = one_hit( ch, victim, dt )) != rNONE )
      return retcode;

    if ( who_fighting( ch ) != victim || dt == gsn_backstab || dt == gsn_circle || dt == gsn_empalar
    || dt == gsn_tangana )
	return rNONE;

    /* Very high chance of hitting compared to chance of going berserk */
    /* 40% or higher is always hit.. don't learn anything here though. */
    /* -- Altrag */
    chance = IS_NPC(ch) ? 100 : (LEARNED(ch, gsn_berserk)*5/2);
    if ( IS_AFFECTED(ch, AFF_BERSERK) && number_percent() < chance )
      if ( (retcode = one_hit( ch, victim, dt )) != rNONE ||
            who_fighting( ch ) != victim )
        return retcode;

    if ( get_eq_char( ch, WEAR_DUAL_WIELD ) )
    {
      dual_bonus = IS_NPC(ch) ? (ch->level / 10) : (LEARNED(ch, gsn_dual_wield) / 10);
      chance = IS_NPC(ch) ? ch->level : LEARNED(ch, gsn_dual_wield);
      if ( number_percent( ) < chance )
      {
	learn_from_success( ch, gsn_dual_wield );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
      }
      else
	learn_from_failure( ch, gsn_dual_wield );
    }
    else
      dual_bonus = 0;

    if ( ch->move < 10 )
      dual_bonus = -20;
    /*
     * NPC predetermined number of attacks			-Thoric
     */
    if ( IS_NPC(ch) && ch->numattacks > 0 )
    {
	for ( chance = 0; chance < ch->numattacks; chance++ )
	{
	   retcode = one_hit( ch, victim, dt );
	   if ( retcode != rNONE || who_fighting( ch ) != victim )
	     return retcode;
	}
	return retcode;
    }

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((LEARNED(ch, gsn_second_attack)+dual_bonus)/1.5);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_second_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_second_attack );

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((LEARNED(ch, gsn_third_attack)+(dual_bonus*1.5))/2);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_third_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_third_attack );

    chance = IS_NPC(ch) ? ch->level
	   : (int) ((LEARNED(ch, gsn_fourth_attack)+(dual_bonus*2))/3);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_fourth_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_fourth_attack );

    chance = IS_NPC(ch) ? ch->level
      : (int) ((LEARNED(ch, gsn_fifth_attack)+(dual_bonus*3))/4);
    if ( number_percent( ) < chance )
    {
	learn_from_success( ch, gsn_fifth_attack );
	retcode = one_hit( ch, victim, dt );
	if ( retcode != rNONE || who_fighting( ch ) != victim )
	    return retcode;
    }
    else
	learn_from_failure( ch, gsn_fifth_attack );

    /*
     * Un ataque mas por los afectados por celeridad o tiempo */
    if(xIS_SET(ch->afectado_por, DAF_CELERIDAD)
     || xIS_SET(ch->act, PLR_TEMPORIS))
    one_hit( ch, victim, dt );

    if( !IS_NPC(ch))
    if( ES_REMORT(ch))
    one_hit( ch, victim, dt );

    retcode = rNONE;


    chance = IS_NPC(ch) ? (int) (ch->level / 2) : 0;
    if ( number_percent( ) < chance )
	retcode = one_hit( ch, victim, dt );

    if ( retcode == rNONE )
    {
	int move;

	if ( !IS_AFFECTED(ch, AFF_FLYING)
	&&   !IS_AFFECTED(ch, AFF_FLOATING) )
	  move = encumbrance( ch, movement_loss[UMIN(SECT_MAX-1, ch->in_room->sector_type)] );
	else
	  move = encumbrance( ch, 1 );
	if ( ch->move )
	  ch->move = UMAX( 0, ch->move - move );
    }
    return retcode;
}


/*
 * Weapon types, haus
 */
int weapon_prof_bonus_check( CHAR_DATA *ch, OBJ_DATA *wield, int *gsn_ptr )
{
    int bonus;

    bonus = 0;	*gsn_ptr = -1;
    if ( !IS_NPC(ch) && ch->level > 5 && wield )
    {
	switch(wield->value[3])
	{
	   default:		*gsn_ptr = -1;			break;
           case DAM_HIT:
	   case DAM_SUCTION:
	   case DAM_BITE:
	   case DAM_BLAST:	*gsn_ptr = gsn_pugilism;	break;
           case DAM_SLASH:
           case DAM_SLICE:	*gsn_ptr = gsn_long_blades;	break;
           case DAM_PIERCE:
           case DAM_STAB:	*gsn_ptr = gsn_short_blades;	break;
           case DAM_WHIP:	*gsn_ptr = gsn_flexible_arms;	break;
           case DAM_CLAW:	*gsn_ptr = gsn_talonous_arms;	break;
           case DAM_POUND:
           case DAM_CRUSH:	*gsn_ptr = gsn_bludgeons;	break;
	   case DAM_BOLT:
	   case DAM_ARROW:
	   case DAM_DART:
	   case DAM_STONE:
	   case DAM_PEA:	*gsn_ptr = gsn_missile_weapons;	break;

	}
	if ( *gsn_ptr != -1 )
	  bonus = (int) ((LEARNED(ch, *gsn_ptr) -50)/10);

       /* Reduce weapon bonuses for misaligned clannies.
       if ( IS_CLANNED(ch) )
       {
          bonus = bonus * 1000 /
          ( 1 + abs( ch->alignment - ch->pcdata->clan->alignment ) );
       }*/

	if ( IS_DEVOTED(ch) )
	   bonus -= ch->pcdata->favor / -400 ;

    }
    return bonus;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll( OBJ_DATA *obj )
{
	int tohit = 0;
	AFFECT_DATA *paf;

	for ( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	for ( paf = obj->first_affect; paf; paf = paf->next )
		if ( paf->location == APPLY_HITROLL )
			tohit += paf->modifier;
	return tohit;
}

/*
 * Offensive shield level modifier
 */
sh_int off_shld_lvl( CHAR_DATA *ch, CHAR_DATA *victim )
{
    sh_int lvl;

    if ( !IS_NPC(ch) )          /* players get much less effect */
    {
        lvl = UMAX( 1, (ch->level - 10) / 2 );
        if ( number_percent() + (victim->level - lvl) < 40 )
	{
	  if ( CAN_PKILL( ch ) && CAN_PKILL( victim ) )
	     return ch->level;
	  else
	     return lvl;
	}
        else
          return 0;
    }
    else
    {
	lvl = ch->level / 2;
	if ( number_percent() + (victim->level - lvl) < 70 )
	  return lvl;
	else
	  return 0;
    }
}

/*
 * Hit one guy once.
 */
ch_ret one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt )
{
    OBJ_DATA *wield;
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int plusris;
    int dam;
    int diceroll;
    int attacktype, cnt;
    int	prof_bonus;
    int	prof_gsn = -1;
    ch_ret retcode = rNONE;
    static bool dual_flip = FALSE;

    /*
     * Can't beat a dead char!
     * Guard against weird room-leavings.
     */
    if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
	return rVICT_DIED;

    used_weapon = NULL;
    /*
     * Figure out the weapon doing the damage			-Thoric
     * Dual wield support -- switch weapons each attack
     */
    if ( (wield = get_eq_char( ch, WEAR_DUAL_WIELD )) != NULL )
    {
	if ( dual_flip == FALSE )
	{
	    dual_flip = TRUE;
	    wield = get_eq_char( ch, WEAR_WIELD );
	}
	else
	    dual_flip = FALSE;
    }
    else
	wield = get_eq_char( ch, WEAR_WIELD );

    used_weapon = wield;

    if ( wield )
	prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
    else
	prof_bonus = 0;

    if ( ch->fighting		/* make sure fight is already started */
    &&   dt == TYPE_UNDEFINED
    &&   IS_NPC(ch)
    &&  !xIS_EMPTY(ch->attacks) )
    {
	cnt = 0;
	for ( ;; )
	{
	    attacktype = number_range( 0, 6 );
	    if ( xIS_SET( ch->attacks, attacktype ) )
		break;
	    if ( cnt++ > 16 )
 	    {
		attacktype = -1;
		break;
	    }
	}
	if ( attacktype == ATCK_BACKSTAB )
	    attacktype = -1;
	if ( wield && number_percent( ) > 25 )
	    attacktype = -1;
	if ( !wield && number_percent( ) > 50 )
	    attacktype = -1;

	switch ( attacktype )
	{
	    default:
		break;
	    case ATCK_BITE:
		do_bite( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_CLAWS:
		do_claw( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_TAIL:
		do_tail( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_STING:
		do_sting( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_PUNCH:
		do_punch( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_KICK:
		do_kick( ch, "" );
		retcode = global_retcode;
		break;
	    case ATCK_TRIP:
		attacktype = 0;
		break;
	}
	if ( attacktype >= 0 )
	    return retcode;
    }

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield && wield->item_type == ITEM_WEAPON )
	    dt += wield->value[3];
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = ch->mobthac0;
	thac0_32 =  0;
    }
    else
    {
	thac0_00 = class_table[ch->class]->thac0_00;
	thac0_32 = class_table[ch->class]->thac0_32;
    }
    thac0     = interpolate( ch->level, thac0_00, thac0_32 ) - GET_HITROLL(ch) ;
    victim_ac = UMAX( -19, (int) (GET_AC(victim) / 10) );

    /* if you can't see what's coming... */
    if ( wield && !can_see_obj( victim, wield) )
	victim_ac += 1;
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    /*
     * "learning" between combatants.  Takes the intelligence difference,
     * and multiplies by the times killed to make up a learning bonus
     * given to whoever is more intelligent		-Thoric
     * (basically the more intelligent one "learns" the other's fighting style)
     */
    if ( ch->fighting && ch->fighting->who == victim )
    {
	sh_int times = ch->fighting->timeskilled;

	if ( times )
	{
	    sh_int intdiff = get_curr_int(ch) - get_curr_int(victim);

	    if ( intdiff != 0 )
		victim_ac += (intdiff*times)/10;
	}
    }

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	if ( prof_gsn != -1 )
	    learn_from_failure( ch, prof_gsn );
	damage( ch, victim, 0, dt );
	tail_chain( );
	return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */

    if ( !wield )       /* bare hand dice formula fixed by Thoric */
	dam = number_range( ch->barenumdie, ch->baresizedie * ch->barenumdie );
    else
	dam = number_range( wield->value[1], wield->value[2] );

    /*
     * Bonuses.
     */
   dam += GET_DAMROLL(ch);

    if ( prof_bonus )
	dam += prof_bonus / 4;

    /*
     * Calculate Damage Modifiers from Victim's Fighting Style
     */
    if ( victim->position == POS_BERSERK )
       dam = 1.2 * dam;
    else if ( victim->position == POS_AGGRESSIVE )
       dam = 1.1 * dam;
    else if ( victim->position == POS_DEFENSIVE )
       dam = .85 * dam;
    else if ( victim->position == POS_EVASIVE )
       dam = .8 * dam;

    /*
     * Calculate Damage Modifiers from Attacker's Fighting Style
     */
    if ( ch->position == POS_BERSERK )
       dam = 1.2 * dam;
    else if ( ch->position == POS_AGGRESSIVE )
       dam = 1.1 * dam;
    else if ( ch->position == POS_DEFENSIVE )
       dam = .85 * dam;
    else if ( ch->position == POS_EVASIVE )
       dam = .8 * dam;

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
    {
	dam += (int) (dam * LEARNED(ch, gsn_enhanced_damage) / 120);
	learn_from_success( ch, gsn_enhanced_damage );
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;
    if ( dt == gsn_backstab )
	dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 8);

     /* Empalar habilidad de Cruzados parecida al bs pero putea mazo mas XD */

    if ( dt == gsn_empalar )
	dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 14);

    if ( dt == gsn_tangana )
	dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 16);

    if ( dt == gsn_circle )
 	dam *= (2 + URANGE( 2, ch->level - (victim->level/4), 30 ) / 16);

    plusris = 0;

    if ( dam <= 0 )
        dam = 1;
    if (dam >=32000)
        dam = number_range(31000,32000); 
    /*KAYSER 2004*/

    if ( wield )
    {
	if ( IS_OBJ_STAT(wield, ITEM_MAGIC) )
 	    dam = ris_damage( victim, dam, RIS_MAGIC );
	else
	    dam = ris_damage( victim, dam, RIS_NONMAGIC );

	/*
	 * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
	 */
	plusris = obj_hitroll( wield );
    }
    else
	dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /* check for RIS_PLUSx 					-Thoric */
    if ( dam )
    {
	int x, res, imm, sus, mod;

	if ( plusris )
	   plusris = RIS_PLUS1 << UMIN(plusris, 7);

	/* initialize values to handle a zero plusris */
	imm = res = -1;  sus = 1;

	/* find high ris */
	for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
	{
	    if ( IS_SET( victim->immune, x ) )
		imm = x;
	    if ( IS_SET( victim->resistant, x ) )
		res = x;
	    if ( IS_SET( victim->susceptible, x ) )
		sus = x;
	}
	mod = 10;
	if ( imm >= plusris )
	    mod -= 10;
	if ( res >= plusris )
	    mod -= 2;
	if ( sus <= plusris )
	    mod += 2;

	/* check if immune */
	if ( mod <= 0 )
	    dam = -1;
	if ( mod != 10 )
	    dam = (dam * mod) / 10;
    }

    if ( prof_gsn != -1 )
    {
	if ( dam > 0 )
	    learn_from_success( ch, prof_gsn );
	else
	    learn_from_failure( ch, prof_gsn );
    }

    /* immune to damage */
    if ( dam == -1 )
    {
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];
	    bool found = FALSE;

	    if ( skill->imm_char && skill->imm_char[0] != '\0' )
	    {
		act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
	    {
		act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		found = TRUE;
	    }
	    if ( skill->imm_room && skill->imm_room[0] != '\0' )
	    {
		act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		found = TRUE;
	    }
	    if ( found )
	      return rNONE;
	}
	dam = 0;
    }

    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
	return retcode;
    if ( char_died(ch) )
	return rCHAR_DIED;
    if ( char_died(victim) )
	return rVICT_DIED;

    retcode = rNONE;
    if ( dam == 0 )
	return retcode;

    /*
     * Weapon spell support				-Thoric
     * Each successful hit casts a spell
     */
    if ( wield
    &&  !IS_SET(victim->immune, RIS_MAGIC)
    &&  !IS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	AFFECT_DATA *aff;

	for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;
	for ( aff = wield->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
		return retcode;
    }

    /*
     * magic shields that retaliate				-Thoric
     */
    if ( IS_AFFECTED( victim, AFF_FIRESHIELD )
    &&  !IS_AFFECTED( ch, AFF_FIRESHIELD ) )
	retcode = spell_smaug( skill_lookup( "flare" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    if ( IS_AFFECTED( victim, AFF_ICESHIELD )
    &&  !IS_AFFECTED( ch, AFF_ICESHIELD ) )
         retcode = spell_smaug( skill_lookup( "iceshard" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD )
    &&  !IS_AFFECTED( ch, AFF_SHOCKSHIELD ) )
	retcode = spell_smaug( skill_lookup( "torrent" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
      return retcode;

    if ( IS_AFFECTED( victim, AFF_ACIDMIST )
    &&  !IS_AFFECTED( ch, AFF_ACIDMIST ) )
	retcode = spell_smaug( skill_lookup( "acidshot" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
	return retcode;

    if ( IS_AFFECTED( victim, AFF_VENOMSHIELD )
    &&  !IS_AFFECTED( ch, AFF_VENOMSHIELD ) )
        retcode = spell_smaug( skill_lookup( "venomshot" ), off_shld_lvl(victim, ch), victim, ch );
    if ( retcode != rNONE || char_died(ch) || char_died(victim) )
        return retcode;

    tail_chain( );
    return retcode;
}

/*
 * Hit one guy with a projectile.
 * Handles use of missile weapons (wield = missile weapon)
 * or thrown items/weapons
 */
ch_ret projectile_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
		       OBJ_DATA *projectile, sh_int dist )
{
    int victim_ac;
    int thac0;
    int thac0_00;
    int thac0_32;
    int plusris;
    int dam;
    int diceroll;
    int	prof_bonus;
    int	prof_gsn = -1;
    int proj_bonus;
    int dt;
    ch_ret retcode;

    if ( !projectile )
	return rNONE;

    if ( projectile->item_type == ITEM_PROJECTILE
    ||   projectile->item_type == ITEM_WEAPON )
    {
	dt = TYPE_HIT + projectile->value[3];
	proj_bonus = number_range(projectile->value[1], projectile->value[2]);
    }
    else
    {
	dt = TYPE_UNDEFINED;
	proj_bonus = number_range(1, URANGE(2, get_obj_weight(projectile), 100) );
    }

    /*
     * Can't beat a dead char!
     */
    if ( victim->position == POS_DEAD || char_died(victim) )
    {
	extract_obj(projectile);
	return rVICT_DIED;
    }

    if ( wield )
	prof_bonus = weapon_prof_bonus_check( ch, wield, &prof_gsn );
    else
	prof_bonus = 0;

    if ( dt == TYPE_UNDEFINED )
    {
	dt = TYPE_HIT;
	if ( wield && wield->item_type == ITEM_MISSILE_WEAPON )
	    dt += wield->value[3];
    }

    /*
     * Calculate to-hit-armor-class-0 versus armor.
     */
    if ( IS_NPC(ch) )
    {
	thac0_00 = ch->mobthac0;
	thac0_32 =  0;
    }
    else
    {
	thac0_00 = class_table[ch->class]->thac0_00;
	thac0_32 = class_table[ch->class]->thac0_32;
    }

    thac0     = interpolate( ch->level, thac0_00, thac0_32 ) - GET_HITROLL(ch) + (dist*2);
    victim_ac = UMAX( -19, (int) (GET_AC(victim) / 10) );

    /* if you can't see what's coming... */
    if ( !can_see_obj( victim, projectile) )
	victim_ac += 1;
    if ( !can_see( ch, victim ) )
	victim_ac -= 4;

    /* Weapon proficiency bonus */
    victim_ac += prof_bonus;

    /*
     * The moment of excitement!
     */
    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
	;

    if ( diceroll == 0
    || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
    {
	/* Miss. */
	if ( prof_gsn != -1 )
	    learn_from_failure( ch, prof_gsn );

	/* Do something with the projectile */
	if ( number_percent() < 50 )
	    extract_obj(projectile);
	else
	{
	    if ( projectile->in_obj )
		obj_from_obj(projectile);
	    if ( projectile->carried_by )
		obj_from_char(projectile);
	    obj_to_room(projectile, victim->in_room);
	}
	damage( ch, victim, 0, dt );
	tail_chain( );
	return rNONE;
    }

    /*
     * Hit.
     * Calc damage.
     */

    if ( !wield )       /* dice formula fixed by Thoric */
	dam = proj_bonus;
    else
	dam = number_range(wield->value[1], wield->value[2]) + (proj_bonus / 10);

    /*
     * Bonuses.
     */
    dam += GET_DAMROLL(ch);

    if ( prof_bonus )
	dam += prof_bonus / 4;

    /*
     * Calculate Damage Modifiers from Victim's Fighting Style
     */
    if ( victim->position == POS_BERSERK )
       dam = 1.2 * dam;
    else if ( victim->position == POS_AGGRESSIVE )
       dam = 1.1 * dam;
    else if ( victim->position == POS_DEFENSIVE )
       dam = .85 * dam;
    else if ( victim->position == POS_EVASIVE )
       dam = .8 * dam;

    if ( !IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
    {
	dam += (int) (dam * LEARNED(ch, gsn_enhanced_damage) / 120);
	learn_from_success( ch, gsn_enhanced_damage );
    }

    if ( !IS_AWAKE(victim) )
	dam *= 2;

    if ( dam <= 0 )
        dam = 1;
   if (dam >=32000)
        dam = number_range(31000,32000); 
   /*KAYSER 2004*/

    plusris = 0;

    if ( IS_OBJ_STAT(projectile, ITEM_MAGIC) )
	dam = ris_damage( victim, dam, RIS_MAGIC );
    else
	dam = ris_damage( victim, dam, RIS_NONMAGIC );

    /*
     * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll	-Thoric
     */
    if ( wield )
	plusris = obj_hitroll( wield );

    /* check for RIS_PLUSx 					-Thoric */
    if ( dam )
    {
	int x, res, imm, sus, mod;

	if ( plusris )
	   plusris = RIS_PLUS1 << UMIN(plusris, 7);

	/* initialize values to handle a zero plusris */
	imm = res = -1;  sus = 1;

	/* find high ris */
	for ( x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1 )
	{
	   if ( IS_SET( victim->immune, x ) )
		imm = x;
	   if ( IS_SET( victim->resistant, x ) )
		res = x;
	   if ( IS_SET( victim->susceptible, x ) )
		sus = x;
	}
	mod = 10;
	if ( imm >= plusris )
	  mod -= 10;
	if ( res >= plusris )
	  mod -= 2;
	if ( sus <= plusris )
	  mod += 2;

	/* check if immune */
	if ( mod <= 0 )
	  dam = -1;
	if ( mod != 10 )
	  dam = (dam * mod) / 10;
    }

    if ( prof_gsn != -1 )
    {
      if ( dam > 0 )
        learn_from_success( ch, prof_gsn );
      else
        learn_from_failure( ch, prof_gsn );
    }

    /* immune to damage */
    if ( dam == -1 )
    {
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];
	    bool found = FALSE;

	    if ( skill->imm_char && skill->imm_char[0] != '\0' )
	    {
		act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		found = TRUE;
	    }
	    if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
	    {
		act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		found = TRUE;
	    }
	    if ( skill->imm_room && skill->imm_room[0] != '\0' )
	    {
		act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		found = TRUE;
	    }
	    if ( found )
	    {
		if ( number_percent() < 50 )
		    extract_obj(projectile);
		else
		{
		    if ( projectile->in_obj )
			obj_from_obj(projectile);
		    if ( projectile->carried_by )
			obj_from_char(projectile);
		    obj_to_room(projectile, victim->in_room);
		}
		return rNONE;
	    }
	}
	dam = 0;
    }
    if ( (retcode = damage( ch, victim, dam, dt )) != rNONE )
    {
	extract_obj(projectile);
	return retcode;
    }
    if ( char_died(ch) )
    {
	extract_obj(projectile);
	return rCHAR_DIED;
    }
    if ( char_died(victim) )
    {
	extract_obj(projectile);
	return rVICT_DIED;
    }

    retcode = rNONE;
    if ( dam == 0 )
    {
	if ( number_percent() < 50 )
	    extract_obj(projectile);
	else
	{
	    if ( projectile->in_obj )
		obj_from_obj(projectile);
	    if ( projectile->carried_by )
		obj_from_char(projectile);
	    obj_to_room(projectile, victim->in_room);
	}
	return retcode;
    }

/* weapon spells	-Thoric */
    if ( wield
    &&  !IS_SET(victim->immune, RIS_MAGIC)
    &&  !IS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC) )
    {
	AFFECT_DATA *aff;

	for ( aff = wield->pIndexData->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
	{
	    extract_obj(projectile);
	    return retcode;
	}
	for ( aff = wield->first_affect; aff; aff = aff->next )
	   if ( aff->location == APPLY_WEAPONSPELL
	   &&   IS_VALID_SN(aff->modifier)
	   &&   skill_table[aff->modifier]->spell_fun )
		retcode = (*skill_table[aff->modifier]->spell_fun) ( aff->modifier, (wield->level+3)/3, ch, victim );
	if ( retcode != rNONE || char_died(ch) || char_died(victim) )
	{
	    extract_obj(projectile);
	    return retcode;
	}
    }

    extract_obj(projectile);

    tail_chain( );
    return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage( CHAR_DATA *ch, int dam, int ris )
{
   int modifier;

   modifier = 10;
   if ( IS_SET(ch->immune, ris )  && !IS_SET(ch->no_immune, ris) )
     modifier -= 10;
   if ( IS_SET(ch->resistant, ris ) && !IS_SET(ch->no_resistant, ris) )
     modifier -= 2;
   if ( IS_SET(ch->susceptible, ris ) && !IS_SET(ch->no_susceptible, ris) )
   {
     if ( IS_NPC( ch )
     &&   IS_SET( ch->immune, ris ) )
	modifier += 0;
     else
	modifier += 2;
   }
   if ( modifier <= 0 )
     return -1;
   if ( modifier == 10 )
     return dam;
   return (dam * modifier) / 10;
}


/*
 * Inflict damage from a hit.   This is one damn big function.
 */
ch_ret damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char filename[256];
    int dameq;
    int maxdam;
    bool npcvict;
    bool loot;
    int  xp_gain;
    OBJ_DATA *damobj;
    OBJ_DATA *obj;
    char dificultad[100];
    CLAN_DATA *familia;
    NIVEL *nivel;
    AREA_DATA *area = get_area( ch->in_room->area->name );
    ch_ret retcode;
    int dampmod;
    CHAR_DATA *gch /*, *lch */;
    int init_gold, new_gold, gold_diff;
    sh_int anopc = 0;  /* # of (non-pkill) pc in a (ch) */
    sh_int bnopc = 0;  /* # of (non-pkill) pc in b (victim) */
    int valor = 100;

    if (dam >=32000)
    	dam = number_range(31000,32000); /*KAYSER 2004*/
    retcode = rNONE;

    if ( !ch )
    {
	bug( "Damage: null ch!", 0 );
	return rERROR;
    }
    if ( !victim )
    {
	bug( "Damage: null victim!", 0 );
	return rVICT_DIED;
    }

    if ( victim->position == POS_DEAD )
	return rVICT_DIED;

   if ( victim->level >= LEVEL_AVATAR
   && victim->level <= LEVEL_ACOLYTE
   && victim->position == POS_PREDECAP )
   {
         sprintf(buf, "&x&w%s &gha quedado malherido a manos de &w%s &gen %s." , victim->name, ch->name, ch->in_room->area->name );
         mensa_todos( ch, "vampiro", buf);
         stop_fighting(victim, TRUE);
         stop_fighting(ch, TRUE);
         if (in_arena(victim))
         char_to_room( victim, get_room_index( ROOM_VNUM_ALTAR ));
         do_sit(ch, FALSE);
         return rNONE;
    }

    if( xIS_SET(victim->act, ACT_AUTOMOB) || IS_SET(ch->in_room->area->flags, AFLAG_AUTOAREA) && !xIS_SET(victim->act, ACT_UALIZADO) )
        crear_automob( ch, victim );

    npcvict = IS_NPC(victim);
    /*
     * Check damage types for RIS				-Thoric
     */
    if ( dam && dt != TYPE_UNDEFINED )
    {
	if ( IS_FIRE(dt) )
	  dam = ris_damage(victim, dam, RIS_FIRE);
	else
	if ( IS_COLD(dt) )
	  dam = ris_damage(victim, dam, RIS_COLD);
	else
	if ( IS_ACID(dt) )
	  dam = ris_damage(victim, dam, RIS_ACID);
	else
	if ( IS_ELECTRICITY(dt) )
	  dam = ris_damage(victim, dam, RIS_ELECTRICITY);
	else
	if ( IS_ENERGY(dt) )
	  dam = ris_damage(victim, dam, RIS_ENERGY);
	else
	if ( IS_DRAIN(dt) )
	  dam = ris_damage(victim, dam, RIS_DRAIN);
	else
	if ( dt == gsn_poison || IS_POISON(dt) )
	  dam = ris_damage(victim, dam, RIS_POISON);
	else
	if ( dt == (TYPE_HIT + DAM_POUND) || dt == (TYPE_HIT + DAM_CRUSH)
	||   dt == (TYPE_HIT + DAM_STONE) || dt == (TYPE_HIT + DAM_PEA) )
	  dam = ris_damage(victim, dam, RIS_BLUNT);
	else
	if ( dt == (TYPE_HIT + DAM_STAB) || dt == (TYPE_HIT + DAM_PIERCE)
	||   dt == (TYPE_HIT + DAM_BITE) || dt == (TYPE_HIT + DAM_BOLT)
	||   dt == (TYPE_HIT + DAM_DART) || dt == (TYPE_HIT + DAM_ARROW) )
	  dam = ris_damage(victim, dam, RIS_PIERCE);
	else
	if ( dt == (TYPE_HIT + DAM_SLICE) || dt == (TYPE_HIT + DAM_SLASH)
	||   dt == (TYPE_HIT + DAM_WHIP)  || dt == (TYPE_HIT + DAM_CLAW) )
	  dam = ris_damage(victim, dam, RIS_SLASH);

	if ( dam == -1 )
	{
	    if ( dt >= 0 && dt < top_sn )
	    {
		bool found = FALSE;
		SKILLTYPE *skill = skill_table[dt];

		if ( skill->imm_char && skill->imm_char[0] != '\0' )
		{
		   act( AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR );
		   found = TRUE;
		}
		if ( skill->imm_vict && skill->imm_vict[0] != '\0' )
		{
		   act( AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT );
		   found = TRUE;
		}
		if ( skill->imm_room && skill->imm_room[0] != '\0' )
		{
		   act( AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT );
		   found = TRUE;
		}
		if ( found )
		   return rNONE;
	    }
	    dam = 0;
	}
    }

/*
 * Para los infectados por Debilidad Extincion nivel 2
 */
  if(xIS_SET(victim->afectado_por, DAF_DEBILIDAD))
  dam *= 1.25;

/*
 * Para los que usan garras de la bestia
 */
 if(xIS_SET(ch->afectado_por, DAF_GARRAS_BESTIA))
 {
 dam *= 1.75;
 if (!xIS_SET(ch->afectado_por, DAF_FORTALEZA))
 {
 if(victim->agravadas <= (victim->max_hit / 4))
 victim->agravadas += number_range(4, 20);
 }
 else
 {
 if(victim->agravadas <= (victim->max_hit / 4))
 victim->agravadas += number_range(10, 50);
 }
 }

    /*
     * Precautionary step mainly to prevent people in Hell from finding
     * a way out. --Shaddai
     */
     if (!IS_IMMORTAL (ch) )
	    if ( IS_SET(victim->in_room->room_flags, ROOM_SAFE) )
		dam = 0;

    if ( dam && npcvict && ch != victim )
    {
	if ( !xIS_SET( victim->act, ACT_SENTINEL ) )
 	{
	    if ( victim->hunting )
	    {
		if ( victim->hunting->who != ch )
		{
		    STRFREE( victim->hunting->name );
		    victim->hunting->name = QUICKLINK( ch->name );
		    victim->hunting->who  = ch;
		}
            }
	    else
            if ( !xIS_SET(victim->act, ACT_PACIFIST)  ) /* Gorog */
		start_hunting( victim, ch );
	}

	if ( victim->hating )
	{
   	    if ( victim->hating->who != ch )
 	    {
		STRFREE( victim->hating->name );
		victim->hating->name = QUICKLINK( ch->name );
		victim->hating->who  = ch;
	    }
	}
	else
	if ( !xIS_SET(victim->act, ACT_PACIFIST)  ) /* Gorog */
	    start_hating( victim, ch );
    }

    /*
     * Stop up any residual loopholes.
     */
    if ( dt == gsn_backstab ||
          dt == gsn_empalar || dt == gsn_tangana)
      maxdam = ch->level * 80;
    else
      maxdam = ch->level * 40;

    if ( ( dam > maxdam ) && ( !IS_IMMORTAL( ch ) ) )
	dam = maxdam;

    if (victim != ch)
    {
	/*
	 * Certain attacks are forbidden.
	 * Most other attacks are returned.
	 */
	if (is_safe(ch, victim, TRUE))
	    return rNONE;
	check_attacker(ch, victim);

	if (victim->position > POS_STUNNED)
	{
	    if (!victim->fighting && victim->in_room == ch->in_room)
		set_fighting(victim, ch);

	    /*
	       vwas: victim->position = POS_FIGHTING;
	     */
	    if ( IS_NPC(victim) && victim->fighting )
		victim->position = POS_FIGHTING;
	    else if (victim->fighting)
	    {
		switch (victim->style)
		{
		    case (STYLE_EVASIVE):
			victim->position = POS_EVASIVE;
			break;
		    case (STYLE_DEFENSIVE):
			victim->position = POS_DEFENSIVE;
			break;
		    case (STYLE_AGGRESSIVE):
			victim->position = POS_AGGRESSIVE;
			break;
		    case (STYLE_BERSERK):
			victim->position = POS_BERSERK;
			break;
		    default:
			victim->position = POS_FIGHTING;
		}

	    }

	}

	if (victim->position > POS_STUNNED)
	{
	    if (!ch->fighting && victim->in_room == ch->in_room)
		set_fighting(ch, victim);

	    /*
	     * If victim is charmed, ch might attack victim's master.
	     */
	    if (IS_NPC(ch)
		&& npcvict
		&& IS_AFFECTED(victim, AFF_CHARM)
		&& victim->master
		&& victim->master->in_room == ch->in_room
		&& number_bits(3) == 0)
	    {
		stop_fighting(ch, FALSE);
		retcode = multi_hit(ch, victim->master, TYPE_UNDEFINED);
		return retcode;
	    }
            if (IS_NPC(ch)
		&& npcvict
		&& victim->master
		&& victim->master->in_room == ch->in_room
		&& number_bits(3) == 0)
	    {
		stop_fighting(ch, FALSE);
		retcode = multi_hit(ch, victim->master, TYPE_UNDEFINED);
		return retcode;
	    }

	}


	/*
	 * More charm stuff.
	 */
	if (victim->master == ch)
	    stop_follower(victim);

	/*
	 * Pkill stuff.  If a deadly attacks another deadly or is attacked by
	 * one, then ungroup any nondealies.  Disabled untill I can figure out
	 * the right way to do it.
	 */


	/*
	   count the # of non-pkill pc in a ( not including == ch )
	 */
	for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
	    if (is_same_group(ch, gch) && !IS_NPC(gch)
		&& !IS_PKILL(gch) && (ch != gch))
		anopc++;

	/*
	   count the # of non-pkill pc in b ( not including == victim )
	 */
	for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
	    if (is_same_group(victim, gch) && !IS_NPC(gch)
		&& !IS_PKILL(gch) && (victim != gch))
		bnopc++;


	/*
	   only consider disbanding if both groups have 1(+) non-pk pc,
	   or when one participant is pc, and the other group has 1(+)
	   pk pc's (in the case that participant is only pk pc in group)
	 */
	if ((bnopc > 0 && anopc > 0)
	    || (bnopc > 0 && !IS_NPC(ch))
	    || (anopc > 0 && !IS_NPC(victim)))
	{
	    /*
	       Disband from same group first
	     */
	    if (is_same_group(ch, victim))
	    {
		/*
		   Messages to char and master handled in stop_follower
		 */
		act(AT_ACTION, "$n disbands from $N's group.",
		    (ch->leader == victim) ? victim : ch, NULL,
		    (ch->leader == victim) ? victim->master : ch->master,
		    TO_NOTVICT);
		if (ch->leader == victim)
		    stop_follower(victim);
		else
		    stop_follower(ch);
	    }
	    /*
	       if leader isnt pkill, leave the group and disband ch
	     */
	    if (ch->leader != NULL && !IS_NPC(ch->leader) &&
		!IS_PKILL(ch->leader))
	    {
		act(AT_ACTION, "$n disbands from $N's group.", ch, NULL,
		    ch->master, TO_NOTVICT);
		stop_follower(ch);
	    }
	    else
	    {
		for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
		    if (is_same_group(gch, ch) && !IS_NPC(gch) &&
			!IS_PKILL(gch) && gch != ch)
		    {
			act(AT_ACTION, "$n disbands from $N's group.", ch, NULL,
			    gch->master, TO_NOTVICT);
			stop_follower(gch);
		    }
	    }
	    /*
	       if leader isnt pkill, leave the group and disband victim
	     */
	    if (victim->leader != NULL && !IS_NPC(victim->leader) &&
		!IS_PKILL(victim->leader))
	    {
		act(AT_ACTION, "$n disbands from $N's group.", victim, NULL,
		    victim->master, TO_NOTVICT);
		stop_follower(victim);
	    }
	    else
	    {
		for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
		    if (is_same_group(gch, victim) && !IS_NPC(gch) &&
			!IS_PKILL(gch) && gch != victim)
		    {
			act(AT_ACTION, "$n disbands from $N's group.", gch, NULL,
			    gch->master, TO_NOTVICT);
			stop_follower(gch);
		    }
	    }
	}

	/*
	 * Inviso attacks ... not.
	 */
                  if (IS_AFFECTED(ch, AFF_INVISIBLE) )
	{
	    affect_strip(ch, gsn_invis);
	    affect_strip(ch, gsn_mass_invis);
	    xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
	    act(AT_MAGIC, "$n se materializa de repente.", ch, NULL, NULL, TO_ROOM);
	}
	/* Take away Hide */
	if (IS_AFFECTED(ch, AFF_HIDE) && !xIS_SET(ch->afectado_por, DAF_CAPA_SOMBRIA) )
	    xREMOVE_BIT(ch->affected_by, AFF_HIDE);

	/*
	 * Damage modifiers.
	 */
	if IS_AFFECTED(victim, AFF_SANCTUARY)
	    dam /= 2;

             if( !IS_NPC(victim) && !IS_NPC(ch) && !IS_IMMORTAL(ch))
                dam /= 2;

                  if(xIS_SET(ch->afectado_por, DAF_POTENCIA))
                  {
                                     if( ch->level < 35 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(3, 7));
                                else if( ch->level > 35 && ch->level  < 50 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(5, 12));
                                else if( ch->level < 70 && ch->level > 50 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(7, 15));
                                else if( ch->level < 90 && ch->level > 70 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(9, 20));
                                else if( ch->level < 105 && ch->level > 90 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(11, 23));
                                else if( ch->level < 125 && ch->level > 105 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(13, 27));
                                else if( ch->level < 150 && ch->level > 125 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(15, 29));
                                else if( ch->level < 175 && ch->level > 200 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(17, 36));
                                else if( ch->level < 225 && ch->level > 200 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(18, 41));
                                else if( ch->level < 239 && ch->level > 225 )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(19, 44));
                                else if( ch->level >= LEVEL_AVATAR )
                                        dam += ( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] * (13 / ch->generacion) * number_range(21, 50));
                                else
                                        dam += 0;
                  }

                 if((xIS_SET(victim->afectado_por, DAF_FORTALEZA) && IS_VAMPIRE(victim)))
                                        dam -= (int) (dam / 4);

        if ((xIS_SET(victim->afectado_por, DAF_INCORPOREO) && IS_VAMPIRE(victim)))
		dam -= (int) (dam / 4);

	if (IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch))
	    dam -= (int) (dam / 4);

            /* Proteccion contra vampiros habilidad unica de Cruzados SiGo */
                  if (IS_AFFECTED(victim, AFF_PROTECCION_VAMPIRICA) && IS_VAMPIRE(ch))
	    dam -= (int) (dam / 4);

	if (dam < 0)
	    dam = 0;


	/*
	 * Check for disarm, trip, parry, dodge and tumble.
	 */
	if (dt >= TYPE_HIT && ch->in_room == victim->in_room)
	{
	    if (IS_NPC(ch)
		&& xIS_SET(ch->defenses, DFND_DISARM)
		&& ch->level > 9
		&& number_percent() < ch->level / 3) /* Was 2 try this --Shaddai*/
		disarm(ch, victim);

	    if (IS_NPC(ch)
		&& xIS_SET(ch->attacks, ATCK_TRIP)
		&& ch->level > 5
		&& number_percent() < ch->level / 2)
		trip(ch, victim);

	    if (check_parry(ch, victim))
		return rNONE;
	    if (check_dodge(ch, victim))
		return rNONE;
	    if (check_tumble(ch, victim))
		return rNONE;
	}

	/*
	 * Check control panel settings and modify damage
	 */
	if (IS_NPC(ch))
	{
	    if (npcvict)
		dampmod = sysdata.dam_mob_vs_mob;
	    else
		dampmod = sysdata.dam_mob_vs_plr;
	}
	else
	{
	    if (npcvict)
		dampmod = sysdata.dam_plr_vs_mob;
	    else
		dampmod = sysdata.dam_plr_vs_plr;
	}
	if (dampmod > 0)
	    dam = (dam * dampmod) / 100;

	dam_message(ch, victim, dam, dt);
	
	if( dam >= 30000 )
	{
		sprintf(buf, "&z[&O%s &gha echo un danyo de &O%d&g pv contra %s. DIOS!!! Q Burro!!!&z]"
         , ch->name, dam,  victim->name );
         echo_to_all(AT_GREY, buf, ECHOTAR_ALL);        
	}
    }

    /*
     * Code to handle equipment getting damaged, and also support  -Thoric
     * bonuses/penalties for having or not having equipment where hit
     */
    if (dam > 10 && dt != TYPE_UNDEFINED)
    {
	/* get a random body eq part */
	dameq  = number_range(WEAR_LIGHT, WEAR_EYES);
	damobj = get_eq_char(victim, dameq);
	if ( damobj )
	{
	    if ( dam > get_obj_resistance(damobj)
	    &&   number_bits(1) == 0 )
	    {
		set_cur_obj(damobj);
		damage_obj(damobj);
	    }
	    dam -= 5;  /* add a bonus for having something to block the blow */
	}
	else
	    dam += 5;  /* add penalty for bare skin! */
    }

    /*
     * Vulnerabilidad al sol de los Cainitas
     * Vampiromud2.0beta COLIKOTRON Code Team(c)
     */

     if(IS_VAMPIRE(victim))
     {
     if((time_info.hour > 6
     || time_info.hour < 20))
     dam *= 1.3;
     else
     dam /= ch->pcdata->descanso;
     }

    /*
     * Debilidad de los Cainitas ante la luz solar
     * Vampiromud2.0beta COLIKOTRON Code Team(c)
     */

     if(IS_VAMPIRE(ch))
     {
     if((time_info.hour > 6
     || time_info.hour < 20))
     dam /= 1.3;
     else
     dam *= ch->pcdata->descanso;
     }


    /*
     * Hurt the victim.
     * Inform the victim of his new state.
     */
    victim->hit -= dam;

    /*
     * A veces luchando se ganara algun px
     */

    if ( dam && ch != victim
    &&  !IS_NPC(ch) && ch->fighting && ch->fighting->xp )
    {
	if ( ch->fighting->who == victim )
	  {
	    xp_gain = number_range (0,10);
	    if (xp_gain > 8)
	    	xp_gain = 1;
	    else
	    	xp_gain = 0;
	  }
	else
	  {
	    xp_gain = number_range (0,10);
	    if (xp_gain > 9)
	    	xp_gain = 1;
	    else
	    	xp_gain = 0;
	  }
	if ( dt == gsn_backstab || dt == gsn_circle || dt == gsn_empalar || dt == gsn_tangana )
	    xp_gain =  number_range (0,1);

        if( IS_NPC(victim))
        {
                MOB_INDEX_DATA *vnum = get_mob_index(victim->pIndexData->vnum);
                if( vnum->vnum < area->low_m_vnum || vnum->vnum > area->hi_m_vnum )
                        bug( "%s mata a %s en %s siendo mob de otro area.\n\r", ch->name, victim->short_descr, area->name );
                else
                gain_exp( ch, xp_gain );
        }
    }


    if ( !IS_NPC(victim)
    &&   victim->level >= LEVEL_CREATOR
    &&   victim->hit < 1 )
       victim->hit = 1;

     /*
      * Los Avatares Angeles y Demonios no mueren a manos de jugadores, deben ser
      * decapitados o diablerizados
      */

     if ( !IS_NPC(victim)
     && !IS_NPC(ch)
     && ch->fighting
     && victim->hit < -10
     && victim->level < LEVEL_IMMORTAL +2
     && victim->level > 129 )
     {
     CHAR_DATA  *rch;
     victim->hit = -10;

     for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
    {
	if ( rch->fighting )
	{
                        stop_fighting( rch, TRUE );
                        stop_fighting( victim, TRUE);
	}

        if ( victim->fighting )
	{
                        stop_fighting( rch, TRUE );
                        stop_fighting( victim, TRUE);
	}


        stop_hating( rch );
        stop_hunting( rch );
        stop_fearing( rch );
        stop_hating( victim );
        stop_hunting( victim );
        stop_fearing( victim );

    }

    /*
     * Si la victima es avatar o superior y esta a menos de -1 no recibe danyos
     */
   if (!IS_NPC(victim)
   && !IS_NPC(ch) )
   {

   if ( victim->level >= LEVEL_AVATAR
   && victim->level <= LEVEL_ACOLYTE
   && victim->position == POS_PREDECAP )
   {
        if( in_arena(victim) )
        {
          char_to_room( victim, get_room_index( ROOM_VNUM_ALTAR ) );
          return rNONE;
        }
        if (IS_VAMPIRE(victim)
        && IS_VAMPIRE(ch) )
        {
                send_to_char( "&OEsta incapacitado, &gDIABLERIZALO.\n\r", ch );
                sprintf(buf, "&z[&O%s &gha quedado malherido a manos de &O%s &gen %s.&z]"
         , victim->name, ch->name,  ch->in_room->area->name );
         echo_to_all(AT_GREY, buf, ECHOTAR_ALL);
                return rNONE;
        }

        if (!IS_VAMPIRE(ch)
        && !IS_VAMPIRE(victim) )
        {
                send_to_char( "&OEsta incapacitado, &gDECAPITALO.\n\r", ch );
                 sprintf(buf, "&z[&O%s &gha quedado malherido a manos de &O%s &gen %s.&z]"
         , victim->name, ch->name,  ch->in_room->area->name );
         echo_to_all(AT_GREY, buf, ECHOTAR_ALL);
                return rNONE;
        }

        if (IS_VAMPIRE(victim)
        && !IS_VAMPIRE(ch) )
        {
                send_to_char("&OEsta incapacitado, &gDECAPITALO.\n\r", ch );
                 sprintf(buf, "&z[&O%s &gha quedado malherido a manos de &O%s &gen %s.&z]"
         , victim->name, ch->name,  ch->in_room->area->name );
         echo_to_all(AT_GREY, buf, ECHOTAR_ALL);
                return rNONE;
        }
        if (IS_VAMPIRE(ch)
        && !IS_VAMPIRE(victim) )
        {
                send_to_char("&OEsta incapacitado, &gDECAPITALO.\n\r", ch );
                 sprintf(buf, "&z[&O%s &gha quedado malherido a manos de &O%s &gen %s.&z]"
         , victim->name, ch->name,  ch->in_room->area->name );
         echo_to_all(AT_GREY, buf, ECHOTAR_ALL);
                return rNONE;
        }
       }
     }

    }

    /* Make sure newbies dont die */

    if (!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
	victim->hit = 1;

    if ( dam > 0 && dt > TYPE_HIT
    &&  !IS_AFFECTED( victim, AFF_POISON )
    &&   is_wielding_poisoned( ch )
    &&  !IS_SET( victim->immune, RIS_POISON )
    &&  !saves_poison_death( ch->level, victim ) )
    {
	AFFECT_DATA af;

	af.type      = gsn_poison;
	af.duration  = 20;
	af.location  = APPLY_STR;
	af.modifier  = -2;
	af.bitvector = meb(AFF_POISON);
	affect_join( victim, &af );
	victim->mental_state = URANGE( 20, ch->mental_state + (IS_PKILL(ch) ? 1 : 2), 100 );
    }

    /*
     * Vampire self preservation				-Thoric
     */
    if ( !IS_NPC(victim) )
    {
    if ( IS_VAMPIRE(victim) )
    {
      if ( dam >= (victim->max_hit / 10) )	/* get hit hard, lose blood */
	gain_condition(victim, COND_BLOODTHIRST, -1 - (victim->level / 20));
      if ( victim->hit <= (victim->max_hit / 8)
      && victim->pcdata->condition[COND_BLOODTHIRST]>5 )
      {
	gain_condition(victim, COND_BLOODTHIRST,
		-URANGE(3, victim->level / 10, 8) );
	victim->hit += URANGE( 4, (victim->max_hit / 30), 15);
	set_char_color(AT_BLOOD, victim);
	send_to_char("Te resistes a morir!\n\r", victim);
      }
    }
    }
    if ( !npcvict
    &&   get_trust(victim) >= LEVEL_IMMORTAL+2
    &&    get_trust(ch)    >= LEVEL_IMMORTAL+2
    &&   victim->hit < 1 )
	victim->hit = 1;
    update_pos( victim );

    switch( victim->position )
    {
    case POS_MORTAL:
	act( AT_DYING, "$n ha sido mortalmente herido y morira si no le ayuda nadie.",
	    victim, NULL, NULL, TO_ROOM );
	act( AT_DANGER, "Has sido herido mortalmente moriras rapido si no recibes ayuda.",
	victim, NULL, NULL, TO_CHAR );
	break;

    case POS_INCAP:
	act( AT_DYING, "$n ha sido incapacitado y morira si nadie le ayuda pronto.",
	    victim, NULL, NULL, TO_ROOM );
	act( AT_DANGER, "Has sido incapacitado y moriras si no recibes ayuda.",
	victim, NULL, NULL, TO_CHAR );
	break;

    case POS_STUNNED:
        if ( !IS_AFFECTED( victim, AFF_PARALYSIS ) )
        {
	  act( AT_ACTION, "$n esta atontado pero se recuperara.",
	    victim, NULL, NULL, TO_ROOM );
	  act( AT_HURT, "Estas antontado pero te recuperaras.",
	    victim, NULL, NULL, TO_CHAR );
	}
	break;

    case POS_DEAD:
	if ( dt >= 0 && dt < top_sn )
	{
	    SKILLTYPE *skill = skill_table[dt];

	    if ( skill->die_char && skill->die_char[0] != '\0' )
	      act( AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR );
	    if ( skill->die_vict && skill->die_vict[0] != '\0' )
	      act( AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT );
	    if ( skill->die_room && skill->die_room[0] != '\0' )
	      act( AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOTVICT );
	}
	act( AT_DGREEN, "$n esta MUERTO!!", victim, 0, 0, TO_ROOM );
	act( AT_DEAD, "Te han MATADO!!\n\r", victim, 0, 0, TO_CHAR );
	break;

    default:
	/*
	 * Victim mentalstate affected, not attacker -- oops ;)
	 * Thanks to gfinello@mail.karmanet.it for finding this bug
	 */
	if ( dam > victim->max_hit / 4 )
	{
	   act( AT_HURT, "Eso realmente DOLIO!", victim, 0, 0, TO_CHAR );
	}
	if ( victim->hit < victim->max_hit / 4 )

	{
	   act( AT_DANGER, "Te gustaria que tus heridas dejaran de SANGRAR tanto!",
		victim, 0, 0, TO_CHAR );
	}
	break;
    }

    if(IS_CLANNED(victim))
    {
    if((!IS_NPC(victim) && victim != ch))
    {
    if((victim->pcdata->cnt_critico < 2000) && dam >= 50)
           victim->pcdata->cnt_critico += dam / 50;
    if((victim->pcdata->cnt_critico < 2000) && dam < 50)
           victim->pcdata->cnt_critico++;

           if(victim->pcdata->cnt_critico >= 1000)
           {
                int aux = 0;
                aux = number_range( 1, 10 );

                if( aux >= 7 )
                send_to_char( "&R&REstas realmente ENFURECIDO!!!\n\r", victim );

                victim->pcdata->cnt_critico = 2000;
           }
    }
    }

    /*
     * Sleep spells and extremely wounded folks.
     */
    if ( !IS_AWAKE(victim)		/* lets make NPC's not slaughter PC's */
    &&   !IS_AFFECTED( victim, AFF_PARALYSIS ) )
    {
	if ( victim->fighting
	&&   victim->fighting->who->hunting
	&&   victim->fighting->who->hunting->who == victim )
	   stop_hunting( victim->fighting->who );

	if ( victim->fighting
	&&   victim->fighting->who->hating
	&&   victim->fighting->who->hating->who == victim )
	   stop_hating( victim->fighting->who );

	if (!npcvict && IS_NPC(ch))
	  stop_fighting( victim, TRUE );
	else
	  stop_fighting( victim, FALSE );
    }

    /*
     * Payoff for killing things.
     */
    if ( victim->position == POS_DEAD )
    {
	group_gain( ch, victim );

	if ( !npcvict )
	{
	    sprintf( log_buf, "%s (%d) asesinado por %s en %d",
		victim->name,
		victim->level,
		(IS_NPC(ch) ? ch->short_descr : ch->name),
		victim->in_room->vnum );
	    log_string( log_buf );
	    to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );

            if (!IS_NPC( ch ) )
            {
            sprintf ( buf, "%s muerto por %s", victim->name, ch->name);
            mensa_todos(ch, "asesinato", buf);
            }
            if (IS_NPC( ch ) )
            {
            sprintf ( buf, "%s muerto por %s", victim->name, ch->short_descr);
            mensa_todos(ch, "muerte", buf);
            }
            if (!IS_NPC( ch ) && !IS_IMMORTAL( ch ) && ch->pcdata->clan
	    &&   ch->pcdata->clan->clan_type != CLAN_ORDER
            &&   ch->pcdata->clan->clan_type != CLAN_GUILD
            &&   victim != ch )
            {
                sprintf( filename, "%s%s.record", CLAN_DIR, ch->pcdata->clan->name );
                sprintf( log_buf, "&P(%2d) %-12s &wvs &P(%2d) %s &P%s ... &w%s",
		  ch->level,
                  ch->name,
		  victim->level,
		  !CAN_PKILL( victim ) ? "&W<Peaceful>" :
		    victim->pcdata->clan ? victim->pcdata->clan->badge :
		      "&P(&WUnclanned&P)",
		  victim->name,
                  ch->in_room->area->name );
		if ( victim->pcdata->clan &&
		     victim->pcdata->clan->name == ch->pcdata->clan->name)
		;
		else
		  append_to_file( filename, log_buf );
            }

	    /*
	     * Dying penalty:
	     * 1/2 way back to previous level.
	     */
	    if ( victim->exp > exp_level(victim, victim->level) )
		gain_exp( victim, (exp_level(victim, victim->level) - victim->exp)/2 );

	    /*
	     * New penalty... go back to the beginning of current level.
	     victim->exp = exp_level( victim, victim->level );
	     */
	}
	else
	if ( !IS_NPC(ch) )		/* keep track of mob vnum killed */
	    add_kill( ch, victim );

	check_killer( ch, victim );

	if ( ch->in_room == victim->in_room )
	    loot = legal_loot( ch, victim );
	else
	    loot = FALSE;

	set_cur_char(victim);
	raw_kill( ch, victim );
	victim = NULL;

	if ( !IS_NPC(ch) && loot )
	{
	   /* Autogold by Scryn 8/12 */
	    if ( xIS_SET(ch->act, PLR_AUTOGOLD) )
	    {
		init_gold = ch->gold;
		do_get( ch, "coins corpse" );
		new_gold = ch->gold;
		gold_diff = (new_gold - init_gold);
		if (gold_diff > 0)
                {
                if ( IS_CLANNED(ch))
                {

                familia = get_clan( ch->in_room->area->propietario );

                if( ch->in_room->area->propietario != ch->pcdata->clan->name )
                {
                if( familia->nv_guerra[ch->pcdata->clan->numero] == 1 );
                valor = 10;

                if( familia->nv_guerra[ch->pcdata->clan->numero] == 2 );
                valor = 25;

                if( familia->nv_guerra[ch->pcdata->clan->numero] == 3 );
                valor = 45;
                }
                if ( gold_diff <= valor )
                {

                ch->pcdata->clan->oro += 1;

                ch_printf( ch, "&wConsigues una onza de oro para tu familia.\n\r" );
                }
                else
                {
                ch->pcdata->clan->oro += gold_diff /valor;
                ch_printf( ch, "&wConsigues %d onzas de oro para tu familia.\n\r", gold_diff /valor );
                }
              }
                  sprintf(buf1,"%d",gold_diff);
		  do_split( ch, buf1 );
		}
	    }
	    if ( xIS_SET(ch->act, PLR_AUTOLOOT)
	    &&   victim != ch )  /* prevent nasty obj problems -- Blodkai */
		do_get( ch, "todo corpse" );
	    else
		do_look( ch, "in corpse" );

	    if ( xIS_SET(ch->act, PLR_AUTOSAC) )
		do_sacrifice( ch, "corpse" );
	}

	if ( IS_SET( sysdata.save_flags, SV_KILL ) )
	   save_char_obj( ch );
	return rVICT_DIED;
    }

    if ( victim == ch )
	return rNONE;

    /*
     * Take care of link dead people.
     */
    if ( !npcvict && !victim->desc
    && !IS_SET( victim->pcdata->flags, PCFLAG_NORECALL ) )
    {
	if ( number_range( 0, victim->wait ) == 0)
	{
	    do_recall( victim, "" );
	    return rNONE;
	}
    }

    /*
     * Wimp out?
     */
    if ( npcvict && dam > 0 )
    {
	if ( ( xIS_SET(victim->act, ACT_WIMPY) && number_bits( 1 ) == 0
	&&   victim->hit < victim->max_hit / 2 )
	||   ( IS_AFFECTED(victim, AFF_CHARM) && victim->master
	&&     victim->master->in_room != victim->in_room ) )
	{
	    start_fearing( victim, ch );
	    stop_hunting( victim );
	    do_flee( victim, "" );
	}
    }

    if ( !npcvict
    &&   victim->hit > 0
    &&   victim->hit <= victim->wimpy
    &&   victim->wait == 0 )
	do_flee( victim, "" );
    else
    if ( !npcvict && xIS_SET( victim->act, PLR_FLEE ) )
	do_flee( victim, "" );

    tail_chain( );
    return rNONE;
}



/*
 * Changed is_safe to have the show_messg boolian.  This is so if you don't
 * want to show why you can't kill someone you can't turn it off.  This is
 * useful for things like area attacks.  --Shaddai
 */
bool is_safe( CHAR_DATA *ch, CHAR_DATA *victim, bool show_messg )
{

    if ( char_died(victim) || char_died(ch) )
    	return TRUE;

    /* Thx Josh! */
    if ( who_fighting( ch ) == ch )
	return FALSE;

    if ( !victim ) /*Gonna find this is_safe crash bug -Blod*/
    {
        bug( "Is_safe: %s opponent does not exist!", ch->name );
        return TRUE;
    }
    if ( !victim->in_room )
    {
	bug( "Is_safe: %s has no physical location!", victim->name );
	return TRUE;
    }

    if ( IS_SET( victim->in_room->room_flags, ROOM_SAFE ) &&  !IS_IMMORTAL (ch ) )
    {
        if ( show_messg ) {
	set_char_color( AT_MAGIC, ch );
	send_to_char( "No puedes pelear en una habitacion sagrada.\n\r", ch );
    }
	return TRUE;
    }

    if((IS_PACIFIST(ch)) && !EN_ARENA(ch)) /* Fireblade */
    {
        if ( show_messg ) {
    	set_char_color(AT_MAGIC, ch);
      ch_printf(ch, "Eres pacifico, no deseas luchar.\n\r");
	}
	return TRUE;
    }

    if ( IS_PACIFIST(victim) ) /* Gorog */
    {
        char buf[MAX_STRING_LENGTH];
        if ( show_messg ) {
        sprintf(buf, "%s es pacifico y no desea luchar.\n\r",
                capitalize(victim->short_descr));
        set_char_color( AT_MAGIC, ch );
        send_to_char( buf, ch);
	}
        return TRUE;
    }

    if ( !IS_NPC( ch ) && ch->level >= LEVEL_IMMORTAL+2 )
        return FALSE;

    if( !IS_NPC( ch ) && !IS_NPC( victim )
    &&   ch != victim
    &&   IS_SET( victim->in_room->area->flags, AFLAG_NOPKILL ) )
    {
        if ( show_messg ) {
        set_char_color( AT_IMMORT, ch );
        send_to_char( "Los Dioses han prohibido el pk en este area.\n\r", ch );
    }
        return TRUE;
    }

    if ( IS_NPC(ch) || IS_NPC(victim) )
	return FALSE;

    if ( (get_age( ch ) < 18 || ch->level < 20 ) && !EN_ARENA(  ch ) )
    {
        if ( show_messg ) {
	set_char_color( AT_WHITE, ch );
   send_to_char( "Aun no estas preparado, necesitas tener mas anyos de experiencia. \n\r", ch );
	}
	return TRUE;
    }

    if ( (get_age( victim ) < 18 || victim->level < 20) && !EN_ARENA( victim ) )
    {
        if ( show_messg ) {
	set_char_color( AT_WHITE, ch );
   send_to_char( "Es demasiado joven para morir.\n\r", ch );
	}
	return TRUE;
    }

    if ( (((ch->level) - (victim->level)) > 5) && !EN_ARENA( ch ) && !xIS_SET(victim->act, PLR_PKTOTAL))
    {
        if ( show_messg ) {
		set_char_color( AT_IMMORT, ch );
        send_to_char( "Los Dioses no permiten que mates a un jugador tan por debajo de tu nivel.\n\r", ch );
    }
	return TRUE;
    }
/* No tokeis nada de aki (SiGo) */

    if( !IS_NPC(victim) )
	if((((ch->level - victim->level) < -5)) && !EN_ARENA(victim) && !xIS_SET(ch->act, PLR_PKTOTAL))
    {
        send_to_char( "&RLe atacas!!! Te conviertes en Pk Total!!\n\r", ch );
        xSET_BIT(ch->act, PLR_PKTOTAL);
        ch->cnt_pktotal = 5;
        return FALSE;
    }

    return FALSE;
}

/*
 * just verify that a corpse looting is legal
 */
bool legal_loot( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /* anyone can loot mobs */
    if ( IS_NPC(victim) )
      return TRUE;
    /* non-charmed mobs can loot anything */
    if ( IS_NPC(ch) && !ch->master )
      return TRUE;
    /* members of different clans can loot too! -Thoric */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
    &&    IS_SET( ch->pcdata->flags, PCFLAG_DEADLY )
    &&    IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
	return TRUE;
    return FALSE;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
    /* -----------------21/12/00 23:40----------------------
     * Modulo para la ganancia de Puntos Pk y de Generacion
     * Por SiGo y los Implementadores de VampiroMud
     * -----------------------------------------------------*/

    /*
     * NPC's are fair game.
     */
    if ( IS_NPC(victim) )
    {
	if ( !IS_NPC( ch ) )
	{
	  int level_ratio;
     	  level_ratio = URANGE( 1, ch->level / victim->level, 80);
	  if ( ch->pcdata->clan )
	    ch->pcdata->clan->mkills++;
	  ch->pcdata->mkills++;
	  ch->in_room->area->mkills++;
	  if ( ch->pcdata->deity )
	  {
	    if ( victim->race == ch->pcdata->deity->npcrace )
	      adjust_favor( ch, 3, level_ratio );
	    else
	      if ( victim->race == ch->pcdata->deity->npcfoe )
		adjust_favor( ch, 17, level_ratio );
	      else
                adjust_favor( ch, 2, level_ratio );
	  }
	}
	return;
    }



    /*
     * If you kill yourself nothing happens.
     */

    if ( ch == victim || ch->level >= LEVEL_IMMORTAL+2 )
      return;

    /*
     * Any character in the arena is ok to kill.
     * Added pdeath and pkills here
     */
    if ( in_arena( ch ) )
    {

      if ( !IS_NPC(ch) && !IS_NPC(victim) )
	{
     ch->pcdata->pkills++;
     victim->pcdata->pdeaths++;
     adjust_hiscore( "pkill", ch, ch->pcdata->pkills ); /* cronel hiscore */

   }

    }

    /*
     * So are killers and thieves.
     */
    if ( xIS_SET(victim->act, PLR_KILLER)
    ||   xIS_SET(victim->act, PLR_THIEF) )
    {
	if ( !IS_NPC( ch ) )
	{
	  if ( ch->pcdata->clan )
	  {

       if ( victim->level < 10 )
	      ch->pcdata->clan->pkills[0]++;
	    else if ( victim->level < 15 )
	      ch->pcdata->clan->pkills[1]++;
	    else if ( victim->level < 20 )
	      ch->pcdata->clan->pkills[2]++;
	    else if ( victim->level < 30 )
	      ch->pcdata->clan->pkills[3]++;
	    else if ( victim->level < 40 )
	      ch->pcdata->clan->pkills[4]++;
	    else if ( victim->level < 50 )
	      ch->pcdata->clan->pkills[5]++;
	    else
	      ch->pcdata->clan->pkills[6]++;

     }

     ch->pcdata->pkills++;
	  ch->in_room->area->pkills++;
          adjust_hiscore( "pkill", ch, ch->pcdata->pkills ); /* cronel hiscore */

   }
	return;
    }

    /* clan checks					-Thoric */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
    &&    IS_SET( ch->pcdata->flags, PCFLAG_DEADLY )
    &&    IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) )
    {
      /* not of same clan? Go ahead and kill!!! */
      if ( !ch->pcdata->clan
      ||   !victim->pcdata->clan
      ||   ( ch->pcdata->clan->clan_type != CLAN_NOKILL
      &&   victim->pcdata->clan->clan_type != CLAN_NOKILL
      &&   ch->pcdata->clan != victim->pcdata->clan ) )
      {
      	if ( ch->pcdata->clan )
	{
            if ( victim->level < 39 )
              ch->pcdata->clan->pkills[0]++;
            else if ( victim->level < 49 )
              ch->pcdata->clan->pkills[1]++;
            else if ( victim->level < 59 )
              ch->pcdata->clan->pkills[2]++;
            else if ( victim->level < 69 )
              ch->pcdata->clan->pkills[3]++;
            else if ( victim->level < 79 )
              ch->pcdata->clan->pkills[4]++;
            else if ( victim->level < LEVEL_HERO )
              ch->pcdata->clan->pkills[5]++;
            else
              ch->pcdata->clan->pkills[6]++;

   }


     ch->pcdata->pkills++;
     adjust_hiscore( "pkill", ch, ch->pcdata->pkills ); /* cronel hiscore */
	  ch->hit = ch->max_hit;
	  ch->mana = ch->max_mana;
	  ch->move = ch->max_move;

   if ( ch->pcdata )
	  ch->pcdata->condition[COND_BLOODTHIRST] = (10 + ch->level + (13 / ch->generacion) * 40 );
	update_pos(victim);
   if ( victim != ch )
	{
     act( AT_MAGIC, "Rayos de tremenda energia fluyen a traves de $n.", ch, victim->name, NULL, TO_ROOM );
     act( AT_MAGIC, "Rayos de tremenda energia fluyen a traves de tu cuerpo.", ch, victim->name, NULL, TO_CHAR );
     act( AT_MAGIC, "Has arrebatado un punto Pk.", ch, victim->name, NULL, TO_CHAR );
	}
	if ( victim->pcdata->clan )
	{
            if ( ch->level < 39 )
              victim->pcdata->clan->pdeaths[0]++;
            else if ( ch->level < 49 )
              victim->pcdata->clan->pdeaths[1]++;
            else if ( ch->level < 59 )
              victim->pcdata->clan->pdeaths[2]++;
            else if ( ch->level < 69 )
              victim->pcdata->clan->pdeaths[3]++;
            else if ( ch->level < 79 )
              victim->pcdata->clan->pdeaths[4]++;
            else if ( ch->level < LEVEL_HERO )
              victim->pcdata->clan->pdeaths[5]++;
            else
              victim->pcdata->clan->pdeaths[6]++;
   }

   victim->pcdata->pdeaths++;
   if( !in_arena(ch) )
   {
   send_to_char( "Rayos de tremenda energia fluyen a traves de tu cuerpo\n\r.", victim );
   send_to_char( "Te han robado un punto Pk.", victim );
	adjust_favor( victim, 11, 1 );
	adjust_favor( ch, 2, 1 );
	add_timer( victim, TIMER_PKILLED, 115, NULL, 0 );
	WAIT_STATE( victim, 3 * PULSE_VIOLENCE );

    if ( ch->level  < LEVEL_AVATAR )
    {
    char buf[MAX_STRING_LENGTH];
    int ganancia;
    int total;
    int diff;
    int los;

    diff = victim->level - ch->level;

    if ( diff <= -10 ) ganancia = 20;
    else if ( diff <=  -5 ) ganancia = 30;
    else if ( diff <=  -2 ) ganancia = 40;
    else if ( diff == 0 ) ganancia = 450;
    else if ( diff <=   1 ) ganancia = 500;
    else if ( diff <=   4 ) ganancia = 700;
    else if ( diff <=   9 ) ganancia = 1000;
    else                    ganancia = (1000 + (100*diff));

    if( ch->pcdata->renacido > victim->pcdata->renacido )
    ganancia = 5;

    total = ganancia;

     sprintf(buf, "&gHas conseguido robar &w%d &gpuntos de experiencia a %s.\n\r", total, victim->name );
     send_to_char( buf, ch );

     sprintf(buf, "&g%s te ha robado &w%d &gpuntos de experiencia.\n\r",  ch->name, total );
     send_to_char( buf, victim );

     sprintf(buf, "&W%s &gha dado muerte a &W%s &ggana &W%d &gpuntos de experiencia" ,  ch->name, victim->name, total);
     mensa_todos(ch,"asesinato",buf);


     gain_exp(ch, total);
     los = total;
     gain_exp(victim, 0 - los);
     }
    if ( ch->level  == LEVEL_HERO)
    {
    char buf[MAX_STRING_LENGTH];
    int ganancia;
    int diff;
    int total;
    int los;

    diff = victim->level - ch->level;

if ( diff <= -10 ) ganancia = 20;
    else if ( diff <=  -5 ) ganancia = 30;
    else if ( diff <=  -2 ) ganancia = 40;
    else if ( diff == 0 ) ganancia = 450;
    else if ( diff <=   1 ) ganancia = 500;
    else if ( diff <=   4 ) ganancia = 700;
    else if ( diff <=   9 ) ganancia = 1000;
    else                    ganancia = (1000 + (100*diff));

    total = ganancia;
    los =  ganancia;

     sprintf(buf, "&gHas conseguido robar &w%d &gpuntos de experiencia a %s.\n\r", total, victim->name );
     send_to_char( buf, ch );

     if ( victim->exp_acumulada > 0 )
     {
     sprintf(buf, "&g%s te ha robado &w%d &gpuntos de experiencia.\n\r",  ch->name, total);
     send_to_char( buf, victim );
     }
     if ( victim->exp_acumulada <= 0 )
     {
     sprintf(buf, "&gNo tienes experiencia acumulada, espabila nen.\n\r" );
     send_to_char( buf, victim );
     }
     sprintf(buf, "&W%s &gha dado muerte a &W%s &ggana &W%d &gpuntos de experiencia" ,  ch->name, victim->name, total);
     mensa_todos(ch,"Asesinato",buf);

     gain_exp(ch, total);
     if ( victim->exp_acumulada >= los )
     victim->exp_acumulada -= los;
     else
     victim->exp_acumulada = 0;
     }
}
     /*
      * Si la victima esta en Torneo y se encuentra en la arena
      */

     if ( in_arena( victim ) || in_arena( ch ) )
     {
      send_to_char( "&PRecuerda: &WNo ganas puntos de Pk en Arenas\n\r", ch );
      send_to_char( "&RRecuerda: &BNo pierdes puntos de Pk en Arenas\n\r", victim );

      ch->pcdata->pkills++;
      victim->pcdata->pdeaths++;
      return;
     }


     /*
      * Si los Puntos Pk de la victima son 0 se cancela la ganancia y perdida
      */


   /*   if( ch->generacion >=12 && victim->pcdata->genepkills <= 0 )
       {
send_to_char("No has conseguido arrebatar puntos de Pk.\n\r", ch );
send_to_char("Ganancia de puntos Pk cancelada: Tu vitima tenia 0 puntos akumulados.\n\r", ch );
send_to_char("Da gracias a SiGo por no perder tu Generacion al no tener puntos Pk.\n\r", victim);
     */
     /*
      * Si se contabiliza la Pk y Pdeath normales
      */

    /*  ch->pcdata->pkills++;
      victim->pcdata->pdeaths++;
      return;
      }*/

     /*
     * Si el jugador es Matusalen o superior y la victima tiene mas de tres generaciones
     * de diferencia, se cancela la ganancia, pero se contabiliza la Pk y la Pdeath normales
     */

    if (ch->generacion <= GENERACION_DECIMA  && ch->generacion > 0 )
     {
     if ( ch->generacion - victim->generacion < -1
    ||   victim->generacion - ch->generacion < -1 )
         {
         send_to_char("No esperarias robarle puntos Pk a el verdad?  \n\r", ch );
         send_to_char("Da gracias de que su Generacion era demasiado buena para ti\n\r", victim );

         ch->pcdata->pkills++;
         victim->pcdata->pdeaths++;
         return;
         }
     }
     /*
      * Si no hay inconvenientes se konsiguen los puntos pk
      */
     if ( !IS_NPC (ch))

     if( ch->generacion  != GENERACION_ANTEDILUVIANO
     || ch->generacion  != GENERACION_SEGUNDA
     || ch->generacion  != GENERACION_CAIN
     || victim->pcdata->genepkills <= 0
     && ch->pcdata->renacido <= victim->pcdata->renacido )
     {
       ganancia_pk( ch );
       perdida_pk ( victim );
     }
	/* xSET_BIT(victim->act, PLR_PK); */
	return;
      }
    }

    /*
     * Charm-o-rama.
     */

    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

	/* stop_follower( ch ); */
	if ( ch->master )
	  check_killer( ch->master, victim );
	return;
    }

    /*
     * Dominacion
     */
     if ( xIS_SET(ch->act, PLR_DOMINADO))
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_killer: %s flag erroneo PLR_DOMINADO",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
            ch->pcdata->cnt_dominacion = -1;
            disciaffect_update( ch );
	    return;
	}

	/* stop_follower( ch ); */
	if ( ch->master )
	  check_killer( ch->master, victim );
	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch) )
    {
      if ( !IS_NPC(victim) )
      {
	int level_ratio;
        if ( victim->pcdata->clan )
          victim->pcdata->clan->mdeaths++;
        victim->pcdata->mdeaths++;
	victim->in_room->area->mdeaths++;
	level_ratio = URANGE( 1, ch->level / victim->level, 50 );
	if ( victim->pcdata->deity )
	{
	  if ( ch->race == victim->pcdata->deity->npcrace )
	    adjust_favor( victim, 12, level_ratio );
	  else
	    if ( ch->race == victim->pcdata->deity->npcfoe )
		adjust_favor( victim, 15, level_ratio );
	    else
	        adjust_favor( victim, 11, level_ratio );
	}
      }
      return;
    }


    if ( !IS_NPC(ch) )
    {
      if ( ch->pcdata->clan )
        ch->pcdata->clan->illegal_pk++;
      ch->pcdata->illegal_pk++;
      ch->in_room->area->illegal_pk++;
    }
    if ( !IS_NPC(victim) )
    {
      if ( victim->pcdata->clan )
      {
            if ( ch->level < 10 )
              victim->pcdata->clan->pdeaths[0]++;
            else if ( ch->level < 15 )
              victim->pcdata->clan->pdeaths[1]++;
            else if ( ch->level < 20 )
              victim->pcdata->clan->pdeaths[2]++;
            else if ( ch->level < 30 )
              victim->pcdata->clan->pdeaths[3]++;
            else if ( ch->level < 40 )
              victim->pcdata->clan->pdeaths[4]++;
            else if ( ch->level < 50 )
              victim->pcdata->clan->pdeaths[5]++;
            else
              victim->pcdata->clan->pdeaths[6]++;
      }
      victim->pcdata->pdeaths++;
      victim->in_room->area->pdeaths++;
    }

    if ( xIS_SET(ch->act, PLR_KILLER) )
      return;

    set_char_color( AT_WHITE, ch );
    send_to_char( "A strange feeling grows deep inside you, and a tingle goes up your spine...\n\r", ch );
    set_char_color( AT_IMMORT, ch );
    send_to_char( "A deep voice booms inside your head, 'Thou shall now be known as a deadly murderer!!!'\n\r", ch );
    set_char_color( AT_WHITE, ch );
    send_to_char( "You feel as if your soul has been revealed for all to see.\n\r", ch );
    xSET_BIT(ch->act, PLR_KILLER);
    if ( xIS_SET( ch->act, PLR_ATTACKER) )
      xREMOVE_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
}

/*
 * See if an attack justifies a ATTACKER flag.
 */
void check_attacker( CHAR_DATA *ch, CHAR_DATA *victim )
{

/*
 * Made some changes to this function Apr 6/96 to reduce the prolifiration
 * of attacker flags in the realms. -Narn
 */
    /*
     * NPC's are fair game.
     * So are killers and thieves.
     */
    if ( IS_NPC(victim)
    ||  xIS_SET(victim->act, PLR_KILLER)
    ||  xIS_SET(victim->act, PLR_THIEF) )
	return;

    /* deadly char check */
    if ( !IS_NPC(ch) && !IS_NPC(victim)
         && CAN_PKILL( ch ) && CAN_PKILL( victim ) )
	return;

/* Pkiller versus pkiller will no longer ever make an attacker flag
    { if ( !(ch->pcdata->clan && victim->pcdata->clan
      && ch->pcdata->clan == victim->pcdata->clan ) )  return; }
*/

    /*
     * Charm-o-rama.
     */
    if ( IS_AFFECTED(ch, AFF_CHARM) )
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_attacker: %s bad AFF_CHARM",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
	    affect_strip( ch, gsn_charm_person );
	    xREMOVE_BIT( ch->affected_by, AFF_CHARM );
	    return;
	}

        /* Won't have charmed mobs fighting give the master an attacker
           flag.  The killer flag stays in, and I'll put something in
           do_murder. -Narn */
	/* xSET_BIT(ch->master->act, PLR_ATTACKER);*/
	/* stop_follower( ch ); */
	return;
    }

    /*
     * Dominacion
     */
     if ( xIS_SET(ch->act, PLR_DOMINADO))
    {
	if ( !ch->master )
	{
	    char buf[MAX_STRING_LENGTH];

	    sprintf( buf, "Check_attacker: %s flag erroneo PLR_DOMINADO",
		IS_NPC(ch) ? ch->short_descr : ch->name );
	    bug( buf, 0 );
            ch->pcdata->cnt_dominacion = -1;
            disciaffect_update( ch );
	    return;
	}


	return;
    }

    /*
     * NPC's are cool of course (as long as not charmed).
     * Hitting yourself is cool too (bleeding).
     * So is being immortal (Alander's idea).
     * And current killers stay as they are.
     */
    if ( IS_NPC(ch)
    ||   ch == victim
    ||   ch->level >= LEVEL_IMMORTAL +2
    ||   xIS_SET(ch->act, PLR_ATTACKER)
    ||   xIS_SET(ch->act, PLR_KILLER)
    ||   EN_ARENA(ch) )
        return;

    xSET_BIT(ch->act, PLR_ATTACKER);
    save_char_obj( ch );
    return;
}


/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim )
{
    if ( !victim )
    {
      bug( "update_pos: null victim", 0 );
      return;
    }

    if ( victim->hit > 0 )
    {
	if ( victim->position <= POS_STUNNED )
	  victim->position = POS_STANDING;
	if ( IS_AFFECTED( victim, AFF_PARALYSIS ) )
	  victim->position = POS_STUNNED;
	if( victim->position == POS_PREDECAP )
   	victim->position = POS_STANDING;
	return;
    }

    if ( IS_NPC(victim) || victim->hit <= -11 )
    {
	if ( victim->mount )
	{
	  act( AT_ACTION, "$n falls from $N.",
		victim, NULL, victim->mount, TO_ROOM );
	  xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	  victim->mount = NULL;
	}
	victim->position = POS_DEAD;
	return;
    }

	 if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
    else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
    else                          victim->position = POS_STUNNED;

    if ( victim->position > POS_STUNNED
    &&   IS_AFFECTED( victim, AFF_PARALYSIS ) )
      victim->position = POS_STUNNED;

   /*
    * Para la posicion de predecapitacion y prediablerizacion
    * SiGo email to sigo@vampiromud.com
    * Colikotron Code Team 199x 200x
    */
   if ( victim->level >= LEVEL_HERO
   && victim->level <= LEVEL_ACOLYTE )
   if( victim->hit <= -1 )
   victim->position = POS_PREDECAP;

    if ( victim->mount )
    {
	act( AT_ACTION, "$n falls unconscious from $N.",
		victim, NULL, victim->mount, TO_ROOM );
	xREMOVE_BIT( victim->mount->act, ACT_MOUNTED );
	victim->mount = NULL;
    }
    return;
}


/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
    FIGHT_DATA *fight;
    DISCI_AFFECT *daf;

    if ( ch->fighting )
    {
	char buf[MAX_STRING_LENGTH];

	sprintf( buf, "Set_fighting: %s -> %s (already fighting %s)",
		ch->name, victim->name, ch->fighting->who->name );
	bug( buf, 0 );
	return;
    }

    if ( IS_AFFECTED(ch, AFF_SLEEP) )
	affect_strip( ch, gsn_sleep );

    if ( xIS_SET(ch->afectado_por, DAF_KOMA) )
    {
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( daf == DAF_KOMA )
        disciplina_remove( ch, daf );
    }

    /* Limit attackers -Thoric */
    if ( victim->num_fighting > max_fight(victim) )
    {
	send_to_char( "Hay demasiada gente luchando para poderte unir.\n\r", ch );
	return;
    }

    CREATE( fight, FIGHT_DATA, 1 );
    fight->who	 = victim;
    fight->xp	 = (int) xp_compute( ch, victim ) * 0.85;
    fight->align = align_compute( ch, victim );
    if ( !IS_NPC(ch) && IS_NPC(victim) )
      fight->timeskilled = times_killed(ch, victim);
    ch->num_fighting = 1;
    ch->fighting = fight;
    /* ch->position = POS_FIGHTING; */
	if ( IS_NPC(ch) )
		ch->position = POS_FIGHTING;
	else
	switch(ch->style)
	{
		case(STYLE_EVASIVE):
			ch->position = POS_EVASIVE;
			break;
		case(STYLE_DEFENSIVE):
			ch->position = POS_DEFENSIVE;
			break;
		case(STYLE_AGGRESSIVE):
			ch->position = POS_AGGRESSIVE;
			break;
		case(STYLE_BERSERK):
			ch->position = POS_BERSERK;
			break;
		default: ch->position = POS_FIGHTING;
	}
    victim->num_fighting++;
    if ( victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS) )
    {
	send_to_char( "You are disturbed!\n\r", victim->switched );
	do_return( victim->switched, "" );
    }
    return;
}


CHAR_DATA *who_fighting( CHAR_DATA *ch )
{
    if ( !ch )
    {
	bug( "who_fighting: null ch", 0 );
	return NULL;
    }
    if ( !ch->fighting )
      return NULL;
    return ch->fighting->who;
}

void free_fight( CHAR_DATA *ch )
{
   if ( !ch )
   {
	bug( "Free_fight: null ch!", 0 );
	return;
   }
   if ( ch->fighting )
   {
     if ( !char_died(ch->fighting->who) )
       --ch->fighting->who->num_fighting;
     DISPOSE( ch->fighting );
   }
   ch->fighting = NULL;
   if ( ch->mount )
     ch->position = POS_MOUNTED;
   else
     ch->position = POS_STANDING;
   /* Berserk wears off after combat. -- Altrag */
   if ( IS_AFFECTED(ch, AFF_BERSERK) )
   {
     affect_strip(ch, gsn_berserk);
     set_char_color(AT_WEAROFF, ch);
     send_to_char(skill_table[gsn_berserk]->msg_off, ch);
     send_to_char("\n\r", ch);
   }
   return;
}


/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
    CHAR_DATA *fch;

    free_fight( ch );
    update_pos( ch );

    if ( !fBoth )   /* major short cut here by Thoric */
      return;

    for ( fch = first_char; fch; fch = fch->next )
    {
	if ( who_fighting( fch ) == ch )
	{
	    free_fight( fch );
	    update_pos( fch );
	}
    }
    if((IS_NPC(ch)
    && xIS_SET(ch->act, ACT_UALIZADO)))
    {
    xREMOVE_BIT(ch->act, ACT_UALIZADO);
    ch->level = ch->nivel_real;
    }

    return;
}

/* Vnums for the various bodyparts */
int part_vnums[] =
{	12,	/* Head */
	14,	/* arms */
	15,	/* legs */
	13,	/* heart */
	44,	/* brains */
	16,	/* guts */
	45,	/* hands */
	46,	/* feet */
	47,	/* fingers */
	48,	/* ear */
	49,	/* eye */
	50,	/* long_tongue */
	51,	/* eyestalks */
   52,  /* tentacles */
   53,  /* fins */
   54,  /* wings */
   55,  /* tail */
   56,  /* scales */
   59,  /* claws */
   87,  /* fangs */
   58,  /* horns */
   57,  /* tusks */
/*   85,   sharpscales
   84,   beak
   86,   haunches
   83,   hooves
   82,   paws
   81,   forelegs
   80,   feathers
   0,  r1
   0   r2 */
};

/* Messages for flinging off the various bodyparts */
char* part_messages[] =
{
	"La cabeza de $n se separa de su cuerpo.",
	"$n's arm is sliced from $s dead body.",
	"$n's leg is sliced from $s dead body.",
	"El corazon de $n sale de su pecho.",
	"El cerebro de $n saler por una brecha de su cabeza.",
	"$n's guts spill grotesquely from $s torso.",
	"$n's hand is sliced from $s dead body.",
	"$n's foot is sliced from $s dead body.",
	"A finger is sliced from $n's dead body.",
	"$n's ear is sliced from $s dead body.",
	"$n's eye is gouged from its socket.",
	"$n's tongue is torn from $s mouth.",
	"An eyestalk is sliced from $n's dead body.",
	"A tentacle is severed from $n's dead body.",
	"A fin is sliced from $n's dead body.",
	"A wing is severed from $n's dead body.",
	"$n's tail is sliced from $s dead body.",
	"A scale falls from the body of $n.",
	"A claw is torn from $n's dead body.",
	"$n's fangs are torn from $s mouth.",
	"A horn is wrenched from the body of $n.",
	"$n's tusk is torn from $s dead body.",
	"$n's tail is sliced from $s dead body.",
	"A ridged scale falls from the body of $n.",
	"$n's beak is sliced from $s dead body.",
	"$n's haunches are sliced from $s dead body.",
	"A hoof is sliced from $n's dead body.",
	"A paw is sliced from $n's dead body.",
	"$n's foreleg is sliced from $s dead body.",
	"Some feathers fall from $n's dead body.",
	"r1 message.",
	"r2 message."
};

/*
 * Improved Death_cry contributed by Diavolo.
 * Additional improvement by Thoric (and removal of turds... sheesh!)
 * Support for additional bodyparts by Fireblade
 */
void death_cry( CHAR_DATA *ch )
{
    ROOM_INDEX_DATA *was_in_room;
    char *msg;
    EXIT_DATA *pexit;
    int vnum, shift, index, i;

    if ( !ch )
    {
      bug( "DEATH_CRY: null ch!", 0 );
      return;
    }

    vnum = 0;
    msg = NULL;

    switch ( number_range(0, 5) )
    {
    default: msg  = "Escuchas el grito de muerte de $n.";          break;
    case  0:
      msg = "Escuchas el grito de muerte de $n."; break;
    case  1:
      msg  = "$n cae al suelo... MUERTO.";			        break;
    case  2:
      msg = "$n cae al suelo... MUERTO.";                                                  break;
    case  3: msg  = "$n ensucia tus ropas con su sangre.";		break;
    case  4: msg  = "$n ensucia tus ropas con su sangre.";                                  break;
    case  5:
    	shift = number_range(0, 31);
    	index = 1 << shift;

       	for(i = 0;i < 32 && ch->xflags;i++)
    	{
    		if(HAS_BODYPART(ch, index))
    		{
    			msg = part_messages[shift];
    			vnum = part_vnums[shift];
    			break;
    		}
    		else
    		{
    			shift = number_range(0, 31);
    			index = 1 << shift;
    		}
    	}

    	if(!msg)
    		msg = "Oyes el ultimo suspiro de $n.";
    	break;
    }

    act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );

    if ( vnum )
    {
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	char *name;

/*
	if(!get_obj_index(vnum))
	{
		bug("death_cry: invalid vnum", 0);
		return;
	}
*/

	name		= IS_NPC(ch) ? ch->short_descr : ch->name;
	obj		= create_object( get_obj_index( vnum ), 0 );
	obj->timer	= number_range( 4, 7 );
	if ( IS_AFFECTED( ch, AFF_POISON ) )
	  obj->value[3] = 10;

	sprintf( buf, obj->short_descr, name );
	STRFREE( obj->short_descr );
	obj->short_descr = STRALLOC( buf );

	sprintf( buf, obj->description, name );
	STRFREE( obj->description );
	obj->description = STRALLOC( buf );

	obj = obj_to_room( obj, ch->in_room );
    }

    if ( IS_NPC(ch) )
	msg = "Oyes el ultimo suspiro de alguien.";
    else
	msg = "Oyes el ultimo suspiro de alguien.";

    was_in_room = ch->in_room;
    for ( pexit = was_in_room->first_exit; pexit; pexit = pexit->next )
    {
	if ( pexit->to_room
	&&   pexit->to_room != was_in_room )
	{
	    ch->in_room = pexit->to_room;
	    act( AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM );
	}
    }
    ch->in_room = was_in_room;

    return;
}



void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim )
{
    if ( !victim )
    {
      bug( "raw_kill: null victim!", 0 );
      return;
    }
/* backup in case hp goes below 1 */
    if (NOT_AUTHED(victim))
    {
      bug( "raw_kill: killing unauthed", 0 );
      return;
    }

    stop_fighting( victim, TRUE );
        
    /* Take care of morphed characters */
    if(victim->morph)
    {
      do_unmorph_char( victim );
      raw_kill(ch, victim);
      return;
    }

    mprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;
 /* death_cry( victim ); */

    rprog_death_trigger( ch, victim );
    if ( char_died(victim) )
      return;

    make_corpse( victim, ch );
    if ( victim->in_room->sector_type == SECT_OCEANFLOOR
    ||   victim->in_room->sector_type == SECT_UNDERWATER
    ||   victim->in_room->sector_type == SECT_WATER_SWIM
    ||   victim->in_room->sector_type == SECT_WATER_NOSWIM )
      act( AT_BLOOD, "$n's blood slowly clouds the surrounding water.", victim, NULL, NULL, TO_ROOM );
    else if ( victim->in_room->sector_type == SECT_AIR )
      act( AT_BLOOD, "$n's blood sprays wildly through the air.", victim, NULL, NULL, TO_ROOM );
    else
      make_blood( victim );

    if ( IS_NPC(victim) )
    {
	victim->pIndexData->killed++;
	extract_char( victim, TRUE );
	victim = NULL;
	return;
    }

    set_char_color( AT_DIEMSG, victim );
    if ( victim->pcdata->mdeaths + victim->pcdata->pdeaths < 3 )
      do_help( victim, "_DIEMSG_" );
    else
      do_help( victim, "_DIEMSG_" );

    extract_char( victim, FALSE );
    if ( !victim )
    {
      bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
      return;
    }
    while ( victim->first_affect )
	affect_remove( victim, victim->first_affect );
    victim->affected_by	= race_table[victim->race]->affected;
    /* Para que se vayan las disciplinas al morir */
    while ( victim->primer_affect )
        disciplina_remove( victim, victim->primer_affect );
/* Kayser para que se vayan disciplinas al morir */
if (IS_VAMPIRE( victim ))
{
    if (victim->pcdata->cnt_pasion > -1)
	    victim->pcdata->cnt_pasion = -1;
    if (victim->pcdata->cnt_dominacion > -1)
            victim->pcdata->cnt_dominacion = -1;
    if(victim->agravadas > 0)
            victim->agravadas = 0;
    disciaffect_update( victim );
}
    victim->cnt_pktotal = 0;
    victim->resistant   = 0;
    victim->susceptible = 0;
    victim->immune      = 0;
    victim->carry_weight= 0;
    victim->armor	= 100;
    victim->armor	+= race_table[victim->race]->ac_plus;
    victim->attacks	= race_table[victim->race]->attacks;
    victim->defenses	= race_table[victim->race]->defenses;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck   	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->mental_state = -10;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
/*  victim->alignment		= race_table[victim->race]->alignment;
-- switched lines just for now to prevent mortals from building up
days of bellyaching about their angelic or satanic humans becoming
neutral when they die given the difficulting of changing align */

    victim->saving_poison_death = race_table[victim->race]->saving_poison_death;
    victim->saving_wand 	= race_table[victim->race]->saving_wand;
    victim->saving_para_petri 	= race_table[victim->race]->saving_para_petri;
    victim->saving_breath 	= race_table[victim->race]->saving_breath;
    victim->saving_spell_staff 	= race_table[victim->race]->saving_spell_staff;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    /* Shut down some of those naked spammer killers - Blodkai */
    if ( victim->level < LEVEL_AVATAR )
	victim->mana	= UMAX( 1, victim->mana );
    else
	victim->mana	= 1;
    victim->move	= UMAX( 1, victim->move );

    /*
     * Pardon crimes...						-Thoric
     */
    if ( xIS_SET( victim->act, PLR_KILLER) )
    {
      xREMOVE_BIT( victim->act, PLR_KILLER);
      send_to_char("Los dioses te perdonan tus crimenes.\n\r",victim);
    }
    if ( xIS_SET( victim->act, PLR_THIEF) )
    {
      xREMOVE_BIT( victim->act, PLR_THIEF);
      send_to_char("Los dioses te perdonan tus crimenes.\n\r",victim);
    }
    victim->pcdata->condition[COND_FULL]   = 12;
    victim->pcdata->condition[COND_THIRST] = 12;
    if ( IS_VAMPIRE( victim ) )
      victim->pcdata->condition[COND_BLOODTHIRST] = (victim->level / 2);

    if ( IS_SET( sysdata.save_flags, SV_DEATH ) )
	save_char_obj( victim );

    /* Pa ver si los ceporros estos estan en guerra y les sume las muertes y todo eso SiGo, SaNgUi & MaCHaKa */
    if ( IS_CLANNED(ch) && IS_CLANNED(victim))
    {
          /* Primero vemos si tan en guerra o no */

          /*if ( ch->pcdata->clan->enemigos[victim->pcdata->clan->numero] == victim->pcdata->clan->name )
          {   */
             /* Despues restamos las muertes de guerra y sumamos las victorias */
           /*  ch->pcdata->clan->victorias_guerra[victim->pcdata->clan->numero] += 1;
             ch->pcdata->clan->victorias += 1;
             victim->pcdata->clan->muerte_guerra[ch->pcdata->clan->numero] += 1;
             victim->pcdata->clan->derrotas += 1;
          } */
          victim->pcdata->cnt_critico = 0;
    }
    if( xIS_SET(victim->act, PLR_ALTERADO))
    {
    victim->pcdata->alterado = -1;
    xREMOVE_BIT(victim->act, PLR_ALTERADO);
    send_to_char( "&RDejas de estar alterado.\n\r", victim );
    }

    if( xIS_SET(victim->act, PLR_PKTOTAL) )
    {
        victim->cnt_pktotal = -1;
        xREMOVE_BIT(victim->act, PLR_PKTOTAL);
        send_to_char( "&RDejas de ser PkTotal.\n\r", victim );
    }

    obj_update( );

    return;
}



void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *gch;
    CHAR_DATA *lch;
    AREA_DATA *area = get_area( ch->in_room->area->name );
    int xp;
    int members;
    int points;

     /*
      * Por si matas al mob objetivo de tu quest
      */

    if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_QUESTOR) && IS_NPC(victim))
        {
            if (ch->pcdata->questmob == victim->pIndexData->vnum)
            {
		send_to_char("&wYa has completado tu QUEST!\n\r",ch);
                send_to_char("Vuelve rapido a tu QuestMaster antes de que se agote tu tiempo!\n\r",ch);
                ch->pcdata->questmob = -1;
            }
        }


    if ( IS_NPC(ch) || victim == ch )
	return;

    members = 0;
    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	if ( is_same_group( gch, ch ) )
	    members++;
    }

    if ( members == 0 )
    {
	bug( "Group_gain: members.", members );
	members = 1;
    }

    lch = ch->leader ? ch->leader : ch;

    for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
    {
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if ( !is_same_group( gch, ch ) )
	    continue;

	if ( gch->level - lch->level >  5 )
	{
	    send_to_char( "Tienes demasiado nivel para este grupo.\n\r", gch );
	    continue;
	}

  /* if ( gch->level - lch->level < -5 )
	{
	    send_to_char( "No tienes suficiente nivel para este grupo.\n\r", gch );
	    continue;
   }*/

	xp = (int) (xp_compute( gch, victim ) ) / members;
	if ( xIS_SET(ch->act, PLR_RENACIDO ) )
	  xp /= ch->pcdata->renacido +1;
        if ( !gch->fighting )
	  xp /= 2;
	gch->alignment = align_compute( gch, victim );

       if ( ch->pcdata->clan != NULL )
       {

       		if ( !ES_PROPIETARIO(ch )  &&  ES_CONQUISTABLE(ch->in_room->area) )
                 {
		 	xp /= 2;	
                  	sprintf( buf, "Ganas &W%d&g puntos de experiencia.\n\r", xp /2 );
                        send_to_char( buf, gch );
                        sprintf( buf, "Ganas &W%d&g puntos de conquista para tu familia.\n\r", xp /2 );
                        send_to_char( buf, gch );
                  }

        	if ( ES_PROPIETARIO(ch) )
                  {
		  	sprintf( buf, "&gGanas &W%d&g puntos de experiencia.\n\r", xp );
			send_to_char( buf, gch );
                 	sprintf( buf, "&gGanas &W%d&g puntos de conquista para tu familia.\n\r", xp );
			send_to_char( buf, gch );
                  }
        	if ( !ES_CONQUISTABLE( ch->in_room->area ) )
                  {
                  	sprintf( buf, "&gGanas &W%d&g puntos de experiencia.\n\r", xp );
			send_to_char( buf, gch );
                 	sprintf( buf, "&gEste area no es conquistable no ganaras puntos de conquista.\n\r" );
			send_to_char( buf, gch );
                  }
       }
       else
       {
                sprintf( buf, "&gGanas &W%d&g puntos de experiencia.\n\r", xp );
                send_to_char( buf, gch );
       }

       if( IS_NPC(victim))
       {
       		MOB_INDEX_DATA *vnum = get_mob_index( victim->pIndexData->vnum );

                if( vnum->vnum < area->low_m_vnum || vnum->vnum > area->hi_m_vnum )
                {
                        ch_printf( ch, "&g&gHas matado a &w%s &gen el area &w%s&g no siendo de ese area el mob\n\r"
                                "Ganancia de &w%d&g px cancelada!\n\r",
                        victim->short_descr, area->name, xp);
                }
 	        else
       		{
       			if ( IS_CLANNED(gch))
       				gain_conquista( gch, xp );
       			gain_exp( gch, xp );
       		}
	}
        if ( xp > 1 )
        {
                if (!IS_NPC(gch) && gch->pcdata->disci_puntos != 999 && gch->pcdata->disci_adquirir != -1 )
                {
                	if( IS_VAMPIRE(ch))
                	{
                		send_to_char( "&gComprendes mejor la disciplina que estudias.\n\r", gch );
                		points = number_range( 1, 30 );
                		sprintf( buf, "&gConsigues &W%d &gpuntos de disciplina.\n\r", points );
                		send_to_char( buf, gch );
                 		ganar_disci_puntos( gch, points );
                 		continue;
                	}
                	else
                	{
                		send_to_char( "&gComprendes mejor la esfera que estudias.\n\r", gch );
                		points = number_range( 1, 30 );
                		sprintf( buf, "&gConsigues &W%d &gpuntos de esfera.\n\r", points );
                		send_to_char( buf, gch );
                 		ganar_disci_puntos( gch, points );
                 		continue;
                	}
                 }
         }
   	for ( obj = ch->first_carrying; obj; obj = obj_next )
	{
	    obj_next = obj->next_content;
	    if ( obj->wear_loc == WEAR_NONE )
		continue;

	    if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
	    ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) )
	    {
		act( AT_MAGIC, "$p te da un calambrazo y lo sueltas.", ch, obj, NULL, TO_CHAR );
		act( AT_MAGIC, "$p le da un calambrazo a $n y lo suelta.", ch, obj, NULL, TO_ROOM );

		obj_from_char( obj );
		obj = obj_to_room( obj, ch->in_room );
		oprog_zap_trigger(ch, obj);  /* mudprogs */
		if ( char_died(ch) );
		  return;
	    }
	}
    }

    return;
}

int align_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int align, newalign, divalign;

    align = gch->alignment - victim->alignment;

    /* slowed movement in good & evil ranges by a factor of 5, h */
    /* Added divalign to keep neutral chars shifting faster -- Blodkai */
    /* This is obviously gonna take a lot more thought */

    if ( gch->alignment > -350 && gch->alignment < 350 )
	divalign = 4;
    else
	divalign = 20;

    if ( align >  500 )
	newalign  = UMIN( gch->alignment + (align-500)/divalign,  1000 );
    else
    if ( align < -500 )
	newalign  = UMAX( gch->alignment + (align+500)/divalign, -1000 );
    else
	newalign  = gch->alignment - (int) (gch->alignment/divalign);

    return newalign;
}


/*
 * Calculate how much XP gch should gain for killing victim
 * Lots of redesigning for new exp system by Thoric
 */

int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim )
{
    int align;
    int xp;
    int base_exp;
    int xp_ratio;

    xp_ratio = victim->level - gch->level;

    /* mirar la experiencia base */

    switch (xp_ratio) 	{
    	default:
    	  base_exp = 0;
    	  break;
    	case -9:
    	  base_exp = 1;
    	  break;
    	case -8:
    	  base_exp = 2;
    	  break;
      	case -7:
      	  base_exp = 5;
      	  break;
      	case -6:
      	  base_exp = 9;
      	  break;
      	case -5:
      	  base_exp = 11;
      	  break;
      	case -4:
      	   base_exp = 22;
      	   break;
      	case -3:
      	   base_exp = 33;
      	   break;
      	case -2:
      	   base_exp = 50;
      	   break;
      	case -1:
      	   base_exp = 66;
   	   break;
   	case 0:
   	   base_exp = 83;
   	   break;
   	case 1:
   	   base_exp = 99;
   	   break;
   	case 2:
   	   base_exp = 121;
   	   break;
   	case 3:
   	   base_exp = 143;
   	   break;
   	case 4:
   	   base_exp = 165;
   	   break;
   	}
   if (xp_ratio > 4)
   	base_exp = 160 + 20 * (xp_ratio - 4);


    /* alineamiento */

     align = gch->alignment - victim->alignment;

    xp = (base_exp * 3 /4);

    /* para los jugadores que? */

    if ( !IS_NPC( victim ) )
      xp /= 4;
    else
    /* reduce exp for killing the same mob repeatedly		-Thoric */
    if ( !IS_NPC( gch ) )
    {
	int times = times_killed( gch, victim );

	if ( times >= 20 )
	   xp = 0;
	else
	if ( times )
	{
	   xp = (xp * (20-times)) / 20;
	   if ( times > 15 )
	     xp /= 3;
	   else
	   if ( times > 10 )
	     xp >>= 1;
	}
    }

    /*
     * Ahora se randomizaran los px
     * y se calcula el total, KAYSER
     */

     xp = number_range (xp * 3 / 4, xp * 5 / 4);


     /* No se ganan mas de 1000 px */

     if ( xp > 1000 )
     	xp = 954 + number_range(-57,43);

    return xp;
}


/*
 * Revamped by Thoric to be more realistic
 * Added code to produce different messages based on weapon type - FB
 * Added better bug message so you can track down the bad dt's -Shaddai
 */
void new_dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, OBJ_DATA *obj )
{
    char buf1[256], buf2[256], buf3[256];
    char bugbuf[MAX_STRING_LENGTH];
    const char *vs;
    const char *vp;
    const char *attack;
    char punct;
    int dampc;
    struct skill_type *skill = NULL;
    bool gcflag = FALSE;
    bool gvflag = FALSE;
    int d_index, w_index;
    ROOM_INDEX_DATA *was_in_room;

    if ( ! dam )
	dampc = 0;
    else
	dampc = ( (dam * 100) / ( victim->max_hit/ 10)) +
              ( 50 - ((victim->hit * 5) / (victim->max_hit/10)) );

    if ( ch->in_room != victim->in_room )
    {
	was_in_room = ch->in_room;
	char_from_room(ch);
	char_to_room(ch, victim->in_room);
    }
    else
	was_in_room = NULL;

    /* Get the weapon index */
    if ( dt > 0 && dt < top_sn )
    {
    	w_index = 0;
    }
    else
    if ( dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
    {
   	w_index = dt - TYPE_HIT;
    }
    else
    {
	sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.",
		dt, ch->name, ch->in_room->vnum );
	bug( bugbuf, 0);
   	dt = TYPE_HIT;
   	w_index = 0;
    }

    /* get the damage index */
    if(dam == 0)
    	d_index = 0;
    else if(dampc < 0)
    	d_index = 1;
    else if(dampc <= 100)
   	d_index = 1 + dampc/10;
    else if(dampc <= 200)
   	d_index = 11 + (dampc - 100)/20;
    else if(dampc <= 900)
   	d_index = 16 + (dampc - 200)/100;
    else
   	d_index = 23;

    /* Lookup the damage message */
    vs = s_message_table[w_index][d_index];
    vp = p_message_table[w_index][d_index];

    punct   = (dampc <= 30) ? '.' : '!';

    if ( dam == 0 && (!IS_NPC(ch) &&
       (IS_SET(ch->pcdata->flags, PCFLAG_GAG)))) gcflag = TRUE;

    if ( dam == 0 && (!IS_NPC(victim) &&
       (IS_SET(victim->pcdata->flags, PCFLAG_GAG)))) gvflag = TRUE;

    if ( dt >=0 && dt < top_sn )
	skill = skill_table[dt];
    if ( dt == TYPE_HIT )
    {
   sprintf( buf1, "El golpe de $n %s a $N%c",  vp, punct );
	sprintf( buf2, "Tu golpe %s a $N%c", vs, punct );
	sprintf( buf3, "El golpe de $n te %s %c", vp, punct );

   if ((( xIS_SET(ch->act, PLR_AUTODANYO)
       || xIS_SET(victim->act, PLR_AUTODANYO))
       &&   victim != ch ))

   {
   sprintf( buf1, "El golpe de $n %s a $N%c",  vp, punct );
   sprintf( buf2, "\x1b[0;37m[\x1b[1;33m %d \x1b[0m\x1b[0;37m] Tu golpe %s a $N%c",dam, vs, punct );
   sprintf( buf3, "\x1b[0;37m[\x1b[1;31m %d \x1b[0m\x1b[0;37m] El golpe de $n te %s %c",dam, vp, punct );
   }
   }
    else if ( dt == 19 )
    {
        sprintf( buf1, "La disciplina de $n %s a $N%c",  vp, punct );
	sprintf( buf2, "Tu disciplina %s a $N%c", vs, punct );
	sprintf( buf3, "La disciplina de $n te %s %c", vp, punct );

   if ((( xIS_SET(ch->act, PLR_AUTODANYO)
       || xIS_SET(victim->act, PLR_AUTODANYO))
       &&   victim != ch))
   {
   sprintf( buf1, "La disciplina de $n %s a $N%c",  vp, punct );
   sprintf( buf2, "\x1b[0;37m[\x1b[1;33m %d \x1b[0m\x1b[0;37m] Tu disciplina %s a $N%c",dam, vs, punct );
   sprintf( buf3, "\x1b[0;37m[\x1b[1;31m %d \x1b[0m\x1b[0;37m] La disciplina de $n te %s %c",dam, vp, punct );
   }
    }
   else if ( dt == 20 )
    {
        sprintf( buf1, "&REl Golpe Critico de $n %s a $N%c",  vp, punct );
	sprintf( buf2, "&RTu Golpe Critico %s a $N%c", vs, punct );
	sprintf( buf3, "&REl Golpe Critico de $n te %s %c", vp, punct );

   if ((( xIS_SET(ch->act, PLR_AUTODANYO)
       || xIS_SET(victim->act, PLR_AUTODANYO))
       &&   victim != ch))
   {
   sprintf( buf1, "El Golpe Critico de $n %s a $N%c",  vp, punct );
   sprintf( buf2, "\x1b[0;37m[\x1b[1;33m %d \x1b[0m\x1b[0;37m] Tu Golpe Critico %s a $N%c",dam, vs, punct );
   sprintf( buf3, "\x1b[0;37m[\x1b[1;31m %d \x1b[0m\x1b[0;37m] El Golpe Critico de $n te %s %c",dam, vp, punct );
   }
    }
   else if ( dt == 21 )
    {
        sprintf( buf1, "&REl apunyalamiento de $n %s a $N%c",  vp, punct );
	sprintf( buf2, "&RTu apunyalamiento %s a $N%c", vs, punct );
	sprintf( buf3, "&REl apunyalamiento de $n te %s %c", vp, punct );

   if ((( xIS_SET(ch->act, PLR_AUTODANYO)
       || xIS_SET(victim->act, PLR_AUTODANYO))
       &&   victim != ch))
   {
   sprintf( buf1, "El Apunyalamiento de $n %s a $N%c",  vp, punct );
   sprintf( buf2, "\x1b[0;37m[\x1b[1;33m %d \x1b[0m\x1b[0;37m] Tu Apunyalamiento %s a $N%c",dam, vs, punct );
   sprintf( buf3, "\x1b[0;37m[\x1b[1;31m %d \x1b[0m\x1b[0;37m] El Apunyalamiento de $n te %s %c",dam, vp, punct );
   }
 }
     else
    if ( dt > TYPE_HIT && is_wielding_poisoned( ch ) )
    {
	if ( dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
	    attack	= attack_table[dt - TYPE_HIT];
	else
	{
         sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.",
                dt, ch->name, ch->in_room->vnum );
         bug( bugbuf, 0);
	    dt  = TYPE_HIT;
	    attack  = attack_table[0];
        }

	sprintf( buf1, "El %s envenenado de $n %s a $N%c", attack, vp, punct );
	sprintf( buf2, "Tu %s envenenado %s a $N%c", attack, vp, punct );
	sprintf( buf3, "El %s envenenado de $n te %s %c", attack, vp, punct );

   if ((( xIS_SET(ch->act, PLR_AUTODANYO)
       || xIS_SET(victim->act, PLR_AUTODANYO))
       &&   victim != ch))
   {
   sprintf( buf1, "El %s envenenado de $n %s a $N%c", attack, vp, punct );
   sprintf( buf2, "\x1b[0;37m[\x1b[1;33m %d \x1b[0m\x1b[0;37m] Tu %s envenenado %s a $N%c", dam, attack, vp, punct );
   sprintf( buf3, "\x1b[0;37m[\x1b[1;31m %d \x1b[0m\x1b[0;37m] El %s envenenado de $n te %s %c", dam, attack, vp, punct );
   }
    }
    else
    {
	if ( skill )
	{
	    attack	= skill->noun_damage;
	    if ( dam == 0 )
	    {
		bool found = FALSE;

		if ( skill->miss_char && skill->miss_char[0] != '\0' )
		{
		   act( AT_HIT, skill->miss_char, ch, NULL, victim, TO_CHAR );
		   found = TRUE;
		}
		if ( skill->miss_vict && skill->miss_vict[0] != '\0' )
		{
		   act( AT_HITME, skill->miss_vict, ch, NULL, victim, TO_VICT );
		   found = TRUE;
		}
		if ( skill->miss_room && skill->miss_room[0] != '\0' )
		{
		   if (strcmp( skill->miss_room,"supress" ) )
			act( AT_GREY, skill->miss_room, ch, NULL, victim, TO_NOTVICT );
		   found = TRUE;
		}
		if ( found )	/* miss message already sent */
		{
		   if ( was_in_room )
		   {
			char_from_room(ch);
			char_to_room(ch, was_in_room);
		   }
		   return;
		}
	    }
	    else
	    {
		if ( skill->hit_char && skill->hit_char[0] != '\0' )
		  act( AT_GREY, skill->hit_char, ch, NULL, victim, TO_CHAR );
		if ( skill->hit_vict && skill->hit_vict[0] != '\0' )
		  act( AT_GREY, skill->hit_vict, ch, NULL, victim, TO_VICT );
		if ( skill->hit_room && skill->hit_room[0] != '\0' )
		  act( AT_GREY, skill->hit_room, ch, NULL, victim, TO_NOTVICT );
	    }
	}
	else if ( dt >= TYPE_HIT
	&& dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
	{
	    if ( obj )
		attack = obj->short_descr;
	    else
		attack = attack_table[dt - TYPE_HIT];
	}
	else
	{
            sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.",
                dt, ch->name, ch->in_room->vnum );
            bug( bugbuf, 0);
	    dt  = TYPE_HIT;
	    attack  = attack_table[0];
	}
   if ((( xIS_SET(ch->act, PLR_AUTODANYO)
       || xIS_SET(victim->act, PLR_AUTODANYO))
       &&   victim != ch))
   {
   sprintf( buf1, "El %s de $n %s a $N%c",  attack, vp, punct );
   sprintf( buf2, "\x1b[0;37m[\x1b[1;33m %d \x1b[0m\x1b[0;37m] Tu %s %s a $N%c", dam,  attack, vp, punct );
   sprintf( buf3, "\x1b[0;37m[\x1b[1;31m %d \x1b[0m\x1b[0;37m] El %s de $n te %s %c", dam, attack, vp, punct );
   }
   else
   {
   sprintf( buf1, "El %s de $n %s a $N%c",  attack, vp, punct );
	sprintf( buf2, "Tu %s %s a $N%c",  attack, vp, punct );
   sprintf( buf3, "El %s de $n te %s %c", attack, vp, punct );
   }
 }

    act( AT_GREY, buf1, ch, NULL, victim, TO_NOTVICT );
    if (!gcflag)  act( AT_GREY, buf2, ch, NULL, victim, TO_CHAR );
    if (!gvflag)  act( AT_GREY, buf3, ch, NULL, victim, TO_VICT );

   if ( was_in_room )
   {
	char_from_room(ch);
	char_to_room(ch, was_in_room);
   }
    return;
}

#ifndef dam_message
void dam_message( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
{
        new_dam_message(ch, victim, dam, dt);
}
#endif

void do_kill( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Matar a quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) && victim->morph)
    {
	send_to_char ( "Esta criatura es muy rara, dejala en paz mientras no sepas a que te enfrentas.", ch);
	return;
    }

    if ( !IS_NPC(victim) )
    {
	if ( !xIS_SET(victim->act, PLR_KILLER)
	&&   !xIS_SET(victim->act, PLR_THIEF) )
	{
	    send_to_char( "Debes ASESINAR a un jugador.\n\r", ch );
	    return;
	}
    }

   /*
    *
    else
    {
	if ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
	{
	    send_to_char( "Debes ASESINAR a una criatura encantada.\n\r", ch );
	    return;
	}
    }
    *
    */

    if ( victim == ch )
    {
	send_to_char( "Te golpeas a ti mismo.  Ouch!\n\r", ch );
	multi_hit( ch, ch, TYPE_UNDEFINED );
	return;
    }

    if ( is_safe( ch, victim, TRUE ) )
	return;

    if (( IS_AFFECTED(ch, AFF_CHARM) || xIS_SET(ch->act, PLR_DOMINADO) && ch->master == victim ))
    {
    act( AT_PLAIN, "$N es tu querido amo.", ch, NULL, victim, TO_CHAR );
	return;
    }

    if ( ch->position == POS_FIGHTING
       || ch->position ==  POS_EVASIVE
       || ch->position ==  POS_DEFENSIVE
       || ch->position ==  POS_AGGRESSIVE
       || ch->position ==  POS_BERSERK
       )
    {
	send_to_char( "Lo haces lo mejor que puedes!\n\r", ch );
	return;
    }
    if (!IS_IMMORTAL( ch ))
	    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    check_attacker( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}



void do_murde( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Si quieres ASESINAR, escribelo entero.\n\r", ch );
    return;
}



void do_murder( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
	send_to_char( "Asesinar a quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( victim == ch )
    {
	send_to_char( "Si te quieres suicidar pegale al ejecutor...\n\r", ch );
	return;
    }

    if ( is_safe( ch, victim, TRUE ) )
	return;

    if ( IS_AFFECTED(ch, AFF_CHARM) || xIS_SET(ch->act, PLR_DOMINADO))
    {
      if ( ch->master == victim )
      {
        act( AT_PLAIN, "$N es tu querido amo.", ch, NULL, victim, TO_CHAR );
	return;
      }
      else
      {
        if ( ch->master )
          xSET_BIT(ch->master->act, PLR_ATTACKER);
      }
    }

    if ( ch->position == POS_FIGHTING
       || ch->position ==  POS_EVASIVE
       || ch->position ==  POS_DEFENSIVE
       || ch->position ==  POS_AGGRESSIVE
       || ch->position ==  POS_BERSERK
       )
    {
	send_to_char( "Lo haces lo mejor que puedes!\n\r", ch );
	return;
    }

    if ( !IS_NPC( victim ) && xIS_SET(ch->act, PLR_NICE ) )
    {
      send_to_char( "Te sientes demasiado bien para hacer eso!\n\r", ch );
      return;
    }
/*
    if ( !IS_NPC( victim ) && xIS_SET(victim->act, PLR_PK ) )
*/

    if (!IS_NPC (victim))
       {
       sprintf( log_buf, "%s: asesina a %s.", ch->name, victim->name );
       log_string_plus( log_buf, LOG_NORMAL, ch->level );
       }

    if (!IS_IMMORTAL ( ch ))
	    WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
    sprintf( buf, "Socorro! Estoy siendo atacado por %s!",
                   IS_NPC( ch ) ? ch->short_descr : ch->name );
    if ( IS_PKILL(victim) )
	do_wartalk( victim, buf );
    else
	do_yell( victim, buf );
    check_illegal_pk( ch, victim );
    check_attacker( ch, victim );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return;
}

/*
 * Check to see if the player is in an "Arena".
 */
 bool in_arena( CHAR_DATA *ch )
{
    if ( IS_SET(ch->in_room->room_flags, ROOM_ARENA) )
	return TRUE;
    if ( IS_SET(ch->in_room->area->flags, AFLAG_FREEKILL) )
	return TRUE;
    if ( ch->in_room->vnum >= 29 && ch->in_room->vnum <= 43 )
	return TRUE;
    if ( !str_cmp(ch->in_room->area->filename, "arena.are") )
	return TRUE;

    return FALSE;
}

bool check_illegal_pk( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if ( !IS_NPC(victim) && !IS_NPC(ch) && !IS_IMMORTAL ( ch) )
  {
	if ( ( !IS_SET(victim->pcdata->flags, PCFLAG_DEADLY)
	|| ch->level - victim->level > 10
	|| !IS_SET(ch->pcdata->flags, PCFLAG_DEADLY) )
	&& !in_arena(ch)
	&& ch != victim
	&& !( IS_IMMORTAL(ch) && IS_IMMORTAL(victim) ) )
	{
	    if ( IS_NPC(ch) )
		sprintf(buf, " (%s)", ch->name);
	    if ( IS_NPC(victim) )
		sprintf(buf2, " (%s)", victim->name);

	    sprintf( log_buf, "&p%s a %s%s en &W***&rILLEGAL PKILL&W*** &pattempt at %d",
		(lastplayercmd),
		(IS_NPC(victim) ? victim->short_descr : victim->name),
		(IS_NPC(victim) ? buf2 : ""),
		victim->in_room->vnum );
	    last_pkroom = victim->in_room->vnum;
	    log_string(log_buf);
	    to_channel( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );
	    return TRUE;
	}
    }
    return FALSE;
}


void do_flee( CHAR_DATA *ch, char *argument )
{
    ROOM_INDEX_DATA *was_in;
    ROOM_INDEX_DATA *now_in;
    char buf[MAX_STRING_LENGTH];
    int attempt, los;
    sh_int door;
    EXIT_DATA *pexit;

    if ( !who_fighting(ch) )
    {
	if ( ch->position == POS_FIGHTING
	||   ch->position == POS_EVASIVE
        ||   ch->position == POS_DEFENSIVE
        ||   ch->position == POS_AGGRESSIVE
        ||   ch->position == POS_BERSERK )
	{
	  if ( ch->mount )
	    ch->position = POS_MOUNTED;
	  else
	    ch->position = POS_STANDING;
	}
	send_to_char( "No estas luchando con nadie.\n\r", ch );
	return;
    }
    if ( IS_AFFECTED( ch, AFF_BERSERK ) ) {
        send_to_char( "No puedes huir con el berseck.\n\r", ch);
        return;
    }
    if ( ch->move <= 0 ) {
	send_to_char( "Estas demasiado cansado!\n\r", ch );
	return;
    }
    /* No fleeing while more aggressive than standard or hurt. - Haus */
    if ( !IS_NPC( ch ) && ch->position < POS_FIGHTING ) {
	send_to_char( "No puedes huir en modo agresivo...\n\r", ch );
	return;
    }

    if ( IS_NPC( ch ) && xIS_SET( ch->act, ACT_AUTOMOB ) )
        return;

    if ( IS_NPC( ch ) && ch->position <= POS_SLEEPING )
	return;
    was_in = ch->in_room;
    for ( attempt = 0; attempt < 8; attempt++ )
    {
	door = number_door( );
	if (( pexit = get_exit( was_in, door ) ) == NULL
	||    !pexit->to_room
	||   IS_SET( pexit->exit_info, EX_NOFLEE )
	|| ( IS_SET( pexit->exit_info, EX_CLOSED )
	&& ( !IS_AFFECTED( ch, AFF_PASS_DOOR ) ||
             !xIS_SET(ch->afectado_por, DAF_INCORPOREO)))	
	|| ( IS_NPC( ch )
	&&   IS_SET( pexit->to_room->room_flags, ROOM_NO_MOB ) ) )
	    continue;
        affect_strip ( ch, gsn_sneak );
        xREMOVE_BIT  ( ch->affected_by, AFF_SNEAK );
	if ( ch->mount && ch->mount->fighting )
	    stop_fighting( ch->mount, TRUE );
	move_char( ch, pexit, 0 );
	if ( ( now_in = ch->in_room ) == was_in )
	    continue;
	ch->in_room = was_in;
	act( AT_FLEE, "$n huye del combate!", ch, NULL, NULL, TO_ROOM );
	ch->in_room = now_in;
	act( AT_FLEE, "$n mira a su alrededor como si lo persiguieran.", ch, NULL, NULL, TO_ROOM );
	if ( !IS_NPC( ch ) )
	{
	    CHAR_DATA *wf = who_fighting( ch );
	    act( AT_FLEE, "Huyes del combate!", ch, NULL, NULL, TO_CHAR );
	    los = 25;
	    if ( ch->level < 85 )
	    {
	      sprintf( buf, "Pierdes %d puntos de experiencia!", los );
 	      act( AT_FLEE, buf, ch, NULL, NULL, TO_CHAR );
	      gain_exp( ch, 0 - los );
	    }
	    if ( wf && ch->pcdata->deity )
	    {
	      int level_ratio = URANGE( 1, wf->level / ch->level, 50 );

	      if ( wf && wf->race == ch->pcdata->deity->npcrace )
		adjust_favor( ch, 1, level_ratio );
   	      else
		if ( wf && wf->race == ch->pcdata->deity->npcfoe )
		  adjust_favor( ch, 16, level_ratio );
		else
		  adjust_favor( ch, 0, level_ratio );
	    }
	}
        if( xIS_SET(ch->fighting->who->act, ACT_AUTOMOB))
        xREMOVE_BIT(ch->fighting->who->act, ACT_UALIZADO);

	stop_fighting( ch, TRUE );
	if (!IS_IMMORTAL(ch))
		WAIT_STATE(ch, 6);
	return;
    }
    los = 10;
    act( AT_FLEE, "Fallaste!", ch, NULL, NULL, TO_CHAR );
    if ( ch->level < 85 && number_bits( 3 ) == 1 )
    {
      sprintf( buf, "Pierdes %d puntos de experiencia!\n\r", los );
      act( AT_FLEE, buf, ch, NULL, NULL, TO_CHAR );
      gain_exp( ch, 0 - los );
    }
    return;
}


void do_sla( CHAR_DATA *ch, char *argument )
{
    send_to_char( "Si quieres usar el SLAY, escribelo entero.\n\r", ch );
    return;
}


void do_slay( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];


    argument = one_argument( argument, arg );
    one_argument( argument, arg2 );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Slay a quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( ch == victim )
    {
	send_to_char( "A ti mismo??\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ))
    {
        if(( !str_cmp( victim->name, "SiGo" ) || !str_cmp( victim->name, "SaNgUiNaRi" )))
        {
	send_to_char( "Fallaste.\n\r", ch );
        sprintf( buf, "%s ha intentado slayear a %s. Sera Mamon!!", ch->name, victim->name);
        mensa_todos( ch, "Joputas", buf);
	return;
        }
    }

    if ( !str_cmp( arg2, "fuego" ) )
    {
      act( AT_FIRE, "Tu fuego infernal carboniza a $N.", ch, NULL, victim, TO_CHAR    );
      act( AT_FIRE, "$n te carboniza con su fuego infernal!", ch, NULL, victim, TO_VICT    );
      act( AT_FIRE, "$n carboniza a $N con su fuego infernal!", ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "hielo" ) )
    {
      act( AT_LBLUE, "Congelas a $N y le haces explotar en mil pedazos.",  ch, NULL, victim, TO_CHAR    );
      act( AT_LBLUE, "$n te congela y seguidamente te hace explotar en mil pedazos.", ch, NULL, victim, TO_VICT    );
      act( AT_LBLUE, "$n congela a $N y lo hace explotar en mil pedazos.",  ch, NULL, victim, TO_NOTVICT );
    }

    else if ( !str_cmp( arg2, "perros" ) )
    {
      act( AT_BLOOD, "Ordenas a tus perros despedazar a $N.", ch, NULL, victim, TO_CHAR );
      act( AT_BLOOD, "Has muerto despedazado por los perros de $n.", ch, NULL, victim, TO_VICT );
      act( AT_BLOOD, "$n ordena a sus perros despedazar a $N.", ch, NULL, victim, TO_NOTVICT );
    }

    else
    {
      act( AT_IMMORT, "Asesinas a $N a sangre fria!",  ch, NULL, victim, TO_CHAR    );
      act( AT_IMMORT, "$n te asesina a sangre fria!", ch, NULL, victim, TO_VICT    );
      act( AT_IMMORT, "$n asesina a $N a sangre fria!",  ch, NULL, victim, TO_NOTVICT );
    }

    set_cur_char(victim);
    raw_kill( ch, victim );
    return;
}

/* -----------------11/01/01 0:04--------------------*
 * Modulo para la Ganancia de Generacion, por SiGo   *
 *   VampiroMud 2000 - 2001 vampiromud@hotmail.com   *
 * --------------------------------------------------*/

void ganancia_pk( CHAR_DATA *ch )
{

int proxima_generacion = 0;

 ch->pcdata->genepkills++;

     switch ( ch->generacion )
     {
      case 13 : proxima_generacion =  10; break;
      case 12 : proxima_generacion =  20; break;
      case 11 : proxima_generacion =  30; break;
      case 10 : proxima_generacion =  40; break;
      case  9 : proxima_generacion =  50; break;
      case  8 : proxima_generacion =  65; break;  
      case  7 : proxima_generacion =  80; break;
      case  6 : proxima_generacion =  95; break;
      case  5 : proxima_generacion = 105; break;
     }
      if ( ch->pcdata->genepkills >= proxima_generacion )
    {
      if ( ch->generacion <= GENERACION_MATUSALEN )
      {
      /*
       * Si el personaje tiene generacion matusalen (4) o menos no ganara generacion
       */
         send_to_char("Ya no puedes mejorar mas tu generacion.\n\r", ch );

         ch->pcdata->pkills++;
         return;
      }
      advance_generacion( ch );
      return;
    }

   return;
}

void perdida_pk( CHAR_DATA *victim )
{

int anterior_generacion = 0;

 victim->pcdata->genepkills--;

if (!IS_NPC (victim) )
    {

if ( victim->generacion <= GENERACION_ANTEDILUVIANO )
      {
    send_to_char( "No has bajado de generacion por tu posicion de Inmortal pero vigila mas hombre!!!.\n\r", victim );
     victim->pcdata->pdeaths++;

     return;
      }

if ( victim->generacion == MAX_GENERACION && victim->pcdata->genepkills <= 0 )
{
        send_to_char( "No tienes puntos de pk sera mejor ke konsigas algunos.\n\r", victim );
        return;
}

            switch ( victim->generacion )
            {
            case 13 : anterior_generacion =  -1; break;
            case 12 : anterior_generacion =   9; break;
            case 11 : anterior_generacion =  19; break;
            case 10 : anterior_generacion =  29; break;
            case  9 : anterior_generacion =  39; break;
            case  8 : anterior_generacion =  49; break;
            case  7 : anterior_generacion =  64; break;
            case  6 : anterior_generacion =  79; break;
            case  5 : anterior_generacion =  94; break;
            case  4 : anterior_generacion = 104; break;
     }

      if  (victim->pcdata->genepkills <= anterior_generacion)
	 {
	      degradacion_generacion_pk( victim );
	 }
     return;
   }
   return;
}

void do_critico( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *victim;
        char nivel[100];
        int chance;
        int danyo = 0;

        if(IS_NPC(ch))
        {
                send_to_char( "Los MOB's no pueden usar golpes Criticos.\n\r", ch );
                return;
        }

        if(!IS_CLANNED(ch))
        {
                send_to_char( "No perteneces a ninguna familia no puedes usar el Golpe Critico.\n\r", ch );
                return;
        }

        if(argument[0]=='\0')
        {
        send_to_char( "&wUso: critico <objetivo>\n\r", ch);

                    if(ch->pcdata->cnt_critico == 0)
                    sprintf(nivel, "                   ");
                    if(ch->pcdata->cnt_critico == 1)
                    sprintf(nivel, "***                ");
                    if((ch->pcdata->cnt_critico >= 10 && ch->pcdata->cnt_critico <= 20))
                    sprintf(nivel, "*****              ");
                    if((ch->pcdata->cnt_critico >= 20 && ch->pcdata->cnt_critico <= 50))
                    sprintf(nivel, "*******            ");
                    if((ch->pcdata->cnt_critico >= 50 && ch->pcdata->cnt_critico <= 100))
                    sprintf(nivel, "********           ");
                    if((ch->pcdata->cnt_critico >= 100 && ch->pcdata->cnt_critico <= 150))
                    sprintf(nivel, "*********          ");
                    if((ch->pcdata->cnt_critico >= 150 && ch->pcdata->cnt_critico <= 200))
                    sprintf(nivel, "**********         ");
                    if((ch->pcdata->cnt_critico >= 200 && ch->pcdata->cnt_critico <= 250))
                    sprintf(nivel, "***********        ");
                    if((ch->pcdata->cnt_critico >= 250 && ch->pcdata->cnt_critico <= 300))
                    sprintf(nivel, "************       ");
                    if((ch->pcdata->cnt_critico >= 300 && ch->pcdata->cnt_critico <= 350))
                    sprintf(nivel, "*************      ");
                    if((ch->pcdata->cnt_critico >= 350 && ch->pcdata->cnt_critico <= 400))
                    sprintf(nivel, "**************     ");
                    if((ch->pcdata->cnt_critico >= 400 && ch->pcdata->cnt_critico <= 450))
                    sprintf(nivel, "***************    ");
                    if((ch->pcdata->cnt_critico >= 450 && ch->pcdata->cnt_critico <= 600))
                    sprintf(nivel, "****************   ");
                    if((ch->pcdata->cnt_critico >= 600 && ch->pcdata->cnt_critico <= 800))
                    sprintf(nivel, "*****************  ");
                    if((ch->pcdata->cnt_critico >= 800 && ch->pcdata->cnt_critico <= 999))
                    sprintf(nivel, "****************** ");
                    if((ch->pcdata->cnt_critico >= 1000 ))
                    sprintf(nivel, "*******************");

                        ch_printf(ch, "\n\r&gNivel de Golpe Critico&w _-][&c%s&w][-_\n\r", nivel);
                    return;
        }

    if ( ( victim = who_fighting( ch ) ) == NULL )
    {
	send_to_char( "No estas luchando con nadie.\n\r", ch );
	return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if(victim == ch)
    {
    send_to_char( "A ti mismo? no creo que te gustara eso...\n\r", ch);
    return;
    }

    if(ch->pcdata->cnt_critico < 2000 )
        {
                send_to_char( "Aun no estas lo suficientemente cabreado.\n\r", ch );
                return;
        }

    chance = number_range( 1, 100 );
                if (( chance > 91 && chance < 100 ))
                {
                send_to_char( "Tu Golpe Critico se desvia herrando el objetivo.\n\r", ch );
                ch->pcdata->cnt_critico = 0;
                if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
                espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 16);
                else
                WAIT_STATE(ch, 16);
	        return;
                }

                act( AT_MAGIC, "$N pone los ojos en blanco y lanza un Golpe Critico a $n!", ch, victim, victim, TO_NOTVICT );
                act( AT_MAGIC, "$N fija su mirada en el vacio y te golpea con una fuerza inhumana!", ch, NULL, ch, TO_VICT );
                act( AT_MAGIC, "Descargas toda tu rabia contra tu enemigo!", ch, NULL, NULL, TO_CHAR );

                if(IS_NPC(victim))
                danyo = number_range((ch->damroll *20), (ch->damroll *20) *2);
                else
                danyo = number_range((ch->damroll *12), (ch->damroll *12) *2);

                damage(ch, victim, danyo, 20);
                if(!IS_IMMORTAL(ch))
                ch->pcdata->cnt_critico = 0;
 return;
 }
