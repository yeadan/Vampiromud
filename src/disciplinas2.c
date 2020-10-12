/****************************************************************************
 *                       [M]ulti [U]ser [D]ungeon                           *
 ****************************************************************************
 * ------------------------------------------------------------------------ *
 * [C]achondeo  [O]rganizado    [L]ibre [I]ndependiente [K][O]jonudamente   *
 * [T]aleguero  [R]eivindicativo [O]scuramente [N]ocivo ***DANGER***        *
 * ------------------------------------------------------------------------ *
 * COLIKOTRON V2.0b No(C) 199x, 200x   por: La "gresca"                       *
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
 *         Modulo para la adkisiscion de disciplinas nuevas y propias       *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifdef sun
  #include <strings.h>
#endif
#include <time.h>
#include "mud.h"

/*
 * Modulo de las disciplinas vampiricas y no vampiricas de VampiroMudV2.0 beta
 * mail a vampiromud@hotmail.com.
 * VampiroMud adaptado y modificado por COLIKOTRON Code Team.
 * Idea original de SaNgUi y SiGo
 */



int disci_puntos_nivel(CHAR_DATA *ch)
{
    if (IS_NPC(ch)) return 1;

    return (ch->pcdata->habilidades[ch->pcdata->disci_adquirir] + 1) * 100;
}

void ganar_disci_puntos(CHAR_DATA *ch, int puntos)
{
    if (IS_NPC(ch)) return;

    if (ch->pcdata->disci_puntos == 666) return;
    if (ch->pcdata->disci_adquirir == -1) return;

    ch->pcdata->disci_puntos += puntos;

    if (ch->pcdata->disci_puntos < 0)
    {
	ch->pcdata->disci_puntos = 0;
	return;
    }

    if (ch->pcdata->disci_puntos >= disci_puntos_nivel(ch) )
    {
	ch->pcdata->disci_puntos = 999;
	WAIT_STATE(ch, 6);
	send_to_char( "&rHas acabado de aprender una disciplina.\n\r", ch);
	send_to_char( "Utiliza el comando 'asimilar'.\n\r", ch);
	return;
    }
}

void do_decapitar( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *victim;
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        CLAN_DATA *familia;
        CLAN_DATA *fam_vict;
        OBJ_DATA *obj;
        int anterior_generacion;
        int proxima_generacion;
        int ganancia;

   if (IS_NPC(ch) )
   {
        send_to_char( "Los mobs no pueden decapitar a nadie.\n\r", ch );
        return;
    }

      if ( argument[0] == '\0' )
      {
        send_to_char( "Decapitar a quien?\n\r", ch );
        return;
       }

       if ( ( victim = get_char_room( ch, argument ) ) == NULL )
       {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
       }

   if (IS_VAMPIRE(ch))
   {
       if(IS_VAMPIRE(victim))
       {
         send_to_char("&wNene, que noooo... Tu tienes que &gDiaBLeRiZaR&w.\n\r", ch);
         return;
       }
   }

       if (victim == ch)
       {
        send_to_char( "Decapitarte a ti mismo... estas de broma no?\n\r", ch );
        return;
       }


    /*
     * Si el jugador esta en una familia y hay declaraciones de alianza y guerra que afecten a
     * las decapitaciones.
     *                                          SaNgUiNaRi & SiGo
     */

     /* Declaracion de guerra piadosa */

     if(( IS_CLANNED(ch) && IS_CLANNED(victim)))
     {
     if (ch->pcdata->clan->name == victim->pcdata->clan->name )
     {
        send_to_char( "A alguien de tu familia?\n\r", ch );
        return;
     }


     /*if ( (ch->pcdata->clan->enemigos[victim->pcdata->clan->numero] = victim->pcdata->clan->name ) && (victim->pcdata->clan->odio[victim->pcdata->clan->numero] <= 1) )
      {
         send_to_char("&wNo puedes decapitarlo, no estas en guerra con el.\n\r", ch);
         return;
      }*/
      }
       if ( victim->hit >= 1 )
       {
        send_to_char( "No esta mortalmente herido, no puedes hacerlo.\n\r", ch );
        return;
       }

       if ( victim->level <= 69 )
       {
        send_to_char( "Solo puedes decapitar a jugadores mayores de nivel 69.\n\r", ch );
        return;
       }

       if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
       {
        send_to_char( "Necesitas llevar un arma para decapitarle.\n\r", ch );
        return;
       }

       if (IS_NPC(victim) )
       {
        send_to_char( "Decapitar a un Mob?.\n\r", ch );
        return;
       }

       send_to_char( "&OTu cabeza queda separada del resto del cuerpo, te han DECAPITADO!\n\r", victim );
       send_to_char( "&gPierdes 30 puntos de vida y movimiento.\n\r", victim );
       send_to_char( "&gHaz AYUDA DECAPITACION para saber que es lo que te ocurre al ser decapitado.\n\r", victim );

       if (ch->level < LEVEL_ENGENDRO )
       {
       ch->level ++;
       if( ch->level > ch->pcdata->max_level )
       ch->pcdata->max_level = ch->level;
       sprintf(buf, "&w%s &gha subido a nivel &Y%d &gdecapitando vilmente", ch->name, ch->level );
       mensa_todos(ch, "DeCaPiTaCioN", buf );
       }
       if (victim->level > LEVEL_AVATAR)
       {
       victim->level --;
       sprintf(buf, "&w%s &gha bajado a nivel &Y%d &gha sido Decapitado", victim->name, victim->level );
       mensa_todos(ch, "DeCaPiTaCioN", buf );
       }

     if (victim->level >= LEVEL_AVATAR )
     victim->exp_acumulada = 0;

     if (victim->generacion <= 13 && ch->generacion > 3 )
        {
        switch ( ch->generacion )
     {
      case 13 : proxima_generacion =  10 ; break;
      case 12 : proxima_generacion =  20 ; break;
      case 11 : proxima_generacion =  30 ; break;
      case 10 : proxima_generacion =  40 ; break;
      case  9 : proxima_generacion =  50 ; break;
      case  8 : proxima_generacion =  65 ; break;
      case  7 : proxima_generacion =  80  ; break;
      case  6 : proxima_generacion =  95  ; break;
      case  5 : proxima_generacion = 105 ; break;
     }
       switch ( victim->generacion )
            {
            case 13 : anterior_generacion =  -1 ; break;
            case 12 : anterior_generacion =   9 ; break;
            case 11 : anterior_generacion =  19 ; break;
            case 10 : anterior_generacion =  29 ; break;
            case  9 : anterior_generacion =  39 ; break;
            case  8 : anterior_generacion =  49 ; break;
            case  7 : anterior_generacion =  64 ; break;
            case  6 : anterior_generacion =  79 ; break;
            case  5 : anterior_generacion =  94 ; break;
            case  4 : anterior_generacion = 104 ; break;
            }
        if((victim->generacion < 13 &&ch->generacion >= victim->generacion))
        {
        victim->pcdata->genepkills = anterior_generacion;
        ch->pcdata->genepkills = proxima_generacion;
        advance_generacion( ch );
        degradacion_generacion_pk(victim);
        }
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
              ch->pcdata->clan->pkills[0] = (ch->pcdata->clan->pkills[0] + (ch->level - victim->level));
            else if ( victim->level < 49 )
              ch->pcdata->clan->pkills[1] = (ch->pcdata->clan->pkills[1] + (ch->level - victim->level));
            else if ( victim->level < 59 )
              ch->pcdata->clan->pkills[2] = (ch->pcdata->clan->pkills[2] + (ch->level - victim->level));
            else if ( victim->level < 69 )
              ch->pcdata->clan->pkills[3] = (ch->pcdata->clan->pkills[3] + (ch->level - victim->level));
            else if ( victim->level < (LEVEL_HERO -1) )
              ch->pcdata->clan->pkills[4] = (ch->pcdata->clan->pkills[4] + (ch->level - victim->level));
            else if ( victim->level < LEVEL_HERO )
              ch->pcdata->clan->pkills[5] = (ch->pcdata->clan->pkills[5] + (ch->level - victim->level));
            else
              ch->pcdata->clan->pkills[6] = (ch->pcdata->clan->pkills[6] + (ch->level - victim->level));

   }

   ch->pcdata->pkills++;
     adjust_hiscore( "pkill", ch, ch->pcdata->pkills ); /* cronel hiscore */
	  ch->hit = ch->max_hit;
	  ch->mana = ch->max_mana;
	  ch->move = ch->max_move;

   if ( victim->pcdata->clan )
	{
            if ( ch->level < 39 )
              victim->pcdata->clan->pdeaths[0] = (victim->pcdata->clan->pdeaths[0] + (victim->level - ch->level));
            else if ( ch->level < 49 )
               victim->pcdata->clan->pdeaths[1] = (victim->pcdata->clan->pdeaths[1]+ (victim->level - ch->level));
            else if ( ch->level < 59 )
               victim->pcdata->clan->pdeaths[2] = (victim->pcdata->clan->pdeaths[2] + (victim->level - ch->level));
            else if ( ch->level < 69 )
               victim->pcdata->clan->pdeaths[3] = (victim->pcdata->clan->pdeaths[3] + (victim->level - ch->level));
            else if ( ch->level < (LEVEL_HERO -1) )
               victim->pcdata->clan->pdeaths[4] = (victim->pcdata->clan->pdeaths[4] + (victim->level - ch->level));
            else if ( ch->level < LEVEL_HERO )
               victim->pcdata->clan->pdeaths[5] = (victim->pcdata->clan->pdeaths[5] + (victim->level - ch->level));
            else
               victim->pcdata->clan->pdeaths[6] = (victim->pcdata->clan->pdeaths[6] + (victim->level - ch->level));


   }
          sprintf(buf, "Consigues %d puntos para tu familia!", (ch->level - victim->level) );
          mensa(ch, "FaMiLia", buf);
          sprintf(buf, "Pierdes %d puntos de tu familia!", (victim->level - ch->level) );
          mensa(victim, "FaMiLia", buf );


           victim->pcdata->pdeaths++;
	adjust_favor( victim, 11, 1 );
	adjust_favor( ch, 2, 1 );
	add_timer( victim, TIMER_PKILLED, 115, NULL, 0 );
	WAIT_STATE( victim, 3 * PULSE_VIOLENCE );
    }
  }

      /* ch->max_hit += 30;
       ch->max_move += 30;
       victim->max_hit -= 30;
       victim->max_move -= 30;

       sprintf(buf, "&w%s &gha Decapitado a %s gana &Y30 &gpuntos de vida y move", ch->name, victim->name );
       mensa_todos(ch, "DeCaPiTaCioN", buf);

       */
       perdida_total( ch, victim );

       ch->pcdata->condition[COND_BLOODTHIRST] = ch->level + 10 + (13 / ch->generacion) * 40;
       ch->hit = ch->max_hit;
       ch->move = ch->max_move;
       send_to_char( "&wRecuperas totalmente tu vitalidad.\n\r", ch );
       send_to_char( "&wTe sientes mas poderoso!\n\r", ch );

