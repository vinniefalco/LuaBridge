-- test lua script

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

-- test functions registered from C
hello();
print("pi = " .. pi());
godel(false);
print("3 squared = " .. square(3));
print("hello world uppercase = " .. upperCase("hello world"));

-- test classes registered from C
object1 = A("object1");
object2 = B("object2");
result = object1:foo(47, object2);
print("object1:foo returned " .. result);
print("Length of object1's name: " .. object1:bar());
result = object1:baz("returned created object");
result:foo(29, object2);
