#include <iostream>
#include "iniparser.h"

// User-defined structure that can be loaded to\from INI
struct test_val
{
	test_val():a(0),b(0.0),c(0){}
	test_val(int pa, double pb, int pc):a(pa),b(pb),c(pc){}
	int a;
	double b;
	int c;
	bool operator== (const test_val& rt) const
	{
        return (a == rt.a && b == rt.b && c == rt.c);
	}
	bool operator!= (const test_val& rt) const {return !(*this == rt);}
};

// You need to declare these 2 operators to use your classes with Leksys' INIParser
std::ostream& operator<< (std::ostream& stream, const test_val& value)
{
	stream << '{' << value.a << ',' << value.b << ',' << value.c << '}';
	return stream;
}
std::istream& operator>> (std::istream& stream, test_val& value)
{
	char sb;
	stream >> sb;
	if (sb != '{') return stream;
	stream >> value.a >> sb >> value.b >> sb >> value.c >> sb;
	if (sb != '}') {value.a = 0; value.b = 0.0; value.c = 0;}
	return stream;
}

void show_help()
{
	std::cout << "Leksys' INIParser test program\n"
		"Usage: test_app [-i input_file] [-o output_file]\n"
		"Pass [input_file] for optional loading file test (TEST11)\n"
		"Pass [output_file] for optional saving file test (TEST12)\n"
		"---------------------------------------------------------"
		<< std::endl;
}
int main(int argc, char** argv)
{
    iniparser::File ft, ft2;
	std::string input_file;
	std::string output_file;
	show_help();
	// Bad way to parse input parameters, but why bother
	for (int i = 1; i < argc; i+=2)
	{
		std::string param = argv[i];
		if (param.size() != 2 || param[0] != '-' || i == argc - 1)
		{
			std::cerr << "ERROR! Wrong parameter " << param << std::endl;
			return -1;
		}
		if (param[1] == 'i')
			input_file = argv[i+1];
		else if (param[1] == 'o')
			output_file = argv[i+1];
		else
		{
			std::cerr << "ERROR! Unknown parameter " << param << std::endl;
			return -1;
		}
	}
	
	// [TEST1] Get & Set INT value through section pointer
	ft.getSection("MainProg")->setValue("value1",456);
	if (ft.getSection("MainProg")->getValue("value1").asInt() != 456 ||
		ft.getSection("MainProg")->name() != "MainProg")
	{
		std::cerr << "Failed to pass [TEST1]" << std::endl;
		return 1;
	}
	std::cout << "[TEST1] passed" << std::endl;
	
	// [TEST2] Get & Set boolean value through File object
	ft.setValue("MainProg:value2_0",false);
	ft.setValue("MainProg:value2_1",true);
	ft.setValue("MainProg:value2_2",0);
	ft.setValue("MainProg:value2_3",1);
	ft.setValue("MainProg:value2_4","FALSE");
	ft.setValue("MainProg:value2_5","TRUE");
	ft.setValue("MainProg:value2_6","false");
	ft.setValue("MainProg:value2_7","true");
	if (ft.getValue("MainProg:value2_0").asBool() || !ft.getValue("MainProg:value2_1").asBool() ||
		ft.getValue("MainProg:value2_2").asBool() || !ft.getValue("MainProg:value2_3").asBool() ||
		ft.getValue("MainProg:value2_4").asBool() || !ft.getValue("MainProg:value2_5").asBool() ||
		ft.getValue("MainProg:value2_6").asBool() || !ft.getValue("MainProg:value2_7").asBool())
	{
		std::cerr << "Failed to pass [TEST2]" << std::endl;
		return 2;
	}
	std::cout << "[TEST2] passed" << std::endl;

	// [TEST3] Set array value
	ft.setValue("MainProg:value3",iniparser::Array() << true << 100 << 200.55 << "String");
	iniparser::Array value3 = ft.getSection("MainProg")->getValue("value3").asArray();
	if (!value3[0].asBool() || value3[1].asInt() != 100 || 
		 value3[2].asDouble() != 200.55 || value3[3].asString() != "String")
	{
		std::cerr << "Failed to pass [TEST3]" << std::endl;
		return 3;
	}
	std::cout << "[TEST3] passed" << std::endl;

	// [TEST4] Direct access to array values
	ft.setArrayValue("MainProg:value3",1,105);
	ft.getSection("MainProg")->setArrayValue("value3",6,700.67);
	if (ft.getValue("MainProg:value3").asArray()[1].asInt() != 105 ||
		ft.getSection("MainProg")->getValue("value3").asArray().getValue(6).asDouble() != 700.67)
	{
		std::cerr << "Failed to pass [TEST4]" << std::endl;
		return 4;
	}
	std::cout << "[TEST4] passed" << std::endl;

	// [TEST5] Comments
	ft.getSection("MainProg")->setComment("value3","Test array");
	ft.setValue("MainProg2:value1", 1, "Comment 2");
	if (ft.getSection("MainProg")->getComment("value3") != "Test array" ||
		ft.getSection("MainProg2")->getComment("value1") != "Comment 2")
	{
		std::cerr << "Failed to pass [TEST5]" << std::endl;
		return 5;
	}
	std::cout << "[TEST5] passed" << std::endl;

	// [TEST6] Section iteration using iterator
	// Add key "value_test" with value "1" to all of the sections, existing so far (MainProg & MainProg2)
	for (iniparser::File::SectionIterator it = ft.sectionsBegin(); it != ft.sectionsEnd(); it++)
		it->second->setValue("value_test",1,"Testing value");
	// Read those keys
	for (iniparser::File::SectionIterator it = ft.sectionsBegin(); it != ft.sectionsEnd(); it++)
	{
		if (it->second->getValue("value_test",0).asInt() != 1)
		{
			std::cerr << "Failed to pass [TEST6]" << std::endl;
			return 6;
		}
	}
	std::cout << "[TEST6] passed" << std::endl;

	// [TEST7] Parent & Child Sections
	ft.getSection("MainProg2")->getSubSection("Sub1")->getSubSection("SubSub1")->setValue("value1",120);
	if (ft.getValue("MainProg2.Sub1.SubSub1:value1").asInt() != 120 || 
		ft.getSection("MainProg2.Sub1")->getSubSection("SubSub1")->getValue("value1").asInt() != 120)
	{
		std::cerr << "Failed to pass [TEST7]" << std::endl;
		return 7;
	}
	std::cout << "[TEST7] passed" << std::endl;

	// [TEST8] User-defined class storage
	test_val tp(1,120.555,3);
	ft.getSection("MainProg2.Sub1")->setValue("test_val",tp,"User-defined class");
	if (tp != ft.getValue("MainProg2.Sub1:test_val").asT<test_val>())
	{
		std::cerr << "Failed to pass [TEST8]" << std::endl;
		return 8;
	}
	std::cout << "[TEST8] passed" << std::endl;

	// [TEST9] Saving and loading files + values and section iteration (main test)
	std::stringstream stream;
	stream << ft;
	stream >> ft2;
	if (ft2.sectionsSize() != ft.sectionsSize())
	{
		std::cerr << "Failed to pass [TEST9]" << std::endl;
		return 9;
	}
	for (iniparser::File::SectionIterator it = ft2.sectionsBegin(); it != ft2.sectionsEnd(); it++)
	{
		iniparser::Section* sect1 = it->second;
		iniparser::Section* sect2 = ft.getSection(sect1->fullName());
		if (sect1->valuesSize() != sect2->valuesSize() || sect1->comment() != sect2->comment())
		{
			std::cerr << "Failed to pass [TEST9]" << std::endl;
			return 9;
		}
		for (iniparser::Section::ValueIterator vit = sect1->valuesBegin(); vit != sect1->valuesEnd(); vit++)
		{
			if (vit->second != sect2->getValue(vit->first) || sect1->getComment(vit->first) != sect2->getComment(vit->first))
			{
				std::cerr << "Failed to pass [TEST9]" << std::endl;
				return 9;
			}
		}
	}
	std::cout << "[TEST9] passed" << std::endl;

	// [TEST10] Saving and loading sections + Unload feature
	stream << ft.getSection("MainProg2.Sub1");
	ft2.unload();
	stream >> ft2;
	// Only 1 section was saved (parent is not created automatically as well as child sections)
	if (ft2.sectionsSize() != 1)
	{
		std::cerr << "Failed to pass [TEST10]" << std::endl;
		return 10;
	}
	iniparser::Section* sect = ft2.getSection("MainProg2")->getSubSection("Sub1");
	if (sect->valuesSize() != 1 || sect->getValue("test_val").asT<test_val>() != tp)
	{
		std::cerr << "Failed to pass [TEST10]" << std::endl;
		return 10;
	}
	std::cout << "[TEST10] passed" << std::endl;

	// [TEST11] Complex arrays test
	ft.setValue("MainProg2:cmp_array1", "{12,3}, {13,5}, {18,9} ");
	ft.setValue("MainProg2:cmp_array2", "Str1, Str2, {{Str3,,3}}, Str4\\,\\,\\{\\,\\,\\}, {Str5\\,,\\{\\{}");
	{
		iniparser::Value val = ft.getSection("MainProg2")->getValue("cmp_array1");
		if (val.asArray()[1].asArray()[0].asInt() != 13 || val.asArray()[2].asArray()[1].asInt() != 9)
		{
			std::cerr << "Failed to pass [TEST11] (check 1)" << std::endl;
			return 11;
		}
		val = ft.getSection("MainProg2")->getValue("cmp_array2");
		val = val.asArray().toValue();
		if (val.asArray()[1].asString() != "Str2" || val.asArray()[2].asString() != "{Str3,,3}"
			|| val.asArray()[3].asString() != "Str4,,{,,}" || val.asArray()[4].asString() != "Str5,,{{")
		{
			std::cerr << "Failed to pass [TEST11] (check 2)" << std::endl;
			return 11;
		}
	}
	std::cout << "[TEST11] passed" << std::endl;

	// [TEST12] Map test
	ft.setValue("MainProg2:map1","1:5, 1:3, 2:9, 3:10");
	ft.setValue("MainProg2:map2","Str1:5, Str2:3, Str3:{:,,:}");
	ft.setValue("MainProg2:map3","Str1:{Sub1,{\\,,\\\\{\\\\{Sub2,},{,:Sub3}}, Str2:1");
	{
		iniparser::Value val = ft.getSection("MainProg2")->getValue("map1");
		val = val.asMap().toValue();
		if (val.asMap()[2].asInt() != 9 || val.asMap()[1].asInt() != 3)
		{
			std::cerr << "Failed to pass [TEST12] (check 1)" << std::endl;
			return 12;
		}
		val = ft.getSection("MainProg2")->getValue("map2");
		iniparser::Map mp = val.asMap();
		if (val.asMap()["Str2"].asInt() != 3 || val.asMap()["Str3"].asString() != ":,,:")
		{
			std::cerr << "Failed to pass [TEST12] (check 2)" << std::endl;
			return 12;
		}
		val = ft.getSection("MainProg2")->getValue("map3");
		mp = val.asMap().toValue().asMap();
		std::string str = val.asMap()["Str1"].asArray()[1].asString();
		if (val.asMap()["Str1"].asArray()[1].asString() != ",,{{Sub2," ||
			val.asMap()["Str1"].asArray()[2].asString() != ",:Sub3" ||
			val.asMap()["Str2"].asInt() != 1)
		{
			std::cerr << "Failed to pass [TEST12] (check 3)" << std::endl;
			return 12;
		}
	}
	std::cout << "[TEST12] passed" << std::endl;

	// [TEST13] Optional. File loading
	if (!input_file.empty())
	{
		if (!ft.load(input_file))
		{
			std::cerr << "Failed to pass optional [TEST13] (file loading). Error: " 
				<< ft.lastResult().getErrorDesc() << std::endl;
			return 13;
		}
		std::cout << "[TEST13] passed!" << std::endl;
	}
	else
		std::cout << "Optional [TEST13] (file loading) SKIPPED" << std::endl;

	// [TEST14] Optional. File saving
	if (!output_file.empty())
	{
		if (!ft.save(output_file))
		{
			std::cerr << "Failed to pass optional [TEST14] (file saving). Wrong output file. File contents comes next"
				<< std::endl << ft;
			return 14;
		}
		std::cout << "[TEST14] passed! (Look in " << output_file << " for your file)!" << std::endl;
	}
	else
		std::cout << "Optional [TEST14] (file saving) SKIPPED" << std::endl;

	// [SUCCESS]
	std::cout << "!!! [SUCCESS] !!! " << std::endl;
    return 0;
}
