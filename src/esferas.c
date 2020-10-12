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
 *                  Modulo de Esferas para Magos y Clerigos                 *
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


ESFERA * primera_esfera;
ESFERA * ultima_esfera;

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
 * Coge un puntero de la estructura de esferas desde un nombre de esfera dado
 */
ESFERA *get_esfera( char *nombre )
{
        ESFERA *esfera;

        for ( esfera = primera_esfera; esfera ; esfera = esfera->next )
                if( !str_prefix( nombre, esfera->nombre ))
                        return esfera;
                return NULL;
}

/*
 * Coge un puntero de la estructura de esferas desde un numero de nivel dado
 */
ESFERA *get_enivel( sh_int dn )
{
        ESFERA *esfera;

        for ( esfera = primera_esfera; esfera ; esfera = esfera->next )
                if( esfera->enivel == dn )
                        return esfera;
                return NULL;
}


char *   const  esferas [MAX_ESFERAS] =
{
        "cardinal", "correspondencia", "entropia", "espiritu", "fuerzas",
        "materia", "mente", "tiempo", "vida"
};

char *   const  esf_affect [] =
{
        "tiempo"
};

/*
 * Funcion que escribe la lista de esferas
 */
void write_esfera_list( )
{
        ESFERA *esfera;
        FILE *fpout;
        char archivo[256];

        sprintf( archivo, "%s%s", ESFERA_DIR, ESFERA_LIST );

        fpout = fopen( archivo, "w" );
        if( !fpout )
        {
                bug( "NO se ha podido abrir esfera.lst para escribir", archivo );
                return;
        }
        for( esfera = primera_esfera; esfera ; esfera = esfera->next )
                fprintf( fpout, "%s\n", esfera->fichero );
         fprintf( fpout, "$\n" );
         fclose( fpout );
}

/*
 * Guarda los datos de esfera_table en su archivo de datos
 */
