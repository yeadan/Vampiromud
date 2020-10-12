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
 *                          modulo de familias                              *
 ****************************************************************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <mysql/mysql.h>


 #include "mud.h"
 #include "familias.h"

 // Pa recorrer los punteros :D
 FAMILIA_DATOS *primeraFamilia;
 FAMILIA_DATOS *ultimaFamilia;

 // Funciones locales
void cargaFamilia( char *nombre, FAMILIA_DATOS *fam );

void fDebug( char *msg )
{
	if( FAM_DEBUG )
	{
		bug( "&z--[&rDepuracion FAMILIAS&z]-- &w%s.\n\r", msg );
		return;
	}
	return;
}

int calculaCoeficiente( FAMILIA_DATOS *fam )
{
	FAMILIA_DATOS *pfam;
	int res, fam_cnt = 0;

	for( pfam = primeraFamilia; pfam; pfam = pfam->sig )
		fam_cnt++;


	res = (( fam->oro + fam->puntos + fam->conq + fam->px ) / fam_cnt );

	return res;
}

 // Esta funcion nos devuelve el puntero a la familia que buscamos
 // a excepcion de que no la encuentre que devuelve nulo.
FAMILIA_DATOS *buscaFamilia( char *nombre )
{
 	FAMILIA_DATOS *fam;

	for( fam = primeraFamilia; fam; fam = fam->sig )
	{
		if( !str_prefix( nombre, fam->alias ))
			return fam;
	}

 	return NULL;
}

int posicionFamilia( FAMILIA_DATOS *familia )
{
	/*
	 * A ver, supongo que no es muy dificil de usar esta funcion
	 * si se le añade algo mas al calculo, se tiene que modificar
	 * la formula final. Ej. Si hay oro, quest i px son 3, por lo
	 * tanto la formula ( Cantidad de familias - Posicion ) / 3
	 * facil, no?
	 *
	 * SaNgUiNaRi
	 */

	FAMILIA_DATOS *fam;
	int pos = 0, cntFam = 0;

	for( fam = primeraFamilia; fam; fam = fam->sig )
	{
		cntFam++;
		if( cntFam == 1 )
		{
			fam->puntos = 4000;
		}
		if( cntFam == 2 )
		{
			fam->puntos = 2400;
		}
		if( cntFam == 3 )
		{
			fam->puntos = 12400;
		}

	}


	for( fam = primeraFamilia; fam; fam = fam->sig )
	{
		if( calculaCoeficiente( familia ) <= calculaCoeficiente( fam ) )
			pos++;
	}

	return pos;
}


/*
 * A ver, si la accion es igual a 1 no se guardaran los puntos, ni los historiales
 * es decir se pone a 0 la familia
 */
