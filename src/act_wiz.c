/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include <unistd.h>

/*
 * Local functions.
 */
ROOM_INDEX_DATA *   find_location   args( ( CHAR_DATA *ch, char *arg ) );
void            call_all    args( ( CHAR_DATA *ch ) );
char            buf     [MAX_STRING_LENGTH];
char            arg     [MAX_INPUT_LENGTH];
char                    arg2            [MAX_INPUT_LENGTH];
bool   write_to_descriptor     args( ( DESCRIPTOR_DATA *d, char *txt, int length ) );
bool   write_to_descriptor_2   args( ( int desc, char *txt, int length ) );
void init_descriptor args ((DESCRIPTOR_DATA *dnew, int desc));
void save_changes args ( ( ) );

OBJ_DATA * oclone(OBJ_DATA * pObj);

void do_owhere(CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;
    int number = 0, max_found;

    found = FALSE;
    number = 0;
    max_found = 200;

    if (argument[0] == '\0')
    {
        send_to_char("Find what?\n\r",ch);
        return;
    }

    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name )
            ||   ch->level < obj->level
            ||   (obj->carried_by != NULL && !can_see(ch,obj->carried_by)))
            continue;

        found = TRUE;
        number++;

        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
            &&   in_obj->carried_by->in_room != NULL)
            sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
                number, obj->short_descr,PERS(in_obj->carried_by, ch),
                in_obj->carried_by->in_room->vnum );
        else if (in_obj->in_room != NULL)
            sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
                    number, obj->short_descr,in_obj->in_room->name,
                    in_obj->in_room->vnum);
        else
            sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);

        send_to_char( buf, ch );

        if (number >= max_found)
            break;
    }

    if ( !found )
        send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
}

void do_resetarea(CHAR_DATA *ch, char *argument)
{
    send_to_char("You patiently twiddle your thumbs, waiting for the reset.\n\r",ch);
    area_update();
}

void do_gemote( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        return;
    }

    if(IS_SET(ch->act, PLR_SILENCE))
    {
        send_to_char("Not while silenced.\n\r", ch);
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING &&
            !IS_SET(d->character->deaf, CHANNEL_INFO) )
        {
            send_to_char( "#L[#0Gemote#L] ", d->character );
            sprintf(buf, "#0%s #w%s#n", ch->name, argument);
            send_to_char( buf, d->character);
            send_to_char( "#n\n\r",   d->character );
        }
    }

    return;
}

void do_tick(CHAR_DATA *ch, char *argument)
{
    send_to_char("TICK!  Now wasn't that fun for you.\n\r",ch);
    //weather_update();
    char_update();
    obj_update();
    area_update();
    room_update();
    update_pos(ch);
    update_best(ch);
}

void do_distort(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if (argument[0]=='\0')
    {
        send_to_char("Change your form to resemble what?\n\r",ch);
        return;
    }

    if (!str_cmp(argument,"Seth") ||
        !str_cmp(argument,"Joshua") ||
        !str_cmp(argument,"Marisella") ||
        !str_cmp(argument,"Reaver") ||
        !str_cmp(argument,"Nibbles") ||
        !str_cmp(argument,"Raziel") ||
        !str_cmp(argument,"Mavrick") ||
        !str_cmp(argument,"Aislynn") ||
        !str_cmp(argument,"Alesyira") ||
        !str_cmp(argument,"Chrono") ||
        !str_cmp(argument,"Angela") ||
        !str_cmp(argument,"Silk") ||
        !str_cmp(argument,"Kage") ||
        !str_cmp(argument,"Talyn") ||
        !str_cmp(argument,"Kabal") ||
        !str_cmp(argument,"Dertaherup") ||
        !str_cmp(argument,"Dert") ||
        !str_cmp(argument,"Dertah") ||
        !str_cmp(argument,"Beorn") ||
        !str_cmp(argument,"Tracey"))
    {
        send_to_char("You cannot distort into an immortal.\n\r",ch);
        return;
    }

    if (!str_cmp(argument,"self") )
    {
        if (!IS_AFFECTED(ch,AFF_POLYMORPH) && !IS_VAMPAFF(ch,VAM_DISGUISED))
        {
            send_to_char( "You already look like yourself!\n\r", ch );
            return;
        }
        stc("You transform back to your old form",ch);
        sprintf(buf, "%s's form blurs and shifts to resemble %s.",  ch->name,argument);
        act(buf,ch,NULL,NULL,TO_ROOM);
        REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
        REMOVE_BIT(ch->pcdata->stats[UNI_AFF], VAM_DISGUISED);
        free_string( ch->morph );
        ch->morph = str_dup( "" );
        set_pc_name(ch, ch->pcdata->switchname);
        return;
    }

    sprintf(buf, "%s's form blurs and shifts to resemble %s.",  ch->name,argument);
    act(buf,ch,NULL,NULL,TO_ROOM);
    set_pc_name(ch, argument);
    free_string( ch->morph );
    ch->morph = str_dup( argument );
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->pcdata->stats[UNI_AFF], VAM_DISGUISED);
    send_to_char("Your form blurs and shifts to resemble ",ch);
    send_to_char(argument,ch);
    send_to_char(".\n\r",ch);
    return;
}

/* show linkdeads - code by Marlow */

void do_linkdead(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *gch;
    char buf[MAX_STRING_LENGTH];
    bool found = FALSE;

    for (gch = char_list; gch != NULL; gch = gch->next)
    {
        if (IS_NPC(gch) || gch->desc) continue;
        found = TRUE;
        sprintf(buf, "Name: %12s. (Room: %5d)\n\r", gch->name, gch->in_room == NULL ? : gch->in_room->vnum);
        send_to_char(buf,ch);
    }
    if(!found) send_to_char("No Linkdead Players found\n\r",ch);
    return;
}

void do_pset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char arg4 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int value;

    sprintf(buf,"%s: Pset %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    if ((ch->pcdata->security < 5) && (ch->level < 13))
    {
        stc (" You do not have the nessessary level 5 security to pset.\n\r",ch);
        return;
    }
    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    strcpy(arg4, argument);
    if ( arg1[0] == '\0' || arg2[0] == '\0'  )
    {
        send_to_char( "#7Syntax: pset <victim> <area> <field> <value>\n\r",ch );
        send_to_char( "\n\r",                           ch );
        send_to_char( "#7Area being one of:\n\r",               ch );
        send_to_char( "  #3quest quest+ quest- weapon immune beast\n\r",    ch );
        send_to_char( "  #3pkill pdeath store pkpoint\n\r",         ch );
        send_to_char( "  #3blue red yellow green purple \n\r",          ch );
        send_to_char( "  #3mongoose crane crab viper bull mantis\n\r",      ch );
        send_to_char( "  #3dragon tiger monkey swallow \n\r",           ch );
        send_to_char( "\n\r",                           ch );
        send_to_char( "#7Field being one of:\n\r",              ch );
        send_to_char( "#3Weapon:  slice stab slash whip claw blast\n\r",    ch );
        send_to_char( "#3Weapon:  pound crush grep bite pierce suck \n\r",  ch );
        send_to_char( "#3WpnLvl:  slice stab slash whip claw blast\n\r",        ch );
        send_to_char( "#3WpnLvl:  pound crush grep bite pierce suck \n\r",      ch );
        send_to_char( "#3Immune:  slash stab smash animal misc charm\n\r",  ch );
        send_to_char( "#3Immune:  heat cold acid summon voodoo \n\r",       ch );
        send_to_char( "#3Immune:  hurl backstab shielded kick disarm\n\r",  ch );
        send_to_char( "#3Immune:  steal sleep drain sunlight\n\r",      ch );
        send_to_char( "#3	  all#n\n\r",                 ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */

    if ( !str_cmp( arg2, "bitch"))
    {
        if(victim->flag4==1)
        {
            victim->flag4=0;
            send_to_char( "Whore removed.\n\r", ch);
        }
        else
        {
            victim->flag4=1;
            send_to_char( "Whore set!\n\r", ch);
            send_to_char( "You bitch!\n\r", victim);
        }
        return;
    }

    if ( !str_cmp( arg2, "beast" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 100 )
        {
            send_to_char( "Beast range is 0 to 100.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->beast = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "pkill" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( value < 0 || value > 100 )
        {
            send_to_char( "Pkill range is 0 to 100.\n\r", ch );
            return;
        }
        victim->pkill = value;
        stcf(ch," %s has his pkill set at: %d.\n\r", victim->name, victim->pkill );
        return;
    }
    if ( !str_cmp( arg2, "pdeath" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( value < 0 || value > 100 )
        {
            send_to_char( "Pdeath range is 0 to 100.\n\r", ch );
            return;
        }
        victim->pdeath = value;
        stcf(ch," %s has his pdeath set at: %d.\n\r", victim->name, victim->pdeath );
        return;
    }

    if ( !str_cmp( arg2, "pkpoint" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( value < 0 || value > 1000 )
        {
            send_to_char( "Pdeath range is 0 to 1000.\n\r", ch );
            return;
        }
        victim->pcdata->pkpoints = value;
        stcf(ch," %s has his pkpoints set at: %d.\n\r", victim->name, victim->pcdata->pkpoints );
        return;
    }

    if ( !str_cmp( arg2, "store" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }
        if ( value < 0 || value > 10000 )
        {
            send_to_char( "Store range is 0 to 10000.\n\r", ch );
            return;
        }
        /*	if ( !str_cmp(ch->name, "Joshua" ) || !str_cmp(ch->name,"Marisella") ||
             !str_cmp(ch->name,"Raziel" )  || !str_cmp(ch->name,"Aislynn" )  || !str_cmp(ch->name,"Reaver" ) ||
            !str_cmp(ch->name,"Nibbles" )  */
        {
            victim->pcdata->store = value;
            stcf(ch," %s has his stores set at: %d.\n\r", victim->name, victim->pcdata->store );
        }
        /*
                send_to_char("Sorry, you're not registered to modify a person's store amount.\n\r",ch);
            return;   */
    }

    if ( !str_cmp( arg2, "quest" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 1 || value > 99999 )
        {
            send_to_char( "Quest range is 1 to 99999.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->pcdata->quest = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "quest+" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 1 || value + victim->pcdata->quest > 99999 )
        {
            send_to_char( "Quest range is 1 to 99999.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->pcdata->quest += value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "quest-" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( victim->pcdata->quest - value < 1 || value > 99999 )
        {
            send_to_char( "Quest range is 1 to 99999.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->pcdata->quest -= value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "viper" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Viper range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_VIPER] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "crane" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Crane range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_CRANE] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "crab" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Crab range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_CRAB] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "mongoose" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Mongoose range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_MONGOOSE] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "bull" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Bull range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_BULL] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "mantis" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Mantis range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_MANTIS] = value;
            victim->stance[STANCE_CRANE]  = 200;
            victim->stance[STANCE_VIPER]  = 200;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "dragon" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Dragon range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_DRAGON] = value;
            victim->stance[STANCE_CRAB]   = 200;
            victim->stance[STANCE_BULL]   = 200;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "tiger" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Tiger range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_TIGER] = value;
            victim->stance[STANCE_BULL]  = 200;
            victim->stance[STANCE_VIPER] = 200;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "monkey" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Monkey range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {

            victim->stance[STANCE_MONKEY] = value;
            victim->stance[STANCE_MONGOOSE]  = 200;
            victim->stance[STANCE_CRANE]  = 200;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "swallow" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance Swallow range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_SWALLOW] = value;
            victim->stance[STANCE_CRAB]  = 200;
            victim->stance[STANCE_MONGOOSE]  = 200;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "ss1" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance ss1 range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_SS1] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "ss2" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance ss2 range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_SS2] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "ss3" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance ss3 range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_SS3] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "ss4" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 200 )
        {
            send_to_char( "Stance ss4 range is 0 to 200.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_SS4] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }
    if ( !str_cmp( arg2, "ss5" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 10000 )
        {
            send_to_char( "Stance ss5 range is 0 to 10000.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->stance[STANCE_SS5] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "purple" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->spl[PURPLE_MAGIC] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "stances") )
    {
        int i;

        if (IS_NPC(victim)) return;
        if (value > 200 || value < 0) return;
        for (i = 1; i < 18; i++)
            victim->stance[i] = value;
        return;
    }
    if ( !str_cmp( arg2, "spells" ) )
    {
        if (IS_NPC(victim))
            return;

        if (!IS_CREATOR(ch))
        {
            do_pset(ch,"");
            return;
        }

        if (value > 32000 || value < -32000)
            return;

        victim->spl[RED_MAGIC]    = value;
        victim->spl[PURPLE_MAGIC] = value;
        victim->spl[GREEN_MAGIC]  = value;
        victim->spl[BLUE_MAGIC]   = value;
        victim->spl[YELLOW_MAGIC] = value;

        return;

    }

    if ( !str_cmp( arg2, "red" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->spl[RED_MAGIC] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "blue" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->spl[BLUE_MAGIC] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "green" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->spl[GREEN_MAGIC] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "yellow" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->spl[YELLOW_MAGIC] = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "immune" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if (arg3 == '\0')
        {
            send_to_char( "pset <victim> immune <immunity>.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            if (!str_cmp(arg3, "voodoo"))
            {
                if (IS_SET(victim->immune, IMM_VOODOO))
                {
                    REMOVE_BIT(victim->immune, IMM_VOODOO);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_VOODOO);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "slash"))
            {
                if (IS_SET(victim->immune, IMM_SLASH))
                {
                    REMOVE_BIT(victim->immune, IMM_SLASH);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_SLASH);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "stab"))
            {
                if (IS_SET(victim->immune, IMM_STAB))
                {
                    REMOVE_BIT(victim->immune, IMM_STAB);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_STAB);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "smash"))
            {
                if (IS_SET(victim->immune, IMM_SMASH))
                {
                    REMOVE_BIT(victim->immune, IMM_SMASH);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_SMASH);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "anmial"))
            {
                if (IS_SET(victim->immune, IMM_ANIMAL))
                {
                    REMOVE_BIT(victim->immune, IMM_ANIMAL);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_ANIMAL);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "misc"))
            {
                if (IS_SET(victim->immune, IMM_MISC))
                {
                    REMOVE_BIT(victim->immune, IMM_MISC);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_MISC);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "charm"))
            {
                if (IS_SET(victim->immune, IMM_CHARM))
                {
                    REMOVE_BIT(victim->immune, IMM_CHARM);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_CHARM);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }
            if (!str_cmp(arg3, "heat"))
            {
                if (IS_SET(victim->immune, IMM_HEAT))
                {
                    REMOVE_BIT(victim->immune, IMM_HEAT);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_HEAT);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }
            if (!str_cmp(arg3, "cold"))
            {
                if (IS_SET(victim->immune, IMM_COLD))
                {
                    REMOVE_BIT(victim->immune, IMM_COLD);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_COLD);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "lightning"))
            {
                if (IS_SET(victim->immune, IMM_LIGHTNING))
                {
                    REMOVE_BIT(victim->immune, IMM_LIGHTNING);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_LIGHTNING);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "acid"))
            {
                if (IS_SET(victim->immune, IMM_ACID))
                {
                    REMOVE_BIT(victim->immune, IMM_ACID);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_ACID);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "shield"))
            {
                if (IS_SET(victim->immune, IMM_SHIELDED))
                {
                    REMOVE_BIT(victim->immune, IMM_SHIELDED);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_SHIELDED);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "hurl"))
            {
                if (IS_SET(victim->immune, IMM_HURL))
                {
                    REMOVE_BIT(victim->immune, IMM_HURL);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_HURL);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "backstab"))
            {
                if (IS_SET(victim->immune, IMM_BACKSTAB))
                {
                    REMOVE_BIT(victim->immune, IMM_BACKSTAB);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_BACKSTAB);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "kick"))
            {
                if (IS_SET(victim->immune, IMM_KICK))
                {
                    REMOVE_BIT(victim->immune, IMM_KICK);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_KICK);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "disarm"))
            {
                if (IS_SET(victim->immune, IMM_DISARM))
                {
                    REMOVE_BIT(victim->immune, IMM_DISARM);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_DISARM);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "steal"))
            {
                if (IS_SET(victim->immune, IMM_STEAL))
                {
                    REMOVE_BIT(victim->immune, IMM_STEAL);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_STEAL);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "sleep"))
            {
                if (IS_SET(victim->immune, IMM_SLEEP))
                {
                    REMOVE_BIT(victim->immune, IMM_SLEEP);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_SLEEP);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "sunlight"))
            {
                if (IS_SET(victim->immune, IMM_SUNLIGHT))
                {
                    REMOVE_BIT(victim->immune, IMM_SUNLIGHT);
                    send_to_char("Ok Immunity Removed.\n\r",ch);
                    return;
                }
                else
                {
                    SET_BIT(victim->immune, IMM_SUNLIGHT);
                    send_to_char("Ok Immunity Added.\n\r",ch);
                    return;
                }
            }

            if (!str_cmp(arg3, "all"))
            {
                SET_BIT(victim->immune, IMM_DRAIN);
                SET_BIT(victim->immune, IMM_VOODOO);
                SET_BIT(victim->immune, IMM_SLASH);
                SET_BIT(victim->immune, IMM_STAB);
                SET_BIT(victim->immune, IMM_SMASH);
                SET_BIT(victim->immune, IMM_ANIMAL);
                SET_BIT(victim->immune, IMM_MISC);
                SET_BIT(victim->immune, IMM_CHARM);
                SET_BIT(victim->immune, IMM_HEAT);
                SET_BIT(victim->immune, IMM_COLD);
                SET_BIT(victim->immune, IMM_LIGHTNING);
                SET_BIT(victim->immune, IMM_ACID);
                SET_BIT(victim->immune, IMM_HURL);
                SET_BIT(victim->immune, IMM_BACKSTAB);
                SET_BIT(victim->immune, IMM_KICK);
                SET_BIT(victim->immune, IMM_DISARM);
                SET_BIT(victim->immune, IMM_STEAL);
                SET_BIT(victim->immune, IMM_SLEEP);
                send_to_char("Ok All Immunities Added.\n\r",ch);
                return;
            }

            send_to_char("No such immunity exists.\n\r",ch);
            return;
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "weapon" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg4 );
        /*
         * Snarf the value (which need not be numeric).
         */
        value = is_number( arg4 ) ? atoi( arg4 ) : -1;

        if (!IS_CREATOR(ch))
        {
            if ( value < 0 || value > 32000 )
            {
                send_to_char( "Weapon skill range is 0 to 32000.\n\r", ch );
                return;
            }
        }

        if (IS_JUDGE(ch))
        {
            if (!str_cmp(arg3, "unarmed"))
            {
                victim->wpn[0] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "slice"))
            {
                victim->wpn[1] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "stab"))
            {
                victim->wpn[2] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "slash"))
            {
                victim->wpn[3] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "whip"))
            {
                victim->wpn[4] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "claw"))
            {
                victim->wpn[5] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "blast"))
            {
                victim->wpn[6] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "pound"))
            {
                victim->wpn[7] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "crush"))
            {
                victim->wpn[8] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "grep"))
            {
                victim->wpn[9] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "bite"))
            {
                victim->wpn[10] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "pierce"))
            {
                victim->wpn[11] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "suck"))
            {
                victim->wpn[12] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "all"))
            {
                victim->wpn[0] = value;
                victim->wpn[1] = value;
                victim->wpn[2] = value;
                victim->wpn[3] = value;
                victim->wpn[4] = value;
                victim->wpn[5] = value;
                victim->wpn[6] = value;
                victim->wpn[7] = value;
                victim->wpn[8] = value;
                victim->wpn[8] = value;
                victim->wpn[9] = value;
                victim->wpn[10] = value;
                victim->wpn[11] = value;
                victim->wpn[12] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }
            send_to_char("No such weapon skill exists.\n\r",ch);
            return;
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "wpnlvl" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        argument = one_argument( argument, arg4 );
        /*
         * Snarf the value (which need not be numeric).
         */
        value = is_number( arg4 ) ? atoi( arg4 ) : -1;

        if (!IS_CREATOR(ch))
        {
            if ( value < 0 || value > 5 )
            {
                send_to_char( "Weapon level range is 0 to 5.\n\r", ch );
                return;
            }
        }

        if (IS_JUDGE(ch))
        {
            if (!str_cmp(arg3, "unarmed"))
            {
                victim->wpn_lvl[0] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "slice"))
            {
                victim->wpn_lvl[1] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "stab"))
            {
                victim->wpn_lvl[2] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "slash"))
            {
                victim->wpn_lvl[3] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "whip"))
            {
                victim->wpn_lvl[4] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "claw"))
            {
                victim->wpn_lvl[5] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "blast"))
            {
                victim->wpn_lvl[6] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "pound"))
            {
                victim->wpn_lvl[7] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "crush"))
            {
                victim->wpn_lvl[8] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "grep"))
            {
                victim->wpn_lvl[9] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "bite"))
            {
                victim->wpn_lvl[10] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "pierce"))
            {
                victim->wpn_lvl[11] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "suck"))
            {
                victim->wpn_lvl[12] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }

            if (!str_cmp(arg3, "all"))
            {
                victim->wpn_lvl[0] = value;
                victim->wpn_lvl[1] = value;
                victim->wpn_lvl[2] = value;
                victim->wpn_lvl[3] = value;
                victim->wpn_lvl[4] = value;
                victim->wpn_lvl[5] = value;
                victim->wpn_lvl[6] = value;
                victim->wpn_lvl[7] = value;
                victim->wpn_lvl[8] = value;
                victim->wpn_lvl[8] = value;
                victim->wpn_lvl[9] = value;
                victim->wpn_lvl[10] = value;
                victim->wpn_lvl[11] = value;
                victim->wpn_lvl[12] = value;
                send_to_char("Ok.\n\r",ch);
                return;
            }
            send_to_char("No such weapon skill exists.\n\r",ch);
            return;
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    /*
     * Generate usage message.
     */
    do_pset( ch, "" );
    return;
}

void do_paradox( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Paradox whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    if (!IS_NPC(victim))
    {
        paradox(victim);
        send_to_char("Done!\n\r", ch );
    }
    else send_to_char("Not on NPCs.\n\r",ch);
    return;
}

void paradox( CHAR_DATA *ch )
{
    char        buf[MAX_STRING_LENGTH];
    int i;

    send_to_char("The sins of your past strike back!\n\r",  ch);
    send_to_char("The paradox has come for your soul!\n\r",  ch);
    if (ch->sex == SEX_MALE) sprintf(buf,"#C%s #pscreams in agony as the #RP#Ga#RR#Ga#RD#Go#RX#p wrecks his puny mortal body#n",ch->name);
    else sprintf(buf,"#C%s #pscreams in agony as the #RP#Ga#RR#Ga#RD#Go#RX#p wrecks her puny mortal body#n",ch->name);
    do_info(ch, buf);
    ch->hit =  -10;
    ch->mana    =  -10;
    ch->move    =  -10;
    if ( ch->pcdata->quest < 1000 )
    {
        if ( ch->generation > 4 )
        {
            if ( ch->pcdata->upgrade_level < 2 )
            {
                for (i=0; i < 13 ; i++)
                    if (ch->wpn[i] >= 1000)
                {
                    ch->wpn[i] -= 1000;
                }
                else
                {
                    ch->wpn[i] = 0;
                }
            }
            else
            {
                ch->pcdata->upgrade_level--;
            }
        }
        else
        {
            ch->generation++;
        }
    }
    else
    {
        ch->pcdata->quest -= 1000;
    }
    update_pos( ch );
    do_escape(ch, "");
    SET_BIT(ch->extra, TIED_UP);
    SET_BIT(ch->extra, GAGGED);
    SET_BIT(ch->extra, BLINDFOLDED);

    return;
}

/*void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int cmd;
    int col;

    col = 0;
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
    if ( cmd_table[cmd].level > LEVEL_HERO
    &&   cmd_table[cmd].level <= get_trust( ch ) )
    {
        sprintf( buf, "%-12s", cmd_table[cmd].name );
        send_to_char( buf, ch );
        if ( ++col % 6 == 0 )
        send_to_char( "\n\r", ch );
    }
    }

    if ( col % 6 != 0 )
    send_to_char( "\n\r", ch );
    return;
}*/
void do_wizhelp( CHAR_DATA *ch, char *argument )
{
    char buf[ MAX_STRING_LENGTH ];
    int cmd;
    int col;
    int level;

    for ( level = get_trust( ch ) ; level >= 4 ; level-- )
    {
        sprintf( buf, "#R<>#C*#R<>#C*#R<>#C*#R<> #7Level #L%2d #R<>#C*#R<>#C*#R<>#C*#R<>#n\n\r", level );
        send_to_char( buf, ch );

        col = 0;
        for ( cmd = 0; cmd_table[ cmd ].name[ 0 ] != '\0'; cmd++
            )
        {
            if ( cmd_table[ cmd ].level == level )
            {
                sprintf( buf, "#7%-12s", cmd_table[ cmd ].name );
                send_to_char( buf, ch );
                if ( ++col % 6 == 0 )
                    send_to_char( "\n\r", ch );
            }
        }

        if ( col % 6 != 0 )
            send_to_char( "\n\r", ch );
    }
    return ;
}

void do_bamfin( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
        free_string( ch->pcdata->bamfin );
        ch->pcdata->bamfin = str_dup( argument );
        send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_bamfout( CHAR_DATA *ch, char *argument )
{
    if ( !IS_NPC(ch) )
    {
        smash_tilde( argument );
        free_string( ch->pcdata->bamfout );
        ch->pcdata->bamfout = str_dup( argument );
        send_to_char( "Ok.\n\r", ch );
    }
    return;
}

void do_qspell( CHAR_DATA *ch, char *argument )
{
    send_to_char("not in yet\n\r", ch );
    return;
}

void do_nosummon( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if ( arg[0] == '\0')
    {
        send_to_char( "Do you wish to switch summon ON or OFF?\n\r", ch );
        return;
    }

    if (IS_IMMUNE(ch, IMM_SUMMON) && !str_cmp(arg,"off"))
    {
        REMOVE_BIT(ch->immune, IMM_SUMMON);
        send_to_char("You now cant be the target of summon and portal.\n\r", ch);
    }
    else if (!IS_IMMUNE(ch, IMM_SUMMON) && !str_cmp(arg,"off"))
    {
        send_to_char("But it is already off!\n\r", ch);
        return;
    }
    else if (!IS_IMMUNE(ch, IMM_SUMMON) && !str_cmp(arg,"on"))
    {
        SET_BIT(ch->immune, IMM_SUMMON);
        send_to_char("You now can be the target of summon and portal.\n\r", ch);
    }
    else if (IS_IMMUNE(ch, IMM_SUMMON) && !str_cmp(arg,"on"))
    {
        send_to_char("But it is already on!\n\r", ch);
        return;
    }
    else
        send_to_char( "Do you wish to switch it ON or OFF?\n\r", ch );
    return;
}

void do_transport( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if ( arg[0] == '\0')
    {
        send_to_char( "Do you wish to switch transport ON or OFF?\n\r", ch );
        return;
    }

    if (IS_IMMUNE(ch, IMM_TRANSPORT) && !str_cmp(arg,"off"))
    {
        REMOVE_BIT(ch->immune, IMM_TRANSPORT);
        send_to_char("You can no longer be the target of transport spells.\n\r", ch);
    }
    else if (!IS_IMMUNE(ch, IMM_TRANSPORT) && !str_cmp(arg,"off"))
    {
        send_to_char("But it is already off!\n\r", ch);
        return;
    }
    else if (!IS_IMMUNE(ch, IMM_TRANSPORT) && !str_cmp(arg,"on"))
    {
        SET_BIT(ch->immune, IMM_TRANSPORT);
        send_to_char("You can now be the target of transport spells.\n\r", ch);
    }
    else if (IS_IMMUNE(ch, IMM_TRANSPORT) && !str_cmp(arg,"on"))
    {
        send_to_char("But it is already on!\n\r", ch);
        return;
    }
    else
        send_to_char( "Do you wish to switch it ON or OFF?\n\r", ch );
    return;
}

void do_watcher( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    one_argument( argument, arg );

    /*    if (IS_NPC(ch) || !IS_SET(ch->act, PLR_WATCHER)
        || (ch->level > 6)
        || (ch->level < 2)
        || (ch->trust > 0) )
        {
        send_to_char( "Huh?\n\r", ch );
        return;
        }

      */
    if ( arg[0] == '\0' )
    {
        do_users(ch,"");
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    /*
        if (ch == victim)
        {
        send_to_char( "Not on yourself.\n\r", ch );
        return;
        }
    */
    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
        send_to_char( "You better not, they are being snooped.\n\r", ch );
        return;
    }

    if( IS_SET(victim->act, PLR_WATCHER))
    {
        REMOVE_BIT(victim->act, PLR_WATCHER);
        act( "You remove $N's watcher status.",ch,NULL,victim,TO_CHAR);
    }
    else
    {
        SET_BIT(victim->act, PLR_WATCHER);
        act( "$n has made you a watcher.", ch, NULL, victim, TO_VICT );
        act( "You make $N a watcher.", ch, NULL, victim, TO_CHAR );
    }
    return;
}

void do_deny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];
    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Deny whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    sprintf(buf,"%s: Deny %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    sprintf( buf, "#rI have denied #R%s#r.  Thank me later.#n", victim->name );
    make_note( "Immortal", ch->name, "imm", "Denied", 5, buf );

    SET_BIT(victim->act, PLR_DENY);
    send_to_char( "You are denied access!\n\r", victim );
    send_to_char( "OK.\n\r", ch );
    victim->fight_timer = 0;
    victim->position = POS_STANDING;
    do_quit( victim, "confirm" );

    return;
}

