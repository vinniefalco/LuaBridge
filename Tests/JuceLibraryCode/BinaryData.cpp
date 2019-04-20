/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file, created by The Introjucer 3.0.0
   Do not edit anything in this file!

*/

namespace BinaryData
{

//================== Tests.lua ==================
static const unsigned char temp_b6c6a671[] =
"-- test lua script to be run with the luabridge test program\r\n"
"\r\n"
"print(\"Running LuaBridge tests:\");\r\n"
"\r\n"
"-- enum from C++\r\n"
"FN_CTOR = 0\r\n"
"FN_DTOR = 1\r\n"
"FN_STATIC = 2\r\n"
"FN_VIRTUAL = 3\r\n"
"FN_PROPGET = 4\r\n"
"FN_PROPSET = 5\r\n"
"FN_STATIC_PROPGET = 6\r\n"
"FN_STATIC_PROPSET = 7\r\n"
"FN_OPERATOR = 8\r\n"
"NUM_FN_TYPES = 9\r\n"
"\r\n"
"-- test static methods of classes registered from C++\r\n"
"A.testStaticProp = 48;          assert(A.testStaticProp == 48);\r\n"
"\r\n"
"print(\"All tests succeeded.\");\r\n";

const char* Tests_lua = (const char*) temp_b6c6a671;


const char* getNamedResource (const char*, int&) throw();
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()
{
    unsigned hash = 0;
    if (resourceNameUTF8 != 0)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + *resourceNameUTF8++;

    switch (hash)
    {
        case 0x322b48ba:
        case 0xeec98b6c:  numBytes = 3877; return Tests_lua;
        default: break;
    }

    numBytes = 0;
    return 0;
}

}