void save_esfera( ESFERA *esfera )
{
        FILE *fp;
        char archivon[256];
        char buf[MAX_INPUT_LENGTH];

        if( !esfera )
        {
                bug( "Save_esfera: puntero de esfera nulo!", 0 );
                return;
        }

        if( !esfera->fichero || esfera->fichero[0] == '\0' )
        {
                sprintf( buf, "save_esfera: %s no tiene nombre de fichero", esfera->nombre );
                bug( buf, 0 );
                return;
        }

        sprintf( archivon, "%s%s", ESFERA_DIR, esfera->fichero );

        fclose( fpReserve );
        if( ( fp = fopen( archivon, "w" ) ) == NULL)
        {
                bug( "Save_esfera : fopen", 0 );
                perror( archivon );
        }
        else
        {
                fprintf( fp, "#ESFERA\n" );
                fprintf( fp, "Nombre            %s~\n",         esfera->nombre );
                fprintf( fp, "Tipo              %d\n",          esfera->tipo );
                fprintf( fp, "Funcion           %s\n",          skill_name(esfera->do_fun));
                fprintf( fp, "Descripcion       %s~\n",         esfera->descripcion );
                fprintf( fp, "Filename          %s~\n",         esfera->fichero );
                fprintf( fp, "Cabala            %s~\n",         esfera->cabala );
                fprintf( fp, "Esfera            %d\n",          esfera->esfera );
                fprintf( fp, "Enivel            %d\n",          esfera->enivel );
                fprintf( fp, "Wait              %d\n",          esfera->wait );
                fprintf( fp, "Mana              %d\n",          esfera->mana );
                fprintf( fp, "Dammes            %s~\n",         esfera->dammes );
                fprintf( fp, "GoMes             %s~\n",         esfera->sevames );
                fprintf( fp, "ExitoCh           %s~\n",         esfera->exitoch );
                fprintf( fp, "ExitoVict         %s~\n",         esfera->exitovict );
                fprintf( fp, "ExitoRoom         %s~\n",         esfera->exitoroom );
                fprintf( fp, "ExitoMud          %s~\n",         esfera->exitomud );
                fprintf( fp, "FalloCh           %s~\n",         esfera->falloch );
                fprintf( fp, "FalloVict         %s~\n",         esfera->fallovict );
                fprintf( fp, "FalloRoom         %s~\n",         esfera->falloroom );
                fprintf( fp, "FalloMud          %s~\n",         esfera->fallomud );
                fprintf( fp, "MuerteCh          %s~\n",         esfera->muertech );
                fprintf( fp, "MuerteVict        %s~\n",         esfera->muertevict );
                fprintf( fp, "MuerteRoom        %s~\n",         esfera->muerteroom );
                fprintf( fp, "MuerteMud         %s~\n",         esfera->muertemud );
                fprintf( fp, "InmuneCh          %s~\n",         esfera->inmunech );
                fprintf( fp, "InmuneVict        %s~\n",         esfera->inmunevict );
                fprintf( fp, "InmuneRoom        %s~\n",         esfera->inmuneroom );
                fprintf( fp, "InmuneMud         %s~\n",         esfera->inmunemud );

                fprintf( fp, "End\n\n" );

                fprintf( fp, "#END\n" );
        }
        fclose( fp );
        fpReserve = fopen( NULL_FILE, "r" );
        return;
}
void fread_esfera( ESFERA *esfera, FILE *fp )
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
                        if( !str_cmp( word, "Cabala"))
                        {
                                fMatch = TRUE;
                                esfera->cabala = fread_string( fp );
                                break;
                        }

                case 'D':
                        KEY( "Descripcion",     esfera->descripcion,        fread_string( fp ));
                        KEY( "Dammes",          esfera->dammes,             fread_string( fp ));
                        break;
                /*
                 * Case 'E' bajado al final del todo
                 */

                case 'F':
                        KEY( "Filename",        esfera->fichero,            fread_string( fp ));
                        KEY( "FalloCh",         esfera->falloch,            fread_string( fp ));
                        KEY( "FalloVict",       esfera->fallovict,          fread_string( fp ));
                        KEY( "FalloRoom",       esfera->falloroom,          fread_string( fp ));
                        KEY( "FalloMud",        esfera->fallomud,           fread_string( fp ));
                        KEY( "Funcion",         esfera->do_fun,             skill_function(fread_word( fp )) );
                        break;

                case 'G':
                        KEY( "GoMes",           esfera->sevames,            fread_string( fp ));
                        break;

                case 'I':
                        KEY( "InmuneCh",        esfera->inmunech,           fread_string( fp ));
                        KEY( "InmuneVict",      esfera->inmunevict,         fread_string( fp ));
                        KEY( "InmuneRoom",      esfera->inmuneroom,         fread_string( fp ));
                        KEY( "InmuneMud",       esfera->inmunemud,          fread_string( fp ));
                        break;

                case 'M':
                        KEY( "Mana",            esfera->mana,               fread_number( fp ));
                        KEY( "MuerteCh",        esfera->muertech,           fread_string( fp ));
                        KEY( "MuerteVict",      esfera->muertevict,         fread_string( fp ));
                        KEY( "MuerteRoom",      esfera->muerteroom,         fread_string( fp ));
                        KEY( "MuerteMud",       esfera->muertemud,          fread_string( fp ));
                        break;

                case 'N':
                        KEY( "Nombre",          esfera->nombre,             fread_string( fp ));

                case 'T':
                        KEY( "Tipo",            esfera->tipo,               fread_number( fp ));
                        break;

                case 'W':
                        KEY( "Wait",            esfera->wait,               fread_number( fp ));
                        break;

                case 'E':
                        KEY( "Enivel",            esfera->enivel,           fread_number( fp ));
                        KEY( "Esfera",            esfera->esfera,           fread_number( fp ));
                        KEY( "ExitoCh",           esfera->exitoch,          fread_string( fp ));
                        KEY( "ExitoVict",         esfera->exitovict,        fread_string( fp ));
                        KEY( "ExitoRoom",         esfera->exitoroom,        fread_string( fp ));
                        KEY( "ExitoMud",          esfera->exitomud,         fread_string( fp ));
                        if( !str_cmp( word, "End" ) )
                        {
                                if( !esfera->nombre)
                                esfera->nombre           = STRALLOC( "" );
                                if( !esfera->cabala)
                                esfera->cabala           = STRALLOC( "" );
                                if( !esfera->descripcion)
                                esfera->descripcion           = STRALLOC( "No fijada aun" );
                                return;
                        }
                        break;


               }

               if( !fMatch )
               {
                        sprintf( buf, "Fread_esfera: no se ha encontrado: %s", word );
                        bug( buf, 0);
               }
         }
}

bool load_esfera_file( char *archivo )
{
        char filenamen[256];
        ESFERA *esfera;
        FILE *fp;
        bool found;

        CREATE( esfera, ESFERA, 1 );

        found = FALSE;
        sprintf( filenamen, "%s%s", ESFERA_DIR, archivo );

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
                        bug( "Load_esfera_file: # No encontrado.", 0 );
                        break;
                }

                word = fread_word( fp );

                if( !str_cmp( word, "ESFERA"     ))
                {
                        fread_esfera( esfera, fp );
                        break;
                }
                else
                if( !str_cmp( word, "END" ) )
                        break;
                else
                {
                        bug( "Load_esfera_file: Seccion erronea.", 0 );
                        break;
                }

              }
                fclose( fp );
            }

       if( found )
           LINK( esfera, primera_esfera, ultima_esfera, next, prev );

       else
         DISPOSE( esfera );

       return found;
}

