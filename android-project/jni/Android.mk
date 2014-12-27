# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_memfile-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_memfile.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_primitives-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_primitives.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_image-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_image.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_font-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_font.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_ttf-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_ttf.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_color-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_color.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := liballegro_physfs-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liballegro_physfs.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libbass-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libbass.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libbassmidi-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/libbassmidi.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_ALLOW_UNDEFINED_SYMBOLS := false
LOCAL_MODULE    := crystalpicnic
LOCAL_SRC_FILES := abilities.cpp android.cpp animation.cpp animation_set.cpp area_loop.cpp area_manager.cpp astar.cpp astar_character_role.cpp bass.cpp bass_fileprocs.cpp battle_ai.cpp battle_entity.cpp battle_loop.cpp battle_pathfind.cpp battle_transition_in.cpp bitmap.cpp bones.cpp camera.cpp character_map_entity.cpp character_role.cpp collidable.cpp collision_detection.cpp config.cpp cpa.cpp credits_loop.cpp crystal_loop.cpp crystalpicnic.cpp difficulty_loop.cpp direct3d.cpp enemy_avatar.cpp enemy_avatar_wander_character_role.cpp engine.cpp entity.cpp error.cpp follow_character_role.cpp frame.cpp game_specific_globals.cpp general.cpp graphics.cpp hqm.c hqm_loop.cpp input_config_loop.cpp language_config_loop.cpp lua.cpp main.cpp main_menu_loop.cpp map_entity.cpp map_loop.cpp mt19937ar.c music.cpp my_load_bitmap.cpp npc.cpp particle.cpp player.cpp resource_manager.cpp runner_loop.cpp saveload_loop.cpp settings_loop.cpp shaders.cpp shop_loop.cpp skeleton.cpp sound.cpp speech_loop.cpp steering.cpp tls.cpp triangulate.cpp video_config_loop.cpp video_player.cpp wander_character_role.cpp weaponized_entity.cpp well512.c whack_a_skunk_loop.cpp widgets.cpp xml.cpp

LOCAL_ARM_MODE := arm
LOCAL_CFLAGS    := -I$(ANDROID_NDK_TOOLCHAIN_ROOT)/sysroot/usr/include -I../include -Wall

LOCAL_LDLIBS    := -L$(ANDROID_NDK_TOOLCHAIN_ROOT)/sysroot/usr/lib -L$(LOCAL_PATH)/$(TARGET_ARCH_ABI) -llog libs/$(TARGET_ARCH_ABI)/liballegro.so libs/$(TARGET_ARCH_ABI)/liballegro_memfile.so libs/$(TARGET_ARCH_ABI)/liballegro_primitives.so libs/$(TARGET_ARCH_ABI)/liballegro_image.so libs/$(TARGET_ARCH_ABI)/liballegro_font.so libs/$(TARGET_ARCH_ABI)/liballegro_ttf.so libs/$(TARGET_ARCH_ABI)/liballegro_color.so libs/$(TARGET_ARCH_ABI)/liballegro_physfs.so -lGLESv1_CM -llua5.2 -lz -lbass -lbassmidi -lphysfs -ltgui2 -lpoly2tri -latlas -lwrap -lcurl -lpng -lz -lstdc++

include $(BUILD_SHARED_LIBRARY)