void do_disconnect( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    sprintf(buf,"%s: Disconnect %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Disconnect whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->desc == NULL )
    {
        act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
        return;
    }

    if ( victim->level >= ch->level )
    {
        send_to_char("Disconnecting you.\n\r",ch);
        do_quit(ch,"");
        return;
    }
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d == victim->desc )
        {
            close_socket( d );
            send_to_char( "Ok.\n\r", ch );
            return;
        }
    }

    bug( "Do_disconnect: desc not found.", 0 );
    send_to_char( "Descriptor not found!\n\r", ch );
    return;
}

void do_info( CHAR_DATA *ch, char *argument )
{
    echo_info(argument);
    return;
}

void echo_info(char * argument)
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING && d->character != NULL && !IS_SET(d->character->deaf, CHANNEL_INFO) )
        {
            send_to_char( "#C<- #RInfo #C->#n ",d->character );
            send_to_char( argument, d->character );
            send_to_char( "\n\r",   d->character );
        }
    }

    return;

}

void do_questchat(CHAR_DATA *ch, char * argument)
{
    if (IS_NPC(ch) || !IS_IMMORTAL(ch))
        return;
    else
        do_questchat2(argument);
    return;
}

void do_questchat2(char * argument)
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING && d->character != NULL && !IS_SET(d->character->deaf, CHANNEL_INFO) )
        {
            send_to_char( "#G<- #yQuest #G->#n ",d->character );
            send_to_char( argument, d->character );
            send_to_char( "\n\r",   d->character );
        }
    }

    return;

}

void do_watching( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING &&
            IS_SET(d->character->act, PLR_WATCHER) &&
            !IS_SET(d->character->deaf, CHANNEL_INFO) )
        {
            send_to_char( "Monitor -> ",d->character );
            send_to_char( argument, d->character );
            send_to_char( "\n\r",   d->character );
        }
    }

    return;
}

void logchan( char *argument )
{
    CHAR_DATA *ch;
    DESCRIPTOR_DATA *d;

    if ( argument[0] == '\0' )
    {
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( ( ch = d->character ) == NULL ) continue;
        if ( (d->connected == CON_PLAYING) && IS_JUDGE(ch) &&
            !IS_SET(ch->deaf,CHANNEL_LOG) && ch->level >= d->character->level)
        {
            send_to_char( "[", ch );
            send_to_char( argument, ch );
            send_to_char( "]\n\r",  ch );
        }
    }

    return;
}

void do_echo( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s: Echo %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    if ( argument[0] == '\0' )
    {
        send_to_char( "Echo what?\n\r", ch );
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING )
        {
            send_to_char( argument, d->character );
            send_to_char( "\n\r",   d->character );
        }
    }

    return;
}

void do_recho( CHAR_DATA *ch, char *argument )
{
    DESCRIPTOR_DATA *d;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s: Recho %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    if ( argument[0] == '\0' )
    {
        send_to_char( "Recho what?\n\r", ch );
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING
            &&   d->character->in_room == ch->in_room )
        {
            send_to_char( argument, d->character );
            send_to_char( "\n\r",   d->character );
        }
    }

    return;
}

void do_cecho( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA * pVictim;
    char pArg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument(argument, pArg1);

    sprintf(buf,"%s: Cecho %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    if ( argument[0] == '\0' )
    {
        send_to_char( "Cecho what?\n\r", ch );
        return;
    }

    pVictim = get_char_world(ch, pArg1);

    if(pVictim == NULL)
    {
        return;
    }

    stc(argument, pVictim);
    stc("\n\r", pVictim);
    stc("Done.\n\r", ch);

    return;
}

void do_crecho( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA * pVictim;
    DESCRIPTOR_DATA *d;
    char pArg1[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument(argument, pArg1);

    sprintf(buf,"%s: Recho %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    if ( argument[0] == '\0' )
    {
        send_to_char( "Crecho what?\n\r", ch );
        return;
    }

    pVictim = get_char_world(ch, pArg1);

    if(pVictim == NULL)
    {
        return;
    }

    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if ( d->connected == CON_PLAYING
            &&   d->character->in_room == ch->in_room
            &&   d->character != pVictim)
        {
            send_to_char( argument, d->character );
            send_to_char( "\n\r",   d->character );
        }
    }

    return;
}

ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, char *arg )
{
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if ( is_number(arg) )
        return get_room_index( atoi( arg ) );

    if ( ( victim = get_char_world( ch, arg ) ) != NULL)
        if( can_see(ch, victim) )
            return victim->in_room;

    if ( ( obj = get_obj_world( ch, arg ) ) != NULL && obj->in_room != NULL)
        return obj->in_room;

    if ( obj != NULL && obj->carried_by != NULL && obj->carried_by->in_room != NULL)
        return obj->carried_by->in_room;

    if ( obj != NULL && obj->in_obj != NULL && obj->in_obj->in_room != NULL)
        return obj->in_obj->in_room;

    if ( obj != NULL && obj->in_obj != NULL && obj->in_obj->carried_by && obj->in_obj->carried_by->in_room != NULL)
        return obj->in_obj->carried_by->in_room;

    return NULL;
}

void do_transfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;
    CHAR_DATA *mount;

    sprintf(buf,"%s: Transfer %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Transfer whom (and where)?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg1, "all" ) )
    {
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->connected == CON_PLAYING
                &&   d->character != ch
                &&   d->character->in_room != NULL
                &&   can_see( ch, d->character ) )
            {
                char buf[MAX_STRING_LENGTH];
                sprintf( buf, "%s %s", d->character->name, arg2 );
                do_transfer( ch, buf );
            }
        }
        return;
    }

    /*
     * Thanks to Grodyn for the optional location parameter.
     */
    if ( arg2[0] == '\0' )
    {
        location = ch->in_room;
    }
    else
    {
        if ( ( location = find_location( ch, arg2 ) ) == NULL )
        {
            send_to_char( "No such location.\n\r", ch );
            return;
        }

        if ( room_is_private( location ) )
        {
            send_to_char( "That room is private right now.\n\r", ch );
            return;
        }
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->in_room == NULL )
    {
        send_to_char( "They are in limbo.\n\r", ch );
        return;
    }

    if ( victim->fighting != NULL )
        stop_fighting( victim, TRUE );
    act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, location );
    act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
    if ( ch != victim )
        act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
    do_look( victim, "auto" );
    send_to_char( "Ok.\n\r", ch );
    if ( (mount = victim->mount) == NULL) return;
    char_from_room( mount );
    char_to_room( mount, get_room_index(victim->in_room->vnum) );
    if ( ch != mount )
        act( "$n has transferred you.", ch, NULL, mount, TO_VICT );
    do_look( mount, "auto" );
}

void do_at( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *original;
    CHAR_DATA *wch;

    sprintf(buf,"%s: At %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "At where what?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    //    if ( room_is_private( location ) )
    //    {
    //	send_to_char( "That room is private right now.\n\r", ch );
    //	return;
    //    }

    original = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, location );
    interpret( ch, argument );

    /*
     * See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case.
     */
    for ( wch = char_list; wch != NULL; wch = wch->next )
    {
        if ( wch == ch )
        {
            char_from_room( ch );
            char_to_room( ch, original );
            break;
        }
    }

    return;
}

void do_goto( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    ROOM_INDEX_DATA *in_room;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Goto where?\n\r", ch );
        return;
    }

    if ( ( location = find_location( ch, arg ) ) == NULL )
    {
        stc("No such location.\n\r",ch);
        return;
    }

    in_room = ch->in_room;
    if ( ch->fighting )
        stop_fighting( ch, TRUE );
    if(ch->level<=12)
        if ( room_is_private( location ) )
    {
        send_to_char( "That room is private right now so Joshua says go away\n\r", ch );
        return;
    }

    //if( IS_SET(victim->in_room->room_flags, ROOM_PRIVATE))
    //{
    //stc( "You can't goto this room cause Joshua says so try back later.\n\r",ch);
    // return;
    // }

    if ( !IS_SET(ch->act, PLR_WIZINVIS) )
        act( "$n $T", ch, NULL,
            (ch->pcdata && ch->pcdata->bamfout[0] != '\0')
            ? ch->pcdata->bamfout : "leaves in a swirling mist.",  TO_ROOM );
    char_from_room( ch );
    char_to_room( ch, location );

    if ( !IS_SET(ch->act, PLR_WIZINVIS) )
        act("$n $T", ch, NULL,
            (ch->pcdata && ch->pcdata->bamfin[0] != '\0')
            ? ch->pcdata->bamfin : "appears in a swirling mist.", TO_ROOM );

    do_look( ch, "auto" );

    if ( ch->in_room == in_room )
        return;

    return;
}

void do_rstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *location;
    OBJ_DATA *obj;
    CHAR_DATA *rch;
    int door;

    one_argument( argument, arg );
    location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
    if ( location == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    if ( ch->in_room != location && room_is_private( location ) )
    {
        send_to_char( "That room is private right now.\n\r", ch );
        return;
    }

    sprintf( buf, "Name: '%s.'\n\rArea: '%s'.\n\r",
        location->name,
        location->area->name );
    send_to_char( buf, ch );

    sprintf( buf,
        "Vnum: %d.  Sector: %d.  Light: %d.\n\r",
        location->vnum,
        location->sector_type,
        location->light );
    send_to_char( buf, ch );

    sprintf( buf,
        "Room flags: %d.\n\rDescription:\n\r%s",
        location->room_flags,
        location->description );
    send_to_char( buf, ch );

    if ( location->extra_descr != NULL )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Extra description keywords: '", ch );
        for ( ed = location->extra_descr; ed; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }
        send_to_char( "'.\n\r", ch );
    }

    send_to_char( "Characters:", ch );
    for ( rch = location->people; rch; rch = rch->next_in_room )
    {
        send_to_char( " ", ch );
        one_argument( rch->name, buf );
        send_to_char( buf, ch );
    }

    send_to_char( ".\n\rObjects:   ", ch );
    for ( obj = location->contents; obj; obj = obj->next_content )
    {
        send_to_char( " ", ch );
        one_argument( obj->name, buf );
        send_to_char( buf, ch );
    }
    send_to_char( ".\n\r", ch );

    for ( door = 0; door <= 5; door++ )
    {
        EXIT_DATA *pexit;

        if ( ( pexit = location->exit[door] ) != NULL )
        {
            sprintf( buf,
                "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

                door,
                pexit->to_room != NULL ? pexit->to_room->vnum : 0,
                pexit->key,
                pexit->exit_info,
                pexit->keyword,
                pexit->description[0] != '\0'
                ? pexit->description : "(none).\n\r" );
            send_to_char( buf, ch );
        }
    }

    return;
}

void do_ostat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char nm1[40];
    char nm2[40];
    AFFECT_DATA *paf;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Ostat what?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
    }

    if (obj->questmaker != NULL && strlen(obj->questmaker) > 1)
        sprintf(nm1,obj->questmaker); else sprintf(nm1,"None");
    if (obj->questowner != NULL && strlen(obj->questowner) > 1)
        sprintf(nm2,obj->questowner); else sprintf(nm2,"None");

    sprintf( buf, "Name: %s.\n\r",
        obj->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d.  Type: %s.\n\r",
        obj->pIndexData->vnum, item_type_name( obj ) );
    send_to_char( buf, ch );

    sprintf( buf, "Short description: %s.\n\rLong description: %s\n\r",
        obj->short_descr, obj->description );
    send_to_char( buf, ch );

    sprintf( buf, "Object creator: %s.	Object owner: %s.  Quest points: %d.\n\r", nm1, nm2, obj->points );
    send_to_char( buf, ch );
    if (obj->quest != 0)
    {
        send_to_char( "Quest selections:", ch );
        if (IS_SET(obj->quest,QUEST_STR)) send_to_char(" Str",ch);
        if (IS_SET(obj->quest,QUEST_DEX)) send_to_char(" Dex",ch);
        if (IS_SET(obj->quest,QUEST_INT)) send_to_char(" Int",ch);
        if (IS_SET(obj->quest,QUEST_WIS)) send_to_char(" Wis",ch);
        if (IS_SET(obj->quest,QUEST_CON)) send_to_char(" Con",ch);
        if (IS_SET(obj->quest,QUEST_HIT)) send_to_char(" Hp",ch);
        if (IS_SET(obj->quest,QUEST_MANA)) send_to_char(" Mana",ch);
        if (IS_SET(obj->quest,QUEST_MOVE)) send_to_char(" Move",ch);
        if (IS_SET(obj->quest,QUEST_HITROLL)) send_to_char(" Hit",ch);
        if (IS_SET(obj->quest,QUEST_DAMROLL)) send_to_char(" Dam",ch);
        if (IS_SET(obj->quest,QUEST_AC)) send_to_char(" Ac",ch);
        send_to_char( ".\n\r", ch );
    }
    sprintf( buf, "Wear bits: %d.  Extra bits: %s.\n\r",
        obj->wear_flags, extra_bit_name( obj->extra_flags ) );
    send_to_char( buf, ch );

    sprintf( buf, "Weight: %d/%d.\n\r",
        obj->weight, get_obj_weight( obj ) );
    send_to_char( buf, ch );

    sprintf( buf, "Cost: %d.  Timer: %d.  Level: %d.\n\r",
        obj->cost, obj->timer, obj->level );
    send_to_char( buf, ch );

    sprintf( buf,
        "In room: %d.  In object: %s.  Carried by: %s.	Wear_loc: %d.\n\r",
        obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
        obj->in_obj == NULL    ? "(none)" : obj->in_obj->short_descr,
        obj->carried_by == NULL    ? "(none)" : obj->carried_by->name,
        obj->wear_loc );
    send_to_char( buf, ch );

    sprintf( buf, "Values: %d %d %d %d.\n\r",
        obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
    send_to_char( buf, ch );

    if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL )
    {
        EXTRA_DESCR_DATA *ed;

        send_to_char( "Extra description keywords: '", ch );

        for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }

        for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next )
        {
            send_to_char( ed->keyword, ch );
            if ( ed->next != NULL )
                send_to_char( " ", ch );
        }

        send_to_char( "'.\n\r", ch );
    }

    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d.\n\r",
            affect_loc_name( paf->location ), paf->modifier );
        send_to_char( buf, ch );
    }

    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf, "Affects %s by %d.\n\r",
            affect_loc_name( paf->location ), paf->modifier );
        send_to_char( buf, ch );
    }

    return;
}

void do_mstat( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    AFFECT_DATA *paf;
    CHAR_DATA *victim;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Mstat whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    sprintf( buf, "Name: %s.\n\r",
        victim->name );
    send_to_char( buf, ch );

    sprintf( buf, "Vnum: %d.  Sex: %s.	Room: %d.\n\r",
        IS_NPC(victim) ? victim->pIndexData->vnum : 0,
        victim->sex == SEX_MALE    ? "male"   :
    victim->sex == SEX_FEMALE  ? "female" : "neutral",
        victim->in_room == NULL    ?        0 : victim->in_room->vnum
        );
    send_to_char( buf, ch );

    sprintf( buf, "Str: %d.  Int: %d.  Wis: %d.  Dex: %d.  Con: %d.\n\r",
        get_curr_str(victim),
        get_curr_int(victim),
        get_curr_wis(victim),
        get_curr_dex(victim),
        get_curr_con(victim) );
    send_to_char( buf, ch );

    sprintf( buf, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d.  Primal: %d.\n\r",
        victim->hit,         victim->max_hit,
        victim->mana,        victim->max_mana,
        victim->move,        victim->max_move,
        victim->practice );
    send_to_char( buf, ch );

    sprintf( buf,
        "Lv: %d.  Align: %d.  AC: %d.  Gold: %d.  Exp: %d.\n\r",
        victim->level,       victim->alignment,
        char_ac(victim),     victim->gold,     victim->exp );
    send_to_char( buf, ch );

    sprintf( buf, "Hitroll: %d.  Damroll: %d.  Position: %d.  Wimpy: %d.\n\r",
        char_hitroll(victim), char_damroll(victim),
        victim->position,    victim->wimpy );
    send_to_char( buf, ch );

    sprintf( buf, "Fighting: %s.\n\r",
        victim->fighting ? victim->fighting->name : "(none)" );
    send_to_char( buf, ch );

    if ( !IS_NPC(victim) )
    {
        sprintf( buf,
            "Thirst: %d.  Full: %d.  Drunk: %d.  Saving throw: %d.\n\r",
            victim->pcdata->condition[COND_THIRST],
            victim->pcdata->condition[COND_FULL],
            victim->pcdata->condition[COND_DRUNK],
            victim->saving_throw );
        send_to_char( buf, ch );

        if (IS_CLASS(victim, CLASS_VAMPIRE) || IS_CLASS(victim, CLASS_WEREWOLF))
        {
            sprintf( buf, "Rage: %d. ", victim->rage);
            send_to_char( buf, ch );
            if (IS_CLASS(victim, CLASS_VAMPIRE))
            {
                sprintf( buf, "Beast: %d. ", victim->beast);
                send_to_char( buf, ch );
                sprintf( buf, "Blood: %d.", victim->pcdata->condition[COND_THIRST]);
                send_to_char( buf, ch );
            }
            send_to_char( "\n\r", ch );
        }

        if (IS_CLASS(victim, CLASS_DEMON))
        {
            sprintf( buf, "Demonic armor: %d pieces. ", victim->pcdata->stats[DEMON_POWER]);
            send_to_char( buf, ch );
            sprintf( buf, "Power: %d (%d).",
                victim->pcdata->stats[DEMON_CURRENT], victim->pcdata->stats[DEMON_TOTAL]);
            send_to_char( buf, ch );
            send_to_char( "\n\r", ch );
        }
    }

    sprintf( buf, "Carry number: %d.  Carry weight: %d.\n\r",
        victim->carry_number, victim->carry_weight );
    send_to_char( buf, ch );

    sprintf( buf, "Age: %d.  Played: %d.  Timer: %d.  Act: %d.\n\r",
        get_age( victim ), (int) victim->played, victim->timer, victim->act );
    send_to_char( buf, ch );

    sprintf( buf, "Master: %s.	Leader: %s.  Affected by: %s.\n\r",
        victim->master      ? victim->master->name   : "(none)",
        victim->leader      ? victim->leader->name   : "(none)",
        affect_bit_name( victim->affected_by ) );
    send_to_char( buf, ch );

    if ( !IS_NPC( victim ) )                                /* OLC */
    {
        sprintf( buf, "Security: %d.\n\r", victim->pcdata->security );
        send_to_char( buf, ch );
    }

    sprintf( buf, "Short description: %s.\n\rLong  description: %s",
        victim->short_descr,
        victim->long_descr[0] != '\0' ? victim->long_descr : "(none).\n\r" );
    send_to_char( buf, ch );

    if ( IS_NPC(victim) && victim->spec_fun != 0 )
        send_to_char( "Mobile has spec fun.\n\r", ch );

    for ( paf = victim->affected; paf != NULL; paf = paf->next )
    {
        sprintf( buf,
            "Spell: '%s' modifies %s by %d for %d hours with bits %s.\n\r",
            skill_table[(int) paf->type].name,
            affect_loc_name( paf->location ),
            paf->modifier,
            paf->duration,
            affect_bit_name( paf->bitvector )
            );
        send_to_char( buf, ch );
    }

    return;
}

void do_mfind( CHAR_DATA *ch, char *argument )
{
    extern int top_mob_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Mfind whom?\n\r", ch );
        return;
    }

    fAll    = FALSE;
    found   = FALSE;
    nMatch  = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
        if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
        {
            nMatch++;
            if ( fAll || is_name( arg, pMobIndex->player_name ) )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %s\n\r",
                    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
                send_to_char( buf, ch );
            }
        }
    }

    if ( !found )
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}

void do_ofind( CHAR_DATA *ch, char *argument )
{
    extern int top_obj_index;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Ofind what?\n\r", ch );
        return;
    }

    fAll    = FALSE;
    found   = FALSE;
    nMatch  = 0;

    /*
     * Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you?
     * -- Furey
     */
    for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    {
        if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
        {
            nMatch++;
            if ( fAll || is_name( arg, pObjIndex->name ) )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %s\n\r",
                    pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
                send_to_char( buf, ch );
            }
        }
    }

    if ( !found )
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );

    return;
}

void do_rfind(CHAR_DATA * ch, char * argument)
{
    extern int top_room;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_DATA *pRoom;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    /*Basically an mfind copy - Magna*/

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Rfind what?\n\r", ch );
        return;
    }

    fAll    = FALSE;
    found   = FALSE;
    nMatch  = 0;

    for ( vnum = 0; nMatch < top_room; vnum++ )
    {
        if ( ( pRoom = get_room_index( vnum ) ) != NULL )
        {
            nMatch++;
            if ( fAll || is_name( arg, pRoom->name ) )
            {
                found = TRUE;
                sprintf( buf, "[%5d] %s\n\r",
                    pRoom->vnum, pRoom->name );
                send_to_char( buf, ch );
            }
        }
    }

    if ( !found )
        send_to_char( "No place like that in hell, earth, or heaven.\n\r", ch );

    return;
}

void do_mwhere( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    bool found;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Mwhere whom?\n\r", ch );
        return;
    }

    found = FALSE;
    for ( victim = char_list; victim != NULL; victim = victim->next )
    {
        if ( IS_NPC(victim)
            &&   victim->in_room != NULL
            &&   is_name( arg, victim->name ) )
        {
            found = TRUE;
            sprintf( buf, "[%5d] %-28s [%5d] %s\n\r",
                victim->pIndexData->vnum,
                victim->short_descr,
                victim->in_room->vnum,
                victim->in_room->name );
            send_to_char( buf, ch );
        }
    }

    if ( !found )
        act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );

    return;
}

void do_shutdown( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    sprintf( buf, "Shutdown by %s.", ch->name );
    append_file( ch, SHUTDOWN_FILE, buf );
    strcat( buf, "\n\r" );
    do_echo( ch, buf );
    do_asave(ch,"changed");
    do_forceauto(ch,"save");
    do_autosave(ch,"");
    merc_down = TRUE;
    return;
}

