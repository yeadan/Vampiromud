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
 *         Modulo para sistema Vampire Roll                                 *
 *             No soportado por SMAUG1.4 ni superiores                      *
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

/* Vampiroroll.c una idea loca en un mundo loco loco */

#define IS_ROLLPLAYER(ch)                                   IS_VAMPIRE(ch)
#define ES_CHIQUILLO(ch)                                      (xIS_SET((ch)->act, PLR_ABRAZADO))

/* Funciones locales */
void mostrar_ficha args((CHAR_DATA *ch, int modo));

/* Comando abrazar para convertir mortales en crueles cainitas xD SiGo y SaNgUi */

int chiquillo;

void do_abrazar( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *abrazado;
        OBJ_DATA *obj;
        int bucle;
        char buf[MAX_STRING_LENGTH];


     if (IS_NPC(ch) )
     {
        send_to_char( "Los mobs no pueden abrazar a nadie.\n\r", ch );
        return;
      }

      if (!IS_VAMPIRE(ch) )
      {
        send_to_char( "Solo los Vampiros pueden abrazar y crear progenie\n\rLos no vampiros podeis usar el comando instruir.\n\r", ch );
        return;
      }

      if ( argument[0] == '\0' )
      {
        send_to_char( "A quien quieres abrazar?, recuerda que abrazar es un acto muy serio\n\r", ch );
        send_to_char( "transmitir la Herejia Cainita no siempre es agradable. Ayuda ABRAZAR.\n\r", ch );
        return;
       }

       if ( ( abrazado = get_char_room( ch, argument ) ) == NULL )
       {
        send_to_char( "Por que no intentas abrazar a alguien que este aqui?\n\r", ch );
        return;
       }

        if (abrazado == ch)
       {
        send_to_char( "Abrazarte a ti mismo... estas de broma no?\n\r", ch );
        return;
       }

       if ( IS_VAMPIRE(abrazado) )
       {
        sprintf(buf, "Ya fue abrazado por %s has llegado tarde no crees?\n\r", abrazado->sire_ch );
        send_to_char( buf , ch );
        return;
        }
       /* Abrazar en WIZINVIS provoca un CRASH... arreglado. KAYSER'02 */
       /* El CRASH era producido por la funcion get_char_world arreglado por SiGo
          el 24 de Julio del 2002 */
       /*if (xIS_SET(ch->act, PLR_WIZINVIS))
       {
       	send_to_char("Quitate el WIZINVIS para abrazar.\n\r",ch);
       	return;
       }*/

       if((!IS_VAMPIRE(abrazado)) && xIS_SET(abrazado->act, PLR_CHIQUILLO))
       {
         ch_printf( ch, "El ya fue adiestrado por %s y pertenece a los %ss.\n\r", abrazado->sire_ch, class_table[abrazado->class]->who_name );
         return;
       }

       if ( abrazado->level < 15 )
       {
        send_to_char( "No puedes abrazar a un pipiolo como ese.\n\r", ch );
        send_to_char( "Sientete codiciado, te han intentado abrazar.\n\r", abrazado );
        return;
       }

       if (( ch->num_chiquillos >= 10
           && !IS_IMMORTAL(ch)))
       {
        send_to_char( "Ya tienes 10 chiquillos no puedes tener mas descendencia.\n\r", ch );
        return;
        }

       if ( ch->level > 129 && abrazado->level > 129)
       {
       if ( abrazado->hit > (abrazado->max_hit * 20) / 100)
       {
        send_to_char( "Aun esta muy fresco para poder abrazarlo cansalo un poco mas.\n\r", ch );
        return;
       }
       else
       {
        sprintf( buf, "Has abrazado a %s lo sumes en letargo y le ofreces tu sangre.\n\r" , abrazado->name );
        send_to_char( buf, ch );
        sprintf( buf, "%s te ha sumido en un letargo, apenas queda sangre en tus venas\n\r", ch->name );
        send_to_char( buf, abrazado );
        sprintf( buf, "Moriras pronto si no aceptas la sangre que de su munyeca te ofrece %s.\n\r", ch->name );
        send_to_char( buf, abrazado );
        sprintf( buf,  "Si la aceptas toda tu vida cambiara y te convertiras en un cainital con un linaje de sangre de los %s.\n\r", class_table[ch->class]->who_name );
        }

        act( AT_GREY, "Te abres un tajo en la munyeca y derramas tu sangre.", ch, NULL, NULL, TO_CHAR );
        act( AT_GREY, "$n se abre un tajo en la munyeca y derrama su sangre.", ch, NULL, NULL, TO_ROOM );

        obj           = create_object( get_obj_index( OBJ_VNUM_BLOODLET ), 0 );
        obj->timer    = 5;
        obj->value[1] = 6;
        obj_to_room( obj, ch->in_room );

        send_to_char( "De repente te sientes sumido en un mundo extranyo, \n\r", abrazado  );
        send_to_char( "estas en la oscuridad solo tremendamente solo, tu corazon \n\r", abrazado );
        send_to_char( "hace apenas unos segundos latia, ahora no hace nada, te \n\r", abrazado );
        send_to_char( "sientes tan solo.... \n\r", abrazado );
        send_to_char( "...notas una gota de sangre en tus secos labios, es lo unico que \n\r", abrazado );
        send_to_char( "puedes sentir, sabes lo que es, el beso rojo y lo que significa \n\r", abrazado );
        send_to_char( "si aceptas la sangre ofrecida dejaras de estar vivo pero no estaras muerto. \n\r", abrazado );
        send_to_char( "Si aceptas el ofrecimiento solo tienes  beber la sangre derramada.\n\r", abrazado );

                xSET_BIT( abrazado->act, PLR_ABRAZADO );
                abrazado->cnt_abrazo = 5; /* Contador de tiempo antes de la muerte por falta de sangre del abrazado */

          abrazado->sire_ch = ch->name;
          abrazado->sire_class = ch->class;
          return;
        }

        if ( abrazado->position != POS_SLEEPING && abrazado->level < 129)
       {
        send_to_char( "Debe de estar dormido para que puedas abrazarlo.\n\r", ch );
        send_to_char( "Habla con algun Malkavian que lo solucione.\n\r", ch );
        return;
       }
       else
       {
        sprintf( buf, "Has abrazado a %s lo sumes en letargo y le ofreces tu sangre.\n\r" , abrazado->name );
        send_to_char( buf, ch );
        sprintf( buf, "%s te ha sumido en un letargo, apenas queda sangre en tus venas\n\r", ch->name );
        send_to_char( buf, abrazado );
        sprintf( buf, "Moriras pronto si no aceptas la sangre que de su munyeca te ofrece %s.\n\r", ch->name );
        send_to_char( buf, abrazado );
        sprintf( buf,  "Si la aceptas toda tu vida cambiara y te convertiras en un cainita con un linaje de sangre de los %s.\n\r", class_table[ch->class]->who_name );
        }

        act( AT_GREY, "Te abres un tajo en la munyeca y derramas tu sangre.", ch, NULL, NULL, TO_CHAR );
        act( AT_GREY, "$n se abre un tajo en la munyeca y derrama su sangre.", ch, NULL, NULL, TO_ROOM );

        obj           = create_object( get_obj_index( OBJ_VNUM_BLOODLET ), 0 );
        obj->timer    = 5;
        obj->value[1] = 6;
        obj_to_room( obj, ch->in_room );

        send_to_char( "De repente te sientes sumido en un mundo extranyo, \n\r", abrazado );
        send_to_char( "estas en la oscuridad solo tremendamente solo, tu corazon \n\r", abrazado );
        send_to_char( "hace apenas unos segundos latia, ahora no hace nada, te \n\r", abrazado );
        send_to_char( "sientes tan solo.... \n\r", abrazado );
        send_to_char( "...notas una gota de sangre en tus secos labios, es lo unico que \n\r", abrazado );
        send_to_char( "puedes sentir, sabes lo que es, el beso rojo y lo que significa \n\r", abrazado );
        send_to_char( "si aceptas la sangre ofrecida dejaras de estar vivo pero no estaras muerto. \n\r", abrazado );
        send_to_char( "Si aceptas el ofrecimiento solo tienes  beber la sangre derramada.\n\r", abrazado );

                xSET_BIT( abrazado->act, PLR_ABRAZADO );
                abrazado->cnt_abrazo = 5; /* Contador de tiempo antes de la muerte por falta de sangre del abrazado */

          abrazado->sire_ch = ch->name;
          abrazado->sire_class = ch->class;

        return;
        }

        void do_decision_hereje( CHAR_DATA *ch )
        {
            CHAR_DATA *victima;
            char buf[100];

            victima = get_char_world( ch, ch->sire_ch );
            victima->num_chiquillos +=1;

            if((!IS_IMMORTAL(victima))
                && victima->num_chiquillos >= 11)
            {
                bug( "%s ha conseguido llegar a la funcion decision_hereje a pesar de tener 10 chiquillos", victima->name);
                bug( "Anulando funcion decision_hereje devuelvo: Sire %s Chiquillo %s", victima->name, ch->name);
                return;
            }


            /* Se le asigna el nombre del chiquillo al Sire que viene de la funcion anterior */
            victima->chiquillos[victima->num_chiquillos] = ch->name;


                send_to_char( "Bebes de la sangre derramada por el que pasa a ser tu Sire!\n\r", ch);
                ch->class = ch->sire_class;                                        /* Cambiamos la clase del abrazado por la de su Sire */
                ch->practice += 30;                                                     /* Se le otorgan 30 Practicas para practicar los nuevos spells y skills de las antiguas class_table */
                asignar_disciplinas( ch );                                              /* Asignacion automatica de las disciplinas del jugador */
                send_to_char( "Sientes ena enorme necesidad de beber.... beber sangre.\n\r", ch );
                ch->pcdata->condition[COND_BLOODTHIRST] = 20;   /* Se le hace pasar sed de sangre */
                xREMOVE_BIT(ch->act, PLR_ABRAZADO);                 /* Se elimina el flag de Abrazado del jugador */
                xSET_BIT(ch->act, PLR_CHIQUILLO);                          /* Se activa el flag de Chiquillo ( entiendase chiquillo del jugador ke le abrazo ) */
                ch->cnt_abrazo = 0;                                                        /* Se resetea el contador de Abrazado */
                sprintf( buf, "%s pasa a ser el chiquillo de %s y se convierte en un Cainita del Clan %s", ch->name, victima->name, class_table[ch->class]->who_name );
                mensa_todos( ch, "abrazo", buf );
                return;
       }