void guardaFamilia( FAMILIA_DATOS *familia, int accion )
 {
 	FILE	*fp;
    char archivo[256];
    char buf[MAX_STRING_LENGTH];

	fDebug( "Iniciando funcion &gguardaFamilia&w" );

	if( !familia )
	{
		fDebug( "No existe un puntero a la familia." );
		bug( "Error en funcion guardaFamilia, no existe la familia." );
		return;
	}

	fclose( fpReserve );

	sprintf( archivo, "%s%s%s", FAM_DIR, capitalize( familia->alias ), FAM_EXT );

	if ( ( fp = fopen( archivo, "w" ) ) == NULL )
    {
		sprintf( buf, "No se puede escribir el archivo de familia: %s comprueba q se puede escribir y que el directorio existe", archivo );
		fDebug( buf );
    	return;
    }
	else
	{
//		fprintf( fp, "#FAMILIA\n" );
		fprintf( fp, "Alias       %s~\n", familia->alias );
		fprintf( fp, "Nombre      %s~\n", familia->nombre );
		fprintf( fp, "Descr       %s~\n", familia->descr );

	if( accion != ACT_FAM_VACIAR )
	{
		fprintf( fp, "Fundador    %s~\n", familia->fundador );
		fprintf( fp, "Lider       %s~\n", familia->lider );
		fprintf( fp, "Protector   %s~\n", familia->protector );
		fprintf( fp, "Heredero    %s~\n", familia->heredero );
		fprintf( fp, "Consejero   %s~\n", familia->consejero );
		fprintf( fp, "Capitan      %s~\n", familia->capitan );
		fprintf( fp, "Sacerdote   %s~\n", familia->sacerdote );
		fprintf( fp, "Tesorero    %s~\n", familia->tesorero );
		fprintf( fp, "rLider       %s~\n", familia->rango_lider );
		fprintf( fp, "rProtector   %s~\n", familia->rango_protector );
		fprintf( fp, "rHeredero    %s~\n", familia->rango_heredero );
		fprintf( fp, "rConsejero   %s~\n", familia->rango_consejero );
		fprintf( fp, "rCapitan      %s~\n", familia->rango_capitan );
		fprintf( fp, "rSacerdote   %s~\n", familia->rango_sacerdote );
		fprintf( fp, "rTesorero    %s~\n", familia->rango_tesorero );
	}

	if( accion != ACT_FAM_LIMPIAR )
	{
		fprintf( fp, "Puntos       %d\n", familia->puntos );
		fprintf( fp, "Oro          %d\n", familia->oro );
		fprintf( fp, "Mineral      %d\n", familia->mineral );
		fprintf( fp, "Comida       %d\n", familia->comida );
		fprintf( fp, "Combustible  %d\n", familia->combustible );
		fprintf( fp, "Px           %d\n", familia->px );
		fprintf( fp, "Conquistas   %d\n", familia->conq );
		fprintf( fp, "Quest        %d\n", familia->quest );
		fprintf( fp, "Areas        %d\n", familia->areas );
		fprintf( fp, "Miembros     %d\n", familia->miembros );

		fprintf( fp, "mPuntos       %d\n", familia->max_puntos );
		fprintf( fp, "mOro          %d\n", familia->max_oro );
		fprintf( fp, "mMineral      %d\n", familia->max_mineral );
		fprintf( fp, "mComida       %d\n", familia->max_comida );
		fprintf( fp, "mCombustible  %d\n", familia->max_combustible );
		fprintf( fp, "mPx           %d\n", familia->max_px );
		fprintf( fp, "mConquistas   %d\n", familia->max_conq );
		fprintf( fp, "mQuest        %d\n", familia->max_quest );
		fprintf( fp, "mAreas        %d\n", familia->max_areas );
		fprintf( fp, "mMiembros     %d\n", familia->max_miembros );

		fprintf( fp, "hPuntos       %d\n", familia->hist_puntos );
		fprintf( fp, "hOro          %d\n", familia->hist_oro );
		fprintf( fp, "hMineral      %d\n", familia->hist_mineral );
		fprintf( fp, "hComida       %d\n", familia->hist_comida );
		fprintf( fp, "hCombustible  %d\n", familia->hist_combustible );
		fprintf( fp, "hPx           %d\n", familia->hist_px );
		fprintf( fp, "hConquistas   %d\n", familia->hist_conq );
		fprintf( fp, "hQuest        %d\n", familia->hist_quest );
		fprintf( fp, "hAreas        %d\n", familia->hist_areas );
		fprintf( fp, "hMiembros     %d\n", familia->hist_miembros );

		fprintf( fp, "MobMuertos    %d\n", familia->mobMuertos );
		fprintf( fp, "MobMatados    %d\n", familia->mobMatados );
		fprintf( fp, "MobHuidas     %d\n", familia->mobHuidas );
		fprintf( fp, "PkVictorias   %d\n", familia->pkVictorias );
		fprintf( fp, "PkDerrotas    %d\n", familia->pkDerrotas );
		fprintf( fp, "PkIlegal      %d\n", familia->pkIlegal );
		fprintf( fp, "PkHuidas      %d\n", familia->pkHuidas );
	}
		fprintf( fp, "#END\n" );
	}
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
 }

 void guardaConfigFamilias()
 {
 	FAMILIA_DATOS *fam;
	FILE	*fp;
	char	archivo[256];
	char	buf[MAX_STRING_LENGTH];

	fDebug( "Iniciando funcion: &gguardaConfigFamilias" );

	sprintf( archivo, "%s%s", FAM_DIR, FAM_CONFIG_FILE );

	fclose( fpReserve );


	if ( ( fp = fopen( archivo, "w" ) ) == NULL )
    {
		sprintf( buf, "No se puede escribir el archivo de familia: %s comprueba q se puede escribir y que el directorio existe", archivo );
		fDebug( buf );
    	return;
	}
	else
	{
		for( fam = primeraFamilia; fam; fam = fam->sig )
		{
			fprintf( fp, "%s\n", capitalize( fam->alias ) );
		}
		fprintf( fp, "$\n" );
	}
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
 }

 /*
  * Esta funcion es para cargar las familias asi como sus opciones
  * si las hay.
  */
 void cargaConfigFamilias()
 {
 	FILE *fp;
	FAMILIA_DATOS *fam;
	char *archivo;
	char famlist[256];
	char *palabra;
	char letra;
	char buf[MAX_STRING_LENGTH];

	primeraFamilia = NULL;
	ultimaFamilia = NULL;

	log_string( "Cargando el modulo de las familias." );

	sprintf( archivo, "%s%s", FAM_DIR, FAM_CONFIG_FILE );

	fclose( fpReserve );


	if ( ( fp = fopen( archivo, "r" ) ) == NULL )
    {
		log_string( "ERROR: No se ha podido abrir el archivo de configuracion de las familias." );
		return;
	}

	for(;;)
	{
			palabra = fread_word( fp );
			if( !str_cmp( palabra, "$" ) )
			{
				fclose( fp );
				break;
			}

				CREATE( fam, FAMILIA_DATOS, 0 );
				cargaFamilia( palabra, fam );
				LINK( fam, primeraFamilia, ultimaFamilia, sig, prev );
    }
	return;
 }

 /*
  * Esta es la funcion cargaFamilia, no confundir con el plural
  * q a ver si bebemos menos tintorro i nos fijamos es q la s
  * es plural.
  */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( palabra, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fmatch = TRUE;                      \
                                    break;                              \
                                }

 void cargaFamilia( char *familia, FAMILIA_DATOS *fam )
 {
	FILE *fp;
	char *archivo;
	char *palabra;
	char letra;
	bool fmatch;

	sprintf( archivo, "%s%s%s", FAM_DIR, capitalize( familia ), FAM_EXT );

	if(( fp = fopen( archivo, "r" )) == NULL )
	 {
	 	bug( "No se ha podido cargar la familia %s. No se puede acceder al archivo.", capitalize( familia ) );
		return;
	 }


	fmatch = FALSE;


	for(;;)
	{
		palabra = feof( fp ) ? "END" : fread_word( fp );

		switch( UPPER( palabra[0] ) )
		{
			case '*':
				fmatch = TRUE;
				fread_to_eol( fp );
				break;

			case 'A':
				KEY( "Alias", fam->alias, fread_word( fp ) );
				KEY( "Areas", fam->areas, fread_number( fp ) );
				KEY( "Activa", fam->activa, fread_number( fp ) );
				break;

			case 'C':
				KEY( "Consejero", fam->consejero, fread_word( fp ) );
				KEY( "Capitan", fam->capitan, fread_word( fp ) );
				KEY( "Comida", fam->comida, fread_number( fp ) );
				KEY( "Combustible", fam->combustible, fread_number( fp ) );
				KEY( "Conquistas", fam->conq, fread_number( fp ) );
				break;

			case 'D':
				KEY( "Descr", fam->descr, fread_string( fp ) );
				break;

			case 'F':
				KEY( "Fundador", fam->fundador, fread_string( fp ) );
				break;

			case 'H':
				KEY( "Heredero", fam->heredero, fread_string( fp ) );
				KEY( "hPuntos", fam->hist_puntos, fread_number( fp ) );
				KEY( "hOro", fam->hist_oro, fread_number( fp ) );
				KEY( "hMineral", fam->hist_mineral, fread_number( fp ) );
				KEY( "hComida", fam->hist_comida, fread_number( fp ) );
				KEY( "hCombustible", fam->hist_combustible, fread_number( fp ) );
				KEY( "hPx", fam->hist_px, fread_number( fp ) );
				KEY( "hConquistas", fam->hist_conq, fread_number( fp ) );
				KEY( "hQuest", fam->hist_quest, fread_number( fp ) );
				KEY( "hAreas", fam->hist_areas, fread_number( fp ) );
				KEY( "hMiembros", fam->hist_miembros, fread_number( fp ) );
				break;

			case 'L':
				KEY( "Lider", fam->lider, fread_string( fp ) );
				break;

			case 'M':
				KEY( "Miembros", fam->miembros, fread_number( fp ) );
				KEY( "mPuntos", fam->max_puntos, fread_number( fp ) );
				KEY( "mOro", fam->max_oro, fread_number( fp ) );
				KEY( "mMineral", fam->max_mineral, fread_number( fp ) );
				KEY( "mComida", fam->max_comida, fread_number( fp ) );
				KEY( "mCombustible", fam->max_combustible, fread_number( fp ) );
				KEY( "mPx", fam->max_px, fread_number( fp ) );
				KEY( "mConquistas", fam->max_conq, fread_number( fp ) );
				KEY( "mQuest", fam->max_quest, fread_number( fp ) );
				KEY( "mAreas", fam->max_areas, fread_number( fp ) );
				KEY( "mMiembros", fam->max_miembros, fread_number( fp ) );
				KEY( "Mineral", fam->mineral, fread_number( fp ) );
				KEY( "MobMuertos", fam->mobMuertos, fread_number( fp ) );
				KEY( "MobMatados", fam->mobMatados, fread_number( fp ) );
				KEY( "Mobhuidas", fam->mobHuidas, fread_number( fp ) );
				break;

			case 'N':
				KEY( "Nombre", fam->nombre, fread_string( fp ) );
				break;

			case 'O':
				KEY( "Oro", fam->oro, fread_number( fp ) );
				break;

			case 'P':
				KEY( "Protector", fam->protector, fread_string( fp ) );
				KEY( "PkVictorias", fam->pkVictorias, fread_number( fp ) );
				KEY( "PkDerrotas", fam->pkDerrotas, fread_number( fp ) );
				KEY( "PkIlegal", fam->pkIlegal, fread_number( fp ) );
				KEY( "PkHuidas", fam->pkHuidas, fread_number( fp ) );
				KEY( "Puntos", fam->puntos, fread_number( fp ) );
				KEY( "Px", fam->puntos, fread_number( fp ) );
				break;

			case 'Q':
				KEY( "Quest", fam->quest, fread_number( fp ) );
				break;

			case 'R':
				KEY( "rLider", fam->rango_lider , fread_string( fp ) );
				KEY( "rProtector", fam->rango_protector , fread_string( fp ) );
				KEY( "rHeredero", fam->rango_heredero , fread_string( fp ) );
				KEY( "rConsejero", fam->rango_consejero , fread_string( fp ) );
				KEY( "rCapitan", fam->rango_capitan , fread_string( fp ) );
				KEY( "rSacerdote", fam->rango_sacerdote , fread_string( fp ) );
				KEY( "rTesorero", fam->rango_tesorero , fread_string( fp ) );
				break;

			case 'S':
				KEY( "Sacerdote", fam->sacerdote , fread_string( fp ) );
				break;

			case 'T':
				KEY( "Tesorero", fam->tesorero , fread_string( fp ) );
				break;
		}

		if( !str_cmp( palabra, "END" ) )
		{
			return;
		}

		if ( !fmatch )
		{
	    	bug( "cargaFamilia: no se encuentra: %s", palabra );
		}
	}
}

 // Para iniciar las familias :D
 void cargaFamilias()
 {
 	FILE	*fp;
	char	archivo[256];
	FAMILIA_DATOS *fam;
	char	letra;
	char	*palabra;
	int 	error = 0;

	log_string( "1" );
	sprintf( archivo, "%s%s", FAM_DIR, FAM_CONFIG_FILE );

	if ( ( fp = fopen( archivo, "r" ) ) == NULL )
	{
		fDebug( "No se pueden iniciar las familias, no se ha cargado el archivo de configuracion." );
		bug( "%s:%d > No se ha conseguido cargar el fichero de configuracion de las familias (%s)", __FILE__, __LINE__, archivo );
		return;
	}
	else
	{
		for(;;)
		{
			letra = fread_letter( fp );

			if( letra == '*' )
			{
				fread_to_eol(fp);
				continue;
			}

			if( letra != '#'  )
			{
				bug( "%s:%d > No se ha encontrado ninguna seccion en el archivo (%s). xD", __FILE__, __LINE__, archivo );
				break;
			}

			palabra = fread_word( fp );

			if( !str_cmp( palabra, "#FAMILIA" ) )
			{
				if((buscaFamilia( palabra )) == TRUE )
				{
					bug( "Esa familia ya esta cargada. (%s)", palabra );
					break;
				}
				else
				{
					CREATE( fam, FAMILIA_DATOS, 0 );
					cargaFamilia( palabra, fam );
					LINK( fam, primeraFamilia, ultimaFamilia, sig, prev );
					break;
				}
			}

			if( !str_cmp( palabra, "#FIN" ) )
			{
				fclose( fp );
				break;
			}

		}
	}
	bug( "2" );
	return;
 }

 // Funciones EXTERNAS

