CPPFLAGS=-static-libgcc -static-libstdc++ -I..\include -ID:\trent\code\local\include -IC:\Progra~1\Micros~1\include -g -Wall -fcheck-new -Wno-unknown-pragmas -Wno-comment -Wno-strict-aliasing
CFLAGS=-static-libgcc -I..\include -ID:\trent\code\local\include -IC:\Progra~1\Micros~1\include -g -Wall -Wno-unknown-pragmas -Wno-comment -Wno-strict-aliasing -DWELL512=1
LDFLAGS=-static-libgcc -static-libstdc++ -LD:\trent\code\local\lib -lallegro_ttf-static -lallegro_font-static -lallegro_primitives-static -lallegro_color-static -lallegro_physfs-static -lallegro_memfile-static -lallegro-static -lphysfs -lz -llua5.2 -lopengl32 -ldxguid -ldinput8 -ldsound -ld3d9 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lole32 -lwinmm -lpsapi -lgdiplus -luuid -lshlwapi -lfreetype -lbass -lbass_fx -lbassmidi C:\Progra~1\Micros~1\Lib\x86\d3dx9.lib D:\trent\code\crystalpicnic\windows_icon.res

all: link

link: compile
	g++ $(CPPFLAGS) crystalpicnic.cpp -o ..\build\crystalpicnic.exe animation.o animation_set.o area_loop.o area_manager.o area_ui_button.o astar.o astar_character_role.o atlas.o atlas_accessor.o bass.o bass_fileprocs.o battle_ai.o battle_entity.o battle_loop.o battle_pathfind.o battle_transition_in.o bitmap.o bones.o button.o camera.o collidable.o collision_detection.o config.o enemy_avatar.o direct3d.o dpad.o earclip.o emoticon_map_entity.o engine.o entity.o error.o follow_character_role.o frame.o game_specific_globals.o general.o graphics.o loop.o lua.o main.o main_menu_loop.o map_entity.o map_loop.o music.o npc.o character_role.o particle.o player.o resource_manager.o runner_loop.o shaders.o skeleton.o sound.o speech_loop.o steering.o tgui2.o tgui2_widgets.o tls.o video_player.o visible_entity.o wander_character_role.o weaponized_entity.o whack_a_skunk_loop.o widgets.o wrap.o xml.o mt19937ar.o well512.o $(LDFLAGS)

compile: animation.o animation_set.o area_loop.o area_manager.o area_ui_button.o astar.o astar_character_role.o atlas.o atlas_accessor.o bass.o bass_fileprocs.o battle_ai.o battle_entity.o battle_loop.o battle_pathfind.o battle_transition_in.o bitmap.o bones.o button.o camera.o collidable.o collision_detection.o config.o enemy_avatar.o direct3d.o dpad.o earclip.o emoticon_map_entity.o engine.o entity.o error.o follow_character_role.o frame.o game_specific_globals.o general.o graphics.o loop.o lua.o main.o main_menu_loop.o map_entity.o map_loop.o music.o npc.o character_role.o particle.o player.o resource_manager.o runner_loop.o shaders.o skeleton.o sound.o speech_loop.o steering.o tgui2.o tgui2_widgets.o tls.o video_player.o visible_entity.o wander_character_role.o weaponized_entity.o whack_a_skunk_loop.o widgets.o wrap.o xml.o mt19937ar.o well512.o

animation.o: animation.cpp
	g++ $(CPPFLAGS) -c animation.cpp
animation_set.o: animation_set.cpp
	g++ $(CPPFLAGS) -c animation_set.cpp
area_loop.o: area_loop.cpp
	g++ $(CPPFLAGS) -c area_loop.cpp
area_manager.o: area_manager.cpp
	g++ $(CPPFLAGS) -c area_manager.cpp
area_ui_button.o: area_ui_button.cpp
	g++ $(CPPFLAGS) -c area_ui_button.cpp
astar.o: astar.cpp
	g++ $(CPPFLAGS) -c astar.cpp
astar_character_role.o: astar_character_role.cpp
	g++ $(CPPFLAGS) -c astar_character_role.cpp
atlas.o: atlas.cpp
	g++ $(CPPFLAGS) -c atlas.cpp
atlas_accessor.o: atlas_accessor.cpp
	g++ $(CPPFLAGS) -c atlas_accessor.cpp
bass.o: bass.cpp
	g++ $(CPPFLAGS) -c bass.cpp
bass_fileprocs.o: bass_fileprocs.cpp
	g++ $(CPPFLAGS) -c bass_fileprocs.cpp
battle_ai.o: battle_ai.cpp
	g++ $(CPPFLAGS) -c battle_ai.cpp
battle_entity.o: battle_entity.cpp
	g++ $(CPPFLAGS) -c battle_entity.cpp
battle_loop.o: battle_loop.cpp
	g++ $(CPPFLAGS) -c battle_loop.cpp
battle_pathfind.o: battle_pathfind.cpp
	g++ $(CPPFLAGS) -c battle_pathfind.cpp
battle_transition_in.o: battle_transition_in.cpp
	g++ $(CPPFLAGS) -c battle_transition_in.cpp
