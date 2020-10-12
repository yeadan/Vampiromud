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
 *                          Modulo de Quest                                 *
 ****************************************************************************
 *         Adaptacion del Codigo cedido por Lrd Elder de:                   *
 *                                         -Aurora                          *
 *                                          EternalEmpress@Lostprophecy.com *
 ****************************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 236
#define QUEST_ITEM2 237
#define QUEST_ITEM3 233
#define QUEST_ITEM4 234
#define QUEST_ITEM5 235



/* Prices for each Quest Reward, in glory */

#define QUEST_VALUE1 3500
#define QUEST_VALUE2 2500
#define QUEST_VALUE3 3000
#define QUEST_VALUE4 3000
#define QUEST_VALUE5 15000

/* Local functions */
void generate_quest	args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update	args(( void ));
bool qchance            args(( int num ));
CHAR_DATA *find_quest_mob args(( CHAR_DATA *ch));
OBJ_DATA *find_quest_obj  args(( CHAR_DATA *ch));
OBJ_DATA *gobj_prev;
void quest_list           args((CHAR_DATA *ch));
void quest_buy            args((CHAR_DATA *questman, CHAR_DATA *ch, sh_int number));
void do_setquest          args(( CHAR_DATA *ch, char *argument ));
void do_qstat             args(( CHAR_DATA *ch, char *argument ));
void do_queststat         args(( CHAR_DATA *ch, char *argument));

bool qchance( int num )
{
 if (number_range(1,65) <= num) return TRUE;
 else return FALSE;
}

/* The main quest function */

void do_aquest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *obj1, *obj2, *obj3, *obj4, *obj5;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];

    if (IS_NPC(ch)) return;

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);
    if (arg1[0] == '\0')
	{
	send_to_char("&wMISIONES: COMPRAR LISTA PUNTOS COMPLETO INFO\n\r",ch);
	send_to_char(" TIEMPO PEDIR ANULAR IDENTIFICAR RECLAMAR.\n\r",ch);
	send_to_char("Para mas informacion HELP 'MISIONES'.\n\r",ch);
	return;
	}
    if (!str_prefix(arg1, "info"))
    {
        if (xIS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->pcdata->questmob == -1 && ch->pcdata->questgiver->short_descr != NULL)
	    {
		sprintf(buf, "&wHas &WCOMPLETADO &wtu mision!\n\rVe a ver a &g%s &wantes de que tu tiempo se acabe!\n\r",ch->pcdata->questgiver->short_descr);
		send_to_char(buf, ch);
	    }
	    else if (ch->pcdata->questobj > 0)
	    {
                questinfoobj = get_obj_index(ch->pcdata->questobj);
		if (questinfoobj != NULL)
		{
		    sprintf(buf, "&wEstas en una mision para recuperar &g%s &w de las garras de unos quinquis &wen &W%s&w.\n\r",questinfoobj->name, ch->pcdata->questarea);
		    send_to_char(buf, ch);
		}
		else send_to_char("&wNo has firmado ningun contrato... recuerda\n\rSin contrato no hay asesinato.\n\r",ch);
		return;
	    }
	    else if (ch->pcdata->questmob > 0)
	    {
                questinfo = get_mob_index(ch->pcdata->questmob);
		if (questinfo != NULL)
		{
	            if ( ch->generacion >= 8 )
                    ch_printf(ch, "Te han contratado para asesinar a &g%s &wque se encuentra por &W%s&w en el area de &g%s&w.\n\r",questinfo->short_descr, ch->pcdata->questroom, ch->pcdata->questarea);
                    else
                    ch_printf(ch, "Te han contratado para asesinar a &g%s &wque se encuentra por&W %s &w\n\r",questinfo->short_descr, ch->pcdata->questroom);

		}
		else send_to_char("&wNo has firmado ningun contrato... recuerda\n\rSin contrato no hay asesinato.\n\r",ch);
		return;
	    }
	}
	else
	     send_to_char("&wNo has firmado ningun contrato... recuerda\n\rSin contrato no hay asesinato.\n\r",ch);
	return;
    }
    if (!str_prefix(arg1, "puntos"))
    {
	sprintf(buf, "Tienes %d quest.\n\r",ch->pcdata->quest_curr);
	send_to_char(buf, ch);
	return;
    }
    else if (!str_prefix(arg1, "tiempo"))
    {
        if (!xIS_SET(ch->act, PLR_QUESTOR))
	{
	    send_to_char("&wNo has firmado ningun contrato... recuerda\n\rSin contrato no hay asesinato.\n\r",ch);
	    if (ch->pcdata->nextquest > 1)
	    {
		sprintf(buf, "&wTe quedan %d minutos para poder pedir una mision.\n\r",ch->pcdata->nextquest);
		send_to_char(buf, ch);
	    }
	    else if (ch->pcdata->nextquest == 1)
	    {
		sprintf(buf, "&wTranquilo, te queda menos de un minuto para acabar el quest.\n\r");
		send_to_char(buf, ch);
	    }
	}
        else if (ch->pcdata->countdown > 0)
        {
	    sprintf(buf, "&wTiempo que te queda para que venza el contrato:&g %d&w\n\r",ch->pcdata->countdown);
	    send_to_char(buf, ch);
	}
	return;
    }