/* Pa ver si los ceporros estos estan en guerra y les sume las muertes y todo eso SiGo, SaNgUi & MaCHaKa */
  if((IS_CLANNED(ch)
  && IS_CLANNED(victim)))
  {
    if ( ch->pcdata->clan != NULL && victim->pcdata->clan != NULL)
    {
          /* Primero vemos si tan en guerra o no */

          if ( ch->pcdata->clan->odio[victim->pcdata->clan->numero] == victim->pcdata->clan->name )
          {
             /* Despues restamos las muertes de guerra y sumamos las victorias */
             ch->pcdata->clan->victorias_guerra[victim->pcdata->clan->numero] += 1;
             ch->pcdata->clan->victorias += 1;
             victim->pcdata->clan->muerte_guerra[ch->pcdata->clan->numero] -= 1;
             victim->pcdata->clan->derrotas += 1;
          }
    }
        familia = get_clan( ch->pcdata->clan->name );
        fam_vict = get_clan( victim->pcdata->clan->name );


    /* Pa ver si los ceporros estos estan en guerra y les sume las muertes y todo eso SiGo, SaNgUi & MaCHaKa */

    if (( familia && fam_vict ))
    {
          /* Primero vemos si tan en guerra o no */

          if ( familia->odio[fam_vict->numero] == fam_vict->name )
          {
             /* Despues restamos las muertes de guerra y sumamos las victorias */
             familia->victorias_guerra[fam_vict->numero] += 1;
             familia->victorias += 1;
             fam_vict->muerte_guerra[ch->pcdata->clan->numero] -= 1;
             fam_vict->derrotas += 1;
          }
    }

   } /* Cierra el IF IS_CLANNED */
       ch->pcdata->genepkills++;
       victim->pcdata->genepkills--;
       death_cry( victim );
       raw_kill(ch, victim);
       save_char_obj( victim );
       return rVICT_DIED;
       return;
}