void load_esfera( )
{
        FILE *fpList;
        char *filename;
        char nlist[256];
        char buf[256];

        primera_esfera = NULL;
        ultima_esfera = NULL;

        log_string( "Cargando Esferas..." );

        sprintf( nlist, "%s%s", ESFERA_DIR, ESFERA_LIST );
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



           if( !load_esfera_file( filename ) )
           {
                sprintf( buf, "No se pudo abrir archivo: %s.", filename );
                bug( buf, 0 );
           }

         }
         fclose( fpList );
         log_string( " Esferas Cargadas " );
         fpReserve = fopen( NULL_FILE, "r" );
         return;
}

/*
 * Esta funcion crea una nueva esfera que despues modificaremos con eset
 */
void do_crearesfera( CHAR_DATA *ch, char *argument )
{
        char filename[256];
        char argumento[256];
        ESFERA *esfera;
        bool found;

        set_char_color( AT_DGREEN, ch );

        if( !argument || argument[0] == '\0' )
        {
                send_to_char( "Uso: crearesfera <nombre esfera>.\n\r", ch );
                return;
        }

        found = FALSE;
        sprintf( argumento, "%s.esf", argument );
        sprintf( filename, "%s%s", ESFERA_DIR, strlower(argument) );

        CREATE( esfera, ESFERA, 1 );
        LINK( esfera, primera_esfera, ultima_esfera, next, prev );

        esfera->tipo                = TIPO_NINGUNO;
        esfera->nombre              = STRALLOC( argument );
        esfera->do_fun              = STRALLOC( "Ninguno" );
        esfera->fichero             = STRALLOC( argumento );
        esfera->descripcion         = STRALLOC( "Descripcion no fijada" );
        esfera->cabala              = STRALLOC( "Ninguna" );
        esfera->esfera                    = 1;
        esfera->wait                     = 1;
        esfera->dammes                   = STRALLOC( "" );
        esfera->sevames                  = STRALLOC( "" );
        esfera->exitoch                  = STRALLOC( "" );
        esfera->exitovict                = STRALLOC( "" );
        esfera->exitoroom                = STRALLOC( "" );
        esfera->exitomud                 = STRALLOC( "" );
        esfera->falloch                  = STRALLOC( "" );
        esfera->fallovict                = STRALLOC( "" );
        esfera->falloroom                = STRALLOC( "" );
        esfera->fallomud                 = STRALLOC( "" );
        esfera->muertech                 = STRALLOC( "" );
        esfera->muertevict               = STRALLOC( "" );
        esfera->muerteroom               = STRALLOC( "" );
        esfera->muertemud                = STRALLOC( "" );
        esfera->inmunech                = STRALLOC( "" );
        esfera->inmunevict              = STRALLOC( "" );
        esfera->inmuneroom              = STRALLOC( "" );
        esfera->inmunemud               = STRALLOC( "" );

        send_to_char( "Creada.\n\r", ch );
        return;
}

/*
 * Con esto se modifican los valores de las diferentes esferas
 */