void do_snoop( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    CHAR_DATA *victim;

    sprintf(buf,"%s: Snoop %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Snoop whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim->desc == NULL )
    {
        send_to_char( "No descriptor to snoop.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Cancelling all snoops.\n\r", ch );
        for ( d = descriptor_list; d != NULL; d = d->next )
        {
            if ( d->snoop_by == ch->desc )
                d->snoop_by = NULL;
        }
        return;
    }

    if ( victim->desc->snoop_by != NULL )
    {
        send_to_char( "Busy already.\n\r", ch );
        return;
    }

    if (IS_SET(victim->act, PLR_WATCHER))
    {
        send_to_char( "Busy already.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( ch->desc != NULL )
    {
        for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by )
        {
            if ( d->character == victim || d->original == victim )
            {
                send_to_char( "No snoop loops.\n\r", ch );
                return;
            }
        }
    }

    victim->desc->snoop_by = ch->desc;
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_oswitch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *mount;

    sprintf(buf,"%s: Oswitch %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( IS_NPC(ch) )
        return;

    if ( arg[0] == '\0' )
    {
        send_to_char( "Switch into what?\n\r", ch );
        return;
    }

    if ( IS_EXTRA(ch, EXTRA_OSWITCH) || IS_HEAD(ch,LOST_HEAD) )
    {
        send_to_char( "You are already oswitched.\n\r", ch );
        return;
    }

    if ( IS_AFFECTED(ch, AFF_POLYMORPH) )
    {
        send_to_char( "Not while polymorphed.\n\r", ch );
        return;
    }

    if ( IS_NPC(ch) || IS_EXTRA(ch,EXTRA_SWITCH) )
    {
        send_to_char( "Not while switched.\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( obj->chobj != NULL )
    {
        send_to_char( "Object in use.\n\r", ch );
        return;
    }

    if ( (mount = ch->mount) != NULL) do_dismount(ch,"");
    obj->chobj = ch;
    ch->pcdata->chobj = obj;
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->extra, EXTRA_OSWITCH);
    free_string(ch->morph);
    ch->morph = str_dup(obj->short_descr);
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_oreturn( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    sprintf(buf,"%s: Oreturn",ch->name);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( IS_NPC(ch) )
        return;

    if ( !IS_EXTRA(ch, EXTRA_OSWITCH) && !IS_HEAD(ch,LOST_HEAD) )
    {
        send_to_char( "You are not oswitched.\n\r", ch );
        return;
    }
    if ( ( obj = ch->pcdata->chobj ) != NULL ) obj->chobj = NULL;
    ch->pcdata->chobj = NULL;
    REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
    REMOVE_BIT(ch->extra, EXTRA_OSWITCH);
    if (IS_HEAD(ch,LOST_HEAD)) REMOVE_BIT(ch->loc_hp[0], LOST_HEAD);
    free_string(ch->morph);
    ch->morph = str_dup("");
    char_from_room(ch);
    char_to_room(ch,get_room_index(ROOM_VNUM_ALTAR));
    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_switch( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    sprintf(buf,"%s: Switch %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Switch into whom?\n\r", ch );
        return;
    }

    if ( ch->desc == NULL )
        return;

    if ( ch->desc->original != NULL )
    {
        send_to_char( "You are already switched.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( victim == ch )
    {
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( victim->desc != NULL )
    {
        send_to_char( "Character in use.\n\r", ch );
        return;
    }
    /*
        if ( !IS_NPC(victim))
        {
        send_to_char( "Only on NPC's.\n\r", ch );
        return;
        }
    */
    SET_BIT(ch->extra, EXTRA_SWITCH);
    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc    = ch->desc;
    ch->desc        = NULL;
    send_to_char( "Ok.\n\r", victim );
    return;
}

void do_return( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s: Return %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    if ( ch->desc == NULL )
        return;

    if ( ch->desc->original == NULL )
    {
        send_to_char( "You aren't switched.\n\r", ch );
        return;
    }

    send_to_char( "You return to your original body.\n\r", ch );
    REMOVE_BIT(ch->desc->original->extra, EXTRA_SWITCH);
    ch->desc->character       = ch->desc->original;
    ch->desc->original        = NULL;
    ch->desc->character->desc = ch->desc;
    ch->desc              = NULL;
    return;
}

void do_mload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;

    if (IS_NPC(ch) ) return;

    if ( ch->pcdata->security < 4 )
    {
        stc( "You do not have the required security 4 to load mobiles\n\r",ch);
        return;
    }

    sprintf(buf,"%s: Mload %s",ch->name,argument);
    if (ch->level < NO_WATCH && ch->trust > 3) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( arg[0] == '\0' || !is_number(arg) )
    {
        send_to_char( "Syntax: mload <vnum>.\n\r", ch );
        return;
    }

    if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL )
    {
        send_to_char( "No mob has that vnum.\n\r", ch );
        return;
    }

    victim = create_mobile( pMobIndex );
    char_to_room( victim, ch->in_room );
    act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
    act( "You have created $N!", ch, NULL, victim, TO_CHAR );
    return;
}

void do_pload( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    ROOM_INDEX_DATA *in_room;
    bool fOld;
    int nLevel = ch->level;

    if ( IS_NPC(ch) || ch->desc == NULL || ch->in_room == NULL ) return;

    if ( argument[0] == '\0' )
    {
        send_to_char( "Syntax: pload <name>.\n\r", ch );
        return;
    }

    if (!check_parse_name( argument ))
    {
        send_to_char( "Thats an illegal name.\n\r", ch );
        return;
    }

    if ( !char_exists(FALSE,argument) )
    {
        send_to_char( "That player doesn't exist.\n\r", ch );
        return;
    }
    else if ( get_char_world( ch, argument ) != NULL )
    {
        send_to_char( "They are already online.....\n\r", ch );
        return;
    }

    sprintf(buf,"%s: Pload %s",ch->name,argument);
    //    if (ch->level < NO_WATCH && ch->trust > 3)
    do_watching(ch,buf);

    argument[0] = UPPER(argument[0]);

    sprintf(buf,"You transform into %s.\n\r",argument);
    send_to_char(buf,ch);
    sprintf(buf,"$n transforms into %s.",argument);
    act(buf,ch,NULL,NULL,TO_ROOM);

    d = ch->desc;
    do_autosave(ch,"");
    in_room = ch->in_room;
    extract_char(ch, TRUE);
    d->character = NULL;
    fOld = load_char_obj( d, argument );
    ch   = d->character;
    ch->next = char_list;
    char_list = ch;
    char_to_room(ch,in_room);
    if ( ch->level >= nLevel ) do_quit(ch,"");
    return;
}

void do_preturn( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    bool fOld;

    if (IS_NPC(ch)) {send_to_char("Huh?\n\r",ch);return;}

    if (ch->pload == NULL) {send_to_char("Huh?\n\r",ch);return;}
    sprintf(arg,ch->pload);
    if (strlen(arg) < 3 || strlen(arg) > 8)
        {send_to_char("Huh?\n\r",ch);return;}

        if (!str_cmp(ch->name,arg)) {send_to_char("Huh?\n\r",ch);return;}

        d = ch->desc;

    sprintf(buf,"You transform back into %s.\n\r",capitalize(ch->pload));
    send_to_char(buf,ch);
    sprintf(buf,"$n transforms back into %s.",capitalize(ch->pload));
    act(buf,ch,NULL,NULL,TO_ROOM);
    do_autosave(ch,"");
    if (ch != NULL && ch->desc != NULL)
        extract_char(ch,TRUE);
    else if (ch != NULL)
        extract_char(ch,TRUE);
    if (ch->desc) ch->desc->character = NULL;
    /*
        ch->next = char_list;
        char_list = ch;
    */
    fOld = load_char_obj(d, capitalize(arg));
    if (ch->in_room != NULL)
        char_to_room(ch,ch->in_room);
    else
        char_to_room(ch,get_room_index(3001));
    free_string(ch->pload);
    ch->pload = str_dup("");
    return;
}

void do_oload( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;

    if (IS_NPC(ch)) {send_to_char("Not while switched.\n\r",ch); return;}
    sprintf(buf,"%s: Oload %s",ch->name,argument);
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );
    //   if ( ch->pcdata->security < 4 && !is_number(arg3))
    //    {
    //	stc( "You do not have the required security 4 to load objects\n\r",ch);
    //	return;
    //    }

    /*  sprintf(buf,"%s: Oload %s",ch->name,argument);*/
    if (ch->level < NO_WATCH && ch->trust > 3) do_watching(ch,buf);

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        level = get_trust( ch );
    }
    else
    {
        /*
         * New feature from Alander.
         */
        if ( !is_number( arg2 ) )
        {
            send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
            return;
        }
        level = atoi( arg2 );
        if ( level < 0 || level > get_trust( ch ) )
        {
            send_to_char( "Limited to your trust level.\n\r", ch );
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL )
    {
        send_to_char( "No object has that vnum.\n\r", ch );
        return;
    }

    obj = create_object( pObjIndex, level );
    if ( CAN_WEAR(obj, ITEM_TAKE) )
    {
        obj_to_char( obj, ch );
        act( "$p appears in $n's hands!", ch, obj, NULL, TO_ROOM );
    }
    else
    {
        obj_to_room( obj, ch->in_room );
        act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
    }
    act( "You create $p.", ch, obj, NULL, TO_CHAR );
    if (obj->questmaker != NULL) free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);
    return;
}

void do_purge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *mount;
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA  *obj_next;

        for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
        {
            vnext = victim->next_in_room;
            if (IS_NPC(victim) && victim->desc == NULL && (mount = victim->mount) == NULL)
                extract_char( victim, TRUE );
        }

        for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next )
        {
            obj_next = obj->next_content;
            extract_obj( obj );
        }

        act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_NPC(victim) )
    {
        send_to_char( "Not on PC's.\n\r", ch );
        return;
    }
    if ( victim->desc != NULL )
    {
        send_to_char( "Not on switched players.\n\r", ch );
        return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}

void do_ppurge( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *mount;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        /* 'purge' */
        CHAR_DATA *vnext;

        for ( victim = ch->in_room->people; victim != NULL; victim = vnext )
        {
            vnext = victim->next_in_room;
            if (IS_NPC(victim) && victim->desc == NULL && (mount = victim->mount) == NULL)
                extract_char( victim, TRUE );
        }

        act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
        send_to_char( "Ok.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !IS_NPC(victim) )
    {
        send_to_char( "Not on PC's.\n\r", ch );
        return;
    }
    if ( victim->desc != NULL )
    {
        send_to_char( "Not on switched players.\n\r", ch );
        return;
    }

    act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
    extract_char( victim, TRUE );
    return;
}

void do_trust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int level;

    sprintf(buf,"%s: Trust %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: trust <char> <trust>.\n\r", ch );
        send_to_char( "Trust being one of: None, Intern, Builder, Questmaker, Judge, or Highjudge.\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if ( !str_cmp( arg2, "none"       ) ) level = 0;
    else if ( !str_cmp( arg2, "intern"     ) ) level = 7;
    else if ( !str_cmp( arg2, "builder"    ) ) level = 8;
    else if ( !str_cmp( arg2, "questmaker" ) ) level = 9;
    else if ( !str_cmp( arg2, "judge"      ) ) level = 10;
    else if ( !str_cmp( arg2, "highjudge"  ) ) level = 11;
    else
    {
        send_to_char( "Please enter: None, Intern, Builder, Questmaker, Judge, or Highjudge.\n\r", ch );
        return;
    }

    if ( level >= get_trust( ch ) )
    {
        send_to_char( "Limited to below your trust.\n\r", ch );
        return;
    }
    send_to_char("Ok.\n\r",ch);
    victim->trust = level;
    return;
}

void do_restore( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;

    sprintf(buf,"%s: Restore %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( get_trust(ch) >=  MAX_LEVEL - 4 && !str_cmp(arg,"all"))
    {
        /* cure all */

        for (d = descriptor_list; d != NULL; d = d->next)
        {
            victim = d->character;

            if (victim == NULL || IS_NPC(victim))
                continue;
            if (ch->fight_timer > 0) continue;

            {
                if (IS_SET(victim->extra,TIED_UP)) REMOVE_BIT(victim->extra,TIED_UP);
                affect_strip(victim,gsn_drowfire);
                affect_strip(victim,gsn_poison);
                affect_strip(victim,gsn_blindness);
                affect_strip(victim,gsn_sleep);
                affect_strip(victim,gsn_curse);
                victim->loc_hp[0] = 0;
                victim->loc_hp[1] = 0;
                victim->loc_hp[2] = 0;
                victim->loc_hp[3] = 0;
                victim->loc_hp[4] = 0;
                victim->loc_hp[5] = 0;
                victim->loc_hp[6] = 0;
                update_pos( victim );
                victim->hit = victim->max_hit;
                victim->mana    = victim->max_mana;
                victim->move    = victim->max_move;
                update_pos( victim);
                if (victim->in_room != NULL)
                {
                    if (!str_cmp(ch->name, "Chrono"))
                    {
                        send_to_char("Chrono thinks you should be healed.\n\rYou Feel strong and younger again.\n\r",victim);
                    }
                    else
                        act("$n has restored you.",ch,NULL,victim,TO_VICT);
                }
            }
        }
        send_to_char("All active players restored.\n\r",ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    victim->hit  = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    victim->loc_hp[0] = 0;
    victim->loc_hp[1] = 0;
    victim->loc_hp[2] = 0;
    victim->loc_hp[3] = 0;
    victim->loc_hp[4] = 0;
    victim->loc_hp[5] = 0;
    victim->loc_hp[6] = 0;
    update_pos( victim );
    if (!str_cmp(ch->name, "Chrono"))
    {
        send_to_char("Chrono thinks you should be healed.\n\rYou Feel strong and younger again.\n\r",victim);
    }
    else
        {act( "$n has restored you.", ch, NULL, victim, TO_VICT );}
        if (!IS_CLASS(ch, CLASS_ANGEL)) send_to_char( "Ok.\n\r", ch );
    return;
}

void do_freeze( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    sprintf(buf,"%s: Freeze %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Bitchslap whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->act, PLR_FREEZE) )
    {
        REMOVE_BIT(victim->act, PLR_FREEZE);
        send_to_char( "You stop crying.\n\r", victim );
        send_to_char( "Crybaby removed.\n\r", ch );
        sprintf( buf, "#rI have thawed #R%s#r.	Thank me later.#n", victim->name );
        make_note( "Immortal", ch->name, "imm", "Thawed", 5, buf );
    }
    else
    {
        SET_BIT(victim->act, PLR_FREEZE);
        send_to_char( "You recieve a BITCHSLAP, and start crying like the bitch you are!\n\r", victim );
        send_to_char( "Crybaby set.\n\r", ch );
        sprintf( buf, "#rI have frozen #R%s#r.	Thank me later.#n", victim->name );
        make_note( "Immortal", ch->name, "imm", "Frozen", 5, buf );
    }

    save_char_obj( victim );

    return;
}

void do_log( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    sprintf(buf,"%s: Log %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Log whom?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        if ( fLogAll )
        {
            fLogAll = FALSE;
            send_to_char( "Log ALL off.\n\r", ch );
        }
        else
        {
            fLogAll = TRUE;
            send_to_char( "Log ALL on.\n\r", ch );
        }
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    /*
     * No level check, gods can log anyone.
     */
    if ( IS_SET(victim->act, PLR_LOG) )
    {
        REMOVE_BIT(victim->act, PLR_LOG);
        send_to_char( "LOG removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_LOG);
        send_to_char( "LOG set.\n\r", ch );
    }

    return;
}

void do_undeny( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char *oldname;
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    ROOM_INDEX_DATA *in_room;

    sprintf(buf,"%s: Undeny %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Undeny whom?", ch );
        return;
    }

    if ((victim = get_char_world(ch, arg)) != NULL)
    {
        send_to_char("They are already online.\n\r", ch );
        return;
    }

    if (!char_exists(FALSE,arg))
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }
    oldname = str_dup(ch->pcdata->switchname);
    d = ch->desc;
    do_autosave(ch,"");
    in_room = ch->in_room;
    extract_char(ch, TRUE);
    d->character = NULL;
    load_char_obj(d, arg);
    ch = d->character;
    ch->next = char_list;
    char_list = ch;
    char_to_room(ch,in_room);

    if ( IS_SET(ch->act, PLR_DENY) )
    {
        REMOVE_BIT(ch->act, PLR_DENY);
        send_to_char( "DENY removed.\n\r", ch );
    }
    else
    {
        send_to_char("They are not DENIED.\n\r", ch );
    }

    d = ch->desc;
    do_autosave(ch,"");
    in_room = ch->in_room;
    extract_char(ch, TRUE);
    d->character = NULL;
    load_char_obj(d, oldname);
    ch = d->character;
    ch->next = char_list;
    char_list = ch;
    char_to_room(ch,in_room);

    sprintf( buf, "#rI have undenied #R%c%s#r.	Thank me later.#n", UPPER(arg[0]),arg+1 );
    make_note( "Immortal", ch->name, "imm", "Undenied", 5, buf );

    return;
}

void do_silence( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    sprintf(buf,"%s: Silence %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Silence whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) )
    {
        send_to_char( "You failed.\n\r", ch );
        return;
    }

    if ( IS_SET(victim->act, PLR_SILENCE) )
    {
        REMOVE_BIT(victim->act, PLR_SILENCE);
        //	send_to_char( "You can use channels again.\n\r", victim );
        send_to_char( "SILENCE removed.\n\r", ch );
    }
    else
    {
        SET_BIT(victim->act, PLR_SILENCE);
        //	send_to_char( "You can't use channels!\n\r", victim );
        //ASCII START

        send_to_char( "             @@@@@@   @@@  @@@  @@@  @@@  @@@@@@@     @@@  @@@  @@@@@@@   @@@  \n\r", victim );
        send_to_char( "            @@@@@@@   @@@  @@@  @@@  @@@  @@@@@@@     @@@  @@@  @@@@@@@@  @@@  \n\r", victim );
        send_to_char( "            !@@       @@!  @@@  @@!  @@@    @@!       @@!  @@@  @@!  @@@  @@!  \n\r", victim );
        send_to_char( "            !@!       !@!  @!@  !@!  @!@    !@!       !@!  @!@  !@!  @!@  !@   \n\r", victim );
        send_to_char( "            !!@@!!    @!@!@!@!  @!@  !@!    @!!       @!@  !@!  @!@@!@!   @!@  \n\r", victim );
        send_to_char( "             !!@!!!   !!!@!!!!  !@!  !!!    !!!       !@!  !!!  !!@!!!    !!!  \n\r", victim );
        send_to_char( "                 !:!  !!:  !!!  !!:  !!!    !!:       !!:  !!!  !!:            \n\r", victim );
        send_to_char( "                !:!   :!:  !:!  :!:  !:!    :!:       :!:  !:!  :!:       :!:  \n\r", victim );
        send_to_char( "            :::: ::   ::   :::  ::::: ::     ::       ::::: ::   ::        ::  \n\r", victim );
        send_to_char( "            :: : :     :   : :   : :  :      :         : :  :    :        :::  \n\r", victim );

        //ASCII FINISH

        send_to_char( "SILENCE set.\n\r", ch );
    }

    return;
}

void do_peace( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *rch;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s: Peace %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room )
    {
        if ( rch->fighting != NULL )
            stop_fighting( rch, TRUE );
    }

    if (!IS_CLASS(ch, CLASS_ANGEL)) send_to_char( "Ok.\n\r", ch );
    return;
}

void do_ban( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    BAN_DATA *pban;

    sprintf(buf2,"%s: Ban %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    if ( IS_NPC(ch) )
        return;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        strcpy( buf, "Banned sites:\n\r" );
        for ( pban = ban_list; pban != NULL; pban = pban->next )
        {
            strcat( buf, pban->name );
            strcat( buf, "    (");
            strcat( buf, pban->reason );
            strcat( buf, ")\n\r" );
        }
        send_to_char( buf, ch );
        return;
    }

    for ( pban = ban_list; pban != NULL; pban = pban->next )
    {
        if ( !str_cmp( arg, pban->name ) )
        {
            send_to_char( "That site is already banned!\n\r", ch );
            return;
        }
    }

    if ( ban_free == NULL )
    {
        pban        = alloc_perm( sizeof(*pban) );
    }
    else
    {
        pban        = ban_free;
        ban_free    = ban_free->next;
    }

    pban->name  = str_dup( arg );
    if (argument[0] == '\0') pban->reason = str_dup("no reason given");
    else pban->reason = str_dup(argument);
    pban->next  = ban_list;
    ban_list    = pban;
    send_to_char( "Ok.\n\r", ch );
    save_bans();

    sprintf( buf, "#rI have banned #R%s#r for #R%s#r.  Thank me later.#n", pban->name, pban->reason );
    make_note( "Immortal", ch->name, "imm", "Banned", 5, buf );
    return;
}

void do_allow( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    BAN_DATA *prev;
    BAN_DATA *curr;

    sprintf(buf,"%s: Allow %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Remove which site from the ban list?\n\r", ch );
        return;
    }

    prev = NULL;
    for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next )
    {
        if ( !str_cmp( arg, curr->name ) )
        {
            if ( prev == NULL )
                ban_list   = ban_list->next;
            else
                prev->next = curr->next;

            sprintf( buf, "#rI have allowed #R%s#r, originally banned for #R%s#r.  Thank me later.#n", curr->name, curr->reason );
            make_note( "Immortal", ch->name, "imm", "Allowed", 5, buf );

            free_string( curr->name );
            free_string( curr->reason );
            curr->next  = ban_free;
            ban_free    = curr;
            send_to_char( "Ok.\n\r", ch );
            save_bans();
            return;
        }
    }

    send_to_char( "Site is not banned.\n\r", ch );
    return;
}

void do_wizlock( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s: Wizlock %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);
    wizlock = !wizlock;

    if ( wizlock )
        send_to_char( "Game wizlocked.\n\r", ch );
    else
        send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}

void do_newlock( CHAR_DATA *ch, char *argument )
{
    extern bool newlock;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s: Newlock %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);
    newlock = !newlock;

    if ( newlock )
        send_to_char( "Game newlocked.\n\r", ch );
    else
        send_to_char( "Game un-newlocked.\n\r", ch );

    return;
}

void do_closemud( CHAR_DATA *ch, char *argument )
{
    extern bool wizlock;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf,"%s: Wizlock %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);
    wizlock = !wizlock;

    if ( wizlock )
        send_to_char( "Game wizlocked.\n\r", ch );
    else
        send_to_char( "Game un-wizlocked.\n\r", ch );

    return;
}

void do_slookup( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument( argument, arg );
    if ( arg[0] == '\0' )
    {
        send_to_char( "Slookup what?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].name == NULL )
                break;
            sprintf( buf, "Sn: %4d Slot: %4d Skill/spell: '%s'\n\r",
                sn, skill_table[sn].slot, skill_table[sn].name );
            send_to_char( buf, ch );
        }
    }
    else
    {
        if ( ( sn = skill_lookup( arg ) ) < 0 )
        {
            send_to_char( "No such skill or spell.\n\r", ch );
            return;
        }

        sprintf( buf, "Sn: %4d Slot: %4d Skill/spell: '%s'\n\r",
            sn, skill_table[sn].slot, skill_table[sn].name );
        send_to_char( buf, ch );
    }

    return;
}

void do_sset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int value;
    int sn;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "#7Syntax: sset <victim> <skill> <value>\n\r",    ch);
        send_to_char( "#7or:	 sset <victim> all     <value>\n\r",   ch);
        send_to_char( "#3Skill being any skill or spell.\n\r",      ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( IS_NPC(victim) )
    {
        send_to_char( "Not on NPC's.\n\r", ch );
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    sn   = 0;
    if ( !fAll && ( sn = skill_lookup( arg2 ) ) < 0 )
    {
        send_to_char( "No such skill or spell.\n\r", ch );
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
        send_to_char( "Value range is 0 to 100.\n\r", ch );
        return;
    }

    if ( fAll )
    {
        for ( sn = 0; sn < MAX_SKILL; sn++ )
        {
            if ( skill_table[sn].name != NULL )
                victim->pcdata->learned[sn] = value;
        }
    }
    else
    {
        victim->pcdata->learned[sn] = value;
    }

    send_to_char("Ok.\n\r",ch);
    return;
}

void do_mset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int value;

    if (IS_NPC(ch) ) return;

    if ( ch->pcdata->security < 6 )
    {
        stc( "You do not have the required security 6 to modify mobiles\n\r",ch);
        return;
    }

    sprintf(buf,"%s: Mset %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "#7Syntax: mset <victim> <field>	<value>\n\r",        ch );
        send_to_char( "#7or:	 mset <victim> <string> <value>\n\r",      ch );
        send_to_char( "\n\r",                           ch );
        send_to_char( "#7Field being one of:\n\r",              ch );
        send_to_char( "#3  str int wis dex con sex level exp\n\r",      ch );
        send_to_char( "#3  gold hp mana move align status\n\r",         ch );
        send_to_char( "#3  thirst drunk full hit dam ac dp cp\n\r",     ch );
        send_to_char( "#3  security rune extra bounty hours\n\r",       ch );
        send_to_char( "#3  hpower origclass dage elfprof\n\r\n\r",      ch );
        send_to_char( "#7String being one of:\n\r",             ch );
        send_to_char( "#3  name short long description title spec clan#n\n\r",  ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;

    /*
     * Set something.
     */
    if( !str_cmp(arg2, "elfprof"))
    {
        if( IS_NPC(victim))
        {
            stc( "Not on mobs.\n\r",ch);
            return;
        }
        if( !IS_CLASS(victim, CLASS_ELF))
        {
            stc( "They are not of the elven bloodline.\n\r",ch);
            return;
        }

        if ( arg3[0] == '\0' )
        {
            stc("Valid options are: Mage, Warrior, Archer, None.\n\r",ch);
            return;
        }
        if( !str_cmp(arg3,"mage"))
        {
            if (IS_SET(victim->special, SPC_ELF_MAGE))
            {
                stc("They are already an elven mage.\n\r",ch);
                return;
            }
            else
            {
                SET_BIT(victim->special, SPC_ELF_MAGE);
                stc("They are now an elven mage.\n\r",ch);
                stc("You are now an elven mage.\n\r",victim);
                return;
            }
        }
        else if( !str_cmp(arg3,"warrior"))
        {
            if (IS_SET(victim->special, SPC_ELF_WARRIOR))
            {
                stc("They are already an elven warrior.\n\r",ch);
                return;
            }
            else
            {
                SET_BIT(victim->special, SPC_ELF_WARRIOR);
                stc("They are now an elven warrior.\n\r",ch);
                stc("You are now an elven warrior.\n\r",victim);
                return;
            }
        }
        else if( !str_cmp(arg3,"archer"))
        {
            if (IS_SET(victim->special, SPC_ELF_ARCHER))
            {
                stc("They are already an elven archer.\n\r",ch);
                return;
            }
            else
            {
                SET_BIT(victim->special, SPC_ELF_ARCHER);
                stc("They are now an elven archer.\n\r",ch);
                stc("You are now an elven archer.\n\r",victim);
                return;
            }
        }
        else if( !str_cmp(arg3,"none"))
        {
            if (IS_SET(victim->special, SPC_ELF_MAGE))
                REMOVE_BIT(victim->special, SPC_ELF_MAGE);
            if (IS_SET(victim->special, SPC_ELF_WARRIOR))
                REMOVE_BIT(victim->special, SPC_ELF_WARRIOR);
            if (IS_SET(victim->special, SPC_ELF_ARCHER))
                REMOVE_BIT(victim->special, SPC_ELF_ARCHER);
            stc("Profession bit removed.\n\r",ch);
            stc("You are now without a profession.\n\r",victim);
            return;
        }
    }

    if( !str_cmp(arg2, "extra"))
    {
        if( IS_NPC(victim))
        {
            stc( "Not on mobs.\n\r",ch);
            return;
        }
        if( !str_cmp(arg3,"pregnant"))
        {
            if( IS_EXTRA(victim,EXTRA_PREGNANT))
                REMOVE_BIT(victim->extra, EXTRA_PREGNANT);
            else
                SET_BIT(victim->extra,EXTRA_PREGNANT);
            stc( "Ok.\n\r",ch);
            return;
        }
        if( !str_cmp(arg3,"labour"))
        {
            if( IS_EXTRA(victim,EXTRA_LABOUR))
                REMOVE_BIT(victim->extra, EXTRA_LABOUR);
            else
                SET_BIT(victim->extra,EXTRA_LABOUR);
            stc( "Ok.\n\r",ch);
            return;
        }
        else if( !str_cmp(arg3,"poly"))
        {
            if( IS_SET(victim->affected_by, AFF_POLYMORPH) )
                REMOVE_BIT(victim->affected_by, AFF_POLYMORPH);
            else
                SET_BIT(victim->affected_by, AFF_POLYMORPH);
            stc( "Ok.\n\r",ch);
            return;
        }
        else if( !str_cmp(arg3,"zuloform"))
        {
            if( IS_SET(victim->polyaff, POLY_ZULOFORM))
                REMOVE_BIT(victim->polyaff, POLY_ZULOFORM);
            else
                SET_BIT(victim->polyaff, POLY_ZULOFORM);
            stc("Ok.\n\r",ch);
            return;
        }
        else if( !str_cmp(arg3,"pdragon"))
        {
            if( IS_SET(victim->polyaff, POLY_DRAGON))
                REMOVE_BIT(victim->polyaff, POLY_DRAGON);
            else
                SET_BIT(victim->polyaff, POLY_DRAGON);
            stc("Ok.\n\r",ch);
            return;
        }
        else if( !str_cmp(arg3,"dragon"))
        {
            if( IS_EXTRA(victim, EXTRA_DRAGON))
                REMOVE_BIT(victim->extra, EXTRA_DRAGON);
            else
                SET_BIT(victim->extra,EXTRA_DRAGON);
            stc("Ok.\n\r",ch);
            return;
        }
        else
        {
            stc( "Pregnant, labour, dragon, pdragon, zuloform, poly.\n\r",ch);
            return;
        }
    }
    if(!str_cmp(arg2,"bounty"))
    {
        if( IS_NPC(victim))
        {
            stc( "Not on mobs.\n\r",ch);
            return;
        }
        if( value < 0) value = 0;
        victim->pcdata->bounty = value;
        sprintf(buf, "%s bounty is now at %d.\n\r",victim->name,victim->pcdata->bounty);
        stc(buf, ch);
        return;
    }

    else if ( !str_cmp( arg2, "status" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 300 )
        {
            send_to_char( "status range is 0 to 300.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->race = value;
            send_to_char("Ok.\n\r", ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    else if (!str_cmp(arg2, "hours"))
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( !is_number( arg3 ) )
        {
            send_to_char( "Value must be numeric.\n\r", ch );
            return;
        }

        value = atoi( arg3 );

        if ( value < 0 || value > 999 )
        {
            send_to_char( "Value must be betwen 0 and 999.\n\r", ch );
            return;
        }

        value *= 3600;
        victim->played = value;
        return;
    }

    if ( !str_cmp( arg2, "str" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 500 )
        {
            send_to_char( "Strength range is 3 to 500.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->pcdata->perm_str = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "dp" ))
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r",ch);
            return;
        }

        /*	if (!IS_CLASS(victim, CLASS_DEMON))
            {
                send_to_char("Only on Demons.\n\r",ch);
                return;
            }
        */
        if ( value < 1 || value > 10000000 )
        {
            send_to_char("Valid range is 1 to 10000000.\n\r",ch);
            return;
        }

        victim->pcdata->stats[DEMON_CURRENT] = value;
        victim->pcdata->stats[DEMON_TOTAL]   = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }
    if ( !str_cmp( arg2, "cp" ))
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r",ch);
            return;
        }

        /*	if (!IS_CLASS(victim, CLASS_DEMON))
            {
                send_to_char("Only on Demons.\n\r",ch);
                return;
            }
        */
        if ( value < 1 || value > 10000000 )
        {
            send_to_char("Valid range is 1 to 10000000.\n\r",ch);
            return;
        }

        victim->pcdata->stats[DROID_POWER] = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "int" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 500 )
        {
            send_to_char( "Intelligence range is 3 to 500.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->pcdata->perm_int = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "wis" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 500 )
        {
            send_to_char( "Wisdom range is 3 to 500.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->pcdata->perm_wis = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "dex" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 500 )
        {
            send_to_char( "Dexterity range is 3 to 500.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->pcdata->perm_dex = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "con" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 3 || value > 500 )
        {
            send_to_char( "Constitution range is 3 to 500.\n\r", ch );
            return;
        }

        if (IS_JUDGE(ch))
        {
            victim->pcdata->perm_con = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "sex" ) )
    {
        if ( value < 0 || value > 2 )
        {
            send_to_char( "Sex range is 0 to 2.\n\r", ch );
            return;
        }
        victim->sex = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
        if ( IS_NPC(victim) && ( value < 1 || value > 10000 ) )
        {
            send_to_char( "Level range is 1 to 10000 for mobs.\n\r", ch );
            return;
        }
        else if (!IS_JUDGE(ch))
        {
            send_to_char( "Sorry, no can do...\n\r", ch );
            return;
        }
        if ( !str_cmp( arg3, "mortal"     ) ) value = 2;
        else if ( !str_cmp( arg3, "avatar"     ) ) value = 3;
        else if ( !str_cmp( arg3, "apprentice" ) ) value = 4;
        else if ( !str_cmp( arg3, "mage"       ) ) value = 5;
        else if ( !str_cmp( arg3, "monk"       ) ) value = 6;
        else if ( !str_cmp( arg3, "archmage"   ) ) value = 6;
        else if ( !str_cmp( arg3, "intern"     ) ) value = 7;
        else if ( !str_cmp( arg3, "builder"    ) ) value = 8;
        else if ( !str_cmp( arg3, "questmaker" ) ) value = 9;
        else if ( !str_cmp( arg3, "judge"      ) ) value = 10;
        else if ( !str_cmp( arg3, "highjudge"  ) ) value = 11;
        //else if ( !str_cmp( arg3, "spouse"  ) ) value = 14;
        else if (!IS_NPC(victim))
        {
            send_to_char( "Level should be one of the following:\n\rMortal, Avatar, Apprentice, Mage, Archmage, Intern, Builder, Questmaker,\n\rJudge, or Highjudge.\n\r", ch );
            return;
        }

        if (value >= (ch->level-2) && !IS_NPC(victim) && ch->level < 13 )
            send_to_char( "Sorry, no can do...\n\r", ch );
        else
        {
            victim->level = value;  victim->trust = value;
            send_to_char("Ok.\n\r",ch);
        }
        if ( victim->level > 7 )
            update_wizlist(victim, value);
        return;
    }

    if ( !str_cmp( arg2, "hitroll" ) || !str_cmp(arg2, "hit" ) )
    {
        if ( !IS_NPC(victim) && ( value < 0 || value > 50 ) )
        {
            send_to_char( "Hitroll range is 0 to 50.\n\r", ch );
            return;
        }
        else if ( IS_NPC(victim) && ( value < 0 || value > 250 ) )
        {
            send_to_char( "Hitroll range is 0 to 250.\n\r", ch );
            return;
        }
        if (!IS_NPC(victim) && !IS_JUDGE(ch) && ch != victim)
        {
            send_to_char( "Sorry, no can do...\n\r", ch );
            return;
        }
        victim->hitroll = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "damroll" ) || !str_cmp( arg2, "dam" ) )
    {
        if ( !IS_NPC(victim) && ( value < 0 || value > 50 ) )
        {
            send_to_char( "Damroll range is 0 to 50.\n\r", ch );
            return;
        }
        else if ( IS_NPC(victim) && ( value < 0 || value > 250 ) )
        {
            send_to_char( "Damroll range is 0 to 250.\n\r", ch );
            return;
        }
        if (!IS_NPC(victim) && !IS_JUDGE(ch) && ch != victim)
        {
            send_to_char( "Sorry, no can do...\n\r", ch );
            return;
        }
        victim->damroll = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "Xhitroll" ) || !str_cmp( arg2, "hit" ) )
    {
        if ( !IS_NPC(victim) && ( value < 0 || value > 50 ) )
        {
            send_to_char( "Xhitroll range is 0 to 50.\n\r", ch );
            return;
        }
        else if ( IS_NPC(victim) && ( value < 0 || value > 250 ) )
        {
            send_to_char( "Xhitroll range is 0 to 250.\n\r", ch );
            return;
        }
        if (!IS_NPC(victim) && !IS_JUDGE(ch) && ch != victim)
        {
            send_to_char( "Sorry, no can do...\n\r", ch );
            return;
        }
        victim->xhitroll = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "Xdamroll" ) || !str_cmp( arg2, "dam" ) )
    {
        if ( !IS_NPC(victim) && ( value < 0 || value > 50 ) )
        {
            send_to_char( "Xdamroll range is 0 to 50.\n\r", ch );
            return;
        }
        else if ( IS_NPC(victim) && ( value < 0 || value > 250 ) )
        {
            send_to_char( "Xdamroll range is 0 to 250.\n\r", ch );
            return;
        }
        if (!IS_NPC(victim) && !IS_JUDGE(ch) && ch != victim)
        {
            send_to_char( "Sorry, no can do...\n\r", ch );
            return;
        }
        victim->xdamroll = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "armor" ) || !str_cmp( arg2, "ac" ) )
    {
        if ( !IS_NPC(victim) && ( value < -200 || value > 200 ) )
        {
            send_to_char( "Armor class range is -200 to 200.\n\r", ch );
            return;
        }
        if (!IS_NPC(victim) && !IS_JUDGE(ch) && ch != victim)
        {
            send_to_char( "Sorry, no can do...\n\r", ch );
            return;
        }
        victim->armor = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "exp" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 )
        {
            send_to_char( "Exp must be at least 0.\n\r", ch );
            return;
        }

        /*	if ( value > 99000000 )
            {
                send_to_char( "No more than 99000000 possible.\n\r", ch );
                return;
            }
        */
        if (IS_JUDGE(ch) || (ch == victim))
        {
            victim->exp = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "gold" ) )
    {
        if (value > 100000 && !IS_JUDGE(ch))
            send_to_char("Don't be so damn greedy!\n\r",ch);
        else
            {victim->gold = value; send_to_char("Ok.\n\r",ch);}
            return;
    }

    if ( !str_cmp( arg2, "hp" ) )
    {
        if ( value < 1 || value > 400000 )
        {
            send_to_char( "Hp range is 1 to 400,000 hit points.\n\r", ch );
            return;
        }
        if (IS_JUDGE(ch) || (ch == victim) || (IS_NPC(victim)) )
        {
            victim->max_hit = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "mana" ) )
    {
        if ( value < 0 || value > 400000 )
        {
            send_to_char( "Mana range is 0 to 400,000 mana points.\n\r", ch );
            return;
        }
        if (IS_JUDGE(ch) || (ch == victim) || (IS_NPC(victim)) )
        {
            victim->max_mana = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "move" ) )
    {
        if ( value < 0 || value > 400000 )
        {
            send_to_char( "Move range is 0 to 400,000 move points.\n\r", ch );
            return;
        }
        if (IS_JUDGE(ch) || (ch == victim) || (IS_NPC(victim)) )
        {
            victim->max_move = value;
            send_to_char("Ok.\n\r",ch);
        }
        else
            send_to_char( "Sorry, no can do...\n\r", ch );
        return;
    }

    if ( !str_cmp( arg2, "align" ) )
    {
        if ( value < -1000 || value > 1000 )
        {
            send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
            return;
        }
        victim->alignment = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "thirst" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 50000 )
        {
            send_to_char( "Thirst range is 0 to 2000.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_THIRST] = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "drunk" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 100 )
        {
            send_to_char( "Drunk range is 0 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_DRUNK] = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "full" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if ( value < 0 || value > 1200 )
        {
            send_to_char( "Full range is 0 to 100.\n\r", ch );
            return;
        }

        victim->pcdata->condition[COND_FULL] = value;
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        free_string( victim->name );
        victim->name = str_dup( arg3 );
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }
        free_string( victim->short_descr );
        victim->short_descr = str_dup( arg3 );
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }
        free_string( victim->long_descr );
        strcpy(buf, arg3);
        strcat(buf, "\n\r");
        victim->long_descr = str_dup( buf );
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "clan" ) )
    {
        free_string( victim->clan );
        victim->clan = str_dup( arg3 );
        send_to_char("Ok.\n\r",ch);
        return;
    }
    if ( !str_cmp( arg2, "title" ) )
    {
        if ( IS_NPC(victim) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        set_title( victim, arg3 );
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "spec" ) )
    {
        if ( !IS_NPC(victim) )
        {
            send_to_char( "Not on PC's.\n\r", ch );
            return;
        }

        if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 )
        {
            send_to_char( "No such spec fun.\n\r", ch );
            return;
        }

        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "security" ) )                     /* OLC */
    {
        if ( IS_NPC( victim ) )
        {
            send_to_char( "Not on NPC's.\n\r", ch );
            return;
        }

        if (( value > (ch->pcdata->security - 2) || value < 0) && ch->level < 13)
        {
            if ( ch->pcdata->security != 0 && ch->level < 13)
            {
                sprintf( buf, "Valid security is 0-%d.\n\r",
                    ch->pcdata->security - 2);
                send_to_char( buf, ch );
                send_to_char( buf, ch );
            }
            else
            {
                send_to_char( "Valid security is 0 only.\n\r", ch );
            }
            return;
        }
        victim->pcdata->security = value;
        return;
    }

    /*
     * Generate usage message.
     */

    do_mset( ch, "" );
    return;
}

void do_oset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    OBJ_DATA *morph;
    OBJ_INDEX_DATA *pObjIndex;
    int value;

    if (IS_NPC(ch)) {send_to_char("Not while switched.\n\r",ch); return;}

    if ( ch->pcdata->security < 6 )
    {
        stc( "You do not have the required security 6 to modify objects\n\r",ch);
        return;
    }

    sprintf(buf,"%s: Oset %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "#7Syntax: oset <object> <field>	<value>\n\r",    ch);
        send_to_char( "#7or:	 oset <object> <string> <value>\n\r",  ch);
        send_to_char( "#7or:	 oset <object> <affect> <value>\n\r",  ch);
        send_to_char( "\n\r",                       ch );
        send_to_char( "#7Field being one of:\n\r",          ch);
        send_to_char( "#3  value0 value1 value2 value3\n\r",        ch);
        send_to_char( "#3  level weight cost timer morph\n\r",      ch);
        send_to_char( "\n\r",                       ch );
        send_to_char( "#7String being one of:\n\r",         ch);
        send_to_char( "#3  name short long ed type extra wear owner\n\r",ch );
        send_to_char( "\n\r",                       ch );
        send_to_char( "#7Affect being one of:\n\r",         ch);
        send_to_char( "#3  str dex int wis con quint\n\r",      ch);
        send_to_char( "#3  hit dam ac hp mana move\n\r",ch );
        stc("  #3attackgood\n\r",ch);
        return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
    }

    if (!IS_QUESTMAKER(ch) && (obj->questmaker == NULL ||
        str_cmp(ch->name,obj->questmaker)))
    {
        send_to_char("You don't have permission to change that item.\n\r", ch);
        return;
    }

    /*
     * Snarf the value (which need not be numeric).
     */
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
    {
        if (obj->item_type == ITEM_WEAPON && !IS_QUESTMAKER(ch))
        {
            send_to_char("You are not authorised to create spell weapons.\n\r", ch);
            return;
        }
        /*	else if (obj->item_type == ITEM_QUEST) {
                send_to_char("You cannot change a quest tokens value with oset.\n\r", ch);
                return;}
            else if (obj->item_type == ITEM_ARMOR && value > 15)
                obj->value[0] = 15;*/
        else
            obj->value[0] = value;
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
    {
        /*	if (obj->item_type == ITEM_WEAPON && value > 10)
                obj->value[1] = 10;
            else*/
        obj->value[1] = value;
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
    {
        /*	if (obj->item_type == ITEM_WEAPON && value > 20)
                obj->value[2] = 20;
            else*/
        obj->value[2] = value;
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
    {
        if (obj->item_type == ITEM_ARMOR && !IS_JUDGE(ch))
            send_to_char("You are not authorised to create spell armour.\n\r", ch);
        else
            {obj->value[3] = value; send_to_char("Ok.\n\r",ch);}
            if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "morph" ) )
    {
        int mnum;
        if ( IS_SET(obj->spectype, SITEM_MORPH) )
        {
            send_to_char( "This item can already morph.\n\r", ch );
            return;
        }
        if ( ( pObjIndex = get_obj_index( value ) ) == NULL )
        {
            send_to_char( "No object has that vnum.\n\r", ch );
            return;
        }
        morph = create_object( pObjIndex, obj->level );
        if ( !CAN_WEAR(morph, ITEM_TAKE) )
            SET_BIT(morph->wear_flags, ITEM_TAKE);
        if ( !IS_SET(obj->spectype, SITEM_MORPH) )
            SET_BIT(obj->spectype, SITEM_MORPH);
        if ( !IS_SET(morph->spectype, SITEM_MORPH) )
            SET_BIT(morph->spectype, SITEM_MORPH);
        obj_to_char( morph, ch );
        act( "$p morphs into $P in $n's hands!", ch, obj, morph, TO_ROOM );
        act( "$p morphs into $P in your hands!", ch, obj, morph, TO_CHAR );
        mnum = obj->wear_loc;
        obj_from_char( obj );
        obj_to_obj( obj, morph );
        if ( morph->wear_flags == obj->wear_flags && mnum != WEAR_NONE )
            equip_char( ch, morph, mnum );
        if (morph->questmaker != NULL) free_string(morph->questmaker);
        morph->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "extra" ) )
    {
        if  ( !str_cmp( arg3, "glow"     )) value = ITEM_GLOW;
        else if ( !str_cmp( arg3, "hum"      )) value = ITEM_HUM;
        else if ( !str_cmp( arg3, "thrown"   )) value = ITEM_THROWN;
        else if ( !str_cmp( arg3, "vanish"   )) value = ITEM_VANISH;
        else if ( !str_cmp( arg3, "nolocate"     )) value = ITEM_NOLOCATE;
        else if ( !str_cmp( arg3, "invis"    )) value = ITEM_INVIS;
        else if ( !str_cmp( arg3, "magic"    )) value = ITEM_MAGIC;
        else if ( !str_cmp( arg3, "nodrop"   )) value = ITEM_NODROP;
        else if ( !str_cmp( arg3, "bless"    )) value = ITEM_BLESS;
        else if ( !str_cmp( arg3, "anti-good"    )) value = ITEM_ANTI_GOOD;
        else if ( !str_cmp( arg3, "anti-evil"    )) value = ITEM_ANTI_EVIL;
        else if ( !str_cmp( arg3, "anti-neutral" )) value = ITEM_ANTI_NEUTRAL;
        else if ( !str_cmp( arg3, "noremove"     )) value = ITEM_NOREMOVE;
        else if ( !str_cmp( arg3, "inventory"    )) value = ITEM_INVENTORY;
        else if ( !str_cmp( arg3, "loyal"    )) value = ITEM_LOYAL;
        else
        {
            send_to_char("Extra flag can be from the following: Glow, Hum, Thrown, Vanish, Invis, Magic, Nodrop, Bless, Anti-Good, Anti-Evil, Anti-Neutral, Noremove, Inventory, Loyal.\n\r",ch); return;
        }

        /* Removing magic flag allows multiple enchants */
        if (IS_SET(obj->extra_flags,value) && value == ITEM_MAGIC && !IS_JUDGE(ch))
            {send_to_char("Sorry, no can do...\n\r",ch); return;}

            if (IS_SET(obj->extra_flags,value))
                REMOVE_BIT(obj->extra_flags, value);
        else
            SET_BIT(obj->extra_flags, value);
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "wear" ) )
    {
        if  ( !str_cmp( arg3, "none" ) || !str_cmp( arg3, "clear" ))
        {
            obj->wear_flags = 0;
            send_to_char("Ok.\n\r",ch);
            if (obj->questmaker != NULL) free_string(obj->questmaker);
            obj->questmaker = str_dup(ch->name);
            return;
        }
        else if ( !str_cmp( arg3, "take"   ))
        {
            if (IS_SET(obj->wear_flags,ITEM_TAKE))
                REMOVE_BIT(obj->wear_flags, ITEM_TAKE);
            else
                SET_BIT(obj->wear_flags, ITEM_TAKE);
            send_to_char("Ok.\n\r",ch);
            if (obj->questmaker != NULL) free_string(obj->questmaker);
            obj->questmaker = str_dup(ch->name);
            return;
        }
        else if ( !str_cmp( arg3, "finger" )) value = ITEM_WEAR_FINGER;
        else if ( !str_cmp( arg3, "neck"   )) value = ITEM_WEAR_NECK;
        else if ( !str_cmp( arg3, "body"   )) value = ITEM_WEAR_BODY;
        else if ( !str_cmp( arg3, "head"   )) value = ITEM_WEAR_HEAD;
        else if ( !str_cmp( arg3, "legs"   )) value = ITEM_WEAR_LEGS;
        else if ( !str_cmp( arg3, "feet"   )) value = ITEM_WEAR_FEET;
        else if ( !str_cmp( arg3, "hands"  )) value = ITEM_WEAR_HANDS;
        else if ( !str_cmp( arg3, "arms"   )) value = ITEM_WEAR_ARMS;
        else if ( !str_cmp( arg3, "about"  )) value = ITEM_WEAR_ABOUT;
        else if ( !str_cmp( arg3, "waist"  )) value = ITEM_WEAR_WAIST;
        else if ( !str_cmp( arg3, "wrist"  )) value = ITEM_WEAR_WRIST;
        else if ( !str_cmp( arg3, "hold"   )) value = ITEM_WIELD;
        else if ( !str_cmp( arg3, "face"   )) value = ITEM_WEAR_FACE;
        else if ( !str_cmp( arg3, "float"   )) value = ITEM_WEAR_FLOAT;
        else if ( !str_cmp( arg3, "medal"   )) value = ITEM_WEAR_MEDAL;
        else if ( !str_cmp( arg3, "special"   )) value = ITEM_WEAR_SPECIAL;
        else if ( !str_cmp( arg3, "bodyart"   )) value = ITEM_WEAR_BODYART;
        else
        {
            send_to_char("Wear location can be from: None, Take, Finger, Neck, Body, Head, Legs, Hands, Arms, About, Waist, Hold, Face,Feet, Float, Medal, Special, Bodyart.\n\r",ch); return;
        }
        if (IS_SET(obj->wear_flags,ITEM_TAKE) ) value += 1;
        obj->wear_flags = value;
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "level" ) )
    {
        if (value < 1) value = 1; else if (value > 200) value = 200;
        if (!IS_JUDGE(ch))
            send_to_char("You are not authorised to change an items level.\n\r", ch);
        else
        {
            obj->level = value; send_to_char("Ok.\n\r",ch);
            if (obj->questmaker != NULL) free_string(obj->questmaker);
            obj->questmaker = str_dup(ch->name);
        }
        return;
    }

    if ( !str_cmp( arg2, "weight" ) )
    {
        obj->weight = value;
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "cost" ) )
    {
        if (value > 100000 && !IS_JUDGE(ch))
            send_to_char("Don't be so damn greedy!\n\r",ch);
        else
        {
            obj->cost = value; send_to_char("Ok.\n\r",ch);
            if (obj->questmaker != NULL) free_string(obj->questmaker);
            obj->questmaker = str_dup(ch->name);
        }
        return;
    }

    if ( !str_cmp( arg2, "timer" ) )
    {
        obj->timer = value;
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "attackgood" ) )
    {
        SET_BIT(obj->extra_flags2, ITEM_ATTACK_GOOD);
        stc("Ok.\n\r",ch);
        return;
    }

    if      ( !str_cmp( arg2, "hitroll" ) || !str_cmp( arg2, "hit" ) )
        {oset_affect(ch,obj,value,APPLY_HITROLL,FALSE);return;}
        else if ( !str_cmp( arg2, "damroll" ) || !str_cmp( arg2, "dam" ) )
            {oset_affect(ch,obj,value,APPLY_DAMROLL,FALSE);return;}
            else if ( !str_cmp( arg2, "armor" ) || !str_cmp( arg2, "ac" ) )
                {oset_affect(ch,obj,value,APPLY_AC,FALSE);return;}
                else if ( !str_cmp( arg2, "hitpoints" ) || !str_cmp( arg2, "hp" ) )
                    {oset_affect(ch,obj,value,APPLY_HIT,FALSE);return;}
                    else if ( !str_cmp( arg2, "mana" ) )
                        {oset_affect(ch,obj,value,APPLY_MANA,FALSE);return;}
                        else if ( !str_cmp( arg2, "move" ) || !str_cmp( arg2, "movement" ) )
                            {oset_affect(ch,obj,value,APPLY_MOVE,FALSE);return;}
                            else if ( !str_cmp( arg2, "str" ) || !str_cmp( arg2, "strength" ) )
                                {oset_affect(ch,obj,value,APPLY_STR,FALSE);return;}
                                else if ( !str_cmp( arg2, "dex" ) || !str_cmp( arg2, "dexterity" ) )
                                    {oset_affect(ch,obj,value,APPLY_DEX,FALSE);return;}
                                    else if ( !str_cmp( arg2, "int" ) || !str_cmp( arg2, "intelligence" ) )
                                        {oset_affect(ch,obj,value,APPLY_INT,FALSE);return;}
                                        else if ( !str_cmp( arg2, "wis" ) || !str_cmp( arg2, "wisdom" ) )
                                            {oset_affect(ch,obj,value,APPLY_WIS,FALSE);return;}
                                            else if ( !str_cmp( arg2, "con" ) || !str_cmp( arg2, "constitution" ) )
                                                {oset_affect(ch,obj,value,APPLY_CON,FALSE);return;}

                                                if( !str_cmp( arg2, "quint") )
                                                {
                                                    if ( obj->item_type != ITEM_WEAPON )
                                                    {
                                                        act("$p is not a weapon.",ch,obj,NULL,TO_CHAR);
                                                        return;
                                                    }
                                                    if( !str_cmp(arg3, "off") )
                                                    {
                                                        REMOVE_BIT(obj->extra_flags,ITEM_MENCHANT);
                                                        stc( "Removed from weapon\n\r", ch);
                                                    }
                                                    else if(!str_cmp(arg3, "on") )
                                                    {
                                                        SET_BIT(obj->extra_flags,ITEM_MENCHANT);
                                                        stc( "Set on weapon\n\r", ch);
                                                    }
                                                    else
                                                    {
                                                        stc( "On or off please\n\r", ch);
                                                        return;
                                                    }
                                                    return;
                                                }

    if ( !str_cmp( arg2, "type" ) )
    {
        if (!IS_JUDGE(ch))
        {
            send_to_char("You are not authorised to change an item type.\n\r", ch);
            return;
        }
        if  (!str_cmp( arg3, "light")) obj->item_type = 1;
        else if (!str_cmp( arg3, "scroll")) obj->item_type = 2;
        else if (!str_cmp( arg3, "wand")) obj->item_type = 3;
        else if (!str_cmp( arg3, "staff")) obj->item_type = 4;
        else if (!str_cmp( arg3, "weapon")) obj->item_type = 5;
        else if (!str_cmp( arg3, "treasure")) obj->item_type = 8;
        else if (!str_cmp( arg3, "armor")) obj->item_type = 9;
        else if (!str_cmp( arg3, "armour")) obj->item_type = 9;
        else if (!str_cmp( arg3, "potion")) obj->item_type = 10;
        else if (!str_cmp( arg3, "furniture")) obj->item_type = 12;
        else if (!str_cmp( arg3, "trash")) obj->item_type = 13;
        else if (!str_cmp( arg3, "container")) obj->item_type = 15;
        else if (!str_cmp( arg3, "drink")) obj->item_type = 17;
        else if (!str_cmp( arg3, "key")) obj->item_type = 18;
        else if (!str_cmp( arg3, "food")) obj->item_type = 19;
        else if (!str_cmp( arg3, "money")) obj->item_type = 20;
        else if (!str_cmp( arg3, "boat")) obj->item_type = 22;
        else if (!str_cmp( arg3, "corpse")) obj->item_type = 23;
        else if (!str_cmp( arg3, "fountain")) obj->item_type = 25;
        else if (!str_cmp( arg3, "pill")) obj->item_type = 26;
        else if (!str_cmp( arg3, "instrument")) obj->item_type = ITEM_INSTRUMENT;
        else if (!str_cmp( arg3, "portal")) obj->item_type = 27;
        else if (!str_cmp( arg3, "stake")) obj->item_type = 30;
        else if (!str_cmp( arg3, "d&d-paper")) obj->item_type = ITEM_DND_PAPER;
        else
        {
            send_to_char("Type can be one of: Light, Scroll, Wand, Staff, Weapon, Treasure, Armor, Potion, Furniture, Trash, Container, Drink, Key, Food, Money, Boat, Corpse, Fountain, Pill, Portal, Stake.\n\r", ch);
            return;
        }
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "owner" ) )
    {
        if (IS_NPC(ch)) {send_to_char("Not while switched.\n\r",ch);return;}
        if (!IS_JUDGE(ch) && (obj->questmaker == NULL || str_cmp(ch->name,obj->questmaker)))
            {send_to_char("Someone else has already changed this item.\n\r",ch);return;}
            if (!str_cmp(arg3,"null") )
                obj->questowner = NULL;
        if ( ( victim = get_char_world( ch, arg3 ) ) == NULL )
            {send_to_char("You cannot find any player by that name.\n\r",ch);return;}
            if (IS_NPC(victim)) {send_to_char("Not on NPC's.\n\r",ch);return;}
            if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        if (obj->questowner != NULL) free_string(obj->questowner);
        obj->questowner = str_dup(victim->name);
        send_to_char("Ok.\n\r",ch);
        return;
    }

    if ( !str_cmp( arg2, "name" ) )
    {
        free_string( obj->name );
        obj->name = str_dup( arg3 );
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "short" ) )
    {
        free_string( obj->short_descr );
        obj->short_descr = str_dup( arg3 );
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "long" ) )
    {
        free_string( obj->description );
        obj->description = str_dup( arg3 );
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    if ( !str_cmp( arg2, "ed" ) )
    {
        EXTRA_DESCR_DATA *ed;

        argument = one_argument( argument, arg3 );
        if ( argument == NULL )
        {
            send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
                ch );
            return;
        }

        if ( extra_descr_free == NULL )
        {
            ed          = alloc_perm( sizeof(*ed) );
        }
        else
        {
            ed          = extra_descr_free;
            extra_descr_free    = extra_descr_free->next;
        }

        ed->keyword     = str_dup( arg3     );
        ed->description     = str_dup( argument );
        ed->next        = obj->extra_descr;
        obj->extra_descr    = ed;
        send_to_char("Ok.\n\r",ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        return;
    }

    /*
     * Generate usage message.
     */
    do_oset( ch, "" );
    return;
}

