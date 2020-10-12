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
 *                   Cabecera del modulo de familias                        *
 ****************************************************************************/

#ifndef FAM_H
#define FAM_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mud.h"

// Definiciones i demas
#define		FAM_DIR				"../familias/"
#define 	FAM_EXT				".fam"
#define		FAM_HIST_EXT		".hist"
#define		FAM_BCK_EXT			".bck"
#define		FAM_WEB_EXT			".web"
#define		FAM_CONFIG_FILE		"familias.conf"
#define 	MAX_CLAN_OBJ		25
#define 	FAMILIAS_ACTIVAS	FALSE

// Una especie de debugeador chapucero xD
#define		FAM_DEBUG			TRUE

 // Definimos el tipo de las estructuras
 typedef struct familia_datos FAMILIA_DATOS;


 typedef enum
 {
 	ACT_FAM_NORMAL, ACT_FAM_LIMPIAR, ACT_FAM_VACIAR
 }fam_act_tipo;

typedef enum
 {
 	FAM_NORMAL, FAM_PACIFICA, FAM_ATACANTE
 }fam_flags;


 // La estructura pricipal de las familias
 struct familia_datos
 {
 	// Estructuras relacionadas con las familias
	FAMILIA_DATOS	*sig;
	FAMILIA_DATOS 	*prev;

	// Texto i variables :D
	char	*	alias;
	char	*	nombre;
	char	*	descr;

	// Rangos de la familia i demas
	char	*	fundador; // Implementador o dios mayor que fundo la familia
	char	*	lider;
	char	*	heredero;
	char	*	consejero;
	char	*	tesorero;
	char	*	capitan;
	char	*	protector;
	char	*	sacerdote;
	// Rangos para el quien :D
	char	*	rango_lider;
	char	*	rango_heredero;
	char	*	rango_consejero;
	char	*	rango_tesorero;
	char	*	rango_capitan;
	char	*	rango_protector;
	char	*	rango_sacerdote;

	// A partir de aqui van todos los valores numericos
	// Variables del estado de la familia
	int		puntos;
	int		oro;
	int		mineral;
	int		comida;
	int		combustible;
	int		px;
	int		conq;
	int		quest;
	int		areas;
	int		miembros;

	// Estadisticas i esas cosas q tanto le gusta a los frikis :D
	int		mobMuertos;
	int		mobMatados;
	int		mobHuidas;
	int		pkVictorias;
	int		pkDerrotas;
	int		pkIlegal;
	int		pkHuidas;


	// Variables Historicas :D
	int		hist_puntos;
	int		hist_oro;
	int		hist_mineral;
	int		hist_comida;
	int		hist_combustible;
	int		hist_px;
	int		hist_conq;
	int		hist_quest;
	int		hist_areas;
	int		hist_miembros;

	// Variables Maximas, o sea para que lo entendais, no se incrementan, simplemente
	// marcan valores maximos.
	int		max_puntos;
	int		max_oro;
	int		max_mineral;
	int		max_comida;
	int		max_combustible;
	int		max_px;
	int		max_conq;
	int		max_quest;
	int		max_areas;
	int		max_miembros;

	// Variables para las posesiones de la familia
	int		min_vnum;
	int		max_vnum;
	int		clan_obj[MAX_CLAN_OBJ];
	int		deposito;
	int		regreso;

	char	*estado;    // El estado de la familia
	short int		activa;		// Puede entrar gente?

	// Estas variables no se guardan
	int		editada;
};



/*
    __    _                          ____  ____  __
   / /   (_)___  __  ___  __   _____/ __ \/ __ \/ /___
  / /   / / __ \/ / / / |/_/  / ___/ / / / / / / /_  /
 / /___/ / / / / /_/ />  <   / /  / /_/ / /_/ / / / /_
/_____/_/_/ /_/\__,_/_/|_|  /_/   \____/\____/_/ /___/

*/
#endif
