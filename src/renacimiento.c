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
 *                          Modulo para el renacimiento de jugadores        *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#include "mud.h"


void do_renacer( CHAR_DATA *ch,  char *argument )
{
        char buf[MAX_STRING_LENGTH];

        if( argument[0] == '\0' )
        {
                send_to_char( "Renacer <password>.\n\r", ch );
                return;
        }

        if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) )
        {
                send_to_char( "Password erroneo!\n\r", ch );
                WAIT_STATE(ch, 40 );
                return;
        }

        if ( IS_IMMORTAL( ch ) )
        {
                send_to_char( "&YLos inmortales no pueden hacer remort melon.\n\r", ch );
                return;
        }

        if ( IS_NPC( ch ) )
        {
                send_to_char( "Los mobs no pueden renacer.\n\r", ch );
                return;
        }

        if ( !ES_ANCIANO( ch ) )
        {
                ch_printf(ch, "No tienes edad suficiente para renacer, deberas esperar %d anyos aun.\n\r", EDAD_REMORT - get_age(ch) );
                ch_printf(ch, "Si los DIOSES se apiadan de ti kizas puedas renacer habla con ellos.\n\r" );
                return;
         }

         if ( ch->level < LEVEL_HERO )
         {
                send_to_char( "Esperate a ser heroe al menos, no seas cobarde.\n\r", ch );
                return;
         }

         renacimiento( ch );
     }


