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
 *         Cabezera     del     Modulo     de        Conquistas
 ***************************************************************************/

/*Este software esta bajo la licencia GNU y su distribucion es libre.
  Como parte del proyecto GNU, no damos ningun tipo de garantia sobre su correcto funcionamiento
  O su inadaptabilidad a diversos sistemas

  Sherlock COLIKOTRON Code Team (no C) email to tystar@terra.es
  Parte del Code Team email to vampiromud@hotmail.com
*/

#include "mud.h"
/*#include "familias.h"*/

CON_DATA * primera_conquista;
CON_DATA * ultima_conquista;


/* Rutinas locales */
void write_con_list args( ( void ) );
void fread_con      args( ( CON_DATA *conquista, FILE *fp ) );
void save_con_file   args ( (CON_DATA *conquista) );
void do_conquistas     args( ( CHAR_DATA *ch, char *argument ) );
bool load_con_file  args( ( char *archivo ) );

void save_con_file (CON_DATA *conquista)
{
   FILE *fp;
   CLAN_DATA *familia;
   char archivo[256];
   char buf[MAX_STRING_LENGTH];

   if (!conquista)
   {
      bug( "Save_con_file: Puntero nulo!" );
      return;
   }

   if( !conquista->filename || conquista->filename[0] == '0' )
   {
	sprintf( buf, "save_con_file: %s no tiene nombre de fichero", conquista->area->name );
	bug( buf );
	return;
   }

   sprintf( archivo, "%s%s", CONQUISTAS_DIR, conquista->filename );

   fclose(fpReserve);

   fp = fopen( archivo, "w" );

   if( fp == NULL )
   {
     bug("Save_con_file:fopen fallado!");
     perror(archivo);
   }
   else
   {
     fprintf( fp,"#CONQUISTA\n" );
     fprintf(fp, "Area            %s~\n", conquista->nombre);
     fprintf(fp,"Filename        %s~\n", conquista->filename);
     fprintf(fp,"Conquistador      %s~\n", conquista->n_conquistador);
     fprintf(fp,"UltimoConc        %s~\n", conquista->u_conquistador);
     fprintf(fp,"Propietario       %s~\n", conquista->propietario );

       for ( familia = first_clan; familia; familia = familia->next )
        fprintf( fp, "Puntos%d   %d\n", familia->numero, conquista->puntos[familia->numero] );


	  /*
	     Anyadir aqui lo de las drogas (cuando te hecho xD)
	  */

          fprintf( fp, "Fecha     %s~\n", ctime(&current_time));

     fprintf( fp, "\nEnd\n" );
    }
    fclose ( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
};

void write_con_list( )
{
   CON_DATA * con;
   FILE *fpout;
   char archivo[256];

   sprintf( archivo, "%scon.lst", CONQUISTAS_DIR );
   if( !fpout )
    {
      bug( "NO se ha podido abrir con.lst para escribir", archivo );
      return;
    }

    for ( con = primera_conquista;con; con = con->next)
    fprintf( fpout, "%s\n", con->filename );

    fclose( fpout );
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
 * Lee datos de la estructura externa y los carga
 */

void fread_con( CON_DATA *conquista, FILE *fp )
{
        char buf[MAX_STRING_LENGTH];
        char *word;
        bool fMatch;
	CLAN_DATA *familia;

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

                        case 'A':
                        KEY( "Area",    conquista->nombre,          fread_string( fp ));
                        break;

                        case 'C':
                        KEY( "Conquistable",    conquista->se_conquista,                fread_number( fp ));
                        KEY( "Conquistador",    conquista->n_conquistador,              fread_string( fp ));
                        break;

                        case 'F':
                        KEY( "Filename",        conquista->filename,                    fread_string( fp ));
                        KEY( "Fecha",           conquista->fecha,                       fread_string( fp ));
                        break;

                        case 'P':
			KEY( "Puntos0",		conquista->puntos[0],			fread_number( fp ));
			KEY( "Puntos1",		conquista->puntos[1],			fread_number( fp ));
			KEY( "Puntos2",		conquista->puntos[2],			fread_number( fp ));
			KEY( "Puntos3",		conquista->puntos[3],			fread_number( fp ));
                        KEY( "Propietario",     conquista->propietario,                 fread_string( fp ));
                        break;

                        case 'U':
                        KEY( "Ultimoconc",      conquista->u_conquistador,              fread_string( fp ));
                        break;

                        case 'E':
                         /*
                          * En caso de que encuentre la palabra End se checkean los campos
                          * anyadiendo lo que sea necesario
                          */
                                if( !str_cmp( word, "End" ) )
                                {
                                        if( !conquista->nombre )
                                        conquista->nombre       =       STRALLOC( "Desconocida" );
                                        if( !conquista->se_conquista )
                                        conquista->se_conquista =       1;
                                        if( !conquista->n_conquistador )
                                        conquista->n_conquistador =     STRALLOC( "Desconocido" );
                                        if( !conquista->fecha )
                                        conquista->fecha        =       STRALLOC( "Desconocida" );
                                        if( !conquista->propietario )
                                        conquista->propietario  =       STRALLOC( "Territorio virgen" );
                                        if( !conquista->u_conquistador )
                                        conquista->u_conquistador =     STRALLOC( "Desconocido" );
                                        return;
                                 }

                                /*
                                 * Si fMatch devuelve nulo lanza un aviso
                                 */

                                if( !fMatch )
                                {
                                        sprintf( buf, "Fread_con: no se ha encontrado: %s", word );
                                        bug( buf, 0);
                                }
                  } /* Cierra switch */
            } /* Cierra el for */
} /* Cierra el void */

