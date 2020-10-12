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
 *                           Modulo de Autoobjetos                          *
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

int value;
int loca;

void crear_autoobjeto(  CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj )
{
        AFFECT_DATA *aff, *paf;
        AREA_DATA *area = get_area( ch->in_room->area->name );
        int vnum = obj->pIndexData->vnum;
        bool opera = TRUE;

        if( vnum < area->low_o_vnum || vnum > area->hi_o_vnum )
        {
                bug( "%s tenia %s que no pertenece a ese area!", victim->short_descr, obj->short_descr );
                return;
        }

        if( ch->level > LEVEL_AVATAR )
        obj->level = URANGE(0, victim->level, MAX_LEVEL );
        else
        obj->level = URANGE(0, victim->level, LEVEL_AVATAR );

        xTOGGLE_BIT(obj->extra_flags, ITEM_AUTOITEM);

                for( aff = obj->pIndexData->first_affect; aff; aff = aff->next )
                {
                        switch( aff->location )
                        {
                                case APPLY_HIT:
                                        value = ( ch->level * victim->dificultad - (ch->level * victim->dificultad * 0.68) + number_range(-10, 10));
                                        if( value > 300 )
                                        value = 300;
                                        loca = APPLY_HIT;
                                        opera = TRUE;
                                        break;
                                case APPLY_MANA:
                                        value = ( ch->level * victim->dificultad - (ch->level * victim->dificultad * 0.68) + number_range(-10, 10));
                                        if( value > 300 )
                                        value = 300;
                                        loca = APPLY_MANA;
                                        opera = TRUE;
                                        break;
                                case APPLY_MOVE:
                                        value = ( ch->level * victim->dificultad - (ch->level * victim->dificultad * 0.68) + number_range(-10, 10));
                                        if( value > 300 )
                                        value = 300;
                                        loca = APPLY_MOVE;
                                        opera = TRUE;
                                        break;
                                case APPLY_HITROLL:
                                        value = ( ch->level * victim->dificultad - (ch->level * victim->dificultad * 0.97) + number_range(-1, 1));
                                        if( value > 30 )
                                        value = 30;
                                        loca = APPLY_HITROLL;
                                        opera = TRUE;
                                        break;
                                case APPLY_DAMROLL:
                                        value = ( ch->level * victim->dificultad - (ch->level * victim->dificultad * 0.95)+ number_range(-1, 1));
                                        if( value > 30 )
                                        value = 30;
                                        loca = APPLY_DAMROLL;
                                        opera = TRUE;
                                        break;
                                case APPLY_STR:
                                        if( victim->dificultad <= 5 )
                                        value = victim->dificultad;
                                        else
                                        value = 5;

                                        loca = APPLY_STR;
                                        opera = TRUE;
                                        break;
                                case APPLY_DEX:
                                        if( victim->dificultad <= 5 )
                                        value = victim->dificultad;
                                        else
                                        value = 5;

                                        loca = APPLY_DEX;
                                        opera = TRUE;
                                        break;
                                case APPLY_CON:
                                        if( victim->dificultad <= 5 )
                                        value = victim->dificultad;
                                        else
                                        value = 5;

                                        loca = APPLY_CON;
                                        opera = TRUE;
                                        break;
                                case APPLY_LCK:
                                        if( victim->dificultad <= 5 )
                                        value = victim->dificultad;
                                        else
                                        value = 5;

                                        loca = APPLY_LCK;
                                        opera = TRUE;
                                        break;
                                case APPLY_WIS:
                                        if( victim->dificultad <= 5 )
                                        value = victim->dificultad;
                                        else
                                        value = 5;

                                        loca = APPLY_WIS;
                                        opera = TRUE;
                                        break;
                                case APPLY_INT:
                                        if( victim->dificultad <= 5 )
                                        value = victim->dificultad;
                                        else
                                        value = 5;

                                        loca = APPLY_INT;
                                        opera = TRUE;
                                        break;
                                case APPLY_CHA:
                                        if( victim->dificultad <= 5 )
                                        value = victim->dificultad;
                                        else
                                        value = 5;

                                        loca = APPLY_CHA;
                                        opera = TRUE;
                                        break;
                                case APPLY_AC:
                                        value = ( ch->level * victim->dificultad - ( ch->level  * victim->dificultad * 0.95) + number_range(-15, 15)) * -1;
                                        loca = APPLY_AC;
                                        opera = TRUE;
                                        break;
                                case APPLY_SAVING_POISON:
                                        value = ( ch->level * victim->dificultad - ( ch->level  * victim->dificultad * 0.99)+ number_range(-1, 1)) * -1;
                                        if( value < -15 )
                                        value = -15;
                                        loca = APPLY_SAVING_POISON;
                                        opera = TRUE;
                                        break;
                                case APPLY_SAVING_ROD:
                                        value = ( ch->level * victim->dificultad - ( ch->level  * victim->dificultad * 0.99)+ number_range(-1, 1)) * -1;
                                        if( value < -15 )
                                        value = -15;
                                        loca = APPLY_SAVING_ROD;
                                        opera = TRUE;
                                        break;
                                case APPLY_SAVING_BREATH:
                                        value = ( ch->level * victim->dificultad - ( ch->level  * victim->dificultad * 0.99)+ number_range(-1, 1)) * -1;
                                        if( value < -15 )
                                        value = -15;
                                        loca = APPLY_SAVING_BREATH;
                                        opera = TRUE;
                                        break;
                                case APPLY_SAVING_SPELL:
                                        value = ( ch->level * victim->dificultad - ( ch->level  * victim->dificultad * 0.99)+ number_range(-1, 1)) * -1;
                                        if( value < -15 )
                                        value = -15;
                                        loca = APPLY_SAVING_SPELL;
                                        opera = TRUE;
                                        break;
                                case APPLY_SAVING_PARA:
                                        value = ( ch->level * victim->dificultad - ( ch->level  * victim->dificultad * 0.99)+ number_range(-1, 1)) * -1;
                                        if( value < -15 )
                                        value = -15;
                                        loca = APPLY_SAVING_PARA;
                                        opera = TRUE;
                                        break;
                                default:
                                        opera = FALSE;
                                        break;


                         }

       if( opera == TRUE )
       {
        separate_obj(obj);
        CREATE(paf, AFFECT_DATA, 1);
	paf->type = -1;
	paf->duration = -1;
	paf->location = loca;
	paf->modifier = value;
	xCLEAR_BITS(paf->bitvector);
	paf->next = NULL;

	    LINK(paf, obj->first_affect, obj->last_affect, next, prev);

            if( aff->modifier != 0 && opera != FALSE )
             aff->modifier = 0;
       }
                }
return;
}























