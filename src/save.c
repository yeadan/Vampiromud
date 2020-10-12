/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------
 *		     Character saving and loading module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
  #include <dirent.h>
#endif

#include "mud.h"
#include "familias.h"

/*
 * Increment with every major format change.
 */
#define SAVEVERSION	3

/*
 * Array to keep track of equipment temporarily.		-Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][8];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int file_ver;

/*
 * Externals
 */
void fwrite_comments( CHAR_DATA *ch, FILE *fp );
void fread_comment( CHAR_DATA *ch, FILE *fp );

/*
 * Array of containers read for proper re-nesting of objects.
 */
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

/*
 * Local functions.
 */
void	fwrite_char	args( ( CHAR_DATA *ch, FILE *fp ) );
void	fread_char	args( ( CHAR_DATA *ch, FILE *fp, bool preload) );
void	write_corpses	args( ( CHAR_DATA *ch, char *name, OBJ_DATA *objrem ) );


#ifdef WIN32                    /* NJG */
UINT timer_code = 0;            /* needed to kill the timer */
/* Note: need to include: WINMM.LIB to link to timer functions */
void caught_alarm();
void CALLBACK alarm_handler(
	UINT IDEvent,		/* identifies timer event */
	UINT uReserved,		/* not used */
	DWORD dwUser,		/* application-defined instance data */
	DWORD dwReserved1,	/* not used */
	DWORD dwReserved2)	/* not used */
{
    caught_alarm();
}

void kill_timer()
{
    if (timer_code)
	timeKillEvent(timer_code);
    timer_code = 0;
}

#endif



/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA *ch )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int x,y;

    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	    save_equipment[x][y] = NULL;
    for ( obj = ch->first_carrying; obj; obj = obj->next_content )
	if ( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
	{

	    if ( get_trust( ch ) >= obj->level )
	    {
		for ( x = 0; x < MAX_LAYERS; x++ )
		   if ( !save_equipment[obj->wear_loc][x] )
		   {
			save_equipment[obj->wear_loc][x] = obj;
			break;
		   }
		if ( x == MAX_LAYERS )
		{
		    sprintf( buf, "%s had on more than %d layers of clothing in one location (%d): %s",
			ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
		    bug( buf, 0 );
		}
	    }
	    else
	    {
	       sprintf( buf, "%s dejo atras %s:  ch->level = %d  obj->level = %d",
		ch->name, obj->name,
	       	ch->level, obj->level );
	       bug( buf, 0 );
	    }
	    unequip_char(ch, obj);
	}
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char( CHAR_DATA *ch )
{
    int x,y;

    for ( x = 0; x < MAX_WEAR; x++ )
	for ( y = 0; y < MAX_LAYERS; y++ )
	   if ( save_equipment[x][y] != NULL )
	   {
		if ( quitting_char != ch )
		   equip_char(ch, save_equipment[x][y], x);
		save_equipment[x][y] = NULL;
	   }
	   else
		break;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA *ch )
{
    char strsave[MAX_INPUT_LENGTH];
    char strback[MAX_INPUT_LENGTH];
    FILE *fp;

    if ( !ch )
    {
	bug( "Save_char_obj: null ch!", 0 );
	return;
    }

    if ( IS_NPC(ch) || ch->level < 2 )
	return;

    saving_char = ch;

    if ( ch->desc && ch->desc->original )
	ch = ch->desc->original;

    de_equip_char( ch );

    ch->save_time = current_time;
    sprintf( strsave, "%s%c/%s",PLAYER_DIR,tolower(ch->pcdata->filename[0]),
				 capitalize( ch->pcdata->filename ) );

    /*
     * Auto-backup pfile (can cause lag with high disk access situtations).
     */
    /* Backup of each pfile on save as above can cause lag in high disk
       access situations on big muds like Realms.  Quitbackup saves most
       of that and keeps an adequate backup -- Blodkai, 10/97 */

    if ( IS_SET( sysdata.save_flags, SV_BACKUP ) ||
       ( IS_SET( sysdata.save_flags, SV_QUITBACKUP ) && quitting_char == ch ))
    {
        sprintf( strback,"%s%c/%s",BACKUP_DIR,tolower(ch->pcdata->filename[0]),
                                 capitalize( ch->pcdata->filename ) );
        rename( strsave, strback );
    }

    /*
     * Save immortal stats, level & vnums for wizlist		-Thoric
     * and do_vnums command
     *
     * Also save the player flags so we the wizlist builder can see
     * who is a guest and who is retired.
     */
    if ( ch->level >= LEVEL_IMMORTAL )
    {
      sprintf( strback, "%s%s", GOD_DIR, capitalize( ch->pcdata->filename ) );

      if ( ( fp = fopen( strback, "w" ) ) == NULL )
      {
	perror( strsave );
	bug( "Save_god_level: fopen", 0 );
      }
      else
      {
	fprintf( fp, "Level        %d\n", ch->level );
	fprintf( fp, "Pcflags      %d\n", ch->pcdata->flags );
        if( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
	   fprintf( fp, "Homepage    %s~\n", ch->pcdata->homepage );
	if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	  fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
	  				       ch->pcdata->r_range_hi	);
	if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	  fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
	  				       ch->pcdata->o_range_hi	);
	if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	  fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
	  				       ch->pcdata->m_range_hi	);
        if( ch->pcdata->email && ch->pcdata->email[0] != '\0' )
	  fprintf( fp, "Email        %s~\n", ch->pcdata->email );
      if ( ch->pcdata->icq > 0 )
	  fprintf( fp, "ICQ          %d\n", ch->pcdata->icq );
	fprintf( fp, "End\n" );
	fclose( fp );
      }
    }

    if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
    {
	perror( strsave );
	bug( "Save_char_obj: fopen", 0 );
    }
    else
    {
	bool ferr;

	fchmod(fileno(fp), S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP | S_IROTH|S_IWOTH);
	fwrite_char( ch, fp );
        if ( ch->morph )
        	fwrite_morph_data ( ch, fp );
	if ( ch->first_carrying )
	  fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY );

	if ( sysdata.save_pets && ch->pcdata->pet )
		fwrite_mobile( fp, ch->pcdata->pet );
	if ( ch->comments )                 /* comments */
	  fwrite_comments( ch, fp );        /* comments */
	fprintf( fp, "#END\n" );
	ferr = ferror(fp);
	fclose( fp );
	if (ferr)
	{
	  perror(strsave);
	  bug("Error writing temp file for %s -- not copying", strsave);
	}
	else
	  rename(TEMP_FILE, strsave);
    }

    re_equip_char( ch );

    quitting_char = NULL;
    saving_char   = NULL;
    return;
}



