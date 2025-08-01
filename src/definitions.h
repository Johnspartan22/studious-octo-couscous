/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2016  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FS_DEFINITIONS_H_877452FEC245450C9F96B8FD268D8963
#define FS_DEFINITIONS_H_877452FEC245450C9F96B8FD268D8963

static constexpr auto STATUS_SERVER_NAME = "Underworld OT";
static constexpr auto STATUS_SERVER_VERSION = "1";
static constexpr auto MINOR_VERSION = "3";
static constexpr auto REVISION_VERSION = "1";
static constexpr auto SOFTWARE_CODENAME = "Chaos 2";
static constexpr auto STATUS_SERVER_DEVELOPERS = "Bezos";
static constexpr auto GIT_REPO = "none";

static constexpr auto CLIENT_VERSION_ITEMS = 3;

#ifdef _MULTIPLATFORM77
static constexpr auto CLIENT_VERSION_MIN = 770;
static constexpr auto CLIENT_VERSION_MAX = 772;
static constexpr auto CLIENT_VERSION_STR = "7.72";
#else
static constexpr auto CLIENT_VERSION_MIN = 760;
static constexpr auto CLIENT_VERSION_MAX = 760;
static constexpr auto CLIENT_VERSION_STR = "7.60";
#endif

#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <cassert>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>
#include <chrono>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WIN32_LEAN_AND_MEAN

#ifdef _MSC_VER
#ifdef NDEBUG
#define _SECURE_SCL 0
#define HAS_ITERATOR_DEBUGGING 0
#endif

#pragma warning(disable:4127) // conditional expression is constant
#pragma warning(disable:4244) // 'argument' : conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4250) // 'class1' : inherits 'class2::member' via dominance
#pragma warning(disable:4267) // 'var' : conversion from 'size_t' to 'type', possible loss of data
#pragma warning(disable:4351) // new behavior: elements of array will be default initialized
#pragma warning(disable:4458) // declaration hides class member
#pragma warning(disable:4996)
#endif

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#ifndef _WIN32_WINNT
// 0x0602: Windows 7
#define _WIN32_WINNT 0x0602
#endif
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef std::vector<std::pair<uint32_t, uint32_t>> IPList;

#endif
