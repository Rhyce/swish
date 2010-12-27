/**
    @file

    Reporting exceptions to the user.

    @if license

    Copyright (C) 2010  Alexander Lamaison <awl03@doc.ic.ac.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    @endif
*/

#ifndef SWISH_SHELL_FOLDER_ANNOUNCE_EXCEPTION_HPP
#define SWISH_SHELL_FOLDER_ANNOUNCE_EXCEPTION_HPP
#pragma once

#include <string>

#include <WinDef.h> // HWND

namespace swish {
namespace shell_folder {

void announce_error(
	HWND hwnd, const std::wstring& title, const std::wstring& details);

__declspec(noreturn) void rethrow_and_announce(
	HWND hwnd, const std::wstring& title);

}} // namespace swish::shell_folder

#endif