/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{

    AFFECT_DATA *paf;
    DISCI_AFFECT *daf;
    ESFERA_AFFECT *eaf;
    ESFERA *esfera;
    NIVEL *nivel;
    int sn, track;
    sh_int pos;
    int i;
    int cnt;
    SKILLTYPE *skill = NULL;

    fprintf( fp, "#PLAYER\n"		);

    fprintf( fp, "Version      %d\n",   SAVEVERSION		);
    fprintf( fp, "Name         %s~\n",	ch->name		);
    if ( ch->description[0] != '\0' )
      fprintf( fp, "Description  %s~\n",	ch->description	);
    fprintf( fp, "Camuflaje    %s~\n",          ch->camuflaje );  /* OJO No soportado por SMAUG1.4 */
    fprintf( fp, "Modoficha     %d\n",          ch->modo_ficha ); /* Modo de la ficha del jugador. OJO No soportado por SMAUG1.4 */
    fprintf( fp, "Conquistas   %d\n",           ch->conquistas ); /* Conquistas del jugador */
    fprintf( fp, "Coeficiente  %d\n",           ch->pcdata->coeficiente ); /* Coeficiente de batalla */
    fprintf( fp, "Sex          %d\n",	ch->sex			);
    fprintf( fp, "Class        %d\n",	ch->class		);
    fprintf( fp, "Race         %d\n",	ch->race		);
    fprintf( fp, "Languages    %d %d\n", ch->speaks, ch->speaking );
    fprintf( fp, "Level        %d\n",	ch->level		);
    fprintf( fp, "MaxLevel     %d\n",   ch->pcdata->max_level   ); /* Maximo nivel alcanzado por el jugador */
    fprintf( fp, "Generacion   %d\n",  ch->generacion      );   /*SiGo*/ /* OJO No soportado por SMAUG1.4 */
    if (ch->tmplevel > LEVEL_HERO)
      fprintf( fp, "tmpLevel        %d\n",	ch->tmplevel		);    /* Otra ves el puto comentario este joder pa kanear mortales es esto xD */
    if (ch->tmptrust > LEVEL_HERO)
      fprintf( fp, "tmpTrust        %d\n",	ch->tmptrust		);

    if ( xIS_SET( ch->act, PLR_ABRAZADO ) )
    fprintf( fp, "Contabrazo     %d\n",   ch->cnt_abrazo ); /* SiGo */ /* OJO No soportado por SMAUG1.4 */

    if ( xIS_SET( ch->act, PLR_PKTOTAL) )
    fprintf( fp, "CntPk          %d\n",   ch->cnt_pktotal );

   if ( xIS_SET( ch->act, PLR_RENACIDO ) )
   {
      fprintf( fp, "Renacimientos      %d\n",         ch->pcdata->renacido   );/* OJO No soportado por SMAUG1.4 */
      fprintf( fp, "Vida_anterior      %d\n",         ch->pcdata->vida_ant   );/* OJO No soportado por SMAUG1.4 */
      fprintf( fp, "Mana_anterior      %d\n",         ch->pcdata->mana_ant   );/* OJO No soportado por SMAUG1.4 */
      fprintf( fp, "Move_anterior      %d\n",         ch->pcdata->mov_ant    );/* OJO No soportado por SMAUG1.4 */
   }

   if( xIS_SET(ch->act, PLR_DOMINADO ) )
   fprintf( fp, "CntDominacion         %d\n",         ch->pcdata->cnt_dominacion );

   /* OJO No soportado por SMAUG1.4 */
    fprintf( fp, "Habilidades %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
       ch->pcdata->habilidades[0],ch->pcdata->habilidades[1],ch->pcdata->habilidades[2],ch->pcdata->habilidades[3],ch->pcdata->habilidades[4], ch->pcdata->habilidades[5],ch->pcdata->habilidades[6],
       ch->pcdata->habilidades[7],ch->pcdata->habilidades[8],ch->pcdata->habilidades[9], ch->pcdata->habilidades[10],ch->pcdata->habilidades[11],ch->pcdata->habilidades[12],ch->pcdata->habilidades[13],
       ch->pcdata->habilidades[14], ch->pcdata->habilidades[15],ch->pcdata->habilidades[16],ch->pcdata->habilidades[17],ch->pcdata->habilidades[18],ch->pcdata->habilidades[19], ch->pcdata->habilidades[20],
       ch->pcdata->habilidades[21],ch->pcdata->habilidades[22],ch->pcdata->habilidades[23],ch->pcdata->habilidades[24], ch->pcdata->habilidades[25],ch->pcdata->habilidades[26],ch->pcdata->habilidades[27],
       ch->pcdata->habilidades[28],ch->pcdata->habilidades[29], ch->pcdata->habilidades[30],ch->pcdata->habilidades[31],ch->pcdata->habilidades[32], ch->pcdata->habilidades[33] );

    fprintf( fp, "DisciAdq     %d\n",   ch->pcdata->disci_adquirir);/* OJO No soportado por SMAUG1.4 */
    fprintf( fp, "DisciPts     %d\n",   ch->pcdata->disci_puntos );/* OJO No soportado por SMAUG1.4 */
    /* OJO No soportado por SMAUG1.4 */
        if ( ch->sire_ch == NULL )
        {
                ch->sire_ch = "Desconocido";
                fprintf( fp, "Sire      %s~\n",        ch->sire_ch );
         }
         else
         fprintf( fp, "Sire      %s~\n",        ch->sire_ch );

    fprintf( fp, "Apartados        %d ",  MAX_BOARD      );  /* SiGo */
    for (i = 0; i < MAX_BOARD; i++)
 		fprintf (fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i]);
 	fprintf (fp, "\n");

    fprintf( fp, "Played       %d\n",
	ch->played + (int) (current_time - ch->logon)		);
    fprintf( fp, "Room         %d\n",
	(  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	&& ch->was_in_room )
	    ? ch->was_in_room->vnum
	    : ch->in_room->vnum );

    fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
	ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    fprintf( fp, "Gold         %d\n",	ch->gold		);
    fprintf( fp, "Exp          %d\n",	ch->exp			);
    if ( ch->level >= LEVEL_AVATAR )
    fprintf( fp, "Expacumulada   %d\n",  ch->exp_acumulada  );
    fprintf( fp, "Revision       %d\n", ch->revision );
    fprintf( fp, "Height          %d\n",	ch->height	);
    fprintf( fp, "Weight          %d\n",	ch->weight	);
    if ( !xIS_EMPTY(ch->act) )
      fprintf( fp, "Act          %s\n", print_bitvector(&ch->act) );
    if ( !xIS_EMPTY(ch->affected_by) )
      fprintf( fp, "AffectedBy   %s\n",	print_bitvector(&ch->affected_by) );
    if ( !xIS_EMPTY(ch->afectado_por) )
      fprintf( fp, "Affect_disciplina %s\n", print_bitvector(&ch->afectado_por) );
    if ( !xIS_EMPTY(ch->no_affected_by) )
      fprintf( fp, "NoAffectedBy %s\n",	print_bitvector(&ch->no_affected_by) );
    if( ch->class == CLASS_MAGE || ch->class == CLASS_CLERIC)
    if ( !xIS_EMPTY(ch->esferas))
      fprintf( fp, "Esferas %s\n", print_bitvector(&ch->esferas));

    /*
     * Strip off fighting positions & store as
     * new style (pos>=100 flags new style in character loading)
     */
    pos = ch->position;
    if (  pos == POS_BERSERK
       || pos == POS_AGGRESSIVE
       || pos == POS_FIGHTING
       || pos == POS_DEFENSIVE
       || pos == POS_EVASIVE
       )
    pos = POS_STANDING;
    pos +=100;
    fprintf( fp, "Position     %d\n", pos);

    fprintf( fp, "Style     %d\n", ch->style);

    fprintf( fp, "Practice     %d\n",	ch->practice		);
    fprintf( fp, "SavingThrows %d %d %d %d %d\n",
    		  ch->saving_poison_death,
		  ch->saving_wand,
    		  ch->saving_para_petri,
    		  ch->saving_breath,
    		  ch->saving_spell_staff			);
    fprintf( fp, "Alignment    %d\n",	ch->alignment		);
    fprintf( fp, "Favor	       %d\n",	ch->pcdata->favor	);
    fprintf( fp, "Balance        %d\n", ch->pcdata->balance );
    fprintf( fp, "Glory        %d\n",   ch->pcdata->quest_curr  );
    fprintf( fp, "MGlory       %d\n",   ch->pcdata->quest_accum );
    fprintf( fp, "Hitroll      %d\n",	ch->hitroll		);
    fprintf( fp, "Damroll      %d\n",	ch->damroll		);
    fprintf( fp, "Armor        %d\n",	ch->armor		);
    /*if (ch->nextquest != 0)
        fprintf( fp, "QuestNext %d\n",  ch->nextquest   );
    else if (ch->countdown != 0)
        fprintf( fp, "QuestNext %d\n",  5              );*/
    if (ch->pcdata->nextquest != 0)
        fprintf( fp, "NextQuest %d\n",  ch->pcdata->nextquest   );
    if ( ch->wimpy )
      fprintf( fp, "Wimpy        %d\n",	ch->wimpy		);
    if ( ch->deaf )
      fprintf( fp, "Deaf         %d\n",	ch->deaf		);
    if ( ch->pcdata->imc_deaf )
      fprintf( fp, "IMC          %ld\n", ch->pcdata->imc_deaf );
    if ( ch->pcdata->imc_allow )
      fprintf( fp, "IMCAllow     %ld\n", ch->pcdata->imc_allow );
    if ( ch->pcdata->imc_deny )
      fprintf( fp, "IMCDeny      %ld\n", ch->pcdata->imc_deny );
    fprintf(fp, "ICEListen %s~\n", ch->pcdata->ice_listen);
    if ( ch->resistant )
      fprintf( fp, "Resistant    %d\n",	ch->resistant		);
    if ( ch->no_resistant )
      fprintf( fp, "NoResistant  %d\n",	ch->no_resistant	);
    if ( ch->immune )
      fprintf( fp, "Immune       %d\n",	ch->immune		);
    if ( ch->no_immune )
      fprintf( fp, "NoImmune     %d\n",	ch->no_immune		);
    if ( ch->susceptible )
      fprintf( fp, "Susceptible  %d\n",	ch->susceptible		);
    if ( ch->no_susceptible )
      fprintf( fp, "NoSusceptible  %d\n",ch->no_susceptible	);
    if ( ch->pcdata && ch->pcdata->outcast_time )
      fprintf( fp, "Outcast_time %ld\n",ch->pcdata->outcast_time );
    if ( ch->pcdata && ch->pcdata->nuisance )
      fprintf( fp, "NuisanceNew %ld %ld %d %d\n", ch->pcdata->nuisance->time,
		ch->pcdata->nuisance->max_time,ch->pcdata->nuisance->flags,
		ch->pcdata->nuisance->power );
    if ( ch->mental_state != -10 )
      fprintf( fp, "Mentalstate  %d\n",	ch->mental_state	);

        fprintf( fp, "Password     %s~\n",	ch->pcdata->pwd		);
	if ( ch->pcdata->rank && ch->pcdata->rank[0] != '\0' )
	  fprintf( fp, "Rank         %s~\n",	ch->pcdata->rank	);
	if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0' )
	  fprintf( fp, "Bestowments  %s~\n", 	ch->pcdata->bestowments );
	fprintf( fp, "Title        %s~\n",	ch->pcdata->title	);
	if ( ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0' )
	  fprintf( fp, "Homepage     %s~\n",	ch->pcdata->homepage	);
        if ( ch->pcdata->email && ch->pcdata->email[0] != '\0' )
        fprintf( fp, "Email	     %s~\n", 	ch->pcdata->email );
        if ( ch->pcdata->icq > 0 )
        fprintf( fp, "ICQ          %d\n",		ch->pcdata->icq );
        if ( ch->pcdata->bio && ch->pcdata->bio[0] != '\0' )
	  fprintf( fp, "Bio          %s~\n",	ch->pcdata->bio 	);
	if ( ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0' )
	  fprintf( fp, "AuthedBy     %s~\n",	ch->pcdata->authed_by	);
	if ( ch->pcdata->min_snoop )
	  fprintf( fp, "Minsnoop     %d\n",	ch->pcdata->min_snoop	);
	if ( ch->pcdata->prompt && *ch->pcdata->prompt )
	  fprintf( fp, "Prompt       %s~\n",	ch->pcdata->prompt	);
 	if ( ch->pcdata->fprompt && *ch->pcdata->fprompt )
	  fprintf( fp, "FPrompt	     %s~\n",    ch->pcdata->fprompt	);
	if ( ch->pcdata->pagerlen != 24 )
	  fprintf( fp, "Pagerlen     %d\n",	ch->pcdata->pagerlen	);

	/* If ch is ignoring players then store those players */
	{
    	IGNORE_DATA *temp;
	for(temp = ch->pcdata->first_ignored; temp; temp = temp->next)
	{
		fprintf(fp,"Ignored      %s~\n", temp->name);
	}
        }

	if ( IS_IMMORTAL( ch ) || WAS_IMMORTAL(ch) )
	{
	  if ( ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0' )
	    fprintf( fp, "Bamfin       %s~\n",	ch->pcdata->bamfin	);
	  if ( ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0' )
	    fprintf( fp, "Bamfout      %s~\n",	ch->pcdata->bamfout	);
          if ( ch->trust )
            fprintf( fp, "Trust        %d\n",	ch->trust		);
          if ( ch->pcdata && ch->pcdata->restore_time )
            fprintf( fp, "Restore_time %ld\n",ch->pcdata->restore_time );
	  fprintf( fp, "WizInvis     %d\n", ch->pcdata->wizinvis );
	  if ( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	    fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo,
	  					 ch->pcdata->r_range_hi	);
	  if ( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
	    fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo,
	  					 ch->pcdata->o_range_hi	);
	  if ( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
	    fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo,
	  					 ch->pcdata->m_range_hi	);
	}
	if ( ch->pcdata->council)
	  fprintf( fp, "Council      %s~\n", 	ch->pcdata->council_name );
        if ( ch->pcdata->deity_name && ch->pcdata->deity_name[0] != '\0' )
	  fprintf( fp, "Deity	     %s~\n",	ch->pcdata->deity_name	 );

	/*
	 * Vampiromud v3.0 -- Familias -- by: SaNgUi
	 */
	 if( ch->familia )
	 	fprintf( fp, "Familia	   %s~\n", ch->familia->alias );

		fprintf( fp, "Flags        %d\n",	ch->pcdata->flags	);
        /* SiGo & SaNgUi */
        fprintf( fp, "FVol      %d\n",  ch->f_voluntad  );
        fprintf( fp, "Tiempo_FVol %d\n",  ch->tiempo_fv  );
        fprintf( fp, "PCamino   %d\n",  ch->pcamino   );
        fprintf( fp, "Camino      %s~\n",   ch->camino );
        fprintf( fp, "Frenesi       %d\n",   ch->frenesi );
        fprintf( fp, "Agravadas       %d\n",  ch->agravadas);
        fprintf( fp, "Manipulacion     %d\n", ch->manipulacion );
        fprintf( fp, "Intimidacion      %d\n", ch->intimidacion );
        fprintf( fp, "Astucia           %d\n", ch->astucia );
        fprintf( fp, "Alerta            %d\n", ch->alerta );
        for ( cnt = 0; cnt < 6; cnt++)
        {
        if ( ch->cazas[cnt] != NULL )
         {
                fprintf( fp, "Caza%d      %s~\n", cnt, ch->cazas[cnt] ? ch->cazas[cnt] : "Ninguna" );
                fprintf( fp, "Recompensa%d      %s~\n", cnt, ch->recompensa[cnt] ? ch->recompensa[cnt] : "Nada" );
                fprintf( fp, "Cazador%d      %s~\n", cnt, ch->cazador[cnt] ? ch->cazador[cnt] : "Ninguno" );
         }
        }
        fprintf( fp, "NumChiquillos     %d\n", ch->num_chiquillos );

        for ( cnt = 0; cnt < MAX_CHIQUILLOS; cnt++)
        {
                if(cnt < 10)
                fprintf(fp,"Chiquillos%d        %s~\n", cnt, ch->chiquillos[cnt] );
                if((IS_IMMORTAL(ch)
                   && cnt >= 10))
                fprintf(fp,"Chiquillos%d        %s~\n", cnt, ch->chiquillos[cnt] );
         }


        if ( ch->pcdata->release_date )
            fprintf( fp, "Helled       %d %s~\n",
        	(int)ch->pcdata->release_date, ch->pcdata->helled_by );
	fprintf( fp, "PKills       %d\n",	ch->pcdata->pkills	);
   fprintf( fp, "PDeaths      %d\n",   ch->pcdata->pdeaths  );
   fprintf( fp, "PksGeneracion %d\n",   ch->pcdata->genepkills );
   if ( get_timer( ch , TIMER_PKILLED)
         && ( get_timer( ch , TIMER_PKILLED) > 0 ) )
         fprintf( fp, "PTimer       %d\n",     get_timer(ch, TIMER_PKILLED));
        fprintf( fp, "MKills       %d\n",	ch->pcdata->mkills	);
	fprintf( fp, "MDeaths      %d\n",	ch->pcdata->mdeaths	);
	fprintf( fp, "IllegalPK    %d\n",	ch->pcdata->illegal_pk	);
	fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n",
	    ch->perm_str,
	    ch->perm_int,
	    ch->perm_wis,
	    ch->perm_dex,
	    ch->perm_con,
	    ch->perm_cha,
	    ch->perm_lck );

	fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n",
	    ch->mod_str,
	    ch->mod_int,
	    ch->mod_wis,
	    ch->mod_dex,
	    ch->mod_con,
	    ch->mod_cha,
	    ch->mod_lck );

	fprintf( fp, "Condition    %d %d %d %d\n",
	    ch->pcdata->condition[0],
	    ch->pcdata->condition[1],
	    ch->pcdata->condition[2],
	    ch->pcdata->condition[3] );
	if ( ch->desc && ch->desc->host )
            fprintf( fp, "Site         %s\n", ch->desc->host );
	else
            fprintf( fp, "Site         (Link-Dead)\n" );
        for (sn = 0; sn < AT_MAXCOLOR; ++sn)
          if (ch->pcdata->colorize[sn] != -1)
            fprintf( fp, "Color        %s %d\n", at_color_table[sn].name,
            		ch->pcdata->colorize[sn] );

	for ( sn = 1; sn < top_sn; sn++ )
	{
	    if ( skill_table[sn]->name && ch->pcdata->learned[sn] > 0 )
		switch( skill_table[sn]->type )
		{
		    default:
			fprintf( fp, "Skill        %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		    case SKILL_SPELL:
			fprintf( fp, "Spell        %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
         break;
		    case SKILL_WEAPON:
			fprintf( fp, "Weapon       %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
			break;
		    case SKILL_TONGUE:
			fprintf( fp, "Tongue       %d '%s'\n",
			  ch->pcdata->learned[sn], skill_table[sn]->name );
      }
	}

    for ( paf = ch->first_affect; paf; paf = paf->next )
    {
	if ( paf->type >= 0 && (skill=get_skilltype(paf->type)) == NULL )
	    continue;

	if ( paf->type >= 0 && paf->type < TYPE_PERSONAL )
	  fprintf( fp, "AffectData   '%s' %3d %3d %3d %s\n",
	    skill->name,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    print_bitvector(&paf->bitvector)
	    );
	else
	  fprintf( fp, "Affect       %3d %3d %3d %3d %s\n",
	    paf->type,
	    paf->duration,
	    paf->modifier,
	    paf->location,
	    print_bitvector(&paf->bitvector)
	    );
    }

    for ( daf = ch->primer_affect; daf; daf = daf->next )
    {
	nivel = get_dnivel(daf->nivel);

	if ( daf->tipo >= 0 )
	  fprintf( fp, "AffectDisciData   '%s' %3d %3d %3d %4d %s\n",
	    nivel->nombre,
	    daf->duracion,
	    daf->modificador,
	    daf->localizacion,
            daf->nivel,
	    print_bitvector(&daf->bitvector)
	    );
	else
	  fprintf( fp, "AffectDisci       %3d %3d %3d %3d %4d %s\n",
	    daf->tipo,
	    daf->duracion,
	    daf->modificador,
	    daf->localizacion,
            daf->nivel,
	    print_bitvector(&daf->bitvector)
	    );
    }

    for ( eaf = ch->primer_eaffect; eaf; eaf = eaf->next )
    {
        nivel = get_enivel(eaf->nivel);

        if( eaf->tipo >= 0 )
                fprintf( fp, "AffectEsferaData    '%s' %3d %3d %3d %4d %s\n",
                esfera->nombre,
                eaf->duracion,
                eaf->modificador,
                eaf->localizacion,
                eaf->nivel,
                print_bitvector(&eaf->bitvector)
                );
        else
                fprintf( fp, "AffectEsferaData    %3d %3d %3d %3d %4d %s\n",
                eaf->tipo,
                eaf->duracion,
                eaf->modificador,
                eaf->localizacion,
                eaf->nivel,
                print_bitvector(&eaf->bitvector)
                );
    }

    track = URANGE( 2, ((ch->level+3) * MAX_KILLTRACK)/LEVEL_AVATAR, MAX_KILLTRACK );
    for ( sn = 0; sn < track; sn++ )
    {
	if ( ch->pcdata->killed[sn].vnum == 0 )
	  break;
	fprintf( fp, "Killed       %d %d\n",
		ch->pcdata->killed[sn].vnum,
		ch->pcdata->killed[sn].count );
    }

    fprintf( fp, "End\n\n" );
    return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest,
		 sh_int os_type )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;
    sh_int wear, wear_loc, x;

    if ( iNest >= MAX_NEST )
    {
	bug( "fwrite_obj: iNest hit MAX_NEST %d", iNest );
	return;
    }

    /*
     * Slick recursion to write lists backwards,
     *   so loading them will load in forwards order.
     */
    if ( obj->prev_content && (os_type != OS_CORPSE ) && (os_type  != OS_TAQUILLA ) )
	fwrite_obj( ch, obj->prev_content, fp, iNest, OS_CARRY );

    /*
     * Castrate storage characters.
     * Catch deleted objects                                    -Thoric
     * Do NOT save prototype items!				-Thoric
     */

    if ( (ch && obj->item_type == ITEM_KEY && !IS_OBJ_STAT(obj, ITEM_CLANOBJECT ))
    ||   obj_extracted(obj)
    ||   IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
	return;

    /* Perdida de equipo por ser de nivel mayor que jugador, excepto ekipo propio marcado */
    if( (ch && obj->level > ch->level ) )
    {
        if( !IS_OBJ_STAT( obj, ITEM_QUEST ) || !IS_OBJ_STAT(obj, ITEM_LOYAL ) )
        return;
    }

    /*    Munch magic flagged containers for now - bandaid */
    if ( obj->item_type == ITEM_CONTAINER
    &&   IS_OBJ_STAT( obj, ITEM_MAGIC ) )
	xTOGGLE_BIT( obj->extra_flags, ITEM_MAGIC );

    /*
    fprintf( fp, (os_type == OS_CORPSE ? "#CORPSE\n" : "#OBJECT\n") );
     */
 if ( os_type == OS_CARRY )
 fprintf( fp, "#OBJECT\n" );
 if ( os_type == OS_CORPSE )
 fprintf( fp, "#CORPSE\n" );
 if ( os_type == OS_TAQUILLA )
 fprintf( fp, "#TAQUILLA\n" );

    if ( iNest )
	fprintf( fp, "Nest         %d\n",	iNest		     );
    if ( obj->count > 1 )
	fprintf( fp, "Count        %d\n",	obj->count	     );
    if ( QUICKMATCH( obj->name, obj->pIndexData->name ) == 0 )
	fprintf( fp, "Name         %s~\n",	obj->name	     );
    if( obj->propietario != NULL )
        fprintf( fp, "Propietario       %s~\n",obj->propietario );
    else
        fprintf( fp, "Propietario       %s~\n","Sin propietario" );
    if ( QUICKMATCH( obj->short_descr, obj->pIndexData->short_descr ) == 0 )
	fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr     );
    if ( QUICKMATCH( obj->description, obj->pIndexData->description ) == 0 )
	fprintf( fp, "Description  %s~\n",	obj->description     );
    if ( QUICKMATCH( obj->action_desc, obj->pIndexData->action_desc ) == 0 )
	fprintf( fp, "ActionDesc   %s~\n",	obj->action_desc     );
    fprintf( fp, "Vnum         %d\n",	obj->pIndexData->vnum	     );
    if ( os_type == OS_TAQUILLA && obj->in_room )
    fprintf( fp, "Room          %d\n", obj->in_room->vnum );
    if ( os_type == OS_CORPSE && obj->in_room )
      fprintf( fp, "Room         %d\n",   obj->in_room->vnum         );
    if ( !xSAME_BITS(obj->extra_flags, obj->pIndexData->extra_flags) )
	fprintf( fp, "ExtraFlags   %s\n",	print_bitvector(&obj->extra_flags) );
    if ( obj->wear_flags != obj->pIndexData->wear_flags )
	fprintf( fp, "WearFlags    %d\n",	obj->wear_flags	     );
    wear_loc = -1;
    for ( wear = 0; wear < MAX_WEAR; wear++ )
	for ( x = 0; x < MAX_LAYERS; x++ )
	   if ( obj == save_equipment[wear][x] )
	   {
		wear_loc = wear;
		break;
	   }
	   else
	   if ( !save_equipment[wear][x] )
		break;
    if ( wear_loc != -1 )
	fprintf( fp, "WearLoc      %d\n",	wear_loc	     );
    if ( obj->item_type != obj->pIndexData->item_type )
	fprintf( fp, "ItemType     %d\n",	obj->item_type	     );
    if ( obj->weight != obj->pIndexData->weight )
      fprintf( fp, "Weight       %d\n",	obj->weight		     );
    if ( obj->level )
      fprintf( fp, "Level        %d\n",	obj->level		     );
    if ( obj->timer )
      fprintf( fp, "Timer        %d\n",	obj->timer		     );
    if ( obj->cost != obj->pIndexData->cost )
      fprintf( fp, "Cost         %d\n",	obj->cost		     );
    if ( obj->value[0] || obj->value[1] || obj->value[2]
    ||   obj->value[3] || obj->value[4] || obj->value[5] )
      fprintf( fp, "Values       %d %d %d %d %d %d\n",
	obj->value[0], obj->value[1], obj->value[2],
	obj->value[3], obj->value[4], obj->value[5]     );

    switch ( obj->item_type )
    {
    case ITEM_PILL: /* was down there with staff and wand, wrongly - Scryn */
    case ITEM_POTION:
    case ITEM_SCROLL:
	if ( IS_VALID_SN(obj->value[1]) )
	    fprintf( fp, "Spell 1      '%s'\n",
		skill_table[obj->value[1]]->name );

	if ( IS_VALID_SN(obj->value[2]) )
	    fprintf( fp, "Spell 2      '%s'\n",
		skill_table[obj->value[2]]->name );

	if ( IS_VALID_SN(obj->value[3]) )
	    fprintf( fp, "Spell 3      '%s'\n",
		skill_table[obj->value[3]]->name );

	break;

    case ITEM_STAFF:
    case ITEM_WAND:
	if ( IS_VALID_SN(obj->value[3]) )
	    fprintf( fp, "Spell 3      '%s'\n",
		skill_table[obj->value[3]]->name );

	break;
    case ITEM_SALVE:
	if ( IS_VALID_SN(obj->value[4]) )
	    fprintf( fp, "Spell 4      '%s'\n",
		skill_table[obj->value[4]]->name );

	if ( IS_VALID_SN(obj->value[5]) )
	    fprintf( fp, "Spell 5      '%s'\n",
		skill_table[obj->value[5]]->name );
	break;
    }

    for ( paf = obj->first_affect; paf; paf = paf->next )
    {
	/*
	 * Save extra object affects				-Thoric
	 */
	if ( paf->type < 0 || paf->type >= top_sn )
	{
	  fprintf( fp, "Affect       %d %d %d %d %s\n",
	    paf->type,
	    paf->duration,
	     ((paf->location == APPLY_WEAPONSPELL
	    || paf->location == APPLY_WEARSPELL
	    || paf->location == APPLY_REMOVESPELL
	    || paf->location == APPLY_STRIPSN
	    || paf->location == APPLY_RECURRINGSPELL)
	    && IS_VALID_SN(paf->modifier))
	    ? skill_table[paf->modifier]->slot : paf->modifier,
	    paf->location,
	    print_bitvector(&paf->bitvector)
	    );
	}
	else
	  fprintf( fp, "AffectData   '%s' %d %d %d %s\n",
	    skill_table[paf->type]->name,
	    paf->duration,
	     ((paf->location == APPLY_WEAPONSPELL
	    || paf->location == APPLY_WEARSPELL
	    || paf->location == APPLY_REMOVESPELL
	    || paf->location == APPLY_STRIPSN
	    || paf->location == APPLY_RECURRINGSPELL)
	    && IS_VALID_SN(paf->modifier))
	    ? skill_table[paf->modifier]->slot : paf->modifier,
	    paf->location,
	    print_bitvector(&paf->bitvector)
	    );
    }

    for ( ed = obj->first_extradesc; ed; ed = ed->next )
	fprintf( fp, "ExtraDescr   %s~ %s~\n",
	    ed->keyword, ed->description );

    fprintf( fp, "End\n\n" );

    if ( obj->first_content )
	fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY );

    return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name, bool preload )
{
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    struct stat fst;
    int i, x;
    extern FILE *fpArea;
    extern char strArea[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    CREATE( ch, CHAR_DATA, 1 );
    for ( x = 0; x < MAX_WEAR; x++ )
	for ( i = 0; i < MAX_LAYERS; i++ )
	    save_equipment[x][i] = NULL;
    clear_char( ch );
    loading_char = ch;

    CREATE( ch->pcdata, PC_DATA, 1 );
    d->character		= ch;
    ch->desc			= d;
    ch->pcdata->filename	= STRALLOC( name );
    ch->name			= NULL;
    ch->act			= multimeb(PLR_BLANK, PLR_COMBINE, PLR_PROMPT, -1);
    ch->pcdata->board           = &boards[DEFAULT_BOARD]; /* Nivel de lectura de notas 0 */
    ch->perm_str		= 13;
    ch->perm_int		= 13;
    ch->perm_wis		= 13;
    ch->perm_dex		= 13;
    ch->perm_con		= 13;
    ch->perm_cha		= 13;
    ch->perm_lck		= 13;
    ch->no_resistant 		= 0;
    ch->no_susceptible 		= 0;
    ch->no_immune 		= 0;
    ch->was_in_room		= NULL;
    xCLEAR_BITS(ch->no_affected_by);
    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->pcdata->condition[COND_BLOODTHIRST] = 10;
    ch->pcdata->nuisance		= NULL;
    ch->pcdata->wizinvis		= 0;
    ch->pcdata->balance                                 = 0;
    ch->pcdata->charmies		= 0;
    ch->mental_state			= -10;
    ch->mobinvis			= 0;
    for(i = 0; i < MAX_SKILL; i++)
        ch->pcdata->learned[i]		= 0;
    ch->pcdata->release_date		= 0;
    ch->pcdata->helled_by		= NULL;
    ch->saving_poison_death 		= 0;
    ch->saving_wand			= 0;
    ch->saving_para_petri		= 0;
    ch->saving_breath			= 0;
    ch->saving_spell_staff		= 0;
    ch->style 				= STYLE_FIGHTING;
    ch->comments                        = NULL;    /* comments */
    ch->pcdata->pagerlen		= 24;
    ch->pcdata->first_ignored		= NULL;    /* Ignore list */
    ch->pcdata->last_ignored		= NULL;
    ch->pcdata->tell_history		= NULL;	/* imm only lasttell cmnd */
    ch->pcdata->lt_index		= 0;	/* last tell index */
    ch->morph                           = NULL;
    ch->pcdata->email                   = NULL;
    ch->pcdata->homepage                = NULL;
    ch->pcdata->icq                     = 0;
    /* Set up defaults for imc stuff */
    ch->pcdata->imc_deaf		= 0;
    ch->pcdata->imc_deny		= 0;
    ch->pcdata->imc_allow		= 0;
    ch->pcdata->ice_listen		= NULL;
    for (i = 0; i < AT_MAXCOLOR; ++i)
      ch->pcdata->colorize[i] = -1;

    /* Valores por defecto de Vampiromud2.0b por COLIKOTRON Code Team 2001 */
    ch->conquistas               = 0; /* Conquistas persoanles */
    ch->camino                  = STRALLOC( "Humanidad" ); /* Camino del jugador */
    ch->camuflaje               = STRALLOC( "Ninguno" ); /* Camuflaje */
    ch->pcamino                 = number_range( 3, 7 ); /* Puntos de Camino del jugador */
    ch->f_voluntad              = number_range( 3, 7 ); /* Puntos de Fuerza de Voluntad */
    ch->frenesi                 = -1; /* Frenesi del jugador */
    ch->posicion                = STRALLOC( "Chusma" ); /* Posicion Social del jugador */
    ch->manipulacion            = number_range( 1, 5 ); /* Poder de Manipulacion del jugador *Importante con Carisma */
    ch->modo_ficha              = 0; /* Modo de la Ficha del Jugador Cambiar a -1 cuando akabemos xD */
    ch->astucia                 = number_range( 1, 5 ); /* Astucia del jugador, util para tiradas de dados */
    ch->alerta                  = number_range( 1, 5 ); /* El jugador esta mas o menos alerta */
    ch->intimidacion            = number_range( 1, 5 ); /* Intimidacion del jugador sobre sus victimas */
    ch->ocultismo               = number_range( 1, 5 ); /* Conocimientos de ocultismo del jugador */

    found = FALSE;
    sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
			capitalize( name ) );
    if ( stat( strsave, &fst ) != -1 )
    {
      if ( fst.st_size == 0 )
      {
	sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower(name[0]),
			capitalize( name ) );
	send_to_char( "Restoring your backup player file...", ch );
      }
      else
      {
	sprintf( buf, "%s player data for: %s (%dK)",
	  preload ? "Preloading" : "Loading", ch->pcdata->filename,
		(int) fst.st_size/1024 );
	log_string_plus( buf, LOG_COMM, LEVEL_GREATER );
      }
    }
    /* else no player file */

    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	int iNest;

	for ( iNest = 0; iNest < MAX_NEST; iNest++ )
	    rgObjNest[iNest] = NULL;

	found = TRUE;
	/* Cheat so that bug will show line #'s -- Altrag */
	fpArea = fp;
	strcpy(strArea, strsave);
	for ( ; ; )
	{
	    char letter;
	    char *word;

	    letter = fread_letter( fp );
	    if ( letter == '*' )
	    {
		fread_to_eol( fp );
		continue;
	    }

	    if ( letter != '#' )
	    {
		bug( "Load_char_obj: # not found.", 0 );
		bug( name, 0 );
		break;
	    }

	    word = fread_word( fp );
	    if ( !strcmp( word, "PLAYER" ) )
	    {
		fread_char ( ch, fp, preload );
		if ( preload )
		  break;
	    }
	    else
	    if ( !strcmp( word, "OBJECT" ) )	/* Objects	*/
		fread_obj  ( ch, fp, OS_CARRY );
	    else
	    if ( !strcmp( word, "MorphData") ) /* Morphs */
		fread_morph_data ( ch, fp );
	    else
	    if ( !strcmp( word, "COMMENT") )
		fread_comment(ch, fp );		/* Comments	*/
	    else
	    if ( !strcmp( word, "MOBILE") )
	    {
		CHAR_DATA *mob;
		mob = fread_mobile( fp );
		ch->pcdata->pet = mob;
		mob->master = ch;
		xSET_BIT(mob->affected_by, AFF_CHARM);
	    }
	    else
	    if ( !strcmp( word, "END"    ) )	/* Done		*/
		break;
	    else
	    {
		bug( "Load_char_obj: bad section.", 0 );
		bug( name, 0 );
		break;
	    }
	}
	fclose( fp );
	fpArea = NULL;
	strcpy(strArea, "$");
    }

    if ( ch->pcdata->ice_listen == NULL )
        ch->pcdata->ice_listen = str_dup("");

    if ( !found )
    {
        ch->name	   		= STRALLOC( name );
	ch->short_descr			= STRALLOC( "" );
	ch->long_descr			= STRALLOC( "" );
	ch->description			= STRALLOC( "" );
	ch->editor			= NULL;
	ch->pcdata->clan_name		= STRALLOC( "" );
	ch->pcdata->clan		= NULL;
	ch->pcdata->council_name 	= STRALLOC( "" );
	ch->pcdata->council 		= NULL;
  	ch->pcdata->deity_name		= STRALLOC( "" );
	ch->pcdata->deity		= NULL;
	ch->pcdata->pet			= NULL;
	ch->pcdata->pwd			= str_dup( "" );
	ch->pcdata->bamfin		= str_dup( "" );
	ch->pcdata->bamfout		= str_dup( "" );
	ch->pcdata->rank		= str_dup( "" );
	ch->pcdata->bestowments		= str_dup( "" );
	ch->pcdata->title		= STRALLOC( "" );
	ch->pcdata->homepage		= str_dup( "" );
	ch->pcdata->bio 		= STRALLOC( "" );
	ch->pcdata->authed_by		= STRALLOC( "" );
	ch->pcdata->prompt		= STRALLOC( "" );
	ch->pcdata->fprompt		= STRALLOC( "" );
        ch->pcdata->email               = str_dup( "" );
        ch->pcdata->icq                 = 0;
	ch->pcdata->r_range_lo		= 0;
	ch->pcdata->r_range_hi		= 0;
	ch->pcdata->m_range_lo		= 0;
	ch->pcdata->m_range_hi		= 0;
	ch->pcdata->o_range_lo		= 0;
	ch->pcdata->o_range_hi		= 0;
	ch->pcdata->wizinvis		= 0;
    }
    else
    {
	if ( !ch->name )
    		ch->name	= STRALLOC( name );
	if ( !ch->pcdata->clan_name )
	{
	  ch->pcdata->clan_name	= STRALLOC( "" );
	  ch->pcdata->clan	= NULL;
	}
	if ( !ch->pcdata->council_name )
	{
	  ch->pcdata->council_name = STRALLOC( "" );
	  ch->pcdata->council 	= NULL;
	}
	if ( !ch->pcdata->deity_name )
	{
	  ch->pcdata->deity_name = STRALLOC( "" );
	  ch->pcdata->deity	 = NULL;
	}
        if ( !ch->pcdata->bio )
          ch->pcdata->bio	 = STRALLOC( "" );

	if ( !ch->pcdata->authed_by )
	  ch->pcdata->authed_by	 = STRALLOC( "" );

	if ( xIS_SET(ch->act, PLR_FLEE) )
	  xREMOVE_BIT(ch->act, PLR_FLEE);

	if ( IS_IMMORTAL( ch ) )
	{
	  if ( ch->pcdata->wizinvis < 2 )
	    ch->pcdata->wizinvis = ch->level;
 	  assign_area( ch );
	}

	if ( file_ver > 1 )
	  for ( i = 0; i < MAX_WEAR; i++ )
	    for ( x = 0; x < MAX_LAYERS; x++ )
		if ( save_equipment[i][x] )
		{
		    equip_char( ch, save_equipment[i][x], i );
		    save_equipment[i][x] = NULL;
		}
                /* else
		    break; */

	/* Must be done *AFTER* eq is worn because of wis/int modifiers */
/*	if ( !IS_IMMORTAL(ch) )
		REMOVE_BIT(ch->speaks, LANG_COMMON | race_table[ch->race]->language);
	if ( countlangs(ch->speaks) < (ch->level / 10) && !IS_IMMORTAL(ch) )
	{
		int prct = 5 + (get_curr_int(ch) / 6) + (get_curr_wis(ch) / 7);

		do
		{
			int iLang;
			int lang = 1;
			int need = (ch->level / 10) - countlangs(ch->speaks);
			int prac = 2 - (get_curr_cha(ch) / 17) * (70 / prct) * need;

			if ( ch->practice >= prac )
				break;

			for ( iLang = 1; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
				if ( number_range( 1, iLang ) == 1 )
					lang = iLang;
			if ( (iLang = bsearch_skill_exact( lang_names[lang], gsn_first_tongue, gsn_top_sn-1  )) < 0 )
				continue;
			if ( ch->pcdata->learned[iLang] > 0 )
				continue;
			SET_BIT(ch->speaks, lang_array[lang]);
			ch->pcdata->learned[iLang] = 70;
			ch->speaks &= VALID_LANGS;
			REMOVE_BIT(ch->speaks,
					   LANG_COMMON | race_table[ch->race]->language);
		}
	}*/
    }

    /* Rebuild affected_by and RIS to catch errors - FB */
    update_aris(ch);
    loading_char = NULL;
    return found;
}



