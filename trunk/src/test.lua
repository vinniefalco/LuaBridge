-- test lua script to be run with the luabridge test program

-- function to print contents of a table
function printtable (t)
	for k, v in pairs(t) do
		if (type(v) == "table") then
			print(k .. " =>", "(table)");
		elseif (type(v) == "function") then
			print(k .. " =>", "(function)");
		elseif (type(v) == "userdata") then
			print(k .. " =>", "(userdata)");
		else
			print(k .. " =>", v);
		end
	end
end

-- test functions registered from C++
testVoid();
print("testInt returned " .. testInt(47));
print("testFloat returned " .. testFloat(3.14159));
print("testConstCharPtr returned \"" .. testConstCharPtr("Hello World!") .. "\"");
print("testStdString returned \"" .. testStdString("Hello World!") .. "\"");

-- test classes registered from C++
object1 = A("object1");
print("object1:testInt returned " .. object1:testInt(47));
testAPtr(object1);
testAPtrConst(object1);
testConstAPtr(object1);
result = testSharedPtrA(object1);
print("testSharedPtrA returned " .. result:getName());

-- test subclass B of A
object2 = B("object2");
object2:testInt(47);
testAPtr(object2);
