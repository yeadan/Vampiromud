 /*
 * This is a new automated arena for Smaug 1.4.
 * You can do anything you want with this code, I hope it will be
 * compatible with the DOS version.
 * Modificado revisado y mejorado aun mas si cabe por implementadore
 * de VampiroMud vampiromud@hotmail.com XD
 *
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"

#define PREP_START  42   /* vnum of first prep room */
#define PREP_END    43   /* vnum of last prep room */
#define ARENA_START number_range( 29, 41)    /* vnum of first real arena room*/
#define ARENA_END   41   /* vnum of last real arena room*/
#define HALL_FAME_FILE  SYSTEM_DIR "asesinos.lst"

struct hall_of_fame_element
{
   char name[MAX_INPUT_LENGTH+1];
   time_t date;
   int award;
   struct  hall_of_fame_element *next;
};

char * const premio_txt[]=
{
  "nada", "vida", "mana", "move", "vida/mana/move", "oro", "practicas", "puntos quest", 
  "px", "nivel", "genepk's"
};


void sportschan(char *);
void torneo_mortales( CHAR_DATA *ch, char *argument);
void start_arena();
void show_jack_pot();
void do_game();
int num_in_arena();

void do_end_game();
void start_game();
void silent_end();
void write_fame_list(void);
void write_one_fame_node(FILE * fp, struct hall_of_fame_element * node);
void load_hall_of_fame(void);
void find_bet_winners(CHAR_DATA *winner);
void plr_torn( CHAR_DATA *ch, short int accion );
struct hall_of_fame_element *fame_list = NULL;

char *plr[100];
bool iniciado;


int ppl_challenged = 0;
int ppl_in_arena = 0;
int in_start_arena = 0;
int start_time;
int game_length;
int lo_lim;
int hi_lim;
int premio = 0;
int time_to_start;
int time_left_in_game;
int arena_pot = 0;
int bet_pot = 0;
int barena = 0;

extern int parsebet (const int currentbet, char *s);
extern int advatoi (char *s);

void do_bet(CHAR_DATA *ch, char *argument)
 {
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   char buf1[MAX_INPUT_LENGTH];
   int newbet;

   argument = one_argument(argument,arg);
   one_argument(argument,buf1);

   if (IS_NPC(ch))
   {
      send_to_char("Anda yaaaa!!! Los Mobs no pueden apostar.\r\n",ch);
      return;
   }

   if(arg[0]=='\0')
   {
      send_to_char("&gUso: &wapostar <jugador> <cantidad>&g\r\n",ch);
      return;
   }
   else if(!in_start_arena && !ppl_challenged)
   {
      send_to_char("Lo siento no hay nadie en torneo, deberas esperar otro reto.\r\n", ch);
      return;
   }
   else if(ppl_in_arena)
   {
      send_to_char("Lo siento el Torneo ha empezado, apuesta en otro mas tarde.\r\n", ch);
      return;
   }
   else if (!(ch->betted_on = get_char_world(ch, arg)))
      send_to_char("No hay nadie asi en el torneo.", ch);
   else if (ch->betted_on == ch)
      send_to_char("Apostar por ti mismo, en serio quieres hacerlo?\r\n", ch);
   else if(!(IS_SET(ch->betted_on->in_room->room_flags, ROOM_ARENA)))
      send_to_char("Pero que dices? Ese no esta en el Torneo.\r\n", ch);
   else
      {
         if(GET_BET_AMT(ch) > 0)
         {
           send_to_char("Lo siento ya has hecho tu apuesta.\r\n", ch);
           return;
         }
         GET_BETTED_ON(ch) = ch->betted_on;
         newbet=parsebet(bet_pot,buf1);
         if(newbet == 0)
         {
            send_to_char("Seras tacanyo, apuesta una cantidad aceptable!\r\n", ch);
            return;
         }
         if (newbet > ch->gold)
         {
            send_to_char("No tienes tanto dinero!\n\r",ch);
            return;
         }
         if(newbet > 10000000)
         {
            send_to_char("Lo siento la casa no apuesta tanto dinerito.\r\n", ch);
            return;
         }

         ch->gold -= newbet;
         arena_pot += (newbet / 2);
         bet_pot += (newbet / 2);
         GET_BET_AMT(ch) = newbet;
         sprintf(buf, "&gApuestas %d monedas por &w%s.\r\n", newbet, ch->betted_on->name);
         send_to_char(buf, ch);
         sprintf(buf,"%s ha apostado %d monedas por %s.", ch->name,
         newbet, ch->betted_on->name);
	 mensa_todos( ch, "Torneo", buf );
  }
}

