/***************************************************************************
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
 *         Nuevo sistema de mejora de ficha del personaje                   *
 *             No soportado por SMAUG 1.4 ni superiores                     *
 *             No soportado por ningun otro tipo de codigo xD               *
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
 *Comando Mejorar obviamente sustituye el Snippet este chungo de ROM que habia
 *para que la gente pudiera entrenar.
 *Solo funciona con VampiroMud2.0b  o superiores XD
 */

void do_mejorar(CHAR_DATA *ch, char *argumento)
{
        char buf[MAX_STRING_LENGTH];
        int gana_mana =0, gana_vida =0, gana_movimiento =0, gana_gnpk =0, gana_prac =0;
        int gana_manipulacion =0, gana_astucia =0, gana_intimidacion = 0;
        int gana_voluntad =0, gana_camino =0, gana_generacion =0;
        int coste_oro =0, coste_px =0, coste_quest =0, coste_prac =0;
        int mensa_aleatorio = 0;
        sh_int gana_stat =0;
        CHAR_DATA *mob;

        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        char arg3[MAX_INPUT_LENGTH];

        bool px;
        bool pq;
        bool oro;
        bool prac;
        bool puede = FALSE;

        argumento = one_argument(argumento, arg1);
        argumento = one_argument(argumento, arg2);
        argumento = one_argument(argumento, arg3);

        if (IS_NPC(ch))
        {
                        send_to_char( "Los mobs no pueden Mejorar sus caracteristicas.\n\r", ch);
                        return;
        }

           for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && xIS_SET(mob->act, ACT_TRAIN) )
            break;
    }

    if ( mob == NULL )
    {
        send_to_char( "No puedes hacerlo aqui busca un entrenador.\n\r", ch );
        return;
    }

    if ( arg1[0] == '\0')
    {
    if (ch->level >= LEVEL_HERO)
    {
     ch_printf(ch,"Tienes %d practicas, %d puntos quest, %s monedas de oro y %d Px.\n\r",
     ch->practice,
     ch->pcdata->quest_curr,
     num_punct(ch->gold),
     ch->exp_acumulada);
    }
    else
    {
     ch_printf(ch,"Tienes %d practicas, %d puntos quest y %s monedas de oro.\n\r",
     ch->practice,
     ch->pcdata->quest_curr,
     num_punct(ch->gold));
    }
    ch_printf(ch, "Uso: mejorar <lista>\n\r     mejorar <atributo> <cantidad|0 = a todo> <modo>\n\r");
    ch_printf(ch, "Nota: La cantidad corresponde a la cantidad de veces que se va a mejorar un atributo\n\r");
    return;
    }
	/* Kayser mejorar practicas */
    if ( !str_prefix(arg1, "practicas"))
    {
    	if ( arg2[0] == '\0')
    	{
    		send_to_char(" Solo puedes mejorar practicas con:\n\r", ch);
 	        send_to_char(" quest, px y oro (sin mas argumentos)\n\r", ch);
    		return;
    	}
    	if (!str_prefix(arg2, "oro") )
    	{	
    		if (ch->gold < 10000000)
		{
			send_to_char(" No tienes suficiente oro para mejorar eso.\n\r", ch);	    
			return;
		}
		else
		{
			send_to_char(" Ganas 10 practicas!\n\r", ch);
			ch->gold -= 10000000;
			ch->practice += 10; 
			return;
		}
    	}
    	else if (!str_prefix(arg2, "px") )
    	{
		if (ch->level < LEVEL_AVATAR)
		{
			send_to_char(" Debes ser heroe para mejorar con px.\n\r", ch);
			return;
		}
		if (ch->exp_acumulada < 1000)
		{
			send_to_char(" No tienes suficiente experiencia.\n\r", ch);
			return;
		}
		else
		{
			send_to_char(" Ganas 10 practicas!\n\r", ch);
			ch->exp_acumulada -= 1000;
			ch->practice += 10;
			return;	
		}
	}
	else if (!str_prefix(arg2, "quest") )
	{
		 if (ch->pcdata->quest_curr < 500)
		 {
		 	send_to_char(" No tienes suficientes puntos de quest.\n\r", ch);
		 	return;
		 }
		 else
		 {
			send_to_char(" Ganas 10 practicas!\n\r", ch);
		 	ch->practice += 10;
		 	ch->pcdata->quest_curr -= 500;
		 	return;
		 }
	}
	send_to_char(" Solo puedes mejorar practicas con:\n\r", ch);
	send_to_char(" quest, px y oro (sin mas argumentos)\n\r", ch);
	return;
    }
    	/* Fin Kayser mejorar practicas */
    if ( !str_prefix(arg1, "lista"))
    {

        ch_printf(ch,"&wAtributo               &g| &wPracticas &g|      &wOro     &g| &wQuest   &g|     &wPx\n\r");
        ch_printf(ch, "&g-----------------------|-----------|--------------|---------|--------\n\r");
        ch_printf(ch, "&wFue Int Car Sab        &g|         &w1 &g|            &w0 &g|       &w0 &g|       &w0 \n\r");
        ch_printf(ch, "&wCon Sue Des            &g|         &w1 &g|            &w0 &g|       &w0 &g|       &w0 \n\r");
        ch_printf(ch, "&w10 Vida                &g|&w        70 &g|            &w0 &g|       &w0 &g|&w     700 \n\r");
        ch_printf(ch, "&w10 Mana                &g|&w        60 &g|            &w0 &g|       &w0 &g|&w     600 \n\r");
        ch_printf(ch, "&w10 Move                &g|        &w50 &g|            &w0 &g|       &w0 &g|&w     500 \n\r");
        ch_printf(ch, "&wAstucia                &g|&r        25 &g|            &w0 &g|&r     250 &g|&w    1000 \n\r");
        ch_printf(ch, "&wManipulacion           &g|&r        25 &g|            &w0 &g|&r     175 &g|&w     750 \n\r");
        ch_printf(ch, "&wIntimidacion           &g|&r        30 &g|            &w0 &g|&r     210 &g|&w     900 \n\r");
        ch_printf(ch, "&wFuerza de Voluntad     &g|&w         0 &g|            &w0 &g|&r     500 &g|&r    2000 \n\r");
        ch_printf(ch, "&wPuntos de Camino       &g|&w         0 &g|            &w0 &g|&w       0 &g|&w    1500 \n\r");
        ch_printf(ch, "&wPuntos de Generacion   &g|&w         0 &g|            &w0 &g|&w    2000 &g|&w   11150 \n\r");
        ch_printf(ch, "&w10 Practicas           &g|&w         0 &g| &w    10000000 &g|&w     500 &g|&w    1000 \n\r");

        ch_printf(ch, "\n\r&gRecuerda que si 2 atributos estan en el mismo color significa que debes tener\n\r");
        ch_printf(ch, "ambas cosas disponibles, ejemplo:\n\r&wastucia                &g|&r        25 &g|            &w0 &g|&r     250 &g|&w    1000 \n\r&gDeberas tener 25 practicas y 250 puntos quest, para adquirirla.\n\rAyuda MEJORAR\n\r");
        ch_printf(ch, "Ademas, solo los heroes puedes adquirir atributos con px.\n\r");
        return;
     }

   if(( !is_number(arg2)
     && str_prefix(arg1, "manipulacion")
     && str_prefix(arg1, "voluntad")))
     {
      ch_printf(ch, "El segundo argumento debe ser numerico (0 para entrenarlo todo en el stat).\n\r");
      return;
     }

     /*
      * Mejorando la fuerza del jugador
      */

        if(!str_prefix(arg1, "fuerza"))
        {
        if(ch->perm_str >= 25)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'Tu fuerza ya esta al maximo'\n\r", mob->short_descr);
                return;
        }

        if(ch->practice == 0)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }


        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La fuerza solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

            gana_stat = (25 - ch->perm_str);

            if(ch->practice < gana_stat)
            	gana_stat = ch->practice;

            if(gana_stat >=1)
            {
                        ch->perm_str += gana_stat;
                        ch->practice -= gana_stat;
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu fuerza'\n\r", mob->short_descr, gana_stat);
                        act( AT_PLAIN, "$n mejora su fuerza!\n\r", ch, NULL, ch, TO_ROOM );
            }
            return;
        }

        if(atoi(arg2) > ch->practice)
        {
                       ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes tantas practicas he...'\n\r", mob->short_descr);
                       return;
        }

        if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La fuerza solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

        ch->perm_str += atoi(arg2);
        ch->practice -= atoi(arg2);
        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu fuerza'\n\r", mob->short_descr, atoi(arg2));
        act( AT_PLAIN, "$n mejora su fuerza!\n\r", ch, NULL, ch, TO_ROOM );
      }

      /*
       * Mejorando la sabiduria del jugador
       */

        if(!str_prefix(arg1, "sabiduria"))
        {
        if(ch->perm_wis >= 25)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'Tu sabiduria ya esta al maximo'\n\r", mob->short_descr);
                return;
        }

        if(ch->practice == 0)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }


        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La sabiduria solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

            gana_stat = (25 - ch->perm_wis);

            if(ch->practice < gana_stat)
            gana_stat = ch->practice;

            if(gana_stat >=1)
            {
                        ch->perm_wis += gana_stat;
                        ch->practice -= gana_stat;
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu sabiduria'\n\r", mob->short_descr, gana_stat);
            }
            return;
        }

        if(atoi(arg2) > ch->practice)
        {
                       ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes tantas practicas he...'\n\r", mob->short_descr);
                       return;
        }

        if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La sabiduria solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

        ch->perm_wis += atoi(arg2);
        ch->practice -= atoi(arg2);
        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu sabiduria'\n\r", mob->short_descr, atoi(arg2));
      }
      /*
       * Mejorando la inteligencia del jugador (si es posible XD)
       */

        if(!str_prefix(arg1, "inteligencia"))
        {
        if(ch->perm_int >= 25)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'Tu inteligencia ya esta al maximo'\n\r", mob->short_descr);
                return;
        }

        if(ch->practice == 0)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }


        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La inteligencia solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

            gana_stat = (25 - ch->perm_int);

            if(ch->practice < gana_stat)
            gana_stat = ch->practice;

            if(gana_stat >=1)
            {
                        ch->perm_int += gana_stat;
                        ch->practice -= gana_stat;
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu inteligencia'\n\r", mob->short_descr, gana_stat);
            }
            return;
        }

        if(atoi(arg2) > ch->practice)
        {
                       ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes tantas practicas he...'\n\r", mob->short_descr);
                       return;
        }

        if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La inteligencia solo se puede mejorar con practicas y leyendo'\n\r", mob->short_descr);
                        return;
                }

        ch->perm_int += atoi(arg2);
        ch->practice -= atoi(arg2);
        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu inteligencia'\n\r", mob->short_descr, atoi(arg2));
      }
      /*
       * Mejorando la destreza del jugador
       */

        if(!str_prefix(arg1, "destreza"))
        {
        if(ch->perm_dex >= 25)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'Tu destreza ya esta al maximo'\n\r", mob->short_descr);
                return;
        }

        if(ch->practice == 0)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }


        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La destreza solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

            gana_stat = (25 - ch->perm_dex);

            if(ch->practice < gana_stat)
            gana_stat = ch->practice;

            if(gana_stat >=1)
            {
                        ch->perm_dex += gana_stat;
                        ch->practice -= gana_stat;
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu destreza'\n\r", mob->short_descr, gana_stat);
                        act( AT_PLAIN, "$n mejora su destreza!\n\r", ch, NULL, ch, TO_ROOM );
            }
            return;
        }

        if(atoi(arg2) > ch->practice)
        {
                       ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes tantas practicas he...'\n\r", mob->short_descr);
                       return;
        }

        if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La destreza solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

        ch->perm_dex += atoi(arg2);
        ch->practice -= atoi(arg2);
        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu destreza'\n\r", mob->short_descr, atoi(arg2));
        act( AT_PLAIN, "$n mejora su destreza!\n\r", ch, NULL, ch, TO_ROOM );
      }
      /*
       * Mejorando la constitucion del jugador
       */

        if(!str_prefix(arg1, "constitucion"))
        {
        if(ch->perm_con >= 25)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'Tu constitucion ya esta al maximo'\n\r", mob->short_descr);
                return;
        }

        if(ch->practice == 0)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }


        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La constitucion solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

            gana_stat = (25 - ch->perm_con);

            if(ch->practice < gana_stat)
            gana_stat = ch->practice;

            if(gana_stat >=1)
            {
                        ch->perm_con += gana_stat;
                        ch->practice -= gana_stat;
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu constitucion'\n\r", mob->short_descr, gana_stat);
                        act( AT_PLAIN, "$n mejora su constitucion!\n\r", ch, NULL, ch, TO_ROOM );
            }
            return;
        }

        if(atoi(arg2) > ch->practice)
        {
                       ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes tantas practicas he...'\n\r", mob->short_descr);
                       return;
        }

        if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La constitucion solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

        ch->perm_con += atoi(arg2);
        ch->practice -= atoi(arg2);
        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu constitucion'\n\r", mob->short_descr, atoi(arg2));
        act( AT_PLAIN, "$n mejora su constitucion!\n\r", ch, NULL, ch, TO_ROOM );
      }
      /*
       * Mejorando el astucia del jugador
       */

        if(!str_prefix(arg1, "carisma"))
        {
        if(ch->perm_cha >= 25)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'Tu carisma ya esta al maximo'\n\r", mob->short_descr);
                return;
        }

        if(ch->practice == 0)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }


        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'El carisma  solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

            gana_stat = (25 - ch->perm_cha);

            if(ch->practice < gana_stat)
            gana_stat = ch->practice;

            if(gana_stat >=1)
            {
                        ch->perm_cha += gana_stat;
                        ch->practice -= gana_stat;
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu carisma'\n\r", mob->short_descr, gana_stat);
                        act( AT_PLAIN, "$n mejora su carisma!\n\r", ch, NULL, ch, TO_ROOM );
            }
            return;
        }

        if(atoi(arg2) > ch->practice)
        {
                       ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes tantas practicas he...'\n\r", mob->short_descr);
                       return;
        }

        if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'El astucia solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

        ch->perm_cha += atoi(arg2);
        ch->practice -= atoi(arg2);
        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu carisma'\n\r", mob->short_descr, atoi(arg2));
        act( AT_PLAIN, "$n mejora su carisma.", ch, NULL, ch, TO_ROOM );
      }
      /*
      * Mejorando la suerte del jugador
      */

        if(!str_prefix(arg1, "suerte"))
        {
        if(ch->perm_lck >= 25)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'Tu suerte ya esta al maximo'\n\r", mob->short_descr);
                return;
        }

        if(ch->practice == 0)
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }


        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La suerte solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

            gana_stat = (25 - ch->perm_lck);

            if(ch->practice < gana_stat)
            gana_stat = ch->practice;

            if(gana_stat >=1)
            {
                        ch->perm_lck += gana_stat;
                        ch->practice -= gana_stat;
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu suerte'\n\r", mob->short_descr, gana_stat);
                        act( AT_PLAIN, "$n mejora su suerte!\n\r", ch, NULL, ch, TO_ROOM );
            }
            return;
        }

        if(atoi(arg2) > ch->practice)
        {
                       ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes tantas practicas he...'\n\r", mob->short_descr);
                       return;
        }

        if(str_prefix(arg3, "practicas")
                || arg3[0] == '\0')
                {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'La suerte solo se puede mejorar con practicas'\n\r", mob->short_descr);
                        return;
                }

        ch->perm_lck += atoi(arg2);
        ch->practice -= atoi(arg2);
        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu suerte'\n\r", mob->short_descr, atoi(arg2));
        act( AT_PLAIN, "$n mejora su suerte!\n\r", ch, NULL, ch, TO_ROOM );
      }
      /*
      * Mejorando la vida del jugador
      */

        if(!str_prefix(arg1, "vida"))
        {

        if((ch->max_hit >= (30000 + ( ch->pcdata->renacido * 10000 ))
           && ch->pcdata->renacido != 0))
        {
                ch_printf(ch, "&w%s te dice telapaticamente 'No puedes tener mas de %d Pv en este remort'\n\r", mob->short_descr, (30000 + ( ch->pcdata->renacido * 10000 )));
                return;
        }

        if((ch->pcdata->renacido == 0
            && ch->max_hit >= 30000))
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes tener mas de 30000 Pv en este remort'\n\r", mob->short_descr);
                return;
        }

        if((ch->practice < 70 && ch->level < LEVEL_AVATAR))
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas eh...'\n\r", mob->short_descr);
                return;
        }

        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(!str_prefix(arg3, "practicas"))
                {
                        if(ch->practice < 70 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu vida con las pocas practicas que tienes'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->renacido == 0)
                        	gana_vida = (30000 - ch->max_hit);
                        else
                        	gana_vida = (30000 + ( ch->pcdata->renacido * 10000 )) - ch->max_hit;

                        coste_prac = (gana_vida / 10);
                         ch_printf(ch,  "&w%s te dice PRAC TRUE.'\n\r",
			mob->short_descr);
			prac = TRUE;
                        px   = FALSE;
                }
                if(!str_prefix(arg3, "px"))
                {
                        if(ch->exp_acumulada < 700)
                        {
                        mensa_aleatorio = number_range(1, 10);
                        if ( mensa_aleatorio < 4)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes bastante px %s, ve a matar y vuelve mas tarde'\n\r", mob->short_descr, ch->name);
                        else if ( mensa_aleatorio < 8 && mensa_aleatorio > 3)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Mejor no, que no tienes bastantes px'\n\r", mob->short_descr);
                        else if ( mensa_aleatorio == 10 )
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Menudo heroe... con la mierda de px que tienes que quieres subir?'\n\r", mob->short_descr);
                        else
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Si hombre!! Y que mas?? Si solo tienes %d px, no me hagas reir!!'\n\r", mob->short_descr, ch->exp_acumulada);
                        return;
                        }

                        if(ch->pcdata->renacido == 0)
                        gana_vida = (30000 - ch->max_hit);
                        else
                        gana_vida = (30000 + ( ch->pcdata->renacido * 10000 )) - ch->max_hit;

                        coste_px = (gana_vida / 10);
                        px =   TRUE;
                        prac = FALSE;
                 }
                 else
                 {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas o px si eres heroes'\n\r", mob->short_descr);
                        return;
                 }

                 if(px == TRUE)
                 {

                                if(coste_px > (ch->exp_acumulada / 700))
                                gana_vida = (ch->exp_acumulada / 700);
                 }

                 if(prac == TRUE)
                 {
                                if(coste_prac > (ch->practice / 70))
                                	gana_vida = (ch->practice / 70);
                 }

            if(gana_vida >=1)
            {

                        if(prac == TRUE)
                        {
                        if  (ch->practice >= (gana_vida * 70))
			        	ch->practice -= (gana_vida * 70);
                        else
                        {
                        	ch_printf(ch, "Error!! No bastan las practicas!!\n\r");
                        	return;
                        }
                        ch->max_hit += (gana_vida * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu Vida'\n\r", mob->short_descr, (gana_vida * 10));
                        sprintf( buf, "&w%s mejora en %d puntos su vida!\n\r", ch->name, (gana_vida * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
                        if(px == TRUE)
                        {
                        if(ch->exp_acumulada < (gana_vida * 700))
                        {
                                ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu vida con los pocos px que tienes'\n\r", mob->short_descr);
                                return;
                        }

                        ch->exp_acumulada -= (gana_vida * 700);
                        ch->max_hit += (gana_vida * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu Vida'\n\r", mob->short_descr, (gana_vida * 10));
                        sprintf( buf, "&w%s mejora en %d puntos su vida!\n\r", ch->name, (gana_vida * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
            }
            return;
        }

        if ( atoi(arg2) >= 1 )
        {
        if(!str_prefix(arg3, "practicas"))
                {
                  if(ch->practice < (atoi(arg2)* 7))
                  {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes las %d practicas necesarias'\n\r", mob->short_descr, (atoi(arg2) *7));
                        return;
                  }

                        gana_vida = atoi(arg2);
                        prac = TRUE;
                        px   = FALSE;
                }
                else if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < (atoi(arg2)* 700))
                        {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes los %d Puntos de experiencia necesarios'\n\r", mob->short_descr, (atoi(arg2) *700));
                        return;
                        }

                        gana_vida = atoi(arg2);
                        px =   TRUE;
                        prac = FALSE;
                 }
                 else
                 {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas o px si eres heroes'\n\r", mob->short_descr);
                        return;
                 }


            if(gana_vida >=1)
            {
                        if(prac == TRUE)
                        {
                        if  (ch->practice >= (gana_vida * 70))
                                        ch->practice -= (gana_vida * 70);
                        else
                        {
                                ch_printf(ch, "Error!! No bastan las practicas!!\n\r");
                                return;
                        }

                        ch->max_hit += (gana_vida * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu Vida'\n\r", mob->short_descr, (gana_vida * 10));
                        sprintf( buf, "&w%s mejora en %d puntos su vida!\n\r", ch->name, (gana_vida * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
                        if(px == TRUE)
                        {
                        if  (ch->exp_acumulada >= (gana_vida * 700))
				ch->exp_acumulada -= (gana_vida * 700);
                        else
                        {
                                ch_printf(ch, "Error!! No bastan las practicas!!\n\r");
                                return;
                        }

                        ch->max_hit += (gana_vida * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu Vida'\n\r", mob->short_descr, (gana_vida *10));
                        sprintf( buf, "&w%s mejora en %d puntos su vida!\n\r", ch->name, (gana_vida * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
            }
            return;
         }
      }

     /*
      * Mejorando el mana del jugador
      */

        if(!str_prefix(arg1, "mana"))
        {

        if(IS_VAMPIRE(ch))
        {
                ch_printf(ch, "&w%s te dice telepaticamente 'Puedo ver tu linaje en ti, eres el chiquillo de %s'\n\r&w%s te dice telepaticamente 'Deberias interesarte en buscar sangre'\n\r", mob->short_descr, ch->sire_ch, mob->short_descr);
                return;
        }

        if((ch->max_mana >= (30000 + ( ch->pcdata->renacido * 10000 ))
           && ch->pcdata->renacido != 0))
        {
                ch_printf(ch, "&w%s te dice telapaticamente 'No puedes tener mas de %d puntos de mana en este remort'\n\r", mob->short_descr, (30000 + ( ch->pcdata->renacido * 10000 )));
                return;
        }

        if((ch->pcdata->renacido == 0
            && ch->max_mana >= 30000))
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes tener mas de 30000 puntos de mana en este remort'\n\r", mob->short_descr);
                return;
        }

        if((ch->practice <= 60
            && ch->level < LEVEL_HERO))
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }

        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(!str_prefix(arg3, "practicas"))
                {
                        if(ch->practice < 60 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu mana con las pocas practicas que tienes'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->renacido == 0)
                        gana_mana = (30000 - ch->max_mana);
                        else
                        gana_mana = (30000 + ( ch->pcdata->renacido * 10000 )) - ch->max_mana;

                        coste_prac = (gana_mana / 10);
                        prac = TRUE;
                        px   = FALSE;
                }
                else if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < 600 )
                        {
                        mensa_aleatorio = number_range(1, 10);
                        if ( mensa_aleatorio < 4)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes bastante px %s, ve a matar y vuelve mas tarde'\n\r", mob->short_descr, ch->name);
                        else if ( mensa_aleatorio < 8 && mensa_aleatorio > 3)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Mejor no, que no tienes bastantes px'\n\r", mob->short_descr);
                        else if ( mensa_aleatorio == 10 )
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Menudo heroe... con la mierda de px que tienes que quieres subir?'\n\r", mob->short_descr);
                        else
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Si hombre!! Y que mas?? Si solo tienes %d px, no me hagas reir!!'\n\r", mob->short_descr, ch->exp_acumulada);
                        return;
                        }

                        if(ch->pcdata->renacido == 0)
                        gana_mana = (30000 - ch->max_mana);
                        else
                        gana_mana = (30000 + ( ch->pcdata->renacido * 10000 )) - ch->max_mana;

                        coste_px = (gana_mana / 10);
                        px =   TRUE;
                        prac = FALSE;
                 }
                 else
                 {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas o px si eres heroes'\n\r", mob->short_descr);
                        return;
                 }

                 if(px == TRUE)
                 {

                                if(coste_px > (ch->exp_acumulada / 600))
                                gana_mana = (ch->exp_acumulada / 600);
                 }

                 if(prac == TRUE)
                 {
                                if(coste_prac > (ch->practice / 60))
                                gana_mana = (ch->practice / 60);
                 }

            if(gana_mana >=1)
            {

                        if(prac == TRUE)
                        {
			if(ch->exp_acumulada >= (gana_mana * 60))
                        	ch->practice -= (gana_mana * 60);
                        else
			{
        			ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu mana con las pocas practicas que tienes'\n\r", 
				mob->short_descr);
                                return;
                        }

			ch->max_mana += (gana_mana * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu mana'\n\r", mob->short_descr, (gana_mana * 10));
                        sprintf( buf, "&w%s mejora en %d puntos su mana!\n\r", ch->name, (gana_mana * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
                        if(px == TRUE)
                        {
                        if(ch->exp_acumulada < (gana_mana * 600))
			{
                                ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu mana con los pocos px que tienes'\n\r", mob->short_descr);
                                return;
                        }

                        ch->exp_acumulada -= (gana_mana * 600);
                        ch->max_mana += (gana_mana*10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu mana'\n\r", mob->short_descr, gana_mana);
                        sprintf( buf, "&w%s mejora en %d puntos su mana!\n\r", ch->name, (gana_mana * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
            }
            return;
        }

        if ( atoi(arg2) >= 1 )
        {
        if(!str_prefix(arg3, "practicas"))
                {
                  if(ch->practice < (atoi(arg2)* 60))
                  {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes las %d practicas necesarias'\n\r", mob->short_descr, (atoi(arg2) *60));
                        return;
                  }

                        gana_mana = atoi(arg2);
                        prac = TRUE;
                        px   = FALSE;
                }
                else if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < (atoi(arg2)* 600))
                        {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes los %d Puntos de experiencia necesarios'\n\r", mob->short_descr, (atoi(arg2) *600));
                        return;
                        }

                        gana_mana = atoi(arg2);
                        px =   TRUE;
                        prac = FALSE;
                 }
                 else
                 {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas o px si eres heroes'\n\r", mob->short_descr);
                        return;
                 }


            if(gana_mana >=1)
            {
                        if(prac == TRUE)
                        {
			if(ch->exp_acumulada < (gana_mana * 60))
			{
				ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu mana con las pocas practicas que tienes'\n\r",
                                mob->short_descr);
                                return;

			}
                        ch->practice -= (gana_mana * 60);
                        ch->max_mana += (gana_mana * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu mana'\n\r", mob->short_descr, (gana_mana * 10));
                        sprintf( buf, "&w%s mejora en %d puntos su mana!\n\r", ch->name, (gana_mana * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
                        if(px == TRUE)
                        {
			if(ch->exp_acumulada < (gana_mana * 600))
                        {
                                ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu mana con los pocos px que tienes.\n\r",mob->short_descr);
                                return;
                        }

                        ch->exp_acumulada -= (gana_mana * 500);
                        ch->max_mana += (gana_mana * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu mana'\n\r", mob->short_descr, (gana_mana *10));
                        sprintf( buf, "&w%s mejora en %d puntos su mana!\n\r", ch->name, (gana_mana * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
            }
            return;
         }
      }

     /*
      * Mejorando el movimiento del jugador
      */

        if(!str_prefix(arg1, "movimiento"))
        {

        if((ch->max_move >= (30000 + ( ch->pcdata->renacido * 10000 ))
           && ch->pcdata->renacido != 0))
        {
                ch_printf(ch, "&w%s te dice telapaticamente 'No puedes tener mas de %d puntos de movimiento en este remort'\n\r", mob->short_descr, (30000 + ( ch->pcdata->renacido * 10000 )));
                return;
        }

        if((ch->pcdata->renacido == 0
            && ch->max_move >= 30000))
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes tener mas de 30000 puntos de movimiento en este remort'\n\r", mob->short_descr);
                return;
        }

        if((ch->practice <= 50
            && ch->level < LEVEL_HERO))
        {
                ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes practicas he...'\n\r", mob->short_descr);
                return;
        }

        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) == 0)
           {
                if(!str_prefix(arg3, "practicas"))
                {
                        if(ch->practice < 50 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu movimiento con las pocas practicas que tienes'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->renacido == 0)
                        gana_movimiento = (30000 - ch->max_move);
                        else
                        gana_movimiento = (30000 + ( ch->pcdata->renacido * 10000 )) - ch->max_move;

                        coste_prac = (gana_movimiento / 10);
                        prac = TRUE;
                        px   = FALSE;
                }
                else if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < 500 )
                        {
                        mensa_aleatorio = number_range(1, 10);
                        if ( mensa_aleatorio < 4)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes bastante px %s, ve a matar y vuelve mas tarde'\n\r", mob->short_descr, ch->name);
                        else if ( mensa_aleatorio < 8 && mensa_aleatorio > 3)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Mejor no, que no tienes bastantes px'\n\r", mob->short_descr);
                        else if ( mensa_aleatorio == 10 )
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Menudo heroe... con la mierda de px que tienes que quieres subir?'\n\r", mob->short_descr);
                        else
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Si hombre!! Y que mas?? Si solo tienes %d px, no me hagas reir!!'\n\r", mob->short_descr, ch->exp_acumulada);
                        return;
                        }

                        if(ch->pcdata->renacido == 0)
                        gana_movimiento = (30000 - ch->max_move);
                        else
                        gana_movimiento = (30000 + ( ch->pcdata->renacido * 10000 )) - ch->max_move;

                        coste_px = (gana_movimiento / 10);
                        px =   TRUE;
                        prac = FALSE;
                 }
                 else
                 {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas o px si eres heroes'\n\r", mob->short_descr);
                        return;
                 }

                 if(px == TRUE)
                 {

                                if(coste_px > (ch->exp_acumulada / 500))
                                gana_movimiento = (ch->exp_acumulada / 500);
                 }

                 if(prac == TRUE)
                 {
                                if(coste_prac > (ch->practice / 50))
                                gana_movimiento = (ch->practice / 50);
                 }

            if(gana_movimiento >=1)
            {

                        if(prac == TRUE)
                        {
                        ch->practice -= (gana_movimiento * 50);
                        ch->max_move += (gana_movimiento * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu movimiento'\n\r", mob->short_descr, (gana_movimiento * 10));
                        sprintf( buf, "&w%s mejora en %d puntos su movimiento!\n\r", ch->name, (gana_mana * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
                        if(px == TRUE)
                        {
                        if(ch->exp_acumulada < (gana_movimiento * 500))
                        {
                                ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu movimiento con los pocos px que tienes'\n\r", mob->short_descr);
                                return;
                        }

                        ch->exp_acumulada -= (gana_movimiento * 50);
                        ch->max_move += (gana_movimiento);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu movimiento'\n\r", mob->short_descr, gana_movimiento);
                        sprintf( buf, "&w%s mejora en %d puntos su movimiento!\n\r", ch->name, (gana_mana * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
            }
            return;
        }

        if ( atoi(arg2) >= 1 )
        {
        if(!str_prefix(arg3, "practicas"))
                {
                  if(ch->practice < (atoi(arg2)* 50))
                  {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes las %d practicas necesarias'\n\r", mob->short_descr, (atoi(arg2) *5));
                        return;
                  }

                        gana_movimiento = atoi(arg2);
                        prac = TRUE;
                        px   = FALSE;
                }
                else if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < (atoi(arg2)* 500))
                        {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes los %d Puntos de experiencia necesarios'\n\r", mob->short_descr, (atoi(arg2) *500));
                        return;
                        }

                        gana_movimiento = atoi(arg2);
                        px =   TRUE;
                        prac = FALSE;
                 }
                 else
                 {
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas o px si eres heroes'\n\r", mob->short_descr);
                        return;
                 }


            if(gana_movimiento >=1)
            {
                        if(prac == TRUE)
                        {
                        ch->practice -= (gana_movimiento * 50);
                        ch->max_move += (gana_movimiento * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu movimiento'\n\r", mob->short_descr, (gana_movimiento * 10));
                        sprintf( buf, "&w%s mejora en %d puntos su movimiento!\n\r", ch->name, (gana_mana * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
                        if(px == TRUE)
                        {
                        ch->exp_acumulada -= (gana_movimiento * 500);
                        ch->max_move += (gana_movimiento * 10);
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Aumentas %d puntos tu movimiento'\n\r", mob->short_descr, (gana_movimiento *10));
                        sprintf( buf, "&w%s mejora en %d puntos su movimiento!\n\r", ch->name, (gana_mana * 10));
                        act( AT_PLAIN, buf, ch, NULL, ch, TO_ROOM );
                        }
            }
            return;
         }
      }

      /*
       * Mejorando la manipulacion del personaje
       */

        if(!str_prefix(arg1, "manipulacion"))
        {

        if(ch->manipulacion >= 10)
        {
                ch_printf(ch, "&w%s te dice telapaticamente 'No puedes tener mas de 10 puntos de manipulacion'\n\r", mob->short_descr );
                return;
        }

        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) != 1)
           {
            ch_printf(ch, "&w%s te dice telapaticamente 'El argumento no puede ser diferente de 1'\n\r", mob->short_descr );
            return;
           }

           if(!str_prefix(arg3, "quest"))
                {
                        if(ch->practice < 25 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu poder de mamilupacion con esas practicas'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->quest_curr < 175 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu poder de mamilupacion con pocos puntos quest'\n\r", mob->short_descr);
                          return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu manipulacion.\n\r");
                        act( AT_PLAIN, "$n mejora su poder de anipulacion.", ch, NULL, ch, TO_ROOM );
                        ch->practice -= 25;
                        ch->pcdata->quest_curr -= 175;
                        ch->manipulacion++;
                        return;
                        }
                }

                if(!str_prefix(arg3, "practicas"))
                {
                        if(ch->practice < 25 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu poder de mamilupacion con esas practicas'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->quest_curr < 175 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu poder de mamilupacion con pocos puntos quest'\n\r", mob->short_descr);
                          return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu manipulacion.\n\r");
                        act( AT_PLAIN, "$n mejora su poder de anipulacion.", ch, NULL, ch, TO_ROOM );
                        ch->practice -= 25;
                        ch->pcdata->quest_curr -= 175;
                        ch->manipulacion++;
                        return;
                        }
                }

                if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < 750 )
                        {
                        mensa_aleatorio = number_range(1, 10);
                        if ( mensa_aleatorio < 4)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes bastante px %s, ve a matar y vuelve mas tarde'\n\r", mob->short_descr, ch->name);
                        else if ( mensa_aleatorio < 8 && mensa_aleatorio > 3)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Mejor no, que no tienes bastantes px'\n\r", mob->short_descr);
                        else if ( mensa_aleatorio == 10 )
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Menudo heroe... con la mierda de px que tienes que quieres subir?'\n\r", mob->short_descr);
                        else
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Si hombre!! Y que mas?? Si solo tienes %d px, no me hagas reir!!'\n\r", mob->short_descr, ch->exp_acumulada);
                        return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu manipulacion.\n\r");
                        act( AT_PLAIN, "$n mejora su poder de anipulacion.", ch, NULL, ch, TO_ROOM );
                        ch->manipulacion++;
                        ch->exp_acumulada -= 750;
                        return;
                        }
                 }
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas quest o px si eres heroe'\n\r", mob->short_descr);
                        return;
        }

      /*
       * Mejorando la intimidacion del personaje
       */

        if(!str_cmp(arg1, "intimidacion"))
        {

        if(ch->intimidacion >= 10)
        {
                ch_printf(ch, "&w%s te dice telapaticamente 'No puedes tener mas de 10 puntos de intimidacion'\n\r", mob->short_descr );
                return;
        }

        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) != 1)
           {
            ch_printf(ch, "&w%s te dice telapaticamente 'El argumento no puede ser diferente de 1'\n\r", mob->short_descr );
            return;
           }

           if(!str_prefix(arg3, "quest"))
                {
                        if(ch->practice < 30 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu capacidad de intimidar con esas practicas'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->quest_curr < 210 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu capacidad de intimidar con pocos puntos quest'\n\r", mob->short_descr);
                          return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu intimidacion.\n\r");
                        act( AT_PLAIN, "$n mejora su poder de intiidacion.", ch, NULL, ch, TO_ROOM );
                        ch->practice -= 30;
                        ch->pcdata->quest_curr -= 210;
                        ch->intimidacion++;
                        return;
                        }
                }

                if(!str_prefix(arg3, "practicas"))
                {
                        if(ch->practice < 30 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu capacidad de intimidar con esas practicas'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->quest_curr < 210 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu capacidad de intimidar con pocos puntos quest'\n\r", mob->short_descr);
                          return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu intimidacion.\n\r");
                        act( AT_PLAIN, "$n mejora su poder de intiidacion.", ch, NULL, ch, TO_ROOM );
                        ch->practice -= 30;
                        ch->pcdata->quest_curr -= 210;
                        ch->intimidacion++;
                        return;
                        }
                }

                if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < 900 )
                        {
                        mensa_aleatorio = number_range(1, 10);
                        if ( mensa_aleatorio < 4)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes bastante px %s, ve a matar y vuelve mas tarde'\n\r", mob->short_descr, ch->name);
                        else if ( mensa_aleatorio < 8 && mensa_aleatorio > 3)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Mejor no, que no tienes bastantes px'\n\r", mob->short_descr);
                        else if ( mensa_aleatorio == 10 )
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Menudo heroe... con la mierda de px que tienes que quieres subir?'\n\r", mob->short_descr);
                        else
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Si hombre!! Y que mas?? Si solo tienes %d px, no me hagas reir!!'\n\r", mob->short_descr, ch->exp_acumulada);
                        return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu intimidacion.\n\r");
                        act( AT_PLAIN, "$n mejora su poder de intiidacion.", ch, NULL, ch, TO_ROOM );
                        ch->intimidacion++;
                        ch->exp_acumulada -= 900;
                        return;
                        }
                 }
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas quest o px si eres heroe'\n\r", mob->short_descr);
                        return;
        }

      /*
       * Mejorando el astucia del personaje
       */

        if(!str_prefix(arg1, "astucia"))
        {

        if(ch->astucia >= 10)
        {
                ch_printf(ch, "&w%s te dice telapaticamente 'No puedes tener mas de 10 puntos de astucia'\n\r", mob->short_descr );
                return;
        }

        /*
         * Algunos stats se deben de entrenar con practicas, otros con
         * oro, otros con puntos quest otros con Experiencia o con todo a la vez XD
         */

           if(atoi(arg2) != 1)
           {
            ch_printf(ch, "&w%s te dice telapaticamente 'El argumento no puede ser diferente de 1'\n\r", mob->short_descr );
            return;
           }

           if(!str_prefix(arg3, "quest"))
                {
                        if(ch->practice < 25 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu astucia con esas practicas'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->quest_curr < 250 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu astucia con pocos puntos quest'\n\r", mob->short_descr);
                          return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu astucia.\n\r");
                        act( AT_PLAIN, "$n mejora su astucia.", ch, NULL, ch, TO_ROOM );
                        ch->practice -= 25;
                        ch->pcdata->quest_curr -= 250;
                        ch->astucia++;
                        return;
                        }
                }

                if(!str_prefix(arg3, "practicas"))
                {
                        if(ch->practice < 25 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu astucia con esas practicas'\n\r", mob->short_descr);
                          return;
                        }

                        if(ch->pcdata->quest_curr < 250 )
                        {
                          ch_printf(ch,  "&w%s te dice telapaticamente 'No puedes mejorar tu astucia con pocos puntos quest'\n\r", mob->short_descr);
                          return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu astucia.\n\r");
                        act( AT_PLAIN, "$n mejora su astucia.", ch, NULL, ch, TO_ROOM );
                        ch->practice -= 25;
                        ch->pcdata->quest_curr -= 250;
                        ch->astucia++;
                        return;
                        }
                }

                if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < 1000 )
                        {
                        mensa_aleatorio = number_range(1, 10);
                        if ( mensa_aleatorio < 4)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes bastante px %s, ve a matar y vuelve mas tarde'\n\r", mob->short_descr, ch->name);
                        else if ( mensa_aleatorio < 8 && mensa_aleatorio > 3)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Mejor no, que no tienes bastantes px'\n\r", mob->short_descr);
                        else if ( mensa_aleatorio == 10 )
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Menudo heroe... con la mierda de px que tienes que quieres subir?'\n\r", mob->short_descr);
                        else
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Si hombre!! Y que mas?? Si solo tienes %d px, no me hagas reir!!'\n\r", mob->short_descr, ch->exp_acumulada);
                        return;
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto tu astucia.\n\r");
                        act( AT_PLAIN, "$n mejora su astucia.", ch, NULL, ch, TO_ROOM );
                        ch->astucia++;
                        ch->exp_acumulada -= 1000;
                        return;
                        }
                 }
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas quest o px si eres heroe'\n\r", mob->short_descr);
                        return;
        }


        /*
         * Mejorando la fuerza de voluntad del pj, aki se lo pondremos un pelin crudo
         */

         if(!str_prefix(arg1, "voluntad"))
         {
           if( ch->f_voluntad >= (ch->level / 10))
           {
             ch_printf(ch, "&w%s te dice telepaticamente 'No puedes entrenar mas de %d puntos de fuerza de voluntad por ahora.'\n\r", mob->short_descr, ch->level / 10);
             ch_printf(ch, "&w%s te dice telepaticamente 'Deberas subirla de otros modos.'\n\r", mob->short_descr);
             return;
           }

           if(ch->tiempo_fv != -1)
           {
             ch_printf(ch, "&w%s te dice telepaticamente 'No podras volver a mejorarla hasta dentro de %d horas.'\n\r", mob->short_descr, ch->tiempo_fv);
             return;
           }

           if( ch->exp_acumulada < 1000 )
           {
              ch_printf(ch, "&w%s te dice telepaticamente 'No tienes bastantes px acumulados.'\n\r", mob->short_descr);
              return;
           }

           if( ch->pcdata->quest_curr < 300 )
           {
              ch_printf(ch, "&w%s te dice telepaticamente 'No tienes bastantes puntos quest acumulados.'\n\r", mob->short_descr);
              return;
           }


           ch->exp_acumulada -= 1000;
           ch->pcdata->quest_curr -= 300;
           ch->f_voluntad++;
           ch->tiempo_fv += number_range(15, ch->level / 2);
           ch_printf(ch, "&w%s te dice telepaticamente 'Hasta dentro de %d horas no podras volver a entrenar la fuerza de voluntad.'\n\r", mob->short_descr, ch->tiempo_fv);

         return;
         } /* Del IF voluntad */

         if(!str_prefix(arg1, "genepk"))
         {
          if( !IS_HERO(ch))
          {
                send_to_char( "Solo los heroes pueden mejorar sus puntos de generacion.\n\r", ch );
                return;
          }

           if( ch->generacion <= (4))
           {
             ch_printf(ch, "&w%s te dice telepaticamente 'No puedes ser menor de %d generacion.'\n\r", mob->short_descr, ch->generacion);
             ch_printf(ch, "&w%s te dice telepaticamente 'Deberas mejorarla de otros modos.'\n\r", mob->short_descr);
             return;
           }

           if(atoi(arg2) != 1)
           {
            ch_printf(ch, "&w%s te dice telapaticamente 'El argumento no puede ser diferente de 1'\n\r", mob->short_descr );
            return;
           }

           if(!str_prefix(arg3, "quest"))
                {
                        if( ch->pcdata->quest_curr < 2000 )
                        {
                        ch_printf(ch, "&w%s te dice telepaticamente 'No tienes bastantes puntos quest acumulados.'\n\r", mob->short_descr);
                        return;
                        }
                        else
                        {
                        if( IS_VAMPIRE(ch))
                        {
                        ch_printf(ch, "&wMejoras un punto de generacion!\n\r");
                        act( AT_PLAIN, "$n mejora un punto de generacion!", ch, NULL, ch, TO_ROOM );
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto de rango!\n\r");
                        act( AT_PLAIN, "$n mejora un punto de rango!", ch, NULL, ch, TO_ROOM );
                        }

                        ch->pcdata->quest_curr -= 2000;
                        ganancia_pk( ch );
                        return;
                        }
                }

           if((!str_prefix(arg3, "px") && ch->level >= LEVEL_HERO))
                {
                        if(ch->exp_acumulada < 11150 )
                        {
                        mensa_aleatorio = number_range(1, 10);
                        if ( mensa_aleatorio < 4)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'No tienes bastante px %s, ve a matar y vuelve mas tarde'\n\r", mob->short_descr, ch->name);
                        else if ( mensa_aleatorio < 8 && mensa_aleatorio > 3)
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Mejor no, que no tienes bastantes px'\n\r", mob->short_descr);
                        else if ( mensa_aleatorio == 10 )
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Menudo heroe... con la mierda de px que tienes que quieres subir?'\n\r", mob->short_descr);
                        else
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Si hombre!! Y que mas?? Si solo tienes %d px, no me hagas reir!!'\n\r", mob->short_descr, ch->exp_acumulada);
                        return;
                        }
                        else
                        {
                        if( IS_VAMPIRE(ch))
                        {
                        ch_printf(ch, "&wMejoras un punto de generacion.\n\r");
                        act( AT_PLAIN, "$n mejora un punto de generacion!.", ch, NULL, ch, TO_ROOM );
                        }
                        else
                        {
                        ch_printf(ch, "&wMejoras un punto de rango.\n\r");
                        act( AT_PLAIN, "$n mejora un punto de rango!.", ch, NULL, ch, TO_ROOM );
                        }

                        ch->exp_acumulada -= 11150;
                        ganancia_pk( ch );
                        return;
                        }
                 }
                        ch_printf(ch,  "&w%s te dice telapaticamente 'Los argumentos solo pueden ser practicas quest o px si eres heroe'\n\r", mob->short_descr);
                        return;

         return;
         } /* Del IF genepk */


	else
      	{
                ch_printf(ch,  "&w%s te dice telapaticamente 'Argumento erroneo...'\n\r",mob->short_descr);
      	}




return; /*RETURN DEL VOID */

 }