/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_char( CHAR_DATA *ch, FILE *fp, bool preload )
{

    char buf[MAX_STRING_LENGTH];
    char *line;
    char *word;
    int x1, x2, x3, x4, x5, x6, x7;
    sh_int killcnt;
    bool fMatch;

    file_ver = 0;
    killcnt = 0;
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
         	    if (!str_cmp(word, "Apartados" ))
 	    {
 			int i,num = fread_number (fp); /* number of boards saved */
 			char *boardname;

 			for (; num ; num-- ) /* for each of the board saved */
 			{
 				boardname = fread_word (fp);
 				i = board_lookup (boardname); /* find board number */

 				if (i == BOARD_NOTFOUND) /* Does board still exist ? */
 				{
 					sprintf (buf, "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);
 					log_string (buf);
 					fread_number (fp); /* read last_note and skip info */
 				}
 				else /* Save it */
 					ch->pcdata->last_note[i] = fread_number (fp);
 			}	 /* for */

 			fMatch = TRUE;
 	    } /* Apartados */

	    KEY( "Act",		ch->act,		fread_bitvector( fp ) );
	    KEY( "AffectedBy",	ch->affected_by,	fread_bitvector( fp ) );
            KEY( "Affect_disciplina", ch->afectado_por,        fread_bitvector( fp ) );
            KEY( "Agravadas",	ch->agravadas,	        fread_number( fp ) );
            KEY( "Alerta",      ch->alerta,             fread_number( fp ) );
	    KEY( "Alignment",	ch->alignment,		fread_number( fp ) );
	    KEY( "Armor",	ch->armor,		fread_number( fp ) );
            KEY( "Astucia",     ch->astucia,            fread_number( fp ) );

	    if ( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;

		if ( preload )
		{
		    fMatch = TRUE;
		    fread_to_eol( fp );
		    break;
		}
		CREATE( paf, AFFECT_DATA, 1 );
		if ( !strcmp( word, "Affect" ) )
		{
		    paf->type	= fread_number( fp );
		}
		else
		{
		    int sn;
		    char *sname = fread_word(fp);

		    if ((sn=skill_lookup(sname)) < 0)

		    {
			if ( (sn=herb_lookup(sname)) < 0 )
			    bug( "Fread_char: unknown skill.", 0 );
			else
			    sn += TYPE_HERB;
		    }
		    paf->type = sn;
		}

		paf->duration	= fread_number( fp );
		paf->modifier	= fread_number( fp );
		paf->location	= fread_number( fp );
		if ( paf->location == APPLY_WEAPONSPELL
		||   paf->location == APPLY_WEARSPELL
		||   paf->location == APPLY_REMOVESPELL
		||   paf->location == APPLY_STRIPSN
		||   paf->location == APPLY_RECURRINGSPELL )
		  paf->modifier	= slot_lookup( paf->modifier );
		paf->bitvector	= fread_bitvector( fp );
		LINK(paf, ch->first_affect, ch->last_affect, next, prev );
		fMatch = TRUE;
		break;
	    }

            /* Affects de disciplinas */
            if ( !strcmp( word, "AffectDisciData" ) )
	    {
		DISCI_AFFECT *daf;

		if ( preload )
		{
		    fMatch = TRUE;
		    fread_to_eol( fp );
		    break;
		}
		CREATE( daf, DISCI_AFFECT, 1 );
		if ( !strcmp( word, "Affect" ) )
		{
		    daf->tipo	= fread_number( fp );
		}
		else
		{
		    NIVEL *dn;
		    char *dname = fread_word(fp);

		    if ((dn=get_nivel(dname)) != NULL)
                    daf->tipo = dn->tipo;
                    else
                    {
                    bug( "fread_char: Nivel no encontrado dn = NULL %s", dname );
                    break;
                    }
		}

		daf->duracion	= fread_number( fp );
		daf->modificador	= fread_number( fp );
		daf->localizacion	= fread_number( fp );
                daf->nivel = fread_number( fp );
		daf->bitvector	= fread_bitvector( fp );
		LINK(daf, ch->primer_affect, ch->ultimo_affect, next, prev );
		fMatch = TRUE;
		break;
	    }

            /* Affects de esferas */
            if ( !strcmp( word, "AffectEsferaData" ) )
	    {
		ESFERA_AFFECT *eaf;

		if ( preload )
		{
		    fMatch = TRUE;
		    fread_to_eol( fp );
		    break;
		}
		CREATE( eaf, ESFERA_AFFECT, 1 );
		if ( !strcmp( word, "Affect" ) )
		{
		    eaf->tipo	= fread_number( fp );
		}
		else
		{
		    NIVEL *en;
		    char *ename = fread_word(fp);

		    if ((en=get_nivel(ename)) != NULL)
                    eaf->tipo = en->tipo;
                    else
                    {
                    bug( "fread_char: Nivel de esfera no encontrado en = NULL %s", ename );
                    break;
                    }
		}

		eaf->duracion	= fread_number( fp );
		eaf->modificador	= fread_number( fp );
		eaf->localizacion	= fread_number( fp );
                eaf->nivel = fread_number( fp );
		eaf->bitvector	= fread_bitvector( fp );
		LINK(eaf, ch->primer_eaffect, ch->ultimo_eaffect, next, prev );
		fMatch = TRUE;
		break;
	    }


	    if ( !strcmp( word, "AttrMod"  ) )
	    {
		line = fread_line( fp );
		x1=x2=x3=x4=x5=x6=x7=13;
		sscanf( line, "%d %d %d %d %d %d %d",
		      &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
		ch->mod_str = x1;
		ch->mod_int = x2;
		ch->mod_wis = x3;
		ch->mod_dex = x4;
		ch->mod_con = x5;
		ch->mod_cha = x6;
		ch->mod_lck = x7;
		if (!x7)
		ch->mod_lck = 0;
		fMatch = TRUE;
		break;
	    }

	    if ( !strcmp( word, "AttrPerm" ) )
	    {
		line = fread_line( fp );
		x1=x2=x3=x4=x5=x6=x7=0;
		sscanf( line, "%d %d %d %d %d %d %d",
		      &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
		ch->perm_str = x1;
		ch->perm_int = x2;
		ch->perm_wis = x3;
		ch->perm_dex = x4;
		ch->perm_con = x5;
		ch->perm_cha = x6;
		ch->perm_lck = x7;
		if (!x7 || x7 == 0)
		  ch->perm_lck = 13;
		fMatch = TRUE;
		break;
	    }
	    KEY( "AuthedBy",	ch->pcdata->authed_by,	fread_string( fp ) );
	    break;

	case 'B':
                       KEY( "Balance",	ch->pcdata->balance,	fread_number( fp ) );
                       KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string_nohash( fp ) );
	    KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string_nohash( fp ) );
	    KEY( "Bestowments", ch->pcdata->bestowments, fread_string_nohash( fp ) );
	    KEY( "Bio",		ch->pcdata->bio,	fread_string( fp ) );
       /*KEY( "Board",    ch->board,  fread_string( fp ) );*/  /* Carga el tablero del jugador SiGo */
	    break;

	case 'C':

            KEY( "Camino",	ch->camino,             fread_string( fp ) );
            KEY( "Camuflaje",   ch->camuflaje,           fread_string( fp ) );
            KEY( "Caza0",       ch->cazas[0],            fread_string( fp ) );
            KEY( "Caza1",       ch->cazas[1],            fread_string( fp ) );
            KEY( "Caza2",       ch->cazas[2],            fread_string( fp ) );
            KEY( "Caza3",       ch->cazas[3],            fread_string( fp ) );
            KEY( "Caza4",       ch->cazas[4],            fread_string( fp ) );
            KEY( "Caza5",       ch->cazas[5],            fread_string( fp ) );
            KEY( "Cazador0",     ch->cazador[0],         fread_string( fp ) );
            KEY( "Cazador1",     ch->cazador[1],         fread_string( fp ) );
            KEY( "Cazador2",     ch->cazador[2],         fread_string( fp ) );
            KEY( "Cazador3",     ch->cazador[3],         fread_string( fp ) );
            KEY( "Cazador4",     ch->cazador[4],         fread_string( fp ) );
            KEY( "Cazador5",     ch->cazador[5],         fread_string( fp ) );
            KEY( "Conquistas",   ch->conquistas,         fread_number( fp ) );
            KEY( "Coeficiente",   ch->pcdata->coeficiente,         fread_number( fp ) );
            KEY( "Chiquillos0",     ch->chiquillos[0],         fread_string( fp ) );
            KEY( "Chiquillos1",     ch->chiquillos[1],         fread_string( fp ) );
            KEY( "Chiquillos2",     ch->chiquillos[2],         fread_string( fp ) );
            KEY( "Chiquillos3",     ch->chiquillos[3],         fread_string( fp ) );
            KEY( "Chiquillos4",     ch->chiquillos[4],         fread_string( fp ) );
            KEY( "Chiquillos5",     ch->chiquillos[5],         fread_string( fp ) );
            KEY( "Chiquillos6",     ch->chiquillos[6],         fread_string( fp ) );
            KEY( "Chiquillos7",     ch->chiquillos[7],         fread_string( fp ) );
            KEY( "Chiquillos8",     ch->chiquillos[8],         fread_string( fp ) );
            KEY( "Chiquillos9",     ch->chiquillos[9],         fread_string( fp ) );
            if( IS_IMMORTAL(ch))
            {
                KEY( "Chiquillos10",     ch->chiquillos[10],         fread_string( fp ) );
                KEY( "Chiquillos11",     ch->chiquillos[11],         fread_string( fp ) );
                KEY( "Chiquillos12",     ch->chiquillos[12],         fread_string( fp ) );
                KEY( "Chiquillos13",     ch->chiquillos[13],         fread_string( fp ) );
                KEY( "Chiquillos14",     ch->chiquillos[14],         fread_string( fp ) );
                KEY( "Chiquillos15",     ch->chiquillos[15],         fread_string( fp ) );
                KEY( "Chiquillos16",     ch->chiquillos[16],         fread_string( fp ) );
                KEY( "Chiquillos17",     ch->chiquillos[17],         fread_string( fp ) );
                KEY( "Chiquillos18",     ch->chiquillos[18],         fread_string( fp ) );
                KEY( "Chiquillos19",     ch->chiquillos[19],         fread_string( fp ) );
                KEY( "Chiquillos20",     ch->chiquillos[20],         fread_string( fp ) );
                KEY( "Chiquillos21",     ch->chiquillos[21],         fread_string( fp ) );
                KEY( "Chiquillos22",     ch->chiquillos[22],         fread_string( fp ) );
                KEY( "Chiquillos23",     ch->chiquillos[23],         fread_string( fp ) );
                KEY( "Chiquillos24",     ch->chiquillos[24],         fread_string( fp ) );
                KEY( "Chiquillos25",     ch->chiquillos[25],         fread_string( fp ) );
             }
            KEY( "Class",	ch->class,		fread_number( fp ) );
                     KEY ( "Contabrazo",  ch->cnt_abrazo,      fread_number( fp ) );
               KEY( "CntPk",    ch->cnt_pktotal,        fread_number( fp ) );
               KEY( "CntPasion",    ch->pcdata->cnt_pasion,  fread_number( fp ) );
               KEY( "CntCritico",       ch->pcdata->cnt_critico,        fread_number( fp ) );
               KEY( "CntDominacion",    ch->pcdata->cnt_dominacion,     fread_number( fp ) );


	    if ( !str_cmp( word, "Color" ) )
	    {
		char *cword;
		int at;

		cword = fread_word(fp);
		for (at = 0; at < AT_MAXCOLOR; ++at)
		  if (!str_cmp(cword, at_color_table[at].name))
		    break;
		if (at < AT_MAXCOLOR)
		  ch->pcdata->colorize[at] = fread_number(fp);
		else
		{
		  bug("Fread_char: color %s invalid.", cword);
		  fread_number(fp);
		}
		fMatch = TRUE;
		break;
	    }

	    if ( !str_cmp( word, "Condition" ) )
	    {
		line = fread_line( fp );
		sscanf( line, "%d %d %d %d",
		      &x1, &x2, &x3, &x4 );
		ch->pcdata->condition[0] = x1;
		ch->pcdata->condition[1] = x2;
		ch->pcdata->condition[2] = x3;
		ch->pcdata->condition[3] = x4;
		fMatch = TRUE;
		break;
	    }

	    if ( !strcmp( word, "Council" ) )
	    {
		ch->pcdata->council_name = fread_string( fp );
		if ( !preload
		&&   ch->pcdata->council_name[0] != '\0'
		&& ( ch->pcdata->council = get_council( ch->pcdata->council_name )) == NULL )
		{
		  sprintf( buf, "Warning: the council %s no longer exists, and herefore you no longer\n\rbelong to a council.\n\r",
		           ch->pcdata->council_name );
		  send_to_char( buf, ch );
		  STRFREE( ch->pcdata->council_name );
		  ch->pcdata->council_name = STRALLOC( "" );
		}
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'D':
	    KEY( "Damroll",	ch->damroll,		fread_number( fp ) );
	    KEY( "Deaf",	ch->deaf,		fread_number( fp ) );
	    if ( !strcmp( word, "Deity" ) )
            {
                ch->pcdata->deity_name = fread_string( fp );

                if ( !preload
                &&   ch->pcdata->deity_name[0] != '\0'
                && ( ch->pcdata->deity = get_deity( ch->pcdata->deity_name )) == NULL )
                {
                  sprintf( buf, "Warning: the deity %s no longer exists.\n\r",
                           ch->pcdata->deity_name );
                  send_to_char( buf, ch );
                  STRFREE( ch->pcdata->deity_name );
                  ch->pcdata->deity_name = STRALLOC( "" );
		  ch->pcdata->favor = 0;
                }
                fMatch = TRUE;
                break;
            }
	    KEY( "Description",	ch->description,	fread_string( fp ) );

         KEY( "DisciAdq",    ch->pcdata->disci_adquirir, fread_number( fp ) );
         KEY( "DisciPts",    ch->pcdata->disci_puntos, fread_number( fp ) );

    	/* 'E' was moved to after 'S' */
        case 'F':


	/*
	 * Vampiromud v3.0 -- Familias -- by: SaNgUi
	 */
        if ( !strcmp( word, "Familia" ) )
	    {
			ch->familia_nombre = fread_string( fp );

		if ( !preload
		&&   ch->familia_nombre[0] != '\0'
		&& ( ch->familia = buscaFamilia( ch->familia_nombre )) == NULL )
		{
		  ch_printf( ch, "Atencion: La familia %s no existe, pregunta a alguien a ver que ha pasado.\n\r" );
		  STRFREE( ch->familia_nombre );
		  ch->familia_nombre = STRALLOC( "" );
		}
		fMatch = TRUE;
		break;
	    }

	/*
	 * Vampiromud v3.0 -- Familias -- by: SaNgUi
	 * FIN
	 */
            KEY( "Favor",	ch->pcdata->favor,	fread_number( fp ) );
	    if ( !strcmp( word, "Filename" ) )
	    {
		/*
		 * File Name already set externally.
		 */
		fread_to_eol( fp );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Flags",	ch->pcdata->flags,	fread_number( fp ) );
	    KEY( "FPrompt",	ch->pcdata->fprompt,	fread_string( fp ) );
            KEY( "Frenesi",	ch->frenesi,	fread_number( fp ) );
            KEY( "FVol",        ch->f_voluntad, fread_number( fp ) );
            break;

	case 'G':
	    KEY( "Glory",       ch->pcdata->quest_curr, fread_number( fp ) );
	    KEY( "Gold",	ch->gold,		fread_number( fp ) );
       KEY( "Generacion",  ch->generacion,      fread_number( fp ) );
	    /* temporary measure */
	    if ( !strcmp( word, "Guild" ) )
	    {
		ch->pcdata->clan_name = fread_string( fp );

		if ( !preload
		&&   ch->pcdata->clan_name[0] != '\0'
		&& ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name )) == NULL )
		{
		  sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
		           ch->pcdata->clan_name );
		  send_to_char( buf, ch );
		  STRFREE( ch->pcdata->clan_name );
		  ch->pcdata->clan_name = STRALLOC( "" );
		}
		fMatch = TRUE;
		break;
	    }
            break;

	case 'H':
        if ( !str_cmp( word, "Habilidades" ) )
	    {
	ch->pcdata->habilidades[0] = fread_number(fp);
                ch->pcdata->habilidades[1] = fread_number(fp);
                ch->pcdata->habilidades[2] = fread_number(fp);
                ch->pcdata->habilidades[3] = fread_number(fp);
                ch->pcdata->habilidades[4] = fread_number(fp);
                ch->pcdata->habilidades[5] = fread_number(fp);
                ch->pcdata->habilidades[6] = fread_number(fp);
                ch->pcdata->habilidades[7] = fread_number(fp);
                ch->pcdata->habilidades[8] = fread_number(fp);
                ch->pcdata->habilidades[9] = fread_number(fp);
                ch->pcdata->habilidades[10] = fread_number(fp);
                ch->pcdata->habilidades[11] = fread_number(fp);
                ch->pcdata->habilidades[12] = fread_number(fp);
                ch->pcdata->habilidades[13] = fread_number(fp);
                ch->pcdata->habilidades[14] = fread_number(fp);
                ch->pcdata->habilidades[15] = fread_number(fp);
                ch->pcdata->habilidades[16] = fread_number(fp);
                ch->pcdata->habilidades[17] = fread_number(fp);
                ch->pcdata->habilidades[18] = fread_number(fp);
                ch->pcdata->habilidades[19] = fread_number(fp);
                ch->pcdata->habilidades[20] = fread_number(fp);
                ch->pcdata->habilidades[21] = fread_number(fp);
                ch->pcdata->habilidades[22] = fread_number(fp);
                ch->pcdata->habilidades[23] = fread_number(fp);
                ch->pcdata->habilidades[24] = fread_number(fp);
                ch->pcdata->habilidades[25] = fread_number(fp);
                ch->pcdata->habilidades[26] = fread_number(fp);
                ch->pcdata->habilidades[27] = fread_number(fp);
                ch->pcdata->habilidades[28] = fread_number(fp);
                ch->pcdata->habilidades[29] = fread_number(fp);
                ch->pcdata->habilidades[30] = fread_number(fp);
                ch->pcdata->habilidades[31] = fread_number(fp);
                ch->pcdata->habilidades[32] = fread_number(fp);
                ch->pcdata->habilidades[33] = fread_number(fp);
                fMatch = TRUE;
                break;
	    }
            KEY( "Height",        ch->height,   fread_number( fp ) );

	    if ( !strcmp(word, "Helled") )
	    {
	      ch->pcdata->release_date = fread_number(fp);
	      ch->pcdata->helled_by = fread_string(fp);
	      fMatch = TRUE;
	      break;
	    }

	    KEY( "Hitroll",	ch->hitroll,		fread_number( fp ) );
	    KEY( "Homepage",	ch->pcdata->homepage,	fread_string_nohash( fp ) );

	    if ( !strcmp( word, "HpManaMove" ) )
	    {
		ch->hit		= fread_number( fp );
		ch->max_hit	= fread_number( fp );
		ch->mana	= fread_number( fp );
		ch->max_mana	= fread_number( fp );
		ch->move	= fread_number( fp );
		ch->max_move	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'I':
	    if(!strcmp(word, "Ignored"))
	    {
	    	char *temp;
	    	char fname[1024];
	    	struct stat fst;
	    	int ign;
	    	IGNORE_DATA *inode;

	    	/* Get the name */
	    	temp = fread_string(fp);

	    	sprintf(fname, "%s%c/%s", PLAYER_DIR,
	    		tolower(temp[0]), capitalize(temp));

	    	/* If there isn't a pfile for the name */
	    	/* then don't add it to the list       */
	    	if(stat(fname, &fst) == -1)
	    	{
	    		fMatch = TRUE;
	    		break;
	    	}

	    	/* Count the number of names already ignored */
	    	for(ign = 0, inode = ch->pcdata->first_ignored; inode;
	    				inode = inode->next)
	    	{
	    		ign++;
	    	}

	    	/* Add the name unless the limit has been reached */
	    	if(ign >= MAX_IGN)
	    	{
	    		bug("fread_char: too many ignored names");
	    	}
	    	else
	    	{
	    		/* Add the name to the list */
	    		CREATE(inode, IGNORE_DATA, 1);
	    		inode->name = STRALLOC(temp);
	    		inode->next = NULL;
	    		inode->prev = NULL;

	    		LINK(inode, ch->pcdata->first_ignored,
	    			ch->pcdata->last_ignored, next,
	    			prev);
	    	}

	    	fMatch = TRUE;
	    	break;
	    }
	    KEY( "IllegalPK",	ch->pcdata->illegal_pk,	fread_number( fp ) );
	    KEY ( "IMC",	ch->pcdata->imc_deaf,	fread_number( fp ) );
	    KEY ( "IMCAllow",	ch->pcdata->imc_allow,	fread_number( fp ) );
	    KEY ( "IMCDeny",	ch->pcdata->imc_deny,	fread_number( fp ) );
	    KEY ( "ICEListen",	ch->pcdata->ice_listen, fread_string_nohash( fp ) );
	    KEY( "Immune",	ch->immune,		fread_number( fp ) );
            KEY ( "Intimidacion", ch->intimidacion,     fread_number( fp ) );
     	    KEY( "ICQ",	ch->pcdata->icq,	fread_number( fp ) );
	    break;

	case 'K':
	    if ( !strcmp( word, "Killed" ) )
	    {
		fMatch = TRUE;
		if ( killcnt >= MAX_KILLTRACK )
		    bug( "fread_char: killcnt (%d) >= MAX_KILLTRACK", killcnt );
		else
		{
		    ch->pcdata->killed[killcnt].vnum    = fread_number( fp );
		    ch->pcdata->killed[killcnt++].count = fread_number( fp );
		}
	    }
	    break;

	case 'L':
	    KEY( "Level",	ch->level,		fread_number( fp ) );
       KEY( "LongDescr",   ch->long_descr,      fread_string( fp ) );
	    if ( !strcmp( word, "Languages" ) )
	    {
	    	ch->speaks = fread_number( fp );
	    	ch->speaking = fread_number( fp );
	    	fMatch = TRUE;
	    }
	    break;

	case 'M':
	  KEY( "Mana_anterior", ch->pcdata->mana_ant,   fread_number( fp ) );/* OJO No soportado por SMAUG1.4 */
          KEY( "Manipulacion", ch->manipulacion,        fread_number( fp ) );/* OJO No soportado por SMAUG1.4 */
          KEY( "MaxLevel",      ch->pcdata->max_level,  fread_number( fp ) ); /* Maximo nivel del jugador */
          KEY( "Modoficha", ch->modo_ficha,             fread_number( fp ) ); /* OJO No soportado por SMAUG1.4 */
          KEY( "Move_anterior", ch->pcdata->mov_ant,    fread_number( fp ) );/* OJO No soportado por SMAUG1.4 */
	    KEY( "MDeaths",	ch->pcdata->mdeaths,	fread_number( fp ) );
	    KEY( "Mentalstate", ch->mental_state,	fread_number( fp ) );
	    KEY( "MGlory",      ch->pcdata->quest_accum,fread_number( fp ) );
	    KEY( "Minsnoop",	ch->pcdata->min_snoop,	fread_number( fp ) );
	    KEY( "MKills",	ch->pcdata->mkills,	fread_number( fp ) );
	    KEY( "Mobinvis",	ch->mobinvis,		fread_number( fp ) );
            KEY( "Modificador_mental",	ch->pcdata->modificador_mental,		fread_number( fp ) );
	    if ( !strcmp( word, "MobRange" ) )
	    {
		ch->pcdata->m_range_lo = fread_number( fp );
		ch->pcdata->m_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'N':
	    KEY ("Name", ch->name, fread_string( fp ) );
            KEY( "NextQuest",   ch->pcdata->nextquest,          fread_number( fp ) );
            KEY ("NoAffectedBy", ch->no_affected_by, fread_bitvector( fp ) );
            KEY ("NoImmune", ch->no_immune, fread_number( fp ) );
            KEY ("NoResistant", ch->no_resistant, fread_number( fp ) );
            KEY ("NoSusceptible", ch->no_susceptible, fread_number( fp ) );
	    if ( !strcmp ( "Nuisance", word ) )
            {
                fMatch = TRUE;
                CREATE( ch->pcdata->nuisance, NUISANCE_DATA, 1 );
                ch->pcdata->nuisance->time = fread_number( fp );
                ch->pcdata->nuisance->max_time = fread_number( fp );
                ch->pcdata->nuisance->flags = fread_number( fp );
		ch->pcdata->nuisance->power = 1;
            }
	    if ( !strcmp ( "NuisanceNew", word ) )
	    {
		fMatch = TRUE;
		CREATE( ch->pcdata->nuisance, NUISANCE_DATA, 1 );
		ch->pcdata->nuisance->time = fread_number( fp );
		ch->pcdata->nuisance->max_time = fread_number( fp );
		ch->pcdata->nuisance->flags = fread_number( fp );
		ch->pcdata->nuisance->power = fread_number( fp );
	    }
            KEY("NumChiquillos", ch->num_chiquillos, fread_number( fp ));
	    break;
	case 'O':
            KEY( "Ocultismo",    ch->ocultismo,            fread_number( fp ) );
            KEY( "Outcast_time", ch->pcdata->outcast_time, fread_number( fp ) );
	    if ( !strcmp( word, "ObjRange" ) )
	    {
		ch->pcdata->o_range_lo = fread_number( fp );
		ch->pcdata->o_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'P':
	    KEY( "Pagerlen",	ch->pcdata->pagerlen,	fread_number( fp ) );
	    KEY( "Password",	ch->pcdata->pwd,	fread_string_nohash( fp ) );
            KEY( "PCamino",     ch->pcamino,            fread_number( fp ) );
	    KEY( "PDeaths",	ch->pcdata->pdeaths,	fread_number( fp ) );
	    KEY( "PKills",	ch->pcdata->pkills,	fread_number( fp ) );
       KEY( "PksGeneracion", ch->pcdata->genepkills,     fread_number( fp ) );
	    KEY( "Played",	ch->played,		fread_number( fp ) );
	    /* KEY( "Position",	ch->position,		fread_number( fp ) );*/
            /*
             *  new positions are stored in the file from 100 up
             *  old positions are from 0 up
             *  if reading an old position, some translation is necessary
             */
            if (!strcmp ( word, "Position" ) )
            {
               ch->position          = fread_number( fp );
               if(ch->position<100){
                  switch(ch->position){
                      default: ;
                      case 0: ;
                      case 1: ;
                      case 2: ;
                      case 3: ;
                      case 4: break;
                      case 5: ch->position=6; break;
                      case 6: ch->position=8; break;
                      case 7: ch->position=9; break;
                      case 8: ch->position=12; break;
                      case 9: ch->position=13; break;
                      case 10: ch->position=14; break;
                      case 11: ch->position=15; break;
                  }
                  fMatch = TRUE;
               } else {
                  ch->position-=100;
                  fMatch = TRUE;
               }
            }
	    KEY( "Practice",	ch->practice,		fread_number( fp ) );
	    KEY( "Prompt",	ch->pcdata->prompt,	fread_string( fp ) );
	    if (!strcmp ( word, "PTimer" ) )
	    {
		add_timer( ch , TIMER_PKILLED, fread_number(fp), NULL, 0 );
		fMatch = TRUE;
		break;
	    }
	    break;
     case 'Q':
            KEY( "QuestNext",   ch->nextquest,          fread_number( fp ) );
            break;

	case 'R':
	    KEY( "Race",        ch->race,		fread_number( fp ) );
	    KEY( "Rank",        ch->pcdata->rank,	fread_string_nohash( fp ) );
            KEY( "Renacimientos",       ch->pcdata->renacido,     fread_number( fp ) );
            KEY( "Recompensa0",        ch->recompensa[0],      fread_string( fp ) );
            KEY( "Recompensa1",        ch->recompensa[1],      fread_string( fp ) );
	    KEY( "Recompensa2",        ch->recompensa[2],      fread_string( fp ) );
            KEY( "Recompensa3",        ch->recompensa[3],      fread_string( fp ) );
	    KEY( "Recompensa4",        ch->recompensa[4],      fread_string( fp ) );
            KEY( "Recompensa5",        ch->recompensa[5],      fread_string( fp ) );
	    KEY( "Resistant",	ch->resistant,		fread_number( fp ) );
	    KEY( "Restore_time",ch->pcdata->restore_time, fread_number( fp ) );
       KEY( "Revision",    ch->revision, fread_number( fp ) );

	    if ( !strcmp( word, "Room" ) )
	    {
		ch->in_room = get_room_index( fread_number( fp ) );
		if ( !ch->in_room )
		    ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		fMatch = TRUE;
		break;
	    }
	    if ( !strcmp( word, "RoomRange" ) )
	    {
		ch->pcdata->r_range_lo = fread_number( fp );
		ch->pcdata->r_range_hi = fread_number( fp );
		fMatch = TRUE;
	    }
	    break;

	case 'S':
	    KEY( "Sex",		ch->sex,		fread_number( fp ) );
	    KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
                       KEY( "Sire",             ch->sire_ch,            fread_string( fp ) );
                       KEY( "Style",	ch->style,		fread_number( fp ) );
	    KEY( "Susceptible",	ch->susceptible,	fread_number( fp ) );
	    if ( !strcmp( word, "SavingThrow" ) )
	    {
		ch->saving_wand 	= fread_number( fp );
		ch->saving_poison_death = ch->saving_wand;
		ch->saving_para_petri 	= ch->saving_wand;
		ch->saving_breath 	= ch->saving_wand;
		ch->saving_spell_staff 	= ch->saving_wand;
		fMatch = TRUE;
		break;
	    }

	    if ( !strcmp( word, "SavingThrows" ) )
	    {
		ch->saving_poison_death = fread_number( fp );
		ch->saving_wand 	= fread_number( fp );
		ch->saving_para_petri 	= fread_number( fp );
		ch->saving_breath 	= fread_number( fp );
		ch->saving_spell_staff 	= fread_number( fp );
		fMatch = TRUE;
		break;
	    }

	    if ( !strcmp( word, "Site" ) )
	    {
		if ( !preload )
		{
		  sprintf( buf, "Last connected from: %s\n\r", fread_word( fp ) );
		  send_to_char( buf, ch );
		}
		else
		  fread_to_eol( fp );
		fMatch = TRUE;
		if ( preload )
		  word = "End";
		else
		  break;
	    }

	    if ( !strcmp( word, "Skill" ) )
	    {
		int sn;
		int value;

		if ( preload )
		  word = "End";
		else
		{
		  value = fread_number( fp );
		  if ( file_ver < 3 )
		    sn = skill_lookup( fread_word( fp ) );
		  else
		    sn = bsearch_skill_exact( fread_word( fp ), gsn_first_skill, gsn_first_weapon-1 );
		  if ( sn < 0 )
		    bug( "Fread_char: unknown skill.", 0 );
		  else
		  {
		    ch->pcdata->learned[sn] = value;
		    /* Take care of people who have stuff they shouldn't     *
		     * Assumes class and level were loaded before. -- Altrag *
		     * Assumes practices are loaded first too now. -- Altrag */
		    if ( ch->level < LEVEL_IMMORTAL )
		    {
		      if ( skill_table[sn]->skill_level[ch->class] >= LEVEL_IMMORTAL )
		      {
		        ch->pcdata->learned[sn] = 0;
		        ch->practice++;
		      }
		    }
		  }
		  fMatch = TRUE;
		  break;
		}
	    }

	    if ( !strcmp( word, "Spell" ) )
	    {
		int sn;
		int value;

		if ( preload )
		  word = "End";
		else
		{
		  value = fread_number( fp );

                                        sn = bsearch_skill_exact( fread_word( fp ), gsn_first_spell, gsn_first_skill-1 );
		  if ( sn < 0 )
		    bug( "Fread_char: unknown spell.", 0 );
		  else
		  {
		    ch->pcdata->learned[sn] = value;
		    /*if ( ch->level < LEVEL_IMMORTAL )
			if ( skill_table[sn]->skill_level[ch->class] >= LEVEL_IMMORTAL )
			{
			    ch->pcdata->learned[sn] = 0;
			    ch->practice++;
			}*/
		  }

		  fMatch = TRUE;
		  break;
		}
	    }
	    if ( strcmp( word, "End" ) )
		break;

	case 'E':
	    if ( !strcmp( word, "End" ) )
	    {
		if (!ch->short_descr)
		  ch->short_descr	= STRALLOC( "" );
		if (!ch->long_descr)
		  ch->long_descr	= STRALLOC( "" );
		if (!ch->description)
		  ch->description	= STRALLOC( "" );
		if (!ch->pcdata->pwd)
		  ch->pcdata->pwd	= str_dup( "" );
		if (!ch->pcdata->bamfin)
		  ch->pcdata->bamfin	= str_dup( "" );
		if (!ch->pcdata->bamfout)
		  ch->pcdata->bamfout	= str_dup( "" );
		if (!ch->pcdata->bio)
		  ch->pcdata->bio	= STRALLOC( "" );
		if (!ch->pcdata->rank)
		  ch->pcdata->rank	= str_dup( "" );
		if (!ch->pcdata->bestowments)
		  ch->pcdata->bestowments = str_dup( "" );
		if (!ch->pcdata->title)
		  ch->pcdata->title	= STRALLOC( "" );
		if (!ch->pcdata->homepage)
		  ch->pcdata->homepage	= str_dup( "" );
		if (!ch->pcdata->authed_by)
		  ch->pcdata->authed_by = STRALLOC( "" );
		if (!ch->pcdata->prompt )
		  ch->pcdata->prompt	= STRALLOC( "" );
	        if (!ch->pcdata->fprompt )
		  ch->pcdata->fprompt   = STRALLOC( "" );
		ch->editor		= NULL;
		killcnt = URANGE( 2, ((ch->level+3) * MAX_KILLTRACK)/LEVEL_AVATAR, MAX_KILLTRACK );
		if ( killcnt < MAX_KILLTRACK )
		  ch->pcdata->killed[killcnt].vnum = 0;
                if (!ch->pcdata->email)
		  ch->pcdata->email = str_dup( "" );

		/* no good for newbies at all */
		if ( !IS_IMMORTAL( ch ) && !ch->speaking )
			ch->speaking = LANG_COMMON;
		/*	ch->speaking = race_table[ch->race]->language; */
		if ( IS_IMMORTAL( ch ) )
		{
			int i;

			ch->speaks = ~0;
			if ( ch->speaking == 0 )
				ch->speaking = ~0;

			CREATE(ch->pcdata->tell_history, char *, 26);
			for(i = 0; i < 26; i++)
				ch->pcdata->tell_history[i] = NULL;
		}
		if ( !ch->pcdata->prompt )
		  ch->pcdata->prompt = STRALLOC("");

                /* this disallows chars from being 6', 180lbs, but easier than a flag */
                if ( ch->height== 72)
                   ch->height = number_range(race_table[ch->race]->height *.9, race_table[ch->race]->height *1.1);
		if ( ch->weight==180)
		   ch->weight = number_range(race_table[ch->race]->weight *.9, race_table[ch->race]->weight *1.1);

		return;
	    }
	    KEY( "Exp",		ch->exp,		fread_number( fp ) );
       KEY( "Expacumulada",     ch->exp_acumulada,   fread_number( fp ) );
       KEY( "Email",	ch->pcdata->email,	fread_string_nohash( fp ) );
       KEY( "Esferas",		ch->esferas,		fread_bitvector( fp ) );
	    break;

	case 'T':
            KEY( "Tiempo_FVol", ch->tiempo_fv, fread_number( fp ) ); /* Pal tiempo de entreno de la fuerza de voluntad */
            KEY( "tmpTrust", ch->tmptrust, fread_number( fp ) );
	    KEY( "tmpLevel", ch->tmplevel, fread_number( fp ) );   /* pa canear lo dicho */
	    if ( !strcmp( word, "Tongue" ) )
	    {
		int sn;
		int value;

		if ( preload )
		  word = "End";
		else
		{
		  value = fread_number( fp );

		  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_tongue, gsn_top_sn-1 );
		  if ( sn < 0 )
		    bug( "Fread_char: unknown tongue.", 0 );
		  else
		  {
		    ch->pcdata->learned[sn] = value;
		    if ( ch->level < LEVEL_IMMORTAL )
			if ( skill_table[sn]->skill_level[ch->class] >= LEVEL_IMMORTAL )
			{
			    ch->pcdata->learned[sn] = 0;
			    ch->practice++;
			}
		  }
		  fMatch = TRUE;
		}
		break;
	    }
	    KEY( "Trust", ch->trust, fread_number( fp ) );
            /* Let no character be trusted higher than one below maxlevel -- Narn */
	    ch->trust = UMIN( ch->trust, MAX_LEVEL - 1 );

	    if ( !strcmp( word, "Title" ) )
	    {
		ch->pcdata->title = fread_string( fp );
		if ( isalpha(ch->pcdata->title[0])
		||   isdigit(ch->pcdata->title[0]) )
		{
		    sprintf( buf, " %s", ch->pcdata->title );
		    if ( ch->pcdata->title )
		      STRFREE( ch->pcdata->title );
		    ch->pcdata->title = STRALLOC( buf );
		}
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'V':
	    if ( !strcmp( word, "Vnum" ) )
	    {
		ch->pIndexData = get_mob_index( fread_number( fp ) );
		fMatch = TRUE;
		break;
	    }
	    KEY( "Version",	file_ver,		fread_number( fp ) );
            KEY( "Vida_anterior",   ch->pcdata->vida_ant,      fread_number( fp ) );
	    break;

	case 'W':
            KEY( "Weight",        ch->weight,   fread_number( fp ) );
	    if ( !strcmp( word, "Weapon" ) )
	    {
		int sn;
		int value;

		if ( preload )
		  word = "End";
		else
		{
		  value = fread_number( fp );

		  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_weapon, gsn_first_tongue-1 );
		  if ( sn < 0 )
		    bug( "Fread_char: unknown weapon.", 0 );
		  else
		  {
		    ch->pcdata->learned[sn] = value;
		    if ( ch->level < LEVEL_IMMORTAL )
			if ( skill_table[sn]->skill_level[ch->class] >= LEVEL_IMMORTAL )
			{
			    ch->pcdata->learned[sn] = 0;
			    ch->practice++;
			}
		  }
		  fMatch = TRUE;
		}
		break;
	    }
	    KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
	    KEY( "WizInvis",	ch->pcdata->wizinvis,	fread_number( fp ) );
	    break;
	}

	if ( !fMatch )
	{
	    sprintf( buf, "Fread_char: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}


void fread_obj( CHAR_DATA *ch, FILE *fp, sh_int os_type )
{
    OBJ_DATA *obj;
    char *word;
    char buf[MAX_STRING_LENGTH];
    int iNest;
    bool fMatch;
    bool fNest;
    bool fVnum;
    ROOM_INDEX_DATA *room = NULL;

    if ( ch )
	room = ch->in_room;
    CREATE( obj, OBJ_DATA, 1 );
    obj->count		= 1;
    obj->wear_loc	= -1;
    obj->weight		= 1;

    fNest		= TRUE;		/* Requiring a Nest 0 is a waste */
    fVnum		= TRUE;
    iNest		= 0;

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
	    KEY( "ActionDesc", obj->action_desc, 	fread_string( fp ) );
	    if ( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
	    {
		AFFECT_DATA *paf;
		int pafmod;

		CREATE( paf, AFFECT_DATA, 1 );
		if ( !strcmp( word, "Affect" ) )
		{
		    paf->type	= fread_number( fp );
		}
		else
		{
		    int sn;

		    sn = skill_lookup( fread_word( fp ) );
		    if ( sn < 0 )
			bug( "Fread_obj: unknown skill.", 0 );
		    else
			paf->type = sn;
		}
		paf->duration	= fread_number( fp );
		pafmod		= fread_number( fp );
		paf->location	= fread_number( fp );
		paf->bitvector	= fread_bitvector( fp );
		if ( paf->location == APPLY_WEAPONSPELL
		||   paf->location == APPLY_WEARSPELL
		||   paf->location == APPLY_STRIPSN
		||   paf->location == APPLY_REMOVESPELL
		||   paf->location == APPLY_RECURRINGSPELL )
		  paf->modifier		= slot_lookup( pafmod );
		else
		  paf->modifier		= pafmod;
		LINK(paf, obj->first_affect, obj->last_affect, next, prev );
		fMatch				= TRUE;
		break;
	    }
	    break;

	case 'C':
	    KEY( "Cost",	obj->cost,		fread_number( fp ) );
	    KEY( "Count",	obj->count,		fread_number( fp ) );
	    break;

	case 'D':
	    KEY( "Description",	obj->description,	fread_string( fp ) );
	    break;

	case 'E':
	    KEY( "ExtraFlags",	obj->extra_flags,	fread_bitvector( fp ) );

	    if ( !strcmp( word, "ExtraDescr" ) )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		fMatch 				= TRUE;
	    }

	    if ( !strcmp( word, "End" ) )
	    {
		if ( !fNest || !fVnum )
		{
		    if ( obj->name )
			sprintf ( buf, "Fread_obj: %s incomplete object.",
				obj->name );
		    else
			sprintf ( buf, "Fread_obj: incomplete object." );
		    bug( buf, 0 );
		    if ( obj->name )
		      STRFREE( obj->name        );
		    if ( obj->description )
		      STRFREE( obj->description );
		    if ( obj->short_descr )
		      STRFREE( obj->short_descr );
		    DISPOSE( obj );
		    return;
		}
		else
		{
		    sh_int wear_loc = obj->wear_loc;

		    if ( !obj->name )
			obj->name = QUICKLINK( obj->pIndexData->name );
		    if ( !obj->description )
			obj->description = QUICKLINK( obj->pIndexData->description );
		    if ( !obj->short_descr )
			obj->short_descr = QUICKLINK( obj->pIndexData->short_descr );
     /*     if ( !obj->action_desc )
			obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );
		    LINK(obj, first_object, last_object, next, prev );
          obj->pIndexData->count += obj->count;  */
         if ( !obj->action_desc )
			obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );
		    LINK(obj, first_object, last_object, next, prev );
                    if ( ( !IS_OBJ_STAT( obj, ITEM_RARE )
                        && !IS_OBJ_STAT( obj, ITEM_UNIQUE ) )
                        ||  IS_NPC( ch ) )
                    {
		            obj->pIndexData->count += obj->count;
                    }
		    if ( !obj->serial )
		    {
			cur_obj_serial = UMAX((cur_obj_serial + 1 ) & (BV30-1), 1);
			obj->serial = obj->pIndexData->serial = cur_obj_serial;
		    }
		    if ( fNest )
		      rgObjNest[iNest] = obj;
		    numobjsloaded += obj->count;
		    ++physicalobjects;
		    if ( file_ver > 1 || obj->wear_loc < -1
		    ||   obj->wear_loc >= MAX_WEAR )
		      obj->wear_loc = -1;
		    /* Corpse saving. -- Altrag */
		    if ( os_type == OS_CORPSE || os_type == OS_TAQUILLA )
		    {
		        if ( !room )
		        {
                                           if ( os_type == OS_CORPSE )
		          bug( "Fread_obj: Corpse without room", 0);
                                          if ( os_type == OS_TAQUILLA )
		          bug( "Fread_obj: Taquilla sin una habitacion", 0);
		          room = get_room_index(ROOM_VNUM_LIMBO);

		        }
                                      if ( os_type == OS_TAQUILLA )
                                      SET_BIT(obj->value[1], CONT_CLOSED);

			/* Give the corpse a timer if there isn't one */

                                     if ( os_type == OS_CORPSE )
                                     {
			if ( obj->timer < 1 )
			   obj->timer = 40;
			if ( room->vnum == ROOM_VNUM_HALLOFFALLEN
				&& obj->first_content )
			   obj->timer = -1;
                                     }
		        obj = obj_to_room( obj, room );
		    }

		    else if ( iNest == 0 || rgObjNest[iNest] == NULL )
		    {
			int slot = -1;
			bool reslot = FALSE;

			if ( file_ver > 1
			&&   wear_loc > -1
			&&   wear_loc < MAX_WEAR )
			{
			   int x;

			   for ( x = 0; x < MAX_LAYERS; x++ )
			      if ( !save_equipment[wear_loc][x] )
			      {
				  save_equipment[wear_loc][x] = obj;
				  slot = x;
				  reslot = TRUE;
				  break;
			      }
			   if ( x == MAX_LAYERS )
				bug( "Fread_obj: too many layers %d", wear_loc );
			}
			obj = obj_to_char( obj, ch );
			if ( reslot && slot != -1 )
			  save_equipment[wear_loc][slot] = obj;
		    }
		    else
		    {
			if ( rgObjNest[iNest-1] )
			{
			   separate_obj( rgObjNest[iNest-1] );
			   obj = obj_to_obj( obj, rgObjNest[iNest-1] );
			}
			else
			   bug( "Fread_obj: nest layer missing %d", iNest-1 );
		    }
		    if ( fNest )
		      rgObjNest[iNest] = obj;
		    return;
		}
	    }
	    break;

	case 'I':
	    KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
	    break;

	case 'L':
	    KEY( "Level",	obj->level,		fread_number( fp ) );
	    break;

	case 'N':
	    KEY( "Name",	obj->name,		fread_string( fp ) );

	    if ( !strcmp( word, "Nest" ) )
	    {
		iNest = fread_number( fp );
		if ( iNest < 0 || iNest >= MAX_NEST )
		{
		    bug( "Fread_obj: bad nest %d.", iNest );
		    iNest = 0;
		    fNest = FALSE;
		}
		fMatch = TRUE;
	    }
	    break;

        case 'P':
            KEY( "Propietario", obj->propietario,       fread_string( fp ) );
            break;

        case 'R':
	    KEY( "Room", room, get_room_index(fread_number(fp)) );

	case 'S':
	    KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );

	    if ( !strcmp( word, "Spell" ) )
	    {
		int iValue;
		int sn;

		iValue = fread_number( fp );
		sn     = skill_lookup( fread_word( fp ) );
		if ( iValue < 0 || iValue > 5 )
		    bug( "Fread_obj: bad iValue %d.", iValue );
		else if ( sn < 0 )
		    bug( "Fread_obj: unknown skill.", 0 );
		else
		    obj->value[iValue] = sn;
		fMatch = TRUE;
		break;
	    }

	    break;

	case 'T':
	    KEY( "Timer",	obj->timer,		fread_number( fp ) );
	    break;

	case 'V':
	    if ( !strcmp( word, "Values" ) )
	    {
		int x1,x2,x3,x4,x5,x6;
		char *ln = fread_line( fp );

		x1=x2=x3=x4=x5=x6=0;
		sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
		/* clean up some garbage */
		if ( file_ver < 3 )
		   x5=x6=0;

		obj->value[0]	= x1;
		obj->value[1]	= x2;
		obj->value[2]	= x3;
		obj->value[3]	= x4;
		obj->value[4]	= x5;
		obj->value[5]	= x6;
		fMatch		= TRUE;
		break;
	    }

	    if ( !strcmp( word, "Vnum" ) )
	    {
		int vnum;

		vnum = fread_number( fp );
		/*  bug( "Fread_obj: bad vnum %d.", vnum );  */
		if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
		    fVnum = FALSE;
		else
		{
		    fVnum = TRUE;
		    obj->cost = obj->pIndexData->cost;
		    obj->weight = obj->pIndexData->weight;
		    obj->item_type = obj->pIndexData->item_type;
		    obj->wear_flags = obj->pIndexData->wear_flags;
		    obj->extra_flags = obj->pIndexData->extra_flags;
		}
		fMatch = TRUE;
		break;
	    }
	    break;

	case 'W':
	    KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
	    KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
	    KEY( "Weight",	obj->weight,		fread_number( fp ) );
	    break;

	}

	if ( !fMatch )
	{
	    EXTRA_DESCR_DATA *ed;
	    AFFECT_DATA *paf;

	    bug( "Fread_obj: no match.", 0 );
	    bug( word, 0 );
	    fread_to_eol( fp );
	    if ( obj->name )
		STRFREE( obj->name        );
	    if ( obj->description )
		STRFREE( obj->description );
	    if ( obj->short_descr )
		STRFREE( obj->short_descr );
	    while ( (ed=obj->first_extradesc) != NULL )
	    {
		STRFREE( ed->keyword );
		STRFREE( ed->description );
		UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		DISPOSE( ed );
	    }
	    while ( (paf=obj->first_affect) != NULL )
	    {
		UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
		DISPOSE( paf );
	    }
	    DISPOSE( obj );
	    return;
	}
    }
}

