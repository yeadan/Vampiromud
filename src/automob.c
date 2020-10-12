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
 *                           Modulo de Automobs                             *
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


void crear_automob( CHAR_DATA *ch, CHAR_DATA *victim )
{
    OBJ_DATA *obj;
    char    dificultad[100];
    /*
     * Modulo del AutoMob VampiroMud2.0b 199x-200x
     */

    if((IS_NPC(victim)
         && ch->level > 15
         && IS_SET(ch->in_room->area->flags, AFLAG_AUTOAREA )))
    xSET_BIT(victim->act, ACT_AUTOMOB);

    if((xIS_SET(victim->act, ACT_AUTOMOB)
         && IS_NPC(victim)
         && !xIS_SET(victim->act, ACT_UALIZADO)))
    {
    MOB_INDEX_DATA *vnum = get_mob_index( victim->pIndexData->vnum );
    AREA_DATA *area = get_area( ch->in_room->area->name );

      int vida =1, nivel =1, dam =1, hit =1, atack =1;
      int armadura =-1, oro =1;

      if( vnum->vnum < area->low_m_vnum || vnum->vnum > area->hi_m_vnum )
    {
        bug( "%s automob ha intentado crearse en %s y esa no es su area!", victim->short_descr, area->name );
        ch_printf( ch, "&w%s &gno pertenece a este area, no se creara el automob!\n\r",
        victim->short_descr );
        return;
    }

      victim->nivel_real = victim->level;

      if(victim->level >= 10)
      	victim->dificultad = 10;
      else
      	victim->dificultad = victim->level;

      if(ch->level < 55)
      	nivel = (ch->level + victim->dificultad);
      if( ch->level < LEVEL_AVATAR && ch->level >=55 )
      	nivel = ( ch->level + victim->dificultad) + number_range(1, victim->dificultad);
      if(ch->level >= LEVEL_AVATAR)
      	nivel = (ch->level + victim->dificultad) + number_range(1, (victim->dificultad * 2));

        if(ch->level <= 15 )
        {
           vida = (21 * nivel) + (10 * victim->dificultad);
           dam  = nivel;
           hit  = nivel;
           atack = number_range(1, 3);
           armadura = number_range(-30, 30);
           oro = nivel * number_range(10, 70);
         }
         if(ch->level <= 45 && ch->level > 15)
        {
           vida = (45 * nivel) + (15 * victim->dificultad);
           dam  = nivel + (victim->dificultad *2);
           hit  = nivel + (victim->dificultad *2);
           atack = number_range(2, 4);
           armadura = number_range(-70, -30);
           oro = nivel * number_range(50, 170);
         }
         if(ch->level <= 65 && ch->level > 45)
        {
           vida = (65 * nivel) + (23 * victim->dificultad);
           dam  = nivel + (victim->dificultad *2);
           hit  = nivel + (victim->dificultad *2);
           atack = (victim->dificultad / 2) +1;
           armadura = number_range(-120, -70);
           oro = nivel * number_range(70, 230);
         }
         if(ch->level <= 125 && ch->level > 65)
        {
           vida = (330 * nivel) + (500 * victim->dificultad);
           dam  = nivel + (victim->dificultad *2);
           hit  = nivel + (victim->dificultad *2);
           atack = number_range(1, victim->dificultad);
           armadura = number_range(-320, -70);
           oro = nivel * number_range(100, 200);
         }

         if(ch->level < LEVEL_HERO && ch->level > 125)
         {
           if(victim->dificultad < 5)
           vida = ch->max_hit * victim->dificultad;

           if(victim->dificultad >= 5)
           vida = ch->max_hit * (victim->dificultad) + number_range(4000, 10000);

           dam  = nivel * (victim->dificultad) ;
           if (dam > 800)
           	dam = number_range(850, 1000);
           hit  = victim->dificultad * (nivel + number_range(-14, 100));
           atack = number_range(2, victim->dificultad);
           armadura = number_range(-520, -270);
           oro  = nivel * number_range(200, 600) + (victim->dificultad * 100);
         }

         if(ch->level >= LEVEL_HERO)
         {
           if(victim->dificultad > 1)
           vida = ch->max_hit * (victim->dificultad * number_range( 2, 5 ));

           if(victim->dificultad <= 1)
           vida = ch->max_hit * (victim->dificultad * 2) + number_range(400, 4000);

           dam  = nivel * (victim->dificultad);
           if (dam > 1600)
           	dam = number_range(1700, 2000);
           hit  = victim->dificultad * (nivel + number_range(-14, 100));

           atack = number_range(3, victim->dificultad);
           armadura = number_range(-520, -270);
           oro  = nivel * number_range(200, 700) + (victim->dificultad * 100);
         }

         victim->hit = vida;
         victim->max_hit = vida;
         victim->damroll = dam;
         victim->hitroll = hit;
         victim->numattacks = atack;
         victim->gold = oro;
         victim->level = nivel;
         xSET_BIT(victim->act, ACT_UALIZADO);

         do_restore( victim, victim->name );

         for ( obj = victim->first_carrying; obj; obj = obj->next_content )
         {
                if( !IS_OBJ_STAT( obj, ITEM_AUTOITEM ) )
                crear_autoobjeto( ch, victim, obj );
         }


         if(victim->dificultad == 1)
         sprintf(dificultad, "Baja" );
         else if(victim->dificultad == 2)
         sprintf(dificultad, "Facil" );
         else if(victim->dificultad == 3)
         sprintf(dificultad, "Normal" );
         else if(victim->dificultad == 4)
         sprintf(dificultad, "algo Elevada" );
         else if(victim->dificultad == 5)
         sprintf(dificultad, "Elevada" );
         else if(victim->dificultad == 6)
         sprintf(dificultad, "Un reto" );
         else if(victim->dificultad == 7)
         sprintf(dificultad, "Bestial" );
         else if(victim->dificultad == 8)
         sprintf(dificultad, "Una pasada" );
         else if(victim->dificultad == 9)
         sprintf(dificultad, "Demasiado elevada como para conseguirlo solo" );
         else
         sprintf(dificultad, "Casi Imposible" );

        ch_printf(ch, "&g%s te estudia y adapta sus cualidades fisicas a ti.\n\r", capitalize(victim->short_descr) );
        ch_printf(ch, "La dificultad es %s\n\r", dificultad);

     }
return;
}