void do_diablerizar( CHAR_DATA *ch, char *argument, CHAR_DATA *victim )
{
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char filename[256];
        CLAN_DATA *familia;
        CLAN_DATA *fam_vict;
        bool npcvict;
        int anterior_generacion;
        int proxima_generacion;

   if (IS_NPC(ch) )
   {
        send_to_char( "Los mobs no pueden diablerizar a nadie.\n\r", ch );
        return;
    }

      if ( argument[0] == '\0' )
      {
        send_to_char( "Diablerizar a quien?\n\r", ch );
        return;
       }

       if ( ( victim = get_char_room( ch, argument ) ) == NULL )
       {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
       }

       if ( victim == ch )
       {
        send_to_char( "Diablerizarte a ti mismo.... no te gustaria, mejor no... vale?\n\r", ch );
        return;
       }

       if ( victim->hit >= 1 )
       {
        send_to_char( "No esta mortalmente herido, no puedes hacerlo.\n\r", ch );
        return;
       }

    /* Si el jugador esta en una familia y hay declaraciones de alianza y guerra que afecten a
     * las decapitaciones.
     *                                          SaNgUiNaRi & SiGo
     */

     /* Declaracion de guerra piadosa */

    if((IS_CLANNED(victim)
        && IS_CLANNED(ch)))
    {
     if ( ch->pcdata->clan->name == victim->pcdata->clan->name )
     {
        send_to_char( "A alguien de tu casa?\n\r", ch );
        return;
     }


    /* if ( ( (victim->pcdata->clan->enemigos[ch->pcdata->clan->numero] == ch->pcdata->clan->name ) || (ch->pcdata->clan->enemigos[victim->pcdata->clan->numero] == victim->pcdata->clan->name ) ) && (ch->pcdata->clan->odio[victim->pcdata->clan->numero] <= 1) )
      {
         send_to_char("&wNo puedes diablerizarlo, no estas en guerra con el.\n\r", ch);
         return;
      }*/
     }

       if ( victim->level <= 69 )
       {
        send_to_char( "Solo puedes diablerizar a jugadores mayores de nivel 69.\n\r", ch );
        return;
       }

       if ( !IS_VAMPIRE(victim) )
       {
        send_to_char( "Solo puedes diablerizar a Vampiros a este decapitalo.\n\r", ch );
        return;
       }

       if (IS_NPC(victim) )
       {
        send_to_char( "Diablerizar a un Mob?.\n\r", ch );
        return;
       }

       if (!IS_VAMPIRE(ch) )
       {
        send_to_char( "Mejor deja la artes cainitas para los cainitas.\n\r", ch );
        return;
       }

       send_to_char( "&OSientes un horrendo dolor en todo tu ser, tu cuerpo se agita freneticamnete\n\r", victim );
       send_to_char( "&Otu alma gime, te han DIABLERIZADO.\n\r", victim );
       send_to_char( "&gPierdes 30 puntos de vida y movimiento.\n\r", victim );
       send_to_char( "&gHaz AYUDA DIABLERIE para saber que es lo que te ocurre al ser dieblaerizado.\n\r", victim );

       if (ch->level < LEVEL_ENGENDRO)
       {
       ch->level ++;
       if( ch->level > ch->pcdata->max_level )
       ch->pcdata->max_level = ch->level;
       sprintf(buf, "&w%s &gha subido a nivel &Y%d &gusando Diablerie", ch->name, ch->level );
       mensa_todos( ch, "DiaBLeRie", buf );
       }
       if (victim->level > LEVEL_AVATAR)
       {
       victim->level --;
       sprintf(buf, "&w%s &gha bajado a nivel &Y%d &gha sido Diablerizado", victim->name, victim->level );
       mensa_todos( ch, "DiaBLeRie", buf );
     }

     if (victim->level >= LEVEL_AVATAR )
     victim->exp_acumulada = 0;

     if (victim->generacion <= 13 && ch->generacion > 3 )
        {
        switch ( ch->generacion )
     {
      case 13 : proxima_generacion =  10 ; break;
      case 12 : proxima_generacion =  20 ; break;
      case 11 : proxima_generacion =  30 ; break;
      case 10 : proxima_generacion =  40 ; break;
      case  9 : proxima_generacion =  50 ; break;
      case  8 : proxima_generacion =  65 ; break;
      case  7 : proxima_generacion =  80  ; break;
      case  6 : proxima_generacion =  95  ; break;
      case  5 : proxima_generacion = 105 ; break;
     }
       switch ( victim->generacion )
            {
            case 13 : anterior_generacion =  -1 ; break;
            case 12 : anterior_generacion =   9 ; break;
            case 11 : anterior_generacion =  19 ; break;
            case 10 : anterior_generacion =  29 ; break;
            case  9 : anterior_generacion =  39 ; break;
            case  8 : anterior_generacion =  49 ; break;
            case  7 : anterior_generacion =  64 ; break;
            case  6 : anterior_generacion =  79 ; break;
            case  5 : anterior_generacion =  94 ; break;
            case  4 : anterior_generacion = 104 ; break;
            }
        if((victim->generacion < 13 &&ch->generacion >= victim->generacion))
        {
        victim->pcdata->genepkills = anterior_generacion;
        ch->pcdata->genepkills = proxima_generacion;
        advance_generacion( ch );
        degradacion_generacion_pk(victim);
        }
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
              ch->pcdata->clan->pkills[0] = (ch->pcdata->clan->pkills[0] + (ch->level - victim->level));
            else if ( victim->level < 49 )
              ch->pcdata->clan->pkills[1] = (ch->pcdata->clan->pkills[1] + (ch->level - victim->level));
            else if ( victim->level < 59 )
              ch->pcdata->clan->pkills[2] = (ch->pcdata->clan->pkills[2] + (ch->level - victim->level));
            else if ( victim->level < 69 )
              ch->pcdata->clan->pkills[3] = (ch->pcdata->clan->pkills[3] + (ch->level - victim->level));
            else if ( victim->level < (LEVEL_HERO -1) )
              ch->pcdata->clan->pkills[4] = (ch->pcdata->clan->pkills[4] + (ch->level - victim->level));
            else if ( victim->level < LEVEL_HERO )
              ch->pcdata->clan->pkills[5] = (ch->pcdata->clan->pkills[5] + (ch->level - victim->level));
            else
              ch->pcdata->clan->pkills[6] = (ch->pcdata->clan->pkills[6] + (ch->level - victim->level));

   }

   ch->pcdata->pkills++;
     adjust_hiscore( "pkill", ch, ch->pcdata->pkills ); /* cronel hiscore */
	  ch->hit = ch->max_hit;
	  ch->mana = ch->max_mana;
	  ch->move = ch->max_move;

   if ( victim->pcdata->clan )
	{
            if ( ch->level < 39 )
              victim->pcdata->clan->pdeaths[0] = (victim->pcdata->clan->pdeaths[0] + (victim->level - ch->level));
            else if ( ch->level < 49 )
               victim->pcdata->clan->pdeaths[1] = (victim->pcdata->clan->pdeaths[1]+ (victim->level - ch->level));
            else if ( ch->level < 59 )
               victim->pcdata->clan->pdeaths[2] = (victim->pcdata->clan->pdeaths[2] + (victim->level - ch->level));
            else if ( ch->level < 69 )
               victim->pcdata->clan->pdeaths[3] = (victim->pcdata->clan->pdeaths[3] + (victim->level - ch->level));
            else if ( ch->level < (LEVEL_HERO -1))
               victim->pcdata->clan->pdeaths[4] = (victim->pcdata->clan->pdeaths[4] + (victim->level - ch->level));
            else if ( ch->level < LEVEL_HERO )
               victim->pcdata->clan->pdeaths[5] = (victim->pcdata->clan->pdeaths[5] + (victim->level - ch->level));
            else
               victim->pcdata->clan->pdeaths[6] = (victim->pcdata->clan->pdeaths[6] + (victim->level - ch->level));


   }
          sprintf(buf, "Consigues %d puntos para tu familia!", (ch->level - victim->level) );
          mensa(ch, "FaMiLia", buf);
          sprintf(buf, "Pierdes %d puntos de tu familia!", (victim->level - ch->level) );
          mensa(ch, "FaMiLia", buf );


           victim->pcdata->pdeaths++;
	adjust_favor( victim, 11, 1 );
	adjust_favor( ch, 2, 1 );
	add_timer( victim, TIMER_PKILLED, 115, NULL, 0 );
	WAIT_STATE( victim, 3 * PULSE_VIOLENCE );
    }
  }

   /*    ch->max_hit += 30;
       ch->max_move += 30;
       victim->max_hit -= 30;
       victim->max_move -= 30;

       sprintf(buf, "&O%s &gha Dieblerizado a %s gana &Y30 &gpuntos de vida y move", ch->name, victim->name );
       mensa_todos(ch, "&RDiaBLeRie", buf);
    */
       perdida_total( ch, victim );

       ch->pcdata->condition[COND_BLOODTHIRST] = ch->level + 10 + (13 / ch->generacion) * 40;
       ch->hit = ch->max_hit;
       ch->move = ch->max_move;
       send_to_char( "&wRecuperas totalmente tu vitalidad.\n\r", ch );
       send_to_char( "&wTe sientes mas poderoso!\n\r", ch );

/* Pa ver si los ceporros estos estan en guerra y les sume las muertes y todo eso SiGo, SaNgUi & MaCHaKa */
  if((IS_CLANNED(ch)
  && IS_CLANNED(victim)))
  {
    if ( ch->pcdata->clan != NULL && victim->pcdata->clan != NULL)
    {
          /* Primero vemos si tan en guerra o no */

          if ( ch->pcdata->clan->odio[victim->pcdata->clan->numero] == victim->pcdata->clan->name )
          {
             /* Despues restamos las muertes de guerra y sumamos las victorias */
             ch->pcdata->clan->victorias_guerra[victim->pcdata->clan->numero] += 1;
             ch->pcdata->clan->victorias += 1;
             victim->pcdata->clan->muerte_guerra[ch->pcdata->clan->numero] -= 1;
             victim->pcdata->clan->derrotas += 1;
          }
    }
        familia = get_clan( ch->pcdata->clan->name );
        fam_vict = get_clan( victim->pcdata->clan->name );


    /* Pa ver si los ceporros estos estan en guerra y les sume las muertes y todo eso SiGo, SaNgUi & MaCHaKa */

    if (( familia && fam_vict ))
    {
          /* Primero vemos si tan en guerra o no */

          if ( familia->odio[fam_vict->numero] == fam_vict->name )
          {
             /* Despues restamos las muertes de guerra y sumamos las victorias */
             familia->victorias_guerra[fam_vict->numero] += 1;
             familia->victorias += 1;
             fam_vict->muerte_guerra[ch->pcdata->clan->numero] -= 1;
             fam_vict->derrotas += 1;
          }
    }

   } /* Cierra el IF IS_CLANNED */
       ch->pcdata->genepkills++;
       victim->pcdata->genepkills--;
       death_cry( victim );
       raw_kill(ch, victim);
       save_char_obj( victim );
       return rVICT_DIED;
       return;
      }