void set_alarm( long seconds )
{
#ifdef WIN32
    kill_timer ();    /* kill old timer */
    timer_code = timeSetEvent(seconds * 1000L, 1000, alarm_handler, 0, TIME_PERIODIC);
#else
    alarm( seconds );
#endif
}

/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
void do_last( CHAR_DATA *ch, char *argument )
{
    char buf [MAX_STRING_LENGTH];
    char arg [MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct stat fst;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Sintaxis: ultima <jugador>\n\r", ch );
	return;
    }
    strcpy( name, capitalize(arg) );
    sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name );
    if ( stat( buf, &fst ) != -1 )
      sprintf( buf, "\n\r Ultima conexion: %s\n\r", ctime( &fst.st_mtime ) );
    else
      sprintf( buf, "%s no fue encontrado.\n\r", name );
   send_to_char( buf, ch );
}

/*
 * Added support for removeing so we could take out the write_corpses
 * so we could take it out of the save_char_obj function. --Shaddai
 */

void write_corpses( CHAR_DATA *ch, char *name, OBJ_DATA *objrem )
{
  OBJ_DATA *corpse;
  FILE *fp = NULL;

  /* Name and ch support so that we dont have to have a char to save their
     corpses.. (ie: decayed corpses while offline) */
  if ( ch && IS_NPC(ch) )
  {
    bug( "Write_corpses: writing NPC corpse.", 0 );
    return;
  }
  if ( ch )
    name = ch->name;
  /* Go by vnum, less chance of screwups. -- Altrag */
  for ( corpse = first_object; corpse; corpse = corpse->next )
    if ( corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC &&
         corpse->in_room != NULL &&
        !str_cmp(corpse->short_descr+14, name) &&
	objrem != corpse )
    {
      if ( !fp )
      {
        char buf[127];

        sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
        if ( !(fp = fopen(buf, "w")) )
        {
          bug( "Write_corpses: Cannot open file.", 0 );
          perror(buf);
          return;
        }
      }
      fwrite_obj(ch, corpse, fp, 0, OS_CORPSE);
    }
  if ( fp )
  {
    fprintf(fp, "#END\n\n");
    fclose(fp);
  }
  else
  {
    char buf[127];

    sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
    remove(buf);
  }
  return;
}