/* Check for questmaster in room. */

    for ( questman = ch->in_room->first_person; questman != NULL; questman = questman->next_in_room )
	if (IS_NPC(questman) && xIS_SET(questman->act, ACT_QUESTMASTER))
         break;

    if (!questman)
    {
        send_to_char("No hay ninguna oficina de cazarecompensas por aqui.\n\r",ch);
        return;
    }

    if ( questman->fighting)
    {
	send_to_char("Mhhh... deberia dejar de luchar. No?\n\r",ch);
        return;
    }

    ch->pcdata->questgiver = questman;

    obj1 = get_obj_index(QUEST_ITEM1);
    obj2 = get_obj_index(QUEST_ITEM2);
    obj3 = get_obj_index(QUEST_ITEM3);
    obj4 = get_obj_index(QUEST_ITEM4);
    obj5 = get_obj_index(QUEST_ITEM5);

    if ( !obj1 || !obj2 || !obj3 || !obj4 || !obj5 )
    {
     bug("Error loading quest objects. Char: ", ch->name);
     return;
    }

    if (!str_prefix(arg1, "lista"))
    {
        act(AT_PLAIN,"$n le pregunta a $N por la lista de objetos.",ch,NULL,questman,TO_ROOM);
	act(AT_PLAIN,"Le preguntas a $N por la lista de objetos.",ch,NULL,questman,TO_CHAR);
        quest_list(ch);
	return;
    }

    else if (!str_prefix(arg1, "comprar"))
    {
	if (arg2[0] == '\0')
	{
	    send_to_char("&wPara comprar algun objeto debes poner &g quest comprar &w<&gnum_objeto&g>&w.\n\r",ch);
	    return;
	}
	if (!str_prefix(arg2, "1") && ch->level < LEVEL_AVATAR)
	{
		send_to_char("Para comprar la Ira tienes que ser heroe.\n\r", ch);
		return;
	}
        quest_buy(questman, ch, atoi(arg2));
	return;
    }
    else if (!str_prefix(arg1, "pedir"))
    {
        act(AT_PLAIN,"$n le pregunta a $N si debe cazar a alguien.", ch, NULL, questman, TO_ROOM);
	act(AT_PLAIN,"Le preguntas a $N para ver si tiene alguna caza o algo para ti.",ch, NULL, questman, TO_CHAR);
        if (sysdata.DENY_NEW_PLAYERS == TRUE)
	{
            sprintf(buf, "Lo siento, pero ahora no puedo vender nada. :(");
	    do_say(questman, buf);
	    return;
	}
        if (xIS_SET(ch->act, PLR_QUESTOR))
	{
            sprintf(buf, "Pero si estas en una mision!\n\rDate prisa y acabala!!");
	    do_say(questman, buf);
	    return;
	}
	if (ch->pcdata->nextquest > 0)
	{
	    sprintf(buf, "Asi me gusta la gente, %s, pero el negocio no va bien.",ch->name);
	    do_say(questman, buf);
	    sprintf(buf, "Vuelve de aqui %d minutos.", ch->pcdata->nextquest);
	    do_say(questman, buf);
	    return;
	}
        /*
         * Para impedir que se queden apalancados subiendo puntos quest
         * SiGo
         */
            if((ch->pcdata->quest_accum >= 1000 && ch->level < LEVEL_HERO))
            {
            sprintf(buf, "%s tienes %d puntos conseguidos en el nivel deberas de subir para acumular mas", ch->name, ch->pcdata->quest_accum );
            do_say(questman, buf);
            return;
            }


            if((ch->level <= LEVEL_HERO && !IS_IMMORTAL(ch)))
            {
            if(ch->pcdata->quest_curr >= 20000)
            {
            sprintf(buf, "%s tienes %d puntos acumulados deberas gastarlos para que te de mas misiones", ch->name, ch->pcdata->quest_curr );
            do_say(questman, buf);
            return;
            }
            }



         /*
          * Para impedir que los no pk puedan hacer quest
          * SiGo
          */
           if(!IS_PKILL(ch))
           {
                sprintf(buf, "Los jugadores no pk no sois bienvenidos a este tipo de asuntos %s", ch->name );
                do_say(questman, buf);
                return;
           }


	generate_quest(ch, questman);

        if (ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0)
	{
            ch->pcdata->countdown = number_range(10,30);
            xSET_BIT(ch->act, PLR_QUESTOR);
	    sprintf(buf, "El contrato vence en %d minutos.",ch->pcdata->countdown);
	    do_say(questman, buf);
	    sprintf(buf, "Date prisa");
	    do_say(questman, buf);
	}
	return;
    }
    else if (!str_prefix(arg1, "identificar"))
    {  /* Kayser para identificar objetos de quest */

    	if (arg2[0] == '\0')
    	{
    		send_to_char("Que NUMERO de objeto quieres identificar?\n\r", ch);
    		return;
    	}
	if (!str_prefix(arg2, "1"))
    	{
    		send_to_char("&gObjeto 1: &WLa Ira de los Dioses&g.\n\r", ch);
    		send_to_char("&gSe lleva como &YLUZ&g.\n\r", ch);
    		send_to_char("&gAfecta: &wvida&g, &wmana &gy &wmove &gen &W2000&g.\n\r", ch);
    		send_to_char("&gAfecta: &wsaves &gen &W-10&g.\n\r", ch);
    		send_to_char("&gAfecta: &warmadura &gen &W-300&g.\n\r", ch);
    		send_to_char("&gAfecta: &wdamroll &gy &whitroll &gen &W150&g.\n\r", ch);
    		return;
    	}
    	if (!str_prefix(arg2, "2"))
    	{
    		send_to_char("&gObjeto 2: &WUna Lenta y Agonizante Muerte&g.\n\r", ch);
    		send_to_char("&gSe lleva como &BESCUDO&g.\n\r", ch);
    		send_to_char("&gArmadura: &W50&g.\n\r", ch);
    		send_to_char("&gAfecta: &wvida &gy &wmana &gen &W300&g.\n\r", ch);
    		send_to_char("&gAfecta: &wsaves vs spell &gy &wbreath &gen &W-8&g.\n\r", ch);
    		send_to_char("&gAfecta: &wdamroll &gy &whitroll &gen &W30&g.\n\r", ch);
    		return;
    	}
    	if (!str_prefix(arg2, "3"))
    	{
    		send_to_char("&gObjeto 3: &WLa EsPaDa De CaiN&g.\n\r", ch);
    		send_to_char("&gSe lleva como &BGUANTES&g.\n\r", ch);
    		send_to_char("&gArmadura: &W25&g.\n\r", ch);
    		send_to_char("&gAfecta: &wvida &gen &W250&g.\n\r", ch);
    		send_to_char("&gAfecta: &wmana &gen &W300&g.\n\r", ch);
    		send_to_char("&gAfecta: &wmove &gen &W-300&g.\n\r", ch);
    		send_to_char("&gAfecta: &wsaves vs breath &gen &W-10&g.\n\r", ch);
		send_to_char("&gAfecta: &wsaves vs paralisis &gen &W-10&g.\n\r", ch);
		send_to_char("&gAfecta: &wdamroll &gy &whitroll &gen &W25&g.\n\r", ch);
		send_to_char("&gAfecta: &wresistencia &ga &rfuego&g.\n\r", ch);
		return;
    	}
    	if (!str_prefix(arg2, "4"))
    	{
    		send_to_char("&gObjeto 4: &WEl SiMBoLo De LoS DioSeS&g.\n\r", ch);
    		send_to_char("&gSe lleva como &BCAPARAZON&g.\n\r", ch);
    		send_to_char("&gArmadura: &W80&g.\n\r", ch);
    		send_to_char("&gAfecta: &wvida &gy &wmana &gen &W400&g.\n\r", ch);
    		send_to_char("&gAfecta: &wconstitucion &gen &W5&g.\n\r", ch);
    		send_to_char("&gAfecta: &wdamroll &gen &W30&g.\n\r", ch);
    		return;
    	}
    	if (!str_prefix(arg2, "5"))
    	{
    		send_to_char("Objeto 5: &WEl PoDeR CeLeSTiaL&g.\n\r", ch);
    		send_to_char("&gSe lleva como &BDAGA&g.\n\r", ch);
    		send_to_char("&gDanyo: &wmedia &W56&g.\n\r", ch);
    		send_to_char("&gAfecta: &wdamroll &gy &whitroll &gen &W50&g.\n\r", ch);
    		send_to_char("&gAfecta: &Genvenenado&g.\n\r", ch);
    		return;
    	}
    	else
    	{
    		send_to_char("Los objetos de quest identificables van del 1 al 5.\n\r", ch);
	    	return;
	}
    }
    else if (!str_prefix(arg1, "anular"))
    {
        act(AT_PLAIN,"$n informa a $N de que rompe el contrato.", ch, NULL, questman,
		TO_ROOM);
        act(AT_PLAIN,"Informas a $N de que rompes el contrato.",ch, NULL,
		questman, TO_CHAR);
	if (ch->pcdata->questgiver != questman)
	{
	    sprintf(buf, "No tengo ningun contrato firmado contigo, asi que no me la ralles.");
	    do_say(questman,buf);
	    return;
	}
        if (!xIS_SET(ch->act, PLR_QUESTOR))
	{
	        ch->pcdata->questgiver = NULL;
	        ch->pcdata->countdown = 0;
	        ch->pcdata->questmob = 0;
		ch->pcdata->questobj = 0;
	        sprintf(buf, "Debes firmar un contrato primero si pretendes romperlo.");
	        do_say(questman,buf);

	        return;
	    }

        if (xIS_SET(ch->act, PLR_QUESTOR))
	{
                xREMOVE_BIT(ch->act, PLR_QUESTOR);
	        ch->pcdata->questgiver = NULL;
	        ch->pcdata->countdown = 0;
	        ch->pcdata->questmob = 0;
		ch->pcdata->questobj = 0;
	        ch->pcdata->nextquest = 10;
	        sprintf(buf, "A vale, ha sido un placer... tira pa la puta calle ya capullo!.");
	        do_say(questman,buf);
                sprintf(buf, "No quiero volver a ver tu puto culo aqui hasta que se me pase el enfado, lo menos %d minutos.", ch->pcdata->nextquest);
                do_say(questman, buf);

	        return;
	    }
    }

    else if (!str_prefix(arg1, "reclamar" ) )
        quest_reclamar( ch, NULL );


    else if (!str_prefix(arg1, "completo"))
    {
        act(AT_PLAIN,"$n le comenta a $N de que ha cumplido con su parte del contrato.", ch, NULL, questman,
		TO_ROOM);
        act(AT_PLAIN,"Dejas caer de forma gracil a $N lo de la pasta que tiene que darte.",ch, NULL,
		questman, TO_CHAR);
	if (ch->pcdata->questgiver != questman)
	{
	    sprintf(buf, "Pero que dices??!! Pero si yo a ti no te he visto en la puta vida...");
	    do_say(questman,buf);
	    return;
	}
        if (xIS_SET(ch->act, PLR_QUESTOR))
	{
	    if (ch->pcdata->questmob == -1 && ch->pcdata->countdown > 0)
	    {
		int reward, pointreward, pracreward;

                reward = number_range(500,15000);
                pointreward = number_range(15,50);

      if ( (pointreward >= 20) && (pointreward <= 25)  )
      	reward = number_range(5000, 50000);
      if ( (reward <= 4500) && (reward >= 3600) )
      	pointreward = number_range(150,200);
      if ( (reward <= 4500) && (reward >= 4100) )
       pointreward = number_range(500,550);
		sprintf(buf, "Hombre! ya veo que has cumplido tu contrato.");
		do_say(questman,buf);
		sprintf(buf,"Te voy a pagar lo acordado (leete la letra pekenya he), son %d puntos quest y %d monedas.",pointreward,reward);
		do_say(questman,buf);
                /*sprintf(buf, "[INFO] %s has completed a quest!", ch->name);
                echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );  //Yzek*/
                if (qchance(15))
		{
                    pracreward = number_range(1,10);
		    sprintf(buf, "Ganas %d practicas!\n\r",pracreward);
		    send_to_char(buf, ch);
		    ch->practice += pracreward;
		}

                xREMOVE_BIT(ch->act, PLR_QUESTOR);
	        ch->pcdata->questgiver = NULL;
	        ch->pcdata->countdown = 0;
	        ch->pcdata->questmob = 0;
		ch->pcdata->questobj = 0;
	        ch->pcdata->nextquest = 5;
		ch->gold += reward;
		ch->pcdata->quest_curr += pointreward;

	        return;
	    }
	    else if (ch->pcdata->questobj > 0 && ch->pcdata->countdown > 0)
	    {
		bool obj_found = FALSE;

                for (obj = ch->first_carrying; obj != NULL; obj=obj_next)
    		{
                    obj_next = obj->next_content;

		    if (obj != NULL && obj->pIndexData->vnum == ch->pcdata->questobj)
		    {
			obj_found = TRUE;
            	        break;
		    }
        	}
		if (obj_found == TRUE)
		{
		    int reward, pointreward, pracreward;

                    reward = number_range(500,10000);
                    pointreward = number_range(10,50);

		    act(AT_PLAIN,"Le das $p a $N tal y como habiais acordado.",ch, obj, questman, TO_CHAR);
		    act(AT_PLAIN,"$n le da $p a $N. Que chanchullos se traeran?",ch, obj, questman, TO_ROOM);

	    	    sprintf(buf, "Enhorabuena! Has conseguido cumplir el contrato");
		    do_say(questman,buf);
		    sprintf(buf,"Lo tuyo eran... %d puntos quest y %d oro, no? A ver... Si, toma.",pointreward,reward);
		    do_say(questman,buf);
                    /*sprintf(buf, "[INFO] %s has completed a quest!", ch->name);
                    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );  //Yzek*/
                    if (qchance(20))
		    {
		        pracreward = number_range(1,5);
		        sprintf(buf, "Ah y toma, te regalo %d practicas!!\n\r",pracreward);
		        send_to_char(buf, ch);
		        ch->practice += pracreward;
		    }

                    xREMOVE_BIT(ch->act, PLR_QUESTOR);
	            ch->pcdata->questgiver = NULL;
	            ch->pcdata->countdown = 0;
	            ch->pcdata->questmob = 0;
		    ch->pcdata->questobj = 0;
	            ch->pcdata->nextquest = 5;
	            ch->gold += reward;
		    ch->pcdata->quest_curr += pointreward;
		    extract_obj(obj);
		    return;
		}
		else
		{
		    sprintf(buf, "Pero si aun no has cumplido tu parte del contrato1");
		    do_say(questman, buf);
		    return;
		}
		return;
	    }
	    else if ((ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0) && ch->pcdata->countdown > 0)
	    {
		sprintf(buf, "Pero si aun no has cumplido tu parte del contrato2");
		do_say(questman, buf);
		return;
	    }
	}
	if (ch->pcdata->nextquest > 0)
	 sprintf(buf, "Pero si aun no has cumplido tu parte del contrato");
	else sprintf(buf, "Para eso necesitas tener un contrato firmado conmigo, %s.",ch->name);
	 do_say(questman, buf);
	return;
    }

    send_to_char("&wMISION: COMPRAR LISTA PUNTOS COMPLETO TIEMPO IDENTIFICAR PEDIR ANULAR INFO RECLAMAR.\n\r",ch);
    send_to_char("Para mas informacion HELP 'MISIONES'.\n\r",ch);
    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim=NULL;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    CHAR_DATA *contratante;
  int rnd;
  int contra;
  int counter, mob_vnum, level_diff;
  int muertos0, muertos1, muertos2;
    char buf[MAX_STRING_LENGTH];

    /*  40 % chance it will send the player on a 'recover any item' quest. */
    if (qchance(25))
    {

        questitem = find_quest_obj( ch);
        victim = questitem->carried_by;

        if (!victim || !questitem)
        {
	    sprintf(buf, "Lo siento pero ahora no tengo putas ganas de trabajar ven mas tarde.");
	    do_say(questman, buf);
	    sprintf(buf, "Y traeme un cafe.");
	    do_say(questman, buf);
            ch->pcdata->questmob = 0;
            ch->pcdata->questobj = 0;
	    ch->pcdata->nextquest = 5;
            return;
        }

        room = victim->in_room;
	/*STRFREE( questitem->creator );*/
        /*
         * Bug arreglado por SiGo y SaNgUi
         */
        questitem->creator = NULL;

	questitem->creator = STRALLOC( ch->name );
	ch->pcdata->questobj = questitem->pIndexData->vnum;

        sprintf(buf, "Te contrato para recuperar %s que me lo han mangao unos quinquis.",questitem->short_descr);
	do_say(questman, buf);
	do_say(questman, "Los desgraciaos esos muertos de hambre suelen frecuentar por los siguientes lugares...");
	sprintf(buf, "Mira por el area de %s %s que puede que lo lleve %s, en %s.",room->area->name, questitem->short_descr, victim->short_descr, room->name);
        ch->pcdata->questroom = STRALLOC(room->name);
        ch->pcdata->questarea = STRALLOC(room->area->name);
	do_say(questman, buf);
        sprintf(buf, "Oie %s, una cosa antes de irte... Puedes comerme el miembro?, es una clausula...", ch->name);
        do_say(questman, buf);
	return;
    }
    /* Quest to kill a mob */
    else
    {
        victim = find_quest_mob(ch);

        if (!victim)
        {
    	    sprintf(buf, "Lo siento pero ahora no tengo putas ganas de trabajar ven mas tarde.");
	    do_say(questman, buf);
	    sprintf(buf, "Pos eso que vengas luego pesao.");
	    do_say(questman, buf);
            ch->pcdata->questmob = 0;
            ch->pcdata->questobj = 0;
	    ch->pcdata->nextquest = 5;
            return;
        }

        room = victim->in_room;
        /*
        switch(number_range(0,1))
        {
	    case 0:
             sprintf(buf, "%s has reneged upon a contract with me.",victim->short_descr);
             do_say(questman, buf);
             do_say(questman,"The penalty for this is death, and you are to deliver the sentence.");
	    break;

	    case 1:
	     do_say(questman,"I have a contract to carry out an assassination.");
             sprintf(buf, "You are to eliminate %s.",victim->short_descr);
	     do_say(questman, buf);
	    break;
        }

        if (room->name != NULL)
        {
            sprintf(buf, "Seek %s out somewhere in the vicinity of %s!",victim->short_descr,room->name);
            do_say(questman, buf);
	    sprintf(buf, "That location is in the general area of %s.",room->area->name);
            ch->pcdata->questroom = room->name;
            ch->pcdata->questarea = room->area->name;
	    do_say(questman, buf);
        }
        */
        if (room->name != NULL)
        {
            ch->pcdata->questroom = STRALLOC( room->name );
            ch->pcdata->questarea = STRALLOC(room->area->name);
	}
        ch->pcdata->questmob = victim->pIndexData->vnum;

  /* Ristrera de numeros aleatorios */

  contra = number_range( 1, 21000 );
  rnd = number_range( 1, 6 );
  muertos0 = number_range(1, 500);
  muertos1 = number_range(500, 5000);
  muertos2 = number_range(999, 9999999);



     for (counter = 0; counter < 2000; counter ++)
     {
	mob_vnum = number_range(50, 32000);

	if ( (contratante = get_mob(mob_vnum) ) != NULL )
	{
	    level_diff = contratante->level - ch->level;

            if (((level_diff < questmaster->rango_nivel && level_diff > -questmaster->rango_nivel)
                || (ch->level > 30 && ch->level < 40 && contratante->level > 30 && contratante->level < 50)
                || (ch->level > 40 && contratante->level > 40))
	        && contratante->pIndexData->pShop == NULL
		&& contratante->pIndexData->rShop == NULL
    		&& !xIS_SET( contratante->act, ACT_PRACTICE)
                && !IS_SET( contratante->in_room->room_flags, ROOM_SAFE)
                && in_hard_range(ch, contratante->in_room->area)
                && !xIS_SET( contratante->act, ACT_NOQUEST))
             break;
	    else
             continue;
	}
    }


  if (!contratante)
   rnd = number_range( 1, 3 );


   switch(rnd)
   {
     case 1:
      ch_printf( ch, "&c%s &wte dice'\n\r", questman->short_descr );
      ch_printf( ch, "&wVamos a ver %s, por aqui tengo una oferta de trabajo, a ver espera que busque...\n\r", ch->name );
      ch_printf( ch, "Si aqui esta... Me ha llegado esta orden de la guardia celestial para acabar con\n\r" );
      ch_printf( ch, "%s, el muy cerdo ha acabado con la vida de %d personas en muy pocas horas, deberias\n\r", victim->short_descr, muertos0 );
      ch_printf( ch, "ir con bastante cuidado, esta en %s.\n\r", ch->pcdata->questroom );
      if (ch->generacion <= 7)
      ch_printf( ch, "Eso esta en el area de %s.&c'&w\n\r", ch->pcdata->questarea );
      break;

     case 2:
      ch_printf( ch, "&c%s te dice'\n\r", questman->short_descr );
      ch_printf( ch, "&wVamos a ver %s, por aqui tengo una oferta de trabajo, a ver espera que busque...\n\r", ch->name );
      ch_printf( ch, "Si aqui esta... Me ha llegado esta orden de la guardia celestial para acabar con\n\r" );
      ch_printf( ch, "%s, el muy cerdo ha acabado con la vida de %d personas en muy pocas horas, deberias\n\r", victim->short_descr, muertos1 );
      ch_printf( ch, "ir con bastante cuidado, esta en %s.\n\r", ch->pcdata->questroom );
      if (ch->generacion <= 7)
      ch_printf( ch, "Eso esta en el area de %s.&c'&w\n\r", ch->pcdata->questarea );
      break;

     case 3:
      ch_printf( ch, "&c%s te dice'&w\n\r", questman->short_descr );
      send_to_char( "&wHe recibido una carta (con extranyos polvos blancos) de &gBin Laden&w en la que\n\r", ch);
      ch_printf( ch, "&wnecesita alguien para estrella... digo...esto matar y traspasar a %s\n\r", victim->short_descr );
      ch_printf( ch, "es un yanki de kojones y se ha cargao a %d afganos inocentes, ha bombardeado\n\r", muertos2 );
      ch_printf( ch, "%d casas de civiles y nosekuantas unidades de la Cruz &rRoja&w\n\r", muertos0 );
      ch_printf( ch, "Se esconde en la Casa &Wbla&w... esto en %s&c'&w\n\r", ch->pcdata->questroom );
      if (ch->generacion <= 7)
      ch_printf( ch, "Eso esta en Estados uni... esto en el area de %s.&c'&w\n\r", ch->pcdata->questarea );
      break;

     case 4 :
      ch_printf( ch, "&c%s te dice'\n\r", questman->short_descr );
      ch_printf( ch, "&wHe recibido una oferta de %s, para cabar con la vida de %s\n\r", contratante->short_descr, victim->short_descr );
      ch_printf( ch, "El hijo de perra ese, ha masacrado y mutilado a %d abuelitas del inserso\n\r", muertos0 );
      ch_printf( ch, "y monjitas de clausura,es una busqueda bastante urgente, asi que deberas\n\r" );
      ch_printf( ch, "darte prisa, y en este asunto no quiero fallos o no te volvere a dar\n\r" );
      ch_printf( ch, "un puto trabajo mas, %s esta por los alrededores de\n\r", victim->short_descr );
      ch_printf( ch, "%s en un sitio llamado %s. Adeu!&c'&w\n\r", ch->pcdata->questarea, ch->pcdata->questroom );
      break;

      case 5 :
      ch_printf( ch, "&c%s te dice'\n\r", questman->short_descr );
      ch_printf( ch, "&wMe acaba de llegar un fax urgentisimo de %s,\n\r", contratante->short_descr );
      ch_printf( ch, "quiere que rajes a ese jodido, %s, el muy capullo se cree\n\r", victim->short_descr );
      ch_printf( ch, "que puede ir por ahi debiendole %d monedas de oro al menda.\n\r", muertos2 );
      ch_printf( ch, "Sacale las entranyas por mi, su culo estara merodeando por %s\n\r", ch->pcdata->questroom );
      if (ch->generacion <= 7)
      ch_printf( ch, "eso esta en el area de %s.\n\r", ch->pcdata->questarea );

      send_to_char("Ten suerte en tu buskeda.... o te sakare las putas entranyas.&c'&w\n\r", ch );
      break;

      /*
       * Un pekenyo y modesto omenaje a Scary Movie
       */
      case 6 :
      ch_printf( ch, "&c%s te dice'\n\r", questman->short_descr );
      ch_printf( ch, "&wAhiba dio!!!Me ha llegado un trabajo urgentisimo relacionado con %s,\n\r", contratante->short_descr );
      ch_printf( ch, "desea que encuentres y mates a, %s,creen que es un sicario de\n\r", victim->short_descr );
      ch_printf( ch, "HaNNiBaL LeCTeR que se ha comido a %d abuelitas y senyoras de buen ver.\n\r", muertos0 );
      ch_printf( ch, "Si te dice que quiere comerte la polla, no lo hagas, mira en %s\n\r", ch->pcdata->questroom );
      if (ch->generacion <= 7)
      ch_printf( ch, "eso esta en el area de %s.\n\r", ch->pcdata->questarea );

      send_to_char("Bon apetit!&c'&w\n\r", ch );
      break;

   }

    }
    return;
}

