// WinProductKey.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>

#include <Windows.h>

// Prototypes
std::string read_from_registry();
std::string decode( LPBYTE digitalProductId );

int main( int argc, char** argv )
{
    std::string key = read_from_registry();

    std::cout << key << std::endl;
}

std::string read_from_registry()
{
    std::string result;

    // TODO
    // - logic for 32 or 64 bit registry?
    // - pre or post Win8

    HKEY hKey;
    LPCWSTR subKey = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
    LPCWSTR valueName = L"DigitalProductId";

    // Open the key
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
    {
        // Get the item size
        DWORD valueSize = 0;
        if ( RegQueryValueEx( hKey, valueName, NULL, NULL, NULL, &valueSize ) == ERROR_SUCCESS )
        {
            // Allocate a buffer
            LPBYTE value = (LPBYTE) malloc( valueSize );

            // Read the value
            if ( RegQueryValueEx( hKey, valueName, NULL, NULL, (LPBYTE) value, &valueSize ) == ERROR_SUCCESS )
            {
                // Extract the key

                result = decode( value );
            }

            free( value );
        }
        RegCloseKey( hKey );
    }

    return result;
}

std::string decode( LPBYTE digitalProductId )
{
    std::string result;

    // Pseudocode from here:
    // https://answers.microsoft.com/en-us/windows/forum/all/how-to-recover-your-windows-product-key/8687ef5d-4d32-41fc-9310-158f8e5f02e3

    const int keyOffset = 52;
    const char* characters = "BCDFGHJKMPQRTVWXY2346789";
    
    BYTE isWin8 = (BYTE) ( ( digitalProductId[ 66 ] / 6 ) & 1 );
    digitalProductId[ 66 ] = (BYTE) ( ( digitalProductId[ 66 ] & 0xf7 ) | ( isWin8 & 2 ) * 4 );

    int last = 0;
    for ( int i = 24; i >= 0; i-- )
    {
        int current = 0;
        for ( int j = 14; j >= 0; j-- )
        {
            current = current * 256;
            current = digitalProductId[ j + keyOffset ] + current;
            digitalProductId[ j + keyOffset ] = (BYTE) ( current / 24 );
            current = current % 24;
            last = current;
        }

        result.insert( result.begin(), characters[ current ] );

    }

    std::string keypart1 = result.substr( 1, last );
    std::string keypart2 = result.substr( last + 1, result.length() - ( last + 1 ));
    result = keypart1 + "N" + keypart2;

    
    int hyphen = 0;
    for ( std::string::iterator it = result.begin(); it != result.end(); it++ )
    {
        hyphen++;

        if ( hyphen > 5 )
        {
            hyphen = 0;
            result.insert( it, '-' );
        }
    }


    return result;
}