/****************************************************************************
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 ****************************************************************************
 *                        Finger and Wizinfo Module                         *
 ****************************************************************************/

#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "mud.h"

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

/* Begin wizinfo stuff - Samson 6-6-99 */

bool     check_parse_name        args( ( char *name, bool newchar ) );

WIZINFO_DATA * first_wizinfo;
WIZINFO_DATA * last_wizinfo;

/* Construct wizinfo list from god dir info - Samson 6-6-99 */
void add_to_wizinfo( char *name, WIZINFO_DATA *wiz )
{
   WIZINFO_DATA *wiz_prev;

   wiz->name = str_dup( name );
   if ( !wiz->email )
      wiz->email = str_dup( "No Fijado" );

   for( wiz_prev = first_wizinfo; wiz_prev; wiz_prev = wiz_prev->next )
      if( strcasecmp( wiz_prev->name, name ) >= 0 )
         break;

   if( !wiz_prev )
      LINK( wiz, first_wizinfo, last_wizinfo, next, prev );
   else
      INSERT( wiz, wiz_prev, first_wizinfo, next, prev );

   return;
}

void clear_wizinfo( bool bootup )
{
   WIZINFO_DATA *wiz, *next;

   if( !bootup )
   {
     for ( wiz = first_wizinfo; wiz; wiz = next )
     {
        next = wiz->next;
        UNLINK( wiz, first_wizinfo, last_wizinfo, next, prev );
	  DISPOSE( wiz->name );
	  DISPOSE( wiz->email );
        DISPOSE( wiz );
     }
   }

   first_wizinfo = NULL;
   last_wizinfo = NULL;

   return;
}