void load_corpses( void )
{
  DIR *dp;
  struct dirent *de;
  extern FILE *fpArea;
  extern char strArea[MAX_INPUT_LENGTH];
  extern int falling;

  if ( !(dp = opendir(CORPSE_DIR)) )
  {
    bug( "Load_corpses: can't open CORPSE_DIR", 0);
    perror(CORPSE_DIR);
    return;
  }

  falling = 1; /* Arbitrary, must be >0 though. */
  while ( (de = readdir(dp)) != NULL )
  {
    if ( de->d_name[0] != '.' )
    {
      sprintf(strArea, "%s%s", CORPSE_DIR, de->d_name );
      fprintf(stderr, "Corpse -> %s\n", strArea);
      if ( !(fpArea = fopen(strArea, "r")) )
      {
        perror(strArea);
        continue;
      }
      for ( ; ; )
      {
        char letter;
        char *word;

        letter = fread_letter( fpArea );
        if ( letter == '*' )
        {
          fread_to_eol(fpArea);
          continue;
        }
        if ( letter != '#' )
        {
          bug( "Load_corpses: # not found.", 0 );
          break;
        }
        word = fread_word( fpArea );
        if ( !strcmp(word, "CORPSE" ) )
          fread_obj( NULL, fpArea, OS_CORPSE );
        else if ( !strcmp(word, "OBJECT" ) )
          fread_obj( NULL, fpArea, OS_CARRY );
        else if ( !strcmp( word, "END" ) )
          break;
        else
        {
          bug( "Load_corpses: bad section.", 0 );
          break;
        }
      }
      fclose(fpArea);
    }
  }
  fpArea = NULL;
  strcpy(strArea, "$");
  closedir(dp);
  falling = 0;
  return;
}