void perdida_total( CHAR_DATA *ch, CHAR_DATA *victim )
{
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char buf3[MAX_STRING_LENGTH];
        char buf4[MAX_STRING_LENGTH];
        char buf5[MAX_STRING_LENGTH];
        int diferencia = 0;
        int perdida = 0;

        sprintf(buf2, "");
        sprintf(buf3, "");
        sprintf(buf4, "");
        sprintf(buf5, "");

        diferencia = victim->level - ch->level;

        if( diferencia == 0)
        perdida = 50;
        else if( diferencia == -1)
        perdida = 30;
        else if( diferencia == -2)
        perdida = 25;
        else if( diferencia == -3)
        perdida = 15;
        else if( diferencia == -4)
        perdida = 10;
        else if( diferencia <= -5)
        perdida = 5;
        else
        perdida = (diferencia * 50);

    if(IS_VAMPIRE(victim))
    {

       if(victim->max_hit >= (500 + perdida))
       {
        ch->max_hit += perdida;
        victim->max_hit -= perdida;
        sprintf(buf2, "vida " );
       }
       if((victim->max_hit >= (500 + perdida) && victim->max_move >= (500 + perdida)))
       sprintf(buf3, "y " );
       if( victim->max_move >= (500 + perdida))
       {
       ch->max_move += perdida;
       victim->max_move -= perdida;
       sprintf(buf4, "movimiento" );
       }
       if(IS_VAMPIRE(ch))
       {
       if((victim->max_hit >= (500 + perdida) || victim->max_move >= (500 + perdida)))
        sprintf(buf, "&w%s &gha Diablerizado a %s gana &Y%d &gpuntos de %s%s%s", ch->name, victim->name, perdida, buf2, buf3, buf4 );
       else
        sprintf(buf, "&w%s &gha Diablerizado a %s, pero %s es muy pardo y no gano nada", ch->name, victim->name, victim->name );
         mensa_todos(ch, "DiaBLeRie", buf);
         return;
       }
       else
       {
       if((victim->max_hit >= (500 + perdida) || victim->max_move >= (500 + perdida)))
        sprintf(buf, "&w%s &gha Decapitado a %s gana &Y%d &gpuntos de %s%s%s", ch->name, victim->name, perdida, buf2, buf3, buf4 );
       else
        sprintf(buf, "&w%s &gha Decapitado a %s, pero %s es muy pardo y no gano nada", ch->name, victim->name, victim->name );
        mensa_todos(ch, "Decapitacion", buf);
        return;
       }
    }
    else
    {
       if(victim->max_hit >= (500 + perdida) )
       {
        ch->max_hit += perdida;
        victim->max_hit -= perdida;
        sprintf(buf2, "vida " );
       }
       if(victim->max_mana >= (500 + perdida))
       {
        ch->max_mana += perdida;
        victim->max_mana  -= perdida;
        sprintf(buf3, "mana " );
       }
       if((victim->max_hit >= (500 + perdida) && victim->max_move >= (500 + perdida) && victim->max_mana >= (500 + perdida)
          || victim->max_hit >= (500 + perdida) && victim->max_mana >= (500 + perdida)
          || victim->max_move >= (500 + perdida) && victim->max_mana >= (500 + perdida)
          || victim->max_hit >= (500 + perdida) && victim->max_move >= (500 +perdida)))
       sprintf(buf4, "y " );

       if( victim->max_move >= (500 + perdida))
       {
       ch->max_move += perdida;
       victim->max_move -= perdida;
       sprintf(buf5, "movimiento" );
       }
       if(IS_VAMPIRE(ch) && IS_VAMPIRE(victim))
       {
       if((victim->max_mana >= (500 + perdida) || victim->max_move >= (500 + perdida) || victim->max_mana >= (500 + perdida)))
        sprintf(buf, "&w%s &gha Diablerizado a %s gana &Y%d &gpuntos de %s%s%s%s", ch->name, victim->name, perdida, buf2, buf3, buf4, buf5 );
       else if((victim->max_hit >= (500 + perdida) && victim->max_mana >= (500 + perdida)
                || victim->max_move >= (500 + perdida) && victim->max_mana >= (500 + perdida)
                || victim->max_move >= (500 + perdida) && victim->max_hit >= (500 + perdida)))
        sprintf(buf, "&w%s &gha Diablerizado a %s gana &Y%d &gpuntos de %s%s%s%s", ch->name, victim->name, perdida, buf2, buf4, buf3, buf5 );
       else
        sprintf(buf, "&w%s &gha Diablerizado a %s, pero %s es muy pardo y no gano nada", ch->name, victim->name, victim->name );

         mensa_todos(ch, "DiaBLeRie", buf);
         return;
        }
        if(IS_VAMPIRE(ch) && !IS_VAMPIRE(victim))
        {
        if((victim->max_mana >= (500 + perdida) || victim->max_move >= (500 + perdida) || victim->max_mana >= (500 + perdida)))
        sprintf(buf, "&w%s &gha Decapitado a %s gana &Y%d &gpuntos de %s%s%s%s", ch->name, victim->name, perdida, buf2, buf3, buf4, buf5 );
       else if((victim->max_hit >= (500 + perdida) && victim->max_mana >= (500 + perdida)
                || victim->max_move >= (500 + perdida) && victim->max_mana >= (500 + perdida)
                || victim->max_move >= (500 + perdida) && victim->max_hit >= (500 + perdida)))
        sprintf(buf, "&w%s &gha Decapitado a %s gana &Y%d &gpuntos de %s%s%s%s", ch->name, victim->name, perdida, buf2, buf4, buf3, buf5 );
       else
        sprintf(buf, "&w%s &gha Decapitado a %s, pero %s es muy pardo y no gano nada", ch->name, victim->name, victim->name );

         mensa_todos(ch, "Decapitacion", buf);
         return;
       }
       if(!IS_VAMPIRE(ch))
       {
       if((victim->max_mana >= (500 + perdida) || victim->max_move >= (500 + perdida) || victim->max_mana >= (500 + perdida)))
        sprintf(buf, "&w%s &gha Decapitado a %s gana &Y%d &gpuntos de %s%s%s%s", ch->name, victim->name, perdida, buf2, buf3, buf4, buf5 );
       else if((victim->max_hit >= (500 + perdida) && (victim->max_mana >= (500 + perdida))
                || victim->max_move >= (500 + perdida) && victim->max_mana >= (500 + perdida)
                || victim->max_move >= (500 + perdida) && victim->max_hit >= (500 + perdida)))
        sprintf(buf, "&w%s &gha Decapitado a %s gana &Y%d &gpuntos de %s%s%s%s", ch->name, victim->name, perdida, buf2, buf4, buf3, buf5 );
       else
        sprintf(buf, "&w%s &gha Decapitado a %s, pero %s es muy pardo y no gano nada", ch->name, victim->name, victim->name );

         mensa_todos(ch, "Decapitacion", buf);
         return;
       }
    }

return;
}

