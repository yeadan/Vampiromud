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
 *                           Modulo                                         *
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
 * Defines varios
 */

#define MAX_TJUGADORES        50         // Maximo de juagdores simultaneos en la arena
#define MAX_VNUM_ARENA      1100         // Fin de la arena 
#define MIN_VNUM_ARENA      1000         // Inicio de la arena
#define PREP_VNUM           1101         // Sala de preparacion
#define JUEZ_VNUM           1000         // Juez de los torneos
#define T_PREMIO              10         // Tipos de premio

typedef struct  torneo_data TORNEO_DATA;

// Estructura de los torneos

struct torneo_data
{
  CHAR_DATA *ch;          // Es bastante logico no?
  CLAN_DATA *familia;     // Para los torneos por familia
  bool       activo;      // Esta activo el torneo?
  short int  jugadores;   // Jugadores que participan
  short int  derrotado;   // Jugadores derrotados 
  short int  abandono;    // jugadores que han abandonado
  short int  tipo;        // Tipo de torneo
  short int  premio;      // Premios del torneo
  char *pj[MAX_TJUGADORES]; // Pa la lista de los jugadores que participan
  short int  t_inicio;    // Tiempo para que empiece
  short int  t_duracion;  // El tiempo de duracion
  short int  ganancia[T_PREMIO]; // Ganancias segun el tipo de premio
  short int  min_lvl;     // Nivel minimo
  short int  max_lvl;     // Nivel maximo
};

#define EN_TORNEO(ch)          (( (ch)->in_room->vnum <= MAX_VNUM_ARENA )   \
                                || ((ch)->in_room->vnum >= MIN_VNUM_ARENA))