bool load_con_file( char *archivo )
{
        char filename[256];
        CON_DATA *con;
        FILE *fp;
        bool found;

        CREATE( con, CON_DATA, 1 );
        found = FALSE;
        sprintf( filename, "%s%s", CONQUISTAS_DIR, archivo );

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
                        bug( "Load_con_file: No se encontro #.", 0);
                        break;
                }

                word = fread_word( fp );
                if( !str_cmp( word, "CONQUISTA" ) )
                {
                        fread_con( con, fp );
                        break;
                }
                else
                if( !str_cmp( word, "END" ) )
                        break;
                else
                {
                        char buf[MAX_STRING_LENGTH];

                        sprintf( buf, "Load_con_file: Seccion erronea: %s.", word );
                        bug( buf, 0);
                        break;
                }
        }
         fclose( fp );
    }

    if( found )
        LINK( con, primera_conquista, ultima_conquista, next, prev );

    else
        DISPOSE( con );

       return found;
}

void load_conquista( )
{
        FILE *fpList;
        char *filename;
        char nlist[256];
        char buf[256];

        primera_conquista = NULL;
        ultima_conquista = NULL;

        log_string( "Cargando conquistas..." );

        sprintf( nlist, "%s%s", CONQUISTAS_DIR, AREA_LIST );
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



           if( !load_con_file( filename ) )
           {
                sprintf( buf, "No se pudo abrir archivo: %s.", filename );
                bug( buf, 0 );
		sprintf( buf, "Se procede a la creacion de la estructura para el siguiente reload" );
		crear_conquista_nula( filename );
           }

         }
         fclose( fpList );
         log_string( " Datos de Conquista Cargados" );
         fpReserve = fopen( NULL_FILE, "r" );
         return;
}

/*
 * Ganancia de puntos de conquista
 */
void gain_conquista( CHAR_DATA *ch, int ganancia  )
{
        CLAN_DATA *victima;
        CLAN_DATA *clan;
        AREA_DATA *tarea;
        CON_DATA *conquista;

        char buf[MAX_STRING_LENGTH];

        tarea = get_area( ch->in_room->area->name );
        conquista = get_conquista( ch->in_room->area->name );
        victima = get_clan( conquista->propietario );


       if ( conquista->propietario != ch->pcdata->clan->name )
       {
                conquista->puntos[ch->pcdata->clan->numero] += ganancia;

                if( conquista->puntos[ch->pcdata->clan->numero] > conquista->puntos[victima->numero] )
                {
                	sprintf( buf, "%s &gha conquistado %s para la familia %s", ch->name, ch->in_room->area->name, ch->pcdata->clan->name );
                	mensa_todos( ch, "conquistas", buf );
                	sprintf( buf, "&gLa familia %s pierde los derechos sobre el area y &w%d &gtierras", conquista->propietario, tarea->room_total );
                	mensa_todos( ch, "conquistas", buf );
                	sprintf( buf, "%s ya ha conquistado %d areas", ch->name, ch->conquistas +1 );
                	mensa_familia(ch, buf);
                	ch->pcdata->clan->puntos += (ch->in_room->area->room_total) / number_range(2,3);
                	ch->pcdata->clan->tierras += ch->in_room->area->room_total;
                	ch->pcdata->clan->conquistas++;
                	ch->conquistas++;
                	victima->tierras -= ch->in_room->area->room_total;
                	if (victima->puntos >= 1)
                	victima->puntos  -= (ch->in_room->area->room_total / 2);
			conquista->u_conquistador = conquista->propietario;
                	conquista->propietario = ch->pcdata->clan->name;
			conquista->n_conquistador = ch->name;
                	if( conquista->puntos[ch->pcdata->clan->numero] >= 2000 )
                		conquista->puntos[ch->pcdata->clan->numero] = 2000;

                	if( conquista->puntos[victima->numero] > 0 )
                		conquista->puntos[victima->numero] -= ganancia;

                	save_con_file( conquista );
                }
	}
        else
        {
                for ( clan = first_clan; clan; clan = clan->next )
               	{
                        if ( clan != ch->pcdata->clan && conquista->puntos[ch->pcdata->clan->numero] >= 2000 && conquista->puntos[clan->numero] > 0 )
                        	conquista->puntos[clan->numero] -= ganancia;

                        if ( conquista->puntos[clan->numero] <= -1 )
                        	conquista->puntos[clan->numero] = 0;

                        if( conquista->puntos[ch->pcdata->clan->numero] >= 2000 )
                        	conquista->puntos[ch->pcdata->clan->numero] = 2000;

                        save_con_file( conquista );
                 }
     	}
	return;
}
/* Kayser 2004 */