/* Comando abrazar para convertir mortales en crueles Magos Asesinos Hadas y demas calaña xD SiGo y SaNgUi */

void do_adiestrar( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *abrazado;
        int chiquillo;
        int bucle;
        char buf[MAX_STRING_LENGTH];


     if (IS_NPC(ch) )
     {
        send_to_char( "Los mobs no pueden adiestrar a nadie.\n\r", ch );
        return;
      }

      if (IS_VAMPIRE(ch) )
      {
        send_to_char( "Solo los No-Vampiros pueden adiestrar y crear una escuela\n\rLos  vampiros podeis usar el comando abrazar.\n\r", ch );
        return;
      }

      if ( argument[0] == '\0' )
      {
        send_to_char( "A quien quieres adiestrar?, recuerda que adiestrar es un acto muy serio\n\r", ch );
        send_to_char( "transmitir tus conocimientos puede causarte problemas. Ayuda ADIESTRAR.\n\r", ch );
        return;
       }

        /* Adiestrar en WIZINVIS provoca un CRASH... arreglado. KAYSER'02 */
        if (xIS_SET(ch->act, PLR_WIZINVIS))
        {
	        send_to_char("Quitate el WIZINVIS para adiestrar.\n\r",ch);
        	return;
        }
                                 
       if ( ( abrazado = get_char_room( ch, argument ) ) == NULL )
       {
        send_to_char( "Por que no intentas adiestrar a alguien que este aqui?\n\r", ch );
        return;
       }

       if (abrazado == ch)
       {
        send_to_char( "Adiestrarte a ti mismo... estas de broma no?\n\r", ch );
        return;
       }

       if( IS_VAMPIRE(abrazado) )
       {
                send_to_char( "Como vas a adiestrar a un Cainita?.\n\r", ch );
                return;
       }

       if ( xIS_SET(abrazado->act, PLR_ABRAZADO))
       {
        sprintf(buf, "Ya fue adiestrado por %s has llegado tarde no crees?\n\r", abrazado->sire_ch );
        send_to_char( buf , ch );
        return;
        }

       if ( abrazado->level < 15 )
       {
        send_to_char( "No puedes adiestrar a un pipiolo como ese.\n\r", ch );
        send_to_char( "Sientete codiciado, te han intentado adiestrar.\n\r", abrazado );
        return;
       }


       if (( ch->num_chiquillos >= 10
           && !IS_IMMORTAL(ch)))
       {
        send_to_char( "Ya tienes 10 aprendices tu escuela no puede ser mayor.\n\r", ch );
        return;
        }

       if ( ch->level > 129 && abrazado->level > 129)
       {
       if ( abrazado->hit > (abrazado->max_hit * 20) / 100)
       {
        send_to_char( "Aun no esta muy convencido de tus leches y habilidades dale un par de guantazos mas.\n\r", ch );
        return;
       }
       else
       {
        sprintf( buf, "Has adiestrado a %s lo sumes en letargo solo le has dejado 2 neuronas conscientes machote.\n\r" , abrazado->name );
        send_to_char( buf, ch );
        sprintf( buf, "%s te ha sumido en un letargo, madre mia cuanto sabes se te va la ollaaaaaaaa!\n\r", ch->name );
        send_to_char( buf, abrazado );
        sprintf( buf, "Moriras pronto si no asimilas el conocimiento que  %s te ofrece.\n\r", ch->name );
        send_to_char( buf, abrazado );
        sprintf( buf,  "Teclea asimilar en el entrenador si quieres aceptar el conocimiento de los %s\n\rTambien puedes esperar a morir.\n\r", class_table[ch->class]->who_name );
        }

        act( AT_GREY, "Abres tu mente y transmites tus conocimientos ocultos a $N.", abrazado, NULL, NULL, TO_CHAR );
        act( AT_GREY, "$n te transmite todos sus conocimientos.", ch, NULL, NULL, TO_VICT );
        send_to_char( "Recuerda teclea asimilar escuela si quieres aprender sus habilidades.\n\r", abrazado );

                xSET_BIT( abrazado->act, PLR_ABRAZADO );
                abrazado->cnt_abrazo = 5; /* Contador de tiempo antes de la muerte por falta de sangre del abrazado */

          abrazado->sire_ch = ch->name;
          abrazado->sire_class = ch->class;
          return;
        }

        if ( abrazado->position != POS_SLEEPING && abrazado->level < 129)
       {
        send_to_char( "Debe de estar dormido para que puedas abrazarlo.\n\r", ch );
        send_to_char( "Habla con algun Malkavian que lo solucione.\n\r", ch );
        return;
       }
       else
       {
        sprintf( buf, "Has adiestrado a %s lo sumes en letargo solo le has dejado 2 neuronas conscientes machote.\n\r" , abrazado->name );
        send_to_char( buf, ch );
        sprintf( buf, "%s te ha sumido en un letargo, madre mia cuanto sabes se te va la ollaaaaaaaa!\n\r", ch->name );
        send_to_char( buf, abrazado );
        sprintf( buf, "Moriras pronto si no asimilas el conocimiento que  %s te ofrece.\n\r", ch->name );
        send_to_char( buf, abrazado );
        sprintf( buf,  "Teclea asimilar 'escuela' si quieres aceptar el conocimiento de los %s\n\rTambien puedes esperar a morir.\n\r", class_table[ch->class]->who_name );
        }

        act( AT_GREY, "Abres tu mente y transmites tus conocimientos ocultos a $n.", abrazado, NULL, NULL, TO_CHAR );
        act( AT_GREY, "$n te transmite todos sus conocimientos.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "Recuerda teclea asimilar escuela si quieres aprender sus habilidades.\n\r", abrazado );

                xSET_BIT( abrazado->act, PLR_ABRAZADO );
                abrazado->cnt_abrazo = 5; /* Contador de tiempo antes de la muerte por falta de sangre del abrazado */

          abrazado->sire_ch = ch->name;
          abrazado->sire_class = ch->class;

        return;
        }

        void do_decision_mortales( CHAR_DATA *ch )
        {
            CHAR_DATA *victima;

            char buf[255];

            victima = get_char_world( ch, ch->sire_ch );
            victima->num_chiquillos +=1;

            if((!IS_IMMORTAL(victima))
                && victima->num_chiquillos >= 11)
            {
                bug( "%s ha conseguido llegar a la funcion decision_mortales a pesar de tener 10 aprendices", victima->name);
                bug( "Anulando funcion decision_mortales devuelvo: Maestro %s Aprendiz %s", victima->name, ch->name);
                return;
            }



            /* Se le asigna el nombre del chiquillo al Sire que viene de la funcion anterior */
            victima->chiquillos[victima->num_chiquillos] = ch->name;


                send_to_char( "Aceptas el conocimiento del que ahora es tu Mentor!\n\r", ch);
                do_help( ch, "mensmentor" );
                ch->class = ch->sire_class;                                        /* Cambiamos la clase del abrazado por la de su Sire */
                ch->practice += 30;                                                     /* Se le otorgan 30 Practicas para practicar los nuevos spells y skills de las antiguas class_table */
                /*asignar_disciplinas( ch );*/                                              /* Asignacion automatica de las disciplinas del jugador */
                send_to_char( "Sientes un enorme picor enla entrepierna... y eso?\n\r", ch );
                xREMOVE_BIT(ch->act, PLR_ABRAZADO);                 /* Se elimina el flag de Abrazado del jugador */
                xSET_BIT(ch->act, PLR_CHIQUILLO);                          /* Se activa el flag de Chiquillo ( entiendase chiquillo del jugador ke le abrazo ) */
                ch->cnt_abrazo = 0;                                                        /* Se resetea el contador de Abrazado */
                sprintf( buf, "%s pasa a ser el discipulo de %s y se convierte en un poderoso %s", ch->name, victima->name, class_table[ch->class]->who_name );
                mensa_todos( ch, "adiestro", buf );
                return;
       }



/*
 * Sistemas de mensajes internos
 */

void do_mensaje(CHAR_DATA *ch, char *argument)
{
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];

        CHAR_DATA *victima;

        /* Le decimos al compilador como funcionan los argumentos */
         argument = one_argument( argument, arg1 );
         argument = one_argument( argument, arg2 );
         strcpy( arg3, argument );

         if( arg1[0] == '\0' )
         {

                send_to_char( "&wSintaxis:      menasaje <jugador/todos> <canal> <mensaje>\n\r", ch );
                return;
         }

         if( arg3[0] == '\0')
        {
                send_to_char( "Debes de poner todos los argumentos melonazo.\n\r", ch);
                return;
        }

         if((!str_prefix(arg1, "todos")
         && arg2[0] != '\0' && arg3[0] != '\0' ))
         {
          mensa_todos(ch, arg2, arg3);
          return;
         }

         victima = get_char_world(ch, arg1);

         if (!victima)
         {
                send_to_char( "Me da a mi la impresion de que ese tipo no esta jugando ahora he.\n\r", ch);
                return;
         }

         mensa(victima, arg2, arg3);
         ch_printf(ch, "Ok mensaje enviado a %s.\n\r", victima->name);
         mensa(ch, arg2, arg3);
 return;
}

