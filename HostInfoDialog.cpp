/*  ATL dialog box implementation for host connection information.

    Copyright (C) 2008  Alexander Lamaison <awl03@doc.ic.ac.uk>

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
*/

#include "stdafx.h"
#include "HostInfoDialog.h"
#include "remotelimits.h"

#define FORBIDDEN_CHARS _T("@: \t\n\r\b\"'\\")
#define FORBIDDEN_PATH_CHARS _T("\"\t\n\r\b\\")

/**
 * Handle dialog initialisation by copying member data into Win32 fields.
 *
 * The member data may have been set using the @ref Accessors 
 * "accessor methods".  Once copied, these fields are validated and the 
 * dialog modified accordingly
 *
 * @pre the dialog must have been initialised by calling DoModal() or
 *      Create() before this function is called.  The fields must exist in
 *      order to copy data into them.
 *
 * @see SetUser() SetHost() SetPath() SetPort()
 * @see _HandleValidity()
 */
LRESULT CHostInfoDialog::OnInitDialog(MESSAGE_HANDLER_PARAMS)
{
	// Copy member data to Win32 object fields
	SetDlgItemText(IDC_USER, m_strUser);
	SetDlgItemText(IDC_HOST, m_strHost);
	SetDlgItemText(IDC_PATH, m_strPath);
	SetDlgItemInt(IDC_PORT, m_uPort, FALSE);

	// Check validity
	_HandleValidity();

	return 1;  // Let the system set the focus
}

/**
 * Handle a change event from one of the dialog fields.
 *
 * The data is the fields is revalidated and the dialog modified accordingly.
 *
 * @pre the dialog must have been initialised by calling DoModal() or
 *      Create() before this function is called.
 *
 * @see _HandleValidity()
 */
LRESULT CHostInfoDialog::OnChange(COMMAND_HANDLER_PARAMS)
{
	_HandleValidity();
	return 0;
}

/**
 * Handle the OK button click event by ending the dialog.
 *
 * The data in the Win32 dialog fields is copied to the member variables
 * thereby making it available to the accessor methods.
 *
 * @pre The _IsValid() function must have passed allowing _HandleValidity() to
 *      enable the OK button before this handler could be invoked.  This means 
 *      we don't need to check the fields.
 *
 * @pre The dialog must have been initialised by calling DoModal() or
 *      Create() before this function is called.  The fields must exist in
 *      order to copy data from them.
 *
 * @returns IDOK to the caller of DoModal()
 *
 * @see GetUser() GetHost() GetPath() GetPort()
 * @see OnCancel()
 */
LRESULT CHostInfoDialog::OnOK(WORD, WORD wID, HWND, BOOL&)
{
	// Get string fields
	GetDlgItemText(IDC_USER, m_strUser);
	GetDlgItemText(IDC_HOST, m_strHost);
	GetDlgItemText(IDC_PATH, m_strPath);

	// Get numeric field (port number)
	m_uPort = (USHORT)GetDlgItemInt(IDC_PORT, NULL, false);

	EndDialog(wID);
	return 0;
}

/**
 * Handle the Cancel button click event by ending the dialog.
 *
 * @pre the dialog must have been initialised by calling DoModal() or
 *      Create() before this function is called.  The dialog must exist before
 *      it can be terminated.
 *
 * @returns IDCANCEL to the caller of DoModal()
 *
 * @see OnOK()
 */
LRESULT CHostInfoDialog::OnCancel(WORD, WORD wID, HWND, BOOL&)
{
	EndDialog(wID);
	return 0;
}

/**
 * Checks if the values in the dialog box fields are valid.
 *
 * Criteria:
 * - The user name field must not be empty, must not contain more than 
 *   @ref MAX_USERNAME_LEN characters and must not contain any characters
 *   from @ref FORBIDDEN_CHARS.
 * - The host name field must not be empty, must not contain more than 
 *   @ref MAX_HOSTNAME_LEN characters and must not contain any characters
 *   from @ref FORBIDDEN_CHARS.
 * - The path field must not be empty, must not contain more than 
 *   @ref MAX_PATH_LEN characters and must not contain any characters
 *   from @ref FORBIDDEN_PATH_CHARS.
 * - The port field must contain a number between 0 and 65535 (@ref MAX_PORT).
 *
 * @pre the dialog must have been initialised by calling DoModal() or
 *      Create() before this function is called.  The fields must exist in
 *      order to check them.
 *
 * @returns
 *     @retval true if all fields meet the validity criteria.
 *     @retval false if any field fails the criteria.
 *
 * @todo The validity criteria are woefully inadequate:
 * - There are many characters that are not allowed in usernames or hostnames.
 *   The test should really check that characters are all from an allowed list
 *   rather than not from a forbidden list.
 * - Windows usernames can contain spaces.  These must be escaped.
 * - Paths can contain almost any character.  Some will have to be escaped.
 * @todo Use balloons or some other method to warn the user of problems as 
 *       they complete the fields.
 *
 * @see _HandleValidity()
 */