// Esta funcion es para crear las familias
void do_fcrear( CHAR_DATA *ch, char *arg )
{
	FAMILIA_DATOS	*fam;

	if( !arg || arg[0] == '\0' )
	{
		ch_printf( ch, "\n\r&gUso:\n\r\t&wfcrear &z<&g alias familia&z >\n\r" );
		ch_printf( ch, "\n\r&WNota:&w El alias de la familia no debe pasar de los &g20&w caracteres.\n\r" );
		return;
	}

	if((fam = buscaFamilia( arg )) != NULL )
	{
		ch_printf( ch, "&wA ver tio, esa familia ya existe!!!. :D\n\r" );
		return;
	}

	// Creamos el puntero
    CREATE( fam, FAMILIA_DATOS, 1 );
    LINK( fam, primeraFamilia, ultimaFamilia, sig, prev );

	// Valores por defecto
	fam->alias  = STRALLOC( arg );
	fam->nombre = STRALLOC( arg );
	fam->lider	= STRALLOC( "Nadie" );
	fam->protector	= STRALLOC( "Nadie" );
	fam->heredero	= STRALLOC( "Nadie" );
	fam->consejero	= STRALLOC( "Nadie" );
	fam->tesorero	= STRALLOC( "Nadie" );
	fam->capitan	= STRALLOC( "Nadie" );
	fam->fundador	= STRALLOC( "Nadie" );
	fam->descr		= STRALLOC( "No tienen" );
	fam->sacerdote  = STRALLOC( "Nadie" );
	fam->tesorero 	= STRALLOC( "Nadie" );
	fam->rango_lider 		= STRALLOC( "Ninguno" );
	fam->rango_protector 	= STRALLOC( "Ninguno" );
	fam->rango_heredero 	= STRALLOC( "Ninguno" );
	fam->rango_consejero 	= STRALLOC( "Ninguno" );
	fam->rango_capitan 		= STRALLOC( "Ninguno" );
	fam->rango_sacerdote 	= STRALLOC( "Ninguno" );
	fam->rango_tesorero 	= STRALLOC( "Ninguno" );

	fam->oro	= 100000;
	fam->max_oro	= 100000;
	fam->hist_oro	= 100000;

	ch_printf( ch, "&wFamilia &g%s &wcreada correctamente, si no la editas se perdera en la \n\rsiguiente recarga del mud.\n\r", arg );
	ch_printf( ch, "\n\r&wPara editar la familia usa el comando &gfeditar&w.\n\r" );
	return;
}