//ESTA FUNCION DBERIA RETORNAR UN INT Q ES EL RESULT DE LA TIRADA
void tira_dados( CHAR_DATA *ch, int ndados, int ncaras, int bonus, int dificultad )
{
  int dados[ndados];
  int cnt;
  int exitos = 0;
  int pifias = 0;
  int fallos = 0;

  /*
   * SISTEMAS DE TIRADAS DE DADOS XD
   *                            By: SaNgUi & SiGo
   *
   * Exitos: Las tiradas que superan la dificultad
   * Pifias: Las que salen 1
   * Fallos: Las tiradas que no superan la difcultad
   *
   */

  for ( cnt = 0; ndados; cnt++ )
  {
    dados[cnt] = number_range(1, ncaras);
    if (dados[cnt] >= dificultad )
      exitos++;
    if (dados[cnt] = 1)
      pifias++;
    if (dados[cnt] < dificultad)
      fallos++;
   }

     if ( pifias > exitos ) {/* la has cagao tio,tienes mas pifias q exitos,algo malo t va a  pasar,MUAHAHAHA*/}
  

/* La variable que se usara mas adelante */
   
     ch->tira_dado = ( exitos - (fallos+pifias) ) + bonus ;

/* Return del Void */
  return;
}

/*
 * Nueva ficha para Vampiro Mundo de Tinieblas VS SMAUG1.4
 */