BOOL CHostInfoDialog::_IsValid()
{
	ASSERT(m_hWnd); // Must call DoModal() or Create() first

	// Check string fields
	CString strUser, strHost, strPath;
	GetDlgItemText(IDC_USER, strUser);
	GetDlgItemText(IDC_HOST, strHost);
	GetDlgItemText(IDC_PATH, strPath);
	if (strUser.IsEmpty() || strHost.IsEmpty() || strPath.IsEmpty())
		return false;
	if (strUser.FindOneOf(FORBIDDEN_CHARS) > -1 ||
	    strHost.FindOneOf(FORBIDDEN_CHARS) > -1 ||
		strPath.FindOneOf(FORBIDDEN_PATH_CHARS) > -1)
		return false;
	if (strUser.GetLength() > MAX_USERNAME_LEN ||
		strHost.GetLength() > MAX_HOSTNAME_LEN ||
		strPath.GetLength() > MAX_PATH_LEN)
		return false;

	// Check numeric field (port number)
	BOOL fTranslated;
	UINT uPort = GetDlgItemInt(IDC_PORT, &fTranslated, false);
	ASSERT( fTranslated );
	if (!fTranslated)
		return false;
	if (uPort > MAX_PORT)
		return false;

	return true;
}

/**
 * Disables the OK button if a field in the dialog is invalid.
 *
 * @pre the dialog must have been initialised by calling DoModal() or
 *      Create() before this function is called.  The fields must exist in
 *      order to check them.
 *
 * @todo Use balloons or some other method to warn the user of problems as 
 *       they complete the fields.
 *
 * @see OnOK()
 */
void CHostInfoDialog::_HandleValidity()
{
	ASSERT(m_hWnd); // Must call DoModal() or Create() first

	::EnableWindow(GetDlgItem(IDOK), _IsValid());
}

/**
 * Get value of the user name field.
 *
 * @pre the OK button must be clicked first in order to copy the data out
 *      of the Win32 field.
 * @returns the user name.
 * @todo copy the data directly from the field or update the member variables
 *       as the field value changes.
 */
CString CHostInfoDialog::GetUser()
{
	return m_strUser;
}

/**
 * Get value of the host name field.
 *
 * @pre the OK button must be clicked first in order to copy the data out
 *      of the Win32 field.
 * @returns the host name.
 * @todo copy the data directly from the field or update the member variables
 *       as the field value changes.
 */
CString CHostInfoDialog::GetHost()
{
	return m_strHost;
}

/**
 * Get value of the path field.
 *
 * @pre the OK button must be clicked first in order to copy the data out
 *      of the Win32 field.
 * @returns the path name.
 * @todo copy the data directly from the field or update the member variables
 *       as the field value changes.
 */
CString CHostInfoDialog::GetPath()
{
	return m_strPath;
}

/**
 * Get value of the port field.
 *
 * @pre the OK button must be clicked first in order to copy the data out
 *      of the Win32 field.
 * @returns the user name.
 * @todo copy the data directly from the field or update the member variables
 *       as the field value changes.
 */
USHORT CHostInfoDialog::GetPort()
{
	return m_uPort;
}


/**
 * Set the value to be loaded into the user name field when dialog is displayed.
 *
 * The value set using this function is copied into the Win32 dialog field
 * when the dialog is initialised.  This is done by the OnInitDialog() 
 * message handler which handle dialog initialisation.
 *
 * @see OnInitDialog()
 */
void CHostInfoDialog::SetUser( LPCTSTR pszUser )
{
	m_strUser = pszUser;
}

/**
 * Set the value to be loaded into the host name field when dialog is displayed.
 *
 * The value set using this function is copied into the Win32 dialog field
 * when the dialog is initialised.  This is done by the OnInitDialog() 
 * message handler which handle dialog initialisation.
 *
 * @see OnInitDialog()
 */
void CHostInfoDialog::SetHost( LPCTSTR pszHost )
{
	m_strHost = pszHost;
}

/**
 * Set the value to be loaded into the path field when dialog is displayed.
 *
 * The value set using this function is copied into the Win32 dialog field
 * when the dialog is initialised.  This is done by the OnInitDialog() 
 * message handler which handle dialog initialisation.
 *
 * @see OnInitDialog()
 */
void CHostInfoDialog::SetPath( LPCTSTR pszPath )
{
	m_strPath = pszPath;
}

/**
 * Set the value to be loaded into the port field when dialog is displayed.
 *
 * The value set using this function is copied into the Win32 dialog field
 * when the dialog is initialised.  This is done by the OnInitDialog() 
 * message handler which handle dialog initialisation.
 *
 * @see OnInitDialog()
 */
void CHostInfoDialog::SetPort( USHORT uPort )
{
	m_uPort = uPort;
}

// CHostInfoDialog
