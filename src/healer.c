/**********************************************************************
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@hypercube.org)                            *
*           Gabrielle Taylor (gtaylor@hypercube.org)                       *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***********************************************************************/
/***********************************************************************
* Adaptado a Smaug 1.4 por Desden, el Chaman Tibetano - Noviembre 1998  *
*           ( Jose Luis Sogorb ) Email: jlalbatros@mx2.redestb.es          *
*                                                                          *
***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mud.h"

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	

    /* busca un act_healer */
    for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && xIS_SET(mob->act, ACT_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "No puedes hacer eso aqui.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        /* muestra lista de precios */
	act(AT_PLAIN,"$N dice 'Te puedo curar los siguientes males:'",ch,NULL,mob,TO_CHAR);
	send_to_char("  leve: curar heridas leves          100 oro\n\r",ch);
	send_to_char("  serio: curar heridas serias        160 oro\n\r",ch);
	send_to_char("  critico: curar heridas críticas    250 oro\n\r",ch);
	send_to_char("  sanar: hechizo de sanar           5000 oro\n\r",ch);
	send_to_char("  ceguera: curar ceguera            2000 oro\n\r",ch);
	send_to_char("  veneno:  curar veneno             2500 oro\n\r",ch); 
	send_to_char("  maldecir: curar las maldiciones   5000 oro\n\r",ch);
	send_to_char("  refrescar: restablecer movimiento  500 oro\n\r",ch);
	send_to_char("  mana:  restablecer mana	    1000 oro\n\r",ch);
	send_to_char(" Escribe curar <tipo> para ser sanado.\n\r",ch);
	return;
    }

    if (!str_cmp(arg,"leve"))
    {
        spell = spell_smaug;      
	sn    = skill_lookup("cure light");
	words = "judicandus dies";
	 cost  = 100;
    }

    else if (!str_cmp(arg,"serio"))
    {
	spell = spell_smaug;       
	sn    = skill_lookup("cure serious");
	words = "judicandus gzfuajg";
	cost  = 160;
    }

    else if (!str_cmp(arg,"critico"))
    {
	spell = spell_smaug;         
	sn    = skill_lookup("cure critical");
	words = "judicandus qfuhuqar";
	cost  = 250;
    }

    else if (!str_cmp(arg,"sanar"))
    {
	spell = spell_smaug;
	sn = skill_lookup("heal");
	words = "pzar";
	cost  = 5000;
    }

    else if (!str_cmp(arg,"ceguera"))
    {
	spell = spell_cure_blindness;
	sn    = skill_lookup("cure blindness");
      	words = "judicandus noselacri";		
        cost  = 2000;
    }

    else if (!str_cmp(arg,"veneno"))
    {
	spell = spell_cure_poison;    
	sn    = skill_lookup("cure poison");
	words = "judicandus sausabru";
	cost  = 2500;
    }
	
    else if (!str_cmp(arg,"maldecir")) 
    {
	spell = spell_remove_curse; 
	sn    = skill_lookup("remove curse");
	words = "candussido judifgz";
	cost  = 5000;
    }

      else if (!str_cmp(arg,"mana")) 
     {
         spell = NULL;
         sn = -1;
         words = "energizer";
         cost = 1000;
     }
  
	
    else if (!str_cmp(arg,"refrescar") )
    {
	spell =  spell_smaug;   
	sn    = skill_lookup("refresh");
	words = "candusima"; 
	cost  = 500;
    }

    else 
    {
	act(AT_PLAIN,"$N dice 'Escribe 'curar' para ver una lista de los hechizos.'",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    if (cost > ch->gold )
    {
	act(AT_PLAIN,"$N dice 'No llevas suficiente oro para reclamar mis servicios.'",
	    ch,NULL,mob,TO_CHAR);
	return;
    }
    if (!IS_IMMORTAL(ch))
	WAIT_STATE(ch,PULSE_VIOLENCE);
    ch->gold -= cost;
act(AT_PLAIN,"$n pronuncia las palabras '$T'.",mob,NULL,words,TO_ROOM);
  
    if (spell == NULL)  
   {
  	ch->mana += dice(2,8) +  ch->level / 3;  
	ch->mana += UMIN(ch->level,ch->max_mana - ch->mana);
	send_to_char("Una calida sensacion se apodera de ti.\n\r",ch);
	return;
    }

    if (sn == -1)
	return;
 
     spell(sn,ch->level,mob,ch);

}