void do_vficha(CHAR_DATA *ch, char *arg )
{
        if(!ch)
        {
        bug( "Jugador Nulo do_vficha", 0);
        return;
        }

        /*
         * Esto llama a la funcion que muestra la ficha al buffer
         */
        if ( arg[0] == '\0')
        {
                mostrar_ficha( ch, ch->modo_ficha);
                return;
        }


        if (!str_prefix( arg, "simple" ))
        {
        ch->modo_ficha = 0;
        send_to_char( "&wTu modo de ficha ahora es &gSIMPLE &wteclea ficha para ver tus stats.\n\r", ch);
        return;
        }

        if (!str_prefix( arg, "normal" ))
        {
        ch->modo_ficha = 1;
        send_to_char( "&wTu modo de ficha ahora es &gNORMAL &wteclea ficha para ver tus stats.\n\r", ch);
        return;
        }

        if (!str_prefix( arg, "completa" ))
        {
        ch->modo_ficha = 2;
        send_to_char( "&wTu modo de ficha ahora es &gCOMPLETA &wteclea ficha para ver tus stats.\n\r", ch);
        return;
        }

        if (!str_prefix( arg, "inmortal" ))
        {
        if ( !IS_IMMORTAL(ch))
        return;

        ch->modo_ficha = 100;
        send_to_char( "&wTu modo de ficha ahora es de &gINMORTAL &wteclea ficha para ver tus stats.\n\r", ch);
        return;
        }

        if (!str_prefix( arg, "secreta" ))
        {
        ch->modo_ficha = 80;
        send_to_char( "&wTu modo de ficha ahora es &gSECRETA &wteclea... no te digo 'na' que es secreto.\n\r", ch);
        return;
        }

        if (!str_prefix( arg, "help")
         || !str_prefix( arg, "ayuda"))
        {
        send_to_char( "&wSintaxis: &gficha &w<SIMPLE|NORMAL|COMPLETA|INMORTAL|???|NOVATO>.\n\r", ch);
        send_to_char( "Sin argumentos muestra tu ficha.\n\r", ch);
        send_to_char( "En caso de que no entiendas algo... Ayuda 'FICHA'.\n\r", ch);
        return;
        }

        if (!str_prefix( arg, "novato" ))
        {
        ch->modo_ficha = -1;
        send_to_char( "&wTu modo de ficha ahora es &gNOVATO(POLLO) &wteclea... ficha y te explicaremos.\n\r", ch);
        return;
        }

        if (str_prefix(arg, "help")
        || str_prefix(arg, "ayuda")
        || str_prefix(arg, "inmortal")
        || str_prefix(arg, "novato")
        || str_prefix(arg, "secreta")
        || str_prefix(arg, "completa")
        || str_prefix(arg, "simple")
        || str_prefix(arg, "normal"))
        {
        send_to_char( "&wSintaxis: &gficha &w<SIMPLE|NORMAL|COMPLETA|INMORTAL|???|NOVATO>.\n\r", ch);
        send_to_char( "Sin argumentos muestra tu ficha.\n\r", ch);
        send_to_char( "En caso de que no entiendas algo... Ayuda 'FICHA'.\n\r", ch);
        return;
        }
 send_to_char("\n\r", ch);
 return;
}