void do_adquirir(CHAR_DATA *ch, char *argument)
{
    int max_gene =0;
    int adquiridas=0;
    int i;
    int cuenta = 0;
    int necesita;
    int max_niv;
    char buf[MAX_STRING_LENGTH];

    max_niv = 0;

     if (IS_VAMPIRE(ch))
     {
        switch( ch->generacion )
        {
                case 13:
                
		max_niv = 3;                
                max_gene = 1;
		break;
		
		case 12:
                
		max_niv = 4;                  
		max_gene = 1;
		break;
                
		case 11:
                
		max_niv = 4;                 
		max_gene = 1;
		break;
                
		case 10:
                
 		max_niv = 5;                 
		max_gene=2;
	        break;
               
		case 9:
               
		 max_niv = 5;                  
		max_gene=2;
		break;
                
		case 8:
                
		max_niv = 6;                  
		max_gene=3;
		break;
                
		case 7:
                
		max_niv = 7;                 
		max_gene = 4;
		 break;
                
		case 6:
                
		max_niv = 8;                  
		max_gene= 5;
		break;
                
		case 5:
                
		max_niv = 9;                 
		max_gene= 6;
		 break;
                
		case 4:
                
		max_niv = 10;                 
		max_gene=7;
		 break;
		
                case 3:
                max_niv = 12;            
		max_gene=8;      
		 break;
                
		case 2:
                max_niv = 14; 
		max_gene=9;
                 break;
                
		case 1:
                max_niv = 15;
	        max_gene=10;
                  break;
          }
     }

    if (IS_NPC(ch))
    return;


    if (!str_cmp(argument, "cancelar"))
    {
	if (ch->pcdata->disci_adquirir == -1)
	{
	    send_to_char("&wAhora no estas aprendiendo ninguna disciplina.\n\r",ch);
	    return;
	}
	send_to_char("&wParas de adquirir disciplinas.\n\r", ch);
	ch->pcdata->disci_adquirir = -1;
	ch->pcdata->disci_puntos = 0;
 	return;
    }

    if ( !IS_VAMPIRE(ch) )
    {
      send_to_char( "&wSolo los Vampiros pueden adquirir disciplinas.\n\r", ch );
      send_to_char( "&wTu debes usar el comando '&gaprender'&w.\n\r", ch );
      return;
    }
 for (i=0;i<MAX_DISCIPLINAS;i++)
 {

	if(ch->pcdata->habilidades[i] > 0) adquiridas++;
 }  
if( adquiridas >= (3 + max_gene + ch->pcdata->renacido))
{
	send_to_char( "&wNo puedes aprender mas disciplinas con esta generacion.\n\r", ch);
	send_to_char( "&wMejora tu generacion para seguir adquiriendo disciplinas.\n\r", ch);
	return;
}
if (argument[0] == '\0')
    {
    for ( i = 1; i < MAX_DISCIPLINAS; i++)
    {
        if(ch->pcdata->habilidades[i] == 10)
        cuenta++;
	
       if(cuenta >= (10 + ch->pcdata->renacido))
        {
                send_to_char( "&wYa conoces todas las disciplinas que puedes aprender y sus niveles.\n\r", ch);
                cuenta = 0;
                return;
        }
    }
	send_to_char("&wQue disciplina quieres adquirir?.\n\r", ch);
	return;
    }

    if (ch->pcdata->disci_adquirir != -1)
    {
	send_to_char("&wYa estas aprendiendo una.\n\r", ch);
	send_to_char("&wUsa '&gadquirir cancelar&w' para adquirir otra.\n\r",ch);
	return;
    }

    for ( i = 1 ; i < MAX_DISCIPLINAS ; i++)
    {
	if (disciplina[i][0] != '\0'
	    && !str_prefix(argument, disciplina[i]) )
	{
		necesita = ((ch->pcdata->habilidades[i] - 5) * 10);
	    if (ch->pcdata->habilidades[i] < 0)
	    {
    		send_to_char("&wNo conoces el poder de ninguna disciplina asi.\n\r", ch);
		return;
	    }

          if (( IS_VAMPIRE(ch)  && ch->pcdata->habilidades[i] >= max_niv))
	  {
		send_to_char("&wNecesitas mejorar tu generacion para conocer los secretos de ese nivel.\n\r",ch);
	        return;
	  }
	  if ( ch->pcdata->habilidades[i] >= 10 )
	  {
		send_to_char("&wYa has adquirido todos los Niveles de esa disciplina.\n\r",ch);
                send_to_char("&wDebe de ser otra.\n\r",ch);
	        return;
	  }
          
	  // CONTROL DE QUE CLANES APRENDEN QUE DISCIPLINAS, BY NKARI

          if(!ch->level>250 && !str_cmp( argument, "daimoinon") && ch->class != CLASS_BAALI )
          {
                send_to_char( "Los secretos de esta disciplina kedan fuera de tu alcance.\n\r", ch );
                return;
          }
          if(!ch->level>250 && !str_cmp( argument, "taumaturgia") && ch->class != CLASS_TREMERE)
	  {
	       send_to_char( "Solo los miembros del clan Tremere pueden aprender Taumaturgia.\n\r", ch);
	       return; 
	   }
          if(!ch->level>250 &&  !str_cmp( argument, "serpentis") && ch->class != CLASS_SETITA)
	  {
	      send_to_char( "Solo los Hijos de Set pueden aprender Serpentis,busca alguno que te la ensenye.\n\r", ch);
	      return;  
           }
	  if(!ch->level>250 && !str_cmp( argument, "dementacion") && ch->class != CLASS_MALKAVIAN)
	  { 
	      send_to_char( "Solo los Malkavian pueden adquirir dementacion,busca alguno que te la ensenye.\n\r", ch);
	      return;
	  }
	  if(!ch->level>250 && !str_cmp( argument, "protean") && ch->class != CLASS_GANGREL)
	  {
		send_to_char( "Solo los gangrel pueden adquirir protean,busca alguno que te la ensenye.\n\r",  ch);
		return;
	  }
	  if(!ch->level>250 && !str_cmp( argument, "mortis") && ch->class != CLASS_DRUID)
	  {
		send_to_char( "Solo los capadocios pueden aprender mortis,busca alguno que te la ensenye.\n\r", ch);
		return;
	  } 
	  if(!ch->level>250 && !str_cmp( argument, "extincion") && ch->class != CLASS_VAMPIRE)
	  {
              send_to_char( "Solo los Assamitas pueden aprender Extincion,busca alguno que te la ensenye.\n\r", ch);
              return;         
          } 
	  if(!ch->level>250 && !str_cmp( argument, "vicisitud") && ch->class != CLASS_TZIMISCE)
	  {
	      send_to_char( "Solo los Tzimisce pueden adquirir Vicisitud,busca alguno que te la ensenye.\n\r", ch);
	      return;
	  }
          if(!ch->level>250 && !str_cmp( argument, "quimerismo") && ch->class != CLASS_RAVNOS)
	  {
	      send_to_char( "Solo los Ravnos pueden adquirir Quimerismo,busca alguno que te la ensenye.\n\r", ch);
	      return;
	  }
	  if(!ch->level>250 && !str_cmp( argument, "obeah") && ch->class != CLASS_SALUBRI)
	  {
	      send_to_char( "Solo los Salubri pueden adquirir Obeah.\n\r", ch);
	      return;
	  }		     
	  if(!ch->level>250 && !str_cmp( argument, "valeren") && ch->class != CLASS_SALUBRI)
	  {
	      send_to_char( "Solo los Salubri pueden adquirir Valeren.\n\r", ch);
	      return;
	  }	 
	if ( !str_prefix( argument, "bardo" )
	       || !str_prefix( argument, "conocimiento" )
               || !str_prefix( argument, "Nigromancia" )
               || !str_prefix( argument, "melepomene" )
               || !str_prefix( argument, "kinetismo" )
               || !str_prefix( argument, "mytheceria" )
	       || !str_prefix( argument, "nihilistica" )
               || !str_prefix( argument, "sanguinus" )
   	       || !str_prefix( argument, "alimentacion")
	       || !str_prefix( argument, "tanatosis")
               || !str_prefix( argument, "visceratika" ))
            {
	      send_to_char("&wLo sentimos pero esa disciplina no esta acabada aun...\n\r", ch );
		return;
            }

	    sprintf(buf, "&wEmpiezas a concentrarte en los secretos del nivel %d de %s.\n\r",
            ch->pcdata->habilidades[i] + 1,
		disciplina[i]);
	    send_to_char(buf, ch);
	    ch->pcdata->disci_puntos = 0;
	    ch->pcdata->disci_adquirir = i;
	    return;
	}
    }
    send_to_char("&wNo conoces ninguna disciplina llamada asi.\n\r", ch);
}

/*const  char * wwgift [MAX_GIFTS] =
{
	"",
	"Homid",		"Metis",
	"Lupus",		"Ragabash",
	"Theurge",		"Philodox",
	"Galliard",		"Ahroun",
	"Black Furies",		"Bone Gnawers",
	"Children of Gaia",	"Fianna",
	"Get of Fenris",	"Glass Walkers",
	"Red Talons",		"Shadow Lords",
	"Silent Striders",	"Silver Fangs",
	"Stargazers",		"Uktena",
	"Wendigo"
};
   */
