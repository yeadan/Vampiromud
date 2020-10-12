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
 *         Codigo referente a los vampiros disciplinas y tal xD             *
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


DISCI * primera_disciplina;
DISCI * ultima_disciplina;

NIVEL * primer_nivel;
NIVEL * ultimo_nivel;

/* Rutinas locales */
void    fread_disci     args(( DISCI *disci, FILE *fp ));
bool    load_disci_file  args(( char *archivo ));
void    write_disci_file    args(( void ));
void    fread_nivel     args(( NIVEL *nivel, FILE *fp ));
bool    load_nivel_file args(( char *archivo ));
void    write_nivel_file    args(( void ));
void apunyalamiento     args(( CHAR_DATA *ch, CHAR_DATA *victim, int potencia ));

/* Rutinas externas */
extern int ris_save( CHAR_DATA *ch, int chance, int ris );

/*
 * Devuelve un nombre en caracteres ascii por un numero dado
 */
char *daffect_loc_name( int localizacion )
{
    switch ( localizacion )
    {
    case AP_NADA:		return "nada";
    case AP_FUE:		return "la fuerza";
    case AP_DES:		return "la destreza";
    case AP_INT:		return "la inteligencia";
    case AP_SAB:		return "la sabiduria";
    case AP_CON:		return "la constitucion";
    case AP_CAR:		return "el carisma";
    case AP_SUE:		return "la suerte";
    case AP_SEX:		return "el sexo";
    case AP_CLASE:		return "la clase";
    case AP_NIVEL:		return "el nivel";
    case AP_EDAD:		return "la edad";
    case AP_MANA:		return "el mana";
    case AP_VIDA:		return "la vida";
    case AP_MOVE:		return "el movimiento";
    case AP_ORO:		return "el oro";
    case AP_EXP:		return "la experiencia";
    case AP_AC:			return "la armadura";
    case AP_HITROLL:		return "el hitroll";
    case AP_DAMROLL:		return "el damroll";
    case AP_SAVING_POISON:	return "save vs veneno";
    case AP_SAVING_ROD:		return "save vs rod";
    case AP_SAVING_PARA:	return "save vs paralisis";
    case AP_SAVING_BREATH:	return "save vs alientos";
    case AP_SAVING_SPELL:	return "save vs conjuros";
    case AP_ALTURA:		return "la altura";
    case AP_PESO:		return "el peso";
    case AP_AFFECT:		return "algo";
    case AP_RESISTENTE:		return "tu resistencia";
    case AP_IMMUNE:		return "tu inmunidad";
    case AP_SUSCEPTIBLE:	return "tu susceptibilidad";
    case AP_FVOL:               return "tu fuerza de voluntad";
    case AP_MAN:                return "tu manipulacion";
    case AP_INTI:               return "tu intimidacion";
    case AP_CAMINO:             return "tus puntos de camino";
    case AP_FRENESI:            return "tu frenesi";
    case AP_OCULTISMO:          return "tus conocimientos ocultos" ;
    case AP_GENERACION:         return "tu generacion";
    case AP_SANGRE:             return "tu sangre";
    case AP_AGRAVADAS:          return "tus heridas agravadas";
    case AP_DRUNK:              return "tu borrachera";
    case AP_MENTAL:             return "tu estado mental";
    case AP_POSICION:           return "tu posicion";
    case AP_ASTUCIA:            return "tu astucia";
    case AP_ALERTA:             return "tu alerta";
    };

    bug( "Nombre_Localizacion_Affect: localizacion desconocida %d.", localizacion );
    return "algo que no se lo que es";
}

/*
 * Coge un puntero de la estructura de disciplinas desde un nombre de disciplina dado
 */
DISCI *get_disci( char *nombre )
{
        DISCI *disci;

        for ( disci = primera_disciplina; disci ; disci = disci->next )
                if( !str_prefix( nombre, disci->nombre ))
                        return disci;
                return NULL;
}

/*
 * Coge un puntero de la estructura de niveles de disciplina desde un nombre de nivel dado
 */
NIVEL *get_nivel( char *nombre )
{
        NIVEL *nivel;

        for ( nivel = primer_nivel; nivel ; nivel = nivel->next )
                if( !str_prefix( nombre, nivel->nombre ))
                        return nivel;
                return NULL;
}

/*
 * Coge un puntero de la estructura niveles de disciplina desde un numero de nivel dado
 */
NIVEL *get_dnivel( sh_int dn )
{
        NIVEL *nivel;

        for ( nivel = primer_nivel; nivel ; nivel = nivel->next )
                if( nivel->dnivel == dn )
                        return nivel;
                return NULL;
}

/* Calcula el coste de sangre del lanzamiento de un nivel de disciplina */

bool coste_sangre( CHAR_DATA *ch, NIVEL *nivel )
{
    bool correcto = TRUE;

          if( !IS_IMMORTAL(ch))
          {
                correcto = FALSE;
                        if ( ch->pcdata->condition[COND_BLOODTHIRST] <= nivel->sangre - ( 13 / ch->generacion  ))
                        {
                        send_to_char( "No tienes suficiente sangre busca de quien alimentarte.\n\r", ch );
                        correcto = FALSE;
                        }
                        else
                        {
                        if( nivel->sangre - ( 13/ ch->generacion ) >= 1)
                        ch->pcdata->condition[COND_BLOODTHIRST] -= (nivel->sangre - ( 13 / ch->generacion ));
                        else
                        ch->pcdata->condition[COND_BLOODTHIRST] -=1;

                        correcto = TRUE;
                        }
          }
          return correcto;
}

/* Realiza la accion de Lanzar una Disciplina */

bool lanzar_disciplina( CHAR_DATA *ch, CHAR_DATA *victim, NIVEL *nivel )
{
  char buf[256];
  char buf2[256];
  char buf3[256];
  int chance;

  if( !ch )
  {
        bug( "Lanzar_disciplina: ch NULO en la funcion.", 0 );
        return FALSE;
  }

  if( !nivel )
  {
	  bug( "Lazar_disciplina: Nivel NULO en la funcion Lanzar_Disciplina: %s %d", __FILE__, __LINE__ );
        return FALSE;
  }

  if( victim )
  if( is_safe( ch, victim, TRUE ) )
  {
        send_to_char( "Los dioses le protegen.\n\r", ch );
        return FALSE;
  }

  if(!coste_sangre( ch, nivel ) )
  return FALSE;

  if( victim )
  {

           chance = number_range( 1, 100 );
           if( chance >= 95 )
           {
                if( nivel->falloch != NULL )
                sprintf( buf, nivel->falloch );
                else
                sprintf(buf, "No consigues concentrarte lo suficiente y fallas tu %s.\n\r", nivel->nombre );

                if( nivel->fallovict != NULL )
                sprintf( buf2, nivel->fallovict );

                if( nivel->falloroom != NULL )
                sprintf( buf3, nivel->falloroom );

                act( AT_MAGIC, buf, ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, buf2, ch, NULL, ch, TO_VICT );
                act( AT_MAGIC, buf3, ch, NULL, victim, TO_NOTVICT );
                multi_hit( victim, ch, TYPE_UNDEFINED );

                if( !IS_IMMORTAL(ch))
                {
                        if( !xIS_SET(ch->afectado_por, DAF_CELERIDAD ) )
                                WAIT_STATE(ch, nivel->wait );
                        else
                                espera_menos( ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], nivel->wait);
                }

                return FALSE;
           }

           /*
            * Aqui se debera de incluir el caso para si son inmunes( por implementar )
            */
               if( !is_safe( victim, ch, TRUE ) );
               {
                if( nivel->exitoch != NULL )
                sprintf( buf, nivel->exitoch );

                if( nivel->exitovict != NULL )
                sprintf( buf2, nivel->exitovict );

                if( nivel->exitoroom != NULL )
                sprintf( buf3, nivel->exitoroom );
               }

               act( AT_MAGIC, buf, ch, NULL, victim, TO_CHAR );
               act( AT_MAGIC, buf2, ch, NULL, victim, TO_VICT );
               act( AT_MAGIC, buf3, ch, NULL, victim, TO_NOTVICT );

                if( !IS_IMMORTAL(ch))
                {
                        if( !xIS_SET(ch->afectado_por, DAF_CELERIDAD ) )
                                WAIT_STATE(ch, nivel->wait );
                        else
                                espera_menos( ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], nivel->wait);
                }
                return TRUE;


return TRUE;
 }/* cierra if( victim ) */
else
{


           chance = number_range( 1, 100 );
           if( chance >= 95 )
           {

                if( nivel->falloch != NULL )
                sprintf( buf, nivel->falloch );
                else
                sprintf(buf, "No consigues concentrarte lo suficiente y fallas tu %s.\n\r", nivel->nombre );

                if( nivel->falloroom != NULL )
                sprintf( buf3, nivel->falloroom );

                act( AT_MAGIC, buf, ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, buf3, ch, NULL, ch, TO_CANSEE );

                if( !IS_IMMORTAL(ch))
                {
                        if( !xIS_SET(ch->afectado_por, DAF_CELERIDAD ) )
                                WAIT_STATE(ch, nivel->wait );
                        else
                                espera_menos( ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], nivel->wait);
                }

                return FALSE;
           }

                if( nivel->exitoch[0] != '\0' )
                sprintf( buf, nivel->exitoch );

                if( nivel->exitoroom[0] != '\0' )
                sprintf( buf2, nivel->exitoroom );

                act( AT_MAGIC, buf, ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, buf2, ch, NULL, ch, TO_CANSEE );

                if( !IS_IMMORTAL(ch))
                {
                        if( !xIS_SET(ch->afectado_por, DAF_CELERIDAD ) )
                                WAIT_STATE(ch, nivel->wait );
                        else
                                espera_menos( ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], nivel->wait);
                }

                return TRUE;
}

  bug( "Lanzar_disciplina: Algo falla" );
  return FALSE;
}

/*
 * Funcion que escribe la lista de disciplinas
 */
void write_disci_list( )
{
        DISCI *disci;
        FILE *fpout;
        char archivo[256];

        sprintf( archivo, "%s%s", DISCIPLINAS_DIR, DISCI_LIST );

        fpout = fopen( archivo, "w" );
        if( !fpout )
        {
                bug( "NO se ha podido abrir disciplinas.lst para escribir", archivo );
                return;
        }
        for( disci = primera_disciplina; disci ; disci = disci->next )
                fprintf( fpout, "%s\n", disci->fichero );
         fprintf( fpout, "$\n" );
         fclose( fpout );
}

/*
 * Funcion que escribe la lista de niveles de disciplina
 */
void write_nivel_list( )
{
        NIVEL *nivel;
        FILE *fpout;
        char archivonivel[256];

        sprintf( archivonivel, "%s%s", DISCIPLINAS_NIVELES_DIR, NIVEL_LIST );

        fpout = fopen( archivonivel, "w" );
        if( !fpout )
        {
                bug( "NO se ha podido abrir nivel.lst para escribir", archivonivel );
                return;
        }

        for( nivel = primer_nivel; nivel ; nivel = nivel->next )
                fprintf( fpout, "%s\n", nivel->fichero );
         fprintf( fpout, "$\n" );
         fclose( fpout );
}

/*
 * Guarda los datos de disci_table en su archivo de datos
 */
void save_disciplinas( DISCI *disci )
{
        FILE *fp;
        char archivo[256];
        char buf[MAX_INPUT_LENGTH];
        int cont = 0;

        if( !disci )
        {
                bug( "Save_disciplinas: puntero de disciplina nulo!", 0 );
                return;
        }

        if( !disci->fichero || disci->fichero[0] == '\0' )
        {
                sprintf( buf, "save_disciplina: %s no tiene nombre de fichero", disci->nombre );
                bug( buf, 0 );
                return;
        }

        sprintf( archivo, "%s%s", DISCIPLINAS_DIR, disci->fichero );

        fclose( fpReserve );
        if( ( fp = fopen( archivo, "w" ) ) == NULL)
        {
                bug( "Save_disciplinas : fopen", 0 );
                perror( archivo );
        }
        else
        {
                fprintf( fp, "#DISCIPLINA\n" );
                fprintf( fp, "Nombre    %s~\n",         disci->nombre );
                fprintf( fp, "Descripcion        %s~\n",         disci->descripcion );
                fprintf( fp, "Filename          %s~\n",         disci->fichero );
                fprintf( fp, "Disciplina        %d\n",          disci->numero );
                fprintf( fp, "Clan              %s~\n",         disci->clan );
                for( cont = 1; cont < 11 ; cont++ )
                fprintf( fp, "Nivel%d           %s~\n",         cont, disci->niveles[cont] ? disci->niveles[cont] : "No creado o no especificado" );

                fprintf( fp, "\nEnd\n\n" );

                fprintf( fp, "#END\n" );
        }
        fclose( fp );
        fpReserve = fopen( NULL_FILE, "r" );
        return;
}

/*
 * Guarda los datos de disci_nivel en su archivo de datos
 */
void save_nivel( NIVEL *nivel )
{
        FILE *fp;
        char archivon[256];
        char buf[MAX_INPUT_LENGTH];

        if( !nivel )
        {
                bug( "Save_nivel: puntero de nivel nulo!", 0 );
                return;
        }

        if( !nivel->fichero || nivel->fichero[0] == '\0' )
        {
                sprintf( buf, "save_nivel: %s no tiene nombre de fichero", nivel->nombre );
                bug( buf, 0 );
                return;
        }

        sprintf( archivon, "%s%s", DISCIPLINAS_NIVELES_DIR, nivel->fichero );

        fclose( fpReserve );
        if( ( fp = fopen( archivon, "w" ) ) == NULL)
        {
                bug( "Save_nivel : fopen", 0 );
                perror( archivon );
        }
        else
        {
                fprintf( fp, "#NIVEL\n" );
                fprintf( fp, "Nombre            %s~\n",         nivel->nombre );
                fprintf( fp, "Tipo              %d\n",          nivel->tipo );
                fprintf( fp, "Funcion           %s\n",          skill_name(nivel->do_fun));
                fprintf( fp, "Descripcion       %s~\n",         nivel->descripcion );
                fprintf( fp, "Filename          %s~\n",         nivel->fichero );
                fprintf( fp, "Disciplina        %d\n",          nivel->disciplina );
                fprintf( fp, "Clan              %s~\n",         nivel->clan );
                fprintf( fp, "Nivel             %d\n",          nivel->nivel );
                fprintf( fp, "Dnivel            %d\n",          nivel->dnivel );
                fprintf( fp, "Wait              %d\n",          nivel->wait );
                fprintf( fp, "Sangre            %d\n",          nivel->sangre );
                fprintf( fp, "Dammes            %s~\n",         nivel->dammes );
                fprintf( fp, "GoMes             %s~\n",         nivel->sevames );
                fprintf( fp, "ExitoCh           %s~\n",         nivel->exitoch );
                fprintf( fp, "ExitoVict         %s~\n",         nivel->exitovict );
                fprintf( fp, "ExitoRoom         %s~\n",         nivel->exitoroom );
                fprintf( fp, "ExitoMud          %s~\n",         nivel->exitomud );
                fprintf( fp, "FalloCh           %s~\n",         nivel->falloch );
                fprintf( fp, "FalloVict         %s~\n",         nivel->fallovict );
                fprintf( fp, "FalloRoom         %s~\n",         nivel->falloroom );
                fprintf( fp, "FalloMud          %s~\n",         nivel->fallomud );
                fprintf( fp, "MuerteCh          %s~\n",         nivel->muertech );
                fprintf( fp, "MuerteVict        %s~\n",         nivel->muertevict );
                fprintf( fp, "MuerteRoom        %s~\n",         nivel->muerteroom );
                fprintf( fp, "MuerteMud         %s~\n",         nivel->muertemud );
                fprintf( fp, "InmuneCh          %s~\n",         nivel->inmunech );
                fprintf( fp, "InmuneVict        %s~\n",         nivel->inmunevict );
                fprintf( fp, "InmuneRoom        %s~\n",         nivel->inmuneroom );
                fprintf( fp, "InmuneMud         %s~\n",         nivel->inmunemud );

                fprintf( fp, "End\n\n" );

                fprintf( fp, "#END\n" );
        }
        fclose( fp );
        fpReserve = fopen( NULL_FILE, "r" );
        return;
}
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if( !str_cmp( word, literal) )          \
                                {                                       \
                                        field = value;                  \
                                        fMatch = TRUE;                  \
                                        break;                          \
                                }