/*
 * Guarda los datos de las taquillas
 */

void write_taquillas( CHAR_DATA *ch, char *name, OBJ_DATA *objrem )
{
  OBJ_DATA *taquilla;
  FILE *fp = NULL;

  for ( taquilla = first_object; taquilla; taquilla = taquilla->next )
    if ( taquilla->pIndexData->vnum == OBJ_VNUM_TAQUILLA &&
        !str_cmp(taquilla->name, name) &&
         taquilla->in_room != NULL )
    {
      if ( !fp )
      {
        char buf[127];

        sprintf(buf, "%s%s", TAQUILLAS_DIR, capitalize(name));
        if ( !(fp = fopen(buf, "w+")) )
        {
          bug( "Write_taquillas: No se pudo abrir el archivo.", 0 );
          perror(buf);
          return;
        }
      }
      fwrite_obj(ch, taquilla, fp, 0, OS_TAQUILLA);
    }
  if ( fp )
  {
    fprintf(fp, "#END\n\n");
    fclose(fp);
  }
  else
  {
    char buf[127];

    sprintf(buf, "%s%s", TAQUILLAS_DIR, capitalize(name));
    remove(buf);
  }
  return;
}

void load_taquillas( void )
{
  DIR *dp;
  struct dirent *de;
  extern FILE *fpArea;
  extern char strArea[MAX_INPUT_LENGTH];
  extern int falling;

  if ( !(dp = opendir(TAQUILLAS_DIR)) )
  {
    bug( "Load_taquillas: no se pudo abrir TQUILLAS_DIR", 0);
    perror(TAQUILLAS_DIR);
    return;
  }

  falling = 1;
  while ( (de = readdir(dp)) != NULL )
  {
    if ( de->d_name[0] != '.' )
    {
      sprintf(strArea, "%s%s", TAQUILLAS_DIR, de->d_name );
      fprintf(stderr, "Taquilla -> %s\n", strArea);
      if ( !(fpArea = fopen(strArea, "r")) )
      {
        perror(strArea);
        continue;
      }
      for ( ; ; )
      {
        char letter;
        char *word;

        letter = fread_letter( fpArea );
        if ( letter == '*' )
        {
          fread_to_eol(fpArea);
          continue;
        }
        if ( letter != '#' )
        {
          bug( "Load_taquilla: # no encontrado.", 0 );
          break;
        }
        word = fread_word( fpArea );
        if ( !strcmp(word, "TAQUILLA" ) )
          fread_obj( NULL, fpArea, OS_TAQUILLA );
        else if ( !strcmp(word, "OBJECT" ) )
          fread_obj( NULL, fpArea, OS_CARRY );
        else if ( !strcmp( word, "END" ) )
          break;
        else
        {
          bug( "Load_taquillas: algo falla.", 0 );
          break;
        }
      }
      fclose(fpArea);
    }
  }
  fpArea = NULL;
  strcpy(strArea, "$");
  closedir(dp);
  falling = 0;
  return;
}