const  char * disciplina [MAX_DISCIPLINAS ]  =
{
    "", "animalismo", "auspex", "bardo", "celeridad", "conocimiento", "daimoinon", "dementacion", "dominacion",
    "extincion", "kinetismo", "fortaleza", "melepomene", "mortis", "mytheceria", "nigromancia", "nihilistica",
    "obeah", "obtenebracion", "ofuscacion", "presencia", "potencia", "protean", "quimerismo", "sanguinus",
    "serpentis", "spiritus", "tanatosis", "temporis", "taumaturgia", "taumaturgia_oscura", "vicisitud",
    "visceratika", "alimentacion" ,"valeren"
};

const char * animalismo [11] =
{
   "","Susurros Amables","Llamada Salvaje","en construccion","en construccion","en construccion","en construccion",
	"en construccion","en construccion","en construccion","en construccion"
};
const char * usoanimalismo [11] =
{
  "","Susurros <mob>","Llamada <mob>","en construccion","en construccion","en construccion","en construccion",
	"en construccion","en construccion","en construccion","en construccion"
};
 const char * obeah [11] =
{
    "","en construccion", "en construccion", "en construccion", "en construccion","en construccion","en construccion",
       "en construccion","en construccion","en construccion", "en construccion"
};
const char * usoobeah [11] =
{
   "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
      "en construccion","en construccion","en construccion","en construccion"
};
const char * mortis [11] =
{
  "","Mascara de Muerte","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * usomortis [11] =
{
  "","Mascara <objetivo>","en construccion","en construccion","en construccion",
     "en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * obtenebracion [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};  
const char * usoobtenebracion [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * ofuscacion [11] =
{
  "","Capa Sombria","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * usoofuscacion [11] =
{
  "","capasombria","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
}; 
const char * presencia [11] =
{
  "","Fascinacion","Mirada Aterradora","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * usopresencia [11] =
{
  "","fascinacion","mirada <objetivo>","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * protean [11] =
{
  "","Testigo Tinieblas","Garras de la Bestia","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * usoprotean [11] =
{
  "","testigo","garras","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion"
};
const char * quimerismo [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * usoquimerismo [11] =
{ 
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * serpentis [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * usoserpentis [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
}; 
const char * vicisitud [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * usovicisitud [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * valeren [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"

}; 
const char * usovaleren [11] =
{
  "","en construccion","en construccion","en construccion","en construccion","en construccion","en construccion",
     "en construccion","en construccion","en construccion","en construccion"
};
const char * dominacion [11] =
{
    "","Dominio", "Mesmerismo", "Memoria del Juerguista", "en construccion","en construccion","en construccion",
       "en construccion","en construccion","en construccion", "en construccion"
};
const char * usodominacion [11] =
{
  "","Dominio '<pj> <secuencia de ordenes>'","Mesmerismo '<pj> <secuencia de ordenes>'","Memoria '<pj> <secuencia de ordenes>",
   "en construccion", "en construccion",
   "en construccion", "en construccion","en construccion","en construccion","en construccion"
};
const char * taumaturgia [11] =
{
    "", "Creo Ignem", "Caldero Sanguineo", "Forma Incorporea", "Corazon Petreo", "en construccion", "en construccion",
     "en construccion", "en construccion", "en construccion", "en construccion"
};
const char *usotauma [11] =
{   "", "Creo <objetivo>", "Caldero <objetivo>", "Incorporeo", "Petrificarse", "en construccion", "en construccion",
     "en construccion", "en construccion", "en construccion", "en construccion"
};
const char * dementacion [11] =
{
    "", "Pasion", "Embrujar Alma", "Cancion Serena", "Oleada Demente", "Koma", "en construccion",
     "en construccion", "en construccion", "en construccion", "en construccion"
};
const char *usodemen [11] =
{   "", "Pasion <objetivo>", "Embrujar <objetivo>", "cancionserena <objetivo>", "Oleadademente <objetivo>", "Koma <objetivo>", "en construccion",
     "en construccion", "en construccion", "en construccion", "en construccion"
};
const char *auspex [11] =
{
    "", "Sentidos Aguzados", "Vision del Alma", "Toque Espiritual", "Proyeccion Psiquica", "en construccion", "en construccion", "en construccion",
    "en construccion", "en construccion", "en construccion"
};
const char *usoauspex [11] =
{
    "", "Sentidosaguzados", "en construccion", "en construccion", "en construccion", "en construccion", "en construccion", "en construccion",
    "en construccion", "en construccion", "en construccion"
};
const char *celeridad [11] =
{
    "", "Celeridad", "Celeridad", "Celeridad", "Celeridad", "Celeridad", "en contruccion", "en contruccion", "en contruccion", "en contruccion",
    "en contruccion"
};
const char *usoceleridad [11] =
{
    "", "Celeridad", "Celeridad", "Celeridad", "Celeridad", "Celeridad", "en contruccion", "en contruccion", "en contruccion", "en contruccion",
    "en contruccion"
};
const char *potencia [11] =
{
    "", "Potencia", "Potencia", "Potencia", "Potencia", "Potencia", "en construccion", "en construccion", "en construccion", "en construccion",
    "en construccion"
};
const char *usopotencia [11] =
{
    "", "Potencia", "Potencia", "Potencia", "Potencia", "Potencia", "en construccion", "en construccion", "en construccion", "en construccion",
    "en construccion"
};
const char *fortaleza [11] =
{
    "", "Fortaleza", "Fortaleza", "Fortaleza", "Fortaleza", "Fortaleza", "en construccion", "en construccion", "en construccion", "en construccion",
    "en construccion"
};
const char *usofortaleza [11] =
{
    "", "Fortaleza", "Fortaleza", "Fortaleza", "Fortaleza", "Fortaleza", "en construccion", "en construccion", "en construccion", "en construccion",
    "en construccion"
};
const char *extincion [11] =
{
    "", "Silencio de la Muerte", "Debilidad", "Enfermedad", "Tormento de la Sangre", "Sabor a Muerte","Sudor de Sangre", "Sanguijuela",
    "Cuajaron de Sangre", "Erosion", "Vitae Inmaculada"
};
const char *usoextincion [11] =
{
    "", "Silenciodelamuerte", "Debilidad <objetivo>", "Enfermedad <objetivo>", "Tormento <objetivo>", "en construccion", "en construccion", "en construccion", "en construccion", "en construccion",
    "en construccion"
};


void do_disciplinas(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    int loop;
    int percent = 0, i;
    int indent = 0;

    if (IS_NPC(ch)) return;

    if ( !IS_VAMPIRE(ch) )
    {
      send_to_char("Hum?.\n\r",ch);
      return;
    }

    if (!str_cmp( argument, "lista" ) )
    {
      for (loop = 1; loop < MAX_DISCIPLINAS -1  ; loop++)
    {
	if (disciplina[loop][0] != '\0'
	&& strlen(disciplina[loop]) != 0 )
	{
	    sprintf(buf2, disciplina[loop]);
	    buf2[0] = UPPER(buf2[0]);

            sprintf(buf,"&g%14.14s",  buf2);
	    send_to_char(buf,ch);
	    indent++;
	    if (indent == 4)
	    {
		send_to_char("\n\r", ch);
		indent = 0;
	    }
            }
         }
        return;
        }
    if (!str_cmp(argument, "animalismo"))
    {
	for(loop =1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		animalismo[loop],
		usoanimalismo[loop]);
	}
	return;
    }
    if (!str_cmp(argument,"dominacion"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		dominacion[loop],
		usodominacion[loop]);
	}
	return;
    }
    if (!str_cmp(argument, "mortis"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf( ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		mortis[loop],
		usomortis[loop]);
	}
	return;
    }
    if (!str_cmp(argument, "obeah"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		obeah[loop],
		usoobeah[loop]);
	}
	return;
    }
    if (!str_cmp(argument, "obtenebracion"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		obtenebracion[loop],
		usoobtenebracion[loop]);
	}
	return;
    }
    if (!str_cmp(argument,"ofuscacion"))
    {
	for(loop=1;loop<11;loop++)
        {
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		ofuscacion[loop],
		usoofuscacion[loop]);
	}
	return;
    }
    if (!str_cmp(argument,"presencia"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,	
		presencia[loop],
		usopresencia[loop]);
	}
	return;
    }
    if (!str_cmp(argument,"protean"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		protean[loop],
		usoprotean[loop]);
	}
	return;
	
    }
    if(!str_cmp(argument,"quimerismo"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		quimerismo[loop],
		usoquimerismo[loop]);
	}
	return;
    }
    if(!str_cmp(argument,"serpentis"))
    {
	for(loop=1;loop<11;loop++)
       	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		serpentis[loop],
		usoserpentis[loop]);
	}
	return;
    }
    if(!str_cmp(argument,"vicisitud"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		vicisitud[loop],
		usovicisitud[loop]);
	}
	return;
    }
    if(!str_cmp(argument,"valeren"))
    {
	for(loop=1;loop<11;loop++)
	{
		ch_printf(ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
		loop,
		valeren[loop],
		usovaleren[loop]);
	}
	return;
    }  
    if (!str_cmp(argument, "taumaturgia" ))
    {
        for(loop = 1 ; loop < 11; loop++)
        {
                ch_printf( ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
                loop,
                taumaturgia[loop],
                usotauma[loop]);
        }
        /*send_to_char( "\n\r&gEscribe disciplina taumaturgia <nombre nivel> para saber mas a cerca de cada uno de ellos.\n\r", ch );*/
        return;
    }

    if (!str_cmp(argument, "dementacion" ))
    {
        for(loop = 1 ; loop < 11; loop++)
        {
                ch_printf( ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
                loop,
                dementacion[loop],
                usodemen[loop]);
        }
        /*send_to_char( "\n\r&gEscribe disciplina taumaturgia <nombre nivel> para saber mas a cerca de cada uno de ellos.\n\r", ch );*/
        return;
    }

    if (!str_cmp(argument, "celeridad" ))
    {
        for(loop = 1 ; loop < 11; loop++)
        {
                ch_printf( ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
                loop,
                celeridad[loop],
                usoceleridad[loop]);
        }
        /*send_to_char( "\n\r&gEscribe disciplina taumaturgia <nombre nivel> para saber mas a cerca de cada uno de ellos.\n\r", ch );*/
        return;
    }

    if (!str_cmp(argument, "potencia" ))
    {
        for(loop = 1 ; loop < 11; loop++)
        {
                ch_printf( ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
                loop,
                potencia[loop],
                usopotencia[loop]);
        }
        /*send_to_char( "\n\r&gEscribe disciplina taumaturgia <nombre nivel> para saber mas a cerca de cada uno de ellos.\n\r", ch );*/
        return;
    }

    if (!str_cmp(argument, "auspex" ))
    {
        for(loop = 1 ; loop < 11; loop++)
        {
                ch_printf( ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
                loop,
                auspex[loop],
                usoauspex[loop]);
        }
        /*send_to_char( "\n\r&gEscribe disciplina taumaturgia <nombre nivel> para saber mas a cerca de cada uno de ellos.\n\r", ch );*/
        return;
    }

    if (!str_cmp(argument, "fortaleza" ))
    {
        for(loop = 1 ; loop < 11; loop++)
        {
                ch_printf( ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
                loop,
                fortaleza[loop],
                usofortaleza[loop]);
        }
        /*send_to_char( "\n\r&gEscribe disciplina taumaturgia <nombre nivel> para saber mas a cerca de cada uno de ellos.\n\r", ch );*/
        return;
    }

    if (!str_cmp(argument, "extincion" ))
    {
        for(loop = 1 ; loop < 11; loop++)
        {
                ch_printf( ch, "&gNivel (&w%-2.2d&g): &w%-25.25s&g Uso: &w%s\n\r",
                loop,
                extincion[loop],
                usoextincion[loop]);
        }
        /*send_to_char( "\n\r&gEscribe disciplina taumaturgia <nombre nivel> para saber mas a cerca de cada uno de ellos.\n\r", ch );*/
        return;
    }


    /*if (IS_CLASS(ch, CLASS_WEREWOLF))
        sprintf(buf, "---===[ GuaRDiaNeS ]===---");
    else*/
        sprintf(buf, "&x&g[ DiSCiPLiNaS DeL CLaN %s]", class_table[ch->class]->who_name);


    cent_to_char(buf, ch);
    send_to_char("\n\r", ch);

    for (loop = 1; loop < MAX_DISCIPLINAS -1  ; loop++)
    {
	if (disciplina[loop][0] != '\0'
	&& strlen(disciplina[loop]) != 0
	&& ch->pcdata->habilidades[loop] > 0 )
	{
	    sprintf(buf2, disciplina[loop]);
	    buf2[0] = UPPER(buf2[0]);

            if(xIS_SET(ch->act, PLR_ANSI))
            {
                if ( ch->pcdata->habilidades[loop] == 1 )
                sprintf( buf3, "&W(&R0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&g" );
                if ( ch->pcdata->habilidades[loop] == 2 )
                sprintf( buf3, "&W(&R0&W)&W(&R0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&g" );
                if ( ch->pcdata->habilidades[loop] == 3 )
                sprintf( buf3,  "&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&g");
                if ( ch->pcdata->habilidades[loop] == 4 )
                sprintf( buf3,  "&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&g" );
                if ( ch->pcdata->habilidades[loop] == 5 )
                sprintf( buf3,  "&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&g");
                if ( ch->pcdata->habilidades[loop] == 6 )
                sprintf( buf3,  "&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&g");
                if ( ch->pcdata->habilidades[loop] == 7 )
                sprintf( buf3,  "&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&w0&W)&W(&w0&W)&W(&w0&W)&g");
                if ( ch->pcdata->habilidades[loop] == 8 )
                sprintf( buf3,  "&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&w0&W)&W(&w0&W)&g");
                if ( ch->pcdata->habilidades[loop] == 9 )
                sprintf( buf3,  "&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&w0&W)&g");
                if ( ch->pcdata->habilidades[loop] >= 10 )
                sprintf( buf3,  "&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&W(&R0&W)&g");


	    sprintf(buf,"%14.14s: %s",  buf2, buf3);
	    send_to_char(buf,ch);
            indent++;
	    if (indent == 1)
	    {
		send_to_char("\n\r", ch);
		indent = 0;
	    }
            }
            else
           {
                if ( ch->pcdata->habilidades[loop] == 1 )
                sprintf( buf3, "(0)( )( )( )( )( )( )( )( )( )&g" );
                if ( ch->pcdata->habilidades[loop] == 2 )
                sprintf( buf3, "(0)(0)( )( )( )( )( )( )( )( )&g" );
                if ( ch->pcdata->habilidades[loop] == 3 )
                sprintf( buf3, "(0)(0)(0)( )( )( )( )( )( )( )&g" );
                if ( ch->pcdata->habilidades[loop] == 4 )
                sprintf( buf3, "(0)(0)(0)(0)( )( )( )( )( )( )&g" );
                if ( ch->pcdata->habilidades[loop] == 5 )
                sprintf( buf3, "(0)(0)(0)(0)(0)( )( )( )( )( )&g" );
                if ( ch->pcdata->habilidades[loop] == 6 )
                sprintf( buf3, "(0)(0)(0)(0)(0)(0)( )( )( )( )&g" );
                if ( ch->pcdata->habilidades[loop] == 7 )
                sprintf( buf3, "(0)(0)(0)(0)(0)(0)(0)( )( )( )&g" );
                if ( ch->pcdata->habilidades[loop] == 8 )
                sprintf( buf3, "(0)(0)(0)(0)(0)(0)(0)(0)( )( )&g" );
                if ( ch->pcdata->habilidades[loop] == 9 )
                sprintf( buf3, "(0)(0)(0)(0)(0)(0)(0)(0)(0)( )&g" );
                if ( ch->pcdata->habilidades[loop] >= 10 )
                sprintf( buf3, "(0)(0)(0)(0)(0)(0)(0)(0)(0)(0)&g" );


	    sprintf(buf,"%14.14s: %s",  buf2, buf3);
	    send_to_char(buf,ch);
            indent++;
	    if (indent == 1)
	    {
		send_to_char("\n\r", ch);
		indent = 0;
	    }
           }
	}
    }
    if(xIS_SET(ch->act, PLR_ANSI))
           send_to_char( "\n\r&gRecuerda: &W(&R0&w) &gtienes el nivel. &W(&w0&w)&g no tienes el nivel.\n\r", ch );
           else
           send_to_char( "\n\r&gRecuerda: (0) tienes el nivel. ( )&g no tienes el nivel.\n\r", ch );

    send_to_char( "\n\r\n\rDisciplinas lista para ver una lista de todas las disciplinas\n\r", ch );

    if (ch->pcdata->disci_adquirir < 0)
    {
	send_to_char("\n\r", ch);
	return;
    }

    if (ch->pcdata->habilidades[ch->pcdata->disci_adquirir] < 0)
    {
	send_to_char("\n\r", ch);
	return;
    }

    if (ch->pcdata->disci_puntos == 999)
    {
	send_to_char("\n\r", ch);
	sprintf(buf, "&rHas terminado de asimilar &w%s&r.",
	disciplina[ch->pcdata->disci_adquirir]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);
	return;
    }

    if (ch->pcdata->disci_adquirir < MAX_DISCIPLINAS)
    {
	send_to_char("\n\r", ch);
	sprintf(buf, "&gEstas asimilando el conociemiento de &w%s&g.",
	    disciplina[ch->pcdata->disci_adquirir]);
    	send_to_char(buf, ch);
    }

    percent = ch->pcdata->disci_puntos * 40 / disci_puntos_nivel(ch);

    sprintf(buf, "&g  -[&o");

    for( i = 0 ; i < percent ; i++)
	strcat(buf, "&w|");

    for( i = percent ; i < 40 ; i++)
	strcat(buf, " ");

    strcat(buf, "&g]-");

    send_to_char(buf, ch);
}


/*
 * Taumaturgia
 */

void do_creo( CHAR_DATA *ch, char *argument )
{
        char buf[MAX_STRING_LENGTH];
        void *vo;
        NIVEL *nivel;
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        OBJ_DATA *obj_lose;
        OBJ_DATA *obj_next;
        int dam;
        int hpch;
        int level;
        int dt;


        if (IS_NPC(ch) )
        {
            if (!IS_IMMORTAL(ch) )
            {
                send_to_char( "Los mobs no pueden usar disciplinas.\n\r", ch );
                return;
             }
             else
             {
             send_to_char( "Usar disciplinas switcheado puede hacer que el mud caiga olvidalo.\n\r", ch );
             return;
             }
         }

         if ( argument[0] == '\0'
         && !ch->fighting )
         {
                send_to_char( "Creo Ignem a quien?.\n\r" , ch );
                return;
         }

         if ( argument[0] == '\0' && (victim=who_fighting(ch)) != NULL )
        argument = victim->name;

         if (!IS_VAMPIRE(ch)
         && !IS_IMMORTAL(ch) )
         {
                send_to_char( "No eres Cainita busca otros conocimientos y deja Taumaturgia para los Tremere.\n\r", ch );
                return;
          }

   if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
    send_to_char( "No esta aqui.\n\r", ch );
    return;
    }

nivel = get_nivel( "creo" );

if( lanzar_disciplina( ch, victim, nivel ))
{
    if ( chance(ch, 12 * level) && !saves_breath( level, victim ) )
    {
       	for ( obj_lose = victim->first_carrying; obj_lose;
	      obj_lose = obj_next )
	{
	    char *msg;

	    obj_next = obj_lose->next_content;
	    if ( number_bits( 2 ) != 0 )
		continue;

	    switch ( obj_lose->item_type )
	    {
	    default:             continue;
	    case ITEM_CONTAINER: msg = "$p se quema!";   break;
	    case ITEM_POTION:    msg = "$p hierve y revienta!";   break;
	    case ITEM_SCROLL:    msg = "$p se quema!";  break;
	    case ITEM_STAFF:     msg = "$p humea y se rompe!";    break;
	    case ITEM_WAND:      msg = "$p humea y revienta!"; break;
	    case ITEM_COOK:
	    case ITEM_FOOD:      msg = "$p se carboniza!"; break;
	    case ITEM_PILL:      msg = "$p se carboniza!";     break;
	    }

	    separate_obj( obj_lose );
	    act( AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR );
	    if ( obj_lose->item_type == ITEM_CONTAINER )
	    {
		act( AT_OBJECT, "El contenido de $p transportado por $N se esparce en el suelo.",
		   victim, obj_lose, victim, TO_ROOM );
		act( AT_OBJECT, "El contenido de $p se esparce en el suelo!",
		   victim, obj_lose, NULL, TO_CHAR );
		empty_obj( obj_lose, NULL, victim->in_room );
	    }
	    extract_obj( obj_lose );
	}
    }

    hpch = UMAX( 10, ch->hit + ch->move );
    dam  = number_range( hpch/12, hpch/5 );
    //dam = hpch;
    if ( saves_breath( level, victim ) )
	dam /= 2;
    if( ch->class != CLASS_TREMERE )
        dam -= ( dam * 0.3);
        dt = 19;
    if( IS_VAMPIRE(victim)) { dam= dam*2; } 
damage( ch, victim, dam, 19 );
if( xIS_SET(ch->afectado_por, DAF_CELERIDAD) && ch->pcdata->habilidades[DISCIPLINA_CELERIDAD] >= 6 )
{
if ( char_died(victim) )
      return;

lanzar_disciplina(ch, victim, nivel );
dam += number_range( -100, 100 );
damage( ch, victim, dam, 19 );
} /* Cierra if( lanzar_disciplina( ch, victim, nivel)) */

}


 return;

  }