void do_arena(CHAR_DATA *ch, char *argument)
{
 char buf[MAX_INPUT_LENGTH];

 if (IS_NPC(ch))
 {
   send_to_char("Los Mobs no pueden competir en Torneos.\r\n",ch);
   return;
 }

 if(!in_start_arena)
 {
   send_to_char("&gEl Campo de Batalla esta &R(&wcerrado&R)&g.\r\n", ch);
   return;
 }

 if(ch->level < lo_lim)
 {
   sprintf(buf, "&gLo siento debes de superar el nivel minimo &g%d &wpara participar en este torneo.\r\n", lo_lim);
   send_to_char(buf, ch);
   return;
 }

 if( ch->level > hi_lim)
 {
    send_to_char("Pero que dices? Tienes demasiado nivel.\n\r", ch);
    return;
 }

 if(IS_SET(ch->in_room->room_flags, ROOM_ARENA))
 {
    send_to_char("Ya te has inscrito en el torneo, borrico\r\n",ch);
    return;
 }
 else
 {
    act(AT_RED, "$n se ha inscrito en el Torneo, desaparece envuelto en odio.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, get_room_index(PREP_START));
    act(AT_WHITE,"$n aparece de la nada, su cara irradia odio y destruccion.", ch, NULL, NULL, TO_ROOM);
    send_to_char("Has llegado al Campo de Batalla.\r\n",ch);
    do_look(ch, "auto");
    sprintf(buf, "%s ha aceptado el Torneo de Sangre.", ch->name);
    mensa_todos( ch, "Torneo", buf );
    send_to_char(buf, ch);
    plr_torn(ch, 2 );
    ch->hit = ch->max_hit;
    ch->mana = ch->max_mana;
    ch->move = ch->max_move;
    return;
  }
}

void do_chaos(CHAR_DATA *ch, char *argument)
{
  char lolimit[MAX_INPUT_LENGTH];
  char hilimit[MAX_INPUT_LENGTH], start_delay[MAX_INPUT_LENGTH];
  char length[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];

  /*Usage: chaos lo hi start_delay cost/lev length*/

  if(!IS_IMMORTAL(ch))
  {
     premio = number_range(1, 10);
     torneo_mortales(ch, argument);
     return;
  } 
  
  argument = one_argument(argument, lolimit);
  lo_lim = atoi(lolimit);
  argument = one_argument(argument, hilimit);
  hi_lim = atoi(hilimit);
  argument = one_argument(argument, start_delay);
  start_time = atoi(start_delay);
  one_argument(argument, length);
  game_length = atoi(length);
  
  premio = number_range(1, 10);

  sprintf(buf,"LowLim %d HiLim %d Delay %d Length %d\n\r", lo_lim,
         hi_lim, start_time, game_length);
  send_to_char(buf,ch);

  if ( IS_NPC( ch))
  {
   send_to_char("Los Mobs no pueden organizar Torneos.\n\r", ch);
   return;
  }

  if(hi_lim >= MAX_LEVEL +1)
  {
     send_to_char("Por favor introduce un max_level por debajo del de los implementadores (95)\r\n", ch);
     return;
  }

  if(!*lolimit || !*hilimit || !*start_delay || !*length)
  {
    send_to_char("Uso: torneo bajo alto inicio_delay duracion\n\r", ch);
    send_to_char("alto y Bajo: nivel participante (15 - 95)\n\r", ch);
    send_to_char("inicio_delay: Tiempo hasta el comienzo del Torneo.\n\r", ch);
    send_to_char("duracion: Duracion del Torneo\n\r", ch);
    return;
  }

  if (lo_lim >= hi_lim)
  {
    send_to_char("Pero tu estas tonto, solo para un rango?.\r\n", ch);
    return;
  }

  if ((lo_lim || hi_lim || game_length) < 0 )
  {
    send_to_char("Me da a mi ke no hay niveles negativos he...\r\n", ch);
    return;
  }

  if ( start_time <= 0)
  {
    send_to_char("Entonces deberia de haber empezado ya!\r\n", ch);
    return;
  }

  if ( in_start_arena  )
    {
      ch_printf( ch, "Ya se ha declarado un torneo, espera a que acabe.\n\r" );
      return;
    }
  plr_torn( NULL, 1);
  ppl_in_arena = 0;
  in_start_arena = 1;
  time_to_start = start_time;
  time_left_in_game =0;
  arena_pot =0;
  bet_pot = 0;
  barena = 1;
  start_arena();

}


void start_arena()
{
  char buf1[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;

 if (!(ppl_challenged))
 {
    if(time_to_start == 0)
    {
      in_start_arena = 0;
      show_jack_pot();
      ppl_in_arena = 1;    /* start the blood shed */
      time_left_in_game = game_length;
      start_game();
    }
    else
    {
      if(time_to_start >1)
      {
         sprintf(buf1, "Torneo de Sangre para niveles %d a %d",lo_lim, hi_lim);
	 mensa_todos( NULL, "Torneo", buf1 );
         sprintf(buf1, "Faltan %d horas para que comience la matanza", time_to_start);
         mensa_todos( NULL, "Torneo", buf1 );
         sprintf(buf1, "El premio sera %s", premio_txt[premio] );
         mensa_todos( NULL, "Torneo", buf1 );
         mensa_todos( NULL, "Torneo", "Escribe inscribir para inscribirte.");
      }
      else
      {
         sprintf(buf1, "Torneo de Sangre para niveles %d a %d",lo_lim, hi_lim);
         mensa_todos( NULL, "Torneo", buf1 );
         sprintf(buf1, "Falta 1 hora para que comienze la matanza" );
         mensa_todos( NULL, "Torneo", buf1 );
      }
      bug("%d %d %d %d", game_length, hi_lim, lo_lim, time_to_start );
      time_to_start--;
    }
  }
  else
  if (!(ppl_in_arena))
  {
    if(time_to_start == 0)
    {
      ppl_challenged = 0;
      show_jack_pot();
      ppl_in_arena = 1;    /* start the blood shed */
      time_left_in_game = 15;
      start_game();
    }
    else
    {
      if(time_to_start >1)
      {
        sprintf(buf1, "El duelo comenzara en %d horas. Haz tus apuestas!", time_to_start);
        sprintf( buf, "El premio sera de %s", premio_txt[premio] );
      }
      else
      {
        sprintf(buf1, "El duelo comenzara en 1 hora. Rapido apuesta!");
        sprintf( buf, "El premio sera de %s", premio_txt[premio]);
      }
      mensa_todos( NULL, "Torneo", buf1 );
      time_to_start--;
    }
  }
}

void start_game()
{
  CHAR_DATA *i;
  DESCRIPTOR_DATA *d;

  for (d = first_descriptor; d; d = d->next)
  if (!d->connected)
  {
    i = d->character;
    if (IS_SET(i->in_room->room_flags, ROOM_ARENA))
    {
       send_to_char("\r\nEl suelo se abre bajo tus pies depositandote en la arena.\r\n", i);
       char_from_room(i);
       char_to_room(i, get_room_index( ARENA_START));
       do_look(i,"auto");
    }
  }
  do_game();
}

void do_game()
{
  char buf[MAX_INPUT_LENGTH];

  bug("Num. Arena: %d Tiempo: %d", num_in_arena(), time_left_in_game);
  if(num_in_arena() == 1)
  {
    ppl_in_arena = 0;
    ppl_challenged = 0;
    find_game_winner();
  }
  else if(time_left_in_game == 0)
  {
    sprintf(buf, "Despues de %d horas de batalla hay un empate!", game_length );
    mensa_todos( NULL, "Torneo", buf ); 
    do_end_game();
  }
  else if(num_in_arena() == 0)
  {
    ppl_in_arena = 0;
    ppl_challenged = 0;
    silent_end();
  }
  else if(time_left_in_game % 5)
  {
     sprintf(buf, "Quedan %d horas para el fin del torneo quedan %d jugadores aun.", time_left_in_game, num_in_arena());
       mensa_todos( NULL, "Torneo", buf );
  }
  else if(time_left_in_game == 1)
  {
    sprintf(buf, "Queda 1 hora para el fin del torneo  quedan %d jugadores.",num_in_arena());
      mensa_todos( NULL, "Torneo", buf );
  }
  else if(time_left_in_game <= 4)
  {
    sprintf(buf, "Quedan %d horas para el fin del torneo quedan %d jugadores aun.",time_left_in_game, num_in_arena());
      mensa_todos( NULL, "Torneo", buf );
  }
  time_left_in_game--;
}

void find_game_winner()
{
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  CHAR_DATA *i;
  DESCRIPTOR_DATA *d;
  struct hall_of_fame_element *fame_node;
  int ganancia;
  int x, y, z, a, b;
  
  for (d = first_descriptor; d; d = d->next)
      {
        i = d->original ? d->original : d->character;
        if (IS_SET(i->in_room->room_flags,ROOM_ARENA)
            && (i->level < LEVEL_IMMORTAL))
        {
          i->hit = i->max_hit;
          i->mana = i->max_mana;
          i->move = i->max_move;
          i->challenged=NULL;
          char_from_room(i);
          char_to_room(i,get_room_index(ROOM_VNUM_TEMPLE));

          if( plr[1] == NULL )
	    {
	      silent_end();
	      return;
	    }

          do_look(i, "auto");
          act(AT_YELLOW,"$n cae desde el cielo.", i, NULL, NULL, TO_ROOM);
          if(time_left_in_game == 1)
          {
             sprintf(buf, "%s se declara vencedor del Torneo antes de una hora para el fin.",i->name);
             mensa_todos( NULL, "Victoria", buf );
          }
          else
          {
             sprintf(buf, "Antes de %d horas para el fin %s se declara vencedor del Torneo.",  game_length - time_left_in_game, i->name);
             mensa_todos( NULL, "Torneo", buf );
          }
          
          premio = 8;
          ganancia = 0;
          x=y=z=a=b=0;
         
          if(premio == 9 && i->level >= LEVEL_HERO)
          premio = 4;
          
          if ( premio <= 0 || premio >= 11)
          premio = number_range(1, 10);
         
          if ( premio == 1)
          {
            ganancia = number_range(1, 20);
            i->max_hit += ganancia;
          }
          else if ( premio == 2)
          {
            ganancia = number_range(1, 20);
            i->max_mana += ganancia;
          }
          else if ( premio == 3)
          {
            ganancia = number_range(1, 30);
            i->max_move += ganancia;
          }
          else if( premio == 4)
          {
           x= number_range(1, 10);
           y= number_range(1, 10);
           z= number_range(1, 15);
           i->max_hit += x;
           i->max_mana += y;
           i->max_move += z;
          }
          else if( premio == 5 )
          {
           ganancia = number_range( i->level * 500, i->level * 9999 );
           i->gold += ganancia;
          }
          else if( premio == 6 )
          {
            ganancia = number_range(1, 10);
            i->practice += ganancia;
          }
          else if( premio == 7)
          {
            ganancia = number_range(10, 100);
            i->pcdata->quest_curr += ganancia;
          }
          else if( premio == 8 )
          {
           ganancia += number_range( i->level, i->level * 7);

           if (i->level <= LEVEL_HERO )
	     gain_exp( i, ganancia );
           else
             i->exp_acumulada += ganancia;
          }
          else if( premio == 9)
             advance_level( i );
          else if( premio == 10)
             i->pcdata->genepkills++;
          else
          {
            bug( "GANANCIA DE TORNEO CANCELADA!!! PREMIO NULO!!" );
            return;
          }
                      
          if(premio != 4)                             
          {
           sprintf(buf, "Has ganado %d %s por tu vistoria en el Torneo", ganancia, premio_txt[premio]);
           mensa_todos( NULL, "Torneo", buf );
           sprintf(buf2, "%s recibe %d %s por su victoria en el Torneo", i->name, ganancia, premio_txt[premio]);
           mensa_todos( NULL, "Torneo", buf2);
          }
          else
          {
           sprintf(buf, "Has ganado %d/%d/%d %s por tu vistoria en el Torneo", x,y,z, premio_txt[premio]);
           mensa_todos( NULL, "Torneo", buf );
           sprintf(buf2, "%s recibe %d/%d/%d %s por su victoria en el Torneo", i->name, x,y,z, premio_txt[premio]);
           mensa_todos( NULL, "Torneo", buf2);
          }
          
          CREATE(fame_node, struct hall_of_fame_element, 1);
          strncpy(fame_node->name, i->name, MAX_INPUT_LENGTH);
          fame_node->name[MAX_INPUT_LENGTH] = '\0';
          fame_node->date = time(0);
          fame_node->award = (arena_pot/2);
          fame_node->next = fame_list;
          fame_list = fame_node;
          write_fame_list();
          find_bet_winners(i);
          ppl_in_arena = 0;
          ppl_challenged = 0;
        }
     }
}

void show_jack_pot()
{
  char buf1[MAX_INPUT_LENGTH];
  
  if( arena_pot != 0)
    sprintf(buf1, "Recaudacion total de las apuestas %d monedas", arena_pot);
  else
    sprintf(buf1, "Nadie ha querido apostar por los jugadores" );
    mensa_todos( NULL, "Apuestas", buf1 );

}

void silent_end()
{
  char buf[MAX_INPUT_LENGTH];
  ppl_in_arena = 0;
  ppl_challenged = 0;
  in_start_arena = 0;
  start_time = 0;
  game_length = 0;
  time_to_start = 0;
  time_left_in_game = 0;
  arena_pot = 0;
  bet_pot = 0;
  sprintf(buf, "No ha habido nadie que haya querido participar en el torneo.");
  mensa_todos( NULL, "Torneo", buf);
  do_end_game();
}

void do_end_game()
{
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *i;
  DESCRIPTOR_DATA *d;

  for (d = first_descriptor; d; d = d->next)
     if (!d->connected)
     {
       i = d->character;
       if (IS_SET(i->in_room->room_flags, ROOM_ARENA))
       {
          i->challenged = NULL;
          stop_fighting(i,TRUE);
          char_from_room(i);
          char_to_room(i, get_room_index( ROOM_VNUM_ALTAR ));
          do_look(i,"auto");
          act(AT_TELL,"$n cae desde el cielo.", i, NULL, NULL, TO_ROOM);
       }
     }
     // sprintf(buf, "Despues de %d horas de batalla hay un empate!",game_length);
     // mensa_todos( NULL, "Torneo", buf );
     time_left_in_game = 0;
     ppl_in_arena=0;
     ppl_challenged = 0;
     i->hit = i->max_hit;
     i->mana = i->max_mana;
     i->move = i->max_move;

     /*while (i->primer_affect )
                        disciplina_remove( i, i->primer_affect );

     while (ES_AFECTADO(i))
        {

	  int critical;
                disciaffect_update( i );
                critical++;
                if (critical > 30000)
                {
                        critical = 0;
                        bug("Error con los affects de disciplina!.\n\r");
                        return;
                }
        }
     while ( i->first_affect )
                affect_remove( i, i->first_affect );
         i->affected_by = race_table[i->race]->affected;*/
	 do_noaffects( i, i->name );
}

int num_in_arena()
{
  CHAR_DATA *i;
  DESCRIPTOR_DATA *d;
  int num = 0;

  for (d = first_descriptor; d; d = d->next)
     {
       i = d->original ? d->original : d->character;
       if (IS_SET(i->in_room->room_flags,ROOM_ARENA))
       {
          if(i->level < LEVEL_IMMORTAL)
          num++;
       }
     }
     return num;
 }

void sportschan(char *argument)
{
  char buf1[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *i;

  sprintf(buf1, "&RInfo: &W%s\r\n", argument);

  for (i = first_descriptor; i; i = i->next)
  {
    if (!i->connected && i->character)
    {
       send_to_char(buf1, i->character);
    }
  }
}

void do_awho(CHAR_DATA *ch, char *argument)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *tch;
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  char buf3[MAX_INPUT_LENGTH];
  int num=num_in_arena();

  if(num==0)
  {
     send_to_char("No hay nadie en la arena ahora mismo.\r\n", ch);
     return;
  }

  sprintf(buf,"&W  Jugadores en la &BNecropolis&W \r\n");
  sprintf(buf,"%s-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-", buf);
  sprintf(buf,"%s&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-\r\n", buf);
  sprintf(buf,"%sDuracion    = &R%-3d  &WHoras Comienzo &R%-3d\r\n", buf, game_length, time_to_start);
  sprintf(buf,"%s&WLimite Nivel &R%d &Wa  &R%d\r\n", buf, lo_lim, hi_lim);
  sprintf(buf,"%s         &WRecaudacion = &R%d\r\n", buf, arena_pot);
  sprintf(buf,"%s&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B", buf);
  sprintf(buf,"%s-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B\r\n", buf);
  send_to_char(buf, ch);
  for (d = first_descriptor; d; d = d->next)
     if (!d->connected)
        {
          tch = d->character;
          if (IS_SET(tch->in_room->room_flags, ROOM_ARENA)
              && (tch->level < LEVEL_IMMORTAL))
          {
             if (tch->pcdata->clan)
             {
               CLAN_DATA *pclan = tch->pcdata->clan;
               strcpy(buf3, pclan->name);
             }
             else strcpy(buf3, "");
             sprintf(buf2, "&W%s         %-11.11s\n\r", tch->name, buf3);
             send_to_char(buf2,ch);
          }
        }
   return;
}

void do_ahall(CHAR_DATA *ch, char *argument)
{
  char site[MAX_INPUT_LENGTH], format[MAX_INPUT_LENGTH], *timestr;
  char format2[MAX_INPUT_LENGTH];
  struct hall_of_fame_element *fame_node;

  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];

  if (!fame_list)
  {
     send_to_char("No hay nadie en el Rancking de asesinos.\r\n", ch);
     return;
  }

     sprintf(buf2, "&B|---------------------------------------|\r\n");
     strcat(buf2, "| &WUltimos vencedores en la &RNecropolis&B |\r\n");
     strcat(buf2, "|---------------------------------------|\r\n\r\n");

     send_to_char(buf2, ch);
     strcpy(format, "%-25.25s  %-10.10s  %-16.16s\r\n");
     sprintf(buf, format,
        "&RNombre",
        "&RFecha",
        "&RPremio");
     send_to_char(buf, ch);
     sprintf(buf, format,
        "&B---------------------------------",
        "&B---------------------------------",
        "&B---------------------------------");

     send_to_char(buf, ch);
     strcpy(format2, "&W%-25.25s  &R%-10.10s  &Y%-16d\r\n");
     for (fame_node = fame_list; fame_node; fame_node = fame_node->next)
     {
        if (fame_node->date)
        {
           timestr = asctime(localtime(&(fame_node->date)));
           *(timestr + 10) = 0;
           strcpy(site, timestr);
        }
     else
        strcpy(site, "NPI");
     sprintf(buf, format2, fame_node->name, site, fame_node->award);
     send_to_char(buf, ch);
     }
     return;
 }

void load_hall_of_fame(void)
{
  FILE *fl;
  int date, award;
  char name[MAX_INPUT_LENGTH + 1];
  struct hall_of_fame_element *next_node;

  fame_list = 0;

  if (!(fl = fopen(HALL_FAME_FILE, "r")))
  {
    perror("Unable to open hall of fame file");
    return;
  }
  while (fscanf(fl, "%s %d %d", name, &date, &award) == 3)
  {
    CREATE(next_node, struct hall_of_fame_element, 1);
    strncpy(next_node->name, name, MAX_INPUT_LENGTH);
    next_node->date = date;
    next_node->award = award;
    next_node->next = fame_list;
    fame_list = next_node;
  }

  fclose(fl);
  return;
}

void write_fame_list(void)
{
  FILE *fl;

  if (!(fl = fopen(HALL_FAME_FILE, "w")))
  {
     bug("Error writing _hall_of_fame_list", 0);
     return;
  }
  write_one_fame_node(fl, fame_list);/* recursively write from end to start */
  fclose(fl);

   return;
}

void write_one_fame_node(FILE * fp, struct hall_of_fame_element * node)
{
  if (node)
  {
    write_one_fame_node(fp, node->next);
    fprintf(fp, "%s %ld %d\n",node->name,(long) node->date, node->award);
  }
}

void find_bet_winners(CHAR_DATA *winner)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *wch;

  char buf1[MAX_INPUT_LENGTH];

  for (d = first_descriptor; d; d = d->next)
    if (!d->connected)
    {
       wch = d->original ? d->original : d->character;
       if ((!IS_NPC(wch)) && (GET_BET_AMT(wch) > 0) && (GET_BETTED_ON(wch) == winner))
       {
          sprintf(buf1, "Has ganado %d coins en tu apuesta.\r\n",(GET_BET_AMT(wch))*2);
          send_to_char(buf1, wch);
          wch->gold += GET_BET_AMT(wch)*2;
          GET_BETTED_ON(wch) = NULL;
          GET_BET_AMT(wch) = 0;
       }
    }
}

