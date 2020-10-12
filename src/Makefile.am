bin_PROGRAMS = vampiromud
vampiromud_SOURCES = ackmapper.c act_comm.c act_info.c act_move.c act_obj.c act_wiz.c automob.c autoobj.c ban.c bank.c boards.c build.c clans.c colorize.c comm.c comments.c conquistas.c const.c db.c declarar.c deity.c disciplinas2.c esferas.c fight.c finger.c grub.c handler.c hashstr.c healer.c hiscores.c ibuild.c ident.c imm_host.c interp.c limite.c magic.c makeobjs.c medit.c mejorar.c misc.c mpxset.c mud_comm.c mud_prog.c newarena.c notas.c oedit.c planes.c player.c polymorph.c quest.c rare.c redit.c renacimiento.c requests.c reset.c revisiones.c save.c services.c shops.c skills.c skills_disciplinas.c special.c stat_obj.c tables.c taquillas.c track.c update.c vampireroll.c vampiros.c familias.c fam_misc.c mapout.c vmudSql.c

# set the include path found by configure
INCLUDES= $(all_includes)

# the library search path.
vampiromud_LDFLAGS = $(all_libraries) 
noinst_HEADERS = ackmapper.h bank.h bet.h board.h cabecera.h conquistas.h editor.h finger.h mud.h planes.h rare.h torneos.h familias.h database.h vmudSql.h
vampiromud_LDADD = -lmysqlclient -lcrypt