int x;
void do_eset( CHAR_DATA *ch, char *argument )
{
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];

        ESFERA *esfera;

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
                send_to_char( "Uso : eset <esfera> <campo> <valor>.\n\r", ch );
                send_to_char( "Estos son los campos posibles:&c nombre, esfera, \n\r"
                              "descripcion, cabala, wait, dammes, gomes, exitoch, exitovict, \n\r"
                              "exitoroom, exitomud, falloch, fallovict, falloroom, fallomud,\n\r"
                              "muertech, muertevict, muerteroom, muertemud,inmunech, inmunevict,\n\r"
                              "inmuneroom, inmunemud, mana, tipo, funcion&w.\n\r", ch );
                send_to_char( "Guarda para guardar la lista de esfera, usar solo si se ha creado una nuevo.\n\r", ch );
                return;
         }

         if( !str_cmp( arg1, "guarda" ) )
         {
             write_esfera_list( );
             send_to_char( "Hecho.\n\r", ch );
             return;
         }

        if( !str_prefix( arg1, "lista" ) )
        {
             if( esfera == NULL )
             {
              send_to_char( "Aun no se ha creado ninguna esfera.\n\r", ch );
              return;
             }
             else
              ch_printf( ch, "\n\r&cEsferas creadas.\n\r" );

             for( esfera = primera_esfera; esfera; esfera = esfera->next )
              ch_printf( ch, "&c%s\n\r", esfera->nombre );

             return;
        }
        esfera = get_esfera( arg1 );

        if( !esfera )
        {
                send_to_char( "Esa esfera no existe ( borracho de mierda ).\n\r", ch );
                return;
        }

        if( arg2[0] == '\0' )
        {
                send_to_char( "Cambiar el que de esta esfera?.\n\r", ch );
                return;
        }

        if( !str_prefix( arg2, "nombre") )
        {
                if( is_number( argument ) )
                {
                        send_to_char( "El argumento no puede ser numerico.\n\r", ch );
                        return;
                }

                esfera->nombre = STRALLOC( argument );
                save_esfera( esfera );
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
                        send_to_char( "Eso no es un tipo de esfera aceptado.\n\r", ch );
                        return;
                }
                else
                {
                        esfera->tipo = atoi(argument);
                        ch_printf( ch, "Cambiando tipo a tipo &c%s&w.\n\r", disci_tnombre[atoi(argument)] );
                        save_esfera( esfera );
                        return;
                }
        }

        if( !str_prefix( arg2, "esfera" ) )
        {

                if( !is_number( argument ) )
                {
                                send_to_char( "El valor debe de ser numerico.\n\r", ch );
                                return;
                }
                else
                {
                        if( atoi(argument) < 0 || atoi(argument) > MAX_ESFERAS )
                        {
                                send_to_char( "No existe esa esfera.\n\r", ch );
                                return;
                        }
                        else
                        {
                                esfera->esfera = atoi(argument);
                                ch_printf( ch, "Cambiando esfera a &c%s&w.\n\r", esferas[atoi(argument)] );
                                save_esfera( esfera );
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
                        esfera->descripcion = STRALLOC( argument );
                        ch_printf( ch, "Cambiando descripcion a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
                        return;
                }
        }

        if( !str_prefix( arg2, "cabala" ) )
        {
                if( is_number( argument ) )
                {
                        send_to_char( "El valor no puede ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        esfera->cabala = STRALLOC( argument );
                        ch_printf( ch, "Cambiando cabala a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->wait = atoi(argument);
                        ch_printf( ch, "Cambiando wait a &c%d&w.\n\r", atoi( argument ));
                        save_esfera( esfera );
                        return;
                }
         }

        if( !str_prefix( arg2, "mana" ) )
        {
                if(  !is_number( argument ) )
                {
                        send_to_char( "El valor debe de ser numerico.\n\r", ch );
                        return;
                }
                else
                {
                        esfera->mana = atoi( argument );
                        ch_printf( ch, "Cambiando mana requerido a &c%d&w.\n\r", atoi( argument ));
                        save_esfera( esfera );
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
                        esfera->do_fun = fun;
                        ch_printf( ch, "Cambiando funcion del esfera a %s.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->dammes = STRALLOC( argument );
                        ch_printf( ch, "Cambiando mensaje de danyo a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->sevames = STRALLOC( argument );
                        ch_printf( ch, "Cambiando mensaje de remove a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->exitoch = STRALLOC( argument );
                        ch_printf( ch, "Cambiando Exitoch a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->exitovict = STRALLOC( argument );
                        ch_printf( ch, "Cambiando Exitovict a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->exitoroom = STRALLOC( argument );
                        ch_printf( ch, "Cambiando Exitoroom a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->exitomud = STRALLOC( argument );
                        ch_printf( ch, "Cambiando Exitomud a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->falloch = STRALLOC( argument );
                        ch_printf( ch, "Cambiando falloch a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->fallovict = STRALLOC( argument );
                        ch_printf( ch, "Cambiando fallovict a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->falloroom = STRALLOC( argument );
                        ch_printf( ch, "Cambiando falloroom a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->fallomud = STRALLOC( argument );
                        ch_printf( ch, "Cambiando fallomud a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->inmunech = STRALLOC( argument );
                        ch_printf( ch, "Cambiando inmunech a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->inmunevict = STRALLOC( argument );
                        ch_printf( ch, "Cambiando inmunevict a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->inmuneroom = STRALLOC( argument );
                        ch_printf( ch, "Cambiando inmuneroom a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->inmunemud = STRALLOC( argument );
                        ch_printf( ch, "Cambiando inmunemud a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->muertech = STRALLOC( argument );
                        ch_printf( ch, "Cambiando muertech a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->muertevict = STRALLOC( argument );
                        ch_printf( ch, "Cambiando muertevict a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->muerteroom = STRALLOC( argument );
                        ch_printf( ch, "Cambiando muerteroom a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
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
                        esfera->muertemud = STRALLOC( argument );
                        ch_printf( ch, "Cambiando muertemud a &c%s&w.\n\r", argument );
                        save_esfera( esfera );
                        return;
                }
         }


         send_to_char( "No se de que me hablas.\n\r", ch );
         return;
}

/*
 * Esto nos sirve para ver los valores de los esferaes
 */
void do_ever( CHAR_DATA *ch, char *argument )
{
        ESFERA *esfera;

        if( IS_NPC(ch))
        {
                send_to_char( "Mande?.\n\r", ch );
                return;
        }


        if( argument[0] == '\0' )
        {
                send_to_char( "Uso: ever <esfera>.\n\r", ch );
                return;
        }

        esfera = get_esfera( argument );
        if( !esfera )
        {
                send_to_char( "Eso no es ningun esfera que haya creado.\n\r", ch );
                return;
        }

        if( get_trust( ch ) > LEVEL_CREATOR )
        ch_printf( ch, "\n\r&cFilename(&w%s&c)\n\r", esfera->fichero );

        ch_printf( ch, "\n\r&cNombre          : &w%-20.20s&c Wait : &w%2.2d\n\r", esfera->nombre, esfera->wait );
        ch_printf( ch, "&cDescripcion     : &w%s&w.\n\r", esfera->descripcion );
        ch_printf( ch, "&cEsfera          : &w%-20.20s\n\r", esfera[esfera->esfera] );
        ch_printf( ch, "&cTipo            : &w%-20.20s&c     Mana : &w%3.3d\n\r", disci_tnombre[esfera->tipo],  esfera->mana );
        ch_printf( ch, "&cCodigo          : &w%-20.20s&c   Cabala : &w%20.20s\n\r", skill_name(esfera->do_fun), esfera->cabala );
        ch_printf( ch,"&c=====================&wMensajes del esfera&w==============================\n\r" );
        ch_printf( ch, "\n\rMensaje de danyo  : &w%s\n\r", esfera->dammes );
        ch_printf( ch, "Mensaje de remove : &w%s\n\r", esfera->sevames );
        ch_printf( ch, "Exitoch           : &w%s\n\r", esfera->exitoch );
        ch_printf( ch, "Exitovict         : &w%s\n\r", esfera->exitovict );
        ch_printf( ch, "Exitoroom         : &w%s\n\r", esfera->exitoroom );
        ch_printf( ch, "Exitomud          : &w%s\n\r", esfera->exitomud );
        ch_printf( ch, "Fallooch          : &w%s\n\r", esfera->falloch );
        ch_printf( ch, "Fallovict         : &w%s\n\r", esfera->fallovict );
        ch_printf( ch, "Falloroom         : &w%s\n\r", esfera->falloroom );
        ch_printf( ch, "Fallomud          : &w%s\n\r", esfera->fallomud );
        ch_printf( ch, "Muertech          : &w%s\n\r", esfera->muertech );
        ch_printf( ch, "Muertevict        : &w%s\n\r", esfera->muertevict );
        ch_printf( ch, "Muerteroom        : &w%s\n\r", esfera->muerteroom );
        ch_printf( ch, "Muertemud         : &w%s\n\r", esfera->muertemud );
        ch_printf( ch, "Inmunech          : &w%s\n\r", esfera->inmunech );
        ch_printf( ch, "Inmuneevict       : &w%s\n\r", esfera->inmunevict );
        ch_printf( ch, "Inmuneeroom       : &w%s\n\r", esfera->inmuneroom );
        ch_printf( ch, "Inmuneemud        : &w%s\n\r", esfera->inmunemud );
        send_to_char( "\n\r", ch );

return;
}



void do_aprender( CHAR_DATA *ch, char *argument )
{
    int i;
    int cuenta = 0;
    int necesita;
    int max_niv;
    char buf[MAX_STRING_LENGTH];

    max_niv = 0;

     if (!IS_VAMPIRE(ch))
     {
        switch( ch->generacion )
        {
                case 13:
                max_niv = 3;                  break;
                case 12:
                max_niv = 4;                  break;
                case 11:
                max_niv = 5;                  break;
                case 10:
                max_niv = 5;                  break;
                case 9:
                max_niv = 5;                  break;
                case 8:
                max_niv = 6;                  break;
                case 7:
                max_niv = 7;                  break;
                case 6:
                max_niv = 8;                  break;
                case 5:
                max_niv = 9;                  break;
                case 4:
                max_niv = 10;                  break;
                case 3:
                max_niv = 12;                  break;
                case 2:
                max_niv = 14;                  break;
                case 1:
                max_niv = 15;                  break;
          }
     }

    if (IS_NPC(ch))
    return;


    if (!str_cmp(argument, "cancelar"))
    {
	if (ch->pcdata->disci_adquirir == -1)
	{
	    send_to_char("&wAhora no estas aprendiendo ninguna esfera.\n\r",ch);
	    return;
	}
	send_to_char("&wParas de aprender esferas.\n\r", ch);
	ch->pcdata->disci_adquirir = -1;
	ch->pcdata->disci_puntos = 0;
 	return;
    }

    if (( ch->class == CLASS_MAGE || ch->class == CLASS_CLERIC ))
    {

    if (argument[0] == '\0')
    {
    for ( i = 1; i < MAX_ESFERAS; i++)
    {
        if(ch->pcdata->habilidades[i] == 10)
        cuenta++;

        if(cuenta >= 10)
        {
                send_to_char( "&wYa conoces todas las esferas que puedes aprender y sus niveles.\n\r", ch);
                cuenta = 0;
                return;
        }
    }
	send_to_char("&wQue esfera quieres aprender?.\n\r", ch);
	return;
    }

    if (ch->pcdata->disci_adquirir != -1)
    {
	send_to_char("&wYa estas aprendiendo una.\n\r", ch);
	send_to_char("&wUsa '&gaprender cancelar&w' para aprender otra.\n\r",ch);
	return;
    }

    for ( i = 1; i < MAX_ESFERAS; i++)
    {
        if(ch->pcdata->habilidades[i] == 10)
        cuenta++;

        if(cuenta >= 10)
        {
                send_to_char( "&wYa conoces todas las esferas que puedes aprender y sus niveles.\n\r", ch);
                cuenta = 0;
                return;
        }
    }

    for ( i = 1 ; i < MAX_ESFERAS ; i++)
    {
	if (esferas[i][0] != '\0'
	    && !str_prefix(argument, esferas[i]) )
	{
		necesita = ((ch->pcdata->habilidades[i] - 5) * 10);
	    if (ch->pcdata->habilidades[i] < 0)
	    {
    		send_to_char("&wNo conoces el poder de ninguna esfera asi.\n\r", ch);
		return;
	    }

          if ( !str_prefix( argument, "cardinal" )
	       || !str_prefix( argument, "correspondencia" )
               || !str_prefix( argument, "entropia" )
               || !str_prefix( argument, "espiritu" )
               || !str_prefix( argument, "fuerzas" )
               || !str_prefix( argument, "materia" )
               || !str_prefix( argument, "mente" )
               || !str_prefix( argument, "vida" ))
            {
	      send_to_char("&wLo sentimos pero esa esfera no esta acabada aun...\n\r", ch );
		return;
            }

          if ( ch->pcdata->habilidades[i] >= max_niv )
	  {
		send_to_char("&wNecesitas mejorar tu rango para conocer los secretos de ese nivel.\n\r",ch);
	        return;
	  }
	  if ( ch->pcdata->habilidades[i] >= 10 )
	  {
		send_to_char("&wYa has adquirido todos los Niveles de esa esfera.\n\r",ch);
                send_to_char("&wDebe de ser otra.\n\r",ch);
	        return;
	  }

	    sprintf(buf, "&wEmpiezas a concentrarte en los secretos de %s.\n\r",
		esferas[i]);
	    send_to_char(buf, ch);
	    ch->pcdata->disci_puntos = 0;
	    ch->pcdata->disci_adquirir = i;
	    return;
	}
    }
    send_to_char("&wNo conoces ninguna esfera llamada asi.\n\r", ch);
   }
   else
   {
   send_to_char( "&wSolo los Magos y Clerigos pueden aprender esferas.\n\r", ch );
      if( IS_VAMPIRE(ch))
      send_to_char( "&wTu debes usar el comando '&gadquirir'&w.\n\r", ch );
      return;
    }
}

void do_esferas( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    int loop;
    int percent = 0, i;
    int indent = 0;

    if(( ch->class == CLASS_MAGE  || ch->class == CLASS_CLERIC ))
    {
    cent_to_char( "&g[EsFeRaS MaGiCaS]\n\r", ch);

    for (loop = 0; loop < MAX_ESFERAS ; loop++)
    {
	if (esferas[loop][0] != '\0'
	&& strlen(esferas[loop]) != 0
	&& ch->pcdata->habilidades[loop] > 0 )
	{
	    sprintf(buf2, esferas[loop]);
	    buf2[0] = UPPER(buf2[0]);

            sprintf(buf,"%15.15s: &wNivel &c%d&g",  buf2, ch->pcdata->habilidades[loop]);
	    send_to_char(buf,ch);
            indent++;
	    if (indent == 1)
	    {
		send_to_char("\n\r", ch);
		indent = 0;
	    }
        }
     }

     if (ch->pcdata->disci_adquirir < 0)
    {
	send_to_char("\n\r", ch);
	return;
    }

    if (ch->pcdata->habilidades[ch->pcdata->disci_adquirir] < 0)
    {
	send_to_char("\n\r", ch);
	return;
    }

    if (ch->pcdata->disci_puntos == 999)
    {
	send_to_char("\n\r", ch);
	sprintf(buf, "&rHas terminado de asimilar &w%s&r.",
	disciplina[ch->pcdata->disci_adquirir]);
	cent_to_char(buf, ch);
	send_to_char("\n\r", ch);
	return;
    }

    if (ch->pcdata->disci_adquirir < MAX_ESFERAS)
    {
	send_to_char("\n\r", ch);
	sprintf(buf, "&gEstas aprendiendo el conociemiento de &w%s&g.",
	    esferas[ch->pcdata->disci_adquirir]);
    	send_to_char(buf, ch);
    }

    percent = ch->pcdata->disci_puntos * 40 / disci_puntos_nivel(ch);

    sprintf(buf, "&g  -[&o");

    for( i = 0 ; i < percent ; i++)
	strcat(buf, "&w|");

    for( i = percent ; i < 40 ; i++)
	strcat(buf, " ");

    strcat(buf, "&g]-");

    send_to_char(buf, ch);
    }
    else
    send_to_char( "Solo los magos y clerigos pueden usar esferas.\n\r", ch );

    return;
}


void do_conjurar( CHAR_DATA *ch, char *argument )
{
        ESFERA *esfera;
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];

        if( !ch )
        {
                bug( "Funcion Conjurar: ch NULO!", 0 );
                return;
        }

        if((IS_IMMORTAL(ch))|| (ch->class == CLASS_MAGE)  || (ch->class == CLASS_CLERIC))
        {
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
                send_to_char( "Conjurar el poder de que esfera magica?\n\r", ch );
                return;
        }


        if( arg1[0] !='\0' )
        esfera = get_esfera( arg1 );


        if( !esfera )
        {
                send_to_char( "No conoces ninguna esfera asi.\n\r", ch );
                return;
        }

    interprete2( ch, esfera, arg2 );
    }
    else
        send_to_char( "Solo los magos y clerigos pueden conjurar esferas( prueba el comando cast ).\n\r", ch );

return;
}

void lanzar_esfera( CHAR_DATA *ch, CHAR_DATA *victim, ESFERA *esfera )
{
  char buf[256];
  char buf2[256];
  char buf3[256];
  int chance;

  if( !ch )
  {
        bug( "Lanzar_esfera: ch NULO en la funcion.", 0 );
        return;
  }

  if(!IS_IMMORTAL(ch))
  if( ch->class != CLASS_MAGE || ch->class != CLASS_CLERIC )
  {
        bug( "Lanzar_esfera: %s ha podido llegar a lanzar_esfera sin ser mago o clerigo", ch->name );
        return;
  }

  if( victim )
  {

           chance = number_range( 1, 100 );
           if( chance >= 95 )
           {
                if( esfera->falloch != NULL )
                sprintf( buf, esfera->falloch );
                else
                sprintf(buf, "No consigues concentrarte lo suficiente y fallas tu %s.\n\r", esfera->nombre );

                if( esfera->fallovict != NULL )
                sprintf( buf2, esfera->fallovict );

                if( esfera->falloroom != NULL )
                sprintf( buf3, esfera->falloroom );

                act( AT_MAGIC, buf, ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, buf2, ch, NULL, ch, TO_VICT );
                act( AT_MAGIC, buf3, ch, NULL, victim, TO_NOTVICT );
                multi_hit( victim, ch, TYPE_UNDEFINED );

                return;
           }

           /*
            * Aqui se debera de incluir el caso para si son inmunes( por implementar )
            */
               if( !is_safe( victim, ch, TRUE ) );
               {
                if( esfera->exitoch != NULL )
                sprintf( buf, esfera->exitoch );

                if( esfera->exitovict != NULL )
                sprintf( buf2, esfera->exitovict );

                if( esfera->exitoroom != NULL )
                sprintf( buf3, esfera->exitoroom );
               }

               act( AT_MAGIC, buf, ch, NULL, victim, TO_CHAR );
               act( AT_MAGIC, buf2, ch, NULL, victim, TO_VICT );
               act( AT_MAGIC, buf3, ch, NULL, victim, TO_NOTVICT );



                return;


return;
 }/* cierra if( victim ) */
else
{


           chance = number_range( 1, 100 );
           if( chance >= 95 )
           {
                if( esfera->falloch != NULL )
                sprintf( buf, esfera->falloch );
                else
                sprintf(buf, "No consigues concentrarte lo suficiente y fallas tu %s.\n\r", esfera->nombre );

                if( esfera->falloroom != NULL )
                sprintf( buf3, esfera->falloroom );

                act( AT_MAGIC, buf, ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, buf3, ch, NULL, ch, TO_CANSEE );
                return;
           }

                if( esfera->exitoch[0] != '\0' )
                sprintf( buf, esfera->exitoch );

                if( esfera->exitoroom[0] != '\0' )
                sprintf( buf2, esfera->exitoroom );

                act( AT_MAGIC, buf, ch, NULL, NULL, TO_CHAR );
                act( AT_MAGIC, buf2, ch, NULL, ch, TO_CANSEE );
                return;
}

  return;
}

/*
 * Modifica el estado del jugador con unos valores dados
 */
void affect_esfera_modificar( CHAR_DATA *ch, ESFERA_AFFECT *eaf, bool fAdd )
{
        int mod;
        int total = (ch->generacion + eaf->modificador);
        ch_ret retcode;

        mod = eaf->modificador;

        if( fAdd )
        {
                xSET_BITS( ch->esferas, eaf->bitvector );
                mod = abs(mod);
        }
        else
        {
                xREMOVE_BITS( ch->esferas, eaf->bitvector );
                mod = 0 - mod;
        }

        switch( eaf->localizacion )
        {
                default:
	bug( "Affect_esfera_modificar: localizacion desconocida %d.", eaf->localizacion );
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

     case AP_REMOVE:       REMOVE_BIT(ch->esferas.bits[0], mod);	break;

    }

return;
}

void do_tiempo( CHAR_DATA *ch, char *argument )
{
        ESFERA *esfera;
        ESFERA_AFFECT eaf;

        if ( ch->mana <= (ch->generacion * 10) )
        {
        send_to_char( "No tienes suficiente mana.\n\r", ch );
        return;
        }
        else
        ch->mana -= (ch->generacion * 10);

                   if( who_fighting( ch ) != NULL)
                   {
                        send_to_char( "&wNo puedes concentrarte en medio de un combate!\n\r", ch);
                        return;
                   }

                   if( xIS_SET(ch->esferas, ESF_TIEMPO ) )
                   {
                      act( AT_MAGIC, "Aun mas rapido???.", ch, NULL, NULL, TO_CHAR );
                      return;
                   }
                   else
      if(argument[0]=='\0')
      {
        act( AT_MAGIC, "Conjuras 'Tiempo'.", ch, NULL, NULL, TO_CHAR );
        act( AT_MAGIC, "$N conjura 'Temporis'.", ch, NULL, ch, TO_ROOM );

        eaf.tipo = ESF_TIEMPO;
        eaf.localizacion = AP_NADA;
        if(ch->pcdata->habilidades[ESF_TIEMPO] > 5)
        eaf.duracion = ((ch->level * 16 / ch->generacion));
        else
        eaf.duracion = ((ch->level * (ch->pcdata->habilidades[ESF_TIEMPO] * 6) / ch->generacion));
        eaf.esfera = 7;
        eaf.modificador = 0;
        eaf.nivel = 701;
        eaf.bitvector = meb(ESF_TIEMPO);
        esfera_to_char( ch, &eaf );

	if (!IS_IMMORTAL(ch))
		WAIT_STATE(ch, 8);

    esfera = get_esfera( esferas[ESF_TIEMPO] );
    lanzar_esfera( ch, NULL, esfera );
      }
      else
      send_to_char( "No debes utilizar tiempo en otros.\n\r", ch );

    return;
}

void esfera_to_char( CHAR_DATA *ch, ESFERA_AFFECT *eaf )
{
        ESFERA_AFFECT *eaf_new;

        if(!ch)
        {
                bug( "esfera_to_char(NULO, %d)", eaf ? eaf->tipo : 0);
                return;
        }

        if(!eaf)
        {
                bug( "esfera_to_char(%s, NULO)", ch->name );
                return;
        }

        CREATE( eaf_new, ESFERA_AFFECT, 1);
        LINK( eaf_new, ch->primer_eaffect, ch->ultimo_eaffect, next, prev );
        eaf_new->duracion	= eaf->duracion;
        eaf_new->localizacion	= eaf->localizacion;
        eaf_new->modificador	= eaf->modificador;
        eaf_new->bitvector	= eaf->bitvector;
        eaf_new->nivel          = eaf->nivel;

        affect_esfera_modificar( ch, eaf_new, TRUE );
        return;
}

void interprete2( CHAR_DATA *ch, ESFERA *esfera, char *argument )
{
        (*esfera->do_fun) (ch, argument );
        return;
}
