/*
 * This will write one mobile structure pointed to be fp --Shaddai
 */

void fwrite_mobile( FILE *fp, CHAR_DATA *mob )
{
  if ( !IS_NPC( mob ) || !fp )
	return;
  fprintf( fp, "#MOBILE\n" );
  fprintf( fp, "Vnum	%d\n", mob->pIndexData->vnum );
  if ( mob->in_room )
	fprintf( fp, "Room	%d\n",
       		(  mob->in_room == get_room_index( ROOM_VNUM_LIMBO )
        		&& mob->was_in_room )
            		? mob->was_in_room->vnum
            		: mob->in_room->vnum );
  if ( QUICKMATCH( mob->name, mob->pIndexData->player_name) == 0 )
        fprintf( fp, "Name     %s~\n", mob->name );
  if ( QUICKMATCH( mob->short_descr, mob->pIndexData->short_descr) == 0 )
  	fprintf( fp, "Short	%s~\n", mob->short_descr );
  if ( QUICKMATCH( mob->long_descr, mob->pIndexData->long_descr) == 0 )
  	fprintf( fp, "Long	%s~\n", mob->long_descr );
  if ( QUICKMATCH( mob->description, mob->pIndexData->description) == 0 )
  	fprintf( fp, "Description %s~\n", mob->description );
  fprintf( fp, "Position %d\n", mob->position );
  fprintf( fp, "Flags %s\n",   print_bitvector(&mob->act) );
/* Might need these later --Shaddai
  de_equip_char( mob );
  re_equip_char( mob );
  */
  if ( mob->first_carrying )
	fwrite_obj( mob, mob->last_carrying, fp, 0, OS_CARRY );
  fprintf( fp, "EndMobile\n" );
  return;
}