/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    CHAR_DATA *ch, *ch_next;

    for ( ch = first_char; ch != NULL; ch = ch_next )
    {
        ch_next = ch->next;

	if (IS_NPC(ch))
          continue;

	if (ch->pcdata->nextquest > 0)
	{
	    ch->pcdata->nextquest--;

	    if (ch->pcdata->nextquest == 0)
	    {
	        send_to_char("Ya puedes volver a pedir una nueva mision.\n\r",ch);
	        return;
	    }
	}
        else if (xIS_SET(ch->act,PLR_QUESTOR))
        {
	    if (--ch->pcdata->countdown <= 0)
	    {
    	        char buf [MAX_STRING_LENGTH];

	        ch->pcdata->nextquest = 5;
	        sprintf(buf, "Se te ha acabado el tiempo de contrato para tu mision!\n\rPodras volver a las oficinas del paro en %d minutos.\n\r",ch->pcdata->nextquest);
	        send_to_char(buf, ch);
                xREMOVE_BIT(ch->act, PLR_QUESTOR);
                /*STRFREE(ch->pcdata->questroom);
                STRFREE(ch->pcdata->questarea);*/
               /*
                * bug arreglado por SaNgUiNaRi y SiGo
                */
                ch->pcdata->questroom = NULL;
                ch->pcdata->questarea = NULL;
                ch->pcdata->questgiver = NULL;
                ch->pcdata->countdown = 0;
                ch->pcdata->questmob = 0;
	    }
	    if (ch->pcdata->countdown > 0 && ch->pcdata->countdown < 3)
	    {
	        ch_printf(ch, "Tu Jefe te dice telepaticamente'Me cagon tus muelas %s, que se te acaba el tiempo!'\n\r", ch->name);
	        return;
	    }
        }
    }
    return;
}

