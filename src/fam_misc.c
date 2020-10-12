/*
 * _____     __  __ _ _     _       ____
 *|  ___|_ _|  \/  (_) |   (_) __ _/ ___|
 *| |_ / _` | |\/| | | |   | |/ _` \___ \
 *|  _| (_| | |  | | | |___| | (_| |___) |
 *|_|  \__,_|_|  |_|_|_____|_|\__,_|____/
 *
 *__     __                    _           __  __           _
 *\ \   / /_ _ _ __ ___  _ __ (_)_ __ ___ |  \/  |_   _  __| |
 * \ \ / / _` | '_ ` _ \| '_ \| | '__/ _ \| |\/| | | | |/ _` |
 *  \ V / (_| | | | | | | |_) | | | | (_) | |  | | |_| | (_| |
 *   \_/ \__,_|_| |_| |_| .__/|_|_|  \___/|_|  |_|\__,_|\__,_|
 *                      |_|
 */
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
 *                   Miscelanea de funciondes de las familias               *
 ****************************************************************************/

#include "mud.h"
#include "familias.h"

void do_finvocar( CHAR_DATA *ch, char *arg )
{
	FAMILIA_DATOS *fam;
	CHAR_DATA *mob;
	OBJ_INDEX_DATA *obj;
	char	*buf[MAX_STRING_LENGTH];
	char	arg1[MAX_INPUT_LENGTH];
	char	arg2[MAX_INPUT_LENGTH];

	arg = one_argument( arg, arg1 );
	arg = one_argument( arg, arg2 );

	if( !FAM_DEBUG )
	if( !ch->familia )
	{
		ch_printf( ch, "&wNo estas en ninguna familia, no puedes &ginvocar&w.\n\r" );
		return;
	}

	if( arg1[0] == '\0' )
	{
		ch_printf( ch, "Que es lo que quieres invocar?\n\r\tUso: invocar &glista&w.\n\r" );
		return;
	}

	if( !str_prefix( arg1, "lista" ) )
	{

	}
	return;
}