void do_challenge(CHAR_DATA *ch, char *argument)
{
 CHAR_DATA *victim;
 char buf[MAX_INPUT_LENGTH];

 if ( !can_see(victim, ch))
 {
   send_to_char( "No veo a nadie asi en el juego.\n\r", ch);
   return;
 }

 if ( ( victim = get_char_world( ch, argument ) ) == NULL)
 {
    send_to_char("&WNo hay nadie asi por aqui!\n\r",ch);
    return;
 }

 if ((ch->level > LEVEL_IMMORTAL + 16) || (victim->level > LEVEL_IMMORTAL +16))
 {
    send_to_char("Lo siento los INMORTALES no pueden participar :P.\n\r",ch);
    return;
 }

 if (IS_NPC(victim))
 {
    send_to_char("&WNo puedes retar Mobs!\n\r",ch);
    return;
 }

 if ( IS_NPC( ch ))
 {
   send_to_char("&WLos Mobs no pueden retar\n\r", ch);
   return;
 }

 if (victim->name == ch->name)
 {
   send_to_char("&WNo puedes retarte a ti mismo!",ch);
   return;
 }

 if (victim->level<5)
 {
   send_to_char("&WEs demasiado joven dejalo.\n\r",ch);
   return;
 }

 if ((!(ch->level-15<victim->level))||(!(ch->level+15>victim->level)))
 {
   send_to_char("&WEsta fuera de tu rango de niveles.\n\r",ch);
   return;
 }

 if (get_timer(victim,TIMER_PKILLED)>0)
 {
   send_to_char("&WFue asesinado en los ultimos 5 minutos !\n\r",ch);
   return;
 }

 if ( get_age( victim ) < 18 || victim->level < 5 )
 {
   send_to_char("Eres demasiado joven para morir.\n\r",ch);
   return;
 }

 if (get_timer(ch,TIMER_PKILLED)>0)
 {
   send_to_char("&WHas sido asesinado en los ultimos 5 minutos no puedes retar a nadie.\n\r",ch);
   return;
 }

 if (num_in_arena()>0)
 {
    send_to_char("&WYa esta en el Torneo!\n\r",ch);
    return;
 }
 sprintf(buf,"&R%s &Wte ha retado a un duelo!\n\r",ch->name);
 send_to_char(buf,victim);
 send_to_char("&WPor favor acepta o rechaza el duelo.\n\r\n\r",victim);
 sprintf(buf,"&R[&PInfo&GReto&R]&G%s &Wha retado a &P%s &Wa combatir en duelo!!\n\r",ch->name,victim->name);
 sportschan(buf);
 victim->challenged = ch;
}