void do_qstat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
	send_to_char( "Qstat a quien?\n\r", ch );
	return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
	send_to_char( "No esta aqui.\n\r", ch );
	return;
    }

    if ( IS_NPC(victim) )
    {
    send_to_char( "Los Mobs no pueden hacer Quest.\n\r", ch );
    return;
    }

    if (!xIS_SET(victim->act, PLR_QUESTOR))
    {
     sprintf(buf, "%s no esta en un quest, debera de esperar %d minutos para hacerlo.\n\r",victim->name, victim->pcdata->nextquest );
     send_to_char(buf, ch);
     return;
    }
    sprintf(buf, "A %s le quedan %d minutos para finalizar su contrato.\n\r",victim->name, victim->pcdata->countdown );
    send_to_char(buf, ch);

    questinfoobj = get_obj_index(victim->pcdata->questobj);
    if (questinfoobj != NULL)
    {
     sprintf(buf, "%s tiene un contrato firmado para encontrar %s.\n\rObjeto Vnum: %d\n\rEsta en %s lo carga %s.\n\r",victim->name, questinfoobj->short_descr, questinfoobj->vnum, victim->pcdata->questroom, victim->pcdata->questarea );
     send_to_char(buf, ch);
    }

    questinfo = get_mob_index(victim->pcdata->questmob);
    if (questinfo != NULL)
    {
     sprintf(buf, "%s tiene un contrato firmado para asesinar a %s.\n\rMob Vnum: %d\n\rEsta en %s Area %s.\n\r",victim->name, questinfo->short_descr, questinfo->vnum, victim->pcdata->questroom, victim->pcdata->questarea );
     send_to_char(buf, ch);
    }
   return;
}