void do_rset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_DATA *location;
    int value;

    if (IS_NPC(ch) ) return;

    if ( ch->pcdata->security < 6 )
    {
        stc( "You do not have the required security 6 to modify the room\n\r",ch);
        return;
    }

    sprintf(buf,"%s: Rset %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        send_to_char( "#7Syntax: rset <location> <field> value\n\r",    ch
            );
        send_to_char( "\n\r",                       ch );
        send_to_char( "#7Field being one of:\n\r",          ch
            );
        send_to_char( "#3  flags sector\n\r",               ch
            );
        return;
    }

    if ( ( location = find_location( ch, arg1 ) ) == NULL )
    {
        send_to_char( "No such location.\n\r", ch );
        return;
    }

    /*
     * Snarf the value.
     */
    if ( !is_number( arg3 ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }
    value = atoi( arg3 );

    /*
     * Set something.
     */
    if ( !str_cmp( arg2, "flags" ) )
    {
        location->room_flags    = value;
        return;
    }

    if ( !str_cmp( arg2, "sector" ) )
    {
        location->sector_type   = value;
        return;
    }

    /*
     * Generate usage message.
     */
    do_rset( ch, "" );
    return;
}

void do_omni(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;

    if (IS_NPC(ch)) return;

    sprintf(buf," Name       |Level|Trust|Gen|    Hit   |   Mana   |   Move   |  HR |  DR |    AC    |   Quest  | Room \n\r");
    send_to_char(buf,ch);
    sprintf(buf,"------------|-----|-----|---|----------|----------|----------|-----|-----|----------|----------|------\n\r");
    send_to_char(buf,ch);

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        CHAR_DATA *wch;

        if (d->connected != CON_PLAYING) continue;
        if ( !can_see( ch, d->character ) ) continue;
        wch = ( d->original != NULL) ? d->original : d->character;

        sprintf(buf,"%-12s|%5d|%5d|%3d|%10d|%10d|%10d|%5d|%5d|%10d|%10d|%6d\n\r",
            wch->pcdata->switchname,
            wch->level,
            wch->trust,
            wch->generation,
            wch->max_hit,
            wch->max_mana,
            wch->max_move,
            char_hitroll(wch),
            char_damroll(wch),
            char_ac(wch),
            wch->pcdata->quest,
            wch->in_room->vnum
            );

        send_to_char(buf,ch);
    }

    return;
}

void do_users( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *d;
    int count;
    char *st;

    count   = 0;
    buf[0]  = '\0';
    send_to_char( "#y-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#n\n\r", ch );
    send_to_char( "#y|#7 Desc.   Conn. State.    Plyr Name.     Login Site        #y|#n\n\r", ch );
    send_to_char( "#y-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#n\n\r", ch );
    for ( d = descriptor_list; d != NULL; d = d->next )
    {
        if (d->lookup_status != STATUS_DONE) continue;

        if ( d->character != NULL && can_see( ch, d->character ) )
        {
            count++;

            switch (d->connected)
            {
                case CON_PLAYING:          st = "#1	 PLAYING   #n ";    break;
                case CON_GET_NAME:         st = "#3	Get Name   #n ";    break;
                case CON_GET_OLD_PASSWORD:     st = "#3Get Old Passwd#n ";    break;
                case CON_CONFIRM_NEW_NAME:     st = "#3 Confirm Name #n ";    break;
                case CON_GET_NEW_PASSWORD:     st = "#3Get New Passwd#n ";    break;
                case CON_CONFIRM_NEW_PASSWORD: st = "#3Confirm Passwd#n ";    break;
                case CON_GET_NEW_SEX:      st = "#3  Get New Sex #n ";    break;
                case CON_GET_NEW_VT102:        st = "#3 Picking VT102#n ";    break;
                case CON_GET_NEW_ANSI:         st = "#6 Picking ANSI #n ";    break;
                case CON_READ_MOTD:        st = "#7  Reading MOTD#n ";    break;
                case CON_GET_NEW_CLASS:        st = "#5 Picking Class#n ";    break;
                case CON_NOT_PLAYING:      st = "#2 *Not* Playing#n ";    break;
                case CON_NOTE_TO:
                case CON_NOTE_SUBJECT:
                case CON_NOTE_EXPIRE:
                case CON_NOTE_TEXT:
                case CON_NOTE_FINISH:
                    st = "#2 Writing Note #n "; break;

                default:               st = "#4	!UNKNOWN!#n   ";    break;
            }

            if (strlen(d->character->lasthost) < 2)
            {
                sprintf( buf + strlen(buf), "#y[#7%d    %s#y]#7  %6s   #y[#7%s#y]#n\n\r",
                    d->descriptor,
                    st,
                    d->original  ? d->original->pcdata->switchname  :
                d->character ? d->character->pcdata->switchname : "(none)",
                    d->host
                    );
            }
            else
            {
                sprintf( buf + strlen(buf), "#y[#7%d    %s#y]#7  %6s   #y[#7%s#y]#n\n\r",
                    d->descriptor,
                    st,
                    d->original  ? d->original->pcdata->switchname  :
                d->character ? d->character->pcdata->switchname : "(none)",
                    d->character->lasthost
                    );
            }

        }
    }
    sprintf( buf2, "#y[#7%d #RUser%s#y]#n\n\r", count, count == 1 ? "" : "#Rs#n" );
    send_to_char( buf2, ch );

    send_to_char( "#y-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#n\n\r", ch );
    send_to_char( buf, ch );
    send_to_char( "#y-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-#n\n\r", ch );
    return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    bool afk;

    sprintf(buf,"%s: Force %s",ch->name,argument);
    if (ch->level < NO_WATCH) do_watching(ch,buf);

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' || argument[0] == '\0' )
    {
        send_to_char( "Force whom to do what?\n\r", ch );
        return;
    }

    if ( !str_cmp( arg, "all" ) )
    {
        CHAR_DATA *vch;
        CHAR_DATA *vch_next;

        if (get_trust(ch) < MAX_LEVEL - 3)
        {
            send_to_char("Not at your level!\n\r",ch);
            return;
        }
        for ( vch = char_list; vch != NULL; vch = vch_next )
        {
            vch_next = vch->next;

            if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) )
            {
                if (IS_SET(vch->extra, EXTRA_AFK))
                    afk = TRUE;
                else
                    afk = FALSE;
                act( buf, ch, NULL, vch, TO_VICT );
                interpret( vch, argument );
                if (afk)
                    SET_BIT(vch->extra, EXTRA_AFK);
            }
        }
    }

    else
    {
        CHAR_DATA *victim;

        if ( ( victim = get_char_world( ch, arg ) ) == NULL )
        {
            send_to_char( "They aren't here.\n\r", ch );
            return;
        }

        if ( victim == ch )
        {
            send_to_char( "Aye aye, right away!\n\r", ch );
            return;
        }

        if ((get_trust( victim ) >= get_trust( ch )) && (ch->level < MAX_LEVEL))
        {
            send_to_char( "Do it yourself!\n\r", ch );
            return;
        }
        if ( ch->level < 10 )
            act( "$n forces you to '$t'.", ch, argument, victim, TO_VICT );
        interpret( victim, argument );
    }

    send_to_char( "Ok.\n\r", ch );
    return;
}

void do_forceauto( CHAR_DATA *ch, char *argument )
{
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    for ( vch = char_list; vch != NULL; vch = vch_next )
    {
        vch_next = vch->next;

        if ( !IS_NPC(vch) && vch != ch )
        {
            act( "Autocommand: $t.", ch, argument, vch, TO_VICT );
            interpret( vch, argument );
        }
    }
    return;
}

/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
        return;

    if ( IS_SET(ch->act, PLR_WIZINVIS) )
    {
        REMOVE_BIT(ch->act, PLR_WIZINVIS);
        act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else
    {
        act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
        send_to_char( "You slowly vanish into thin air.\n\r", ch );
        SET_BIT(ch->act, PLR_WIZINVIS);
    }

    return;
}

void do_holylight( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch) )
        return;

    if ( IS_SET(ch->act, PLR_HOLYLIGHT) )
    {
        REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
        send_to_char( "#3Holy light mode off.\n\r", ch );
    }
    else
    {
        SET_BIT(ch->act, PLR_HOLYLIGHT);
        send_to_char( "#3Holy light mode on.\n\r", ch );
    }

    return;
}

void do_safe( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];

    one_argument( argument, arg );

    if ( IS_SET(ch->in_room->room_flags,ROOM_SAFE) )
        send_to_char( "You cannot be attacked by other players here.\n\r", ch );
    else
        send_to_char( "You are not safe from player attacks in this room.\n\r", ch );

    if (!IS_CLASS(ch, CLASS_VAMPIRE) )
        return;

    if (ch->in_room->sector_type == SECT_INSIDE)
    {
        send_to_char( "You are inside, which means you are safe from sunlight.\n\r", ch );
        return;
    }

    if (weather_info.sunlight == SUN_DARK)
    {
        send_to_char( "It is not yet light out, so you are safe from the sunlight...for now.\n\r", ch );
        return;
    }

    if ( room_is_dark(ch->in_room) )
    {
        send_to_char( "This room is dark, and will protect you from the sunlight.\n\r", ch );
        return;
    }

    send_to_char( "This room will provide you no protection from the sunlight.\n\r", ch );
    return;
}

