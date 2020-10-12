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
 *                          Modulo de Guerra y Guerrilas XD                 *
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


CLAN_DATA *first_clan;
/* Funcion para declarar guerras */
void do_declarar( CHAR_DATA *ch, char *argument  )
{
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];

        DESCRIPTOR_DATA *d;
        CLAN_DATA  *familia_victima;
        CLAN_DATA *clan;
        char buf[MAX_STRING_LENGTH];


        /* casos en los cuales la funcion retorna sin hacer nada */

        if ( IS_NPC(ch ) )
        {
                send_to_char( "&wUn mob va a declarar que?...\n\r", ch );
                return;
        }

        if ( !IS_CLANNED(ch) )
        {
                send_to_char( "&wHuh?\n\r", ch );
                return;
         }
	 /*KAYSER*/
         if (!IS_IMMORTAL(ch) && ( ch->pcdata->clan->leader != ch->name ) )
         {
                send_to_char( "&wTu no eres el lider de la familia para declarar nada.\n\r", ch );
                return;
         }

         /* Le decimos al compilador como funcionan los argumentos por enesima vez*/
         argument = one_argument( argument, arg1 );
         argument = one_argument( argument, arg2 );
         argument = one_argument( argument, arg3 );

         if( arg1[0] == '\0' )
         {

                send_to_char( "&wSintaxis:      declarar <familia> <guerra|paz|alianza> <nivel>\n\r", ch );
                send_to_char( "&wAyuda 'DECLARAR' para mas informacion.\n\r\n\r", ch );
                send_to_char( "&gDeclaraciones actuales:\n\r", ch );

         for ( clan = first_clan; clan; clan = clan->next )
                {
                  if ( clan->nv_guerra[ch->pcdata->clan->numero] == 1 )
                   sprintf( buf, "El nivel es piadoso.\n\r");

                  if ( clan->nv_guerra[ch->pcdata->clan->numero] == 2 )
                   sprintf( buf, "El nivel es Encarnizado.\n\r");

                  if ( clan->nv_guerra[ch->pcdata->clan->numero] == 3 )
                   sprintf( buf, "El nivel es de ANIKILACION TOTAL.\n\r");

                  ch_printf( ch, "&gFamilia: &w%-20.20s     ", clan->name );
                  if ( clan->nv_guerra[ch->pcdata->clan->numero] != 0 )
                   ch_printf( ch, "Estas en guerra     %s", buf);

                  else
                   send_to_char( "No estas en guerra   \n\r", ch);
                }
                return;
          }

          /* Si la familia no existe */

                if ( (familia_victima = get_clan( arg1 ) ) == NULL)
                {
                        send_to_char( "&wEsa familia no existe elije una familia valida.\n\r", ch );
                        return;
                }

                /* En caso de que el  argumento sea nulo */
                if ( arg2[0] == '\0' )
                {
                        send_to_char( "&rQue quieres con ellos guerra tregua o alianza?&w\n\r", ch );
                        return;
                }

              if ( ( familia_victima = get_clan( arg1 ) ) == ch->pcdata->clan )
              {
                        send_to_char( "&wLe declaras la guerra a tu propia familia???\n\r", ch );
                        return;
               }

          /* En caso de que el primer argumento sea guerra */

          if ( !str_prefix( arg2, "guerra" ) )
          {
                /* Si la familia agresora no tiene ya 9 declaraciones de guerra */
                if ( !ch->pcdata->clan->deguerra[9])
                {
                                if ( ch->pcdata->clan->name == familia_victima->enemigos[ch->pcdata->clan->numero] )
                                {
                                /* Si el argumento que corresponde al nivel esta vacio manda este mensaje y devuelve la funcion */

                                     if ( arg3 == NULL )
                                     {
                                        send_to_char( "&wQue nivel de crueldad quieres con ellos?.\n\r", ch );
                                        return;
                                      }

                                     if ( ch->pcdata->clan->agresor[familia_victima->numero] != 0 )
                                     {
                                        ch_printf( ch, "&gFue la familia %s la que te declaro la guerra a ti.\n\rNo puedes aumentar el nivel de tu guerra con ellos.&w\n\r", familia_victima->name );
                                        return;
                                     }

                              /* Si el argumento del nivel no es nulo */
                              /* Nivel de crueldad piadoso, el cuerpo de la victima va al deposito de su familia se consiguen px normales y todo normal */

                                if ( !str_prefix( arg3, "piadoso" ) )
                                {
                                         /* Si el nivel ya es piadoso */
                                         if ( familia_victima->nv_guerra[ch->pcdata->clan->numero] == 1 )
                                         {
                                                send_to_char( "&gTu nivel de guerra con ellos ya es piadoso.\n\r", ch );
                                                send_to_char( "Usa el comando infofamilia para ver los datos relacionados\n\r", ch );
                                                send_to_char( "con tus guerras, pactos y alianzas.&w\n\r", ch );
                                                return;
                                         }

                                                if ( familia_victima->enemigos[ch->pcdata->clan->numero] == ch->pcdata->clan->name )
                                                {
							/* Mensajitos de rigor */
                                                        sprintf( buf, "&wEl nivel de crueldad de tu guerra contra &W%s&w &wes &ppiadoso.&w\n\r",
                                                        familia_victima->name );
                                                        send_to_char( buf, ch );
                                                        sprintf( buf, "[&g &gLa familia &W%s&g &gmantiene una actitud piadosa contra &W%s&w.&z]\n\r", ch->pcdata->clan->name, familia_victima->name );
                                                        echo_to_all(AT_DGREY, buf, ECHOTAR_ALL );

                                                        for(d = first_descriptor; d; d = d->next)
                                                        {
                                                          if ( ch->pcdata->clan == d->character->pcdata->clan || ch == d)
                                                          {
                                                           ch_printf( ch, "&g[  &wInfoGuerra   &g][&w Tu familia esta en guerra contra %s, estas son las normas &g]\n\r", familia_victima->name);
                                                           send_to_char( "&g   &w             &g [&w Mas Cantidad de px en areas enemigas                          &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w No hay posibilidad de decapitacion o diablerizacion.          &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Sin posibilidad de hacer grupo con el enemigo.                &g]\n\r", d->character);
                                                          }
                                                        }

                                                        for(d = first_descriptor; d; d = d->next)
                                                        {
                                                          if ( familia_victima == d->character->pcdata->clan)
                                                          {
                                                           ch_printf( d->character, "&g[  &wInfoGuerra   &g][&w La familia %s te declara la guerra, estas son las normas &g]\n\r", ch->pcdata->clan->name);
                                                           send_to_char( "&g   &w             &g [&w Mas Cantidad de px en areas enemigas                          &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w No hay posibilidad de decapitacion o diablerizacion.          &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Sin posibilidad de hacer grupo con el enemigo.                &g]\n\r", d->character);
                                                          }
                                                        }

                                                        /* Cambiamos los valores de odio y nivel de guerra de las familias enfrentadas */
                                                        familia_victima->nv_guerra[ch->pcdata->clan->numero] = 1;
                                                        /* Es agredido */
                                                        familia_victima->agresor[ch->pcdata->clan->numero] =  1;

                                                        /* Para la familia del jugador */
                                                        ch->pcdata->clan->odio[familia_victima->numero] = 1;
                                                        /* Es agresor */
                                                        ch->pcdata->clan->agresor[familia_victima->numero] = 0;

                                                        /* Se asignan los valores de nombre */
                                                        ch->pcdata->clan->enemigos[familia_victima->numero] = familia_victima->name;
                                                        familia_victima->enemigos[ch->pcdata->clan->numero] = ch->pcdata->clan->name;

                                                        /* Guardamos los archivos de las dos familias */
                                                        bug( "Enemigo del ch %s Enemigo del victim %s Niveles %d %d", ch->pcdata->clan->enemigos[familia_victima->numero]
                                                        , familia_victima->enemigos[ch->pcdata->clan->numero]
                                                        , ch->pcdata->clan->odio[familia_victima->numero]
                                                        , familia_victima->nv_guerra[ch->pcdata->clan->numero] );
                                                        save_clan( ch->pcdata->clan );
                                                        save_clan( familia_victima );
                                                        return;
                                                }
                                            }
                                /*  Nivel de crueldad encarnizado  plus de experiencia y stats el cuerpo se keda en la room del asesinato y puede ser sakeado*/

                                if ( !str_prefix( arg3, "encarnizado" ) )
                                {
                                /* Si el nivel ya es encarnizado */
                                         if ( familia_victima->nv_guerra[ch->pcdata->clan->numero] == 2 )
                                         {
                                                send_to_char( "&gTu nivel de guerra con ellos ya es encarnizado.\n\r", ch );
                                                send_to_char( "Usa el comando infofamilia para ver los datos relacionados\n\r", ch );
                                                send_to_char( "con tus guerras, pactos y alianzas.&w\n\r", ch );
                                                return;
                                         }
                                                if ( familia_victima->enemigos[ch->pcdata->clan->numero] == ch->pcdata->clan->name )
                                                {

                                                /* Mensajitos de rigor */
                                                        sprintf( buf, "&wEl nivel de crueldad de tu guerra contra &W%s&w &wes &rencarnizado.&w\n\r",
                                                        familia_victima->name );
                                                        send_to_char( buf, ch );
                                                        sprintf( buf, "[&g &gLa familia &W%s&g &gmantiene una actitud carnicera contra &W%s&w&z].\n\r", ch->pcdata->clan->name, familia_victima->name );
                                                        echo_to_all(AT_GREY, buf, ECHOTAR_ALL );

                                                        for(d = first_descriptor; d; d = d->next)
                                                        {
                                                          if ( ch->pcdata->clan == d->character->pcdata->clan || ch == d)
                                                          {
                                                           ch_printf( ch, "&g[  &wInfoGuerra   &g][&w Tu familia esta en guerra contra %s, estas son las normas &g]\n\r", familia_victima->name);
                                                           send_to_char( "&g   &w             &g [&w Bonificacion para la familia (Oro, posicion, etc...)          &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Posibilidad de decapitacion y diablerizacion.                 &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Ganancia de puntos quest en muertes pk contra el enemigo.     &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Posibilidad de saquear el cuerpo.                             &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w El cuerpo no va al deposito de cadaveres de la familia.       &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w 25% mas de stats al asesinar a un enemigo.                    &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Apatia natural (...) contra tus enemigos.                     &g]\n\r", d->character);
                                                          }
                                                        }

                                                        for(d = first_descriptor; d; d = d->next)
                                                        {
                                                          if ( familia_victima == d->character->pcdata->clan)
                                                          {
                                                           ch_printf( d->character, "&g[  &wInfoGuerra   &g][&w La familia %s te declara la guerra, estas son las normas &g]\n\r", ch->pcdata->clan->name);
                                                           send_to_char( "&g   &w             &g [&w Bonificacion para la familia (Oro, posicion, etc...)          &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Posibilidad de decapitacion y diablerizacion.                 &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Ganancia de puntos quest en muertes pk contra el enemigo.     &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Posibilidad de saquear el cuerpo.                             &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w El cuerpo no va al deposito de cadaveres de la familia.       &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w 25% mas de stats al asesinar a un enemigo.                    &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Apatia natural (...) contra tus enemigos.                     &g]\n\r", d->character);
                                                          }
                                                        }


                                                        /* Cambiamos los valores de odio y nivel de guerra de las familias enfrentadas */
                                                        familia_victima->nv_guerra[ch->pcdata->clan->numero] = 2;

                                                        /* Para la familia del jugador */
                                                        ch->pcdata->clan->odio[familia_victima->numero] = 2;
                                                        ch->pcdata->clan->enemigos[familia_victima->numero] = familia_victima->name;
                                                        familia_victima->enemigos[ch->pcdata->clan->numero] = ch->pcdata->clan->name;

                                                        /* Guardamos los archivos de las dos familias */
                                                        save_clan( ch->pcdata->clan );
                                                        save_clan( familia_victima );
                                                        return;
                                                }
                                            }
                                    /* Nivel de crueldad Xenofobo total se pierde.... de TODO */

                                if ( !str_prefix( arg3, "aniquilacion" ) )
                                {
                                /* Si el nivel ya es aniquilacion */
                                         if ( familia_victima->nv_guerra[ch->pcdata->clan->numero] == 3 )
                                         {
                                                send_to_char( "&gTu nivel de guerra con ellos ya es de mutua masacre.\n\r", ch );
                                                send_to_char( "Usa el comando infofamilia para ver los datos relacionados\n\r", ch );
                                                send_to_char( "con tus guerras, pactos y alianzas.&w\n\r", ch );
                                                return;
                                         }
                                                if ( familia_victima->enemigos[ch->pcdata->clan->numero] == ch->pcdata->clan->name )
                                                {
                                                /* Mensajitos de rigor */
                                                        sprintf( buf, "&wEl nivel de crueldad de tu guerra contra &W%s&w &wes &RANIKILACION TOTAL.&w\n\r",
                                                        familia_victima->name );
                                                        send_to_char( buf, ch );
                                                        sprintf( buf, "[&g &gLa familia &W%s&g &gdesea la ANIKILACION TOTAL de la familia &W%s&w&z].\n\r", ch->pcdata->clan->name, familia_victima->name );
                                                        echo_to_all(AT_GREY, buf, ECHOTAR_ALL );
                                                        for(d = first_descriptor; d; d = d->next)
                                                        {
                                                          if ( ch->pcdata->clan == d->character->pcdata->clan || ch == d)
                                                          {
                                                           send_to_char( "&g[  &RInfoGuerra   &g][&w Habeis llegado a una situacion imposible de dialogar con la   &g]\n\r", d->character);
                                                           ch_printf( ch, "&g   &w             &g [&w familia %s, las reglas para la mutua masacre seran estas: &g]\n\r", familia_victima->name);
                                                           send_to_char( "&g   &w             &g [&w Bonificaciones extra para la familia (Oro, posicion, etc...)  &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Posibilidad de decapitacion y diablerizacion desde nivel 2.   &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w 75% mas de stats al asesinar a un enemigo.                    &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Aversion a tus enemigos, les escupes, insultas etc, etc, si   &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w no pasas una tirada de Fuerza de Voluntad dificultad 8 te     &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w atacara automaticamente.                                      &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w En momentos de alta tension entre miembros de las dos familias&g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w en habitaciones seguras (SAFE) los jugadores pueden entrar en &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w frenesi o atakes de locura para los no vampiros, pudiendo     &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w llegar a perder el control de tu ficha por unos minutos.      &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Si tu familia mata una vez a todos y kada uno de los miembros &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w de la familia enemiga, la familia ganara en poder y posicion. &g]\n\r", d->character);
                                                          }
                                                        }

                                                        for(d = first_descriptor; d; d = d->next)
                                                        {
                                                          if ( familia_victima == d->character->pcdata->clan)
                                                          {
                                                           send_to_char( "&g[  &RInfoGuerra   &g][&w Habeis llegado a una situacion imposible de dialogar con la   &g]\n\r", d->character);
                                                           ch_printf( d->character, "&g   &w             &g [&w familia %s, las reglas para la mutua masacre seran estas: &g]\n\r", ch->pcdata->clan->name);
                                                           send_to_char( "&g   &w             &g [&w Bonificaciones extra para la familia (Oro, posicion, etc...)  &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Posibilidad de decapitacion y diablerizacion desde nivel 2.   &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w 75% mas de stats al asesinar a un enemigo.                    &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Aversion a tus enemigos, les escupes, insultas etc, etc, si   &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w no pasas una tirada de Fuerza de Voluntad dificultad 8 te     &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w atacara automaticamente.                                      &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w En momentos de alta tension entre miembros de las dos familias&g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w en habitaciones seguras (SAFE) los jugadores pueden entrar en &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w frenesi o atakes de locura para los no vampiros, pudiendo     &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w llegar a perder el control de tu ficha por unos minutos.      &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Si tu familia mata una vez a todos y kada uno de los miembros &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w de la familia enemiga, la familia ganara en poder y posicion. &g]\n\r", d->character);
                                                          }
                                                        }
                                                        /* Cambiamos los valores de odio y nivel de guerra de las familias enfrentadas */
                                                        familia_victima->nv_guerra[ch->pcdata->clan->numero] = 3;
                                                        /* Para la familia del jugador */
                                                        ch->pcdata->clan->odio[familia_victima->numero] = 3;
                                                        ch->pcdata->clan->enemigos[familia_victima->numero] = familia_victima->name;
                                                        familia_victima->enemigos[ch->pcdata->clan->numero] = ch->pcdata->clan->name;

                                                        /* Guardamos los archivos de las dos familias */
                                                        save_clan( ch->pcdata->clan );
                                                        save_clan( familia_victima );
                                                        return;
                                                }
                                            }

                                      return;
                                     }
                                     else
                                     {

                                     if ( str_prefix(arg3,"piadoso"  ) )
                                     {
                                     send_to_char( "Debes de elejir un nivel piadoso o dejar el nivel en blanco.\n\r", ch );
                                     return;
                                     }
                                                  for(d = first_descriptor; d; d = d->next)
                                                        {
                                                          if ( familia_victima == d->character->pcdata->clan)
                                                          {
                                                           send_to_char( "&g[  &RInfoGuerra   &g][&w Habeis llegado a una situacion imposible de dialogar con la   &g]\n\r", d->character);
                                                           ch_printf( d->character, "&g   &w             &g [&w familia %s, las reglas para la mutua masacre seran estas: &g]\n\r", ch->pcdata->clan->name);
                                                           send_to_char( "&g   &w             &g [&w Bonificaciones extra para la familia (Oro, posicion, etc...)  &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Posibilidad de decapitacion y diablerizacion desde nivel 2.   &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w 75% mas de stats al asesinar a un enemigo.                    &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Aversion a tus enemigos, les escupes, insultas etc, etc, si   &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w no pasas una tirada de Fuerza de Voluntad dificultad 8 te     &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w atacara automaticamente.                                      &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w En momentos de alta tension entre miembros de las dos familias&g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w en habitaciones seguras (SAFE) los jugadores pueden entrar en &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w frenesi o atakes de locura para los no vampiros, pudiendo     &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w llegar a perder el control de tu ficha por unos minutos.      &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w Si tu familia mata una vez a todos y kada uno de los miembros &g]\n\r", d->character);
                                                           send_to_char( "&g   &w             &g [&w de la familia enemiga, la familia ganara en poder y posicion. &g]\n\r", d->character);
                                                          }
                                                        }

                                                /* Mensajitos de rigor */
                                                        ch_printf( ch, "&wEmpiezas una guerra contra la familia &W%s&w.\n\r", familia_victima->name );
                                                        sprintf( buf, "&wEl nivel de crueldad de tu guerra contra &W%s&w &wes &ppiadoso.&w\n\r",
                                                        familia_victima->name );
                                                        send_to_char( buf, ch );
                                                        sprintf( buf, "[&g &gLa familia &W%s&g &gle declara la Guerra a la familia &W%s&w.&z]\n\r", ch->pcdata->clan->name, familia_victima->name );
                                                        echo_to_all(AT_DGREY, buf, ECHOTAR_ALL );
                                                        sprintf( buf, "[&g &gLa familia &W%s&g &gmantiene una actitud piadosa contra &W%s&w.&z]\n\r", ch->pcdata->clan->name, familia_victima->name );
                                                        echo_to_all(AT_DGREY, buf, ECHOTAR_ALL );



                                                        /* Cambiamos los valores de odio y nivel de guerra de las familias enfrentadas */
                                                        familia_victima->nv_guerra[ch->pcdata->clan->numero] = 1;

                                                        /* Para la familia del jugador */
                                                        ch->pcdata->clan->odio[familia_victima->numero] = 1;

                                                        ch->pcdata->clan->agresor[familia_victima->numero] = 0;
                                                        familia_victima->agresor[ch->pcdata->clan->numero] = 1;

                                                        /* Se asignan los valores de nombre */
                                                        ch->pcdata->clan->enemigos[familia_victima->numero] = familia_victima->name;
                                                        familia_victima->enemigos[ch->pcdata->clan->numero] = ch->pcdata->clan->name;

                                                        bug( "Enemigo del ch %s Enemigo del victim %s Niveles %d %d", ch->pcdata->clan->enemigos[familia_victima->numero]
                                                        , familia_victima->enemigos[ch->pcdata->clan->numero]
                                                        , ch->pcdata->clan->odio[familia_victima->numero]
                                                        , familia_victima->nv_guerra[ch->pcdata->clan->numero] );
                                                        /* Guardamos los archivos de las dos familias */
                                                        save_clan( ch->pcdata->clan );
                                                        save_clan( familia_victima );
                                                        return;
                                             }

 return;
		}
/* Else del  if ( !ch->pcdata->clan->deguerra[9]) */
else
{
            send_to_char( "Tu familia ya tiene nueve declaraciones de Guerra sera mejor que achantes un poko ya.\n\r", ch );
            return;
}
		return;
	  }

          /* Devolvemos el valor del void */
	  return;
}