void do_accept(CHAR_DATA *ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];

  if (num_in_arena()>0)
  {
   send_to_char("Por favor espera mientras se cierra el Torneo despues acepta.\n\r",ch);
   return;
  }

  if (!(ch->challenged))
  {
    send_to_char("Si no te han retado!\n\r",ch);
    return;
  }
  else
  {
    CHAR_DATA *dch;
    dch = ch->challenged;
    sprintf(buf,"&R[&PInfo&GReto&R]&G%s &Wha aceptado el duelo de &P%s!\n\r",ch->name,dch->name);
    sportschan(buf);
    ch->challenged = NULL;
    char_from_room(ch);
    char_to_room(ch, get_room_index(PREP_END));
    do_look(ch,"auto");
    char_from_room(dch);
    char_to_room(dch, get_room_index(PREP_START));
    do_look(dch,"auto");
    ppl_in_arena = 0;
    ppl_challenged = 1;
    time_to_start = 3;
    time_left_in_game = 0;
    arena_pot =0;
    bet_pot = 0;
    start_arena();
    return;
   }
}

void do_decline(CHAR_DATA *ch, char *argument)
{
 char buf[MAX_INPUT_LENGTH];

 if (ch->challenged)
 {
   sprintf(buf,"&R[&PInfo&GReto&R]&G%s &Wha &rRECHAZADO &W el reto de &P%s! &WCOBARDE!!!\n\r",ch->name,ch->challenged->name);
   sportschan(buf);
   ch->challenged=NULL;
   return;
 }
 else
 {
   send_to_char("Nadie te ha retado, me recuerdas al Reena ya!\n\r",ch);
   return;
 }
}