void do_qstat( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char("Which item?\n\r",ch);
        return;
    }
    if ( ( obj = get_obj_carry( ch, arg) ) == NULL )
    {
        send_to_char("You are not carrying that item.\n\r",ch);
        return;
    }
    sprintf(buf,"Object %s.\n\r",obj->short_descr);
    send_to_char(buf,ch);
    sprintf(buf,"Owner when worn: %s\n\r",obj->chpoweron);
    send_to_char(buf,ch);
    sprintf(buf,"Other when worn: %s\n\r",obj->victpoweron);
    send_to_char(buf,ch);
    sprintf(buf,"Owner when removed: %s\n\r",obj->chpoweroff);
    send_to_char(buf,ch);
    sprintf(buf,"Other when removed: %s\n\r",obj->victpoweroff);
    send_to_char(buf,ch);
    sprintf(buf,"Owner when used: %s\n\r",obj->chpoweruse);
    send_to_char(buf,ch);
    sprintf(buf,"Other when used: %s\n\r",obj->victpoweruse);
    send_to_char(buf,ch);
    send_to_char("Type:",ch);
    if ( IS_SET(obj->spectype, SITEM_ACTIVATE))
        send_to_char(" Activate",ch);
    if ( IS_SET(obj->spectype, SITEM_TWIST))
        send_to_char(" Twist",ch);
    if ( IS_SET(obj->spectype, SITEM_PRESS))
        send_to_char(" Press",ch);
    if ( IS_SET(obj->spectype, SITEM_PULL))
        send_to_char(" Pull",ch);
    if ( IS_SET(obj->spectype, SITEM_TARGET))
        send_to_char(" Target",ch);
    if ( IS_SET(obj->spectype, SITEM_SPELL))
        send_to_char(" Spell",ch);
    if ( IS_SET(obj->spectype, SITEM_TRANSPORTER))
        send_to_char(" Transporter",ch);
    if ( IS_SET(obj->spectype, SITEM_TELEPORTER))
        send_to_char(" Teleporter",ch);
    if ( IS_SET(obj->spectype, SITEM_OBJECT))
        send_to_char(" Object",ch);
    if ( IS_SET(obj->spectype, SITEM_MOBILE))
        send_to_char(" Mobile",ch);
    if ( IS_SET(obj->spectype, SITEM_ACTION))
        send_to_char(" Action",ch);
    if ( IS_SET(obj->spectype, SITEM_DELAY1))
        send_to_char(" Delay1",ch);
    if ( IS_SET(obj->spectype, SITEM_DELAY2))
        send_to_char(" Delay2",ch);
    if (   !IS_SET(obj->spectype, SITEM_ACTIVATE    )
        && !IS_SET(obj->spectype, SITEM_TWIST       )
        && !IS_SET(obj->spectype, SITEM_PRESS       )
        && !IS_SET(obj->spectype, SITEM_PULL        )
        && !IS_SET(obj->spectype, SITEM_TARGET      )
        && !IS_SET(obj->spectype, SITEM_SPELL       )
        && !IS_SET(obj->spectype, SITEM_TELEPORTER  )
        && !IS_SET(obj->spectype, SITEM_DELAY1      )
        && !IS_SET(obj->spectype, SITEM_DELAY2      )
        && !IS_SET(obj->spectype, SITEM_OBJECT      )
        && !IS_SET(obj->spectype, SITEM_MOBILE      )
        && !IS_SET(obj->spectype, SITEM_ACTION      )
        && !IS_SET(obj->spectype, SITEM_TRANSPORTER ) )
        send_to_char(" No flags set",ch);
    send_to_char(".\n\r",ch);
    sprintf(buf,"Power: %d.\n\r",obj->specpower);
    send_to_char(buf,ch);
    return;
}

void do_qset( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    strcpy( arg3, argument );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '0' )
    {
        send_to_char("You can change the following fields...\n\r", ch);
        send_to_char("chwear   = Message to owner when item is worn.\n\r",ch);
        send_to_char("chrem    = Message to owner when item is removed.\n\r",ch);
        send_to_char("chuse    = Message to owner when item is used.\n\r",ch);
        send_to_char("victwear = Message to others in room when item is worn.\n\r",ch);
        send_to_char("victrem  = Message to others in room when item is removed.\n\r",ch);
        send_to_char("victuse  = Message to others in room when item is used.\n\r",ch);
        send_to_char("type	 activate     = Item can be activated.\n\r",ch);
        send_to_char("		 twist	      = Item can be twisted.\n\r",ch);
        send_to_char("		 press	      = Item can be pressed.\n\r",ch);
        send_to_char("		 pull	      = Item can be pulled.\n\r",ch);
        send_to_char("		 target       = Item can target people (for spell, etc).\n\r",ch);
        send_to_char("		 spell	      = Item can cast spells.\n\r",ch);
        send_to_char("		 transporter  = Owner can transport freely between two locations.\n\r",ch);
        send_to_char("		 teleporter   = Owner can transport to target location at will.\n\r",ch);
        send_to_char("		 object       = Owner can create the specified object.\n\r",ch);
        send_to_char("		 mobile       = Owner can create the specified mobile.\n\r",ch);
        send_to_char("		 action       = Target must perform an action.\n\r",ch);
        send_to_char("		 delay1       = Sets a delay of half a round on spell items.\n\r",ch);
        send_to_char("		 delay2       = Sets a delay of one round on spell items.\n\r",ch);
        send_to_char("power	 <value>      = Spell number/transporter room number.\n\r",ch);
        return;
    }
    if ( ( obj = get_obj_carry( ch, arg1) ) == NULL )
    {
        send_to_char("You are not carrying that item.\n\r",ch);
        return;
    }
    value = is_number( arg3 ) ? atoi( arg3 ) : -1;
    if (!str_cmp(arg2,"chwear") )
    {
        if (obj->chpoweron != NULL) strcpy(buf,obj->chpoweron);
        if (!str_cmp(arg3,"clear"))
        {
            free_string( obj->chpoweron );
            obj->chpoweron = str_dup( "(null)" );
        }
        else if (obj->chpoweron != NULL && buf[0] != '\0' && str_cmp(buf,"(null)"))
        {
            if (strlen(buf)+strlen(arg3) >= MAX_STRING_LENGTH-4)
            {
                send_to_char("Line too long.\n\r",ch);
                return;
            }
            else
            {
                free_string( obj->chpoweron );
                strcat( buf, "\n\r" );
                strcat( buf, arg3 );
                obj->chpoweron = str_dup( buf );
            }
        }
        else
        {
            free_string( obj->chpoweron );
            obj->chpoweron = str_dup( arg3 );
        }
    }
    else if (!str_cmp(arg2,"chrem") )
    {
        if (obj->chpoweroff != NULL) strcpy(buf,obj->chpoweroff);
        if (!str_cmp(arg3,"clear"))
        {
            free_string( obj->chpoweroff );
            obj->chpoweroff = str_dup( "(null)" );
        }
        else if (obj->chpoweroff != NULL && buf[0] != '\0' && str_cmp(buf,"(null)"))
        {
            if (strlen(buf)+strlen(arg3) >= MAX_STRING_LENGTH-4)
            {
                send_to_char("Line too long.\n\r",ch);
                return;
            }
            else
            {
                free_string( obj->chpoweroff );
                strcat( buf, "\n\r" );
                strcat( buf, arg3 );
                obj->chpoweroff = str_dup( buf );
            }
        }
        else
        {
            free_string( obj->chpoweroff );
            obj->chpoweroff = str_dup( arg3 );
        }
    }
    else if (!str_cmp(arg2,"chuse") )
    {
        if (obj->chpoweruse != NULL) strcpy(buf,obj->chpoweruse);
        if (!str_cmp(arg3,"clear"))
        {
            free_string( obj->chpoweruse );
            obj->chpoweruse = str_dup( "(null)" );
        }
        else if (obj->chpoweruse != NULL && buf[0] != '\0' && str_cmp(buf,"(null)"))
        {
            if (strlen(buf)+strlen(arg3) >= MAX_STRING_LENGTH-4)
            {
                send_to_char("Line too long.\n\r",ch);
                return;
            }
            else
            {
                free_string( obj->chpoweruse );
                strcat( buf, "\n\r" );
                strcat( buf, arg3 );
                obj->chpoweruse = str_dup( buf );
            }
        }
        else
        {
            free_string( obj->chpoweruse );
            obj->chpoweruse = str_dup( arg3 );
        }
    }
    else if (!str_cmp(arg2,"victwear") )
    {
        if (obj->victpoweron != NULL) strcpy(buf,obj->victpoweron);
        if (!str_cmp(arg3,"clear"))
        {
            free_string( obj->victpoweron );
            obj->victpoweron = str_dup( "(null)" );
        }
        else if (obj->victpoweron != NULL && buf[0] != '\0' && str_cmp(buf,"(null)"))
        {
            if (strlen(buf)+strlen(arg3) >= MAX_STRING_LENGTH-4)
            {
                send_to_char("Line too long.\n\r",ch);
                return;
            }
            else
            {
                free_string( obj->victpoweron );
                strcat( buf, "\n\r" );
                strcat( buf, arg3 );
                obj->victpoweron = str_dup( buf );
            }
        }
        else
        {
            free_string( obj->victpoweron );
            obj->victpoweron = str_dup( arg3 );
        }
    }
    else if (!str_cmp(arg2,"victrem") )
    {
        if (obj->victpoweroff != NULL) strcpy(buf,obj->victpoweroff);
        if (!str_cmp(arg3,"clear"))
        {
            free_string( obj->victpoweroff );
            obj->victpoweroff = str_dup( "(null)" );
        }
        else if (obj->victpoweroff != NULL && buf[0] != '\0' && str_cmp(buf,"(null)"))
        {
            if (strlen(buf)+strlen(arg3) >= MAX_STRING_LENGTH-4)
            {
                send_to_char("Line too long.\n\r",ch);
                return;
            }
            else
            {
                free_string( obj->victpoweroff );
                strcat( buf, "\n\r" );
                strcat( buf, arg3 );
                obj->victpoweroff = str_dup( buf );
            }
        }
        else
        {
            free_string( obj->victpoweroff );
            obj->victpoweroff = str_dup( arg3 );
        }
    }
    else if (!str_cmp(arg2,"victuse") )
    {
        if (obj->victpoweruse != NULL) strcpy(buf,obj->victpoweruse);
        if (!str_cmp(arg3,"clear"))
        {
            free_string( obj->victpoweruse );
            obj->victpoweruse = str_dup( "(null)" );
        }
        else if (obj->victpoweruse != NULL && buf[0] != '\0' && str_cmp(buf,"(null)"))
        {
            if (strlen(buf)+strlen(arg3) >= MAX_STRING_LENGTH-4)
            {
                send_to_char("Line too long.\n\r",ch);
                return;
            }
            else
            {
                free_string( obj->victpoweruse );
                strcat( buf, "\n\r" );
                strcat( buf, arg3 );
                obj->victpoweruse = str_dup( buf );
            }
        }
        else
        {
            free_string( obj->victpoweruse );
            obj->victpoweruse = str_dup( arg3 );
        }
    }
    else if ( !str_cmp( arg2, "type" ) )
    {
        if ( !str_cmp( arg3, "activate" ) && IS_SET(obj->spectype, SITEM_ACTIVATE) )
            REMOVE_BIT(obj->spectype, SITEM_ACTIVATE);
        else if ( !str_cmp( arg3, "activate" ) && !IS_SET(obj->spectype, SITEM_ACTIVATE) )
            SET_BIT(obj->spectype, SITEM_ACTIVATE);
        else if ( !str_cmp( arg3, "twist" ) && IS_SET(obj->spectype, SITEM_TWIST) )
            REMOVE_BIT(obj->spectype, SITEM_TWIST);
        else if ( !str_cmp( arg3, "twist" ) && !IS_SET(obj->spectype, SITEM_TWIST) )
            SET_BIT(obj->spectype, SITEM_TWIST);
        else if ( !str_cmp( arg3, "press" ) && IS_SET(obj->spectype, SITEM_PRESS) )
            REMOVE_BIT(obj->spectype, SITEM_PRESS);
        else if ( !str_cmp( arg3, "press" ) && !IS_SET(obj->spectype, SITEM_PRESS) )
            SET_BIT(obj->spectype, SITEM_PRESS);
        else if ( !str_cmp( arg3, "pull" ) && IS_SET(obj->spectype, SITEM_PULL) )
            REMOVE_BIT(obj->spectype, SITEM_PULL);
        else if ( !str_cmp( arg3, "pull" ) && !IS_SET(obj->spectype, SITEM_PULL) )
            SET_BIT(obj->spectype, SITEM_PULL);
        else if ( !str_cmp( arg3, "target" ) && IS_SET(obj->spectype, SITEM_TARGET) )
            REMOVE_BIT(obj->spectype, SITEM_TARGET);
        else if ( !str_cmp( arg3, "target" ) && !IS_SET(obj->spectype, SITEM_TARGET) )
            SET_BIT(obj->spectype, SITEM_TARGET);
        else if ( !str_cmp( arg3, "spell" ) && IS_SET(obj->spectype, SITEM_SPELL) )
            REMOVE_BIT(obj->spectype, SITEM_SPELL);
        else if ( !str_cmp( arg3, "spell" ) && !IS_SET(obj->spectype, SITEM_SPELL) )
            SET_BIT(obj->spectype, SITEM_SPELL);
        else if ( !str_cmp( arg3, "transporter" ) && IS_SET(obj->spectype, SITEM_TRANSPORTER) )
            REMOVE_BIT(obj->spectype, SITEM_TRANSPORTER);
        else if ( !str_cmp( arg3, "transporter" ) && !IS_SET(obj->spectype, SITEM_TRANSPORTER) )
            SET_BIT(obj->spectype, SITEM_TRANSPORTER);
        else if ( !str_cmp( arg3, "teleporter" ) && IS_SET(obj->spectype, SITEM_TELEPORTER) )
            REMOVE_BIT(obj->spectype, SITEM_TELEPORTER);
        else if ( !str_cmp( arg3, "teleporter" ) && !IS_SET(obj->spectype, SITEM_TELEPORTER) )
            SET_BIT(obj->spectype, SITEM_TELEPORTER);
        else if ( !str_cmp( arg3, "object" ) && IS_SET(obj->spectype, SITEM_OBJECT) )
            REMOVE_BIT(obj->spectype, SITEM_OBJECT);
        else if ( !str_cmp( arg3, "object" ) && !IS_SET(obj->spectype, SITEM_OBJECT) )
            SET_BIT(obj->spectype, SITEM_OBJECT);
        else if ( !str_cmp( arg3, "mobile" ) && IS_SET(obj->spectype, SITEM_MOBILE) )
            REMOVE_BIT(obj->spectype, SITEM_MOBILE);
        else if ( !str_cmp( arg3, "mobile" ) && !IS_SET(obj->spectype, SITEM_MOBILE) )
            SET_BIT(obj->spectype, SITEM_MOBILE);
        else if ( !str_cmp( arg3, "action" ) && IS_SET(obj->spectype, SITEM_ACTION) )
            REMOVE_BIT(obj->spectype, SITEM_ACTION);
        else if ( !str_cmp( arg3, "action" ) && !IS_SET(obj->spectype, SITEM_ACTION) )
            SET_BIT(obj->spectype, SITEM_ACTION);
        else if ( !str_cmp( arg3, "delay1" ) && IS_SET(obj->spectype, SITEM_DELAY1) )
            REMOVE_BIT(obj->spectype, SITEM_DELAY1);
        else if ( !str_cmp( arg3, "delay1" ) && !IS_SET(obj->spectype, SITEM_DELAY1) )
            SET_BIT(obj->spectype, SITEM_DELAY1);
        else if ( !str_cmp( arg3, "delay2" ) && IS_SET(obj->spectype, SITEM_DELAY2) )
            REMOVE_BIT(obj->spectype, SITEM_DELAY2);
        else if ( !str_cmp( arg3, "delay2" ) && !IS_SET(obj->spectype, SITEM_DELAY2) )
            SET_BIT(obj->spectype, SITEM_DELAY2);
        else
        {
            send_to_char("No such flag to set.\n\r",ch);
            return;
        }
        sprintf(buf,"%s flag toggled.\n\r",capitalize(arg3));
        send_to_char(buf,ch);
        return;
    }
    else if ( !str_cmp( arg2, "power" ) )
        obj->specpower = value;
    else
    {
        send_to_char("No such flag.\n\r",ch);
        return;
    }
    send_to_char("Ok.\n\r",ch);
    return;
}

OBJ_DATA * oclone(OBJ_DATA * pObj)
{
    OBJ_INDEX_DATA * pObjIndex;
    OBJ_DATA * pNewObj;
    AFFECT_DATA *paf;
    AFFECT_DATA *paf2;

    pObjIndex = get_obj_index( pObj->pIndexData->vnum);
    pNewObj = create_object( pObjIndex, pObj->level );
    /* Copy any changed parts of the object. */
    free_string(pNewObj->name);
    pNewObj->name=str_dup(pObj->name);
    free_string(pNewObj->short_descr);
    pNewObj->short_descr=str_dup(pObj->short_descr);
    free_string(pNewObj->description);
    pNewObj->description=str_dup(pObj->description);

    if (pObj->questmaker != NULL && strlen(pObj->questmaker) > 1)
    {
        free_string(pNewObj->questmaker);
        pNewObj->questmaker=str_dup(pObj->questmaker);
    }

    if (pObj->chpoweron != NULL)
    {
        free_string(pNewObj->chpoweron);
        pNewObj->chpoweron=str_dup(pObj->chpoweron);
    }
    if (pObj->chpoweroff != NULL)
    {
        free_string(pNewObj->chpoweroff);
        pNewObj->chpoweroff=str_dup(pObj->chpoweroff);
    }
    if (pObj->chpoweruse != NULL)
    {
        free_string(pNewObj->chpoweruse);
        pNewObj->chpoweruse=str_dup(pObj->chpoweruse);
    }
    if (pObj->victpoweron != NULL)
    {
        free_string(pNewObj->victpoweron);
        pNewObj->victpoweron=str_dup(pObj->victpoweron);
    }
    if (pObj->victpoweroff != NULL)
    {
        free_string(pNewObj->victpoweroff);
        pNewObj->victpoweroff=str_dup(pObj->victpoweroff);
    }
    if (pObj->victpoweruse != NULL)
    {
        free_string(pNewObj->victpoweruse);
        pNewObj->victpoweruse=str_dup(pObj->victpoweruse);
    }
    pNewObj->item_type     = pObj->item_type;
    pNewObj->extra_flags   = pObj->extra_flags;
    pNewObj->wear_flags    = pObj->wear_flags;
    pNewObj->weight    = pObj->weight;
    pNewObj->spectype      = pObj->spectype;
    pNewObj->specpower     = pObj->specpower;
    pNewObj->condition     = pObj->condition;
    pNewObj->toughness     = pObj->toughness;
    pNewObj->resistance    = pObj->resistance;
    pNewObj->quest     = pObj->quest;
    pNewObj->points    = pObj->points;
    pNewObj->cost      = pObj->cost;
    pNewObj->value[0]      = pObj->value[0];
    pNewObj->value[1]      = pObj->value[1];
    pNewObj->value[2]      = pObj->value[2];
    pNewObj->value[3]      = pObj->value[3];
    /*****************************************/

    if (pObj->affected != NULL)
    {
        for ( paf = pObj->affected; paf != NULL; paf = paf->next )
        {
            if (affect_free == NULL)
                paf2 = alloc_perm( sizeof(*paf) );
            else
            {
                paf2 = affect_free;
                affect_free = affect_free->next;
            }
            paf2->type      = 0;
            paf2->duration  = paf->duration;
            paf2->location  = paf->location;
            paf2->modifier  = paf->modifier;
            paf2->bitvector = 0;
            paf2->next      = pNewObj->affected;
            pNewObj->affected      = paf2;
        }
    }

    return pNewObj;
}

void recursive_clone(CHAR_DATA * ch, OBJ_DATA * pContainer, OBJ_DATA * pObj)
{
    OBJ_DATA * pNewObj;
    OBJ_DATA * pContent;

    pNewObj = oclone(pObj);
    obj_to_obj(pNewObj, pContainer);

    if(pObj->item_type == ITEM_CONTAINER)
    {
        for(pContent = pObj->contains; pContent != NULL; pContent = pContent->next_content)
        {
            recursive_clone(ch, pNewObj, pContent);
        }
    }

    return;
}

void do_oclone( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *obj2;
    OBJ_DATA *obj3;

    if (IS_NPC(ch)) {send_to_char("Not while switched.\n\r",ch); return;}
    argument = one_argument( argument, arg1 );

    if ( ch->pcdata->security < 6 )
    {
        stc( "You do not have the required security 6 to clone objects\n\r",ch);
        return;
    }

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Make a clone of what object?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
    }

    obj2 = oclone(obj);
    obj_to_char(obj2,ch);

    if(obj->item_type == ITEM_CONTAINER)
    {
        for(obj3 = obj->contains; obj3 != NULL; obj3 = obj3->next_content)
        {
            recursive_clone(ch, obj2, obj3);
        }
    }

    act( "You create a clone of $p.", ch, obj, NULL, TO_CHAR );
    return;
}

void do_evileye( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int value;

    smash_tilde( argument );
    argument = one_argument( argument, arg1 );
    strcpy( arg2, argument );

    if (ch->power[DISC_DAEM_DISC] < 2)
    {
        stc("You must obtain a level two Mastery of Discord.\n\r",ch);
        return;
    }

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Format is: evileye <option> <value>\n\r", ch );
        send_to_char( "Option ACTION is a text string action performed by you or the viewer.\n\r", ch );
        send_to_char( "Option MESSAGE is a text string shown to the person looking at you.\n\r", ch );
        send_to_char( "Option TOGGLE has values: spell, self, other.\n\r\n\r", ch );
        send_to_char( "Using actions that will cause players to quit, lose connection\n\r",ch);
        send_to_char( "or spend experience on anything (train commands)\n\r",ch);
        send_to_char( "is not allowed, and you will regret using such an evileye.\n\r\n\r",ch);
        if ( ch->poweraction != NULL)
        {
            sprintf(buf,"Current action: %s.\n\r",ch->poweraction);
            send_to_char(buf,ch);
        }
        if ( ch->powertype != NULL)
        {
            sprintf(buf,"Current message: %s.\n\r",ch->powertype);
            send_to_char(buf,ch);
        }
        send_to_char("Current flags:",ch);
        if (IS_SET(ch->spectype,EYE_SPELL)) send_to_char(" Spell",ch);
        if (IS_SET(ch->spectype,EYE_SELFACTION)) send_to_char(" Self",ch);
        if (IS_SET(ch->spectype,EYE_ACTION)) send_to_char(" Other",ch);
        if (!IS_SET(ch->spectype,EYE_SPELL) &&
            !IS_SET(ch->spectype,EYE_SELFACTION) &&
            !IS_SET(ch->spectype,EYE_ACTION)) send_to_char(" None",ch);
        send_to_char(".\n\r",ch);
        return;
    }
    value = is_number( arg2 ) ? atoi( arg2 ) : -1;
    if ( !str_cmp( arg1, "action" ) )
    {
        free_string( ch->poweraction );
        ch->poweraction = str_dup( arg2 );
        return;
    }
    else if ( !str_cmp( arg1, "message" ) )
    {
        free_string( ch->powertype );
        ch->powertype = str_dup( arg2 );
        return;
    }
    else if ( !str_cmp( arg1, "toggle" ) )
    {
        if ( !str_cmp( arg2, "spell" ) && IS_SET(ch->spectype,EYE_SPELL))
            REMOVE_BIT(ch->spectype,EYE_SPELL);
        else if ( !str_cmp( arg2, "spell" ) && !IS_SET(ch->spectype,EYE_SPELL))
            SET_BIT(ch->spectype,EYE_SPELL);
        else if ( !str_cmp( arg2, "self" ) && IS_SET(ch->spectype,EYE_SELFACTION))
            REMOVE_BIT(ch->spectype,EYE_SELFACTION);
        else if ( !str_cmp( arg2, "self" ) && !IS_SET(ch->spectype,EYE_SELFACTION))
            SET_BIT(ch->spectype,EYE_SELFACTION);
        else if ( !str_cmp( arg2, "other" ) && IS_SET(ch->spectype,EYE_ACTION))
            REMOVE_BIT(ch->spectype,EYE_ACTION);
        else if ( !str_cmp( arg2, "other" ) && !IS_SET(ch->spectype,EYE_ACTION))
            SET_BIT(ch->spectype,EYE_ACTION);
        else
        {
            send_to_char("TOGGLE flag should be one of: spell, self, other.\n\r",ch);
            return;
        }
        sprintf(buf,"%s flag toggled.\n\r",capitalize(arg2));
        send_to_char(buf,ch);
        return;
    }
    else
    {
        send_to_char( "Format is: evileye <option> <value>\n\r", ch );
        send_to_char( "Option ACTION is a text string action performed by you or the viewer.\n\r", ch );
        send_to_char( "Option MESSAGE is a text string shown to the person looking at you.\n\r", ch );
        send_to_char( "Option TOGGLE has values: spell, self, other.\n\r\n\r", ch );
        if ( ch->poweraction != NULL)
        {
            sprintf(buf,"Current action: %s.\n\r",ch->poweraction);
            send_to_char(buf,ch);
        }
        if ( ch->powertype != NULL)
        {
            sprintf(buf,"Current message: %s.\n\r",ch->powertype);
            send_to_char(buf,ch);
        }
        send_to_char("Current flags:",ch);
        if (IS_SET(ch->spectype,EYE_SPELL)) send_to_char(" Spell",ch);
        if (IS_SET(ch->spectype,EYE_SELFACTION)) send_to_char(" Self",ch);
        if (IS_SET(ch->spectype,EYE_ACTION)) send_to_char(" Other",ch);
        if (!IS_SET(ch->spectype,EYE_SPELL) &&
            !IS_SET(ch->spectype,EYE_SELFACTION) &&
            !IS_SET(ch->spectype,EYE_ACTION)) send_to_char(" None",ch);
        send_to_char(".\n\r",ch);
    }
    return;
}

void do_artifact( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;

    if (IS_NPC(ch)) {send_to_char("Not while switched.\n\r",ch); return;}

    found = FALSE;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !IS_SET(obj->quest, QUEST_ARTIFACT) ) continue;

        found = TRUE;

        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by != NULL )
        {
            sprintf( buf, "%s carried by %s.\n\r",
                obj->short_descr, PERS(in_obj->carried_by, ch) );
        }
        else
        {
            sprintf( buf, "%s in %s.\n\r",
                obj->short_descr, in_obj->in_room == NULL ? "somewhere" : in_obj->in_room->name );
        }

        buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );
    }

    if ( !found )
        send_to_char( "There are no artifacts in the game.\n\r", ch );

    return;
}

void do_locate( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    OBJ_DATA *in_obj;
    bool found;

    if (IS_NPC(ch)) {send_to_char("Not while switched.\n\r",ch); return;}

    found = FALSE;
    for ( obj = object_list; obj != NULL; obj = obj->next )
    {
        if ( !can_see_obj( ch, obj ) || obj->questowner == NULL ||
            strlen(obj->questowner) < 2 || str_cmp( ch->pcdata->switchname, obj->questowner ))
            continue;

        found = TRUE;

        for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by != NULL )
        {
            sprintf( buf, "%s carried by %s.\n\r",
                obj->short_descr, PERS(in_obj->carried_by, ch) );
        }
        else
        {
            sprintf( buf, "%s in %s.\n\r",
                obj->short_descr, in_obj->in_room == NULL
                ? "somewhere" : in_obj->in_room->name );
        }

        buf[0] = UPPER(buf[0]);
        send_to_char( buf, ch );
    }

    if ( !found )
        send_to_char( "You cannot locate any items belonging to you.\n\r", ch );

    return;
}