void fread_info( WIZINFO_DATA *wiz, FILE *fp )
{
   char *word;
   bool fMatch;

     for ( ; ; )
     {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'E':
	    KEY( "Email", 	wiz->email,		fread_string_nohash( fp ) );
	    if ( !str_cmp( word, "End" ) )
		return;
	    break;

	case 'I':
	    KEY( "ICQ",	wiz->icq,	fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level", wiz->level,	fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	    fread_to_eol( fp );
    }
}

void build_wizinfo( bool bootup )
{
   DIR *dp;
   struct dirent *dentry;
   FILE *fp;
   WIZINFO_DATA *wiz;
   char buf[MSL];

   clear_wizinfo( bootup ); /* Clear out the table before rebuilding a new one */

   dp = opendir( GOD_DIR );

   dentry = readdir( dp );

   while ( dentry )
   {
      if ( dentry->d_name[0] != '.' )
      {
	  sprintf( buf, "%s%s", GOD_DIR, dentry->d_name );
	  fp = fopen( buf, "r" );
	  if ( fp )
	  {
	    CREATE( wiz, WIZINFO_DATA, 1 );
          fread_info( wiz, fp );
	    add_to_wizinfo( dentry->d_name, wiz );
	    FCLOSE( fp );
	  }
      }
      dentry = readdir( dp );
   }
   closedir( dp );
   return;
}

/*
 * Wizinfo information.
 * Added by Samson on 6-6-99
 */
void do_wizinfo( CHAR_DATA *ch, char *argument )
{
   WIZINFO_DATA *wiz;
   char buf[MSL];

   send_to_pager( "Informacion de contacto entre inmortales:\n\r\n\r", ch );
   send_to_pager( "Nombre       Email                             ICQ#\n\r", ch );
   send_to_pager( "------------+---------------------------------+----------\n\r", ch );

   for ( wiz = first_wizinfo; wiz; wiz = wiz->next )
   {
      sprintf( buf, "%-12s %-33s %10d", wiz->name, wiz->email, wiz->icq );
	strcat( buf, "\n\r" );
      send_to_pager( buf, ch );
   }
   return;
}

/* End wizinfo stuff - Samson 6-6-99 */

const char* rango[14] =
{
        "", "HeRalDo", "LeGeNDaRio", "SeR DiViNo", "DeSTRuCToR", "CeLeSTiaL", "iNHuMaNo", "eXPeRTo", "MaeSTRo", "SaBio", "iLuMiNaDo", "eSTuDiaNTe",
        "aPReNDiZ", "NoVaTo"
};

const char* gene[14] =
{
        "", "PRiMeRa GeNeRaCioN", "SeGuNDa GeNeRaCioN", "aNTeDiLuViaNo", "MaTuSaLeN", "QuiNTa GeNeRaCioN", "SeXTa GeNeRaCioN", "sePTiMa GeNeRaCioN",
        "oCTaVa GeNeRaCioN", "NoVeNa GeNeRaCioN", "DeCiMa GeNeRaCioN", "uNDeCiMa GeNeRaCioN", "DuoDeCiMa GeNwRaCioN", "NeoNaTo"
};

void fread_finger( CHAR_DATA *ch, FILE *fp, char *laston )
{
    char *word;
    char *email = NULL;
    char *homepage = NULL;
    char *name = NULL;
    char *site = NULL;
    char *title = NULL;
    char *bio = NULL;
    char *authed = NULL;
    char *temp = NULL;
    char *sire = NULL;
    char *familia = NULL;
    sh_int class = 0, level = 0, race = 0, sex = 0;
    int icq = 0, flags = 0, played = 0;
    int coeficiente = 0, remort = 0, generacion = 13;
    int chiquillos = 0, pkills = 0, pdeaths = 0;
    int genepkills = 0, mdeaths = 0, conquistas = 0;
    int mkills = 0;
    sh_int file_ver = 0;
    bool fMatch;

    for ( ; ; )
    {
	word   = feof( fp ) ? "End" : fread_word( fp );
	fMatch = FALSE;

	switch ( UPPER(word[0]) )
	{
	case '*':
	    fMatch = TRUE;
	    fread_to_eol( fp );
	    break;

	case 'A':
	    KEY( "AuthedBy",	authed,	fread_string( fp ) );
	    break;

	case 'B':
	    KEY( "Bio",		bio,		fread_string( fp ) );
	    break;

      case 'C':
    	    KEY( "Class",	    class,		fread_number( fp ) );
            KEY( "Coeficiente",     coeficiente,        fread_number( fp ) );
            KEY( "Conquistas",      conquistas,         fread_number( fp ) );
	    break;

      case 'D':
	    KEY( "Description",	temp,		fread_string( fp ) );
	    break;

      case 'E':
	    if ( !strcmp( word, "End" ) )
	        goto finger_display;
	    KEY( "Email",	    email,		fread_string_nohash( fp ) );
	    break;

	case 'F':
	    KEY( "Flags",		flags,	fread_number( fp ) );
            KEY( "Familia",             familia,    fread_string( fp ) );
	    break;

        case 'G':
            KEY( "Generacion",  generacion,      fread_number( fp ) );
            break;

   	case 'H':
	    KEY( "Homepage",	homepage,	fread_string_nohash( fp ) );
	    break;

	case 'I':
	    KEY( "ICQ",		icq,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",		level,	fread_number( fp ) );
	    break;

        case 'M':
            KEY( "MDeaths",	mdeaths,	fread_number( fp ) );
            KEY( "MKills",      mkills,         fread_number( fp ) );
            break;

	case 'N':
	    KEY( "Name", 		name, 	fread_string( fp ) );
            KEY("NumChiquillos", chiquillos, fread_number( fp ));
	    break;

	case 'P':
	    KEY( "Played",	played,		fread_number( fp ) );
            KEY( "Pkills",      pkills,         fread_number( fp ) );
            KEY( "Pdeaths",     pdeaths,        fread_number( fp ) );
            KEY( "PksGeneracion", genepkills,     fread_number( fp ) );
	    break;

	case 'R':
	    KEY( "Race",        race,		fread_number( fp ) );
            KEY( "Renacimientos",       remort,     fread_number( fp ) );
	    break;

	case 'S':
	    KEY( "Sex",		sex,		fread_number( fp ) );
            KEY( "Sire",        sire,           fread_string( fp ) );
          if ( !strcmp( word, "Site" ) )
	    {
		site = STRALLOC( fread_word( fp ) );
		fMatch = TRUE;
	    }
	    break;

	case 'T':
	    KEY( "Title",		title,	fread_string( fp ) );
	    break;

	case 'V':
	    KEY( "Version",	file_ver,	fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	   fread_to_eol( fp );
    }

/* Extremely ugly and disgusting goto hack, if there's a better way to do this, I'd sure like to know - Samson */

finger_display:

    if ( IS_SET( flags, PCFLAG_PRIVACY ) && !IS_IMMORTAL(ch) )
    {
	send_to_char( "La informacion de este jugador es privada.\n\r", ch );
	return;
    }

    if ( level >= LEVEL_IMMORTAL && !IS_HERO(ch) )
    {
	send_to_char( "No puedes hacerle finger a un Inmortal.\n\r", ch );
	return;
    }

    send_to_char("&g=====================================================================\n\r", ch);
    ch_printf(ch, "&gInformacion de &r%s &g%s\n\r", name, title);
    send_to_char("&g=====================================================================\n\r", ch);
    ch_printf(ch, "&gNivel   : &c%-20d &g      Clase: &c%s\n\r", level, capitalize( npc_class[class] ) );
    ch_printf(ch, "&gSexo    : &c%-20s &gPertenencia: &c%s\n\r",
                sex == SEX_MALE   ? "Hombre"   :
                sex == SEX_FEMALE ? "Mujer" : "Neutral",
		    capitalize( npc_race[race] ) );
    if( familia != NULL)
    ch_printf(ch, "&gFamilia : &c%-20s  &gConquistas: &c%d\n\r", familia, conquistas );
    else
    ch_printf(ch, "&gFamilia : &c%-20s  &gConquistas: &c%d\n\r", "Es un pobre Huerfano", conquistas );

    if(class == 4 || class == 5 || class == 9 || class == 10 || class == 11 || class == 12 || class == 13 || class == 14)
    ch_printf(ch, "&gSire    : &c%-20s      &g  Es un &c%s\n\r", sire, gene[generacion] );
    else if(class == 3)
    ch_printf(ch, "&gMaestro : &c%-20s      &g  Es un &c%s\n\r", "No abrazado/adiestrado", rango[generacion] );
    else
    ch_printf(ch, "&gMestro  : &c%-20s      &g  Es un &c%s\n\r", sire, rango[generacion] );

    ch_printf(ch, "&gPkills  : &c%-20d     &gPdeaths: &c%d\n\r", pkills, pdeaths );
    if(class == 4 || class == 5 || class == 9 || class == 10 || class == 11 || class == 12 || class == 13 || class == 14){
    ch_printf(ch, "&gMdeaths : &c%-20d     &g Genepk: &c%d\n\r", mdeaths, genepkills );
    ch_printf(ch, "&gMkills  : &c%-20d &gCoeficiente: &c%d\n\r", mkills, coeficiente );
    ch_printf(ch, "&gRemort  : &c%-20d  &gChiquillos: &c%d\n\r", remort, chiquillos );}
    else{
    ch_printf(ch, "&gMdeaths : &c%-20d     &gRangopk: &c%d\n\r", mdeaths, genepkills );
    ch_printf(ch, "&gMkills  : &c%-20d &gCoeficiente: &c%d\n\r", mkills, coeficiente );
    ch_printf(ch, "&gRemort  : &c%-20d  &gDiscipulos: &c%d\n\r", remort, chiquillos );}

    ch_printf(ch, "&gHomepage: &c%-20s\n\r", homepage ? homepage : "No especificada" );
    ch_printf(ch, "&gEmail   : &c%s\n\r", email ? email : "No especificada" );
    ch_printf(ch, "&gICQ#    : &c%d\n\r", icq );
    send_to_char("&g=====================================================================\n\r", ch);
    ch_printf(ch, "\n\r&gUltima conexion : &c%s\n\r", laston );
    if ( IS_IMMORTAL(ch) )
    {
	send_to_char( "&gInformacion para Inmortales\n\r", ch );
	send_to_char("&g=====================================================================\n\r", ch);
        ch_printf( ch, "&gUltima IP     : &c%s\n\r", site );
	ch_printf( ch, "&gTiempo jugado : &c%d horas\n\r", ( played / 3600 ) );
	ch_printf( ch, "&gAutorizado por: &c%s\n\r", authed ? authed : ( sysdata.WAIT_FOR_AUTH ? "No Autorizado" : "El Codigo" ) );
	ch_printf( ch, "&gPrivacidad    : &c%s\n\r",
 	   IS_SET( flags, PCFLAG_PRIVACY ) ? "Activo" : "Inactivo" );
    }
    ch_printf(ch, "&gBio:\n\r&c%s\n\r", bio ? bio : "No creada" );

    STRFREE( site );
    return;
}

void read_finger( CHAR_DATA *ch, char *argument )
{
  FILE *fpFinger;
  char fingload[MAX_INPUT_LENGTH];
  char *laston = NULL;
  struct stat fst;

  sprintf( fingload, "%s%c/%s", PLAYER_DIR, tolower(argument[0]),	capitalize( argument ) );

  /* Bug fix here provided by Senir to stop /dev/null crash */
  if ( stat( fingload, &fst ) == -1 || !check_parse_name( capitalize( argument ), FALSE ))
  {
     send_to_char( "&gNo existe ese jugador.\n\r", ch );
     return;
  }

  laston = ctime( &fst.st_mtime );

  if ( stat( fingload, &fst ) != -1 )
  {
    if ( ( fpFinger = fopen ( fingload, "r" ) ) != NULL )
    {
      for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fpFinger );

	    if ( letter != '#' )
	       continue;

	    word = fread_word( fpFinger );

          if ( !str_cmp( word, "End" ) )
		break;

	    if ( !str_cmp( word, "PLAYER" ) )
	    	fread_finger( ch, fpFinger, laston );
	    else
	    if ( !str_cmp( word, "END" ) )	/* Done		*/
		break;
	}
	FCLOSE( fpFinger );
    }
  }
  return;
}

/* Finger snippet courtesy of unknown author. Installed by Samson 4-6-98 */
/* File read/write code redone using standard Smaug I/O routines - Samson 9-12-98 */
/* Data gathering now done via the pfiles, eliminated separate finger files - Samson 12-21-98 */
void do_finger( CHAR_DATA *ch, char *argument )
{
  CHAR_DATA *victim;
  char buf[MSL];

  if( IS_NPC(ch) )
  {
     send_to_char( "Los mobs no pueden usar el comando finger.\n\r", ch );
     return;
  }

  if ( !argument || argument[0] == '\0' )
  {
      send_to_char("Finger a quien?\n\r", ch );
      return;
  }

  strcpy( buf, "0." );
  strcat( buf, argument );
  victim = get_char_world( ch, buf );

  if( !victim )
  {
      read_finger( ch, argument );
      return;
  }

#ifdef OVERLANDCODE
  if ( !can_see( ch, victim, TRUE ) )
#else
  if ( !can_see( ch, victim ) )
#endif
  {
	read_finger( ch, argument );
      return;
  }

  if ( IS_SET( victim->pcdata->flags, PCFLAG_PRIVACY ) && !IS_IMMORTAL(ch) )
  {
	ch_printf( ch, "Los datos de %s son privados.\n\r", victim->name );
	return;
  }

  if ( IS_IMMORTAL(victim) && !IS_HERO(ch) )
  {
	send_to_char( "No puedes hacer finger a un inmortal.\n\r", ch );
	return;
  }

  send_to_char("&g=====================================================================\n\r", ch);
    ch_printf(ch, "&gInformacion de &r%s&g%s\n\r", victim->name, victim->pcdata->title);
    send_to_char("&g=====================================================================\n\r", ch);
  ch_printf(ch, "&gAnyos   : &c%d\n\r",get_age( victim ) );
  ch_printf(ch, "&gNivel   : &c%-20d &g      Clase: &c%s\n\r", victim->level, capitalize( get_class(victim) ) );
  ch_printf(ch, "&gSexo    : &c%-20s &gPertenencia: &c%s\n\r",
                victim->sex == SEX_MALE   ? "Hombre"   :
                victim->sex == SEX_FEMALE ? "Mujer" : "Neutral",
		    capitalize( get_race(victim) ) );
    if(victim->pcdata->clan)
    ch_printf(ch, "&gFamilia : &c%-20s  &gConquistas: &c%d\n\r", victim->pcdata->clan->name, victim->conquistas );
    else
    ch_printf(ch, "&gFamilia : &c%-20s  &gConquistas: &c%d\n\r", "Es un pobre Huerfano", victim->conquistas );

    if(IS_VAMPIRE(victim))
    ch_printf(ch, "&gSire    : &c%-20s      &g  Es un &c%s\n\r", victim->sire_ch, gene[victim->generacion] );
    else if(victim->class == 3)
    ch_printf(ch, "&gMaestro : &c%-20s      &g  Es un &c%s\n\r", "No abrazado/adiestrado", rango[victim->generacion] );
    else
    ch_printf(ch, "&gMestro  : &c%-20s      &g  Es un &c%s\n\r", victim->sire_ch, rango[victim->generacion] );

    ch_printf(ch, "&gPkills  : &c%-20d     &gPdeaths: &c%d\n\r", victim->pcdata->pkills, victim->pcdata->pdeaths );
    if(IS_VAMPIRE(victim)){
    ch_printf(ch, "&gMdeaths : &c%-20d     &g Genepk: &c%d\n\r", victim->pcdata->mdeaths, victim->pcdata->genepkills );
    ch_printf(ch, "&gMkills  : &c%-20d &gCoeficiente: &c%d\n\r", victim->pcdata->mkills, victim->pcdata->coeficiente );
    ch_printf(ch, "&gRemort  : &c%-20d  &gChiquillos: &c%d\n\r", victim->pcdata->renacido, victim->num_chiquillos );}
    else{
    ch_printf(ch, "&gMdeaths : &c%-20d     &gRangopk: &c%d\n\r", victim->pcdata->mdeaths, victim->pcdata->genepkills );
    ch_printf(ch, "&gMkills  : &c%-20d &gCoeficiente: &c%d\n\r", victim->pcdata->mkills, victim->pcdata->coeficiente );
    ch_printf(ch, "&gRemort  : &c%-20d  &gDiscipulos: &c%d\n\r", victim->pcdata->renacido, victim->num_chiquillos );}

  ch_printf(ch, "&gHomepage: &c%s\n\r", victim->pcdata->homepage != NULL ? victim->pcdata->homepage : "Not specified" );
  ch_printf(ch, "&gEmail   : &c%s\n\r", victim->pcdata->email != NULL ? victim->pcdata->email : "Not specified" );
  ch_printf(ch, "&gICQ#    : &c%d\n\r", victim->pcdata->icq );
    send_to_char("&g=====================================================================\n\r", ch);
  ch_printf(ch, "\n\r&gUltima conexion: &c%s\n\r", (char *) ctime( &ch->logon ) );
  if ( IS_IMMORTAL(ch) )
  {
     char ipbuf[MSL];

	send_to_char( "&gInformacion para Inmortales\n\r", ch );
	  send_to_char("&g=====================================================================\n\r", ch);
     if ( victim->desc )
	  sprintf( ipbuf, "%s@%s", victim->desc->user, victim->desc->host );
     else
	  strcpy( ipbuf, "(Link-Dead)" );
     ch_printf(ch, "&gIP Info: &c%s\n\r", ipbuf );
	ch_printf( ch, "&gTime played   : &c%d hours\n\r", GET_TIME_PLAYED( victim ) );
      ch_printf( ch, "&gAuthorized by : &c%s\n\r",
	   victim->pcdata->authed_by ? victim->pcdata->authed_by : ( sysdata.WAIT_FOR_AUTH ? "Not Authed" : "The Code" ) );
      ch_printf( ch, "&gPrivacy Status: &c%s\n\r", IS_SET( victim->pcdata->flags, PCFLAG_PRIVACY ) ? "Enabled" : "Disabled" );
  }
  ch_printf(ch, "&gBio:\n\r&c%s\n\r", victim->pcdata->bio ? victim->pcdata->bio : "Not created" );
  return;
}

/* Added a clone of homepage to let players input their email addy - Samson 4-18-98 */
void do_email( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if ( IS_NPC(ch) )
	return;

    if ( argument[0] == '\0' )
    {
	if ( !ch->pcdata->email )
	  ch->pcdata->email = str_dup( "" );
	ch_printf( ch, "Tu direccion de email es: %s\n\r",	show_tilde( ch->pcdata->email ) );
	return;
    }

    if ( !str_cmp( argument, "borrar" ) )
    {
	if ( ch->pcdata->email )
	  DISPOSE(ch->pcdata->email);
	ch->pcdata->email = str_dup("");

      if ( IS_IMMORTAL( ch ) );
      {
	  save_char_obj( ch );
	  build_wizinfo( FALSE );
      }

	send_to_char( "Direccion Email borrada.\n\r", ch );
	return;
    }

    strcpy( buf, argument );

    if ( strlen(buf) > 70 )
	buf[70] = '\0';

    hide_tilde( buf );
    if ( ch->pcdata->email )
      DISPOSE(ch->pcdata->email);
    ch->pcdata->email = str_dup(buf);
    if ( IS_IMMORTAL( ch ) );
    {
	save_char_obj( ch );
	build_wizinfo( FALSE );
    }
    send_to_char( "Direccion Email fijada.\n\r", ch );
}

void do_icq_number( CHAR_DATA *ch, char *argument )
{
    int icq;

    if ( IS_NPC( ch ) )
	return;

    if ( argument[0] == '\0' )
    {
	if ( !ch->pcdata->icq )
	  ch->pcdata->icq = 0;
	ch_printf( ch, "Tu ICQ# es: %d\n\r", ch->pcdata->icq );
	return;
    }

    if ( !str_cmp( argument, "borrar" ) )
    {
	ch->pcdata->icq = 0;

      if ( IS_IMMORTAL( ch ) );
      {
	  save_char_obj( ch );
	  build_wizinfo( FALSE );
      }

	send_to_char( "ICQ# borrado.\n\r", ch );
	return;
    }

    if ( !is_number( argument ) )
    {
	send_to_char( "El argumento debe de ser numerico.\n\r", ch );
	return;
    }

    icq = atoi( argument );

    if ( icq < 1 )
    {
	send_to_char( "El rango valido es siempre mayor de 0.\n\r", ch );
	return;
    }

    ch->pcdata->icq = icq;

    if ( IS_IMMORTAL( ch ) );
    {
	save_char_obj( ch );
	build_wizinfo( FALSE );
    }

    send_to_char( "ICQ# fijado.\n\r", ch );
    return;
}

void do_homepage( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if ( IS_NPC(ch) )
	return;

    if ( !argument || argument[0] == '\0' )
    {
	if ( !ch->pcdata->homepage )
	  ch->pcdata->homepage = str_dup( "" );
	ch_printf( ch, "Tu homepage es: %s\n\r", show_tilde( ch->pcdata->homepage ) );
	return;
    }

    if ( !str_cmp( argument, "borrar" ) )
    {
	if ( ch->pcdata->homepage )
	  DISPOSE(ch->pcdata->homepage);
	ch->pcdata->homepage = str_dup("");
	send_to_char( "Homepage borrada.\n\r", ch );
	return;
    }

    if ( strstr( argument, "://" ) )
	strcpy( buf, argument );
    else
	sprintf( buf, "http://%s", argument );
    if ( strlen(buf) > 70 )
	buf[70] = '\0';

    hide_tilde( buf );
    if ( ch->pcdata->homepage )
      DISPOSE(ch->pcdata->homepage);
    ch->pcdata->homepage = str_dup(buf);
    send_to_char( "Homepage fijada.\n\r", ch );
}

void do_privacy( CHAR_DATA *ch, char *argument )
{
   if ( IS_NPC( ch ) )
   {
	send_to_char( "Los mobs no pueden tener privacidad.\n\r", ch );
	return;
   }

   TOGGLE_BIT( ch->pcdata->flags, PCFLAG_PRIVACY );

   if ( IS_SET( ch->pcdata->flags, PCFLAG_PRIVACY ) )
   {
	send_to_char( "&wLa informacion de tu finger ahora es &gPRIVADA&w.\n\r", ch );
	return;
   }
   else
   {
	send_to_char( "&wLa informacion de tu fnger ahora es &gPUBLICA&w.\n\r", ch );
	return;
   }
}