bitmap.o: bitmap.cpp
	g++ $(CPPFLAGS) -c bitmap.cpp
bones.o: bones.cpp
	g++ $(CPPFLAGS) -c bones.cpp
button.o: button.cpp
	g++ $(CPPFLAGS) -c button.cpp
camera.o: camera.cpp
	g++ $(CPPFLAGS) -c camera.cpp
collidable.o: collidable.cpp
	g++ $(CPPFLAGS) -c collidable.cpp
collision_detection.o: collision_detection.cpp
	g++ $(CPPFLAGS) -c collision_detection.cpp
config.o: config.cpp
	g++ $(CPPFLAGS) -c config.cpp
enemy_avatar.o: enemy_avatar.cpp
	g++ $(CPPFLAGS) -c enemy_avatar.cpp
direct3d.o: direct3d.cpp
	g++ $(CPPFLAGS) -c direct3d.cpp
dpad.o: dpad.cpp
	g++ $(CPPFLAGS) -c dpad.cpp
earclip.o: earclip.cpp
	g++ $(CPPFLAGS) -c earclip.cpp
emoticon_map_entity.o: emoticon_map_entity.cpp
	g++ $(CPPFLAGS) -c emoticon_map_entity.cpp
engine.o: engine.cpp
	g++ $(CPPFLAGS) -c engine.cpp
entity.o: entity.cpp
	g++ $(CPPFLAGS) -c entity.cpp
error.o: error.cpp
	g++ $(CPPFLAGS) -c error.cpp
follow_character_role.o: follow_character_role.cpp
	g++ $(CPPFLAGS) -c follow_character_role.cpp
frame.o: frame.cpp
	g++ $(CPPFLAGS) -c frame.cpp
game_specific_globals.o: game_specific_globals.cpp
	g++ $(CPPFLAGS) -c game_specific_globals.cpp
general.o: general.cpp
	g++ $(CPPFLAGS) -c general.cpp
graphics.o: graphics.cpp
	g++ $(CPPFLAGS) -c graphics.cpp
loop.o: loop.cpp
	g++ $(CPPFLAGS) -c loop.cpp
lua.o: lua.cpp
	g++ $(CPPFLAGS) -c lua.cpp
main.o: main.cpp
	g++ $(CPPFLAGS) -c main.cpp
main_menu_loop.o: main_menu_loop.cpp
	g++ $(CPPFLAGS) -c main_menu_loop.cpp
map_entity.o: map_entity.cpp
	g++ $(CPPFLAGS) -c map_entity.cpp
map_loop.o: map_loop.cpp
	g++ $(CPPFLAGS) -c map_loop.cpp
music.o: music.cpp
	g++ $(CPPFLAGS) -c music.cpp
npc.o: npc.cpp
	g++ $(CPPFLAGS) -c npc.cpp
character_role.o: character_role.cpp
	g++ $(CPPFLAGS) -c character_role.cpp
particle.o: particle.cpp
	g++ $(CPPFLAGS) -c particle.cpp
player.o: player.cpp
	g++ $(CPPFLAGS) -c player.cpp
resource_manager.o: resource_manager.cpp
	g++ $(CPPFLAGS) -c resource_manager.cpp
runner_loop.o: runner_loop.cpp
	g++ $(CPPFLAGS) -c runner_loop.cpp
shaders.o: shaders.cpp
	g++ $(CPPFLAGS) -c shaders.cpp
skeleton.o: skeleton.cpp
	g++ $(CPPFLAGS) -c skeleton.cpp
sound.o: sound.cpp
	g++ $(CPPFLAGS) -c sound.cpp
speech_loop.o: speech_loop.cpp
	g++ $(CPPFLAGS) -c speech_loop.cpp
steering.o: steering.cpp
	g++ $(CPPFLAGS) -c steering.cpp
tgui2.o: tgui2.cpp
	g++ $(CPPFLAGS) -c tgui2.cpp
tgui2_widgets.o: tgui2_widgets.cpp
	g++ $(CPPFLAGS) -c tgui2_widgets.cpp
tls.o: tls.cpp
	g++ $(CPPFLAGS) -c tls.cpp
video_player.o: video_player.cpp
	g++ $(CPPFLAGS) -c video_player.cpp
visible_entity.o: visible_entity.cpp
	g++ $(CPPFLAGS) -c visible_entity.cpp
wander_character_role.o: wander_character_role.cpp
	g++ $(CPPFLAGS) -c wander_character_role.cpp
weaponized_entity.o: weaponized_entity.cpp
	g++ $(CPPFLAGS) -c weaponized_entity.cpp
whack_a_skunk_loop.o: whack_a_skunk_loop.cpp
	g++ $(CPPFLAGS) -c whack_a_skunk_loop.cpp
widgets.o: widgets.cpp
	g++ $(CPPFLAGS) -c widgets.cpp
wrap.o: wrap.cpp
	g++ $(CPPFLAGS) -c wrap.cpp
xml.o: xml.cpp
	g++ $(CPPFLAGS) -c xml.cpp
mt19937ar.o: mt19937ar.c
	gcc $(CFLAGS) -c mt19937ar.c
well512.o: well512.c
	gcc $(CFLAGS) -c well512.c

clean:
	rm *.o
