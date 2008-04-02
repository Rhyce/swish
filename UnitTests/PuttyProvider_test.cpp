//PuttyProvider_Test.cpp  -   defines the class PuttyProvider_Test

#include "stdafx.h"
#include "PuttyProvider_test.h"

#include <ATLComTime.h>

#define LISTING_PATH OLESTR("/tmp")

CPPUNIT_TEST_SUITE_REGISTRATION( CPuttyProvider_test );

void CPuttyProvider_test::setUp()
{
	HRESULT hr;

	// Start up COM
	hr = ::CoInitialize(NULL);
	CPPUNIT_ASSERT(SUCCEEDED(hr));

	// Test registry structure (for psftp.exe path - TODO: more?)
	testRegistryStructure();

	// Create instance of CPuttyProvider using CLSID
	hr = ::CoCreateInstance(
		CLSID_CPuttyProvider,     // CLASSID for CPuttyProvider.
        NULL,                     // Ignore this.
        CLSCTX_INPROC_SERVER,     // Server.
        IID_IPuttyProvider,       // Interface you want.
        (LPVOID *)&m_pProvider);  // Place to store interface.

	CPPUNIT_ASSERT(SUCCEEDED(hr));
}

/**
 * Test that the class responds to IUnknown::QueryInterface correctly.
 *
 * This test will be roughly the same for *any* valid COM object except
 * one that implement IShellView as this has been chosen to test failure. The
 * cases being tested are based on those explained by Raymond Chen:
 * http://blogs.msdn.com/oldnewthing/archive/2004/03/26/96777.aspx
 */
void CPuttyProvider_test::testQueryInterface()
{
	HRESULT hr;

	// Supports IUnknown (valid COM object)?
	IUnknown *pUnk;
	hr = m_pProvider->QueryInterface(__uuidof(IUnknown), (void **)&pUnk);
	CPPUNIT_ASSERT(SUCCEEDED(hr));
	pUnk->Release();

	// Supports IPuttyProvider (valid self!)?
	IPuttyProvider *pProv;
	hr = m_pProvider->QueryInterface(__uuidof(IPuttyProvider), (void **)&pProv);
	CPPUNIT_ASSERT(SUCCEEDED(hr));
	pProv->Release();

	// Says no properly (must return NULL)
	IShellView *pShell = (IShellView *)this; // Arbitrary non-null
	hr = m_pProvider->QueryInterface(__uuidof(IShellView), (void **)&pShell);
	if (SUCCEEDED(hr))
	{
		pShell->Release();
		CPPUNIT_ASSERT(FAILED(hr));
	}
	CPPUNIT_ASSERT(pShell == NULL);
}

void CPuttyProvider_test::testInitialize()
{
	BSTR bstrUser = GetUserName().AllocSysString();
	BSTR bstrHost = GetHostName().AllocSysString();

	CPPUNIT_ASSERT( SUCCEEDED(
		m_pProvider->Initialize( bstrUser, bstrHost, GetPort() )
	));

	::SysFreeString( bstrUser );
	::SysFreeString( bstrHost );
}

struct testFILEDATA
{
	BOOL fIsFolder;
	CString strPath;
	CString strOwner;
	CString strGroup;
	CString strAuthor;

	ULONGLONG uSize; // 64-bit allows files up to 16 Ebibytes (a lot)
	time_t dtModified;
	DWORD dwPermissions;
};