/*
 * This will read one mobile structure pointer to by fp --Shaddai
 */
CHAR_DATA *  fread_mobile( FILE *fp )
{
  CHAR_DATA *mob = NULL;
  char *word;
  bool fMatch;
  int inroom = 0;
  ROOM_INDEX_DATA *pRoomIndex = NULL;

  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
  if ( !strcmp(word, "Vnum") )
  {
    int vnum;

    vnum = fread_number( fp );
    mob = create_mobile( get_mob_index(vnum) );
    if ( !mob )
    {
	for ( ; ; ) {
	  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
	  /* So we don't get so many bug messages when something messes up
	   * --Shaddai
	   */
	  if ( !strcmp( word, "EndMobile" ) )
		break;
        }
	bug("Fread_mobile: No index data for vnum %d", vnum );
	return NULL;
    }
  }
  else
  {
	for ( ; ; ) {
	  word   = feof( fp ) ? "EndMobile" : fread_word( fp );
	  /* So we don't get so many bug messages when something messes up
	   * --Shaddai
	   */
	  if ( !strcmp( word, "EndMobile" ) )
		break;
        }
	extract_char(mob, TRUE);
	bug("Fread_mobile: Vnum not found", 0 );
	return NULL;
  }
  for ( ; ;) {
       word   = feof( fp ) ? "EndMobile" : fread_word( fp );
       fMatch = FALSE;
       switch ( UPPER(word[0]) ) {
	case '*':
           fMatch = TRUE;
           fread_to_eol( fp );
           break;
	case '#':
		if ( !strcmp( word, "#OBJECT" ) )
			fread_obj ( mob, fp, OS_CARRY );
	case 'D':
		KEY( "Description", mob->description, fread_string(fp));
		break;
	case 'E':
		if ( !strcmp( word, "EndMobile" ) )
		{
		if ( inroom == 0 )
			inroom = ROOM_VNUM_TEMPLE;
		pRoomIndex = get_room_index( inroom );
		if ( !pRoomIndex )
			pRoomIndex = get_room_index( ROOM_VNUM_TEMPLE );
		char_to_room(mob, pRoomIndex);
		return mob;
		}
		break;
 	case 'F':
		KEY( "Flags", mob->act, fread_bitvector(fp));
	case 'L':
		KEY( "Long", mob->long_descr, fread_string(fp ) );
		break;
	case 'N':
		KEY( "Name", mob->name, fread_string( fp ) );
		break;
	case 'P':
		KEY( "Position", mob->position, fread_number( fp ) );
		break;
	case 'R':
		KEY( "Room",  inroom, fread_number(fp));
		break;
	case 'S':
		KEY( "Short", mob->short_descr, fread_string(fp));
		break;
	}
	if ( !fMatch )
	{
	   bug ( "Fread_mobile: no match.", 0 );
	   bug ( word, 0 );
	}
  }
  return NULL;
}

/*
 * This will write in the saved mobile for a char --Shaddai
 */
void write_char_mobile( CHAR_DATA *ch , char *argument )
{
  FILE *fp;
  CHAR_DATA *mob;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC( ch ) || !ch->pcdata->pet )
	return;

  fclose( fpReserve );
  if ( (fp = fopen( argument, "w")) == NULL )
  {
	sprintf(buf, "Write_char_mobile: couldn't open %s for writing!\n\r",
		argument );
	bug(buf, 0 );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
  }
  mob = ch->pcdata->pet;
  xSET_BIT( mob->affected_by, AFF_CHARM );
  fwrite_mobile( fp, mob );
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

/*
 * This will read in the saved mobile for a char --Shaddai
 */

void read_char_mobile( char *argument )
{
  FILE *fp;
  CHAR_DATA *mob;
  char buf[MAX_STRING_LENGTH];

  fclose( fpReserve );
  if ( (fp = fopen( argument, "r")) == NULL )
  {
	sprintf(buf, "Read_char_mobile: couldn't open %s for reading!\n\r",
		argument );
	bug(buf, 0 );
	fpReserve = fopen( NULL_FILE, "r" );
	return;
  }
  mob = fread_mobile( fp );
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}