void torneo_mortales( CHAR_DATA *ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int precio;
  
  if( !ch )
  {
    bug( "%s:%d -- Funcion torneo_mortales CH NULO!!!", __FILE__, __LINE__ );
    return;
  }
  
  if( in_start_arena )
    {
      ch_printf( ch, "Ya hay un torneo iniciado, esperate a que se acabe!!" );
      return;
    }
  
  precio = ( ch->level * number_range(10, 1000) );
  
  if( ch->gold <= precio )
  {
    ch_printf( ch, "No tienes bastante pasta tio, necesitas ma o menos %d monedas.\n\r", precio );
    return;
  }
  
  sprintf( buf, "%s ha convocado un torneo de sangre", ch->name );
  mensa_todos( ch, "Torneo", buf );
 
  ch_printf(ch, "Le das al juez del torneo %d monedas!!\n\r&rEl juez del torneo te dice&w'Muchas gracias %s'\n\r", precio, ch->name ); 
  
  ppl_in_arena = 0;
  lo_lim = ch->level - number_range(3, 8);
  hi_lim = ch->level + number_range(3, 8);
  in_start_arena = 1;
  game_length = number_range(10, 50);
  time_to_start = number_range(2, 5);
  time_left_in_game = 0;
  arena_pot = 0;
  bet_pot = 0;
  barena = 0;
  start_arena(); 
  
  return;
}
/*
 * Para ver el asunto ese de los jugadores, pa que no pase ne
 * los restaure i tal
 */

void plr_torn( CHAR_DATA *ch, short int accion )
{
  int cnt;
  
  if( accion == 1 )   // Borrar la lista de anteriores torneos
    {
      for( cnt = 0; cnt <= 100; cnt++ )
	{
	  STRFREE( plr[cnt] );
          return;
        }
    }

  else if( accion == 2 ) // Anyade un jugador al torneo cuando se inscribe
    {
      for( cnt = 0; cnt <= 100; cnt++ )
	{
	  if( plr[cnt] == NULL )
	    {
	      plr[cnt] = STRALLOC( ch->name );
              ch_printf( ch, "\n\r&wBienvenido %s!! Estas preparado para la masacre?\n\r", plr[cnt] );
              return;
	    }
	}
    }
  else 
    {
      bug( "%s:%d --> Fallo en la funcion plr_torn", __FILE__, __LINE__ );
      return;
    }
  return;
}
  
  