void do_caldero( CHAR_DATA *ch, char *argument )
{
        NIVEL *nivel;
        char buf[MAX_STRING_LENGTH];
        void *vo;
        int dt;
        CHAR_DATA *victim = (CHAR_DATA *) vo;
        int dam;
        int level;
        int cald;

        if (IS_NPC(ch) )
        {
            if (!IS_IMMORTAL(ch) )
            {
                send_to_char( "Los mobs no pueden usar disciplinas.\n\r", ch );
                return;
             }
             else
             {
             send_to_char( "Usar disciplinas switcheado puede hacer que el mud caiga olvidalo.\n\r", ch );
             return;
             }
         }

         if ( ch->pcdata->habilidades[DISCIPLINA_TAUMATURGIA] < 2 && !IS_IMMORTAL(ch) )
         {
                send_to_char( "No conoces ese nivel de Taumaturgia\n\rAyuda 'TAUMATURGIA'\n\r", ch );
                return;
         }

         if (!IS_VAMPIRE(ch) && !IS_IMMORTAL(ch) )
         {
                send_to_char( "No eres Cainita busca otros conocimientos y deja Taumaturgia para los Tremere.\n\r", ch );
                return;
          }

         if ( argument[0] == '\0' && (victim=who_fighting(ch)) == NULL  )
         {
                send_to_char( "Caldero Sanguineo a quien?.\n\r" , ch );
                return;
         }

         if  (argument[0] == '\0' && (victim=who_fighting(ch)) != NULL )
        	argument = victim->name;

   if ( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
   	send_to_char( "No esta aqui.\n\r", ch );
   	return;
   }
   nivel = get_dnivel( 2902 );
   if( lanzar_disciplina( ch, victim, nivel ) )
   {
   	if ( ch->level < 30 ) {
    		cald = UMAX( 10, ch->pcdata->condition[COND_BLOODTHIRST] * (get_curr_lck(ch) ) + ch->hitroll + 10*ch->level*(13/ch->generacion) );
    	}else{
    		cald = UMAX( 10, ch->pcdata->condition[COND_BLOODTHIRST] * ch->pcdata->habilidades[DISCIPLINA_TAUMATURGIA]  * (get_curr_lck(ch) ) + ch->hitroll + 10*ch->level*(13/ch->generacion) ); }
    	dam  = number_range( cald/6, cald/3 );

   	if ( saves_breath( level, victim ) )
        	dam /= 2;
        if (IS_VAMPIRE(victim)) { dam= dam*100/125;}
	if (IS_NPC(victim)) {dam = dam*125/100;}
	damage( ch, victim, dam, 19 );	
    }
    return;
}