void do_claim( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;

    one_argument( argument, arg );

    if (IS_NPC(ch)) {send_to_char("Not while switched.\n\r",ch); return;}

    //if ( ch->exp < 500 ) {send_to_char("It costs 500 exp to claim ownership of an item.\n\r",ch); return;}

    if ( arg[0] == '\0' )
    {
        send_to_char( "What object do you wish to claim ownership of?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry(ch, arg) ) == NULL )
        {send_to_char("You are not carrying that item.\n\r",ch); return;}

        if (obj->item_type == ITEM_QUEST || obj->item_type == ITEM_AMMO ||
        obj->item_type == ITEM_EGG   || obj->item_type == ITEM_VOODOO ||
        obj->item_type == ITEM_MONEY || obj->item_type == ITEM_TREASURE ||
        obj->item_type == ITEM_PAGE  || IS_SET(obj->quest, QUEST_ARTIFACT) ||
        IS_SET(obj->extra_flags,ITEM_NOCLAIM) )
    {
        send_to_char( "You cannot claim that item.\n\r", ch );
        return;
    }
    /* 
    if (!IS_SET(obj->spectype, SITEM_ADAMANTITE) &&
        !IS_SET(obj->spectype, SITEM_COPPER) &&
        !IS_SET(obj->spectype, SITEM_IRON) &&
        !IS_SET(obj->spectype, SITEM_STEEL))
    {
    send_to_char("You can only claim items forged from a rare metal.\n\r",ch);
    return;
    }
    */
    else if (obj->chobj != NULL && !IS_NPC(obj->chobj) &&
        obj->chobj->pcdata->obj_vnum != 0)
    {
        send_to_char( "You cannot claim that item.\n\r", ch );
        return;
    }

    if ( obj->questowner != NULL && strlen(obj->questowner) > 1 )
    {
        if (!str_cmp(ch->pcdata->switchname,obj->questowner))
            send_to_char("But you already own it!\n\r",ch);
        else
            send_to_char("Someone else has already claimed ownership to it.\n\r",ch);
        return;
    }

    //  ch->exp -= 500;
    if (obj->questowner != NULL) free_string(obj->questowner);
    obj->questowner = str_dup(ch->pcdata->switchname);
    act("You are now the owner of $p.",ch,obj,NULL,TO_CHAR);
    act("$n is now the owner of $p.",ch,obj,NULL,TO_ROOM);
    return;
}

void do_gift( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (IS_NPC(ch)) {send_to_char("Not while switched.\n\r",ch); return;}

    if ( ch->exp < 500 ) {send_to_char("It costs 500 exp to make a gift of an item.\n\r",ch); return;}

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Make a gift of which object to whom?\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry(ch, arg1) ) == NULL )
        {send_to_char("You are not carrying that item.\n\r",ch); return;}
        if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
            {send_to_char("Nobody here by that name.\n\r",ch); return;}

            if ( IS_NPC(victim) )
            {
                send_to_char("Not on NPC's.\n\r",ch);
                return;
            }
    if ( obj->questowner == NULL || strlen(obj->questowner) < 2 )
    {
        send_to_char("That item has not yet been claimed.\n\r",ch);
        return;
    }
    if (str_cmp(ch->pcdata->switchname,obj->questowner))
    {
        send_to_char("But you don't own it!\n\r",ch);
        return;
    }
    if (IS_SET(obj->quest, QUEST_RELIC) && ch->level < 7)
    {
        send_to_char("You can't gift relics.\n\r",ch);
        return;
    }
    if ( (IS_SET(obj->quest, QUEST_ANLA) || IS_SET(obj->item_type, ITEM_PIECE) || IS_SET(obj->extra_flags, ITEM_ENGRAVED) ) && ch->level < 7)
    {
        send_to_char("You can't gift that item.\n\r",ch);
        return;
    }
    ch->exp -= 500;
    if (obj->questowner != NULL) free_string(obj->questowner);
    obj->questowner = str_dup(victim->pcdata->switchname);
    act("You grant ownership of $p to $N.",ch,obj,victim,TO_CHAR);
    act("$n grants ownership of $p to $N.",ch,obj,victim,TO_NOTVICT);
    act("$n grants ownership of $p to you.",ch,obj,victim,TO_VICT);
    return;
}

void do_create( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int itemtype = 13;
    int level;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )       itemtype = ITEM_TRASH;
    else if (!str_cmp(arg1,"light"    )) itemtype = ITEM_LIGHT;
    else if (!str_cmp(arg1,"scroll"   )) itemtype = ITEM_SCROLL;
    else if (!str_cmp(arg1,"wand"     )) itemtype = ITEM_WAND;
    else if (!str_cmp(arg1,"staff"    )) itemtype = ITEM_STAFF;
    else if (!str_cmp(arg1,"weapon"   )) itemtype = ITEM_WEAPON;
    else if (!str_cmp(arg1,"treasure" )) itemtype = ITEM_TREASURE;
    else if (!str_cmp(arg1,"armor"    )) itemtype = ITEM_ARMOR;
    else if (!str_cmp(arg1,"armour"   )) itemtype = ITEM_ARMOR;
    else if (!str_cmp(arg1,"potion"   )) itemtype = ITEM_POTION;
    else if (!str_cmp(arg1,"furniture")) itemtype = ITEM_FURNITURE;
    else if (!str_cmp(arg1,"trash"    )) itemtype = ITEM_TRASH;
    else if (!str_cmp(arg1,"container")) itemtype = ITEM_CONTAINER;
    else if (!str_cmp(arg1,"drink"    )) itemtype = ITEM_DRINK_CON;
    else if (!str_cmp(arg1,"key"      )) itemtype = ITEM_KEY;
    else if (!str_cmp(arg1,"food"     )) itemtype = ITEM_FOOD;
    else if (!str_cmp(arg1,"money"    )) itemtype = ITEM_MONEY;
    else if (!str_cmp(arg1,"boat"     )) itemtype = ITEM_BOAT;
    else if (!str_cmp(arg1,"corpse"   )) itemtype = ITEM_CORPSE_NPC;
    else if (!str_cmp(arg1,"fountain" )) itemtype = ITEM_FOUNTAIN;
    else if (!str_cmp(arg1,"pill"     )) itemtype = ITEM_PILL;
    else if (!str_cmp(arg1,"portal"   )) itemtype = ITEM_PORTAL;
    else if (!str_cmp(arg1,"egg"      )) itemtype = ITEM_EGG;
    else if (!str_cmp(arg1,"stake"    )) itemtype = ITEM_STAKE;
    else if (!str_cmp(arg1,"missile"  )) itemtype = ITEM_MISSILE;
    else                 itemtype = ITEM_TRASH;

    if ( arg2[0] == '\0' || !is_number( arg2 ) )
    {
        level = 0;
    }
    else
    {
        level = atoi( arg2 );
        if ( level < 1 || level > 50 )
        {
            send_to_char( "Level should be within range 1 to 50.\n\r", ch );
            return;
        }
    }

    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_PROTOPLASM ) ) == NULL )
    {
        send_to_char( "Error...missing object, please inform KaVir.\n\r", ch );
        return;
    }

    obj = create_object( pObjIndex, level );
    obj->level = level;
    obj->item_type = itemtype;
    obj_to_char(obj,ch);
    if (obj->questmaker != NULL) free_string(obj->questmaker);
    obj->questmaker = str_dup(ch->name);

    act( "You reach up into the air and draw out a ball of protoplasm.", ch, obj, NULL, TO_CHAR );
    act( "$n reaches up into the air and draws out a ball of protoplasm.", ch, obj, NULL, TO_ROOM );
    return;
}

void do_happybuy( CHAR_DATA *ch, char *argument )
{
    char sBuffer[MAX_STRING_LENGTH];
    if (IS_NPC(ch)) return;

    if ((!IS_IMMORTAL(ch)) && ch->pcdata->quest < 5000 )
    {
        send_to_char("You don't have 5000 qps to turn on happy hour.\n\r",ch);
        return;
    }

    if ((!IS_IMMORTAL(ch)) && pulse_doubleexp >= 120 )
    {
        send_to_char("No can do. 120 minutes already for Happy Hour.\n\r",ch);
        return;
    }
    if ((!IS_IMMORTAL(ch)) && pulse_doubleqp >= 120 )
    {
        send_to_char("No can do. 120  minutes already for Questors Delight.\n\r",ch);
        return;
    }
    if ( ch->level <= 6 )
    {
        if ( argument[0] != '\0' )
        {
            sprintf(sBuffer,"#R<#yHappy Hour#y: #L%s #y%s #R>#n\n\r",ch->name,argument);
            do_info(ch,sBuffer);
        }
        else
        {
            sprintf(sBuffer,"#R<#yHappy Hour#y: #L%s#y buys you happyhour.#R>#n\n\r",ch->name);
            do_info(ch,sBuffer);
        }
    }
    else
    {
        if ( argument[0] != '\0' )
        {
            sprintf(sBuffer,"#R<#yImmortal Happy Hour#y: #L%s #y%s #R>#n\n\r",ch->name,argument);
            do_info(ch,sBuffer);
        }
        else
        {
            sprintf(sBuffer,"#R<#yImmortal Happy Hour#y: #L%s#y buys you happyhour.#R>#n\n\r",ch->name);
            do_info(ch,sBuffer);
        }
    }
    global_exp = TRUE;
    global_qp  = TRUE;
    pulse_doubleqp += 8;
    pulse_doubleexp += 16;

    ch->pcdata->quest -= 5000;
    if ( ch->trust > 12 )
    {
        pulse_doubleqp += number_range(1,100);
        pulse_doubleexp += number_range(1,100) * 6;
    }
    if ( number_range(1,1000) <= 50 )
    {
        do_info(ch,"2x time on the counter!");
        pulse_doubleqp += 4;
        pulse_doubleexp += 8;
    }
    if ( number_range(1,1000) <= 100 )
    {
        do_info(ch,"4x time on the counter!");
        pulse_doubleqp += 16;
        pulse_doubleexp += 32;
    }
}

void do_bankbounce( CHAR_DATA *ch, char *argument )
{
    int nEcon;
    bool bOver = FALSE;
    if ( IS_NPC(ch) ) return;
    if ( ch->pcdata->quest < 500 )
    {
        send_to_char("You don't have 500 quest points to bank bounce.\n\r",ch);
        return;
    }
    for ( nEcon = 0; nEcon < MAX_ECONOMY; nEcon++ )
    {
        if ( economy_table[nEcon].nTimeTillRegeneration < 60 )
        {
            if ( economy_table[nEcon].nExpAmount > ( economy_table[nEcon].nMaxAmount*.75 ) )
                economy_table[nEcon].nExpAmount -= economy_table[nEcon].nMaxAmount/100;
            if ( economy_table[nEcon].nExpAmount < ( economy_table[nEcon].nMaxAmount*.75 ) )
                economy_table[nEcon].nExpAmount += economy_table[nEcon].nMaxAmount/100;

            if ( economy_table[nEcon].nPrimalAmount > ( economy_table[nEcon].nMaxAmount*.75 ) )
                economy_table[nEcon].nPrimalAmount -= economy_table[nEcon].nMaxAmount/100;
            if ( economy_table[nEcon].nPrimalAmount < ( economy_table[nEcon].nMaxAmount*.75 ) )
                economy_table[nEcon].nPrimalAmount += economy_table[nEcon].nMaxAmount/100;

            if ( economy_table[nEcon].nQuestAmount > ( economy_table[nEcon].nMaxAmount*.75 ) )
                economy_table[nEcon].nQuestAmount -= economy_table[nEcon].nMaxAmount/100;
            if ( economy_table[nEcon].nQuestAmount < ( economy_table[nEcon].nMaxAmount*.75 ) )
                economy_table[nEcon].nQuestAmount += economy_table[nEcon].nMaxAmount/100;

            if ( economy_table[nEcon].nClassAmount > ( economy_table[nEcon].nMaxAmount*.75 ) )
                economy_table[nEcon].nClassAmount -= economy_table[nEcon].nMaxAmount/100;
            if ( economy_table[nEcon].nClassAmount < ( economy_table[nEcon].nMaxAmount*.75 ) )
                economy_table[nEcon].nClassAmount += economy_table[nEcon].nMaxAmount/100;
            economy_table[nEcon].nTimeTillRegeneration += number_range(1,10);
        }
        else
        {
            stcf(ch,"Bank %d has already gone over the maximum regeneration time. Come back later.\n\r",nEcon);
            bOver = TRUE;
        }
    }
    Save_Economy();
    send_to_char("Bank has been bounced.\n\r",ch);
    if ( bOver == FALSE )
        ch->pcdata->quest -= 500;
    return;
}

void do_award( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_INPUT_LENGTH];
    int kill;
    int value = 1;
    int value2;

    argument = one_argument( argument, buf );

    if (ch->pcdata->score[SCORE_TOTAL_LEVEL] < 1000)
    {
        sprintf( buf, "You only have %d Mob levels. Min of 1000 needed\n\r",ch->pcdata->score[SCORE_TOTAL_LEVEL] );
        send_to_char(buf,ch);
        return;
    }
    if (ch->pcdata->score[SCORE_HIGH_LEVEL] < 1000)
    {
        sprintf( buf, "You wimpy Knight, Fight harder mobs. Your highest mob fight level is %d.\n\r",ch->pcdata->score[SCORE_HIGH_LEVEL] );
        send_to_char(buf,ch);
        return;
    }
    if (ch->pcdata->score[SCORE_TOTAL_LEVEL] >= 1000 && ch->pcdata->score[SCORE_HIGH_LEVEL] >= 100)
    {
        if ( ch->generation == 5 )
        {
            value = 1;
        }
        if ( ch->generation == 4 )
        {
            value = 2;
        }
        if ( ch->generation == 3 )
        {
            value = 3;
        }
        if ( ch->generation == 2 )
        {
            value = 4;
        }
        if ( ch->generation == 1 )
        {
            value = 5;
        }
        if ( ch->generation == 6 )
        {
            send_to_char( "Your generation is 6, get at least 5.\n\r", ch );
            return;
        }

        kill = value*ch->pcdata->score[SCORE_HIGH_LEVEL];

        if (kill > ch->pcdata->score[SCORE_TOTAL_LEVEL])
        {
            sprintf(buf, "You dont have enough mob levels. You need %d moblevels, you have %d.\n\r", kill, ch->pcdata->score[SCORE_TOTAL_LEVEL] );
            send_to_char(buf,ch);
            return;
        }

        value2 =
            ((ch->pcdata->score[SCORE_HIGH_LEVEL]/1000)+(ch->pcdata->score[SCORE_TOTAL_LEVEL]/ch->pcdata->score[SCORE_HIGH_LEVEL]));
        ch->pcdata->quest += (ch->pcdata->score[SCORE_HIGH_LEVEL]/1000) +
            (ch->pcdata->score[SCORE_TOTAL_LEVEL]/ch->pcdata->score[SCORE_HIGH_LEVEL]);
        ch->pcdata->score[SCORE_TOTAL_LEVEL] = 0;
        ch->pcdata->score[SCORE_HIGH_LEVEL] = 0;
        ch->pcdata->score[SCORE_NUM_QUEST] += 1;
        ch->pcdata->score[SCORE_QUEST] += value2;
        sprintf(buf, "You gained %d qp with bonus, your mobscores reset.\n\r", value2 );
        send_to_char(buf,ch);
        return;
    }
}

void do_convert( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if(ch->pkill < 2 )
    {
        send_to_char( "You don't even have 2 Pkills to start out with..\n\r",ch);
        return;
    }
    if(ch->pdeath <= 0)
    {
        send_to_char( "You don't need to modify your PScore.\n\r",ch );
        return;
    }
    if(ch->pcdata->quest < 10000 )
    {
        send_to_char( "You don't have the required 10,000 Qps needed to remove a Pkill from your score.\n\r",ch);
        return;
    }
    else
    {
        ch->pcdata->quest -= 10000;
        ch->pdeath -= 1;
        ch->pkill -= 2;
        sprintf(buf,"Your pkrecord is now at %d, pdrecord at %d.\n\r",ch->pkill , ch->pdeath );
        send_to_char(buf,ch);
        sprintf(buf2,"%s has converted his Pdeath down by one! Someone Get it back up.\n\r",ch->pcdata->switchname );
        do_info(NULL,buf2);
        while(downgrade(ch));                               // See bool downgrade() in upgrade.c
        return;
    }
}

void do_stanceset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    CHAR_DATA *victim;
    int sn = 0, amount = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char( "#wSyntax: #Bstanceset #w<person> <amount>#n\n\r",        ch);
        return;
    }
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !is_number( arg2 ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }

    amount = atoi( arg2 );

    if (amount > 10000 || amount < 0 )
    {
        stc("Max is 10000 for stances.\n\r", ch);
        return;
    }

    for( sn=0 ; sn < 12 ; sn++ )
        victim->stance[sn]                    = amount;

    send_to_char( "Done.\n\r", ch );
}

void do_spellset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    CHAR_DATA *victim;
    int sn = 0, amount = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char( "#wSyntax: #Bspellset #w<person> <amount>#n\n\r", ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !is_number( arg2 ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }
    amount = atoi( arg2 );

    if (amount > 32000 || amount < 0 )
    {
        stc("Max is 32000 for spells.\n\r", ch);
        return;
    }

    for( sn=0 ; sn < 5 ; sn++ )
        victim->spl[sn]                    = amount;

    send_to_char( "Done.\n\r", ch );
}

void do_montset( CHAR_DATA *ch, char *argument )
{
    // do_weapset("Mont",11000);
}