void do_conquistas( CHAR_DATA *ch, char *argument )
{
	CON_DATA *conquista;
	AREA_DATA *area;
	bool found = FALSE;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument( argument, arg1 );
	argument = one_argument( argument, arg2 );

	if( !str_cmp( arg1, "guardar" ) && IS_IMMORTAL(ch) )
	{
		area = get_area_fp( arg2 );
		if( area == NULL )
		{
			ch_printf( ch, "Esa conquista no existe.\n\r", ch );
			found = FALSE;
		}
		else
		found = TRUE;

		if( found )
		{
			conquista = get_conquista( area->name );

			if( conquista )
			save_con_file( conquista );
			else
			bug( "Do_conquistas: Algo ha fallado, no se pudo guardar la conquista solicitada." );
		}
		send_to_char( "&gHecho!\n\r", ch );
		return;
	}
	else
	{
 	if( arg1[0] != '\0' )
	{
		area = get_area( arg1 );

		if( area == NULL )
		{
		ch_printf( ch, "&g%s no es un area.\n\r", arg1);
		return;
		}
		else
		{
			conquista = get_conquista( area->name );

			ch_printf( ch, "&w%-20.20s &gPuntos en ella: &w%d &gArrebatada a: &w%-12.12s &gPor: &w%-12.12s\n\r",
			conquista->nombre,
			conquista->puntos[ch->pcdata->clan->numero],
			conquista->u_conquistador,
			conquista->n_conquistador );
			return;
		}
 	}

	for (conquista = primera_conquista; conquista; conquista = conquista->next )
	{
	ch_printf( ch, "&gArea : &w%25.25s &c%20.20s ", conquista->nombre, conquista->propietario );
	ch_printf( ch, "&gPor: &w%12s\n\r", conquista->n_conquistador );
	}
	}

	return;
}


void crear_conquista_nula( char *filename )
{
	CON_DATA *con;
	AREA_DATA *area = get_area_fp( filename );
	char strsave[MAX_INPUT_LENGTH];
	int bucle = 0;

	if( !filename )
	{
		bug( "Crear_conquista_nula: No hay nombre de archivo! CANCELANDO!!!" );
		return;
	}

	sprintf( strsave, "%s%s", CONQUISTAS_DIR, filename );

	CREATE( con, CON_DATA, 1 );
	LINK( con, primera_conquista, ultima_conquista, next, prev );

	con->nombre = STRALLOC( area->name );
	con->filename = STRALLOC(  filename );
	con->propietario = STRALLOC( "Territorio Virgen" );
	con->u_conquistador = STRALLOC ( "Ninguno" );
	con->n_conquistador = STRALLOC ( "Ninguno" );
	for ( bucle = 0 ; bucle < 4; bucle++ )
	con->puntos[bucle] = 0;
	log_string( "Conquista Creada..." );
return;
}





/*
 * Devuelve una conquista con una cadena dada
 * Ultima modificacion 20/06/2002
 * Colikotron Code Team
 *                      SiGo
 */
CON_DATA *get_conquista(char *name)
{
	CON_DATA *con;

	if(!name)
	{
		bug("get_conquista: No hay cadena con la que comparar.");
		return NULL;
	}

	for(con = primera_conquista; con; con = con->next)
	{
		if(nifty_is_name(name, con->nombre))
			break;
	}

        return con;
}

/*
 * Devuelve un area con una cadena dada
 * identificandola por el nombre del fichero
 * Ultima modificacion 22/07/2002
 * Colikotron Code Team
 *			SiGo
 */
AREA_DATA *get_area_fp( char *filename )
{
	AREA_DATA *area;

	if( !filename )
	{
		bug( "get_area_fp: No hay cadena con la que comparar." );
		return NULL;
	}

	for (area = first_area; area; area = area->next )
	{
		if(nifty_is_name(filename, area->filename))
			break;
	}

	return area;
}