void CPuttyProvider_test::testGetListing()
{
	HRESULT hr;

	BSTR bstrUser = GetUserName().AllocSysString();
	BSTR bstrHost = GetHostName().AllocSysString();
	CPPUNIT_ASSERT( SUCCEEDED(
		m_pProvider->Initialize( bstrUser, bstrHost, GetPort() )
	));
	::SysFreeString( bstrUser );
	::SysFreeString( bstrHost );

	// Fetch listing enumerator
	IEnumListing *pEnum;
	BSTR bstrDirectory = ::SysAllocString(LISTING_PATH);
	hr = m_pProvider->GetListing(bstrDirectory, &pEnum);
	if (FAILED(hr))
		pEnum = NULL;
	::SysFreeString(bstrDirectory);
	CPPUNIT_ASSERT( SUCCEEDED(hr) );

	// Check format of listing is sensible
	CPPUNIT_ASSERT( SUCCEEDED(pEnum->Reset()) );
	Listing lt;
	hr = pEnum->Next(1, &lt, NULL);
	CPPUNIT_ASSERT_EQUAL( S_OK, hr );
	while (hr == S_OK)
	{
		CString strFilename(lt.bstrFilename),
		        strPermissions(lt.bstrPermissions),
				strOwner(lt.bstrOwner),
				strGroup(lt.bstrGroup);

		testFILEDATA fd;
		//::ZeroMemory(&fd, sizeof(fd));
		fd.strPath = lt.bstrFilename;
		fd.strOwner = lt.bstrOwner;
		fd.strGroup = lt.bstrGroup;
		fd.uSize = lt.cSize;
		fd.dtModified = (time_t) COleDateTime(lt.dateModified);

		CString strOwner2 = lt.bstrOwner;
		CPPUNIT_ASSERT( !strPermissions.IsEmpty() );
		CPPUNIT_ASSERT( !strFilename.IsEmpty() );

		CPPUNIT_ASSERT( lt.cHardLinks > 0 );
		CPPUNIT_ASSERT( lt.cSize >= 0 );
		CPPUNIT_ASSERT( !strOwner.IsEmpty() );
		CPPUNIT_ASSERT( !strGroup.IsEmpty() );

		CPPUNIT_ASSERT( lt.dateModified );
		COleDateTime dateModified( lt.dateModified );
		// Check year
		CPPUNIT_ASSERT( dateModified.GetYear() >= 1604 );
		CPPUNIT_ASSERT( 
			dateModified.GetYear() <= COleDateTime::GetCurrentTime().GetYear()
		);
		// Check month
		CPPUNIT_ASSERT( dateModified.GetMonth() > 0 );
		CPPUNIT_ASSERT( dateModified.GetMonth() <= 12 );
		// Check date
		CPPUNIT_ASSERT( dateModified.GetDay() > 0 );
		CPPUNIT_ASSERT( dateModified.GetDay() <= 31 );
		// Check hour
		CPPUNIT_ASSERT( dateModified.GetHour() >= 0 );
		CPPUNIT_ASSERT( dateModified.GetHour() <= 23 );
		// Check minute
		CPPUNIT_ASSERT( dateModified.GetMinute() >= 0 );
		CPPUNIT_ASSERT( dateModified.GetMinute() <= 59 );
		// Check second
		CPPUNIT_ASSERT( dateModified.GetSecond() >= 0 );
		CPPUNIT_ASSERT( dateModified.GetSecond() <= 59 );
		// Check overall validity
		CPPUNIT_ASSERT_EQUAL( COleDateTime::valid, dateModified.GetStatus() );

		// TODO: use old swish C permissions functions here
		CPPUNIT_ASSERT(
			strPermissions[0] == _T('d') ||
			strPermissions[0] == _T('b') ||
			strPermissions[0] == _T('c') ||
			strPermissions[0] == _T('l') ||
			strPermissions[0] == _T('p') ||
			strPermissions[0] == _T('s') ||
			strPermissions[0] == _T('-'));

		hr = pEnum->Next(1, &lt, NULL);
	}

	ULONG cRefs = pEnum->Release();
	CPPUNIT_ASSERT_EQUAL( (ULONG)0, cRefs );
}

void CPuttyProvider_test::tearDown()
{
	if (m_pProvider) // Possible for test to fail before m_pProvider initialised
	{
		ULONG cRefs = m_pProvider->Release();
		CPPUNIT_ASSERT_EQUAL( (ULONG)0, cRefs );
	}
	m_pProvider = NULL;

	// Shut down COM
	::CoUninitialize();
}

/*----------------------------------------------------------------------------*
 * Private functions
 *----------------------------------------------------------------------------*/

/**
 * Test that the HKCR\CLSID\{Class ID of CPuttyProvider}\InprocServer32 registry
 * key can be successfully opened and that the path stored there as the default
 * value can be used to compute the location of the PuTTY executable: psftp.exe
 */