/*
 * Lee los datos de la estructura externa guardada en en su propio archivo
 * de datos
 */
void fread_disci( DISCI *disci, FILE *fp )
{
        char buf[MAX_STRING_LENGTH];
        char *word;
        bool fMatch;

        for( ; ; )
        {
                word = feof( fp ) ? "End" : fread_word( fp );

                fMatch = FALSE;

                switch( UPPER(word[0]) )
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol( fp );
                        break;

                case 'C':
                        if( !str_cmp( word, "Clan"))
                        {
                                fMatch = TRUE;
                                disci->clan = fread_string( fp );
                                break;
                        }

                case 'D':
                        KEY( "Descripcion",     disci->descripcion,        fread_string( fp ));
                        KEY( "Disciplina",      disci->numero,             fread_number( fp ));
                        break;

                case 'E':
                        if( !str_cmp( word, "End" ) )
                        {
                                if( !disci->nombre)
                                disci->nombre           = STRALLOC( "" );
                                if( !disci->clan)
                                disci->clan           = STRALLOC( "" );
                                if( !disci->descripcion)
                                disci->descripcion           = STRALLOC( "No fijada aun" );
                                return;
                        }

                case 'F':
                        KEY( "Filename",        disci->fichero,            fread_string( fp ));
                        break;

                case 'N':
                        KEY( "Nivel1",          disci->niveles[1],              fread_string( fp ));
                        KEY( "Nivel2",          disci->niveles[2],              fread_string( fp ));
                        KEY( "Nivel3",          disci->niveles[3],              fread_string( fp ));
                        KEY( "Nivel4",          disci->niveles[4],              fread_string( fp ));
                        KEY( "Nivel5",          disci->niveles[5],              fread_string( fp ));
                        KEY( "Nivel6",          disci->niveles[6],              fread_string( fp ));
                        KEY( "Nivel7",          disci->niveles[7],              fread_string( fp ));
                        KEY( "Nivel8",          disci->niveles[8],              fread_string( fp ));
                        KEY( "Nivel9",          disci->niveles[9],              fread_string( fp ));
                        KEY( "Nivel10",         disci->niveles[10],             fread_string( fp ));
                        if( !str_cmp( word, "Nombre" ) )
                        {
                                fMatch = TRUE;
                                disci->nombre = fread_string( fp );
                        }
               }

               if( !fMatch )
               {
                        sprintf( buf, "Fread_disci: no se ha encontrado: %s", word );
                        bug( buf, 0);
               }
         }
}


void fread_nivel( NIVEL *nivel, FILE *fp )
{
        char buf[MAX_STRING_LENGTH];
        char *word;
        bool fMatch;

        for( ; ; )
        {
                word = feof( fp ) ? "End" : fread_word( fp );
                fMatch = FALSE;

                switch( UPPER(word[0]) )
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol( fp );
                        break;

                case 'C':
                        if( !str_cmp( word, "Clan"))
                        {
                                fMatch = TRUE;
                                nivel->clan = fread_string( fp );
                                break;
                        }

                case 'D':
                        if( !str_cmp( word, "Disciplina") )
                        {
                                fMatch = TRUE;
                                nivel->disciplina = fread_number( fp );
                                break;
                        }
                        KEY( "Descripcion",     nivel->descripcion,        fread_string( fp ));
                        KEY( "Dammes",          nivel->dammes,             fread_string( fp ));
                        KEY( "Dnivel",          nivel->dnivel,             fread_number( fp ));
                        break;
                /*
                 * Case 'E' bajado al final del todo
                 */

                case 'F':
                        KEY( "Filename",        nivel->fichero,            fread_string( fp ));
                        KEY( "FalloCh",         nivel->falloch,            fread_string( fp ));
                        KEY( "FalloVict",       nivel->fallovict,          fread_string( fp ));
                        KEY( "FalloRoom",       nivel->falloroom,          fread_string( fp ));
                        KEY( "FalloMud",        nivel->fallomud,           fread_string( fp ));
                        KEY( "Funcion",         nivel->do_fun,             skill_function(fread_word( fp )) );
                        break;

                case 'G':
                        KEY( "GoMes",           nivel->sevames,            fread_string( fp ));
                        break;

                case 'I':
                        KEY( "InmuneCh",        nivel->inmunech,           fread_string( fp ));
                        KEY( "InmuneVict",      nivel->inmunevict,         fread_string( fp ));
                        KEY( "InmuneRoom",      nivel->inmuneroom,         fread_string( fp ));
                        KEY( "InmuneMud",       nivel->inmunemud,          fread_string( fp ));
                        break;

                case 'M':
                        KEY( "MuerteCh",        nivel->muertech,           fread_string( fp ));
                        KEY( "MuerteVict",      nivel->muertevict,         fread_string( fp ));
                        KEY( "MuerteRoom",      nivel->muerteroom,         fread_string( fp ));
                        KEY( "MuerteMud",       nivel->muertemud,          fread_string( fp ));
                        break;

                case 'N':
                        KEY( "Nivel",           nivel->nivel,              fread_number( fp ));
                        KEY( "Nombre",          nivel->nombre,             fread_string( fp ));

                case 'S':
                        KEY( "Sangre",          nivel->sangre,             fread_number( fp ));
                        break;

                case 'T':
                        KEY( "Tipo",            nivel->tipo,               fread_number( fp ));
                        break;

                case 'W':
                        KEY( "Wait",            nivel->wait,               fread_number( fp ));
                        break;

                case 'E':
                        KEY( "ExitoCh",           nivel->exitoch,          fread_string( fp ));
                        KEY( "ExitoVict",         nivel->exitovict,        fread_string( fp ));
                        KEY( "ExitoRoom",         nivel->exitoroom,        fread_string( fp ));
                        KEY( "ExitoMud",          nivel->exitomud,         fread_string( fp ));
                        if( !str_cmp( word, "End" ) )
                        {
                                if( !nivel->nombre)
                                nivel->nombre           = STRALLOC( "" );
                                if( !nivel->clan)
                                nivel->clan           = STRALLOC( "" );
                                if( !nivel->descripcion)
                                nivel->descripcion           = STRALLOC( "No fijada aun" );
                                return;
                        }
                        break;


               }

               if( !fMatch )
               {
                        sprintf( buf, "Fread_nivel: no se ha encontrado: %s", word );
                        bug( buf, 0);
               }
         }
}

bool load_disci_file( char *archivo )
{
    char filename[256];
    DISCI *disci;
    FILE *fp;
    bool found;

    CREATE( disci, DISCI, 1 );
    found = FALSE;
    sprintf( filename, "%s%s", DISCIPLINAS_DIR, archivo );

    if(( fp = fopen( filename, "r" )) != NULL )
    {
        found = TRUE;
        for( ; ; )
        {
                char letter;
                char *word;

                letter = fread_letter( fp );
                if ( letter == '*' )
                {
                        fread_to_eol( fp );
                        continue;
                }

                if( letter != '#' )
                {
                        bug( "Load_disci_file: No se encontro #.", 0);
                        break;
                }

                word = fread_word( fp );
                if( !str_cmp( word, "DISCIPLINA" ) )
                {
                        fread_disci( disci, fp );
                        break;
                }
                else
                if( !str_cmp( word, "END" ) )
                        break;
                else
                {
                        char buf[MAX_STRING_LENGTH];

                        sprintf( buf, "Load_disci_file: Seccion erronea: %s.", word );
                        bug( buf, 0);
                        break;
                }
        }
         fclose( fp );
    }

    if( found )
    {
        LINK( disci, primera_disciplina, ultima_disciplina, next, prev );

        sprintf( filename, "%s%s.vault", DISCIPLINAS_DIR, disci->fichero );

        if(( fp = fopen( filename, "r" ) ) != NULL )
        {
                bool found;

                found = TRUE;
                for ( ; ; )
                {
                        char letter;
                        char *word;

                        letter = fread_letter( fp );
                        if( letter == '*' )
                        {
                                fread_to_eol( fp );
                                continue;
                        }

                        if( letter != '#' )
                        {
                                bug( "Load_disci_vault: # No encontrado.", 0 );
                                bug( disci->nombre, 0 );
                                break;
                        }

                        word = fread_word( fp );

                        if( !str_cmp( word, "END"       ) )
                        break;
                        else
                        {
                                bug( "Load_disci_vault: Seccion erronea.", 0 );
                                bug( disci->nombre, 0 );
                                break;
                        }

                 }
                  fclose( fp );
        }
        else
                log_string( "No se pudo abrir disciplina vault" );

     }
     else
        DISPOSE( disci );

  return found;
}

bool load_nivel_file( char *archivo )
{
        char filenamen[256];       
        NIVEL *nivel;
        FILE *fp;
        bool found;

        CREATE( nivel, NIVEL, 1 );

        found = FALSE;
        sprintf( filenamen, "%s%s", DISCIPLINAS_NIVELES_DIR, archivo );

        if(( fp = fopen( filenamen, "r" ) ) != NULL)
        {

             found = TRUE;
             for( ; ; )
             {
                char letter;
                char *word;

                letter = fread_letter( fp );
                if( letter == '*' )
                {
                        fread_to_eol( fp );
                        continue;
                }

                if( letter != '#' )
                {
                        bug( "Load_nivel_file: # No encontrado.", 0 );
                        break;
                }

                word = fread_word( fp );

                if( !str_cmp( word, "NIVEL"     ))
                {
                        fread_nivel( nivel, fp );
                        break;
                }
                else
                if( !str_cmp( word, "END" ) )
                        break;
                else
                {
                        bug( "Load_nivel_file: Seccion erronea.", 0 );
                        break;
                }

              }
                fclose( fp );
            }

       if( found )
           LINK( nivel, primer_nivel, ultimo_nivel, next, prev );

       else
         DISPOSE( nivel );

       return found;
}


/*
 * Esta funcion nos carga todas las disciplinas es invocada desde db.c
 * al cargar el Mud
 */
void load_disciplinas( )
{
        FILE *fpList;
        //DISCI *disci;
        char *filename;
        char discilist[256];
        char buf[MAX_STRING_LENGTH];


        primera_disciplina = NULL;
        ultima_disciplina = NULL;

        log_string( "Cargando Disciplinas..." );

        sprintf( discilist, "%s%s", DISCIPLINAS_DIR, DISCI_LIST );
        fclose( fpReserve );
        if(( fpList = fopen( discilist, "r") ) == NULL )
        {
                perror( discilist );
                exit( 1 );
        }

        for ( ; ; )
        {
                filename = feof( fpList ) ? "$" : fread_word( fpList );
                log_string( filename );
                if( filename[0] == '$' )
                   break;

                if( !load_disci_file( filename ))
                {
                        sprintf( buf, "No se pudo abrir archivo: %s.", filename );
                        bug( buf, 0);
                }
        }
        fclose( fpList );
        log_string( " Disciplinas Cargadas " );
        fpReserve = fopen( NULL_FILE, "r" );
        return;
}

void load_nivel( )
{
        FILE *fpList;
        char *filename;
        char nlist[256];
        char buf[256];

        primer_nivel = NULL;
        ultimo_nivel = NULL;

        log_string( "Cargando Niveles de Disciplinas..." );

        sprintf( nlist, "%s%s", DISCIPLINAS_NIVELES_DIR, NIVEL_LIST );
        fclose( fpReserve );
        if(( fpList = fopen( nlist, "r" ) ) == NULL )
        {
                perror( nlist );
                exit( 1 );
        }

        for ( ; ; )
        {
                filename = feof( fpList ) ? "$" : fread_word( fpList );
                log_string( filename );
                if( filename[0] == '$' )
                        break;



           if( !load_nivel_file( filename ) )
           {
                sprintf( buf, "No se pudo abrir archivo: %s.", filename );
                bug( buf, 0 );
           }

         }
         fclose( fpList );
         log_string( " Niveles de Disciplina Cargados " );
         fpReserve = fopen( NULL_FILE, "r" );
         return;
}


/*
 * Con esto se modifican los valores de las diferentes disciplinas
 */
DISCI *disci;
void do_dset( CHAR_DATA *ch, char *argument )
{ 
		char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];        

        set_char_color( AT_GREY, ch );
        if( IS_NPC( ch ))
        {
                send_to_char( "Si hombre pero tu que te has pensao.\n\n", ch );
                return;
        }

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        if( arg1[0] == '\0' )
        {
                send_to_char( "Uso: dset <disciplina> <campo> <valor>\n\r", ch );
                send_to_char( "\n\rEstos son los campos posibles\n\r", ch );
                send_to_char( "nombre|numero|fichero|descripcion|clan|\n\r"
                              "niv1|niv2|niv3|niv4|niv5|niv6|niv7|niv8|niv9|niv10\n\r", ch );
                if( get_trust( ch ) >= LEVEL_IMPLEMENTOR )
                        send_to_char( "Limpiar = pone la disciplina a 0\n\r&rNo usar ni se esta seguro de lo que se hace.\n\r", ch );
                send_to_char( "Guarda para guardar la lista de disciplinas, usar solo si se ha creado nueva.\n\r", ch );
                return;
        }

        if( !str_cmp( arg1, "guarda" ) )
        {
                write_disci_list( );
                send_to_char( "Hecho.\n\r", ch );
                return;
        }

        if( !str_prefix( arg1, "lista" ) )
        {
             if( disci == NULL )
             {
              send_to_char( "Aun no se ha creado ninguna disciplina.\n\r", ch );
              return;
             }
             else
              ch_printf( ch, "\n\r&cDisciplinas creadas.\n\r" );

             for( disci = primera_disciplina; disci; disci = disci->next )
              ch_printf( ch, "&c%s\n\r", disci->nombre );

             return;
        }
        disci = get_disci( arg1 );
        if( !disci )
        {
                send_to_char( "Esa disciplina no existe ( borracho de mierda ).\n\r", ch );
                return;
        }

        if( arg2[0] == '\0' )
        {
                send_to_char( "Cambiar el que de esta disciplina?.\n\r", ch );
                return;
        }

        if( !str_prefix( arg2, "nombre" ) && argument[0] != '\0' )
        {
                ch_printf( ch, "Cambiando nombre de la disciplina %s a %s.\n\r", disci->nombre, argument );
                STRFREE( disci->nombre );
                disci->nombre = STRALLOC( argument );
                save_disciplinas( disci );
                send_to_char( "Hecho.\n\r", ch );
                return;
         }

         else if( !str_prefix( arg2, "numero" ) && argument[0] != '\0' )
         {
                if( !is_number( argument ))
                {
                        send_to_char( "El argumento debe de ser un numero de disciplina valido( del 1 al 34).\n\r", ch );
                        return;
                }

                if( atoi( argument ) < 1 || atoi( argument ) > 34 )
                {
                        send_to_char( "El rango de disciplinas va de 1 a 34.\n\r", ch );
                        return;
                }

                ch_printf( ch, "Cabiando numero de disciplina de %d a %d.\n\r", disci->numero, atoi(argument) );
                disci->numero = atoi(argument);
                save_disciplinas( disci );
                send_to_char( "Hecho.\n\r", ch );
                return;
         }

         else if( !str_prefix( arg2, "fichero" ) && argument[0] != '\0' )
         {
                ch_printf( ch, "Cambiando antiguo nombre de fichero %s por %s.\n\r", disci->fichero, argument );
                DISPOSE( disci->fichero );
                disci->fichero = str_dup( argument);
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                log_string( "Modificando disciplinas.lst" );
                write_disci_list( );
                return;
          }

          else if( !str_prefix( arg2, "descripcion" ) && argument[0] != '\0' )
          {
                ch_printf( ch, "Cambiendo antigua descripcion '%s' por '%s'.\n\r", disci->descripcion, argument );
                STRFREE( disci->descripcion );
                disci->descripcion = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_prefix( arg2, "clan" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo clan %s por clan %s.\n\r", disci->clan, argument );
                STRFREE(  disci->clan );
                disci->clan = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv1" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel uno %s por %s.\n\r", disci->niveles[1], argument );
                STRFREE( disci->niveles[1] );
                disci->niveles[1] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv2" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel dos %s por %s.\n\r", disci->niveles[2], argument );
                STRFREE( disci->niveles[2] );
                disci->niveles[2] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv3" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel tres %s por %s.\n\r", disci->niveles[3], argument );
                STRFREE( disci->niveles[3] );
                disci->niveles[3] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv4" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel cuatro %s por %s.\n\r", disci->niveles[4], argument );
                STRFREE( disci->niveles[4] );
                disci->niveles[4] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv5" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel cinco %s por %s.\n\r", disci->niveles[5], argument );
                STRFREE( disci->niveles[5] );
                disci->niveles[5] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv6" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel seis %s por %s.\n\r", disci->niveles[6], argument );
                STRFREE( disci->niveles[6] );
                disci->niveles[6] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv7" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel siete %s por %s.\n\r", disci->niveles[7], argument );
                STRFREE( disci->niveles[7] );
                disci->niveles[7] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv8" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel ocho %s por %s.\n\r", disci->niveles[8], argument );
                STRFREE( disci->niveles[8] );
                disci->niveles[8] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv9" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel nueve %s por %s.\n\r", disci->niveles[9], argument );
                STRFREE( disci->niveles[9] );
                disci->niveles[9] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }

           else if( !str_cmp( arg2, "niv10" ) && argument[0] != '\0' )
           {
                ch_printf( ch, "Cambiando antiguo nivel diez %s por %s.\n\r", disci->niveles[10], argument );
                STRFREE( disci->niveles[10] );
                disci->niveles[10] = STRALLOC( argument );
                send_to_char( "Hecho.\n\r", ch );
                save_disciplinas( disci );
                return;
           }
           else
           {
                send_to_char( "Eso que dices no se yo que sera.\n\r", ch );
                return;
           }