/* setquest <victim> <mob|obj> <vnum> <location name> */

void do_setquest( CHAR_DATA *ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int vnum;
  bool object=TRUE;

  set_char_color( AT_IMMORT, ch );

  if ( IS_NPC(ch) )
  {
    send_to_char( "Pero si eres un mob de mierda nen.\n\r", ch );
    return;
  }
  if ( get_trust( ch ) < LEVEL_IMMORTAL )
  {
    send_to_char( "Huh?\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  vnum = atoi( arg3 );
  if ( arg[0] == '\0' || arg2[0] == '\0' || vnum <= 0)
  {
    send_to_char( "Syntaxis: setquest <jugador> <mob|obj> <vnum> <localizacion>\n\r", ch );
    send_to_char( "El valor del Vnum debe de ser real.\n\r", ch );
    return;
  }
  if ( ( victim = get_char_world( ch, arg ) ) == NULL )
  {
    send_to_char( "Ese no esta conectado, deja el vinillo chaval.\n\r", ch );
    return;
  }
  if ( IS_NPC( victim ) )
  {
    send_to_char( "A un mob? Tu estas bien de la chota?\n\r", ch );
    return;
  }

  set_char_color( AT_IMMORT, victim );
  if ( nifty_is_name_prefix( arg2, "mob" ) )
    object = FALSE;
  else if ( nifty_is_name_prefix( arg2, "obj" ) )
    object = TRUE;
  else
  {
    do_setquest( ch, "" );
    return;
  }


  if ( object )
  {
   if (obj_by_vnum(vnum) == NULL)
   {
    send_to_char( "No hay ningun objeto asi en el MUD.\n\r", ch );
    return;
   }
   if (obj_by_vnum(vnum)->carried_by)
   {
    victim->pcdata->questarea = STRALLOC(obj_by_vnum(vnum)->carried_by->in_room->area->name);
    victim->pcdata->questroom = STRALLOC(obj_by_vnum(vnum)->carried_by->in_room->name);
   }
   if (obj_by_vnum(vnum)->in_room)
   {
    victim->pcdata->questarea = STRALLOC(obj_by_vnum(vnum)->in_room->area->name);
    victim->pcdata->questroom = STRALLOC(obj_by_vnum(vnum)->in_room->name);
   }

   victim->pcdata->questmob = 0;
   victim->pcdata->questobj = vnum;
   ch_printf( ch, "Contratas a %s para que busque %s en %s en el area de %s\n\r", victim->name, obj_by_vnum(vnum)->short_descr, victim->pcdata->questroom, victim->pcdata->questarea );
   ch_printf( victim, "%s te ha contratado para que encuentres %s en %s en el area de %s\n\r",ch->name, obj_by_vnum(vnum)->short_descr, victim->pcdata->questroom, victim->pcdata->questarea );
  }
  else
  {
   if (get_mob(vnum) == NULL)
   {
    send_to_char( "Ese Mob no existe mameluco.\n\r", ch );
    return;
   }
   victim->pcdata->questarea = STRALLOC(get_mob(vnum)->in_room->area->name);
   victim->pcdata->questroom = STRALLOC(get_mob(vnum)->in_room->name);
   victim->pcdata->questobj = 0;
   victim->pcdata->questmob = vnum;
   ch_printf( ch, "Contratas a %s para que asesine a %s, en %s en el area de %s\n\r", victim->name, get_mob(vnum)->short_descr, victim->pcdata->questroom, victim->pcdata->questarea );
   ch_printf( victim, "%s te ha contratado para que asesines a %s en %s en el area de %s\n\r", ch->name, get_mob(vnum)->short_descr, victim->pcdata->questroom, victim->pcdata->questarea );
  }
  xSET_BIT(victim->act, PLR_QUESTOR);
  victim->pcdata->nextquest = 0;
  victim->pcdata->countdown = 15;

  return;
}

CHAR_DATA *find_quest_mob( CHAR_DATA *ch)
{
  CHAR_DATA *victim=NULL;
  int counter, mob_vnum, level_diff;

     for (counter = 0; counter < 2000; counter ++)
     {
	mob_vnum = number_range(50, 32000);

	if ( (victim = get_mob(mob_vnum) ) != NULL )
	{
	    level_diff = victim->level - ch->level;

            if (((level_diff < questmaster->rango_nivel && level_diff > -questmaster->rango_nivel)
                || (ch->level > 30 && ch->level < 40 && victim->level > 30 && victim->level < 50)
                || (ch->level > 40 && victim->level > 40))
	        && victim->pIndexData->pShop == NULL
		&& victim->pIndexData->rShop == NULL
    		&& !xIS_SET( victim->act, ACT_PRACTICE)
                && !IS_SET( victim->in_room->room_flags, ROOM_SAFE)
                && in_hard_range(ch, victim->in_room->area)
                && !xIS_SET( victim->act, ACT_NOQUEST)
                && !xIS_SET( victim->act, ACT_PET)
    		&& !xIS_SET( victim->act, ACT_IMMORTAL)
                && !xIS_SET( victim->act, ACT_PROTOTYPE))
             return victim;
	    else
             continue;
	}
    }
  return victim;
}

OBJ_DATA *find_quest_obj( CHAR_DATA *ch)
{
  CHAR_DATA *victim=NULL;
  int counter, obj_vnum, level_diff;
  OBJ_DATA *obj=NULL;

     for (counter = 0; counter < 2000; counter ++)
     {
        OBJ_DATA *obj;

   	obj_vnum = number_range(50, 32000); /* Raise 32000 to your highest obj vnum */

   	if ( (obj = obj_by_vnum(obj_vnum) ) != NULL )
   	{

            if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE)
            || IS_OBJ_STAT(obj, ITEM_INVENTORY)
            || (obj->timer > 0)
            || !obj->carried_by
            || (obj->carried_by && !IS_NPC(obj->carried_by))
            || IS_OBJ_STAT(obj, ITEM_DEATHROT))
              continue;

            if ( (victim = obj->carried_by ) == NULL )
              continue;
            if (!victim || !IS_NPC(victim))
              continue;

	    level_diff = victim->level - ch->level;

            if (((level_diff < questmaster->rango_nivel && level_diff > -questmaster->rango_nivel)
                || (ch->level > 30 && ch->level < 40 && victim->level > 30 && victim->level < 50)
                || (ch->level > 40 && victim->level > 40))
	        && victim->pIndexData->pShop == NULL
                && victim->pIndexData->rShop == NULL
                && !xIS_SET( victim->act, ACT_PRACTICE)
                && !IS_SET(victim->in_room->room_flags, ROOM_SAFE)
                && in_hard_range(ch, victim->in_room->area)
                && !xIS_SET( victim->act, ACT_NOQUEST)
                && !xIS_SET( victim->act, ACT_PET)
                && !xIS_SET( victim->act, ACT_IMMORTAL))
             return obj;
	    else
             continue;

   	}
     }
  return obj;
}