void CPuttyProvider_test::testRegistryStructure() const
{
	// Construct subkey using CLSID as a string
	CString strSubkey;
	LPOLESTR pszCLSID = NULL;
	::StringFromCLSID( __uuidof(CPuttyProvider), &pszCLSID );
	strSubkey += _T("CLSID\\");
	strSubkey += pszCLSID;
	strSubkey += _T("\\InprocServer32");
	::CoTaskMemFree( pszCLSID );

	// Test opening registry key
	HKEY hKey;
	CPPUNIT_ASSERT(
		::RegOpenKey(HKEY_CLASSES_ROOT, strSubkey, &hKey) == ERROR_SUCCESS
	);
	CPPUNIT_ASSERT( ::RegCloseKey( hKey ) == ERROR_SUCCESS );

	// Get path of Swish DLL e.g. C:\Program Files\Swish\Swish.dll
	TCHAR szPath[MAX_PATH];
	CPPUNIT_ASSERT(
		::SHRegGetPath(HKEY_CLASSES_ROOT, strSubkey, 0, szPath, NULL)
		== ERROR_SUCCESS
	);

	// Use to contruct psftp path e.g. C:\Program Files\Swish\psftp.exe
	CPPUNIT_ASSERT( ::PathRemoveFileSpec(szPath) );
	CString strExePath(szPath);
	strExePath += _T("\\psftp.exe");
	CPPUNIT_ASSERT( ::PathFileExists(strExePath) );
}


/**
 * Get the host name of the machine to connect to for remote testing.
 *
 * The host name is retrieved from the TEST_HOST_NAME environment variable.
 * If this variable is not set, a CPPUNIT exception is thrown.
 * 
 * In order to be useful, the host name should exist and the machine 
 * should be accessible via SSH.
 * 
 * @pre the host name should have at least 3 characters
 * @pre the host name should have less than 255 characters
 *
 * @return the host name
 */
CString CPuttyProvider_test::GetHostName() const
{
	CString strHostName;
	/* static CString strHostName;

	if (strHostName.IsEmpty()) // Might be cached in static variable
	{*/
		if(!strHostName.GetEnvironmentVariable(_T("TEST_HOST_NAME")))
			CPPUNIT_FAIL("Please set TEST_HOST_NAME environment variable");
	//}

	CPPUNIT_ASSERT(!strHostName.IsEmpty());
	CPPUNIT_ASSERT(strHostName.GetLength() > 2);
	CPPUNIT_ASSERT(strHostName.GetLength() < 255);
	
	return strHostName;
}

/**
 * Get the user name of the SSH account to connect to on the remote machine.
 *
 * The user name is retrieved from the TEST_USER_NAME environment variable.
 * If this variable is not set, a CPPUNIT exception is thrown.
 * 
 * In order to be useful, the user name should correspond to a valid SSH
 * account on the testing machine.
 * 
 * @pre the user name should have at least 3 characters
 * @pre the user name should have less than 64 characters
 *
 * @return the user name
 */
CString CPuttyProvider_test::GetUserName() const
{
	CString strUser;
	/*static CString strUser;

	if (strUser.IsEmpty()) // Might be cached in static variable
	{*/
		if(!strUser.GetEnvironmentVariable(_T("TEST_USER_NAME")))
			CPPUNIT_FAIL("Please set TEST_USER_NAME environment variable");
	//}

	CPPUNIT_ASSERT(!strUser.IsEmpty());
	CPPUNIT_ASSERT(strUser.GetLength() > 2);
	CPPUNIT_ASSERT(strUser.GetLength() < 64);
	
	return strUser;
}

/**
 * Get the port to connect to on the remote testing machine.
 *
 * The port is retrieved from the TEST_HOST_PORT environment variable.
 * If this variable is not set, the default SSH port 22 is returned.
 * 
 * In order to be useful, the machine should be accessible via SSH on
 * this port.
 * 
 * @post the port should be between 0 and 65535 inclusive
 *
 * @return the port number
 */
USHORT CPuttyProvider_test::GetPort() const
{
	/* static */ CString strPort;

	//if (strPort.IsEmpty())
	//{
		if(!strPort.GetEnvironmentVariable(_T("TEST_HOST_PORT")))
			return 22; // Default SSH port
	//}
	CPPUNIT_ASSERT(!strPort.IsEmpty());

	int uPort = ::StrToInt(strPort);
	CPPUNIT_ASSERT(uPort >= 0);
	CPPUNIT_ASSERT(uPort <= 65535);

	return (USHORT)uPort;
}