void renacimiento( CHAR_DATA *ch )
{
       char buf[MAX_STRING_LENGTH];
       OBJ_DATA *obj, *obj_next;
       AFFECT_DATA *paf;

       int affected_by = 0;
       int sn;
       int bucle;



           /* Eliminamos los afects del jugador */
          for ( paf = ch->first_affect; paf; paf = paf->next )
                if ( xIS_SET(paf->bitvector, affected_by) )
                        break;
          if ( paf )
                affect_remove ( ch, paf );

          /* Eliminamos los afects de disciplinas */
          while ( ch->primer_affect )
                disciplina_remove( ch, ch->primer_affect );

         /* Escaneamos su ficha para encontrar su equipo y le desvestimos */
      for ( obj = ch->first_carrying; obj != NULL ; obj = obj_next )
      {
        obj_next = obj->next_content;
        if ( obj->wear_loc != WEAR_NONE )
          remove_obj ( ch, obj->wear_loc, TRUE );
      }

      /* Se ajustan los valores */

      /* Te aumenta en uno su remort */

      ch->pcdata->renacido ++;
      bug ( "%s ha efectuado su renacimiento numero %d.\n\r", ch->name, ch->pcdata->renacido );

      /* Se le pone el PLR_RENACIDO */
      xSET_BIT(ch->act, PLR_RENACIDO);

      /* Vida Mana y Move */
      ch->pcdata->vida_ant = ch->max_hit;       /* La vida ke tenia el jugador antes del remort */

      if( IS_VAMPIRE(ch))
      ch->pcdata->mana_ant = ch->level * number_range( 50, 55 );  /* El mana que tenia el jugador antes del remort si es vampiro */
      else
      ch->pcdata->mana_ant = ch->max_mana; /* El mana que tenia el jugador antes del remort */

      ch->pcdata->mov_ant = ch->max_move;  /* El movimiento que tenia el jugador antes del remort */

       /* Stats */
       ch->perm_wis = 15 + ch->pcdata->renacido;
       ch->perm_str = 15 + ch->pcdata->renacido;
       ch->perm_int = 15 + ch->pcdata->renacido;
       ch->perm_dex = 15 + ch->pcdata->renacido;
       ch->perm_con = 15 + ch->pcdata->renacido;
       ch->perm_cha = 15 + ch->pcdata->renacido;
       ch->perm_lck = 15 + ch->pcdata->renacido;

       /* Nivel */
       ch->level = 2; /* oleeeeeeeeeeeeeeeeeeeeee */

       /* Experiencia */
       ch->exp = (100 + exp_level( ch, ch->level));

       /* Generacion */
       if ( ch->generacion != 1 )
       ch->generacion = 13;

       ch->pcdata->genepkills = 0;

       /* Clase */
       ch->class = CLASS_WARRIOR;

       /* Familia */
       if( IS_CLANNED(ch))
       {
       ch->pcdata->clan->members -= 1;

       /* Se elimina del cargo de familia en caso de tenerlo */

       /* Lider */
       if( !str_cmp( ch->pcdata->clan->leader, ch->name ))
       ch->pcdata->clan->leader = STRALLOC( "Desocupado" );

       /* Heredero */
       if( !str_cmp( ch->pcdata->clan->number1, ch->name ))
       ch->pcdata->clan->leader = STRALLOC( "Desocupado" );

       /* Consejero */
       if( !str_cmp( ch->pcdata->clan->number2, ch->name ))
       ch->pcdata->clan->leader = STRALLOC( "Desocupado" );

       save_clan( ch->pcdata->clan );
       ch->pcdata->clan = NULL;
       }

       /* Oro */

       ch->gold = 10;

       /* Titulo */
       sprintf(buf, " eL ReNaCiDo..." );
       set_title( ch, buf );

       /* Vida Mana y Move de salida */
       ch->max_mana = 100 * ch->pcdata->renacido;
       ch->max_hit  = 100 * ch->pcdata->renacido;
       ch->max_move = 100 * ch->pcdata->renacido;

       /* Skills y Spells */
       for ( sn = 0; sn < top_sn; sn++ )
        ch->pcdata->learned[sn] = 0;

       /* Se restaura al jugador */
       ch->hit = ch->max_hit;
       ch->mana = ch->max_mana;
       ch->move = ch->max_move;

       /* Se le otorgan 20 practicas de salida */
       ch->practice = 20;

       /* Se restauran diversos valores */
            ch->armor		 += race_table[ch->race]->ac_plus;
            ch->alignment	 += race_table[ch->race]->alignment;
            ch->attacks              = race_table[ch->race]->attacks;
            ch->defenses             = race_table[ch->race]->defenses;
	    ch->saving_poison_death  	= race_table[ch->race]->saving_poison_death;
	    ch->saving_wand  		= race_table[ch->race]->saving_wand;
	    ch->saving_para_petri  	= race_table[ch->race]->saving_para_petri;
	    ch->saving_breath  		= race_table[ch->race]->saving_breath;
	    ch->saving_spell_staff	= race_table[ch->race]->saving_spell_staff;

	    ch->height = number_range(race_table[ch->race]->height *.9, race_table[ch->race]->height *1.1);
	    ch->weight = number_range(race_table[ch->race]->weight *.9, race_table[ch->race]->weight *1.1);

       /* Se devuelve al jugador a la adolescencia xD */
       ch->played = 0;

       /* Se pone max level al jugador komo 2 */
       ch->pcdata->max_level = ch->level;

       /* Se le quitan las disciplinas a los que fueron vampiros
          o las esferas a magos y clerigos */

        for( bucle = 0; bucle; bucle = ( 10 + ch->pcdata->renacido ))
                ch->pcdata->habilidades[bucle] = 0;



       /* Mensajitos de rigor */
       if ( IS_VAMPIRE(ch) )
       {
                send_to_char( "La pena se apodera de ti, decides acabar con tu inmortalidad cainita.\n\r", ch );
                send_to_char( "Realizas un poderoso ritual Cainita, La Misa Roja recuperas tu mortalidad.\n\r", ch );
                send_to_char( "Durante unos escasos segundos sientes como tu corazon vuelve a bombear sangre...\n\r", ch );
                send_to_char( "Como si estuvieras vivo....  eres consciente de las atrocidades cometidas\n\r", ch );
                send_to_char( "y mueres agonicamente... cuando despiertes todo sera diferente...", ch );
         }
         else;
         {
               send_to_char( "Te sientes realmente extranyo, como si fueras a explotar....\n\r", ch );
               send_to_char( "...ooooooh Dios mio!!! revientas como un globo, que asco todo\n\r", ch );
               send_to_char( "lleno de tus trocitos por todas partes...\n\r", ch );
               send_to_char( "\n\rReNaCeS De TuS TRoCiToS CuaL aVe FeNiX!!\n\r", ch );
           }
      ch->position = POS_SLEEPING;
      sprintf( buf, "%s &gHa ReNaCiDo!!!", ch->name );
      mensa_todos( ch, "vampiro" , buf );
         return;
        }

void advanceLevelRenacidos( CHAR_DATA *ch )
{
	/*!
	Esta funcion no es para que los renacidos avancen de nivel, mas exactamente <br>
	es para que la ganancia <b>se corresponda</b>.
	*/
	int add_mana;
	int add_hp;
	int add_move;
	int add_prac;

	add_mana = (ch->pcdata->mana_ant / LEVEL_HERO -1);
    add_hp = (ch->pcdata->vida_ant / LEVEL_HERO -1);
    add_move = (ch->pcdata->mov_ant / LEVEL_HERO -1);
    add_prac	= wis_app[get_curr_wis(ch)].practice;

	return;
}