void quest_list(CHAR_DATA *ch)
{
  sh_int i, count=0;

  ch_printf( ch, "&Y[&W%2s&Y] [&W%-35s&Y] [&W%6s&Y] \n\r", "#", "Objeto", "Coste" );
  for(i = 0; i <= MAX_QDATA; i++)
  {
   if (questmaster->premio_vnum[i] && questmaster->premio_valor[i])
   {
     ++count;
     ch_printf( ch, "&Y[&W%2d&Y] [&g%-35s&Y] [&W%6s&Y] \n\r", count, get_obj_index(questmaster->premio_vnum[i])->short_descr, num_punct(questmaster->premio_valor[i]) );
   }
  }
  ch_printf( ch, "&Y[&W%2d&Y] [%s Oro                     ] [&W%6s&Y]\n\r", count+1, num_punct(questmaster->premio_oro), num_punct(questmaster->coste_oro));
  ch_printf( ch, "&Y[&W%2d&Y] [%d Practicas                       ] [&W%6s&Y]\n\r", count+2, questmaster->num_prac, num_punct(questmaster->coste_prac) );
  return;
}

void quest_buy(CHAR_DATA *questman, CHAR_DATA *ch, sh_int number)
{
  sh_int i, count=0;
  OBJ_DATA *obj;

  for(i = 0; i <= MAX_QDATA; i++)
  {
   if (questmaster->premio_vnum[i] > 0 && questmaster->premio_valor[i] > 0)
   {
     ++count;
     if (count == number)
      break;
   }
  }
  if (number > count+2 || number < 1)
  {
    do_say(questman, "No se si se te ha colado un trozo de miga de pan por el teclao chaval, que conyo dices que quieres?");
    return;
  }

  if (number == (count+1)) /* Gold */
  {
   if (ch->pcdata->quest_curr >= questmaster->coste_oro)
   {
    ch->pcdata->quest_curr -= questmaster->coste_oro;
    ch->gold += questmaster->premio_oro;
    act(AT_MAGIC,"$N le da un saco de monedas de oro a $n.", ch, NULL, questman, TO_ROOM );
    act(AT_MAGIC,"$N te da un saco de monedas de oro.", ch, NULL, questman, TO_CHAR );
    return;
   }
   else
   {
    do_say(questman, "Con los Puntos Quest que tienes mejor te enculo un ratico chati.");
    return;
   }
  }
  else if (number == (count+2)) /* Practices */
  {
   if (ch->pcdata->quest_curr >= questmaster->coste_prac)
   {
    ch->pcdata->quest_curr -= questmaster->coste_prac;
    ch->practice += questmaster->num_prac;
    act(AT_MAGIC,"$N expande su mente para recibir un nuevo conocimiento y un dolor de cabeza.", ch, NULL, questman, TO_ROOM );
    act(AT_MAGIC,"$N abre tu mente y te transmite practicas para aumentar tu conocimiento... que dolor de cabeza.", ch, NULL, questman, TO_CHAR );
    return;
   }
   else
   {
    do_say(questman, "Con los Puntos Quest que tienes mejor te enculo un ratico chati.");
    return;
   }
  }
  else if (ch->pcdata->quest_curr >= questmaster->premio_valor[i] && (questmaster->premio_valor[i] != 0))
  {
   ch->pcdata->quest_curr -= questmaster->premio_valor[i];
   /* Para los heroes y esas cosas.... KAYSER */
   obj = create_object(get_obj_index(questmaster->premio_vnum[i]), ch->level-2);
   obj->propietario = STRALLOC( ch->name );

   act(AT_PLAIN,"$N le da $p a $n.", ch, obj, questman, TO_ROOM );
   act(AT_PLAIN,"$N te da $p.",   ch, obj, questman, TO_CHAR );
   obj_to_char(obj, ch);
  }
  else
  {
   do_say(questman, "Con los Puntos Quest que tienes mejor te enculo un ratico chati.");
   return;
  }
  return;
}


