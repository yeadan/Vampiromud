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
 * 		Commands for personal player settings/statictics	    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 *  Locals
 */
char *tiny_affect_loc_name(int location);

void do_gold(CHAR_DATA * ch, char *argument)
{
   set_char_color( AT_GOLD, ch );
   ch_printf( ch,  "Tienes %s monedas de oro.\n\r", num_punct(ch->gold) );
   return;
}


void do_worth(CHAR_DATA *ch, char *argument)
{
    char            buf[MAX_STRING_LENGTH];
    char            buf2[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
      return;

    set_pager_color(AT_SCORE, ch);
    pager_printf(ch, "\n\rDignidad de %s%s.\n\r", ch->name, ch->pcdata->title);
    send_to_pager("-----------------------------------------------------------------------------\n\r", ch);
    if (!ch->pcdata->deity)		 sprintf( buf, "N/A" );
    else if (ch->pcdata->favor > 2250)	 sprintf( buf, "amado" );
    else if (ch->pcdata->favor > 2000)	 sprintf( buf, "querido" );
    else if (ch->pcdata->favor > 1750) 	 sprintf( buf, "honrado" );
    else if (ch->pcdata->favor > 1500)	 sprintf( buf, "elogiado" );
    else if (ch->pcdata->favor > 1250)	 sprintf( buf, "privilegiado" );
    else if (ch->pcdata->favor > 1000)	 sprintf( buf, "respetado" );
    else if (ch->pcdata->favor > 750)	 sprintf( buf, "gusta" );
    else if (ch->pcdata->favor > 250)	 sprintf( buf, "tolerado" );
    else if (ch->pcdata->favor > -250)	 sprintf( buf, "ignorado" );
    else if (ch->pcdata->favor > -750)	 sprintf( buf, "esquivado" );
    else if (ch->pcdata->favor > -1000)	 sprintf( buf, "disgusta" );
    else if (ch->pcdata->favor > -1250)	 sprintf( buf, "deshonrado" );
    else if (ch->pcdata->favor > -1500)	 sprintf( buf, "abandonado" );
    else if (ch->pcdata->favor > -1750)	 sprintf( buf, "olvidado" );
    else if (ch->pcdata->favor > -2000)	 sprintf( buf, "despreciado" );
    else if (ch->pcdata->favor > -2250)	 sprintf( buf, "odiado" );
    else				 sprintf( buf, "maldito" );

    if ( ch->level < 10 )
    {
       if (ch->alignment > 900)		 sprintf(buf2, "angelico");
       else if (ch->alignment > 700)	 sprintf(buf2, "noble");
       else if (ch->alignment > 350)	 sprintf(buf2, "honorable");
       else if (ch->alignment > 100)	 sprintf(buf2, "bueno");
       else if (ch->alignment > -100)	 sprintf(buf2, "neutral");
       else if (ch->alignment > -350)	 sprintf(buf2, "malo");
       else if (ch->alignment > -700)	 sprintf(buf2, "malvado");
       else if (ch->alignment > -900)	 sprintf(buf2, "caotico");
       else				 sprintf(buf2, "satanico");
    }
    else
	sprintf(buf2, "%d", ch->alignment );
    pager_printf(ch, "|Nivel: %-4d |Favor: %-10s|Alin.: %-9s     |Experiencia: %-9d|\n\r",
                     ch->level, buf, buf2, ch->exp );
    send_to_pager("-----------------------------------------------------------------------------\n\r", ch);
        switch (ch->style) {
        case STYLE_EVASIVE:
                sprintf(buf, "evasivo");
                break;
        case STYLE_DEFENSIVE:
                sprintf(buf, "defensivo");
                break;
        case STYLE_AGGRESSIVE:
                sprintf(buf, "aggresivo");
                break;
        case STYLE_BERSERK:
                sprintf(buf, "berserk");
                break;
        default:
                sprintf(buf, "estandar");
                break;
        }
    pager_printf(ch, "|PQuest: %-3d |Peso: %-9d  |Estilo: %-12s |Oro: %-14s   |\n\r",
                 ch->pcdata->quest_curr, ch->carry_weight, buf, num_punct(ch->gold) );
    send_to_pager("-----------------------------------------------------------------------------\n\r", ch);
    if ( ch->level < 15 && !IS_PKILL(ch) )
    	pager_printf(ch, "|            |Hitroll: -------- |Damroll: ----------- |                     |\n\r" );
    else
      pager_printf(ch, "|            |Hitroll: %-7d |Damroll: %-11d |Generacion:%d          |\n\r", GET_HITROLL(ch), GET_DAMROLL(ch), ch->generacion );
    send_to_pager("-----------------------------------------------------------------------------\n\r", ch);
    return;
}

/*
 * New score command by Haus
 */
void do_score(CHAR_DATA * ch, char *argument)
{
    char            buf[MAX_STRING_LENGTH];
    AFFECT_DATA    *paf;

    if (IS_NPC(ch))
    {
	do_oldscore(ch, argument);
	return;
    }
    set_pager_color(AT_GREY, ch);

    pager_printf(ch, "\n\rFicha para %s%s.\n\r", ch->name, ch->pcdata->title);
    if ( get_trust( ch ) != ch->level )
	pager_printf( ch, "tienes un trust a nivel %d.\n\r", get_trust( ch ) );

    send_to_pager("----------------------------------------------------------------------------\n\r", ch);

    pager_printf(ch,    "NIVEL: %-3d             Raza : %-10.10s     Horas: %d horas\n\r",
   ch->level, capitalize(get_race(ch)), (get_age(ch) - 17) * 2);

    if ( IS_VAMPIRE(ch) ) {
    pager_printf(ch, "EDAD:  %-6d      ClanCainita: %-11.11s   Log In: %s\r",
		get_age(ch), capitalize(get_class(ch)), ctime(&(ch->logon)) ); }
    if ( !IS_VAMPIRE(ch) ) {
    pager_printf(ch, "EDAD:  %-6d      Profesion: %-11.11s   Log In: %s\r",
		get_age(ch), capitalize(get_class(ch)), ctime(&(ch->logon)) ); }

    send_to_pager("----------------------------------------------------------------------------\n\r", ch);

    if (ch->level >= 15
    ||  IS_PKILL( ch ) )
    {
	pager_printf(ch, "FUE  : %2.2d(%2.2d)      HitRoll: %-4d            Salvado:  %s\r",
		get_curr_str(ch), ch->perm_str, GET_HITROLL(ch), ch->save_time ? ctime(&(ch->save_time)) : "no save this session\n" );

	pager_printf(ch, "INT  : %2.2d(%2.2d)      DamRoll: %-4d            Hora:   %s\r",
		get_curr_int(ch), ch->perm_int, GET_DAMROLL(ch), ctime(&current_time) );
    }
    else
    {
	pager_printf(ch, "FUE  : %2.2d(%2.2d)                               Salvado:  %s\r",
		get_curr_str(ch), ch->perm_str, ch->save_time ? ctime(&(ch->save_time)) : "no\n" );

	pager_printf(ch, "INT  : %2.2d(%2.2d)                               Hora:   %s\r",
		get_curr_int(ch), ch->perm_int, ctime(&current_time) );
    }

    if (GET_AC(ch) >= 101)
	sprintf(buf, "peor que DESNUDO!");
    else if (GET_AC(ch) >= 90)
	sprintf(buf, "sin nada que te proteja");
    else if (GET_AC(ch) >= 75)
	sprintf(buf, "con unos pocos harapos");
    else if (GET_AC(ch) >= 60)
	sprintf(buf, "mal vestido");
    else if (GET_AC(ch) >= 30)
	sprintf(buf, "sin armadura");
    else if (GET_AC(ch) >= 10)
	sprintf(buf, "mal protegido");
    else if (GET_AC(ch) >= 0)
	sprintf(buf, "algo protegido");
    else if (GET_AC(ch) >= -10)
	sprintf(buf, "bien protegido");
    else if (GET_AC(ch) >= -30)
	sprintf(buf, "muy bien protegido");
    else if (GET_AC(ch) >= -60)
	sprintf(buf, "la envidia de los caballeros");
    else if (GET_AC(ch) >= -90)
	sprintf(buf, "la envidia de los duques");
    else if (GET_AC(ch) >= -120)
	sprintf(buf, "la envidia de los reyes");
    else if (GET_AC(ch) >= -200)
	sprintf(buf, "como un avatar");
    else
	sprintf(buf, "INVENCIBLE!");
    if (ch->level > 24)
	pager_printf(ch, "SAB  : %2.2d(%2.2d)                               Armadura: %4.4d, %s\n\r",
		get_curr_wis(ch), ch->perm_wis, GET_AC(ch), buf);
    else
	pager_printf(ch, "SAB  : %2.2d(%2.2d)                               Armadura: %s \n\r",
		get_curr_wis(ch), ch->perm_wis, buf);

    if (ch->alignment > 900)
	sprintf(buf, "angelico");
    else if (ch->alignment > 700)
	sprintf(buf, "noble");
    else if (ch->alignment > 350)
	sprintf(buf, "honorable");
    else if (ch->alignment > 100)
	sprintf(buf, "bueno");
    else if (ch->alignment > -100)
	sprintf(buf, "neutral");
    else if (ch->alignment > -350)
	sprintf(buf, "malo");
    else if (ch->alignment > -700)
	sprintf(buf, "malvado");
    else if (ch->alignment > -900)
	sprintf(buf, "caotico");
    else
	sprintf(buf, "satanico");
    if (ch->level < 10)
	pager_printf(ch, "DES  : %2.2d(%2.2d)      Alin: %-20.20s  Items: %5.5d   (max %5.5d)\n\r",
		get_curr_dex(ch), ch->perm_dex, buf, ch->carry_number, can_carry_n(ch));
    else
	pager_printf(ch, "DES  : %2.2d(%2.2d)      Alin: %+4.4d, %-10.10s  Items: %5.5d   (max %5.5d)\n\r",
		get_curr_dex(ch), ch->perm_dex, ch->alignment, buf, ch->carry_number, can_carry_n(ch));

    switch (ch->position)
    {
	case POS_DEAD:
		sprintf(buf, "Muerto");
		break;
	case POS_MORTAL:
		sprintf(buf, "Mortalmente herido");
		break;
	case POS_INCAP:
		sprintf(buf, "Incapacitado");
		break;
	case POS_STUNNED:
		sprintf(buf, "Aturdido");
		break;
	case POS_SLEEPING:
		sprintf(buf, "Durmiendo");
		break;
	case POS_RESTING:
		sprintf(buf, "Descansando");
		break;
	case POS_STANDING:
		sprintf(buf, "De pie");
		break;
	case POS_FIGHTING:
		sprintf(buf, "Luchando");
		break;
        case POS_EVASIVE:
                sprintf(buf, "Luchando (evasivo)");   /* Fighting style support -haus */
                break;
        case POS_DEFENSIVE:
                sprintf(buf, "Luchando (defensivo)");
                break;
        case POS_AGGRESSIVE:
                sprintf(buf, "Luchando (agresivo)");
                break;
        case POS_BERSERK:
                sprintf(buf, "Luchando (berserk)");
                break;
	case POS_MOUNTED:
		sprintf(buf, "Montado");
		break;
        case POS_SITTING:
		sprintf(buf, "Sentado");
		break;
    }
    pager_printf(ch, "CON  : %2.2d(%2.2d)      Pos'n: %-17.17s Peso: %5.5d (max %7.7d)\n\r",
	get_curr_con(ch), ch->perm_con, buf, ch->carry_weight, can_carry_w(ch));


    /*
     * Fighting style support -haus
     */
    pager_printf(ch, "CAR  : %2.2d(%2.2d)      Wimpy: %-5d            ",
	get_curr_cha(ch), ch->perm_cha, ch->wimpy);

        switch (ch->style) {
        case STYLE_EVASIVE:
                sprintf(buf, "evasivo");
                break;
        case STYLE_DEFENSIVE:
                sprintf(buf, "defensivo");
                break;
        case STYLE_AGGRESSIVE:
                sprintf(buf, "agresivo");
                break;
        case STYLE_BERSERK:
                sprintf(buf, "berserk");
                break;
        default:
                sprintf(buf, "estandar");
                break;
        }
    pager_printf(ch, " Estilo: %-10.10s\n\r", buf);

    pager_printf(ch, "SUE  : %2.2d(%2.2d) \n\r",
	get_curr_lck(ch), ch->perm_lck);

    send_to_pager("----------------------------------------------------------------------------\n\r", ch);

    pager_printf(ch, "PQuest: %4.4d(%4.4d) \n\r",
	ch->pcdata->quest_curr, ch->pcdata->quest_accum );

    send_to_pager("----------------------------------------------------------------------------\n\r", ch);

    pager_printf(ch, "PRACT: %3.3d             PV: %-5d of %5d     Pag: (%c) %3d        AutoExit(%c)\n\r",
	ch->practice, ch->hit, ch->max_hit,
	IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ',
	ch->pcdata->pagerlen, xIS_SET(ch->act, PLR_AUTOEXIT) ? 'X' : ' ');

    if (IS_VAMPIRE(ch))
	pager_printf(ch, "XP   : %-9d       Sangre: %-5d of %5d MobKills:  %-5.5d    AutoLoot(%c)\n\r",
		ch->exp, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level + (13 / ch->generacion)  * 40, ch->pcdata->mkills,
		xIS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');
    else if (ch->class == CLASS_WARRIOR)
	pager_printf(ch, "XP   : %-9d                               MobKills:  %-5.5d  AutoLoot(%c)\n\r",
		ch->exp, ch->pcdata->mkills, xIS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');
    else
	pager_printf(ch, "XP   : %-9d       Mana: %-5d of %5d   MobKills:  %-5.5d    AutoLoot(%c)\n\r",
		ch->exp, ch->mana, ch->max_mana, ch->pcdata->mkills, xIS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');

    pager_printf(ch, "ORO : %-13s    Mov: %-5d of %5d    Mobdeaths: %-5.5d    AutoSac (%c)\n\r",
	num_punct(ch->gold), ch->move, ch->max_move, ch->pcdata->mdeaths, xIS_SET(ch->act, PLR_AUTOSAC) ? 'X' : ' ');
    if ( !IS_NPC(ch) && ch->generacion <= GENERACION_MATUSALEN)
    pager_printf(ch, "Datos de Generacion: Eres %d generacion - No puedes mejorararla\n\r", ch->generacion );
    if ( !IS_NPC(ch) && ch->generacion >= GENERACION_QUINTA)
    pager_printf(ch, "Datos de Generacion: Eres %d generacion - Aun puedes mejorarlas \n\r", ch->generacion );

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	send_to_pager("Estas borracho.\n\r", ch);
    if (!IS_NPC(ch) && !IS_VAMPIRE(ch) && ch->pcdata->condition[COND_THIRST] == 0)
	send_to_pager("Estas en peligro de deshidratacion.\n\r", ch);
    if (!IS_NPC(ch) && !IS_VAMPIRE(ch) && ch->pcdata->condition[COND_FULL] == 0)
	send_to_pager("Te estas muriendo de hambre.\n\r", ch);
    if ( ch->position != POS_SLEEPING )
	switch( ch->mental_state / 10 )
	{
	    default:   send_to_pager( "Estas completamente ido!\n\r", ch );	break;
	    case -10:  send_to_pager( "Estas perdiendo la consciencia.\n\r", ch );	break;
	    case  -9:  send_to_pager( "No puedes mantener tus ojos abiertos.\n\r", ch );	break;
	    case  -8:  send_to_pager( "Tienes MUCHO suenyo.\n\r", ch );	break;
	    case  -7:  send_to_pager( "Estas muy desmotivado.\n\r", ch );	break;
	    case  -6:  send_to_pager( "Te sientes sedado.\n\r", ch );		break;
	    case  -5:  send_to_pager( "Tienes suenyo.\n\r", ch );		break;
	    case  -4:  send_to_pager( "Estas cansado.\n\r", ch );		break;
	    case  -3:  send_to_pager( "Podrias descansar.\n\r", ch );		break;
	    case  -2:  send_to_pager( "Te pesan algo los ojos.\n\r", ch );	break;
	    case  -1:  send_to_pager( "Estas bastante bien.\n\r", ch );		break;
	    case   0:  send_to_pager( "Estas muy bien.\n\r", ch );		break;
	    case   1:  send_to_pager( "Te sientes bien.\n\r", ch );	break;
	    case   2:  send_to_pager( "Te sientes poderoso.\n\r", ch );	break;
	    case   3:  send_to_pager( "No puedes pensar claramente.\n\r", ch );	break;
	    case   4:  send_to_pager( "Tu mente va a 100 km/h.\n\r", ch );	break;
	    case   5:  send_to_pager( "Todo te da vueltas.\n\r", ch );	break;
	    case   6:  send_to_pager( "Tu cuerpo y tu mente se estan separando.\n\r", ch );	break;
	    case   7:  send_to_pager( "La realidad se viene abajo.\n\r", ch );	break;
	    case   8:  send_to_pager( "No sabes lo que es real y lo que no.\n\r", ch );	break;
	    case   9:  send_to_pager( "Te sientes inmortal.\n\r", ch );	break;
	    case  10:  send_to_pager( "Eres una Entidad Suprema.\n\r", ch );	break;
	}
    else
    if ( ch->mental_state >45 )
	send_to_pager( "Tu dormir esta mezclado con estranyos y reales suenyos.\n\r", ch );
    else
    if ( ch->mental_state >25 )
	send_to_pager( "Dormir no es facil para ti.\n\r", ch );
    else
    if ( ch->mental_state <-35 )
	send_to_pager( "Estas sumido en una enorme necesidad de dormir.\n\r", ch );
    else
    if ( ch->mental_state <-25 )
	send_to_pager( "Estas profundamente dormido.\n\r", ch );

    if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
	pager_printf( ch, "Se te han anyadido los comandos: %s.\n\r",
		ch->pcdata->bestowments );

    if ( ch->morph && ch->morph->morph )
    {
      send_to_pager ("-----------------------------------------------------------------------------\n\r", ch);
      if ( IS_IMMORTAL( ch ) )
         pager_printf (ch, "Polimorfado como (%d) %s con un timer de %d.\n\r",
                ch->morph->morph->vnum, ch->morph->morph->short_desc,
		ch->morph->timer
                );
      else
        pager_printf (ch, "Te has polimorfado en un %s.\n\r",
                ch->morph->morph->short_desc );
      send_to_pager ("-----------------------------------------------------------------------------\n\r", ch);
    }
    if ( CAN_PKILL( ch ) )
    {
   send_to_pager( "----------------------------------------------------------------------------\n\r", ch);
   pager_printf(ch, "DATOS DE PK:Pks (%3.3d) Pks Ilegalales (%3.3d) Pkdeaths (%3.3d) GenePk (%3.3d)\n\r",
      ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths, ch->pcdata->genepkills );
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER  && ch->pcdata->clan->clan_type != CLAN_GUILD )
    {
   send_to_pager( "----------------------------------------------------------------------------\n\r", ch);
   pager_printf(ch, "DATOS DE CLAN:  %-14.14s  Clan AvPkills : %-5d  Clan NoAvpkills : %-5d\n\r",
		ch->pcdata->clan->name, ch->pcdata->clan->pkills[6],
		(ch->pcdata->clan->pkills[1]+ch->pcdata->clan->pkills[2]+
		 ch->pcdata->clan->pkills[3]+ch->pcdata->clan->pkills[4]+
		 ch->pcdata->clan->pkills[5]) );
        pager_printf(ch, "                                Clan AvPdeaths: %-5d  Clan NoAvpdeaths: %-5d\n\r",
		ch->pcdata->clan->pdeaths[6],
		( ch->pcdata->clan->pdeaths[1] + ch->pcdata->clan->pdeaths[2] +
		  ch->pcdata->clan->pdeaths[3] + ch->pcdata->clan->pdeaths[4] +
		  ch->pcdata->clan->pdeaths[5] ) );
    }
  if (ch->level >= LEVEL_AVATAR)
  {
   send_to_pager( "----------------------------------------------------------------------------\n\r", ch);
   if (xIS_SET(ch->act, PLR_CAMUFLAJE) )
   pager_printf(ch, "CAMUFLAJE: %-50.50s Activado\n\r", ch->camuflaje);

   if (!xIS_SET(ch->act, PLR_CAMUFLAJE) )
   pager_printf(ch, "CAMUFLAJE: %-50.50s Desactivado\n\r", ch->camuflaje);
  }
  if (ch->pcdata->deity)
    {
	send_to_pager( "----------------------------------------------------------------------------\n\r", ch);
	if (ch->pcdata->favor > 2250)
	  sprintf( buf, "amado" );
	else if (ch->pcdata->favor > 2000)
	  sprintf( buf, "querido" );
	else if (ch->pcdata->favor > 1750)
	  sprintf( buf, "honrado" );
	else if (ch->pcdata->favor > 1500)
	  sprintf( buf, "elogiado" );
	else if (ch->pcdata->favor > 1250)
	  sprintf( buf, "privilegiado" );
	else if (ch->pcdata->favor > 1000)
	  sprintf( buf, "respetado" );
	else if (ch->pcdata->favor > 750)
	  sprintf( buf, "gusta" );
	else if (ch->pcdata->favor > 250)
	  sprintf( buf, "tolerado" );
	else if (ch->pcdata->favor > -250)
	  sprintf( buf, "ignorado" );
	else if (ch->pcdata->favor > -750)
	  sprintf( buf, "esquivado" );
	else if (ch->pcdata->favor > -1000)
	  sprintf( buf, "disgusta" );
	else if (ch->pcdata->favor > -1250)
	  sprintf( buf, "deshonrado" );
	else if (ch->pcdata->favor > -1500)
	  sprintf( buf, "abandonado" );
	else if (ch->pcdata->favor > -1750)
	  sprintf( buf, "olvidado" );
	else if (ch->pcdata->favor > -2000)
	  sprintf( buf, "despreciado" );
	else if (ch->pcdata->favor > -2250)
	  sprintf( buf, "odiado" );
	else
	  sprintf( buf, "maldito" );
	pager_printf(ch, "Deidad: %-20s  Favor: %s\n\r", ch->pcdata->deity->name, buf );
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
    {
        send_to_pager( "----------------------------------------------------------------------------\n\r", ch);
	pager_printf(ch, "Orden:  %-20s  Orden Mobkills:  %-6d   Orden MobDeaths:  %-6d\n\r",
		ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD )
    {
        send_to_pager( "----------------------------------------------------------------------------\n\r", ch);
        pager_printf(ch, "Gremio:  %-20s  Gremio Mobkills:  %-6d   Gremio MobDeaths:  %-6d\n\r",
                ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
    }
    if (IS_IMMORTAL(ch))
    {
	send_to_pager( "----------------------------------------------------------------------------\n\r", ch);

	pager_printf(ch, "DATOS DE INMORTAL:  Wizinvis [%s]  Wizlevel (%d)\n\r",
		xIS_SET(ch->act, PLR_WIZINVIS) ? "X" : " ", ch->pcdata->wizinvis );

	pager_printf(ch, "Bamfin:  %s %s\n\r", ch->name, (ch->pcdata->bamfin[0] != '\0')
		? ch->pcdata->bamfin : "appears in a swirling mist.");
	pager_printf(ch, "Bamfout: %s %s\n\r", ch->name, (ch->pcdata->bamfout[0] != '\0')
		? ch->pcdata->bamfout : "leaves in a swirling mist.");


	/* Area Loaded info - Scryn 8/11*/
	if (ch->pcdata->area)
	{
	    pager_printf(ch, "Vnums:   Room (%-5.5d - %-5.5d)   Objeto (%-5.5d - %-5.5d)   Mob (%-5.5d - %-5.5d)\n\r",
		ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
		ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum,
		ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
	    pager_printf(ch, "Area Cargada [%s]\n\r", (IS_SET (ch->pcdata->area->status, AREA_LOADED)) ? "si" : "no");
	}
    }
    if (ch->first_affect)
    {
	int i;
	SKILLTYPE *sktmp;

	i = 0;
	send_to_pager( "----------------------------------------------------------------------------\n\r", ch);
	send_to_pager("AFECTADO POR:                           ", ch);
	for (paf = ch->first_affect; paf; paf = paf->next)
	{
	    if ( (sktmp=get_skilltype(paf->type)) == NULL )
		continue;
	    if (ch->level < 20)
	    {
		pager_printf(ch, "[%-34.34s]    ", sktmp->name);
		if (i == 0)
		   i = 2;
		if ((++i % 3) == 0)
		   send_to_pager("\n\r", ch);
	     }
	     if (ch->level >= 20)
	     {
		if (paf->modifier == 0)
		    pager_printf(ch, "[%-24.24s;%5d rds]    ",
			sktmp->name,
			paf->duration/100);
		else
		if (paf->modifier > 999)
		    pager_printf(ch, "[%-15.15s; %7.7s;%5d rds]    ",
			sktmp->name,
			tiny_affect_loc_name(paf->location),
			paf->duration/100);
		else
		    pager_printf(ch, "[%-11.11s;%+-3.3d %7.7s;%5d rds]    ",
			sktmp->name,
			paf->modifier,
			tiny_affect_loc_name(paf->location),
			paf->duration/100);
		if (i == 0)
		    i = 1;
		if ((++i % 2) == 0)
		    send_to_pager("\n\r", ch);
	    }
	}
    }
    send_to_pager("\n\r", ch);
    return;
}

void do_newscore(CHAR_DATA * ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA    *paf;
    int iLang;

    if (IS_NPC(ch))
    {
	do_oldscore(ch, argument);
	return;
    }
    set_pager_color(AT_SCORE, ch);

    pager_printf_color(ch, "\n\r&C%s%s.\n\r", ch->name, ch->pcdata->title);
    if ( get_trust( ch ) != ch->level )
	pager_printf( ch, "Tienes un trust a nivel %d.\n\r", get_trust( ch ) );

    send_to_pager_color("&W----------------------------------------------------------------------------\n\r", ch);

    if (IS_VAMPIRE(ch))
    pager_printf_color(ch, "Nivel: &W%-3d         &CSecta : &W%-10.10s          &CRemort &W(&w%d&W)&C Horas:&W%d \n\r",
    ch->level, capitalize(get_race(ch)), ch->pcdata->renacido, (get_age(ch) - 17) * 2);
    else if (ES_MAGO(ch))
    pager_printf_color(ch, "Nivel: &W%-3d        &CCabala : &W%-10.10s           &CRemort &W(&w%d&W)&C Horas:&W%d \n\r",
    ch->level, capitalize(get_race(ch)), ch->pcdata->renacido, (get_age(ch) - 17) * 2);
    else if (ES_CRUZADO(ch))
    pager_printf_color(ch, "Nivel: &W%-3d         &CSendero : &W%-10.10s           &CRemort &W(&w%d&W)&C Horas:&W%d \n\r",
    ch->level, capitalize(get_race(ch)), ch->pcdata->renacido, (get_age(ch) - 17) * 2);
    else if (ES_HADA(ch))
    pager_printf_color(ch, "Nivel: &W%-3d          &CTribu : &W%-10.10s           &CRemort &W(&w%d&W)&C Horas:&W%d \n\r",
    ch->level, capitalize(get_race(ch)), ch->pcdata->renacido, (get_age(ch) - 17) * 2);
    else if (ES_LICANTROPO(ch))
    pager_printf_color(ch, "Nivel: &W%-3d         &CCamada : &W%-10.10s           &CRemort &W(&w%d&W)&C Horas:&W%d \n\r",
    ch->level, capitalize(get_race(ch)), ch->pcdata->renacido, (get_age(ch) - 17) * 2);
    else
    pager_printf_color(ch, "Nivel: &W%-3d         &CRaza : &W%-10.10s        &CRemort &W(&w%d&W)&C Horas:&W%d \n\r",
    ch->level, capitalize(get_race(ch)), ch->pcdata->renacido, (get_age(ch) - 17) * 2);


    if ( IS_VAMPIRE(ch))
    pager_printf_color(ch, "&CAnyos: &W%-6d      &CClan Cainita: &W%-11.11s   &CLog In: %s\r",
		get_age(ch), capitalize(get_class(ch)), ctime(&(ch->logon)) );

    if ( !IS_VAMPIRE(ch))
    pager_printf_color(ch, "&CAnyos: &W%-6d      &CProfesion: &W%-11.11s   &CLog In: %s\r",
		get_age(ch), capitalize(get_class(ch)), ctime(&(ch->logon)) );

  send_to_pager_color("&W----------------------------------------------------------------------------\n\r", ch);

    if (ch->level >= 15
    ||  IS_PKILL( ch ) )
    {
   pager_printf_color(ch, "&CFUE  : &W%2.2d&C(&w%2.2d&C)      HitRoll: &R%-4d            &CSalvado: %s\r",
		get_curr_str(ch), ch->perm_str, GET_HITROLL(ch), ch->save_time ? ctime(&(ch->save_time)) : "no en esta sesion\n" );

   pager_printf_color(ch, "&CINT  : &W%2.2d&C(&w%2.2d&C)      DamRoll: &R%-4d            &CHora: %s\r",
		get_curr_int(ch), ch->perm_int, GET_DAMROLL(ch), ctime(&current_time) );
    }
    else
    {
	pager_printf_color(ch, "&CFUE  : &W%2.2d&C(&w%2.2d&C)                             Salvado:  %s\r",
		get_curr_str(ch), ch->perm_str, ch->save_time ? ctime(&(ch->save_time)) : "no\n" );

	pager_printf_color(ch, "&CINT  : &W%2.2d&C(&w%2.2d&C)                               Hora:   %s\r",
		get_curr_int(ch), ch->perm_int, ctime(&current_time) );
    }

    if (GET_AC(ch) >= 101)
	sprintf(buf, "peor que DESNUDO!");
    else if (GET_AC(ch) >= 90)
	sprintf(buf, "sin nada que te proteja");
    else if (GET_AC(ch) >= 75)
	sprintf(buf, "con unos pocos arapos");
    else if (GET_AC(ch) >= 60)
	sprintf(buf, "mal vestido");
    else if (GET_AC(ch) >= 30)
	sprintf(buf, "sin armadura");
    else if (GET_AC(ch) >= 10)
	sprintf(buf, "mal protegido");
    else if (GET_AC(ch) >= 0)
	sprintf(buf, "algo protegido");
    else if (GET_AC(ch) >= -10)
	sprintf(buf, "bien protegido");
    else if (GET_AC(ch) >= -30)
	sprintf(buf, "muy bien protegido");
    else if (GET_AC(ch) >= -60)
	sprintf(buf, "la envidia de los caballeros");
    else if (GET_AC(ch) >= -90)
	sprintf(buf, "la envidia de los duques");
    else if (GET_AC(ch) >= -120)
	sprintf(buf, "la envidia de los reyes");
    else if (GET_AC(ch) >= -200)
	sprintf(buf, "como un avatar");
    else
	sprintf(buf, "INVENCIBLE!");
    if (ch->level > 24)
	pager_printf_color(ch, "&CSAB  : &W%2.2d&C(&w%2.2d&C)      Armadura: &W%-d; %s\n\r",
		get_curr_wis(ch), ch->perm_wis, GET_AC(ch), buf);
    else
	pager_printf_color(ch, "&CSAB  : &W%2.2d&C(&w%2.2d&C)      Armadura: &W%s \n\r",
		get_curr_wis(ch), ch->perm_wis, buf);

    if (ch->alignment > 900)
	sprintf(buf, "angelico");
    else if (ch->alignment > 700)
	sprintf(buf, "noble");
    else if (ch->alignment > 350)
	sprintf(buf, "honrado");
    else if (ch->alignment > 100)
	sprintf(buf, "bueno");
    else if (ch->alignment > -100)
	sprintf(buf, "neutral");
    else if (ch->alignment > -350)
	sprintf(buf, "malo");
    else if (ch->alignment > -700)
	sprintf(buf, "malvado");
    else if (ch->alignment > -900)
	sprintf(buf, "caotico");
    else
	sprintf(buf, "satanico");
    if (ch->level < 10)
	pager_printf_color(ch, "&CDES  : &W%2.2d&C(&w%2.2d&C)      Alin: &W%-20.20s     &CItems:  &W%d (max %d)\n\r",
		get_curr_dex(ch), ch->perm_dex, buf, ch->carry_number, can_carry_n(ch));
    else
	pager_printf_color(ch, "&CDES  : &W%2.2d&C(&w%2.2d&C)      Alin: &W%4d; %-14.14s    &CItems:  &W%d &w(max %d)\n\r",
		get_curr_dex(ch), ch->perm_dex, ch->alignment, buf, ch->carry_number, can_carry_n(ch));

    switch (ch->position)
    {
	case POS_DEAD:
		sprintf(buf, "muerto");
		break;
	case POS_MORTAL:
		sprintf(buf, "mortalmente herido");
		break;
	case POS_INCAP:
		sprintf(buf, "incapacitado");
		break;
	case POS_STUNNED:
		sprintf(buf, "aturdido");
		break;
	case POS_SLEEPING:
		sprintf(buf, "durmiendo");
		break;
	case POS_RESTING:
		sprintf(buf, "descansando");
		break;
	case POS_STANDING:
		sprintf(buf, "de pie");
		break;
	case POS_FIGHTING:
		sprintf(buf, "luchando");
		break;
        case POS_EVASIVE:
                sprintf(buf, "luchando (evasivo)");   /* Fighting style support -haus */
                break;
        case POS_DEFENSIVE:
                sprintf(buf, "luchando (defensivo)");
                break;
        case POS_AGGRESSIVE:
                sprintf(buf, "luchando (agresivo)");
                break;
        case POS_BERSERK:
                sprintf(buf, "luchando (berserk)");
                break;
	case POS_MOUNTED:
		sprintf(buf, "montado");
		break;
        case POS_SITTING:
		sprintf(buf, "sentado");
		break;
    }
    pager_printf_color(ch, "&CCON  : &W%2.2d&C(&w%2.2d&C)      Pos'n: &W%-21.21s  &CPeso: &W%d &w(max %d)\n\r",
	get_curr_con(ch), ch->perm_con, buf, ch->carry_weight, can_carry_w(ch));


    /*
     * Fighting style support -haus
     */
    pager_printf_color(ch, "&CCAR  : &W%2.2d&C(&w%2.2d&C)      Wimpy: &Y%-5d      ",
	get_curr_cha(ch), ch->perm_cha, ch->wimpy);

        switch (ch->style) {
        case STYLE_EVASIVE:
                sprintf(buf, "evasivo");
                break;
        case STYLE_DEFENSIVE:
                sprintf(buf, "defensivo");
                break;
        case STYLE_AGGRESSIVE:
                sprintf(buf, "agresivo");
                break;
        case STYLE_BERSERK:
                sprintf(buf, "berserk");
                break;
        default:
                sprintf(buf, "estandar");
                break;
        }
    pager_printf_color(ch, "\n\r&CSUE  : &W%2.2d&C(&w%2.2d&C)      Estilo: &W%-10.10s\n\r",
	get_curr_lck(ch), ch->perm_lck, buf );
    send_to_pager_color("&W----------------------------------------------------------------------------&C\n\r", ch);
    switch ( ch->generacion)
    {
               case MAX_GENERACION :
               sprintf( buf, "NeoNaTo" );
               break;
               case GENERACION_DECIMOSEGUNDA :
               sprintf( buf, "DoCeaVa GeNeRaCioN" );
               break;
               case GENERACION_DECIMOPRIMERA :
               sprintf( buf, "DeCiMoPRiMeRa GeNeRaCioN" );
               break;
               case GENERACION_DECIMA :
               sprintf( buf, "DeCiMa GeNeRaCioN" );
               break;
               case GENERACION_NOVENA :
               sprintf( buf, "NoVeNa GeNeRaCioN" );
               break;
               case GENERACION_OCTAVA :
               sprintf( buf, "OcTaVa GeNeRaCioN" );
               break;
               case GENERACION_SEPTIMA :
               sprintf( buf, "SepTiMa GeNeRaCioN" );
               break;
               case GENERACION_SEXTA :
               sprintf( buf, "SexTa GeNeRaCioN" );
               break;
               case GENERACION_QUINTA :
               sprintf( buf, "QuiNTa GeNeRaCioN" );
               break;
               case GENERACION_MATUSALEN :
               sprintf( buf, "MaTuSaLeN" );
               break;
               case GENERACION_ANTEDILUVIANO :
               sprintf( buf, "AnTeDiLuViaNo" );
               break;
               case GENERACION_SEGUNDA :
               sprintf( buf, "SeGuNDa GeNeRaCioN" );
               break;
               case GENERACION_CAIN :
               sprintf( buf, "CaIn" );
               break;
            }

    if (IS_VAMPIRE(ch))
    pager_printf_color(ch, "&CPQuest: &W%d&C/&w%d   &CEres un  &R%s\n\r",
    ch->pcdata->quest_curr, ch->pcdata->quest_accum, buf );
       switch ( ch->generacion )
       {
               case MAX_GENERACION :
               sprintf( buf, "Novato" );
               break;
               case GENERACION_DECIMOSEGUNDA :
               sprintf( buf, "Aprendiz" );
               break;
               case GENERACION_DECIMOPRIMERA :
               sprintf( buf, "Estudiante" );
               break;
               case GENERACION_DECIMA :
               sprintf( buf, "Iluminado" );
               break;
               case GENERACION_NOVENA :
               sprintf( buf, "Sabio" );
               break;
               case GENERACION_OCTAVA :
               sprintf( buf, "Maestro" );
               break;
               case GENERACION_SEPTIMA :
               sprintf( buf, "Experto" );
               break;
               case GENERACION_SEXTA :
               sprintf( buf, "Inhumano" );
               break;
               case GENERACION_QUINTA :
               sprintf( buf, "Celestial" );
               break;
               case GENERACION_MATUSALEN :
               sprintf( buf, "Destructor" );
               break;
               case GENERACION_ANTEDILUVIANO :
               sprintf( buf, "Divinidad" );
               break;
               case GENERACION_SEGUNDA :
               sprintf( buf, "Legendario" );
               break;
               case GENERACION_CAIN :
               sprintf( buf, "HeRaLDo" );
               break;
            }

    if (!IS_VAMPIRE(ch))
    pager_printf_color(ch, "&CPQuest: &W%d&C/&w%d   &CRango: &R%s\n\r",
    ch->pcdata->quest_curr, ch->pcdata->quest_accum, buf );

    send_to_pager_color("&W----------------------------------------------------------------------------&C\n\r", ch);

    pager_printf_color(ch, "&CPRACT: &W%3d            &CPv: &G%-5d &Cof &g%5d       &CPag: (&W%c&C) &W%3d    &CAutoExit(&W%c&C)\n\r",
	ch->practice, ch->hit, ch->max_hit,
	IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ',
	ch->pcdata->pagerlen, xIS_SET(ch->act, PLR_AUTOEXIT) ? 'X' : ' ');

    if (IS_VAMPIRE(ch))
   pager_printf_color(ch, "&CEXP  : &W%-9d       &CSangre: &R%-5d &Cof &r%5d   &CMobKills:  &W%5d   &CAutoLoot(&W%c&C)\n\r",
		ch->exp, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level + (13 / ch->generacion) * 40, ch->pcdata->mkills,
		xIS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');
    else if (ch->class == CLASS_WARRIOR)
	pager_printf_color(ch, "&CEXP  : &W%-9d                              &CMobKills:  &W%5d   &CAutoLoot(&W%c&C)\n\r",
		ch->exp, ch->pcdata->mkills, xIS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');
    else
	pager_printf_color(ch, "&CEXP  : &W%-9d       &CMana: &B%-5d &Cof &b%5d   &CMobKills:  &W%5d   &CAutoLoot(&W%c&C)\n\r",
		ch->exp, ch->mana, ch->max_mana, ch->pcdata->mkills, xIS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');

    pager_printf_color(ch, "&CORO : &Y%-13s    &CMov: &W%-5d &Cof &w%5d      &CMobdeaths: &W%5d   &CAutoSac (&W%c&C)\n\r",
	num_punct(ch->gold), ch->move, ch->max_move, ch->pcdata->mdeaths, xIS_SET(ch->act, PLR_AUTOSAC) ? 'X' : ' ');

    if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
	send_to_pager("Estas borracho.\n\r", ch);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0)
	send_to_pager("Estas en peligro de deshidratacion.\n\r", ch);
    if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] == 0)
	send_to_pager("Te estas muriendo de hambre.\n\r", ch);
    if ( ch->position != POS_SLEEPING )
	switch( ch->mental_state / 10 )
	{
          default:   send_to_pager( "Estas completamente ido!\n\r", ch );	break;
	    case -10:  send_to_pager( "Estas perdiendo la consciencia.\n\r", ch );	break;
	    case  -9:  send_to_pager( "No puedes mantener tus ojos abiertos.\n\r", ch );	break;
	    case  -8:  send_to_pager( "Tienes MUCHO suenyo.\n\r", ch );	break;
	    case  -7:  send_to_pager( "Estas muy desmotivado.\n\r", ch );	break;
	    case  -6:  send_to_pager( "Te sientes sedado.\n\r", ch );		break;
	    case  -5:  send_to_pager( "Tienes suenyo.\n\r", ch );		break;
	    case  -4:  send_to_pager( "Estas cansado.\n\r", ch );		break;
	    case  -3:  send_to_pager( "Podrias descansar.\n\r", ch );		break;
	    case  -2:  send_to_pager( "Te pesan algo los ojos.\n\r", ch );	break;
	    case  -1:  send_to_pager( "Estas bastante bien.\n\r", ch );		break;
	    case   0:  send_to_pager( "Estas muy bien.\n\r", ch );		break;
	    case   1:  send_to_pager( "Te sientes bien.\n\r", ch );	break;
	    case   2:  send_to_pager( "Te sientes poderoso.\n\r", ch );	break;
	    case   3:  send_to_pager( "No puedes pensar claramente.\n\r", ch );	break;
	    case   4:  send_to_pager( "Tu mente va a 100 km/h.\n\r", ch );	break;
	    case   5:  send_to_pager( "Todo te da vueltas.\n\r", ch );	break;
	    case   6:  send_to_pager( "Tu cuerpo y tu mente se estan separando.\n\r", ch );	break;
	    case   7:  send_to_pager( "La realidad se viene abajo.\n\r", ch );	break;
	    case   8:  send_to_pager( "No sabes lo que es real y lo que no.\n\r", ch );	break;
	    case   9:  send_to_pager( "Te sientes inmortal.\n\r", ch );	break;
	    case  10:  send_to_pager( "Eres una Entidad Suprema.\n\r", ch );	break;
	    	}
    else
    if ( ch->mental_state >45 )
	send_to_pager( "Tu dormir esta mezclado con extranyos y reales suenyos.\n\r", ch );
    else
    if ( ch->mental_state >25 )
	send_to_pager( "Dormir no es facil para ti.\n\r", ch );
    else
    if ( ch->mental_state <-35 )
	send_to_pager( "Estas sumido en una enorme necesidad de dormir.\n\r", ch );
    else
    if ( ch->mental_state <-25 )
	send_to_pager( "Estas profundamente dormido.\n\r", ch );
  send_to_pager("Idiomas: ", ch );
    for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
	if ( knows_language( ch, lang_array[iLang], ch )
	||  (IS_NPC(ch) && ch->speaks == 0) )
	{
	    if ( lang_array[iLang] & ch->speaking
	    ||  (IS_NPC(ch) && !ch->speaking) )
		set_pager_color( AT_RED, ch );
	    send_to_pager( lang_names[iLang], ch );
	    send_to_pager( " ", ch );
	    set_pager_color( AT_SCORE, ch );
	}
    send_to_pager( "\n\r", ch );

    if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
	pager_printf_color(ch, "&CSe te han anyadido los comandos: &Y%s\n\r",
		ch->pcdata->bestowments );

    if ( ch->morph && ch->morph->morph )
    {
      send_to_pager_color("&W----------------------------------------------------------------------------&C\n\r", ch);
      if ( IS_IMMORTAL( ch ) )
         pager_printf (ch, "Polimorfado como (%d) %s con un timer de %d.\n\r",
                ch->morph->morph->vnum, ch->morph->morph->short_desc,
		ch->morph->timer
                );
      else
        pager_printf (ch, "Tu estas polimorfado en un %s.\n\r",
                ch->morph->morph->short_desc );
      send_to_pager_color("&W----------------------------------------------------------------------------&C\n\r", ch);
    }
    if ( CAN_PKILL( ch ) )
    {
	send_to_pager_color("&W----------------------------------------------------------------------------&C\n\r", ch);
   pager_printf_color(ch, "&CDATOS DE PK:   Pks (&W%d&C)     Pks ilegales (&W%d&C)    PKdeaths (&W%d&C) Gene&RPk &C(&W%3.3d&C)\n\r",
      ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths, ch->pcdata->genepkills );
    }
    if (ch->level >= LEVEL_AVATAR)
   pager_printf_color(ch, "&CEXPERIENCIA ACUMULADA EN NIVEL %d:   (&W%d&C) \n\r",
   ch->level, ch->exp_acumulada );
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER  && ch->pcdata->clan->clan_type != CLAN_GUILD )
    {

	send_to_pager_color( "&W----------------------------------------------------------------------------&C\n\r", ch);

   pager_printf_color(ch, "&CDATOS DE CLAN: &W%-14.14s &CClan AvPkills : &W%-5d   &CClan NonAvpkills : &W%-5d\n\r",
		ch->pcdata->clan->name, ch->pcdata->clan->pkills[5],
		(ch->pcdata->clan->pkills[0]+ch->pcdata->clan->pkills[1]+
		 ch->pcdata->clan->pkills[2]+ch->pcdata->clan->pkills[3]+
		 ch->pcdata->clan->pkills[4]) );
        pager_printf_color(ch, "&C                            Clan AvPkdeaths: &W%-5d  &CClan NonAvpkdeaths: &W%-5d\n\r",
		ch->pcdata->clan->pdeaths[5],
		( ch->pcdata->clan->pdeaths[0] + ch->pcdata->clan->pdeaths[1] +
		  ch->pcdata->clan->pdeaths[2] + ch->pcdata->clan->pdeaths[3] +
		  ch->pcdata->clan->pdeaths[4] ) );
    }
    if (ch->level >= LEVEL_AVATAR)
  {
   send_to_pager( "----------------------------------------------------------------------------\n\r", ch);
   if (xIS_SET(ch->act, PLR_CAMUFLAJE) )
   pager_printf(ch, "&CCAMUFLAJE: &W%-50.50s &YActivado\n\r", ch->camuflaje);

   if (!xIS_SET(ch->act, PLR_CAMUFLAJE) )
   pager_printf(ch, "&CCAMUFLAJE: &W%-50.50s &gDesactivado\n\r", ch->camuflaje);
  }
    if (ch->pcdata->deity)
    {
	send_to_pager_color( "&W----------------------------------------------------------------------------&C\n\r", ch);
	if (ch->pcdata->favor > 2250)
	  sprintf( buf, "amado" );
	else if (ch->pcdata->favor > 2000)
	  sprintf( buf, "querido" );
	else if (ch->pcdata->favor > 1750)
	  sprintf( buf, "honrado" );
	else if (ch->pcdata->favor > 1500)
	  sprintf( buf, "elogiado" );
	else if (ch->pcdata->favor > 1250)
	  sprintf( buf, "privilegiado" );
	else if (ch->pcdata->favor > 1000)
	  sprintf( buf, "respetado" );
	else if (ch->pcdata->favor > 750)
	  sprintf( buf, "gusta" );
	else if (ch->pcdata->favor > 250)
	  sprintf( buf, "tolerado" );
	else if (ch->pcdata->favor > -250)
	  sprintf( buf, "ignorado" );
	else if (ch->pcdata->favor > -750)
	  sprintf( buf, "esquivado" );
	else if (ch->pcdata->favor > -1000)
	  sprintf( buf, "disgusta" );
	else if (ch->pcdata->favor > -1250)
	  sprintf( buf, "deshonrado" );
	else if (ch->pcdata->favor > -1500)
	  sprintf( buf, "abandonado" );
	else if (ch->pcdata->favor > -1750)
	  sprintf( buf, "olvidado" );
	else if (ch->pcdata->favor > -2000)
	  sprintf( buf, "despreciado" );
	else if (ch->pcdata->favor > -2250)
	  sprintf( buf, "odiado" );
	else
	  sprintf( buf, "maldito" );
	pager_printf_color(ch, "&CDeity:  &W%-20s &CFavor:  &W%s&C\n\r", ch->pcdata->deity->name, buf );
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER )
    {
        send_to_pager_color( "&W----------------------------------------------------------------------------&C\n\r", ch);
	pager_printf_color(ch, "&COrden:  &W%-20s  &COrden Mobkills: &W%-6d   &COrden MobDeaths: &W%-6d\n\r",
		ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
    }
    if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD )
    {
        send_to_pager_color( "&W----------------------------------------------------------------------------&C\n\r", ch);
        pager_printf_color(ch, "&CGremio: &W%-20s  &CGremio Mobkills: &W%-6d   &CGremio MobDeaths: &W%-6d\n\r",
                ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
    }
    argument = one_argument( argument, arg );
    if (ch->first_affect )
    {
	int i;
	SKILLTYPE *sktmp;

	i = 0;
	send_to_pager_color( "&W----------------------------------------------------------------------------&C\n\r", ch);
	send_to_pager_color("AFECTADO POR:                           ", ch);
	for (paf = ch->first_affect; paf; paf = paf->next)
	{
	    if ( (sktmp=get_skilltype(paf->type)) == NULL )
		continue;
	    if (ch->level < 20)
	    {
		pager_printf_color(ch, "&C[&W%-34.34s&C]    ", sktmp->name);
		if (i == 0)
		   i = 2;
		if ((++i % 3) == 0)
		   send_to_pager("\n\r", ch);
	     }
	     if (ch->level >= 20)
	     {
		if (paf->modifier == 0)
		    pager_printf_color(ch, "&C[&W%-24.24s;%5d &Crds]    ",
			sktmp->name,
			paf->duration/100);
		else
		if (paf->modifier > 999)
		    pager_printf_color(ch, "&C[&W%-15.15s; %7.7s;%5d &Crds]    ",
			sktmp->name,
			tiny_affect_loc_name(paf->location),
			paf->duration/100);
		else
		    pager_printf_color(ch, "&C[&W%-11.11s;%+-3.3d %7.7s;%5d &Crds]    ",
			sktmp->name,
			paf->modifier,
			tiny_affect_loc_name(paf->location),
			paf->duration/100);
		if (i == 0)
		    i = 1;
		if ((++i % 2) == 0)
		    send_to_pager("\n\r", ch);
	    }
	}
    }
    send_to_pager("\n\r", ch);
    return;
}

void do_gocial (CHAR_DATA * ch, char *command, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  SOCIALTYPE *social;

  if ((social = find_social (command)) == NULL)
    {
      send_to_char ("Eso no es un social.\n\r", ch);
      return;
    }

  if (!IS_NPC (ch) && xIS_SET (ch->act, PLR_NO_EMOTE))
    {
      send_to_char ("Eres anti-social!\n\r", ch);
      return;
    }

  switch (ch->position)
    {
    case POS_DEAD:
      send_to_char ("Lee esto; tu estas MUERTO.\n\r", ch);
      return;
    case POS_INCAP:
    case POS_MORTAL:
      send_to_char ("Estas demasiado herido para poder hacer eso.\n\r", ch);
      return;
    case POS_STUNNED:
      send_to_char ("Estas demasiado aturdido para hacer eso.\n\r", ch);
      return;
    case POS_SLEEPING:
      /*
         * I just know this is the path to a 12" 'if' statement.  :(
         * But two players asked for it already!  -- Furey
       */
      if (!str_cmp (social->name, "roncar"))
	break;
      send_to_char ("En tus suenyos, o que?\n\r", ch);
      return;
    }

  one_argument (argument, arg);
  victim = NULL;

  if (arg[0] == '\0')
    {
      sprintf (buf, "[GOCIAL] %s", social->others_no_arg);
      act (AT_SOCIAL, buf, ch, NULL, victim, TO_MUD);
      sprintf (buf, "[GOCIAL] %s", social->char_no_arg);
      act (AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("No deberias hablar de gente que no esta conectada.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      sprintf (buf, "[GOCIAL] %s", social->others_auto);
      act (AT_SOCIAL, buf, ch, NULL, victim, TO_MUD);
      sprintf (buf, "[GOCIAL] %s", social->char_auto);
      act (AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
      return;
    }
  else
    {
      sprintf (buf, "[GOCIAL] %s", social->others_found);
      act (AT_SOCIAL, buf, ch, NULL, victim, TO_MUD);
      sprintf (buf, "[GOCIAL] %s", social->char_found);
      act (AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
      sprintf (buf, "[GOCIAL] %s", social->vict_found);
      act (AT_SOCIAL, buf, ch, NULL, victim, TO_VICT);
      return;
    }
}
/*
 * Return ascii name of an affect location.
 */
char           *
tiny_affect_loc_name(int location)
{
	switch (location) {
	case APPLY_NONE:		return "NADA";
	case APPLY_STR:			return " FUE  ";
	case APPLY_DEX:			return " DES  ";
	case APPLY_INT:			return " INT  ";
	case APPLY_WIS:			return " SAB  ";
	case APPLY_CON:			return " CON  ";
	case APPLY_CHA:			return " CAR  ";
	case APPLY_LCK:			return " SUE  ";
	case APPLY_SEX:			return " SEXO  ";
	case APPLY_CLASS:		return " PROF";
	case APPLY_LEVEL:		return " NIVEL ";
	case APPLY_AGE:			return " EDAD ";
	case APPLY_MANA:		return " MANA ";
	case APPLY_HIT:			return " PV   ";
	case APPLY_MOVE:		return " MOV  ";
	case APPLY_GOLD:		return " ORO  ";
	case APPLY_EXP:			return " EXP  ";
	case APPLY_AC:			return " AC   ";
	case APPLY_HITROLL:		return " HITRL";
	case APPLY_DAMROLL:		return " DAMRL";
	case APPLY_SAVING_POISON:	return "SV VEN";
	case APPLY_SAVING_ROD:		return "SV ROD";
	case APPLY_SAVING_PARA:		return "SV PARA";
	case APPLY_SAVING_BREATH:	return "SV ALI";
	case APPLY_SAVING_SPELL:	return "SV SPEL";
	case APPLY_HEIGHT:		return "ALTURA";
	case APPLY_WEIGHT:		return " PESO ";
	case APPLY_AFFECT:		return "AF POR";
	case APPLY_RESISTANT:		return "RESIST";
	case APPLY_IMMUNE:		return "IMMUNE";
	case APPLY_SUSCEPTIBLE:		return "SUSCEPT";
	case APPLY_WEAPONSPELL:		return " ARMA ";
	case APPLY_BACKSTAB:		return "BACKSTB";
	case APPLY_PICK:		return " COGER ";
	case APPLY_TRACK:		return " CAZAR ";
	case APPLY_STEAL:		return " ROBAR ";
	case APPLY_SNEAK:		return " SNEAK ";
	case APPLY_HIDE:		return " HIDE  ";
	case APPLY_PALM:		return " PALM  ";
	case APPLY_DETRAP:		return " DETRAP";
	case APPLY_DODGE:		return " ESQUIV";
	case APPLY_PEEK:		return " PEEK  ";
	case APPLY_SCAN:		return " OTEA  ";
	case APPLY_GOUGE:		return " GOUGE ";
	case APPLY_SEARCH:		return " SEARCH";
	case APPLY_MOUNT:		return " MONT  ";
	case APPLY_DISARM:		return " DISARM";
	case APPLY_KICK:		return " PATA  ";
	case APPLY_PARRY:		return " BLOQ  ";
	case APPLY_BASH:		return " BASH  ";
	case APPLY_STUN:		return " ATUR  ";
	case APPLY_PUNCH:		return " PUNYO ";
	case APPLY_CLIMB:		return " ESCAL ";
	case APPLY_GRIP:		return " GRIP  ";
	case APPLY_SCRIBE:		return " SCRIBE";
	case APPLY_BREW:		return " BREW  ";
	case APPLY_WEARSPELL:		return " LLEVAR";
	case APPLY_REMOVESPELL:		return " QUITA";
	case APPLY_EMOTION:		return "EMOCION";
	case APPLY_MENTALSTATE:		return " MENTAL";
	case APPLY_STRIPSN:		return " DISIPA";
	case APPLY_REMOVE:		return " DESVES";
	case APPLY_DIG:			return " CAVAR ";
	case APPLY_FULL:		return " HAMBRE";
	case APPLY_THIRST:		return " SED ";
	case APPLY_DRUNK:		return " BORRACHO";
	case APPLY_BLOOD:		return " SANGRE";
	case APPLY_COOK:		return " COCIN ";
	case APPLY_RECURRINGSPELL:	return " RECURR";
	case APPLY_CONTAGIOUS:		return "CONTAGI";
	case APPLY_ODOR:		return " ODOR  ";
	case APPLY_ROOMFLAG:		return " RMFLG ";
	case APPLY_SECTORTYPE:		return " SECTOR";
	case APPLY_ROOMLIGHT:		return "  LUZ  ";
	case APPLY_TELEVNUM:		return " TELEVN";
	case APPLY_TELEDELAY:		return " TELEDY";
	};

	bug("Affect_location_name: unknown location %d.", location);
	return "(???)";
}

char *
get_class(CHAR_DATA *ch)
{
    if ( IS_NPC(ch) && ch->class < MAX_NPC_CLASS && ch->class >= 0)
    	return ( npc_class[ch->class] );
    else if ( !IS_NPC(ch) && ch->class < 40 && ch->class >= 0 )
        return class_table[ch->class]->who_name;
    return ("Desconocida");
}


char *
get_race( CHAR_DATA *ch)
{
    if(  ch->race < MAX_PC_RACE  && ch->race >= 0)
        return (race_table[ch->race]->race_name);
    if ( ch->race < MAX_NPC_RACE && ch->race >= 0)
	return ( npc_race[ch->race] );
    return ("Desconocida");
}

void do_oldscore( CHAR_DATA *ch, char *argument )
{
    AFFECT_DATA *paf;
    SKILLTYPE   *skill;

    set_pager_color( AT_SCORE, ch );
    pager_printf( ch,
	"Eres %s%s, nivel %d, %d anyos(%d horas).\n\r",
	ch->name,
	IS_NPC(ch) ? "" : ch->pcdata->title,
	ch->level,
	get_age(ch),
	(get_age(ch) - 17) * 2 );

    if ( get_trust( ch ) != ch->level )
	pager_printf( ch, "Tienes un trust a nivel %d.\n\r",
	    get_trust( ch ) );

    if (  IS_NPC(ch) && xIS_SET(ch->act, ACT_MOBINVIS) )
      pager_printf( ch, "Eres mobinvis a nivel %d.\n\r",
            ch->mobinvis);

    if ( !IS_NPC(ch) && IS_VAMPIRE(ch) )
      pager_printf( ch,
	"Tienes %d/%d puntos de vida, %d/%d nivel de sangre, %d/%d movimiento, %d practicas.\n\r",
	ch->hit,  ch->max_hit,
	ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level + (13 / ch->generacion ) * 40,
	ch->move, ch->max_move,
	ch->practice );
    else
      pager_printf( ch,
	"Tienes %d/%d puntos de vida, %d/%d mana, %d/%d movimiento, %d practicas.\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->practice );

    pager_printf( ch,
	"Estas llevando %d/%d objetos con un peso de %d/%d kg.\n\r",
	ch->carry_number, can_carry_n(ch),
	ch->carry_weight, can_carry_w(ch) );

    pager_printf( ch,
	"Fue: %d  Int: %d  Sab: %d  Des: %d  Con: %d  Car: %d  Sue: %d.\n\r",
	get_curr_str(ch),
	get_curr_int(ch),
	get_curr_wis(ch),
	get_curr_dex(ch),
	get_curr_con(ch),
	get_curr_cha(ch),
	get_curr_lck(ch) );

    pager_printf( ch,
	"Has conseguido %s puntos de experiencia, y tienes %s monedas de oro.\n\r",
	num_punct(ch->exp), num_punct(ch->gold) );

    if ( !IS_NPC(ch) )
    pager_printf( ch,
	"Has logrado %d puntos de gloria durante tu vida, y ahora tienes %d.\n\r",
	ch->pcdata->quest_accum, ch->pcdata->quest_curr );

    pager_printf( ch,
	"Autoexit: %s   Autoloot: %s   Autosac: %s   Autogold: %s\n\r",
	(!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
	(!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
	(!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOSAC) ) ? "yes" : "no",
  	(!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOGOLD)) ? "yes" : "no" );

    pager_printf( ch, "Wimpy fijado a %d puntos de vida.\n\r", ch->wimpy );

    if ( !IS_NPC(ch) ) {
       if ( ch->pcdata->condition[COND_DRUNK]   > 10 )
	   send_to_pager( "Estas borracho.\n\r",   ch );
       if ( ch->pcdata->condition[COND_THIRST] ==  0 )
	   send_to_pager( "Estas sediento.\n\r", ch );
       if ( ch->pcdata->condition[COND_FULL]   ==  0 )
	   send_to_pager( "Estas hambriento.\n\r",  ch );
    }

    switch( ch->mental_state / 10 )
    {
          default:   send_to_pager( "Estas completamente ido!\n\r", ch );	break;
	    case -10:  send_to_pager( "Estas perdiendo la consciencia.\n\r", ch );	break;
	    case  -9:  send_to_pager( "No puedes mantener tus ojos abiertos.\n\r", ch );	break;
	    case  -8:  send_to_pager( "Tienes MUCHO suenyo.\n\r", ch );	break;
	    case  -7:  send_to_pager( "Estas muy desmotivado.\n\r", ch );	break;
	    case  -6:  send_to_pager( "Te sientes sedado.\n\r", ch );		break;
	    case  -5:  send_to_pager( "Tienes suenyo.\n\r", ch );		break;
	    case  -4:  send_to_pager( "Estas cansado.\n\r", ch );		break;
	    case  -3:  send_to_pager( "Podrias descansar.\n\r", ch );		break;
	    case  -2:  send_to_pager( "Te pesan algo los ojos.\n\r", ch );	break;
	    case  -1:  send_to_pager( "Estas bastante bien.\n\r", ch );		break;
	    case   0:  send_to_pager( "Estas muy bien.\n\r", ch );		break;
	    case   1:  send_to_pager( "Te sientes bien.\n\r", ch );	break;
	    case   2:  send_to_pager( "Te sientes poderoso.\n\r", ch );	break;
	    case   3:  send_to_pager( "No puedes pensar claramente.\n\r", ch );	break;
	    case   4:  send_to_pager( "Tu mente va a 100 km/h.\n\r", ch );	break;
	    case   5:  send_to_pager( "Todo te da vueltas.\n\r", ch );	break;
	    case   6:  send_to_pager( "Tu cuerpo y tu mente se estan separando.\n\r", ch );	break;
	    case   7:  send_to_pager( "La realidad se viene abajo.\n\r", ch );	break;
	    case   8:  send_to_pager( "No sabes lo que es real y lo que no.\n\r", ch );	break;
	    case   9:  send_to_pager( "Te sientes inmortal.\n\r", ch );	break;
	    case  10:  send_to_pager( "Eres una Entidad Suprema.\n\r", ch );	break;
    }

    switch ( ch->position )
    {
    case POS_DEAD:
	send_to_pager( "Estas MUERTO!!\n\r",		ch );
	break;
    case POS_MORTAL:
	send_to_pager( "Estas mortalmente herido.\n\r",	ch );
	break;
    case POS_INCAP:
	send_to_pager( "Estas incapacitado.\n\r",	ch );
	break;
    case POS_STUNNED:
	send_to_pager( "Estas aturdido.\n\r",		ch );
	break;
    case POS_SLEEPING:
	send_to_pager( "Estas durmiendo.\n\r",		ch );
	break;
    case POS_RESTING:
	send_to_pager( "Estas descansando.\n\r",		ch );
	break;
    case POS_STANDING:
	send_to_pager( "Estas de pie.\n\r",		ch );
	break;
    case POS_FIGHTING:
	send_to_pager( "Estas luchando.\n\r",		ch );
	break;
    case POS_MOUNTED:
	send_to_pager( "Montado.\n\r",			ch );
	break;
    case POS_SHOVE:
	send_to_pager( "Siendo empujado.\n\r",		ch );
	break;
    case POS_DRAG:
	send_to_pager( "Siendo arrastrado.\n\r",		ch );
	break;
    }

    if ( ch->level >= 25 )
	pager_printf( ch, "AC: %d.  ", GET_AC(ch) );

    send_to_pager( "Estas ", ch );
	 if ( GET_AC(ch) >=  101 ) send_to_pager( "PEOR que desnudo!\n\r", ch );
    else if ( GET_AC(ch) >=   80 ) send_to_pager( "desnudo.\n\r",            ch );
    else if ( GET_AC(ch) >=   60 ) send_to_pager( "vestido.\n\r",  ch );
    else if ( GET_AC(ch) >=   30 ) send_to_pager( "ligeramente protegido.\n\r", ch );
    else if ( GET_AC(ch) >=   20 ) send_to_pager( "algo protegido.\n\r", ch );
    else if ( GET_AC(ch) >=    0 ) send_to_pager( "protegido.\n\r",          ch );
    else if ( GET_AC(ch) >= - 10 ) send_to_pager( "bien protegido.\n\r",     ch );
    else if ( GET_AC(ch) >= - 30 ) send_to_pager( "bastante bien protegido.\n\r", ch );
    else if ( GET_AC(ch) >= - 60 ) send_to_pager( "muy bien protegido.\n\r",  ch );
    else if ( GET_AC(ch) >= - 90 ) send_to_pager( "protegido como un avatar!\n\r", ch );
    else if ( GET_AC(ch) >= -120 ) send_to_pager( "divinamente protegido.\n\r",
ch );
    else                           send_to_pager( "INVENCIBLE!\n\r",       ch );

    if ( ch->level >= 15
    ||   IS_PKILL( ch ) )
	pager_printf( ch, "Hitroll: %d  Damroll: %d.\n\r",
	    GET_HITROLL(ch), GET_DAMROLL(ch) );

    if ( ch->level >= 10 )
	pager_printf( ch, "Alineamiento: %d.  ", ch->alignment );

    send_to_pager( "Eres ", ch );
	 if ( ch->alignment >  900 ) send_to_pager( "angelico.\n\r", ch );
    else if ( ch->alignment >  700 ) send_to_pager( "un santo.\n\r", ch );
    else if ( ch->alignment >  350 ) send_to_pager( "bueno.\n\r",    ch );
    else if ( ch->alignment >  100 ) send_to_pager( "amable.\n\r",    ch );
    else if ( ch->alignment > -100 ) send_to_pager( "neutral.\n\r", ch );
    else if ( ch->alignment > -350 ) send_to_pager( "mezquino.\n\r",    ch );
    else if ( ch->alignment > -700 ) send_to_pager( "malvado.\n\r",    ch );
    else if ( ch->alignment > -900 ) send_to_pager( "demoniaco.\n\r", ch );
    else                             send_to_pager( "satanico.\n\r", ch );

    if ( ch->first_affect )
    {
	send_to_pager( "Estas afectado por:\n\r", ch );
	for ( paf = ch->first_affect; paf; paf = paf->next )
	    if ( (skill=get_skilltype(paf->type)) != NULL )
	{
	    pager_printf( ch, "Hechizo: '%s'", skill->name );

	    if ( ch->level >= 20 )
		pager_printf( ch,
		    " modifica %s en %d durante %d horas",
		    affect_loc_name( paf->location ),
		    paf->modifier,
		    paf->duration/100 );/*Kayser 2004*/

	    send_to_pager( ".\n\r", ch );
	}
    }

    if ( !IS_NPC( ch ) && IS_IMMORTAL( ch ) )
    {
	pager_printf( ch, "\n\rNivel Wizinvis nivel: %d   WizInvis esta %s\n\r",
			ch->pcdata->wizinvis,
			xIS_SET(ch->act, PLR_WIZINVIS) ? "ON" : "OFF" );
	if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	  pager_printf( ch, "Rango de la room: %d - %d\n\r", ch->pcdata->r_range_lo,
					 	   ch->pcdata->r_range_hi	);
	if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	  pager_printf( ch, "Rango del objeto : %d - %d\n\r", ch->pcdata->o_range_lo,
	  					   ch->pcdata->o_range_hi	);
	if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	  pager_printf( ch, "Rango del Mob : %d - %d\n\r", ch->pcdata->m_range_lo,
	  					   ch->pcdata->m_range_hi	);
    }

    return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int x, lowlvl, hilvl;

    if ( ch->level == 1 )
      lowlvl = 1;
    else
      lowlvl = UMAX( 2, ch->level - 5 );
    hilvl = URANGE( ch->level, ch->level + 5, MAX_LEVEL );
    ch_printf( ch, "\n\r&wExperiencia requirda, niveles %d a %d:\n\r&g______________________________________________\n\r\n\r", lowlvl, hilvl );
    sprintf( buf, " &wexp  &g(&wActualmente: %12s&g)", num_punct(ch->exp) );
    sprintf( buf2," &wexp  &g(&wNecesitados:  %12s&g)", num_punct( exp_level(ch, ch->level+1) - ch->exp) );
    for ( x = lowlvl; x <= hilvl; x++ )
	ch_printf( ch, " &g(&w%2d&g)&w %12s%s\n\r", x, num_punct( exp_level( ch, x ) ),
		(x == ch->level) ? buf : (x == ch->level+1) ? buf2 : " exp" );
    send_to_char( "&g______________________________________________\n\r", ch );
}

/* 1997, Blodkai */
void do_remains( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    bool found = FALSE;

    if ( IS_NPC( ch ) )
      return;
    set_char_color( AT_MAGIC, ch );
    if ( !ch->pcdata->deity ) {
      send_to_pager( "No tienes ninguna deidad para que pueda ayudarte...\n\r", ch );
      return;
    }
    if ( ch->pcdata->favor < ch->level*2 ) {
      send_to_pager( "No tienes suficientes puntos de favor...\n\r", ch );
      return;
    }
    pager_printf( ch, "%s aparece de repente, revelando lo que has pedido... ", ch->pcdata->deity->name );
    sprintf( buf, "el cuerpo de %s", ch->name );
    for ( obj = first_object; obj; obj = obj->next ) {
      if ( obj->in_room && !str_cmp( buf, obj->short_descr )
      && ( obj->pIndexData->vnum == 11 ) ) {
        found = TRUE;
        pager_printf( ch, "\n\r  - a %s le servira durante %d ticks",
          obj->in_room->name,
          obj->timer );
      }
    }
    if ( !found )
      send_to_pager( " no existe.\n\r", ch );
    else
    {
      send_to_pager( "\n\r", ch );
      ch->pcdata->favor -= ch->level*2;
    }
    return;
}

/* Affects-at-a-glance, Blodkai */
void do_affected ( CHAR_DATA *ch, char *argument )
{
    char arg [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    AFFECT_DATA *paf;
    SKILLTYPE *skill;
    DISCI_AFFECT *daf;
    NIVEL *nivel;

    if ( IS_NPC(ch) )
	return;


    argument = one_argument( argument, arg );
    if ( !str_cmp( arg, "por" ) )
    {
        send_to_char_color( "\n\r&BAfectado por:\n\r", ch );
	ch_printf_color( ch, "&c%s\n\r",
	  !xIS_EMPTY(ch->affected_by) ? affect_bit_name( &ch->affected_by ) : "nada" );
        if ( ch->level >= 20 )
        {
            send_to_char( "\n\r", ch );
            if ( ch->resistant > 0 )
	    {
                send_to_char_color( "&BResistencias:  ", ch );
                ch_printf_color( ch, "&C%s\n\r", flag_string(ch->resistant, ris_flags) );
	    }
            if ( ch->immune > 0 )
	    {
                send_to_char_color( "&BInmunidades:   ", ch);
                ch_printf_color( ch, "&C%s\n\r",  flag_string(ch->immune, ris_flags) );
	    }
            if ( ch->susceptible > 0 )
	    {
                send_to_char_color( "&BSusceptible:     ", ch );
                ch_printf_color( ch, "&C%s\n\r", flag_string(ch->susceptible, ris_flags) );
	    }
        }
	return;
    }

    if( ES_AFECTADO(ch))
        {
        send_to_char( "\n\r&BDisciplinas:  \n\r", ch );
        if( xIS_SET(ch->afectado_por, DAF_POTENCIA))
        {
        	nivel = get_nivel( "potencia" );
        	for( daf = ch->primer_affect; daf; daf = daf->next )
        	if( nivel->dnivel == daf->nivel)
        	ch_printf( ch, "&cPotencia     : &wnivel &g%d&w afecta &g%s &wen &c%d &wdurante &c%d &wHoras.\n\r",
        	ch->pcdata->habilidades[DISCIPLINA_POTENCIA],
		daffect_loc_name(daf->localizacion), daf->modificador, daf->duracion/100);
        }

        if( xIS_SET(ch->afectado_por, DAF_KOMA ))
        {
                nivel = get_nivel( "koma" );
                for( daf = ch->primer_affect; daf; daf = daf->next )
                if( nivel->dnivel == daf->nivel)
                ch_printf( ch, "&cKoma         : &wReduce el &g%s &wen &c%d &wdurante &c%d &wHoras.\n\r",
                daffect_loc_name(daf->localizacion), daf->modificador, daf->duracion/100 );
        }

        if( xIS_SET(ch->afectado_por, DAF_CELERIDAD))
        {
        nivel = get_nivel( "celeridad" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel)
        {
        if(ch->pcdata->habilidades[DISCIPLINA_CELERIDAD] <= 5)
        ch_printf( ch, "&cCeleridad    : &wnivel &g%d&w reduce el &gWait &wen un &c%d0% &wdurante &c%d &wHoras.\n\r",
        ch->pcdata->habilidades[DISCIPLINA_CELERIDAD],ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], daf->duracion/100 );
        else
        ch_printf( ch, "&cCeleridad    : &wnivel &g%d&w reduce el &gWait &wen un &c50% &wdurante &c%d &wHoras.\n\r"
        "&cCeleridad    : &wTe otorga dos cast en ciertos conjuros y niveles de disciplina.\n\r",
        ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], daf->duracion/100);
        }
        }

	if( xIS_SET(ch->afectado_por, DAF_MASCARA))
        {
        	nivel = get_nivel( "mascara" );
        	for( daf = ch->primer_affect; daf; daf = daf->next )
        	if( nivel->dnivel == daf->nivel)
        	ch_printf( ch, "&cMortis    : &wnivel &g1&w afecta &g%s &wen &c%d &wdurante &c%d &wHoras.\n\r",
        	daffect_loc_name(daf->localizacion), daf->modificador,
		daf->duracion/100);
        }
        
 	if( xIS_SET(ch->afectado_por, DAF_CAPA_SOMBRIA))
        {
        	nivel = get_nivel( "capa" );
        	for( daf = ch->primer_affect; daf; daf = daf->next )
        	if( nivel->dnivel == daf->nivel)
		ch_printf( ch, "&cOfuscacion   : &wnivel &g1&w Crea estado &gCapa Sombria&w que te oculta durante &c%d&w Horas.\n\r",
         	daf->duracion/100 );
        }


        if( xIS_SET(ch->afectado_por, DAF_FORTALEZA))
        {
        nivel = get_nivel( "fortaleza" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel)
        ch_printf( ch, "&cFortaleza    : &wnivel &g%d&w afecta &g%s &wen &c%d &wdurante &c%d &wHoras.\n\r",
        ch->pcdata->habilidades[DISCIPLINA_FORTALEZA],daffect_loc_name(daf->localizacion), 
	daf->modificador, daf->duracion/100);
        }

        if( xIS_SET( ch->afectado_por, DAF_CORAZON_PETREO ) )
        {
        nivel = get_nivel( "corazon" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
        ch_printf( ch, "&cTaumaturgia  : &wnivel &g%d&w Te da &gInmunidad a los empalamientos&w durante &c%d &wHoras.\n\r",
        daf->nivel-2900, daf->duracion/100 );
        }

	if( xIS_SET(ch->afectado_por, DAF_TRAUMATIZADO))
        {
        nivel = get_nivel( "pasion" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel)
        ch_printf( ch, "&cDementacion  : &wnivel &g%d&w Traumatiza y afecta &g%s &wen &c%d &wdurante &c%d &wHoras.\n\r",
        daf->nivel - 700,daffect_loc_name(daf->localizacion),
        daf->modificador, daf->duracion/100);
        }

	if( xIS_SET(ch->afectado_por, DAF_MIRADA))
        {
        nivel = get_nivel( "mirada" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel)
        ch_printf( ch, "&cPresencia    : &wnivel &g%d&w afecta &g%s &wen &c%d &wdurante &c%d &wHoras.\n\r",
        daf->nivel - 2000,daffect_loc_name(daf->localizacion),
        daf->modificador, daf->duracion/100);
        }

	if( xIS_SET(ch->afectado_por, DAF_SILENCIO_MORTAL ))
        {
        nivel = get_dnivel( 901 );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
	ch_printf( ch, "&cExtincion    : &wnivel &g1&w Crea estado &gSilencio de la muerte&w durante &c%d&w Horas.\n\r",
	daf->duracion/100);
	}
        if( xIS_SET(ch->afectado_por, DAF_DEBILIDAD))
        {
        nivel = get_nivel( "debilidad" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
        ch_printf( ch, "&cExtincion    : &wnivel &g2&w &wDebilidad &gReduce tu resistencia&w al danyo durante &c%d &wHoras.\n\r",
        daf->duracion/100 );
	}
	if( xIS_SET(ch->afectado_por, DAF_ENFERMEDAD))
        {
        nivel = get_nivel( "enfermedad" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
        ch_printf( ch, "&cExtincion    : &wnivel &g3 &wHas &gEnfermado&w durante &c%d &wHoras.\n\r",
        daf->duracion/100 );
        }
	if( xIS_SET(ch->afectado_por, DAF_SENTIDOS_AGUZADOS))
	{
	nivel = get_nivel( "sentidos" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
	ch_printf( ch, "&cAuspex       : &wnivel &g1&w Te permite &gVer lo no visible&w durante &c%d &wHoras.\n\r",
	daf->duracion/100 );
        }
	if( xIS_SET(ch->afectado_por, DAF_TESTIGO))
	{
	nivel = get_nivel( "testigo" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
        ch_printf( ch, "&cProtean      : &wnivel &g1&w Te permite &gVer en la oscuridad absoluta&w durante &c%d&w Horas.\n\r", 
	daf->duracion/100 );
        }
	
	if( xIS_SET(ch->afectado_por, DAF_INCORPOREO))
        {
        nivel = get_nivel( "forma" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
        ch_printf( ch, "&cTaumaturgia  : &wnivel &g%d&w Eres &gIncorporeo&w durante &c%d&w Horas.\n\r",
        daf->nivel-2900,daf->duracion/100 );
        }
	
	 if( xIS_SET(ch->afectado_por, DAF_EMBRUJADO))
        {
        nivel = get_nivel( "embruja" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
        ch_printf( ch, "&cDementacion  : &wnivel &g%d&w estas &gEmbrujado &wdurante &c%d &wHoras.\n\r",
        daf->nivel-700,daf->duracion/100 );
        }


	if( xIS_SET(ch->afectado_por, DAF_FASCINACION))
        {
        nivel = get_nivel( "fascinacion" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        if( nivel->dnivel == daf->nivel )
	ch_printf( ch, "&cPresencia    : &wnivel &g%d&w Fascinacion afecta &g%s &wen &c%d &wdurante &c%d &wHoras.\n\r",
        daf->nivel-2000,daffect_loc_name(daf->localizacion), daf->modificador, daf->duracion/100 );
        }

	if( xIS_SET(ch->afectado_por, DAF_GARRAS_BESTIA))
        {
        nivel = get_nivel( "garras" );
        for( daf = ch->primer_affect; daf; daf = daf->next )
        	if( nivel->dnivel == daf->nivel )
		{
                        ch_printf( ch, "&cProtean      : &wnivel &g%d&w Produces &gDanyos Agravados&w durante &c%d&w Horas.\n\r",
                        daf->nivel-2200,daf->duracion/100 );
        		ch_printf( ch, "&cProtean      : &wnivel &g%d&w Tus manos se transforman en Garras durante &c%d&w Horas.\n\r",
        		daf->nivel-2200,daf->duracion/100 );
        	}
	}

        if( xIS_SET(ch->act, PLR_DOMINADO))
        ch_printf( ch, "&cAnimalismo   : &wnivel &g1&w Pierdes parte del control de tu personaje y eres dominado por otro durante &c%d&w Horas.\n\r",
        ch->pcdata->cnt_dominacion );

    /*if( ch->primer_affect )
    {
    send_to_char( "\n\r", ch );
    send_to_char( "&BDisciplinas:  \n\r", ch );
    for(daf = ch->primer_affect; daf; daf = daf->next)
    if(( nivel=get_dnivel( daf->nivel )) != NULL )
    {
            if (daf->duracion >= 25 )
            sprintf(buf, "&c");
            if (daf->duracion <= 24 )
            sprintf(buf, "&r");
            if (daf->duracion <= 6)
            sprintf(buf, "&W");



            if (daf->modificador == 0)
            ch_printf( ch, "     &c%-18s &b: &wModifica &g%s&w durante %s%d &whoras.\n\r", nivel->nombre,daffect_loc_name(daf->localizacion),buf, daf->duracion/100);
            if (paf->modifier != 0)
            ch_printf( ch, "     &c%-18s &b: &wModifica &g%s&w en &c%d&w
durante %s%d &whoras.\n\r", nivel->nombre,daffect_loc_name(daf->localizacion), daf->modificador, buf,daf->duracion/100);
    }
    } */
   }
    if ( !ch->first_affect )
    {
        send_to_char_color( "\n\r&cNo estas afectado por nada.\n\r", ch );
    }
    else
    {
	send_to_char( "\n\r", ch );
        send_to_char( "&BAfectado:  \n\r", ch );
        for (paf = ch->first_affect; paf; paf = paf->next)
        if ((skill=get_skilltype(paf->type)) != NULL )
        {
            if (paf->duration >= 25 )
            sprintf(buf, "&c");
            if (paf->duration <= 24 )
            sprintf(buf, "&r");
            if (paf->duration <= 6)
            sprintf(buf, "&W");


            if( ch->level <= 25 )
            {            
            if (paf->modifier == 0)
            ch_printf( ch, "     &c%-18s &b: ?????\n\r", skill->name);
            if (paf->modifier != 0)
            ch_printf( ch, "     &c%-18s &b: ?????\n\r", skill->name);
            }
            else
            { 
            if (paf->modifier == 0)
            ch_printf( ch, "     &c%-18s &b: &wModifica &g%s&w durante %s%d &whoras.\n\r", skill->name, affect_loc_name(paf->location), buf,paf->duration/100);
            if (paf->modifier != 0)
            ch_printf( ch, "     &c%-18s &b: &wModifica &g%s&w en &c%d&w durante %s%d &whoras.\n\r", skill->name,affect_loc_name(paf->location),paf->modifier, buf, paf->duration/100);/*Kayser 2004*/
           }
        }
    }
    return;
}

/*  for (paf = ch->first_affect; paf; paf = paf->next)
	{
	    if ( (sktmp=get_skilltype(paf->type)) == NULL )
		continue;
	    if (ch->level < 20)
	    {
		pager_printf_color(ch, "&C[&W%-34.34s&C]    ", sktmp->name);
		if (i == 0)
		   i = 2;
		if ((++i % 3) == 0)
		   send_to_pager("\n\r", ch);
	     }
	     if (ch->level >= 20)
	     {
		if (paf->modifier == 0)
		    pager_printf_color(ch, "&C[&W%-24.24s;%5d &Crds]    ",
			sktmp->name,
			paf->duration/100);
		else
		if (paf->modifier > 999)
		    pager_printf_color(ch, "&C[&W%-15.15s; %7.7s;%5d &Crds]    ",
			sktmp->name,
			tiny_affect_loc_name(paf->location),
			paf->duration/100);
		else
		    pager_printf_color(ch, "&C[&W%-11.11s;%+-3.3d %7.7s;%5d &Crds]    ",
			sktmp->name,
			paf->modifier,
			tiny_affect_loc_name(paf->location),
			paf->duration/100);
		if (i == 0)
		    i = 1;
		if ((++i % 2) == 0)
		    send_to_pager("\n\r", ch);
	    }
	}
    }
*/

void do_inventory( CHAR_DATA *ch, char *argument )
{
    set_char_color( AT_RED, ch );
    send_to_char( "Estas llevando:\n\r", ch );
    show_list_to_char( ch->first_carrying, ch, TRUE, TRUE );
    return;
}


void do_equipment( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    int iWear;
    bool found;
    int start,cond;

    set_char_color( AT_RED, ch );
    send_to_char( "Estas usando:\n\r", ch );
    found = FALSE;
    set_char_color( AT_OBJECT, ch );
    for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
    {
	for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	   if ( obj->wear_loc == iWear )
	   {
                if( (!IS_NPC(ch)) && (ch->race>0) && (ch->race<MAX_PC_RACE))
                    send_to_char(race_table[ch->race]->where_name[iWear], ch);
                else
                    send_to_char( where_name[iWear], ch );

		if ( can_see_obj( ch, obj ) )
		{
			/* Kayser para estado del equipo */
			switch(obj->item_type)
			{
				case ITEM_LIGHT:
					cond = 10;
				break;
				case ITEM_ARMOR:
					cond = (int) ((10 * obj->value[0] / obj->value[1]) );
				break;
				case ITEM_WEAPON:
					cond = (int) ((10* obj->value[0] / 12) );
				break;
				default:
					cond = 1;
				break;

			}
			send_to_char("&c<&w",ch);
			if (cond >= 0)
			{
				for (start = 1;start <= 10; start++)
				{
					if (start <= cond)
						send_to_char("+",ch);
					else
						send_to_char(" ",ch);
				}
			}
			send_to_char("&c>&w ",ch);
		    	send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
		    	send_to_char( "\n\r", ch );
		}
		else
		    send_to_char( "algo.\n\r", ch );
		found = TRUE;
	   }
    }

    if ( !found )
	send_to_char( "Nada.\n\r", ch );

    return;
}



void set_title( CHAR_DATA *ch, char *title )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
    {
	bug( "Set_title: NPC.", 0 );
	return;
    }

    if ( isalpha(title[0]) || isdigit(title[0]) )
    {
	buf[0] = ' ';
	strcpy( buf+1, title );
    }
    else
	strcpy( buf, title );

    STRFREE( ch->pcdata->title );
    ch->pcdata->title = STRALLOC( buf );
    return;
}



void do_title( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
	return;

    set_char_color( AT_SCORE, ch );
    if ( ch->level < 5 )
    {
	send_to_char( "debes ser al menos nivel 5 para cambiarte el titulo...\n\r", ch );
	return;
    }
    if ( IS_SET( ch->pcdata->flags, PCFLAG_NOTITLE ))
    {
	set_char_color( AT_IMMORT, ch );
        send_to_char_color( "Los dioses te prohiben cambiarte el titulo.\n\r", ch );
        return;
    }

    if ( argument[0] == '\0' )
    {
   send_to_char_color( "Cambiar tu titulo a que?\n\r", ch );
	return;
    }

    if ( strlen(argument) > 50 )
	argument[50] = '\0';

    smash_tilde( argument );
    set_title( ch, argument );
    send_to_char_color( "Ok.\n\r", ch );
}


/*void do_homepage( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if ( IS_NPC(ch) )
	return;

    if ( ch->level < 5 )
    {
	send_to_char( "Debes ser al menos nivel 5 para poder hacer eso./n/r", ch );
	return;
    }

    if ( argument[0] == '\0' )
    {
	if ( !ch->pcdata->homepage )
	  ch->pcdata->homepage = str_dup( "" );
	ch_printf( ch, "Tu homepage es: %s\n\r",
		show_tilde( ch->pcdata->homepage ) );
	return;
    }

    if ( !str_cmp( argument, "clear" ) )
    {
	if ( ch->pcdata->homepage )
	  DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup("");
	send_to_char( "Homepage vaciada.\n\r", ch );
	return;
    }

    if ( strstr( argument, "://" ) )
	strcpy( buf, argument );
    else
	sprintf( buf, "http://%s", argument );
    if ( strlen(buf) > 70 )
	buf[70] = '\0';

    hide_tilde( buf );
    if ( ch->pcdata->homepage )
      DISPOSE(ch->pcdata->homepage);
    ch->pcdata->homepage = str_dup(buf);
    send_to_char( "Ok.\n\r", ch );
} */



/*
 * Set your personal description				-Thoric
 */
void do_description( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Los mobs son demasiado tontos para hacer eso!\n\r", ch );
	return;
    }

    if ( !ch->desc )
    {
	bug( "do_description: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_description: illegal substate", 0 );
	   return;

	case SUB_RESTRICTED:
	   send_to_char( "No puedes.\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_DESC;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->description );
	   return;

	case SUB_PERSONAL_DESC:
	   STRFREE( ch->description );
	   ch->description = copy_buffer( ch );
	   stop_editing( ch );
	   return;
    }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) )
    {
	send_to_char( "Son Mobs, no pueden.\n\r", ch );
	return;
    }
    if ( ch->level < 5 )
    {
	set_char_color( AT_SCORE, ch );
	send_to_char( "Debes ser al menos nivel 5 para hacer eso...\n\r", ch );
	return;
    }
    if ( !ch->desc )
    {
	bug( "do_bio: no descriptor", 0 );
	return;
    }

    switch( ch->substate )
    {
	default:
	   bug( "do_bio: illegal substate", 0 );
	   return;

	case SUB_RESTRICTED:
	   send_to_char( "No puedes.\n\r", ch );
	   return;

	case SUB_NONE:
	   ch->substate = SUB_PERSONAL_BIO;
	   ch->dest_buf = ch;
	   start_editing( ch, ch->pcdata->bio );
	   return;

	case SUB_PERSONAL_BIO:
	   STRFREE( ch->pcdata->bio );
	   ch->pcdata->bio = copy_buffer( ch );
	   stop_editing( ch );
	   return;
    }
}



/*
 * New stat and statreport command coded by Morphina
 * Bug fixes by Shaddai
 */

void do_statreport( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    if ( IS_NPC(ch) )
    {
	send_to_char("Huh?\n\r", ch );
	return;
    }

    if ( IS_VAMPIRE(ch) )
    {
      ch_printf( ch, "&gReportas: &w%d&g/&w%d &gpv &w%d&g/&w%d &gsangre &w%d&g/&w%d &gmv &w%d &gpx.&g\n\r",
 		ch->hit,  ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
		 10 + ch->level + (13 / ch->generacion) * 40, ch->move, ch->max_move, ch->exp   );
      sprintf( buf, "&g$n reporta: &w%d&g/&w%d &gpv &w%d&g/&w%d &gsangre &w%d&g/&w%d &gmv &w%d &gpx.&g",
 		ch->hit,  ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
		 10 + ch->level + (13 / ch->generacion) * 40, ch->move, ch->max_move, ch->exp   );
      act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
    }
    else
    {
      ch_printf( ch, "&gReportas: &w%d&g/&w%d &gpv &w%d&g/&w%d &gmana &w%d&g/&w%d &gmv &w%d &gpx.&g\n\r",
 		ch->hit,  ch->max_hit, ch->mana, ch->max_mana,
 		ch->move, ch->max_move, ch->exp   );
      sprintf( buf, "&g$n reporta &w%d&g/&w%d &gpv &w%d&g/&w%d &gmana &w%d&g/&w%d &gmv &w%d &gpx.&g",
 		ch->hit,  ch->max_hit, ch->mana, ch->max_mana,
 		ch->move, ch->max_move, ch->exp   );
      act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
    }
    return;
}

void do_stat( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
    {
	send_to_char("Huh?\n\r", ch );
	return;
    }

    if ( IS_VAMPIRE(ch) )
      ch_printf( ch, "Reportas: %d/%d pv %d/%d sangre %d/%d mv %d px.\n\r",
 		ch->hit,  ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST],
		 10 + ch->level + (13 / ch->generacion) * 40, ch->move, ch->max_move, ch->exp   );
    else
      ch_printf( ch, "Reportas: %d/%d pv %d/%d mana %d/%d mv %d px.\n\r",
 		ch->hit,  ch->max_hit, ch->mana, ch->max_mana,
 		ch->move, ch->max_move, ch->exp   );
          return;
}


void do_report( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];

    if ( IS_NPC( ch ) && ch->fighting )
	return;

    if ( IS_AFFECTED(ch, AFF_POSSESS) )
    {
       send_to_char("No puedes hacer eso en tu estado actual!\n\r", ch);
       return;
    }


    if ( IS_VAMPIRE(ch) )
      ch_printf( ch,
	"&gReportas: &w%d&g/&w%d &gpv &w%d&g/&w%d &gsangre &w%d&g/&w%d &gmv &w%d &gpx.&g\n\r",
	ch->hit,  ch->max_hit,
	ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level + (13 / ch->generacion) * 40,
	ch->move, ch->max_move,
	ch->exp   );
    else
      ch_printf( ch,
	"&gReportas: &w%d&g/&w%d &gpv &w%d&g/&w%d &gmana &w%d&g/&w%d &gmv &w%d &gpx.&g\n\r",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    if ( IS_VAMPIRE(ch) )
      sprintf( buf, "&g$n reporta: &w%d&g/&w%d &gpv &w%d&g/&w%d &gsangre &w%d&g/&w%d &gmv &w%d &gpx.&g.\n\r",
	ch->hit,  ch->max_hit,
	ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level + (13 / ch->generacion) * 40,
	ch->move, ch->max_move,
	ch->exp   );
    else
      sprintf( buf, "&g$n reporta &w%d&g/&w%d &gpv &w%d&g/&w%d &gmana &w%d&g/&w%d &gmv &w%d &gpx.&g",
	ch->hit,  ch->max_hit,
	ch->mana, ch->max_mana,
	ch->move, ch->max_move,
	ch->exp   );

    act( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );

    return;
}

void do_fprompt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  set_char_color( AT_GREY, ch );

  if ( IS_NPC(ch) )
  {
    send_to_char( "No puede cambiar su prompt..\n\r", ch );
    return;
  }
  smash_tilde( argument );
  one_argument( argument, arg );
  if ( !*arg || !str_cmp( arg, "display" ) )
  {
    send_to_char( "Tu actual prompt de lucha:\n\r", ch );
    set_char_color( AT_WHITE, ch );
    ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->fprompt, "" ) ? "(default prompt)"
				 				: ch->pcdata->fprompt );
    set_char_color( AT_GREY, ch );
    send_to_char( "Prueba 'help prompt' para mas informacion sobre el prompt.\n\r", ch );
    return;
  }
  send_to_char( "Cambiando antiguo prompt de:\n\r", ch );
  set_char_color( AT_WHITE, ch );
  ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->fprompt, "" ) ? "(default prompt)"
							      : ch->pcdata->fprompt );
  if (ch->pcdata->fprompt)
    STRFREE(ch->pcdata->fprompt);
  if ( strlen(argument) > 128 )
    argument[128] = '\0';

  /* Can add a list of pre-set prompts here if wanted.. perhaps
     'prompt 1' brings up a different, pre-set prompt */
  if ( !str_cmp(arg, "default") )
    ch->pcdata->fprompt = STRALLOC("");
  else
    ch->pcdata->fprompt = STRALLOC(argument);
  return;
}

void do_delete (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    argument = one_argument (argument, arg);

      if (arg[0] == '\0')
      {
          	send_to_char ("Para borrar el personaje escribe: DELETE <password>\n\r", ch);
          	return;
      }

      if (strcmp (crypt (arg, ch->pcdata->pwd), ch->pcdata->pwd))
      {
          	send_to_char ("Password erroneo.\n\r", ch);
           	return;
      }
      else
      {
      	do_destroy (ch, ch->name);
      	sprintf (buf, "%s ha borrado su personaje.", ch->name);
      	log_string (buf);
      }
      return;
}

void do_prompt( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];

  set_char_color( AT_GREY, ch );

  if ( IS_NPC(ch) )
  {
    send_to_char( "No puede cambiar su prompt..\n\r", ch );
    return;
  }
  smash_tilde( argument );
  one_argument( argument, arg );
  if ( !*arg || !str_cmp( arg, "display" ) )
  {
    send_to_char( "tu prompt actual es:\n\r", ch );
    set_char_color( AT_WHITE, ch );
    ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)"
				 				: ch->pcdata->prompt );
    set_char_color( AT_GREY, ch );
    send_to_char( "Prueba 'help prompt' para mas informacion sobre el prompt.\n\r", ch );
    return;
  }
  send_to_char( "Cambaindo antiguo prompt de:\n\r", ch );
  set_char_color( AT_WHITE, ch );
  ch_printf( ch, "%s\n\r", !str_cmp( ch->pcdata->prompt, "" ) ? "(default prompt)"
							      : ch->pcdata->prompt );
  if (ch->pcdata->prompt)
    STRFREE(ch->pcdata->prompt);
  if ( strlen(argument) > 128 )
    argument[128] = '\0';

  /* Can add a list of pre-set prompts here if wanted.. perhaps
     'prompt 1' brings up a different, pre-set prompt */
  if ( !str_cmp(arg, "default") )
    ch->pcdata->prompt = STRALLOC("");
  else
    ch->pcdata->prompt = STRALLOC(argument);

  return;
}

/*
 * Figured this belonged here seeing it involves players...
 * really simple little function to tax players with a large
 * amount of gold to help reduce the overall gold pool...
 *  --TRI
 */
void tax_player( CHAR_DATA *ch )
{

  /*int gold = ch->gold;
  int mgold = (ch->level * 2000000);
  int tax = (ch->gold * .05);

  if ( gold > mgold )
  {
    set_char_color( AT_WHITE, ch );
    ch_printf( ch, "Eres nivel %d y llevas mas de %d monedas de oro.\n\r",
	ch->level, mgold );
    ch_printf( ch, "Los impuestos son del %5 por ciento (%d monedas) de tus %d monedas,\n\r",
	tax, ch->gold );
    ch_printf( ch, "y te deja con %d monedas.\n\r",
	(ch->gold - tax));
    ch->gold -= tax;

  } */
  return;
}

void do_topten( CHAR_DATA *ch, char *argument )
{
        ch_printf( ch, "&wTu coeficiente es &g%d&w.\n\r", ch->pcdata->coeficiente );
        return;
}