void do_weapset( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    CHAR_DATA *victim;
    int sn = 0, amount = 0;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0')
    {
        send_to_char( "#7Syntax: #Lweapset #7<person> <amount>#n\n\r",  ch);
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( !is_number( arg2 ) )
    {
        send_to_char( "Value must be numeric.\n\r", ch );
        return;
    }

    amount = atoi( arg2 );

    if (amount > 110000 || amount < 0 )
    {
        stc("Max is 110000 for weapons.\n\r", ch);
        return;
    }

    for( sn=0 ; sn < 13 ; sn++ )
        victim->wpn[sn] = amount;

    send_to_char( "Done.\n\r", ch );
}

void do_nuke(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    DESCRIPTOR_DATA *d;
    char buf[MSL];
    char arg1 [MAX_INPUT_LENGTH];
    char   strsave[MAX_STRING_LENGTH];

    /*send_to_char("Nuke is disabled.  Denying instead...\n\r", ch);
    do_deny(ch, argument);*/

    argument = one_argument( argument, arg1 );
    victim = get_char_world(ch, arg1);

    if ( arg1[0] == '\0' )
    {
        send_to_char("Syntax: nuke <player name>\n\r",ch);
        return;
    }
    if (victim == NULL)
    {
        send_to_char("They must be playing.\n\r", ch);
        return;
    }
    if (IS_NPC(victim))
    {
        send_to_char("Not on NPC's\n\r", ch);
        return;
    }
    if (IS_IMMORTAL(victim))
    {
        if ( !str_cmp(ch->name, "Dertaherup") || !str_cmp(ch->name, "Marisella") || !str_cmp(ch->name, "Joshua"))
        {
            send_to_char("Authorization granted.{/", ch);
            //  bkup_char_obj(victim);
            send_to_char("#YYour character has been deleted.#n\n\r", victim);
            sprintf( strsave, "%s%s", PLAYER_DIR, capitalize(victim->name ) );
            unlink(strsave);
            sprintf(buf, "#R%s has been fired by %s#n.", victim->name, ch->name);
            do_info(ch, buf);
            sprintf(buf, "[*****] NUKE: %s has been fired.", victim->name);
            wiznet( NULL, WIZ_MISC, 0, buf );
            log_string(buf);
            d = victim->desc;
            extract_char( victim, TRUE );
            if ( d != NULL )
                close_socket( d );
            return;
        }
        else
        {
            send_to_char("Only Dert or Marisella has the Authorization to Nuke Immortal characters.",ch);
            sprintf(buf, "%s just attempted to NUKE you, but thats ok cuz %s doesn't have that authority.",  ch->name,         ch->sex == SEX_MALE ? "he" : ch->sex == SEX_FEMALE ? "she" : "it");
            send_to_char(buf, victim);
            return;
        }
    }
    if (!IS_NPC(victim))
    {
        //  bkup_char_obj(victim);
        send_to_char("#YYour character has been deleted.#n\n\r", victim);
        sprintf( strsave, "%s%s", PLAYER_DIR, capitalize(victim->name ) );
        unlink(strsave);
        //START ASCII TEST

        //sprintf(buf,"");

        sprintf(buf,"#7               .           .   #0________________#7    .        .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7                     .    #0____( (  (    )   )  )___#n");
        do_echo(ch, buf);
        sprintf(buf,"#7               .         #0(( (  (  )   _    ))  )   ))#7        .   .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7   .                   #0((     (   )(    )  )   (   )  )#7   .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7            .    .   #0(((  ( _(   )   (   _) ) (  () )  )#7        .   .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7                    #0( (  ( (_)   ((    (   )  .((_ ) .  )_#n");
        do_echo(ch, buf);
        sprintf(buf,"#7                   #0(                        )             )#n");
        do_echo(ch, buf);
        sprintf(buf,"#G          *****  ****   *** ***  ******#0 ( _)    #G****  *    * ****** *****#n");
        do_echo(ch, buf);
        sprintf(buf,"#G        *      *    * *   *   * *       #0)     #G*    * *    * *      *    *#n");
        do_echo(ch, buf);
        sprintf(buf,"#G       *  *** ****** *   *   * ******  #0) (   #G*    * *    * ****** *****#7   .#n");
        do_echo(ch, buf);
        sprintf(buf,"#G      *    * *    * *   *   * *        #0( _  #G*    * *    * *      *   *#n");
        do_echo(ch, buf);
        sprintf(buf,"#G      ***** *    * *   *   * ******  #0_  )   #G****   ****  ****** *     *#n");
        do_echo(ch, buf);
        sprintf(buf,"#7                                      #0_        _  _ _     _#7       .   .   .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7     .       .     #0(_((__(_(__(( ( ( |  ) ) ) )_))__))_)___)#7   .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7         .         #0((__)        ))||lll|l||(((          (_))#7       .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7                  .       . #0( (  |(||(|)|||))  )#7     .    .      .      .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7    .       .           .   #0(   ((( (  )  ) ))#7          .     .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7        .      .    .     #0(  . ( ( ( | | ) ) ))   )#7               .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7                .            #0(((((| ( ( )) ) )))) )#7    .   .  .       .  .  .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7    .     .       .  .   #0(  .  ((((((_(|)_)))))     )#7            .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7            .  .          #0(    . ||)(|(|)|(|| . . )#7        .        .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7        .           .   #0(   .    |(||(||)||||   .    )#7 .      .         .  .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7    .      .      .       #0(     ((|(l||||||)))     )#7      .      .   .#n");
        do_echo(ch, buf);
        sprintf(buf,"#7                        #0(( ( ((  (|((||||))|)  ) )  ) _)#n");
        do_echo(ch, buf);
        sprintf(buf,"#G-------------------------------------------------------------------------------#n");
        do_echo(ch, buf);

        //do_echo(ch, buf);
        //END ASCII TEST
        sprintf(buf, "#L%s pulls out a big #RRED NUKE#L and nukes %s#n.", ch->name, victim->name);
        do_info(ch, buf);
        sprintf(buf, "[*****] NUKE: %s has been nuked by %s", victim->name, ch->name);
        wiznet( NULL, WIZ_MISC, 0, buf );
        log_string(buf);
        d = victim->desc;
        extract_char( victim, TRUE );
        if ( d != NULL )
            close_socket( d );
        return;
    }
}

void do_unrestore(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *vict = get_char_world(ch,argument);

    if (!str_cmp(argument, "all"))
    {
        DESCRIPTOR_DATA *d;
        for (d = descriptor_list; d; d = d->next)
        {
            if (!d->character || d->connected > CON_PLAYING || IS_NPC(d->character))
                continue;
            if (get_trust(ch) >= get_trust(d->character))
                do_unrestore(ch,d->character->pcdata->switchname);
        }
        return;
    }
    if (!*argument || vict == NULL)
    {
        stc("Unrestore who??\n\r", ch);
        return;
    }
    vict->hit = 1;
    vict->mana = 1;
    vict->move = 1;

    sprintf(log_buf, "#R%s #DHas been #sUN-#sRESTORED#W!! #sK#sI#sL#sL#s!#n",
        IS_NPC(vict) ? vict->short_descr : vict->pcdata->switchname);
    do_info(ch, log_buf);
    return;
}

void do_railram(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    char buf[MSL];
    char arg1 [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    victim = get_char_world(ch, arg1);

    if ( arg1[0] == '\0' )
    {
        send_to_char("Syntax: railram <player name>\n\r",ch);
        return;
    }
    if (victim == NULL)
    {
        send_to_char("They must be playing.\n\r", ch);
        return;
    }
    if (IS_NPC(victim))
    {
        send_to_char("Not on NPC's\n\r", ch);
        return;
    }
    if (IS_IMMORTAL(victim))
    {
        sprintf(buf, "%s just attempted to FUCK you!", ch->name);
        send_to_char(buf, victim);
        return;
    }
    if (!IS_NPC(victim))
    {
        sprintf(buf, "#R%s bends %s over the nearest rail, and #rRAMS THEIR BUTT HARD!#n.", ch->name, victim->name);
        do_info(ch, buf);
        sprintf(buf, "[*****] RAILRAM %s has been railed by  %s", victim->name, ch->name);
        wiznet( NULL, WIZ_MISC, 0, buf );
        log_string(buf);

        act( "$n has been fucked in a way that should never be done.", victim, NULL, NULL, TO_ROOM );
        act( "$n is mortally sore, and hurting everywhere.", victim, NULL, NULL, TO_ROOM );
        send_to_char("You are  mortally sore, and hurting everywhere.\n\r", victim );
        victim->hit = victim->max_hit/2;
        return;
    }

    return;
}

void do_twit( CHAR_DATA *ch, char *argument )
{
    char    buf[MAX_STRING_LENGTH];
    CHAR_DATA   *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char("Twit whom?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_world( ch, arg ) ) == NULL )
    {
        send_to_char("They aren't here.\n\r", ch );
        return;
    }

    if ( victim->level >= ch->level )
    {
        send_to_char( "You can't touch them.\n\r",ch);
        return;
    }
    victim->max_hit -= 1000;
    victim->max_mana -= 1000;
    victim->max_move -= 1000;
    victim->pcdata->quest -=1000;
    victim->exp = 0;
    victim->move = 1;
    victim->mana = 1;
    victim->hit = 1;
    do_remove(victim,"all");
    do_drop(victim,"all");
    do_recall(victim,"");
    do_sit(victim,"");
    do_sleep(victim, "");
    SET_BIT(victim->extra, TIED_UP);
    SET_BIT(victim->extra, BLINDFOLDED);
    SET_BIT(victim->extra, GAGGED);
    SET_BIT(victim->affected_by, AFF_FLAMING);
    SET_BIT(ch->pcdata->jflags, JFLAG_BULLY);

    if(!str_cmp(ch->pcdata->switchname, "Beorn"))
    {
        do_chat(victim,"Yay, I got sent to Niflheim by Beorn, Its so fun I would not recommend it.\n\r");
        sprintf(buf, "%s got sent to Niflheim by %s, don't try it people.\n\r", victim->pcdata->switchname, ch->pcdata->switchname );
    }
    else if(!str_cmp(ch->pcdata->switchname, "Chrono"))
    {
        do_chat(victim,"Yay, I got farted on by Chrono, Its so fun I would not recommend it.\n\r");
        sprintf(buf, "%s got farted on by %s, don't try it people.\n\r", victim->pcdata->switchname, ch->pcdata->switchname );
    }
    else
    {
        do_chat(victim,"Yay, I got twited by a Immortal, Its so fun I would not recommend it.\n\r");
        sprintf(buf, "%s got Twitted by %s, and lost stats for being an idiot, dont be stupid people!.\n\r", victim->pcdata->switchname, ch->pcdata->switchname );
    }
    WAIT_STATE(victim,48);
    do_info(NULL,buf);
    return;
}

void do_commit( CHAR_DATA *ch, char *argument )
{
    if ( ch->played/3600 > 5 )
    {
        send_to_char( "You have more then 10 hours already.\n\r",ch);
        return;
    }
    if ( ch->played/3600 < 5 )
    {
        send_to_char( "You will not be able to bitch about this.\n\r",ch );
        ch->played = 18001;
        do_save(ch,"");
        send_to_char( "Done.\n\r",ch );
        return;
    }
}

void do_genbuy( CHAR_DATA *ch, char *argument )
{
    if( ch->generation < 6 )
    {
        send_to_char( "Your generation is already lowered.\n\r",ch);
        return;
    }
    if( ch->generation == 6 )
    {
        send_to_char( "You already HAVE gen6!!\n\r",ch);
        return;
    }
    if( ch->pcdata->quest < 100000 )
    {
        send_to_char( "You need more Qps.\n\r",ch);
        return;
    }
    if (ch->level < 3 )
    {
        send_to_char( "You must be avatar.\n\r",ch);
        return;
    }
    ch->pcdata->quest -= 100000;
    ch->generation = 6;
    send_to_char( "Done.\n\r",ch);
    return;
}

void do_realm( CHAR_DATA *ch, char *argument )
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    int vnum = 0;
    int price;
    char buf1[MSL];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    price = 0;
    //if ( (ch->pcdata->upgrade_level+1) < 5 )
    //{
    //	price = ( (5 - (ch->pcdata->upgrade_level+1)) * 2000 );
    //  }
    price = 150000;
    /*    if ( ch->generation != 1 )
        {
        send_to_char( "You dont have generation 1.\n\r",ch);
        return;
        }*/

    if (ch->pcdata->upgrade_level < 3)
    {
        send_to_char("You must at least upgrade 4 to buy Realm EQ.\n\r",ch);
        return;
    }

    if ( ch->pcdata->quest < price )
    {
        sprintf(buf1,"You do not have 150000 qps to make a Another Land Item.\n\r");
        send_to_char(buf1,ch);
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Please specify which piece of Another Land armor you wish to make: Ring Collar Plate Helmet Leggings Boots Gauntlets Sleeves Cape Belt Bracer Visor Katana Longsword Float Girdle Tattoo Dagger.\n\r",ch);
        return;
    }

    else if (!str_cmp(arg,"ring"     ))   vnum = 101000;
    else if (!str_cmp(arg,"collar"   ))   vnum = 101001;
    else if (!str_cmp(arg,"plate"    ))   vnum = 101002;
    else if (!str_cmp(arg,"helmet"   ))   vnum = 101003;
    else if (!str_cmp(arg,"leggings" ))   vnum = 101004;
    else if (!str_cmp(arg,"boots"    ))   vnum = 101005;
    else if (!str_cmp(arg,"gauntlets"))   vnum = 101006;
    else if (!str_cmp(arg,"sleeves"  ))   vnum = 101007;
    else if (!str_cmp(arg,"cape"     ))   vnum = 101008;
    else if (!str_cmp(arg,"belt"     ))   vnum = 101009;
    else if (!str_cmp(arg,"bracer"   ))   vnum = 101010;
    else if (!str_cmp(arg,"visor"    ))   vnum = 101011;
    else if (!str_cmp(arg,"longsword"))   vnum = 101013;
    else if (!str_cmp(arg,"float"))   vnum = 101012;
    else if (!str_cmp(arg,"girdle"))      vnum = 101014;
    else if (!str_cmp(arg,"katana"))      vnum = 101015;
    else if (!str_cmp(arg,"tattoo"))      vnum = 101016;
    else if (!str_cmp(arg,"dagger"))      vnum = 200504;
    else
    {
        do_realm(ch,"");
        return;
    }

    if ( vnum == 0 || (pObjIndex = get_obj_index( vnum )) == NULL)
    {
        send_to_char("Missing object, please inform Jinnai.\n\r",ch);
        return;
    }
    ch->pcdata->quest -= price;
    obj = create_object(pObjIndex, 50);
    obj->questowner = str_dup(ch->pcdata->switchname);
    SET_BIT(obj->quest, QUEST_ANLA);
    SET_BIT(obj->quest, QUEST_SPELLPROOF);
    SET_BIT(obj->extra_flags,ITEM_ENGRAVED);
    obj_to_char(obj, ch);
    act("$p floats into your hands from a mysterious source.",ch,obj,NULL,TO_CHAR);
    act("$p floats into $n's hands from a mysterious source.",ch,obj,NULL,TO_ROOM);
    return;
}

void do_chaoseq( CHAR_DATA *ch, char *argument )
// New upgrade eq code -Beorn
{
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    char arg[MAX_INPUT_LENGTH];
    int vnum = 0;
    int price;
    char buf1[MSL];

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;
    price = 0;
    price = 300000;

    if (ch->pcdata->upgrade_level < 4)
    {
        send_to_char("You must at least upgrade 5 to buy Realm EQ.\n\r",ch);
        return;
    }

    if ( ch->pcdata->quest < price )
    {
        sprintf(buf1,"You do not have 300000 qps to make a Chaos Item.\n\r");
        send_to_char(buf1,ch);
        return;
    }

    if (arg[0] == '\0')
    {
        send_to_char("Please specify which piece of Chaos armor you wish to make: Ring Collar Plate Helm Leggings Boots Gauntlets Sleeves Cape Belt Bracer Visor Katana Longsword Float Satchel Tattoo Dagger.\n\r",ch);
        return;
    }

    else if (!str_cmp(arg,"ring"     ))   vnum = 200501;
    else if (!str_cmp(arg,"collar"   ))   vnum = 200502;
    else if (!str_cmp(arg,"plate"    ))   vnum = 200503;
    else if (!str_cmp(arg,"helm"     ))   vnum = 200505;
    else if (!str_cmp(arg,"leggings" ))   vnum = 200506;
    else if (!str_cmp(arg,"boots"    ))   vnum = 200507;
    else if (!str_cmp(arg,"gauntlets"))   vnum = 200508;
    else if (!str_cmp(arg,"sleeves"  ))   vnum = 200509;
    else if (!str_cmp(arg,"cape"     ))   vnum = 200510;
    else if (!str_cmp(arg,"belt"     ))   vnum = 200511;
    else if (!str_cmp(arg,"bracer"   ))   vnum = 200512;
    else if (!str_cmp(arg,"visor"    ))   vnum = 200513;
    else if (!str_cmp(arg,"longsword"))   vnum = 200515;
    else if (!str_cmp(arg,"float"    ))   vnum = 200516;
    else if (!str_cmp(arg,"satchel"  ))   vnum = 200517;
    else if (!str_cmp(arg,"katana"   ))   vnum = 200514;
    else if (!str_cmp(arg,"tattoo"   ))   vnum = 200519;
    else if (!str_cmp(arg,"dagger"   ))   vnum = 200518;
    else
    {
        do_chaoseq(ch,"");
        return;
    }

    if ( vnum == 0 || (pObjIndex = get_obj_index( vnum )) == NULL)
    {
        send_to_char("Missing object, please inform Beorn.\n\r",ch);
        return;
    }
    ch->pcdata->quest -= price;
    obj = create_object(pObjIndex, 50);
    obj->questowner = str_dup(ch->pcdata->switchname);
    SET_BIT(obj->quest, QUEST_ANLA);
    SET_BIT(obj->quest, QUEST_SPELLPROOF);
    SET_BIT(obj->extra_flags,ITEM_ENGRAVED);
    obj_to_char(obj, ch);
    act("$p floats into your hands from the twisting nether.",ch,obj,NULL,TO_CHAR);
    act("$p floats into $n's hands from the twisting nether.",ch,obj,NULL,TO_ROOM);
    return;
}

void do_validexchange( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC(ch)) return;

    if ( ch->pcdata->quest < 1000 )
    {
        send_to_char(" You need at least 1K Qps.\n\r",ch);
        return;
    }

    ch->pcdata->quest -= 250;
    ch->pcdata->score[SCORE_NUM_QUEST] += 1;
    ch->pcdata->score[SCORE_QUEST] += 750;
    send_to_char( " Transaction Complete: Taken 250 Qps as interest.\n\r",ch );
    ch->pcdata->questsrun += 1;
    ch->pcdata->questtotal += 750;
    return;
}

void do_mightscore( CHAR_DATA *ch, char *argument )
{
    char mig[MAX_STRING_LENGTH];

    if ( IS_NPC(ch)) return;

    sprintf(mig,"Your Mightscore: %d.\n\r",getMight(ch));
    send_to_char(mig,ch);
    if ( (get_age(ch) - 17) < 2 || ch->level < 3 || reachedDecapLimit(ch))
    {
        stcf(ch,"You cannot pkill for a legitiment pkscore.\n\r");
        return;
    }
    else if ( (ch->pkill-ch->pdeath-3) < 1 )
        stcf(ch,"Low PK Range: %d High PK Range: %d\n\r",1,ch->pkill-ch->pdeath+3);
    else
        stcf(ch,"Low PK Range: %d High PK Range: %d\n\r",ch->pkill-ch->pdeath-3,ch->pkill-ch->pdeath+3);
    return;
}

void do_primalgain( CHAR_DATA *ch, char *argument)
{
    char arg[MIL];
    int value;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (arg[0] == '\0')
    {
        send_to_char("How many primal do you want?\n\r",ch);
        return;
    }

    if (!is_number(arg))
    {
        stc( "Huh, Me talk numbers only...\n\r",ch);
        return;
    }

    value = atoi(arg);

    if (value > 1000 || value < 1 )
    {
        send_to_char("From 1-1000 possible.\n\r",ch);
        return;
    }

    if ( value*1000000 > ch->exp )
    {
        sprintf(buf, "You need %d exp. you have %d.\n\r",value*1000000,
            ch->exp);
        send_to_char(buf,ch);
        return;
    }

    if ( ch->practice > 5000 )
    {
        stc( " You have already over 5,000 Primal. Loose some.\n\r",ch);
        return;
    }

    ch->practice += value;
    ch->exp -= value*1000000;
    sprintf( buf, "You gain %d primal.\n\r",value);
    send_to_char(buf,ch);
    return;
}

void do_buystatus(CHAR_DATA *ch, char *argument)
{
    char arg[MIL];
    int value;

    argument = one_argument( argument, arg );

    if (IS_NPC(ch)) return;

    if (arg[0] == '\0')
    {
        send_to_char("Syntxt buystatus <amount>\n\r",ch);
        return;
    }

    if (!is_number(arg))
    {
        stc( "Numbers only...\n\r",ch);
        return;
    }

    value = atoi(arg);

    if (value > 5 || value < 1 )
    {
        send_to_char("From 1-5 possible.\n\r",ch);
        return;
    }
    if ( value*250000 > ch->pcdata->quest )
    {
        sprintf(buf, "You need %d quest points. you have %d.\n\r",value*250000, ch->pcdata->quest);
        send_to_char(buf,ch);
        return;
    }

    if ( ch->race > 25 )
    {
        stc( " You already have 25 status or above. Loose some.\n\r",ch);
        return;
    }

    ch->race += value;
    ch->pcdata->quest -= value*250000;
    sprintf( buf, "You gain %d status.\n\r",value);
    send_to_char(buf,ch);
    return;
}

void do_store( CHAR_DATA *ch, char *argument )
{
    char caCommand[100];
    if ( IS_NPC(ch) ) return;
    argument = one_argument(argument,caCommand);
    if ( caCommand[0] == '\0' )
    {
        send_to_char("Syntax: Store Add/Remove/List.\n\r",ch);
        return;
    }
    if ( !str_cmp(caCommand,"Add") )
    {
        if ( ch->exp < 2000000000 )
        {
            send_to_char("Sorry, you don't have 2billion exp to store the exp.\n\r",ch);
            return;
        }
        else
        {
            ch->pcdata->store++;
            ch->exp -= 2000000000;
        }
    }
    else if ( !str_cmp(caCommand,"Remove") )
    {
        if ( ch->exp > 0 )
        {
            do_token(ch,"exp all");
        }
        if ( ch->pcdata->store <= 0 )
        {
            send_to_char("You don't have any exp stored.\n\r",ch);
            return;
        }
        else
        {
            ch->pcdata->store--;
            ch->exp += 2000000000;
        }
    }
    else if ( !str_cmp(caCommand,"List") )
    {
        stcf(ch,"You have: %d 2 Billion Exp Stored.\n\r",ch->pcdata->store);
        return;
    }
    else
    {
        send_to_char("Sorry, you didn't follow the syntax.\n\r",ch);
    }
    return;
}

void do_token( CHAR_DATA *ch, char *argument )
{
    char non[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    int value;

    argument = one_argument( argument, arg );
    argument = one_argument(argument, arg2);

    if (IS_NPC(ch)) return;
    if (arg[0] == '\0')
    {
        send_to_char("Syntax: token <exp|primal|quest|store> <amount/all>.\n\r",ch);
        return;
    }
    if (!str_cmp(arg, "exp"))
    {
        if ( ch->exp <= 0 ) return;
        value = atoi(arg2);
        if (((value = atoi(arg2)) < 1 || value > 2000000000) && str_cmp(arg2,"all"))
        {
            send_to_char("From 1 to 2,000,000,000 exp please.\n\r",ch);
            return;
        }
        if (!is_number(arg2) && str_cmp(arg2,"all"))
        {
            send_to_char("Please enter a numeric value.\n\r",ch);
            return;
        }
        if (value > ch->exp)
        {
            sprintf(non,"You only have %d left to put into a exp token.\n\r",ch->exp);
            send_to_char(non,ch);
            return;
        }
        if ( !str_cmp(arg2,"all" ) ) value = ch->exp;
        victim = get_char_room( ch, arg2 );

        if ( ( pObjIndex = get_obj_index( OBJ_VNUM_PROTOPLASM ) ) == NULL )
        {
            send_to_char( "Error...missing object, please inform Trace.\n\r",ch );
            return;
        }

        ch->exp -= value;
        if (ch->exp < 0) ch->exp = 0;
        obj = create_object( pObjIndex, value );
        obj->value[1] = value;
        obj->level = 1;
        obj->cost = value;
        obj->item_type = ITEM_XPTOKEN;
        obj_to_char(obj,ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        if (obj->questowner != NULL) free_string(obj->questowner);
        obj->questowner = str_dup(ch->pcdata->switchname);
        free_string( obj->name );
        obj->name = str_dup( "exp token" );
        sprintf(buf,"a %d exp token",value);
        free_string( obj->short_descr );
        obj->short_descr = str_dup( buf );
        sprintf(buf,"A %d exp token lies on the floor.",value);
        free_string( obj->description );
        obj->description = str_dup( buf );
        if (victim != NULL && victim != ch)
        {
            act( "You reach into $N's ear and produce $p.", ch, obj, victim, TO_CHAR );
            act( "$n reaches into $N's ear and produces $p.", ch, obj, victim, TO_NOTVICT );
            act( "$n reaches into your ear and produces $p.", ch, obj, victim, TO_VICT );
        }
        else
        {
            act( "You pick your nose and reveal $p.", ch, obj, NULL, TO_CHAR );
            act( "$n picks $s nose and reveals $p.", ch, obj, NULL, TO_ROOM );
        }
        return;
    }
    else if (!str_cmp(arg, "primal"))
    {
        if ( ch->practice <= 0 ) return;
        value = atoi(arg2);

        if (((value = atoi(arg2)) < 1 || value > 1000000 ) && str_cmp(arg2,"all"))
        {
            send_to_char("From 1 to 1,000,000 primals please.\n\r",ch);
            return;
        }
        if (!is_number(arg2) && str_cmp(arg2,"all"))
        {
            send_to_char("Please enter a numeric value.\n\r",ch);
            return;
        }
        if (value > ch->practice)
        {
            sprintf(non,"You only have %d left to put into a primal token.\n\r",ch->practice);
            send_to_char(non,ch);
            return;
        }
        if ( !str_cmp(arg2,"all" ) ) value = ch->practice;
        victim = get_char_room( ch, arg2 );

        if ( ( pObjIndex = get_obj_index( OBJ_VNUM_PROTOPLASM ) ) == NULL )
        {
            send_to_char( "Error...missing object, please inform Trace.\n\r",ch );
            return;
        }

        ch->practice -= value;
        if (ch->practice < 0) ch->practice = 0;
        obj = create_object( pObjIndex, value );
        obj->value[2] = value;
        obj->level = 1;
        obj->cost = value;
        obj->item_type = ITEM_PRTOKEN;
        obj_to_char(obj,ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        free_string( obj->name );
        obj->name = str_dup( "primal token" );
        sprintf(buf,"a %d primal token",value);
        free_string( obj->short_descr );
        obj->short_descr = str_dup( buf );
        sprintf(buf,"A %d primal token lies on the floor.",value);
        free_string( obj->description );
        obj->description = str_dup( buf );
        if (victim != NULL && victim != ch)
        {
            act( "You blow into $N's ear and produce $p.", ch, obj, victim, TO_CHAR );
            act( "$n blows into $N's ear and produces $p.", ch, obj, victim, TO_NOTVICT );
            act( "$n blows into your ear and produces $p.", ch, obj, victim, TO_VICT );
        }
        else
        {
            act( "You fart reveal $p.", ch, obj, NULL, TO_CHAR );
            act( "$n farts and out $s butt comes $p.", ch, obj, NULL, TO_ROOM );
        }
        return;
    }
    else if (!str_cmp(arg, "store"))
    {
        if ( ch->pcdata->store <= 0 ) return;
        value = atoi(arg2);
        if (((value = atoi(arg2)) < 1 || value > 1000000) && str_cmp(arg2,"all"))
        {
            send_to_char("From 1 to 1,000,000 stores please.\n\r",ch);
            return;
        }
        if (!is_number(arg2) && str_cmp(arg2,"all"))
        {
            send_to_char("Please enter a numeric value.\n\r",ch);
            return;
        }
        if (value > ch->pcdata->store && ch->level < 10)
        {
            sprintf(non,"You only have %d left to put into a store token.\n\r",ch->pcdata->store);
            send_to_char(non,ch);
            return;
        }
        if ( !str_cmp(arg2,"all" ) ) value = ch->pcdata->store;
        victim = get_char_room( ch, arg2 );

        if ( ( pObjIndex = get_obj_index( OBJ_VNUM_PROTOPLASM ) ) == NULL )
        {
            send_to_char( "Error...missing object, please inform Trace.\n\r",ch );
            return;
        }

        ch->pcdata->store -= value;
        if (ch->pcdata->store < 0) ch->pcdata->store = 0;
        obj = create_object( pObjIndex, value );
        obj->value[1] = value;
        obj->level = 1;
        obj->cost = value;
        obj->item_type = ITEM_STORETOKEN;
        obj_to_char(obj,ch);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        if (obj->questowner != NULL) free_string(obj->questowner);
        obj->questowner = str_dup(ch->pcdata->switchname);
        free_string( obj->name );
        obj->name = str_dup( "store token" );
        sprintf(buf,"a %d store token",value);
        free_string( obj->short_descr );
        obj->short_descr = str_dup( buf );
        sprintf(buf,"A %d store token lies on the floor.",value);
        free_string( obj->description );
        obj->description = str_dup( buf );
        if (victim != NULL && victim != ch)
        {
            act( "You sneeze onto $N's face and out flies $p.", ch, obj, victim, TO_CHAR );
            act( "$n sneezes onto $N's face and out flies $p.", ch, obj, victim, TO_NOTVICT );
            act( "$n sneezes onto your face and out comes $p.", ch, obj, victim, TO_VICT );
        }
        else
        {
            act( "You sneeze and $p flies out.", ch, obj, NULL, TO_CHAR );
            act( "$n sneezes and $p flies out.", ch, obj, NULL, TO_ROOM );
        }
        return;
    }
    else if (!str_cmp(arg, "quest"))
    {
        if ( ch->pcdata->quest <= 0 && !str_cmp(arg2,"all")) return;
        value = atoi(arg2);

        if (((value = atoi(arg2)) < 1 || value > 10000000 ) && str_cmp(arg2,"all"))
        {
            send_to_char("From 1 to 10,000,000 quest points please.\n\r",ch);
            return;
        }
        if (!is_number(arg2) && str_cmp(arg2,"all"))
        {
            send_to_char("Please enter a numeric value.\n\r",ch);
            return;
        }
        else if ( value > ch->pcdata->quest && !IS_QUESTMAKER(ch) && ch->pcdata->security < 2)
        {
            sprintf(non,"You only have %d left to put into a quest token.\n\r",ch->pcdata->quest);
            send_to_char(non,ch);
            return;
        }
        if ( !str_cmp(arg2,"all" ) ) value = ch->pcdata->quest;
        victim = get_char_room( ch, arg2 );

        if ( ( pObjIndex = get_obj_index( OBJ_VNUM_PROTOPLASM ) ) == NULL )
        {
            send_to_char( "Error...missing object, please inform Trace.\n\r",ch );
            return;
        }

        ch->pcdata->quest -= value;
        if (ch->pcdata->quest < 0) ch->pcdata->quest = 0;
        obj = create_object( pObjIndex, value );
        obj->value[0] = value;
        obj->level = 1;
        obj->cost = value*1000;
        obj->item_type = ITEM_QUEST;
        obj_to_char(obj,ch);
        obj->questowner = str_dup(ch->pcdata->switchname);
        if (obj->questmaker != NULL) free_string(obj->questmaker);
        obj->questmaker = str_dup(ch->name);
        if (obj->questowner != NULL) free_string(obj->questowner);
        obj->questowner = str_dup(ch->pcdata->switchname);
        free_string( obj->name );
        obj->name = str_dup( "quest token" );
        sprintf(buf,"a %d point quest token",value);
        free_string( obj->short_descr );
        obj->short_descr = str_dup( buf );
        sprintf(buf,"A %d point quest token lies on the floor.",value);
        free_string( obj->description );
        obj->description = str_dup( buf );
        if (victim != NULL && victim != ch)
        {
            act( "You reach behind $N's ear and produce $p.", ch, obj, victim, TO_CHAR );
            act( "$n reaches behind $N's ear and produces $p.", ch, obj, victim, TO_NOTVICT );
            act( "$n reaches behind your ear and produces $p.", ch, obj, victim, TO_VICT );
        }
        else
        {
            act( "You snap your fingers and reveal $p.", ch, obj, NULL, TO_CHAR );
            act( "$n snaps $s fingers and reveals $p.", ch, obj, NULL, TO_ROOM );
        }
        return;
    }
    else
    {
        do_token(ch,"");
    }
    return;
}

void do_mdice( CHAR_DATA *ch, char *argument)

{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int value;
    int value2;
    int total;
    int total2;
    int totald;
    totald = 0;
    value = 3;
    value2 = 1;
    total = 0;
    total2 = 0;
    argument = one_argument( argument, arg2 );
    value2 = atoi( arg2 );
    argument =one_argument( argument, arg1 );
    value = atoi( arg1 );
    if ( IS_NPC(ch))
    {
        send_to_char("Mobs dont have a mind good enough to play.\n\r",ch);
        return;
    }

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        send_to_char("Please specify a value for the amount of sides.\n\r",ch);
        return;
    }

    if ( value < 3 || value > 10000 )
    {
        send_to_char("No dice can less than 3 sides, or more than 10,000 sides.\n\r",ch);
        return;
    }
    if(value2 <1 || value2>20)
    {
        send_to_char("You cannot roll less than 1 dice, or more than 20 dice at a time.\n\r",ch);
        return;
    }
    sprintf(buf,"#R%s is rolling %d dice.\n\r#n",ch->name, value2);
    act(buf,ch,NULL,NULL,TO_ROOM);
    send_to_char(buf,ch);
    totald = value2;
    for(value2=value2;value2>0;value2--)
    {
        total=number_range(1,value);
        sprintf(buf,"#R%s rolled a dice onto a %d off of a %d sided dice.\n\r#n",ch->name, total, value);
        total2+=total;
        act(buf,ch,NULL,NULL,TO_ROOM);
        send_to_char(buf,ch);
    }
    sprintf(buf,"#R%s has rolled %d dice for a total of %d.\n\r#n",ch->name, totald,total2);
    act(buf,ch,NULL,NULL,TO_ROOM);
    send_to_char(buf,ch);

    if (ch->level < 7 )
    {
        WAIT_STATE(ch,5);
    }
    return;
}

void do_dice( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];
    int value;
    value = 3;

    argument = one_argument( argument, arg1 );
    value = atoi( arg1 );

    if ( IS_NPC(ch))
    {
        send_to_char("Mobs dont have a mind good enough to play.\n\r",ch);
        return;
    }

    if ( arg1[0] == '\0' || !is_number( arg1 ) )
    {
        send_to_char("Please specify a value for the amount of sides.\n\r",ch);
        return;
    }

    if ( value < 3 || value > 10000 )
    {
        send_to_char("No dice can less than 3 sides, or more than 10,000 sides.\n\r",ch);
        return;
    }
    sprintf(buf,"#R%s rolled a dice onto a %d off of a %d sided dice.\n\r#n",ch->name, number_range(1,value), value);
    act(buf,ch,NULL,NULL,TO_ROOM);
    send_to_char(buf,ch);
    if (ch->level < 7 )
    {
        WAIT_STATE(ch,2);
    }
    return;
}

void do_dddamage( CHAR_DATA *ch, char *argument )
{
    char buf[MSL];

    if ( IS_NPC(ch))
    {
        send_to_char("How can a mob play a game?\n\r",ch);
        return;
    }

    if( !IS_SET(ch->in_room->room_flags, ROOM_DD) )
    {
        send_to_char("Not in the proper D&D Room.\n\r",ch);
        return;
    }

    sprintf(buf,"%s does %d damage.\n\r",ch->name, number_range(1,20));
    act(buf,ch,NULL,NULL,TO_ROOM);
    send_to_char(buf,ch);
    return;
}

void do_pkreset( CHAR_DATA *ch, char *argument )
{

    if ( IS_NPC(ch))
    {
        send_to_char("How can a mob have Pkills?\n\r",ch);
        return;
    }

    /*    if ( ch->pkill > ch->pdeath )
        {
        send_to_char("You wouldn't want to waste your pk record do you? Use Convert.\n\r",ch);
        return;
        }*/

    if ( ch->pkill == 0 && ch->pdeath == 0)
    {
        send_to_char("Pkrecord already reset.\n\r",ch);
        return;
    }
    if ( (ch->pdeath-5) >= ch->pkill )
    {
        stcf(ch,"Free pkreset for you.\n\r");
        ch->pdeath = 0;
        ch->pkill = 0;
        return;
    }
    /*    if ( ch->pcdata->quest < ((ch->pdeath-ch->pkill)*5000)+1000)
        {
        sprintf(buf,"You need %d Qps, You only have %d.\n\r",((ch->pdeath-ch->pkill)*5000)+1000, ch->pcdata->quest );
        send_to_char(buf,ch);
        return;
        }*/
    if ( ch->pcdata->quest < 10000 ) return stcf(ch,"You need 10000 qps to pkreset.\n\r");

    ch->pcdata->quest -= 10000;
    ch->pkill = 0;
    ch->pdeath = 0;
    send_to_char("Done, Your Pk Record has been wiped.\n\r",ch);
    while(downgrade(ch));                                   // See bool downgrade() in upgrade.c
    return;
}

void do_reroll( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
    {
        send_to_char("Mobiles don't know how to use reroll.\n\r",ch);
        return;
    }

    if( !IS_SET(ch->in_room->room_flags, ROOM_DD) )
    {
        send_to_char("Not in the proper D&D Room.\n\r",ch);
        return;
    }

    sprintf(buf, "Cha:%d. Wis:%d. Con:%d. Dex:%d. Str:%d. Int:%d..\n\r",
        number_range(3,20), number_range(3,20), number_range(3,20),
        number_range(3,20), number_range(3,20), number_range(3,20) );
    send_to_char(buf,ch);
    act(buf,ch,NULL,NULL,TO_ROOM);
    return;
}

void do_avatar( CHAR_DATA *ch, char *argument )
{
    if ( IS_NPC( ch ) ) return;
    if ( (ch->level < 10) && (ch->trust < 10)) return;
    if ( ch->trust > ch->level )
    {
        if ( ch->trust >= 10 )
        {
            ch->level = ch->trust;
            send_to_char( " You have been set back to your immortal level.\n\r",ch );
            return;
        }
        return;
    }

    ch->level = 3;
    send_to_char( " You have been set to level avatar.\n\r",ch );
    return;
}

void do_updatemud()
{
    pid_t pid = -1;
    FILE *fp;

    if ((fp = fopen("../src/mud.txt","w")) == NULL)
    {
        bug ( "error opening mud.txt file\n\r", 0 );
        return;
    }
    pid = getpid();
    fprintf(fp, "%d", pid);
    fclose(fp);
    return;
}

void do_qtrust( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Syntax: qtrust <char> <on/off>.\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' )
    {
        send_to_char( "Do you wish to set qtrust ON or OFF?\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }

    if (str_cmp(arg2,"on") && str_cmp(arg2,"off"))
    {
        send_to_char( "Do you want to set their qtrust ON or OFF?\n\r", ch);
        return;
    }
    if (!str_cmp(arg2,"off"))
    {
        if (!IS_EXTRA(victim, EXTRA_TRUSTED))
        {
            send_to_char("Their qtrust is already off.\n\r",ch);
            return;
        }
        REMOVE_BIT(victim->extra, EXTRA_TRUSTED);
        send_to_char("Quest trust OFF.\n\r",ch);
        send_to_char("You are no longer quest trusted.\n\r",victim);
        return;
    }
    else if (!str_cmp(arg2,"on"))
    {
        if (IS_EXTRA(victim, EXTRA_TRUSTED))
        {
            send_to_char("Their qtrust is already on.\n\r",ch);
            return;
        }
        SET_BIT(victim->extra, EXTRA_TRUSTED);
        send_to_char("Quest trust ON.\n\r",ch);
        send_to_char("You are now quest trusted.\n\r",victim);
        return;
    }
    return;
}

void do_clearstats( CHAR_DATA *ch, char *argument )
{
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;
    if (IS_NPC(ch)) return;

    if ((ch->in_room == NULL || ch->in_room->vnum != 3054))
    {
        send_to_char("You can only clear your stats at the Temple Altar of Midgaard.\n\r",ch);
        return;
    }

    powerdown(ch);                                          /* remove class shit - Jobo */

    for ( obj = ch->carrying; obj != NULL; obj = obj_next )
    {
        obj_next = obj->next_content;
        if ( obj->wear_loc != WEAR_NONE )
            {obj_from_char(obj); obj_to_char(obj,ch);}
    }

    while ( ch->affected )
        affect_remove( ch, ch->affected );

    if (IS_SET(ch->affected_by, AFF_POLYMORPH)) REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
    if (IS_SET(ch->affected_by, AFF_ETHEREAL)) REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
    if (IS_EXTRA(ch,EXTRA_DRAGON)) REMOVE_BIT(ch->extra, EXTRA_DRAGON);
    if (IS_VAMPAFF(ch, VAM_CHANGED) && IS_POLYAFF(ch, POLY_WOLF))  REMOVE_BIT(ch->polyaff, POLY_WOLF);

    ch->affected_by  = 0;
    ch->armor        = 100;
    ch->hit      = UMAX( 1, ch->hit  );
    ch->mana         = UMAX( 1, ch->mana );
    ch->move         = UMAX( 1, ch->move );
    ch->hitroll      = 0;
    ch->damroll      = 0;
    ch->rage         = 0;
    ch->form         = 1048575;
    ch->saving_throw     = 0;
    ch->pcdata->mod_str  = 0;
    ch->pcdata->mod_int  = 0;
    ch->pcdata->mod_wis  = 0;
    ch->pcdata->mod_dex  = 0;
    ch->pcdata->mod_con  = 0;
    ch->pcdata->followers = 0;
    save_char_obj( ch );
    send_to_char("Your stats have been cleared.  Please rewear your equipment.\n\r",ch);
    return;
}

void do_otransfer( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *chroom;
    ROOM_INDEX_DATA *objroom;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' )
    {
        send_to_char( "Otransfer which object?\n\r", ch );
        return;
    }

    if ( arg2[0] == '\0' ) victim = ch;
    else if ( ( victim = get_char_world( ch, arg2 ) ) == NULL )
    {
        send_to_char( "They aren't here.\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL )
    {
        send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
        return;
    }

    if (!IS_JUDGE(ch) && (obj->questmaker == NULL ||
        str_cmp(ch->name,obj->questmaker) || strlen(obj->questmaker) < 2))
    {
        send_to_char("You don't have permission to otransfer that item.\n\r", ch);
        return;
    }

    if (obj->carried_by != NULL)
    {
        act("$p vanishes from your hands in an explosion of energy.",obj->carried_by,obj,NULL,TO_CHAR);
        act("$p vanishes from $n's hands in an explosion of energy.",obj->carried_by,obj,NULL,TO_ROOM);
        obj_from_char(obj);
    }
    else if (obj->in_obj     != NULL) obj_from_obj(obj);
    else if (obj->in_room != NULL)
    {
        chroom = ch->in_room;
        objroom = obj->in_room;
        char_from_room(ch);
        char_to_room(ch,objroom);
        act("$p vanishes from the ground in an explosion of energy.",ch,obj,NULL,TO_ROOM);
        if (chroom == objroom) act("$p vanishes from the ground in an explosion of energy.",ch,obj,NULL,TO_CHAR);
        char_from_room(ch);
        char_to_room(ch,chroom);
        obj_from_room(obj);
    }
    else
    {
        send_to_char( "You were unable to get it.\n\r", ch );
        return;
    }
    obj_to_char(obj,victim);
    act("$p appears in your hands in an explosion of energy.",victim,obj,NULL,TO_CHAR);
    act("$p appears in $n's hands in an explosion of energy.",victim,obj,NULL,TO_ROOM);
    return;
}

void bind_char( CHAR_DATA *ch )
{
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    ROOM_INDEX_DATA *location;

    if ( IS_NPC(ch) || ch->pcdata->obj_vnum < 1 )
        return;

    if ( ( pObjIndex = get_obj_index( ch->pcdata->obj_vnum ) ) == NULL )
        return;

    if ( ch->in_room == NULL || ch->in_room->vnum == ROOM_VNUM_IN_OBJECT )
    {
        location = get_room_index(ROOM_VNUM_ALTAR);
        char_from_room(ch);
        char_to_room(ch,location);
    }
    else location = ch->in_room;

    obj = create_object( pObjIndex, 50 );
    obj_to_room(obj,location);
    obj->chobj = ch;
    ch->pcdata->chobj = obj;
    SET_BIT(ch->affected_by, AFF_POLYMORPH);
    SET_BIT(ch->extra, EXTRA_OSWITCH);
    free_string(ch->morph);
    ch->morph = str_dup(obj->short_descr);
    send_to_char( "You reform yourself.\n\r", ch );
    act("$p fades into existance on the floor.",ch,obj,NULL,TO_ROOM);
    do_look(ch,"auto");
    return;
}

void do_bind( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        send_to_char( "Syntax: bind <player> <object>\n\r", ch );
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
    {
        send_to_char( "That player is not here.\n\r", ch);
        return;
    }
    if (ch == victim)
    {
        send_to_char( "You can't do this to yourself.\n\r", ch);
        return;
    }
    if (IS_NPC(victim))
    {
        send_to_char( "Not on NPC's.\n\r", ch);
        return;
    }
    else if (IS_AFFECTED(victim,AFF_POLYMORPH))
    {
        send_to_char( "You cannot do this while they are polymorphed.\n\r", ch);
        return;
    }
    else if (IS_IMMORTAL(victim))
    {
        send_to_char( "Only on mortals or avatars.\n\r", ch);
        return;
    }
    if ( ( obj = get_obj_carry( ch, arg2) ) == NULL )
    {
        send_to_char("You are not carrying that item.\n\r",ch);
        return;
    }
    if (obj->chobj != NULL)
    {
        send_to_char( "That item already has someone bound in it.\n\r", ch);
        return;
    }
    send_to_char("Ok.\n\r",ch);
    act("$n transforms into a white vapour and pours into $p.",victim,obj,NULL,TO_ROOM);
    act("You transform into a white vapour and pour into $p.",victim,obj,NULL,TO_CHAR);
    victim->pcdata->obj_vnum = obj->pIndexData->vnum;
    obj->chobj = victim;
    victim->pcdata->chobj = obj;
    SET_BIT(victim->affected_by, AFF_POLYMORPH);
    SET_BIT(victim->extra, EXTRA_OSWITCH);
    free_string(victim->morph);
    victim->morph = str_dup(obj->short_descr);
    return;
}

void do_release( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Syntax: release <object>\n\r", ch );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg) ) == NULL )
    {
        send_to_char("You are not carrying that item.\n\r",ch);
        return;
    }
    if ( (victim = obj->chobj) == NULL)
    {
        send_to_char( "There is nobody bound in that item.\n\r", ch);
        return;
    }
    send_to_char("Ok.\n\r",ch);
    victim->pcdata->obj_vnum = 0;
    obj->chobj = NULL;
    victim->pcdata->chobj = NULL;
    REMOVE_BIT(victim->affected_by, AFF_POLYMORPH);
    REMOVE_BIT(victim->extra, EXTRA_OSWITCH);
    free_string(victim->morph);
    victim->morph = str_dup("");
    act("A white vapour pours out of $p and forms into $n.",victim,obj,NULL,TO_ROOM);
    act("Your spirit floats out of $p and reforms its body.",victim,obj,NULL,TO_CHAR);
    return;
}

void do_contraception( CHAR_DATA *ch, char *argument)
{
    if(IS_NPC(ch)) return;
    if( ch->sex != SEX_FEMALE) return;
    if( IS_SET(ch->affected_by2, AFF_CONTRACEPTION))
    {
        stc( "You remove your contraception.\n\r", ch);
        REMOVE_BIT(ch->affected_by2, AFF_CONTRACEPTION);
        return;
    }
    else
    {
        stc( "You use your contraception.\n\r", ch);
        SET_BIT(ch->affected_by2, AFF_CONTRACEPTION);
        return;
    }
    return;
}

/*----------------------------------------------------------------
 * functions for RELOAD command.  --Zarniwoop@Dutch Mountains  (4-28-97)
 * Snippet intended for Merc 2.0, 2.1 and 2.2
 *
 * ready to go, except for two things:
 * 1. make the usual modifications for do_reload (merc.h, interp.c)
 * 2. in merc.h, after the declaration of help_first, declare help_last
 *    so it is global.
 *
 * Bugs, comments: remmelt@kosterix.icce.rug.nl
 */
bool free_helps( CHAR_DATA *ch, char *arg )
{
    char buf[MAX_STRING_LENGTH];
    HELP_DATA *h, *h_next;
    HELP_DATA *prev = NULL;
    bool found = FALSE;

    prev = help_first;
    for ( h = help_first; h; h = h_next )
    {
        h_next = h->next;

        if ( !str_cmp( h->keyword, arg ) )
        {
            free_string( h->keyword );
            free_string( h->text );
            if ( h == help_first )
                help_first = h->next;
            else
                prev->next = h->next;
            if ( h == help_last )
                help_last = prev;
            free_mem( h, sizeof(*h) );
            found = TRUE;
        }
        prev = h;
    }

    if ( !found )
    {
        sprintf( buf, "Help entry %s not found.\n\r", arg );
        send_to_char( buf, ch );
        return FALSE;
    }

    return TRUE;
}

bool read_entry( CHAR_DATA *ch, FILE *fp, char *filename, char *arg )
{
    char buf[MAX_STRING_LENGTH];
    HELP_DATA *new;
    HELP_DATA *debug;
    char *test_keyword = 0;
    char *test_text = 0;
    int test_level = 0;

    if ( fread_letter( fp ) != '#' )
    {
        sprintf( buf, "read_entry: # not in %s.\n\r", filename );
        send_to_char( buf, ch );
        return FALSE;
    }

    fread_word( fp );                                       /* read the word HELPS */

    new      = alloc_mem( sizeof(*new) );
    new->next    = NULL;

    for ( ; ; )
    {
        test_level   = fread_number( fp );
        test_keyword = fread_string( fp );

        if ( !str_cmp( test_keyword, "$" ))                 /* end of file */
        {
            free_string( test_keyword );
            free_string( test_text );
            free_mem( new, sizeof(*new) );
            return FALSE;
        }

        test_text    = fread_string( fp );

        if ( !str_cmp( test_keyword, arg ))                 /* match */
        {
            new->keyword = test_keyword;
            new->level   = test_level;
            new->text    = test_text;

            debug = help_last;
            if ( help_last )
                help_last->next = new;
            help_last = new;

            sprintf( buf, "entry %s updated.\n\r", new->keyword );
            send_to_char( buf, ch );
            return TRUE;
        }
        else
        {
            free_string( test_keyword );
            free_string( test_text );
        }
    }

    return FALSE;
}

void do_hreload( CHAR_DATA *ch, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    FILE *fp;
    bool found = FALSE;
    int i;

    /* in order of 'importance' */
    char *helpfile[0] =
    {
    };

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        send_to_char( "Usage: reload <help-keyword>\n\r", ch );
        return;
    }

    /* remove help entry from the list */
    if ( !free_helps( ch, arg ))
        return;

    /* here we go */
    fclose( fpReserve );
    for ( i = 0; i < 9; i++ )
    {
        if ( ( fp = fopen( helpfile[i], "r" ) ) == NULL )
        {
            sprintf( buf, "do_reload: couldn't open %s.\n\r", helpfile[i] );
            send_to_char( buf, ch );
        }
        else
        {
            if ( ( found = read_entry( ch, fp, helpfile[i], arg ) ) == TRUE )
            {
                fclose( fp );
                break;
            }
            fclose( fp );
        }
    }
    fpReserve = fopen( NULL_FILE, "r" );
    return;
}

void do_resetpassword( CHAR_DATA *ch, char *argument )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    char *pwdnew;

    if ( IS_NPC(ch) )
        return;

    argument=one_argument( argument, arg1 );
    argument=one_argument( argument, arg2 );

    victim = get_char_world(ch, arg1);

    if (  ( ch->pcdata->pwd != '\0' )
        && ( arg1[0] == '\0' || arg2[0] == '\0')  )
    {
        send_to_char( "Syntax: password <char> <new>.\n\r", ch );
        return;
    }
    if( victim == '\0' )
    {
        send_to_char( "That person isn't here, they have to be here to reset pwd's.\n\r", ch);
        return;
    }
    if ( IS_NPC( victim ) )
    {
        send_to_char( "You cannot change the password of NPCs!\n\r",ch);
        return;
    }

    /*    if (( victim->level > LEVEL_IMMORTAL ) || ( get_trust(victim) > LEVEL_IMMORTAL ))

        {
            send_to_char( "You can't change the password of that player.\n\r",ch);
            return;
        }*/

    if ( strlen(arg2) < 5 )
    {
        send_to_char( "New password must be at least five characters long.\n\r", ch );
        return;
    }

    pwdnew = crypt( arg2, victim->name );
    free_string( victim->pcdata->pwd );
    victim->pcdata->pwd = str_dup( pwdnew );
    save_char_obj( victim );
    send_to_char( "Ok.\n\r", ch );
    sprintf( buf, "Your password has been changed to %s.", arg2 );
    send_to_char( buf, victim);
    return;
}

#define CH(descriptor)  ((descriptor)->original ? \
    (descriptor)->original : (descriptor)->character) \

/* This file holds the copyover data */
#define COPYOVER_FILE "copyover.data"

/* This is the executable file */
#define EXE_FILE      "../src/dystopia"

extern int port,control;                                    /* db.c */

void do_copyover (CHAR_DATA *ch, char * argument)
{
    FILE *fp;
    CHAR_DATA *gch;
    DESCRIPTOR_DATA *d, *d_next;
    extern int port,control;                                /* db.c */
    char buf[100], buf2[100], buf3[100];
	bool silent = FALSE;
	bool msg = FALSE;
	int cust = 0;

	if ( !str_cmp(argument,"silent") )
	{
		silent = TRUE;
	}
	else if ( !str_cmp(argument,"matrix") )
	{
		cust = 1;
		silent = TRUE;
		msg = TRUE;
	}
	else if ( argument[0] != '\0' )
	{
		msg = TRUE;
		silent = TRUE;
	}
	
    fp = fopen (COPYOVER_FILE, "w");

    if (!fp)
    {
        send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
        Logf ("Could not write to copyover file: %s", COPYOVER_FILE);
        perror ("do_copyover:fopen");
        return;
    }

    do_restore(ch,"all");
    do_force(ch,"all stand");
    do_force(ch,"all call all");
    do_force(ch,"all save");
    do_asave(ch,"changed");
    save_changes();

    /* Headform off - hmm, work on this*/

    for (gch = char_list; gch != NULL; gch = gch->next)
    {
        bool afk = FALSE;

        if (IS_NPC(gch)) continue;

        /*    if (!IS_NPC(gch) && (IS_HEAD(gch,LOST_HEAD) || IS_SET(gch->extra, EXTRA_OSWITCH)))
            {
              if (IS_HEAD(gch,LOST_HEAD)) REMOVE_BIT(gch->loc_hp[0],LOST_HEAD);
              REMOVE_BIT(gch->affected_by,AFF_POLYMORPH);
              if (IS_SET(gch->extra, EXTRA_OSWITCH)) REMOVE_BIT(gch->extra,EXTRA_OSWITCH);
        //	free_string(gch->morph); // not threadsafe.
              gch->morph = str_dup("");
              if (gch->pcdata->chobj != NULL) gch->pcdata->chobj = NULL;
              if (gch->pcdata->obj_vnum != 0) gch->pcdata->obj_vnum = 0;
              char_from_room(gch);
              char_to_room(gch,get_room_index(ROOM_VNUM_ALTAR));
            }*/

        if (IS_SET(gch->extra, EXTRA_AFK))
            afk = TRUE;

        gch->name = str_dup(gch->pcdata->switchname);
        if (IS_HEAD(gch, LOST_HEAD) || IS_SET(gch->extra, EXTRA_OSWITCH))
        {
            REMOVE_BIT(gch->loc_hp[0], LOST_HEAD);
            REMOVE_BIT(gch->affected_by,AFF_POLYMORPH);
            REMOVE_BIT(gch->extra, EXTRA_OSWITCH);
            gch->morph = str_dup("");
            gch->pcdata->chobj = NULL;
            gch->pcdata->obj_vnum = 0;
            char_from_room(gch);
            char_to_room(gch,get_room_index(ROOM_VNUM_ALTAR));
        }
        if (afk)
            SET_BIT(gch->extra, EXTRA_AFK);

    }

	if ( silent )
		sysdata.silent_reboot = TRUE;
	else
		sysdata.silent_reboot = FALSE;
	save_sysdata();

	if ( cust == 1 )
		sprintf( buf, "\n\rYou notice a black cat walking past... And then another one, just like it!\n\rTrinity faces you and says, \"It's a glitch in the Matrix, it happens when they change something!\"\n\r" );
	else if ( msg )
		sprintf( buf, "\n\r%s\n\r", argument );
    /* Have to disable compression when doing a copyover */

    for (d = descriptor_list; d ; d = d->next)
    {
        if (d->lookup_status != STATUS_DONE) continue;
        if (d->character != NULL) gch = d->character;
        else continue;
        if (gch->desc->out_compress)
        {
            if (!compressEnd2(gch->desc)) send_to_char("Could not disable compression, you'll have to reconect in 5 seconds.\n", gch);
        }
    }

    /* For each playing descriptor, save its state */
    for (d = descriptor_list; d ; d = d_next)
    {
        CHAR_DATA * och = CH (d);
        d_next = d->next;                                   /* We delete from the list , so need to save this */

        if (!d->character || d->connected != 0)             /* drop those logging on */
        {
            write_to_descriptor_2(d->descriptor, "\n\rSorry, we are rebooting. Come back in 30 seconds.\n\r", 0);
            close_socket (d);                               /* throw'em out */
        }
        else
        {
            fprintf (fp, "%d %s %s\n", d->descriptor, och->name, d->host);
            if (och->level == 1)
            {
                write_to_descriptor(d, "Since you are level one, and level one characters do not save, you gain a free level!\n\r", 0);
                och->level++;                               /* Advance_level doesn't do that */
            }
            save_char_obj (och);
            write_to_descriptor(d, buf, 0);
        }
    }

    fprintf (fp, "-1\n");
    fclose (fp);

    /* Close reserve and other always-open files and release other resources */

    fclose (fpReserve);

    /* recycle descriptors */
    recycle_descriptors();

    #ifdef IMC
    imc_hotboot();
    #endif
    /* exec - descriptors are inherited */

    sprintf (buf, "%d", port);
    sprintf (buf2, "%d", control);
    #ifdef IMC
    if( this_imcmud )
        snprintf( buf3, 100, "%d", this_imcmud->desc );
    else
        strncpy( buf3, "-1", 100 );
    #else
    strncpy( buf3, "-1", 100 );
    #endif

    //	execl (EXE_FILE, "dystopia", buf, "copyover", buf2, (char *) NULL);
    execl (EXE_FILE, "dystopia", buf, "copyover", buf2, buf3, (char *)NULL );

    /* Failed - sucessful exec will not return */

    perror ("do_copyover: execl");
    send_to_char ("Copyover FAILED!\n\r",ch);

    /* Here you might want to reopen fpReserve */
}

/* Recover from a copyover - load players */
void copyover_recover ()
{
    DESCRIPTOR_DATA *d;
    FILE *fp;
    char name [100];
    char host[MSL];
    int desc;
    bool fOld;

    Logf ("Copyover recovery initiated");

    fp = fopen (COPYOVER_FILE, "r");

    if (!fp)                                                /* there are some descriptors open which will hang forever then ? */
    {
        perror ("copyover_recover:fopen");
        Logf ("Copyover file not found. Exitting.\n\r");
        exit (1);
    }

    unlink (COPYOVER_FILE);                                 /* In case something crashes - doesn't prevent reading	*/

    for (;;)
    {
        fscanf (fp, "%d %s %s\n", &desc, name, host);
        if (desc == -1)
            break;

        /* Write something, and check if it goes error-free */
        if (!write_to_descriptor_2(desc, "\n\r",0))
        {
            close (desc);                                   /* nope */
            continue;
        }

        d = alloc_perm (sizeof(DESCRIPTOR_DATA));
        init_descriptor (d,desc);                           /* set up various stuff */

        players_logged++;

        d->host = str_dup (host);
        d->next = descriptor_list;
        descriptor_list = d;
        d->connected = CON_COPYOVER_RECOVER;                /* -15, so close_socket frees the char */

        /* Now, find the pfile */

        fOld = load_char_obj (d, name);

        if (!fOld)                                          /* Player file not found?! */
        {
            write_to_descriptor_2(desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
            close_socket (d);
        }
        else                                                /* ok! */
        {
            write_to_descriptor_2(desc, "\n\r",0);

            /* Just In Case */
            if (!d->character->in_room)
                d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

            /* Insert in the char_list */
            d->character->next = char_list;
            char_list = d->character;

            char_to_room (d->character, d->character->in_room);
	if (sysdata.silent_reboot != 1)
	{
            do_look (d->character, "");
            act ("$n materializes!", d->character, NULL, NULL, TO_ROOM);
	}
            d->connected = CON_PLAYING;
            d->lookup_status = STATUS_DONE;
            retell_mccp(d);
        }

    }

    fclose (fp);
}

void do_implag(CHAR_DATA *ch, char *argument)
{
    char      arg[MAX_INPUT_LENGTH];
    char      buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument(argument, arg);

    if (arg[0] == '\0')
    {
        send_to_char("Who you wanna lag?\n\r", ch);
        return;
    }

    if ((victim = get_char_world( ch, arg ) )==NULL )
    {
        send_to_char("They aren't playing you dufus!\n\r", ch);
        return;
    }
    else
    {
        if (IS_NPC(victim))
        {
            send_to_char("Not on NPC's!\n\r", ch);
            return;
        }
        else if (IS_AFF2(victim, PLR_IMPLAG))
        {
            REMOVE_BIT(victim->affected_by2, PLR_IMPLAG);
            sprintf(buf, "You make %s's life better.\n\r", victim->name);
            send_to_char(buf, ch);
            sprintf(buf, "You suddenly look at the world normally.\n\r");
            send_to_char(buf, victim);
            return;
        }
        else
        {
            SET_BIT(victim->affected_by2, PLR_IMPLAG);
            sprintf(buf, "You make %s's life totally suck.\n\r", victim->name);
            send_to_char(buf, ch);
            return;
        }
    }
}

void do_doublexp( CHAR_DATA *ch, char *argument )
{
    char arg[MIL];
    char buf[MSL];
    char arg2[MIL];
    int time_on = 0;

    argument = one_argument( argument, arg );
    argument = one_argument(argument, arg2);

    if (arg[0] == '\0')
    {
        send_to_char("Syntax: double <qp|exp> <time>.\n\r",ch);
        return;
    }
    if (!str_cmp(arg, "exp"))
    {
        //    if (!is_number(arg2))
        //    {
        //      send_to_char("A number please, how long should double exp be on??\n\r",ch);
        //      return;
        //    }
        if ((time_on = atoi(arg2)) < -31 || time_on > 6000)
        {
            send_to_char("From 1 to 6000 minutes please.\n\r",ch);
            return;
        }
        global_exp = TRUE;
        pulse_doubleexp = atoi(arg2);
        sprintf( buf, "#GH#Ra#Gp#Rp#Gy #GH#Ro#Gu#Rr #0(#7STARTED#0)x100!!!#n");
        do_info( ch, buf );
        return;
    }
    else if (!str_cmp(arg, "qp"))
    {
        //    if (!is_number(arg2))
        //    {
        //      send_to_char("A number please, how long should double qps be on??\n\r",ch);
        //      return;
        //    }
        if ((time_on = atoi(arg2)) < -3 || time_on > 60)
        {
            send_to_char("From 1 to 60 minutes please.\n\r",ch);
            return;
        }
        global_qp = TRUE;
        pulse_doubleqp = atoi(arg2);
        sprintf( buf, "#GQ#RU#GE#RS#GT#RO#GR#RS #GD#RE#GL#RI#GG#RH#GT #0(#7STARTED#0)#n");
        do_info( ch, buf );
        return;
    }
    else do_doublexp(ch,"");
    return;
}

/*  by Hutoshi */
void do_cprime(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];

    if(IS_NPC(ch)) return;

    do_force(ch, "all wake");
    do_force(ch, "all stand");
    do_force(ch, "all call all");
    //do_force(ch, "all asave changed");
    //do_asave(ch,"changed");
    do_restore(ch, "all");
    do_force(ch, "all distort self");
    do_force(ch, "all save");
    do_asave(ch, "world");
    do_asave(ch, "list");
    do_asave(ch, "changed");
    do_asave(ch, "helps");
    do_save(ch,"auto");
    sprintf(buf, "%s is about to copyover. Kiss your ass goodbye",ch->name);
    do_info(ch, buf);
    stc("#y[#RWorld prepared for copyover.#y]#n\n\r",ch);

    sprintf(buf,"#R::'#G[[[[[[[#R:::'#G[[[[[[[#R::'#G[[[[[[[[#R::'#G[[#R:::'#G[[#R:#n");
    do_info(ch, buf);
    sprintf(buf,"#R:'#G[[#R.... #G[[#R:'#G[[#R.... #G[[#R: #G[[#R.... #G[[#R:. #G[[#R:'#G[[#R::#n");
    do_info(ch, buf);
    sprintf(buf,"#R: #G[[#R::::..:: #G[[#R:::: #G[[#R: #G[[#R:::: #G[[#R::. #G[[[[#R:::#n");
    do_info(ch, buf);
    sprintf(buf,"#R: #G[[#R:::::::: #G[[#R:::: #G[[#R: #G[[[[[[[[#R::::. #G[[#R::::#n");
    do_info(ch, buf);
    sprintf(buf,"#R: #G[[#R:::::::: #G[[#R:::: #G[[#R: #G[[#R.....:::::: #G[[#R::::#n");
    do_info(ch, buf);
    sprintf(buf,"#R: #G[[#R:::: #G[[#R: #G[[#R:::: #G[[#R: #G[[#R::::::::::: #G[[#R::::#n");
    do_info(ch, buf);
    sprintf(buf,"#R:. #G[[[[[[[#R::. #G[[[[[[[#R:: #G[[#R::::::::::: #G[[#R::::#n");
    do_info(ch, buf);
    sprintf(buf,"#R::.......::::.......:::..::::::::::::..:::::#n");
    do_info(ch, buf);
    sprintf(buf,"#R::'#G[[[[[[[#R::'#G[[#R::::'#G[[#R:'#G[[[[[[[[#R:'#G[[[[[[[[#R::#n");
    do_info(ch, buf);
    sprintf(buf,"#R:'#G[[#R.... #G[[#R: #G[[#R:::: #G[[#R: #G[[#R.....:: #G[[#R.... #G[[#R:#n");
    do_info(ch, buf);
    sprintf(buf,"#R: #G[[#R:::: #G[[#R: #G[[#R:::: #G[[#R: #G[[#R::::::: #G[[#R:::: #G[[#R:#n");
    do_info(ch, buf);
    sprintf(buf,"#R: #G[[#R:::: #G[[#R: #G[[#R:::: #G[[#R: #G[[[[[[#R::: #G[[[[[[[[#R::#n");
    do_info(ch, buf);
    sprintf(buf,"#R: #G[[#R:::: #G[[#R::. #G[[ [[#R::: #G[[#R::::::: #G[[#R.. #G[[#R:::#n");
    do_info(ch, buf);
    sprintf(buf,"#R: #G[[#R:::: #G[[#R::. #G[[ [[#R::: #G[[#R::::::: #G[[#R::. #G[[#R::#n");
    do_info(ch, buf);
    sprintf(buf,"#R:. #G[[[[[[[#R::::. #G[[[#R:::: #G[[[[[[[[#R: #G[[#R:::. #G[[#R:#n");
    do_info(ch, buf);
    sprintf(buf,"#R::......::::.......:::..:::::::::..:::::..::#n\n\r");
    do_info(ch, buf);
}

/*Magna Commands*/
void do_ignite(CHAR_DATA * ch, char * argument)
{
    CHAR_DATA * pVictim;
    char pBuf1[MAX_INPUT_LENGTH];

    argument = one_argument(argument, pBuf1);

    pVictim = get_char_room(ch, pBuf1);

    if(pVictim == NULL)
    {
        stc("Ignite whom?\n\r", ch);
        return;
    }
    else
    {
        if(!IS_AFFECTED(pVictim, AFF_FLAMING))
        {
            SET_BIT(pVictim->affected_by, AFF_FLAMING);
            act("$n snaps $s fingers, and $N bursts into flames!", ch, NULL, pVictim, TO_NOTVICT);
            act("You snap your fingers, and $N bursts into flames!", ch, NULL, pVictim, TO_CHAR);
            act("$n snaps $s fingers, and you burst into flames!", ch, NULL, pVictim, TO_VICT);
        }
        else
        {
            act("$E's already on fire, is that really necessary?", ch, NULL, pVictim, TO_CHAR);
        }
    }

    return;
}

void do_remtrans(CHAR_DATA * ch, char * argument)
{
    OBJ_DATA * obj;
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);

    obj = get_obj_carry(ch, arg1);

    if(obj == NULL)
    {
        stc("Cannot find item.\n\r", ch);
        return;
    }

    if(IS_SET(obj->spectype, SITEM_TELEPORTER))
    {
        REMOVE_BIT(obj->spectype, SITEM_TELEPORTER);
        obj->specpower = 0;
        REMOVE_BIT(obj->spectype, SITEM_ACTIVATE);
        stc("Done.\n\r", ch);
    }
    else
    {
        stc("The item is not a transporter.\n\r",ch);
    }

    return;
}

void do_warn(CHAR_DATA * ch, char * argument)
{
    CHAR_DATA * victim;
    char * reason;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg1[MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);

    if(arg1[0] == '\0')
    {
        send_to_char("Syntax: warn <character> <reason>\n\r", ch);
        return;
    }

    if((victim = get_char_world(ch, arg1)) == NULL)
    {
        send_to_char("Can not find that character.\n\r", ch);
        return;
    }

    if(IS_NPC(victim))
    {
        send_to_char("You can not warn NPC's.\n\r", ch);
        return;
    }

    if(IS_IMMORTAL(victim) && (victim->level >= ch->level))
    {
        send_to_char("You can not warn them.\n\r", ch);
        return;
    }

    if(argument[0] == '\0')
    {
        reason = "no reason given";
    }
    else
    {
        reason = argument;
    }

    sprintf(buf, "#rYou have been warned by #R%s#r for #R%s#n\n\r\n\r", ch->name, reason);
    send_to_char(buf, victim);
    sprintf(buf, "#rThis is an official warning and is noted on your record.#n\n\r#rYou have not been punished, however, further infractions may warrant action by the immortals.#n\n\r");
    send_to_char(buf, victim);

    sprintf(buf, "#R%s#r has been given an official warning by #R%s#r.#n\n\r#rReason given: #R%s#n\n\r\n\r#rPlease take this into account for further infractions by #R%s#r.#n", victim->pcdata->switchname, ch->name, reason, victim->pcdata->switchname);
    sprintf(buf2, "Official Warning for %s", victim->pcdata->switchname);
    make_note("Immortal", ch->name, "imm", "Official Warning", 30, buf);

    sprintf(buf, "You give %s an official warning for %s.\n\r", victim->name, reason);
    send_to_char(buf, ch);

    sprintf(buf, "%s has been warned by %s for %s.", victim->name, ch->name, reason);
    wiznet( NULL, WIZ_MISC, 0, buf );
    log_string(buf);

    return;
}

void do_unshadow(CHAR_DATA * ch, char * argument)
{
    CHAR_DATA * pVictim;

    if(argument[0] == '\0')
    {
        send_to_char("Syntax: unshadow <character>", ch);
        return;
    }

    pVictim = get_char_room(ch, argument);

    if(pVictim == NULL)
    {
        send_to_char("You can not find that person.\n\r", ch);
        return;
    }

    if(!IS_AFFECTED(pVictim, AFF_SHADOWPLANE) && !IS_AFFECTED(pVictim, AFF_ETHEREAL))
    {
        send_to_char("They are not in the shadow plane.\n\r", ch);
        return;
    }
    else
    {
        REMOVE_BIT(pVictim->affected_by, AFF_SHADOWPLANE);
        REMOVE_BIT(pVictim->affected_by, AFF_ETHEREAL);
        act("$n brings $N out of the shadowplane.\n\r", ch, NULL, pVictim, TO_NOTVICT);
        act("You bring $N out of the shadowplane.\n\r", ch, NULL, pVictim, TO_CHAR);
        act("$n brings you out of the shadowplane.\n\r", ch, NULL, pVictim, TO_VICT);
        return;
    }
}