void do_feditar( CHAR_DATA *ch, char *arg )
{
	char	arg1[MAX_INPUT_LENGTH];
	char	arg2[MAX_INPUT_LENGTH];
	char	arg3[MAX_INPUT_LENGTH];
	FAMILIA_DATOS *fam;

	arg = one_argument( arg, arg1 );
	arg = one_argument( arg, arg2 );
	arg = one_argument( arg, arg3 );

	if( arg1[0] == '\0' )
	{
		ch_printf( ch, "&wUso:\n\r" );
		ch_printf( ch, "\t&wfeditar &z<&g alias familia&z > <&g campo &z > <&g valor &z>\n\r" );
		ch_printf( ch, "\n\r&wPon familia &gayuda&w para ver los posibles campos.\n\r" );
		return;
	}

	if( !str_prefix( arg1, "ayuda" ))
	{
		ch_printf( ch, "&wPosibles valores para la &gedicion &wde las &gfamilias&w:\n\r" );
		ch_printf( ch, "\tnombre, lider, heredero, consejero, \n\r" );
		return;
	}


	if((fam = buscaFamilia( arg1 )) == NULL )
	{
		ch_printf( ch, "&wEsa &gfamilia&w no existe.\n\r&zQue tas fumao tio?\n\r" );
		return;
	}

	if( !str_prefix( arg2, "nombre" ) )
	{
		if( arg3[0] == '\0' )
		{
			ch_printf( ch, "No has especificado un nombre, seras borracho.\n\r" );
			return;
		}

		fam->nombre = STRALLOC( arg3 );
		ch_printf( ch, "Nombre de la familia cambiado :D\n\r" );
		return;
	}

	if( !str_prefix( arg2, "alias" ) )
	{
		if( ch->level <= MAX_LEVEL-1 )
		{
			ch_printf( ch, "No puedes cambiar el alias de la familia, solo los dioses mas poderosos son capaces de hacerlo.\n\r" );
			return;
		}

		if( arg3[0] == '\0' )
		{
			ch_printf( ch, "No has especificado un alias para la familia, seras borracho.\n\r" );
			return;
		}

		fam->alias = STRALLOC( arg3 );
		ch_printf( ch, "Alias de la familia cambiado :D\n\r" );
		return;
	}

	if( !str_prefix( arg2, "lider" ) )
	{
		if( arg3[0] == '\0' )
		{
			ch_printf( ch, "No has especificado un jugador, seras borracho.\n\r" );
			return;
		}

		fam->lider = capitalize( arg3 );
		ch_printf( ch, "Lider de la familia cambiado :D\n\r" );
		return;
	}

	if( !str_prefix( arg2, "heredero" ) )
	{
		if( arg3[0] == '\0' )
		{
			ch_printf( ch, "No has especificado un jugador, seras borracho.\n\r" );
			return;
		}

		fam->heredero = capitalize( arg3 );

		ch_printf( ch, "Heredero de la familia cambiado :D\n\r" );
		return;
	}

	if( !str_prefix( arg2, "activa" ) )
	{
		if( arg3[0] == '\0' )
		{
			ch_printf( ch, "&wDebes poner &gsi&w o &gno&w. (&gS&w/&gN&w)\n\r&zAla prueba otra vez. :D\n\r" );
			return;
		}


		if( !str_prefix( arg3, "si" ) )
		{
			fam->activa = TRUE;
			ch_printf( ch, "&wAhora la gente puede entrar en esa familia.\n\r" );
			return;
		}


		if( !str_prefix( arg3, "no" ) )
		{
			fam->activa = TRUE;
			ch_printf( ch, "&wAhora la gente no puede entrar en esa familia.\n\r" );
			return;
		}
	}

	ch_printf( ch, "&wEse argumento no es valido.\n\r" );
	ch_printf( ch, "\n\r&wPon familia &gayuda&w para ver los posibles campos.\n\r" );
	guardaFamilia( fam, ACT_FAM_NORMAL );
	return;
}