void fwrite_questmaster()
{
    FILE *fp;
    char filename[256];
    int counter;

    sprintf( filename, "%s", QUEST_FILE );

    fclose( fpReserve );
    if ( ( fp = fopen( filename, "w" ) ) == NULL )
    {
    	bug( "fwrite_questmaster: fopen", 0 );
    	perror( filename );
    }
    fprintf( fp, "Lrange %d\n", questmaster->rango_nivel);
    fprintf( fp, "Pamt %d\n",	questmaster->num_prac);
    fprintf( fp, "Pcost %d\n",	questmaster->coste_prac);
    fprintf( fp, "Gamt %d\n",	questmaster->premio_oro	);
    fprintf( fp, "Gcost %d\n",	questmaster->coste_oro	);
    for(counter = 0; counter <= MAX_QDATA; counter++)
     if (questmaster->premio_valor[counter] && questmaster->premio_vnum[counter])
	fprintf( fp, "Object %d %d\n", questmaster->premio_vnum[counter], questmaster->premio_valor[counter] );
     else
        continue;
    fprintf( fp, "End\n"						);
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

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


void fread_questmaster()
{
    char filename[256];
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool fMatch;
    int vnum=-1, value=-1, counter;

    sprintf( filename, "%s", QUEST_FILE );

    if ( ( fp = fopen( filename, "r" ) ) == NULL )
    {
    	bug( "fread_questmaster: fopen", 0 );
    	perror( filename );
    }
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
	    if ( !str_cmp( word, "End" ) )
            {
                fclose( fp );
                for(counter = vnum; counter != MAX_QDATA; counter++)
                {
		 questmaster->premio_vnum[++vnum] = 0;
		 questmaster->premio_valor[++value] = 0;
                }
		return;
            }
	    break;
	case 'G':
	    KEY( "Gamt",	questmaster->premio_oro,	fread_number( fp ) );
	    KEY( "Gcost",	questmaster->coste_oro,	fread_number( fp ) );
	    break;
	case 'L':
	    KEY( "Lrange",	questmaster->rango_nivel, fread_number( fp ) );
	    break;
	case 'P':
	    KEY( "Pamt",	questmaster->num_prac,	fread_number( fp ) );
	    KEY( "Pcost",	questmaster->coste_prac,	fread_number( fp ) );
	    break;
	case 'O':
	    if ( !str_cmp( word, "Object" ) )
            {
                if (vnum >= MAX_QDATA)
                 break;
		questmaster->premio_vnum[++vnum]	= fread_number( fp );
		questmaster->premio_valor[++value] = fread_number( fp );
		fMatch = TRUE;
                break;
            }
	    break;
	}
	if ( !fMatch )
	{
	    sprintf( buf, "fread_questmaster: no match: %s", word );
	    bug( buf, 0 );
	}
    }
}