/*
 * Esto es lo que te muestra la ficha
 * No es soportado por SMAUG1.4
 */
void mostrar_ficha(CHAR_DATA *ch, int modo)
{
        CLAN_DATA *familia;
        AFFECT_DATA *af;
        AREA_DATA *tarea;
	CON_DATA *conquista;
        char *estrella = "&w******************************************************************************\n\r";
        char *barra = "&g-----------------------------------&r===&OoO0Oo&r===&g--------------------------------\n\r";
        char *cabfam = "&g---------------------------------&wDaToS De FaMiLia&g-----------------------------\n\r";
        char *cabcain = "&g----------------------------------&wDaToS CaiNiTas&g------------------------------\n\r";
        char *cabpk = "&g------------------------------------&wDaToS De PK&g-------------------------------\n\r";
        char *cabstat ="&g--------------------------------------&wSTaTs&g-----------------------------------\n\r";
        char *cabinm = "&g---------------------------------&wDaToS De InMoRTaL&g----------------------------\n\r";
        char *cabgen = "&g---------------------------------&wDaToS GeNeRaLeS&g------------------------------\n\r";
        char *famsec = "&w***********************************&cMiS CoLeGaS&w********************************\n\r";
        char *statsec = "&w**************************&caMoS a VeR Lo ToCHo Que eSToY&w***********************\n\r";
        char *pksec =   "&w*************************&caMoS a VeR Lo HiJoPuTa Que SoY&w***********************\n\r";
        char *inmsec = "&w*********************************&cDaToS De InMoRaL&w*****************************\n\r";
        char *gensec = "&w*******************************&cBoRReGuiSMo GeNeRaL&w****************************\n\r";
        char *papelwc = "&wVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV\n\r";
        char poder[MAX_STRING_LENGTH];
        char secta[MAX_STRING_LENGTH];
        char etipk[MAX_STRING_LENGTH];
        int gana_por;
        int pierde_por;

        char buf[MAX_STRING_LENGTH];
        char buf1[MAX_STRING_LENGTH];

        if (!ch)
        {
                bug( "Jugador Nulo en mostrar_ficha. vampireroll.c", 0);
                return;
        }

        if( ( ch->modo_ficha >= 0 && ch->modo_ficha != 80) )
        {
        	pager_printf(ch, "\n\r");;
                pager_printf(ch, barra);
                pager_printf(ch, cabgen);
                if( !IS_NPC(ch))
                {
                pager_printf(ch, "&wNombre: &c%6.6s,%-34.34s         &wMaxNivel: &c%-3.3d\n\r", ch->name, ch->pcdata->title, ch->pcdata->max_level );
                pager_printf(ch, "&wNivel : &c%-3.3d         &wAnyos: &c%-4.4d &w       Horas    :&c %-4.4d&w    Remort  : &c%d\n\r", ch->level, get_age(ch) ,((get_age(ch) - 17) * 2),ch->pcdata->renacido);
                }
                else
                {
                pager_printf(ch, "&wNombre: &c%6.6s,%-34.34s\n\r", ch->short_descr );
                pager_printf(ch, "&wNivel : &c%-3.3d         &wAnyos: &c%-4.4d &w       Horas    :&c %-4.4d&w\n\r", ch->level, get_age(ch) ,((get_age(ch) - 17) * 2));
                }

    switch (ch->position)
    {
	case POS_DEAD:
		sprintf(buf, "muerto");
		break;
	case POS_MORTAL:
		sprintf(buf, "agonizando");
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
                sprintf(buf, "evasivo");
                break;
        case POS_DEFENSIVE:
                sprintf(buf, "defensivo");
                break;
        case POS_AGGRESSIVE:
                sprintf(buf, "agresivo");
                break;
        case POS_BERSERK:
                sprintf(buf, "berserker");
                break;
	case POS_MOUNTED:
		sprintf(buf, "montado");
		break;
        case POS_SITTING:
		sprintf(buf, "sentado");
		break;
	case POS_PREDECAP:
		sprintf(buf, "herido_mortalmente");
    }
                pager_printf(ch, "&wPeso  : &c%-3.3d Kg      &wOro  : &Y%-10.10s&x &w Practicas:&c %-4.4d &w   Posicion:&c %-12.12s\n\r", (ch->weight /2), num_punct(ch->gold), ch->practice, buf);
        if (IS_VAMPIRE(ch) )
        {
                if (ch->race == 1)
                sprintf( secta, "&wpeteneces al &cSabbath&w.\n\r" );
                else if (ch->race == 2)                sprintf( secta, "&wpeteneces a la &cCamarilla&w.\n\r" );
                else if (ch->race == 3)                sprintf( secta, "&weres &cIndependiente&w.\n\r" );
                else
                sprintf( secta, "&wno perteneces a algo acorde con tu naturaleza cainita&w.\n\r" );

                pager_printf(ch, "&wEres miembro del clan &c%s &wy &c%s", capitalize(get_class(ch)), secta);
        }
        else
          pager_printf(ch, "&wEres miembro de los &c%s.\n\r", capitalize(get_class(ch)));

                pager_printf(ch, cabstat );
                if ( ch->modo_ficha != 0)
                pager_printf(ch, "&wFue   : &c%-3.3d&w/&c%-3.3d     &wCon  : &c%-3.3d&w/&c%-3.3d     &wDes      : &c%-3.3d&w/&c%-3.3d &wInt     : &c%-3.3d&w/&c%-3.3d\n\r",
                get_curr_str(ch), ch->perm_str,
                get_curr_con(ch), ch->perm_con,
                get_curr_dex(ch), ch->perm_dex,
                get_curr_int(ch), ch->perm_int);
                if ( ch->modo_ficha != 0)
                pager_printf(ch, "&wSab   : &c%-3.3d&w/&c%-3.3d     &wSue  : &c%-3.3d&w/&c%-3.3d     &wCar      : &c%-3.3d&w/&c%-3.3d &wMan     : &c%-3.3d&w/&c010\n\r",
                get_curr_wis(ch), ch->perm_wis,
                get_curr_lck(ch), ch->perm_lck,
                get_curr_cha(ch), ch->perm_cha,
                ch->manipulacion);
                pager_printf(ch, "&wAstuc : &c%-3.3d&w/&c010     &wInti : &c%-3.3d&w/&c010     &wOcultismo: &c%-3.3d&w/&c010 &wAlerta  : &c%-3.3d&w/&c010\n\r",
                ch->astucia,
                ch->intimidacion,
                ch->ocultismo,
                ch->alerta);
                if (ch->level > 15)
                {
                    if (GET_AC(ch) >= 25)
	sprintf(buf, "Vas peor que DESNUDO!");
    else if (GET_AC(ch) >= 20)
	sprintf(buf, "Vas sin nada que te proteja");
    else if (GET_AC(ch) >= 15)
	sprintf(buf, "Vas con unos pocos arapos");
    else if (GET_AC(ch) >= 0)
	sprintf(buf, "Vas mal vestido");
    else if (GET_AC(ch) >= -10)
	sprintf(buf, "Estas sin armadura");
    else if (GET_AC(ch) >= -20)
	sprintf(buf, "Estas mal protegido");
    else if (GET_AC(ch) >= -40)
	sprintf(buf, "Estas algo protegido");
    else if (GET_AC(ch) >= -70)
	sprintf(buf, "Estas bien protegido");
    else if (GET_AC(ch) >= -70)
	sprintf(buf, "Estas muy bien protegido");
    else if (GET_AC(ch) >= -130)
	sprintf(buf, "No te destruiria ni un dragon");
    else if (GET_AC(ch) >= -190)
	sprintf(buf, "Te sientes muy protegido");
    else if (GET_AC(ch) >= -300)
	sprintf(buf, "Eres la envidia de los reyes");
    else if (GET_AC(ch) >= -400)
	sprintf(buf, "Eres como un avatar");
    else if (GET_AC(ch) >= -1000)
	sprintf(buf, "Eres INVENCIBLE!");
    else
        sprintf(buf, "ToDoS KieReN KoMeRTe eL MieMBRo!");

                 pager_printf(ch, "&wAC    : &c%-4.4d        %-32.32s      &wAline   : &c%-4.4d\n\r", GET_AC(ch),buf,ch->alignment);
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
                 pager_printf(ch, "&wHit   : &Y%-4.4d        &x&wDam  : &Y%-4.4d   &x                        &wEres &c%s\n\r", GET_HITROLL(ch), GET_DAMROLL(ch), buf);
        }
        pager_printf(ch, "&wVida  : &c%-9.9s&w   De   : &W%-9.9s  &x &wCobardia : &c%-5.5d\n\r", num_punct(ch->hit), num_punct(ch->max_hit), ch->wimpy);
        switch (ch->style) {
        case STYLE_EVASIVE:
                sprintf(buf, "Evasivo");
                break;
        case STYLE_DEFENSIVE:
                sprintf(buf, "Defensivo");
                break;
        case STYLE_AGGRESSIVE:
                sprintf(buf, "Agresivo");
                break;
        case STYLE_BERSERK:
                sprintf(buf, "Berserk");
                break;
        default:
                sprintf(buf, "Estandar");
                break;
        }
        if(!IS_VAMPIRE(ch))
        pager_printf(ch, "&wMana  : &c%-9.9s&w   De   : &W%-9.9s     \n\r", num_punct(ch->mana), num_punct(ch->max_mana));

        pager_printf(ch, "&x&wMove  : &c%-9.9s&w   De   : &W%-9.9s  &x &wEstilo   : &c%s\n\r", num_punct(ch->move), num_punct(ch->max_move), buf);

        if((exp_level( ch, ch->level + 1) - ch->exp) <= 2500)
        sprintf( buf, "&r" );
        if((exp_level( ch, ch->level + 1) - ch->exp) <= 1500)
        sprintf( buf, "&c" );
        if((exp_level( ch, ch->level + 1) - ch->exp) <= 1000)
        sprintf( buf, "&b" );
        if((exp_level( ch, ch->level + 1) - ch->exp) <= 500)
        sprintf( buf, "&Y" );
        if((exp_level( ch, ch->level + 1) - ch->exp) <= 100)
        sprintf( buf, "&W" );/*KAYSER 2004*/
        pager_printf(ch, "&wExp.  : &c%-7d&w     Nivel: %-4.4d", ch->exp, exp_level(ch, ch->level+1) - ch->exp);
        if (IS_HERO(ch))
        pager_printf(ch, "       &wExp.Acum : &c%-5.5d\n\r", ch->exp_acumulada);
        else
        pager_printf(ch, "\n\r");
        pager_printf(ch, "&wPeso  : &c%-7.7d&w/&c%-7.7d   &w             Objetos  : &c%-5.5d&w/&c%-5.5d&w\n\r", ch->carry_weight, can_carry_w(ch), ch->carry_number, can_carry_n(ch));


        if (ch->modo_ficha > 0)
        {
        if (IS_VAMPIRE(ch))
          {
           pager_printf(ch, cabcain);
           pager_printf(ch,"&wClan  : &c%-9.9s",capitalize(get_class(ch)));
           pager_printf(ch," &wSecta   :&c %-13.13s &wGeneracion: &r%-2.2d      &wFrenesi: &c%-2.2d &w/&r 10\n\r", capitalize(get_race(ch)), ch->generacion, ch->frenesi );
           pager_printf(ch,"&wSire  : &c%-9.9s &wCamino  : &c%-13.13s&w Puntos Cam: &c%-2.2d&w / &r10 &wF.Volun: &c%2.2d &w/&r 10\n\r", ch->sire_ch, ch->camino, ch->pcamino, ch->f_voluntad);
          }

         if (IS_CLANNED(ch))
         {
         sh_int areas = 0;
         int cnt = 0;
         int pwr = 0;

         for ( tarea = first_area; tarea; tarea = tarea->next )
         {
	 	conquista = get_conquista( tarea->name );
                if(ch->pcdata->clan->name == conquista->propietario)
                areas++;
         }

          if (ch->pcdata->clan->members != 0)
	  pwr = ch->pcdata->clan->puntos;
          else
	  pwr = 1;

          if (pwr < 100)            sprintf(poder, "&gTu familia es considerada menos que una &Omierda&w.\n\r");
          else if (pwr < 330)       sprintf(poder, "&gLa gente ha oido hablar algo de tu familia.\n\r");
          else if (pwr < 650)       sprintf(poder, "&gEs considerada otra mas.\n\r");
          else if (pwr < 866)       sprintf(poder, "&gLa gente empieza a confiar en tu familia.\n\r");
          else if (pwr < 1135)      sprintf(poder, "&gLas demas familias empiezan a respetarte.\n\r");
          else if (pwr < 2000)      sprintf(poder, "&gEmpezais a controlar a los demas.\n\r");
          else if (pwr < 5000)      sprintf(poder, "&gCasi todos os sonrien por la calle, sienten miedo...\n\r");
          else if (pwr < 7500)      sprintf(poder, "&gSimplemente... os temen.\n\r");
          else if (pwr < 10000)     sprintf(poder, "&wLa penya se caga encima al veros...\n\r");
          else if (pwr < 15000)     sprintf(poder, "&WSois los PuToS aMoS del Mundo de Tinieblas...&w\n\r");
          else
          sprintf(poder, "&RSe NoTa, Se SieNTe &W%s&R eSTaN PReSeNTeS!\n\r", ch->pcdata->clan->name);

           pager_printf(ch, cabfam);
           if (!str_cmp(ch->name, ch->pcdata->clan->leader))
           {
             pager_printf(ch,"&wEres el &gLider&w de la familia&w %s.\n\r", ch->pcdata->clan->name );
             pager_printf(ch,"&wTeneis &c%d&w areas conquistadas con un total de &c%d&w tierras.\n\r", areas, ch->pcdata->clan->tierras );
             pager_printf(ch,"&wTu has conquistado &c%d &wareas para tu familia.\n\r", ch->conquistas );
             if(ch->modo_ficha > 1)
             {
             pager_printf(ch,"&wLos &c%d&w miembros de tu familia han asesinado a &w%d&w mobs\n\ry han sido asesinados &c%d&w veces por mobs.\n\r",ch->pcdata->clan->members, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
             pager_printf(ch,"&wTu familia ha conquistado un total de &c%d&w areas en su historia.\n\r", ch->pcdata->clan->conquistas);
             pager_printf(ch,"&wTeneis &Y%d&w onzas de oro, &c%d&w puntos y &c%d&w puntos de experiencia.\n\r", ch->pcdata->clan->oro, ch->pcdata->clan->puntos, ch->pcdata->clan->experiencia);
             }
             pager_printf(ch,poder );
           }
           else if (ch->pcdata->clan->number1 == ch->name )
           {
           pager_printf(ch, "&wEres el &gHeredero&w de la familia &w%s\n\r", ch->pcdata->clan->name );
           pager_printf(ch,"&wTeneis &c%d&w areas conquistadas con un total de &c%d&w tierras.\n\r", areas, ch->pcdata->clan->tierras );
           pager_printf(ch,"&wTu has conquistado &c%d &wareas para tu familia.\n\r", ch->conquistas );
           pager_printf(ch,poder );
           }
           else if (ch->pcdata->clan->number2 == ch->name )
           {
           pager_printf(ch, "&wEres el &gConsejero&w de la familia &w%s\n\r", ch->pcdata->clan->name );
           pager_printf(ch,"&wTeneis &c%d&w areas conquistadas.\n\r", areas);
           pager_printf(ch,"&wTu has conquistado &c%d &wareas para tu familia.\n\r", ch->conquistas );
           pager_printf(ch,poder );
           }
           else if (ch->pcdata->clan->deity == ch->name )
           {
           pager_printf(ch, "&wEres el &gPRoTeCToR&w de la familia &w%s\n\r", ch->pcdata->clan->name);
           pager_printf(ch,"&wTeneis &c%d&w areas conquistadas con un total de &c%d&w tierras.\n\r", areas, ch->pcdata->clan->tierras );
           pager_printf(ch,"&wTu has conquistado &c%d &wareas para tu familia.\n\r", ch->conquistas );
           if(ch->modo_ficha > 1)
           {
           pager_printf(ch,"&wTu familia ha conquistado un total de &c%d&w areas en su historia.\n\r", ch->pcdata->clan->conquistas);
           pager_printf(ch,"&wLos &c%d&w miembros de tu familia han asesinado a &w%d&w mobs\n\ry han sido asesinados &c%d&w veces por mobs.\n\r",ch->pcdata->clan->members, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
           pager_printf(ch,"&wTeneis &Y%d&w onzas de oro, &c%d&w puntos y &c%d&w puntos de experiencia.\n\r", ch->pcdata->clan->oro, ch->pcdata->clan->puntos, ch->pcdata->clan->experiencia);
           }
           pager_printf(ch,poder );
           }
           else
           {
             pager_printf(ch, "&wEres miembro de la familia &w%s\n\r", ch->pcdata->clan->name );
             pager_printf(ch,"&wTu has conquistado &c%d &wareas para tu familia.\n\r", ch->conquistas );
             pager_printf(ch,"&wTeneis &c%d&w areas conquistadas.\n\r", ch->pcdata->clan->conquistas);
             pager_printf(ch,poder );
           }


          areas = 0;
         }
          if (ch->level >= 15)
          {
            /*
             * Cabecera PK
             */

             /* Definicion de la etiketa de pk etipk */
             int calpk = 0;
             calpk = ( ch->pcdata->pkills - ch->pcdata->pdeaths );

             if ( calpk <= -150 )       sprintf(etipk, "&pEres la victima perfecta\n\r" );
             else if (calpk <= -75 )    sprintf(etipk, "&oDeberias plantearte dejar de jugar a esto.\n\r" );
             else if (calpk <= -25 )    sprintf(etipk, "&wSi yo se que tu lo intentas, pero eres un mierda en pk.\n\r" );
             else if (calpk <= 0 )      sprintf(etipk, "&wO una de dos, o eres nuevo o eres muy malo\n\r" );
             else if (calpk <= 5 )      sprintf(etipk, "&wEmpiezas a saber como destripar a la gente.\n\r" );
             else if (calpk <= 15  )    sprintf(etipk, "&wEmpiezas a desenvolverte en eso del pk.\n\r" );
             else if (calpk <= 25 )     sprintf(etipk, "&wLa gente te pide permiso para salir a matar.\n\r" );
             else if (calpk <= 45 )     sprintf(etipk, "&wDas miedo cuando vas a hacer px.\n\r" );
             else if (calpk <= 65)      sprintf(etipk, "&wNi el Kayser en sus tiempos mozos.\n\r" );
             else if (calpk <= 75 )     sprintf(etipk, "&wEn tu casco pone &gNaSio Pa MaTa&w.\n\r" );
             else if (calpk <= 100 )    sprintf(etipk, "&WHasta los dioses te piden permiso para hacer algo nel mud.\n\r" );
             else if (calpk <= 200 )    sprintf(etipk, "&WHas pensao en hacerte tu propio mud??\n\r" );
             else
             sprintf(etipk, "&wContesta!! Quien ta tokao la ficha??\n\r");

             if( !IS_NPC(ch))
             {
             if (IS_IMMORTAL(ch))
             sprintf(etipk, "&wPk?? Pk pa ke?? Pa kagarla? &YSLAY&w!!!!\n\r");

            /* Porcentajes de Pk */
            if(ch->pcdata->pkills + ch->pcdata->pdeaths != 0)
            {
            gana_por = (100 * ch->pcdata->pkills) / (ch->pcdata->pkills + ch->pcdata->pdeaths);
            pierde_por = (100 * ch->pcdata->pdeaths) / (ch->pcdata->pkills + ch->pcdata->pdeaths);
            }
            else
            {
            gana_por = 0;
            pierde_por = 0;
            }

            pager_printf(ch,cabpk);
            pager_printf(ch, etipk);

            pager_printf(ch,"&wHas salido vencedor de &c%d&w combates y te han humillado &c%d&w veces.\n\r", ch->pcdata->pkills, ch->pcdata->pdeaths);
            if(ch->modo_ficha > 1)
            pager_printf(ch,"&wHas ganado un &c%d% &wde conflictos y has perdido un &c%d%&w.\n\r", gana_por, pierde_por);
            if (ch->pcdata->illegal_pk <= 0 )
            pager_printf(ch,"&wEres un angelito y no has cometido ninguna pk ilegal.\n\r");
            else
            pager_printf(ch,"&wEres un cabron y has cometido &c%d&w pk's ilegales.\n\r", ch->pcdata->illegal_pk);
            }
         }
        }



                  /*ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths, ch->pcdata->genepkills );*/
                /*ch_printf( ch, "%s%s%s%s%s%s%s%s%s%s%s%s%s",barra, estrella, cabfam, cabpk, cabstat, cabinm, cabgen, famsec, statsec, pksec, inmsec, gensec, papelwc );*/
        }
return;
}