void updateFamilias()
{
	FAMILIA_DATOS *fam;

	for( fam = primeraFamilia; fam; fam = fam->sig )
	{
		// Ponemos en orden las maximas de las estadisticas
		if( fam->oro >= fam->max_oro )
			fam->max_oro = fam->oro;
		if( fam->puntos >= fam->max_puntos )
			fam->max_puntos = fam->puntos;
		if( fam->mineral >= fam->max_mineral )
			fam->max_mineral = fam->mineral;
		if( fam->comida >= fam->max_comida )
			fam->max_comida = fam->comida;
		if( fam->combustible >= fam->max_combustible )
			fam->max_combustible = fam->combustible;
		if( fam->px >= fam->max_px )
			fam->max_px = fam->px;
		if( fam->conq >= fam->max_conq )
			fam->max_conq = fam->conq;
		if( fam->quest >= fam->max_quest )
			fam->max_quest = fam->quest;
		if( fam->areas >= fam->max_areas )
			fam->max_areas = fam->areas;
		if( fam->miembros >= fam->max_miembros )
			fam->max_miembros = fam->miembros;
	}

	guardaConfigFamilias();
}

void do_fadmitir( CHAR_DATA *ch, char *arg )
{
}

void do_fexpulsar( CHAR_DATA *ch, char *arg )
{
}