#define MIL MAX_INPUT_LENGTH

void do_queststat( CHAR_DATA *ch, char *argument)
{
  char arg1[MIL];
  char arg2[MIL];
  char arg3[MIL];
  char arg4[MIL];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  argument = one_argument( argument, arg4 );

  if (arg1[0] == '\0')
  {
    sh_int i, count=0;

    for(i = 0; i != MAX_QDATA; i++)
    {
       ch_printf( ch, "&p[&w%2d&p] [&g%-35s&p] [&w%6s&p] [&w%6d&p] \n\r", count, questmaster->premio_vnum[i] ? get_obj_index(questmaster->premio_vnum[i])->short_descr : "Empty", num_punct(questmaster->premio_valor[i]), questmaster->premio_vnum[i] );
       ++count;
    }
    ch_printf( ch, "&p[&w**&p] [&Y%-10s Oro%-21s&p] [&w%6s&p] \n\r", num_punct(questmaster->premio_oro), "", num_punct(questmaster->coste_oro));
    ch_printf( ch, "&p[&w**&p] [&Y%-3d Practicas%-22s&p] [&w%6s&p] \n\r", questmaster->num_prac, "", num_punct(questmaster->coste_prac)  );
    ch_printf( ch, "&p[&w**&p] [%-37s&p] [&w%6d&p]\n\r", "&YLevel Range", questmaster->rango_nivel  );
    return;
  }
  if (!str_prefix(arg1, "Object"))
  {
   int seek=atoi(arg2), vnum=atoi(arg3), value=atoi(arg4);
   sh_int i, count=0;

   if (vnum < 0|| seek < 0|| value < 0)
   {
    send_to_char( "Syntaxis: queststat object # vnum value\n\r", ch );
    return;
   }

   for(i = 0; i != MAX_QDATA; i++)
   {
     if (count == seek)
      break;
     ++count;
   }

   if (seek != count)
   {
    send_to_char( "# Not found.\n\r", ch );
    send_to_char( "Syntax: queststat object # vnum value\n\r", ch );
    return;
   }
   else
   {
    questmaster->premio_vnum[i] = vnum;
    questmaster->premio_valor[i] = value;
   }
   fwrite_questmaster();
   send_to_char( "Done.\n\r", ch );
   return;
  }
  if (!str_prefix(arg1, "Gold"))
  {
   int amount=atoi(arg2), value=atoi(arg3);

   if (!amount || !value)
   {
    send_to_char( "Syntax: queststat gold amount value\n\r", ch );
    return;
   }
   questmaster->premio_oro = amount;
   questmaster->coste_oro = value;
   fwrite_questmaster();
   send_to_char( "Done.\n\r", ch );
   return;
  }
  if (!str_prefix(arg1, "Practices"))
  {
   int amount=atoi(arg2), value=atoi(arg3);

   if (!amount || !value)
   {
    send_to_char( "Syntax: queststat practices amount value\n\r", ch );
    return;
   }
   questmaster->num_prac = amount;
   questmaster->coste_prac = value;
   fwrite_questmaster();
   send_to_char( "Done.\n\r", ch );
   return;
  }
  if (!str_prefix(arg1, "Range"))
  {
   int value=atoi(arg2);

   if (!value)
   {
    send_to_char( "Syntax: queststat range value\n\r", ch );
    return;
   }
   questmaster->rango_nivel = value;
   fwrite_questmaster();
   send_to_char( "Done.\n\r", ch );
   return;
  }
  do_queststat(ch, "");
  return;
}


void quest_reclamar( CHAR_DATA *ch, OBJ_DATA *obj )
{
        CHAR_DATA *victim;
        if( !ch )
        {
                bug( "Quest_reclamar: Ch NULO!" );
                return;
        }

        if( !obj )
        {
                bug( "Quest_reclamar: Objeto NULO!" );
                return;
        }


                        if( !str_cmp( obj->propietario, ch->name ) )
                        {
                                if( obj->carried_by )
                                {
                                   victim = obj->carried_by;

                                        obj_from_char(obj);
                                        save_char_obj( victim );
                                        ch_printf( victim, "&w%s &gte abandona y regresa a manos de &w%s, &gsu amo.\n\r", obj->short_descr, ch->name );
                                }

                                else if( obj->in_obj )
                                       obj_from_obj( obj );


                                else if( obj->in_room )
                                        obj_from_room(obj);

                                else
                                {
                                        bug( "Quest_reclamar: %s no tiene localizacion fisica", obj->short_descr );
                                        return;
                                }

                                obj_to_char(obj, ch );
                                save_char_obj( ch );

                                ch_printf( ch, "Reclamas %s que regresa a ti\n\r", obj->short_descr );
                                return;
                        }
}