return;
}


/*
 * Esta funcion crea una nueva disciplina que despues modificaremos con dset
 */
void do_creardisciplina( CHAR_DATA *ch, char *argument )
{
        char filename[256];
        char argumento[256];
        DISCI *disci;
        bool found;
        int bucle;

        set_char_color( AT_DGREEN, ch );

        if( !argument || argument[0] == '\0' )
        {
                send_to_char( "Uso: creardisci <nombre disciplina>.\n\r", ch );
                return;
        }

        found = FALSE;
        sprintf( argumento, "%s.vpm", argument );
        sprintf( filename, "%s%s", DISCIPLINAS_DIR, strlower(argument) );

        CREATE( disci, DISCI, 1 );
        LINK( disci, primera_disciplina, ultima_disciplina, next, prev );

        disci->nombre              = STRALLOC( argument );
        disci->fichero             = STRALLOC( argumento );
        disci->descripcion         = STRALLOC( "Descripcion no fijada" );
        disci->clan                = STRALLOC( "Ninguno" );
        disci->numero              = -1;
        for( bucle = 1 ; bucle < 11; bucle++ )
        disci->niveles[bucle]        = STRALLOC( "Desconocido por el momento" );

        send_to_char( "Creada.\n\r", ch );
}

void do_dver( CHAR_DATA *ch, char *argument )
{
        DISCI *disci;
        int cnt;

        if( IS_NPC(ch))
        {
                send_to_char( "Mande?.\n\r", ch );
                return;
        }


        if( argument[0] == '\0' )
        {
                send_to_char( "Uso: dver <disciplina>.\n\r", ch );
                return;
        }

        disci = get_disci( argument );
        if( !disci )
        {
                send_to_char( "Eso no es ninguna disciplina que haya creada.\n\r", ch );
                return;
        }

        if( get_trust( ch ) > LEVEL_CREATOR )
        ch_printf( ch, "\n\r&cNombre          : &w%s&c      Filename(&w%s&c)\n\r", disci->nombre, disci->fichero );
        else
        ch_printf( ch, "\n\r&cNombre          : &w%s&c\n\r", disci->nombre );

        ch_printf( ch, "&cDisciplina Nº   : &w%d&c          Propia del clan : &w%s\n\r", disci->numero, disci->clan );
        ch_printf( ch, "&cDescripcion     : &w%s&c\n\r", disci->descripcion );
        for( cnt = 1; cnt < 11; cnt++)
        ch_printf( ch, "&cNivel %-2.2d        : &w%s&c\n\r", cnt, disci->niveles[cnt] );

return;
}

/*
 * Esta funcion crea un nuevo nivel que despues modificaremos con nset
 */
void do_crearnivel( CHAR_DATA *ch, char *argument )
{
        char filename[256];
        char argumento[256];
        NIVEL *nivel;
        bool found;
        //int bucle;

        set_char_color( AT_DGREEN, ch );

        if( !argument || argument[0] == '\0' )
        {
                send_to_char( "Uso: crearnivel <nombre nivel>.\n\r", ch );
                return;
        }

        found = FALSE;
        sprintf( argumento, "%s.dnv", argument );
        sprintf( filename, "%s%s", DISCIPLINAS_NIVELES_DIR, strlower(argument) );

        CREATE( nivel, NIVEL, 1 );
        LINK( nivel, primer_nivel, ultimo_nivel, next, prev );

        nivel->tipo                = TIPO_NINGUNO;
        nivel->nombre              = STRALLOC( argument );
        nivel->do_fun              = NULL;
        nivel->fichero             = STRALLOC( argumento );
        nivel->descripcion         = STRALLOC( "Descripcion no fijada" );
        nivel->clan                = STRALLOC( "Ninguno" );
        nivel->disciplina               = -1;
        nivel->nivel                    = 1;
        nivel->dnivel                   = -1;
        nivel->wait                     = 1;
        nivel->dammes                   = STRALLOC( "" );
        nivel->sevames                  = STRALLOC( "" );
        nivel->exitoch                  = STRALLOC( "" );
        nivel->exitovict                = STRALLOC( "" );
        nivel->exitoroom                = STRALLOC( "" );
        nivel->exitomud                 = STRALLOC( "" );
        nivel->falloch                  = STRALLOC( "" );
        nivel->fallovict                = STRALLOC( "" );
        nivel->falloroom                = STRALLOC( "" );
        nivel->fallomud                 = STRALLOC( "" );
        nivel->muertech                 = STRALLOC( "" );
        nivel->muertevict               = STRALLOC( "" );
        nivel->muerteroom               = STRALLOC( "" );
        nivel->muertemud                = STRALLOC( "" );
        nivel->inmunech                = STRALLOC( "" );
        nivel->inmunevict              = STRALLOC( "" );
        nivel->inmuneroom              = STRALLOC( "" );
        nivel->inmunemud               = STRALLOC( "" );

        send_to_char( "Creado.\n\r", ch );
        return;
}

/*
 * Con esto se modifican los valores de los diferentes niveles
 */