void do_familia( CHAR_DATA *ch, char *arg )
{
	FAMILIA_DATOS *fam;
	FAMILIA_DATOS *familia;
	bool prim = FALSE;
	int fam_cnt = 0;

	if( !ch )
	{
		fDebug( "No existe CHAR_DATA en la funcion do_familia( CHAR_DATA, char* )." );
		bug( "[%s:%d] No existe el puntero a ch.", __FILE__, __LINE__ );
		return;
	}

	ch_printf( ch, "\n\r&w/=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\\\n\r" );
	ch_printf( ch, "|                     &wFamilias de &rVampiroMud&w: &zMundo de tinieblas &w              |\n\r" );
	ch_printf( ch, "&w<=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=->\n\r" );

	for( fam = primeraFamilia; fam; fam = fam->sig )
	{
		guardaFamilia( fam, ACT_FAM_NORMAL );
		fam_cnt++;
	}
	if( fam_cnt == 0 )
	{
		ch_printf( ch, "&w<=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=->\n\r" );
		ch_printf( ch, "|                          Aun no existe ninguna familia.                      | \n\r\
|                          Te crees capaz de fundar una?                       |\n\r\
|                             Habla con un inmortal.                           |\n\r" );
	}
	else
	{
		ch_printf( ch, "&w|&g P&w| &g Familia           &w| &gLider&w         | &gMiem &w|&g Conq &w|&g Puntos &w|&g MC &w|&g AC &w|&g Est &w|\n\r" );
		ch_printf( ch, "&w|--|--------------------|---------------|------|------|--------|----|----|-----&w|\n\r" );
		for( fam = primeraFamilia; fam; fam = fam->sig )
		{
			if( posicionFamilia( fam ) <= 1 )
			{
				ch_printf( ch, "&w| &Y%1.1d&w|", posicionFamilia( fam ));
				prim = TRUE;
			}
			else if( posicionFamilia( fam ) <= 2 )
				ch_printf( ch, "&w| &w%1.1d&w|", posicionFamilia( fam ));
			else if( posicionFamilia( fam ) <= 3 )
				ch_printf( ch, "&w| &c%1.1d&w|", posicionFamilia( fam ));
			else if( posicionFamilia( fam ) >= 4 )
				ch_printf( ch, "&w| &r%1.1d&w|", posicionFamilia( fam ));


			ch_printf( ch, "  &g%-17.17s &w|", capitalize( fam->alias ) );


			ch_printf( ch, " &g%-13.13s &w|", capitalize( fam->lider ) );

			if( fam->miembros >= 1 )
				ch_printf( ch, " &g%4.4d &w|", fam->miembros );
			else
				ch_printf( ch, " &z%4.4d &w|", fam->miembros );

			if( fam->conq >= 1 )
				ch_printf( ch, " &g%4.4d &w|", fam->conq );
			else
				ch_printf( ch, " &z%4.4d &w|", fam->conq );

			if( fam->puntos <= 1 )
				ch_printf( ch, " &r%6.6d &w|", fam->puntos );
			else if( fam->puntos <= 5000 )
				ch_printf( ch, " &g%6.6d &w|", fam->puntos );
			else if( fam->puntos <= 100000 )
				ch_printf( ch, " &w%6.6d &w|", fam->puntos );
			else if( fam->puntos <= 150000 )
				ch_printf( ch, " &Y%6.6d &w|", fam->puntos );

			ch_printf( ch, "----|" );
			ch_printf( ch, "----|" );
			ch_printf( ch, "-----|" );

			ch_printf( ch, "\n\r" );
		}

		ch_printf( ch, "&w<=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=->\n\r");
		ch_printf( ch, "&w| En total existen &g%1.1d &wlineas de sangre en el mundo de tinieblas, no hay ninguna |\n\r", fam_cnt );
		ch_printf( ch, "&w| guerra declarada. " );

		if( prim == TRUE )
			ch_printf( ch, "                                                           |\n\r" );
		else
			ch_printf( ch, " &gNo&w existe ninguna familia predominante.                   |\n\r" );

	}
	ch_printf( ch, "&w\\=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-/\n\r" );

	guardaConfigFamilias();
	return;
}

void do_fvotar( CHAR_DATA *ch, char *arg )
{

}

// Funcion para pruebas :D
void do_famtest( CHAR_DATA *ch, char *arg )
{
	initDatabase( ch );
	return;
}


/*
    __    _                          ____  ____  __
   / /   (_)___  __  ___  __   _____/ __ \/ __ \/ /___
  / /   / / __ \/ / / / |/_/  / ___/ / / / / / / /_  /
 / /___/ / / / / /_/ />  <   / /  / /_/ / /_/ / / / /_
/_____/_/_/ /_/\__,_/_/|_|  /_/   \____/\____/_/ /___/

*/

