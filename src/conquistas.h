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
 *                  Cabecera de datos de conquista                          *
 ****************************************************************************/

/*
 * Definicion de estructuras
 */

typedef struct                  conquistas_data         CON_DATA;
typedef struct                  recursos_data           REC_DATA;

/*
 * Estructuras
 */

struct conquistas_data
{
        CON_DATA *              next; /* Proxima conquista en la lista */
        CON_DATA *              prev; /* Anterior conquista en la lista */
        AREA_DATA *             area; /* Area de la conquista */
        CHAR_DATA *             conquistador; /* Ql que ha conqustado */
        CLAN_DATA *             familia; /* Familia que ha conquistado */
        CLAN_DATA *             destronado; /* Familia a la que se le arrebata el area */
        GUERRA_DATA *           enguerra; /* Estan en guerra? */
        ALIANZA_DATA*           enalianza;  /* Estan en alianza? */
        TREGUA_DATA *           entregua; /* Estan en tregua? */
        DROGAS_DATA *           drogas; /* Hay drogas en el area conquistada? */
        short int               puntos[MAX_FAMILIAS]; /* Puntos de las familias en el area */
        char *                  propietario; /* Propietario del area */
        char *                  nombre; /* Nombre del area */
        char *                  filename; /* Nombre del fichero */
        char *                  n_conquistador; /* Nombre del conquistador */
        char *                  u_conquistador; /* Ultima familia propietaria */
        short int               se_conquista; /* Se puede conquistar el area? */
        bool                    bonus; /* Hay bonus? */
        bool                    malus; /* Hay malus? */
        bool                    conquistable; /* Es conquistable? */
        time_t                  fecha; /* Fecha de la conquista */
};

ddf( do_conquistas );           /* Para ver el estado de conquista de las areas */

struct recursos_data
{
  sh_int     agua;
  sh_int     madera;
  sh_int     viveres;
  sh_int     combustible;
  sh_int     mineral;
};

void save_con_file    args(( CON_DATA *Conquista));