int x;
NIVEL *nivel;
void do_nset( CHAR_DATA *ch, char *argument )
{
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];        

        set_char_color( AT_GREY, ch );
        if( IS_NPC( ch ))
        {
                send_to_char( "Si hombre pero tu que te has pensao.\n\n", ch );
                return;
        }

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        if( arg1[0] == '\0' )
        {
                send_to_char( "Uso : nset <nivel> <campo> <valor>.\n\r", ch );
                send_to_char( "Estos son los campos posibles:&c nombre, nivel, disciplina, \n\r"
                              "descripcion, clan, wait, dammes, gomes, exitoch, exitovict, \n\r"
                              "exitoroom, exitomud, falloch, fallovict, falloroom, fallomud,\n\r"
                              "muertech, muertevict, muerteroom, muertemud,inmunech, inmunevict,\n\r"
                              "inmuneroom, inmunemud, sangre, tipo, funcion, dnivel&w.\n\r", ch );
                send_to_char( "Guarda para guardar la lista de niveles, usar solo si se ha creado uno nuevo.\n\r", ch );
                return;
         }

         if( !str_cmp( arg1, "guarda" ) )
         {
             write_nivel_list( );
             send_to_char( "Hecho.\n\r", ch );
             return;
         }

        if( !str_prefix( arg1, "lista" ) )
        {
             if( nivel == NULL )
             {
              send_to_char( "Aun no se ha creado ningun nivel.\n\r", ch );
              return;
             }
             else
              ch_printf( ch, "\n\r&cNiveles creados.\n\r" );

             for( nivel = primer_nivel; nivel; nivel = nivel->next )
              ch_printf( ch, "&c%s\n\r", nivel->nombre );

             return;
        }
        nivel = get_nivel( arg1 );

        if( !nivel )
        {
                send_to_char( "Ese nivel no existe ( borracho de mierda ).\n\r", ch );
                return;
        }

        if( arg2[0] == '\0' )
        {
                send_to_char( "Cambiar el que de este nivel?.\n\r", ch );
                return;
        }

        if( !str_prefix( arg2, "nombre") )
        {
                if( is_number( argument ) )
                {
                        send_to_char( "El argumento no puede ser numerico.\n\r", ch );
                        return;
                }

                nivel->nombre = STRALLOC( argument );
                save_nivel( nivel );
                send_to_char( "Hecho.\n\r", ch );
                return;
        }

        if( !str_prefix( arg2, "tipo" ) )
        {

                if( !is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }

                if( x < 0 || x > MAX_TIPOS )
                {
                        send_to_char( "Eso no es un tipo de disciplina aceptado.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->tipo = atoi(argument);
                        ch_printf( ch, "Cambiando tipo a tipo &c%s&w.\n\r", disci_tnombre[atoi(argument)] );
                        save_nivel( nivel );
                        return;
                }
        }

        if( !str_prefix( arg2, "disciplina" ) )
        {

                if( !is_number( argument ) )
                {
                                send_to_char( "El valor debe de ser numerico.\n\r", ch );
                                return;
                }
                else
                {
                        if( atoi(argument) < 0 || atoi(argument) > MAX_DISCIPLINAS )
                        {
                                send_to_char( "No existe esa disciplina.\n\r", ch );
                                return;
                        }
                        else
                        {
                                nivel->disciplina = atoi(argument);
                                ch_printf( ch, "Cambiando disciplina a &c%s&w.\n\r", dis[atoi(argument)] );
                                save_nivel( nivel );
                                return;
                        }
                }
        }

        if( !str_prefix( arg2, "descripcion" ) )
        {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no puede ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->descripcion = STRALLOC( argument );
                        ch_printf( ch, "Cambiando descripcion a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
        }

        if( !str_prefix( arg2, "clan" ) )
        {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no puede ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->clan = STRALLOC( argument );
                        ch_printf( ch, "Cambiando clan a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
        }

        if( !str_prefix( arg2, "wait" ) )
        {
                if( !is_number( argument ) )
                {
                        send_to_char( "El valor debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->wait = atoi(argument);
                        ch_printf( ch, "Cambiando wait a &c%d&w.\n\r", atoi( argument ));
                        save_nivel( nivel );
                        return;
                }
         }

        if( !str_prefix( arg2, "sangre" ) )
        {
                if(  !is_number( argument ) )
                {
                        send_to_char( "El valor debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->sangre = atoi( argument );
                        ch_printf( ch, "Cambiando sangre requerida a &c%d&w.\n\r", atoi( argument ));
                        save_nivel( nivel );
                        return;
                }
         }

        if( !str_prefix( arg2, "nivel" ) )
        {
                if( !is_number( argument ) )
                {
                        send_to_char( "El valor debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->nivel = atoi( argument );
                        ch_printf( ch, "Cambiando nivel a &c%d&w.\n\r", atoi( argument ) );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "dnivel" ) )
        {
                if( !is_number( argument ) )
                {
                        send_to_char( "El valor debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->dnivel = atoi( argument );
                        ch_printf( ch, "Cambiando dnivel a &c%d&w.\n\r", atoi( argument ) );
                        save_nivel( nivel );
                        return;
                }
         }

        if( !str_prefix( arg2, "funcion" ) )
        {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        DO_FUN *fun = skill_function( argument );

                        if( fun == skill_notfound )
                        {
                                send_to_char( "Codigo no encontrado.\n\r", ch );
                                return;
                        }
                        nivel->do_fun = fun;
                        ch_printf( ch, "Cambiando funcion del nivel a %s.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

        if( !str_prefix( arg2, "dammes" ) )
        {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->dammes = STRALLOC( argument );
                        ch_printf( ch, "Cambiando mensaje de danyo a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "gomes" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->sevames = STRALLOC( argument );
                        ch_printf( ch, "Cambiando mensaje de remove a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "exitoch" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->exitoch = STRALLOC( argument );
                        ch_printf( ch, "Cambiando Exitoch a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "exitovict" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->exitovict = STRALLOC( argument );
                        ch_printf( ch, "Cambiando Exitovict a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "exitoroom" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->exitoroom = STRALLOC( argument );
                        ch_printf( ch, "Cambiando Exitoroom a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "exitomud" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->exitomud = STRALLOC( argument );
                        ch_printf( ch, "Cambiando Exitomud a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "falloch" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->falloch = STRALLOC( argument );
                        ch_printf( ch, "Cambiando falloch a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "fallovict" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->fallovict = STRALLOC( argument );
                        ch_printf( ch, "Cambiando fallovict a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "falloroom" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->falloroom = STRALLOC( argument );
                        ch_printf( ch, "Cambiando falloroom a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "fallomud" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->fallomud = STRALLOC( argument );
                        ch_printf( ch, "Cambiando fallomud a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "inmunech" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->inmunech = STRALLOC( argument );
                        ch_printf( ch, "Cambiando inmunech a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "inmunevict" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->inmunevict = STRALLOC( argument );
                        ch_printf( ch, "Cambiando inmunevict a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "inmuneroom" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->inmuneroom = STRALLOC( argument );
                        ch_printf( ch, "Cambiando inmuneroom a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "inmunemud" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->inmunemud = STRALLOC( argument );
                        ch_printf( ch, "Cambiando inmunemud a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "muertech" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->muertech = STRALLOC( argument );
                        ch_printf( ch, "Cambiando muertech a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "muertevict" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->muertevict = STRALLOC( argument );
                        ch_printf( ch, "Cambiando muertevict a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "muerteroom" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->muerteroom = STRALLOC( argument );
                        ch_printf( ch, "Cambiando muerteroom a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }

         if( !str_prefix( arg2, "muertemud" ) )
         {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        nivel->muertemud = STRALLOC( argument );
                        ch_printf( ch, "Cambiando muertemud a &c%s&w.\n\r", argument );
                        save_nivel( nivel );
                        return;
                }
         }


         send_to_char( "No se de que me hablas.\n\r", ch );
         return;
}

/*
 * Esto nos sirve para ver los valores de los niveles
 */
void do_nver( CHAR_DATA *ch, char *argument )
{
        NIVEL *nivel;

        if( IS_NPC(ch))
        {
                send_to_char( "Mande?.\n\r", ch );
                return;
        }


        if( argument[0] == '\0' )
        {
                send_to_char( "Uso: nver <nivel>.\n\r", ch );
                return;
        }

        nivel = get_nivel( argument );
        if( !nivel )
        {
                send_to_char( "Eso no es ningun nivel que haya creado.\n\r", ch );
                return;
        }

        if( get_trust( ch ) > LEVEL_CREATOR )
        ch_printf( ch, "\n\r&cFilename(&w%s&c)\n\r", nivel->fichero );

        ch_printf( ch, "\n\r&cNombre          : &w%-20.20s&c Wait : &w%2.2d\n\r", nivel->nombre, nivel->wait );
        ch_printf( ch, "&cDescripcion     : &w%s&w.\n\r", nivel->descripcion );
        ch_printf( ch, "&cDisciplina      : &w%-20.20s&c Nivel : &w%2.2d\n\r", dis[nivel->disciplina], nivel->nivel );
        ch_printf( ch, "&cTipo            : &w%-20.20s&c Sangre: &w%3.3d\n\r", disci_tnombre[nivel->tipo],  nivel->sangre );
        ch_printf( ch, "&cCodigo          : &w%-20.20s&c Dnivel: &w%4.4d\n\r", skill_name(nivel->do_fun), nivel->dnivel );
        ch_printf( ch,"&c=====================&wMensajes del nivel&w==============================\n\r" );
        ch_printf( ch, "\n\rMensaje de danyo  : &w%s\n\r", nivel->dammes );
        ch_printf( ch, "Mensaje de remove : &w%s\n\r", nivel->sevames );
        ch_printf( ch, "Exitoch           : &w%s\n\r", nivel->exitoch );
        ch_printf( ch, "Exitovict         : &w%s\n\r", nivel->exitovict );
        ch_printf( ch, "Exitoroom         : &w%s\n\r", nivel->exitoroom );
        ch_printf( ch, "Exitomud          : &w%s\n\r", nivel->exitomud );
        ch_printf( ch, "Fallooch          : &w%s\n\r", nivel->falloch );
        ch_printf( ch, "Fallovict         : &w%s\n\r", nivel->fallovict );
        ch_printf( ch, "Falloroom         : &w%s\n\r", nivel->falloroom );
        ch_printf( ch, "Fallomud          : &w%s\n\r", nivel->fallomud );
        ch_printf( ch, "Muertech          : &w%s\n\r", nivel->muertech );
        ch_printf( ch, "Muertevict        : &w%s\n\r", nivel->muertevict );
        ch_printf( ch, "Muerteroom        : &w%s\n\r", nivel->muerteroom );
        ch_printf( ch, "Muertemud         : &w%s\n\r", nivel->muertemud );
        ch_printf( ch, "Inmunech          : &w%s\n\r", nivel->inmunech );
        ch_printf( ch, "Inmuneevict       : &w%s\n\r", nivel->inmunevict );
        ch_printf( ch, "Inmuneeroom       : &w%s\n\r", nivel->inmuneroom );
        ch_printf( ch, "Inmuneemud        : &w%s\n\r", nivel->inmunemud );
        send_to_char( "\n\r", ch );

return;
}




/*
 * Modifica el estado del jugador con unos valores dados
 */
void affect_disci_modificar( CHAR_DATA *ch, DISCI_AFFECT *daf, bool fAdd )
{
        int mod;
        //int total = (ch->generacion + daf->modificador);
        //ch_ret retcode;

        mod = daf->modificador;

        if( fAdd )
        {
                xSET_BITS( ch->afectado_por, daf->bitvector );
				// SiGo, arreglado el error de los Affects negativos ya no aumenta disminuye
				if(mod > 0)
					mod = abs(mod);
				else
					mod = mod;
					
        }
        else
        {
                xREMOVE_BITS( ch->afectado_por, daf->bitvector );
				// SiGo, arreglado el error de los Affects negativos ya no aumenta disminuye
				if(mod > 0)
					mod -= (mod*2);
				else
					mod = abs(mod);
        }

        switch( daf->localizacion )
        {
                default:
	bug( "Affect_disci_modificar: localizacion desconocida %d.", daf->localizacion );
	return;

    case AP_NADA:						break;	
    case AP_FUE:           ch->mod_str		+= mod;	break;
    case AP_DES:           ch->mod_dex		+= mod;	break;
    case AP_INT:           ch->mod_int		+= mod;	break;
    case AP_SAB:           ch->mod_wis		+= mod;	break;
    case AP_CON:	   ch->mod_con		+= mod;	break;
    case AP_CAR:	   ch->mod_cha		+= mod; break;
    case AP_SUE:	   ch->mod_lck		+= mod; break;

    case AP_SEX:
        ch->sex = (ch->sex+mod) % 3;
	if ( ch->sex < 0 )
	    ch->sex += 2;
	ch->sex = URANGE( 0, ch->sex, 2 );
	break;

    /* No usados por posibles problemas futuros */
    case AP_CLASE:                                      break;
    case AP_EDAD:                                       break;
    case AP_ORO:                                        break;
    case AP_EXP:                                        break;

    case AP_ALTURA:        ch->height           += mod; break;
    case AP_PESO:          ch->weight           += mod; break;
    case AP_MANA:          ch->max_mana         += mod; break;
    case AP_VIDA:          ch->max_hit          += mod; break;
    case AP_MOVE:          ch->max_move         += mod; break;
    case AP_AC:            ch->armor            += mod; break;
    case AP_HITROLL:       ch->hitroll          += mod; break;
    case AP_DAMROLL:       ch->damroll          += mod; break;
    case AP_SAVING_POISON: ch->saving_poison_death	+= mod;	break;
    case AP_SAVING_ROD:    ch->saving_wand		+= mod;	break;
    case AP_SAVING_PARA:   ch->saving_para_petri	+= mod;	break;
    case AP_SAVING_BREATH: ch->saving_breath		+= mod;	break;
    case AP_SAVING_SPELL:  ch->saving_spell_staff	+= mod;	break;
    case AP_AFFECT:        SET_BIT( ch->afectado_por.bits[0], mod );break;
    case AP_RESISTENTE:    SET_BIT( ch->resistant, mod );	break;
    case AP_IMMUNE:        SET_BIT( ch->immune, mod );	break;
    case AP_SUSCEPTIBLE:   SET_BIT( ch->susceptible, mod );	break;
    case AP_FVOL:          ch->f_voluntad       += mod; break;
    case AP_MAN:           ch->manipulacion     += mod; break;
    case AP_INTI:          ch->intimidacion     += mod; break;
    case AP_CAMINO:        ch->pcamino          += mod; break;
    case AP_FRENESI:       ch->frenesi          += mod; break;
    case AP_OCULTISMO:     ch->ocultismo        += mod; break;
    case AP_GENERACION:    ch->generacion       += mod; break;
    case AP_SANGRE:                                     break; /* Pendiente del cambio del sistema de sangre para vampiros */
    case AP_AGRAVADAS:     ch->agravadas        += mod; break;
    case AP_DRUNK:
    if ( !IS_NPC(ch) )
	    ch->pcdata->condition[COND_DRUNK] =
		URANGE( 0, ch->pcdata->condition[COND_DRUNK] + mod, 48 );
	break;

    case AP_MENTAL:        ch->mental_state	= URANGE(-100, ch->mental_state + mod, 100); break;
    case AP_POSICION:                                   break; /* Pendiente */
    case AP_ASTUCIA:       ch->astucia          += mod; break;
    case AP_ALERTA:        ch->alerta           += mod; break;
    case AP_REMOVESPELL:
    if ( IS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC)
	||   IS_SET(ch->immune, RIS_MAGIC)
	||   saving_char == ch
	||   loading_char == ch )
	   return;

	mod = abs(mod);
	break;

     case AP_REMOVE:       REMOVE_BIT(ch->afectado_por.bits[0], mod);	break;

    }

return;
}



char * const    disci_tnombre [MAX_TIPOS] =
{
    "Ninguno", "Danyo", "Curativa", "Celeridad", "Usopropio", "Proteccion", "Subterfugio", "Senda",
    "Ritual", "Taumaturgia", "Mental", "Alma", "Invocacion", "Agravadas", "Frenesi", "Berserk",
    "Rothserker"
};

char * const    dis[MAX_DISCIPLINAS] =
{
    "", "animalismo", "auspex", "bardo", "celeridad", "conocimiento", "daimoinon", "dementacion", "dominacion",
    "extincion", "kinetismo", "fortaleza", "melepomene", "mortis", "mytheceria", "nigromancia", "nihilistica",
    "obeah", "obtenebracion", "ofuscacion", "presencia", "potencia", "protean", "quimerismo", "sanguinus",
    "serpentis", "spiritus", "tanatosis", "temporis", "taumaturgia", "taumaturgia_oscura", "vicisitud",
    "visceratika", "alimentacion" ,"valeren"
};

char * const    dis_affect[] =
{
        "potencia", "fortaleza", "celeridad", "traumatizado",
        "corazon_petreo", "capasombria", "sentidos_aguzados", "silencio_mortal",
        "debilidad", "testigo_de_las_tinieblas", "garras_bestia", "dominado", "forma_incorporea",
	"koma", "mascara_de_muerte", "fascinacion", "mirada_aterradora", "embrujar",
	"enfermo"
};

int coste;
int sangre;
int fallo;

/*
 * Disciplina Animalismo numero 1
 */

/*
 * Nivel 1 Susurros Amables, controla mob
 */

void do_susurrosamables( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    NIVEL *nivel = get_nivel( "susurros" );
    int chance;
    char buf[MAX_STRING_LENGTH];

    if(argument[0]=='\0')
    {
        send_to_char( "&wControlar con tus susurros a que mob?\n\r", ch );
        send_to_char( "susurrosamables <mob>.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
    }

    victim = get_char_room( ch, argument );

    if ( victim == ch )
    {
	send_to_char( "Este poder no te afectara a ti mismo.\n\r", ch );
	return;
    }

    if ( IS_SET( victim->immune, RIS_CHARM ) )
    {
        send_to_char( "Parece que no le afecta.\n\r", ch );
	return;
    }

    if(!IS_NPC(victim))
    {
        send_to_char( "No a jugadores.\n\r", ch );
        return;
    }

    chance = number_range( 1, 100 );

    if(( chance > 91 && chance < 100))
    {
        send_to_char( "No consigues dominar su mente!\n\r", ch );
        multi_hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

    if(xIS_SET(victim->act, PLR_DOMINADO)
    || IS_AFFECTED(victim, AFF_CHARM))
    {
        ch_printf( ch, "%s ya ha sido dominado.\n\r", victim->short_descr );

    if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
    	espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 8);
    else
    	WAIT_STATE(ch, 8);
    return;
    }

    if( lanzar_disciplina( ch, victim, nivel ))
    {
    if ( victim->master )
	stop_follower( victim );
    add_follower( victim, ch );
    af.type      = 1;
    af.duration  = (number_fuzzy( (ch->level + 1) / 5 ) + 1) * (13 / ch->generacion);
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = meb(AFF_CHARM);
    affect_to_char( victim, &af );

    sprintf( buf, "%s ha dominado con Animalismo a %s.", ch->name, victim->name);
    log_string_plus( buf, LOG_NORMAL, ch->level );

    if ( !IS_NPC(ch) )
    	ch->pcdata->charmies++;
    if ( IS_NPC( victim ) )
      start_hating( victim, ch );
    }
    return;
}




/*
 * Nivel 2 Llamada Salvaje invoca mob y lo controla
 */
void do_llamadasalvaje( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *victim;
    AFFECT_DATA af;
    char buf[MAX_STRING_LENGTH];
    NIVEL *nivel = get_nivel("llamada");
    int chance;

    if(argument[0]=='\0')
    {
        send_to_char( "&wLlamada salvaje a que mob?\n\r", ch );
        send_to_char( "llamadasalvaje <mob>.\n\r", ch );
        return;
    }

        if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion * 2 )
        {
        send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
        return;
        }
        else
        ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion * 2;

	chance = number_range( 1, 100 );

	if(( chance > 91 && chance < 100))
	{
        	send_to_char( "Nadie acude a tu llamada.\n\r", ch );
        	return;
    	}

    if ( ( victim = get_char_world( ch, argument ) ) == NULL
    ||   victim == ch
    ||   !IS_NPC(victim)
    ||   IS_SET(ch->in_room->room_flags,     ROOM_NO_ASTRAL)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SAFE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
    ||   IS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_SUMMON)
    ||   IS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
    ||   victim->level >= ch->level + 15 
    ||   victim->fighting
    || ( ch->in_room->area != victim->in_room->area )
    ||  (IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
    ||  (IS_NPC(victim) && saves_spell_staff( victim->level + 3, victim ))
    ||   !in_hard_range( victim, ch->in_room->area ))
    {
        send_to_char( "Te sientes decepcionado al no acudir nadie a esa llamada.\n\r", ch);
        return;
    }

 act( AT_MAGIC, "Pronuncias 'Llamada Salvaje'.", ch, NULL, NULL, TO_CHAR);
 act( AT_MAGIC, "$N pronuncia 'Animan animalum'.", ch, NULL, ch, TO_ROOM);

if( lanzar_disciplina( ch, victim, nivel ))
{
    if ( !IS_NPC( ch ) )
    {
	sprintf( buf, "%s llama a %s con la disciplina de Animalismo.", ch->name, victim->name );
	log_string_plus( buf, LOG_NORMAL, ch->level );
	to_channel( buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level ) );
    }
    char_from_room( victim );
    char_to_room( victim, ch->in_room );
    if ( victim->master )
    stop_follower( victim );
    add_follower( victim, ch );

    af.type      = 1;
    af.duration  = (number_fuzzy( (ch->level + 50) / 5 ) + 1) * (13 / ch->generacion );
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = meb(AFF_CHARM);
    affect_to_char( victim, &af );

    if ( !IS_NPC(ch) )
    	ch->pcdata->charmies++;
    if ( IS_NPC( victim ) )
      start_hating( victim, ch );

    if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
    espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 12);
    else
    WAIT_STATE(ch, 12);
}
    return;
}

/*
 * Disciplina Celeridad
 */

/* Nivel uno a cinco Celeridad divide el WAIT_STATE segun el nivel de la disciplina
   y da un atake adcional todo ello en fight.c */

void do_celeridad( CHAR_DATA *ch, char *argument )
{
        NIVEL *nivel;
        DISCI_AFFECT daf;
        int fallo;
        
	if(!IS_VAMPIRE(ch))
        {
                send_to_char( "Mejor deja Celeridad para los vampiros.\n\r", ch );
                return;
        }
        fallo=number_range(1,100);
        if( who_fighting( ch ) != NULL && fallo>=75 && fallo<=100)
        { 
		send_to_char( "&wNo puedes concentrarte en medio de un combate!\n\r", ch);
               	return;
        }

        if( xIS_SET(ch->afectado_por, DAF_CELERIDAD ) )
        {
        act( AT_MAGIC, "Aun mas rapido???.", ch, NULL, NULL, TO_CHAR );
        return;
        }
              else
      if(argument[0]=='\0')
      {
        act( AT_MAGIC, "Pronuncias 'Celeridad'.", ch, NULL, NULL, TO_CHAR );
        act( AT_MAGIC, "$N pronuncia 'Shoram tak'.", ch, NULL, ch, TO_ROOM );

        nivel = get_nivel( "celeridad" );

        if ( lanzar_disciplina( ch, NULL, nivel ))
        {
        daf.tipo = TIPO_CELERIDAD;
        daf.localizacion = AP_NADA;
        if(ch->pcdata->habilidades[DISCIPLINA_CELERIDAD] > 5)
        daf.duracion = ((ch->level * 16 / ch->generacion));
        else
        daf.duracion = ((ch->level * (ch->pcdata->habilidades[DISCIPLINA_CELERIDAD] * 6) / ch->generacion));

        daf.disciplina = 4;
        daf.modificador = 0;
        daf.nivel = 405;
        daf.bitvector = meb(DAF_CELERIDAD);
        disciplina_to_char( ch, &daf );

	if (!IS_IMMORTAL(ch))
		WAIT_STATE(ch, 8);
       } /* Cierra if( lanzar_disciplina( ch, NULL, nivel) ) */
      }
      else
      send_to_char( "No debes utilizar Celeridad en otros.\n\r", ch );

    return;
}

/*
 * Disciplina Ofuscacion
 */

/* Nivel uno afectado por hide  incluso en posicion de lucha */
void do_capasombria( CHAR_DATA *ch, char *argument )
{
	NIVEL *nivel;
        DISCI_AFFECT daf;
	
        if ( ch->pcdata->condition[COND_BLOODTHIRST] <= 10 )
                     {
                      send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
                      return;
                     }
                     else
                     ch->pcdata->condition[COND_BLOODTHIRST] -= 10;

        if(who_fighting( ch ) != NULL)
        {
                send_to_char( "Estas demasiado ocupado como para hacerlo.\n\r", ch);
                return;
        }

	if (xIS_SET(ch->afectado_por, DAF_CAPA_SOMBRIA))
	{
            send_to_char( "Ya estas cobijado por las sombras.\n\r", ch);
	    return;
	}
        act( AT_MAGIC, "$n pronuncia 'Tenebrum soa'.", ch, NULL, NULL, TO_ROOM );
	act( AT_MAGIC, "$n realiza un ritual de ofuscacion y desaparece en medio de una capa de sombras.", ch, NULL, NULL, TO_ROOM );

	act( AT_MAGIC, "Pronuncias 'Mascara de Sombras'.", ch, NULL, NULL, TO_CHAR );
        act( AT_MAGIC, "Una mascara de sombras te cobija.", ch, NULL, NULL, TO_CHAR );
	
	nivel = get_nivel( "capa" );
        if ( lanzar_disciplina( ch, NULL, nivel ))
        {
        	daf.tipo = 1;
        	daf.localizacion = AP_NADA;
        	daf.duracion = 250 * 16 / ch->generacion;
        	daf.disciplina = 19;
        	daf.modificador = 0;
        	daf.nivel = 1901;
        	daf.bitvector = meb(DAF_CAPA_SOMBRIA);
        	disciplina_to_char( ch, &daf );
	}
    if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
    	espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 4);
    else
    	WAIT_STATE(ch, 4);

    return;
}


/*
 * Disciplina Dementacion
 */

/* Discilina Dementacion */
/* Nivel uno Aumenta el estado mental de la victima y baja int segun generacion jugador */
void do_pasion( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *victim;
        int fallo;
	NIVEL *nivel;
	DISCI_AFFECT daf;

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if (xIS_SET(victim->afectado_por, DAF_TRAUMATIZADO ) )
    {
	    act( AT_MAGIC, "No puedes volver a conjurar Pasion sobre el.", ch, NULL, NULL, TO_CHAR );
            return;
    }

    if ( !IS_NPC(victim) && victim->fighting )
    {
	send_to_char( "No puedes usar Pasion contra un jugador que esta luchando.\n\r", ch );
	return;
    }

    if ( ( who_fighting( ch ) ) != NULL )
    {
	send_to_char( "No puedes concentrarte estas luchando!\n\r", ch );
	return;
    }

    if ( is_safe(ch, victim, TRUE) )
    {
        send_to_char( "En esta habitacion mejor no.\n\r", ch );
        return;
    }

     if ( !IS_NPC(victim) && IS_IMMORTAL(victim) && (ch->level <= victim->level) )
    {
	send_to_char( "No puedes usar Pasion con Inmortales de igual o mayor rango.\n\r", ch);
	return;
    }

        if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion / 2 )
        {
        	send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
        	return;
        }
        else
        	ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion / 2;

   /* Probabilidad de fallar de un 15% */
    fallo = number_range(1, 100);

    if((!IS_NPC(ch) && !IS_NPC(victim)
    && fallo <= 100  && fallo >= 85 ))
    {
        send_to_char( "No has podido traumatizarle!.\n\r", ch);
        ch_printf( victim, "%s ha intentado ponerte en estado de trauma!.\n\r", ch->name );
        multi_hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

      if(!IS_NPC(victim))
      {
	act( AT_MAGIC, "Pronuncias 'Pasion'.", ch, NULL, NULL, TO_CHAR );
        act( AT_MAGIC, "$n pronuncia 'Arkach'.", ch, NULL, NULL, TO_ROOM );

	nivel = get_nivel("pasion");
        if( lanzar_disciplina( ch, victim, nivel ) )
        {
			daf.duracion = ch->level*10/ch->generacion;
                        daf.localizacion = AP_INT;
                        daf.modificador = -13/ch->generacion;
                        daf.disciplina = 7;
                        daf.nivel = 701;
                        daf.bitvector = meb(DAF_TRAUMATIZADO);
                        disciplina_to_char( victim, &daf);
        }

     }
     else
	send_to_char( "En mobs no va a funcionar.\n\r", ch );		
   return;
}

/* Nivel 2 Embrujar el alma: embruja al enemigo elevando la probabilidad de q falle*/
/* Nkari 2004 */

void do_embrujar( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *victim;
        NIVEL *nivel;
        DISCI_AFFECT daf;
        int fallo;


	if(argument[0]=='\0')
    	{
            send_to_char( "Embrujar a quien?\n\r", ch);
            return;
        }
	
	if (( victim = get_char_room( ch, argument ) ) == NULL )
        {
            send_to_char( "No esta aqui.\n\r", ch );
            return;
        }

        if (victim == ch)
        {
            send_to_char( "No puedes embrujarte a ti mismo.\n\r", ch );
            return;
	}
        
        if (xIS_SET(victim->afectado_por, DAF_EMBRUJADO))
        {
            send_to_char( "El ya esta embrujado.\n\r", ch);
            return;
        }
        
        if ( !IS_NPC(victim) && victim->fighting )
    	{
            send_to_char( "No puedes Embrujar a alguien que esta luchando.\n\r", ch );
            return;
    	}
	
	if (victim->generacion < ch->generacion-2)
	{
	    send_to_char( "No tienes suficiente poder como para embrujarle!.\n\r", ch);
	    ch_printf( victim, "%s ha intentado embrujarte,pero no tiene suficiente poder!.\n\r", ch->name );
 	    return;       
        }

	act( AT_MAGIC, "$n pronuncia 'xelom maliek'.", ch, NULL, NULL, TO_ROOM );
        act( AT_MAGIC, "Pronuncias 'Embrujar el Alma'.", ch, NULL, NULL, TO_CHAR );

        
        fallo=number_range(1 , 100);
        if (fallo >= 80 && fallo <= 100)
        {
            send_to_char( "No has conseguido embrujarle!.\n\r", ch);
            ch_printf( victim, "%s ha intentado embrujarte!.\n\r", ch->name );
            if( IS_NPC(victim))
            multi_hit(victim, ch, TYPE_UNDEFINED);
            return;
        }
        
        nivel = get_nivel( "embrujar" );
        if ( lanzar_disciplina( ch, victim, nivel ) &&  !is_safe(ch,victim, TRUE) )
        {
        
                daf.tipo = 1;
                daf.localizacion = AP_NADA;
                daf.duracion = 250 * 16 / ch->generacion;
                daf.disciplina = 7;
                daf.modificador = 0;
                daf.nivel = 702;
                daf.bitvector = meb(DAF_EMBRUJADO);
                disciplina_to_char( victim, &daf );
        }
}



/* Nivel 3 Cancion serena afecta su Wait */

void do_cancionserena( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA  *victim;
  NIVEL *nivel;

    if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    {
	send_to_char( "A quien quieres paralizar con tu canto sereno?\n\r", ch );
	return;
    }

    if ( !IS_NPC(victim) && victim->fighting )
    {
	send_to_char( "No puedes paralizar a un jugador que esta luchando.\n\r", ch );
	return;
    }

    if ( is_safe(ch, victim, TRUE) )
    {
        send_to_char( "&wEn esta habitacion mejor no.\n\r", ch );
        return;
    }

    if ( ( who_fighting( ch ) ) != NULL )
    {
        send_to_char( "No puedes concentrarte estas luchando!\n\r", ch );
        return;
    }

    if ( !IS_NPC(victim) && IS_IMMORTAL(victim) && (ch->level <= victim->level) )
    {
	send_to_char( "No puedes paralizar a Inmortales de igual o mayor rango.\n\r", ch);
	return;
    }

    if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion )
       {
       send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
       return;
       }
       else
       ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion;

nivel = get_nivel( "cancion" );
if( lanzar_disciplina( ch, victim, nivel ) )
{
    /* Probabilidad de fallar de un 25% */
    fallo = number_range(1, 100);

   if ( (who_fighting(ch)) != NULL )
   {
    if(!IS_NPC(victim))
    if(( fallo <= 100  && fallo >= 75 )
    || victim->generacion < ch->generacion - 1)
    {
        send_to_char( "&wNo has podido paralizarle!.\n\r", ch);
        ch_printf( victim, "&w%s ha intentado paralizarte!.\n\r", ch->name );
        multi_hit(victim, ch, TYPE_UNDEFINED);

        if ( IS_NPC( victim ) )
        start_hating( victim, ch );
        return;
    }
  }
  else
  {
   if(!IS_NPC(victim))
    if(( fallo <= 100  && fallo >= 40 )
    || victim->generacion < ch->generacion - 1)
    {
        send_to_char( "&wNo has podido paralizarle!.\n\r", ch);
        ch_printf( victim, "&w%s ha intentado paralizarte!.\n\r", ch->name );
        multi_hit(victim, ch, TYPE_UNDEFINED);

        if ( IS_NPC( victim ) )
        start_hating( victim, ch );
        return;
    }
  }

    if ( IS_AWAKE(victim) )
    {
        act( AT_MAGIC, "$N pronuncia 'Serakina miutseik'.", ch, NULL, ch, TO_ROOM );
        act( AT_MAGIC, "Pronuncias 'Canto Sereno'.", ch, NULL, NULL, TO_CHAR );
	act( AT_MAGIC, "Tus brazos y piernas te pesan y no te responden, estas paralizado.", victim, NULL, NULL, TO_CHAR );
	act( AT_MAGIC, "$n queda paralizado por el Canto Sereno de $N.", victim, NULL, ch, TO_ROOM );
        ch_printf( ch, "&BConsigues meterle %d de wait a %s.\n\r", ch->pcdata->habilidades[DISCIPLINA_DEMENTACION] * 8, victim->name );
    }

	if (!IS_IMMORTAL(ch))
	{
      		if(xIS_SET(victim->afectado_por, DAF_CELERIDAD))
      			WAIT_STATE(victim, (ch->pcdata->habilidades[DISCIPLINA_DEMENTACION] * 6));
      		else
      			WAIT_STATE(victim, (ch->pcdata->habilidades[DISCIPLINA_DEMENTACION] * 8));

      		if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
      			espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 5);
      		else
      			WAIT_STATE(ch, 5);
	}
}    
return;
}

/* Nivel cinco, coma crea estado coma */
CHAR_DATA *victim;

void do_coma( CHAR_DATA *ch, char *argumento )
{
    DISCI_AFFECT daf;    
    NIVEL *nivel;
    int fallo = 0;

    set_char_color( AT_WHITE, ch );

    if(!IS_VAMPIRE(ch) && !IS_IMMORTAL(ch))
        {
                send_to_char( "Mejor deja Dementacion para los vampiros.\n\r", ch );
                return;
        }

    if(argumento[0]=='\0')
    {
        send_to_char( "Dejar a quien en coma?\n\r", ch);
        return;
    }

    if (victim == ch)
    {
            send_to_char( "No puedes echartelo a ti mismo.\n\r", ch );
            return;
    }


    if (( victim = get_char_room( ch, argumento ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if(who_fighting( ch ) != NULL)
    {
        send_to_char( "No puedes hacerlo en mitad de un combate!\n\r", ch);
        return;
    }

    if ( !IS_NPC(victim) && victim->fighting )
    {
     send_to_char( "No puedes poner en coma a un jugador que esta luchando.\n\r", ch );
     return;
    }

    if ( is_safe(ch, victim, TRUE) )
        return;

    if ( IS_SET( victim->immune, RIS_MAGIC ) )
    {
	send_to_char( "Parece que no le afecta en absoluto.\n\r", ch);
        send_to_char( "No has podido dejarle en coma!.\n\r", ch);
        ch_printf( victim, "%s ha intentado ponerte en estado de coma!.\n\r", ch->name );
        if( IS_NPC(victim))
        multi_hit(victim, ch, TYPE_UNDEFINED);
	return;
    }

    /* Anyadido por Kayser, para no dormir Inmortales :_) */
    if ( !IS_NPC(victim) && IS_IMMORTAL(victim) && (ch->level <= victim->level) )
    {
    send_to_char( "No puedes poner en coma a Inmortales de igual o mayor rango.\n\r", ch);
    return;
    }

    /*Anyadido por Nkari pa no dejar en koma a vampiros de mas de 3 generaciones q la tuya*/

     if ((!IS_NPC(victim)) && ( ch->generacion-2 > victim->generacion))
     { 
     send_to_char( "No tienes suficiente poder para dejar en coma a ese cainita.\n\r", ch);
     return;
     } 
    /* Probabilidad de fallar de un 25% */
    fallo = number_range(1, 100);

    if((!IS_NPC(ch) && !IS_NPC(victim)
    && fallo <= 100  && fallo >= 80))
    {
        send_to_char( "No has podido dejarle en coma!.\n\r", ch);
        ch_printf( victim, "%s ha intentado ponerte en estado de coma!.\n\r", ch->name );
        if( IS_NPC(victim))
        multi_hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

    nivel = get_nivel( "koma" );
    if( lanzar_disciplina( ch, victim, nivel ))
    {
    if ( IS_AWAKE(victim) )
    {

   act( AT_MAGIC, "Pronuncias 'Koma'.", ch, NULL, NULL, TO_CHAR );
   act( AT_MAGIC, "$n pronuncia 'kmtoum'.", ch, NULL, NULL, TO_ROOM );
   victim->position = POS_SLEEPING;
    }
    else
    if( !IS_NPC(victim))
    {
     ch_printf( ch, "&B%s ya esta durmiendo no puedes dejarlo en coma.\n\r", victim->name );
     return;
    }
    else
    {
     ch_printf( ch, "&B%s ya esta durmiendo no puedes dejarlo en coma.\n\r", victim->short_descr );
     return;
    }

    daf.duracion  = (20+ch->generacion);
    daf.localizacion  = AP_DAMROLL;
    daf.modificador  = -20;
    daf.nivel = 705;
    daf.disciplina = 7;
    daf.bitvector = meb(DAF_KOMA);
    disciplina_to_char( victim, &daf );

    if ( !IS_NPC( victim ) )
    {
   sprintf( log_buf, "%s ha dejado en coma a %s.", ch->name, victim->name );
	log_string_plus( log_buf, LOG_NORMAL, ch->level );
	to_channel( log_buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level ) );
    }
    if ( IS_NPC( victim ) )
      start_hating( victim, ch );
    }

    return;
}

void do_oleadademente( CHAR_DATA *ch, char *argumento )
{
    CHAR_DATA *victim;
    NIVEL *nivel;
    int dam;
    int chance;	

    set_char_color( AT_GREY, ch );


    if(( argumento[0]=='\0'
       && !ch->fighting ))
    {
        send_to_char( "&wOleada demente a quien?.\n\r", ch);
        return;
    }

    if ( (victim=who_fighting(ch)) != NULL )
        argumento = victim->name;

    if (( victim = get_char_room( ch, argumento ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if(victim == ch)
    {
    send_to_char( "A ti mismo? no creo que te gustara eso...\n\r", ch);
    return;
    }


    chance = ris_save( victim, victim->level, RIS_DRAIN ) * 2;
    if ( saves_spell_staff( chance, victim ) )
    {
        if(!IS_NPC(victim))
	ch_printf(ch, "&gParece que %s consigue bloquear tu ataque demente.\n\r", victim->name);
        else
        ch_printf(ch, "&gParece que %s consigue bloquear tu ataque demente.\n\r", victim->short_descr);

        ch_printf(victim, "&gBloqueas la Oleada Demente de %s.\n\r", ch->name );
	return;
    }

if((!IS_PACIFIST(ch))
   && !is_safe(ch, victim, TRUE))
{
nivel = get_nivel( "oleada" );
if( lanzar_disciplina( ch, victim, nivel ) )
{
act( AT_MAGIC, "Pronuncias 'Oleada Demente'.", ch, NULL, NULL, TO_CHAR );
act( AT_MAGIC, "$N pronuncia 'sigfrigt'.", ch, NULL, ch, TO_ROOM );

if (ch->class != CLASS_MALKAVIAN)
	dam -= dam * 0.3;

if(!IS_NPC(victim))
	victim->exp -= number_range( ch->level / 2, 3 * ch->level / 2 );

        if(victim->mana > 1)
	victim->mana	-= (victim->mana / number_range(15, 30));
        if(victim->move > 1)
	victim->move	-= (victim->move / number_range(65, 100));
	dam		 = (ch->level * (100 / ch->generacion)) + number_range(100, 500);
        if((ch->hit != ch->max_hit
            && victim->level >= ch->level -2
            && !xIS_SET(victim->affected_by, AFF_SANCTUARY)))
	ch->hit		+= dam / 4;
        if((ch->hit != ch->max_hit
            && victim->level >= ch->level -2
            && xIS_SET(victim->affected_by, AFF_SANCTUARY)))
        ch->hit         += (dam / 4) / 2;

    if ( ch->hit > ch->max_hit )
	ch->hit = ch->max_hit;

damage( ch, victim, dam, 19);
}
}
return;
}

/*
 * Disciplina Dominacion
 */

/* Nivel uno Dominio, para hacer decir a otros frases simples. */
/* Kayser 2004 */
void do_dominio( CHAR_DATA *ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
	int fallo = 0;
	NIVEL *nivel;

	argument = one_argument( argument, arg );
	if ( arg[0] == '\0' || argument[0] == '\0' )
    	{
        	send_to_char( "Dominio necesita un objetivo y una orden.\n\r", ch );
        	return;
    	}

        if ( ( victim = get_char_world( ch, arg ) ) == NULL )
        {
            send_to_char( "No esta por aqui.\n\r", ch );
            return;
        }

	if ( IS_NPC( victim ))
	{
            send_to_char( "No puedes dominar a mobs.\n\r", ch );
            return;
        }
	
	if (IS_IMMORTAL( victim ))
        {
            send_to_char( "No puedes dominar a Inmortales.\n\r", ch );
            return;
        }

	if(who_fighting( ch ) != NULL)
    	{
        	send_to_char( "No puedes hacerlo en mitad de un combate!\n\r", ch);
        	return;
    	}
	if ( victim->generacion < ch->generacion ) 
	{
	    send_to_char( "No puedes dominar a un vampiro mas anciano que tu.\n\r", ch);
	    return;
	}	 
	if ( victim == ch )
        {
            send_to_char( "Si quieres decir algo dilo...\n\r", ch );
            return;
        }
	
	act( AT_MAGIC, "Pronuncias 'Dominio'.", ch, NULL, NULL, TO_CHAR);
 	act( AT_MAGIC, "$N pronuncia 'Dominustequ'.", ch, NULL, ch, TO_ROOM);

	fallo = number_range(1, 100);
        if ( fallo <= 100  && fallo >= 75)
        {
		send_to_char( "No has podido dominarle!.\n\r", ch);
                return;
        }

	nivel = get_nivel( "dominio" );
    	if( lanzar_disciplina( ch, victim, nivel ))
	{
		if (!str_prefix( "decir", argument ) || !str_prefix( "charlar", argument)  || !str_prefix( "gritar", argument))
		{
			interpret(victim, argument);
			return;
		}
	}
	else
		return;
	send_to_char( "Solo puedes hacerle DECIR, GRITAR o CHARLAR cosas.\n\r", ch );
	return;
}
/* Nivel Dos: Mesmerismo, para hacer que otros hagan cosas simples. */
/* Kayser 2004 */

void do_mesmerismo( CHAR_DATA *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        int fallo = 0;
        NIVEL *nivel;

        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' || argument[0] == '\0' )
        {
                send_to_char( "Mesmerismo necesita un objetivo y una orden.\n\r", ch );
                return;
        }

        if ( ( victim = get_char_world( ch, arg ) ) == NULL )
        {
            send_to_char( "No esta por aqui.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ))
        {
            send_to_char( "No puedes dominar a mobs.\n\r", ch );
            return;
        }

        if (IS_IMMORTAL( victim ))
        {
            send_to_char( "No puedes dominar a Inmortales.\n\r", ch );
            return;
        }

        if(who_fighting( ch ) != NULL)
        {
                send_to_char( "No puedes hacerlo en mitad de un combate!\n\r", ch);
                return;
        }
	if ( victim->generacion < ch->generacion )
	{
	        send_to_char("No puedes dominar a vampiros mas ancianos que tu.\n\r", ch);
		return;
	}
	if ( victim == ch )
        {
            send_to_char( "Si quieres hacer algo hazlo...\n\r", ch );
            return;
        }

	act( AT_MAGIC, "Pronuncias 'Mesmerismo'.", ch, NULL, NULL, TO_CHAR);
 	act( AT_MAGIC, "$N pronuncia 'Ebeneditus Frunturis'.", ch, NULL, ch, TO_ROOM);

	fallo = number_range(1, 100);
        if ( fallo <= 100  && fallo >= 75)
        {
                send_to_char( "No has podido dominarle!.\n\r", ch);
                return;
        }


        nivel = get_nivel( "mesmerismo" );
        if( lanzar_disciplina( ch, victim, nivel ))
        {
                if (!str_prefix( "dormir", argument ) || !str_prefix( "sentar", argument ) || !str_prefix( "descansar", argument))
                {
                        interpret(victim, argument);
	                return;
                }
        }
	else
		return;
        send_to_char( "Solo puedes hacerle SENTAR, DORMIR o DESCANSAR.\n\r", ch );
        return;
}

/* Nivel Tres: Memoria del Juerguista, para hacer que otros se muevan. */
/* Kayser 2004 */

void do_memoria_del_juerguista( CHAR_DATA *ch, char *argument )
{
        char arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        int fallo = 0;
        NIVEL *nivel;

        argument = one_argument( argument, arg );
        if ( arg[0] == '\0' || argument[0] == '\0' )
        {
                send_to_char( "Memoria del Juerguista necesita un objetivo y una orden.\n\r", ch );
                return;
        }

        if ( ( victim = get_char_world( ch, arg ) ) == NULL )
        {
            send_to_char( "No esta por aqui.\n\r", ch );
            return;
        }

        if ( IS_NPC( victim ))
        {
            send_to_char( "No puedes dominar a mobs.\n\r", ch );
            return;
        }
	if (IS_IMMORTAL( victim ))
        {
            send_to_char( "No puedes dominar a Inmortales.\n\r", ch );
            return;
        }

        if(who_fighting( ch ) != NULL)
        {
                send_to_char( "No puedes hacerlo en mitad de un combate!\n\r", ch);
                return;
        }
        if ( victim->generacion < ch->generacion )
        {
                send_to_char("No puedes dominar a vampiros mas ancianos que tu.\n\r", ch);
                return;
        }
	if ( victim == ch )
        {
            send_to_char( "Si quieres hacer algo hazlo...\n\r", ch );
            return;
        }
	
	fallo = number_range(1, 100);
        if ( fallo <= 100  && fallo >= 75)
        {
                send_to_char( "No has podido dominarle!.\n\r", ch);
                return;
        }


	act( AT_MAGIC, "Pronuncias 'Memoria de Juerguista'.", ch, NULL, NULL, TO_CHAR);
 	act( AT_MAGIC, "$N pronuncia 'Jentristux ist Meridium'.", ch, NULL, ch, TO_ROOM);

        nivel = get_nivel( "memoria" );
	if( lanzar_disciplina( ch, victim, nivel ))
        {
                if (!str_prefix( "norte", argument ) 
		 || !str_prefix( "sur", argument ) 
                 || !str_prefix( "este", argument )
		 || !str_prefix( "oeste", argument )
		 || !str_prefix( "sureste", argument )
		 || !str_prefix( "suroeste", argument )
		 || !str_prefix( "noreste", argument )
 		 || !str_prefix( "noroeste", argument )
		 || !str_prefix( "arriba", argument )
		 || !str_prefix( "abajo", argument ))
		{
                        interpret(victim, argument);
                        return;
                }
        }
	else
		return;
        send_to_char( "Solo puedes hacerle MOVER en UNA DIRECCION.\n\r", ch );
        return;
}



 
/*
  *Disciplina Mortis
  */
 /*Nkari 2004*/
 

 /* Nivel 1, MascaraDeMuerte: Marchitas a tu enemigo hasta que parece un cadaver
  */
void do_mascara(CHAR_DATA *ch, char* argumento) 
{
	CHAR_DATA *victim;
	int fallo;
	NIVEL *nivel;
	int damvida=0,dammov=0,dammana=0,dam=0;
	int dura;
	DISCI_AFFECT daf,daf2,daf3,daf4,daf5;	

	if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion / 3 )
        {
        	send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
        	return;
        }
	else
        	ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion / 3;

	if (( victim = get_char_room( ch, argumento ) ) == NULL )
	{
        	send_to_char( "No esta aqui.\n\r", ch );
        	return;
    	}

	if ( xIS_SET(victim->afectado_por, DAF_MASCARA ))
        {
        	act( AT_MAGIC, "Ya esta afectado.", ch, NULL, NULL, TO_CHAR );
                return;
        }
	if(victim == ch )
    	{
    		send_to_char( "A ti mismo? no creo que te gustara eso...\n\r", ch);
    		return;
    	}

	if(who_fighting( ch ) != NULL)
        {
        	send_to_char( "&wNo puedes concentrarte en medio de un combate!\n\r", ch);
                return;
        }
        /* Probabilidad de fallar de un 15% */

	fallo = number_range(1, 100);
	if( fallo <= 100  && fallo >= 85)
    	{
        	send_to_char( "No has podido marchitarle!.\n\r", ch);
        	ch_printf( victim, "%s ha intentado marchitarte!.\n\r", ch->name );
        	if( IS_NPC(victim)) {  multi_hit(victim, ch, TYPE_UNDEFINED); }
        	return;
    	}


	if(!IS_PACIFIST(ch) && !is_safe(ch, victim, TRUE)) {
	
		act( AT_MAGIC, "Pronuncias 'Mascara de Muerte.", ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, "$N pronuncia 'xeok ta rol'.", ch, NULL, ch, TO_ROOM );

		nivel = get_dnivel( 1301 );
	        if( lanzar_disciplina( ch, victim, nivel ) )
		{

        	act( AT_MAGIC, "Pronuncias 'Mascara de Muerte.", ch, NULL, NULL, TO_CHAR );
			act( AT_MAGIC, "$N pronuncia 'xeok ta rol'.", ch, NULL, ch, TO_ROOM );

		   	

        		damvida -= ((10000 / ch->generacion) + number_range(1 , 200));
			dammana -= (victim->mana / number_range(25, 35));
			dammov -= (victim->move / number_range(100, 120));
			dam -= 15;
			dura = 50 + victim->generacion;
		
		
			daf.duracion = dura;
              		daf.localizacion = AP_VIDA;
              		daf.modificador = damvida;   										
              		daf.disciplina = 13;
              		daf.nivel = 1301;
			daf.bitvector = meb(DAF_MASCARA);
             		disciplina_to_char( victim, &daf);
               	
			daf2.duracion = dura;
               		daf2.localizacion = AP_MOVE;
               		daf2.modificador = dammov;
         		daf2.disciplina = 13;
               		daf2.nivel = 1301;
			daf.bitvector = meb(DAF_MASCARA);
               		disciplina_to_char( victim, &daf2);
               		
			if (!IS_VAMPIRE ( victim ))
			{	
				daf3.duracion = dura;
               			daf3.localizacion = AP_MANA;
               			daf3.modificador = dammana;
               			daf3.disciplina = 13;
               			daf3.nivel = 1301;
				daf.bitvector = meb(DAF_MASCARA);
               			disciplina_to_char( victim, &daf3);
  			}             	       
			daf4.tipo = 1;
			daf4.duracion = dura;
               		daf4.localizacion = AP_HITROLL;
               		daf4.modificador = dam;
              		daf4.disciplina = 13;
               		daf4.nivel = 1301;
			daf.bitvector = meb(DAF_MASCARA);
               		disciplina_to_char( victim, &daf4);
               	
			daf5.tipo = 1;
			daf5.duracion = dura;
               		daf5.localizacion = AP_DAMROLL;
               		daf5.modificador = dam;
               		daf5.disciplina = 13;
               		daf5.nivel = 1301;
			daf.bitvector = meb(DAF_MASCARA);
               		disciplina_to_char( victim, &daf5);

		/* SiGo
			Ahora se resetea la vida mana y move de la victima en vez de hacer el canelo cambiandolo antes 
		   	Tambien me cercioro de no estar restaurando a la victima si su mana move o vida actual es menor ya,
		   	que la que le produce el estado Mascara de Muerte 
		*/

			if(victim->hit > victim->max_hit)
				victim->hit = victim->max_hit;
			if(victim->mana > victim->max_mana)
				victim->mana = victim->max_mana;
			if(victim->move > victim->max_move)
				victim->move = victim->max_move;


			}//cierro el si estaba ya afectado     
		}//cierro if is pacifist
     
	if (!IS_IMMORTAL(ch))
     	{
        	if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
                	espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 12);
        	else
                	WAIT_STATE(ch, 12);
     	}
	
	return;
}
 
/*
 * Disciplina Presencia
 * Nkari 2004
 */

/* Nivel 1: Fascinacion: Te da 5 de car y un plus al hitroll */

void do_fascinacion( CHAR_DATA *ch, char *argument )
{
      NIVEL *nivel = get_nivel( "fascinacion" );
      DISCI_AFFECT daf,daf2;

     if ( (  who_fighting( ch ) ) != NULL )
     	{
        	 send_to_char( "&wNo puedes concentrarte en medio de un combate!\n\r", ch);
                 return;
        }

     if (xIS_SET(ch->afectado_por, DAF_FASCINACION))
        {
         	 act( AT_MAGIC, "Ya eres fascinante!.", ch, NULL, NULL, TO_CHAR );
                 return;
        }
     else
    	 if(argument[0]=='\0')
      		{
			act( AT_MAGIC, "$N pronuncia 'melenteriom'.", ch, NULL, ch, TO_ROOM );
                        act( AT_MAGIC, "Pronuncias 'Fascinacion'.", ch, NULL, NULL, TO_CHAR );
     
  			if( lanzar_disciplina( ch, NULL, nivel ) )
       			{
        		
        		daf.tipo = TIPO_USOPROPIO;
        		daf.localizacion = AP_CAR;
        		daf.modificador = 5;
        		daf.disciplina = DISCIPLINA_PRESENCIA;
        		daf.nivel = 2001;
        		daf.duracion = ch->pcdata->habilidades[DISCIPLINA_PRESENCIA] * ch->level;
        		daf.bitvector = meb(DAF_FASCINACION);
        		disciplina_to_char( ch, &daf );
        		
        		daf2.tipo = TIPO_USOPROPIO;
        		daf2.localizacion = AP_HITROLL;
        		daf2.modificador = (500/ch->generacion);
        		daf2.disciplina = DISCIPLINA_PRESENCIA;
        		daf2.nivel = 2001;
        		daf2.duracion = ch->pcdata->habilidades[DISCIPLINA_PRESENCIA] * ch->level;
        		daf2.bitvector = meb(DAF_FASCINACION);
        		disciplina_to_char( ch, &daf2 );
       			}
      		}
        else
      	send_to_char( "No puedes usar Fascinacion en otros.\n\r", ch );

    return;
}

/* Nivel 2: MIrada Aterradora: Te kita hit,dam y saves en caso de entrar */
/* Nkari 2004 */

void do_mirada(CHAR_DATA *ch,char *argumento) 
{
	CHAR_DATA *victim;
	int aff=0;
	int fallo;
	NIVEL *nivel;
	int dam=0;
	int dura;
	DISCI_AFFECT daf1,daf2,daf3;

   if(( argumento[0]=='\0'
       && !ch->fighting ))
    {
        send_to_char( "&wMirada aterradora a quien?.\n\r", ch);
        return;
    }

    if ( (victim=who_fighting(ch)) != NULL )
        argumento = victim->name;

    if (( victim = get_char_room( ch, argumento ) ) == NULL )
    {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
    }

    if(victim == ch)
    {
    send_to_char( "A ti mismo? no creo que te gustara eso...\n\r", ch);
    return;
    }

	if (xIS_SET(victim->afectado_por, DAF_MIRADA))
        {
                act( AT_MAGIC, "Ya esta afectado.", ch, NULL, NULL, TO_CHAR );
                return;
        }
/* Probabilidad de fallar de un 15% o si la victima tiene 3 o mas puntos de intimidacion q el ch*/

if (!IS_NPC(victim))
	aff=(ch->intimidacion+3 - victim->intimidacion); 
else
	aff=ch->intimidacion/2;

fallo = number_range(1, 100);
if((fallo <= 100  && fallo >= 85) || (aff <= 0) )
    {
        send_to_char( "No has podido aterrarle!.\n\r", ch);
        ch_printf( victim, "%s ha intentado aterrarte!.\n\r", ch->name );
        if( IS_NPC(victim))
        multi_hit(victim, ch, TYPE_UNDEFINED);
        return;
    }

if((!IS_PACIFIST(ch)) && !is_safe(ch, victim, TRUE)) {
	
		act( AT_MAGIC, "Pronuncias 'Mirada Aterradora.", ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, "$N pronuncia 'Dommon engelin'.", ch, NULL, ch, TO_ROOM );

		nivel = get_dnivel( 2002 );
	        if( lanzar_disciplina( ch, victim, nivel ) )
		{

        	if(aff > 0 ){ dam=10*aff; 
        	} else { aff=1;
        		 dam=10;
        	} 
        	
        	
        	dura= 150 + ch->generacion;
               	
		daf1.duracion = dura;
               	daf1.localizacion = AP_HITROLL;
               	daf1.bitvector = meb(DAF_MIRADA);
               	daf1.modificador = -dam;
               	daf1.disciplina = 20;
               	daf1.nivel = 2002;
               	disciplina_to_char( victim, &daf1);
               	
		daf2.duracion = dura;
               	daf2.localizacion = AP_DAMROLL;
               	daf2.bitvector = meb(DAF_MIRADA);
               	daf2.modificador = -dam;
               	daf2.disciplina = 20;
               	daf2.nivel = 2002;
               	disciplina_to_char( victim, &daf2);
		
		if (dam > 30) { dam = 30; }
		
		daf3.duracion = dura; 
		daf3.localizacion = AP_SAVING_BREATH;
		daf3.bitvector = meb(DAF_MIRADA);
		daf3.modificador = dam;
		daf3.disciplina = 20;
		daf3.nivel = 2002;
		disciplina_to_char( victim, &daf3);
		
		
		
		}//cierro el if lanzar disci   
}//cierro if is pacifist
     
return;
}




/*
 * Disciplina Auspex
 */

/* Nivel uno Sentidos Aguzados Afectado por Detect invis detect traps detect hide */

void do_sentidosaguzados( CHAR_DATA *ch, char *argument )
{
	NIVEL *nivel;
        DISCI_AFFECT daf;

        if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion / 3 )
        {
		send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
		return;
        }
        else
	        ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion / 3;

        if(argument[0]!='\0')
        {
                send_to_char( "No debes usar Auspex sobre otros.\n\r", ch );
                return;
        }

        if (xIS_SET(ch->afectado_por, DAF_SENTIDOS_AGUZADOS ) )
        {
        	act( AT_MAGIC, "Tus sentidos ya fueron aguzados por Auspex.", ch, NULL, NULL, TO_CHAR );
                return;
        }
	
	if(who_fighting( ch ) != NULL)
        {
                send_to_char( "Estas demasiado ocupado como para hacerlo.\n\r", ch);
                return;
        }

        act( AT_MAGIC, "$N pronuncia 'Shatorium deizmenum'.", ch, NULL, ch, TO_ROOM );
        act( AT_MAGIC, "Pronuncias 'Sentidos Aguzados'.", ch, NULL, NULL, TO_CHAR );

	nivel = get_nivel( "sentidos" );
        if ( lanzar_disciplina( ch, NULL, nivel ))
	{
                daf.localizacion = AP_NADA;
                daf.duracion = ch->level * 16 / ch->generacion;
                daf.disciplina = 2;
                daf.nivel = 201;
                daf.bitvector = meb(DAF_SENTIDOS_AGUZADOS);
                disciplina_to_char( ch, &daf );
	}
    


    if (!IS_IMMORTAL(ch))
    {
    	if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
    		espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 12);
    	else
    		WAIT_STATE(ch, 12);
    }

    return;
}

void do_visiondelalma( CHAR_DATA *ch, char* argumento )
{
    NIVEL *nivel = get_nivel("vision");
    AFFECT_DATA *paf;
    SKILLTYPE *skill;
    CHAR_DATA *victim;

	if (( victim = get_char_room( ch, argumento ) ) == NULL )
        {
                send_to_char( "No esta aqui.\n\r", ch );
                return;
        }

	if(who_fighting( ch ) != NULL)
        {
                send_to_char( "Estas demasiado ocupado como para hacerlo.\n\r", ch);
                return;
        }

   	if ( IS_NPC(victim) )
   	{
		send_to_char( "No puedes leer en las almas de los Mobs.\n\r", ch );
                return;
	}

        act( AT_MAGIC, "$N pronuncia 'Animas Id Seeyum'.", ch, NULL, ch, TO_ROOM );
        act( AT_MAGIC, "Pronuncias 'Vision del Alma'.", ch, NULL, NULL, TO_CHAR );
	
	if ( lanzar_disciplina( ch, victim, nivel ))
	{
	        if ( !victim->first_affect )
    		{
        		send_to_char_color( "\n\r&CNo esta afectado por nada.\n\r", ch );
    		}
    		else
    		{
        		for (paf = victim->first_affect; paf; paf = paf->next)
            			if ( (skill=get_skilltype(paf->type)) != NULL )
        			{
			        	set_char_color( AT_BLUE, ch );
            				send_to_char( "Esta afectado por:  ", ch );
            				set_char_color( AT_SCORE, ch );
            				if ( victim->level >= 10
            				||   IS_PKILL( victim ) )
                				ch_printf( ch, "(%3d horas)   ", paf->duration/100 );
            				ch_printf( ch, "%-18s\n\r", skill->name );
        			}
    		}
    	}
        return;
}



/* Potencia */
/* Niveles uno al cinco Potencia plus de hit dam y dam total */

int aff;
void do_potencia( CHAR_DATA *ch, char *argument )
{

    DISCI_AFFECT daf, daf2;
    NIVEL *nivel;

         if(!IS_VAMPIRE(ch))
        {
                send_to_char( "Mejor deja Potencia para los vampiros.\n\r", ch );
                return;
        }

           switch( ch->pcdata->habilidades[DISCIPLINA_POTENCIA] )
           {
                case 0: bug("%s puede usar Potencia con nivel 0 en la disciplina.", ch->name ); return;
                send_to_char("&gTu disciplina potencia esta a nivel 0 consulta con un inmortal.\n\r", ch );
                break;
                case 1: aff = 1;
                break;
                case 2: aff = 2;
                break;
                case 3: aff = 3;
                break;
                case 4: aff = 4;
                break;
                case 5: aff = 5;
		break;
		case 6: aff = 6;
		break;
		case 7: aff = 7;
		break;
		case 8: aff = 8;
		break;
		case 9: aff = 9;
		break;
		case 10: aff= 10;
		break; 
	}
	     if((ch->class!=CLASS_BAALI)||(ch->class!=CLASS_LASOMBRA)||(ch->class!=CLASS_NOSFERATU)||(ch->class!=CLASS_BRUJAH))
	             aff = (aff * (100 / ch->generacion)) + number_range(10,30);
	     else 
	     	aff= (aff* (75 / ch->generacion)) + number_range(10, 30);
	    
    if( ch )
    {

                if (xIS_SET(ch->afectado_por, DAF_POTENCIA))
                {
                      act( AT_MAGIC, "No puedes tener mas potencia que ahora.", ch, NULL, NULL, TO_CHAR );
                      return;
                }

                if ( who_fighting( ch ) != NULL)
                {
                        send_to_char( "No consigues concentrar tu mente... estas algo liado.\n\r", ch );
                        return;
                }

                act( AT_MAGIC, "Pronuncias 'Potencia'.", ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, "$n pronuncia 'Pakupkau'.", ch, NULL, NULL, TO_ROOM );
		
		nivel = get_nivel( "potencia" );
                if( lanzar_disciplina( ch, NULL, nivel ) )
                {
                daf.tipo = 1;
                daf.duracion = (7500  / ch->generacion);
                daf.localizacion = AP_DAMROLL;
                daf.bitvector = meb(DAF_POTENCIA);
                daf.modificador = aff;
                daf.disciplina = 21;
                daf.nivel = 2105;
                disciplina_to_char( ch, &daf );

                daf2.tipo = 1;
		daf2.duracion = (7500  / ch->generacion);
                daf2.localizacion = AP_HITROLL;
                daf2.bitvector = meb(DAF_POTENCIA);
                daf2.modificador = aff;
                daf2.disciplina = 21;
                daf2.nivel = 2105;
                disciplina_to_char( ch, &daf2 );
                }
    }
    return;
}

/*
 * Disciplina Taumaturgia
 * VampiroMUD2.0b
 */
void do_forma_incorporea( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *victim;
        int fallo = number_range( 1, 100 );
	NIVEL *nivel;
	DISCI_AFFECT daf;

        if (IS_NPC(ch) )
        {
                send_to_char( "Los mobs no pueden usar disciplinas.\n\r", ch );
                return;
        }

         if ( argument[0] == '\0' )
                argument = ch->name;

	if(xIS_SET(ch->afectado_por, DAF_INCORPOREO))
         {
                send_to_char( "Ya eres intangible como la niebla.\n\r",ch);
                return;
         }

	if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion * 2 )
        {
        	send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
        	return;
        }
        else
        	ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion * 2;

   	if( fallo >= 85 && fallo < 100)
   	{
        	send_to_char( "&BParece que no lo consigues.&x&w\n\r", ch);
        	return;
   	}

   	if ( ( victim = get_char_room( ch, argument ) ) == NULL )
    	{
    		send_to_char( "No esta aqui.\n\r", ch );
    		return;
    	}
        act( AT_MAGIC, "Pronuncias 'Forma Incorporea'.", ch, NULL, NULL, TO_CHAR );
        act( AT_MAGIC, "$n pronuncia 'Kirsalak kruisarke'.", ch, NULL, NULL, TO_ROOM );

    	nivel = get_nivel("forma");
    
	if( lanzar_disciplina( ch, NULL, nivel ) )
        {
                daf.tipo = 4;
                daf.duracion = (5000  / ch->generacion) + number_range(1,100);
                daf.localizacion = AP_NADA;
                daf.bitvector = meb(DAF_INCORPOREO);
                daf.disciplina = 29;
                daf.nivel = 2903;
                disciplina_to_char( ch, &daf );

	}
   	return;
}



/* Corazon Petreo nivel 4 */

CHAR_DATA *victim;
void do_petrificarse( CHAR_DATA *ch, char *argument )
{

        DISCI_AFFECT daf;
        NIVEL *nivel;
        char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch) )
        {
            if (!IS_IMMORTAL(ch) )
            {
                send_to_char( "Los mobs no pueden usar disciplinas.\n\r", ch );
                return;
             }
             else
             {
             send_to_char( "Usar disciplinas switcheado puede hacer que el mud caiga olvidalo.\n\r", ch );
             return;
             }
         }

           if ( xIS_SET(ch->afectado_por, DAF_CORAZON_PETREO ))
           {
                send_to_char( "Tu corazon ya es tan duro como la piedra.\n\r", ch );
                return;
           }

           if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion )
                     {
                      send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
                      return;
                     }
                     else
                     ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion;


                daf.tipo = 1;
                daf.localizacion = AP_NADA;
                daf.modificador = 0;
                daf.bitvector = meb(DAF_CORAZON_PETREO);
                daf.disciplina = DISCIPLINA_TAUMATURGIA;
                daf.nivel = 2904;
                daf.duracion = (ch->level * (13/ch->generacion));
                disciplina_to_char( ch, &daf );

	sprintf(buf,"Pronuncias la oracion 'Shaon man doen dagdasoma caint lab laba'.\r" );
	act( AT_MAGIC, buf, ch, NULL, victim, TO_CHAR );
	sprintf(buf,"$n pronuncia la oracion 'Shaon man doen dagdasoma caint lab laba'.\r" );
	act(AT_MAGIC, buf,ch,NULL,victim,TO_NOTVICT);

        nivel = get_nivel( "corazon" );
        lanzar_disciplina( ch, NULL, nivel );
return;
}

/*
 * Disciplina fortaleza  numero 11
 */

/* Nivel uno a cinco afectado por fortaleza aumento del hit temporal */

int aff;
void do_fortaleza( CHAR_DATA *ch, char *argument )
{
    DISCI_AFFECT daf, daf2;
    NIVEL *nivel;

    int vida;
    int dura;

    if(!IS_VAMPIRE(ch))
        {
                send_to_char( "Mejor deja Fortaleza para los vampiros.\n\r", ch );
                return;
        }

                  if (xIS_SET(ch->afectado_por, DAF_FORTALEZA ) )
                   {
                   	act( AT_MAGIC, "No puedes volver a conjurar Fortaleza sobre ti.", ch, NULL, NULL, TO_CHAR );
                        return;
                   }
                   else

          switch( ch->pcdata->habilidades[DISCIPLINA_FORTALEZA] )
           {
                case 0: bug("%s puede usar Fortaleza con nivel 0 en la disciplina.", ch->name );
                	send_to_char("Tu nivel de Fortaleza inexplikablemente es 0 habla con un implementador.\n\r", ch );
                	return;
                	break;
                case 1: aff = 1;
                	break;
                case 2: aff = 2;
                	break;
                case 3: aff = 3;
                	break;
                case 4: aff = 4;
                	break;
                case 5: aff = 5;
			break;
		case 6: aff = 6;
			break;
		case 7: aff = 7;
			break;
		case 8: aff = 8;
			break;
		case 9: aff = 9;
			break;
		case 10:aff = 10;
			break; 
		}

             act( AT_MAGIC, "Pronuncias 'Fortaleza'.", ch, NULL, NULL, TO_CHAR );
             act( AT_MAGIC, "$n pronuncia 'Kraisech'.", ch, NULL, ch, TO_ROOM );

             nivel = get_nivel( "fortaleza" );
             if( lanzar_disciplina( ch, NULL, nivel ) )
             {

            //if(ch->pcdata->habilidades[DISCIPLINA_FORTALEZA] >= 5)
            //aff = 5;
            if((ch->class!=CLASS_GANGREL)||(ch->class!=CLASS_RAVNOS)||(ch->class!=CLASS_VENTRUE))
           { 
            vida = aff * number_range(100, 300) + (3000/ ch->generacion);
            } else {
	    vida = ((aff * number_range(100 , 300) + (3000 / ch->generacion)*125)/100);
            }
        /* Primer affect */
        dura = number_range( 5, 15 );

        daf.tipo        = 4;
        daf.localizacion = AP_VIDA;
        daf.duracion = (300 * dura) / ch->generacion;
        daf.modificador = vida;
        daf.disciplina = 11;
        daf.nivel      = 1105;
        daf.bitvector = meb(DAF_FORTALEZA);
        disciplina_to_char( ch, &daf );

        /* Segundo affect */
        daf2.tipo        = 4;
        daf2.localizacion = AP_FUE;
        daf2.duracion = (300 * dura) /ch->generacion;
        daf2.modificador = aff;
        daf2.disciplina = 11;
        daf2.nivel      = 1105;
        daf2.bitvector   = meb(DAF_FORTALEZA);
        disciplina_to_char( ch, &daf2 );
        }

    return;
}

/*
 * Disciplina Extincion
 */

 /* Nivel uno Muerte silenciosa */
void do_silenciodelamuerte( CHAR_DATA *ch, char *argument )
{
      NIVEL *nivel;
      DISCI_AFFECT daf;


	if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion )
        {
                send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
                return;
        }
        else
                ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion;


      if ( (  who_fighting( ch ) ) != NULL )
      {
      		send_to_char( "&wNo puedes concentrarte en medio de un combate!\n\r", ch);
                return;
      }
	
	if(argument[0]!='\0')
        {
                send_to_char( "No debes usarlo sobre otros.\n\r",ch );
                return;
        }

      if (xIS_SET(ch->afectado_por, DAF_SILENCIO_MORTAL))
      {
      	 	act( AT_MAGIC, "El silencio ya te rodea.", ch, NULL, NULL, TO_CHAR );
                return;
      }

      act( AT_MAGIC, "$N pronuncia 'Shika teime'.", ch, NULL, ch, TO_ROOM );
      act( AT_MAGIC, "Pronuncias 'Silencio Mortal'.", ch, NULL, NULL, TO_CHAR );

      nivel = get_dnivel( 901 );
      if( lanzar_disciplina( ch, NULL, nivel ))
      {
              daf.localizacion = AP_NADA;
              daf.duracion = ch->level * 16 / ch->generacion;
              daf.disciplina = 9;
              daf.nivel = 901;
              daf.bitvector = meb(DAF_SILENCIO_MORTAL);
              disciplina_to_char( ch, &daf );

              if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
   	           espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 8);
              else
                   WAIT_STATE(ch, 8);
      }
        
     

    return;
}

void do_debilidad( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *victim;
        DISCI_AFFECT daf;
        NIVEL *nivel;
        sh_int danyo = 1;

        if(!IS_VAMPIRE(ch))
        {
                send_to_char( "Mejor deja Extincion para los Assamitas.\n\r", ch );
                return;
        }

    if(( argument[0]=='\0'
    && !ch->fighting ))
    {
        send_to_char( "&wA quien quieres debilitar?.\n\r", ch);
        return;
    }

    if  (argument[0]=='\0' && (victim=who_fighting(ch)) != NULL )
        argument = victim->name;

    if (( victim = get_char_room( ch, argument ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if(victim == ch)
    {
    send_to_char( "A ti mismo? no creo que te gustara eso...\n\r", ch);
    return;
    }


         if( !is_safe(ch, victim, TRUE) )
         {

	  act( AT_MAGIC, "Pronuncias 'Debilidad'.", ch, NULL, NULL, TO_CHAR );
          act( AT_MAGIC, "$N pronuncia 'Deiteris shaoktarum'.", ch, NULL, ch, TO_NOTVICT );


          nivel = get_nivel( "debilidad" );
          if( lanzar_disciplina( ch, victim, nivel ))
          {
                if(!xIS_SET(victim->afectado_por, DAF_DEBILIDAD))
                {
                act( AT_MAGIC, "Infectas a $N!",victim, NULL, victim, TO_CHAR );
                act( AT_MAGIC, "$N te ha infectado!. Te sietes menos resistente.", ch, NULL, ch, TO_VICT );
                act( AT_MAGIC, "$N ha quedado infectado!", victim, NULL, victim, TO_NOTVICT );
                daf.tipo = TIPO_NINGUNO;
                daf.localizacion = AP_NADA;
                daf.modificador = 0;
                daf.disciplina = 9;
                daf.nivel = 902;
                daf.duracion = (ch->level + (13 /ch->generacion ));
                daf.bitvector = meb(DAF_DEBILIDAD);
                disciplina_to_char( victim, &daf );
                }
                danyo = number_range((ch->max_hit / 25 * ch->generacion)
		+ ch->pcdata->habilidades[DISCIPLINA_EXTINCION],
		(ch->max_hit / 5 * ch->generacion));
                
		if (ch->class != CLASS_VAMPIRE)
	 	        danyo -= danyo * 0.3;

                        damage(ch, victim, danyo, 19);
          }
         }
return;
}
/* Extincion nivel 3: Enfermedad */
/* Kayser 2004 */
void do_enfermedad( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *victim;
        DISCI_AFFECT daf;
        NIVEL *nivel;
        sh_int danyo = 1;


    if(( argument[0]=='\0'
    && !ch->fighting ))
    {
        send_to_char( "&wA quien quieres enfermar?.\n\r", ch);
        return;
    }

    if  (argument[0]=='\0' && (victim=who_fighting(ch)) != NULL )
        argument = victim->name;

    if (( victim = get_char_room( ch, argument ) ) == NULL )
    {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
    }

    if(victim == ch)
    {
    	send_to_char( "A ti mismo? no creo que te gustara eso...\n\r", ch);
    	return;
    }


         if( !is_safe(ch, victim, TRUE) )
         {

          act( AT_MAGIC, "Pronuncias 'Enfermedad'.", ch, NULL, NULL, TO_CHAR );
          act( AT_MAGIC, "$N pronuncia 'Poitonus Kreianyo'.", ch, NULL, ch, TO_NOTVICT );

          nivel = get_nivel( "enfermedad" );
          if( lanzar_disciplina( ch, victim, nivel ))
          {
                if(!xIS_SET(victim->afectado_por, DAF_ENFERMEDAD))
                {
                act( AT_MAGIC, "Infectas a $N!",victim, NULL, victim, TO_CHAR );
                act( AT_MAGIC, "$N te ha infectado!. Te sietes menos resistente.", ch, NULL, ch, TO_VICT );
                act( AT_MAGIC, "$N ha quedado infectado!", victim, NULL, victim, TO_NOTVICT );
                daf.tipo = TIPO_NINGUNO;
                daf.localizacion = AP_FUE;
                daf.modificador = 0;
                daf.disciplina = 9;
                daf.nivel = 903;
                daf.duracion = (ch->level/2 + (13 /ch->generacion ));
                daf.bitvector = meb(DAF_ENFERMEDAD);
                disciplina_to_char( victim, &daf );
                }
                danyo = number_range((ch->max_hit / 25 * ch->generacion),
                ((ch->max_hit * ch->pcdata->habilidades[DISCIPLINA_EXTINCION]) / 25 * ch->generacion));

                if (ch->class != CLASS_VAMPIRE)
                        danyo -= danyo * 0.5;

                        damage(ch, victim, danyo, 19);
          }
         }
return;
}




/* Extincion nivel 4: Tormento de la Sangre */
/* Nkari 2004 */
void do_tormento( CHAR_DATA *ch, char *argument )
{
        CHAR_DATA *victim;
        NIVEL *nivel;
        int fallo;
        int enf;
        sh_int dam = 1;

    if ( (victim=who_fighting(ch)) != NULL )
    {
     argument = victim->name;
    }
 
    if( argument[0]=='\0' )
    {
        send_to_char( "&wA quien quieres atormentar?.\n\r", ch);
        return;
    }
	
    if (( victim = get_char_room( ch, argument ) ) == NULL )
    {
        send_to_char( "No esta aqui.\n\r", ch );
        return;
    }	
    
    if(victim == ch)
    {
    send_to_char( "A ti mismo? no creo que te gustara eso...\n\r", ch);
    return;
    }

    fallo=number_range(1 ,100);
    if ( fallo >= 85 && fallo <= 100)
    {
    send_to_char( " No consigues atormentarle!.\n\r", ch );
    multi_hit(victim, ch, TYPE_UNDEFINED);
    return;
    }

    if((!IS_PACIFIST(ch)) && !is_safe(ch, victim, TRUE)) {

 	act( AT_MAGIC, "Pronuncias 'Tormento de la Sangre'.", ch, NULL, NULL, TO_CHAR );
        act( AT_MAGIC, "$N pronuncia 'Volaeriom euk xoktabr'.", ch, NULL, ch, TO_NOTVICT );


    	nivel = get_nivel( "tormento" );
          if( lanzar_disciplina( ch, victim, nivel ))
          {

                if ( ch->level < 30 ) {
           	     enf = UMAX( 10, ch->pcdata->condition[COND_BLOODTHIRST] * (get_curr_lck(ch) ) + ch->hitroll + 10*ch->level*(13/ch->generacion) );
        	}else{
                	enf = UMAX( 10, ch->pcdata->condition[COND_BLOODTHIRST] * ch->pcdata->habilidades[DISCIPLINA_EXTINCION]  * (get_curr_lck(ch) ) + ch->hitroll + 10*ch->level*(13/ch->generacion) ); }
        	dam  = number_range( enf/10, enf/15 );


        if (IS_VAMPIRE(victim)) { dam= dam*110/150; }
        if (IS_NPC(victim)) { dam = dam*110/100; }
        damage( ch, victim, dam, 19 );
                   
          }
         }
	return;
}

/*
 * Disciplina Protean
 */

 /*
  * Nivel 1 testigo de las tinieblas permite ver en la oscuridad extrema
  */
void do_testigodelastinieblas( CHAR_DATA *ch, char *argument )
{
        NIVEL *nivel;
	DISCI_AFFECT daf;
        
	if ( ch->pcdata->condition[COND_BLOODTHIRST] <= ch->generacion )
        {
	        send_to_char( "No tienes suficiente sangre usa tu disciplina comun para alimentarte.\n\r", ch );
        	return;
        }
        else
	        ch->pcdata->condition[COND_BLOODTHIRST] -= ch->generacion;

        if(who_fighting( ch ) != NULL)
        {
        	send_to_char( "&wNo puedes concentrarte en medio de un combate!\n\r", ch);
                return;
        }

        if (xIS_SET(ch->afectado_por, DAF_TESTIGO ))
        {
        	act( AT_MAGIC, "La Bestia ya se manifiesta en tu vision.", ch, NULL, NULL, TO_CHAR );
                return;
        }

        
	if(argument[0]=='\0')
      	{
		act( AT_MAGIC, "$N pronuncia 'Tenebrum shiej'.", ch, NULL, ch, TO_ROOM );
                act( AT_MAGIC, "Pronuncias 'Testigo de las Tinieblas'.", ch, NULL, NULL, TO_CHAR );


		nivel = get_nivel( "testigo" );
	        if( lanzar_disciplina( ch, NULL, nivel ))
          	{
			daf.localizacion = AP_NADA;
        	        daf.duracion = ch->level * 16 / ch->generacion;
                	daf.disciplina = 22;
                	daf.nivel = 2201;
                	daf.bitvector = meb(DAF_TESTIGO);
                	disciplina_to_char( ch, &daf );

		    	if(xIS_SET(ch->afectado_por, DAF_CELERIDAD))
    				espera_menos(ch, ch->pcdata->habilidades[DISCIPLINA_CELERIDAD], 8);
			else
			WAIT_STATE(ch, 8);
        	 }
	}
        else
      		send_to_char( "No debes utilizar Testigo de las Tinieblas en otros.\n\r", ch );

    return;
}

/*
 * Nivel 2 Garras de la Bestia produce danyos agravados xD
 */

void do_garrasdelabestia( CHAR_DATA *ch, char *argument )
{
        DISCI_AFFECT daf;
	NIVEL *nivel;
        OBJ_DATA *obj;

        if ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) != NULL )
	{
           send_to_char( "No puedes blandir un arma si usas las Garras de la Bestia.\n\r", ch );
           return;
        }

        if( xIS_SET(ch->afectado_por, DAF_GARRAS_BESTIA))
        {
                send_to_char( "Tus garras ya poseen el poder de la Bestia.\n\r", ch );
                return;
        }

        if(argument[0]=='\0')
        {
		act( AT_MAGIC, "$N ruje de dolor cuando sus dedos cambian adoptando el aspecto de unas grandes garras.", ch, NULL, ch, TO_ROOM );
        	act( AT_MAGIC, "Las garras de $N brillan con una luz tenue.", ch, NULL, ch, TO_ROOM );

		nivel = get_nivel( "garras" );
                if( lanzar_disciplina( ch, NULL, nivel ))
                {
                        daf.localizacion = AP_NADA;
                        daf.duracion = ch->level * 10 / ch->generacion;
                        daf.disciplina = 22;
                        daf.nivel = 2202;
                        daf.bitvector = meb(DAF_GARRAS_BESTIA);
                        disciplina_to_char( ch, &daf );

		}
	}
	else
      		send_to_char( "Las garras de la bestia se utiliza sin argumento.\n\r", ch );
	
	return;
}


void disciplina_to_char( CHAR_DATA *ch, DISCI_AFFECT *daf )
{
        DISCI_AFFECT *daf_new;

        if(!ch)
        {
                bug( "disciplina_to_char(NULO, %d)", daf ? daf->tipo : 0);
                return;
        }

        if(!daf)
        {
                bug( "disciplina_to_char(%s, NULO)", ch->name );
                return;
        }

        CREATE( daf_new, DISCI_AFFECT, 1);
        LINK( daf_new, ch->primer_affect, ch->ultimo_affect, next, prev );
        daf_new->duracion	= daf->duracion;
        daf_new->localizacion	= daf->localizacion;
        daf_new->modificador	= daf->modificador;
        daf_new->bitvector	= daf->bitvector;
        daf_new->nivel          = daf->nivel;

        affect_disci_modificar( ch, daf_new, TRUE );
        return;
}

void disciplina_remove( CHAR_DATA *ch, DISCI_AFFECT *daf )
{
        if( !ch->primer_affect )
        {
                bug( "disciplina_remove(%s, %d): no affect de disciplina.", ch->name,
                        daf ? daf->tipo : 0 );
                return;
        }

        affect_disci_modificar( ch, daf, FALSE );

        UNLINK( daf, ch->primer_affect, ch->ultimo_affect ,next, prev );
        DISPOSE( daf );
        return;
}

void do_concentrar( CHAR_DATA *ch, char *argument )
{
        NIVEL *nivel;
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        int fallo;

        if( !ch )
        {
                bug( "Funcion Concentrar: ch NULO!", 0 );
                return;
        }

        if( !IS_VAMPIRE(ch) && !IS_IMMORTAL(ch))
        {
                send_to_char( "Solo los vampiros pueden concentrar su sangre.\n\r", ch );
                return;
        }

        set_char_color( AT_GREY, ch );
        if( IS_NPC( ch ))
        {
                send_to_char( "Si hombre pero tu que te has pensao.\n\n", ch );
                return;
        }

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );

        if( arg1[0] == '\0' )
        {
                send_to_char( "Concentrar el poder de tu sangre para hacer que?\n\r", ch );
                return;
        }


        if( arg1[0] !='\0' )
        nivel = get_nivel( arg1 );


        if( !nivel )
        {

                send_to_char( "No conoces ningun nivel de disciplina asi.\n\r", ch );
                return;
        }

        if(nivel->disciplina > 0)
        {
          if(IS_NPC(ch))
          {
                send_to_char( "Los MOB's no pueden usar disciplinas.\n\r", ch);
                return;
          }
            if(ch->pcdata->habilidades[nivel->disciplina] < nivel->nivel && !IS_IMMORTAL( ch ) )
            {
                ch_printf(ch, "&wNo conoces ese nivel de &g%s&w.\n\rAyuda '&g%s&w' para saber mas sobre ello.\n\r",
                capitalize(disciplina[nivel->disciplina]),
                disciplina[nivel->disciplina]);
                return;
            }
        }
   /* Control del Embrujo por Nkari */
    if(xIS_SET(ch->afectado_por, DAF_EMBRUJADO))
    {
	fallo=number_range(1 , 100);
	if(fallo >= 95 && fallo <= 100){
	   send_to_char("&rTu embrujo te distrae y no consigues concentrar el poder de tu sangre&w\n\r.",ch);
	   return;
        }
    }  
 interprete( ch, nivel, arg2 );
 return;
}

void interprete( CHAR_DATA *ch, NIVEL *nivel, char *argument )
{
        if(nivel->disciplina > 0)
        {
          if(IS_NPC(ch))
          {
                send_to_char( "Los MOB's no pueden usar disciplinas.\n\r", ch);
                return;
          }
            if(ch->pcdata->habilidades[nivel->disciplina] < nivel->nivel && !IS_IMMORTAL( ch ) )
            {
                ch_printf(ch, "&wNo conoces ese nivel de &g%s&w.\n\rAyuda '&g%s&w' para saber mas sobre ello.\n\r",
                capitalize(disciplina[nivel->disciplina]),
                disciplina[nivel->disciplina]);
                return;
            }
        }

        (*nivel->do_fun) (ch, argument );
        return;
}

void apunyalamiento( CHAR_DATA *ch, CHAR_DATA *victim, int potencia )
{
        int danyo = 0;
        if( !ch )
        {
                bug( "Apunyalamiento: ch NULO!" );
                return;
        }

        if( !victim )
        {
                bug( "apunyalamiento: victim NULO!" );
                return;
        }

                act( AT_BLUE, "Sacas una daga camuflada de tu cinturon y apunyalas salvajemente a $N!", ch, NULL, victim, TO_CHAR );
                act( AT_BLUE, "$n saca una daga camuflada de su cinturon y apunyala salvajemente a $N!", ch, NULL, victim, TO_NOTVICT );
                act( AT_BLUE, "$n sacas una daga camuflada de su cinturon y te cose a punyaladas!", ch, NULL, ch, TO_VICT );
                
        if( IS_NPC(victim))
        {

                danyo = potencia;
                damage( ch, victim, danyo, 21 );
        }
        else
        {
                danyo = potencia * 0.5;
                damage( ch, victim, danyo, 21 );
        }

        return;
}

