#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "document.h"
#include "prettywriter.h"
#include "filestream.h"
#include <cstdio>

using namespace std;
using namespace rapidjson;

class RapidJsonTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(RapidJsonTest);
	CPPUNIT_TEST(Scenario);
	CPPUNIT_TEST_SUITE_END();

public:

	void Scenario()
	{
		////////////////////////////////////////////////////////////////////////////
		// 1. Parse a JSON text string to a document.

		const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
		printf("Original JSON:\n %s\n", json);

		Document document;	// Default template parameter uses UTF8 and MemoryPoolAllocator.

#if 0
		// "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream().
		if (document.Parse<0>(json).HasParseError())
			CPPUNIT_FAIL("parse error!");
#else
		// In-situ parsing, decode strings directly in the source string. Source must be string.
		char buffer[sizeof(json)];
		memcpy(buffer, json, sizeof(json));
		if (document.ParseInsitu<0>(buffer).HasParseError())
			CPPUNIT_FAIL("parse error!");
#endif

		printf("\nParsing to document succeeded.\n");
		
		////////////////////////////////////////////////////////////////////////////
		// 2. Access values in document. 

		printf("\nAccess values in document:\n");
		//assert(document.IsObject());	// Document is a JSON value represents the root of DOM. Root can be either an object or array.
		CPPUNIT_ASSERT(document.IsObject());

		CPPUNIT_ASSERT(document.HasMember("hello"));
		CPPUNIT_ASSERT(document["hello"].IsString());
		printf("hello = %s\n", document["hello"].GetString());

		CPPUNIT_ASSERT(document["t"].IsBool());		// JSON true/false are bool. Can also uses more specific function IsTrue().
		printf("t = %s\n", document["t"].GetBool() ? "true" : "false");

		CPPUNIT_ASSERT(document["f"].IsBool());
		printf("f = %s\n", document["f"].GetBool() ? "true" : "false");

		printf("n = %s\n", document["n"].IsNull() ? "null" : "?");

		CPPUNIT_ASSERT(document["i"].IsNumber());	// Number is a JSON type, but C++ needs more specific type.
		CPPUNIT_ASSERT(document["i"].IsInt());		// In this case, IsUint()/IsInt64()/IsUInt64() also return true.
		printf("i = %d\n", document["i"].GetInt());	// Alternative (int)document["i"]

		CPPUNIT_ASSERT(document["pi"].IsNumber());
		CPPUNIT_ASSERT(document["pi"].IsDouble());
		printf("pi = %g\n", document["pi"].GetDouble());

		{
			const Value& a = document["a"];	// Using a reference for consecutive access is handy and faster.
			CPPUNIT_ASSERT(a.IsArray());
			for (SizeType i = 0; i < a.Size(); i++)	// rapidjson uses SizeType instead of size_t.
				printf("a[%d] = %d\n", i, a[i].GetInt());
		
			// Note:
			//int x = a[0].GetInt();					// Error: operator[ is ambiguous, as 0 also mean a null pointer of const char* type.
			int y = a[SizeType(0)].GetInt();			// Cast to SizeType will work.
			int z = a[0u].GetInt();						// This works too.
			(void)y;
			(void)z;
		}

		// 3. Modify values in document.

		// Change i to a bigger number
		{
			uint64_t f20 = 1;	// compute factorial of 20
			for (uint64_t j = 1; j <= 20; j++)
				f20 *= j;
			document["i"] = f20;	// Alternate form: document["i"].SetUint64(f20)
			CPPUNIT_ASSERT(!document["i"].IsInt());	// No longer can be cast as int or uint.
		}

		// Adding values to array.
		{
			Value& a = document["a"];	// This time we uses non-const reference.
			Document::AllocatorType& allocator = document.GetAllocator();
			for (int i = 5; i <= 10; i++)
				a.PushBack(i, allocator);	// May look a bit strange, allocator is needed for potentially realloc. We normally uses the document's.

			// Fluent API
			a.PushBack("Lua", allocator).PushBack("Mio", allocator);
		}

		// Making string values.

		// This version of SetString() just store the pointer to the string.
		// So it is for literal and string that exists within value's life-cycle.
		{
			document["hello"] = "rapidjson";	// This will invoke strlen()
			// Faster version:
			// document["hello"].SetString("rapidjson", 9);
		}

		// This version of SetString() needs an allocator, which means it will allocate a new buffer and copy the the string into the buffer.
		Value author;
		{
			char buffer[10];
			int len = sprintf(buffer, "%s %s", "Milo", "Yip");	// synthetic example of dynamically created string.

			author.SetString(buffer, len, document.GetAllocator());
			// Shorter but slower version:
			// document["hello"].SetString(buffer, document.GetAllocator());

			// Constructor version: 
			// Value author(buffer, len, document.GetAllocator());
			// Value author(buffer, document.GetAllocator());
			memset(buffer, 0, sizeof(buffer)); // For demonstration purpose.
		}
		// Variable 'buffer' is unusable now but 'author' has already made a copy.
		document.AddMember("author", author, document.GetAllocator());

		CPPUNIT_ASSERT(author.IsNull());		// Move semantic for assignment. After this variable is assigned as a member, the variable becomes null.

		////////////////////////////////////////////////////////////////////////////
		// 4. Stringify JSON

		printf("\nModified JSON with reformatting:\n");
		FileStream f(stdout);
		PrettyWriter<FileStream> writer(f);
		document.Accept(writer);	// Accept() traverses the DOM and generates Handler events.
	}
	
};

CPPUNIT_TEST_SUITE_REGISTRATION(RapidJsonTest);

// vim:sw=4 ts=4 smarttab autoindent
